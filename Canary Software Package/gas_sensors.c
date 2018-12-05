/*
 * gas_sensors.c
 * 
 * This routine contains the processing routines for dealing with the Analog 
 * gas sensors in the Canary project
 *
 * Created: 11/15/2017 8:55:43 PM
 *  Author: Craig
 */ 
/********************************************************************************
						Includes
********************************************************************************/
#include "gas_sensors.h"

/********************************************************************************
						Functions
********************************************************************************/
void get_gas_sensor_limits(void) {
	uint8_t ii;
	/* THIS IS A TEMPORARY ROUTINE, to be replaced by the EEPROM read commented out below.
	This just sets some upper and lower bounds for the initialization code to use.
	Actual values will need to be derived empirically (using the UCAR test chamber) */
	for (ii=0; ii<5; ii++)
	{
		gas_sensor_boundaries[ii][0] = 0;
		gas_sensor_boundaries[ii][1] = 1023;
	}
}

void get_gas_sensor_limits_from_EEPROM(void) {
	uint8_t ii;
	// This routine reads the empirically derived upper and lower limits
	// for the gas sensors from EEPROM.  We can also use defined constants
	// if we need to...
	for (ii=0; ii<5; ii++)
	{
		gas_sensor_boundaries[ii][0] = eeprom_read_word(&gas_sensor_boundary_constants[ii][0]);
		gas_sensor_boundaries[ii][1] = eeprom_read_word(&gas_sensor_boundary_constants[ii][1]);
	}
}

void gas_sensors_init(void)
{
	volatile uint8_t sensor_id;
	int16_t ADC_result;
	
	// Clear our error trackers
	gas_sensor_initialization_errors=0; // Keeps track (by bit) of errors we've encountered.
	gas_sensor_operational_errors=0;  // Keeps track (by bit) of errors we've encountered.
	// Get the upper and lower boundaries on the gas sensors...
	// The first cal is just to test the EEPROM code
	get_gas_sensor_limits_from_EEPROM();
	// This second cal is temporary and simply initializes the array to usable values...
	get_gas_sensor_limits();
	
	// Enable the ADC
	SetBit(ADCSRA, ADEN);
	
	//Read initial value from the sensors
	for(sensor_id = 0; sensor_id < 5; sensor_id++)
	{
		ADC_SetMux(sensor_id);
		// Start conversion
		SetBit(ADCSRA, ADSC);
		while (BitIsClear(ADCSRA, ADIF))
		{
			// Wait for conversion to be complete
		}
		// Read result
		ADC_result = ADC;
		// sen0 = ADC_result;
		// Test the reading to ensure it is within acceptable boundaries
		if ((ADC_result < gas_sensor_boundaries[sensor_id][0])|(ADC_result > gas_sensor_boundaries[sensor_id][1]))
		{
			SetBit(gas_sensor_initialization_errors,(sensor_id+3));  // If not, report an error
		}		
	}
	ClearBit(ADCSRA, ADEN);  //Disable the ADC until we go operational
	// Report status via the LED that the gas sensors are ready (or not)
	//display_status(gas_sensors, gas_sensor_initialization_errors);
}

void start_gas_sensor_read(void)
{
	volatile uint8_t sensor_id;
	int16_t ADC_result;
	volatile int16_t total;
	int16_t gas_average;
	
	// Enable the ADC
	SetBit(ADCSRA, ADEN);
	//Read initial value from the sensors
	for(sensor_id = 0; sensor_id < 5; sensor_id++)
	{
		ADC_SetMux(sensor_id);
		total = 0;	// re-zero the average
		
		for(int i = 0; i < max_gas_sample_count; i++)
		{
			// Start conversion
			SetBit(ADCSRA, ADSC);
			while (BitIsClear(ADCSRA, ADIF))
			{
				// Wait for conversion to be complete
				// NOTE: In production version, we should wait for the interrupt and process in the background
			}
			// Read result
			ADC_result = ADC;
			// Test the reading to ensure it is within acceptable boundaries
			if ((ADC_result < gas_sensor_boundaries[sensor_id][0])|(ADC_result > gas_sensor_boundaries[sensor_id][1]))
			{
				SetBit(gas_sensor_initialization_errors,(sensor_id+3));  // If not, report an error
			}
			total += ADC_result;
		}
		gas_average = total / max_gas_sample_count;
		raw_gas_vector[sensor_id] = gas_average;
	}
}

