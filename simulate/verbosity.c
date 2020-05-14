/*
This file is part of the simulator for the LKV373
(c) 2019-2020 by kitten_nb_five
freenode #lkv373a

licence: AGPL v3 or later

THIS PROGRAM COMES WITHOUT ANY WARRANTY!
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#include "verbosity.h"
#include "cmd_parser.h"

static uint32_t verbosity_flags=0;
static bool log_to_file=false;
FILE *out=NULL;

#define MSG_INVALID 0xffffffff

typedef struct
{
	uint32_t bitmask;
	char * name;
} msg_type_t;

//keep this up to date with #defines in header file!
//MSG_ALWAYS can't be disabled, don't add it
static const msg_type_t msg_types[]=
{
	{ MSG_PC, "PC" },
	{ MSG_CALL, "CALL" },
	{ MSG_RET, "RET" },
	{ MSG_IRET, "IRET" },
	{ MSG_GOTO, "GOTO" },
	{ MSG_IRQ, "IRQ" },
	{ MSG_INT, "INT" },
	{ MSG_PERIPH, "PERIPH" },
	{ MSG_VARIOUS, "VARIOUS" },
	
	{ 0, NULL }
};

static bool show_this(const uint32_t msg_type)
{
	if(verbosity_flags&msg_type || msg_type==MSG_ALWAYS)
		return true;
	else
		return false;
}

static void logfile_cleanup(void)
{
	fclose(out);
} 

static uint32_t get_bitmask(char const * const str)
{
	uint8_t i;
	for(i=0; msg_types[i].name; i++)
	{
		if(!strcmp(str, msg_types[i].name))
			return msg_types[i].bitmask;
	}
	
	return MSG_INVALID;	
}

static void set_verbosity(char const * const str, const bool onoff)
{
	uint32_t bitmask=get_bitmask(str);
	if(bitmask==MSG_INVALID)
	{
		printf("set_verbosity: unknown message %s\n", str);
		return;
	}
	
	if(onoff)
	{
		verbosity_flags|=bitmask;
		printf("messages %s activated\n", str);
	}
	else
	{
		verbosity_flags&=~bitmask;
		printf("messages %s disabled\n", str);
	}
}

static void enable_all(void)
{
	uint8_t i;
	for(i=0; msg_types[i].name; i++)
		verbosity_flags|=msg_types[i].bitmask;
}

static void disable_all(void)
{
	uint8_t i;
	for(i=0; msg_types[i].name; i++)
		verbosity_flags&=~msg_types[i].bitmask;
}

void cmd_set_verbosity(PROTOTYPE_ARGS_HANDLER) //min 2 args guaranteed by cmd_parser.c
{
	ARGS_HANDLER_UNUSED;
	
	char * action=get_next_argument();
	
	if(!strcmp(action, "on"))
	{
		if(get_number_remaining_args()==1 && !strcmp(peek_next_argument(), "all"))
			enable_all();
		else
		{
			while(get_number_remaining_args())
				set_verbosity(get_next_argument(), true);
		}
	}
	else if(!strcmp(action, "off"))
	{
		if(get_number_remaining_args()==1 && !strcmp(peek_next_argument(), "all"))
			disable_all();
		else
		{
			while(get_number_remaining_args())
				set_verbosity(get_next_argument(), false);
		}
	}
	else if(!strcmp(action, "file"))
	{
		if(log_to_file)
		{
			printf("already logging to file\n");
			return;
		}
		
		char * logfile=get_next_argument();
		out=fopen(logfile, "w");
		if(!out)
		{
			printf("failed to open logfile %s\n", logfile);
			return;
		}
		log_to_file=true;
		atexit(&logfile_cleanup);
		printf("logging now to file %s\n", logfile);
	}
	else
		printf("unknown action %s\n", action);
}

void message_printf(const uint32_t msg_type, char const * const fmt, ...)
{
	if(!show_this(msg_type))
		return;
	
	static char last_msg[250]={0};
	char buf[250];
	
	va_list ap;
	va_start(ap, fmt);
	vsprintf(buf, fmt, ap);
	va_end(ap);
	
	if(!strcmp(buf, last_msg))
		return;
	
	strcpy(last_msg, buf);
	printf("%s", buf);
	
	if(log_to_file)
		fprintf(out, "%s", buf);
}
