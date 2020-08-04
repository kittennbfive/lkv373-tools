/*
This file is part of a disassembler for Andestech NDS32.

(c) 2019-2020 kitten_nb_five

THIS WORK COMES WITHOUT ANY WARRANTY and is released under the AGPL version 3 or later
*/

#ifndef __DECODE_DISASSM_H__
#define __DECODE_DISASSM_H__
#include <stdint.h>
#include <stdbool.h>

#include "instruction.h"

//if decode_only==true then the mnemonic will not be printed but only decoded, this saves a lot of time in the simulator because sprintf() is a slow function and the mnemonics are not needed for the actual simulation
uint8_t decode_instr(instr_t * const instr_struct, const uint32_t PC, const bool decode_only);

#endif
