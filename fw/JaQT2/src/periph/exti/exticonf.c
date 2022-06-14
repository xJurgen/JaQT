#include "exticonf.h"

#include <stdint.h>

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/cm3/nvic.h>

void extiWiegandSETUP(uint16_t *exti8_direction, uint16_t *exti9_direction)
{
	nvic_enable_irq(NVIC_EXTI9_5_IRQ);

	exti_select_source(EXTI8, GPIOB);
	exti_select_source(EXTI9, GPIOB);
	*exti8_direction = 0;
	*exti9_direction = 0;
	exti_set_trigger(EXTI8, EXTI_TRIGGER_FALLING);
	exti_set_trigger(EXTI9, EXTI_TRIGGER_FALLING);
	exti_enable_request(EXTI8);
	exti_enable_request(EXTI9);
}
