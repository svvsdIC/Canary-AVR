/********************************************************************************
 * uart1.h
 *
 * This set of functions implements an interrupt driven UART interface on
 * UART1 in an Atmel atmega324PA and 1284P.  UART1 is set up to talk to the
 * Sparkfun GPS sensor which sends data at 9600 Baud, 8 data bits, 1 stop bit,
 *  no parity, and no flow control.  
 * Based on Atmel's series "Getting Started with AVR" on Youtube.
 *
 * Created: 3/19/2017 8:39:04 PM
 * Author: Craig R
 *******************************************************************************/ 

#ifndef UART1_H_
#define UART1_H_
/********************************************************************************
						Includes
********************************************************************************/
#include <avr/interrupt.h>
#include <stdio.h>

/********************************************************************************
						Macros and Defines
********************************************************************************/
/* UART Buffer Defines - make them long enough for the GPS messages */
#define UART1_RX_BUFFER_SIZE 256     /* 2,4,8,16,32,64,128 or 256 bytes */
#define UART1_TX_BUFFER_SIZE 256
#define UART1_RX_BUFFER_MASK (UART1_RX_BUFFER_SIZE - 1)
#if (UART1_RX_BUFFER_SIZE & UART1_RX_BUFFER_MASK)
#error RX buffer size is not a power of 2
#endif

#define UART1_TX_BUFFER_MASK (UART1_TX_BUFFER_SIZE - 1)
#if (UART1_TX_BUFFER_SIZE & UART1_TX_BUFFER_MASK)
#error TX buffer size is not a power of 2
#endif

// Calculate the baud rate register value using equations from the data sheet
#define BAUD1 9600
#define MYUBRR1 F_CPU/16/BAUD1-1

/********************************************************************************
						Global Variables
********************************************************************************/
char latitude [9];
char longitude [10];
char altitude [8];
uint8_t GPSlock; //flag to indicate if we're getting position data.

/********************************************************************************
						Function Prototypes
********************************************************************************/
void USART1_init(uint16_t ubrr_val);
unsigned char USART1_ReceiveByte(void);
void USART1_TransmitByte(unsigned char data);
void USART1_putstring(char* StringPtr);
int USART1_Transmit_IO(char data, FILE *stream);


#endif /* UART1_H_ */