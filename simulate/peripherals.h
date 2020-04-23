/*
This file is part of the simulator for the LKV373
(c) 2019-2020 by kitten_nb_five
freenode #lkv373a

licence: AGPL v3 or later

THIS PROGRAM COMES WITHOUT ANY WARRANTY!
*/

#ifndef __PERIPHERALS_H__
#define __PERIPHERALS_H__
#include <stdint.h>
#include <stdbool.h>

#include "window.h"
//#include "breakpoints.h" //type_mem_access_t

#define PERIPH_CB_WRITE_ARGUMENTS const uint32_t addr, const uint32_t val
#define PERIPH_CB_READ_ARGUMENTS const uint32_t addr, uint32_t * const val

typedef struct
{
	char *name;
	uint32_t addr_start;
	uint32_t addr_end;
	void (*cb_write)(PERIPH_CB_WRITE_ARGUMENTS);
	bool (*cb_read)(PERIPH_CB_READ_ARGUMENTS); //returns true if *val contains a valid value
} peripheral_t;

bool peripheral_write(const uint32_t addr, const uint32_t val);
bool peripheral_read(const uint32_t addr, uint32_t * const val);

#endif
