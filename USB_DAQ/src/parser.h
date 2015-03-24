/*
 * parser.h
 *
 * Created: 4.3.2015 12:29:51
 *  Author: Matjaz
 */ 

#include "stdint-gcc.h"

#ifndef PARSER_H_
#define PARSER_H_

#define TRUE	1
#define FALSE	0

//Packet number definitions
#define DAQ_START				0x10		
#define DAQ_STOP				0x20
#define DAQ_SET_FREQUENCY		0x11
#define DAQ_SET_SEQUENCER		0x18
#define DAQ_SET_SAMPLE_NBR		0x12
#define DAQ_SET_CYCLE_NBR		0x22
#define DAQ_SET_ANALOG_OUT		0x32


//Structure definitions
typedef struct
{
	uint8_t newData;
	uint32_t timerBase;
	uint8_t sequence[8];
	uint16_t samplesNbr;
	uint16_t cycles;
}daq_settings_t;

//Finite state machine states definitions 

#define FSM_ID_BYTE				0
#define	FSM_WAIT_2_BYTES		1
#define FSM_WAIT_8_BYTES		2
#define FSM_SET_FREQ			3



void parse_comands(void);
daq_settings_t * get_current_DAQ_settings (void);

#endif /* PARSER_H_ */