/*
experimental r2-support for nds32

written by kitten_nb_five (c) 2019-2020

HIGHLY EXPERIMENTAL - NO WARRANTY - PROVIDED AS IS

licence: AGPL version 3 or later
*/

#ifndef __DEBUG_H__
#define __DEBUG_H__

#define DEBUG 0

#if DEBUG
#define PRINTF_DEBUG(str, ...) printf(str, ##__VA_ARGS__) //this is GCC-specific, the ## removes the comma if __VA_ARGS__ is empty
#else
#define PRINTF_DEBUG(str, ...) 
#endif

#endif
