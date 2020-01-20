/*
experimental r2-support for nds32

written by kitten_nb_five (c) 2019-2020

HIGHLY EXPERIMENTAL - NO WARRANTY - PROVIDED AS IS

licence: AGPL version 3 or later
*/

#ifndef __SIGN_EXTEND_H__
#define __SIGN_EXTEND_H__

#define NEG(nb) ((~nb)+1)
#define ZE(nb) (nb) //zero-extend is a no-op basically

int32_t nds32_sign_extend(const uint32_t nb, const uint32_t length, const uint32_t length_ext);

#endif
