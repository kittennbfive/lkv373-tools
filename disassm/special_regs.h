/*
This file is part of a disassembler for Andestech NDS32.

(c) 2019-2020 kitten_nb_five

THIS WORK COMES WITHOUT ANY WARRANTY and is released under the AGPL version 3 or later
*/

#ifndef __SPECIAL_REGS_H__
#define __SPECIAL_REGS_H__
#include <stdint.h>

typedef struct
{
	uint16_t id;
	char * name;
} sr_t;

char * get_special_reg_name(const uint16_t sridx);

#endif
