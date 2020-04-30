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

#include "timer.h"
#include "peripherals.h"
#include "memory.h"

#define COUNTER_REG 0x98400000
#define AUTO_RELOAD_VAL_REG 0x98400004
#define MATCH1_VAL_REG 0x98400008
#define MATCH2_VAL_REG 0x9840000C
#define TIMER_CTRL_REG 0x98400030
#define T1_OFENABLE	(1<<2) //what's this?
#define T1_CLOCK (1<<1)
#define T1_ENABLE (1<<0)

#define TIMER_INT_STATE_REG 0x98400034
#define TIMER_INT_MASK_REG 0x98400038

static uint32_t cnt_reg=0;
static uint32_t auto_reload_val_reg=0;
static uint32_t match1_val_reg=0; 
static uint32_t ctrl_reg=0; //init value??
static uint32_t int_state_reg=0; //init value??
static uint32_t int_mask_reg=0; //init value??

static bool is_irq_pending=false; //do it this way because we don't know the details about the specific register

void init_timer(void)
{
	printf("warning: TIMER1 is only partially implemented!\n");
}

void timer_write(PERIPH_CB_WRITE_ARGUMENTS)
{
	switch(addr)
	{
		case COUNTER_REG:
			printf("TIMER1: value 0x%x written to cnt_reg\n", val);
			cnt_reg=val;
			break;
		
		case AUTO_RELOAD_VAL_REG:
			printf("TIMER1: 0x%x written to auto_reload_val_reg\n", val);
			auto_reload_val_reg=val;
			break;
		
		case MATCH1_VAL_REG:
			printf("TIMER1: 0x%x written to match1_reg\n", val);
			match1_val_reg=val;
			break;
		
		case TIMER_CTRL_REG:
			ctrl_reg=val;
			if(ctrl_reg&T1_ENABLE)
			{
				printf("TIMER1: ENABLED\n");
				//is_irq_pending=true; //TEST
			}
			else
				printf("TIMER1: DISABLED\n");
			break;
		
		case TIMER_INT_STATE_REG:
			int_state_reg=val;
			if(val==0)
				is_irq_pending=false;
			break;
		
		case TIMER_INT_MASK_REG:
			int_mask_reg=val;
			break;
		
		default:
			printf("TIMER1: unhandled register write 0x%x @0x%x\n", val, addr);
	}
}

bool timer_read(PERIPH_CB_READ_ARGUMENTS)
{
	(void)val;
	printf("TIMER1: unhandled register read 0x%x\n", addr);
	return false;
}

static uint8_t prescaler=2; //IS THIS CORRECT??

bool timer_tick(void)
{
	if(!(ctrl_reg&T1_ENABLE))
	{
		return false;
	}
	
	if(--prescaler)
		return false;
	
	prescaler=2;	
	
	cnt_reg--;
	
	//printf("cnt_reg is %x\n", cnt_reg);
	
	if(cnt_reg==match1_val_reg)
	{
		cnt_reg=auto_reload_val_reg;
		printf("TIMER1 IRQ\n");
		is_irq_pending=true;
		return true;
	}
	
	return false;
}

bool timer_is_irq_pending(void)
{
	return is_irq_pending;
}

void timer_clear_irq(void)
{
	is_irq_pending=false;
}
