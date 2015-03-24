/*
 * DAC.h
 *
 * Created: 5.3.2015 11:54:20
 *  Author: Matjaz
 */ 
#include "stdint-gcc.h"

#ifndef DAC_H_
#define DAC_H_

#define DAC_GAIN
#define DAC_REF		3300




void dac_init (void);

void dac_set (uint16_t ch0, int16_t ch1);



#endif /* DAC_H_ */