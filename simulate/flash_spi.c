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

#include "flash_spi.h"
#include "my_err.h"
#include "verbosity.h"

uint8_t * flash;

void flash_cleanup(void)
{
	free(flash);
}

void flash_init(char const * const file)
{
	flash=malloc(FLASH_SIZE);
	if(!flash)
		ERR(1, "malloc failed");
	FILE *f=fopen(file, "rb");
	if(!f)
		ERR(1, "could not open %s", file);
	fread(flash, FLASH_SIZE, 1, f);
	fclose(f);
	
	atexit(flash_cleanup);
}

static uint32_t addr=0;
static uint8_t status_reg=0;

#define WRITE_ENABLE_LATCH 1

uint32_t flash_spi_transfer(const uint32_t val)
{
	if(((val>>24)&0xFF)==0x03)
	{
		addr=val&0x00FFFFFF;		
		MSG(MSG_PERIPH_SSPC, "FLASH set addr to 0x%x\n", addr);
		return 0;
	}
	else if(val==0x06)
	{
		MSG(MSG_PERIPH_SSPC, "FLASH: write enable bit set\n");
		status_reg|=(1<<WRITE_ENABLE_LATCH);
		return 0;
	}
	else if(((val>>24)&0xFF)==0x05)
	{
		MSG(MSG_PERIPH_SSPC, "FLASH: read status reg\n");
		return status_reg;
	}
	else if(((val>>24)&0xFF)==0x20)
	{
		addr=val&0x00FFFFFF;		
		MSG(MSG_PERIPH_SSPC, "FLASH sector erase 0x%x (not impl)\n", addr);
		status_reg&=~(1<<WRITE_ENABLE_LATCH);
		return 0;
	}
	else if(val==0)
	{
		uint32_t r=(flash[addr+0]<<24)|(flash[addr+1]<<16)|(flash[addr+2]<<8)|(flash[addr+3]<<0);
		addr+=4;
		return r;
	}
	else
	{
		MSG(MSG_PERIPH_SSPC, "flash_spi_transfer: don't know how to handle received value 0x%08x\n", val);
		return 0;
	}	
}
