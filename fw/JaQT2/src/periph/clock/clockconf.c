/*
*
*	Author: satoshinm
*	Changed and modularised by: xvever12
*	Taken from usbusart.c and made into separate module by xvever12
*	Enables clocks
*
*/
#include "clockconf.h"

#include <libopencm3/stm32/rcc.h>

void clockSETUP(void)
{
	rcc_periph_clock_enable(RCC_USART1);
	rcc_periph_clock_enable(RCC_USART2);
	rcc_periph_clock_enable(RCC_USART3);
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_AFIO);
	rcc_periph_clock_enable(RCC_TIM1);
	rcc_periph_clock_enable(RCC_TIM2);
	rcc_periph_clock_enable(RCC_TIM3);
	rcc_periph_clock_enable(RCC_TIM4);
}
