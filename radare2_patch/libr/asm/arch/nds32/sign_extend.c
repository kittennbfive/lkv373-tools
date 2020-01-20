/*
experimental r2-support for nds32

written by kitten_nb_five (c) 2019-2020

HIGHLY EXPERIMENTAL - NO WARRANTY - PROVIDED AS IS

licence: AGPL version 3 or later
*/

#include <stdint.h>

#include "debug.h"

#include "sign_extend.h"

int32_t nds32_sign_extend(const uint32_t nb, const uint32_t length, const uint32_t length_ext)
{
	uint32_t mask;
	uint32_t nb_ext;
	
	if(nb&(1<<(length-1)))
		mask=(((uint64_t)1<<length_ext)-1)&~((1<<length)-1);
	else
		mask=0;
	
	nb_ext=nb|mask;
	
	return nb_ext;
}
