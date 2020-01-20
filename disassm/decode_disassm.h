/*
This file is part of a disassembler for Andestech NDS32.

(c) 2019-2020 kitten_nb_five

THIS WORK COMES WITHOUT ANY WARRANTY and is released under the AGPL version 3 or later
*/

#ifndef __DECODE_DISASSM_H__
#define __DECODE_DISASSM_H__

#include "instruction.h"

uint8_t decode_instr(instr_t * const instr_struct, const uint32_t PC);

#endif
