/******************************************************************************
 * LIDAR.h
 *
 * Defines and function prototypes for using the SParkfun LIDAR device
 * on project Canary.  Copied from Morgan's code.
 * 
 * Created: 11/21/2017 3:12:47 PM
 *  Author:  Mostly Morgan
********************************************************************************/ 

#ifndef LIDAR_H_
#define LIDAR_H_

/********************************************************************************
						Includes
********************************************************************************/
#include <avr/io.h>
#include "canary_common.h"
#include "TWI_Master.h"

/********************************************************************************
						Macros and Defines
********************************************************************************/
#define LIDARaddress 0x62
#define LIDAR_WRITE (LIDARaddress << 1)
#define LIDAR_READ (LIDARaddress << 1) | 1
#define LIDAR_RESET 0x00
#define LIDAR_NO_BIAS_CORRECTION 0x03
#define LIDAR_USE_BIAS_CORRECTION 0x04
#define LIDAR_DISTANCE_HIGH_BYTE 0x0F
#define LIDAR_AUTO_INCREMENT_ON_READ 0x80

/********************************************************************************
						Global Variables
********************************************************************************/
uint16_t distance;

/********************************************************************************
						Function Prototypes
********************************************************************************/
uint16_t LIDAR_distance(void);

#endif /* LIDAR_H_ */