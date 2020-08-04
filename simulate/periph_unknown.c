#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "periph_unknown.h"
#include "peripherals.h"
#include "verbosity.h"
#include "my_err.h"

typedef struct
{
	uint32_t addr;
	uint32_t val;
} mem_t;

#define MAX_MEM 500

static mem_t mem[MAX_MEM];
static uint32_t nb_vals=0;


static int32_t search(const uint32_t addr)
{
	uint32_t i;
	for(i=0; i<nb_vals; i++)
	{
		if(mem[i].addr==addr)
			return i;
	}
	
	return -1;
}

static void save(const uint32_t addr, const uint32_t val)
{
	int32_t pos=search(addr);
	if(pos==-1)
	{
		if(nb_vals>=MAX_MEM)
			ERRX(1, "full");
		pos=nb_vals++;
	}
	mem[pos].addr=addr;
	mem[pos].val=val;
}

void unknown_write(PERIPH_CB_WRITE_ARGUMENTS)
{
	(void)sz;
	
	MSG(MSG_PERIPH_UNKNOWN, "UNKNOWN: write 0x%x to 0x%x\n", val, addr);

	save(addr, val);
}


bool unknown_read(PERIPH_CB_READ_ARGUMENTS)
{
	(void)sz; (void)val;
	
	MSG(MSG_PERIPH_UNKNOWN, "UNKNOWN: trying to read from 0x%x\n", addr);
	
	int32_t pos=search(addr);
	if(pos==-1)
		return false;
	
	(*val)=mem[pos].val;
	return true;
}

void unknown_init(void)
{
	save(0x9090a800, 0x191a);
	save(0x9090a80c, 0xc2);
	save(0x9090c408, 0);
	save(0x9090c40c, 0);
	save(0x9090b050, 0);
	save(0x90909600, 0);
	
	printf("init unknown done\n");
}
