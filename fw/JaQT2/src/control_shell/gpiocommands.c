/*
*   Author: Jiří Veverka
*   Module implementing command processing
*/
#include "gpiocommands.h"
#include "usbstrings.h"
#include "tokenizer.h"
#include "../gpio/gpiocontrol.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


const char* parseGPIOS()
{
	static char gpio_state[15];
	const char* messageptr = undefined;

	uint8_t tokenCounter = 0;
	for (; getToken(tokenCounter) != NULL; tokenCounter++);

	if (tokenCounter >= 3) {
		uint8_t gpioNum = atoi(getToken(1));

		if (strcmp(getToken(2), "set") == 0) {				// SET --
			uint8_t gpioVal = atoi(getToken(3));

			if (gpioVal != 1 && gpioVal != 0) return undefinedGpioVal();

			if (strcmp(getToken(1), "all") == 0) {
				setAllGPIOS(gpioVal);
			} else if (!setGPIO(gpioNum, gpioVal)){
				return undefinedGpio();
			}
			messageptr = labelOK;							// --

		} else if (strcmp(getToken(2), "get") == 0) {		// GET --
			int8_t val = getGPIO(gpioNum);
			if (val < 0) {
				return undefinedGpio();
			}
			sprintf(gpio_state, "[OK] %d\r\n", val);
			messageptr = gpio_state;
		}													// --
	}

	return messageptr;
}
