/*
This file is part of the simulator for the LKV373
(c) 2019-2020 by kitten_nb_five
freenode #lkv373a

licence: AGPL v3 or later

THIS PROGRAM COMES WITHOUT ANY WARRANTY!
*/

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "mac.h"
#include "peripherals.h"
#include "verbosity.h"
#include "connector_serial.h"
#include "cmd_parser.h"
#include "my_err.h"

#define ADDR_TX_BUFFER 0x90900004 //bytes
#define ADDR_SZ_TX_DATA 0x90900002 //halfword

#define ADDR_RX_BUFFER 0x909003f4 //bytes

typedef struct
{
	uint32_t addr;
	uint32_t val;
} mem_t;

#define MAX_MEM 500

mem_t mem[MAX_MEM];
uint32_t nb_vals=0;

#define SZ_TX_RX_BUFFER 1024
uint8_t tx_buffer[SZ_TX_RX_BUFFER];
uint8_t rx_buffer[SZ_TX_RX_BUFFER];
uint16_t sz_data_tx_buffer=0;
//uint16_t sz_data_rx_buffer=0;

int32_t search(const uint32_t addr)
{
	uint32_t i;
	for(i=0; i<nb_vals; i++)
	{
		if(mem[i].addr==addr)
			return i;
	}
	
	return -1;
}

void save(const uint32_t addr, const uint32_t val)
{
	if(addr>=ADDR_TX_BUFFER && addr<ADDR_TX_BUFFER+SZ_TX_RX_BUFFER)
	{
		tx_buffer[addr-ADDR_TX_BUFFER]=val&0xff;
		return;
	}
	else if(addr>=ADDR_RX_BUFFER && addr<ADDR_RX_BUFFER+SZ_TX_RX_BUFFER)
	{
		rx_buffer[addr-ADDR_RX_BUFFER]=val&0xff;
		return;
	}
	else if(addr==ADDR_SZ_TX_DATA)
	{
		sz_data_tx_buffer=val;
		return;
	}
		
	int32_t pos=search(addr);
	if(pos==-1)
	{
		if(nb_vals>=MAX_MEM)
			ERRX(1, "full");
		pos=nb_vals++;
	}
	mem[pos].addr=addr;
	mem[pos].val=val;
}

bool load(const uint32_t addr, uint32_t * const val)
{
	if(addr>=ADDR_TX_BUFFER && addr<ADDR_TX_BUFFER+SZ_TX_RX_BUFFER)
	{
		(*val)=tx_buffer[addr-ADDR_TX_BUFFER];
		return true;
	}
	else if(addr>=ADDR_RX_BUFFER && addr<ADDR_RX_BUFFER+SZ_TX_RX_BUFFER)
	{
		(*val)=rx_buffer[addr-ADDR_RX_BUFFER];
		return true;
	}
	else if(addr==ADDR_SZ_TX_DATA)
	{
		(*val)=sz_data_tx_buffer;
		return true;
	}
	
	int32_t pos=search(addr);
	if(pos==-1)
		return false;
	
	(*val)=mem[pos].val;
	return true;
}

void init_mac(void)
{
	;
}


uint8_t save_tx(void)
{
	MSG(MSG_PERIPH_MAC, "MAC: TX: %d bytes\n", sz_data_tx_buffer);

	static uint8_t filenumber=0;
	
	char filename[10];
	sprintf(filename, "tx%02d.bin", filenumber);
	
	FILE *f=fopen(filename, "wb");
	if(!f)
		ERR(1, "could not open %s", filename);
	
	fwrite(tx_buffer, sz_data_tx_buffer, 1, f);
	
	fclose(f);
	
	return filenumber++;
}

void mac_write(PERIPH_CB_WRITE_ARGUMENTS)
{
	(void)sz;
	
	MSG(MSG_PERIPH_MAC, "MAC: write 0x%x @0x%x\n", val, addr);
	
	if(addr==0x90907118 && val==0x90)
	{
		MSG(MSG_PERIPH_MAC, "MAC: TX packet saved to tx%02d.bin\n", save_tx());
		save(0x90907118, 0x10); //value after TX, read from real device
	}
	else
	{
		save(addr, val);
	}
}


bool mac_read(PERIPH_CB_READ_ARGUMENTS)
{
	(void)sz;
	
	bool r=load(addr, val);
	
	if(r)
		MSG(MSG_PERIPH_MAC, "MAC: read from 0x%x: 0x%x\n", addr, *val);
		
	return r;	
}

