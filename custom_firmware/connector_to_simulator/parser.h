/*
(c) 2020 by kitten_nb_five
freenode #lkv373a

licence: AGPL v3 or later

THIS CODE COMES WITHOUT ANY WARRANTY!
*/

#ifndef __PARSER_H__
#define __PARSER_H__

void parse(unsigned char const * const str);

/*
BINARY FORMAT:

each command=10 bytes (without '\0'!)

1 byte '?' or '=' for read/write
1 byte size mem access =1,2,4
4 bytes addr
4 byte val (dummy for read)

return
'o'+'k'+4 bytes val (dummy for write) = 6 bytes
*/

#endif

