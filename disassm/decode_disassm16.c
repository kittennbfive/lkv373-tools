/*
This file is part of a disassembler for Andestech NDS32.

(c) 2019-2020 kitten_nb_five

THIS WORK COMES WITHOUT ANY WARRANTY and is released under the AGPL version 3 or later
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "opc16.h"
#include "instruction.h"
#include "sign_extend.h"

#include "opcodes16.h"
#include "opcodes32.h"

#include "../simulate/my_err.h"

extern opcode16_t opc16_list[];
extern const sub_opc16_t opc16_sub_list[];

static uint8_t reg_4T5(const uint8_t reg_4)
{
	if(reg_4<=11)
		return reg_4;
	else
		return reg_4+4;
}

static void fill_args_instr_type16(const uint32_t instr, const uint16_t opc, const opc16_type_t type, instr_t * const instr_struct)
{
	instr_struct->opc=opc;
	instr_struct->type16=type;
	switch(type)
	{
		/*
		case TYPE16_16:
			//no operands
			break;
		*/
		case TYPE16_10:
			instr_struct->imm1_10=(instr&0x03ff);
			break;
		
		case TYPE16_333:
			instr_struct->rt=(instr&0x01C0)>>6;
			instr_struct->ra=(instr&0x0038)>>3;
			instr_struct->rb=(instr&0x0007);
			instr_struct->imm1_3=(instr&0x0007);
			break;
		
		case TYPE16_333_SUB:
			instr_struct->rt=(instr&0x01C0)>>6;
			instr_struct->ra=(instr&0x0038)>>3;
			instr_struct->sub=(instr&0x0007);
			break;
		
		case TYPE16_37_SUB:
			instr_struct->rt=(instr&0x0700)>>8;
			instr_struct->sub=(instr&0x0080)>>7;
			instr_struct->imm1_7=(instr&0x007F);
			break;
		
		case TYPE16_38:
			instr_struct->rt=(instr&0x0700)>>8;
			instr_struct->imm1_8=(instr&0x00FF);
			break;
		
		case TYPE16_45:
			instr_struct->ra=(instr&0x01E0)>>5;
			instr_struct->rt=(instr&0x01E0)>>5; //for ADD45
			instr_struct->rb=(instr&0x001F);
			instr_struct->imm1_5=(instr&0x001F);
			break;
		
		case TYPE16_450:
			instr_struct->rt=(instr&0x01E0)>>5;
			instr_struct->ra=(instr&0x001F);
			break;
		
		case TYPE16_5:
			instr_struct->rb=(instr&0x001F); //rB!
			break;
		
		case TYPE16_55:
			instr_struct->rt=(instr&0x03E0)>>5;
			instr_struct->ra=(instr&0x001F);
			instr_struct->imm1_5=(instr&0x001F);
			break;
		
		case TYPE16_8:
			instr_struct->imm1_8=(instr&0x00FF);
			break;
	}
}

