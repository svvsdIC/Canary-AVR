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
uint8_t printType = 1;
uint8_t GPSlock = 0; //flag to indicate if we're getting position data.


//look up tables - taken from Aileen sensor order is: CO, H , NH3, CH4, O3

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
	// Turn on the RED LED until we have finished initialization
	SetBit(PORTB, PORTB0);
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
	// Start all interrupts
	sei();
	//
	// Initialize and check the BME interface...
//!	bme280basic_init();
	//
	// Now that we've tried to initialize everything, we need to report status to the three LEDs sitting on
	// the circuit board...the current placeholder routine does not do this so well.  Need to rethink this.
	// display_status(gas_sensors, gas_sensor_initialization_errors);
	//
	// Clear the Red LED to indicate all things are initialized.
	ClearBit(PORTB, PORTB0);
	//
	// ***modify UART1 interrupt code to set the GPSlock flag is distance data is detected ***
	// *** (and turn on the BLUE LED so the operator knows when it is locked).***
	//
	// Wait here for the start/standby button to be selected.. (PORTB pin 3).
//	while(BitIsSet(PORTB,PORTB3)) {}//makes program wait until everything is ready(button is pushed) //not working we don't know why
	//
	//Proceed to main loop with warning to ground system re: GPSlock
	if (GPSlock==1) 
		{
			printf("\n%s", "Proceeding with GPS lock");
		}
		else
		{
			printf("\n%s", "Proceeding without GPS lock");
		} 
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
//  			printf("\nSeconds = %u", seconds);
// 			// Wait until the transmission is complete
// 			while(UART0TransmitInProgress) {}
			// The next several lines sweep through ALL of the attached sensors and sends the data out the serial port.
			// It is VERY simple at present:
			// - Read each sensor
			// - Send the data over the serial port
			// - Wait for the transmission to complete, then
			// - go to the next sensor 
			//**********************************
			// The GPS message triggers the whole collection cycle, so we can send it now...
// 			printf("\n%s",messageWant);
			while(UART0TransmitInProgress) {}
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
//  			printf("\nLIDAR distance = %u", distance);
// 			while(UART0TransmitInProgress) {}
// 			printf("\n LiDAR message = http://canary.chordsrt.com/measurements/url_create?instrument_id=3&dist=%u&key=4e6fba7420ec9e881f510bcddb&", distance); //need key
			// NOTE: Will need to change the write mechanism below to use the stdout (FILE stream). 
// 			for (uint8_t i = 8; i<= 13; i++)//adds in time (***Index may be off by one to fix string problem.  Try starting at [7] to <=14)
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
//  			printf("\nCO = %u", raw_gas_vector[0]);
// 			while(UART0TransmitInProgress) {}
//  			printf("\nH = %u", raw_gas_vector[1]);
// 			while(UART0TransmitInProgress) {}
//  			printf("\nNA = %u", raw_gas_vector[2]);
// 			while(UART0TransmitInProgress) {} 
//  			printf("\nCH4 = %u", raw_gas_vector[3]);
// 			while(UART0TransmitInProgress) {}
//  			printf("\nO3 = %u", raw_gas_vector[4]);
// 			while(UART0TransmitInProgress) {}
			//
			//============================
			// Now read the BME interface...
//! 		bme280basic_bulk_data_read();
 			// Calculate the temperature and print it
//!			tempCelsius = BME280_compensate_T_int32(rawTemp);
// 			sprintf(temperatureBuf, "%lu", tempCelsius);
//  			printf("\nCelsius = %lu", tempCelsius);
//			while(UART0TransmitInProgress) {}
 			// Calculate the pressure and print it
//!			pressure = BME280_compensate_P_int64(rawPress);
// 			printf("\nPressure in Pa = %lu", pressure>>8);
//			while(UART0TransmitInProgress) {}
 			// Calculate the humidity and print it
//!			 humidity = bme280_compensate_H_int32(rawHum);
// 			printf("\nHumidity%% = %lu.%lu\n", humidity>>10, ((humidity*1000)>>10));
//			while(UART0TransmitInProgress) {}
// 			printf("\n BME message = http://canary.chordsrt.com/measurements/url_create?instrument_id=1&temp=%.5s.%.5s&pres=%lu&hum=%lu&key=4e6fba7420ec9e881f510bcddb%.3s:%.4s:%.3s", temp, temp+2, pressure, humidity, time, time+2, time+4); //need key
			//
			//============================
			
			for(int i = 0; i < 5; i++)
			{
				convert_to_ppm(i); //only works for co right now
			}
// 			printf("ppm value CO: %d \n", ppmValue[0]);
// 			printf("ppm value H2: %d \n", ppmValue[1]);
// 			printf("ppm value NH3: %d \n", ppmValue[2]);
// 			printf("ppm value CH4: %d \n", ppmValue[3]);
// 			printf("ppm value O3: %d \n", ppmValue[4]);
// 			while(UART0TransmitInProgress) {}

			//print statements
			// ----------- debug -------------- //
			if (printType == 0)
			{
				//# seconds
				printf("\nSeconds = %u", seconds);
				// LiDAR
				while(UART0TransmitInProgress) {}
 				printf("\nLIDAR distance = %u", distance);
				// Gas sensors
 				while(UART0TransmitInProgress) {}
				printf("\nCO = %u", raw_gas_vector[0]);
				while(UART0TransmitInProgress) {}
				printf("\nH = %u", raw_gas_vector[1]);
				while(UART0TransmitInProgress) {}
				printf("\nNA = %u", raw_gas_vector[2]);
				while(UART0TransmitInProgress) {}
				printf("\nCH4 = %u", raw_gas_vector[3]);
				while(UART0TransmitInProgress) {}
				printf("\nO3 = %u", raw_gas_vector[4]);
				while(UART0TransmitInProgress) {}
				//BME 280
				printf("\nCelsius = %d.%.2d", (int) (tempCelsius/100), (int) (tempCelsius%100));
				while(UART0TransmitInProgress) {}
				printf("\nPressure in Pa = %lu", pressure>>8);
				while(UART0TransmitInProgress) {}
				printf("\nHumidity%% = %lu.%lu\n", humidity>>10, ((humidity*1000)>>10));
				while(UART0TransmitInProgress) {}
				//ppm values of gas sensors
				printf("ppm value CO: %d \n", ppmValue[0]);
				printf("ppm value H2: %d \n", ppmValue[1]);
				printf("ppm value NH3: %d \n", ppmValue[2]);
				printf("ppm value CH4: %d \n", ppmValue[3]);
				printf("ppm value O3: %d \n", ppmValue[4]);
				while(UART0TransmitInProgress) {}
				//GPS data
				printf("\n%s",messageWant);
			}
			// ---------- final --------------- //
			/*			instrument_id,data,moreData,LastData|instrument2_id...
			Instrument ids:
			1 = BME280
			Order: temp, pressure, humidity
			2 = Gas sensors
			Order: CO, NH3, O3, CH4, H2
			3 = LiDAR
			Order: Distance
			4 = GPS
			Order: lat, long, altitude */
			if (printType == 1)
			{
				printf("1,%d.%.2d,%d,%d|", (int) (tempCelsius/100), (int) (tempCelsius%100), pressure>>8, humidity>>10);
				printf("2,%d,%d,%d,%d,%d,%d|", raw_gas_vector[0], raw_gas_vector[1], raw_gas_vector[2], raw_gas_vector[3], raw_gas_vector[4], raw_gas_vector[5]);
				printf("3,%d|",distance);
				printf("4,%d,%d,%d|",latitude,longitude,altitude);
			}
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