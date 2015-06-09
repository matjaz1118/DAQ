/**
 * \file
 *
 * \brief Empty user application template
 *
 */

/**
 * \mainpage User Application template doxygen documentation
 *
 * \par Empty user application template
 *
 * Bare minimum empty user application template
 *
 * \par Content
 *
 * -# Include the ASF header files (through asf.h)
 * -# "Insert system clock initialization code here" comment
 * -# Minimal main function that starts with a call to board_init()
 * -# "Insert application code here" comment
 *
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
#include "asf.h"
#include "delay.h"
#include "parser.h"
#include "DAC.h"
#include "ADC_core.h"
#include "wdt.h"

int main (void)
{
	
	wdt_disable(WDT);
	board_init();
	
	pio_set_output(PIOA, PIO_PA9, LOW, DISABLE, DISABLE);
	timer_init();
	ADC_init();
	/*
	ADC_init();
	timer_init();
	pio_set_output(PIOA, PIO_PA17, LOW, DISABLE, DISABLE);
	*/
	
	
	
	init_daq_settings_struct();
	udc_start();

	
		
	while(1)
	{
		parse_comands();
	}
	// Insert application code here, after the board has been initialized.
}
