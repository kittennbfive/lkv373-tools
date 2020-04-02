/*
This file is part of a disassembler for Andestech NDS32.

(c) 2019-2020 kitten_nb_five

THIS WORK COMES WITHOUT ANY WARRANTY and is released under the AGPL version 3 or later
*/

#include <stdlib.h>

#define FALSE 0
#define TRUE (!FALSE)

#include "opc32.h"

const opcode32_t opc32_list[]= 
{	//page 52-223
	{"ALU_1",	0b100000, TYPE_T4,	TRUE}, //ALU1 //UNSIGNED if(is(imm))
	{"ADDI",	0b101000, TYPE_T2,	FALSE}, //add immediate
	{"ANDI",	0b101010, TYPE_T2,	FALSE}, //and immediate
	{"BR1",		0b100110, TYPE_T2,	TRUE}, //conditional branch 1
	{"BR2",		0b100111, TYPE_T1,	TRUE}, //conditional branch 2
	{"MISC",	0b110010, TYPE_T4,	TRUE}, //misc instructions
	{"ALU_2",	0b100001, TYPE_T5,	TRUE}, //ALU2
	{"J",		0b100100, TYPE_T0,	TRUE}, //unconditional branch
	{"JREG",	0b100101, TYPE_T4,	TRUE}, //unconditional branch register
	{"MEM",		0b011100, TYPE_T3,	TRUE}, //load from memory
	{"LBI",		0b000000, TYPE_T2,	FALSE}, //load immediate 1
	{"LBI.bi",	0b000100, TYPE_T2,	FALSE}, //load immediate 2
	{"LBSI",	0b010000, TYPE_T2,	FALSE}, //load immediate signed 1
	{"LBSI.bi",	0b010100, TYPE_T2,	FALSE}, //load immediate signed 2
	{"LHI",		0b000001, TYPE_T2,	FALSE}, //load halfword immediate 1
	{"LHI.bi",	0b000101, TYPE_T2,	FALSE}, //load halfword immediate 2
	{"LHSI",	0b010001, TYPE_T2,	FALSE}, //load halfword signed immediate 1
	{"LHSI.bi",	0b010101, TYPE_T2,	FALSE}, //load halfword signed immediate 2
	{"LSMW",	0b011101, TYPE_T5,	TRUE}, //load/store multiple words
	{"LWI",		0b000010, TYPE_T2,	FALSE}, //load word immediate 1
	{"LWI.bi",	0b000110, TYPE_T2,	FALSE}, //load word immediate 2
	{"MOVI",	0b100010, TYPE_T1,	FALSE}, //move immediate
	{"ORI",		0b101100, TYPE_T2,	FALSE}, //and immediate
	{"SBI",		0b001000, TYPE_T2,	FALSE}, //store byte immediate 1
	{"SBI.bi",	0b001100, TYPE_T2,	FALSE}, //store byte immediate 2
	{"SETHI",	0b100011, TYPE_T1,	FALSE}, //set high immediate
	{"SHI",		0b001001, TYPE_T2,	FALSE}, //store halfword immediate 1
	{"SHI.bi",	0b001101, TYPE_T2,	FALSE}, //store halfword immediate 2
	{"SLTI",	0b101110, TYPE_T2,	FALSE}, //set on less than immediate
	{"SLTSI",	0b101111, TYPE_T2,	FALSE}, //set on less than immediate signed
	{"SUBRI",	0b101001, TYPE_T2,	FALSE}, //substract reverse immediate
	{"SWI",		0b001010, TYPE_T2,	FALSE}, //store word immediate 1
	{"SWI.bi",	0b001110, TYPE_T2,	FALSE}, //store word immediate 2
	{"XORI",	0b101011, TYPE_T2,	FALSE}, //xor immediate
	
	{NULL, 0, 0, FALSE}
};

