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
#define BME280_ADDRESS                (0x76)
#define BME_WRITE_ADDRESS	(BME280_ADDRESS<<1)
#define BME_READ_ADDRESS	((BME280_ADDRESS<<1)|1)

volatile uint16_t temp1;

/********************************************************************************
						Global Variables
********************************************************************************/
// These first few variables are here for debug purposes... ---UART STUFF
uint8_t debugdata;
char String[]="Hello World!! The serial port is working!";
extern char messageWant [UART1_RX_BUFFER_SIZE];
volatile uint16_t seconds;
uint8_t BMEtriggerbyte; //, debugdata;
long rawPress, rawTemp, rawHum, t_fine, tempCelsius, pressure, humidity;
/*long var1, var2, p;*/
// Correction parameters for Temperature
uint16_t dig_T1;
short dig_T2, dig_T3;
char temperatureBuf [20];
char time [6];
// Correction parameters for Pressure
uint16_t dig_P1;
short dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
// correction parameters for humidity
uint16_t dig_H1, dig_H3;
short dig_H2, dig_H4, dig_H5, dig_H6;
// This variable starts the data collection loop in main
extern volatile uint8_t ItsTime;
// To test basic functionality of the BME280...
unsigned char BMEmessageBuf[TWI_BUFFER_SIZE], RawBMEdata[40];
/********************************************************************************
						Functions
********************************************************************************/
// void bme280_structure_and_device_init(void) {
// 	dev.dev_id = BME280_I2C_ADDR_PRIM;
// 	dev.intf = BME280_I2C_INTF;
// 	dev.read = user_i2c_read;
// 	dev.write = user_i2c_write;
// 	dev.delay_ms = user_delay_ms;
// 	// Call the HW initialization routine
// 	rslt = bme280_init(&dev);
// 	
// }

