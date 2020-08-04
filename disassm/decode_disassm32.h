/*
This file is part of a disassembler for Andestech NDS32.

(c) 2019-2020 kitten_nb_five

THIS WORK COMES WITHOUT ANY WARRANTY and is released under the AGPL version 3 or later
*/

#ifndef __DECODE_DISASSM32_H__
#define __DECODE_DISASSM32_H__
#include <stdint.h>
#include <stdbool.h>

#include "instruction.h"

#define SHORT_INSTR_IS_WIDTH32(byte) ((byte&(1<<15))==0)

uint8_t decode_32(const uint32_t instr, instr_t * const instr_struct, const uint32_t PC, const bool decode_only);

#endif
