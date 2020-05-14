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

#include "cpe.h"
#include "peripherals.h"
#include "verbosity.h"
    
#define CPE_AHB_SLAVE0_REG 0x90100000
#define CPE_AHB_SLAVE1_REG 0x90100004
#define CPE_AHB_SLAVE2_REG 0x90100008
#define CPE_AHB_SLAVE3_REG 0x9010000c
#define CPE_AHB_SLAVE4_REG 0x90100010
#define CPE_AHB_SLAVE5_REG 0x90100014
#define CPE_AHB_SLAVE6_REG 0x90100018
#define CPE_AHB_SLAVE7_REG 0x9010001c
#define CPE_AHB_SLAVE8_REG 0x90100020
#define CPE_AHB_SLAVE9_REG 0x90100024
#define CPE_AHB_SLAVE10_REG 0x9010028
#define CPE_PRIORITY_REG 0x90100080
#define CPE_DEFAULT_MASTER_REG 0x90100084
#define CPE_REMAP_REG 0x90100088
#define CPE_REMAP_BIT (1<<0)

#define REMAP_OFFSET 0x10000000 //hardcode this for now

static uint32_t remap_reg=0;

void init_cpe(void)
{
	MSG(MSG_ALWAYS, "warning: most of CPE config/writes is/are ignored!\n");
}

void cpe_write(PERIPH_CB_WRITE_ARGUMENTS)
{
	(void)sz;
	
	switch(addr)
	{
		case CPE_REMAP_REG:
			remap_reg=val;
			break;
		
		default:
			MSG(MSG_PERIPH, "CPE: unhandled register write 0x%x @0x%x\n", val, addr);
	}
}

bool cpe_read(PERIPH_CB_READ_ARGUMENTS)
{
	(void)sz;
	
	switch(addr)
	{
		case CPE_REMAP_REG:
			(*val)=remap_reg;
			return true;
			break;
		
		default:
			MSG(MSG_PERIPH, "CPE: unhandled register read 0x%x\n", addr);
			return false;
	}
}

uint32_t remap_addr(const uint32_t addr) //exchange base addr of ROM/FLASH and SDRAM
{
	if(!(remap_reg&CPE_REMAP_BIT))
		return addr;
	
	if(addr<REMAP_OFFSET)
		return addr+REMAP_OFFSET;
	else
		return addr;
}


