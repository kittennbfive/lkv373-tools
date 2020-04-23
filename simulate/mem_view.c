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

#include "mem_view.h"
#include "window.h"
#include "ansi.h"
#include "memory.h"
#include "cmd_parser.h"
#include "parse_hex.h"
#include "simulate.h"
#include "dispatcher.h"

static int w_mem;
static uint32_t addr_start;
static uint8_t values_per_row;
static uint16_t rows, cols;

typedef enum
{
	HEX,
	ASCII,
} display_type_t;

display_type_t display_type=HEX;

int init_mem_view(void)
{
	w_mem=open_new_window("memory", NULL, NULL);
	addr_start=0;
	values_per_row=0;
	rows=0;
	cols=0;
	return w_mem;
}

#define SIZE_ADDR 12
#define SIZE_VAL 3

char mask_unprintable(const char c)
{
	if(c<0x20 || c>0x7E)
		return '.';
	else
		return c;
}

void print_mem(void)
{
	win_printf(w_mem, "%s", CLEAR_SCREEN);
	
	//                     height width
	get_window_size(w_mem, &rows, &cols);
	
	if(cols<(SIZE_ADDR+SIZE_VAL)) //too small
	{
		win_printf(w_mem, "size!");
		return;
	}
	
	values_per_row=(cols-SIZE_ADDR)/SIZE_VAL;
	uint16_t values_total=values_per_row*rows;
	
	uint32_t i=0, j;
	while(i<values_total)
	{
		win_printf(w_mem, "0x%08x: ", addr_start+i);
		j=0;
		while(j<values_per_row && i+j<values_total)
		{
			mem_byte_t val=memory_get_byte(addr_start+i+j, NULL);
			if(val.is_initialized)
			{
				if(display_type==HEX)
					win_printf(w_mem, "%02x ", val.val);
				else
					win_printf(w_mem, "%c  ", mask_unprintable(val.val));
			}
			else
			{
				win_printf(w_mem, COLOR(GREY,"xx "));
			}
			j++;
		}
		if(i+j<values_total)
			win_printf(w_mem, "\n");
		
		i+=values_per_row;
	}
}

uint8_t mem_scroll_up(const scroll_amount_t a)
{
	if(a==LINE)
	{
		if(addr_start>=values_per_row)
		{
			addr_start-=values_per_row;
			return 1;
		}
	}
	else
	{
		if(addr_start>=values_per_row*rows)
		{
			addr_start-=values_per_row*rows;
			return 1;
		}
	}
	
	return 0;
}

uint8_t mem_scroll_down(const scroll_amount_t a)
{
	if(a==LINE)
		addr_start+=values_per_row;
	else
		addr_start+=values_per_row*rows;
	
	return 1;
}

void mem_show_addr(PROTOTYPE_ARGS_HANDLER)
{
	(void)cmd;
	(void)nb_args;
	
	char * arg=get_next_argument();
	
	if(arg[0]=='r')
	{
		uint8_t reg;
		if(parse_register(&arg[1], &reg))
			return;
		
		addr_start=get_register(reg);
	}
	else
	{
		uint32_t addr;
		if(parse_hex(arg, &addr))
			return;

		addr_start=addr;
	}
	
	print_mem();
}

void switch_ascii_hex(void)
{
	if(display_type==HEX)
		display_type=ASCII;
	else
		display_type=HEX;
}

uint8_t mem_keypress(char c)
{
	if(c=='t')
	{
		switch_ascii_hex();
		return 1;
	}
	
	return 0;
}
