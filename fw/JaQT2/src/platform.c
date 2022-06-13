/*
 * This file is part of the Black Magic Debug project.
 *
 * Copyright (C) 2011  Black Sphere Technologies Ltd.
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

/* This file implements the platform specific functions for the ST-Link
 * implementation.
 */

#include "general.h"
/*
----------- ADDED BY xvever12 -----------
	Includes implemented modules
-----------------------------------------
*/
#include "usb/cdcacm.h"
#include "usb/usbuart.h"
#include "control_shell/usbshell.h"
#include "i2c/i2cbase.h"
#include "i2c/i2cgpio.h"
/*
----------- End of section -----------
-----------------------------------------
*/

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/cm3/scs.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/stm32/adc.h>
/*
----------- ADDED BY xvever12 -----------
	Include I2C module for board initialization
-----------------------------------------
*/
#include <libopencm3/stm32/i2c.h>
/*
----------- End of section -----------
-----------------------------------------
*/

/*
----------- ADDED BY xvever12 -----------
	Add variable which is used to indicate if
	platform is initialized and ready to start
-----------------------------------------
*/
int platform_ready;
/*
----------- End of section -----------
-----------------------------------------
*/
static uint32_t rev;

/* return 0 for stlink V1, 1 for stlink V2 and 2 for stlink V2.1 */
uint32_t detect_rev(void)
{
	uint32_t rev;
	int res;

	while (RCC_CFGR & 0xf) /* Switch back to HSI. */
		RCC_CFGR &= ~3;
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_GPIOC);
	rcc_periph_clock_enable(RCC_USB);
	rcc_periph_reset_pulse(RST_USB);
	rcc_periph_clock_enable(RCC_AFIO);
	rcc_periph_clock_enable(RCC_CRC);
	/* First, get Board revision by pulling PC13/14 up. Read
	 *  11 for ST-Link V1, e.g. on VL Discovery, tag as rev 0
	 *  00 for ST-Link V2, e.g. on F4 Discovery, tag as rev 1
	 *  01 for ST-Link V2, else,                 tag as rev 1
	 */
	gpio_set_mode(GPIOC, GPIO_MODE_INPUT,
				  GPIO_CNF_INPUT_PULL_UPDOWN, GPIO14 | GPIO13);
	gpio_set(GPIOC, GPIO14 | GPIO13);
	for (int i = 0; i < 100; i ++)
		res = gpio_get(GPIOC, GPIO13);
	if (res)
		rev = 0;
	else {
		/* Check for V2.1 boards.
		 * PA15/TDI is USE_RENUM, pulled with 10 k to U5V on V2.1,
		 * Otherwise unconnected. Enable pull low. If still high.
		 * it is V2.1.*/
		rcc_periph_clock_enable(RCC_AFIO);
		AFIO_MAPR |= 0x02000000; /* Release from TDI.*/
		gpio_set_mode(GPIOA, GPIO_MODE_INPUT,
                                 GPIO_CNF_INPUT_PULL_UPDOWN, GPIO15);
		gpio_clear(GPIOA, GPIO15);
		for (int i = 0; i < 100; i++)
			res =  gpio_get(GPIOA, GPIO15);
		if (res) {
			rev = 2;
			/* Pull PWR_ENn low.*/
			gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_2_MHZ,
						  GPIO_CNF_OUTPUT_OPENDRAIN, GPIO15);
			gpio_clear(GPIOB, GPIO15);
			/* Pull USB_RENUM low!*/
			gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_2_MHZ,
						  GPIO_CNF_OUTPUT_OPENDRAIN, GPIO15);
			gpio_clear(GPIOA, GPIO15);
		} else
			/* Catch F4 Disco board with both resistors fitted.*/
			rev = 1;
		/* On Rev > 0 unconditionally activate MCO on PORTA8 with HSE! */
		RCC_CFGR &= ~(0xf << 24);
		RCC_CFGR |= (RCC_CFGR_MCO_HSE << 24);
		gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
		GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO8);
	}
	if (rev < 2) {
		gpio_clear(GPIOA, GPIO12);
		gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_2_MHZ,
					  GPIO_CNF_OUTPUT_OPENDRAIN, GPIO12);
	}
	return rev;
}

