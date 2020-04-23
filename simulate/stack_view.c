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

#include "stack_view.h"
#include "window.h"
#include "ansi.h"
#include "memory.h"
#include "simulate.h"
#include "cmd_parser.h"
#include "parse_hex.h"

static int w_stack;
static uint32_t addr_start;
static uint8_t values_per_row;
static 	uint16_t rows, cols;

int init_stack_view(void)
{
	w_stack=open_new_window("stack", NULL, NULL);
	addr_start=get_stack_pointer();
	values_per_row=0;
	rows=0;
	cols=0;
	return w_stack;
}

#define SIZE_ADDR 12
#define SIZE_VAL 3
#define OFFSET_SP 16 //show n bytes before current location of SP

void print_stack(void)
{
	win_printf(w_stack, CLEAR_SCREEN);
	
	//                       height width
	get_window_size(w_stack, &rows, &cols);
	
	if(cols<(SIZE_ADDR+SIZE_VAL)) //too small
	{
		win_printf(w_stack, "size!");
		return;
	}
	
	values_per_row=(cols-SIZE_ADDR)/SIZE_VAL;
	uint16_t values_total=values_per_row*rows;
	
	uint32_t i=0, j;
	uint32_t SP=get_stack_pointer();
	while(i<values_total)
	{
		win_printf(w_stack, "0x%08x: ", addr_start+i);
		j=0;
		while(j<values_per_row && i+j<values_total)
		{
			mem_byte_t val=memory_get_byte(addr_start+i+j, NULL);
			if(val.is_initialized)
			{
				if(addr_start+i+j==SP)
					win_printf(w_stack, COLOR(RED,"%02X "), val.val);
				else
					win_printf(w_stack, "%02x ", val.val);
			}
			else
			{
				if(addr_start+i+j==SP)
					win_printf(w_stack, COLOR(RED,"XX "));
				else
					win_printf(w_stack, COLOR(GREY,"xx "));	
			}
			j++;
		}
		if(i+j<values_total)
			win_printf(w_stack, "\n");
		
		i+=values_per_row;
	}
}


uint8_t stack_scroll_up(const scroll_amount_t a)
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

uint8_t stack_scroll_down(const scroll_amount_t a)
{
	if(a==LINE)
		addr_start+=values_per_row;
	else
		addr_start+=values_per_row*rows;
	
	return 1;
}

void stack_reset_view(void)
{
	addr_start=get_stack_pointer();
}

uint8_t stack_keypress(char c)
{
	if(c=='r')
	{
		addr_start=get_stack_pointer();
		return 1;
	}
	
	return 0;
}

void stack_show_addr(PROTOTYPE_ARGS_HANDLER)
{
	(void)cmd;
	
	if(nb_args==0)
	{
		stack_reset_view();
		return;
	}
	
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

	print_stack();
}
