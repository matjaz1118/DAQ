/*
 * power.c
 *
 * Created: 6. 10. 2016 22:49:43
 *  Author: Matjaz
 */ 

#include "power.h"
#include "pio.h"
#include "pmc.h"

void power_manegment_enable (void)
{
	pmc_enable_periph_clk(ID_PIOA);
	pio_set_output(PIOA, PIO_PA1, LOW, DISABLE, DISABLE);
}

void hv_enable (void)
{
	pio_set_pin_high(1);
}

void hv_disable (void)
{
	pio_set_pin_low(PIO_PA1);
}