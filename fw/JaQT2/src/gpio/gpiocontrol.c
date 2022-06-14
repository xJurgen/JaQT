#include "../general.h"
#include "../i2c/i2cbase.h"
#include "../i2c/i2cgpio.h"
#include "../control_shell/usbstrings.h"
#include "../control_shell/usbshell.h"
#include "../control_shell/tokenizer.h"

#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/gpio.h>

#define GPIO_MIN 1
#define GPIO_MAX 14

static const char gpioUndefined[] = "[ERROR] GPIO not found!\r\n";
static const char gpioValUndefined[] = "[ERROR] Undefined GPIO state!\r\n";

uint16_t gpioMasks[] = {
	0, //offset
	GPIO15,
	GPIO8,
	GPIO15,
	GPIO3,
	GPIO4,
	GPIO5,
	0x01,
	0x02,
	0x04,
	0x08,
	0x10,
	0x20,
	0x40,
	0x80
};

// 0 = GPIOS, 1 = I2C and 2 = BOTH peripherals
enum peripherals{
	PERIPH_GPIO,
	PERIPH_I2C,
	PERIPH_BOTH
};

//base - gpioa, gpiob or i2caddr
//mask - gpio num or i2c cmd
//periph 0 = gpio, 1 = i2c, 2 = both
static void GPIOon(uint32_t base, uint16_t mask, int periph)
{
	if(!base || periph == -1) {
		undefinedCommand();
		return;
	}

	if(periph == PERIPH_GPIO) {

		gpio_set_mode(base, GPIO_MODE_OUTPUT_2_MHZ,
	              GPIO_CNF_OUTPUT_PUSHPULL, mask);
		gpio_set(base, mask);

	} else if(periph == PERIPH_I2C) {

		cmd |= (uint8_t) mask;
		i2c_transfer7(I2C1, base, &cmd, 1, &i2c_data, 1);

	} else if(periph == PERIPH_BOTH) {

		gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_2_MHZ,
	              GPIO_CNF_OUTPUT_PUSHPULL, GPIO15 | GPIO3 | GPIO4 | GPIO5);
		gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_2_MHZ,
	              GPIO_CNF_OUTPUT_PUSHPULL, GPIO8 | GPIO15);
		gpio_set(GPIOB, GPIO15 | GPIO3 | GPIO4 | GPIO5);
		gpio_set(GPIOA, GPIO8 | GPIO15);

		cmd = 0xFF;
		i2c_transfer7(I2C1, base, &cmd, 1, &i2c_data, 1);
	}
}

static void GPIOoff(uint32_t base, uint16_t mask, int periph)
{
	if(!base || periph == -1) {
		undefinedCommand();
		return;
	}

	if(periph == PERIPH_GPIO) {

		gpio_set_mode(base, GPIO_MODE_OUTPUT_2_MHZ,
	              GPIO_CNF_OUTPUT_PUSHPULL, mask);
		gpio_clear(base, mask);

	} else if(periph == PERIPH_I2C) {

		cmd &= ~(uint8_t) mask;
		i2c_transfer7(I2C1, base, &cmd, 1, &i2c_data, 1);

	} else if(periph == PERIPH_BOTH) {

		gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_2_MHZ,
	              GPIO_CNF_OUTPUT_PUSHPULL, GPIO15 | GPIO3 | GPIO4 | GPIO5);
		gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_2_MHZ,
	              GPIO_CNF_OUTPUT_PUSHPULL, GPIO8 | GPIO15);
		gpio_clear(GPIOB, GPIO15 | GPIO3 | GPIO4 | GPIO5);
		gpio_clear(GPIOA, GPIO8 | GPIO15);

		cmd = 0x00;
		i2c_transfer7(I2C1, base, &cmd, 1, &i2c_data, 1);
	}
	return;
}

static uint32_t getBase(uint8_t gpioNum)
{
	if(gpioNum >= GPIO_MIN && gpioNum <= GPIO_MAX) {
		if(gpioNum < 7) {
			if(gpioNum == 2 || gpioNum == 3) return GPIOA;
			else return GPIOB;
		}
		return I2C_EXPANDER_ADDRESS;
	}
	return 0;
}

