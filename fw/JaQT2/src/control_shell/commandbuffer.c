#include "commandbuffer.h"
#include "tokenizer.h"
#include "gpiocommands.h"

// Buffer for reading commands to be executed
char packetBuffer[MAX_PACKET_SIZE];

//Pointer to the top of packet buffer (index)
int packetPtr = 0;

uint8_t getPacketPtr()
{
	return packetPtr;
}

char *getPacketBuffer()
{
	return packetBuffer;
}

/*
	Initializes packet (command) buffer
*/
void init_command_buffer() {
	statusOff(2);
	packetPtr = 0;
	for(int i = 0; i < MAX_PACKET_SIZE; i++) {
		packetBuffer[i] = 0;
	}
	for(int i = 0; i < 8; i++) {
		setToken(0, i);
	}
}

/*
	Deletes last symbol in command buffer, is called after backspace was pressed
*/
void delete_last() {
	if(packetPtr != 0)
		packetPtr--;
	if(packetPtr == 0)
		init_command_buffer();
	if(packetPtr)
		packetBuffer[packetPtr] = 0;
}

/*
	Adds received symbol on top of the command buffer
*/
//TODO: Change void to char
void add_to_buffer(const void *messageBuffer, int pos){
	if(packetPtr == MAX_PACKET_SIZE) return;
	if (pos == 0) statusOn(2);
	packetBuffer[packetPtr++] = ((char*)messageBuffer)[pos];
}
