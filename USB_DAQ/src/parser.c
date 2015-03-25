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
	uint32_t charsPrinted, entryCounter;
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
				case COMAND_SET_AVERAGE_COUNT:
				case COMAND_SET_MEASURMENT_NBR_COUNT:
					comandByte = *startOfData;
					//skip_blank_chars(startOfData);
					startOfData++;
					n = 0;
					while(*startOfData >= '0' && *startOfData <= '9')
					{
						if(startOfData > (holdingBuffer + HOLDING_BUFFER_SIZE - 1)) break;
						tempBuffer[n++] = *startOfData++;
					}
					if(comandByte == COMAND_SET_SAMPLE_PERIOD)
					{
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
					else if(comandByte == COMAND_SET_AVERAGE_COUNT)
					{
						if(*startOfData == '\r')
						{
							tempBuffer[n] = 0;
							daqSettings.samplesNbr = atoi(tempBuffer);
							charsPrinted = sprintf(printBuffer, "DAQ will atempt to take %u samples per channel\n\r", daqSettings.samplesNbr);
							udi_cdc_write_buf(printBuffer, charsPrinted);
							//todo: limit samples per channel
						}
					}
					else if(comandByte == COMAND_SET_MEASURMENT_NBR_COUNT)
					{
						if(*startOfData == '\r')
						{
							tempBuffer[n] = 0;
							daqSettings.cycles = atoi(tempBuffer);
							charsPrinted = sprintf(printBuffer, "DAQ will sample all enebled channels %u times\n\r", daqSettings.cycles);
							udi_cdc_write_buf(printBuffer, charsPrinted);
							//todo: limit samples per channel
						}
					}
					break;
				
				case COMAND_SET_SEQUENCER:
					//skip_blank_chars();
					startOfData++;
					entryCounter = 0;
					n = 0;
					while(entryCounter < 8)
					{
						n = 0;
						while(*startOfData != ',')
						{
							tempBuffer[n++] = *startOfData++;
							if(*startOfData == '\r') break;
						}
						tempBuffer[n]  = 0;
						a = atoi(tempBuffer);
						if(a)
						{
							daqSettings.sequence[entryCounter] = a;
						}
						else
						{
							daqSettings.sequence[entryCounter] = 0;
							break;
						}
						if(*startOfData == '\r') break;
						entryCounter++;
						startOfData++;
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
