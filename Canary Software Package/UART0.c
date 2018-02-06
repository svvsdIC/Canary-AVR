/********************************************************************************
 * UART0.c
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

/********************************************************************************
						Includes
********************************************************************************/
#include "UART0.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>

/********************************************************************************
						Global Variables
********************************************************************************/
static unsigned char UART0_RxBuf[UART0_RX_BUFFER_SIZE];
static volatile unsigned char UART0_RxHead;
static volatile unsigned char UART0_RxTail;
static unsigned char UART0_TxBuf[UART0_TX_BUFFER_SIZE];
static volatile unsigned char UART0_TxHead;
static volatile unsigned char UART0_TxTail;

/********************************************************************************
*********************************************************************************
						Functions
*********************************************************************************
********************************************************************************/

/* Initialize UART0 */
void USART0_init(uint16_t ubrr_val)
{
	unsigned char x;

	// Set baud rate register
	UBRR0H = (unsigned char)(ubrr_val>>8);
	UBRR0L = (unsigned char) ubrr_val;
	// Enable UART0 receiver, transmitter, and the receive complete interrupt
	UCSR0B = ((1<<RXEN0) | (1<<TXEN0) | (1<<RXCIE0));
	// Set UART mode and frame format: 8 data, 2 stop bits, no parity
	UCSR0C = (3<<UCSZ00) | (1<<USBS0);	

	/* Flush receive buffer */
	x = 0;
	UART0_RxTail = x;
	UART0_RxHead = x;
	UART0_TxTail = x;
	UART0_TxHead = x;
}

unsigned char USART0_ReceiveByte(void)
{
	unsigned char tmptail;
	// Wait for incoming data
	while (UART0_RxHead == UART0_RxTail);
	// Calculate buffer index 
	tmptail = (UART0_RxTail + 1) & UART0_RX_BUFFER_MASK;
	// Store new index 
	UART0_RxTail = tmptail;
	// Return data 
	return UART0_RxBuf[tmptail];
}

void USART0_TransmitByte(unsigned char data)
{
	unsigned char tmphead;
	// Calculate buffer index
	tmphead = (UART0_TxHead + 1) & UART0_TX_BUFFER_MASK;
	// Wait for free space in buffer
	while (tmphead == UART0_TxTail);
	// Store data in buffer 
	UART0_TxBuf[tmphead] = data;
	// Store new index 
	UART0_TxHead = tmphead;
	// Enable UDRE interrupt 
	UCSR0B |= (1<<UDRIE0);
}

//This version is used to send formatted strings...
int USART0_Transmit_IO(char data, FILE *stream)
{
	unsigned char tmphead;
	// Calculate buffer index
	tmphead = (UART0_TxHead + 1) & UART0_TX_BUFFER_MASK;
	// Wait for free space in buffer
	while (tmphead == UART0_TxTail);
	// Store data in buffer
	UART0_TxBuf[tmphead] = data;
	// Store new index
	UART0_TxHead = tmphead;
	// Enable UDRE interrupt
	UCSR0B |= (1<<UDRIE0);
	return(0);
}

// This put string function stops when it reaches the end of a string, which is
// always a null character (0x00) in c.
void USART0_putstring(char* StringPtr) {
	while(*StringPtr != 0x00) {
		USART0_TransmitByte(*StringPtr);
		StringPtr++;
	}
}



/********************************************************************************
*********************************************************************************
						Interrupt Service Routines
*********************************************************************************
********************************************************************************/


/*ISR(USART0_RX_vect)
{
	unsigned char data;
	unsigned char tmphead;
	
	// Read the received data 
	data = UDR0;
	// Calculate buffer index 
	tmphead = (UART0_RxHead + 1) & UART0_RX_BUFFER_MASK;
	// Store new index 
	UART0_RxHead = tmphead;

	if (tmphead == UART0_RxTail) {
		// ERROR! Receive buffer overflow 
	}
	// Store received data in buffer 
	UART0_RxBuf[tmphead] = data;
}*/

ISR(USART0_UDRE_vect)
{
	unsigned char tmptail;
	// Check if all data is transmitted 
	if (UART0_TxHead != UART0_TxTail) {
		// Calculate buffer index
		tmptail = ( UART0_TxTail + 1 ) & UART0_TX_BUFFER_MASK;
		// Store new index 
		UART0_TxTail = tmptail;
		// Start transmission 
		UDR0 = UART0_TxBuf[tmptail];
		} else {
		// Disable UDRE interrupt 
		UCSR0B &= ~(1<<UDRIE0);
	}
}