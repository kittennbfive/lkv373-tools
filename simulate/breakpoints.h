/*
This file is part of the simulator for the LKV373
(c) 2019-2020 by kitten_nb_five
freenode #lkv373a

licence: AGPL v3 or later

THIS PROGRAM COMES WITHOUT ANY WARRANTY!
*/

#ifndef __BREAKPOINTS_H__
#define __BREAKPOINTS_H__
#include <stdint.h>
#include <stdbool.h>

#include "cmd_parser.h"

typedef enum
{
	MEMORY_READ,			//BP before read is executed
	MEMORY_WRITE,			//BP after write is executed
	REGISTER_IS_VALUE,		//BP before instr is executed
	REGISTER_IS_NOT_VALUE,	//BP before instr is executed
	PC_IS_VALUE				//BP before instr is executed
} event_type_t;

typedef enum
{
	ADDR,
	RANGE
} mem_type_addr_t;

typedef enum
{
	IS_ACCESSED,
	IS_VALUE,
	IS_NOT_VALUE,
	IS_UNDEF
} mem_type_match_t;

typedef enum
{
	MESSAGE, //only print a message
	STOP, //stop execution
	JUMP, //jump to another PC - useful for getting over delay-loops
	SCRIPT //run script
} event_action_t;

#define SZ_BUF_BP_NAME_SCRIPT 20

typedef struct event
{
	bool disabled;
	event_type_t type;
	mem_type_addr_t type_addr_mem;
	mem_type_match_t type_match_mem;
	uint32_t addr;
	uint32_t range_from;
	uint32_t range_to;
	uint32_t value;
	uint8_t reg;
	uint32_t PC;
	event_action_t action;
	uint32_t jump_target;
	char script[SZ_BUF_BP_NAME_SCRIPT];
} event_t;

typedef enum
{
	READ,
	WRITE,
} type_mem_access_t;

#define NB_BREAKPOINTS_MAX 20

int init_breakpoints_view(void);
void print_breakpoints(void);
void cmd_breakpoint(PROTOTYPE_ARGS_HANDLER);

void simulate_check_for_breakpoints(uint32_t * const PC, uint32_t const * const regs, bool * const PC_manipulated, bool * const stop);
void mem_rw_check_for_breakpoints(const type_mem_access_t type, const uint32_t addr, const uint32_t val, const bool mem_is_initialized, bool * const stop);

#endif
