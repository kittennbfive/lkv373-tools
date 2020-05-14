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

#include "power_management.h"
#include "peripherals.h"
#include "memory.h"
#include "verbosity.h"

#define OSC_CTRL_REG 0x98100008

#define UNKNOWN_REG_2 0x98100020

#define MULTIFUNC_PORT_SETTING_REG 0x98100028

#define PLL_DLL_CTRL_REG0 0x98100030

#define UNKNOWN_REG 0x981000dc

//TODO: INIT VALUES??
static uint32_t osc_ctrl=0;
static uint32_t unknown2=0;
static uint32_t multifunc_port_set=0;
static uint32_t pll_dll_ctrl=0;
static uint32_t unknown1=0;

void init_pmu(void)
{
	MSG(MSG_ALWAYS, "warning: PMU config is completely ignored!\n");
}

void pmu_write(PERIPH_CB_WRITE_ARGUMENTS)
{
	(void)sz;
	
	switch(addr)
	{
		case OSC_CTRL_REG:
			osc_ctrl=val;
			break;
		
		case UNKNOWN_REG_2:
			unknown2=val;
			break;
		
		case MULTIFUNC_PORT_SETTING_REG:
			multifunc_port_set=val;
			break;
		
		case PLL_DLL_CTRL_REG0:
			pll_dll_ctrl=val;
			break;
		
		case UNKNOWN_REG:
			unknown1=val;
			break;
			
		default:
			MSG(MSG_PERIPH, "PMU: unhandled register write 0x%x @0x%x\n", val, addr);
	}
}

bool pmu_read(PERIPH_CB_READ_ARGUMENTS)
{
	(void)sz;
	
	switch(addr)
	{
		case OSC_CTRL_REG:
			(*val)=osc_ctrl;
			return true;
			break;
		
		case UNKNOWN_REG_2:
			(*val)=unknown2;
			return true;
			break;
		
		case MULTIFUNC_PORT_SETTING_REG:
			(*val)=multifunc_port_set;
			return true;
			break;
		
		case PLL_DLL_CTRL_REG0:
			(*val)=pll_dll_ctrl;
			return true;
			break;
		
		case UNKNOWN_REG:
			(*val)=unknown1;
			return true;
			break;
	
		default:
			MSG(MSG_PERIPH, "PMU: unhandled register read 0x%x\n", addr);
			return false;
	}
}
