/*
 * canary_common.c
 *
 * Created: 11/15/2017 9:27:40 PM
 *  Author: craig
 */ 

/********************************************************************************
						Includes
********************************************************************************/
#include <avr/io.h>
#include <stdlib.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include "canary_common.h"
#include "gas_sensors.h"
#include "ADC.h"


/********************************************************************************
						Functions
********************************************************************************/
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

void canary_io_pin_initialization(void) {
	// For Canary, Port B pins 0, 1, and 2 are the red, green, and blue LED control (output) pins
	// Port B pin 3 is an input pin, so DDRB pin 3 should be set to zero.
	DDRB  = 0x07;
	//
	// Set all LEDs off
	// PORTB Pin 0(PORTB2) is the RED LED
	// PORTB Pin 1(PORTB2) is the GREEN LED
	// PORTB Pin 2(PORTB2) is the BLUE LED
	PORTB &= !0x07;
	// The Canary start button is on Port B pin 3.  Set the pull-up resistor for pin 3:
	SetBit(PORTB, PORTB3);
	//
	// Set the Data Direction Register and pullups for the TWI system
	SetBit(DDRC, PORTC0); // Set clock as output
	SetBit(DDRC, PORTC1); // Data Line as output
	SetBit(PORTC, PORTC0); //Set pull-up resistor for clock
	SetBit(PORTC, PORTC1); //Set pull-up resistor for data
}