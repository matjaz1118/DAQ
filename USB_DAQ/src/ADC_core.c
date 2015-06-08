/*
 * ADC_core.c
 *
 * Created: 9.3.2015 12:16:16
 *  Author: Matjaz
 */ 
#include "ADC_core.h"
#include "pmc.h"
#include "tc.h"
#include "pio.h"
#include "parser.h"
#include "adc.h"
#include "interrupt.h"
#include "sysclk.h"
#include "stdint-gcc.h"
#include "udi_cdc.h"
#include "adc.h"
#include "delay.h"


uint16_t adcResults [200]; //used to be 8000
daq_settings_t *DAQSettingsPtr;
uint32_t result, sequencePosition = 0, avgCounter, sampleCounter, chCntr;

void adc_enable_freerun (void)
{
	ADC -> ADC_MR |= ADC_MR_FREERUN;
}

void adc_disable_freerun (void)
{
	ADC -> ADC_MR &= ~ADC_MR_FREERUN;
}

void timer_init (void)
{
	
	
	pmc_enable_periph_clk(ID_TC0);
	pmc_enable_periph_clk(ID_PIOA);
	//pio_set_peripheral(PIOA, PIO_TYPE_PIO_PERIPH_B, PIO_PA0);
	
	tc_init(TC0, 0, TC_CMR_TCCLKS_TIMER_CLOCK4 | TC_CMR_WAVE | TC_CMR_WAVSEL_UP_RC );
	tc_write_rc(TC0, 0, 50000);
	tc_enable_interrupt(TC0, 0, TC_IER_CPCS);
	NVIC_SetPriority(TC0_IRQn, 2);
	NVIC_EnableIRQ(TC0_IRQn);
}

void ADC_init (void)
{
	uint32_t dummy;
	pmc_enable_periph_clk(ID_ADC);
	pmc_enable_periph_clk(ID_PIOA);
	pmc_enable_periph_clk(ID_PIOB);
	
	pio_set_peripheral(PIOA, PIO_TYPE_PIO_PERIPH_D, PIO_PA17 | PIO_PA18 | PIO_PA19 | PIO_PA20);
	pio_set_peripheral(PIOB, PIO_TYPE_PIO_PERIPH_D, PIO_PB0 | PIO_PB1 | PIO_PB2 | PIO_PB3);
	
	//todo: set inputs to bi differential!!!
	adc_init(ADC, sysclk_get_main_hz(), ADC_FREQ_MAX, ADC_STARTUP_TIME_1);
	adc_configure_timing(ADC, 0, ADC_SETTLING_TIME_0, 1);
	adc_set_resolution(ADC, ADC_MR_LOWRES_BITS_12);
	adc_configure_trigger(ADC, ADC_TRIG_SW, 0); // there is a bug in ASF driver for ADC, so free run mode has to be enabled manualy
	ADC->ADC_MR |= ADC_MR_USEQ;  // use sequencer
	adc_enable_tag(ADC);
	delay_ms(1);
	dummy = ADC->ADC_ISR;

}

void limit_average_nbr (void)
{
	uint32_t  maxAvg, charsPrinted;
	uint8_t printBuffer[50];
	
	maxAvg = ((DAQSettingsPtr->timerBase * 20) - chCntr * TIME_PRINT_PER_CH) / ((chCntr * (TIME_CALCULATE_PER_CH_SAMPLE + TIME_SAMPLE)));
	if(maxAvg)
	{
		if (maxAvg < avgCounter)
		{
			avgCounter = maxAvg;
			charsPrinted = sprintf(printBuffer, "Avering limited to %u samples per channel\n\r", avgCounter);
			udi_cdc_write_buf(printBuffer, charsPrinted);
		}
	}
	else
	{
		//todo: we should handle this some other way...
		charsPrinted = sprintf(printBuffer, "Avering limited\n\r");
		udi_cdc_write_buf(printBuffer, charsPrinted);
		avgCounter = 1;
	}
}



void aquisition_start (void)
{
	uint32_t i = 0;
	chCntr = 0;
	DAQSettingsPtr = get_current_DAQ_settings();
	avgCounter = DAQSettingsPtr->avgCounter;
	sampleCounter = DAQSettingsPtr->cycles;
	DAQSettingsPtr->newData = FALSE;
	ADC->ADC_MR &= ~ ADC_MR_FREERUN;
	adc_stop_sequencer(ADC);
	adc_disable_all_channel(ADC);
	ADC->ADC_SEQR1 = 0;
	for(i = 0; DAQSettingsPtr->sequence[i]; i++)
	{
		ADC->ADC_SEQR1 |= (DAQSettingsPtr->sequence[i] << (i * 4));
		adc_enable_channel(ADC, chCntr++);
	}
	limit_average_nbr();
	tc_write_rc(TC0, 0, DAQSettingsPtr->timerBase);
	ADC->ADC_RPR = adcResults;
	ADC->ADC_RCR = chCntr * avgCounter;
	ADC->ADC_PTCR = ADC_PTCR_RXTEN;
	adc_start_sequencer(ADC);
	adc_enable_interrupt(ADC, ADC_IER_RXBUFF);
	NVIC_EnableIRQ(ADC_IRQn);
	tc_start(TC0, 0);
	ADC->ADC_MR |= ADC_MR_FREERUN;	//enable freerun mode
	
	//Setup adc and start the timer. Everithing else happens in TIMER ISR
}

void aquisition_stop (void)
{
	tc_stop(TC0, 0);
	ADC->ADC_MR &= ~ADC_MR_FREERUN;
	
}

void ADC_Handler (void)
{
	uint8_t printBuffer[20];
	volatile uint32_t finalValues [4] = {0, 0, 0, 0};
	uint32_t i, charsPrinted;
	volatile uint32_t reg0, reg1, reg2, reg3, reg4;
	
	ADC->ADC_MR &= ~ADC_MR_FREERUN;
	ADC->ADC_PTCR |= ADC_PTCR_RXTDIS;
	
	if(adc_get_status(ADC) & ADC_ISR_RXBUFF)
	{
		reg0 = ADC->ADC_MR;
		reg1 = ADC->ADC_SEQR1;
		reg2 = ADC->ADC_CHSR;
		reg3 = ADC->ADC_EMR;
		reg4 = ADC->ADC_IMR;
		adc_stop(ADC);
		for(i = 0; i < (chCntr * avgCounter); i++)
		{
			finalValues[i % chCntr] += adcResults[i];
		}
		for(i = 0; i < chCntr; i++)
		{
			charsPrinted = sprintf(printBuffer, "CH%u: %u\n\r", i, (finalValues[i] / avgCounter));
			udi_cdc_write_buf(printBuffer, charsPrinted);
		}
		sampleCounter--;
		ADC->ADC_MR = reg0;
		ADC->ADC_SEQR1 = reg1;
		ADC->ADC_CHER = reg2;
		ADC->ADC_EMR = reg3;
		ADC->ADC_IER = reg4;
		ADC->ADC_RPR = adcResults;
		ADC->ADC_RCR = chCntr * avgCounter;
		
	}
	
	
}


void TC0_Handler (void)
{
	volatile uint32_t ul_dummy;

	// Clear status bit to acknowledge interrupt
	ul_dummy = tc_get_status(TC0, 0);	
	pio_toggle_pin_group(PIOA, PIO_PA9);
	if(sampleCounter)
	{	
		ADC->ADC_PTCR = ADC_PTCR_RXTEN;
		ADC->ADC_MR |= ADC_MR_FREERUN;	//enable freerun mode
	}
	else
	{
		aquisition_stop();	
	}
}