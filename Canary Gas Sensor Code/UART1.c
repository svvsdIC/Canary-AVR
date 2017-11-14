/*
 * UART1.c
 *
 * Created: 8/3/2017 2:47:28 PM
 *  Author: reitzig_axel
 */ 
/********************************************************************************
 * uart1.c
 *
 * This set of functions implements an interrupt driven UART interface on
 * UART1 in an Atmel atmega324PA.  It has been tested @4800 baud, 8 data bits,
 * 2 stop bits, no parity, and no flow control on a chip with no external
 * crystal.  It should run much faster with a better clock source.
 * Based on Atmel's series "Getting Started with AVR" on Youtube.
 *
 * Created: 3/19/2017 8:39:04 PM
 * Author: Craig R
 *******************************************************************************/ 

/********************************************************************************
						Includes
********************************************************************************/
#include "UART1.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>

/********************************************************************************
						Global Variables
********************************************************************************/
static unsigned char UART1_RxBuf[UART1_RX_BUFFER_SIZE];
static volatile unsigned char UART1_RxHead;
static volatile unsigned char UART1_RxTail;
static unsigned char UART1_TxBuf[UART1_TX_BUFFER_SIZE];
static volatile unsigned char UART1_TxHead;
static volatile unsigned char UART1_TxTail;

/********************************************************************************
*********************************************************************************
						Functions
*********************************************************************************
********************************************************************************/

/* Initialize UART1 */
void USART1_init(uint16_t ubrr_val)
{
	unsigned char x;

	// Set baud rate register
	UBRR0H = (unsigned char)(ubrr_val>>8);
	UBRR0L = (unsigned char) ubrr_val;
	// Enable UART1 receiver, transmitter, and the receive complete interrupt
	UCSR0B = ((1<<RXEN0) | (1<<TXEN0) | (1<<RXCIE0));
	// Set UART mode and frame format: 8 data, 2 stop bits, no parity
	UCSR0C = (3<<UCSZ00) | (1<<USBS0);
	//UCSR0C = 0b00000011;
	

	/* Flush receive buffer */
	x = 0;

	UART1_RxTail = x;
	UART1_RxHead = x;
	UART1_TxTail = x;
	UART1_TxHead = x;
}

unsigned char USART1_ReceiveByte(void)
{
	unsigned char tmptail;
	// Wait for incoming data
	while (UART1_RxHead == UART1_RxTail);
	// Calculate buffer index 
	tmptail = (UART1_RxTail + 1) & UART1_RX_BUFFER_MASK;
	// Store new index 
	UART1_RxTail = tmptail;
	// Return data 
	return UART1_RxBuf[tmptail];
}

void USART1_TransmitByte(unsigned char data)
{
	unsigned char tmphead;
	// Calculate buffer index
	tmphead = (UART1_TxHead + 1) & UART1_TX_BUFFER_MASK;
	// Wait for free space in buffer
	while (tmphead == UART1_TxTail);
	// Store data in buffer 
	UART1_TxBuf[tmphead] = data;
	// Store new index 
	UART1_TxHead = tmphead;
	// Enable UDRE interrupt 
	UCSR0B |= (1<<UDRIE0);
}

//This version is used to send formatted strings...
int USART1_Transmit_IO(char data, FILE *stream)
{
	unsigned char tmphead;
	// Calculate buffer index
	tmphead = (UART1_TxHead + 1) & UART1_TX_BUFFER_MASK;
	// Wait for free space in buffer
	while (tmphead == UART1_TxTail);
	// Store data in buffer
	UART1_TxBuf[tmphead] = data;
	// Store new index
	UART1_TxHead = tmphead;
	// Enable UDRE interrupt
	UCSR0B |= (1<<UDRIE0);
	return(0);
}

// This put string function stops when it reaches the end of a string, which is
// always a null character (0x00) in c.
void USART1_putstring(char* StringPtr) {
	while(*StringPtr != 0x00) {
		USART1_TransmitByte(*StringPtr);
		StringPtr++;
	}
}



/********************************************************************************
*********************************************************************************
						Interrupt Service Routines
*********************************************************************************
********************************************************************************/


ISR(USART1_RX_vect)
{
	unsigned char data;
	unsigned char tmphead;
	
	// Read the received data 
	data = UDR0;
	// Calculate buffer index 
	tmphead = (UART1_RxHead + 1) & UART1_RX_BUFFER_MASK;
	// Store new index 
	UART1_RxHead = tmphead;

	if (tmphead == UART1_RxTail) {
		// ERROR! Receive buffer overflow 
	}
	// Store received data in buffer 
	UART1_RxBuf[tmphead] = data;
}

ISR(USART1_UDRE_vect)
{
	unsigned char tmptail;
	// Check if all data is transmitted 
	if (UART1_TxHead != UART1_TxTail) {
		// Calculate buffer index
		tmptail = ( UART1_TxTail + 1 ) & UART1_TX_BUFFER_MASK;
		// Store new index 
		UART1_TxTail = tmptail;
		// Start transmission 
		UDR0 = UART1_TxBuf[tmptail];
		} else {
		// Disable UDRE interrupt 
		UCSR0B &= ~(1<<UDRIE0);
	}
}