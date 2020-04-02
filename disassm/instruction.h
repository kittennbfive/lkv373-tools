/*
This file is part of a disassembler for Andestech NDS32.

(c) 2019-2020 kitten_nb_five

THIS WORK COMES WITHOUT ANY WARRANTY and is released under the AGPL version 3 or later
*/

#ifndef __INSTRUCTION_H__
#define __INSTRUCTION_H__

#include <stdint.h>

#include "opc32.h"
#include "opc16.h"

#define SZ_BUFFER_DISASSM 25

typedef enum
{
	WIDTH16,
	WIDTH32
} instr_width_t;

typedef struct
{
	instr_width_t width;
	opc32_type_t type32;
	opc16_type_t type16;
	uint8_t was_16bit_instr; //PC=PC+_2_
	uint8_t opc_length;
	uint16_t opc;
	uint16_t sub; //was 8 bit, modified to fit ALU_2 baseline ver 2
	uint16_t sub_10;
	uint8_t rt;
	uint8_t ra;
	uint8_t rb;
	uint8_t rd;
	uint8_t re;
	uint8_t rs; //32, DIVR
	uint8_t dt;
	uint8_t enable;
	uint32_t imm1_24;
	uint32_t imm1_20;
	uint32_t imm1_16;
	uint32_t imm1_15;
	uint32_t imm1_14;
	uint32_t imm1_10; //16
	uint32_t imm1_8; //16
	uint32_t imm1_7; //16
	uint32_t imm1_5;
	uint32_t imm1_3; //16
	uint32_t imm1_2;
	uint32_t imm2_5;
	uint8_t usr; //MTUSR and stuff
	uint8_t group; //idem
	uint16_t sridx;
	char *mnemonic;
	char disassm[SZ_BUFFER_DISASSM];
} instr_t;

#endif
