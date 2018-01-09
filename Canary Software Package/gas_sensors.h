/*
 * gas_sensors.h
 *
 * This routine contains the processing routines for dealing with the Analog
 * gas sensors in the Canary project
 *
 * Created: 12/30/2017 2:34:22 PM
 *  Author: Craig
 */ 


#ifndef GAS_SENSORS_H_
#define GAS_SENSORS_H_

/********************************************************************************
						Includes
********************************************************************************/
#include "canary_common.h"
#include <stdio.h>

/********************************************************************************
						Macros and Defines
********************************************************************************/
//Defines for mapping gas sensors to ADC input pins... 
#define CO 0
#define H2 1
#define NH3 2
#define CH4 3
#define O3 4
#define max_gas_sample_count 4 // How many samples get averaged into one report

/********************************************************************************
						Global Variables
********************************************************************************/
volatile uint8_t gas_sensor_id;  // Keeps track of the current sensor being measured
volatile uint8_t gas_sample_count;  // Defines how many samples we've collected so far
volatile uint16_t gas_average;  // variable to hold the current sensor's average reading
volatile uint16_t gas_sensor_boundaries[5][2]; //row=sensor id, column=min and max
volatile uint8_t data_collection_flags;  //Keeps track of which data has been collected
volatile uint8_t gas_sensor_initialization_errors; // Keeps track (by bit) of errors we've encountered.
volatile uint8_t gas_sensor_operational_errors;  // Keeps track (by bit) of errors we've encountered.
uint16_t sen0;
uint16_t raw_gas_vector[5]; // holds the average gas sensor reading for all five sensors.
// Define the data structure in the EEPROM for reading the gas sensor limit data
// uint16_t EEMEM gas_sensor_boundary_constants[5][2]; //row=sensor id, column=min and max
// We'll temporarily use this definition....
uint16_t  gas_sensor_boundary_constants[5][2]; //row=sensor id, column=min and max
// ***WE NEED TO WRITE THESE VALUES AT SOME POINT***

/********************************************************************************
						Function Prototypes
********************************************************************************/
void get_gas_sensor_limits(void);
void get_gas_sensor_limits_from_EEPROM(void);
void gas_sensors_init(void);
void start_gas_sensor_read(void);

#endif /* GAS_SENSORS_H_ */