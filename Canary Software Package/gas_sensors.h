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
#define max_gas_sample_count 4 // How many samples get averaged into one report; use factor of two

/********************************************************************************
						Global Variables
********************************************************************************/
volatile uint8_t R0; //Keeps track of the voltage of the current sensor
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
//variables for calibration routines only used for when not in calibration
int R0_VALS_GAS_SENSORS[6] = {381, 508, 308, 405, 38, 0}; //all R0 values. We may have a 6th sensor but it is currently not in existence
int R0_RATIOS_CO[2][64];
R0_RATIOS_CO[0] = {101, 99, 98, 96, 95, 93, 92, 90, 89, 87, 86, 84, 83, 81, 80, 78, 77, 75, 74, 72, 71, 69, 67, 66, 64, 63, 61, 60,
58, 57, 55, 54, 52, 51, 49, 48, 46, 45, 43, 42, 40, 39, 37, 35, 34, 32, 31, 29, 28, 26, 25, 23, 22, 20, 19, 17, 16, 14, 13, 11, 10, 8, 7, 5}; //R0 ratio values (x64)
R0_RATIOS_CO[1] = {43.740732, 51.99912229, 52.77702869, 54.33284151, 55.11074792, 56.66656074, 57.44446715, 59.03378881, 60.188585, 62.49817738,
	63.65297357, 65.96256595, 67.11736214, 69.68246398, 71.12536736, 74.01117411, 75.45407748, 78.33988423, 79.78278761, 83.12547386, 84.80048068,
	88.15049432, 91.50050796, 93.80909189, 99.2448773, 101.96277, 107.4199221, 110.2733166, 115.9801057, 118.8335003, 124.6256075, 128.1237867,
	135.1201452, 138.6183244, 145.9536912, 150.2446403, 158.8265385, 163.1174876, 173.7166757, 179.9672873, 192.4685105, 200.0052686, 216.0398659,
	233.3410534, 243.2136832, 263.5427718, 277.657019, 305.8855135, 323.0957871, 358.8166888, 381.3626714, 432.3686829, 466.2362158, 542.0774711,
585.5972283, 698.7645118, 778.8746442, 980.2963805, 1100.959331, 1434.302565, 1683.737691, 2408.727606, 2955.401483, 4563.139439};//PPM values
int baseIndexToTable = 0;
int interpolationNum = 0;
int numTimes256 = 0;
int ppmValue[6] = {0, 0, 0, 0, 0, 0};
int valueOfIndex = 0;
/********************************************************************************
						Function Prototypes
********************************************************************************/
void get_gas_sensor_limits(void);
void get_gas_sensor_limits_from_EEPROM(void);
void gas_sensors_init(void);
void start_gas_sensor_read(void);
uint16_t convert_to_ppm(uint8_t sensor_id);
void binary_search(int[], int, int, int);

#endif /* GAS_SENSORS_H_ */