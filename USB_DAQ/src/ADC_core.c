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
uint32_t result, sequencePosition = 0, avgCounter, sampleCounter;

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
	pmc_enable_periph_clk(ID_ADC);
	pmc_enable_periph_clk(ID_PIOA);
	pmc_enable_periph_clk(ID_PIOB);
	
	pio_set_peripheral(PIOA, PIO_TYPE_PIO_PERIPH_D, PIO_PA17 | PIO_PA18 | PIO_PA19 | PIO_PA20);
	pio_set_peripheral(PIOB, PIO_TYPE_PIO_PERIPH_D, PIO_PB0 | PIO_PB1 | PIO_PB2 | PIO_PB3);
	
	//todo: set inputs to bi differential!!!
	adc_init(ADC, sysclk_get_main_hz(), ADC_FREQ_MAX, ADC_STARTUP_TIME_1);
	adc_configure_timing(ADC, 0, ADC_SETTLING_TIME_0, 1);
	adc_set_resolution(ADC, ADC_MR_LOWRES_BITS_12);
	adc_enable_interrupt(ADC, ADC_ISR_DRDY);
	adc_configure_trigger(ADC, ADC_TRIG_SW, 0); // there is a bug in ASF driver for ADC, so free run mode has to be enabled manualy
	//adc_enable_channel(ADC, ADC_CHANNEL_4);
	NVIC_SetPriority(ADC_IRQn, 1);
	NVIC_EnableIRQ(ADC_IRQn);
	delay_ms(1);	
}


void aquisition_start (void)
{

	DAQSettingsPtr = get_current_DAQ_settings();
	avgCounter = DAQSettingsPtr->avgCounter;
	sampleCounter = DAQSettingsPtr->cycles;
	DAQSettingsPtr->newData = FALSE;
	adc_disable_all_channel(ADC);
	tc_write_rc(TC0, 0, DAQSettingsPtr->timerBase);
	tc_start(TC0, 0);
	//Setup adc and start the timer. Everithing else happens in TIMER ISR
	
}

void aquisition_stop (void)
{
	tc_stop(TC0, 0);
}

void ADC_Handler (void)
{ 
	static uint32_t accumulator = 0;
	uint32_t result, status;
	uint32_t charsPrinted;
	uint8_t printBuffer[20];
	
	status = adc_get_status(ADC);
	accumulator += adc_get_latest_value(ADC);
	avgCounter--;
	if(avgCounter == 1) {adc_disable_freerun(); adc_disable_all_channel(ADC);}
	if(!avgCounter)
	{
		//adc_stop(ADC);
		adc_disable_freerun();
		result = accumulator  / DAQSettingsPtr->avgCounter; 
		accumulator = 0;
		//todo: convert result to mV
		charsPrinted = sprintf(printBuffer, "%u\n\r", result);
		udi_cdc_write_buf(printBuffer, charsPrinted);
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
		avgCounter = DAQSettingsPtr->avgCounter;
		adc_disable_all_channel(ADC);
		adc_enable_channel(ADC, DAQSettingsPtr->sequence[sequencePosition]);
		adc_enable_freerun();
		adc_start(ADC);
		
		sequencePosition++;
		if(!DAQSettingsPtr->sequence[sequencePosition])
		{
			sampleCounter--;
			sequencePosition = 0;
		}
	}
	else
	{
		adc_disable_all_channel(ADC);
		adc_disable_freerun();
		tc_stop(TC0, 0);
	}
}