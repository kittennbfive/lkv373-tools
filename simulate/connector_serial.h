/*
This file is part of the simulator for the LKV373
(c) 2019-2020 by kitten_nb_five
freenode #lkv373a

licence: AGPL v3 or later

THIS PROGRAM COMES WITHOUT ANY WARRANTY!
*/

#ifndef __CONNECTOR_SERIAL_H__
#define __CONNECTOR_SERIAL_H__
#include <stdint.h>

void init_connector(void);
void con_setval(const uint8_t sz, const uint32_t addr, const uint32_t val);
uint32_t con_getval(const uint8_t sz, const uint32_t addr);

#endif
