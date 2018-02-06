/*
 * RGBsensor.c
 *
 * Created: 11/21/2017 2:01:04 PM
 *  Author: craig
 */ 

/********************************************************************************
						Includes
********************************************************************************/
#include "RGBsensor.h"


/********************************************************************************
						Functions
********************************************************************************/
void RGBsensor_init(void) {
	// Set up the light sensor integration time and gain
	// Process for writing data to registers for the TCS34725:
	// 1) Send the sensor I2C address and the write bit set
	// 2) Write the command register with the transaction mode and register address(es)
	// 3) Send the data for the register(s)
	// First turn everything on...
	messageBuf[0] = RGB_SENSOR_WRITE; // The first byte must always have TWI slave address.
	messageBuf[1] = (0x80 | RGB_ENABLE_REGISTER_ADDRESS); // Where we gonna write
	messageBuf[2] = RGB_ENABLE_REGISTER_SETTING; // What we gonna write
	TWI_Start_Transceiver_With_Data( messageBuf, 3);
	// Let initialization transaction complete...
	while ( TWI_Transceiver_Busy() );
	// Now verify we're talking to the right device...set up a read of the chip ID:
	messageBuf[0] = RGB_SENSOR_WRITE; // The first byte must always have TWI slave address.
	messageBuf[1] = (0x80 | TCS34725_ID_ADDRESS); //  Address from which to read on the next transaction
	TWI_Start_Transceiver_With_Data( messageBuf, 2);
	// Let initialization transaction complete...
	while ( TWI_Transceiver_Busy() );// and for the read part...NOTE: FOR THIS DEVICE, YOU HAVE TO SET THE NUMBER OF TRANSMIT
	// BYTES TO (number_of_bytes_to_read (on next cycle) +1).  Kinda weird, but that is how it is set up.
	messageBuf[0] = RGB_SENSOR_READ; // The first byte must always have TWI slave address.
	messageBuf[1] = 0; // this is where the read data should appear...
	TWI_Start_Transceiver_With_Data( messageBuf, 2); //We only want one byte back, so use 2 in the function call.
	// Let initialization transaction complete...
	while ( TWI_Transceiver_Busy() );
	// Now get the data we just read...note this call just copies the data from the TWI routine buffer to our local buffer (messageBuf)...
	TWI_XFER_STATUS = TWI_Get_Data_From_Transceiver(messageBuf, 2);
	// Note that the data we want starts in messageBuf[1], not messageBuf[0]
	if (!(messageBuf[1]==0x44))
	{
		temp=255; // We're talking to the wrong device!!
	}
	else
	{
		temp=0xAD; // We're talking to the right device.
	}
	// Now set the light sensor integration time...
	messageBuf[0] = RGB_SENSOR_WRITE; // The first byte must always have TWI slave address.
	messageBuf[1] = (0x80 | TCS34725_ATIME_ADDRESS); // Where we gonna write
	messageBuf[2] = TCS34725_INTEG_700MS; // What we gonna write
	TWI_Start_Transceiver_With_Data( messageBuf, 3 );
	// Let initialization transaction complete...
	while ( TWI_Transceiver_Busy() );
	//And set the light sensor gain	
	messageBuf[0] = RGB_SENSOR_WRITE; // The first byte must always have TWI slave address.
	messageBuf[1] = (0x80 | TCS34725_CONTROL_ADDRESS); // Where we gonna write
	messageBuf[2] = TCS34725_GAIN_1X; // What we gonna write
	TWI_Start_Transceiver_With_Data( messageBuf, 3 );
	// Let initialization transaction complete...
	while ( TWI_Transceiver_Busy() );
	//
	//Now we are ready to read some sensor values (I think)...
	// To read the light sensor values,
	// 1) Send the sensor I2C address and write bit
	// 2) Set the command register with block (sequential) read and cdatal sensor address (low)
	// 3) send the sensor I2C address with the READ bit
	// 4) Read eight bytes of light sensor readings.
	messageBuf[0] = RGB_SENSOR_WRITE;     // The first byte must always consists of General Call code or the TWI slave address.
	messageBuf[1] = (0x80 | TCS34725_CDATAL_ADDRESS);  // 0xA0 specifies sequential read
	TWI_Start_Transceiver_With_Data( messageBuf, 2 );
	// Let initialization transaction complete...
	while ( TWI_Transceiver_Busy() );
	// The RGB sensor is now ready to read...
}

unsigned char read_RGB_values(void){
	// and for the read part...note I don't have to reset the read address each time - it will always start where the command buffer is pointing (see line 174 above)
	messageBuf[0] = RGB_SENSOR_READ; // The first byte must always have TWI slave address.
	TWI_Start_Transceiver_With_Data( messageBuf, 9); // We want 4 16 bit values, or 8 bytes. Use a 9 in this function call.
	// Wait for the transfer to complete...
	while ( TWI_Transceiver_Busy() );
	// Now get the data we just read...note that it starts in messageBuf[1], not [0]
	TWI_XFER_STATUS = TWI_Get_Data_From_Transceiver(messageBuf, 9);
	//Cool beans.  Now check to see if the data is in the array...
	raw_clear = (messageBuf[2]<<8)+messageBuf[1];
	raw_red = (messageBuf[4]<<8)+messageBuf[3];
	raw_green = (messageBuf[6]<<8)+messageBuf[5];
	raw_blue = (messageBuf[8]<<8)+messageBuf[7];
	return(TWI_XFER_STATUS);
}