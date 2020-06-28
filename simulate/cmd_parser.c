/*
This file is part of the simulator for the LKV373
(c) 2019-2020 by kitten_nb_five
freenode #lkv373a

licence: AGPL v3 or later

THIS PROGRAM COMES WITHOUT ANY WARRANTY!
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "cmd_parser.h"
#include "my_err.h"
#include "main.h"
#include "disassembly_view.h"
#include "reg_view.h"
#include "mem_view.h"
#include "stack_view.h"
#include "simulate.h"
#include "dispatcher.h"
#include "breakpoints.h"
#include "memory.h"
#include "window.h"
#include "special_regs.h"
#include "verbosity.h"
#include "mac.h"
#include "os_func.h"
#include "uart.h"

char cmd[SZ_BUFFER_COMMAND];
char args[NB_ARGUMENTS_MAX][SZ_BUFFER_ARGUMENTS];
uint8_t nb_args=0;
uint8_t index_args=0;

const command_t commands[]=
{
	{"quit", 0, 0, &stop},
	{"exit", 0, 0, &stop},
	{"r", 0, 0, &refresh},
	{"mem", 1, 1, &mem_show_addr},
	{"stack", 0, 1, &stack_show_addr},
	{"s", 0, 0, &single_step},
	{"asm", 0, 0, &disassm_reset_view},
	{"run", 1, 1, &run_sim},
	{"bp", 2, 6, &cmd_breakpoint},
	{"msp", 2, 2, &memory_set_page},
	{"reg", 2, 2, &write_register},
	{"byte", 2, 2, &memory_write_value},
	{"word", 2, 2, &memory_write_value},
	{"read", 1, 1, &execute_script_cmd},
	{"layout", 2, 2, &cmd_layout},
	{"dump", 3, 3, &memory_dump_region},
//	{"writesr", 2, 2, &cmd_write_sr}, //unimplemented
	{"setpc", 1, 1, &set_pc_cmd}, //use this with great caution, maybe this should be removed?
	{"verbosity", 2, 10, &cmd_set_verbosity},
	{"os", 1, 1, &show_os_info},
	{"u2f", 1, 1, &uart_to_file},
	{"rx", 1, 1, &rx},
	
	{NULL, 0, 0, NULL}
};

#define SZ_LINE_BUFFER 250

uint8_t execute_script(char const * const filename)
{
	FILE *f=fopen(filename, "r");
	if(!f)
	{
		MSG(MSG_ALWAYS, "can't open %s!\n", filename);
		return 1;
	}
	
	char line[SZ_LINE_BUFFER];
	
	while(fgets(line, SZ_LINE_BUFFER, f))
	{
		if(line[strlen(line)-1]=='\n')
		line[strlen(line)-1]='\0';
		if(line[0]=='#')
			continue;
		if(strlen(line)==0)
			continue;
		MSG(MSG_ALWAYS, "executing %s\n", line);
		if(parse_input(line))
			return 1;
	}
	
	fclose(f);
	
	return 0;
}

void execute_script_cmd(PROTOTYPE_ARGS_HANDLER)
{
	ARGS_HANDLER_UNUSED;
	
	execute_script(get_next_argument());
}

uint8_t parse_input(char * const inp)
{
	if(inp[strlen(inp)-1]=='\n')
		inp[strlen(inp)-1]='\0';
	
	nb_args=0;
	index_args=0;
	
	char *ptr;
	
	ptr=strtok(inp, " ");
	if(!ptr)
		return 0;

	strncpy(cmd, ptr, SZ_BUFFER_COMMAND-1);
	cmd[SZ_BUFFER_COMMAND-1]='\0';
	
	while((ptr=strtok(NULL, " ")) && nb_args<NB_ARGUMENTS_MAX)
	{
		strncpy(args[nb_args], ptr, SZ_BUFFER_ARGUMENTS-1);
		args[nb_args++][SZ_BUFFER_ARGUMENTS-1]='\0';
	}
	
	uint8_t i;
	uint8_t found=0;
	for(i=0; commands[i].cmd; i++)
	{
		if(!strcmp(cmd, commands[i].cmd))
		{
			found=1;
			break;
		}
	}
	if(!found)
	{
		MSG(MSG_ALWAYS, "unknown command %s\n", cmd);
		return 1;
	}
	
	if(nb_args<commands[i].nb_args_min)
	{
		MSG(MSG_ALWAYS, "not enough arguments for %s\n", cmd);
		return 1;
	}
	if(nb_args>commands[i].nb_args_max)
	{
		MSG(MSG_ALWAYS, "too many arguments for %s\n", cmd);
		return 1;
	}
	
	if(!commands[i].handler)
		ERRX(1, "no handler specified for command %s", cmd);
	
	commands[i].handler(cmd, nb_args);
	
	return 0;
}

char * get_next_argument(void)
{
	if(index_args<nb_args)
		return args[index_args++];
	else
		return NULL;
}

char * peek_next_argument(void)
{
	if(index_args<nb_args)
		return args[index_args];
	else
		return NULL;
}

uint8_t get_number_remaining_args(void)
{
	return nb_args-index_args;
}
