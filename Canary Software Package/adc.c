/********************************************************************************
 * adc.c
 *
 * This set of functions initializes the ADC peripheral and tests it against
 * ground (0 volts) and the bandgap (1.1 volts).    It also provides a routine
 * for pointing the mux at different inputs.
 *
 * Created: 8/14/2017 2:33:20 PM
 * Author: Craig R
 *******************************************************************************/ 

/********************************************************************************
						Includes
********************************************************************************/
#include "canary_common.h"
#include "adc.h"

/********************************************************************************
						Macros and Defines
********************************************************************************/
// none??

/********************************************************************************
						Variable definitions
********************************************************************************/
// Keeps track (by bit) of errors we've encountered.
volatile uint8_t gas_sensor_initialization_errors; 

/********************************************************************************
						Functions
********************************************************************************/
void ADC_SetMux(uint8_t source) // 
{
	/* A short routine to clear the ADC input select mux bits then set
	those same bits to the source defined in the input.  */
	ADMUX &= 0b11100000; // Clear the select bits then
	ADMUX |= source; // Set the ADC input source
}

void ADC_init(void)
{
	/* This routine is design to set up the ADC hardware on the ATMega324PA for our use. */
	static volatile int16_t ADC_result;
	
	// Select the external AVCC pin as the ADC reference voltage by setting the REFS[1:0] pin in the ADC MUX register to 0b01
	SetBit(ADMUX, REFS0);
	
	// Set clock prescaler to 128 to provide a 156kHz ADC clock.  ADPS[2:0] = 0b111
	SetBit(ADCSRA, ADPS0);
	SetBit(ADCSRA, ADPS1);
	SetBit(ADCSRA, ADPS2);
	
	// Clears power reduction register to ensure we get power to the ADC hardware
	ClearBit(PRR0, PRADC);
	
	// Clears the Left Adjust Result bit. This means that output will be right adjusted to get the max resolution
	ClearBit(ADMUX, ADLAR);
	
	// Sets Digital Input Disable Register. Writing a one to bits in the DIDR0 to disable the digital input circuitry
	DIDR0 = 0b00111111;
	
	// Enable the ADC
	SetBit(ADCSRA, ADEN);
	
	// Enable the ADC interrupt
	//*************TURNED OFF FOR LAB TESTING********************************
	// SetBit(ADCSRA, ADIE);  // May have to turn this off for debug
	
	// Set Register ADCSRB to free running mode, single input mode (ADTS[2:0] = 0x00);
	ADCSRB = 0x00;
	
	//  Select test source = ground (GND;  ADMUX.MUX = 0b11111). Result should be 0V.
	ADC_SetMux(0b00011111); 
	
	// Discard the first conversion and use the second one
	// Sets ADC Start Conversion bit
	SetBit(ADCSRA, ADSC);
	while (BitIsClear(ADCSRA, ADIF))
	{
		// Wait for conversion to be complete
	}
	
	//   Restart conversion. We'll use this result.
	SetBit(ADCSRA, ADSC);
	while (BitIsClear(ADCSRA, ADIF))
	{
		// Wait for conversion to be complete
	}
	
	//  Read result and report error if not real close to zero
	ADC_result = ADCL;
	ADC_result = ADC_result + (ADCH<<8);
	if (abs(ADC_result) >4 ) //   We want to be within a few LSBs of zero.
	{
		SetBit(gas_sensor_initialization_errors, ADC_gnd_test_error);
	}
	
	// Select test source = bandgap (BG;  ADMUX.MUX = 0b11110). Result should be 1.1V
	ADC_SetMux(0b00011110); 
	SetBit(ADCSRA, ADSC); // Start conversion process
	while (BitIsClear(ADCSRA, ADIF))
	{
		// Wait for conversion to be complete
	}
	
	// Read result and report error if not 1.1
	ADC_result = ADCL;
	ADC_result = ADC_result + (ADCH<<8);
	ADC_result -= 225;  // 225 is the expected value: 1.1V*1023/5v=225
	if (abs(ADC_result) >= 5)  // We want to be within 4 of target voltage reading.
	{
		SetBit(gas_sensor_initialization_errors, ADC_BG_test_error);
	}
	
	// Clear the analog to digital enable bit until we need it again
	ClearBit(ADCSRA, ADEN);
}