const sub_opc32_t opc32_sub_list[]=
{
	//ALU_1
	{"ADD",		0b100000,	0b00000}, //add
	{"AND", 	0b100000,	0b00010}, //and
	{"CMOVN",	0b100000,	0b11011}, //conditional move on not zero
	{"CMOVZ",	0b100000,	0b11010}, //conditional move on zero
//	{"NOP",		0b100000,	0b01001}, //nop
	{"NOR",		0b100000,	0b00101}, //nor
	{"OR",		0b100000,	0b00100}, //or
	{"ROTR",	0b100000,	0b01111}, //rotate right
	{"ROTRI",	0b100000,	0b01011}, //rotate right immediate
	{"SEB",		0b100000,	0b10000}, //sign extend byte
	{"SEH",		0b100000,	0b10001}, //sign extend halfword
	{"SLL",		0b100000,	0b01100}, //shift left logical
	{"SLLI",	0b100000,	0b01000}, //shift left logical immediate
	{"SLT",		0b100000,	0b00110}, //set on less than
	{"SLTS",	0b100000,	0b00111}, //set on less than signed
	{"SRA",		0b100000,	0b01110}, //shift right arithmetic
	{"SRAI",	0b100000,	0b01010}, //shift right arithmetic immediate
	{"SRL",		0b100000,	0b01101}, //shift right logical
	{"SRLI",	0b100000,	0b01001}, //shift right logical immediate
	{"SUB",		0b100000,	0b00001}, //sub
	{"SVA",		0b100000,	0b11000}, //set on overflow add
	{"SVS",		0b100000,	0b11001}, //set on overflow substract
	{"WSBH",	0b100000,	0b10100}, //word swap within halfword
	{"XOR",		0b100000,	0b00011}, //xor
	{"ZEH",		0b100000,	0b10011}, //zero extend halfword

	//ALU_2
	{"DIV",		0b100001,	0b0000101111}, //unsigned integer div
	{"DIVS",	0b100001,	0b0000101110}, //signed integer div
	{"MADD32",	0b100001,	0b0000110011}, //multiply and add to data low
	{"MADD64",	0b100001,	0b0000101011}, //multiply and add unsigned
	{"MADDS64",	0b100001,	0b0000101010}, //multiply and add signed
	{"MFUSR",	0b100001,	0b0000100000}, //move from user special register
	{"MSUB32",	0b100001,	0b0000110101}, //multiply and substract to data low
	{"MSUB64",	0b100001,	0b0000101101}, //multiply and substract unsigned
	{"MSUBS64",	0b100001,	0b0000101100}, //multiply and substract signed
	{"MTUSR",	0b100001,	0b0000100001}, //move to user special register
	{"MUL",		0b100001,	0b0000100100}, //multiply word to register
	{"MULT32",	0b100001,	0b0000110001}, //multiply word to data low
	{"MULT64",	0b100001,	0b0000101001}, //multiply word unsigned
	{"MULTS64",	0b100001,	0b0000101000}, //multiply word signed
	
	//BR1
	{"BEQ",	0b100110,	0b0}, //branch on equal
	{"BNE",	0b100110,	0b1}, //branch on not equal
	
	//BR2
	{"BEQZ",	0b100111,	0b0010}, //branch on equal zero
	{"BGEZ",	0b100111,	0b0100}, //branch on greater than or equal to zero
	{"BGEZAL",	0b100111,	0b1100}, //branch on greater than or equal to zero and link
	{"BGTZ",	0b100111,	0b0110}, //branch on greater than zero
	{"BLEZ",	0b100111,	0b0111}, //branch on less than or equal to zero
	{"BLTZ",	0b100111,	0b0101}, //branch on less than zero
	{"BLTZAL",	0b100111,	0b1101}, //branch on less than zero and link
	{"BNEZ",	0b100111,	0b0011}, //branch on not equal zero	
	
	//MISC
	{"BREAK",	0b110010,	0b01010}, //breakpoint
	{"CCTL",	0b110010,	0b00001}, //cache control
	{"DSB",		0b110010,	0b01000}, //data serialization barrier
	{"IRET",	0b110010,	0b00100}, //interrupt return
	{"ISB",		0b110010,	0b01001}, //instruction serialization barrier
	{"ISYNC",	0b110010,	0b01101}, //instruction data coherence synchronization
	{"MFSR",	0b110010,	0b00010}, //move from system register
	{"MSYNC",	0b110010,	0b01100}, //memory data coherence synchronization
	{"MTSR",	0b110010,	0b00011}, //move to system register
	{"SETEND",	0b110010,	0b00001}, //set data endian
	{"SETGIE",	0b110010,	0b00010}, //set global interrupt enable
	{"STANDBY",	0b110010,	0b00000}, //wait for external event
	{"SYSCALL",	0b110010,	0b01011}, //system call
	{"TEQZ",	0b110010,	0b00110}, //trap if equal zero
	{"TNEZ",	0b110010,	0b00111}, //trap if not equal zero
	{"TLBOP",	0b110010,	0b01110}, //TLB operation
	{"TRAP",	0b110010,	0b00101}, //trap exception
	
	//J
	{"J",	0b100100,	0b0}, //jump
	{"JAL",	0b100100,	0b1}, //jump and link
	
	//JREG
	{"JR",		0b100101,	0b000000}, //jump register
	{"JRAL",	0b100101,	0b000001}, //jump register and link
	{"RET",		0b100101,	0b100000}, //return from register
	
	//MEM
//	{"DPREF",	0b011100,	0b00010011}, //data prefetch //???? format??
	{"LB",		0b011100,	0b00000000}, //load byte 1
	{"LB.bi",	0b011100,	0b00000100}, //load byte 2
	{"LBS",		0b011100,	0b00010000}, //load byte signed 1
	{"LBS.bi",	0b011100,	0b00010100}, //load byte signed 2
	{"LH",		0b011100,	0b00000001}, //load halfword 1
	{"LH.bi",	0b011100,	0b00000101}, //load halfword 2
	{"LHS",		0b011100,	0b00010001}, //load halfword signed 1
	{"LHS.bi",	0b011100,	0b00010101}, //load halfword signed 2
	{"LLW",		0b011100,	0b00011000}, //load locked word
	{"LW",		0b011100,	0b00000010}, //load word 1
	{"LW.bi",	0b011100,	0b00000110}, //load word 2
//	{"LWUP",	0b011100,	0b00100010}, //load word with user privilege translation
	{"SB",		0b011100,	0b00001000}, //store byte 1
	{"SB.bi",	0b011100,	0b00001100}, //store byte 2
	{"SCW",		0b011100,	0b00011001}, //store conditional word
	{"SH",		0b011100,	0b00001001}, //store halfword 1
	{"SH.bi",	0b011100,	0b00001101}, //store halfword 2
	{"SW",		0b011100,	0b00001010}, //store word 1
	{"SW.bi",	0b011100,	0b00001110}, //store word 2
//	{"SWUP",	0b011100,	0b00101010}, //store word with user privilege translation
	
	//LMW load multiple words - see page 124
	{"LMW.ai",		0b011101,	0b010000},
	{"LMW.bi",		0b011101,	0b000000},
	{"LMW.ad",		0b011101,	0b011000},
	{"LMW.bd",		0b011101,	0b001000},
	{"LMW.aim",		0b011101,	0b010100},
	{"LMW.bim",		0b011101,	0b000100},
	{"LMW.adm",		0b011101,	0b011100},
	{"LMW.bdm",		0b011101,	0b001100},
	
	{"SMW.ai",		0b011101,	0b110000},
	{"SMW.bi",		0b011101,	0b100000},
	{"SMW.ad",		0b011101,	0b111000},
	{"SMW.bd",		0b011101,	0b101000},
	{"SMW.aim",		0b011101,	0b110100},
	{"SMW.bim",		0b011101,	0b100100},
	{"SMW.adm",		0b011101,	0b111100},
	{"SMW.bdm",		0b011101,	0b101100},
	
	//sub from LSMW:
	//SMWA store multiple words with alignment check
	//baseline ver 2 - page 331
	{"SMWA.adm", 	0b011101, 0b111101},
	{"SMWA.bdm", 	0b011101, 0b101101},
	
	{"LMWA.bim", 	0b011101, 0b000101},
	
	//sub from ALU_2 - SUB-OPCODE HACKED TO 10 BITS
	//32 bit performance extension
	{"BSET",		0b100001,	0b0000001000},
	{"BTST",		0b100001,	0b0000001011},
	{"MIN",			0b100001,	0b0000000001},
	{"BSP",			0b100001,	0b0000001101}, //bit stream packing
	{"ABS",			0b100001,	0b0000000011},
	{"MAX",			0b100001,	0b0000000000},
	//uses general-purpose regs, GPR is 0b0001
	{"MADDR32",		0b100001,	0b0001110011},
	{"MSUBR32",		0b100001,	0b0001110101},
	{"MULR64",		0b100001,	0b0001101001},
	{"MULSR64",		0b100001,	0b0001101000},
	
	
	//sub from ALU_1
	//32 bit baseline ver 2
	{"DIVR",		0b100000,	0b10111},
	
	//sub from ALU_1
	//32 bit baseline optional
	{"DIVSR", 		0b100000,	0b10110},
	
	{NULL, 0, 0}
};
