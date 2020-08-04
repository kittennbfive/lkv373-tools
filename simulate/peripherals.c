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
#include "periph_unknown.h"
#include "i2c.h"

#include "connector_serial.h"

//if compiled with -DCONNECT_TO_REAL: If a peripheral is NOT in this list the real one on the LKV will be used. Does not work for CPE/PMU/TMR1/INTC and of course UART because that's used for the connector
static peripheral_t per[]=
{
	{"CPE", 0x90100000, 0x90100088, &cpe_write, &cpe_read}, //what does this mean "CPE"?? it's important, there is the remap bit inside!!
	{"PMU", 0x98100000, 0x981000dc, &pmu_write, &pmu_read}, //power management unit
	{"TMR1", 0x98400000, 0x98400038, &timer_write, &timer_read}, //timer1
	{"INTC", 0x98800000, 0x98810000, &intc_write, &intc_read}, //interrupt controller
	{"UART", 0x99600000, 0x9960001c, &uart_write, &uart_read},
	{"SSPC", 0x98b00000, 0x98b00020, &sspc_write, &sspc_read}, //synchronous serial port controller - SPI connected to FLASH
	{"GPIO", 0x99300000, 0x99300044, &gpio_write, &gpio_read}, //LED and reset button
	{"MAC", 0x90900000, 0x90907118, &mac_write, &mac_read}, //ethernet - UPPER ADDR????
	{"I2C", 0x99c00000, 0x99c00014, &i2c_write, &i2c_read}, //I2C interface to IT6604


	{"UNK", 0x90907120, 0x9090ffff, &unknown_write, &unknown_read },
	
//???? 0x90908050-0x9090807c
//???? 0x90909008
//???? 0x9090a80c-0x9090a8fc
//???? 0x9090b044-0x9090b054
//???? 0x9090d02c
//IR 0x99900000-... ???
//0x98500000 Watchdog Timer

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
	{
#ifdef CONNECT_TO_REAL
		if(addr>0x20000000)
		{
			con_setval(sz, addr, val);
			return true;
		}
		else
			return false;
#else
		return false;
#endif
	}
	
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
	{
#ifdef CONNECT_TO_REAL
		if(addr>0x20000000)
		{
			(*val)=con_getval(sz, addr);
			return true;
		}
		else
			return false;
#else
		return false;
#endif
	}
	
	if(!per[i].cb_read)
		return false;
	
	return per[i].cb_read(sz, addr, val);
}
