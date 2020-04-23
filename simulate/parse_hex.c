/*
This file is part of the simulator for the LKV373
(c) 2019-2020 by kitten_nb_five
freenode #lkv373a

licence: AGPL v3 or later

THIS PROGRAM COMES WITHOUT ANY WARRANTY!
*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include "parse_hex.h"

uint8_t is_valid_hex(char const * const str)
{
	if(!str)
	{
		printf("is_valid_hex: ptr is NULL\n");
		return 0;
	}
	
	char const *ptr=str;
	
	if(!memcmp(ptr, "0x", 2))
		ptr+=2;
	
	uint8_t i;
	
	for(i=0; i<strlen(ptr); i++)
	{
		if(!isxdigit(ptr[i]))
			return 0;
	}
	
	return 1;
}

uint8_t parse_hex(char const * const str, uint32_t * const val)
{
	if(!str)
	{
		printf("hex value missing (ptr is NULL)\n");
		return 1;
	}

	char const *ptr=str;
	
	if(!memcmp(ptr, "0x", 2))
		ptr+=2;
	
	uint8_t i;
	
	for(i=0; i<strlen(ptr); i++)
	{
		if(!isxdigit(ptr[i]))
		{
			printf("invalid hex value\n");
			return 1;
		}
	}
	
	sscanf(ptr, "%x", val);
	
	return 0;
}

uint8_t parse_register(char const * const str, uint8_t * const reg)
{
	if(!str)
	{
		printf("register missing (ptr is NULL)\n");
		return 1;
	}

	
	char const *ptr=str;
	if(*ptr=='r')
		ptr++;
	
	unsigned int val; //just to make the compiler happy, sscanf expects unsigned int
	sscanf(ptr, "%u", &val);
	
	if(val==0 && (*ptr!='0'))
	{
		printf("invalid register\n");
		return 1;
	}
	
	if(val>31)
	{
		printf("invalid register\n");
		return 1;
	}
	
	(*reg)=val;
	
	return 0;
}