static void getMask(uint16_t *mask, int *periph, uint8_t *gpioNum)
{
	if(*gpioNum < 7) {
		*mask = gpioMasks[*gpioNum];
		*periph = PERIPH_GPIO;
	}
	else if(*gpioNum <= GPIO_MAX) {
		*mask = gpioMasks[*gpioNum];
		*periph = PERIPH_I2C;
	}
	else *periph = -1;

	return;
}

static uint8_t gpioRange(uint8_t gpio)
{
	if (getBase(gpio))
		return 1;
	return 0;
}

const char* undefinedGpio() {
	return gpioUndefined;
}

const char* undefinedGpioVal() {
	return gpioValUndefined;
}

void setAllGPIOS(uint8_t value)
{
	if (value)
		GPIOon(I2C_EXPANDER_ADDRESS, 0, PERIPH_BOTH);
	else
		GPIOoff(I2C_EXPANDER_ADDRESS, 0, PERIPH_BOTH);
}

int8_t setGPIO(uint8_t gpio, uint8_t value, uint8_t state)
{
	uint16_t mask;
	int periph = 0;

	if (state) {
		if (!gpioRange(gpio)) {
			return 0;
		} else {
			getMask(&mask, &periph, &gpio);

			if (value) {
				GPIOon((uint32_t) getBase(gpio), mask, periph);
			} else {
				GPIOoff((uint32_t) getBase(gpio), mask, periph);
			}
		}
		return 1;
	} else {
		if (!gpioRange(gpio)) {
			return 0;
		} else {
			getMask(&mask, &periph, &gpio);

			if (periph == PERIPH_I2C) {
				cmd |= gpioMasks[gpio];
				i2c_transfer7(I2C1, (uint32_t) getBase(gpio),
				&cmd, sizeof(cmd), NULL, 0);
			}
			gpio_set_mode((uint32_t) getBase(gpio), GPIO_MODE_INPUT,
			GPIO_CNF_INPUT_PULL_UPDOWN, mask);


			//uint32_t port_state = GPIO_ODR(getBase(gpio));
			if (value) // | port_state
				gpio_port_write(getBase(gpio), (mask));	//Input mode - pull-up (write 1 to GPIOx_ODR)
			else // & port_state
				gpio_port_write(getBase(gpio), (~mask));	//Input mode - pull-down (write 0 to GPIOx_ODR

			//unsigned int delay = 0;
			//for (; delay < 72000000; delay++);
		}
		return 1;
	}
}

int16_t getGPIO(uint8_t gpio)
{
	uint16_t mask = 0;
	int periph = 0;
	getMask(&mask, &periph, &gpio);

	if(!gpioRange(gpio)) {
		return -1;
	}

	if (periph == PERIPH_I2C) {
		cmd |= gpioMasks[gpio];
		i2c_transfer7(I2C1, (uint32_t) getBase(gpio),
		&cmd, sizeof(cmd), &i2c_data, sizeof(i2c_data));

		return (i2c_data & mask) ? 1 : 0;
	}

	//return gpio_get((uint32_t) getBase(gpio), mask);
	return (gpio_get((uint32_t) getBase(gpio), mask) > 0) ? 1 : 0;
}


/*------------------STATE LED functions-----------------------*/
uint16_t statusLEDs[] = {
	0, //offset
	GPIO14,
	GPIO13,
	GPIO12,
};

void statusLEDOn(int led)
{
		gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_2_MHZ,
	            GPIO_CNF_OUTPUT_PUSHPULL, statusLEDs[led]);
		gpio_clear(GPIOB, statusLEDs[led]);
}

void statusLEDOff(int led)
{
	gpio_set_mode(GPIOB, GPIO_MODE_INPUT,
				GPIO_CNF_INPUT_PULL_UPDOWN, statusLEDs[led]);

	gpio_set(GPIOB, statusLEDs[led]);
}

