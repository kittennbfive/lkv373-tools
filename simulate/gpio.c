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

#include "gpio.h"
#include "peripherals.h"
#include "verbosity.h"

#define GPIO_DATA_OUT_REG 0x99300000 //RW
#define GPIO_DATA_INP_REG 0x99300004 //R
#define GPIO_PINDIR_REG 0x99300008 //RW, 1==output
#define GPIO_DATA_SET_REG 0x99300010 //W (but is read in firmware - ??)
#define GPIO_DATA_CLR_REG 0x99300014 //W (but is read in firmware - ??)
#define GPIO_PULL_ENABLE_REG 0x99300018 //RW
#define GPIO_PULL_TYPE_REG 0x9930001C //RW
#define GPIO_INT_ENABLE_REG 0x99300020 //RW
//...

#define PIN_UNKNOWN_OUTPUT 13 //out //0x2000
#define PIN_LED 14 //out //0x4000
#define PIN_RESET_BUTTON 16 //in 0x10000

static uint32_t data=(1<<PIN_RESET_BUTTON);
static uint32_t pindir=0;

void gpio_write(PERIPH_CB_WRITE_ARGUMENTS)
{
	(void)sz;
	
	MSG(MSG_PERIPH_GPIO, "GPIO: writing 0x%x to 0x%x\n", val, addr);
	
	switch(addr)
	{
		case GPIO_DATA_OUT_REG:
			data=val;
			break;
			
		case GPIO_PINDIR_REG:
			pindir=val;
			break;
			
		case GPIO_DATA_SET_REG:
			data|=val;
			break;
			
		case GPIO_DATA_CLR_REG:
			data&=~val;
			break;
			
		default:
			MSG(MSG_PERIPH_GPIO, "GPIO: unhandled register write 0x%x @0x%x\n", val, addr);
			break;
	}
}

bool gpio_read(PERIPH_CB_READ_ARGUMENTS)
{
	(void)sz;
	
	switch(addr)
	{
		case GPIO_DATA_OUT_REG:
			(*val)=data;
			MSG(MSG_PERIPH_GPIO, "GPIO: reading from %x: 0x%x\n", addr, *val);
			return true;
			break;
		
		case GPIO_DATA_INP_REG:
			(*val)=data;
			MSG(MSG_PERIPH_GPIO, "GPIO: reading from %x: 0x%x\n", addr, *val);
			return true;
			break;
		
		case GPIO_PINDIR_REG:
			(*val)=pindir;
			MSG(MSG_PERIPH_GPIO, "GPIO: reading from %x: 0x%x\n", addr, *val);
			return true;
			break;
		
		//this is not readable according to doc
		case GPIO_DATA_CLR_REG:
			(*val)=0;
			return true;
			break;
		
		//this is not readable according to doc
		case GPIO_DATA_SET_REG:
			(*val)=0;
			return true;
			break;
		
		default:
			MSG(MSG_PERIPH_GPIO, "GPIO: unhandled read from 0x%x\n", addr);
			return false;
	}
}

bool get_LED_state(void)
{
	//LED is light if output is LOW (anode of LED connected to VCC via resistor)
	if(pindir&(1<<PIN_LED) && !(data&(1<<PIN_LED)))
		return true;
	else
		return false;
}

/*
bool get_button_state(void)
{
	return !(data&(1<<PIN_RESET_BUTTON));
}
*/
