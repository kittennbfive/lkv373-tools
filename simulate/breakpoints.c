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
#include <ctype.h>
#include <stdbool.h>

#include "breakpoints.h"
#include "simulate.h"
#include "memory.h"
#include "my_err.h"
#include "window.h"
#include "ansi.h"
#include "cmd_parser.h"
#include "parse_hex.h"

static int w_bp;
static event_t bp[NB_BREAKPOINTS_MAX];
static uint32_t nb_bp;

int init_breakpoints_view(void)
{
	w_bp=open_new_window("breakpoints", NULL, NULL);
	nb_bp=0;
	
	return w_bp;
}

//TODO: test this
static uint8_t event_compare(event_t const * const ev1, event_t const * const ev2) //returns equal if different actions for same bp
{
	if(ev1->type!=ev2->type)
		return 0;
	if((ev1->type==MEMORY_READ || ev1->type==MEMORY_WRITE) && ev1->type_addr_mem!=ev2->type_addr_mem)
		return 0;
	if((ev1->type==MEMORY_READ || ev1->type==MEMORY_WRITE) && ev1->type_addr_mem==ADDR && ev1->addr!=ev2->addr)
		return 0;
	if((ev1->type==MEMORY_READ || ev1->type==MEMORY_WRITE) && ev1->type_addr_mem==ADDR && ev1->type_match_mem!=ev2->type_match_mem)
		return 0;
	if((ev1->type==MEMORY_READ || ev1->type==MEMORY_WRITE) && ev1->type_addr_mem==ADDR && (ev1->type_match_mem==IS_VALUE || ev1->type_match_mem==IS_NOT_VALUE) && ev1->value!=ev2->value)
		return 0;
	if((ev1->type==MEMORY_READ || ev1->type==MEMORY_WRITE) && ev1->type_addr_mem==RANGE && (ev1->range_from!=ev2->range_from || ev1->range_to!=ev2->range_to)) //todo: overlapping ranges
		return 0;
	if((ev1->type==REGISTER_IS_VALUE || ev1->type==REGISTER_IS_NOT_VALUE) && (ev1->reg!=ev2->reg || ev1->value!=ev2->value))
		return 0;
	if(ev1->type==PC_IS_VALUE && ev1->PC!=ev2->PC)
		return 0;
	
	return 1;
}

static void register_breakpoint(event_t const * const ev)
{
	if(nb_bp>=NB_BREAKPOINTS_MAX)
	{
		printf("no space left for more breakpoints, increase NB_BREAKPOINTS_MAX\n");
		return;
	}
	
	uint8_t i;
	for(i=0; i<nb_bp; i++)
	{
		if(event_compare(&bp[i], ev))
		{
			printf("breakpoint already exists (nb %d)\n", i+1);
			return;
		}
	}
	
	memcpy(&bp[nb_bp], ev, sizeof(event_t));
	nb_bp++;
	
	printf("breakpoint saved, total %u\n", nb_bp);
}

void print_bp_memory(const uint8_t i)
{
	switch(bp[i].type_addr_mem)
	{
		case ADDR:
			win_printf(w_bp, "[0x%08x]", bp[i].addr);
			break;
		case RANGE:
			win_printf(w_bp, "[0x%08x-0x%08x]", bp[i].range_from, bp[i].range_to);
			break;
	}
	
	switch(bp[i].type_match_mem)
	{
		case IS_ACCESSED:
			win_printf(w_bp, " ACCESS");
			break;
		case IS_VALUE:
			win_printf(w_bp, "==0x%08x", bp[i].value);
			break;	
		case IS_NOT_VALUE:
			win_printf(w_bp, "!=0x%08x", bp[i].value);
			break;
		case IS_UNDEF:
			win_printf(w_bp, "==undef");
			break;
	}
}

