/*
This file is part of the simulator for the LKV373
(c) 2020 by kitten_nb_five
freenode #lkv373a

licence: AGPL v3 or later

THIS PROGRAM COMES WITHOUT ANY WARRANTY!
*/

#ifndef __OS_INFO_H__
#define __OS_INFO_H__
#include <stdint.h>

typedef enum
{
	START,
	END
} pos_t;

void os_info(const uint32_t PC);
#endif