void BME_read_correction_coefficients(void) {
	// This routine (will) reads the corrective coefficients for temperature, pressure, and humidity...
	 static uint8_t i;
// 	// Just reading temperature for now...
// 	BMEmessageBuf[0] = BME_WRITE_ADDRESS; // The first byte must always have TWI slave address.
// 	BMEmessageBuf[1] = 0x88; // The register we want to start reading from
/*	TWI_Start_Transceiver_With_Data( BMEmessageBuf, 2);*/
	// Let initialization transaction complete...
/*	while ( TWI_Transceiver_Busy() );*/
	static volatile uint8_t BMEbusy = 1;
	while (BMEbusy) {
		BMEmessageBuf[0] = BME_WRITE_ADDRESS; // The first byte must always have TWI slave address.
		BMEmessageBuf[1] = 0xF3; // The BME280 status register
		TWI_Start_Transceiver_With_Data( BMEmessageBuf, 2);
		// Let initialization transaction complete...
		while ( TWI_Transceiver_Busy() );
		// Bytes to read = (number_of_bytes_to_read (on next cycle) +1). 
		BMEmessageBuf[0] = BME_READ_ADDRESS; // The first byte must always have TWI slave address.
		TWI_Start_Transceiver_With_Data( BMEmessageBuf, 26); //We want one bytes back, so use 2 in the function call.
		// Let initialization transaction complete...
		while ( TWI_Transceiver_Busy() );
		// Copy the data we want...
		TWI_XFER_STATUS = TWI_Get_Data_From_Transceiver(BMEmessageBuf, 26);
		// CHeck to see if things are still busy
		BMEbusy = ((0b00001000 & BMEmessageBuf[1])>>3);
	}
	// Now for the read part...
	// Bytes to read = (number_of_bytes_to_read (on next cycle) +1).
// 	BMEmessageBuf[0] = BME_READ_ADDRESS; // The first byte must always have TWI slave address.
// 	TWI_Start_Transceiver_With_Data( BMEmessageBuf, 26); //We want six bytes back, so use 7 in the function call.
// 	// Let initialization transaction complete...
// 	while ( TWI_Transceiver_Busy() );
// 	// Now get the data we just read...note this call just copies the data from the TWI routine buffer to our local buffer (BMEmessageBuf)...
// 	TWI_XFER_STATUS = TWI_Get_Data_From_Transceiver(BMEmessageBuf, 26);
	// Note that the data we want starts in BMEmessageBuf[1], not BMEmessageBuf[0]
	// Transfer the data to a variable we can manipulate to get our data out...
		// Device is completed the measurement cycle, now get the temperature data...
	// We want to start reading the data from register 0xF7 and read to register 0xFE - 8 bytes total
	BMEmessageBuf[0] = BME_WRITE_ADDRESS; // The first byte must always have TWI slave address.
	BMEmessageBuf[1] = 0x88; // The register we want to start reading from 
	TWI_Start_Transceiver_With_Data( BMEmessageBuf, 2);
	// Let initialization transaction complete...
	while ( TWI_Transceiver_Busy() );
	// Now for the read part...
	// Bytes to read = (number_of_bytes_to_read (on next cycle) +1). Zero origin.   If we want to read eight bytes, we pass "9".
	BMEmessageBuf[0] = BME_READ_ADDRESS; // The first byte must always have TWI slave address.
	TWI_Start_Transceiver_With_Data( BMEmessageBuf, 27); //We want eight bytes back, so use 9 in the function call.
	// Let initialization transaction complete...
	while ( TWI_Transceiver_Busy() );
	// Now get the data we just read...note this call just copies the data from the TWI routine buffer to our local buffer (BMEmessageBuf)...
	TWI_XFER_STATUS = TWI_Get_Data_From_Transceiver(BMEmessageBuf, 27);
	// Note that the data we want starts in BMEmessageBuf[1], not BMEmessageBuf[0]
	// Transfer the data to a variable we can manipulate to get our data out...
	for (i=0;i<27;i++) 	{
		RawBMEdata[i] = BMEmessageBuf[i+1];
	}
	dig_T1 = /*0x7069;*/ RawBMEdata[0] | (RawBMEdata[1]<<8);
	dig_T2 = /*0x6738;*/ RawBMEdata[2] | (RawBMEdata[3]<<8);
	dig_T3 = /*0x32; */RawBMEdata[4] | (RawBMEdata[5]<<8);
	dig_P1 = RawBMEdata[6] | (RawBMEdata[7]<<8);
	dig_P2 = RawBMEdata[8] | (RawBMEdata[9]<<8);
	dig_P3 = RawBMEdata[10] | (RawBMEdata[11]<<8);
	dig_P4 = RawBMEdata[12] | (RawBMEdata[13]<<8);
	dig_P5 = RawBMEdata[14] | (RawBMEdata[15]<<8);
	dig_P6 = RawBMEdata[16] | (RawBMEdata[17]<<8);
	dig_P7 = RawBMEdata[18] | (RawBMEdata[19]<<8);
	dig_P8 = RawBMEdata[20] | (RawBMEdata[21]<<8);
	dig_P9 = RawBMEdata[22] | (RawBMEdata[23]<<8);
	dig_H1 = RawBMEdata[25];
	//Now grab the rest of the humidity sensor data
	BMEmessageBuf[0] = BME_WRITE_ADDRESS; // The first byte must always have TWI slave address.
	BMEmessageBuf[1] = 0xE1; // The register we want to start reading from
	TWI_Start_Transceiver_With_Data( BMEmessageBuf, 2);
	// Let initialization transaction complete...
	while ( TWI_Transceiver_Busy() );
	// Now for the read part...
	// Bytes to read = (number_of_bytes_to_read (on next cycle) +1). Zero origin.   If we want to read eight bytes, we pass "9".
	BMEmessageBuf[0] = BME_READ_ADDRESS; // The first byte must always have TWI slave address.
	TWI_Start_Transceiver_With_Data( BMEmessageBuf, 8); //We want eight bytes back, so use 9 in the function call.
	// Let initialization transaction complete...
	while ( TWI_Transceiver_Busy() );
	// Now get the data we just read...note this call just copies the data from the TWI routine buffer to our local buffer (BMEmessageBuf)...
	TWI_XFER_STATUS = TWI_Get_Data_From_Transceiver(BMEmessageBuf, 8);
	// Note that the data we want starts in BMEmessageBuf[1], not BMEmessageBuf[0]
	// Transfer the data to a variable we can manipulate to get our data out...
	for (i=0;i<7;i++) 	{
		RawBMEdata[i] = BMEmessageBuf[i+1];
	}
	dig_H2 = RawBMEdata[1] | (RawBMEdata[2]<<8); 
	dig_H3 = RawBMEdata[3]; 
	dig_H4 = (RawBMEdata[4]<<4) | (RawBMEdata[5]>>5);
	dig_H5 = (RawBMEdata[5]>>5) | (RawBMEdata[6]<<4);
	dig_H6 = RawBMEdata[7];
}


