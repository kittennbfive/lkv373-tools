/*
This file is part of a disassembler for Andestech NDS32.

(c) 2019-2020 kitten_nb_five

THIS WORK COMES WITHOUT ANY WARRANTY and is released under the AGPL version 3 or later
*/

#ifndef __SIGN_EXTEND_H__
#define __SIGN_EXTEND_H__

#define NEG(nb) ((~nb)+1)
#define ZE(nb) (nb) //zero-extend is a no-op basically

int32_t nds32_sign_extend(const uint32_t nb, const uint32_t length, const uint32_t length_ext);

#endif
