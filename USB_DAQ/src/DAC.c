/*
 * DAC.c
 *
 * Created: 5.3.2015 11:54:05
 *  Author: Matjaz
 */ 
#include "DAC.h"
#include "dacc.h"
#include "pmc.h"
#include "pio.h"
#include "stdint-gcc.h"

void dac_init (void)
{
	pmc_enable_periph_clk(ID_DACC);
	pmc_enable_periph_clk(ID_PIOB);
	
	pio_set_peripheral(PIOB, PIO_TYPE_PIO_PERIPH_D, PIO_PB13);
	pio_set_peripheral(PIOB, PIO_TYPE_PIO_PERIPH_D, PIO_PB14);
	dacc_reset(DACC);
	dacc_set_writeprotect(DACC, 0);
	dacc_enable_channel(DACC, 0);
	dacc_enable_channel(DACC, 1);	
	dacc_set_timing(DACC, 1, 0, 15);
	dacc_disable_trigger(DACC);
	
	
	
}

/* Sets DAC value iv mV */

void dac_set (uint32_t ch, uint32_t val)
{
	if(ch)
	{
		dacc_set_channel_selection(DACC, 1);
		dacc_write_conversion_data(DACC, val);
	}
	else
	{
		dacc_set_channel_selection(DACC, 0);
		dacc_write_conversion_data(DACC, val);
	}
}