void bme280basic_init(void) {
	// This routine resets the BME280 then checks the chip ID to see if it is the right device.
	//
	// Process for writing data to registers for the BME280:
	// 1) First byte is the sensor I2C address (x2) and the read/write bit set to 0
	// 2) Second byte is the register to which to write the data value
	// 3) Third byte is the data value to be written tot he register identified in 2 above
	// Repeat 2 and three for each byte to be written if there is to be more than one register written at a time.
	// Send the array of bytes
	//
	// The process for reading data from the BME280:
	// 1) First byte is the sensor I2C address (x2) and the read/write bit set to 0
	// 2) Second byte is the register from which to start reading data values (auto increment allows you to read more than 1 byte).
	// 3) Send the two bytes and restart
	// 4) First byte is the sensor I2C address (x2) and the read/write bit set to 1
	// 5) Start the transaction and send a NACK after you've received the last byte you want.
	//
	// First, reset the device per section 5.4.2 of the data sheet
	BMEmessageBuf[0] = BME_WRITE_ADDRESS; // The first byte must always have TWI slave address.
	BMEmessageBuf[1] = 0xE0; // The register we want to write to
	BMEmessageBuf[2] = 0xB6; // This value forces a reset to the device
	TWI_Start_Transceiver_With_Data( BMEmessageBuf, 3);
	// Wait for the transaction to complete...
	while ( TWI_Transceiver_Busy() );
	//
	// Now read the chip ID from register 0x0D
	BMEmessageBuf[0] = BME_WRITE_ADDRESS; // The first byte must always have TWI slave address.
	BMEmessageBuf[1] = 0xD0; // The register we want to read from
	TWI_Start_Transceiver_With_Data( BMEmessageBuf, 2);
	// Let initialization transaction complete...
	while ( TWI_Transceiver_Busy() );
	// Now for the read part...
	// Bytes to read = (number_of_bytes_to_read (on next cycle) +1). Zero origin.   If we want to read one byte, we pass "2".
	BMEmessageBuf[0] = BME_READ_ADDRESS; // The first byte must always have TWI slave address.
	TWI_Start_Transceiver_With_Data( BMEmessageBuf, 2); //We only want one byte back, so use 2 in the function call.
	// Let initialization transaction complete...
	while ( TWI_Transceiver_Busy() );
	// Now get the data we just read...note this call just copies the data from the TWI routine buffer to our local buffer (BMEmessageBuf)...
	TWI_XFER_STATUS = TWI_Get_Data_From_Transceiver(BMEmessageBuf, 2);
	// Note that the data we want starts in BMEmessageBuf[1], not BMEmessageBuf[0]
	if (BMEmessageBuf[1]==0x60) { 
		// We're talking to the right device.  Set up the control registers...
		//
		// We want Humidity oversampling set to x1 (ctrl_hum (0xF2) [2:0] = 0b001)
		BMEmessageBuf[0] = BME_WRITE_ADDRESS; // The first byte must always have TWI slave address.
		BMEmessageBuf[1] = 0xF2; // The register we want to write to
		BMEmessageBuf[2] = 0x01; // Set humidity oversampling to x1
		TWI_Start_Transceiver_With_Data( BMEmessageBuf, 3);
		// Wait for the transaction to complete...
		while ( TWI_Transceiver_Busy() );
		//		
		//Set Tstandby to its smallest value (0)
		//  Per table 12 and 28 we want the filter coefficient at 16 (config (0xF5) [4:2] = 0b100)
		BMEmessageBuf[0] = BME_WRITE_ADDRESS; // The first byte must always have TWI slave address.
		BMEmessageBuf[1] = 0xF5; // The register we want to write to
		BMEmessageBuf[2] = (0b100<<2); // Set temp, pressure, and mode
		TWI_Start_Transceiver_With_Data( BMEmessageBuf, 3);
		// Wait for the transaction to complete...
		while ( TWI_Transceiver_Busy() );
		//
		BME_read_correction_coefficients();
		// We want Temperature oversampling set to x1 (ctrl_meas (0xF4) [7:5] = 0b001)
		// We want Pressure oversampling set to x8 (ctrl_meas (0xF4) [4:2] = 0b100)
		// Put the device into Forced mode (we want to tell the device to "go measure") (ctrl_meas (0xF4) [1:0] = 0b01)
		BMEtriggerbyte = (0b01<<5) | (0b100<<2) | (0b01<<0);
		// NOTE: THIS BYTE MUST BE RESENT EACH TIME TO GET A NEW MEASUREMENT
		BMEmessageBuf[0] = BME_WRITE_ADDRESS; // The first byte must always have TWI slave address.
		BMEmessageBuf[1] = 0xF4; // The register we want to write to
		BMEmessageBuf[2] = BMEtriggerbyte; // Set temp, pressure, and mode
		TWI_Start_Transceiver_With_Data( BMEmessageBuf, 3);
		// Wait for the transaction to complete...
		while ( TWI_Transceiver_Busy() );
		//  All done - and first measurement cycle has  been kicked off!
	}
}

