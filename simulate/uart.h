/*
This file is part of the simulator for the LKV373
(c) 2019-2020 by kitten_nb_five
freenode #lkv373a

licence: AGPL v3 or later

THIS PROGRAM COMES WITHOUT ANY WARRANTY!
*/

#ifndef __UART_H__
#define __UART_H__
#include <stdint.h>
#include <stdbool.h>

#include "peripherals.h"
#include "breakpoints.h"
#include "cmd_parser.h"

int init_uart_view(void);
void uart_write(PERIPH_CB_WRITE_ARGUMENTS);
bool uart_read(PERIPH_CB_READ_ARGUMENTS);
uint8_t uart_keypress(char c);
void uart_to_file(PROTOTYPE_ARGS_HANDLER);

#endif