void binary_search(int array[], int top, int bottom, int number) //return value directly above & below number
{
	if ((number > array[top]) && (number < array[bottom]))
	{
		int mid = bottom + (top - bottom)/2;
		
		// If the element is closest to the middle
		if (number >= array[mid] && number <= array [mid + 1])
		{
			valueOfIndex = mid + 1;
		}
		
		// If element is smaller than mid, then
		// it can only be present in left subarray
		else if (number < array[mid])
		return binary_search(array, mid, bottom, number);
		
		// Else the element can only be present
		// in right subarray
		return binary_search(array, top, mid+1, number);
	}
	else
	{
		return 0;
	}
	// We reach here when element is not
	// present in array
	//printf('value not in array');
}

uint16_t convert_to_ppm(uint8_t sensor_id)
{
	int R0_VALS_GAS_SENSORS[6] = {381, 508, 308, 405, 38, 0}; //all R0 values. We may have a 6th sensor but it is currently not in existence
	int R0_RATIOS_CO[2][64] = {{101, 99, 98, 96, 95, 93, 92, 90, 89, 87, 86, 84, 83, 81, 80, 78, 77, 75, 74, 72, 71, 69, 67, 66, 64, 63, 61, 60,
	58, 57, 55, 54, 52, 51, 49, 48, 46, 45, 43, 42, 40, 39, 37, 35, 34, 32, 31, 29, 28, 26, 25, 23, 22, 20, 19, 17, 16, 14, 13, 11, 10, 8, 7, 5}, //R0 ratio corresponding to certain ppm val
	{44, 52, 53, 54, 55, 57, 57, 59, 60, 62, 64, 66, 67, 70, 71, 74, 75, 78, 80, 83, 85, 88, 92, 94, 99, 102, 107, 110, 116, 119, 125, 128,
	135, 139, 146, 150, 159, 163, 174, 180, 192, 200, 216, 233, 243, 264, 278, 306, 323, 359, 381, 432, 466, 542, 586, 699, 779, 980, 1101, 1434, 1684, 2409, 2955, 4563}};//PPM values
	baseIndexToTable = 0;
	interpolationNum = 0;
	numTimes256 = 0;
	valueOfIndex = 0;
	//Take the smallest R1/R0 value and the largest R1/R0 value from the original set of data points
	//Multiply them by a number that is a power of two (so that the values are large enough to divide by 64 later)
	//Round the two values to the nearest integer value
	//Divide the values by 64
	//Now you should have a table of 64 values ranging from the original smallest value to the original largest value
	//Divide each of the 64 values by the number that is a power of two
	//Create a lookup table for the values, this will create a list of approx. ppm values
	//Manually enter in new equation to calculate the actual ppm
	
	//Per sensor - make look-up table w/ values corresponding to out put. 
	//Get approx ppm
	//write equation
	//First sensor is CO
	numTimes256 = (raw_gas_vector[0]<<8)/(R0_VALS_GAS_SENSORS[0]); //temporary routine while there is only 1 sensor, will be put into a loop later
	interpolationNum = 0x03 || numTimes256;
	baseIndexToTable = numTimes256 >> 2;
	binary_search(R0_RATIOS_CO[0], 0, 63, baseIndexToTable);
	ppmValue[0] = (((R0_RATIOS_CO[1][valueOfIndex] - R0_RATIOS_CO[1][valueOfIndex-1])*interpolationNum)>>4) + R0_RATIOS_CO[1][valueOfIndex-1];

}
