/*
experimental r2-support for nds32

written by kitten_nb_five (c) 2019-2020

HIGHLY EXPERIMENTAL - NO WARRANTY - PROVIDED AS IS

licence: AGPL version 3 or later
*/

#include <r_asm.h>
#include <r_anal.h>
#include <r_lib.h>

#include <stdio.h>

#include "../../asm/arch/nds32/decode_disassm32.h"
#include "../../asm/arch/nds32/decode_disassm16.h"
#include "../../asm/arch/nds32/opcodes32.h"
#include "../../asm/arch/nds32/opcodes16.h"
#include "../../asm/arch/nds32/instruction.h"
#include "../../asm/arch/nds32/sign_extend.h"

static int nds32_op(RAnal *a, RAnalOp *op, ut64 addr, const ut8 *data, int len, RAnalOpMask mask)
{
	memset (op, '\0', sizeof (RAnalOp));
	
	static ut32 LP=0;
	static ut32 regs[32]={0};
	
	if(len<2)
		return -1;
	
	instr_t instr_struct;
	ut16 instr=data[0]<<8|data[1];
	if(SHORT_INSTR_IS_WIDTH32(instr))
	{
		if(len<4)
			return -1;

		ut32 instr_32=data[0]<<24|data[1]<<16|data[2]<<8|data[3];
		decode_32(instr_32, &instr_struct, addr);
		op->size=4;
		instr_struct.width=WIDTH32;
	}
	else
	{
		decode_16(instr, &instr_struct, addr);
		op->size=2;
		instr_struct.width=WIDTH16;
	}

	if(instr_struct.width==WIDTH32)
	{
		switch(instr_struct.opc)
		{
			case OPC_J:
				switch(instr_struct.sub)
				{
					case SUB_J_J:
						op->type=R_ANAL_OP_TYPE_JMP;
						op->jump=nds32_sign_extend(instr_struct.imm1_24<<1, 25, 32)+addr;
						break;
					
					case SUB_J_JAL:
						LP=addr+4;
						op->type=R_ANAL_OP_TYPE_JMP;
						op->jump=nds32_sign_extend(instr_struct.imm1_24<<1, 25, 32)+addr;
						break;
				}
				break;

			case OPC_JREG:
				switch(instr_struct.sub)
				{
					case SUB_JREG_JR:
						op->type=R_ANAL_OP_TYPE_RJMP;
						op->jump=regs[instr_struct.rb];
						break;
					
					case SUB_JREG_JRAL:
						LP=addr+4;
						op->type=R_ANAL_OP_TYPE_RJMP;
						break;
					
					case SUB_JREG_RET:
						op->type=R_ANAL_OP_TYPE_RET;
						op->jump=LP;
						op->eob=true;
						break;
				}
				break;
			
			case OPC_BR1:
				op->type=R_ANAL_OP_TYPE_CJMP;
				op->jump=nds32_sign_extend(instr_struct.imm1_14<<1, 15, 32)+addr;
				op->fail=addr+4;
				break;
			
			case OPC_BR2:
				op->type=R_ANAL_OP_TYPE_CJMP;
				op->jump=nds32_sign_extend(instr_struct.imm1_16<<1, 17, 32)+addr;
				op->fail=addr+4;
				break;
				
			case OPC_SETHI:
				regs[instr_struct.rt]=instr_struct.imm1_20<<12;
				break;
			
			case OPC_ORI:
				regs[instr_struct.rt]=regs[instr_struct.ra]|instr_struct.imm1_15;
				break;
		}
	}
	else
	{
		switch(instr_struct.opc)
		{
			case OPC16_BEQS38:
			case OPC16_BNES38:
			case OPC16_BEQZ38:
			case OPC16_BNEZ38:
			case OPC16_BNEZS8:
				op->type=R_ANAL_OP_TYPE_CJMP;
				op->jump=nds32_sign_extend(instr_struct.imm1_8<<1, 9, 32)+addr;
				op->fail=addr+2;
				break;
			
			case OPC16_J8:
				op->type=R_ANAL_OP_TYPE_JMP;
				op->jump=nds32_sign_extend(instr_struct.imm1_8<<1, 9, 32)+addr;
				break;
			
			case OPC16_JR5:
				op->type=R_ANAL_OP_TYPE_JMP;
				op->jump=regs[instr_struct.rb];
				break;
			
			case OPC16_RET5:
				op->type=R_ANAL_OP_TYPE_RJMP;
				op->jump=LP;
				op->eob=true;
				break;
			
			case OPC16_JRAL5:
				LP=addr+2;
				op->type=R_ANAL_OP_TYPE_JMP;
				op->jump=regs[instr_struct.rb];
				break;
		}
	}
	
	return op->size;
}

RAnalPlugin r_anal_plugin_nds32 = {
	.name = "nds32",
	.desc = "Andes NDS32 [experimental!!]",
	.license = "AGPL3",
	.bits = 16|32,
	.arch = "nds32",
	.esil = false,
	.op = &nds32_op
};

#ifndef R2_PLUGIN_INCORE
R_API RLibStruct radare_plugin = {
	.type = R_LIB_TYPE_ANAL,
	.data = &r_anal_plugin_nds32,
	.version = R2_VERSION
};
#endif
