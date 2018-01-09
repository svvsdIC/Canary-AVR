/*
 * RGBsensor.h
 *
 * Created: 11/21/2017 1:56:53 PM
 *  Author: Craig
 */ 


#ifndef RGBSENSOR_H_
#define RGBSENSOR_H_

/********************************************************************************
						Includes
********************************************************************************/
#include <avr/io.h>
#include "TWI_Master.h"

/********************************************************************************
						Macros and Defines
********************************************************************************/
// =========== TCS34725 RGB SENSOR DEFINES =====================
#define TWI_RGB_SENSOR_ADDRESS 0x29 // This is the address for the TCS34725
#define RGB_SENSOR_WRITE TWI_RGB_SENSOR_ADDRESS << 1
#define RGB_SENSOR_READ ((TWI_RGB_SENSOR_ADDRESS << 1)|1)
#define RGB_ENABLE_REGISTER_ADDRESS 0x00
#define RGB_ENABLE_REGISTER_SETTING 0x03
#define TCS34725_ATIME_ADDRESS 0x01 //Integration time
#define TCS34725_WTIME_ADDRESS 0x03 // Wait time (if TCS34725_ENABLE_WEN is asserted)
#define TCS34725_CONTROL_ADDRESS 0x0F // Controls the gain
#define TCS34725_ID_ADDRESS 0x12 // register where the device ID is stored.
#define TCS34725_CDATAL_ADDRESS 0x14 // Clear channel data 
#define TCS34725_CDATAH_ADDRESS 0x15
#define TCS34725_RDATAL_ADDRESS 0x16 // Red channel data
#define TCS34725_RDATAH_ADDRESS 0x17
#define TCS34725_GDATAL_ADDRESS 0x18 // Green channel data 
#define TCS34725_GDATAH_ADDRESS 0x19
#define TCS34725_BDATAL_ADDRESS 0x1A // Blue channel data 
#define TCS34725_BDATAH_ADDRESS 0x1B
#define TCS34725_GAIN_1X 0x00 // Gain = 1x 
#define TCS34725_GAIN_4X 0x01 // Gain = 4x
#define TCS34725_GAIN_16X 0x02 // Gain = 16x
#define TCS34725_GAIN_64X 0x03 // Gain = 64x
#define TCS34725_INTEG_24MS 0xF6
#define TCS34725_INTEG_50MS 0xEB 
#define TCS34725_INTEG_101MS 0xD5  
#define TCS34725_INTEG_154MS 0xC0  
#define TCS34725_INTEG_700MS 0x00


/********************************************************************************
						Global Variables
********************************************************************************/
uint16_t raw_clear, raw_red, raw_green, raw_blue;
volatile unsigned char temp;

/********************************************************************************
						Function Prototypes
********************************************************************************/
void RGBsensor_init(void);
unsigned char read_RGB_values(void);

#endif /* RGBSENSOR_H_ */