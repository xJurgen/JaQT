#ifndef __COMMAND_BUFFER_H
#define __COMMAND_BUFFER_H

#define MAX_PACKET_SIZE 32

#include <stdint.h>

void init_command_buffer();
void delete_last();
void add_to_buffer(const void *messageBuffer, int pos);
uint8_t getPacketPtr();
char *getPacketBuffer();

#endif
