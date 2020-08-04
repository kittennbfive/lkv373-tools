/*
This file is part of the simulator for the LKV373
(c) 2019-2020 by kitten_nb_five
freenode #lkv373a

licence: AGPL v3 or later

THIS PROGRAM COMES WITHOUT ANY WARRANTY!
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "uart.h"
#include "peripherals.h"
#include "breakpoints.h"
#include "window.h"
#include "ansi.h"
#include "memory.h"
#include "verbosity.h"
#include "cmd_parser.h"

static int w_uart;

FILE *output=NULL;

//some adresses are corresponding to different registers depending on access mode (read or write)!
//page 320

//WRITE
#define TRANSMITTER_HOLDING_REG 0x99600000

#define INT_ENABLE_REG 0x99600004
#define RECEIVER_DATA_AVAILABLE 0

#define FIFO_CTRL_REG 0x99600008
#define ENABLE_FIFO 1

#define LINE_CONTROL_REG 0x9960000c
#define DIVISOR_LATCH_ACCESS_BIT 0x80
//with enabled access bit:
#define BAUD_RATE_DIVISOR_LSB_REG 0x99600000
#define BAUD_RATE_DIVISOR_MSB_REG 0x99600004
#define PRESCALER_REG 0x99600008

#define MODEM_CTRL_REG 0x99600010

#define TESTING_REG 0x99600014

#define SCRATCH_PAD_REG 0x9960001C

//READ
#define RECEIVER_BUFFER_REG 0x99600000

#define INT_IDENT_REG 0x99600008

#define LINE_STATUS_REG 0x99600014
#define STATUS_REG_DATA_READY 0
#define STATUS_REG_THR_EMPTY 5

#define MODEM_STATUS_REG 0x99600018


uint32_t int_enable_reg=0;
uint32_t int_ident_reg=0;
uint32_t line_ctrl_reg=0;
uint32_t line_status_reg=(1<<STATUS_REG_THR_EMPTY); //in simulation we are always ready to accept new char


#define FIFO_RX_SIZE 32
static uint32_t fifo_rx[FIFO_RX_SIZE]={0};
static uint8_t nb_entries_fifo_rx=0;

static uint32_t fifo_pop(void)
{
	if(nb_entries_fifo_rx)
	{
		uint32_t val=fifo_rx[0];
		memmove(&fifo_rx[0], &fifo_rx[1], (FIFO_RX_SIZE-1)*sizeof(uint32_t));
		nb_entries_fifo_rx--;
		if(nb_entries_fifo_rx==0)
			line_status_reg&=~(1<<STATUS_REG_DATA_READY);
		
		return val;
	}
	else
	{
		MSG(MSG_PERIPH_UART, "UART: trying to read from empty fifo, returning 0\n");
		return 0;
	}
}

static void fifo_push(const uint32_t val)
{
	if(nb_entries_fifo_rx>=FIFO_RX_SIZE)
		(void)fifo_pop();
	
	fifo_rx[nb_entries_fifo_rx++]=val;
	
	line_status_reg|=(1<<STATUS_REG_DATA_READY);
}
/*
static void fifo_clear(void)
{
	nb_entries_fifo_rx=0;
}
*/
int init_uart_view(void)
{
	w_uart=open_new_window("UART", NULL, NULL);
	
	//win_printf(w_uart, HIDE_CURSOR); //so we can see newlines
	return w_uart;
}

void uart_write(PERIPH_CB_WRITE_ARGUMENTS)
{
	(void)sz;
	
	static bool divisor_latch_access_bit=false;
	
	switch(addr)
	{
		case TRANSMITTER_HOLDING_REG:
			if(divisor_latch_access_bit)
			{
				MSG(MSG_PERIPH_UART, "UART: prescaler set to %x\n", val);
			}
			else
			{
				win_printf(w_uart, "%c", (uint8_t)val);
				if(output)
					fprintf(output, "%c", (uint8_t)val);
				
				//fprintf(stderr, "%c", (uint8_t)val);
			}
				
			break;
		
		case LINE_CONTROL_REG:
			line_ctrl_reg=val;
			if(line_ctrl_reg&DIVISOR_LATCH_ACCESS_BIT)
				divisor_latch_access_bit=true;
			else
				divisor_latch_access_bit=false;
			break;
		
		case INT_ENABLE_REG:
			int_enable_reg=val;
			MSG(MSG_PERIPH_UART, "val 0x%x written to UART INT_ENABLE_REG\n", val);
			break;
		
		case FIFO_CTRL_REG:
			break;
		
		default:
			MSG(MSG_PERIPH_UART, "UART: unhandled register write 0x%x @0x%x\n", val, addr);
			break;
	}
}

bool uart_read(PERIPH_CB_READ_ARGUMENTS)
{
	(void)sz;
	
	switch(addr)
	{
		case RECEIVER_BUFFER_REG:
			(*val)=fifo_pop();
			return true;
			break;
		
		case INT_ENABLE_REG:
			(*val)=int_enable_reg;
			return true;
			break;
		
		case INT_IDENT_REG:
			(*val)=int_ident_reg;
			return true;
			break;
		
		case LINE_STATUS_REG:
			(*val)=line_status_reg;
			return true;
			break;
		
		case MODEM_STATUS_REG:
			(*val)=0;
			return true;
			break;
		
		case LINE_CONTROL_REG:
			(*val)=line_ctrl_reg;
			return true;
			break;
		
		default:
			MSG(MSG_PERIPH_UART, "UART: unhandled read from 0x%x\n", addr);
			return false;
	}
}

uint8_t uart_keypress(char c)
{
	fifo_push(c);
	return 0;
}

static void cleanup(void)
{
	if(output)
		fclose(output);
}

void uart_to_file(PROTOTYPE_ARGS_HANDLER)
{
	ARGS_HANDLER_UNUSED;
	
	char *filename=get_next_argument();
	
	output=fopen(filename, "wb");
	if(!output)
	{
		printf("UART: can't open file %s for logging!\n", filename);
		return;
	}	
	
	atexit(&cleanup);
	printf("writing UART to file %s\n", filename);
}
