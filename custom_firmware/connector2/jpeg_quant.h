/*
(c) 2020 by kitten_nb_five
freenode #lkv373a

licence: AGPL v3 or later

THIS CODE COMES WITHOUT ANY WARRANTY!
*/

#ifndef __JPEG_QUANT_H__
#define __JPEG_QUANT_H__

void set_q(unsigned char Q, unsigned char const * const unknown4);

void make_tables(unsigned char Q, unsigned char * const luma_tbl, unsigned char * const chroma_tbl);

void write_quant_tables(unsigned char const * const unknown4, unsigned char const * const luma_tbl, unsigned char const * const chroma_tbl);

#endif
