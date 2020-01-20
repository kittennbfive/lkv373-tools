/*
experimental r2-support for nds32

written by kitten_nb_five (c) 2019-2020

HIGHLY EXPERIMENTAL - NO WARRANTY - PROVIDED AS IS

licence: AGPL version 3 or later
*/

#ifndef __DECODE_DISASSM32_H__
#define __DECODE_DISASSM32_H__

#include "instruction.h"

#define SHORT_INSTR_IS_WIDTH32(byte) ((byte&(1<<15))==0)

uint8_t decode_32(const uint32_t instr, instr_t * const instr_struct, const uint32_t PC);

#endif
