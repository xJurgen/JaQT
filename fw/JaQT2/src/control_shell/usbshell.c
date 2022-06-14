/*
*   Basic USB shell for virtual serial comm
*   Author: Jiří Veverka
*/


#include "usbshell.h"
#include "usbstrings.h"

#include "tokenizer.h"
#include "commandbuffer.h"
#include "gpiocommands.h"
#include "wiegandcommands.h"

#include "../usb/cdcacm.h"
#include "../usb/usartconf.h"

#include "../i2c/i2cina219.h"
#include "../i2c/i2cbase.h"
#include "../print_float.h"

#include "../serial/tester/honeywell/honeywell.h"

#include "../flash/flash_eeprom.h"

#if DEBUG_COMMANDS
#include "../serial/rs485/debug/debugrs485.h"
#endif

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/timer.h>

#define SKIP_CYCLE(x,y)		x = x + (y); continue;
#define GET_ASCII(x)		(int)(((char*)messageBuffer)[x])

/*
	TODO:
		- Parse and test line starts in Python script (pyserial) [ ]
		- Remove "continuous wiegand" (will be handled in python) [ ]
*/

uint8_t ina219_connected = 0;

#if DEBUG_COMMANDS
int read232 = 0;
int read485 = 0;
int readdebug = 0;
#endif

void undefinedCommand()
{
	send_usbd_packet(usbdev, VIRTUAL_OUT, undefined, sizeof(undefined), 0);
}

/*
	Process current command packet
*/
void read_command(){
	const char *messageptr = undefined;

	initToken(getPacketBuffer());
	if(getToken(0) == NULL) {
		/* Do nothing, go to the end */
	} else if(checkTokens("reset device")) {
		scb_reset_system();
	} else if(strcmp(getToken(0), "gpio") == 0) {
		messageptr = parseGPIOS();
	} else if(strcmp(getToken(0), "wiegand") == 0) {
		messageptr = parseWiegand();
	} else if(checkTokens("help")) {
		messageptr = help;
	} else if(checkTokens("clear")){
		messageptr = NULL;
		for(int i = 0; i < 100; i++) send_usbd_packet(usbdev, 0x87, "\r\n", sizeof("\r\n"), 0);
	} else if(checkTokens("info")) {
		messageptr = infomessage;
	} else if(checkTokens("port info")) {
		messageptr = port_info;
	} else if(checkTokens("test rs485")) {
		messageptr = "";
		begin_test_rs485();
	} else if(checkTokens("stop test rs485")) {
		stop_test_rs485();
		messageptr = "";
	} else if(checkTokens("init ina219 32v 1a")) {
		if (ina219_connected) {
			ina219Init_32V_1A();
			messageptr = "Done!\r\n";
		} else {
			messageptr = ina219error;
		}
	} else if(checkTokens("init ina219 16v")) {
		if (ina219_connected) {
			ina219Init_16V_400mA();
			messageptr = "Done!\r\n";
		} else {
			messageptr = ina219error;
		}
	} else if(checkTokens("get ina219")) {
		if (!ina219_connected) {
			if (i2c_probe(get_ina219_addr())) {
				ina219Init_32V_2A();
				ina219_connected = 1;
			}
		}
		if (ina219_connected) {
			if (!i2c_probe(get_ina219_addr())) {
				ina219_connected = 0;
				messageptr = ina219error;
			} else {
				uint8_t numsize = 20;
				float shuntvoltage = ina219GetShuntVoltage_mV();
				float busvoltage = ina219GetBusVoltage_V();

				char currStr[numsize];
				format_float(ina219GetCurrent_mA(), currStr, numsize);

				char powStr[numsize];
				format_float(ina219GetPower_mW(), powStr, numsize);

				char loadStr[numsize];
				format_float(busvoltage + (shuntvoltage / 1000), loadStr, numsize);

				char shuntStr[numsize];
				format_float(shuntvoltage, shuntStr, numsize);

				char busStr[numsize];
				format_float(busvoltage, busStr, numsize);

				char result[110];
				sprintf(result, "[OK]\r\nBus voltage: %sV\r\nShunt voltage: %smV\r\n"
				"Load voltage: %sV\r\nCurrent: %smA\r\nPower: %smW\r\n", busStr, shuntStr, loadStr, currStr, powStr);
				messageptr = result;
			}
		} else {
			messageptr = ina219error;
		}
	} else if(strcmp(getToken(0), "set") == 0) {
		if(strcmp(getToken(1), "ina219") == 0) {
			if (getToken(2) != NULL) {
				set_ina219_addr(strtol(getToken(2), NULL, 16));

				char addrstr[22];
				sprintf(addrstr, "Addr set to: 0x%x\r\n", get_ina219_addr());
				messageptr = addrstr;
			}
		}
	} else if(strcmp(getToken(0), "i2c") == 0) {
		if(strcmp(getToken(1), "probe") == 0) {
			if (getToken(2) != NULL) {
				uint8_t addr;
				addr = strtol(getToken(2), NULL, 16);

				uint8_t found = i2c_probe(addr);

				if (!found) {
					messageptr = "[ERROR] I2C device not found!\r\n";
				} else {
					messageptr = "[OK] I2C device found!\r\n";
				}
			}
		}
	} else if(strcmp(getToken(0), "board_no") == 0) {
		if(strcmp(getToken(1), "set") == 0) {
			if (getToken(2) != NULL) {
				uint8_t board_no, checksum;
				uint16_t data;
				board_no = strtol(getToken(2), NULL, 10);
				checksum = 255 - board_no;
				data = ((uint16_t)checksum << 8) | board_no;

				if (!flash_write_board_no(data)) {
					if (*flash_read_board_no() + *flash_read_checksum() == 255) {
						scb_reset_system();
					} else {
						messageptr = "[ERROR] Couldn't set board_no - checksum check failed\r\n";
					}
				} else {
					messageptr = "[ERROR] Couldn't set board_no\r\n";
				}
			}
		} else if (strcmp(getToken(1), "get") == 0) {
			char board_str[15];
			sprintf(board_str, "Board no.: %d\r\n", *flash_read_board_no());
			messageptr = board_str;
		} else if (strcmp(getToken(1), "checksum") == 0) {
			char board_str[15];
			sprintf(board_str, "Board no.: %d\r\n", *flash_read_checksum());
			messageptr = board_str;
		}
	}

#if DEBUG_COMMANDS
	else if (checkTokens("set usart 115200")) {
		setUSART(115200, USART3);
		messageptr = "RS485 is now at 115200bd speed";
	} else if (checkTokens("set usart 19200")) {
		setUSART(19200, USART3);
		messageptr = "RS485 is now at 19200bd speed";
	} else if(checkTokens("debug 485 info")) {
		char response[20];
		response = get_485debug_info();
		messageptr = response;
	} else if(strcmp(packetBuffer, "read debug start") == 0){
		readdebug = 1;
	} else if(strcmp(packetBuffer, "read debug stop") == 0){
		readdebug = 0;
	} else if(strcmp(packetBuffer, "read 232 start") == 0){
		read232 = 1;
	} else if(strcmp(packetBuffer, "read 232 stop") == 0){
		read232 = 0;
	} else if(strcmp(packetBuffer, "read 485 start") == 0){
		read485 = 1;
	} else if(strcmp(packetBuffer, "read 485 stop") == 0){
		read485 = 0;
	} else if(strcmp(packetBuffer, "read all start") == 0){
		read485 = 1;
		read232 = 1;
		readdebug = 1;
	} else if(strcmp(packetBuffer, "read all stop") == 0){
		read485 = 0;
		read232 = 0;
		readdebug = 0;
	}
#endif
	init_command_buffer();
	if(messageptr)
		send_usbd_packet(usbdev, VIRTUAL_OUT, messageptr, strlen(messageptr), 0);
}

