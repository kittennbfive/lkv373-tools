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

#include "dispatcher.h"
#include "disassembly_view.h"
#include "reg_view.h"
#include "mem_view.h"
#include "stack_view.h"
#include "my_err.h"
#include "cmd_parser.h"
#include "breakpoints.h"
#include "uart.h"

const dispatch_data_t dispatch_data[]=
{
	{W_ASM, &print_disassembly, &disassm_scroll_up, &disassm_scroll_down, &disassm_keypress},
	{W_REG, &print_registers, NULL, NULL, NULL},
	{W_MEM, &print_mem, &mem_scroll_up, &mem_scroll_down, &mem_keypress},
	{W_STACK, &print_stack, &stack_scroll_up, &stack_scroll_down, &stack_keypress},
	{W_BP, &print_breakpoints, NULL, NULL, NULL},
	{W_UART, NULL, NULL, NULL, &uart_keypress},
	
	{W_EMPTY, NULL, NULL, NULL, NULL}
};

void dispatcher(const window_t win, const int fd_win)
{
	uint8_t i;
	uint8_t found=0;
	for(i=0; dispatch_data[i].win!=W_EMPTY; i++)
	{
		if(dispatch_data[i].win==win)
		{
			found=1;
			break;
		}
	}
	if(!found)
		ERRX(1, "win not found in dispatch_data[]");
		
	uint8_t events[SZ_EVENT_BUFFER];
	uint32_t nb_ev;
	uint32_t j;
	nb_ev=read_input(fd_win, events, SZ_EVENT_BUFFER);
	for(j=0; j<nb_ev; j++)
	{
		switch(events[j])
		{
			case EV_SCROLL_UP:
				if(dispatch_data[i].func_scroll_up)
				{
					if(dispatch_data[i].func_scroll_up(LINE))
						dispatch_data[i].func_redraw();
				}
				break;
			
			case EV_SCROLL_DOWN:
				if(dispatch_data[i].func_scroll_down)
				{
					if(dispatch_data[i].func_scroll_down(LINE))
						dispatch_data[i].func_redraw();
				}
				break;
			
			case EV_PAGE_UP:
				if(dispatch_data[i].func_scroll_up)
				{
					if(dispatch_data[i].func_scroll_up(PAGE))
						dispatch_data[i].func_redraw();
				}
				break;
			
			case EV_PAGE_DOWN:
				if(dispatch_data[i].func_scroll_down)
				{
					if(dispatch_data[i].func_scroll_down(PAGE))
						dispatch_data[i].func_redraw();
				}
				break;
			
			default:
				if(dispatch_data[i].func_keypress)
				{
					if(dispatch_data[i].func_keypress(events[j]))
						dispatch_data[i].func_redraw();
				}
		}
	}
}

void redraw(const window_t win)
{
	uint8_t i;
	uint8_t found=0;
	for(i=0; dispatch_data[i].win!=W_EMPTY; i++)
	{
		if(dispatch_data[i].win==win)
		{
			found=1;
			break;
		}
	}
	if(!found)
		ERRX(1, "win not found in dispatch_data[]");
	
	if(dispatch_data[i].func_redraw)
		dispatch_data[i].func_redraw();
}

void refresh(PROTOTYPE_ARGS_HANDLER)
{
	ARGS_HANDLER_UNUSED;
	redraw_all();
}

void redraw_all(void)
{
	uint8_t i;
	for(i=0; dispatch_data[i].win; i++)
	{
		if(dispatch_data[i].func_redraw)
			dispatch_data[i].func_redraw();
	}
}
