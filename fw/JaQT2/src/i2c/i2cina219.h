/*
    Author: Jiří Veverka
    Code inspired and based on: https://github.com/FriedCircuits/USBMultimeter/tree/master/STM32F429-Discovery-Prototype/Firmware/lib/ina219
*/
#ifndef __I2C_INA219_H
#define __I2C_INA219_H

#include <stdint.h>

void ina219Init_32V_2A(void);
void ina219Init_32V_1A(void);
void ina219Init_16V_400mA(void);

float ina219GetPower_mW(void);
float ina219GetBusVoltage_V(void);
float ina219GetShuntVoltage_mV(void);
float ina219GetCurrent_mA(void);

void set_ina219_addr(uint8_t addr);
uint8_t get_ina219_addr(void);

#endif