void bme280basic_bulk_data_read(void) {
	// Routine to do a block read of the temperature, pressure, and humidity registers on the BME280
	// See the bme280basic_init routine for the read and write protocols for using this sensor...
	uint8_t i;
	static volatile uint8_t BMEbusy;
	//
	// Ensure the device has completed the read cycle...
	BMEbusy = 1;
	while (BMEbusy) {
		BMEmessageBuf[0] = BME_WRITE_ADDRESS; // The first byte must always have TWI slave address.
		BMEmessageBuf[1] = 0xF3; // The BME280 status register
		TWI_Start_Transceiver_With_Data( BMEmessageBuf, 2);
		// Let initialization transaction complete...
		while ( TWI_Transceiver_Busy() );
		// Bytes to read = (number_of_bytes_to_read (on next cycle) +1). 
		BMEmessageBuf[0] = BME_READ_ADDRESS; // The first byte must always have TWI slave address.
		TWI_Start_Transceiver_With_Data( BMEmessageBuf, 2); //We want one bytes back, so use 2 in the function call.
		// Let initialization transaction complete...
		while ( TWI_Transceiver_Busy() );
		// Copy the data we want...
		TWI_XFER_STATUS = TWI_Get_Data_From_Transceiver(BMEmessageBuf, 2);
		// CHeck to see if things are still busy
		BMEbusy = ((0b00001000 & BMEmessageBuf[1])>>3);
	}
	//
	// Device is completed the measurement cycle, now get the temperature data...
	// We want to start reading the data from register 0xF7 and read to register 0xFE - 8 bytes total
	BMEmessageBuf[0] = BME_WRITE_ADDRESS; // The first byte must always have TWI slave address.
	BMEmessageBuf[1] = 0xF7; // The register we want to start reading from 
	TWI_Start_Transceiver_With_Data( BMEmessageBuf, 2);
	// Let initialization transaction complete...
	while ( TWI_Transceiver_Busy() );
	// Now for the read part...
	// Bytes to read = (number_of_bytes_to_read (on next cycle) +1). Zero origin.   If we want to read eight bytes, we pass "9".
	BMEmessageBuf[0] = BME_READ_ADDRESS; // The first byte must always have TWI slave address.
	TWI_Start_Transceiver_With_Data( BMEmessageBuf, 9); //We want eight bytes back, so use 9 in the function call.
	// Let initialization transaction complete...
	while ( TWI_Transceiver_Busy() );
	// Now get the data we just read...note this call just copies the data from the TWI routine buffer to our local buffer (BMEmessageBuf)...
	TWI_XFER_STATUS = TWI_Get_Data_From_Transceiver(BMEmessageBuf, 9);
	// Note that the data we want starts in BMEmessageBuf[1], not BMEmessageBuf[0]
	// Transfer the data to a variable we can manipulate to get our data out...
	for (i=0;i<8;i++) 	{
		RawBMEdata[i] = BMEmessageBuf[i+1];
	}
	rawPress = ((uint32_t)RawBMEdata[0]<<12) | ((uint32_t)RawBMEdata[1]<<4) | ((uint32_t)RawBMEdata[2]>>4);
	rawTemp = ((uint32_t)RawBMEdata[3]<<12) | ((uint32_t)RawBMEdata[4]<<4) | ((uint32_t)RawBMEdata[5]>>4);
	rawHum = ((uint32_t)RawBMEdata[6]<<8) | (uint32_t)RawBMEdata[7];
	//
	BMEmessageBuf[0] = 0xFF; //Here for a breakpoint when debugging.  Remove.
	// Now kick off the measurement cycle for the next read...
	// We want Temperature oversampling set to x1 (ctrl_meas (0xF4) [7:5] = 0b001)
	// We want Pressure oversampling set to x8 (ctrl_meas (0xF4) [4:2] = 0b100)
	// Put the device into Forced mode (we want to tell the device to "go measure") (ctrl_meas (0xF4) [1:0] = 0b01)
	// BMEtriggerbyte ^= 0x03; // toggle the forced mode (not sure this is required)
	// NOTE: THIS BYTE MUST BE RESENT EACH TIME TO GET A NEW MEASUREMENT
	BMEmessageBuf[0] = BME_WRITE_ADDRESS; // The first byte must always have TWI slave address.
	BMEmessageBuf[1] = 0xF4; // The register we want to write to
	BMEmessageBuf[2] = (0b01<<5) | (0b100<<2) | (0b01<<0); // Set temp, pressure, and mode
	TWI_Start_Transceiver_With_Data( BMEmessageBuf, 3);
	// Wait for the transaction to complete...
	while ( TWI_Transceiver_Busy() );
	//  All done - and the next measurement cycle has  been kicked off!
}

