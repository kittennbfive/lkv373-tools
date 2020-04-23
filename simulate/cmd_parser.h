/*
This file is part of the simulator for the LKV373
(c) 2019-2020 by kitten_nb_five
freenode #lkv373a

licence: AGPL v3 or later

THIS PROGRAM COMES WITHOUT ANY WARRANTY!
*/

#ifndef __CMD_PARSER_H__
#define __CMD_PARSER_H__
#include <stdint.h>

#define SZ_BUFFER_COMMAND 20
#define SZ_BUFFER_ARGUMENTS 25
#define NB_ARGUMENTS_MAX 10

#define PROTOTYPE_ARGS_HANDLER char const * const cmd, const uint8_t nb_args

#define ARGS_HANDLER_UNUSED (void)cmd; (void)nb_args

typedef struct
{
	char *cmd;
	uint8_t nb_args_min;
	uint8_t nb_args_max;
	void (*handler)(PROTOTYPE_ARGS_HANDLER);
} command_t;

uint8_t parse_input(char * const inp);
char * get_next_argument(void);
char * peek_next_argument(void);
uint8_t get_number_remaining_args(void);
void execute_script_cmd(PROTOTYPE_ARGS_HANDLER);
uint8_t execute_script(char const * const filename);

#endif