void print_breakpoints(void)
{
	win_printf(w_bp, CLEAR_SCREEN);	
	win_printf(w_bp, HIDE_CURSOR);	
	
	if(nb_bp==0)
	{
		win_printf(w_bp, "no breakpoints defined\n");
		return;
	}
	
	uint8_t i;
	for(i=0; i<nb_bp; i++)
	{
		win_printf(w_bp, "%02d: ", i+1); //first bp is number 1, not 0
		
		if(bp[i].disabled)
			win_printf(w_bp, SET_COLOR, GREY);
				
		switch(bp[i].type)
		{
			case MEMORY_READ:
				win_printf(w_bp, "MEM READ ");
				print_bp_memory(i);
				break;
			case MEMORY_WRITE:
				win_printf(w_bp, "MEM WRITE ");
				print_bp_memory(i);
				break;
			case REGISTER_IS_VALUE:
				win_printf(w_bp, "REG r%02d==0x%08x", bp[i].reg, bp[i].value);
				break;
			case REGISTER_IS_NOT_VALUE:
				win_printf(w_bp, "REG r%02d!=0x%08x", bp[i].reg, bp[i].value);
				break;
			case PC_IS_VALUE:
				win_printf(w_bp, "PC==0x%08x", bp[i].PC);
				break;
		}
		
		switch(bp[i].action)
		{
			case MESSAGE:
				win_printf(w_bp, " ->MSG\n");
				break;
			case STOP:
				win_printf(w_bp, " ->STOP\n");
				break;
			case JUMP:
				win_printf(w_bp, " ->JUMP 0x%08x\n", bp[i].jump_target);
				break;
			case SCRIPT:
				win_printf(w_bp, " ->SCRIPT %s\n", bp[i].script);
				break;
		}
		
		win_printf(w_bp, RESET_COLOR);
	}
}

////////////////////////////////////////////////////////////////////////

static uint8_t bp_parse_action(event_t * const ev)
{
	char * arg=get_next_argument();
	if(!arg)
	{
		printf("missing action\n");
		return 1;
	}
	
	if(!strcmp(arg, "msg"))
	{
		ev->action=MESSAGE;
	}
	else if(!strcmp(arg, "stop"))
	{
		ev->action=STOP;
	}
	else if(!strcmp(arg, "jump"))
	{
		ev->action=JUMP;
		if(get_number_remaining_args()==0)
		{
			printf("missing jump target address\n");
			return 1;
		}
		if(parse_hex(get_next_argument(), &ev->jump_target))
			return 1;
	}
	else if(!strcmp(arg, "script"))
	{
		ev->action=SCRIPT;
		if(get_number_remaining_args()==0)
		{
			printf("missing script name\n");
			return 1;
		}
		strncpy(ev->script, get_next_argument(), SZ_BUF_BP_NAME_SCRIPT);
	}
	else
	{
		printf("invalid action\n");
		return 1;
	}
	
	return 0;
}

#define SZ_BUFFER 25

static uint8_t bp_parse_mem_rw(event_t * const ev)
{
	char * arg=get_next_argument();
	
	if(!arg)
	{
		printf("missing address\n");
		return 1;
	}
	
	char buffer[SZ_BUFFER];
	strncpy(buffer, arg, SZ_BUFFER-1);
		
	char *is_range=strchr(buffer, '-');
	if(is_range)
	{
		ev->type_addr_mem=RANGE;

		if(parse_hex(is_range+1, &ev->range_to))
			return 1;
		
		is_range[0]='\0';
		
		if(parse_hex(buffer, &ev->range_from))
			return 1;

	}
	else
	{
		if(parse_hex(buffer, &ev->addr))
			return 1;
	}
	
	//printf("addr or range parsed\n");
	
	arg=peek_next_argument();
	if(!arg)
	{
		printf("missing value or action\n");
		return 1;
	}
	
	//printf("parsing %s\n", arg);
	
	if(arg[0]=='!')
	{
		(void)get_next_argument(); //remove peeked arg it from list
		ev->type_match_mem=IS_NOT_VALUE;
		if(parse_hex(&arg[1], &ev->value))
			return 1;
		return 0;
	}
	else if(!strcmp(arg, "undef"))
	{
		(void)get_next_argument(); //remove peeked arg it from list
		ev->type_match_mem=IS_UNDEF;
		return 0;
	}
	else if(is_valid_hex(arg))
	{
		(void)get_next_argument(); //remove peeked arg it from list
		ev->type_match_mem=IS_VALUE;
		if(parse_hex(arg, &ev->value))
			return 1;
		return 0;
	}
	else
	{
		//printf("type is is_accessed\n");
		ev->type_match_mem=IS_ACCESSED;
		return 0;
	}
	
	return 0;
}

