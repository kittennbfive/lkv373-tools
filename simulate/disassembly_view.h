/*
This file is part of the simulator for the LKV373
(c) 2019-2020 by kitten_nb_five
freenode #lkv373a

licence: AGPL v3 or later

THIS PROGRAM COMES WITHOUT ANY WARRANTY!
*/

#ifndef __DISASSEMBLY_VIEW_H__
#define __DISASSEMBLY_VIEW_H__
#include <stdint.h>

#include "cmd_parser.h"
#include "dispatcher.h"

int init_disassembly_view(void);
void print_disassembly(void);
uint8_t disassm_scroll_up(const scroll_amount_t a);
uint8_t disassm_scroll_down(const scroll_amount_t a);
void disassm_reset(void);
void disassm_reset_view(PROTOTYPE_ARGS_HANDLER);
uint8_t disassm_keypress(char c);

#endif
