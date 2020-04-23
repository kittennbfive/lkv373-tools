/*
This file is part of the simulator for the LKV373
(c) 2019-2020 by kitten_nb_five
freenode #lkv373a

licence: AGPL v3 or later

THIS PROGRAM COMES WITHOUT ANY WARRANTY!
*/

#ifndef __UART_H__
#define __UART_H__

#include "peripherals.h"
#include "breakpoints.h"

int init_uart_view(void);
void uart_write(PERIPH_CB_WRITE_ARGUMENTS);
bool uart_read(PERIPH_CB_READ_ARGUMENTS);

#endif
