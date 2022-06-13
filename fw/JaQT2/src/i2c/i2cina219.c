/*
    Author: Jiří Veverka
    Code inspired and based on: https://github.com/FriedCircuits/USBMultimeter/tree/master/STM32F429-Discovery-Prototype/Firmware/lib/ina219
*/

#include "i2cina219.h"
#include <stdlib.h>
#include <libopencm3/stm32/i2c.h>

#define INA219_REG_CONFIG                      (0x00)

#define INA219_CONFIG_BVOLTAGERANGE_16V        (0x0000)  // 0-16V Range
#define INA219_CONFIG_BVOLTAGERANGE_32V        (0x2000)  // 0-32V Range
#define INA219_CONFIG_GAIN_1_40MV              (0x0000)  // Gain 1, 40mV Range
#define INA219_CONFIG_GAIN_8_320MV             (0x1800)  // Gain 8, 320mV Range
#define INA219_CONFIG_BADCRES_12BIT            (0x0400)  // 12-bit bus res = 0..4097
#define INA219_CONFIG_SADCRES_12BIT_1S_532US   (0x0018)  // 1 x 12-bit shunt sample
#define INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS (0x0007)

#define INA219_REG_SHUNTVOLTAGE                (0x01)
#define INA219_REG_BUSVOLTAGE                  (0x02)
#define INA219_REG_POWER                       (0x03)
#define INA219_REG_CURRENT                     (0x04)
#define INA219_REG_CALIBRATION                 (0x05)

static void ina219WriteRegister(uint8_t reg, uint16_t value);
static int16_t ina219ReadRegister(uint8_t reg_addr);

static int16_t ina219GetPower_raw(void);
static int16_t ina219GetBusVoltage_raw(void);
static int16_t ina219GetShuntVoltage_raw(void);
static int16_t ina219GetCurrent_raw(void);

static void ina219SetCalibration_32V_2A(void);
static void ina219SetCalibration_32V_1A(void);
static void ina219SetCalibration_16V_400mA(void);

uint32_t ina219CurrentDivider_mA = 0;
float ina219PowerDivider_mW = 0;
uint8_t ina219_addr = 0x40;

void set_ina219_addr(uint8_t addr)
{
	ina219_addr = addr;
}

uint8_t get_ina219_addr(void)
{
	return ina219_addr;
}

void ina219Init_32V_2A(void) {
	ina219SetCalibration_32V_2A();
}

void ina219Init_32V_1A(void) {
	ina219SetCalibration_32V_1A();
}

void ina219Init_16V_400mA(void) {
	ina219SetCalibration_16V_400mA();
}

static void ina219WriteRegister (uint8_t reg_addr, uint16_t value)
{
	uint8_t	*write_buf = malloc(sizeof(write_buf)*3);
	write_buf[0] = reg_addr;
	write_buf[1] = ((value >> 8) & 0xFF);
	write_buf[2] = (value & 0xFF);

	i2c_transfer7(I2C1, ina219_addr, write_buf, 3, 0, 0);
	free(write_buf);
}

static int16_t ina219ReadRegister(uint8_t reg_addr)
{
	uint8_t *read_buf = malloc(sizeof(read_buf)*2);
	read_buf[0] = 0;
	read_buf[1] = 0;

	i2c_transfer7(I2C1, ina219_addr, &reg_addr, sizeof(reg_addr), read_buf, 2);

	uint16_t result = (((uint16_t)read_buf[0]) << 8) | read_buf[1];
	free(read_buf);
	return result;
}

static void ina219SetCalibration_32V_2A(void)
{
	ina219CurrentDivider_mA = 10;  // Current LSB = 100uA per bit (1000/100 = 10)
	ina219PowerDivider_mW = 2;     // Power LSB = 1mW per bit (2/1)

	// Set Calibration register to 'Cal' calculated above
	ina219WriteRegister(INA219_REG_CALIBRATION, 0x1000);

    // Set Config register to take into account the settings above
	uint16_t config = INA219_CONFIG_BVOLTAGERANGE_32V |
		INA219_CONFIG_GAIN_8_320MV |
		INA219_CONFIG_BADCRES_12BIT |
		INA219_CONFIG_SADCRES_12BIT_1S_532US |
		INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS;
	ina219WriteRegister(INA219_REG_CONFIG, config);
}

static void ina219SetCalibration_32V_1A(void)
{
	// Set multipliers to convert raw current/power values
	ina219CurrentDivider_mA = 25;      // Current LSB = 40uA per bit (1000/40 = 25)
	ina219PowerDivider_mW = 1;         // Power LSB = 800uW per bit

	// Set Calibration register to 'Cal' calculated above
	ina219WriteRegister(INA219_REG_CALIBRATION, 0x2800);

	// Set Config register to take into account the settings above
	uint16_t config = INA219_CONFIG_BVOLTAGERANGE_32V |
			INA219_CONFIG_GAIN_8_320MV |
			INA219_CONFIG_BADCRES_12BIT |
			INA219_CONFIG_SADCRES_12BIT_1S_532US |
			INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS;
	ina219WriteRegister(INA219_REG_CONFIG, config);
}

static void ina219SetCalibration_16V_400mA(void)
{
	// Set multipliers to convert raw current/power values
	ina219CurrentDivider_mA = 20;      // Current LSB = 40uA per bit (1000/40 = 25)
	ina219PowerDivider_mW = 1;         // Power LSB = 800uW per bit

	// Set Calibration register to 'Cal' calculated above
	ina219WriteRegister(INA219_REG_CALIBRATION, 8192);

	// Set Config register to take into account the settings above
	uint16_t config = INA219_CONFIG_BVOLTAGERANGE_16V |
			INA219_CONFIG_GAIN_1_40MV |
			INA219_CONFIG_BADCRES_12BIT |
			INA219_CONFIG_SADCRES_12BIT_1S_532US |
			INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS;
	ina219WriteRegister(INA219_REG_CONFIG, config);
}

float ina219GetPower_mW()
{
	float value = ina219GetPower_raw();
	value *= ina219PowerDivider_mW;
	return value;
}

float ina219GetShuntVoltage_mV()
{
	return ina219GetShuntVoltage_raw() * 0.01;
}

float ina219GetCurrent_mA()
{
	float value = ina219GetCurrent_raw();
	value /= ina219CurrentDivider_mA;
	return value;
}

float ina219GetBusVoltage_V()
{
	return ina219GetBusVoltage_raw() * 0.001;
}

/* ------------RAW------------ */

static int16_t ina219GetPower_raw()
{
	return ina219ReadRegister(INA219_REG_POWER);
}

static int16_t ina219GetShuntVoltage_raw()
{
	return ina219ReadRegister(INA219_REG_SHUNTVOLTAGE);
}

static int16_t ina219GetCurrent_raw()
{
	return ina219ReadRegister(INA219_REG_CURRENT);
}

static int16_t ina219GetBusVoltage_raw()
{
	uint16_t value = ina219ReadRegister(INA219_REG_BUSVOLTAGE);

	// Shift to the right 3 to drop CNVR and OVF and multiply by LSB
	return (int16_t)((value >> 3) * 4);
}
