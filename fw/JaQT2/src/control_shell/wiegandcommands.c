/*
*   Author: Jiří Veverka
*   Module implementing wiegand commands from shell
*/
#include "wiegandcommands.h"
#include "usbstrings.h"
#include "tokenizer.h"

#include "../wiegand/wiegand.h"

#include <stdlib.h>
#include <string.h>

const char* checkWiegandSize()
{
	if(getWiegandSize() <= 0)
		return underLimitErrorMessage;
	else if(getWiegandSize() > WIEGAND_MAX_SIZE)
		return overLimitErrorMessage;
	return NULL;
}

const char* parseWiegand()
{
	int i = 1, continuous = 0;
	const char* messageptr = undefined;

	if(strcmp(getToken(i), "continuous") == 0){
		continuous = 1;
		i++;
	}
	if(atoi(getToken(i)) != 0 || strcmp(getToken(i), "0") == 0){
		setWiegandSize((uint8_t) atoi(getToken(i)));
		if((messageptr = checkWiegandSize())) return messageptr; //If given size is not within interval
	}
	if(strcmp(getToken(i), "start") == 0){
		if(i == 1) setWiegandSize(26);
		startWiegand();
		if(continuous) startContinuousWiegand();
		messageptr = wiegandStartMessage;
	}
	else if(strcmp(getToken(i), "stop") == 0){
		stopWiegand();
		stopContinuousWiegand();
		setWiegandSize(26);
		messageptr = wiegandStopMessage;
	}
	else messageptr = undefined;

	return messageptr;
}
