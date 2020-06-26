/*
This file is part of the simulator for the LKV373
(c) 2020 by kitten_nb_five
freenode #lkv373a

licence: AGPL v3 or later

THIS PROGRAM COMES WITHOUT ANY WARRANTY!
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "os_common.h"

#include "memory.h"

//lets make some wrappers that return directly the value to simplify following code
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
		
		default: return "???";
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
		
		case 0xff: return "OS_PRIO_SELF";
		
		default: return "???";
	}
}

//OSTCBCur
#define POS_OSTCBCURR 0x8934c
uint16_t get_current_task_id(void)
{
	uint32_t ptr=get_word(POS_OSTCBCURR);
	//printf("Pointer to current TCB is 0x%x\n", ptr);
	if(ptr)
		return get_halfword(ptr+0x12);
	else
	{
		//printf("could not determine current task ID, ptr is NULL\n");
		return 0;
	}
}
