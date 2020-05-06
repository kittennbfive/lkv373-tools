/*
This file is part of the simulator for the LKV373
(c) 2019-2020 by kitten_nb_five
freenode #lkv373a

licence: AGPL v3 or later

THIS PROGRAM COMES WITHOUT ANY WARRANTY!
*/

#ifndef __GPIO_H__
#define __GPIO_H__
#include <stdint.h>
#include <stdbool.h>

#include "peripherals.h"

void gpio_write(PERIPH_CB_WRITE_ARGUMENTS);
bool gpio_read(PERIPH_CB_READ_ARGUMENTS);

bool get_LED_state(void);
//bool get_button_state(void);

#endif
