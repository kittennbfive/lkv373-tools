/*
This file is part of the simulator for the LKV373
(c) 2019-2020 by kitten_nb_five
freenode #lkv373a

licence: AGPL v3 or later

THIS PROGRAM COMES WITHOUT ANY WARRANTY!
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "sspc.h"
#include "peripherals.h"
#include "memory.h"
#include "flash_spi.h"
#include "verbosity.h"

#define SSP_CTRL_REG0 0x98b00000
#define SSP_CTRL_REG1 0x98b00004
#define SSP_CTRL_REG2 0x98b00008
#define SSP_TX_FIFO_CLR (1<<3)
#define SSP_RX_FIFO_CLR (1<<2)
#define SSP_TX_DATA_OUT_ENABLE (1<<1)
#define SSP_ENABLE (1<<0)

#define SSP_STATUS_REG 0x98b0000c

#define SSP_INT_CTRL_REG 0x98b00010
#define SSP_INT_STATUS_REG 0x98b00014
#define SSP_DATA_REG 0x98b00018
#define SSP_INFO_REG 0x98b0001c
#define SSP_AC_SLOT_VALID_REG 0x98b00020


uint32_t status_reg;

#define FIFO_RX_SIZE 32
static uint32_t fifo_rx[FIFO_RX_SIZE]={0};
static uint8_t nb_entries_fifo_rx=0;


void init_sspc(char const * const file_flash)
{
	flash_init(file_flash);
	
	status_reg=0;
	
	MSG(MSG_ALWAYS, "SSPC init ok!\n");
}

static uint32_t fifo_pop(void)
{
	if(nb_entries_fifo_rx)
	{
		uint32_t val=fifo_rx[0];
		memmove(&fifo_rx[0], &fifo_rx[1], (FIFO_RX_SIZE-1)*sizeof(uint32_t));
		nb_entries_fifo_rx--;
		return val;
	}
	else
	{
		MSG(MSG_PERIPH, "SSPC: trying to read from empty fifo, returning 0\n");
		return 0;
	}
}

static void fifo_push(const uint32_t val)
{
	if(nb_entries_fifo_rx>=FIFO_RX_SIZE)
		(void)fifo_pop();
	
	fifo_rx[nb_entries_fifo_rx++]=val;
}

static void fifo_clear(void)
{
	nb_entries_fifo_rx=0;
}

void sspc_write(PERIPH_CB_WRITE_ARGUMENTS)
{
	(void)sz;
	
	MSG(MSG_PERIPH, "SSPC: write 0x%x to reg 0x%x\n", val, addr);
	
	switch(addr)
	{
		case SSP_CTRL_REG0:
			break;
		case SSP_CTRL_REG1:
			break;
		case SSP_CTRL_REG2:
			if(val&SSP_RX_FIFO_CLR)
			{
				MSG(MSG_PERIPH, "SSPC: RX FIFO CLR\n");
				fifo_clear();
			}
			if(val&SSP_TX_FIFO_CLR)
			{
				MSG(MSG_PERIPH, "SSPC: TX FIFO CLR\n");
			}
			break;
		
		case SSP_DATA_REG:
			fifo_push(flash_spi_transfer(val));
			break;
		
		default:
			MSG(MSG_PERIPH, "SSPC: unhandled register write 0x%x @0x%x\n", val, addr);
			break;
	}
	
	
}

bool sspc_read(PERIPH_CB_READ_ARGUMENTS)
{
	(void)sz;
	
	MSG(MSG_PERIPH, "SSPC: read from 0x%x == ", addr);
	
	if(addr==SSP_DATA_REG)
	{
		(*val)=fifo_pop();
		MSG(MSG_PERIPH, "data reg: %x\n", (*val));
		return true;
	}
	else if(addr==SSP_STATUS_REG)
	{
		(*val)=status_reg;
		MSG(MSG_PERIPH, "status reg: %x\n", (*val));
		return true;
	}
	else
	{
		MSG(MSG_PERIPH, "unhandled\n");
		return false;
	}
}
