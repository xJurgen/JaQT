/*
*
*	Author: satoshinm
*	Changed and modularised by: xvever12
*	Taken from usbusart.c and made into separate module by xvever12
*	Initializes timer peripherals
*/

#include "timerconf.h"

#include <stdint.h>

#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>

#define USBUART_TIMER_FREQ_HZ 1000000U /* 1us per tick */
#define USBUART_RUN_FREQ_HZ 5000U /* 200us (or 100 characters at 2Mbps) */

static void timer_reset(uint32_t timer)
{
	timer_set_counter(timer, 0);
}

void timerSETUP(void)
{
	rcc_periph_reset_pulse(RST_TIM1); //Added by xvever12 to enable TIM1
	rcc_periph_reset_pulse(RST_TIM2);
	rcc_periph_reset_pulse(RST_TIM3);
	rcc_periph_reset_pulse(RST_TIM4);

	timer_reset(TIM1); //Added by xvever12 to reset TIM1
	timer_reset(TIM2);
	timer_reset(TIM3);
	timer_reset(TIM4);
	timer_set_mode(TIM1, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP); //Added by xvever12 to configure TIM1
	timer_set_mode(TIM2, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
	timer_set_mode(TIM3, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
	timer_set_mode(TIM4, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);

	/* Set prescaler (to count one tick from given frequency */
	timer_set_prescaler(TIM1,
			rcc_apb1_frequency / USBUART_TIMER_FREQ_HZ * 2 - 1); //Added by xvever12 to configure TIM1
	timer_set_prescaler(TIM2,
			rcc_apb2_frequency / USBUART_TIMER_FREQ_HZ * 2 - 1);
	timer_set_prescaler(TIM3,
			rcc_apb2_frequency / USBUART_TIMER_FREQ_HZ * 2 - 1);
	timer_set_prescaler(TIM4,
			rcc_apb2_frequency / USBUART_TIMER_FREQ_HZ * 2 - 1);
	
	timer_disable_preload(TIM1); //Added by xvever12 to configure TIM1
	timer_continuous_mode(TIM1);//Added by xvever12 to configure TIM1

	/* Set timers period */
	timer_set_period(TIM1, 2010U); //Added by xvever12 to configure TIM1

	timer_set_period(TIM2,
			USBUART_TIMER_FREQ_HZ / USBUART_RUN_FREQ_HZ - 1);
	timer_set_period(TIM3,
			USBUART_TIMER_FREQ_HZ / USBUART_RUN_FREQ_HZ - 1);
	timer_set_period(TIM4,
			USBUART_TIMER_FREQ_HZ / USBUART_RUN_FREQ_HZ - 1);

	/* Setup update interrupt in NVIC */
	nvic_set_priority(NVIC_TIM1_UP_IRQ, 1); //Added by xvever12 to configure TIM1 interrupt
	nvic_set_priority(NVIC_TIM2_IRQ, IRQ_PRI_USBUSART_TIM);
	nvic_set_priority(NVIC_TIM3_IRQ, IRQ_PRI_USBUSART_TIM);
	nvic_set_priority(NVIC_TIM4_IRQ, IRQ_PRI_USBUSART_TIM);

	nvic_enable_irq(NVIC_TIM1_UP_IRQ); //Added by xvever12 to configure TIM1 interrupt
	nvic_enable_irq(NVIC_TIM2_IRQ);
	nvic_enable_irq(NVIC_TIM3_IRQ);
	nvic_enable_irq(NVIC_TIM4_IRQ);

	/* turn the timer on */
	timer_enable_counter(TIM1); //Added by xvever12 to enable TIM1
	timer_enable_counter(TIM2);
	timer_enable_counter(TIM3);
	timer_enable_counter(TIM4);
}
