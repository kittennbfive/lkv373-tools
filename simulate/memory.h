/*
This file is part of the simulator for the LKV373
(c) 2019-2020 by kitten_nb_five
freenode #lkv373a

licence: AGPL v3 or later

THIS PROGRAM COMES WITHOUT ANY WARRANTY!
*/

#ifndef __MEMORY_H__
#define __MEMORY_H__
#include <stdint.h>
#include <stdbool.h>

#include "cmd_parser.h"

typedef struct
{
	bool is_initialized;
	uint8_t val;
} mem_byte_t;

typedef struct
{
	bool is_initialized;
	uint32_t val;
} mem_word_t;

typedef struct
{
	bool is_initialized;
	uint16_t val;
} mem_halfword_t;

void memory_init(void);

void read_memory_from_file(char const * const filename, const uint32_t addr);

void memory_set_page(PROTOTYPE_ARGS_HANDLER);
void memory_write_value(PROTOTYPE_ARGS_HANDLER);
void memory_dump_region(PROTOTYPE_ARGS_HANDLER);

mem_word_t memory_get_word(const uint32_t addr, bool * const breakpoint_stop);
mem_byte_t memory_get_byte(const uint32_t addr, bool * const breakpoint_stop);
void memory_set_word(const uint32_t data, const uint32_t addr, bool * const breakpoint_stop, const bool no_periph_callback);
void memory_set_byte(const uint8_t data, const uint32_t addr, bool * const breakpoint_stop, const bool no_periph_callback);
void memory_set_halfword(const uint16_t data, const uint32_t addr, bool * const breakpoint_stop, const bool no_periph_callback);
mem_halfword_t memory_get_halfword(const uint32_t addr, bool * const breakpoint_stop);

#endif
