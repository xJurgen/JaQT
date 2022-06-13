/*
*
*	Author: satoshinm
*	Changed and modularised by: xvever12
*	Taken from usbusart.c and made into separate module by xvever12
*	Initializes gpios
*/

#include "gpioconf.h"
#include <libopencm3/stm32/gpio.h>

void gpioSETUP(void)
{
	/*
	* Changed by xvever12 to support jaqt expansion board pinout
	*/
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO9); //USART1 Tx - Debug
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO2); //USART2 Tx - RS232
	gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO10); //USART3 Tx - RS485
 
 	gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, GPIO10); //USART1 Rx - Debug
 	gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, GPIO3);  //USART2 Rx - RS232
 	gpio_set_mode(GPIOB, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, GPIO11); //USART3 Rx - RS485

 	gpio_set_mode(GPIOB, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, GPIO8); //D0 - wiegand
	gpio_set_mode(GPIOB, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, GPIO9); //D1 - wiegand
	/*----------------------------end of section------------------------*/
}
