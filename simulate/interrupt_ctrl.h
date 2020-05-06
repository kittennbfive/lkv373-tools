/*
This file is part of the simulator for the LKV373
(c) 2019-2020 by kitten_nb_five
freenode #lkv373a

licence: AGPL v3 or later

THIS PROGRAM COMES WITHOUT ANY WARRANTY!
*/

#ifndef __INTERRUPT_CTRL_H__
#define __INTERRUPT_CTRL_H__
#include <stdbool.h>

#include "peripherals.h"
#include "cmd_parser.h"

void init_intc(void);

void intc_write(PERIPH_CB_WRITE_ARGUMENTS);
bool intc_read(PERIPH_CB_READ_ARGUMENTS);

bool check_for_pending_irq(void);

#endif
