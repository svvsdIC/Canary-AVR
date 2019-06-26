/******************************************************************************
 * LIDAR.c
 *
 * Defines and function prototypes for using the SParkfun LIDAR device
 * on project Canary.  Copied from Morgan's code.
 *
 * 1/2017 3:23:35 PM
 *  Author: Mostly Morgan Chen
********************************************************************************/

/********************************************************************************
						Includes
********************************************************************************/
#include "LIDAR.h"
#include "TWI_Master.h"

/********************************************************************************
						Macros and Defines
********************************************************************************/

/********************************************************************************
						Functions
********************************************************************************/

uint16_t LIDAR_distance(void) {
	uint16_t Ldistance;
	// From the LIDAR datasheet page 4, simplest way to use the LIDAR:
	// Step 1: Write bias correction use (0x04) to register 0
	messageBuf[0] = LIDAR_WRITE; // LIDAR slave address +W.
	messageBuf[1] = 0x00; // Select register 0x00
	messageBuf[2] = LIDAR_USE_BIAS_CORRECTION; // What we're gonna write
	TWI_Start_Transceiver_With_Data( messageBuf, 3);
	// Let initialization transaction complete...
	while ( TWI_Transceiver_Busy() );
	//
	// Step 2: read register 0x01 and watch the low bit.  
	// From page 6, we have to write the address we want to read first...
	messageBuf[0] = LIDAR_WRITE; // LIDAR slave address +W.
	messageBuf[1] = 0x01; // Select register 0x01
	TWI_Start_Transceiver_With_Data( messageBuf, 2);
	// Let initialization transaction complete...
	while ( TWI_Transceiver_Busy() );	
	// Now verify we're talking to the right device...set up a read of the chip ID:
	messageBuf[1] = 0x01; // Set up to wait for 0x01 LSB to go low...
	while (messageBuf[1] & 0x01)
	{
		messageBuf[0] = LIDAR_READ; // LIDAR slave address +R.
		messageBuf[1] = 0x00; // Clear the field where reg 0x01 will be written
		TWI_Start_Transceiver_With_Data( messageBuf, 2);
		while ( TWI_Transceiver_Busy() );	
		// Now get the data we just read...note that it starts in messageBuf[1], not [0]
		TWI_XFER_STATUS = TWI_Get_Data_From_Transceiver(messageBuf, 2);
	}
	// return;
	// LSB must now be low, so...
	// Step 3: Read two bytes from 0x8f (distance reg high byte, low byte)
	// First, WRITE the read-from address...
	messageBuf[0] = LIDAR_WRITE; // LIDAR slave address +W.
	messageBuf[1] = LIDAR_AUTO_INCREMENT_ON_READ | LIDAR_DISTANCE_HIGH_BYTE;
	TWI_Start_Transceiver_With_Data( messageBuf, 2);
	// Let initialization transaction complete...
	while (TWI_Transceiver_Busy());
	// Now READ from that address
	messageBuf[0] = LIDAR_READ; // LIDAR slave address +R.
	messageBuf[1] = 0x00; //clear the receive area to see if it gets written
	messageBuf[2] = 0x00; //clear the receive area to see if it gets written	
	TWI_Start_Transceiver_With_Data( messageBuf, 3);
	while ( TWI_Transceiver_Busy() );	
	// Now transfer the data into our buffer
	TWI_XFER_STATUS = TWI_Get_Data_From_Transceiver(messageBuf, 3);
	Ldistance = (messageBuf[1]<<8) + messageBuf[2];
	// That's all, folks...
	return(Ldistance);
}
