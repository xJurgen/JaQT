#include "gpiocommands.h"
#include "usbstrings.h"
#include "tokenizer.h"
#include "../gpio/gpiocontrol.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void statusOn(int led)
{
	statusLEDOn(led);
}

void statusOff(int led)
{
	statusLEDOff(led);
}

const char* parseGPIOS()
{
	static char gpio_state[15];
	const char* messageptr = undefined;

	uint8_t tokenCounter = countTokens();
	//for (; getToken(tokenCounter) != NULL; tokenCounter++);

	if (tokenCounter >= 3) {
		uint8_t gpioNum = atoi(getToken(1));

		if (strcmp(getToken(2), "set") == 0) {				// SET --
			uint8_t gpioState = 1;
			if (strcmp(getToken(3), "in") == 0) gpioState = 0;
			else if (strcmp(getToken(3), "out") == 0) gpioState = 1;
			else if (tokenCounter != 4) return labelERROR;

			uint8_t gpioVal = atoi(getToken(4));

			if (gpioVal != 1 && gpioVal != 0) return undefinedGpioVal();

			if (strcmp(getToken(1), "all") == 0) {
				setAllGPIOS(gpioVal);
			} else if (!setGPIO(gpioNum, gpioVal, gpioState)){
				return undefinedGpio();
			}
			messageptr = labelOK;							// --

		} else if (strcmp(getToken(2), "get") == 0) {		// GET --
			int16_t val = getGPIO(gpioNum);
			if (val < 0) {
				return undefinedGpio();
			}
			sprintf(gpio_state, "[OK] %d\r\n", val);
			messageptr = gpio_state;
		}													// --
	}

	return messageptr;
}
