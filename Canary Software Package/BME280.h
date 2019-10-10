/*
 * BME280.h
 *
 * This routine contains the processing routines for dealing with the BME280 sensor in the Canary project
 *
 * Created: 12/30/2017 2:34:22 PM
 *  Author: Canary Team
 */ 


#ifndef BME280_H_
#define BME280_H_

/********************************************************************************
						Includes
********************************************************************************/
#include "canary_common.h"
#include <stdio.h>

/********************************************************************************
						Macros and Defines
********************************************************************************/
#define BME280_ADDRESS                (0x76)
#define BME_WRITE_ADDRESS	(BME280_ADDRESS<<1)
#define BME_READ_ADDRESS	((BME280_ADDRESS<<1)|1)


/********************************************************************************
						Global Variables
********************************************************************************/
int32_t rawPress, rawTemp, rawHum, t_fine, tempCelsius, pressure, humidity;
// Correction parameters for Temperature
uint16_t dig_T1;
int16_t dig_T2, dig_T3;
uint8_t temperatureBuf [20];
uint8_t time [6];
// Correction parameters for Pressure
uint16_t dig_P1;
int16_t dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
// correction parameters for humidity
uint8_t dig_H1, dig_H3;
int16_t dig_H2, dig_H4, dig_H5;
int8_t dig_H6;
// This variable starts the data collection loop in main
extern volatile uint8_t ItsTime;
// To test basic functionality of the BME280...
uint8_t BMEmessageBuf[TWI_BUFFER_SIZE], RawBMEdata[40];

// We want Temperature oversampling set to x1 (ctrl_meas (0xF4) [7:5] = 0b001)
// We want Pressure oversampling set to x8 (ctrl_meas (0xF4) [4:2] = 0b100)
// Put the device into Forced mode (we want to tell the device to "go measure") (ctrl_meas (0xF4) [1:0] = 0b01)
#define BMEtriggerbyte (0b01<<5) | (0b100<<2) | (0b01<<0)

/********************************************************************************
						Function Prototypes
********************************************************************************/
void BME_read_correction_coefficients(void);

void bme280basic_init(void);

void bme280basic_bulk_data_read(void);

int32_t BME280_compensate_T_int32(long adc_T);

int32_t BME280_compensate_P_int64(long adc_P);

uint32_t bme280_compensate_H_int32(long adc_H);

#endif /* BME280_H*/