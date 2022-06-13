/*
*
*	Author: satoshinm
*	Changed and modularised by: xvever12
*	Created from former usbusart.c by xvever12
*	Implements usart configuration
*
*/
#include "usartconf.h"

#include <libopencm3/stm32/usart.h>
#include <libopencm3/cm3/nvic.h>

#define IRQ_PRI_USBUSART	(1 << 4)

void setUSART(int baudrate, int USBUSART)
{
    usart_disable(USBUSART);

	usart_set_baudrate(USBUSART, baudrate);
	usart_set_databits(USBUSART, 8);
	usart_set_stopbits(USBUSART, USART_STOPBITS_1);
	usart_set_mode(USBUSART, USART_MODE_TX_RX);
	usart_set_parity(USBUSART, USART_PARITY_NONE);
	usart_set_flow_control(USBUSART, USART_FLOWCONTROL_NONE);

	usart_enable(USBUSART);
}

void uartSETUP(void)
{
	setUSART(115200, USART1);
	setUSART(115200, USART2);
	setUSART(115200, USART3);

	/* Enable interrupts */
	USART1_CR1 |= USART_CR1_RXNEIE;
	nvic_set_priority(NVIC_USART1_IRQ, IRQ_PRI_USBUSART);
	nvic_enable_irq(NVIC_USART1_IRQ);

	USART2_CR1 |= USART_CR1_RXNEIE;
	nvic_set_priority(NVIC_USART2_IRQ, IRQ_PRI_USBUSART);
	nvic_enable_irq(NVIC_USART2_IRQ);

	USART3_CR1 |= USART_CR1_RXNEIE;
	nvic_set_priority(NVIC_USART3_IRQ, IRQ_PRI_USBUSART);
	nvic_enable_irq(NVIC_USART3_IRQ);
}
