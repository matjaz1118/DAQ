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

const uint16_t adcSampleRateLUT [15] = {
	ADC_SAMPLE_RATE_10HZ,
	ADC_SAMPLE_RATE_25HZ,
	ADC_SAMPLE_RATE_50HZ,
	ADC_SAMPLE_RATE_100HZ,
	ADC_SAMPLE_RATE_250HZ,
	ADC_SAMPLE_RATE_500HZ,
	ADC_SAMPLE_RATE_1000HZ,
	ADC_SAMPLE_RATE_2500HZ,
	ADC_SAMPLE_RATE_5000HZ,
	ADC_SAMPLE_RATE_10000HZ,
	ADC_SAMPLE_RATE_20000HZ,
	ADC_SAMPLE_RATE_25000HZ,
	ADC_SAMPLE_RATE_50000HZ,
	ADC_SAMPLE_RATE_100000HZ,
	ADC_SAMPLE_RATE_250000HZ
};


daq_settings_t *DAQSettingsPtr;
uint32_t result, sequencePosition = 1, avgCounter, sampleCounter;



void timer_init (void)
{
	
	
	pmc_enable_periph_clk(ID_TC0);
	pmc_enable_periph_clk(ID_PIOA);
	pio_set_peripheral(PIOA, PIO_TYPE_PIO_PERIPH_B, PIO_PA0);
	
	tc_init(TC0, 0, TC_CMR_TCCLKS_TIMER_CLOCK4 | TC_CMR_CPCTRG | TC_CMR_WAVE | TC_CMR_WAVSEL_UP_RC | TC_CMR_ACPA_CLEAR | TC_CMR_ACPC_SET);
	tc_write_rc(TC0, 0, 50000);
	tc_write_ra(TC0, 0, 100);
	tc_start(TC0, 0);
}

void ADC_init (void)
{
	pmc_enable_periph_clk(ID_ADC);
	pmc_enable_periph_clk(ID_PIOA);
	pmc_enable_periph_clk(ID_PIOB);
	
	pio_set_peripheral(PIOA, PIO_TYPE_PIO_PERIPH_D, PIO_PA17 | PIO_PA18 | PIO_PA19 | PIO_PA20);
	pio_set_peripheral(PIOB, PIO_TYPE_PIO_PERIPH_D, PIO_PB0 | PIO_PB1 | PIO_PB2 | PIO_PB3);
	
	//set inputs to bi differential!!!
	adc_init(ADC, sysclk_get_main_hz(), ADC_CLK, ADC_STARTUP_TIME_1);
	adc_configure_timing(ADC, 0, 1, 2);
	adc_set_resolution(ADC, ADC_MR_LOWRES_BITS_12);
	adc_enable_interrupt(ADC, ADC_ISR_DRDY);
	adc_configure_trigger(ADC, ADC_TRIG_TIO_CH_0, ADC_MR_FREERUN_OFF);
	adc_enable_channel(ADC, ADC_CHANNEL_4);
	NVIC_EnableIRQ(ADC_IRQn);	
}


void aquisition_start (void)
{
	DAQSettingsPtr = get_current_DAQ_settings();
	avgCounter = DAQSettingsPtr->cycles;
	sampleCounter = DAQSettingsPtr->avgCounter;
	DAQSettingsPtr->newData = FALSE;
	tc_write_rc(TC0, 0, DAQSettingsPtr->timerBase);
	tc_write_ra(TC0, 0, 2);
	//Setup adc and start the timer. Everithing else happens in ADC ISR
}

void next_in_sequence (void)
{
	
}

void ADC_Handler (void)
{ 
	static uint32_t accumulator = 0;
	uint32_t result;
	uint32_t charsPrinted;
	uint8_t printBuffer[20];
	
	accumulator += adc_get_latest_value(ADC);
	if(avgCounter++ == DAQSettingsPtr->avgCounter)
	{
		//stop th efree run mode of adc!!
		result = accumulator / DAQSettingsPtr->avgCounter;
		//todo: convert result to mV
		charsPrinted = sprintf(printBuffer, "%u", result);
		udi_cdc_write_buf(printBuffer, charsPrinted);
		next_in_sequence();
			
	}
}


