/********************************************************************************
 * UART1.c
 *
 * This set of functions implements an interrupt driven UART interface on
 * UART1 in an Atmel atmega324PA and 1284P.  UART1 is set up to talk to the
 * Sparkfun GPS sensor which sends data at 9600 Baud, 8 data bits, 1 stop bit,
 *  no parity, and no flow control.
 * Based on Atmel's series "Getting Started with AVR" on Youtube.
 * This sensor is read only, so no transmit functions are used and the UDRE
 * interrupt is therefore not enabled.  Reception of the GPGGA message from 
 * the sensor sets a flag indicating we have the message we want.  this triggers
 * the data collection sequence in main().
 *
 * Created: 3/19/2017 8:39:04 PM
 * Author: Canary Team
 *******************************************************************************/ 

/********************************************************************************
						Includes
********************************************************************************/
#include "UART1.h"
#include "canary_common.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>

/********************************************************************************
						Global Variables
********************************************************************************/
//unsigned char UART1_RxBuf[UART1_RX_BUFFER_SIZE];
static volatile unsigned char UART1_RxHead;
static volatile unsigned char UART1_RxTail;
static unsigned char UART1_TxBuf[UART1_TX_BUFFER_SIZE];
static volatile unsigned char UART1_TxHead;
static volatile unsigned char UART1_TxTail;
unsigned char UART1_RxBuf [UART1_RX_BUFFER_SIZE];
char messageWant [UART1_RX_BUFFER_SIZE];
extern volatile uint8_t ItsTime;

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
	UBRR1H = (unsigned char)(ubrr_val>>8);
	UBRR1L = (unsigned char) ubrr_val;
	// For the GPS sensor, enable the UART1 receiver hardware 
	// and the receive complete interrupt only.  The GPS sensor is receive only.
	UCSR1B = ((1<<RXEN1) | (1<<RXCIE1));
	// Set UART mode and frame format: 8 data, 1 stop bits, no parity
	UCSR1C = (3<<UCSZ10);
	//
	// Flush receive buffer 
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
	UCSR1B |= (1<<UDRIE1);
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
	UCSR1B |= (1<<UDRIE1);
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
	unsigned char i;
	// Read the received data 
	data = UDR1;
	// Calculate buffer index 
	tmphead = (UART1_RxHead + 1) & UART1_RX_BUFFER_MASK;
	// Store new index 
	UART1_RxHead = tmphead;

	if (tmphead == UART1_RxTail) {
		// ERROR! Receive buffer overflow 
	}
	// Store received data in buffer 
	UART1_RxBuf[tmphead] = data;
	
	if (data == 10)
	{
		//USART0_TransmitByte('n');
		if (UART1_RxBuf[5] == 'G')
		{
			//USART0_TransmitByte(UART1_RxBuf[5]);
			for (i = 0; i<= tmphead; i++)
			{
				messageWant[i] = UART1_RxBuf[i+1]; //UART1_RxBuf[i];

			}
			messageWant[i-3]=0x00; //This sets the end of the string after the checksum, removing the CR/LF codes from the string.
 			UCSR1B &= !((1<<RXCIE1)|(1<<RXEN1));  //Clear the receive interrupt on USART 1 until we're done reading all other sensors.
 			ItsTime = 1;
 			ToggleBit(PORTB, PORTB1);
		}
		UART1_RxTail = 0;
		UART1_RxHead = 0;
	}
}

// The transmit data register empty interrupt is not needed, as we never send things to the GPS sensor.
//  ISR(USART1_UDRE_vect)
// {
// 	unsigned char tmptail;
// 	// Check if all data is transmitted 
// 	if (UART1_TxHead != UART1_TxTail) {
// 		// Calculate buffer index
// 		tmptail = ( UART1_TxTail + 1 ) & UART1_TX_BUFFER_MASK;
// 		// Store new index 
// 		UART1_TxTail = tmptail;
// 		// Start transmission 
// 		UDR1 = UART1_TxBuf[tmptail];
// 		} else {
// 		// Disable UDRE interrupt 
// 		UCSR1B &= ~(1<<UDRIE1);
// 	}
// }