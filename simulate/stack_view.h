/*
This file is part of the simulator for the LKV373
(c) 2019-2020 by kitten_nb_five
freenode #lkv373a

licence: AGPL v3 or later

THIS PROGRAM COMES WITHOUT ANY WARRANTY!
*/

#ifndef __STACK_VIEW_H__
#define __STACK_VIEW_H__
#include <stdint.h>

#include "cmd_parser.h"
#include "dispatcher.h"

int init_stack_view(void);
void print_stack(void);
uint8_t stack_scroll_up(const scroll_amount_t a);
uint8_t stack_scroll_down(const scroll_amount_t a);
void stack_reset_view(void);
uint8_t stack_keypress(const char c);
void stack_show_addr(PROTOTYPE_ARGS_HANDLER);

#endif
