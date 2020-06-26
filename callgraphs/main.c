/*
make callgraphs for functions inside the LKV373-firmware using graphviz
(c) 2020 by kitten_nb_five
freenode #lkv373a
NO WARRANTY!

licence: AGPL version 3 or later

THIS IS EXPERIMENTAL STUFF!

Please see README for how to compile and use

known limitation:
If a register-value for a call is written to and then read from memory it will not be catched because memory-IO is not emulated.
To fix this we would need to implement all the memory-IO with is a lot of work.
Also calls/jumps to adresses computed at runtime (like jumptables) will not be seen. We would need a complete simulator for this.
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "instruction.h"
#include "decode_disassm.h"
#include "opcodes32.h"
#include "sign_extend.h"

#define VERSION 2

uint8_t * firmware;
uint32_t fsize;

uint32_t decode_disassm_memory_get_word(const uint32_t addr)
{
	if(addr>fsize)
	{
		printf("error: memory read from addr 0x%x is out of range (size is 0x%x)\n", addr, fsize);
		exit(1);
	}
	uint32_t value=(firmware[addr]<<24)|(firmware[addr+1]<<16)|(firmware[addr+2]<<8)|firmware[addr+3];
	
	return value;
}

uint8_t is_valid(const uint32_t addr)
{
	if(addr==0 || addr==0xffffffff)
		return 0;
	else
		return 1;
}

//If we jump to an address we put it here so we know we already went there. This is needed to avoid endless-loops
#define JUMP_TARGETS_MAX 5000
uint32_t * jump_targets;
uint32_t nb_jmp_targets;

void push(const uint32_t addr)
{
	if(nb_jmp_targets<JUMP_TARGETS_MAX)
		jump_targets[nb_jmp_targets++]=addr;
	else
	{
		printf("jump_targets is full, increase JUMP_TARGETS_MAX\n");
		exit(1);
	}
}

uint8_t is_known_target(const uint32_t addr)
{
	uint32_t i;
	for(i=0; i<nb_jmp_targets; i++)
		if(jump_targets[i]==addr)
			return 1;
	
	return 0;
}

uint32_t regs[32];

#define ADDR_STOP 0xffffffff

FILE *output;

uint8_t print_disassm=0;
uint8_t print_calls=0;

uint32_t go(uint32_t PC, const uint32_t LP, const uint32_t start_of_func)
{
	if(PC==ADDR_STOP)
		return 0;
		
	uint32_t regs_save[32];
	memset(regs_save, 0xff, 32*sizeof(uint32_t));
	
	instr_t instr;
	uint32_t dest;
	uint8_t is_jump=0;
	uint8_t run=1;
	
	while(run)
	{
		decode_instr(&instr, PC);
		
		if(print_disassm)
			printf("%x: %s\n", PC, instr.disassm);

		is_jump=0;
		
		switch(instr.opc)
		{
			//INSTR LOAD REG
			
			case OPC_SETHI:
				regs[instr.rt]=instr.imm1_20<<12;
				break;
				
			case OPC_ORI:
				regs[instr.rt]=regs[instr.ra]|instr.imm1_15;
				break;
			
			case OPC_MOVI:
				regs[instr.rt]=nds32_sign_extend(instr.imm1_20, 20, 32);
				break;
			
			case OPC_ADDI: //MOV=ADDI r,r,0
				regs[instr.rt]=regs[instr.ra]+nds32_sign_extend(instr.imm1_15, 15, 32);
				break;
			
			//we don't simulate memory-IO so we have no idea what is being loaded into this register. mark it as invalid jump-target
			case OPC_LWI:
			case OPC_LWI_bi:
				regs[instr.rt]=ADDR_STOP;
				break;
			case OPC_MEM:
				switch(instr.sub)
				{
					case SUB_MEM_LW:
					case SUB_MEM_LW_bi:
						regs[instr.rt]=ADDR_STOP;
						break;
				}
				break;
			
			
			//UNCOND JUMP/CALL
			
			case OPC_J:
				switch(instr.sub)
				{
					case SUB_J_J: //JUMP
						dest=PC+nds32_sign_extend(instr.imm1_24<<1, 25, 32);
						if(!is_known_target(dest))
						{
							push(dest);
							PC=dest;
							is_jump=1;
						}
						else
						{
							run=0;
							is_jump=1;
						}
						break;
						
					case SUB_J_JAL: //CALL
						dest=PC+nds32_sign_extend(instr.imm1_24<<1, 25, 32);
						if(!is_valid(dest))
						{
							printf("%x: CALL to invalid addr %x ignored!\n", PC, dest);
						}
						else if(!is_known_target(dest))
						{
							push(dest);
							memcpy(regs_save, regs, 32*sizeof(uint32_t));
							if(print_calls)
								printf("%x: CALL %x\n", PC, dest);
							fprintf(output, "\tfunc_%x -> func_%x;\n", start_of_func, dest);
							PC=go(dest, PC+(instr.was_16bit_instr?2:4), dest);
							memcpy(regs, regs_save, 32*sizeof(uint32_t));
							is_jump=1;
						}
						break;
					//J is completed
				}
				break;
			
			case OPC_JREG: //T4_SUB
				switch(instr.sub)
				{
					case SUB_JREG_JR: //JUMP
						dest=regs[instr.rb];
						if(!is_known_target(dest))
						{
							push(dest);
							PC=dest;
							is_jump=1;
						}
						else
						{
							run=0;
							is_jump=1;
						}
						break;
					
					case SUB_JREG_RET: //RETURN
						run=0;
						is_jump=1;
						break;
					
					case SUB_JREG_JRAL: //CALL
						dest=regs[instr.rb];
						if(!is_valid(dest))
						{
							printf("%x: CALL to invalid addr %x ignored!\n", PC, dest);
						}
						else if(!is_known_target(dest))
						{
							push(dest);
							memcpy(regs_save, regs, 32*sizeof(uint32_t));
							if(print_calls)
								printf("%x: CALL %x\n", PC, dest);
							fprintf(output, "\tfunc_%x -> func_%x;\n", start_of_func, dest); fflush(output);
							PC=go(dest, PC+(instr.was_16bit_instr?2:4), dest);
							memcpy(regs, regs_save, 32*sizeof(uint32_t));
							is_jump=1;
						}
						break;
				
					//JREG is completed
				}
				break;
			
			case OPC_MISC:
				switch(instr.sub)
				{
					case SUB_MISC_IRET:
						run=0;
						is_jump=1;
						break;
				}
				break;
				
			
			//COND JUMP
			
			case OPC_BR1:
				dest=PC+nds32_sign_extend(instr.imm1_14<<1, 15, 32);
				if(!is_known_target(dest))
				{
					push(dest);
					memcpy(regs_save, regs, 32*sizeof(uint32_t));
					go(dest, PC+(instr.was_16bit_instr?2:4), start_of_func);
					memcpy(regs, regs_save, 32*sizeof(uint32_t));
				}
				/* else
				{
					run=0;
					is_jump=1;
				} */
				break;
			
			case OPC_BR2:
				dest=PC+nds32_sign_extend(instr.imm1_16<<1, 17, 32);
				if(!is_known_target(dest))
				{
					push(dest);
					memcpy(regs_save, regs, 32*sizeof(uint32_t));
					go(dest, PC+(instr.was_16bit_instr?2:4), start_of_func);
					memcpy(regs, regs_save, 32*sizeof(uint32_t));
				}
				/* else
				{
					run=0;
					is_jump=1;
				} */
				break;
			
			default:
				break;
		}
			
		if(!is_jump)
		{
			if(instr.was_16bit_instr)
				PC+=2;
			else
				PC+=4;
		}
		
	}
		
	return LP;
}

