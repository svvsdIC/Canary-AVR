/******************************************************************************
 * main.c
 * 
 * This is the main routine for all of the Canary airborne software.  The .h
 * and associated .c files contain the routines for individual sensors or 
 * chip peripherals.  The code was migrated to and tested on an Atmel1284P
 * device.  It should work the same on the 324PAs in the IC lab.
 *
 *
 * Created: 5/11/2017 3:21:45 PM
 * Author : chen.morgan01
 ********************************************************************************/ 

/********************************************************************************
						Includes
********************************************************************************/
#include "canary_common.h" // Contains all Canary Project global definitions
#include <avr/io.h>		// Contains the standard IO definitions

/********************************************************************************
						Macros and Defines
********************************************************************************/


/********************************************************************************
						Global Variables
********************************************************************************/
// These first few variables are here for debug purposes... ---UART STUFF
volatile uint16_t u16data = 10, seconds; 
uint8_t debugdata;
char String[]="Hello World!! The serial port is working!";
extern char messageWant [UART1_RX_BUFFER_SIZE];

/********************************************************************************
						Functions
********************************************************************************/

/********************************************************************************
						Main
********************************************************************************/
int main(void)
{
	
	// *************************************************************************
	//        Initialization code & device configuration
	// *************************************************************************
 	//
	// Set the I/O pins
	canary_io_pin_initialization();
	//
	// Set a simple counter for the loop below.  Debug only.
	seconds=0;
	ItsTime=0;
	//
	// set up our output for handling printf and string operations,,,
	stdout = &mystdout0;
	//
	// Initialize the timer counter 1 for 1Hz interrupt
	initialize_timer_counter_1();
	//
	// Initialize timer counter 0 for 100Hz interrupt (used for SD card debug only)
	// initialize_timer_counter_0();  // ONLY ENABLE if using SD card
	//
	// Initialize our main communication to the ground (UART0)
	USART0_init(MYUBRR0);
	//
	// Initialize the GPS module interface handler
	USART1_init(MYUBRR1);
	//
	// Initialize the TWI peripheral
	TWI_Master_Initialise();
	//
	// Initialize the Analog to digital hardware - and test the I/F.
	ADC_init();
	//
 	// initialize the gas sensors
	gas_sensors_init();
	// 
	// Start all interrupts
	sei();
	//
	// Initialize the pressure / temperature /  humidity sensor
	// BME280_init(); 
	//
	// Now that we've tried to initialize everything, we need to report status to the three LEDs sitting on
	// the circuit board...the current placeholder routine does not do this so well.  Need to rethink this.
	// display_status(gas_sensors, gas_sensor_initialization_errors);
	
	// Wait here for the start/standby button to be selected.. (PORTB pin 3).
	// Now call the routines to "kick off" the sensor measurements
	
	// ===================================================
	// The next few lines are for debugging the (working at one time) UART0 routines...remove these lines once it is working again.
	USART0_putstring(String);
	// ====================================================
	////////////////////////////////////////////////////////////////////////////
 	// *************************************************************************
 	// main loop
 	// *************************************************************************
    printf("Starting main loop.");
	while (1) 
    {
		///////////////////////////////////////////////////////////
		// These next few lines are for debug.  Uncomment the USART0_Trans..
		// line to see the behavior below. (Comment out the while loop below it as well)
		// Echo the received character:  with a terminal window open on your PC,
		// you should see everything you type echoed back to the terminal window
		// USART0_TransmitByte(USART0_ReceiveByte());
		//////////////////////////////////////////////////////////
		// reached steady state...do nothing for the moment - wait for interrupts.
		// The variable ItsTime gets set to 1 every second...
		
		if (ItsTime == 1){ //wait for our 1Hz flag
			ItsTime = 0; 
			seconds++;
			printf("\nSeconds = %u \n", seconds);
			// The next several lines sweep through ALL of the attached sensors and sends the data out the serial port.
			// It is VERY simple at present:
			// - do a blocking read of the sensor
			// - Send the data over the serial port
			// - go to the next sensor 
			// ....
			//GPS Message
			//USART0_putstring(&messageWant[0]);
			for (uint8_t i = 0; i<= 50; i++) // The next few lines are commented out and can be deleted once the line above is tested.
			{
				USART0_TransmitByte(messageWant[i]);
			}
			// For this simple approach, we should probably visit the sensors in the following order:
			//   1. Write the most recent GPS position to UART0
			//   2. Kick off the gas sensor reads - and go back for the results in a few milliseconds.  
			//   2. Read the LIDAR distance and send it over UART0
			//   3. Read the BME device and send temp, pressure, humidity over UART0
			//   4. Go get the gas sensor results (when completed) and send over UART0
			//   5. Read the battery status, send it over UART0 and then update the LED status.
			//
			// NOTE:  This is not necessarily the way I think we should do this.
			//  The more I integrated other pieces of code, the more I realized
			// we could better use interrupts.   For now, let's go with this and
			// we'll redesign it when we meet with Pablo.
			//
			// Now we want to read each sensor - and send out the data,
			// each time through this loop.  Comment out the code you
			// don't want to use when debugging the code you are adding... 
			//
			//============================
			// Now test reading the LIDAR interface
			distance = LIDAR_distance();
			printf("\nLIDAR distance = %u", distance);
			//============================
			//
			//============================
			// Now test the gas sensor interface...
			start_gas_sensor_read();
			// Note that this is a blocking read (stops all other activity)
			// At present, the print statements are in that routine....
			//...but the routine needs to be redesigned to operate in the background
 			printf("\nCarbon Monoxide = %u", raw_gas_vector[0]);
 			printf("\nHydrogen = %u", raw_gas_vector[1]);
 			printf("\nAmmonia = %u", raw_gas_vector[2]);
 			printf("\nMethane = %u", raw_gas_vector[3]);
 			printf("\nOzone = %u\n", raw_gas_vector[4]);
			//============================
			//
			// That completes the sensor sweep		
		} else {
		}
    }	
}

/********************************************************************************
*********************************************************************************
						Interrupt Service Routines
*********************************************************************************
********************************************************************************/

