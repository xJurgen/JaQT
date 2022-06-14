#ifndef __USBSTRINGS_H
#define __USBSTRINGS_H

#include "version.h"

/*

	CDC Shell strings definitions

*/
static const char bufMessageLining[] = "*****************************\r\n";

static const char undefined[] = "[ERROR] Command not found. Type \"help\" to see all commands\r\n";

static const char help[] = "[OK] Commands are:\r\n"
		"help - shows this help\r\n"
		"info - shows board info\r\n"
		"port info - shows an information about cdc_acm ports\r\n"
		"clear - clear screen\r\n"
		"reset device - resets this device\r\n"
		"gpio [n] set [in|out] [1|0] - set gpio into output or input HIGH/LOW state:\r\n"
		"       input low (pull-down): gpio [1-6] set in 0\r\n"
		"       input high (pull-up): gpio [1-14] set in 1\r\n"
		"       output high|low: gpio [1-14|all] set out [1|0]\r\n"
		"gpio [n] get - get value from gpio [1-14], results = (1 | 0)\r\n"
		"wiegand [continuous] [size] start|stop - starts or stops reading from wiegand.\r\n"
		"    Optional arguments:\r\n"
		"       [continuous] - to read from wiegand continually\r\n"
		"       [size] - set wiegand packet size (1-200). Default 26.\r\n"
		"test rs485 - Starts a simulation of Honeywell RS485 tester. Cannot run simultaneously with wiegand.\r\n"
		"stop test rs485 - Stops a simulation of Honeywell RS485 tester, prints test results and sets everything in default state.\r\n"
		"get ina219 - Get data from INA219 DC sensor (if connected).\r\n"
		"set ina219 [address] - Set i2c address of INA219 DC sensor. Default is: 0x40. Usage example: set ina219 0x41\r\n"
		"init ina219 32v 1a - Initializes INA219 on 32V and 1A\r\n"
		"init ina219 16v - Initializes INA219 on 16V and 400mA\r\n"
		"i2c probe - Probes given I2C address. Usage example: i2c probe 0x20\r\n"
		"board_no set [n] - sets the serial number of device to given number in range from 0 to 255.\r\n"
		"     If number has been correctly set, device automatically resets itself.\r\n"
		"board_no get - returns device serial number\r\n"
		"board_no checksum - returns the checksum of saved device serial number\r\n"
#if DEBUG_COMMANDS
"DEBUG:\r\n"
		"read [line] start - starts listening on specified line (232 | 485 | debug | all)\r\n"
		"read [line] stop - stops listening on specified line (232 | 485 | debug | all)\r\n"
#endif
		;

static const char port_info[] = "| TX pin | RX pin | Port | Serial |\r\n"
							   "| ------ | ------ | ---- | ------ |\r\n"
							   "| PB10   | PB11   | ACM0 | RS232  |\r\n"
							   "| PA2    | PA3    | ACM1 | RS485  |\r\n"
							   "| PA9    | PA10   | ACM2 | Debug  |\r\n"
							   "| -      | -      | ACM3 | Shell  |\r\n";

static const char infomessage[] = "[OK] JaQT2 (JustAQuickTest2)\r\n"
							"Version: "FIRMWARE_VERSION"\r\nBuild time: "BUILD_TIME"\r\n";

static const char wiegandStartMessage[] = "[OK] Wiegand reading has started, please connect the device\r\n";

static const char wiegandStopMessage[] = "[OK] Wiegand reading has been stopped\r\n";

static const char underLimitErrorMessage[] = "[ERROR] Wiegand size is equal or lower than 0\r\n";

static const char overLimitErrorMessage[] = "[ERROR] Wiegand size is bigger than 200\r\n";

static const char labelOK[] = "[OK]\r\n";

static const char labelERROR[] = "[ERROR]\r\n";

static const char wiegandDone[] = "Wiegand done\r\n";

static const char dataText[] = "\r\nData: ";

static const char dataTimes[] = "\r\nData times: ";

static const char pauseTimes[] = "\r\nPause times: ";

static const char line0Error[] = "Wiegand line 0 error, try again\r\n";

static const char line1Error[] = "Wiegand line 1 error, try again\r\n";

static const char wiegandPacketErr[] = " - Wiegand packet error\r\n";

static const char timeoutWaitingErr[] = " - Timeout while waiting for next packet ";

static const char missingPackets[] = "(Missing packets)\r\n";

static const char timeoutText[] = "(Timeout)\r\n";

static const char wrongInterval[] = " - Wrong packet interval: ";

static const char packetNumber[] = "Packet number: ";

static const char ina219error[] = "[ERROR] INA219 is not connected!\r\n";

	#ifdef WIEGAND_DEBUG
		static const char time0Times[] = "\r\nTime0 times: ";
		static const char time1Times[] = "\r\nTime1 times: ";
		static const char pause0Times[] = "\r\nPause0 times: ";
		static const char pause1Times[] = "\r\nPause1 times: ";
	#endif

#endif
