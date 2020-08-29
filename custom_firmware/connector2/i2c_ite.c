/*
(c) 2020 by kitten_nb_five
freenode #lkv373a

licence: AGPL v3 or later

THIS CODE COMES WITHOUT ANY WARRANTY!
*/

#include <stdlib.h>
#include <stdio.h>

#include "i2c_ite.h"

#define I2C_PRESCALER_REG (*(volatile unsigned char*)0x99c00000) //write 0xC8 for 100kHz (TODO: measure actual speed)
#define I2C_UNKNOWN1_REG (*(volatile unsigned char*)0x99c00004) //write 0x80 to enable I2C
#define I2C_DATA_TX_REG (*(volatile unsigned char*)0x99c00008)
#define I2C_CTRL_REG (*(volatile unsigned char*)0x99c0000c)
#define I2C_DATA_RX_REG (*(volatile unsigned char*)0x99c00010)
#define I2C_STATUS_REG (*(volatile unsigned char*)0x99c00014)

#define ITE_ADDR_WRITE 0x90 //write
#define ITE_ADDR_READ 0x91 //read

void ite_i2c_init(void)
{
	I2C_PRESCALER_REG=0xc8;
	I2C_UNKNOWN1_REG=0x80;
}

void ite_write_register(const unsigned char reg, const unsigned char val)
{
	I2C_DATA_TX_REG=ITE_ADDR_WRITE;
	I2C_CTRL_REG=0x90; //START, send addr
	while(I2C_STATUS_REG!=0x41); 
	I2C_DATA_TX_REG=reg;
	I2C_CTRL_REG=0x10; //send byte
	while(I2C_STATUS_REG!=0x41);
	I2C_DATA_TX_REG=val;
	I2C_CTRL_REG=0x10; //send byte
	while(I2C_STATUS_REG!=0x41);
	I2C_CTRL_REG=0x40; //STOP
	//status=0x01
}

unsigned char ite_read_register(const unsigned char reg)
{
	I2C_DATA_TX_REG=ITE_ADDR_WRITE;
	I2C_CTRL_REG=0x90; //START, send addr
	while(I2C_STATUS_REG!=0x41);
	I2C_DATA_TX_REG=reg;
	I2C_CTRL_REG=0x10; //send byte
	while(I2C_STATUS_REG!=0x41);
	I2C_DATA_TX_REG=ITE_ADDR_READ;
	I2C_CTRL_REG=0x90; //RESTART
	while(I2C_STATUS_REG!=0x41);
	I2C_CTRL_REG=0x28;
	while(I2C_STATUS_REG!=0xc1);
	unsigned char r=I2C_DATA_RX_REG;
	I2C_CTRL_REG=0x40; //STOP
	//status=0x81
	return r;
}
