/********************************************************************************
 * timercounters.h
 *
 * This set of functions initializes the timer counters (0 and 1) on the xx4P chip
 * and implements periodic interrupts to drive data collection on board the
 * drone.  A 1Hz interrupt on TC1 is for data collection.  A 100Hz interrupt
 * om TC0 is used for timing purposes on the FATFS when writing to the SD
 * card.  This latter interrupt is not needed in production.
 *
 * Created: 8/14/2017 1:52:26 PM
 * Author: Craig R
 *******************************************************************************/ 
 
#ifndef TIMERCOUNTERS_H_
#define TIMERCOUNTERS_H_
/********************************************************************************
						Includes
********************************************************************************/
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>
//#include "diskio.h" //needed for the 100Hz interrupt - MicroSD file system

/********************************************************************************
						Variable Definitions
********************************************************************************/
//extern uint16_t Timer;
//extern uint8_t ItsTime;

/********************************************************************************
						Macros and Defines
********************************************************************************/
#define production // comment this out for development builds

/********************************************************************************
						Function Prototypes
********************************************************************************/
// The first two lines are the initialization and 1Hz interrupt vector for sensor sampling
void initialize_timer_counter_1(void); //Sets up the 1Hz interrupt for sensor sampling
ISR(TIMER1_COMPA_vect); //1Hz interrupt routine.  

// The 100Hz is for performance measuring of the FATFS -NOT NEEDED FOR PRODUCTION.
void initialize_timer_counter_0(void); //Sets up the 100Hz interrupt for FATFS
//ISR(TIMER0_COMPA_vect); //100Hz interrupt routine.  


#endif /* TIMERCOUNTERS_H_ */