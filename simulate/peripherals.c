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

#include "peripherals.h"
#include "my_err.h"
#include "memory.h"
#include "uart.h"
#include "window.h"
#include "dispatcher.h"
#include "simulate.h"
#include "power_management.h"
#include "interrupt_ctrl.h"
#include "timer.h"
#include "cpe.h"
#include "sspc.h"
#include "mac.h"
#include "gpio.h"

static peripheral_t per[]=
{
	{"CPE", 0x90100000, 0x90100088, &cpe_write, &cpe_read}, //what does this mean "CPE"?? it's important, there is the remap bit inside!!
	{"PMU", 0x98100000, 0x981000dc, &pmu_write, &pmu_read}, //power management unit
	{"TMR1", 0x98400000, 0x98400038, &timer_write, &timer_read}, //timer1
	{"INTC", 0x98800000, 0x98810000, &intc_write, &intc_read}, //interrupt controller
	{"UART", 0x99600000, 0x9960001c, &uart_write, &uart_read},
	{"SSPC", 0x98b00000, 0x98c00000, &sspc_write, &sspc_read}, //synchronous serial port controller - SPI connected to FLASH - wider range to catch unknown registers
	{"GPIO", 0x99300000, 0x99300044, &gpio_write, &gpio_read}, //LED and reset button
	
	{"MAC", 0x90900000, 0x90907fff, &mac_write, &mac_read}, //TODO
	
	{NULL, 0, 0, NULL, NULL}
};

bool peripheral_write(const sz_mem_access_t sz, const uint32_t addr, const uint32_t val)
{	
	uint8_t i;
	bool found=false;
	for(i=0; per[i].name; i++)
	{
		if(addr>=per[i].addr_start && addr<=per[i].addr_end)
		{
			found=true;
			break;
		}
	}
	
	if(!found)
		return false;
	
	//printf("periph_write: %x @%x == %s\n", val, addr, per[i].name);

	//if(addr%4)
	//	printf("periph_write: UNALIGNED ADDR %x", addr);
		
	if(!per[i].cb_write)
		ERRX(1, "no callback for peripherals[%u]", i);
	
	per[i].cb_write(sz, addr, val);
	
	return true;
}

bool peripheral_read(const sz_mem_access_t sz, const uint32_t addr, uint32_t * const val)
{
		uint8_t i;
	bool found=false;
	for(i=0; per[i].name; i++)
	{
		if(addr>=per[i].addr_start && addr<=per[i].addr_end)
		{
			found=true;
			break;
		}
	}
	
	if(!found)
		return false;
	
	//if(addr%4)
	//	printf("periph_read: UNALIGNED ADDR %x", addr);
		
	if(!per[i].cb_read)
		return false;
	
	return per[i].cb_read(sz, addr, val);
}
