/*
This file is part of a disassembler for Andestech NDS32.

(c) 2019-2020 kitten_nb_five

THIS WORK COMES WITHOUT ANY WARRANTY and is released under the AGPL version 3 or later
*/

#ifndef __OPC32_H__
#define __OPC32_H__

#include <stdint.h>

typedef enum
{
	TYPE_T0,
	TYPE_T1,
	TYPE_T2,
	TYPE_T3,
	TYPE_T4,
	TYPE_T5,
} opc32_type_t;

typedef struct
{
	char * mnemonic;
	uint8_t opc;
	opc32_type_t type;
	uint8_t has_sub:1;
} opcode32_t;

typedef struct
{
	char * mnemonic;
	uint8_t opc;
	uint8_t sub;
} sub_opc32_t;

#endif