/*
	Sends usbd packet (message) to the given device and address.

	dev - usbd_device to which given message is to be send
	addr - device interface address (eg.: 0x87 etc.)
	messageBuffer - message to be send
	messageBufferLen - length of message to be send
	isCommand - flag determining if given message is a command and has to be executed

	TODO: Remove isCommand variable and split this function into 2 - Command setting and printing ones
*/
void send_usbd_packet(usbd_device *dev, uint8_t addr, const char *messageBuffer, uint16_t messageBufferLen, int isCommand)
{

	if(!usbdev) usbdev = dev;

	//Change to while(...)
	for(int i = 0; i < messageBufferLen; i++){
		//isArrow(...)
		if((GET_ASCII(i) == 27) && 
		   (GET_ASCII(i+1) == 91) && 
		   ((GET_ASCII(i+2) == 66) || 
		   (GET_ASCII(i+2) == 65) || 
		   (GET_ASCII(i+2) == 68) || 
		   (GET_ASCII(i+2) == 67)))
		{
			//Expand
			SKIP_CYCLE(i, 2);
		}
		if(GET_ASCII(i) == 9){
			SKIP_CYCLE(i, 1);
		}

		if(messageBuffer[i] == 27){
			SKIP_CYCLE(i, 1);
		}


		#ifdef SHELL_DEBUG
		    char testbuf[3];
		    sprintf(testbuf, "%d", ((char*)messageBuffer)[i]);
		    while(usbd_ep_write_packet(dev, addr, testbuf, sizeof(testbuf)) == 0);
		    while(usbd_ep_write_packet(dev, addr, "\r\n", sizeof("\r\n")) == 0);
		#endif


		if(GET_ASCII(i) == 13 || GET_ASCII(i) == 10){ //13 == CR
			if(GET_ASCII(i) == 10){
				while(usbd_ep_write_packet(usbdev, addr, "\r", sizeof("\r")) == 0);
			}
			if(GET_ASCII(i) == 13){
				while(usbd_ep_write_packet(usbdev, addr, "\r\n", sizeof("\r\n")) == 0);
			}
			if(isCommand && strlen(getPacketBuffer()) && getPacketPtr()){
				read_command();
				init_command_buffer();
			}
		} else if(GET_ASCII(i) == 127){ //127 == DEL
			if(getPacketPtr()){
				while(usbd_ep_write_packet(usbdev, addr, "\b \b", sizeof("\b \b")) == 0);
				delete_last();
			}
		} else{
			if(getPacketPtr() < MAX_PACKET_SIZE){
				while(usbd_ep_write_packet(usbdev, addr, &(((char*)(messageBuffer))[i]), 1) == 0);
				if(isCommand) add_to_buffer(messageBuffer, i);
			}
		}
	}
}
