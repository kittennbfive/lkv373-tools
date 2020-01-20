/*
This file is part of a disassembler for Andestech NDS32.

(c) 2019-2020 kitten_nb_five

THIS WORK COMES WITHOUT ANY WARRANTY and is released under the AGPL version 3 or later
*/

#ifndef __OPC16_H__
#define __OPC16_H__

#include <stdint.h>

typedef enum
{
	TYPE16_16,
	TYPE16_333,
	TYPE16_333_SUB,
	TYPE16_37_SUB,
	TYPE16_38,
	TYPE16_45,
	TYPE16_450,
	TYPE16_5,
	TYPE16_55,
	TYPE16_8,
	TYPE16_10
} opc16_type_t;

typedef struct
{
	char * mnemonic;
	uint8_t opc_length;
	uint16_t opc;
	opc16_type_t type;
	uint8_t has_sub:1;
} opcode16_t;

typedef struct
{
	char * mnemonic;
	uint16_t opc;
	uint8_t sub;
} sub_opc16_t;


#endif
