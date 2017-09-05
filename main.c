/*
 * LIDAREditedLibrary.c
 *
 * Created: 8/29/2017 2:25:59 PM
 * Author : chen.morgan01
 */ 

#include <avr/io.h>

// Default address is 0x62

void begin()
{
	// Start I2C
	// Configure for maximum range
}

void measure()
{
	// If measurement is being taken with bias correction, write 0x04 to 0x00
	// If measurement is being taken without bias correction, write 0x03 to 0x00
	// Create array to store distance
	// Read two bytes from 0x8f
	// The distance is the first byte shifted left 8 plus the second byte
}

// Extra: add counter so that if device doesn't respond enough times in a row it stops the read
void read()
{
	// Wait until device is not busy
	// Once device is not busy:
		// Set the register to be read
		// Report error over serial if device is not responding
		// Read the value
		// Add to an array
}

void write()
{
	// Take in address to be written to
	// Take in value to be written
	// Write value to device with I2C
	// Report error over serial if device is not responding
	// Wait 1 millisecond for better quality in successive writes
}

void reset()
{
	// Write 0x00 to 0x00
}

int main(void)
{
	// begin();
	// Interrupt setup stuff
	
    /* Replace with your application code */
    while (1) 
    {
    }
}

// 1 Hz Interrupt:
	// measure();

