/*
make flowgraphs of functions inside the LKV373-firmware using graphviz
(c) 2020 by kitten_nb_five
freenode #lkv373a
THIS PROGRAM COMES WITHOUT ANY WARRANTY!

licence: AGPL version 3 or later

THIS IS EXPERIMENTAL STUFF!
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "instruction.h"
#include "decode_disassm.h"
#include "opcodes32.h"
#include "sign_extend.h"

#include "my_err.h"

#define VERSION "0.1"

//make orthogonal lines instead of rounded ones
//#define SPLINES_ORTHO

uint8_t * firmware;
uint32_t fsize;

FILE * output;

uint32_t decode_disassm_memory_get_word(const uint32_t addr)
{
	if(addr>fsize)
		ERRX(1, "memory read from addr 0x%x is out of range (size is 0x%x)\n", addr, fsize);

	uint32_t value=(firmware[addr]<<24)|(firmware[addr+1]<<16)|(firmware[addr+2]<<8)|firmware[addr+3];
	
	return value;
}

#define JUMP_TARGETS_MAX 5000
uint32_t * jump_targets;
uint32_t nb_jmp_targets=0;

#define BLOCK_ADDR_MAX 500
uint32_t * block_addr;
uint32_t nb_block_addr;

void push_jump_target(const uint32_t addr)
{
	if(nb_jmp_targets<JUMP_TARGETS_MAX)
		jump_targets[nb_jmp_targets++]=addr;
	else
		ERRX(1, "jump_targets is full, increase JUMP_TARGETS_MAX\n");
}

uint8_t is_known_target(const uint32_t addr)
{
	uint32_t i;
	for(i=0; i<nb_jmp_targets; i++)
		if(jump_targets[i]==addr)
			return 1;
	
	return 0;
}

void push_block_addr(const uint32_t addr)
{
	if(nb_block_addr<BLOCK_ADDR_MAX)
		block_addr[nb_block_addr++]=addr;
	else
		ERRX(1, "block_addr is full, increase BLOCK_ADDR_MAX\n");
}

uint8_t is_block_addr(const uint32_t addr)
{
	uint32_t i;
	for(i=0; i<nb_block_addr; i++)
		if(block_addr[i]==addr)
			return 1;
	
	return 0;
}

uint32_t addr_next_instr(const uint32_t PC, instr_t const * const instr)
{
	if(instr->was_16bit_instr)
		return PC+2;
	else
		return PC+4;
}

uint32_t get_target(const uint32_t PC, instr_t const * const instr)
{
	if(instr->opc==OPC_J && instr->sub==SUB_J_J)
		return PC+nds32_sign_extend(instr->imm1_24<<1, 25, 32);
//	else if(instr->opc==OPC_JREG && instr->sub==SUB_JREG_JR)  ////////////TODO: registers
//		return regs[instr->rb];
	else if(instr->opc==OPC_BR1)
		return PC+nds32_sign_extend(instr->imm1_14<<1, 15, 32);
	else if(instr->opc==OPC_BR2)
		return PC+nds32_sign_extend(instr->imm1_16<<1, 17, 32);
	else
		ERR(1, "unknown instr %s", instr->disassm);
}

//we first need to know where to start a new block because of a jump from above or below
void get_blocks(const uint32_t addr_start)
{
	uint32_t PC;
	instr_t instr;
	
	PC=addr_start;
		
	do
	{
		if(decode_instr(&instr, PC))
			ERR(1, "decode_instr");

		if(instr.opc==OPC_BR1 || instr.opc==OPC_BR2) //COND JUMP
		{
			uint32_t addr_target=get_target(PC, &instr);
			
			if(!is_block_addr(addr_target))
				push_block_addr(addr_target);	
		}
		else if((instr.opc==OPC_J && instr.sub==SUB_J_J) || (instr.opc==OPC_JREG && instr.sub==SUB_JREG_JR)) //UNCOND JUMP
		{			
			uint32_t addr_target=get_target(PC, &instr);
			
			if(!is_block_addr(addr_target))
				push_block_addr(addr_target);
		}
		
		PC=addr_next_instr(PC, &instr);
		
	} while(!(instr.opc==OPC_JREG && instr.sub==SUB_JREG_RET));
}

//this is needed to avoid a warning from dot
#ifdef SPLINES_ORTHO
#define EDGE_LABEL "xlabel"
#else
#define EDGE_LABEL "label"
#endif 

//do the actual work, recursive function
void parse(const uint32_t addr_start)
{
#define SZ_BUFFER 20000
	char buffer[SZ_BUFFER]={0};
	
	uint32_t PC;
	instr_t instr;
	
	uint32_t addr_curr_block=addr_start;
	
	for(PC=addr_start; ; )
	{
		if(decode_instr(&instr, PC))
			ERR(1, "decode_instr");

		if(!is_known_target(PC))
			push_jump_target(PC);
		else
		{
			if(strlen(buffer))
			{
				fprintf(output, "\taddr_%x [shape=box; label=\"%s\"];\n", addr_curr_block, buffer);
				
				fprintf(output, "\taddr_%x -> addr_%x;\n", addr_curr_block, PC);
			}
			return;
		}
		
		if(instr.opc==OPC_JREG && instr.sub==SUB_JREG_RET)
		{
			if(strlen(buffer))
			{
				fprintf(output, "\taddr_%x [shape=box; label=\"%s\"];\n", addr_curr_block, buffer);
				
				fprintf(output, "\taddr_%x -> addr_%x;\n", addr_curr_block, PC);
			}
			fprintf(output, "\taddr_%x [shape=oval; rang=max; label=\"%s\"];\n", PC, instr.disassm);
			return;
		}
		
		else if(instr.opc==OPC_BR1 || instr.opc==OPC_BR2) //COND JUMP
		{
			uint32_t addr_target=get_target(PC, &instr);
			
			if(strlen(buffer))
				fprintf(output, "\taddr_%x [shape=box; label=\"%s\"];\n", addr_curr_block, buffer);
			
			fprintf(output, "\taddr_%x [shape=diamond; label=\"%s\"]; //COND\n", PC, instr.disassm);
			
			if(addr_curr_block!=PC)
				fprintf(output, "\taddr_%x -> addr_%x;\n", addr_curr_block, PC);
			
			fprintf(output, "\taddr_%x -> addr_%x [%s=\"false\"]; //cond false\n", PC, addr_next_instr(PC, &instr), EDGE_LABEL);
			
			fprintf(output, "\taddr_%x -> addr_%x [%s=\"true\"]; //cond true\n", PC, addr_target, EDGE_LABEL);
			
			buffer[0]='\0';
						
			addr_curr_block=addr_next_instr(PC, &instr);
			
			if(!is_known_target(addr_target))
				parse(addr_target);
		}
		else if((instr.opc==OPC_J && instr.sub==SUB_J_J) || (instr.opc==OPC_JREG && instr.sub==SUB_JREG_JR)) //UNCOND JUMP
		{
			if(strlen(buffer))
				strcat(buffer, "\\n");
			strcat(buffer, instr.disassm);
			
			uint32_t addr_target=get_target(PC, &instr);
						
			fprintf(output, "\taddr_%x [shape=box; label=\"%s\"];\n", addr_curr_block, buffer);
			
			fprintf(output, "\taddr_%x -> addr_%x;\n", addr_curr_block, addr_target);
			
			buffer[0]='\0';
						
			if(!is_known_target(addr_target))
				parse(addr_target);
			
			return;
		}
		else
		{	
			if(is_block_addr(PC))
			{
				if(strlen(buffer))
				{
					fprintf(output, "\taddr_%x [shape=box; label=\"%s\"];\n", addr_curr_block, buffer);
						
					fprintf(output, "\taddr_%x -> addr_%x;\n", addr_curr_block, PC);
					
					buffer[0]='\0';
				}
				addr_curr_block=PC;
			}
			
			if(strlen(buffer))
				strcat(buffer, "\\n");
			strcat(buffer, instr.disassm);
		}
		
		PC=addr_next_instr(PC, &instr);
	}
}

int main(int argc, char **argv)
{
	printf("This is codeflow version %s (c) 2020 by kitten_nb_five.\n", VERSION);
	printf("This programm is experimental and comes without any warranty.\n\n");

	if(argc!=2)
	{
		printf("usage: ./flow $addr_in_block_2\n");
		return 0;
	}
	
	uint32_t addr_start;
	sscanf(argv[1], "%x", &addr_start);
	if(addr_start==0)
	{
		printf("invalid addr 0\n");
		return 0;
	}	
	
	FILE *f=fopen("block2.bin", "rb");
	if(!f)
		ERR(1, "fopen block2.bin");
		
	fseek(f, 0, SEEK_END);
	fsize=ftell(f);
	fseek(f, 0, SEEK_SET);
	
	firmware=malloc(fsize*sizeof(uint8_t));
	
	if(!firmware)
		ERR(1, "malloc for firmware\n");
	
	fread(firmware, fsize, 1, f);
	
	fclose(f);

	jump_targets=malloc(JUMP_TARGETS_MAX*sizeof(uint32_t));
	if(!jump_targets)
		ERR(1, "malloc jump_targets");

	block_addr=malloc(BLOCK_ADDR_MAX*sizeof(uint32_t));
	if(!block_addr)
		ERR(1, "malloc block_addr");

	output=fopen("flow.txt", "w");
	if(!output)
		ERR(1, "fopen flow.txt\n");
	
	printf("parsing function at 0x%x\n", addr_start);
	
	printf("looking for jump targets...");
	get_blocks(addr_start);
	printf("done\n");
	
	fprintf(output, "digraph flow {\n");
	
#ifdef SPLINES_ORTHO
	fprintf(output, "\tsplines=ortho;\n");
#endif
	
	fprintf(output, "\tstart [shape=oval; rang=min; label=\"start 0x%x\"];\n", addr_start);
	
	fprintf(output, "\tstart -> addr_%x;\n", addr_start);
	
	printf("parsing file...");
	parse(addr_start);
	printf("done\n");
	
	fprintf(output, "}\n");
	
	fclose(output);
	free(jump_targets);
	free(block_addr);
	
	printf("flow.txt written\n");
	
	return 0;
}
