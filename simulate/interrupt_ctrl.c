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
#include <stdbool.h>

#include "interrupt_ctrl.h"
#include "peripherals.h"
#include "cmd_parser.h"
#include "simulate.h"
#include "special_regs.h"
#include "timer.h"
#include "verbosity.h"
#include "uart.h"
#include "connector_serial.h"

//FIQ: Fast interrupt request

#define IRQ_SOURCE_REG 0x98800000
#define IRQ_MASK_REG 0x98800004
#define IRQ_MASK_TMR1 (1<<19) //0x80000

#define IRQ_INT_CLEAR_REG 0x98800008
#define IRQ_TRIGGER_MODE_REG 0x9880000C
#define IRQ_TRIGGER_LEVEL_REG 0x98800010
#define IRQ_STATUS_REG 0x98800014
#define FIQ_SOURCE_REG 0x98800020
#define FIQ_MASK_REG 0x98800024
#define FIQ_INT_CLEAR_REG 0x98800028
#define FIQ_TRIGGER_MODE_REG 0x9880002C
#define FIQ_TRIGGER_LEVEL_REG 0x98800030
#define FIQ_STATUS_REG 0x98800034
#define INTC_REV_NB_REG 0x98800050
#define INTC_FEATURE_REG_FOR_INPUT_NB_REG 0x98800054
#define IRQ_FEATURE_DEBOUNCE_REG 0x98800058
#define FIQ_FEATURE_DEBOUNCE_REG 0x9880005C

void init_intc(void)
{
	MSG(MSG_ALWAYS, "warning: INTC implementation is incomplete!\n");
}

static uint32_t trigger_level_reg=0;
static uint32_t trigger_mode_reg=0;
static uint32_t int_clear_reg=0;
static uint32_t int_mask_reg=0;
static uint32_t fiq_mask_reg=0;
static uint32_t fiq_int_clear_reg=0;

void intc_write(PERIPH_CB_WRITE_ARGUMENTS)
{
	(void)sz;
	
	MSG(MSG_PERIPH, "INTC: writing 0x%x to 0x%x\n", val, addr);
	
	switch(addr)
	{
		case IRQ_MASK_REG:
			int_mask_reg=val;
			//if(int_mask_reg&IRQ_MASK_TMR1)
			//	printf("INT for TIMER1 set\n");
			break;
		
		case IRQ_TRIGGER_LEVEL_REG:
			trigger_level_reg=val;
			break;
		
		case IRQ_TRIGGER_MODE_REG:
			trigger_mode_reg=val;
			break;
		
		case IRQ_INT_CLEAR_REG:
			//printf("INTC: %x written to IRQ_INT_CLEAR_REG\n", val);
			int_clear_reg=val;
			if(int_clear_reg&IRQ_MASK_TMR1)
				timer_clear_irq();
			break;
		
		case FIQ_MASK_REG:
			fiq_mask_reg=val;
			break;
		
		case FIQ_INT_CLEAR_REG:
			fiq_int_clear_reg=val;
			break;
		
		default:
			MSG(MSG_PERIPH, "INTC: unhandled register write 0x%x @0x%x\n", val, addr);
	}
}

bool intc_read(PERIPH_CB_READ_ARGUMENTS)
{
	(void)sz;
	
	MSG(MSG_PERIPH, "INTC: reading from %x\n", addr);
	
	switch(addr)
	{
		case IRQ_STATUS_REG:	
			(*val)=IRQ_MASK_TMR1;
			return true;
			break;
		
		case IRQ_TRIGGER_LEVEL_REG:
			(*val)=trigger_level_reg;
			return true;
			break;
		
		case IRQ_TRIGGER_MODE_REG:
			(*val)=trigger_mode_reg;
			return true;
			break;
		
		case IRQ_INT_CLEAR_REG:
			(*val)=int_clear_reg;
			return true;
			break;
		
		case IRQ_MASK_REG:
			(*val)=int_mask_reg;
			return true;
			break;
		
		default:
			MSG(MSG_PERIPH, "INTC: unhandled read from 0x%x\n", addr);
			return false;
	}
}

//hardcoded for now
#define INT_VECT_TABLE_ADDR 0x210000
#define INT_VECT_TABLE_BYTES_PER_ENTRY 16

static void trigger_interrupt(const uint8_t nb_int)
{
	if(get_gie()==false)
	{
		MSG(MSG_PERIPH, "trigger_interrupt: GIE is 0, not executing\n");
		return;
	}
	
	uint32_t offset=nb_int*INT_VECT_TABLE_BYTES_PER_ENTRY;
	
	//printf("triggering interrupt %d, offset in table is 0x%x\n", nb_int, offset);
	
	write_to_special_reg(SR_INT_PEND, 2); //this is only true for INT 10
		
	write_to_special_reg(SR_PROC_INT_STATUS_WORD, read_from_special_reg(SR_PROC_STATUS_WORD));
	write_to_special_reg(SR_INT_PC, get_PC());
	//printf("INT: PC %x saved\n", get_PC());

	//disable GIE on ISR
	special_reg_setgie(0);
	
	set_PC(INT_VECT_TABLE_ADDR+offset);
}

bool check_for_pending_irq(void)
{
	if(!get_gie())
		return false;
		
	if(timer_is_irq_pending())
	{
		trigger_interrupt(10);
		return true;
	}
	else
		return false;
}
