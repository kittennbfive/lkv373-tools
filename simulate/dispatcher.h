/*
This file is part of the simulator for the LKV373
(c) 2019-2020 by kitten_nb_five
freenode #lkv373a

licence: AGPL v3 or later

THIS PROGRAM COMES WITHOUT ANY WARRANTY!
*/

#ifndef __DISPATCHER_H__
#define __DISPATCHER_H__
#include <stdint.h>

#include "window.h"
#include "cmd_parser.h"

#define SZ_EVENT_BUFFER 50 //every single keypress is an event, so this thing must be able to hold an entire command string!

typedef enum
{
	LINE,
	PAGE
} scroll_amount_t;

typedef struct
{
	window_t win;
	void (*func_redraw)(void);
	uint8_t (*func_scroll_up)(const scroll_amount_t a);
	uint8_t (*func_scroll_down)(const scroll_amount_t a);
	uint8_t (*func_keypress)(const char c);
} dispatch_data_t;

void dispatcher(const window_t win, const int fd_win);
void redraw(const window_t win);
void redraw_all(void);
void refresh(PROTOTYPE_ARGS_HANDLER);

#endif
