/********************************************************************************
 * adc.h
 *
 * This set of functions initializes the ADC peripheral and tests it against
 * ground (0 volts) and the bandgap (1.1 volts).    It also provides a routine
 * for pointing the mux at different inputs.
 *
 * Created: 8/14/2017 2:33:20 PM
 * Author: Craig R
 *******************************************************************************/ 
#ifndef ADC_H_
#define ADC_H_

/********************************************************************************
						Includes
********************************************************************************/
#include <avr/io.h>
#include "canary_common.h"

/********************************************************************************
						Function Prototypes
********************************************************************************/
void ADC_SetMux(uint8_t source);
void ADC_init(void);


#endif /* ADC_H_ */