/*
 * parser.c
 *
 * Created: 4.3.2015 12:29:30
 *  Author: Matjaz
 */ 

#include "parser.h"
#include "udi_cdc.h"
#include "stdint-gcc.h"
#include <string.h>
#include <stdio.h>


daq_settings_t daqSettings;



void skip_blank_chars (uint8_t *string)
{
	string++;
	while((*string) == ' ' )
	{
		string++;
	}
}


void parse_comands (void)
{
	static uint8_t fsmState = FSM_ID_BYTE;
	static uint8_t comandByte = 0;
	static uint8_t insertPointer = 0;
	uint8_t temp, n = 0;
	static uint8_t holdingBuffer[HOLDING_BUFFER_SIZE];
	static uint8_t tempBuffer[10];
	uint8_t *startOfData;
	uint8_t printBuffer [50];
	uint32_t charsPrinted;
	uint32_t a;
	
	if(udi_cdc_is_rx_ready())
	{
		temp = udi_cdc_getc();
		udi_cdc_putc(temp);
		
		if(insertPointer < (HOLDING_BUFFER_SIZE - 2))
		{
			holdingBuffer[insertPointer] = temp;
			insertPointer++;
		}	
		if(temp == '\r')
		{
			udi_cdc_putc('\n');
			udi_cdc_putc('\r');
			
			holdingBuffer[insertPointer] = 0;
			startOfData = strpbrk(holdingBuffer, LIST_OF_KNOWN_COMANDS);
			//after this executes startOfData should point to first know character in string
			switch (*(startOfData))
			{
				case COMAND_START_ACQ:
					daqSettings.startAcq = 1;
					charsPrinted = sprintf(printBuffer, "Acquisition started\n\r");
					udi_cdc_write_buf(printBuffer, charsPrinted);
					break;
				
				case COMAND_STOP_ACQ:
					daqSettings.stopAcq = 1;
					charsPrinted = sprintf(printBuffer, "Acquisition stoped\n\r");
					udi_cdc_write_buf(printBuffer, charsPrinted);
					break;
					
				case COMAND_SET_SAMPLE_PERIOD:
					skip_blank_chars(startOfData);
					n = 0;
					while(*startOfData >= '0' && *startOfData <= '9')
					{
						if(startOfData > (holdingBuffer + HOLDING_BUFFER_SIZE - 1)) break;
						tempBuffer[n++] = *startOfData++;
					}
					if(*startOfData == '\r')
					{
						tempBuffer[n] = 0;
						daqSettings.timerBase = atoi(tempBuffer);
						charsPrinted = sprintf(printBuffer, "Sample period set to %u uS\n\r", daqSettings.timerBase);
						udi_cdc_write_buf(printBuffer, charsPrinted);
						//todo: limit sample rate period
						//todo: calcualte timeer base based on sample period
					}
			}
			
			insertPointer = 0;
		}
	
	}
}




daq_settings_t * get_current_DAQ_settings (void)
{
		daqSettings.sequence[0]= 1;
		daqSettings.sequence[1]= 2;
		daqSettings.sequence[2]= 3;

	return (&daqSettings);
}
