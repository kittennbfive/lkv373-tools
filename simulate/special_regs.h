/*
This file is part of the simulator for the LKV373
(c) 2019-2020 by kitten_nb_five
freenode #lkv373a

licence: AGPL v3 or later

THIS PROGRAM COMES WITHOUT ANY WARRANTY!
*/

#ifndef __SPECIAL_REGS_H__
#define __SPECIAL_REGS_H__
#include <stdint.h>
#include <stdbool.h>

#include "cmd_parser.h"

typedef struct
{
	uint16_t id;
	char * name;
} sr_t;

#define SR_PROC_STATUS_WORD 0x80
#define SR_PROC_INT_STATUS_WORD 0x81
#define SR_INT_PEND 0xc8
#define SR_INT_PC 0xa9

//PSW
#define PSW_GIE (1<<0)
#define PSW_PROC_OP_MODE ((1<<3)|(1<<4))

char * get_special_reg_name(const uint16_t sridx);

void write_to_special_reg(const uint16_t sridx, const uint32_t val);
uint32_t read_from_special_reg(const uint16_t sridx);
void special_reg_setgie(const uint8_t en);
bool get_gie(void);

void cmd_write_sr(PROTOTYPE_ARGS_HANDLER);

#endif
