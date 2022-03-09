#include "nrf_gpio.h"
#include "nrfx_uart.h"
#include "nrfx_rtc.h"
#include "nrfx_clock.h"
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

/**
&uart0 {
	compatible = "nordic,nrf-uarte";
	status = "okay";
	current-speed = <115200>;
	tx-pin = <6>;
	rx-pin = <8>;
	rx-pull-up;
	rts-pin = <5>;
	cts-pin = <7>;
	cts-pull-up;
};
*/

#define TX_PIN NRF_GPIO_PIN_MAP(0, 6)
#define RX_PIN NRF_GPIO_PIN_MAP(0, 8)
#define RTS_PIN NRF_GPIO_PIN_MAP(0, 5)
#define CTS_PIN NRF_GPIO_PIN_MAP(0, 7)

static const nrfx_uart_t uart = NRFX_UART_INSTANCE(0);

int _write(int handle, char *data, int size ) 
{
	(void)handle;
	nrfx_uart_tx(&uart, data, size);
	return size;
}
static void blinky_app(void)
{
	const uint32_t pin = NRF_GPIO_PIN_MAP(0, 13);
	nrf_gpio_cfg_output(pin);
	while (1)
	{
		NRFX_DELAY_US(500 * 1000);
		nrf_gpio_pin_toggle(pin);
	}
}

static void uart_app(void)
{
	nrfx_uart_config_t cfg = NRFX_UART_DEFAULT_CONFIG(TX_PIN, RX_PIN);

	uint8_t hello_world[] = "Hello World!\n";
	nrfx_err_t e = nrfx_uart_init(&uart, &cfg, NULL);

	while (NRFX_SUCCESS == e)
	{
		e = nrfx_uart_tx(&uart, hello_world, sizeof(hello_world));
		NRFX_DELAY_US(1000 * 1000);
	}
}

static void on_rtc_event(nrfx_rtc_int_type_t int_type)
{
}

static void on_clock_event(nrfx_clock_evt_type_t event)
{
}

static void rtc_app(void)
{
	nrfx_clock_init(on_clock_event);
	nrfx_clock_start(NRF_CLOCK_DOMAIN_LFCLK);
	nrfx_clock_lfclk_start();

	nrfx_rtc_t rtc = NRFX_RTC_INSTANCE(0);
	nrfx_rtc_config_t cfg = NRFX_RTC_DEFAULT_CONFIG;
	nrfx_err_t e = nrfx_rtc_init(&rtc, &cfg, on_rtc_event);
	nrfx_rtc_enable(&rtc);
	uint32_t tick = nrfx_rtc_counter_get(&rtc);
	NRFX_DELAY_US(1000 * 1000);
	tick = nrfx_rtc_counter_get(&rtc);
}

static void rtc_v2_app(void)
{
	// LFCLK = 32 kHz
	nrfx_clock_init(on_clock_event);
	nrfx_clock_start(NRF_CLOCK_DOMAIN_LFCLK);

	nrfx_rtc_t rtc = NRFX_RTC_INSTANCE(0);
	nrfx_rtc_config_t cfg = NRFX_RTC_DEFAULT_CONFIG;
	nrfx_err_t e = nrfx_rtc_init(&rtc, &cfg, on_rtc_event);
	nrfx_rtc_enable(&rtc);

	nrfx_uart_config_t ucfg = NRFX_UART_DEFAULT_CONFIG(TX_PIN, RX_PIN);

	uint8_t buffer[300] = {0};
	e = nrfx_uart_init(&uart, &ucfg, NULL);

	uint32_t ticks = 0;
	while (NRFX_SUCCESS == e)
	{
		// ticks = nrfx_rtc_counter_get(&rtc);
		puts("hello World\n");
		// printf("hello World\n");
		NRFX_DELAY_US(1000 * 1000);
	}
}

int main()
{
	// uart_app();
	rtc_v2_app();

	while (1)
	{
		__WFE();
	}

	return 0;
}