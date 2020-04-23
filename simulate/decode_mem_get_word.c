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

#include "decode_mem_get_word.h"
#include "memory.h"
#include "my_err.h"
#include "cpe.h"

uint32_t decode_disassm_memory_get_word(const uint32_t addr)
{
	//no remap here, this is a wrapper, remapping is done inside memory_get_word
		
	mem_word_t val=memory_get_word(addr, NULL);
	if(!val.is_initialized)
	{
		//what to do here? can happen because of asm-view at end of prog or interrupt so fatal exit is not good
		//printf("WARN: decode_disassm tried to read from uninitialized mem @0x%x", addr);
		val.val=0xffffffff;
	}
		//ERRX(1, "decode_disassm tried to read from uninitialized mem @0x%x", addr);
	
	uint32_t v=val.val;
	
	v=((v&0xFF)<<24)|((v&0xFF00)<<8)|((v&0xFF0000)>>8)|((v&0xFF000000)>>24);
	
	return v;
}