// Returns temperature in DegC, resolution is 0.01 DegC. Output value of ?5123? equals 51.23 Deg C.
// t_fine carries fine temperature as global value
long BME280_compensate_T_int32(long adc_T) {
	long var1, var2, T;
	var1  = ((((adc_T>>3) - (dig_T1<<1))) * (dig_T2)) >> 11;
	var2  = (((((adc_T>>4) - (dig_T1)) * ((adc_T>>4) - (dig_T1))) >> 12) * (dig_T3)) >> 14;
	t_fine = var1 + var2;
	T  = (t_fine * 5 + 128) >> 8;
	return T;
}


//Return pressure in Pa as unsigned 32 bit int in Q24.8 format(24 int bits, 8 fractional)
//Output value of "24674867" represents 24674867/256 = 96386.2 Pa = 963.862 hPa
long BME280_compensate_P_int64(long adc_P)
{
	long long var1, var2, p;
	var1 = ((long)t_fine)-128000;
	var2 = var1*var1*(long)dig_P6;
	var2 = var2 + ((var1*(long)dig_P5)<<17);
	var2 = var2 + (((long long)dig_P4)<<35);
	var1 = ((var1*var1*(long)dig_P3)>>8)+((var1*(long)dig_P2)<<12);
	var1 = (((((long long)1)<<47)+var1))*((long long)dig_P1)>>33;
	if (var1 == 0)
	{
		return 0;
	}
	p = 1048576 - adc_P;
	p = (((p<<31)-var2)*3125)/var1;
	var1 = (((long)dig_P9)*(p>>13)*(p>>13))>>25;
	var2 = (((long)dig_P8)*p)>>19;
	p = ((p+var1+var2)>>8)+(((long)dig_P7)<<4);
	return(long)p;
}

