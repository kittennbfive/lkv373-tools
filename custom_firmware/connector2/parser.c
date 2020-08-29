/*
(c) 2020 by kitten_nb_five
freenode #lkv373a

licence: AGPL v3 or later

THIS CODE COMES WITHOUT ANY WARRANTY!
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "jpeg_quant.h"
#include "i2c_ite.h"

static unsigned int readmem(const unsigned char sz, const unsigned int addr)
{
	if(sz==4)
		return *((volatile unsigned int *)addr);
	else if(sz==2)
		return *((volatile unsigned short *)addr);
	else
		return *((volatile unsigned char *)addr);
}

static void writemem(const unsigned char sz, const unsigned int addr, const unsigned int val)
{
	if(sz==4)
		*((volatile unsigned int *)addr)=val;
	else if(sz==2)
		*((volatile unsigned short *)addr)=val;
	else
		*((volatile unsigned char *)addr)=val;
}

void parse(unsigned char const * const str)
{
	unsigned char sz;
	unsigned int addr;
	unsigned int val;
	
	if(str[0]=='?') //read memory location
	{
		memcpy(&sz, &str[1], sizeof(unsigned char));
		memcpy(&addr, &str[2], sizeof(unsigned int));
		
		unsigned int val=readmem(sz, addr);
		
		printf("ok");
		fwrite(&val, sizeof(unsigned int), 1, stdout);
	}
	else if(str[0]=='=') //write memory location
	{
		memcpy(&sz, &str[1], sizeof(unsigned char));
		memcpy(&addr, &str[2], sizeof(unsigned int));
		memcpy(&val, &str[6], sizeof(unsigned int));
		
		writemem(sz, addr, val);
		
		printf("ok\xab\xcd\xef\x12");
	}
	else if(str[0]=='Q') //write quantization tables for MJPEG quality
	{
		unsigned char Q;
		unsigned char unknown4[4];
		
		memcpy(&Q, &str[1], sizeof(unsigned char));
		memcpy(unknown4, &str[2], 4*sizeof(unsigned char));
		
		set_q(Q, unknown4);
		
		printf("ok\xab\xcd\xef\x12");
	}
	else if(str[0]=='I') //communicate with ITE-chip over I2C
	{
		unsigned char read_write=str[1];
		unsigned char reg=str[2];
		unsigned char val=str[3];
		
		if(read_write=='r') //read single register
		{
			val=ite_read_register(reg);
			printf("ok");
			fwrite(&val, sizeof(unsigned char), 1, stdout);
		}
		else if(read_write=='w') //write single register
		{
			ite_write_register(reg, val);
			printf("ok\xaa\xbb\xcc\xdd");
		}
		else
			printf("erri%c\r\n", read_write);
	}
	else
		printf("err%c\r\n", str[0]);
}
