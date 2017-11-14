/******************************************************************************
 * AtmelGasSensors1.c
 *
 * Created: 5/11/2017 3:21:45 PM
 * Author : Morgan Chen & Davita Bird
 ********************************************************************************/ 

/********************************************************************************
    Define CPU frequency before the includes to avoid definition errors...
********************************************************************************/
#define F_CPU 20000000UL

/********************************************************************************
						Includes
********************************************************************************/
#include <avr/io.h>
#include <avr/power.h>
#include <stdlib.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include "UART0.h"
#include "UART1.h"

/********************************************************************************
						Macros and Defines
********************************************************************************/
// macros for manipulating bits...
#define SetBit(port, bit) port|=(1<<bit)
#define ClearBit(port, bit) port&=~(1<<bit)
#define ToggleBit(port, bit) port ^= (1<<bit)
#define BitIsSet(port, bit) (port & (1<<bit))
#define BitIsClear(port, bit) (!(port & (1<<bit)))

//Defines for mapping gas sensors to ADC input pins... (***0 origin or 1 origin???)
#define CO 0
#define H2 1
#define NH3 2
#define CH4 3
#define O3 4
#define max_gas_sample_count 4 // How many samples get averaged into one report

//defines for data collection of gas sensors
uint16_t sen0;

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
volatile uint8_t gas_sensor_id;  // Keeps track of the current sensor being measured
volatile uint8_t gas_sample_count;  // Defines how many samples we've collected so far
volatile uint16_t gas_average;  // variable to hold the current sensor's average reading
volatile uint16_t gas_sensor_boundaries[5][2]; //row=sensor id, column=min and max
volatile uint8_t data_collection_flags;  //Keeps track of which data has been collected
volatile uint8_t gas_sensor_initialization_errors=0; // Keeps track (by bit) of errors we've encountered.
volatile uint8_t gas_sensor_operational_errors=0;  // Keeps track (by bit) of errors we've encountered.
volatile uint16_t u16data = 10;

// These first two variables are here for debug purposes... ---UART STUFF
uint8_t debugdata;
char String[]="Hello World!! The serial port is working!";
// This next line opens a virtual file that writes to the serial port
static FILE mystdout = FDEV_SETUP_STREAM(USART0_Transmit_IO, NULL, _FDEV_SETUP_WRITE);
//static FILE mystdout = FDEV_SETUP_STREAM(USART1_Transmit_IO, NULL, _FDEV_SETUP_WRITE);
// Define the data structure in the EEPROM for reading the gas sensor limit data
uint16_t EEMEM gas_sensor_boundary_constants[5][2]; //row=sensor id, column=min and max
// ***WE NEED TO WRITE THESE VALUES AT SOME POINT***

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

void display_status(uint8_t subsystem, uint8_t status){
	// This is a placeholder routine.  This simply blinks the red or green LED several times depending 
	// upon how the hardware and sensors powered up.
	uint8_t blink_count;
	switch(subsystem) {
		case gas_sensors:
			blink_count=3;
			break;
		case GPS:
			blink_count=4;
			break;
		default:
			blink_count=6;
			status=1;
	}
	if (status==0) {
		for (uint8_t i=0; i<blink_count; i++)
		{
			// If we're good, toggle the green LED
			ToggleBit(PORTB, PORTB1);
			_delay_ms(1000);
			ToggleBit(PORTB, PORTB1);
			_delay_ms(1000);
		} 
	} else { 
		for (uint8_t i=0; i<blink_count; i++)
		{
			// if we're bad, toggle the red LED
			ToggleBit(PORTB, PORTB0);
			_delay_ms(1000);
			ToggleBit(PORTB, PORTB0);
			_delay_ms(1000);
		}
	}	
}


void Initialize_timer_counter_1(void) 
{
	/* This routine is designed to set up a 1Hz interrupt on the 328PA
	F_CPU is expected to be defined before this routine is called.
	@ 20MHz, the prescaler needs to be set to 1024, so CS1[2:0]=0b101.
	The waveform gen mode is 4 (CTC, TOP count in OCR1A), so WGM[3:0]=0b0010
	Not using any of the other compare features, so all other CR bits are 0
	Finally, we have to enable the interrupt vector on compare register A
	TCCR1A = 0; Control Register A is reset to 0x00 on startup. */
	//  
	// Set the WGM mode and prescaler select in control register B
	TCCR1B = ((1<<WGM12) | (1<<CS12) | (1<<CS10));
	// TCCR1C = 0; Control Register C is reset to 0x00 on startup.
	// Now we need to enter out TOP value in OCR1A
	OCR1A = 19530;
	// Now enable the interrupt on the compare register A
	TIMSK1 = (1<<OCIE1A);
	// Timer counter 1 set up.  1Hz interrupts will start when sei() is called
}

void SetMux(uint8_t source) // *** new routine
{
	/* A short routine to clear the ADC input select mux bits then set
	those same bits to the source defined in the input.  */
	ADMUX &= 0b11100000; // Clear the select bits then
	ADMUX |= source; // Set the ADC input source
}

void ADC_init(void)
{
	/* This routine is design to set up the ADC hardware on the ATMega324PA for our use. */
	
	int16_t ADC_result;
	
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
	// SetBit(ADCSRA, ADIE); //*********temporarily turned off*******************
	
	// Set Register ADCSRB to free running mode, single input mode (ADTS[2:0] = 0x00);
	ADCSRB = 0x00;
	
	//  Select test source = ground (GND;  ADMUX.MUX = 0b11111). Result should be 0V.
	SetMux(0b00011111); 
	
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
	SetMux(0b00011110); 
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
	//printf("\nsen = %u",sen0);
	
	// Clear the analog to digital enable bit until we need it again
	ClearBit(ADCSRA, ADEN);
}

