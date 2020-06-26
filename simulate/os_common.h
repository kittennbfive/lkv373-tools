/*
This file is part of the simulator for the LKV373
(c) 2020 by kitten_nb_five
freenode #lkv373a

licence: AGPL v3 or later

THIS PROGRAM COMES WITHOUT ANY WARRANTY!
*/

#ifndef __OS_COMMON_H__
#define __OS_COMMON_H__
#include <stdint.h>

uint32_t get_word(const uint32_t addr);
uint32_t get_halfword(const uint32_t addr);
uint32_t get_byte(const uint32_t addr);

char * get_task_name_from_id(const uint16_t id);
char * get_task_name_from_priority(const uint8_t prio);

uint16_t get_current_task_id(void);

#endif
