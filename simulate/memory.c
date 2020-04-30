/*
This file is part of the simulator for the LKV373
(c) 2019-2020 by kitten_nb_five
freenode #lkv373a

licence: AGPL v3 or later

THIS PROGRAM COMES WITHOUT ANY WARRANTY!
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "memory.h"
#include "simulate.h"
#include "breakpoints.h"
#include "my_err.h"
#include "cmd_parser.h"
#include "parse_hex.h"
#include "dispatcher.h"
#include "window.h"
#include "peripherals.h"
#include "cpe.h" //addr translation

//representing the full adressable 4 GB of memory using pages of 16MB each and allocating memory if needed only
mem_byte_t * mem_ptr[256];

static void memory_cleanup(void)
{
	uint32_t i;
	for(i=0; i<256; i++)
		free(mem_ptr[i]); //according to K&R free(NULL) is save
}

void memory_init(void)
{
	memset(mem_ptr, 0, 256*sizeof(mem_byte_t *));
	atexit(&memory_cleanup);
}

void mem_alloc_page(const uint8_t page)
{
	if(mem_ptr[page]) //already allocated?
		return;
	
	mem_ptr[page]=malloc(PAGE_SIZE*sizeof(mem_byte_t));
	if(mem_ptr[page]==0)
		ERR(1, "mem_alloc_page failed");

	uint32_t i;
	for(i=0; i<PAGE_SIZE; i++)
		mem_ptr[page][i].is_initialized=0;
}

mem_word_t memory_get_word(const uint32_t addr, bool * const breakpoint_stop)
{
	mem_word_t ret;
	
	uint32_t addr_remapped=remap_addr(addr);
	uint32_t periph_val;
	
	if(peripheral_read(addr_remapped, &periph_val))
	{
		ret.is_initialized=true;
		ret.val=periph_val;
		return ret;
	}
	
	//if(breakpoint_stop && addr>0x20000000)
	//	printf("get word for %x\n", addr);
			
	uint8_t page=(addr_remapped>>24)&0xFF;
	if(mem_ptr[page]==0)
	{
		ret.is_initialized=false;
#ifdef WARN_IF_READING_UNINITIALIZED
		printf("warning: memory_get_word: requested uninitialized memory page! (0x%x)\n", page);
#endif
		return ret;
	}
	
	uint32_t addr_in_page=addr_remapped&0x00FFFFFF;
	
	if(! (mem_ptr[page][addr_in_page].is_initialized && mem_ptr[page][addr_in_page+1].is_initialized && mem_ptr[page][addr_in_page+2].is_initialized && mem_ptr[page][addr_in_page+3].is_initialized))
	{
		ret.is_initialized=false;
#ifdef WARN_IF_READING_UNINITIALIZED
		printf("warning: memory_get_word: read from uninitialized memory! (0x%x)\n", addr);
#endif
	}
	else
	{
		ret.is_initialized=true;
		ret.val=(mem_ptr[page][addr_in_page+3].val<<24)|(mem_ptr[page][addr_in_page+2].val<<16)|(mem_ptr[page][addr_in_page+1].val<<8)|(mem_ptr[page][addr_in_page+0].val);
	}
	
	mem_rw_check_for_breakpoints(READ, addr, ret.val, ret.is_initialized, breakpoint_stop);
	
	return ret;
}

mem_byte_t memory_get_byte(const uint32_t addr, bool * const breakpoint_stop)
{
	mem_byte_t ret;
	
	uint32_t addr_remapped=remap_addr(addr);
	uint32_t periph_val;
	
	if(peripheral_read(addr_remapped, &periph_val))
	{
		ret.is_initialized=true;
		ret.val=periph_val&0xFF;
		return ret;
	}
	
	uint8_t page=(addr_remapped>>24)&0xFF;
	if(mem_ptr[page]==0)
	{
		ret.is_initialized=false;
#ifdef WARN_IF_READING_UNINITIALIZED
		printf("warning: memory_get_byte: requested uninitialized memory page! (0x%x)\n", page);
#endif
		return ret;
	}
	
	uint32_t addr_in_page=addr_remapped&0x00FFFFFF;
	
	if(!mem_ptr[page][addr_in_page].is_initialized)
	{
		ret.is_initialized=false;
#ifdef WARN_IF_READING_UNINITIALIZED
		printf("warning: memory_get_byte: read from uninitialized memory! (0x%x)\n", addr);
#endif
	}
	else
	{
		ret.is_initialized=true;
		ret.val=mem_ptr[page][addr_in_page].val;
	}
	
	//fprintf(stderr, "%x: memory read byte %x = %x (%c)\n", get_PC(), addr_remapped, ret.val, ret.val?ret.val:' ');
	
	mem_rw_check_for_breakpoints(READ, addr, ret.val, ret.is_initialized, breakpoint_stop);
	
	return ret;
}

void memory_set_word(const uint32_t data, const uint32_t addr, bool * const breakpoint_stop, const bool no_periph_callback)
{
	uint32_t addr_remapped=remap_addr(addr);
	
	if(!no_periph_callback)
	{
		if(peripheral_write(addr_remapped, data))
			return;
	}
	
	uint8_t page=(addr_remapped>>24)&0xFF;
	if(mem_ptr[page]==0)
		mem_alloc_page(page);
		
	uint32_t addr_in_page=addr_remapped&0x00FFFFFF;
	
	unsigned int i;
	for(i=0; i<4; i++)
	{
		mem_ptr[page][addr_in_page+i].is_initialized=true;
		mem_ptr[page][addr_in_page+i].val=(data>>(8*i))&0xFF;
	}
	
	mem_rw_check_for_breakpoints(WRITE, addr, data, 1, breakpoint_stop);
	
	//fprintf(stderr, "%c%c%c%c", (data>>0)&0xff, (data>>8)&0xff, (data>>16)&0xff, (data>>24)&0xff);
	
	//if(addr>0x4FFFFFFF)
	//	fwrite(&data, 4, 1, stderr);
	
	//if(addr>0x4FFFFFFF)
	//	printf("mem write word %x %x\n", addr, data);
}

void memory_set_byte(const uint8_t data, const uint32_t addr, bool * const breakpoint_stop, const bool no_periph_callback)
{
	uint32_t addr_remapped=remap_addr(addr);
	
	if(!no_periph_callback)
	{
		if(peripheral_write(addr_remapped, data))
			return;
	}
	
	uint8_t page=(addr_remapped>>24)&0xFF;
	if(mem_ptr[page]==0)
		mem_alloc_page(page);
	
	uint32_t addr_in_page=addr_remapped&0x00FFFFFF;
	
	mem_ptr[page][addr_in_page].is_initialized=true;
	mem_ptr[page][addr_in_page].val=data;
	
	mem_rw_check_for_breakpoints(WRITE, addr, data, 1, breakpoint_stop);
	
	//fprintf(stderr, "byte: %x=%x (%c)\n", addr, data, data?data:' ');
	//fwrite(&data, 1, 1, stderr);
	
	//if(addr>0x4FFFFFFF)
	//	printf("mem write byte %x %x (%c)\n", addr, data, data);
}

void memory_set_halfword(const uint16_t data, const uint32_t addr, bool * const breakpoint_stop, const bool no_periph_callback)
{
	uint32_t addr_remapped=remap_addr(addr);
	
	if(!no_periph_callback)
	{
		if(peripheral_write(addr_remapped, data))
			return;
	}
	
	uint8_t page=(addr_remapped>>24)&0xFF;
	if(mem_ptr[page]==0)
		mem_alloc_page(page);
	
	(void)data; (void)breakpoint_stop;
	
	uint32_t addr_in_page=addr_remapped&0x00FFFFFF;
	
	unsigned int i;
	for(i=0; i<2; i++)
	{
		mem_ptr[page][addr_in_page+i].is_initialized=true;
		mem_ptr[page][addr_in_page+i].val=(data>>(8*i))&0xFF;
	}
	
	mem_rw_check_for_breakpoints(WRITE, addr, data, 1, breakpoint_stop);
	
	//fwrite(&data, 2, 1, stderr);
}

mem_halfword_t memory_get_halfword(const uint32_t addr, bool * const breakpoint_stop)
{
	mem_halfword_t ret;
	
	uint32_t addr_remapped=remap_addr(addr);
	uint32_t periph_val;
	
	if(peripheral_read(addr_remapped, &periph_val))
	{
		ret.is_initialized=true;
		ret.val=periph_val&0xFFFF;
		return ret;
	}
	
	uint8_t page=(addr_remapped>>24)&0xFF;
	if(mem_ptr[page]==0)
	{
		ret.is_initialized=false;
#ifdef WARN_IF_READING_UNINITIALIZED
		printf("warning: memory_get_word: requested uninitialized memory page! (0x%x)\n", page);
#endif
		return ret;
	}
		
	uint32_t addr_in_page=addr_remapped&0x00FFFFFF;
	
	if(! (mem_ptr[page][addr_in_page].is_initialized && mem_ptr[page][addr_in_page+1].is_initialized))
	{
		ret.is_initialized=false;
#ifdef WARN_IF_READING_UNINITIALIZED
		printf("warning: memory_get_word: read from uninitialized memory! (0x%x)\n", addr);
#endif
	}
	else
	{
		ret.is_initialized=true;
		ret.val=(mem_ptr[page][addr_in_page+1].val<<8)|(mem_ptr[page][addr_in_page+0].val);
	}
	
	mem_rw_check_for_breakpoints(READ, addr, ret.val, ret.is_initialized, breakpoint_stop);
	
	return ret;
}


void read_memory_from_file(char const * const filename, const uint32_t addr)
{
	FILE *f=fopen(filename, "rb");
	if(!f)
		ERR(1, "read_mempage_from_file: can't open file %s\n", filename);
	
	fseek(f, 0, SEEK_END);
	uint32_t fsize=ftell(f);
	fseek(f, 0, SEEK_SET);
	
	uint8_t page=(addr>>24)&0xFF;
	if(mem_ptr[page]==0)
		mem_alloc_page(page);
	
	uint32_t addr_in_page=addr&0x00FFFFFF;
	
	uint8_t *buf=malloc(PAGE_SIZE);
	if(!buf)
		ERR(1, "malloc for temporary buffer failed");
	
	if(fread(buf, fsize, 1, f)!=1)
		ERR(1, "read_mempage_from_file: error reading file %s\n", filename);
	
	fclose(f);
	
	uint32_t i;
	for(i=0; i<fsize; i++)
	{
		mem_ptr[page][addr_in_page+i].is_initialized=true;
		mem_ptr[page][addr_in_page+i].val=buf[i];
	}
	
	free(buf);
	
	printf("%s copied to 0x%x (0x%x bytes)\n", filename, addr, fsize);
}

void memory_set_page(PROTOTYPE_ARGS_HANDLER) //2 args
{
	(void)cmd; (void)nb_args;
	
	uint8_t page;
	uint8_t val;
	uint32_t v;
	
	if(parse_hex(get_next_argument(), &v))
		return;
	if(v>0xff)
	{
		printf("page must be between 0 and 0xff\n");
		return;
	}
	page=(uint8_t)v;
	
	if(parse_hex(get_next_argument(), &v))
		return;
	if(v>0xff)
	{
		printf("init value must be between 0 and 0xff\n");
		return;
	}
	val=v;
	
	mem_alloc_page(page);
	
	uint32_t i;
	for(i=0; i<PAGE_SIZE; i++)
	{
		mem_ptr[page][i].is_initialized=1;
		mem_ptr[page][i].val=val;
	}
	
	printf("memory 0x%02xxxxxxxxx initialized to 0x%02x\n", page, val);
	
	redraw(W_MEM);
}

void memory_write_value(PROTOTYPE_ARGS_HANDLER)
{
	(void)nb_args;
	
	uint32_t addr;
	if(parse_hex(get_next_argument(), &addr))
		return;

	uint32_t val;
	if(parse_hex(get_next_argument(), &val))
		return;
		
	if(!strcmp(cmd, "byte"))
	{
		if(val>0xff)
		{
			printf("value is too big for a byte\n");
			return;
		}
		memory_set_byte((uint8_t)val, addr, NULL, false);
	}
	else if(!strcmp(cmd, "word"))
	{
		memory_set_word(val, addr, NULL, false);
	}
	else
		ERRX(1, "invalid command");
	
	printf("memory value written\n");
	
	redraw(W_MEM);
}

void memory_dump_region(PROTOTYPE_ARGS_HANDLER)
{
	ARGS_HANDLER_UNUSED;
	
	uint32_t addr_from;
	if(parse_hex(get_next_argument(), &addr_from))
		return;
	
	uint32_t addr_to;
	if(parse_hex(get_next_argument(), &addr_to))
		return;
	
	char *filename=get_next_argument();
	FILE *f=fopen(filename, "wb");
	if(!f)
	{
		printf("can't open %s\n", filename);
		return;
	}
	
	printf("dumping memory 0x%08x-0x%08x to %s...\n", addr_from, addr_to, filename);
	
	//TODO: use direct memory access and write chunks of data to make this faster!
	//actually it's fine, it's really quick already
	
	uint32_t addr;
	mem_byte_t byte;
	for(addr=addr_from; addr<addr_to; addr++)
	{
		byte=memory_get_byte(addr, NULL);
		if(!byte.is_initialized)
		{
			printf("error: memory at 0x%08x is uninitialized\n", addr);
			break;
		}
		fwrite(&byte.val, sizeof(uint8_t), 1, f);
	}
	
	fclose(f);
	printf("dump finished\n");
}
