/*
experimental r2-support for nds32

written by kitten_nb_five (c) 2019-2020

HIGHLY EXPERIMENTAL - NO WARRANTY - PROVIDED AS IS

licence: AGPL version 3 or later
*/

#ifndef __DECODE_DISASSM16_H__
#define __DECODE_DISASSM16_H__

#include "instruction.h"

uint8_t decode_16(const uint16_t instr, instr_t * const instr_struct, const uint32_t PC);

#endif
