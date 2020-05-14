/*
(c) 2020 by kitten_nb_five
freenode #lkv373a

licence: AGPL v3 or later

THIS CODE COMES WITHOUT ANY WARRANTY!
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>

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
	
	if(str[0]=='?')
	{
		memcpy(&sz, &str[1], sizeof(unsigned char));
		memcpy(&addr, &str[2], sizeof(unsigned int));
		
		unsigned int val=readmem(sz, addr);
		
		printf("ok");
		fwrite(&val, sizeof(unsigned int), 1, stdout);
	}
	else if(str[0]=='=')
	{
		memcpy(&sz, &str[1], sizeof(unsigned char));
		memcpy(&addr, &str[2], sizeof(unsigned int));
		memcpy(&val, &str[6], sizeof(unsigned int));
		
		writemem(sz, addr, val);
		
		printf("ok\xab\xcd\xef\x12");
	}
	else
		printf("err%c\r\n", str[0]);
}
