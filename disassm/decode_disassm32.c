/*
This file is part of a disassembler for Andestech NDS32.

(c) 2019-2020 kitten_nb_five

THIS WORK COMES WITHOUT ANY WARRANTY and is released under the AGPL version 3 or later
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "opc32.h"
#include "instruction.h"
#include "sign_extend.h"

#include "opcodes32.h"

extern opcode32_t opc32_list[];
extern sub_opc32_t opc32_sub_list[];

static char * get_usr_name(const uint8_t group, const uint8_t usr)
{
	switch(group)
	{
		case 0:
			if(usr==0)
				return "D0.LO";
			else if(usr==1)
				return "D0.HI";
			else if(usr==2)
				return "D1.LO";
			else if(usr==3)
				return "D1.HI";
			else
				return "<RESERVED>";
			break;
		
		default:
			return "<disassm: unimpl>";
			break;
	}
}


static void fill_args_instr_type32(const uint32_t instr, const uint8_t opc, const opc32_type_t type, instr_t * const instr_struct)
{
	instr_struct->opc=opc;
	instr_struct->type32=type;
	switch(type)
	{
		case TYPE_T0:
			instr_struct->sub=(instr&0x01000000)>>24;
			instr_struct->imm1_24=(instr&0x00FFFFFF);
			break;
		
		case TYPE_T1:
			instr_struct->rt=(instr&0x01F00000)>>20;
			instr_struct->imm1_20=(instr&0x000FFFFF);
			instr_struct->imm1_16=(instr&0x0000FFFF);
			instr_struct->sub=(instr&0x000F0000)>>16;
			break;
			
		case TYPE_T2:
			instr_struct->rt=(instr&0x01F00000)>>20;
			instr_struct->ra=(instr&0x000F8000)>>15;
			instr_struct->sub=(instr&0x00004000)>>14;
			instr_struct->imm1_15=(instr&0x00007FFF);
			instr_struct->imm1_14=(instr&0x00003FFF);
			break;
			
		case TYPE_T3:
			instr_struct->rt=(instr&0x01F00000)>>20;
			instr_struct->ra=(instr&0x000F8000)>>15;
			instr_struct->rb=(instr&0x00007C00)>>10;
			instr_struct->imm1_5=(instr&0x00007C00)>>10;
			instr_struct->imm1_2=(instr&0x00000300)>>8;
			instr_struct->sub=(instr&0x0000000FF);
			instr_struct->sub_10=(instr&0x0000003FF); //10 bit for T3_2
			break;
			
		case TYPE_T4:
			instr_struct->rt=(instr&0x01F00000)>>20;
			instr_struct->ra=(instr&0x000F8000)>>15;
			instr_struct->rb=(instr&0x00007C00)>>10;
			instr_struct->rd=(instr&0x000003E0)>>5;
			instr_struct->rs=(instr&0x000003E0)>>5;
			instr_struct->imm1_5=(instr&0x00007C00)>>10;
			instr_struct->imm2_5=(instr&0x000003E0)>>5;
			instr_struct->sub=(instr&0x0000001F);
			instr_struct->sridx=(instr&0x000FFC00)>>10;
			break;

		case TYPE_T5:
			if(instr_struct->opc==OPC_ALU_2)
			{
				//there is a HACK here: we include GPR in the sub-opcode to distinguish between e.g. MULR64 and MULT64 without modifying to much code
				
				instr_struct->dt=(instr&0x200000000)>>21;
				instr_struct->rt=(instr&0x01F00000)>>20;
				instr_struct->ra=(instr&0x000F8000)>>15;
				instr_struct->rb=(instr&0x00007C00)>>10;
				instr_struct->sub=(instr&0x000003FF); //this has now 10 bits
				
				instr_struct->imm1_5=(instr&0x00007C00)>>10;
				
				instr_struct->usr=(instr&0x000F8000)>>15;
				instr_struct->group=(instr&0x00007C00)>>10;
			}
			else //LSMW
			{
				instr_struct->rb=(instr&0x01F00000)>>20;
				instr_struct->ra=(instr&0x000F8000)>>15;
				instr_struct->re=(instr&0x00007C00)>>10;
				instr_struct->enable=(instr&0x000003C0)>>6;
				instr_struct->sub=(instr&0x0000003F);
			}
			break;
	}
}

void disassm32(instr_t * const instr_struct, const uint32_t PC)
{
	switch(instr_struct->type32)
	{
		case TYPE_T0:
			sprintf(instr_struct->disassm, "%s 0x%x", instr_struct->mnemonic, nds32_sign_extend(instr_struct->imm1_24<<1, 25, 32)+PC);
			break;
		
		case TYPE_T1:
			switch(instr_struct->opc)
			{
				//%s rt, SE(imm20s)
				case OPC_MOVI:
					sprintf(instr_struct->disassm, "%s r%u, 0x%x", instr_struct->mnemonic, instr_struct->rt, nds32_sign_extend(instr_struct->imm1_20, 20, 32));
					break;
				
				//%s rt, imm20u
				case OPC_SETHI:
					sprintf(instr_struct->disassm, "%s r%u, 0x%x", instr_struct->mnemonic, instr_struct->rt, instr_struct->imm1_20);
					break;
				
				case OPC_BR2:
					switch(instr_struct->sub)
					{
						//%s rt, SE(imm16s<<1)+PC
						case SUB_BR2_BEQZ:
						case SUB_BR2_BNEZ:
						case SUB_BR2_BGEZ:
						case SUB_BR2_BLTZ:
						case SUB_BR2_BGTZ:
						case SUB_BR2_BLEZ:
							sprintf(instr_struct->disassm, "%s r%u, 0x%x", instr_struct->mnemonic, instr_struct->rt, nds32_sign_extend(instr_struct->imm1_16<<1, 17, 32)+PC);
					}
					break;
			}
			break;
			
		case TYPE_T2:
			switch(instr_struct->opc)
			{
				//%s rt, ra, SE(imm15s)
				case OPC_ADDI:
				case OPC_SUBRI:
				case OPC_SLTI:
				case OPC_SLTSI:
					sprintf(instr_struct->disassm, "%s r%u, r%u, 0x%x", instr_struct->mnemonic, instr_struct->rt, instr_struct->ra, nds32_sign_extend(instr_struct->imm1_15, 15, 32));
					break;
					
				//%s rt, ra, ZE(imm15u)
				case OPC_ANDI:
				case OPC_ORI:
				case OPC_XORI:
					sprintf(instr_struct->disassm, "%s r%u, r%u, 0x%x", instr_struct->mnemonic, instr_struct->rt, instr_struct->ra, instr_struct->imm1_15);
					break;
				
				//%s rt, [ra+SE(imm15s)]
				case OPC_LBI:
				case OPC_LBSI:
				case OPC_SBI:
					sprintf(instr_struct->disassm, "%s r%u, [r%u + 0x%x]", instr_struct->mnemonic, instr_struct->rt, instr_struct->ra, nds32_sign_extend(instr_struct->imm1_15, 15, 32));
					break;
				
				//%s rt, [ra+SE(imm15s<<1)]
				case OPC_LHI:
				case OPC_LHSI:
				case OPC_SHI:
				sprintf(instr_struct->disassm, "%s r%u, [r%u + 0x%x]", instr_struct->mnemonic, instr_struct->rt, instr_struct->ra, nds32_sign_extend(instr_struct->imm1_15<<1, 16, 32));
					break;
				
				//%s rt, [ra+SE(imm15s<<2)]
				case OPC_LWI:
				case OPC_SWI:
					sprintf(instr_struct->disassm, "%s r%u, [r%u + 0x%x]", instr_struct->mnemonic, instr_struct->rt, instr_struct->ra, nds32_sign_extend(instr_struct->imm1_15<<2, 17, 32));
					break;
				
				//%s rt, [ra], SE(imm15s)]
				case OPC_LBI_bi:
				case OPC_LBSI_bi:
					sprintf(instr_struct->disassm, "%s r%u, [r%u], 0x%x", instr_struct->mnemonic, instr_struct->rt, instr_struct->ra, nds32_sign_extend(instr_struct->imm1_15, 15, 32));
					break;
				
				//%s rt, [ra], SE(imm15s<<1)]
				case OPC_LHI_bi:
				case OPC_LHSI_bi:
				case OPC_SHI_bi:
					sprintf(instr_struct->disassm, "%s r%u, [r%u], 0x%x", instr_struct->mnemonic, instr_struct->rt, instr_struct->ra, nds32_sign_extend(instr_struct->imm1_15<<1, 16, 32));
					break;
				
				//%s rt, [ra], SE(imm15s<<2)]
				case OPC_LWI_bi:
				case OPC_SWI_bi:
					sprintf(instr_struct->disassm, "%s r%u, [r%u], 0x%x", instr_struct->mnemonic, instr_struct->rt, instr_struct->ra, nds32_sign_extend(instr_struct->imm1_15<<2, 17, 32));
					break;
				
				//%s rt, [ra]
				case OPC_SBI_bi:
					sprintf(instr_struct->disassm, "%s r%u, [r%u]", instr_struct->mnemonic, instr_struct->rt, instr_struct->ra);
					break;
				
				case OPC_BR1:
					switch(instr_struct->sub)
					{
						//%s rt, ra, SE(imm14s<<1)+PC
						case SUB_BR1_BEQ:
						case SUB_BR1_BNE:
							sprintf(instr_struct->disassm, "%s r%u, r%u, 0x%x", instr_struct->mnemonic, instr_struct->rt, instr_struct->ra, nds32_sign_extend(instr_struct->imm1_14<<1, 15, 32)+PC);
							break;
					}
					break;
			}
			break;
			
		case TYPE_T3:
			switch(instr_struct->opc)
			{
				case OPC_MEM:
					switch(instr_struct->sub)
					{
						//%s rt, [ra + (rb << imm)]
						case SUB_MEM_LW:
						case SUB_MEM_LH:
						case SUB_MEM_LHS:
						case SUB_MEM_LB:
						case SUB_MEM_LBS:
						case SUB_MEM_SW:
						case SUB_MEM_SH:
						case SUB_MEM_SB:
						case SUB_MEM_LLW:
						case SUB_MEM_SCW:
							sprintf(instr_struct->disassm, "%s r%u, [r%u + (r%u << %u)]", instr_struct->mnemonic, instr_struct->rt, instr_struct->ra, instr_struct->rb, instr_struct->imm1_2);
							break;
						
						//%s rt, [ra], rb << imm
						case SUB_MEM_LW_bi:
						case SUB_MEM_LH_bi:
						case SUB_MEM_LHS_bi:
						case SUB_MEM_LB_bi:
						case SUB_MEM_LBS_bi:
						case SUB_MEM_SW_bi:
						case SUB_MEM_SH_bi:
						case SUB_MEM_SB_bi:
							sprintf(instr_struct->disassm, "%s r%u, [r%u], (r%u << %u)", instr_struct->mnemonic, instr_struct->rt, instr_struct->ra, instr_struct->rb, instr_struct->imm1_2);					
							break;
					}
					break;
			}
			break;
			
		case TYPE_T4:
		switch(instr_struct->opc)
		{
			case OPC_ALU_1:
				switch(instr_struct->sub)
				{
					//%s rt, ra, rb
					case SUB_ALU_1_ADD:
					case SUB_ALU_1_SUB:
					case SUB_ALU_1_AND:
					case SUB_ALU_1_NOR:
					case SUB_ALU_1_OR:
					case SUB_ALU_1_XOR:
					case SUB_ALU_1_SLT:
					case SUB_ALU_1_SLTS:
					case SUB_ALU_1_SVA:
					case SUB_ALU_1_SVS:
					case SUB_ALU_1_SEB:
					case SUB_ALU_1_SEH:
					case SUB_ALU_1_ZEH:
					case SUB_ALU_1_SLL:
					case SUB_ALU_1_SRL:
					case SUB_ALU_1_SRA:
					case SUB_ALU_1_ROTR:
					case SUB_ALU_1_CMOVZ:
					case SUB_ALU_1_CMOVN:
						sprintf(instr_struct->disassm, "%s r%u, r%u, r%u", instr_struct->mnemonic, instr_struct->rt, instr_struct->ra, instr_struct->rb);
						break;
					
					//%s rt, ra, imm5u
					case SUB_ALU_1_SLLI:
					case SUB_ALU_1_SRLI:
					case SUB_ALU_1_SRAI:
					case SUB_ALU_1_ROTRI:
						sprintf(instr_struct->disassm, "%s r%u, r%u, %u", instr_struct->mnemonic, instr_struct->rt, instr_struct->ra, instr_struct->imm1_5);
						break;
					
					//%s rt, rs, ra, rb
					case SUB_ALU_1_DIVR:
					case SUB_ALU_1_DIVSR:
						sprintf(instr_struct->disassm, "%s r%u, r%u, r%u, r%u", instr_struct->mnemonic, instr_struct->rt, instr_struct->rs, instr_struct->ra, instr_struct->rb);
						break;
					
				}
				break;
			
			case OPC_JREG:
				switch(instr_struct->sub)
				{
					//%s rb
					case SUB_JREG_JR:
					case SUB_JREG_RET:
						sprintf(instr_struct->disassm, "%s r%u", instr_struct->mnemonic, instr_struct->rb);
						break;
					
					//%s rb OR %s rt, rb if(rt) 
					case SUB_JREG_JRAL:
						if(instr_struct->rt)
							sprintf(instr_struct->disassm, "%s r%u, r%u", instr_struct->mnemonic, instr_struct->rt, instr_struct->rb);
						else
							sprintf(instr_struct->disassm, "%s r%u", instr_struct->mnemonic, instr_struct->rb);
						break;
				}
				break;
			
			case OPC_MISC:
				switch(instr_struct->sub)
				{
					case SUB_MISC_MFSR:
						sprintf(instr_struct->disassm, "%s r%u, <sysreg>", instr_struct->mnemonic, instr_struct->rt);
						break;
					
					case SUB_MISC_MTSR:
						sprintf(instr_struct->disassm, "%s r%u, <sysreg>", instr_struct->mnemonic, instr_struct->ra);
						break;
					
					//complicated internal processor stuff...
					case SUB_MISC_CCTL:
					case SUB_MISC_MSYNC:
					case SUB_MISC_ISB:
						sprintf(instr_struct->disassm, "%s <something>", instr_struct->mnemonic);
						break;
					
					case SUB_MISC_IRET:
						sprintf(instr_struct->disassm, "%s", instr_struct->mnemonic);
						break;
				}
				break;
			}
			break;
			
		case TYPE_T5:
		switch(instr_struct->opc)
		{
			case OPC_ALU_2:
				switch(instr_struct->sub)
				{
					//%s rt, ra, rb
					case SUB_ALU_2_MUL:
					case SUB_ALU_2_MIN:
					case SUB_ALU_2_BSP:
					case SUB_ALU_2_MAX:
						sprintf(instr_struct->disassm, "%s r%u, r%u, r%u", instr_struct->mnemonic, instr_struct->rt, instr_struct->ra, instr_struct->rb);
						break;
					
					//%s dt, ra, rb
					case SUB_ALU_2_MULTS64:
					case SUB_ALU_2_MULT64:
					case SUB_ALU_2_MADDS64:
					case SUB_ALU_2_MADD64:
					case SUB_ALU_2_MSUBS64:
					case SUB_ALU_2_MSUB64:
					case SUB_ALU_2_MULT32:
					case SUB_ALU_2_MADD32:
					case SUB_ALU_2_MSUB32:
					case SUB_ALU_2_DIV:
					case SUB_ALU_2_DIVS:
						sprintf(instr_struct->disassm, "%s d%u, r%u, r%u", instr_struct->mnemonic, instr_struct->dt, instr_struct->ra, instr_struct->rb);
						break;
					
					//%s rt, ra, rb
					case SUB_ALU_2_MADDR32:
					case SUB_ALU_2_MSUBR32:
					case SUB_ALU_2_MULR64:
					case SUB_ALU_2_MULSR64:
						sprintf(instr_struct->disassm, "%s r%u, r%u, r%u", instr_struct->mnemonic, instr_struct->rt, instr_struct->ra, instr_struct->rb);
						break;
					
					//%s rt, ra, imm5u
					case SUB_ALU_2_BSET:
					case SUB_ALU_2_BTST:
						sprintf(instr_struct->disassm, "%s r%u, r%u, %u", instr_struct->mnemonic, instr_struct->rt, instr_struct->ra, instr_struct->imm1_5);
						break;
					
					//%s rt, ra
					case SUB_ALU_2_ABS:
						sprintf(instr_struct->disassm, "%s r%u, r%u", instr_struct->mnemonic, instr_struct->rt, instr_struct->ra);
						break;
					
					//%s rt, USR_Name
					case SUB_ALU_2_MFUSR:
					case SUB_ALU_2_MTUSR:
						sprintf(instr_struct->disassm, "%s r%u, %s", instr_struct->mnemonic, instr_struct->rt, get_usr_name(instr_struct->group, instr_struct->usr));
						break;
				}
				break;
			
			case OPC_LSMW:
				switch(instr_struct->sub)
				{
					//%s rb, [ra], re, enable
					case SUB_LSMW_LMW_ad:
					case SUB_LSMW_LMW_adm:
					case SUB_LSMW_LMW_ai:
					case SUB_LSMW_LMW_aim:
					case SUB_LSMW_LMW_bd:
					case SUB_LSMW_LMW_bdm:
					case SUB_LSMW_LMW_bi:
					case SUB_LSMW_LMW_bim:
						sprintf(instr_struct->disassm, "%s r%u, [r%u], r%u, %u", instr_struct->mnemonic, instr_struct->rb, instr_struct->ra, instr_struct->re, instr_struct->enable);
						break;
					
					//%s rb, [ra], re, enable
					case SUB_LSMW_SMW_ad:
					case SUB_LSMW_SMW_adm:
					case SUB_LSMW_SMW_ai:
					case SUB_LSMW_SMW_aim:
					case SUB_LSMW_SMW_bd:
					case SUB_LSMW_SMW_bdm:
					case SUB_LSMW_SMW_bi:
					case SUB_LSMW_SMW_bim:
						sprintf(instr_struct->disassm, "%s r%u, [r%u], r%u, %u", instr_struct->mnemonic, instr_struct->rb, instr_struct->ra, instr_struct->re, instr_struct->enable);
						break;
					
					//%s rb, [ra], re, enable
					case SUB_LSMW_SMWA_adm:
					case SUB_LSMW_SMWA_bdm:
					case SUB_LSMW_LMWA_bim:
						sprintf(instr_struct->disassm, "%s r%u, [r%u], r%u, %u", instr_struct->mnemonic, instr_struct->rb, instr_struct->ra, instr_struct->re, instr_struct->enable);
						break;
				}
				break;
			}
			break;
	}
}

uint8_t decode_32(const uint32_t instr, instr_t * const instr_struct, const uint32_t PC)
{
	strcpy(instr_struct->disassm, "unknown");
	
	uint8_t opc=(instr&0x7E000000)>>25;
	
	unsigned int i, found=0;
	for(i=0; opc32_list[i].mnemonic!=NULL; i++)
	{
		if(opc32_list[i].opc==opc)
		{
			found=1;
			break;
		}
	}
	if(!found)
	{
#ifdef DISASSM_WARN_UNKNOWN
		printf("decode_32: unknown opc 0x%02x @0x%x\n", opc, PC);
#endif
		return 1;
	}
	opc32_type_t type=opc32_list[i].type;
	instr_struct->width=WIDTH32;
	instr_struct->mnemonic=opc32_list[i].mnemonic;
	
	fill_args_instr_type32(instr, opc, type, instr_struct);
	
	if(opc32_list[i].has_sub)
	{
		found=0;
		for(i=0; opc32_sub_list[i].mnemonic!=NULL; i++)
		{
			if(opc32_sub_list[i].opc==opc && opc32_sub_list[i].sub==instr_struct->sub)
			{
				found=1;
				break;
			}
		}
		if(!found)
		{
#ifdef DISASSM_WARN_UNKNOWN
			printf("decode_32: unknown sub 0x%02x for opc 0x%02x @0x%x\n", instr_struct->sub, instr_struct->opc, PC);
#endif
			return 1;
		}
			
		instr_struct->mnemonic=opc32_sub_list[i].mnemonic;
	}
	
	disassm32(instr_struct, PC);
	
	return 0;
}

