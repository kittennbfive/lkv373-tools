/*
This file is part of the simulator for the LKV373
(c) 2020 by kitten_nb_five
freenode #lkv373a

licence: AGPL v3 or later

THIS PROGRAM COMES WITHOUT ANY WARRANTY!
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "hooks.h"

#include "simulate.h"
#include "verbosity.h"

static void func_memcmp(void)
{
	uint32_t ptr1=get_register(0);
	uint32_t ptr2=get_register(1);
	uint32_t length=get_register(2);
	MSG(MSG_HOOKS, "memCMP 0x%x, 0x%x len 0x%x bytes\n", ptr1, ptr2, length);
}	

static void func_memcpy(void)
{
	uint32_t dest=get_register(0);
	uint32_t src=get_register(1);
	uint32_t length=get_register(2);
	MSG(MSG_HOOKS, "copy 0x%x -> 0x%x len 0x%x bytes\n", src, dest, length);
}

static void func_strcmp(void)
{
	uint32_t str1=get_register(0);
	uint32_t str2=get_register(1);
	MSG(MSG_HOOKS, "strCMP 0x%x, 0x%x\n", str1, str2);
}

typedef struct
{
	uint32_t PC;
	char * func_name;
	void (*callback)(void);
} func_t;

static const func_t list[]=
{
	{ 0x1a798, "memcmp()", &func_memcmp },
	{ 0x1a980, "memcpy()", &func_memcpy },
//	{ 0x1aa64, "memset()", &func_memset },
	{ 0x1ab08, "strcmp()", &func_strcmp },
//	{ 0x1ad10, "strlen()", &func_strlen },
	
	
	
	{0, NULL, NULL}
};

void hooks(const uint32_t PC)
{
	uint8_t i;
	for(i=0; list[i].PC; i++)
	{
		if(list[i].PC==PC)
		{
			list[i].callback();
			break;
		}
	}
}
