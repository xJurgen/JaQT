#ifndef __GPIO_CONTROL_H
#define __GPIO_CONTROL_H

#include <stdint.h>

uint8_t flash_write_board_no(uint16_t halfword);
uint8_t* flash_read_board_no();
uint8_t* flash_read_checksum();

#endif