void disassm16(instr_t * const instr_struct, const uint32_t PC __attribute__((__unused__)))
{
	switch(instr_struct->type16)
	{
		/*
		case TYPE16_16:
			switch(instr_struct->opc)
			{
				case OPC16_NOP16:
					sprintf(instr_struct->disassm, "NOP");
					break;
			}
			break;
		*/
		case TYPE16_10:
			switch(instr_struct->opc)
			{
				case OPC16_ADDI10S:
					sprintf(instr_struct->disassm, "%s 0x%x", instr_struct->mnemonic, nds32_sign_extend(instr_struct->imm1_10, 10, 32));
					break;
			}
			break;
		
		case TYPE16_333:
			switch(instr_struct->opc)
			{
				//%s rt, ra, rb
				case OPC16_ADD333:
				case OPC16_SUB333:
					sprintf(instr_struct->disassm, "%s r%u, r%u, r%u", instr_struct->mnemonic, instr_struct->rt, instr_struct->ra, instr_struct->rb);
					break;
				
				//%s rt, ra, imm_u
				case OPC16_ADDI333:
				case OPC16_SLLI333:
				case OPC16_SUBI333:
					sprintf(instr_struct->disassm, "%s r%u, r%u, 0x%x", instr_struct->mnemonic, instr_struct->rt, instr_struct->ra, instr_struct->imm1_3);
					break;
				
				//%s rt, [ra+imm_u]
				case OPC16_LBI333:
				case OPC16_SBI333:
					sprintf(instr_struct->disassm, "%s r%u, [r%u + 0x%x]", instr_struct->mnemonic, instr_struct->rt, instr_struct->ra, instr_struct->imm1_3);
					break;
				
				//%s rt, [ra+imm_u<<1]
				case OPC16_LHI333:
				case OPC16_SHI333:
					sprintf(instr_struct->disassm, "%s r%u, [r%u + 0x%x]", instr_struct->mnemonic, instr_struct->rt, instr_struct->ra, instr_struct->imm1_3<<1);
					break;
				
				//%s rt, [ra+imm_u<<2]
				case OPC16_LWI333:
				case OPC16_SWI333:
					sprintf(instr_struct->disassm, "%s r%u, [r%u + 0x%x]", instr_struct->mnemonic, instr_struct->rt, instr_struct->ra, instr_struct->imm1_3<<2);
					break;
				
				//%s rt, [ra], imm_u<<2
				case OPC16_LWI333_bi:
				case OPC16_SWI333_bi:
					sprintf(instr_struct->disassm, "%s r%u, [r%u], 0x%x", instr_struct->mnemonic, instr_struct->rt, instr_struct->ra, instr_struct->imm1_3<<2);
					break;
				
			}
			break;
		
		case TYPE16_333_SUB:
			switch(instr_struct->opc)
			{
				case OPC16_BFMI333:
					switch(instr_struct->sub)
					{
						//%s rt, ra
						case SUB_BMFI333_SEB33:
						case SUB_BMFI333_SEH33:
						case SUB_BMFI333_ZEB33:
						case SUB_BMFI333_ZEH33:
						case SUB_BMFI333_XLSB33:
							sprintf(instr_struct->disassm, "%s r%u, r%u", instr_struct->mnemonic, instr_struct->rt, instr_struct->ra);
							break;
					}
					break;
			}
			break;
		
		case TYPE16_37_SUB:
			switch(instr_struct->opc)
			{
				case OPC16_XWI37:
					switch(instr_struct->sub)
					{
						//%s rt, [fp+imm_u<<2]
						case SUB_XWI37_LWI37:
						case SUB_XWI37_SWI37:
							sprintf(instr_struct->disassm, "%s r%u, [fp + 0x%x]", instr_struct->mnemonic, instr_struct->rt, instr_struct->imm1_7<<2);
							break;
					}
					break;
				
				case OPC16_XWI37SP:
					//%s rt, [sp+imm_u<<2]
					sprintf(instr_struct->disassm, "%s r%u, [ + 0x%x]", instr_struct->mnemonic, instr_struct->rt, instr_struct->imm1_7<<2);
					break;
			}
			break;
		
		case TYPE16_38:
			switch(instr_struct->opc)
			{
				//%s rt, imm_s
				case OPC16_BEQS38:
				case OPC16_BEQZ38:
				case OPC16_BNES38:
				case OPC16_BNEZ38:
					sprintf(instr_struct->disassm, "%s r%u, 0x%x", instr_struct->mnemonic, instr_struct->rt, nds32_sign_extend(instr_struct->imm1_8<<1, 9, 32)+PC);
					break;
			}
			break;
		
		case TYPE16_45:
			switch(instr_struct->opc)
			{
				//%s rt, rb
				case OPC16_ADD45:
				case OPC16_SUB45:
					sprintf(instr_struct->disassm, "%s r%u, r%u", instr_struct->mnemonic, reg_4T5(instr_struct->rt), instr_struct->rb);
					break;
				
				//%s ra, rb
				case OPC16_SLT45:
				case OPC16_SLTS45:
					sprintf(instr_struct->disassm, "%s r%u, r%u", instr_struct->mnemonic, reg_4T5(instr_struct->ra), instr_struct->rb);
					break;
				
				//%s rt, imm_u
				case OPC16_ADDI45:
				case OPC16_SRAI45:
				case OPC16_SRLI45:
				case OPC16_SUBI45:
					sprintf(instr_struct->disassm, "%s r%u, 0x%x", instr_struct->mnemonic, reg_4T5(instr_struct->rt), instr_struct->imm1_5);
					break;
				
				case OPC16_SLTI45:
				case OPC16_SLTSI45:
					sprintf(instr_struct->disassm, "%s r%u, 0x%x", instr_struct->mnemonic, reg_4T5(instr_struct->ra), instr_struct->imm1_5);
			}
			break;
		
		case TYPE16_450:
			switch(instr_struct->opc)
			{
				//%s rt, [ra]
				case OPC16_LWI450:
				case OPC16_SWI450:
					sprintf(instr_struct->disassm, "%s r%u, [r%u]", instr_struct->mnemonic, reg_4T5(instr_struct->rt), instr_struct->ra);
					break;
			}
			break;
		
		case TYPE16_5:
			switch(instr_struct->opc)
			{
				//%s rb
				case OPC16_JR5:
				case OPC16_JRAL5:
				case OPC16_RET5:
					sprintf(instr_struct->disassm, "%s r%u", instr_struct->mnemonic, instr_struct->rb);
					break;
			}
			break;
		
		case TYPE16_55:
			switch(instr_struct->opc)
			{
				//%s rt, ra
				case OPC16_MOV55:
					sprintf(instr_struct->disassm, "%s r%u, r%u", instr_struct->mnemonic, instr_struct->rt, instr_struct->ra);
					break;
				
				//%s rt, imm_s
				case OPC16_MOVI55:
					sprintf(instr_struct->disassm, "%s r%u, 0x%x", instr_struct->mnemonic, instr_struct->rt, nds32_sign_extend(instr_struct->imm1_5, 5, 32));
					break;
			}
			break;
		
		case TYPE16_8:
			switch(instr_struct->opc)
			{
				//%s imm_s
				case OPC16_BEQZS8:
				case OPC16_BNEZS8:
					sprintf(instr_struct->disassm, "%s 0x%x", instr_struct->mnemonic, nds32_sign_extend(instr_struct->imm1_8<<1, 9, 32)+PC);
					break;
				
				//%s imm_s<<1
				case OPC16_J8:
					sprintf(instr_struct->disassm, "%s 0x%x", instr_struct->mnemonic, nds32_sign_extend(instr_struct->imm1_8<<1, 9, 32)+PC);
					break;
			}
			break;
	}
}

