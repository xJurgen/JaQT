/*
 * This file is part of the Black Magic Debug project.
 *
 * Copyright (C) 2012  Black Sphere Technologies Ltd.
 * Written by Gareth McMullin <gareth@blacksphere.co.nz>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "usbuart.h"

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/scs.h>
#include <libopencm3/stm32/exti.h>

#include "general.h"
#include "cdcacm.h"
#include "usartconf.h"

#include "../control_shell/usbshell.h"
#include "../wiegand/wiegand.h"

#include "../serial/tester/honeywell/honeywell.h"

#include "../periph/timer/timerconf.h"
#include "../periph/exti/exticonf.h"
#include "../periph/clock/clockconf.h"
#include "../periph/gpio/gpioconf.h"

#if DEBUG_COMMANDS
#include "rs485/debug/debugrs485.h"
#endif

#define FIFO_SIZE 128

/* RX Fifo buffer */
static uint8_t buf_rx1[FIFO_SIZE];
static uint8_t buf_rx2[FIFO_SIZE];
static uint8_t buf_rx3[FIFO_SIZE];
/* Fifo in pointer, writes assumed to be atomic, should be only incremented within RX ISR */
static uint8_t buf_rx1_in;
static uint8_t buf_rx2_in;
static uint8_t buf_rx3_in;
/* Fifo out pointer, writes assumed to be atomic, should be only incremented outside RX ISR */
static uint8_t buf_rx1_out;
static uint8_t buf_rx2_out;
static uint8_t buf_rx3_out;

static void usbuart_run(int USBUSART_TIM, uint8_t *buf_rx_out, uint8_t *buf_rx_in, uint8_t *buf_rx, int CDCACM_UART_ENDPOINT);


/*
 Reconsider renaming it to "system_init(void)" and moving it somewhere else...
*/
void usbuart_init(void)
{
	/* Restart wiegand periph on init to set default values */
	wiegandReset();

	/* Setup RCC clocks */
	clockSETUP();

	/* Setup GPIO parameters. */
	gpioSETUP();

	/* Setup UART parameters. */
	uartSETUP();

	/* Setup EXTI8 and EXTI9 (Wiegand) parameters. */
	extiWiegandSETUP(&exti8_direction, &exti9_direction);

	/* Setup timer for running deferred FIFO processing */
	timerSETUP();
}

/*
 * Runs deferred processing for usb uart rx, draining RX FIFO by sending
 * characters to host PC via CDCACM.  Allowed to read from FIFO in pointer,
 * but not write to it. Allowed to write to FIFO out pointer.
 */
static void usbuart_run(int USBUSART_TIM, uint8_t *buf_rx_out, uint8_t *buf_rx_in, uint8_t *buf_rx, int CDCACM_UART_ENDPOINT)
{
	gpio_clear(LED_PORT_UART, LED_UART);
	/* forcibly empty fifo if no USB endpoint */
	if (cdcacm_get_config() != 1)
	{
		*buf_rx_out = *buf_rx_in;
	}

	/* if fifo empty, nothing further to do */
	if (*buf_rx_in == *buf_rx_out) {
		/* turn off LED, disable IRQ */
		timer_disable_irq(USBUSART_TIM, TIM_DIER_UIE);
	}
	else
	{
		uint8_t packet_buf[CDCACM_PACKET_SIZE];
		uint8_t packet_size = 0;
		uint8_t buf_out = *buf_rx_out;

		/* copy from uart FIFO into local usb packet buffer */
		while (*buf_rx_in != buf_out && packet_size < CDCACM_PACKET_SIZE)
		{
			packet_buf[packet_size++] = buf_rx[buf_out++];

			/* wrap out pointer */
			if (buf_out >= FIFO_SIZE)
			{
				buf_out = 0;
			}

		}

		/* advance fifo out pointer by amount written */
		*buf_rx_out += usbd_ep_write_packet(usbdev,
				CDCACM_UART_ENDPOINT, packet_buf, packet_size);
		*buf_rx_out %= FIFO_SIZE;

#if DEBUG_COMMANDS
		if(read232 && CDCACM_UART_ENDPOINT == 1){
			usbd_ep_write_packet(usbdev, VIRTUAL_OUT, packet_buf, packet_size);
		}
		if(readdebug && CDCACM_UART_ENDPOINT == 5){
			usbd_ep_write_packet(usbdev, VIRTUAL_OUT, packet_buf, packet_size);
		}
		if(read485 && CDCACM_UART_ENDPOINT == 3){
			usbd_ep_write_packet(usbdev, VIRTUAL_OUT, packet_buf, packet_size);
		}
#endif
	}
}

void usbuart_set_line_coding(struct usb_cdc_line_coding *coding, int USBUSART)
{
	usart_set_baudrate(USBUSART, coding->dwDTERate);

	if (coding->bParityType)
		usart_set_databits(USBUSART, coding->bDataBits + 1);
	else
		usart_set_databits(USBUSART, coding->bDataBits);

	switch(coding->bCharFormat) {
	case 0:
	#if DEBUG_COMMANDS
		if (USBUSART == USART3) stopbits = 1;
	#endif
		usart_set_stopbits(USBUSART, USART_STOPBITS_1);
		break;
	case 1:
	#if DEBUG_COMMANDS
		if (USBUSART == USART3) stopbits = 2;
	#endif
		usart_set_stopbits(USBUSART, USART_STOPBITS_1_5);
		break;
	case 2:
	#if DEBUG_COMMANDS
		if (USBUSART == USART3) stopbits = 3;
	#endif
		usart_set_stopbits(USBUSART, USART_STOPBITS_2);
		break;
	}

	switch(coding->bParityType) {
	case 0:
	#if DEBUG_COMMANDS
		if (USBUSART == USART3) parity = 1;
	#endif
		usart_set_parity(USBUSART, USART_PARITY_NONE);
		break;
	case 1:
	#if DEBUG_COMMANDS
		if (USBUSART == USART3) parity = 2;
		#endif
		usart_set_parity(USBUSART, USART_PARITY_ODD);
		break;
	case 2:
	#if DEBUG_COMMANDS
		if (USBUSART == USART3) parity = 3;
	#endif
		usart_set_parity(USBUSART, USART_PARITY_EVEN);
		break;
	}
}

