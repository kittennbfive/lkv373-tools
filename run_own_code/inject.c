/*
This is part of a "toolchain" to inject arbitrary code into the LKV373 firmware. Please see README.TXT

(c) 2020 kitten_nb_five

licence: AGPL v3 or later

version 2 with bugfix

THIS WORK IS EXPERIMENTAL AND COMES WITHOUT ANY WARRANTY!
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "my_err.h"

uint8_t *buffer;

#define BLOCK2_FILESIZE 561952

//put code somewhere where non-critical (??) strings are
//we can't put it at the end of the existing code as the space is overwritten with 0 and then used for data (RAM)
#define POSITION_NEW_CODE 0x55dc2

//for testing: the version-info call is at 8a64

uint32_t eof_addr;

void inj(const uint32_t inject_at_addr, const uint32_t addr_jumptarget)
{
	printf("inj %x->%x\n", inject_at_addr, addr_jumptarget);
	
	//check that inject_at_addr points to a 4 byte instr
	if(buffer[inject_at_addr]&(1<<7))
		ERRX(1, "addr 0x%x points to 2 byte instr, this is unsupported", inject_at_addr);
		
	//add instr to be overwritten at the end of inj code
	printf("saving instr at 0x%x to be overwritten at 0x%x\n", inject_at_addr, POSITION_NEW_CODE+eof_addr);
	memcpy(&buffer[POSITION_NEW_CODE+eof_addr], &buffer[inject_at_addr], 4);
	
	//overwrite instr with jump
	int32_t offset_for_jump=POSITION_NEW_CODE+addr_jumptarget-inject_at_addr;
	printf("offset for jump to start of injected code is %d\n", offset_for_jump);
	uint8_t jmp_instr[4]={0b01001000, 0, 0, 0};
	offset_for_jump>>=1;
	jmp_instr[1]=(offset_for_jump>>16)&0xff;
	jmp_instr[2]=(offset_for_jump>>8)&0xff;
	jmp_instr[3]=(offset_for_jump>>0)&0xff;
	memcpy(&buffer[inject_at_addr], jmp_instr, 4);
	
	//add return jump at the end of injected code
	offset_for_jump=inject_at_addr-(POSITION_NEW_CODE+eof_addr);
	printf("offset for jump back to existing code is %d\n", offset_for_jump);
	offset_for_jump>>=1;
	jmp_instr[1]=(offset_for_jump>>16)&0xff;
	jmp_instr[2]=(offset_for_jump>>8)&0xff;
	jmp_instr[3]=(offset_for_jump>>0)&0xff;
	memcpy(&buffer[POSITION_NEW_CODE+eof_addr+4], jmp_instr, 4);
	
	eof_addr+=2*4;
}

int main(int argc, char **argv)
{
	if(argc<6)
	{
		printf("arguments:\n\t1)unmodified firmware file\n\t2)code to inject (.bin)\n\t3)name of output file\n\t4)addr of eof (dez!)\n\t5)number of injections (dez!)\n\t6...n)addr_to_inj jumptarget (dez!)\n(use assemble.pl!)\n");
		return 0;
	}
	
	char *filename_firmware=argv[1];
	char *filename_code_to_inj=argv[2];
	char *filename_output=argv[3];
	
	eof_addr=atoi(argv[4]);
	if(eof_addr==0)
		ERRX(1, "invalid eof_addr");
	
	printf("eof_addr is %u\n", eof_addr);
	
	uint8_t nb_inj=atoi(argv[5]);
	if(nb_inj==0)
		ERRX(1, "invalid nb_inj");
	
	if(argc<(6+2*nb_inj))
		ERRX(1, "missing injection parameters");
	
	FILE *firmware=fopen(filename_firmware, "rb");
	if(!firmware)
		ERR(1, "could not open file");
	buffer=malloc(BLOCK2_FILESIZE*sizeof(uint8_t));
	fread(buffer, BLOCK2_FILESIZE*sizeof(uint8_t), 1, firmware);
	fclose(firmware);
	
	uint32_t filesize_to_inject;
	FILE *to_inject=fopen(filename_code_to_inj, "rb");
	if(!to_inject)
		ERR(1, "could not open file");
	fseek(to_inject, 0, SEEK_END);
	filesize_to_inject=ftell(to_inject);
	fseek(to_inject, 0, SEEK_SET);
	uint32_t size_buffer=filesize_to_inject+2*4*nb_inj; //for each inj we need 4 bytes for the overwritten instr and 4 bytes for return jump to actual code
	uint8_t *buf_to_inject=malloc(size_buffer*sizeof(uint8_t));
	fread(buf_to_inject, filesize_to_inject*sizeof(uint8_t), 1, to_inject);
	fclose(to_inject);
	printf("filesize of code to be injected is %u, size_buffer is %u\n", filesize_to_inject, size_buffer);
	
	memcpy(&buffer[POSITION_NEW_CODE], buf_to_inject, filesize_to_inject);
	
	printf("injecting...\n");
	uint8_t i;
	for(i=0; i<nb_inj; i++)
		inj(atoi(argv[6+2*i]), atoi(argv[6+2*i+1]));
	
	
	printf("writing output file\n");
	FILE *output=fopen(filename_output, "wb");
	if(!output)
		ERR(1, "could not open file");
	fwrite(buffer, BLOCK2_FILESIZE*sizeof(uint8_t), 1, output);
	fclose(output);
	
	free(buf_to_inject);
	free(buffer);
	
	printf("file written\n");
	return 0;
}
