/*
 * parser.c
 *
 * Created: 4.3.2015 12:29:30
 *  Author: Matjaz
 */ 

#include "parser.h"
#include "udi_cdc.h"
#include "stdint-gcc.h"


daq_settings_t daqSettings;



void parse_comands (void)
{
	static uint8_t fsmState = FSM_ID_BYTE;
	static uint8_t comandByte = 0;
	static uint8_t bytesToGo = 0;
	static uint8_t temp[8];
	
	if(udi_cdc_is_rx_ready())
	{
		switch(fsmState)
		{
			
			case FSM_ID_BYTE :
				comandByte = udi_cdc_getc();
				switch(comandByte)
				{
					case(DAQ_START):
						//daq_start();
						break;
					case(DAQ_STOP): 
						//daq_stop();
						break;
					case(DAQ_SET_FREQUENCY):
						bytesToGo = 1;
						fsmState = FSM_SET_FREQ;
						break;
					case(DAQ_SET_SAMPLE_NBR):
						bytesToGo = 2;
						fsmState = FSM_WAIT_2_BYTES;
						break;
					case(DAQ_SET_CYCLE_NBR):
						bytesToGo = 2;
						fsmState = FSM_WAIT_2_BYTES;
						break;
					case(DAQ_SET_ANALOG_OUT):
						bytesToGo = 2;
						fsmState = FSM_WAIT_2_BYTES;
						break;
					case(DAQ_SET_SEQUENCER):
						bytesToGo = 8;
						fsmState = FSM_WAIT_8_BYTES;
						break;
					default:
						fsmState = FSM_ID_BYTE;
						break;
					
				}
				break;
			
			case FSM_SET_FREQ :
				daqSettings.timerBase = udi_cdc_getc();
				daqSettings.newData = TRUE;
				fsmState = FSM_ID_BYTE;
				break;
				
				
			case FSM_WAIT_2_BYTES :
				temp[2 - bytesToGo] = udi_cdc_getc();
				bytesToGo--;
				if(!bytesToGo)
				{
					if(comandByte == 0x12)
					{
						daqSettings.samplesNbr = ((uint16_t)temp[0] << 8) | temp[1]; 
						daqSettings.newData = TRUE;
					}
					else if(comandByte == 0x22)
					{
						daqSettings.cycles = ((uint16_t)temp[0] << 8) | temp[1];
						daqSettings.newData = TRUE;
					}
					else if(comandByte == 0x32)
					{
						//dac_set(((uint16_t)temp[0] << 8) | temp[1]);
					}
					fsmState = FSM_ID_BYTE;
				}
				
				break;
				
				
			case FSM_WAIT_8_BYTES :
				temp[8 - bytesToGo] = udi_cdc_getc();
				bytesToGo--;
				if(!bytesToGo)
				{
					for(bytesToGo = 0; bytesToGo < 8; bytesToGo++)
					{
						daqSettings.sequence[bytesToGo] = temp[bytesToGo];
					}
					daqSettings.newData = TRUE;
					fsmState = FSM_ID_BYTE;
				}
				break;
				
			default :
				break;
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
