/*
*   I2C Base functions inspired by libopencm3 I2C example
*   Author: Jiří Veverka
*/
#include <errno.h>
#include <stdio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/gpio.h>

#include "i2cbase.h"


void i2c_setup(void)
{
	/* Enable clocks for I2C1 and AFIO. */
	rcc_periph_clock_enable(RCC_I2C1);
	rcc_periph_clock_enable(RCC_AFIO);

	/* Set alternate functions for the SCL and SDA pins of I2C1. */
	gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ,
		      GPIO_CNF_OUTPUT_ALTFN_OPENDRAIN,
		      GPIO_I2C1_SCL | GPIO_I2C1_SDA);

	/* Disable the I2C before changing any configuration. */
	i2c_peripheral_disable(I2C1);

	/* APB1 is running at 8MHz. */
	i2c_set_clock_frequency(I2C1, rcc_apb1_frequency / 1000000U * 4 - 1);

	/* 400KHz - I2C Fast Mode */

	i2c_set_fast_mode(I2C1);

	/*
	 * fclock for I2C is 8MHz APB1 -> cycle time 112ns, low time at 400kHz
	 * incl trise -> Thigh = 1600ns; CCR = tlow/tcycle = 0x1C,9;
	 * Datasheet suggests 0x1e.
	 */
	i2c_set_ccr(I2C1, 0x1e);

	/*
	 * fclock for I2C is 8MHz -> cycle time 112s, rise time for
	 * 400kHz => 300ns and 100kHz => 1000ns; 300ns/28ns = 10;
	 * Incremented by 1 -> 11.
	 */
	i2c_set_trise(I2C1, 0x0b);

	/*
	 * This is our slave address - needed only if we want to receive from
	 * other masters.
	 */
	i2c_set_own_7bit_slave_address(I2C1, I2C_EXPANDER_ADDRESS);

	/* If everything is configured -> enable the peripheral. */
	i2c_peripheral_enable(I2C1);
}

uint8_t i2c_probe(uint8_t addr)
{
	i2c_send_start(I2C1);

	/* Wait for the end of the start condition, master mode selected, and BUSY bit set */
	while ( !( (I2C_SR1(I2C1) & I2C_SR1_SB)
	&& (I2C_SR2(I2C1) & I2C_SR2_MSL)
	&& (I2C_SR2(I2C1) & I2C_SR2_BUSY) ));

	i2c_send_7bit_address(I2C1, addr, I2C_WRITE);

	uint8_t probecounter = 0;
	while(!(I2C_SR1(I2C1) & I2C_SR1_ADDR) && probecounter != 255) {
		probecounter++;
	}

	(void)I2C_SR2(I2C1);

	i2c_send_stop(I2C1);

	return (probecounter == 255) ? 0 : 1;
}
