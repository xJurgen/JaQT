/*
*   Author: Jiří Veverka
*   Module implementing flash reading and writing functionalities
*/
#include "flash_eeprom.h"

#include <stdint.h>

#include <libopencm3/stm32/flash.h>


#define FLASH_EEPROM_ADDRESS 	((uint32_t)0x0801F7DC)


uint8_t flash_write_board_no(uint16_t halfword) {

	flash_unlock();

	flash_erase_page(FLASH_EEPROM_ADDRESS);

	flash_program_half_word(FLASH_EEPROM_ADDRESS, halfword);

    /* verify the write */
    if (*(volatile uint16_t *)FLASH_EEPROM_ADDRESS != halfword) {
        return 1;
    }

    return 0;
}

uint8_t* flash_read_board_no() {
    return (uint8_t *)FLASH_EEPROM_ADDRESS;
}

uint8_t* flash_read_checksum() {
    return (uint8_t *)FLASH_EEPROM_ADDRESS + 1;
}
