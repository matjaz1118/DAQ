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


uint16_t adcResults [1000]; //used to be 8000
uint8_t binaryBuffer [52];
daq_settings_t *DAQSettingsPtr;
uint32_t result, sequencePosition = 0, avgCounter, sampleCounter, chCntr, binCntr;

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
	NVIC_SetPriority(TC0_IRQn, 5);
	NVIC_EnableIRQ(TC0_IRQn);
}

void ADC_init (void)
{
	uint32_t dummy;
	pmc_enable_periph_clk(ID_ADC);
	pmc_enable_periph_clk(ID_PIOA);
	pmc_enable_periph_clk(ID_PIOB);
	
	pio_set_peripheral(PIOA, PIO_TYPE_PIO_INPUT, PIO_PA17 | PIO_PA18 | PIO_PA19 | PIO_PA20);
	pio_set_peripheral(PIOB, PIO_TYPE_PIO_INPUT, PIO_PB0 | PIO_PB1 | PIO_PB2 | PIO_PB3);
	
	PIOA->PIO_PUDR = (PIO_PUDR_P17 | PIO_PUDR_P18 | PIO_PUDR_P19 | PIO_PUDR_P20);
	PIOB->PIO_PUDR = (PIO_PUDR_P0 | PIO_PUDR_P1 | PIO_PUDR_P2 | PIO_PUDR_P3);
	
	
	ADC->ADC_COR = (0x000000FF << 16); // set inputs to bi fully differentzial
	adc_init(ADC, sysclk_get_main_hz(), ADC_FREQ_MAX, ADC_STARTUP_TIME_1);
	adc_configure_timing(ADC, 0, ADC_SETTLING_TIME_0, 1);
	adc_set_resolution(ADC, ADC_MR_LOWRES_BITS_12);
	adc_configure_trigger(ADC, ADC_TRIG_SW, 0); // there is a bug in ASF driver for ADC, so free run mode has to be enabled manualy
	ADC->ADC_MR |= ADC_MR_USEQ;  // use sequencer
	adc_enable_tag(ADC);
	delay_ms(1);
	dummy = ADC->ADC_ISR;

}

static uint32_t limit_average_nbr (void)
{
	int32_t  maxAvg, charsPrinted;
	uint8_t printBuffer[50];
	
	maxAvg = (((int32_t)DAQSettingsPtr->timerBase * 20) - (int32_t)chCntr * TIME_PRINT_PER_CH) / (((int32_t)chCntr * (TIME_CALCULATE_PER_CH_SAMPLE + TIME_SAMPLE)));
	if(maxAvg > 0)
	{
		if (maxAvg < avgCounter)
		{
			avgCounter = maxAvg;
			charsPrinted = sprintf(printBuffer, "Avering limited to %u samples per channel\n\r", avgCounter);
			udi_cdc_write_buf(printBuffer, charsPrinted);
		}
		return 1;
	}
	else
	{
		//todo: we should handle this some other way...
		/*
		charsPrinted = sprintf(printBuffer, "Impossible to start samplig\n\r");
		udi_cdc_write_buf(printBuffer, charsPrinted);
		avgCounter = 1;
		return 0;
		*/
	}
	
}

static void validate_daq_settings_struct (void)
{
	if(DAQSettingsPtr->avgCounter < 1) {DAQSettingsPtr->avgCounter = 1;}
	if(DAQSettingsPtr->sequence[0] == 0) {DAQSettingsPtr->sequence[0] = 1; DAQSettingsPtr->sequence[1] = 0;}
	if(DAQSettingsPtr->cycles < 1) {DAQSettingsPtr->cycles = 1;}
}

