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
uint8_t BMEtriggerbyte;
long rawPress, rawTemp, rawHum, t_fine, tempCelsius, pressure, humidity;
/*long var1, var2, p;*/
// Correction parameters for Temperature
uint16_t dig_T1;
short dig_T2, dig_T3;
char temperatureBuf [20];
char time [6];
// Correction parameters for Pressure
uint16_t dig_P1;
short dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
// correction parameters for humidity
uint16_t dig_H1, dig_H3;
short dig_H2, dig_H4, dig_H5, dig_H6;
// This variable starts the data collection loop in main
extern volatile uint8_t ItsTime;
// To test basic functionality of the BME280...
unsigned char BMEmessageBuf[TWI_BUFFER_SIZE], RawBMEdata[40];

/********************************************************************************
						Function Prototypes
********************************************************************************/
void BME_read_correction_coefficients(void);

void bme280basic_init(void);

void bme280basic_bulk_data_read(void);

long BME280_compensate_T_int32(long adc_T);

long BME280_compensate_P_int64(long adc_P);

long bme280_compensate_H_int32(long adc_H);

#endif /* BME280_H*/