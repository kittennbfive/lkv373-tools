/*
experimental r2-support for nds32

written by kitten_nb_five (c) 2019-2020

HIGHLY EXPERIMENTAL - NO WARRANTY - PROVIDED AS IS

licence: AGPL version 3 or later
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <r_asm.h>

#include "../arch/nds32/instruction.h"
#include "../arch/nds32/decode_disassm16.h"
#include "../arch/nds32/decode_disassm32.h"

#define SHORT_INSTR_IS_WIDTH32(byte) ((byte&(1<<15))==0)

static int disassemble(RAsm *a, RAsmOp *op, const ut8 *buf, int len) 
{
	if(len<2)
	{
		r_strbuf_set(&op->buf_asm, "invalid");
		return -1;
	}
	
	ut16 instr=buf[0]<<8|buf[1];
	if(SHORT_INSTR_IS_WIDTH32(instr))
	{
		if(len<4)
		{
			r_strbuf_set (&op->buf_asm, "invalid");
			return -1;
		}
		ut32 instr_32=buf[0]<<24|buf[1]<<16|buf[2]<<8|buf[3];
		instr_t instr_struct;
		decode_32(instr_32, &instr_struct, (ut32)(a->pc));
		r_strbuf_set(&op->buf_asm, instr_struct.disassm);
		op->size=4;
	}
	else
	{
		instr_t instr_struct;
		decode_16(instr, &instr_struct, (ut32)(a->pc));
		r_strbuf_set(&op->buf_asm, instr_struct.disassm);
		op->size=2;
	}
	
	return op->size;
}

RAsmPlugin r_asm_plugin_nds32 = {
	.name = "nds32",
	.arch = "nds32",
	.license = "AGPL3",
	.bits = 16|32,
	.endian = R_SYS_ENDIAN_LITTLE,
	.desc = "Andes NDS32 [experimental!!]",
	.disassemble = &disassemble,
};

#ifndef R2_PLUGIN_INCORE
R_API RLibStruct radare_plugin = {
	.type = R_LIB_TYPE_ASM,
	.data = &r_asm_plugin_nds32,
	.version = R2_VERSION
};
#endif
