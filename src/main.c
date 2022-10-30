#include "nrf_gpio.h"
#include "nrfx_uart.h"
#include "nrfx_clock.h"
#include "ulog/ulog.h"

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define TX_PIN NRF_GPIO_PIN_MAP(0, 6)
#define RX_PIN NRF_GPIO_PIN_MAP(0, 8)
#define RTS_PIN NRF_GPIO_PIN_MAP(0, 5)
#define CTS_PIN NRF_GPIO_PIN_MAP(0, 7)
#define BTN1_PIN NRF_GPIO_PIN_MAP(0, 11)

#ifndef MAX
    #define MAX(A, B) ((A) > (B) ? (A) : (B))
#endif

#ifndef MIN
    #define MIN(A, B) ((A) < (B) ? (A) : (B))
#endif

static const nrfx_uart_t UART0 = NRFX_UART_INSTANCE(0);

int _write(int handle, char *data, int size )
{
    (void)handle;
    nrfx_uart_tx(&UART0, data, size);
    return size;
}

static void on_clock_event(nrfx_clock_evt_type_t event)
{
    (void)event;
}

int ulog_backend_init(void* arg)
{
    (void)arg;
    return 0;
}

int ulog_backend_tx(uint8_t const* data, size_t size)
{
    (void)nrfx_uart_tx(&UART0, data, size);
    return size;
}

void ulog_backend_deinit(void)
{
}

static inline void dwt_init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    DWT->CYCCNT = 0;
}

static inline void dwt_deinit(void)
{
    DWT->CTRL &= ~DWT_CTRL_CYCCNTENA_Msk;
    CoreDebug->DEMCR &= ~CoreDebug_DEMCR_TRCENA_Msk;
}

static inline void dwt_cyccnt_reset(void)
{
    if ((DWT->CTRL & DWT_CTRL_CYCCNTENA_Msk) == 0)
    {
        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    }
    DWT->CYCCNT = 0;
}

static inline uint32_t dwt_cyccnt(void)
{
    return DWT->CYCCNT;
}

struct bench_step {
    char const* name;
    void (*func)(void);
};

struct bench_suit {
    char const* name;
    void (*setup)(struct bench_suit const* const suit);
    void (*execute)(struct bench_suit const* const suit);
    struct bench_step const* const* const steps;
    void (*teardown)(struct bench_suit const* const suit);
};

void bench_setup(struct bench_suit const* const suit) {
    printf("#SETUP SUIT[%s]\n", suit->name);
    dwt_init();
}

void bench_teardown(struct bench_suit const* const suit) {
    printf("#TEARDOWN SUIT[%s]\n", suit->name);
}

void bench_execute(struct bench_suit const* const suit) {
    struct bench_step const* step = suit->steps[0];

    for (size_t i = 0; step != NULL; ++i)
    {
        step = suit->steps[i];
        if (step != NULL && step->func != NULL)
        {
            uint32_t time_us = 0;

            #if 0
                const size_t avgs = 4;
                uint32_t min_time_us = UINT32_MAX;
                uint32_t max_time_us = 0;
                uint32_t avg_time_us = 0;

                for (size_t avg = 0; avg < avgs; ++avg)
                {
                    DWT->CYCCNT = 0;
                    step->func();
                    time_us = (DWT->CYCCNT) >> 6;

                    min_time_us = MIN(min_time_us, time_us);
                    max_time_us = MAX(max_time_us, time_us);
                    avg_time_us += time_us;

                    printf("\n");
                }

                printf("#STEP[%u] %s: avg(%lu) us min(%lu) us max(%lu) us\n",
                        i,
                        step->name != NULL ? step->name : "",
                        avg_time_us / avgs,
                        min_time_us,
                        max_time_us);
            #else
                // DWT->CYCCNT = 0;
                dwt_cyccnt_reset();
                step->func();
                //time_us = dwt_cyccnt() >> 6; /* Divide # of cycles with 64 */
                time_us = dwt_cyccnt() / (SystemCoreClock / (1000 * 1000)); /* Divide # of cycles with 64 */

                printf("\n");
                printf("#STEP[%u] %s: %lu us\n",
                        i,
                        step->name != NULL ? step->name : "",
                        time_us);
            #endif
        }
    }
}

