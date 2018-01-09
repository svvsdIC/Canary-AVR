/********************************************************************************
 * UART0.h
 *
 * This set of functions implements an interrupt driven UART interface on
 * UART0 in an Atmel atmega324PA.  It has been tested @4800 baud, 8 data bits,
 * 2 stop bits, no parity, and no flow control on a chip with no external
 * crystal.  It should run much faster with a better clock source.
 * Based on Atmel's series "Getting Started with AVR" on Youtube.
 *
 * Created: 3/19/2017 8:39:04 PM
 * Author: Craig R
 *******************************************************************************/ 

#ifndef UART0_H_
#define UART0_H_
/********************************************************************************
						Includes
********************************************************************************/
#include <avr/interrupt.h>
#include <stdio.h>

/********************************************************************************
						Macros and Defines
********************************************************************************/
/* UART Buffer Defines - make them long enough for the GPS messages */
#define UART0_RX_BUFFER_SIZE 256     /* 2,4,8,16,32,64,128 or 256 bytes */
#define UART0_TX_BUFFER_SIZE 256
#define UART0_RX_BUFFER_MASK (UART0_RX_BUFFER_SIZE - 1)
#if (UART0_RX_BUFFER_SIZE & UART0_RX_BUFFER_MASK)
#error RX buffer size is not a power of 2
#endif

#define UART0_TX_BUFFER_MASK (UART0_TX_BUFFER_SIZE - 1)
#if (UART0_TX_BUFFER_SIZE & UART0_TX_BUFFER_MASK)
#error TX buffer size is not a power of 2
#endif

// Calculate the baud rate register value using equations from the data sheet
#define BAUD0 4800
#define MYUBRR0 F_CPU/16/BAUD0-1


/********************************************************************************
						Function Prototypes
********************************************************************************/
void USART0_init(uint16_t ubrr_val);
unsigned char USART0_ReceiveByte(void);
void USART0_TransmitByte(unsigned char data);
void USART0_putstring(char* StringPtr);
int USART0_Transmit_IO(char data, FILE *stream);

#endif /* UART0_H_ */