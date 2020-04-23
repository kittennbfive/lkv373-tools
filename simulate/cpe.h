/*
This file is part of the simulator for the LKV373
(c) 2019-2020 by kitten_nb_five
freenode #lkv373a

licence: AGPL v3 or later

THIS PROGRAM COMES WITHOUT ANY WARRANTY!
*/

#ifndef __CPE_H__
#define __CPE_H__
#include <stdint.h>
#include <stdbool.h>

#include "peripherals.h"

void init_cpe(void);

void cpe_write(PERIPH_CB_WRITE_ARGUMENTS);
bool cpe_read(PERIPH_CB_READ_ARGUMENTS);

uint32_t remap_addr(const uint32_t addr);

#endif
