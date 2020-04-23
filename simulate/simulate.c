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
	
	//timer_tick();
	if(timer_tick())
	{
		printf("IRQ TIMER\n");
		//return SIM_STOPPED_ON_IRQ;
	}

	
	if(check_for_pending_irq())
		printf("ENTERING ISR @ 0x%x\n", PC);
	
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
			mem_byte_t val=memory_get_byte(addr, &stop);
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
			memory_set_byte(regs[instr->rt], addr, &stop, false);
			break;
		}

		case OPC_SBI_bi:
		{
			uint32_t addr_update;
			addr_update=regs[instr->ra]+nds32_sign_extend(instr->imm1_15, 15, 32);
			memory_set_byte(regs[instr->rt], regs[instr->ra], &stop, false);
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
					regs[instr->rt]=regs[instr->ra];
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
						printf("DIV BY ZERO\n");
						return SIM_GENERIC_ERROR;
					}
					regs[instr->rt]=regs[instr->ra]/regs[instr->rb];
					regs[instr->rs]=regs[instr->ra]%regs[instr->rb];
					break;
				
				case SUB_ALU_1_DIVSR:
					if(regs[instr->rb]==0)
					{
						printf("DIV BY ZERO\n");
						return SIM_GENERIC_ERROR;
					}
					regs[instr->rt]=(signed)regs[instr->ra]/(signed)regs[instr->rb];
					regs[instr->rs]=(signed)regs[instr->ra]%(signed)regs[instr->rb];
					break;
				
				default:
					printf("simulate: unimplemented sub 0x%x for OPC_ALU_1 (%s)\n", instr->sub, instr->mnemonic);
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
					if(instr->dt!=0 && instr->dt!=1)
						ERRX(1, "ALU_2_MADD32: invalid dt\n");
					uint64_t Mresult=regs[instr->ra]*regs[instr->rb];
					usr[instr->dt].L=Mresult&0xffffffff;
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
						printf("ALU_2_MFUSR: unsupported group %u\n", instr->group);
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
							printf("ALU_2_MFUSR: invalid usr value %u\n", instr->group);
							return SIM_GENERIC_ERROR;
					}
					break;
				
				case SUB_ALU_2_MTUSR:
					if(instr->group!=0)
					{
						printf("ALU_2_MTUSR: unsupported group %u\n", instr->group);
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
							printf("ALU_2_MTUSR: invalid usr value %u\n", instr->group);
							return SIM_GENERIC_ERROR;
					}
					break;
				
				default:
					printf("simulate: unimplemented sub 0x%x for OPC_ALU_2 (%s)\n", instr->sub, instr->mnemonic);
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
					printf("simulate: unimplemented sub 0x%x for OPC_BR2 (%s)\n", instr->sub, instr->mnemonic);
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
					printf("simulate: unimplemented sub 0x%x for OPC_LSMW (%s)\n", instr->sub, instr->mnemonic);
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
					//printf("instr %s from OPC_MISC ignored\n", instr->mnemonic);
					break;
				
				case SUB_MISC_IRET:
					PC=read_from_special_reg(SR_INT_PC);
					write_to_special_reg(SR_PROC_STATUS_WORD, read_from_special_reg(SR_PROC_INT_STATUS_WORD));
					is_branch=BR_IRET;
					break;
				
				default:
					printf("OPC_MISC: unimplemented: %s\n", instr->mnemonic);
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
					memory_set_byte(regs[instr->rt], addr, &stop, false);
					break;
				}
				
				default:
					printf("OPC_MEM: unimplemented: %s\n", instr->mnemonic);
					return SIM_UNIMPLEMENTED;
			}
			break;
		
		case OPC_XORI:
			regs[instr->rt]=regs[instr->ra]^instr->imm1_15;
			break;
		
		default:
			printf("simulate: unimplemented: %s\n", instr->mnemonic);
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
		printf("%x: CALL to 0x%x\n", PC_old, PC);
	}
	else if(is_branch==BR_GOTO || is_branch==BR_RET || is_branch==BR_IRET)
	{
		if(PC==PC_old)
			return SIM_ENDLESS_LOOP;
		
		if(history_check())
			return SIM_ENDLESS_LOOP;
	}
	
	if(is_branch==BR_RET)
		printf("RET from %x to %x\n", PC_old, PC);
	
	if(is_branch==BR_IRET)
		printf("ISR finished, resuming at 0x%x\n", PC);
	
	//if(is_branch==BR_GOTO)
	//	printf("GOTO 0x%x\n", PC);
	
	if(stop && !ignore_breakpoints)
		return SIM_STOPPED_ON_BP;
	
	return SIM_NO_ERROR;
}

static sim_t sim_step(const bool ignore_breakpoints)
{
	instr_t instr;
	sim_t ret;
	
	if(decode_instr(&instr, PC))
	{
		printf("error in decode_instr(), stop\n");
		return SIM_ERROR_DECODE_INSTR;
	}
	
	ret=simulate(&instr, ignore_breakpoints);
	if(ret==SIM_UNIMPLEMENTED)
		printf("unimplemented instr in simulate()\n");
	else if(ret==SIM_ENDLESS_LOOP)
		printf("endless loop detected in simulate()\n");
	else if(ret==SIM_READ_FROM_UNINITIALIZED)
		printf("tried to read from uninitialized memory in simulate()\n");
	else if(ret==SIM_GENERIC_ERROR)
		printf("unknown error in simulate()\n");
	else if(ret==SIM_STOPPED_ON_BP)
		printf("simulate() stopped due to BP\n");
	else if(ret==SIM_STOPPED_ON_IRQ)
		printf("IRQ request from TIMER1\n");
	
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
	
	uint32_t nb_steps=atoi(get_next_argument());
	
	if(nb_steps==0)
	{
		printf("invalid number of steps\n");
		return;
	}
	
	uint8_t err_happened=0;
	uint32_t step;
	for(step=1; step<=nb_steps; step++)
	{
		if(sim_step(false)!=SIM_NO_ERROR)
		{
			err_happened=1;
			printf("stopped after %u steps\n", step);
			break;
		}
	}
	
	if(!err_happened)
		printf("%u steps successfully executed\n", nb_steps);
	
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
	
	printf("PC set to 0x%x\n", val);
	
	redraw(W_ASM); //this will not do what we want for now, the disassm-viewer needs work
}
	
	
