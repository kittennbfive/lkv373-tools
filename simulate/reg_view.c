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

#include "reg_view.h"
#include "window.h"
#include "ansi.h"
#include "simulate.h"
#include "special_regs.h"

static int w_reg;

int init_reg_view(void)
{
	w_reg=open_new_window("registers", NULL, NULL);
	return w_reg;
}

//no decimal output, takes to much space. real geeks know hexa
#define SIZE_REG_VAL 16

void print_registers(void)
{
	win_printf(w_reg, CLEAR_SCREEN);
	win_printf(w_reg, HIDE_CURSOR);
	
	uint16_t cols;
	get_window_size(w_reg, NULL, &cols);
	
	uint8_t regs_per_line=cols/(SIZE_REG_VAL+1);
	
	if(regs_per_line==0) //too small
	{
		win_printf(w_reg, "[too small]");
		return;
	}
	
	uint8_t i,j;
	i=0;
	while(i<32)
	{
		j=regs_per_line;
		while(j-- && i<32)
		{
			win_printf(w_reg, "r%02u: 0x%08x%s", i, get_register(i), j?" ":"");
			i++;
		}
		if(i<32)
			win_printf(w_reg, "\n");
	}
	
	win_printf(w_reg, "\n");
	usr_t usr;
	usr=get_usr(0);
	win_printf(w_reg, "D0.H=0x%08x .L=0x%08x\n", usr.H, usr.L);
	usr=get_usr(1);
	win_printf(w_reg, "D1.H=0x%08x .L=0x%08x\n", usr.H, usr.L);

	win_printf(w_reg, "\n");	
	uint32_t PSW=read_from_special_reg(SR_PROC_STATUS_WORD);
	win_printf(w_reg, "PSW: 0x%x\n", PSW);
	win_printf(w_reg,"  GIE: %c\n", PSW&PSW_GIE?'1':'0');
	uint8_t mode=(PSW&PSW_PROC_OP_MODE)>>3;
	win_printf(w_reg, "  PROC_OP_MODE: ");
	if(mode==0)
		win_printf(w_reg, "User");
	else if(mode==1)
		win_printf(w_reg, "Superuser");
	else
		win_printf(w_reg, "(invalid)");
}
