/*
This file is part of a disassembler for Andestech NDS32.

(c) 2019-2020 kitten_nb_five

THIS WORK COMES WITHOUT ANY WARRANTY and is released under the AGPL version 3 or later

This file is just a quick example of how to use all this code. Some stuff has been omitted for brevety.
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "instruction.h"
#include "decode_disassm.h"

#define INVERT_ENDIAN 1

uint8_t *buffer;

uint32_t memory_get_word(const uint32_t addr)
{
#ifdef INVERT_ENDIAN
	return (buffer[addr]<<24)|(buffer[addr+1]<<16)|(buffer[addr+2]<<8)|buffer[addr+3];
#else
	return (buffer[addr+3]<<24)|(buffer[addr+2]<<16)|(buffer[addr+1]<<8)|buffer[addr];
#endif
}

int main(int argc, char **argv)
{
	instr_t instr;
	uint32_t PC=0;
	
	if(argc<2)
	{
		printf("first and only argument: file to disassemble\n");
		return 1;
	}
	
	buffer=malloc(100*sizeof(uint8_t));
	
	FILE *f=fopen(argv[1], "rb");
	if(!f)
	{
		printf("could not open file\n");
		return 1;
	}
	
	fread(buffer, 100*sizeof(uint8_t), 1, f);
	
	fclose(f);
	
	while(PC<100)
	{
		if(decode_instr(&instr, PC))
		{
			printf("error!\n");
			break;
		}
		
		printf("%s\n", instr.disassm);
		
		if(instr.width==WIDTH16)
			PC+=2;
		else
			PC+=4;
	}
	
	free(buffer);
	
	return 0;
}
