/*
 * canary_common.h
 * Common definitions for the Canary project
 * Created: 11/15/2017 8:37:17 PM
 *  Author: Craig
 */ 
#ifndef CANARY_COMMON_H_
#define CANARY_COMMON_H_

/********************************************************************************
    Define CPU frequency before the includes to avoid definition errors...
********************************************************************************/
#define F_CPU 20000000UL

/********************************************************************************
						Includes
********************************************************************************/
#include <avr/io.h>		// Contains the standard IO definitions
#include <avr/interrupt.h>  // Required for interrupt processing
#include <stdio.h>	// Required for UART routines
#include <avr/eeprom.h>  // Required for reading presets from EEPROM
#include <avr/power.h>  // Required for minimizing power consumption
#include <stdlib.h>  //Need to see if this is needed.
#include <util/delay.h>  // Used for debug.  May not need in production
#include <string.h>  //Req'd for formatting strings to send to ground
#include "timercounters.h"  // Timer and counter routines and initialization
#include "UART0.h"  // UART routines and initialization - radio interface
#include "UART1.h"  // UART routines and initialization - gps interface
#include "adc.h"   // Analog to digital routines and initialization
#include "gas_sensors.h"	//Several routines for talking to the gas sensors.
#include "TWI_Master.h"   // I2C routines and initialization
#include "RGBsensor.h"  // Not needed for production - here to test I2C
#include "LIDAR.h"	//LIDAR specific routines

// ************** The following are needed for the file system and MicroSD card
//#include <avr/pgmspace.h>
//#include "ffconf.h"
//#include "ff.h"
//#include "diskio.h"
//#include "tempfat.h"
//#include "myrtc.h"

/********************************************************************************
						Macros
********************************************************************************/
// macros for manipulating bits...
#define SetBit(port, bit) port|=(1<<bit)
#define ClearBit(port, bit) port&=~(1<<bit)
#define ToggleBit(port, bit) port ^= (1<<bit)
#define BitIsSet(port, bit) (port & (1<<bit))
#define BitIsClear(port, bit) (!(port & (1<<bit)))

/********************************************************************************
						Defines
********************************************************************************/
// Some defines for tracking the collection of all sensor data and displaying status
#define gas_sensors 0
#define lidar 1
#define BME280 2
#define GPS 3
#define done 0b00001111

// Some defines for collecting errors:
#define ADC_gnd_test_error 0
#define ADC_BG_test_error 1
#define EEPROM_read_error 2
#define CO_sensor_error 3
#define H2_sensor_error 4
#define NH3_sensor_error 5
#define CH4_sensor_error 6
#define O3_sensor_error 7

/********************************************************************************
						Global Variables
********************************************************************************/
// This variable starts the data collection loop in main
volatile uint8_t ItsTime;
// This next line opens a virtual file that writes to the serial port
static FILE mystdout1 = FDEV_SETUP_STREAM(USART1_Transmit_IO, NULL, _FDEV_SETUP_WRITE);
static FILE mystdout0 = FDEV_SETUP_STREAM(USART0_Transmit_IO, NULL, _FDEV_SETUP_WRITE);
/********************************************************************************
						Function Prototypes
********************************************************************************/
void display_status(uint8_t subsystem, uint8_t status);
void canary_io_pin_initialization(void);

#endif /* CANARY_COMMON_H_ */