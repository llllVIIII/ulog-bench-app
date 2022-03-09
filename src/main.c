#include "nrf_gpio.h"
#include "nrfx_uart.h"
#include "nrfx_rtc.h"
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

static const nrfx_uart_t UART0 = NRFX_UART_INSTANCE(0);
static const nrfx_rtc_t RTC0 = NRFX_RTC_INSTANCE(0);

int _write(int handle, char *data, int size ) 
{
    (void)handle;
    nrfx_uart_tx(&UART0, data, size);
    return size;
}

static void on_rtc_event(nrfx_rtc_int_type_t int_type)
{
    (void)int_type;
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

int main()
{
    nrfx_uart_config_t uart_cfg = NRFX_UART_DEFAULT_CONFIG(TX_PIN, RX_PIN);
    nrfx_rtc_config_t rtc_cfg = NRFX_RTC_DEFAULT_CONFIG;

    if (NRFX_SUCCESS == nrfx_uart_init(&UART0, &uart_cfg, NULL)
        && NRFX_SUCCESS == nrfx_clock_init(on_clock_event))
    {
        nrfx_clock_start(NRF_CLOCK_DOMAIN_LFCLK);

        if (NRFX_SUCCESS == nrfx_rtc_init(&RTC0, &rtc_cfg, on_rtc_event))
        {
            nrfx_rtc_enable(&RTC0);

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
                static const uint32_t rtc_tick_us = 1 / 32768 * 1000 * 1000;

                uint32_t rtc_ticks = nrfx_rtc_counter_get(&RTC0);
                NRFX_DELAY_US(250 * 1000);
                #if 1
                    printf("Hello World\n");
                #else 
                    ULOG_DBG("Hello World");
                #endif
                nrfx_rtc_counter_clear(&RTC0);

                printf("time %lu ms\n", rtc_ticks * rtc_tick_us / 1000);
            }
        }
    }

    while (true)
    {
        __WFE();
    }

    return 0;
}