void rx(PROTOTYPE_ARGS_HANDLER)
{
	ARGS_HANDLER_UNUSED;
	
	save(0x9090700c, 0x382);
	save(0x90907010, 0x3f40066);
	save(0x909003f4, 0x0);
	save(0x909003f5, 0xb);
	save(0x909003f6, 0x78);
	save(0x909003f7, 0x0);
	save(0x909003f8, 0x60);
	save(0x909003f9, 0x1);
	save(0x909003fa, 0x0);
	save(0x909003fb, 0xe0);
	save(0x909003fc, 0x4c);
	save(0x909003fd, 0x53);
	save(0x909003fe, 0x44);
	save(0x909003ff, 0x58);
	save(0x90900400, 0x8);
	save(0x90900401, 0x0);
	save(0x90900402, 0x45);
	save(0x90900403, 0x0);
	save(0x90900404, 0x0);
	save(0x90900405, 0x54);
	save(0x90900406, 0x22);
	save(0x90900407, 0xc2);
	save(0x90900408, 0x40);
	save(0x90900409, 0x0);
	save(0x9090040a, 0x40);
	save(0x9090040b, 0x1);
	save(0x9090040c, 0x46);
	save(0x9090040d, 0x54);
	save(0x9090040e, 0xc0);
	save(0x9090040f, 0xa8);
	save(0x90900410, 0xa8);
	save(0x90900411, 0xa);
	save(0x90900412, 0xc0);
	save(0x90900413, 0xa8);
	save(0x90900414, 0xa8);
	save(0x90900415, 0x37);
	save(0x90900416, 0x8);
	save(0x90900417, 0x0);
	save(0x90900418, 0x9);
	save(0x90900419, 0x59);
	save(0x9090041a, 0x7d);
	save(0x9090041b, 0x7d);
	save(0x9090041c, 0x0);
	save(0x9090041d, 0x1);
	save(0x9090041e, 0x1b);
	save(0x9090041f, 0x70);
	save(0x90900420, 0xee);
	save(0x90900421, 0x5e);
	save(0x90900422, 0x0);
	save(0x90900423, 0x0);
	save(0x90900424, 0x0);
	save(0x90900425, 0x0);
	save(0x90900426, 0xa8);
	save(0x90900427, 0x86);
	save(0x90900428, 0x0);
	save(0x90900429, 0x0);
	save(0x9090042a, 0x0);
	save(0x9090042b, 0x0);
	save(0x9090042c, 0x0);
	save(0x9090042d, 0x0);
	save(0x9090042e, 0x10);
	save(0x9090042f, 0x11);
	save(0x90900430, 0x12);
	save(0x90900431, 0x13);
	save(0x90900432, 0x14);
	save(0x90900433, 0x15);
	save(0x90900434, 0x16);
	save(0x90900435, 0x17);
	save(0x90900436, 0x18);
	save(0x90900437, 0x19);
	save(0x90900438, 0x1a);
	save(0x90900439, 0x1b);
	save(0x9090043a, 0x1c);
	save(0x9090043b, 0x1d);
	save(0x9090043c, 0x1e);
	save(0x9090043d, 0x1f);
	save(0x9090043e, 0x20);
	save(0x9090043f, 0x21);
	save(0x90900440, 0x22);
	save(0x90900441, 0x23);
	save(0x90900442, 0x24);
	save(0x90900443, 0x25);
	save(0x90900444, 0x26);
	save(0x90900445, 0x27);
	save(0x90900446, 0x28);
	save(0x90900447, 0x29);
	save(0x90900448, 0x2a);
	save(0x90900449, 0x2b);
	save(0x9090044a, 0x2c);
	save(0x9090044b, 0x2d);
	save(0x9090044c, 0x2e);
	save(0x9090044d, 0x2f);
	save(0x9090044e, 0x30);
	save(0x9090044f, 0x31);
	save(0x90900450, 0x32);
	save(0x90900451, 0x33);
	save(0x90900452, 0x34);
	save(0x90900453, 0x35);
	save(0x90900454, 0x36);
	save(0x90900455, 0x37);
	
	MSG(MSG_PERIPH_MAC, "real ping data saved\n"); //why are we getting no answer?????
}
