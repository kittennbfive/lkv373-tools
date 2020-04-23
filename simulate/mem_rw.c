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

#include "mem_rw.h"
#include "memory.h"
#include "my_err.h"

static uint8_t lmw_smw_count_reg_make_list(instr_t const * const instr, uint8_t * const reg_list)
{
	memset(reg_list, 0, 32*sizeof(uint8_t));
	
	uint8_t i;
	uint8_t TNReg=0;
	
	if(!(instr->rb==31 && instr->re==31))
	{
		for(i=instr->rb; i<=instr->re; i++)
		{
			reg_list[i]=1;
			TNReg++;
		}
	}
	
	if(instr->enable&0x01)
	{
		reg_list[31]=1;
		TNReg++;
	}
	
	if(instr->enable&0x02)
	{
		reg_list[30]=1;
		TNReg++;
	}
	
	if(instr->enable&0x04)
	{
		reg_list[29]=1;
		TNReg++;
	}
	
	if(instr->enable&0x08)
	{
		reg_list[28]=1;
		TNReg++;
	}
	
	return TNReg;
}

#define AB_A 1
#define AB_B 0
#define ID_I 0
#define ID_D 1

sim_t sim_opc_lmw(instr_t const * const instr, uint32_t * const regs, bool * const stop)
{
	uint8_t ab=(instr->sub&(1<<4))>>4;
	uint8_t id=(instr->sub&(1<<3))>>3;
	uint8_t m=(instr->sub&(1<<2))>>2;
	
	uint8_t reg_list[32];
	uint8_t TNReg=lmw_smw_count_reg_make_list(instr, reg_list);
	uint32_t B_Addr, E_Addr;
	
	if(ab==AB_B && id==ID_I) //BI
	{
		B_Addr=regs[instr->ra];
		E_Addr=regs[instr->ra]+(TNReg*4)-4;
	}
	else if(ab==AB_A && id==ID_I) //AI
	{
		B_Addr=regs[instr->ra]+4;
		E_Addr=regs[instr->ra]+(TNReg*4);
	}
	else if(ab==AB_B && id==ID_D) //BD
	{
		B_Addr=regs[instr->ra]-(TNReg*4)+4;
		E_Addr=regs[instr->ra];
	}
	else if(ab==AB_A && id==ID_D) //AD
	{
		B_Addr=regs[instr->ra]-(TNReg*4);
		E_Addr=regs[instr->ra]-4;
	}
	else
		ERRX(1, "lmw: unknown");
	
	uint32_t VA=B_Addr;
	uint8_t i;
	for(i=0; i<32; i++)
	{
		if(reg_list[i])
		{
			mem_word_t val;
			val=memory_get_word(VA, stop);
			if(*stop)
				return SIM_STOPPED_ON_BP;
			if(!val.is_initialized)
				return SIM_READ_FROM_UNINITIALIZED;
			regs[i]=val.val;
			VA+=4;
		}
	}
	
	if(VA-4!=E_Addr)
		ERRX(1,"sim_opc_lmw addr err");
	
	if(id==ID_I && m) //IM
	{
		regs[instr->ra]+=(TNReg*4);
	}
	else if(id==ID_D && m) //DM
	{
		regs[instr->ra]-=(TNReg*4);
	}
	//else
	//	ERRX(1, "lmw: unknown");
	
	return SIM_NO_ERROR;
}

void sim_opc_smw(instr_t const * const instr, uint32_t * const regs, bool * const stop)
{
	uint8_t ab=(instr->sub&(1<<4))>>4;
	uint8_t id=(instr->sub&(1<<3))>>3;
	uint8_t m=(instr->sub&(1<<2))>>2;
	
	uint8_t reg_list[32];
	uint8_t TNReg=lmw_smw_count_reg_make_list(instr, reg_list);
	uint32_t B_Addr, E_Addr;
	
	if(ab==AB_B && id==ID_I) //BI
	{
		B_Addr=regs[instr->ra];
		E_Addr=regs[instr->ra]+(TNReg*4)-4;
	}
	else if(ab==AB_A && id==ID_I) //AI
	{
		B_Addr=regs[instr->ra]+4;
		E_Addr=regs[instr->ra]+(TNReg*4);
	}
	else if(ab==AB_B && id==ID_D) //BD
	{
		B_Addr=regs[instr->ra]-(TNReg*4)+4;
		E_Addr=regs[instr->ra];
	}
	else if(ab==AB_A && id==ID_D) //AD
	{
		B_Addr=regs[instr->ra]-(TNReg*4);
		E_Addr=regs[instr->ra]-4;
	}
	else
		ERRX(1, "smw: unknown");
	
	uint32_t VA=B_Addr;
	uint8_t i;
	for(i=0; i<32; i++)
	{
		if(reg_list[i])
		{
			memory_set_word(regs[i], VA, stop, false);
			VA+=4;
		}
	}
	
	if(VA-4!=E_Addr)
		ERRX(1,"sim_opc_smw addr err");
	
	if(id==ID_I && m) //IM
	{
		regs[instr->ra]+=(TNReg*4);
	}
	else if(id==ID_D && m) //DM
	{
		regs[instr->ra]-=(TNReg*4);
	}
}

