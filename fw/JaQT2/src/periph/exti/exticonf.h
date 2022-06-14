#ifndef __EXTI_CONF_H
#define __EXTI_CONF_H

#include <stdint.h>

#define FALLING 0
#define RISING 1

void extiWiegandSETUP(uint16_t *exti8_direction, uint16_t *exti9_direction);

#endif
