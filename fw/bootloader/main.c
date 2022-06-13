/* *****************************************************************************
 * The MIT License
 *
 * Copyright (c) 2010 LeafLabs LLC.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * ****************************************************************************/

/**
 *  @file main.c
 *
 *  @brief main loop and calling any hardware init stuff. timing hacks for EEPROM
 *  writes not to block usb interrupts. logic to handle 2 second timeout then
 *  jump to user code.
 *
 */

#include "common.h"
#include "dfu.h"
extern volatile dfuUploadTypes_t userUploadType;

/*
----------- ADDED BY xvever12 -----------
    Defines and functions for I2C
    communication.
    This is needed to change the state
    of GPIO expander pins on start.
-----------------------------------------
*/

#define I2C_SR2_DUALF			(1 << 7)

/* SMBHOST: SMBus host header (slave mode) */
#define I2C_SR2_SMBHOST			(1 << 6)

/* SMBDEFAULT: SMBus device default address (slave mode) */
#define I2C_SR2_SMBDEFAULT		(1 << 5)

/* GENCALL: General call address (slave mode) */
#define I2C_SR2_GENCALL			(1 << 4)

/* Note: Bit 3 is reserved, and forced to 0 by hardware. */

/* TRA: Transmitter / receiver */
#define I2C_SR2_TRA			(1 << 2)

/* BUSY: Bus busy */
#define I2C_SR2_BUSY			(1 << 1)

/* MSL: Master / slave */
#define I2C_SR2_MSL			(1 << 0)

/* BTF: Byte transfer finished */
#define I2C_SR1_BTF			(1 << 2)

/* ADDR: Address sent (master mode) / address matched (slave mode) */
#define I2C_SR1_ADDR			(1 << 1)

/* SB: Start bit (master mode) */
#define I2C_SR1_SB			(1 << 0)

/* START: START generation */
#define I2C_CR1_START			(1 << 8)

/* STOP: STOP generation */
#define I2C_CR1_STOP			(1 << 9)


#define I2C1 (0x40000000U + 0x5400) + 0x00
#define I2C_SR1(i2c_base)		GET_REG((i2c_base) + 0x14)
#define I2C_SR2(i2c_base)		GET_REG((i2c_base) + 0x18)
#define I2C_WRITE			0
#define I2C_DR(i2c_base)		GET_REG((i2c_base) + 0x10)
#define I2C_DR_BASE             0x10
#define I2C_CR1(i2c_base)		GET_REG((i2c_base) + 0x00)
#define I2C_CR1_BASE		    0x00

void i2c_send_stop(u32 i2c)
{
	I2C_CR1(i2c) |= I2C_CR1_STOP;
}

void i2c_send_start(u32 i2c)
{
    SET_REG(i2c + I2C_CR1_BASE, I2C_CR1(i2c) | I2C_CR1_START);
}

void i2c_send_7bit_address(u32 i2c, u8 slave, u8 readwrite)
{
	SET_REG(i2c + I2C_DR_BASE, (u8)((slave << 1) | readwrite));
}

void i2c_send_data(u32 i2c, u8 data)
{
    SET_REG(i2c + I2C_DR_BASE, data);
}

static void i2c_write7_v1(u32 i2c, int addr, u8 *data)
{
	while ((I2C_SR2(i2c) & I2C_SR2_BUSY)) {
	}

	i2c_send_start(i2c);

	/* Wait for master mode selected */
	while (!((I2C_SR1(i2c) & I2C_SR1_SB)
		& (I2C_SR2(i2c) & (I2C_SR2_MSL | I2C_SR2_BUSY))));

	i2c_send_7bit_address(i2c, addr, I2C_WRITE);

	/* Waiting for address is transferred. */
	while (!(I2C_SR1(i2c) & I2C_SR1_ADDR));

	/* Clearing ADDR condition sequence. */
	(void)I2C_SR2(i2c);

	for (int i = 0; i < 1; i++) {
		i2c_send_data(i2c, data[i]);
		while (!(I2C_SR1(i2c) & (I2C_SR1_BTF)));
	}

	i2c_send_stop(i2c);
}

void i2c_transfer7(u32 i2c, u8 addr, u8 *w, u8 *r) {
    (void) r;
    i2c_write7_v1(i2c, addr, w);
    //i2c_read7_v1(i2c, addr, r, 1);
}

void setGPIOs(){
    //gpio_write_bit(GPIOC, 13, 0);
    gpio_write_bit(GPIOB, 15, 0);
    gpio_write_bit(GPIOA, 8, 0);
	gpio_write_bit(GPIOA, 15, 0);
    gpio_write_bit(GPIOB, 3, 0);
    gpio_write_bit(GPIOB, 4, 0);
    gpio_write_bit(GPIOB, 5, 0);

    u8 cmd = 0x00;
	u8 data;
	i2c_transfer7(I2C1, 0x20, &cmd, &data);

    /* Disable I2C periph */
    SET_REG(I2C1, GET_REG(I2C1) & ~(1 << 0));
}

/*
----------- END OF SECTION -----------
*/

int main()
{
    bool no_user_jump = FALSE;
    bool dont_wait=FALSE;

    systemReset(); // peripherals but not PC
    setupCLK();
    setupLEDAndButton();
    setupUSB();
    setupFLASH();
    setGPIOs(); //Added by xvever12 - setsGPIOs

    switch(checkAndClearBootloaderFlag())
    {
        case 0x01:
            no_user_jump = TRUE;
#if defined(LED_BANK) && defined(LED_PIN) && defined(LED_ON_STATE)
            strobePin(LED_BANK, LED_PIN, STARTUP_BLINKS, BLINK_FAST,LED_ON_STATE);
#endif
        break;
        case 0x02:
            dont_wait=TRUE;
        break;
        default:
			#ifdef FASTBOOT
				dont_wait=TRUE;
			#else
				#if defined(LED_BANK) && defined(LED_PIN) && defined(LED_ON_STATE)
							strobePin(LED_BANK, LED_PIN, STARTUP_BLINKS, BLINK_FAST,LED_ON_STATE);
				#endif
			#endif            

            if (!checkUserCode(USER_CODE_FLASH0X8005000) && !checkUserCode(USER_CODE_FLASH0X8002000))
            {
                no_user_jump = TRUE;
            }
            else if (readButtonState())
            {
				no_user_jump = TRUE;
				#ifdef FASTBOOT
					dont_wait=FALSE;
				#endif
            }
        break;
    }

    if (!dont_wait)
    {
        int delay_count = 0;

        while ((delay_count++ < BOOTLOADER_WAIT) || no_user_jump)
        {
#if defined(LED_BANK) && defined(LED_PIN) && defined(LED_ON_STATE)
            strobePin(LED_BANK, LED_PIN, 1, BLINK_SLOW,LED_ON_STATE);
#endif
            if (dfuUploadStarted())
            {
                dfuFinishUpload(); // systemHardReset from DFU once done
            }
        }
    }

    if (checkUserCode(USER_CODE_FLASH0X8002000))
    {
        jumpToUser(USER_CODE_FLASH0X8002000);
    }
    else
    {
        if (checkUserCode(USER_CODE_FLASH0X8005000))
        {
            jumpToUser(USER_CODE_FLASH0X8005000);
        }
        else
        {
            // Nothing to execute in either Flash or RAM
#if defined(LED_BANK) && defined(LED_PIN) && defined(LED_ON_STATE)
            strobePin(LED_BANK, LED_PIN, 5, BLINK_FAST,LED_ON_STATE);
#endif
            systemHardReset();
        }
    }

    return 0;// Added to please the compiler
}
