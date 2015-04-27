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



#define ADC_SAMPLE_RATE_10HZ			50000
#define ADC_SAMPLE_RATE_25HZ			20000
#define ADC_SAMPLE_RATE_50HZ			10000
#define ADC_SAMPLE_RATE_100HZ			5000
#define ADC_SAMPLE_RATE_250HZ			2000
#define ADC_SAMPLE_RATE_500HZ			1000
#define ADC_SAMPLE_RATE_1000HZ			500
#define ADC_SAMPLE_RATE_2500HZ			200
#define ADC_SAMPLE_RATE_5000HZ			100
#define ADC_SAMPLE_RATE_10000HZ			50
#define ADC_SAMPLE_RATE_20000HZ			25
#define ADC_SAMPLE_RATE_25000HZ			20
#define ADC_SAMPLE_RATE_50000HZ			10
#define ADC_SAMPLE_RATE_100000HZ		5
#define ADC_SAMPLE_RATE_250000HZ		2


#define ADC_CLK		1000000


#endif /* ADC_CORE_H_ */