void gps_init(void)
{
	int16_t ADC_result;
	SetMux(72);
	// Start conversion
	SetBit(ADCSRA, ADSC);
	while (BitIsClear(ADCSRA, ADIF))
	{
		// Wait for conversion to be complete
	}
	// Read result
	ADC_result = ADC;
	sen0 = ADC_result;
	printf("\nGPS Sensor");
	printf(" = %u", sen0);
}

void gas_sensors_init(void)
{
	volatile uint8_t sensor_id;
	int16_t ADC_result;
	
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
		if (sensor_id == 0)
		{
			printf("\nCarbon Monoxide");
		}
		else if (sensor_id == 1)
		{
			printf("\nHydrogen");
		}
		else if (sensor_id == 2)
		{
			printf("\nAmmonia");
		}
		else if (sensor_id == 3)
		{
			printf("\nMethane");
		}
		else
		{
			printf("\nOzone");
		}

		SetMux(sensor_id);
		// Start conversion
		SetBit(ADCSRA, ADSC);
		while (BitIsClear(ADCSRA, ADIF))
		{
			// Wait for conversion to be complete
		}
		// Read result 
		//ADC_result = ADCL;
		//ADC_result = ADC_result + (ADCH<<8);
		ADC_result = ADC;
		sen0 = ADC_result;

		// Test the reading to ensure it is within acceptable boundaries
		if ((ADC_result < gas_sensor_boundaries[sensor_id][0])|(ADC_result > gas_sensor_boundaries[sensor_id][1]))
		{
			 SetBit(gas_sensor_initialization_errors,(sensor_id+3));  // If not, report an error
		}
		printf(" = %u",ADC_result);
		
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
		SetMux(sensor_id);
		
		total = 0;
		
		for(int i = 0; i < 5; i ++)
		{
			// Start conversion
			SetBit(ADCSRA, ADSC);
			while (BitIsClear(ADCSRA, ADIF))
			{
				// Wait for conversion to be complete
			}
			
			// Test the reading to ensure it is within acceptable boundaries
			if ((ADC_result < gas_sensor_boundaries[sensor_id][0])|(ADC_result > gas_sensor_boundaries[sensor_id][1]))
			{
				SetBit(gas_sensor_initialization_errors,(sensor_id+3));  // If not, report an error
			}
			
			// Read result
			ADC_result = ADC;
			
			total += ADC_result;
		}
		
		gas_average = total / 5;
				
		if (sensor_id == 0)
		{
			printf("\nCarbon Monoxide = %u\n", gas_average);
		}
		else if(sensor_id == 1)
		{
			printf("Hydrogen = %u\n", gas_average);
		}
		else if(sensor_id == 2)
		{
			printf("Ammonia = %u\n", gas_average);
		}
		else if(sensor_id == 3)
		{
			printf("Methane = %u\n", gas_average);
		}
		else
		{
			printf("Ozone = %u\n", gas_average);
		}
		
	}
	gps_init();
	

	
	/*SetMux(gas_sensor_id);  //  point the ADC to the current sensor
	
	// Multiple readings for each sensor will be taken per second, and the average will be transmitted each second. gas_average stores the average.
	gas_average = 0;
	
	// This number tells the program which of the five gas sensors to read
	gas_sensor_id = 0;
	
	SetBit(ADCSRA, ADIE);  //Enable the conversion complete interrupt
	
	gas_sample_count = 0;
	
	SetBit(ADCSRA, ADSC); // start conversion.  Will be completed by the interrupt service routine.
	//printf("\ni = %u", gas_average);*/
}

/********************************************************************************
						Main
********************************************************************************/
int main(void)
{
	
	// *************************************************************************
	//        Initialization code & device configuration
	// *************************************************************************
 	Initialize_timer_counter_1();
	USART0_init(259/*MYUBRR0*/);
	USART1_init(259/*MYUBRR0*/);
	stdout = &mystdout;
	sei();
 	ADC_init();
	start_gas_sensor_read();
 	//gas_sensors_init();
	//start_gas_sensor_read();
	//////////////////////////////////////////////////////////////////////////Maybe put print in here
	//UART code - read gas sensors in serial
	// Wait here for the start/standby button to be selected...
	// Now call the routines to "kick off" the sensor measurements
	// *************************************************************************
	// initialization code & device
	// *************************************************************************
	/* Set the baud rate to 9800 bps using internal 8MHz RC Oscillator */
	
		
	// Provide some data to print out...
	
			
	// enable interrupts
	
		
	// Set up our output file (to computer)
	
	//Now send our fixed string message via USART call
	//USART0_putstring(String);
		
	// Send another text message via our std out using printf
	//printf("Hi, again, world.\n");
	
	//Now try to write debug data to the computer
	//printf("\nunsigned int = %u", u16data);
	//printf("\nPORTA = %u\n", PINA);
// 	for(uint8_t i=0; i<11; i++) 
// 	{
// 		printf("\ni = %u", i);
// 	}
	//end	
	//////////////////////////////////////////////////////////////////////////
 	// *************************************************************************
 	// main loop
 	// *************************************************************************
    while (1) 
    {
		//UART0
		// Echo the received character:  with a terminal window open on your PC,
		// you should see everything you type echoed back to the terminal window
		USART0_TransmitByte(USART0_ReceiveByte());
		// reached steady state...do nothing for the moment - wait for interrupts.
		
		//call gas_sensor_read
		
    }
	printf("hello");
	
}

/********************************************************************************
*********************************************************************************
						Interrupt Service Routines
*********************************************************************************
********************************************************************************/
