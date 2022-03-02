#include "nrf_gpio.h"
#include <stdint.h>

int main()
{
	const uint32_t pin = NRF_GPIO_PIN_MAP(0, 13);
	nrf_gpio_cfg_output(pin);
	while (1)
	{
		NRFX_DELAY_US(500 * 1000);
		nrf_gpio_pin_toggle(pin);
	}
	return 0;
}