static uint8_t bp_parse_reg_is_isnot_val(event_t * const ev)
{
	char buffer[25];
	strncpy(buffer, get_next_argument(), SZ_BUFFER-1);
	
	char *compare;
	char *is_equal=strstr(buffer, "==");
	char *is_not_equal=strstr(buffer, "!=");
	
	if(is_equal)
	{
		compare=is_equal;
		ev->type=REGISTER_IS_VALUE;
	}
	else if(is_not_equal)
	{
		compare=is_not_equal;
		ev->type=REGISTER_IS_NOT_VALUE;
	}
	else
	{
		printf("missing operator\n");
		return 1;
	}

	if(compare==buffer)
	{
		printf("missing address\n");
		return 1;
	}	
	
	compare[0]='\0';
	
	if(buffer[0]!='r')
	{
		printf("missing register\n");
		return 1;
	}
	
	if(!isdigit(buffer[1]) || (buffer[2] && !isdigit(buffer[2])))
	{
		printf("invalid register\n");
		return 1;
	}
	ev->reg=atoi(&buffer[1]);
	if(ev->reg>31)
	{
		printf("invalid register\n");
		return 1;
	}
	
	compare+=2;
	if((*compare)=='\0')
	{
		printf("missing value\n");
		return 1;
	}	
	if(parse_hex(compare, &ev->value))
		return 1;
		
	return 0;
}

static void create_bp_from_string(void)
{
	event_t ev;
	memset(&ev, 0, sizeof(event_t));
	
	char * arg=get_next_argument();
	
	if(!strcmp(arg, "read"))
	{
		ev.type=MEMORY_READ;
	
		if(bp_parse_mem_rw(&ev))
			return;
		//printf("about to call bp_parse_action\n");
		if(bp_parse_action(&ev))
			return;
	}
	else if(!strcmp(arg, "write"))
	{
		ev.type=MEMORY_WRITE;
				
		if(bp_parse_mem_rw(&ev))
			return;
		if(bp_parse_action(&ev))
			return;
	}
	else if(!strcmp(arg, "reg"))
	{
		if(bp_parse_reg_is_isnot_val(&ev))
			return;
		if(bp_parse_action(&ev))
			return;
	}
	else if(!strcmp(arg, "PC") || !strcmp(arg, "pc"))
	{
		ev.type=PC_IS_VALUE;
		if(parse_hex(get_next_argument(),&ev.PC))
			return;
		if(bp_parse_action(&ev))
			return;
	}
	else
	{
		printf("invalid event type\n");
		return;
	}
	
	register_breakpoint(&ev);
}