void print_usage(void)
{
	printf("callgraph [-pd] [-pc] file address_in_hex \n-pd print disassembly\n-pc print calls\n");
}

#define RUN(addr) go(addr, ADDR_STOP, addr)

int main(int argc, char **argv)
{
	printf("This is callgraph version %u (c) 2020 by kitten_nb_five.\n", VERSION);
	printf("This programm is experimental and comes without any warranty.\n\n");
	
	if(argc<3)
	{
		print_usage();
		return 1;
	}
	
	uint8_t argv_offset=1;	
	
	while(argv_offset<argc-2)
	{
		if(strcmp(argv[argv_offset],"-pd")==0)
		{
			print_disassm=1;
			argv_offset++;
		}
		else if(strcmp(argv[argv_offset],"-pc")==0)
		{
			print_calls=1;
			argv_offset++;
		}
		else
		{
			printf("invalid argument: %s\n", argv[argv_offset]);
			return 1;
		}
	}
	
	char * filename=argv[argv_offset];
	
	uint32_t addr;
	sscanf(argv[argv_offset+1], "%x", &addr);
	if(!is_valid(addr))
	{
		printf("invalid addr: 0x%x\n", addr);
		return 1;
	}
	
	printf("parsing calls for addr 0x%x from file %s\n", addr, filename);
	
	FILE *f=fopen(filename, "rb");
	if(!f)
	{
		printf("can't open file\n");
		exit(1);
	}
	
	fseek(f, 0, SEEK_END);
	fsize=ftell(f);
	fseek(f, 0, SEEK_SET);
	
	firmware=malloc(fsize*sizeof(uint8_t));
	
	if(!firmware)
	{
		printf("malloc for firmware failed\n");
		exit(1);
	}
	
	fread(firmware, fsize, 1, f);
	
	fclose(f);
	
	jump_targets=malloc(JUMP_TARGETS_MAX*sizeof(uint32_t));
	nb_jmp_targets=0;
	if(!jump_targets)
	{
		printf("malloc for jump_targets failed\n");
		exit(1);
	}

	output=fopen("calls.txt", "w");
	if(!output)
	{
		printf("opening output file failed\n");
		exit(1);
	}
	
	memset(regs, 0, 32*sizeof(uint32_t));
	
	fprintf(output, "#file is %s\n", filename);
	fprintf(output, "digraph calls {\n");
	
	RUN(addr);
		
	fprintf(output, "}\n");
	
	fclose(output);
	
	printf("parsing done, data file written\n");
	
	return 0;
}
