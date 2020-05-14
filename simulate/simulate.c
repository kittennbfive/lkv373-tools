/*
This file is part of the simulator for the LKV373
(c) 2019-2020 by kitten_nb_five
freenode #lkv373a

licence: AGPL v3 or later

THIS PROGRAM COMES WITHOUT ANY WARRANTY!
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "instruction.h"
#include "decode_disassm.h"
#include "opcodes32.h"
#include "sign_extend.h"
#include "debug.h"

#include "simulate.h"
#include "breakpoints.h"
#include "memory.h"
#include "endless_loop_detect.h"
#include "mem_rw.h"
#include "cmd_parser.h"
#include "dispatcher.h"
#include "disassembly_view.h"
#include "parse_hex.h"
#include "my_err.h"
#include "special_regs.h"
#include "timer.h"
#include "interrupt_ctrl.h"
#include "verbosity.h"

//registers
static uint32_t regs[32];

usr_t usr[2]; //are these used on lkv373?

//PC
static uint32_t PC;

void init_sim(char const * const filename)
{
	memset(regs, 0, 32*sizeof(uint32_t));
	memset(usr, 0, 2*sizeof(usr_t));
	
	memory_init();
	read_memory_from_file(filename, 0x00);
	
	PC=0;
	
	history_init();
}

uint32_t get_register(const uint8_t i)
{
	return regs[i];
}

usr_t get_usr(const uint8_t i)
{
	return usr[i];
}

uint32_t get_stack_pointer(void)
{
	return regs[REG_SP];
}

uint32_t get_PC(void)
{
	return PC;
}

void set_PC(const uint32_t addr)
{
	PC=addr;
}

uint32_t get_bits(const uint32_t val, const uint8_t to, const uint8_t from)
{
	if(from>to)
		ERRX(1, "from>to\n");
		
	return (val&((((uint64_t)1<<(to+1))-1)&(~((1<<from)-1))))>>from;
}

void set_bits(uint32_t * const val, const uint8_t to, const uint8_t from, const uint32_t bits)
{
	if(from>to)
		ERRX(1, "from>to\n");
	
	if(from==to)
		(*val)=(((*val)&~(1<<from))|(bits<<from));
	else
		(*val)=(((*val)&(~((((uint64_t)1<<(to+1))-1)&(~((1<<from)-1)))))|(bits<<from));
}

typedef enum
{
	NO_BRANCH,
	BR_GOTO,
	BR_CALL,
	BR_RET,
	BR_IRET
} is_branch_t;

sim_t simulate(instr_t const * const instr, const bool ignore_breakpoints)
{
	bool PC_manipulated=false;
	bool stop=false;
	
	if(!ignore_breakpoints)
		simulate_check_for_breakpoints(&PC, regs, &PC_manipulated, &stop);
	
	if(PC_manipulated)
		return SIM_NO_ERROR;
	
	if(stop)
		return SIM_STOPPED_ON_BP;
	
	uint32_t PC_old=PC;
	is_branch_t is_branch=NO_BRANCH;
	
	switch(instr->opc)
	{
		case OPC_SETHI:
			regs[instr->rt]=instr->imm1_20<<12;
			break;
			
		case OPC_ORI:
			regs[instr->rt]=regs[instr->ra]|instr->imm1_15;
			break;
		
		case OPC_MOVI:
			regs[instr->rt]=nds32_sign_extend(instr->imm1_20, 20, 32);
			break;
		
		case OPC_LBI:
		{
			uint32_t addr;
			addr=regs[instr->ra]+nds32_sign_extend(instr->imm1_15, 15, 32); //no shift
			mem_byte_t val=memory_get_byte(addr, &stop);
			if(stop && !ignore_breakpoints)
				return SIM_STOPPED_ON_BP;
			if(!val.is_initialized)
				return SIM_READ_FROM_UNINITIALIZED;
			regs[instr->rt]=val.val;
			break;
		}
		
		case OPC_LBI_bi:
		{
			uint32_t addr;
			addr=regs[instr->ra]+nds32_sign_extend(instr->imm1_15, 15, 32); //no shift
			mem_byte_t val=memory_get_byte(regs[instr->ra], &stop);
			if(stop && !ignore_breakpoints)
				return SIM_STOPPED_ON_BP;
			if(!val.is_initialized)
				return SIM_READ_FROM_UNINITIALIZED;
			regs[instr->rt]=val.val;
			regs[instr->ra]=addr;
			break;
		}
		
		case OPC_LBSI:
		{
			uint32_t addr;
			addr=regs[instr->ra]+nds32_sign_extend(instr->imm1_15, 15, 32); //no shift
			mem_byte_t val=memory_get_byte(addr, &stop);
			if(stop && !ignore_breakpoints)
				return SIM_STOPPED_ON_BP;
			if(!val.is_initialized)
				return SIM_READ_FROM_UNINITIALIZED;
			regs[instr->rt]=nds32_sign_extend(val.val, 8 , 32);
			break;
		}
		
		case OPC_LBSI_bi:
		{
			uint32_t addr_update;
			addr_update=regs[instr->ra]+nds32_sign_extend(instr->imm1_15, 15, 32); //no shift
			mem_byte_t val=memory_get_byte(regs[instr->ra], &stop);
			if(stop && !ignore_breakpoints)
				return SIM_STOPPED_ON_BP;
			if(!val.is_initialized)
				return SIM_READ_FROM_UNINITIALIZED;
			regs[instr->rt]=nds32_sign_extend(val.val, 8 , 32);
			regs[instr->ra]=addr_update;
			break;
		}
		
		case OPC_SBI:
		{
			uint32_t addr;
			addr=regs[instr->ra]+nds32_sign_extend(instr->imm1_15, 15, 32);
			memory_set_byte(regs[instr->rt]&0xff, addr, &stop, false);
			break;
		}

		case OPC_SBI_bi:
		{
			uint32_t addr_update;
			addr_update=regs[instr->ra]+nds32_sign_extend(instr->imm1_15, 15, 32);
			memory_set_byte(regs[instr->rt]&0xff, regs[instr->ra], &stop, false);
			regs[instr->ra]=addr_update;
			break;
		}
		
		case OPC_LWI:
		{
			uint32_t addr;
			addr=regs[instr->ra]+nds32_sign_extend(instr->imm1_15<<2, 17, 32);
			mem_word_t val=memory_get_word(addr, &stop);
			if(stop && !ignore_breakpoints)
				return SIM_STOPPED_ON_BP;
			if(!val.is_initialized)
				return SIM_READ_FROM_UNINITIALIZED;
			regs[instr->rt]=val.val;
			break;
		}
		
		case OPC_LWI_bi:
		{
			uint32_t addr_update;
			addr_update=regs[instr->ra]+nds32_sign_extend(instr->imm1_15<<2, 17, 32);
			mem_word_t val=memory_get_word(regs[instr->ra], &stop);
			if(stop && !ignore_breakpoints)
				return SIM_STOPPED_ON_BP;
			if(!val.is_initialized)
				return SIM_READ_FROM_UNINITIALIZED;
			regs[instr->rt]=val.val;
			regs[instr->ra]=addr_update;
			break;
		}
		
		case OPC_SWI:
		{
			uint32_t addr;
			addr=regs[instr->ra]+nds32_sign_extend(instr->imm1_15<<2, 17, 32);
			memory_set_word(regs[instr->rt], addr, &stop, false);
			break;
		}
		
		case OPC_SWI_bi:
		{
			uint32_t addr_update;
			addr_update=regs[instr->ra]+nds32_sign_extend(instr->imm1_15<<2, 17, 32);
			memory_set_word(regs[instr->rt], regs[instr->ra], &stop, false);
			regs[instr->ra]=addr_update;
			break;
		}
		
		case OPC_LHI:
		{
			uint32_t addr;
			addr=regs[instr->ra]+nds32_sign_extend(instr->imm1_15<<1, 16, 32);
			mem_halfword_t val=memory_get_halfword(addr, &stop);
			if(stop && !ignore_breakpoints)
				return SIM_STOPPED_ON_BP;
			if(!val.is_initialized)
				return SIM_READ_FROM_UNINITIALIZED;
			regs[instr->rt]=val.val;
			break;
		}
		
		case OPC_LHSI:
		{
			uint32_t addr;
			addr=regs[instr->ra]+nds32_sign_extend(instr->imm1_15<<1, 16, 32);
			mem_halfword_t val=memory_get_halfword(addr, &stop);
			if(stop && !ignore_breakpoints)
				return SIM_STOPPED_ON_BP;
			if(!val.is_initialized)
				return SIM_READ_FROM_UNINITIALIZED;
			regs[instr->rt]=nds32_sign_extend(val.val, 16, 32);
			break;
		}
		
		
		case OPC_SHI:
		{
			uint32_t addr;
			addr=regs[instr->ra]+nds32_sign_extend(instr->imm1_15<<1, 16, 32);
			memory_set_halfword(regs[instr->rt], addr, &stop, false);
			break;
		}
		
		case OPC_ALU_1:
			switch(instr->sub)
			{
				case SUB_ALU_1_OR:
					regs[instr->rt]=regs[instr->ra]|regs[instr->rb];
					break;
					
				case SUB_ALU_1_NOR:
					regs[instr->rt]=~(regs[instr->ra]|regs[instr->rb]);
					break;
				
				case SUB_ALU_1_AND:
					regs[instr->rt]=regs[instr->ra]&regs[instr->rb];
					break;
				
				case SUB_ALU_1_SLT:
					if(regs[instr->ra]<regs[instr->rb])
						regs[instr->rt]=1;
					else
						regs[instr->rt]=0;
					break;
					
				case SUB_ALU_1_SLTS:
					if((signed)regs[instr->ra]<(signed)regs[instr->rb])
						regs[instr->rt]=1;
					else
						regs[instr->rt]=0;
					break;
				
				case SUB_ALU_1_SUB:
					regs[instr->rt]=regs[instr->ra]-regs[instr->rb];
					break;
				
				case SUB_ALU_1_ADD:
					regs[instr->rt]=regs[instr->ra]+regs[instr->rb];
					break;
				
				case SUB_ALU_1_SRAI:
					regs[instr->rt]=((signed)regs[instr->ra])>>instr->imm1_5;
					break;
					
				case SUB_ALU_1_SRLI:
					regs[instr->rt]=regs[instr->ra]>>instr->imm1_5;
					break;
				
				case SUB_ALU_1_SLL:
					regs[instr->rt]=regs[instr->ra]<<(regs[instr->rb]&0x1f);
					break;
				
				case SUB_ALU_1_SRL:
					regs[instr->rt]=regs[instr->ra]>>(regs[instr->rb]&0x1f);
					break;
				
				case SUB_ALU_1_SEB:
					regs[instr->rt]=nds32_sign_extend(regs[instr->ra]&0xff, 8, 32);
					break;
				
				case SUB_ALU_1_ZEH:
					regs[instr->rt]=regs[instr->ra]&0xffff;
					break;
				
				case SUB_ALU_1_SLLI:
					regs[instr->rt]=regs[instr->ra]<<instr->imm1_5;
					break;
				
				case SUB_ALU_1_CMOVZ:
					if(regs[instr->rb]==0)
						regs[instr->rt]=regs[instr->ra];
					break;
				
				case SUB_ALU_1_CMOVN:
					if(regs[instr->rb]!=0)
						regs[instr->rt]=regs[instr->ra];
					break;
				
				case SUB_ALU_1_DIVR:
					if(regs[instr->rb]==0)
					{
						MSG(MSG_ALWAYS, "DIV BY ZERO\n");
						return SIM_GENERIC_ERROR;
					}
					regs[instr->rt]=regs[instr->ra]/regs[instr->rb];
					regs[instr->rs]=regs[instr->ra]%regs[instr->rb];
					break;
				
				case SUB_ALU_1_DIVSR:
					if(regs[instr->rb]==0)
					{
						MSG(MSG_ALWAYS, "DIV BY ZERO\n");
						return SIM_GENERIC_ERROR;
					}
					regs[instr->rt]=(signed)regs[instr->ra]/(signed)regs[instr->rb];
					regs[instr->rs]=(signed)regs[instr->ra]%(signed)regs[instr->rb];
					break;
				
				case SUB_ALU_1_XOR:
					regs[instr->rt]=regs[instr->ra]^regs[instr->rb];
					break;
				
				default:
					MSG(MSG_ALWAYS, "simulate: unimplemented sub 0x%x for OPC_ALU_1 (%s)\n", instr->sub, instr->mnemonic);
					return SIM_UNIMPLEMENTED;
			}
			break;
		
		case OPC_ALU_2:
			switch(instr->sub)
			{
				case SUB_ALU_2_BSET:
					regs[instr->rt]=regs[instr->ra]|(1<<instr->imm1_5);
					break;
					
				case SUB_ALU_2_MADD32: //DT
				{
					if(instr->dt!=0 && instr->dt!=1)
						ERRX(1, "ALU_2_MADD32: invalid dt\n");
					uint64_t Mresult=regs[instr->ra]*regs[instr->rb];
					usr[instr->dt].L+=Mresult&0xffffffff; //ADD!
					break;
				}
				
				case SUB_ALU_2_MUL:
				{
					uint64_t result=regs[instr->ra]*regs[instr->rb];
					regs[instr->rt]=result&0xffffffff;
					break;
				}
				
				case SUB_ALU_2_MADDR32:
				{
					uint64_t Mresult=regs[instr->ra]*regs[instr->rb];
					regs[instr->rt]+=Mresult&0xffffffff; //ADD!
					break;
				}
				
				case SUB_ALU_2_BTST:
					if(regs[instr->ra]&(1<<instr->imm1_5))
						regs[instr->rt]=1;
					else
						regs[instr->rt]=0;
					break;
				
				case SUB_ALU_2_MULR64:
				{
					uint64_t Mresult=regs[instr->ra]*regs[instr->rb];
					regs[((regs[instr->rt]&0xe)<<1)|1]=Mresult>>32;
					regs[((regs[instr->rt]&0xe)<<1)|0]=Mresult&0xffffffff;
					break;
				}
				
				case SUB_ALU_2_MSUBR32:
				{
					uint64_t Mresult=regs[instr->ra]*regs[instr->rb];
					regs[instr->rt]-=Mresult&0xffffffff; //SUBSTRACT!
					break;
				}
				
				case SUB_ALU_2_MFUSR:
					if(instr->group!=0)
					{
						MSG(MSG_ALWAYS, "ALU_2_MFUSR: unsupported group %u\n", instr->group);
						return SIM_GENERIC_ERROR;
					}
					switch(instr->usr)
					{
						case 0:
							regs[instr->rt]=usr[0].L;
							break;
						case 1:
							regs[instr->rt]=usr[0].H;
							break;
						case 2:
							regs[instr->rt]=usr[1].L;
							break;
						case 3:
							regs[instr->rt]=usr[1].H;
							break;
						case 31:
							regs[instr->rt]=PC;
							break;
						default:
							MSG(MSG_ALWAYS, "ALU_2_MFUSR: invalid usr value %u\n", instr->group);
							return SIM_GENERIC_ERROR;
					}
					break;
				
				case SUB_ALU_2_MTUSR:
					if(instr->group!=0)
					{
						MSG(MSG_ALWAYS, "ALU_2_MTUSR: unsupported group %u\n", instr->group);
						return SIM_GENERIC_ERROR;
					}
					switch(instr->usr)
					{
						case 0:
							usr[0].L=regs[instr->rt];
							break;
						case 1:
							usr[0].H=regs[instr->rt];
							break;
						case 2:
							usr[1].L=regs[instr->rt];
							break;
						case 3:
							usr[1].H=regs[instr->rt];
							break;
						case 31:
							regs[instr->rt]=PC;
							break;
						default:
							MSG(MSG_ALWAYS, "ALU_2_MTUSR: invalid usr value %u\n", instr->group);
							return SIM_GENERIC_ERROR;
					}
					break;
				
				case SUB_ALU_2_BSP: //this should be tested...
				{
					uint32_t M=get_bits(regs[instr->rb], 12, 8);
					uint32_t N=get_bits(regs[instr->rb], 4, 0);
					uint32_t D=M+N;
					set_bits(&regs[instr->rb], 7, 5, 1);
					if(31>D) //normal condition
					{
						set_bits(&regs[instr->rb], 4, 0, D+1);
						set_bits(&regs[instr->rb], 31, 31, 0);
						set_bits(&regs[instr->rt], 31-N, 31-N-M, get_bits(regs[instr->ra], M, 0));
						if(get_bits(regs[instr->rb], 30, 30)==1)
						{
							set_bits(&regs[instr->rb], 12, 8, get_bits(regs[instr->rb], 20, 16));
							set_bits(&regs[instr->rb], 15, 13, 0);
						}
						set_bits(&regs[instr->rb], 30, 30, 0);
					}
					else if(31==D) //full condition
					{
						set_bits(&regs[instr->rb], 4, 0, 0);
						set_bits(&regs[instr->rb], 30, 30, 0);
						set_bits(&regs[instr->rb], 31, 31, 1);
						set_bits(&regs[instr->rt], M, 0, get_bits(regs[instr->ra], M, 0));
					}
					else if(31<D) //overflow condition
					{
						set_bits(&regs[instr->rb], 20, 16, M);
						set_bits(&regs[instr->rb], 12, 8, D-32);
						set_bits(&regs[instr->rb], 4, 0, 0);
						set_bits(&regs[instr->rb], 30, 30, 1);
						set_bits(&regs[instr->rb], 31, 31, 1);
						set_bits(&regs[instr->rt], 31-N, 0, get_bits(regs[instr->ra], M, M+N-31));
					}	
				}
				break;
				
				
				default:
					MSG(MSG_ALWAYS, "simulate: unimplemented sub 0x%x for OPC_ALU_2 (%s)\n", instr->sub, instr->mnemonic);
					return SIM_UNIMPLEMENTED;
			}
			break;
				
				
		case OPC_ADDI:
			regs[instr->rt]=regs[instr->ra]+nds32_sign_extend(instr->imm1_15, 15, 32);
			break;
		
		case OPC_BR1:
			switch(instr->sub)
			{
				case SUB_BR1_BEQ:
					if(regs[instr->rt]==regs[instr->ra])
					{
						uint32_t offset=nds32_sign_extend(instr->imm1_14<<1, 15, 32);
						PC+=offset;
						is_branch=BR_GOTO;
					}
					break;
				
				case SUB_BR1_BNE:
					if(regs[instr->rt]!=regs[instr->ra])
					{
						uint32_t offset=nds32_sign_extend(instr->imm1_14<<1, 15, 32);
						PC+=offset;
						is_branch=BR_GOTO;
					}
					break;
				//OPC_BR1 is complete
			}
			break;
		
		case OPC_BR2:
			switch(instr->sub)
			{
				case SUB_BR2_BNEZ:
					if(regs[instr->rt]!=0)
					{
						uint32_t offset=nds32_sign_extend(instr->imm1_16<<1, 17, 32);
						PC+=offset;
						is_branch=BR_GOTO;
					}
					break;
				
				case SUB_BR2_BGEZ:
					if((signed)regs[instr->rt]>=0)
					{
						PC+=nds32_sign_extend(instr->imm1_16<<1, 17, 32);
						is_branch=BR_GOTO;
					}
					break;
				
				case SUB_BR2_BLEZ:
					if((signed)regs[instr->rt]<=0)
					{
						PC+=nds32_sign_extend(instr->imm1_16<<1, 17, 32);
						is_branch=BR_GOTO;
					}
					break;
				
				case SUB_BR2_BEQZ:
					if(regs[instr->rt]==0)
					{
						PC+=nds32_sign_extend(instr->imm1_16<<1, 17, 32);
						is_branch=BR_GOTO;
					}
					break;
				
				case SUB_BR2_BGTZ:
					if((signed)regs[instr->rt]>0)
					{
						PC+=nds32_sign_extend(instr->imm1_16<<1, 17, 32);
						is_branch=BR_GOTO;
					}
					break;
				
				case SUB_BR2_BLTZ:
					if((signed)regs[instr->rt]<0)
					{
						PC+=nds32_sign_extend(instr->imm1_16<<1, 17, 32);
						is_branch=BR_GOTO;
					}
					break;
				
				default:
					MSG(MSG_ALWAYS, "simulate: unimplemented sub 0x%x for OPC_BR2 (%s)\n", instr->sub, instr->mnemonic);
					return SIM_UNIMPLEMENTED;
			}
			break;
		
		case OPC_ANDI:
			regs[instr->rt]=regs[instr->ra]&instr->imm1_15;
			break;
		
		case OPC_J:
			switch(instr->sub)
			{
				case SUB_J_J:
					{
						uint32_t offset=nds32_sign_extend(instr->imm1_24<<1, 25, 32);
						PC+=offset;
						is_branch=BR_GOTO;
					}
					break;
				
				case SUB_J_JAL:
					{
						regs[REG_LP]=PC+4;
						uint32_t offset=nds32_sign_extend(instr->imm1_24<<1, 25, 32);
						PC+=offset;
						is_branch=BR_CALL;
					}
					break;
				//J is completed
			}
			break;
		
		case OPC_JREG:
			switch(instr->sub)
			{
				case SUB_JREG_JR:
					PC=regs[instr->rb];
					is_branch=BR_GOTO;
					break;
				
				case SUB_JREG_JRAL:
				{
					//rt and rb can be the same so we need this intermediate variable
					uint32_t jaddr=regs[instr->rb];
					regs[instr->rt]=PC+(instr->was_16bit_instr?2:4);
					PC=jaddr;
					is_branch=BR_CALL;
					break;
				}
				
				case SUB_JREG_RET:
					PC=regs[instr->rb];
					is_branch=BR_RET;
					break;
				
				//JREG is completed
			}
			break;
		
		case OPC_LSMW:
			switch(instr->sub)
			{
				case SUB_LSMW_LMW_ai:
				case SUB_LSMW_LMW_bi:
				case SUB_LSMW_LMW_ad:
				case SUB_LSMW_LMW_aim:
				case SUB_LSMW_LMW_bim:
				case SUB_LSMW_LMW_adm:
				case SUB_LSMW_LMW_bdm:
				{
					sim_t ret=sim_opc_lmw(instr, regs, &stop);
					if(ret!=SIM_NO_ERROR)
						return ret;
					break;
				}
				
				case SUB_LSMW_SMW_ai:
				case SUB_LSMW_SMW_bi:
				case SUB_LSMW_SMW_ad:
				case SUB_LSMW_SMW_aim:
				case SUB_LSMW_SMW_bim:
				case SUB_LSMW_SMW_adm:
				case SUB_LSMW_SMW_bdm:				
					sim_opc_smw(instr, regs, &stop);
					break;
				
				case SUB_LSMW_SMWA_adm:
				case SUB_LSMW_SMWA_bdm:
					sim_opc_smwa(instr, regs, &stop);
					break;
				
				case SUB_LSMW_LMWA_bim:
				{
					sim_t ret=sim_opc_lmwa(instr, regs, &stop);
					if(ret!=SIM_NO_ERROR)
						return ret;
					break;
				}
				
				default:
					MSG(MSG_ALWAYS, "simulate: unimplemented sub 0x%x for OPC_LSMW (%s)\n", instr->sub, instr->mnemonic);
					return SIM_UNIMPLEMENTED;
			
			}
			break;
		
		case OPC_MISC:
			switch(instr->sub)
			{
				case SUB_MISC_MFSR:
					regs[instr->rt]=read_from_special_reg(instr->sridx);
					break;
				case SUB_MISC_MTSR:
					if(instr->imm2_5==0)
						write_to_special_reg(instr->sridx, regs[instr->ra]);
					else if(instr->imm2_5==0b00010)
						special_reg_setgie(instr->en);
					break;
					
				case SUB_MISC_ISB:
					break;
				
				case SUB_MISC_IRET:
					PC=read_from_special_reg(SR_INT_PC);
					special_reg_setgie(1);
					is_branch=BR_IRET;
					break;
				
				default:
					MSG(MSG_ALWAYS, "OPC_MISC: unimplemented: %s\n", instr->mnemonic);
					return SIM_UNIMPLEMENTED;
			}
			break;
		
		case OPC_SLTI:
			if(regs[instr->ra]<instr->imm1_15)
				regs[instr->rt]=1;
			else
				regs[instr->rt]=0;
			break;
		
		case OPC_SLTSI:
			if((signed)regs[instr->ra]<nds32_sign_extend(instr->imm1_15, 15, 32))
				regs[instr->rt]=1;
			else
				regs[instr->rt]=0;
			break;
		
		case OPC_MEM:
			switch(instr->sub)
			{
				case SUB_MEM_LW:
				{
					uint32_t addr;
					addr=regs[instr->ra]+(regs[instr->rb]<<instr->imm1_2); //imm1_2 == sv
					mem_word_t val=memory_get_word(addr, &stop);
					if(stop && !ignore_breakpoints)
						return SIM_STOPPED_ON_BP;
					if(!val.is_initialized)
						return SIM_READ_FROM_UNINITIALIZED;
					regs[instr->rt]=val.val;
					break;
				}
				
				case SUB_MEM_SW:
				{
					uint32_t addr;
					addr=regs[instr->ra]+(regs[instr->rb]<<instr->imm1_2); //imm1_2 == sv
					memory_set_word(regs[instr->rt], addr, &stop, false);
					break;
				}
				
				case SUB_MEM_LB:
				{
					uint32_t addr;
					addr=regs[instr->ra]+(regs[instr->rb]<<instr->imm1_2); //imm1_2 == sv
					mem_byte_t val=memory_get_byte(addr, &stop);
					if(stop && !ignore_breakpoints)
						return SIM_STOPPED_ON_BP;
					if(!val.is_initialized)
						return SIM_READ_FROM_UNINITIALIZED;
					regs[instr->rt]=val.val;
					break;
				}
				
				case SUB_MEM_LBS:
				{
					uint32_t addr;
					addr=regs[instr->ra]+(regs[instr->rb]<<instr->imm1_2); //imm1_2 == sv
					mem_byte_t val=memory_get_byte(addr, &stop);
					if(stop && !ignore_breakpoints)
						return SIM_STOPPED_ON_BP;
					if(!val.is_initialized)
						return SIM_READ_FROM_UNINITIALIZED;
					regs[instr->rt]=nds32_sign_extend(val.val, 8, 32);
					break;
				}
				
				
				case SUB_MEM_SB:
				{
					uint32_t addr;
					addr=regs[instr->ra]+(regs[instr->rb]<<instr->imm1_2); //imm1_2 == sv
					memory_set_byte(regs[instr->rt]&0xff, addr, &stop, false);
					break;
				}
				
				default:
					MSG(MSG_ALWAYS, "OPC_MEM: unimplemented: %s\n", instr->mnemonic);
					return SIM_UNIMPLEMENTED;
			}
			break;
		
		case OPC_XORI:
			regs[instr->rt]=regs[instr->ra]^instr->imm1_15;
			break;
		
		case OPC_SUBRI:
			regs[instr->rt]=nds32_sign_extend(instr->imm1_15, 15, 32)-regs[instr->ra];
			break;
		
		default:
			MSG(MSG_ALWAYS, "simulate: unimplemented: %s\n", instr->mnemonic);
			return SIM_UNIMPLEMENTED;
	}
	
	history_add(PC_old, regs);
	
	if(is_branch==NO_BRANCH)
	{
		if(instr->was_16bit_instr)
				PC+=2;
		else 
				PC+=4;
	}
	else if(is_branch==BR_CALL)
	{
		//decrease verbosity by removing calls from/to idle task
		if(PC!=0x5ad4)
			MSG(MSG_CALL, "%x: CALL to 0x%x\n", PC_old, PC);
	}
	else if(is_branch==BR_GOTO || is_branch==BR_RET || is_branch==BR_IRET)
	{
		MSG(MSG_GOTO, "%x: GOTO 0x%x\n", PC_old, PC);
		
		if(PC==PC_old)
			return SIM_ENDLESS_LOOP;

//we need to disable this at some point because it does not know about interrupts
#ifndef NO_ENDLESS_LOOP_DETECT		
		if(history_check())
			return SIM_ENDLESS_LOOP;
#endif
	}
	
	if(is_branch==BR_RET)
	{
		//decrease verbosity by removing calls from/to idle task
		if(PC!=0x1bf2)
			MSG(MSG_RET, "RET from %x to %x\n", PC_old, PC);
	}
	
	if(is_branch==BR_IRET)
		MSG(MSG_IRET, "ISR finished, resuming at 0x%x\n", PC);
	
	if(stop && !ignore_breakpoints)
		return SIM_STOPPED_ON_BP;
	
	return SIM_NO_ERROR;
}

static sim_t sim_step(const bool ignore_breakpoints)
{
	instr_t instr;
	sim_t ret;
	
	timer_tick();
	
	if(check_for_pending_irq())
		MSG(MSG_INT, "ENTERING ISR @ 0x%x\n", PC);
	
	if(decode_instr(&instr, PC))
	{
		MSG(MSG_ALWAYS, "error in decode_instr(), stop\n");
		return SIM_ERROR_DECODE_INSTR;
	}
	
	ret=simulate(&instr, ignore_breakpoints);
	if(ret==SIM_UNIMPLEMENTED)
		MSG(MSG_ALWAYS, "unimplemented instr in simulate()\n");
	else if(ret==SIM_ENDLESS_LOOP)
		MSG(MSG_ALWAYS, "endless loop detected in simulate()\n");
	else if(ret==SIM_READ_FROM_UNINITIALIZED)
		MSG(MSG_ALWAYS, "tried to read from uninitialized memory in simulate()\n");
	else if(ret==SIM_GENERIC_ERROR)
		MSG(MSG_ALWAYS, "unknown error in simulate()\n");
	else if(ret==SIM_STOPPED_ON_BP)
		MSG(MSG_ALWAYS, "simulate() stopped due to BP\n");
	
	return ret;
}

void single_step(PROTOTYPE_ARGS_HANDLER)
{
	ARGS_HANDLER_UNUSED;
	
	sim_step(true);

	disassm_reset();
	redraw_all();
}

void run_sim(PROTOTYPE_ARGS_HANDLER)
{
	(void)cmd;
	(void)nb_args;
	
	char arg[SZ_BUFFER_ARGUMENTS];
	strcpy(arg, get_next_argument());
	char *ptr;
	uint32_t multiplier=1;
	
	if((ptr=strchr(arg, 'k')))
	{
		(*ptr)='\0';
		multiplier=1000;
	}
	else if((ptr=strchr(arg, 'M')))
	{
		(*ptr)='\0';
		multiplier=1000*1000;
	}
	
	uint32_t nb_steps=atoi(arg)*multiplier;
	
	if(nb_steps==0)
	{
		MSG(MSG_ALWAYS, "invalid number of steps\n");
		return;
	}
	
	MSG(MSG_ALWAYS, "executing %u steps...\n", nb_steps);
	
	uint8_t err_happened=0;
	uint32_t step;
	for(step=1; step<=nb_steps; step++)
	{
		if(sim_step(false)!=SIM_NO_ERROR)
		{
			err_happened=1;
			MSG(MSG_ALWAYS, "stopped after %u steps\n", step);
			break;
		}
	}
	
	if(!err_happened)
		MSG(MSG_ALWAYS, "%u steps successfully executed\n", nb_steps);
	
	disassm_reset();
	redraw_all();
}

void write_register(PROTOTYPE_ARGS_HANDLER)
{
	ARGS_HANDLER_UNUSED;
	
	uint8_t reg;
	if(parse_register(get_next_argument(), &reg))
		return;
		
	uint32_t val;
	if(parse_hex(get_next_argument(), &val))
		return;
	
	regs[reg]=val;
	
	redraw(W_REG);
}

void set_pc_cmd(PROTOTYPE_ARGS_HANDLER)
{
	ARGS_HANDLER_UNUSED;
	
	uint32_t val;
	if(parse_hex(get_next_argument(), &val))
		return;
	
	PC=val;
	
	MSG(MSG_ALWAYS, "PC set to 0x%x\n", val);
	
	redraw(W_ASM); //this will not do what we want for now, the disassm-viewer needs work
}