static void usbuart_usb_out_cb(int USBUSART, usbd_device *dev, uint8_t ep, int CDCACM_UART_ENDPOINT)
{
	(void)ep;

	char buf[CDCACM_PACKET_SIZE];
	int len = usbd_ep_read_packet(dev, CDCACM_UART_ENDPOINT,
					buf, CDCACM_PACKET_SIZE);


	//gpio_set(LED_PORT_UART, LED_UART);
	for(int i = 0; i < len; i++){
		usart_send_blocking(USBUSART, buf[i]);
	}
	gpio_clear(LED_PORT_UART, LED_UART);
}

void usbuart1_usb_out_cb(usbd_device *dev, uint8_t ep)
{
    usbuart_usb_out_cb(USART1, dev, ep, 5);
}

void usbuart2_usb_out_cb(usbd_device *dev, uint8_t ep)
{
    usbuart_usb_out_cb(USART2, dev, ep, 1);
}

void usbuart3_usb_out_cb(usbd_device *dev, uint8_t ep)
{
    usbuart_usb_out_cb(USART3, dev, ep, 3);
}

void usbuartvirt_usb_out_cb(usbd_device *dev, uint8_t ep)
{
    (void)ep;

	char buf[CDCACM_PACKET_SIZE];
	int len = usbd_ep_read_packet(dev, VIRTUAL_IN, buf, CDCACM_PACKET_SIZE);

	if(len)	send_usbd_packet(dev, VIRTUAL_OUT, buf, len, 1);
}

void usbuart_usb_in_cb(usbd_device *dev, uint8_t ep)
{
	(void) dev;
	(void) ep;
}

/*
 * Read a character from the UART RX and stuff it in a software FIFO.
 * Allowed to read from FIFO out pointer, but not write to it.
 * Allowed to write to FIFO in pointer.
 */
// USBUSART_ISR
static void usart_isr(int USBUSART, int USBUSART_TIM, uint8_t *buf_rx_out, uint8_t *buf_rx_in, uint8_t *buf_rx)
{
	if (rs485_test && (USBUSART == USART3) && rs485_sent && !rs485_tim_wait) {
		timer_disable_irq(TIM1, TIM_DIER_UIE);
		timer_disable_counter(TIM1);
	}

	uint32_t err = USART_SR(USBUSART);
	char c = usart_recv(USBUSART);
	if (err & (USART_SR_ORE | USART_SR_FE))
		return;

	if (rs485_test && USBUSART == USART3) {
		if (rs485_tim_wait || !rs485_sent) return;
		rs485_sent = 0; //Should always get here
		rs485_test++;

		if ((int) c != 0x14) {
			data_failed_rs485_test++;
		}
		test_rs485();
		return;
	}

	/* Turn on LED */
	gpio_set(LED_PORT_UART, LED_UART);
	/* If the next increment of rx_in would put it at the same point
	* as rx_out, the FIFO is considered full.
	*/
	if (((*buf_rx_in + 1) % FIFO_SIZE) != *buf_rx_out)
	{
		/* insert into FIFO */
		buf_rx[(*buf_rx_in)++] = c;

		/* wrap out pointer */
		if (*buf_rx_in >= FIFO_SIZE)
		{
			*buf_rx_in = 0;
		}

		/* enable deferred processing if we put data in the FIFO */
		timer_enable_irq(USBUSART_TIM, TIM_DIER_UIE);
	}
}

void usart1_isr(void)
{
    usart_isr(USART1, TIM2, &buf_rx1_out, &buf_rx1_in, buf_rx1);
}

void usart2_isr(void)
{
    usart_isr(USART2, TIM3, &buf_rx2_out, &buf_rx2_in, buf_rx2);
}

void usart3_isr(void)
{
    usart_isr(USART3, TIM4, &buf_rx3_out, &buf_rx3_in, buf_rx3);
}

void tim2_isr(void)
{
	/* need to clear timer update event */
	timer_clear_flag(TIM2, TIM_SR_UIF);

	/* process FIFO */
	usbuart_run(TIM2, &buf_rx1_out, &buf_rx1_in, buf_rx1, 5);
}

void tim3_isr(void)
{
	/* need to clear timer update event */
	timer_clear_flag(TIM3, TIM_SR_UIF);

	/* process FIFO */
	usbuart_run(TIM3, &buf_rx2_out, &buf_rx2_in, buf_rx2, 1);
}

void tim4_isr(void)
{
	/* need to clear timer update event */
	timer_clear_flag(TIM4, TIM_SR_UIF);

	/* process FIFO */
	usbuart_run(TIM4, &buf_rx3_out, &buf_rx3_in, buf_rx3, 3);
}
