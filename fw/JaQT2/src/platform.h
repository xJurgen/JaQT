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

/* This file implements the platform specific functions for the STM32
 * implementation.
 */
#ifndef __PLATFORM_H
#define __PLATFORM_H

#include "version.h"

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/memorymap.h>
#include <libopencm3/usb/usbd.h>

#define BOARD_IDENT       "TBS JaQT2 Firmware v" FIRMWARE_VERSION ""

/* Use PC13 for a "dummy" uart led. So we can observe the LED. */
#define LED_PORT_UART	GPIOC
#define LED_UART	GPIO13

/* Use newlib provided integer only stdio functions */
#define sscanf siscanf
#define sprintf siprintf
#define vasprintf vasiprintf
#define snprintf sniprintf

static inline void _gpio_set(uint32_t gpioport, uint16_t gpios)
{
	GPIO_BSRR(gpioport) = gpios;
}
#define gpio_set _gpio_set

static inline void _gpio_clear(uint32_t gpioport, uint16_t gpios)
{
	//GPIO_BRR(gpioport) = gpios;
	GPIO_BSRR(gpioport) = (gpios << 16);
}
#define gpio_clear _gpio_clear

static inline uint16_t _gpio_get(uint32_t gpioport, uint16_t gpios)
{
	return (uint16_t)GPIO_IDR(gpioport) & gpios;
}
#define gpio_get _gpio_get

uint8_t get_platform_status();

uint32_t detect_rev(void);
void platform_timing_init(void);
void platform_init(void);

#endif

