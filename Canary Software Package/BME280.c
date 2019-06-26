/******************************************************************************
 * BME280.c
 *
 * Defines and function prototypes for BME280 device
 * 1/2017 3:23:35 PM
 *  Author: Canary SW Team
********************************************************************************/

/********************************************************************************
						Includes
********************************************************************************/
#include "BME280.h"
#include <avr/io.h>		// Contains the standard IO definitions

/********************************************************************************
						Macros and Defines
********************************************************************************/

/********************************************************************************
						Functions
********************************************************************************/
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