__attribute__((__unused__)) uint8_t translate_to_32(instr_t * const instr_struct) 
{
	if(instr_struct->width==WIDTH32)
		return 0;
	
	//ordered as in § 3.1 but grouped by opc_length
	
	switch(instr_struct->opc_length)
	{
		case 4:
			switch(instr_struct->opc)
			{
				case OPC16_BEQS38:
					instr_struct->opc=OPC_BR1;
					instr_struct->sub=SUB_BR1_BEQ;
					instr_struct->ra=5; //reg5 implied (not 15)
					instr_struct->imm1_14=nds32_sign_extend(instr_struct->imm1_8, 8, 14);
					break;
				
				case OPC16_BNES38:
					instr_struct->opc=OPC_BR1;
					instr_struct->sub=SUB_BR1_BNE;
					instr_struct->ra=5; //reg5 implied (not 15)
					instr_struct->imm1_14=nds32_sign_extend(instr_struct->imm1_8, 8, 14);
					break;
				
				case OPC16_BEQZ38:
					instr_struct->opc=OPC_BR2;
					instr_struct->sub=SUB_BR2_BEQZ;
					instr_struct->imm1_16=nds32_sign_extend(instr_struct->imm1_8, 8, 20);
					break;
				
				case OPC16_BNEZ38:
					instr_struct->opc=OPC_BR2;
					instr_struct->sub=SUB_BR2_BNEZ;
					instr_struct->imm1_16=nds32_sign_extend(instr_struct->imm1_8, 8, 20);
					break;
				
				case OPC16_XWI37:
				switch(instr_struct->sub)
				{
					
					case SUB_XWI37_LWI37:
						instr_struct->opc=OPC_LWI;
						instr_struct->ra=28; //fp
						instr_struct->imm1_15=instr_struct->imm1_7;
						break;
					
					case SUB_XWI37_SWI37:
						instr_struct->opc=OPC_SWI;
						instr_struct->ra=28; //fp
						instr_struct->imm1_15=instr_struct->imm1_7;
						break;
					
					default:
						ERRX(1, "unknown in translate16->32");
						break;
				}
				break;
				
				
				case OPC16_XWI37SP:
				switch(instr_struct->sub)
				{
					
					case SUB_XWI37SP_LWI37SP:
						instr_struct->opc=OPC_LWI;
						instr_struct->ra=31; //sp
						instr_struct->imm1_15=instr_struct->imm1_7; //no shift here!
						break;
					
					case SUB_XWI37SP_SWI37SP:
						instr_struct->opc=OPC_SWI;
						instr_struct->ra=31; //sp
						instr_struct->imm1_15=instr_struct->imm1_7; //no shift here!
						break;
					
					default:
						ERRX(1, "unknown in translate16->32");
						break;
				}
				break;
				
				default:
						ERRX(1, "unknown in translate16->32");
						break;
			}
			break;
		
		case 5:
			switch(instr_struct->opc)
			{
				case OPC16_MOVI55:
					instr_struct->opc=OPC_MOVI;
					instr_struct->imm1_20=nds32_sign_extend(instr_struct->imm1_5, 5, 20);
					break;
				
				case OPC16_MOV55:
					instr_struct->opc=OPC_ADDI;
					instr_struct->imm1_15=0;
					break;
				
				case OPC16_ADDI10S:
					instr_struct->opc=OPC_ADDI;
					instr_struct->rt=31; //sp
					instr_struct->ra=31; //sp
					instr_struct->imm1_15=nds32_sign_extend(instr_struct->imm1_10, 10, 15);
					break;
				default:
					ERRX(1, "unknown in translate16->32");
					break;
			}
			break;
		
		case 6:
			switch(instr_struct->opc)
			{
				case OPC16_ADDI45:
					instr_struct->opc=OPC_ADDI;
					instr_struct->rt=reg_4T5(instr_struct->rt);
					instr_struct->ra=instr_struct->rt;
					instr_struct->imm1_15=instr_struct->imm1_5;
					break;
				
				case OPC16_ADDI333:
					instr_struct->opc=OPC_ADDI;
					instr_struct->imm1_15=instr_struct->imm1_3;
					break;
				
				case OPC16_SUBI45:
					instr_struct->opc=OPC_ADDI;
					instr_struct->rt=reg_4T5(instr_struct->rt);
					instr_struct->ra=instr_struct->rt;
					instr_struct->imm1_15=NEG(instr_struct->imm1_5);
					break;
						
				case OPC16_SUBI333:
					instr_struct->opc=OPC_ADDI;
					instr_struct->imm1_15=NEG(instr_struct->imm1_3);
					break;
				
				case OPC16_ADD45:
					instr_struct->opc=OPC_ALU_1;
					instr_struct->sub=SUB_ALU_1_ADD;
					instr_struct->rt=reg_4T5(instr_struct->rt);
					instr_struct->ra=instr_struct->rt;
					break;
				
				case OPC16_ADD333:
					instr_struct->opc=OPC_ALU_1;
					instr_struct->sub=SUB_ALU_1_ADD;
					break;
				
				case OPC16_SUB45:
					instr_struct->opc=OPC_ALU_1;
					instr_struct->sub=SUB_ALU_1_SUB;
					instr_struct->rt=reg_4T5(instr_struct->rt);
					instr_struct->ra=instr_struct->rt;
					break;
						
				case OPC16_SUB333:
					instr_struct->opc=OPC_ALU_1;
					instr_struct->sub=SUB_ALU_1_SUB;
					break;
					
				case OPC16_SRAI45:
					instr_struct->opc=OPC_ALU_1;
					instr_struct->sub=SUB_ALU_1_SRAI;
					instr_struct->rt=reg_4T5(instr_struct->rt);
					instr_struct->ra=instr_struct->rt;
					//imm_5 -> imm_5
					break;
				
				case OPC16_SRLI45:
					instr_struct->opc=OPC_ALU_1;
					instr_struct->sub=SUB_ALU_1_SRLI;
					instr_struct->rt=reg_4T5(instr_struct->rt);
					instr_struct->ra=instr_struct->rt;
					break;
				
				case OPC16_SLLI333:
					instr_struct->opc=OPC_ALU_1;
					instr_struct->sub=SUB_ALU_1_SLLI;
					instr_struct->imm1_5=instr_struct->imm1_3;
					break;
				
				case OPC16_BFMI333:
					switch(instr_struct->sub)
					{
						
						case SUB_BMFI333_ZEB33:
							instr_struct->opc=OPC_ANDI;
							instr_struct->imm1_15=0xFF;
							break;
						
						case SUB_BMFI333_ZEH33:
							instr_struct->opc=OPC_ALU_1;
							instr_struct->sub=SUB_ALU_1_ZEH;
							break;
						
						case SUB_BMFI333_SEB33:
							instr_struct->opc=OPC_ALU_1;
							instr_struct->sub=SUB_ALU_1_SEB;
							break;
						
						case SUB_BMFI333_SEH33:
							instr_struct->opc=OPC_ALU_1;
							instr_struct->sub=SUB_ALU_1_SEH;
							break;
						
						case SUB_BMFI333_XLSB33:
							instr_struct->opc=OPC_ANDI;
							instr_struct->imm1_15=1;
							break;
						
						default:
							ERRX(1, "unknown in translate16->32");
						break;
					}
					break;
				
				case OPC16_SLTI45:
					instr_struct->opc=OPC_SLTI;
					instr_struct->ra=reg_4T5(instr_struct->ra);
					instr_struct->rt=15;
					instr_struct->imm1_15=instr_struct->imm1_5;
					break;
				
				case OPC16_SLTSI45:
					instr_struct->opc=OPC_SLTSI;
					instr_struct->ra=reg_4T5(instr_struct->ra);
					instr_struct->rt=15;
					instr_struct->imm1_15=instr_struct->imm1_5;
					break;
				
				case OPC16_SLT45:
					instr_struct->opc=OPC_ALU_1;
					instr_struct->sub=SUB_ALU_1_SLT;
					instr_struct->rt=15;
					instr_struct->ra=reg_4T5(instr_struct->ra);
					break;
				
				case OPC16_SLTS45:
					instr_struct->opc=OPC_ALU_1;
					instr_struct->sub=SUB_ALU_1_SLTS;
					instr_struct->rt=15;
					instr_struct->ra=reg_4T5(instr_struct->ra);
					break;
					
				case OPC16_LWI450:
					instr_struct->opc=OPC_LWI;
					instr_struct->rt=reg_4T5(instr_struct->rt);
					instr_struct->imm1_15=0;
					break;
				
				case OPC16_LWI333:
					instr_struct->opc=OPC_LWI;
					instr_struct->imm1_15=instr_struct->imm1_3;
					break;
				
				case OPC16_LWI333_bi:
					instr_struct->opc=OPC_LWI_bi;
					instr_struct->imm1_15=instr_struct->imm1_3;
					break;
				
				case OPC16_LHI333:
					instr_struct->opc=OPC_LHI;
					instr_struct->imm1_15=instr_struct->imm1_3;
					break;
				
				case OPC16_LBI333:
					instr_struct->opc=OPC_LBI;
					instr_struct->imm1_15=instr_struct->imm1_3;
					break;
				
				case OPC16_SWI450:
					instr_struct->opc=OPC_SWI;
					instr_struct->rt=reg_4T5(instr_struct->rt);
					instr_struct->imm1_15=0;
					break;
				
				case OPC16_SWI333:
					instr_struct->opc=OPC_SWI;
					instr_struct->imm1_15=instr_struct->imm1_3;
					break;
				
				case OPC16_SWI333_bi:
					instr_struct->opc=OPC_SWI_bi;
					instr_struct->imm1_15=instr_struct->imm1_3;
					break;
				
				case OPC16_SHI333:
					instr_struct->opc=OPC_SHI;
					instr_struct->imm1_15=instr_struct->imm1_3;
					break;
				
				case OPC16_SBI333:
					instr_struct->opc=OPC_SBI;
					instr_struct->imm1_15=instr_struct->imm1_3;
					break;
				
				default:
					ERRX(1, "unknown in translate16->32");
						break;
				
			}
			break;
			
		case 7:
			switch(instr_struct->opc)
			{
				case OPC16_BEQZS8:
					instr_struct->opc=OPC_BR2;
					instr_struct->sub=SUB_BR2_BEQZ;
					instr_struct->rt=15;
					instr_struct->imm1_16=nds32_sign_extend(instr_struct->imm1_8, 8, 16);
					break;
				
				case OPC16_BNEZS8:
					instr_struct->opc=OPC_BR2;
					instr_struct->sub=SUB_BR2_BNEZ;
					instr_struct->rt=15;
					instr_struct->imm1_16=nds32_sign_extend(instr_struct->imm1_8, 8, 16);
					break;
			
				case OPC16_J8:
					instr_struct->opc=OPC_J;
					instr_struct->imm1_24=nds32_sign_extend(instr_struct->imm1_8, 8, 24);
					break;
				
				default:
					ERRX(1, "unknown in translate16->32");
						break;
			}
			break;
					
		case 10:
			switch(instr_struct->opc)
			{
				case OPC16_JR5:
					instr_struct->opc=OPC_JREG;
					instr_struct->sub=SUB_JREG_JR;
					break;
				
				case OPC16_RET5:
					instr_struct->opc=OPC_JREG;
					instr_struct->sub=SUB_JREG_RET;
					break;
				
				case OPC16_JRAL5:
					instr_struct->opc=OPC_JREG;
					instr_struct->sub=SUB_JREG_JRAL;
					instr_struct->rt=30;
					break;
				
				default:
					ERRX(1, "unknown in translate16->32");
						break;
			}
			break;
			
			default:
				ERRX(1, "unknown in translate16->32");
				break;
	}
	
	instr_struct->width=WIDTH32;
	instr_struct->was_16bit_instr=1;
	
	//printf("instr translated to opc 0x%x (sub 0x%x)\n", instr_struct->opc, instr_struct->sub);
	
	return 0;
}

