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
		return;
	}
	// We reach here when element is not
	// present in array
	//printf('value not in array');
}

void convert_to_ppm(uint8_t sensor_id)
{
	int VALS_TO_MULTIPLY[6] = {9, 6, 10, 8, 7}; // values for multiplication
	int R0_VALS_GAS_SENSORS[6] = {381, 508, 308, 405, 38, 0}; //all R0 values. We may have a 6th sensor but it is currently not in existence
	int R0_RATIOS_CO[6][2][128] = {
		{{11734,3908,2616,2035,1694,1466,1301,1176,1077,996,928,871,822,779,741,707,677,650,626,603,583,564,546,530,515,508,494,481,469,457,446,436,427,417,409,400,393,385,378,371,364,358,352,346,341,335,330,325,320,316,311,307,303,299,295,291,287,284,280,277,273,270,267,264,261,258,256,253,250,248,245,243,240,238,236,233,231,229,227,225,223,221,219,217,215,214,212,210,208,207,205,204,202,200,199,197,196,194,193,192,191,190,189,188,187,186,185,184,183,182,181,180,179,178,177,176,175,174,173,172,171,170,169,169,168,167,166,165}, //ratios CO
			{1,5,9,13,17,21,25,29,33,37,41,45,49,53,57,61,65,69,73,77,81,85,89,93,97,101,105,109,113,117,121,125,129,133,137,141,145,149,153,157,161,165,169,173,177,181,185,189,193,197,201,205,209,213,217,221,225,229,233,237,241,245,249,253,257,261,265,269,273,277,281,285,289,293,297,301,305,309,313,317,321,325,329,333,337,341,345,349,353,357,361,365,369,373,377,381,385,389,393,397,401,405,409,413,417,421,425,429,433,437,441,445,449,453,457,461,465,469,473,477,481,485,489,493,497,501,505,509}}, //ppm CO
		{{58604,19185,10676,7155,5279,4130,3362,2817,2411,2099,1852,1653,1489,1352,1236,1136,1050,975,909,851,799,752,710,672,638,606,577,551,526,504,483,464,446,429,413,398,385,372,359,348,337,327,317,308,299,291,283,275,268,261,255,248,242,237,231,226,221,216,211,207,202,198,194,190,186,183,179,176,173,169,166,161,163,156,153,151,148,146,143,141,139,137,135,133,131,129,127,125,123,121,120,118,116,115,113,112,110,109,108,106,105,104,102,101,100,99,97,96,95,94,93,92,91,90,89,88,87,86,85,84,83,82,81,80,79,78,77,76}, // ratios H2
			{5,12,19,26,33,40,47,54,61,68,75,82,89,96,103,110,117,124,131,138,145,152,159,166,173,180,187,194,201,208,215,222,229,236,243,250,257,264,271,278,285,292,299,306,313,320,327,334,341,348,355,362,369,376,383,390,397,404,411,418,425,432,439,446,453,460,467,474,481,488,495,507,514,521,528,535,542,549,556,563,570,577,584,591,598,605,612,619,626,633,640,647,654,661,668,675,682,689,696,703,710,717,724,731,738,745,752,759,766,773,780,787,794,801,808,815,822,829,836,843,850,857,865,879,886,893,900,910}}, //ppm H2
		{{2854,1930,1648,1490,1384,1305,1244,1193,1151,1115,1083,1055,1030,1008,1000,979,960,943,927,912,898,885,872,861,850,839,829,820,811,786,795,787,780,772,765,758,752,745,739,733,727,721,716,711,706,701,696,691,686,682,678,673,669,665,661,657,653,650,646,643,639,636,633,629,626,623,620,617,614,611,608,606,603,600,597,595,592,590,587,585,583,580,578,576,573,571,569,567,565,563,561,559,557,555,553,551,549,547,545,544,542,540,538,537,535,533,532,530,529,527,525,524,522,521,519,518,516,515,513,512,511,509,508,507,505,504,503,501}, // ratios NH3
			{1,4,7,10,13,16,19,22,25,28,31,34,37,40,43,46,49,52,55,58,61,64,67,70,73,76,79,82,85,94,97,100,103,106,109,112,115,118,121,124,127,130,133,136,139,142,145,148,151,154,157,160,163,166,169,172,175,178,181,184,187,190,193,196,199,202,205,208,211,214,217,220,223,226,229,232,235,238,241,244,247,250,253,256,259,262,265,268,271,274,277,280,283,286,289,292,295,298,301,304,307,310,313,316,319,322,325,328,331,334,337,340,343,346,349,352,355,358,361,364,367,370,373,376,379,382,385,388}}, // ppm NH3
		{{2889,1966,1644,1462,1338,1248,1177,1119,1071,1030,995,964,936,911,888,868,849,832,816,801,787,774,762,750,739,729,719,710,701,693,685,677,670,663,656,649,643,637,631,625,620,615,610,605,600,595,591,586,582,578,574,570,566,562,559,555,552,548,545,542,539,536,533,530,527,524,521,518,516,513,511,508,506,503,501,498,496,494,492,490,487,485,483,481,479,477,475,473,471,470,468,466,464,462,461,459,457,456,454,453,451,449,448,446,445,443,442,440,439,438,436,435,433,432,431,429,428,427,426,424,423,422,421,420,418,417,416,415}, // ratios CH4
			{1,3,5,7,9,11,13,15,17,19,21,23,25,27,29,31,33,35,37,39,41,43,45,47,49,51,53,55,57,59,61,63,65,67,69,71,73,75,77,79,81,83,85,87,89,91,93,95,97,99,101,103,105,107,109,111,113,115,117,119,121,123,125,127,129,131,133,135,137,139,141,143,145,147,149,151,153,155,157,159,161,163,165,167,169,171,173,175,177,179,181,183,185,187,189,191,193,195,197,199,201,203,205,207,209,211,213,215,217,219,221,223,225,227,229,231,233,235,237,239,241,243,245,247,249,251,253,255}}, // ppm CH4
		{{364,362,361,360,358,357,356,354,353,351,350,348,347,346,344,343,341,340,338,337,335,334,332,331,329,328,326,325,323,322,320,319,317,315,314,312,311,309,307,306,304,303,301,299,297,296,294,292,291,289,287,285,284,282,280,278,276,275,273,271,269,267,265,263,261,259,257,255,253,251,249,247,245,243,241,239,237,234,232,230,228,225,223,221,218,216,214,211,209,206,204,201,198,196,193,190,188,185,182,179,176,173,170,167,163,160,157,153,150,146,142,139,135,131,126,122,117,113,108,102,97,91,84,77,69,60,50,36}, // ratios O3
			{128,127,126,125,124,123,122,121,120,119,118,117,116,115,114,113,112,111,110,109,108,107,106,105,104,103,102,101,100,99,98,97,96,95,94,93,92,91,90,89,88,87,86,85,84,83,82,81,80,79,78,77,76,75,74,73,72,71,70,69,68,67,66,65,64,63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1}}}; // ppm O3
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
	numTimes256 = (raw_gas_vector[0]<<VALS_TO_MULTIPLY[valueOfIndex])/(R0_VALS_GAS_SENSORS[sensor_id]); //temporary routine while there is only 1 sensor, will be put into a loop later
	interpolationNum = 0x03 || numTimes256;
	baseIndexToTable = numTimes256 >> 2;
	binary_search(R0_RATIOS_CO[sensor_id][0], 0, 63, baseIndexToTable);
	ppmValue[sensor_id] = (((R0_RATIOS_CO[sensor_id][1][valueOfIndex] - R0_RATIOS_CO[sensor_id][1][valueOfIndex-1])*interpolationNum)>>(VALS_TO_MULTIPLY[valueOfIndex]-2)) + R0_RATIOS_CO[sensor_id][1][valueOfIndex-1];

}
