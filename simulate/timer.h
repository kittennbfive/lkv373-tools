/*
This file is part of the simulator for the LKV373
(c) 2019-2020 by kitten_nb_five
freenode #lkv373a

licence: AGPL v3 or later

THIS PROGRAM COMES WITHOUT ANY WARRANTY!
*/

#ifndef __TIMER_H__
#define __TIMER_H__
#include <stdbool.h>

#include "peripherals.h"

void init_timer(void);

void timer_write(PERIPH_CB_WRITE_ARGUMENTS);
bool timer_read(PERIPH_CB_READ_ARGUMENTS);

bool timer_tick(void);

bool timer_is_irq_pending(void);

void timer_clear_irq(void);

#endif