// Returns humidity in %RH as unsigned 32 bit integer in Q22.10 format (22 integer and 10 fractional bits).
// Output value of ?47445? represents 47445/1024 = 46.333 %RH
long bme280_compensate_H_int32(long adc_H)
{
	long long v_x1_u32r;
	v_x1_u32r = (t_fine - ((long long)76800));
	v_x1_u32r = (((((adc_H << 14) - (((long long)dig_H4) << 20) - (((long long)dig_H5) * v_x1_u32r)) +
	((long long)16384)) >> 15) * (((((((v_x1_u32r * ((long long)dig_H6)) >> 10) * (((v_x1_u32r *
	((long long)dig_H3)) >> 11) + ((long long)32768))) >> 10) + ((long long)2097152)) *
	((long long)dig_H2) + 8192) >> 14));
	v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((long long)dig_H1)) >> 4));
	v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
	v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
	return (long)(v_x1_u32r>>12);
}
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
	_delay_ms(500);
	ClearBit(PORTB, PORTB2);
	_delay_ms(500);
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
		if (ItsTime == 1){ //wait for our 1Hz flag
			ItsTime = 0; 
			seconds++;
			printf("\nSeconds = %u", seconds);
//			sprintf(temperatureBuf, "%x\n", seconds);
//			USART0_putstring(temperatureBuf);
			// The next several lines sweep through ALL of the attached sensors and sends the data out the serial port.
			// It is VERY simple at present:
			// - do a blocking read of the sensor
			// - Send the data over the serial port
			// - go to the next sensor 
			//**********************************
			// The GPS message triggers the whole collection cycle, so we can send it now...
			printf("%s",messageWant);
			_delay_ms(200);
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
// 			printf("\n LiDAR message = http://canary.chordsrt.com/measurements/url_create?instrument_id=3&dist=%u&key=4e6fba7420ec9e881f510bcddb&", distance); //need key
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
 			printf("\nH = %u", raw_gas_vector[1]);
 			printf("\nNA = %u", raw_gas_vector[2]);
 			printf("\nCH4 = %u", raw_gas_vector[3]);
 			printf("\nO3 = %u\n", raw_gas_vector[4]);
			//
			//============================
			// Now read the BME interface...
// 			bme280basic_bulk_data_read();
// 			tempCelsius = BME280_compensate_T_int32(rawTemp);
// 			sprintf(temperatureBuf, "%lu", tempCelsius);
//  			printf("\nCelsius = %lu\n", tempCelsius);
// 			pressure = BME280_compensate_P_int64(rawPress);
// 			printf("\nPressure in Pa = %lu\n", pressure>>8);
// 			humidity = bme280_compensate_H_int32(rawHum);
// 			printf("\n Humidity in percent relative humidity= %lu.%lu\n", humidity>>10, ((humidity*1000)>>10));
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