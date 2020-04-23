/*
This file is part of the simulator for the LKV373
(c) 2019-2020 by kitten_nb_five
freenode #lkv373a

licence: AGPL v3 or later

THIS PROGRAM COMES WITHOUT ANY WARRANTY!
*/

#ifndef __PARSE_HEX_H__
#define __PARSE_HEX_H__
#include <stdint.h>

uint8_t is_valid_hex(char const * const str);
uint8_t parse_hex(char const * const str, uint32_t * const val);
uint8_t parse_register(char const * const str, uint8_t * const reg);

#endif
