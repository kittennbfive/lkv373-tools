/*
(c) 2020 by kitten_nb_five
freenode #lkv373a

licence: AGPL v3 or later

THIS CODE COMES WITHOUT ANY WARRANTY!
*/

#ifndef __I2C_ITE_H__
#define __I2C_ITE_H__

void ite_i2c_init(void);
void ite_write_register(const unsigned char reg, const unsigned char val);
unsigned char ite_read_register(const unsigned char reg);

#endif
