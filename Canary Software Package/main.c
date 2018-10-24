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
volatile uint16_t temp1;

/********************************************************************************
						Global Variables
********************************************************************************/
// These first few variables are here for debug purposes... ---UART STUFF
uint8_t debugdata;
char String[]="Hello World!! The serial port is working!";
extern char messageWant [UART1_RX_BUFFER_SIZE];
volatile uint16_t seconds;

/********************************************************************************
						Functions
********************************************************************************/
// void bme280_structure_and_device_init(void) {
// 	dev.dev_id = BME280_I2C_ADDR_PRIM;
// 	dev.intf = BME280_I2C_INTF;
// 	dev.read = user_i2c_read;
// 	dev.write = user_i2c_write;
// 	// Call the HW initialization routine
// 	rslt = bme280_init(&dev);
// 	
// }


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
	stdout = &mystdout;
	//
	// Initialize the timer counter 1 for 1Hz interrupt
	// This interrupt is only enabled if we don't have the GPS sensor connected.
	initialize_timer_counter_1();
	//
	// Initialize our main communication to the ground (UART0)
	USART0_init(MYUBRR0);
	//
	// Initialize the connection to the GPS sensor (UART1)
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
	// Test our bad interrupt light...
	SetBit(PORTB, PORTB2);
	ClearBit(PORTB, PORTB2);
	// 
	// Start all interrupts
	sei();
	//
	// Initialize and check the BME interface...
	bme280basic_init();
	//
	// Now that we've tried to initialize everything, we need to report status to the three LEDs sitting on
	// the circuit board...the current placeholder routine does not do this so well.  Need to rethink this.
	// display_status(gas_sensors, gas_sensor_initialization_errors);
	//
	// Wait here for the start/standby button to be selected.. (PORTB pin 3).
	// Now call the routines to "kick off" the sensor measurements
	//
	// ===================================================
	// The next few lines are for debugging the (working at one time) UART0 routines...remove these lines once it is working again.
	USART0_putstring(String);
	// ====================================================
	////////////////////////////////////////////////////////////////////////////
 	// *************************************************************************
 	// main loop
 	// *************************************************************************
	while (1) 
    {
		//////////////////////////////////////////////////////////
		// reached steady state...do nothing for the moment - wait for interrupts.
		// The variable ItsTime gets set to 1 every second using the 1Hz interrupt
		// When GPS is connected and enabled, then ItsTime gets set 
		// to 1 whenever we get a $GPGGA message from that sensor.
		///////////////////////////////////////////////////////////
		if (ItsTime == 1){ //wait for our 1Hz flag (from GPS or Interrupt)
			ItsTime = 0; 
			seconds++;
			printf("\nSeconds = %u", seconds);
			while(UART0TransmitCompleteFlag != 0) {}
//			sprintf(temperatureBuf, "%x\n", seconds);
//			USART0_putstring(temperatureBuf);
			// The next several lines sweep through ALL of the attached sensors and sends the data out the serial port.
			// It is VERY simple at present:
			// - do a blocking read of the sensor
			// - Send the data over the serial port
			// - go to the next sensor 
			//**********************************
			// The GPS message triggers the whole collection cycle, so we can send it now...
			printf("\n%s",messageWant);
			while(UART0TransmitCompleteFlag != 0) {}
			//REPLACE THE ABOVE DELAY WITH THE TRANSMIT COMPLETE FLAG WHILE STATEMENT
			//
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
			//============================
			 //Now test reading the LIDAR interface
 			distance = LIDAR_distance();
 			printf("LIDAR distance = %u", distance);
			while(UART0TransmitCompleteFlag != 0) {}
// 			printf("\n LiDAR message = http://canary.chordsrt.com/measurements/url_create?instrument_id=3&dist=%u&key=4e6fba7420ec9e881f510bcddb&", distance); //need key
			// NOTE: Will need to change the write mechanism below to use the stdout (FILE stream). 
// 			for (uint8_t i = 8; i<= 13; i++)//adds in time (***Index may be off by onbe to fix string problem.  Try starting at [7] to <=14)
// 			{
// 				USART0_TransmitByte(messageWant[i]);
// 				time[i] = messageWant[i];
// 				if (i%2 ==1)
// 				{
// 					printf(":");
// 				}
// 			}
// 			printf("Z");
			//============================
			// Now test the gas sensor interface...
			start_gas_sensor_read();
			// Note that this is a blocking read (stops all other activity)
			// At present, the print statements are in that routine....
			//...but the routine needs to be redesigned to operate in the background
//  			printf("\nCarbon Monoxide = %u", raw_gas_vector[0]);
//  			printf("\nHydrogen = %u", raw_gas_vector[1]);
//  			printf("\nAmmonia = %u", raw_gas_vector[2]);
//  			printf("\nMethane = %u", raw_gas_vector[3]);
//  			printf("\nOzone = %u\n", raw_gas_vector[4]);
 			printf("\nCO = %u", raw_gas_vector[0]);
			while(UART0TransmitCompleteFlag != 0) {}
 			printf("\nH = %u", raw_gas_vector[1]);
			while(UART0TransmitCompleteFlag != 0) {}
 			printf("\nNA = %u", raw_gas_vector[2]);
			while(UART0TransmitCompleteFlag != 0) {} 
 			printf("\nCH4 = %u", raw_gas_vector[3]);
			while(UART0TransmitCompleteFlag != 0) {}
 			printf("\nO3 = %u", raw_gas_vector[4]);
			while(UART0TransmitCompleteFlag != 0) {}
			//
			//============================
			// Now read the BME interface...
 			bme280basic_bulk_data_read();
 			tempCelsius = BME280_compensate_T_int32(rawTemp);
// 			sprintf(temperatureBuf, "%lu", tempCelsius);
  			printf("\nCelsius = %lu", tempCelsius);
			while(UART0TransmitCompleteFlag != 0) {}
 			pressure = BME280_compensate_P_int64(rawPress);
 			printf("\nPressure in Pa = %lu", pressure>>8);
			while(UART0TransmitCompleteFlag != 0) {}
 			humidity = bme280_compensate_H_int32(rawHum);
 			printf("\nHumidity%% = %lu.%lu\n", humidity>>10, ((humidity*1000)>>10));
			while(UART0TransmitCompleteFlag != 0) {}
// 			printf("\n BME message = http://canary.chordsrt.com/measurements/url_create?instrument_id=1&temp=%.5s.%.5s&pres=%lu&hum=%lu&key=4e6fba7420ec9e881f510bcddb%.3s:%.4s:%.3s", temp, temp+2, pressure, humidity, time, time+2, time+4); //need key
			//
			//============================
			//re-enable the GPS receiver & interrupt after processing all sensor data
			UCSR1B |= ((1<<RXCIE1)|(1<<RXEN1));
			// Go back to top of loop and wait for the GPS message to be received.
		} else {
		}
    }	
}

/********************************************************************************
*********************************************************************************
						Interrupt Service Routines
*********************************************************************************
********************************************************************************/

ISR(BADISR_vect)
{
	// Code should never reach this point.  This is a bad interrupt trap.
	SetBit(PORTB, PORTB2);
}