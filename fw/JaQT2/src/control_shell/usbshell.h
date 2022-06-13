/*
*   Basic USB shell for virtual serial comm
*   Author: Jiří Veverka
*/
#ifndef __USBSHELL_H
#define __USBSHELL_H

#include <libopencm3/usb/usbd.h>
#include <stdint.h>

void send_usbd_packet(usbd_device *dev, uint8_t addr, const char *messageBuffer, uint16_t messageBufferLen, int isCommand);

#if DEBUG_COMMANDS
extern int read232;
extern int read485;
extern int readdebug;
#endif

void undefinedCommand();

#endif
