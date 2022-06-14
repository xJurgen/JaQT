#ifndef __GPIO_CONTROL_H
#define __GPIO_CONTROL_H

#include <stdint.h>

void setAllGPIOS(uint8_t value);
int8_t setGPIO(uint8_t gpio, uint8_t value, uint8_t state);
int16_t getGPIO(uint8_t gpio);
const char* undefinedGpio();
const char* undefinedGpioVal();
void statusLEDOn(int led);
void statusLEDOff(int led);

#endif