void aquisition_start (void)
{
	uint32_t i = 0;
	chCntr = 0;
	binCntr = 0;
	DAQSettingsPtr = get_current_DAQ_settings();
	validate_daq_settings_struct();
	avgCounter = DAQSettingsPtr->avgCounter;
	sampleCounter = DAQSettingsPtr->cycles;
	DAQSettingsPtr->newData = FALSE;
	ADC->ADC_MR &= ~ ADC_MR_FREERUN;
	adc_stop_sequencer(ADC);
	adc_disable_all_channel(ADC);
	ADC->ADC_SEQR1 = 0;
	for(i = 0; DAQSettingsPtr->sequence[i]; i++)
	{
		ADC->ADC_SEQR1 |= (((DAQSettingsPtr->sequence[i] - 1) * 2) << (i * 4));
		adc_enable_channel(ADC, chCntr++);
	}
	if(limit_average_nbr())
	{
		tc_write_rc(TC0, 0, DAQSettingsPtr->timerBase);
		ADC->ADC_RPR = adcResults;
		ADC->ADC_RCR = chCntr * avgCounter;
		ADC->ADC_PTCR = ADC_PTCR_RXTEN;
		adc_start_sequencer(ADC);
		adc_enable_interrupt(ADC, ADC_IER_RXBUFF);
		NVIC_SetPriority(ADC_IRQn, 5);
		NVIC_EnableIRQ(ADC_IRQn);
		tc_start(TC0, 0);
		//TC0->TC_CHANNEL[0].TC_CCR |= TC_CCR_SWTRG;
		ADC->ADC_MR |= ADC_MR_FREERUN;	//enable freerun mode
		
		//Setup adc and start the timer. Everithing else happens in TIMER ISR
	}
	else
	{
		aquisition_stop();
	}
	
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
	uint32_t i, charsPrinted, chnannel;
	int32_t result;
	volatile uint32_t reg0, reg1, reg2, reg3, reg4, reg5;
	
	union
	{
		uint16_t value;
		uint8_t result[2];
	} spliter;
	
	ADC->ADC_MR &= ~ADC_MR_FREERUN;
	ADC->ADC_PTCR |= ADC_PTCR_RXTDIS;
	
	if(adc_get_status(ADC) & ADC_ISR_RXBUFF)
	{
		reg0 = ADC->ADC_MR;
		reg1 = ADC->ADC_SEQR1;
		reg2 = ADC->ADC_CHSR;
		reg3 = ADC->ADC_EMR;
		reg4 = ADC->ADC_IMR;
		reg5 = ADC->ADC_COR;
		adc_stop(ADC);
		for(i = 0; i < (chCntr * avgCounter); i++)
		{
			finalValues[i % chCntr] += adcResults[i];
		}
		if(DAQSettingsPtr->comMode == ASCII_MODE)
		{
			for(i = 0; i < chCntr; i++)
			{
				chnannel = (finalValues[i] / avgCounter) >> 12;
				result = (finalValues[i] / avgCounter) & 0x0FFF;
				result = ((result - 2048) * ADC_REF) / (4096 * AMP_GAIN);
				charsPrinted = sprintf(printBuffer, "CH%u: %imV\n\r", chnannel, result);
				udi_cdc_write_buf(printBuffer, charsPrinted);
			}
		}
		else
		{
			for(i = 0; i < chCntr; i++)
			{
				spliter.value = (finalValues[i] / avgCounter);
				binaryBuffer[binCntr++] = spliter.result[1];
				binaryBuffer[binCntr++] = spliter.result[0];
			}
		}
		

		sampleCounter--;
		ADC->ADC_MR = reg0;
		ADC->ADC_SEQR1 = reg1;
		ADC->ADC_CHER = reg2;
		ADC->ADC_EMR = reg3;
		ADC->ADC_IER = reg4;
		ADC->ADC_RPR = adcResults;
		ADC->ADC_RCR = chCntr * avgCounter;
		ADC->ADC_COR = reg5;
		
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
		if(DAQSettingsPtr->comMode == FAST_MODE)
		{
			udi_cdc_write_buf(binaryBuffer, binCntr);
			udi_cdc_write_buf("\n\r", 2);
		}
	}
}