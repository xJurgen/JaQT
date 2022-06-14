/*
*   I2C Base functions inspired by libopencm3 I2C example
*   Author: Jiří Veverka
*/
#ifndef __I2CBASE_H
#define __I2CBASE_H

#include "general.h"

#define I2C_EXPANDER_ADDRESS 0x20
void i2c_setup(void);
uint8_t i2c_probe(uint8_t addr);

#endif
