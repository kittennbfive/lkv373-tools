/*
This file is part of a disassembler for Andestech NDS32.

(c) 2019-2020 kitten_nb_five

THIS WORK COMES WITHOUT ANY WARRANTY and is released under the AGPL version 3 or later
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "decode_disassm.h"
#include "decode_disassm32.h"
#include "decode_disassm16.h"
#include "debug.h"

uint32_t decode_disassm_memory_get_word(const uint32_t addr);

uint8_t decode_instr(instr_t * const instr_struct, const uint32_t PC, const bool decode_only)
{
	uint16_t instr;
	memset(instr_struct, 0xff, sizeof(instr_t));
	
	instr=decode_disassm_memory_get_word(PC)>>16;
	
	PRINTF_DEBUG("decoding 0x%04x\n", (uint16_t)instr);
	
	memset(instr_struct, 0, sizeof(instr_t));
	
	if(SHORT_INSTR_IS_WIDTH32(instr))
	{
		PRINTF_DEBUG("32 bit instr\n");
		uint32_t instr32=decode_disassm_memory_get_word(PC);
		PRINTF_DEBUG("instr is now 0x%08x\n", instr32);
		instr_struct->width=WIDTH32;
		if(decode_32(instr32, instr_struct, PC, decode_only))
			return 1;
	}
	else
	{
		PRINTF_DEBUG("16 bit instr\n");
		instr_struct->width=WIDTH16;
		if(decode_16(instr, instr_struct, PC, decode_only))
			return 1;
	}
	
	return 0;
}