void sim_opc_smwa(instr_t const * const instr, uint32_t * const regs, bool * const stop)
{
	uint8_t ab=(instr->sub&(1<<4))>>4;
	uint8_t id=(instr->sub&(1<<3))>>3;
	uint8_t m=(instr->sub&(1<<2))>>2;
	
	uint8_t reg_list[32];
	uint8_t TNReg=lmw_smw_count_reg_make_list(instr, reg_list);
	uint32_t B_Addr, E_Addr;
	
	if(reg_list[31])
		ERRX(1, "31 is in list");
	
	if(ab==AB_B && id==ID_I) //BI
	{
		B_Addr=regs[instr->ra];
		E_Addr=regs[instr->ra]+(TNReg*4)-4;
	}
	else if(ab==AB_A && id==ID_I) //AI
	{
		B_Addr=regs[instr->ra]+4;
		E_Addr=regs[instr->ra]+(TNReg*4);
	}
	else if(ab==AB_B && id==ID_D) //BD
	{
		B_Addr=regs[instr->ra]-(TNReg*4)+4;
		E_Addr=regs[instr->ra];
	}
	else if(ab==AB_A && id==ID_D) //AD
	{
		B_Addr=regs[instr->ra]-(TNReg*4);
		E_Addr=regs[instr->ra]-4;
	}
	else
		ERRX(1, "smwa: unknown");
	
	uint32_t VA=B_Addr;
	
	const uint8_t reg_order[32]={0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 31, 29, 30, 28}; // !!!
	
	uint8_t i;
	for(i=0; i<32; i++)
	{
		if(reg_list[reg_order[i]])
		{
			memory_set_word(regs[reg_order[i]], VA, stop, false);
			VA+=4;
		}
	}
	
	if(VA-4!=E_Addr)
		ERRX(1, "sim_opc_smwa addr err\n");
	
	if(id==ID_I && m) //IM
	{
		regs[instr->ra]+=(TNReg*4);
	}
	else if(id==ID_D && m) //DM
	{
		regs[instr->ra]-=(TNReg*4);
	}
}

sim_t sim_opc_lmwa(instr_t const * const instr, uint32_t * const regs, bool * const stop)
{
	uint8_t ab=(instr->sub&(1<<4))>>4;
	uint8_t id=(instr->sub&(1<<3))>>3;
	uint8_t m=(instr->sub&(1<<2))>>2;
	
	uint8_t reg_list[32];
	uint8_t TNReg=lmw_smw_count_reg_make_list(instr, reg_list);
	uint32_t B_Addr, E_Addr;
	
	if(ab==AB_B && id==ID_I) //BI
	{
		B_Addr=regs[instr->ra];
		E_Addr=regs[instr->ra]+(TNReg*4)-4;
	}
	else if(ab==AB_A && id==ID_I) //AI
	{
		B_Addr=regs[instr->ra]+4;
		E_Addr=regs[instr->ra]+(TNReg*4);
	}
	else if(ab==AB_B && id==ID_D) //BD
	{
		B_Addr=regs[instr->ra]-(TNReg*4)+4;
		E_Addr=regs[instr->ra];
	}
	else if(ab==AB_A && id==ID_D) //AD
	{
		B_Addr=regs[instr->ra]-(TNReg*4);
		E_Addr=regs[instr->ra]-4;
	}
	else
		ERRX(1, "lmwa: unknown");
		
	uint32_t VA=B_Addr;
	
	const uint8_t reg_order[32]={0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 31, 29, 30, 28}; // !!!
	
	uint8_t i;
	for(i=0; i<32; i++)
	{
		if(reg_list[reg_order[i]])
		{
			mem_word_t val;
			val=memory_get_word(VA, stop);
			if(*stop)
				return SIM_STOPPED_ON_BP;
			if(!val.is_initialized)
				return SIM_READ_FROM_UNINITIALIZED;
			regs[reg_order[i]]=val.val;
			VA+=4;
		}
	}
	
	if(VA-4!=E_Addr)
		ERRX(1, "sim_opc_lmwa addr err");
	
	if(id==ID_I && m) //IM
	{
		regs[instr->ra]+=(TNReg*4);
	}
	else if(id==ID_D && m) //DM
	{
		regs[instr->ra]-=(TNReg*4);
	}
	
	return SIM_NO_ERROR;
}
