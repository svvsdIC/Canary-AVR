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
	// For Canary, Port B pins 0, 1, and 2 are the red, green, and blue LED feedback pins
	DDRB  = 0x07;
	//
	// Set all LEDs off
	PORTB &= !0x07;
	// The Canary start button is on Port B pin 3.  Set the pull-up resistor for pin 3:
	SetBit(PORTB, PORTB3);
	//
	// Set the Data Direction Register and pullups for the TWI system
	SetBit(DDRC, PORTC0); // Set clock as output
	SetBit(DDRC, PORTC1); // Data Line as output
	SetBit(PORTC, PORTC0); //Set pull-up resistor for clock
	SetBit(PORTC, PORTC1); //Set pull-up resistor for data
	//
	// For the SPI interface to the uSD card, set MISO as input (with pull-up) and the rest as output:
	SetBit(DDRB, PORTB4); //Chip Select is output
	SetBit(DDRB, PORTB5); //MOSI is output
	ClearBit(DDRB, PORTB6); //MISO is input
	SetBit(PORTB, PORTB6); //Set pull-up resistor for MISO
	SetBit(DDRB, PORTB7); //SPI clock is output
	ClearBit(DDRC, PORTC7); //Chip Detect is an input
	SetBit(PORTC, PORTC7); //Set pull-up resistor for Chip Detect	
	//
	// For the UART, use the pull-up resistors for the RX lines and set the TX lines as output
	SetBit(DDRD, PORTD1); //TX for USART0 is an output to the XBee
	SetBit(DDRD, PORTD3); //TX for USART1 is an output to the GPS (not used operationally)
	ClearBit(PORTD, PORTD0); // Set the GPS Tx pin low.
	ClearBit(DDRD, PORTD0); //RX for USART0 is an input from the XBee (not used operationally)
	SetBit(PORTD, PORTD0); //Set pull-up resistor for USART0 Rx
	ClearBit(DDRD, PORTD2); //RX for USART1 is an input from the GPS
	SetBit(PORTD, PORTD2); //Set pull-up resistor for USART1 Rx
	//
	// Port D pins 4-7 are not used.  Set the pull-up resistors on those pins to prevent issues...
	PORTD |= 0b11110000;
	//
	// All port A pins are set up as inputs.  Pin A6 is not connected, so should enable the pull-up resistor
	SetBit(PORTA, PORTA6);
	//
	// JTAG pins are set automagically
	//	
	
}