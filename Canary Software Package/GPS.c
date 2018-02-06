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
//#include "canary_common.h"
// #ifndef INCLUDED_UART1_C;
 #include "GPS.h"
// #define INCLUDED_UART1_C;
// #endif

//extern char UART1_RxBuf[0];
void GPSRun(void)
{
	if (USART1_ReceiveByte() == 10)
	{
		if (UART1_RxBuf[4] == 'L')
		{
			for (uint8_t i = 4; i<= 59; i++)//print 53 bytes of RxBuf 
			{
				printf("%d ", UART1_RxBuf[i]);
			}
		}
	}
}