void bench_printf_hello_world(void) { printf("Hello World\n"); }
void bench_ulog_hello_world(void) { ULOG_DBG("Hello World"); }
void bench_printf_arg1(void) { printf("%d\n", 42); }
void bench_ulog_arg1(void) { ULOG_DBG("%d", 42); }
void bench_printf_arg3(void) { printf("%d %d %d\n", 42, 43, 44); }
void bench_ulog_arg3(void) { ULOG_DBG("%d %d %d", 42, 43, 44); }
void bench_printf_text_len64(void) { printf("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vivam\n"); }
void bench_ulog_text_len64(void) { ULOG_DBG("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vivamn"); }

static const uint8_t hexdump_data[64] = {
    0x00, 0x00, 0x04, 0x20, 0x1d, 0x08, 0x00, 0x00, 0x45, 0x08, 0x00, 0x00, 0x47, 0x08, 0x00, 0x00,
    0x49, 0x08, 0x00, 0x00, 0x4b, 0x08, 0x00, 0x00, 0x4d, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4f, 0x08, 0x00, 0x00,
    0x51, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x53, 0x08, 0x00, 0x00, 0x55, 0x08, 0x00, 0x00,
};

void bench_printf_hexdump(void) {
    for (size_t i = 0; i < sizeof(hexdump_data); ++i)
    {
        if((i != 0) && ((i & 0xf) == 0))
            printf("\n");
        printf("%02X ", hexdump_data[i]);
    }
}

void bench_ulog_hexdump(void) { ULOG_HEXDUMP_DBG(hexdump_data, sizeof(hexdump_data), ""); }

static const struct bench_suit bsuit = {
    .name = "Printf vs ULOG",
    .setup = bench_setup,
    .execute = bench_execute,
    .steps = (struct bench_step const* const[]) {
        &(const struct bench_step){ .name = "printf_hello_world", .func = bench_printf_hello_world },
        &(const struct bench_step){ .name = "ulog_hello_world", .func = bench_ulog_hello_world },
        &(const struct bench_step){ .name = "printf_arg1", .func = bench_printf_arg1 },
        &(const struct bench_step){ .name = "ulog_arg1", .func = bench_ulog_arg1 },
        &(const struct bench_step){ .name = "printf_arg3", .func = bench_printf_arg3 },
        &(const struct bench_step){ .name = "ulog_arg3", .func = bench_ulog_arg3 },
        &(const struct bench_step){ .name = "printf_text_len64", .func = bench_printf_text_len64 },
        &(const struct bench_step){ .name = "ulog_text_len64", .func = bench_ulog_text_len64 },
        &(const struct bench_step){ .name = "printf_hexdump", .func = bench_printf_hexdump },
        &(const struct bench_step){ .name = "ulog_hexdump", .func = bench_ulog_hexdump },
        NULL,
    },
    .teardown = bench_teardown,
};

int main()
{
    #if !(BENCHMARK_ULOG_SIZE || BENCHMARK_PRINTF_SIZE)
        static const bool fast_uart = false;
        nrfx_uart_config_t uart_cfg = NRFX_UART_DEFAULT_CONFIG(TX_PIN, RX_PIN);
        uart_cfg.baudrate = fast_uart ? NRF_UART_BAUDRATE_1000000 : NRF_UART_BAUDRATE_115200;
        nrf_gpio_cfg_input(BTN1_PIN, NRF_GPIO_PIN_PULLUP);

        if (NRFX_SUCCESS == nrfx_uart_init(&UART0, &uart_cfg, NULL)
            && NRFX_SUCCESS == nrfx_clock_init(on_clock_event))
        {
            nrfx_clock_start(NRF_CLOCK_DOMAIN_LFCLK);
            ulog_init(&(ulog_config_t) {
                        .buffer = (uint8_t[256]){0},
                        .size = 256,
                    },
                    &(ulog_backend_t) {
                        .init = ulog_backend_init,
                        .tx = ulog_backend_tx,
                        .deinit = ulog_backend_deinit,
                    },
                    NULL);


            while (true)
            {
                if (nrf_gpio_pin_read(BTN1_PIN) == 0)
                {
                    bsuit.setup(&bsuit);
                    bsuit.execute(&bsuit);
                    bsuit.teardown(&bsuit);
                    NRFX_DELAY_US(500 * 1000);
                }
            }
        }
    #else
        extern void text(void);
        text();
    #endif

    while (true)
    {
        __WFE();
    }

    return 0;
}