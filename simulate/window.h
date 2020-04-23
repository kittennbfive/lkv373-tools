/*
This file is part of the simulator for the LKV373
(c) 2019-2020 by kitten_nb_five
freenode #lkv373a

licence: AGPL v3 or later

THIS PROGRAM COMES WITHOUT ANY WARRANTY!
*/

#ifndef __WINDOW_H__
#define __WINDOW_H__
#include <stdint.h>

#include "cmd_parser.h"

#define NB_WINDOW_MAX 10
#define NAME_FIFO "fifo_sim"

#define CMD_XFCE4_TERMINAL "xfce4-terminal -T \"%s\" --hide-toolbar --hide-menubar --hide-scrollbar --minimize -x" //do not change
#define CMD_X_TERMINAL_SIMULATOR "x-terminal-emulator -T \"%s\" -e" //do not change

#define CMD_TERMINAL CMD_XFCE4_TERMINAL //select here

#define EV_SCROLL_UP 252 //\033[A
#define EV_SCROLL_DOWN 253 //\033[B
#define EV_KEY_LEFT 254 //\033[D
#define EV_KEY_RIGHT 255 //\033[C
#define EV_PAGE_UP 251 //\033[5~
#define EV_PAGE_DOWN 250 //\033[6~

#define W_MAIN 0 //file descriptor for stdin, do not change

typedef enum
{
	W_EMPTY=0,
	W_ASM,
	W_REG,
	W_MEM,
	W_STACK,
	W_BP
} window_t;

void window_init(void);
int open_new_window(char const * const title, uint16_t * const rows, uint16_t * const cols);
void get_window_size(const int fd, uint16_t * const rows, uint16_t * const cols);
int win_printf(const int fd, char const * const fmt, ...);
uint32_t read_input(const int fd, uint8_t * const buf, const uint32_t sz_buf);
uint32_t wait_for_input(const int fd, uint8_t * const buf, const uint32_t sz_buf);
void cmd_layout(PROTOTYPE_ARGS_HANDLER);

#endif