uint8_t decode_16(const uint16_t instr, instr_t * const instr_struct, const uint32_t PC, const bool decode_only)
{
	strcpy(instr_struct->disassm, "unknown");
	
	const uint8_t opc_length[]={10, 7, 6, 5, 4};
	unsigned int index_opc_length=0, i, found=0;
	uint16_t length, mask;
	uint16_t opc;
	
	for(index_opc_length=0; index_opc_length<sizeof(opc_length)/sizeof(uint8_t); index_opc_length++)
	{
		length=opc_length[index_opc_length];
		mask=((1<<15)-1)&~((1<<(16-1-length))-1);
		opc=(instr&mask)>>(16-1-length);
		
		for(i=0; opc16_list[i].mnemonic!=NULL; i++)
		{
			if(opc16_list[i].opc_length==opc_length[index_opc_length] && opc16_list[i].opc==opc)
			{
				found=1;
				break;
			}
		}
		if(found)
			break;
	}
	if(!found)
	{
#ifdef DISASSM_WARN_UNKNOWN
		printf("decode_16: unknown opc 0x%x @0x%x\n", opc, PC);
#endif
		return 1;
	}
	
	opc16_type_t type=opc16_list[i].type;
	
	instr_struct->width=WIDTH16;
	instr_struct->mnemonic=opc16_list[i].mnemonic;
	instr_struct->opc_length=length;
	
	fill_args_instr_type16(instr, opc, type, instr_struct);
	
	if(!decode_only)
	{
		if(opc16_list[i].has_sub)
		{
			found=0;
			for(i=0; opc16_sub_list[i].mnemonic!=NULL; i++)
			{
				if(opc16_sub_list[i].opc==opc && opc16_sub_list[i].sub==instr_struct->sub)
				{
					found=1;
					break;
				}
			}
			if(!found)
			{
#ifdef DISASSM_WARN_UNKNOWN
				printf("decode_16: unknown sub 0x%02x for opc 0x%x @0x%x\n", instr_struct->sub, instr_struct->opc, PC);
#endif
				return 1;
			}
			
			instr_struct->mnemonic=opc16_sub_list[i].mnemonic;
		}
		
		disassm16(instr_struct, PC);
	}
	
	//turn this on or off depending on what you want to do
#ifdef DISASSM_TRANSLATE_TO_32
	translate_to_32(instr_struct);
#endif
	return 0;
}

