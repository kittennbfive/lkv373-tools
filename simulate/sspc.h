/*
This file is part of the simulator for the LKV373
(c) 2019-2020 by kitten_nb_five
freenode #lkv373a

licence: AGPL v3 or later

THIS PROGRAM COMES WITHOUT ANY WARRANTY!
*/

#ifndef __SSPC_H__
#define __SSPC_H__
#include <stdint.h>
#include <stdbool.h>

#include "peripherals.h"

void init_sspc(char const * const file_flash);

void sspc_write(PERIPH_CB_WRITE_ARGUMENTS);
bool sspc_read(PERIPH_CB_READ_ARGUMENTS);

#endif
