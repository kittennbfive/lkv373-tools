/*
This file is part of the simulator for the LKV373
(c) 2019-2020 by kitten_nb_five
freenode #lkv373a

licence: AGPL v3 or later

THIS PROGRAM COMES WITHOUT ANY WARRANTY!
*/

#ifndef __ENDLESS_LOOP_DETECT_H__
#define __ENDLESS_LOOP_DETECT_H__
#include <stdint.h>

typedef struct
{
	uint32_t PC;	
	uint32_t regs[32];
} history_t;

#define HISTORY_MAX_ENTRIES 25

void history_init(void);
void history_add(const uint32_t PC_old, uint32_t const * const regs);
uint8_t history_check(void);

#endif
