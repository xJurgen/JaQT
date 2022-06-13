/*
*
*	Author: Jiří Veverka (xvever12):
*	Module implementing rs485 debug testing
*
*/

#include "debugrs485.h"

#include <stdio.h>

int stopbits;
int parity;
char response[30];

char* get_485debug_info()
{
		char stoptext[5];
		char paritytext[5];

		if (stopbits == 1) sprintf(stoptext, "1");
		else if (stopbits == 2) sprintf(stoptext, "1.5");
		else sprintf(stoptext, "2");

		if (parity == 1) sprintf(paritytext, "None");
		else if (parity == 2) sprintf(paritytext, "Odd");
		else sprintf(paritytext, "Even");

		sprintf(response, "Stopbits: %s Parity: %s\r\n", stoptext, paritytext);
		return response;
}
