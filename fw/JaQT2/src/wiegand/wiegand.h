#ifndef __WIEGAND_H
#define __WIEGAND_H

#include <stdint.h>

#define WIEGAND_MAX_SIZE 200

void wiegandReset();
void setWiegandSize(uint8_t size);
void stopWiegand();
void startWiegand();
void startContinuousWiegand();
void stopContinuousWiegand();
uint8_t getContinuousWiegandStatus();
uint8_t getWiegandStatus();
uint8_t getWiegandSize();

extern uint16_t exti8_direction;
extern uint16_t exti9_direction;

#endif
