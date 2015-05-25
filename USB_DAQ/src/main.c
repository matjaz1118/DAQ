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
	// Insert system clock initialization code here (sysclk_init()).
	
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
	//udc_start();

	
	adc_enable_tag(ADC);

	
	
	ADC->ADC_SEQR1 = ((3 << 0) | (2 << 4) | (1 << 8) | (0 << 12));
	adc_enable_channel(ADC, ADC_CHANNEL_0);
	adc_enable_channel(ADC, ADC_CHANNEL_1);
	adc_enable_channel(ADC, ADC_CHANNEL_2);
	adc_enable_channel(ADC, ADC_CHANNEL_3);
	ADC->ADC_MR |= ADC_MR_USEQ;
	
	ADC->ADC_MR |= ADC_MR_FREERUN;
	//ADC->ADC_RPR = adcResults;
	ADC->ADC_RCR = 8;
	ADC->ADC_PTCR = ADC_PTCR_RXTEN;
	adc_start(ADC);
	
	

	
	while(1)
	{
		//adcResults[12]++;
		//if(adcResults[12] < 3) adcResults[6]++;
	}
	
		
	while(1)
	{
		parse_comands();
		delay_ms(10);
	}
	// Insert application code here, after the board has been initialized.
}