void cmd_breakpoint(PROTOTYPE_ARGS_HANDLER) //min 2 args guaranteed by cmd_parser
{
	(void)cmd; (void)nb_args;
	
	char * arg=get_next_argument();
	
	if(!strcmp(arg, "create"))
	{
		create_bp_from_string();
	}
	else if(!strcmp(arg, "remove"))
	{
		uint8_t nb=atoi(get_next_argument());
		if(nb==0 || nb>nb_bp)
		{
			printf("invalid breakpoint specified\n");
			return;
		}
		nb--; //first bp is called number 1 but has index 0 of course
		if(nb==nb_bp-1)
			nb_bp--;
		else
		{
			memmove(&bp[nb], &bp[nb+1], (nb_bp-nb-1)*sizeof(event_t));
			nb_bp--;
		}
	}
	else if(!strcmp(arg, "disable"))
	{
		arg=get_next_argument();
		if(!strcmp(arg, "all"))
		{
			uint8_t i;
			for(i=0; i<nb_bp; i++)
				bp[i].disabled=true;
		}
		else
		{
			uint8_t nb=atoi(arg);
			if(nb==0 || nb>nb_bp)
			{
				printf("invalid breakpoint specified\n");
				return;
			}
			bp[nb-1].disabled=true; //first bp is called number 1 but has index 0 of course
		}
	}	
	else if(!strcmp(arg, "enable"))
	{
		arg=get_next_argument();
		if(!strcmp(arg, "all"))
		{
			uint8_t i;
			for(i=0; i<nb_bp; i++)
				bp[i].disabled=false;
		}
		else
		{
			uint8_t nb=atoi(arg);
			if(nb==0 || nb>nb_bp)
			{
				printf("invalid breakpoint specified\n");
				return;
			}
			bp[nb-1].disabled=false;
		}
	}
	else
	{
		printf("set breakpoint: missing command\n");
		return;
	}

	print_breakpoints();
}

////////////////////////////////////////////////////////////////////////

void handle_breakpoint(const uint32_t nb_bp, uint32_t * const PC, bool * const PC_manipulated, bool * const stop)
{
	switch(bp[nb_bp].action)
	{
		case MESSAGE:
			printf("BP %u triggered!\n", nb_bp);
			break;
		
		case STOP:
			if(stop)
				*stop=true;
			else
			{
				printf("handle_bp: stop-ptr is NULL\n");
				exit(1);
			}
			break;
			
		case JUMP:
			printf("BP: jump to 0x%08x\n", bp[nb_bp].jump_target);
			*PC=bp[nb_bp].jump_target;
			*PC_manipulated=true;
			break;
		
		case SCRIPT:
			printf("BP: executing script %s\n", bp[nb_bp].script);
			if(execute_script(bp[nb_bp].script))
				*stop=true;
			break;
	}
}

void mem_rw_check_for_breakpoints(const type_mem_access_t type, const uint32_t addr, const uint32_t val, const bool mem_is_initialized, bool * const stop)
{
	if(!stop)
		return;
	
	uint32_t i;
	for(i=0; i<nb_bp; i++)
	{
		if(bp[i].disabled)
			continue;
		
		if(type==READ && bp[i].type!=MEMORY_READ)
			continue;
		
		if(type==WRITE && bp[i].type!=MEMORY_WRITE)
			continue;
		
		if(bp[i].type_addr_mem==ADDR && bp[i].addr!=addr)
			continue;
		
		if(bp[i].type_addr_mem==RANGE && !(addr>=bp[i].range_from && addr<=bp[i].range_to))
			continue;
		
		if(bp[i].type_match_mem==IS_VALUE && bp[i].value!=val)
			continue;
		
		if(bp[i].type_match_mem==IS_UNDEF && mem_is_initialized)
			continue;
		
		handle_breakpoint(i, NULL, NULL, stop);
	}
}

void simulate_check_for_breakpoints(uint32_t * const PC, uint32_t const * const regs, bool * const PC_manipulated, bool * const stop)
{
	uint32_t i;
	
	for(i=0; i<nb_bp; i++)
	{
		if(bp[i].disabled)
			continue;
		
		switch(bp[i].type)
		{
			//these are checked during the actual mem access by mem_rw_check_for_breakpoints()
			case MEMORY_READ:
			case MEMORY_WRITE:
				break;
			
			case REGISTER_IS_VALUE:
				if(regs[bp[i].reg]==bp[i].value)
				{
					handle_breakpoint(i, PC, PC_manipulated, stop);
				}
				break;
				
			case REGISTER_IS_NOT_VALUE:
				if(regs[bp[i].reg]!=bp[i].value)
				{
					handle_breakpoint(i, PC, PC_manipulated, stop);
				}
				break;
			
			case PC_IS_VALUE:
				if(*PC==bp[i].PC)
				{
					handle_breakpoint(i, PC, PC_manipulated, stop);
				}
				break;
		}
	}
}