/*
----------- ADDED BY xvever12 -----------
	Function initializing GPIO states on startup
-----------------------------------------
*/
void gpio_init(void){
	gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ,
	              GPIO_CNF_OUTPUT_PUSHPULL, GPIO13); //Status LED

	gpio_set_mode(GPIOB, GPIO_MODE_INPUT,
	              GPIO_CNF_INPUT_PULL_UPDOWN, GPIO15);
	
	gpio_set_mode(GPIOA, GPIO_MODE_INPUT,
	              GPIO_CNF_INPUT_PULL_UPDOWN, GPIO8);

	gpio_set_mode(GPIOA, GPIO_MODE_INPUT,
	              GPIO_CNF_INPUT_PULL_UPDOWN, GPIO15);

	gpio_set_mode(GPIOB, GPIO_MODE_INPUT,
	              GPIO_CNF_INPUT_PULL_UPDOWN, GPIO3);

	gpio_set_mode(GPIOB, GPIO_MODE_INPUT,
	              GPIO_CNF_INPUT_PULL_UPDOWN, GPIO4);

	gpio_set_mode(GPIOB, GPIO_MODE_INPUT,
	              GPIO_CNF_INPUT_PULL_UPDOWN, GPIO5);

	gpio_clear(GPIOA, GPIO15);
	gpio_clear(GPIOB, GPIO4);

	gpio_clear(GPIOA, GPIO15 | GPIO8);
	gpio_clear(GPIOB, GPIO15 | GPIO3 | GPIO4 | GPIO5);
	gpio_toggle(GPIOB, GPIO15);
}
/*
----------- End of section -----------
-----------------------------------------
*/

void platform_init(void)
{
	/*
	----------- ADDED BY xvever12 -----------
		Set flag to indicate platform is not ready yet
	-----------------------------------------
	*/
	platform_ready = 0;
	/*
	----------- End of section -----------
	-----------------------------------------
	*/
	rev = detect_rev();
	SCS_DEMCR |= SCS_DEMCR_VC_MON_EN;

	/*
	----------- ADDED BY xvever12 -----------
		Disable JTAG but keep SWD on
	-----------------------------------------
	*/
	uint32_t reg = AFIO_MAPR;
	reg &= ~(AFIO_MAPR_SWJ_CFG_JTAG_OFF_SW_ON);
	reg |= (1<<25);
	AFIO_MAPR = reg;
	/*
	----------- End of section -----------
	-----------------------------------------
	*/


	/*
	----------- ADDED BY xvever12 -----------
		New version of libopencm3 library
		has different function for setting clock
		frequency. Added for compatibility.
	-----------------------------------------
	*/
	rcc_clock_setup_pll(&rcc_hse_configs[RCC_CLOCK_HSE8_72MHZ]);
	
	/*
	----------- End of section -----------
	-----------------------------------------
	*/

	/* Relocate interrupt vector table here */
	extern int vector_table;
	SCB_VTOR = (uint32_t)&vector_table;

	platform_timing_init();
	if (rev > 1) /* Reconnect USB */
		gpio_set(GPIOC, GPIO13);
	cdcacm_init();

	/* Enable UART if we're not being debugged. */
	if (!(SCS_DEMCR & SCS_DEMCR_TRCENA))
		usbuart_init();

	/*
	----------- ADDED BY xvever12 -----------
		Call init functions of newly added modules
	-----------------------------------------
	*/
	gpio_init();
	i2c_setup();
	/*
	----------- End of section -----------
	-----------------------------------------
	*/

	cmd |= 0xFF; //Set all GPIO I2C exp pins to 1 (req for default/read mode)
	i2c_transfer7(I2C1, I2C_EXPANDER_ADDRESS, &cmd, sizeof(cmd), 0, 0);

	while(!cdcacm_get_config());

	platform_ready = 1;

	while(1)
		usbd_poll(usbdev);
}

static volatile uint32_t time_ms;

void platform_timing_init(void)
{
	/* Setup heartbeat timer */
	systick_set_clocksource(STK_CSR_CLKSOURCE_AHB_DIV8);
	systick_set_reload(900000);	/* Interrupt us at 10 Hz */
	SCB_SHPR(11) &= ~((15 << 4) & 0xff);
	SCB_SHPR(11) |= ((14 << 4) & 0xff);
	systick_interrupt_enable();
	systick_counter_enable();
}

void sys_tick_handler(void)
{
	time_ms += 100;
}

/*
----------- REMOVED BY xvever12 -----------
	Everything that was after this point
	was removed because it was unnecessary
	to have (it did not brought any functionality)
-----------------------------------------
*/
