/*
 * DAC.h
 *
 * Created: 5.3.2015 11:54:20
 *  Author: Matjaz
 */ 
#include "stdint-gcc.h"

#ifndef DAC_H_
#define DAC_H_

#define DAC_GAIN	4883
#define DAC_REF		3300




void dac_init (void);

void dac_set (uint32_t ch, uint32_t val);



#endif /* DAC_H_ */