/*
This file is part of the simulator for the LKV373
(c) 2019-2020 by kitten_nb_five
freenode #lkv373a

licence: AGPL v3 or later

THIS PROGRAM COMES WITHOUT ANY WARRANTY!
*/

#ifndef __FLASH_SPI_H__
#define __FLASH_SPI_H__
#include <stdint.h>

#define FLASH_SIZE (4*1024*1024)

void flash_init(char const * const file);

uint32_t flash_spi_transfer(const uint32_t val);

#endif
