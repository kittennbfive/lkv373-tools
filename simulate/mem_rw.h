/*
This file is part of the simulator for the LKV373
(c) 2019-2020 by kitten_nb_five
freenode #lkv373a

licence: AGPL v3 or later

THIS PROGRAM COMES WITHOUT ANY WARRANTY!
*/

#ifndef __MEM_RW_H__
#define __MEM_RW_H__
#include <stdint.h>
#include <stdbool.h>

#include "instruction.h"
#include "simulate.h"

sim_t sim_opc_lmw(instr_t const * const instr, uint32_t * const regs, bool * const stop);
void sim_opc_smw(instr_t const * const instr, uint32_t * const regs, bool * const stop);
sim_t sim_opc_lmwa(instr_t const * const instr, uint32_t * const regs, bool * const stop);
void sim_opc_smwa(instr_t const * const instr, uint32_t * const regs, bool * const stop);

#endif
