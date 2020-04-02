/*
This file is part of a disassembler for Andestech NDS32.

(c) 2019-2020 kitten_nb_five

THIS WORK COMES WITHOUT ANY WARRANTY and is released under the AGPL version 3 or later
*/

#include <stdlib.h>
#include "opc16.h"

#define FALSE 0
#define TRUE (!FALSE)

opcode16_t opc16_list[]=
{
	{"ADD333",	6,	0b001100,	TYPE16_333,	FALSE}, //Add Register
	{"ADD45",	6,	0b000100,	TYPE16_45,	FALSE}, //Add Register
	{"ADDI333",	6,	0b001110,	TYPE16_333,	FALSE}, //Add Immediate
	{"ADDI45",	6,	0b000110,	TYPE16_45,	FALSE}, //Add Immediate
	{"BEQS38",	4,	0b1010,	TYPE16_38,	FALSE}, //Branch on Equal Implied R5
	{"BEQZ38",	4,	0b1000,	TYPE16_38,	FALSE}, //Branch on Equal Zero
	{"BEQZS8",	7,	0b1101000,	TYPE16_8,	FALSE}, //Branch on Equal Zero Implied R15
	{"BFMI333",	6,	0b001011,	TYPE16_333_SUB,	TRUE}, //SEB, SEH, ZEB, ZEH
	{"BNES38",	4,	0b1011,	TYPE16_38,	FALSE}, //Branch on Not Equal Implied R5
	{"BNEZ38",	4,	0b1001,	TYPE16_38,	FALSE}, //Branch on Not Equal Zero
	{"BNEZS8",	7,	0b1101001,	TYPE16_8,	FALSE}, //Branch on Not Equal Zero Implied R15
	{"J8",	7,	0b1010101,	TYPE16_8,	FALSE}, //Jump Immediate
	{"JR5",	10,	0b1011101000,	TYPE16_5,	FALSE}, //Jump Register
	{"JRAL5",	10,	0b1011101001,	TYPE16_5,	FALSE}, //Jump Register and Link
	{"LBI333",	6,	0b010011,	TYPE16_333,	FALSE}, //Load Byte Immediate Unsigned
	{"LHI333",	6,	0b010010,	TYPE16_333,	FALSE}, //Load Halfword Immediate Unsigned
	{"LWI333",	6,	0b010000,	TYPE16_333,	FALSE}, //Load Word Immediate
	{"LWI333.bi",	6,	0b010001,	TYPE16_333,	FALSE}, //Load Word Immediate
	{"LWI450",	6,	0b011010,	TYPE16_450,	FALSE}, //Load Word Immediate
	{"MOV55",	5,	0b00000,	TYPE16_55,	FALSE}, //Move Register
	{"MOVI55",	5,	0b00001,	TYPE16_55,	FALSE}, //Move Immediate
	{"NOP16",	6,	0b001001,	TYPE16_16,	FALSE}, //No Operation
	{"RET5",	10,	0b1011101100,	TYPE16_5,	FALSE}, //Return from Register
	{"SBI333",	6,	0b010111,	TYPE16_333,	FALSE}, //Store Byte Immediate
	{"SHI333",	6,	0b010110,	TYPE16_333,	FALSE}, //Store Halfword Immediate
	{"SLLI333",	6,	0b001010,	TYPE16_333,	FALSE}, //Shift Left Logical Immediate
	{"SLT45",	6,	0b110001,	TYPE16_45,	FALSE}, //Set on Less Than Unsigned
	{"SLTI45",	6,	0b110011,	TYPE16_45,	FALSE}, //Set on Less Than Unsigned Immediate
	{"SLTS45",	6,	0b110000,	TYPE16_45,	FALSE}, //Set on Less Than Signed
	{"SLTSI45",	6,	0b110010,	TYPE16_45,	FALSE}, //Set on Less Than Signed Immediate
	{"SRAI45",	6,	0b001000,	TYPE16_45,	FALSE}, //Shift Right Arithmetic Immediate
	{"SRLI45",	6,	0b001001,	TYPE16_45,	FALSE}, //Shift Right Logical Immediate
	{"SUB333",	6,	0b001101,	TYPE16_333,	FALSE}, //Subtract Register
	{"SUB45",	6,	0b000101,	TYPE16_45,	FALSE}, //Subtract Register
	{"SUBI333",	6,	0b001111,	TYPE16_333,	FALSE}, //Subtract Immediate
	{"SUBI45",	6,	0b000111,	TYPE16_45,	FALSE}, //Subtract Immediate
	{"SWI333",	6,	0b010100,	TYPE16_333,	FALSE}, //Store Word Immediate
	{"SWI333.bi",	6,	0b010101,	TYPE16_333,	FALSE}, //Store Word Immediate
	{"SWI450",	6,	0b011011,	TYPE16_450,	FALSE}, //Store Word Immediate
	{"XWI37",	4,	0b0111,	TYPE16_37_SUB,	TRUE}, //LWI, SWI
	{"ADDI10.sp", 5,	0b11011, TYPE16_10, FALSE}, //add immediate with implied stack pointer
	{"XWI37SP", 4, 0b1110, TYPE16_37_SUB, TRUE}, //load word immediate with implied stack pointer
	
	{NULL, 0, 0, 0, FALSE}
};

const sub_opc16_t opc16_sub_list[]=
{
	//BFMI333
	{"SEB33",	0b001011,	0b010}, //sign extend byte
	{"SEH33",	0b001011,	0b011}, //sign extend halfword
	{"ZEB33",	0b001011,	0b000}, //zero extend byte
	{"ZEH33",	0b001011,	0b001}, //zero extend halfword
	{"XLSB33",	0b001011,	0b100}, //extract LSB
	
	//XWI37
	{"LWI37",	0b0111,	0b0}, //(Load Word Immediate with Implied FP)
	{"SWI37",	0b0111,	0b1}, //(Store Word Immediate with Implied FP) 
	
	//XWI37SP
	//16-bit baseline ver.2
	{"LWI37.sp", 0b1110, 0b0}, //Load Word Immediate with Implied Stack Pointer
	{"SWI37.sp", 0b1110, 0b1}, //Store Word Immediate with Implied Stack Pointer
	
	{NULL, 0, 0}
};
