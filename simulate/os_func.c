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

#include "cmd_parser.h"
#include "memory.h"

//OFFSETS FOR NORMAL FIRMWARE - BLOCK2 ONLY
//NOT VALID FOR BOOTLOADER!

//----------------------------------------------------------------------
//GENERAL FUNCTIONS

uint32_t get_word(const uint32_t addr)
{
	mem_word_t word;
	word=memory_get_word(addr, NULL);
	return word.val;
}

uint32_t get_halfword(const uint32_t addr)
{
	mem_halfword_t halfword;
	halfword=memory_get_halfword(addr, NULL);
	return halfword.val;
}

uint32_t get_byte(const uint32_t addr)
{
	mem_byte_t byte;
	byte=memory_get_byte(addr, NULL);
	return byte.val;
}

//again, this is for block2 only!
char * get_task_name_from_id(const uint16_t id)
{
	switch(id)
	{
		case 0x0b: return "TF_MAIN";
		case 0x0a: return "Net";
		case 0x36: return "unknown1 (MAC)";
		case 0x0d: return "TF_AV";
		case 0x0e: return "TF_Console";
		case 0x10: return "TF_HTTP";
		case 0x11: return "TF_TFVEP";
		case 0xf: return "unknown2";
		
		case 0xffff: return "IDLE";
		case 0xfffe: return "STAT";
		case 0xfffd: return "TMR";
		
		default: return "unknown!";
	}
}

char * get_task_name_from_priority(const uint8_t prio)
{
	switch(prio)
	{
		case 0x0b: return "TF_MAIN";
		case 0x0a: return "Net";
		case 0x36: return "unknown1 (MAC)";
		case 0x0d: return "TF_AV";
		case 0x0e: return "TF_Console";
		case 0x10: return "TF_HTTP";
		case 0x11: return "TF_TFVEP";
		case 0xf: return "unknown2";
		
		case 0x3f: return "IDLE";
		case 0x3e: return "STAT";
		case 0x05: return "TMR";
		
		default: return "unknown!";
	}
}

//----------------------------------------------------------------------
//TASK CONTROL BLOCKS

#define POS_TCBTABLE 0xd0b74
#define NB_ENTRIES_TCBTABLE 22 //22
#define SIZEOF_TCBTABLE 92

#define TCB_WORD(entry, offset) get_word(POS_TCBTABLE+(entry)*SIZEOF_TCBTABLE+offset)
#define TCB_HALFWORD(entry, offset) get_halfword(POS_TCBTABLE+(entry)*SIZEOF_TCBTABLE+offset)
#define TCB_BYTE(entry, offset) get_byte(POS_TCBTABLE+(entry)*SIZEOF_TCBTABLE+offset)

#define SZ_NAME 15

void show_all_tcb(void)
{
	uint8_t i, j;
	uint32_t stack_top, stack_bottom;
	uint32_t task_id;
	uint32_t sp, addr_ipc, ipc;
	
	
	char name[SZ_NAME];
	
	for(i=0; i<NB_ENTRIES_TCBTABLE; i++)
	{
		printf("entry %d @0x%x:\n", i, POS_TCBTABLE+i*SIZEOF_TCBTABLE);

		task_id=TCB_HALFWORD(i, 0x12);
		
		if(!task_id)
		{
			printf("  (empty)\n");
			continue;
		}
		
		stack_top=TCB_WORD(i, 0x0);
		stack_bottom=TCB_WORD(i, 0x8);
		
		printf("  top of stack: 0x%x\n", stack_top);
		printf("  used defined data: 0x%x\n", TCB_WORD(i, 0x4));
		printf("  bottom of stack: 0x%x\n", stack_bottom);
		printf("  (stack size: 0x%x)\n", stack_top-stack_bottom);
		printf("  Task ID: 0x%x (%s)\n", task_id, get_task_name_from_id(task_id));
		printf("  prev TCB: 0x%x\n", TCB_WORD(i, 0x14));
		printf("  next TCB: 0x%x\n", TCB_WORD(i, 0x18));
		
		j=0;
		do
		{
			name[j]=TCB_BYTE(i, 0x4c+j);
		} while(name[j++] && j<SZ_NAME);
		name[j]='\0';
		printf("  name: %s\n", name);
		
		sp=get_word(stack_top);
		addr_ipc=stack_top+8;
		if((sp&7)==0) //don't ask me why this is done, i just copied from the disassembly. must be some alignment/ABI-stuff
			addr_ipc+=4;
		ipc=get_word(addr_ipc);
		printf("  sp: 0x%x\n", sp);
		printf("  IPC: 0x%x\n", ipc);
	}	
}

//----------------------------------------------------------------------
//READY LIST
#define POS_RDYTBL 0xcfefc

void decode_rdy_tbl(void)
{
	uint8_t priority;
	
	for(priority=0; priority<64; priority++)
	{
		if((get_byte(POS_RDYTBL+priority/8)>>(priority%8))&1)
			printf("Task with priority 0x%x (%s) is ready to run\n", priority, get_task_name_from_priority(priority));
	}
	
}

//----------------------------------------------------------------------
//DISPATCHER

void show_os_info(PROTOTYPE_ARGS_HANDLER)
{
	ARGS_HANDLER_UNUSED;
	
	char *show_what=get_next_argument();
	
	if(!strcmp(show_what, "tcb"))
		show_all_tcb();
	else if(!strcmp(show_what, "rdy"))
		decode_rdy_tbl();
	else
		printf("unknown: %s\n", show_what);
}

