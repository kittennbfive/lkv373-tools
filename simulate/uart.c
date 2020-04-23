/*
This file is part of the simulator for the LKV373
(c) 2019-2020 by kitten_nb_five
freenode #lkv373a

licence: AGPL v3 or later

THIS PROGRAM COMES WITHOUT ANY WARRANTY!
*/

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "uart.h"
#include "peripherals.h"
#include "breakpoints.h"
#include "window.h"
#include "ansi.h"
#include "memory.h"

static int w_uart;

//some adresses are corresponding to different registers depending on access mode (read or write)!
//page 320

//WRITE
#define TRANSMITTER_HOLDING_REG 0x99600000

#define INT_ENABLE_REG 0x99600004

#define FIFO_CTRL_REG 0x99600008
#define ENABLE_FIFO 0x01

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

#define MODEM_STATUS_REG 0x99600018

int init_uart_view(void)
{
	w_uart=open_new_window("UART", NULL, NULL);
	
	//win_printf(w_uart, HIDE_CURSOR); //so we can see newlines
	return w_uart;
}

uint32_t int_enable_reg=0;
uint32_t int_ident_reg=0x01;
uint32_t line_ctrl_reg=0;

void uart_write(PERIPH_CB_WRITE_ARGUMENTS)
{
	static bool divisor_latch_access_bit=false;
	
	switch(addr)
	{
		case TRANSMITTER_HOLDING_REG:
			if(divisor_latch_access_bit)
			{
				printf("UART: prescaler set to %x\n", val);
			}
			else
				win_printf(w_uart, "%c", (uint8_t)val);
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
			printf("val 0x%x written to UART INT_ENABLE_REG\n", val);
			break;
		
		case FIFO_CTRL_REG:
			break;
		
		default:
			printf("UART: unhandled register write 0x%x @0x%x\n", val, addr);
			break;
	}
}

bool uart_read(PERIPH_CB_READ_ARGUMENTS)
{
	switch(addr)
	{
		case RECEIVER_BUFFER_REG:
			printf("UART: receiver buffer is read, returning 0\n");
			(*val)=0;
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
			(*val)=(1<<5)|(1<<6); //in simulation we are always ready to accept new char
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
			printf("UART: unhandled read from 0x%x\n", addr);
			return false;
	}
}

