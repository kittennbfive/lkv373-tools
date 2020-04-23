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
#include <string.h>
#include <err.h>

#include "endless_loop_detect.h"
#include "simulate.h"
#include "my_err.h"

static history_t * history;
static uint32_t history_nb_entries;

static void history_cleanup(void)
{
	free(history);
}

void history_init(void)
{
	history=malloc(HISTORY_MAX_ENTRIES*sizeof(history_t));
	if(!history)
		ERR(1,"init_sim: malloc for history failed\n");
	
	history_nb_entries=0;
	atexit(&history_cleanup);
}

void history_add(const uint32_t PC_old, uint32_t const * const regs)
{
	memmove(&history[1], &history[0], (HISTORY_MAX_ENTRIES-1)*sizeof(history_t));
	
	history[0].PC=PC_old;
	memcpy(&history[0].regs, regs, 32*sizeof(uint32_t));
		
	if(history_nb_entries<HISTORY_MAX_ENTRIES)
		history_nb_entries++;
}

static uint8_t entries_are_equal(const uint32_t b1_start, const uint32_t b1_end, const uint32_t b2_start, const uint32_t b2_end)
{
	if(b1_end-b1_start!=b2_end-b2_start)
		ERRX(1, "cmp_entries: err: blocks of different size\n");
	
	uint32_t i,j;
	for(i=b1_start, j=b2_start; i<=b1_end && j<=b2_end; i++, j++)
	{
		if(memcmp(&history[i], &history[j], sizeof(history_t)))
			return 0;
	}
	
	return 1;
}

uint8_t history_check(void)
{
	uint32_t i=1;
	
	while(memcmp(&history[i], &history[0], sizeof(history_t)) && i<history_nb_entries/2)
		i++;
	
	if(entries_are_equal(0, i-1, i, 2*i-1))
		return 1;
	
	return 0;
}
