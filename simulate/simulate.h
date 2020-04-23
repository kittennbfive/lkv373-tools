/*
This file is part of the simulator for the LKV373
(c) 2019-2020 by kitten_nb_five
freenode #lkv373a

licence: AGPL v3 or later

THIS PROGRAM COMES WITHOUT ANY WARRANTY!
*/

#ifndef __SIMULATE_H__
#define __SIMULATE_H__
#include <stdint.h>
#include <stdbool.h>

#include "instruction.h"
#include "cmd_parser.h"

typedef enum
{
	SIM_NO_ERROR,
	SIM_ERROR_DECODE_INSTR,
	SIM_ENDLESS_LOOP,
	SIM_UNIMPLEMENTED,
	SIM_READ_FROM_UNINITIALIZED,
	SIM_STOPPED_ON_BP,
	SIM_STOPPED_ON_IRQ,
	SIM_GENERIC_ERROR
} sim_t;

void init_sim(char const * const filename);
sim_t simulate(instr_t const * const instr, const bool ignore_breakpoints);
void single_step(PROTOTYPE_ARGS_HANDLER);
void run_sim(PROTOTYPE_ARGS_HANDLER);

typedef struct
{
	uint32_t H;
	uint32_t L;
} usr_t;

uint32_t get_register(const uint8_t i);
usr_t get_usr(const uint8_t i);
uint32_t get_stack_pointer(void);
uint32_t get_PC(void);
void set_PC(const uint32_t addr);

void write_register(PROTOTYPE_ARGS_HANDLER);
void set_pc_cmd(PROTOTYPE_ARGS_HANDLER);

#define PAGE_SIZE (16*1024*1024) //do not change!

#define REG_FP 28
#define REG_GP 29
#define REG_LP 30
#define REG_SP 31

#endif
