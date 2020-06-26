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
#define __USE_MISC
#include <unistd.h>
#include <ctype.h>
       
#include "instruction.h"
#include "decode_disassm.h"
#include "main.h"
#include "simulate.h"
#include "breakpoints.h"
#include "memory.h"
#include "window.h"
#include "disassembly_view.h"
#include "reg_view.h"
#include "mem_view.h"
#include "stack_view.h"
#include "my_err.h"
#include "dispatcher.h"
#include "cmd_parser.h"
#include "breakpoints.h"
#include "uart.h"
#include "power_management.h"
#include "interrupt_ctrl.h"
#include "timer.h"
#include "cpe.h"
#include "sspc.h"
#include "mac.h"
#include "connector_serial.h"

/*
little-endian

todo: WHY ON EARTH DOES THIS THING NOT RESPOND TO NETWORK REQUESTS???

todo: disassm-view needs work

todo: mem_view for periph-mem does not work properly, only shows 1 byte of 4

todo: RE MAC

todo: add more peripherals

todo: a lot more
*/

static uint8_t run;

void stop(PROTOTYPE_ARGS_HANDLER)
{
	ARGS_HANDLER_UNUSED;
	run=0;
}

#define SZ_INP_BUFFER 50

#define FILENAME "lkv373.bin"
//#define FILENAME "lkv373_mod.bin"
//#define FILENAME "output_full.bin"
//#define FILENAME "more_verbose.bin"
//#define FILENAME "bootloader.bin"

int main(void)
{	
	init_sim(FILENAME);
	
	window_init();
	int w_disassembly=init_disassembly_view();
	init_reg_view();
	int w_mem=init_mem_view();
	int w_stack=init_stack_view();
	init_breakpoints_view();
	int w_uart=init_uart_view();
	
	init_pmu();
	init_intc();
	init_timer();
	init_cpe();
	init_sspc(FILENAME);
	init_mac();
		
#ifdef CONNECT_TO_REAL
	init_connector();
#endif
	
	
	//unknown periph(s) - values read from device with connector serial
	memory_set_word(0xffff, 0x99c00000, NULL, true);
	memory_set_word(0x00, 0x99c00004, NULL, true);
	memory_set_word(0x00, 0x99c00008, NULL, true);
	memory_set_word(0x00, 0x99c0000c, NULL, true);
	memory_set_word(0x00, 0x99c00010, NULL, true);
	memory_set_word(0x00, 0x99c00014, NULL, true);
	memory_set_word(0xfe, 0x99c00018, NULL, true);
	memory_set_word(0x07, 0x99c0001c, NULL, true);
	memory_set_word(0x00, 0x99c00020, NULL, true);
	memory_set_word(0x00, 0x99c00024, NULL, true);
	memory_set_word(0x00, 0x99c00028, NULL, true);
	memory_set_word(0x00, 0x99c0002c, NULL, true);
	memory_set_word(0x00, 0x99c00030, NULL, true);
	memory_set_word(0x0a, 0x99c00034, NULL, true);
	memory_set_word(0x00, 0x99c00038, NULL, true);
	memory_set_word(0x14, 0x99c0003c, NULL, true);
	memory_set_word(0xffff, 0x99c00040, NULL, true);
	memory_set_word(0x00, 0x99c00044, NULL, true);
	memory_set_word(0x00, 0x99c00048, NULL, true);
	memory_set_word(0x00, 0x99c0004c, NULL, true);
	memory_set_word(0x00, 0x99c00050, NULL, true);
	memory_set_word(0x00, 0x99c00054, NULL, true);
	memory_set_word(0x60, 0x99c00058, NULL, true);
	memory_set_word(0x00, 0x99c0005c, NULL, true);
	memory_set_word(0x00, 0x99c00060, NULL, true);
	memory_set_word(0x00, 0x99c00064, NULL, true);
	memory_set_word(0x00, 0x99c00068, NULL, true);
	memory_set_word(0x00, 0x99c0006c, NULL, true);
	memory_set_word(0x00, 0x99c00070, NULL, true);
	memory_set_word(0x0a, 0x99c00074, NULL, true);
	memory_set_word(0x00, 0x99c00078, NULL, true);
	memory_set_word(0x52, 0x99c0007c, NULL, true);
	memory_set_word(0xee, 0x99c00080, NULL, true);
	memory_set_word(0x191a, 0x9090a800, NULL, true);
	memory_set_word(0xc2, 0x9090a80c, NULL, true);
	memory_set_word(0, 0x9090c408, NULL, true);
	memory_set_word(0, 0x9090c40c, NULL, true);
	memory_set_word(0x00, 0x90907070, NULL, true); //TODO: real value?
	memory_set_word(0x92, 0x90907074, NULL, true);
	memory_set_word(0x00, 0x90907078, NULL, true); //idem
	memory_set_word(0, 0x9090707c, NULL, true);
	memory_set_word(0, 0x90907204, NULL, true);
	memory_set_word(0, 0x9090b050, NULL, true);
	
	//is this correct?
	uint32_t a;
	for(a=0; a<PAGE_SIZE; a+=4)
	{
		memory_set_word(0, a+0x10000000, NULL, true);
		memory_set_word(0, a+0x20000000, NULL, true);
	}
	
	redraw_all();
	
	printf("ready...\n");
	putc('>', stdout);
	
	char input_buffer[SZ_INP_BUFFER];
	uint8_t pos_input_buffer=0;
	
	run=1;
	while(run)
	{
		//for scrolling and keypresses in other windows
		dispatcher(W_ASM, w_disassembly);
		dispatcher(W_MEM, w_mem);
		dispatcher(W_STACK, w_stack);
		dispatcher(W_UART, w_uart);
		
		//event for main window
		//we need all this stuff because we can't use blocking input as it would block the loop and disable scrolling in other windows that relies on
		//regular dispatcher() calls
		uint8_t events[SZ_EVENT_BUFFER];
		uint32_t nb_ev;
		uint32_t i;
		nb_ev=read_input(W_MAIN, events, SZ_EVENT_BUFFER);
		for(i=0; i<nb_ev; i++)
		{
			//this would need some improvements like cursor movement, insert/replace and so on
			if(isprint(events[i]) && pos_input_buffer<SZ_INP_BUFFER-1) //place for '\0'!
				input_buffer[pos_input_buffer++]=events[i];
			else if(events[i]==0x7f && pos_input_buffer) //backspace, \b does not work
				pos_input_buffer--;
			else if(events[i]=='\n')
			{
				input_buffer[pos_input_buffer]='\0';
				parse_input(input_buffer);
				pos_input_buffer=0;
				if(run)
					putc('>', stdout);
			}
		}
		
		usleep(0.25*1000);
	}
	
	printf("bye...\n");
		
	return 0;
}
