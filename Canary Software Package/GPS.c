/*
 * GPS.c
 *
 * Created: 1/2/2018 3:47:11 PM
 *  Author: bird.davita04
 */ 
/*
*Psuedocode
*if(BYTE == 10)
*{
*	if(UART1_RXBuff[4]== "L")
*	{Copy Bytes;
*	}
*Zero.Buffer}
*/
#include "canary_common.h"
unsigned char GPSRun(void)
{
	extern UART1_RxBuf[UART1_RX_BUFFER_SIZE];
	if (UART1_RxBuf[USART1_ReceiveByte()] == 10)
	{
		if (UART1_RxBuf[4] == 'L')
		{
			while(i=4; i<= 59; i++)//print 53 bytes of RxBuf 
			{
				printf("%d ", UART1_RxBuf[i]);
			}
		}
		UART1_RxBuf = 0;
	}
}