/*
*
*	Author: satoshinm
*	Changed and modularised by: xvever12
*	Taken from usbusart.h and made into separate module by xvever12
*	Initializes timer peripherals
*/

#ifndef __TIMER_CONF_H
#define __TIMER_CONF_H

#define IRQ_PRI_USBUSART_TIM	(3 << 4)

void timerSETUP(void);

#endif
