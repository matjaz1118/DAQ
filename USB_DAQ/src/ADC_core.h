/*
 * ADC_core.h
 *
 * Created: 9.3.2015 12:16:46
 *  Author: Matjaz
 */ 


#include "stdint-gcc.h"

#ifndef ADC_CORE_H_
#define ADC_CORE_H_



void timer_init (void);
void ADC_init (void);
void aquisition_start (void);
void aquisition_stop (void);


#define ADC_CLK		1000000


#endif /* ADC_CORE_H_ */