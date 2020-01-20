/*
This file is part of a disassembler for Andestech NDS32.

(c) 2019-2020 kitten_nb_five

THIS WORK COMES WITHOUT ANY WARRANTY and is released under the AGPL version 3 or later
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
