/******************************************************************************
 * BME280.h
 *
 * Defines and function prototypes for using the BME280 breakout board
 * from Adafruit.  
 *
 * Created: 11/21/2017 4:23:59 PM
 *  Author: Craig
********************************************************************************/
#ifndef BME280_H_
#define BME280_H_

/********************************************************************************
						Includes
********************************************************************************/
#include <avr/io.h>
#include "canary_common.h"
#include "TWI_Master.h"

/********************************************************************************
						Macros and Defines
********************************************************************************/
#define BME280_address 0x76
#define BME280_WRITE (BME280_address << 1)
#define BME280_READ (BME280_address << 1) | 1
#define BME280_CHIP_ID  0x60
// Register Addresses 
#define BME280_CHIP_ID_ADDR	0xD0
#define BME280_RESET_ADDR 0xE0
#define BME280_TEMP_PRESS_CALIB_DATA_ADDR 0x88
#define BME280_HUMIDITY_CALIB_DATA_ADDR	0xE1
#define BME280_PWR_CTRL_ADDR 0xF4
#define BME280_CTRL_HUM_ADDR 0xF2
#define BME280_CTRL_MEAS_ADDR 0xF4
#define BME280_CONFIG_ADDR 0xF5
#define BME280_DATA_ADDR 0xF7
// API error codes
#define BME280_OK (0)
#define BME280_E_NULL_PTR (-1)
#define BME280_E_DEV_NOT_FOUND (-2)
#define BME280_E_INVALID_LEN (-3)
#define BME280_E_COMM_FAIL (-4)
#define BME280_E_SLEEP_MODE_FAIL (-5)
#define BME280_W_INVALID_OSR_MACRO (1)
// name Macros related to size
#define BME280_TEMP_PRESS_CALIB_DATA_LEN	UINT8_C(26)
#define BME280_HUMIDITY_CALIB_DATA_LEN		UINT8_C(7)
#define BME280_P_T_H_DATA_LEN				UINT8_C(8)
// Sensor power modes 
#define	BME280_SLEEP_MODE		UINT8_C(0x00)
#define	BME280_FORCED_MODE		UINT8_C(0x01)
#define	BME280_NORMAL_MODE		UINT8_C(0x03)
// Macro to combine two 8 bit bytes to form a 16 bit word
#define BME280_CONCAT_BYTES(msb, lsb)     (((uint16_t)msb << 8) | (uint16_t)lsb)
// Other bit manipulation macros
#define BME280_SET_BITS(reg_data, bitname, data) \
				((reg_data & ~(bitname##_MSK)) | \
				((data << bitname##_POS) & bitname##_MSK))
#define BME280_SET_BITS_POS_0(reg_data, bitname, data) \
				((reg_data & ~(bitname##_MSK)) | \
				(data & bitname##_MSK))
#define BME280_GET_BITS(reg_data, bitname)  ((reg_data & (bitname##_MSK)) >> \
							(bitname##_POS))
#define BME280_GET_BITS_POS_0(reg_data, bitname)  (reg_data & (bitname##_MSK))
// Macros for bit masking */
#define BME280_SENSOR_MODE_MSK	UINT8_C(0x03)
#define BME280_SENSOR_MODE_POS	UINT8_C(0x00)
#define BME280_CTRL_HUM_MSK		UINT8_C(0x07)
#define BME280_CTRL_HUM_POS		UINT8_C(0x00)
#define BME280_CTRL_PRESS_MSK	UINT8_C(0x1C)
#define BME280_CTRL_PRESS_POS	UINT8_C(0x02)
#define BME280_CTRL_TEMP_MSK	UINT8_C(0xE0)
#define BME280_CTRL_TEMP_POS	UINT8_C(0x05)
#define BME280_FILTER_MSK		UINT8_C(0x1C)
#define BME280_FILTER_POS		UINT8_C(0x02)
#define BME280_STANDBY_MSK		UINT8_C(0xE0)
#define BME280_STANDBY_POS		UINT8_C(0x05)
// Sensor component selection macros
#define BME280_PRESS		UINT8_C(1)
#define BME280_TEMP			UINT8_C(1 << 1)
#define BME280_HUM			UINT8_C(1 << 2)
#define BME280_ALL			UINT8_C(0x07)
// Settings selection macros 
#define BME280_OSR_PRESS_SEL		UINT8_C(1)
#define BME280_OSR_TEMP_SEL			UINT8_C(1 << 1)
#define BME280_OSR_HUM_SEL			UINT8_C(1 << 2)
#define BME280_FILTER_SEL			UINT8_C(1 << 3)
#define BME280_STANDBY_SEL			UINT8_C(1 << 4)
#define BME280_ALL_SETTINGS_SEL		UINT8_C(0x1F)
// Oversampling macros 
#define BME280_NO_OVERSAMPLING		UINT8_C(0x00)
#define BME280_OVERSAMPLING_1X		UINT8_C(0x01)
#define BME280_OVERSAMPLING_2X		UINT8_C(0x02)
#define BME280_OVERSAMPLING_4X		UINT8_C(0x03)
#define BME280_OVERSAMPLING_8X		UINT8_C(0x04)
#define BME280_OVERSAMPLING_16X		UINT8_C(0x05)
// Standby duration macros 
#define BME280_STANDBY_TIME_1_MS              (0x00)
#define BME280_STANDBY_TIME_62_5_MS           (0x01)
#define BME280_STANDBY_TIME_125_MS			  (0x02)
#define BME280_STANDBY_TIME_250_MS            (0x03)
#define BME280_STANDBY_TIME_500_MS            (0x04)
#define BME280_STANDBY_TIME_1000_MS           (0x05)
#define BME280_STANDBY_TIME_10_MS             (0x06)
#define BME280_STANDBY_TIME_20_MS             (0x07)
//Filter coefficient selection macros 
#define BME280_FILTER_COEFF_OFF               (0x00)
#define BME280_FILTER_COEFF_2                 (0x01)
#define BME280_FILTER_COEFF_4                 (0x02)
#define BME280_FILTER_COEFF_8                 (0x03)
#define BME280_FILTER_COEFF_16 (0x04)

/********************************************************************************
						Global Variables
********************************************************************************/
uint16_t distance;

/********************************************************************************
						Function Prototypes
********************************************************************************/
void BME280_init(void);



#endif /* BME280_H_ */