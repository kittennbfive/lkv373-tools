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
#include <stdbool.h>

#include "instruction.h"
#include "decode_disassm.h"

#include "disassembly_view.h"
#include "window.h"
#include "ansi.h"
#include "simulate.h"
#include "cmd_parser.h"
#include "dispatcher.h"
#include "my_err.h"
#include "verbosity.h"

//TODO this needs work!


#define ADDR_LIST_BLOCKSIZE 5000

static int w_disassembly;
static uint32_t addr_start;
static uint32_t *addr_list;
static uint32_t index_addr_list_nb_blocks;
static uint32_t index_addr_list_max;
static uint32_t index_addr_list;

static void push_addr(const uint32_t addr)
{
	if(index_addr_list>=index_addr_list_nb_blocks*ADDR_LIST_BLOCKSIZE || !addr_list)
	{
		index_addr_list_nb_blocks++;
		addr_list=realloc(addr_list, index_addr_list_nb_blocks*ADDR_LIST_BLOCKSIZE*sizeof(uint32_t));
		if(!addr_list)
			ERR(1, "realloc failed\n");
	}
	addr_list[index_addr_list++]=addr;
	index_addr_list_max++;
}

static void disassembly_cleanup(void)
{
	free(addr_list);
}

int init_disassembly_view(void)
{
	w_disassembly=open_new_window("disassembly", NULL, NULL);
	
	addr_start=0;

	index_addr_list_nb_blocks=0;
	index_addr_list_max=0;
	index_addr_list=0;
	uint32_t addr=addr_start;
	instr_t instr;
	while(decode_instr(&instr, addr, false)==0)
	{
		push_addr(addr);
		if(instr.was_16bit_instr)
			addr+=2;
		else
			addr+=4;
	}
	MSG(MSG_ALWAYS, "%d instructions decoded\n", index_addr_list_max);
	index_addr_list=0;
	
	atexit(&disassembly_cleanup);
	
	return w_disassembly;
}

void print_disassembly(void)
{
	uint8_t i;
	uint32_t addr=addr_start;
	instr_t instr;
	
	uint16_t rows;
	get_window_size(w_disassembly, &rows, NULL);
	
	win_printf(w_disassembly, CLEAR_SCREEN);
	win_printf(w_disassembly, HIDE_CURSOR);	
	
	uint32_t PC=get_PC();
	
	for(i=0; i<rows; i++)
	{
		if(decode_instr(&instr, addr, false))
		{
			win_printf(w_disassembly, COLOR(RED,"(error in decode_instr, invalid addr?)"));
			break;
		}
		
		if(addr==PC)
			win_printf(w_disassembly, COLOR(YELLOW, "%8x: %s"), addr, instr.disassm);
		else
			win_printf(w_disassembly, "%8x: %s", addr, instr.disassm);
		if(i<rows-1)
			win_printf(w_disassembly, "\n");
		
		if(instr.was_16bit_instr)
			addr+=2;
		else
			addr+=4;
	}
}

uint8_t disassm_scroll_up(const scroll_amount_t a)
{
	(void)a;
	
	if(index_addr_list)
	{
		addr_start=addr_list[--index_addr_list];
	}
			
	return 1;
}

uint8_t disassm_scroll_down(const scroll_amount_t a)
{
	(void)a;
	
	if(index_addr_list<index_addr_list_max)
		addr_start=addr_list[index_addr_list++];
	
	return 1;
}

void disassm_reset(void)
{
	addr_start=get_PC();
	
	while(index_addr_list && addr_list[index_addr_list]>addr_start)
		index_addr_list--;
	while(index_addr_list<index_addr_list_max && addr_list[index_addr_list]<addr_start)
		index_addr_list++;
}

void disassm_reset_view(PROTOTYPE_ARGS_HANDLER)
{
	ARGS_HANDLER_UNUSED;
	disassm_reset();
	print_disassembly();
}

uint8_t disassm_keypress(char c)
{
	if(c=='r')
	{
		disassm_reset();
		return 1;
	}
	
	return 0;
}
