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

#include "mac.h"
#include "peripherals.h"
#include "verbosity.h"
#include "connector_serial.h"

//This does not do anything useful at the moment except returning some hardcoded values with unknown meaning. At least it permits to avoid an error message in the serial output.

void mac_write(PERIPH_CB_WRITE_ARGUMENTS)
{
	//(void)val; (void)addr;
#ifdef CONNECT_TO_REAL
	con_setval(sz, addr, val);
#else
	(void)sz;
	
	MSG(MSG_ALWAYS, "MAC: write 0x%x @0x%x\n", val, addr);
#endif
}

bool mac_read(PERIPH_CB_READ_ARGUMENTS)
{
#ifdef CONNECT_TO_REAL
	(*val)=con_getval(sz, addr);
	return true;
#else
	(void)sz;
	
	if(addr!=0x9090700c)
		MSG(MSG_ALWAYS, "MAC: read from 0x%x\n", addr);
	
	if(addr>=0x909003f0 && addr<0x90900500)
	{
		(*val)=0xdeadbeef;
		return true;
	}
	
	switch(addr)
	{
		case 0x90907070:
			(*val)=0x0000009E;
			return true;
			break;
			
		case 0x90907074:
			(*val)=0x00000092;
			return true;
			break;
			
		case 0x90907078:
			(*val)=0x0000001F;
			return true;
			break;
			
		case 0x9090707C:
			(*val)=0;
			return true;
			break;
			
		case 0x90907200:
			(*val)=0x00000001;
			return true;
			break;
			
		case 0x90907204:
			(*val)=0x00000800;
			return true;
			break;
		
		case 0x9090700c:
			(*val)=0;
			return true;
			break;
		
		//VALUE??
		case 0x90907010:
			(*val)=0;
			return true;
			break;
	}
	
	return false;
#endif
}

