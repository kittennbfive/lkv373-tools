/*
This file is part of the simulator for the LKV373
(c) 2019-2020 by kitten_nb_five
freenode #lkv373a

licence: AGPL v3 or later

THIS PROGRAM COMES WITHOUT ANY WARRANTY!
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>

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
uint16_t sz_data_rx_buffer=0;

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
	if(addr==ADDR_SZ_TX_DATA)
	{
		sz_data_tx_buffer=val;
		return;
	}
	else if(addr>=ADDR_TX_BUFFER && addr<ADDR_TX_BUFFER+SZ_TX_RX_BUFFER)
	{
		tx_buffer[addr-ADDR_TX_BUFFER]=val&0xff;
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
	if(addr==ADDR_SZ_TX_DATA)
	{
		(*val)=sz_data_tx_buffer;
		return true;
	}
	else if(addr>=ADDR_RX_BUFFER && addr<ADDR_RX_BUFFER+SZ_TX_RX_BUFFER)
	{
		(*val)=rx_buffer[addr-ADDR_RX_BUFFER];
		return true;
	}
	
	int32_t pos=search(addr);
	if(pos==-1)
		return false;
	
	(*val)=mem[pos].val;
	return true;
}

#ifdef CONNECT_TO_TAP
#define NAME_FIFO_1 "connector_tap/fifo_net1" //from tap to sim - READ from there
#define NAME_FIFO_2 "connector_tap/fifo_net2" //from sim to tap - WRITE to there

int fifo_rx, fifo_tx;

static void cleanup(void)
{
	close(fifo_rx);
	close(fifo_tx);
}
#endif

void init_mac(void)
{
#ifdef CONNECT_TO_TAP
	fifo_rx=open(NAME_FIFO_1, O_RDWR|O_NONBLOCK);
	if(fifo_rx<0)
		err(1, "open fifo_rx (1)");

	fifo_tx=open(NAME_FIFO_2, O_RDWR);
	if(fifo_tx<0)
		err(1, "open fifo_tx (2)");
	
	atexit(&cleanup);
#endif

}

#ifndef CONNECT_TO_TAP
static uint8_t save_tx(void)
{
	MSG(MSG_PERIPH_MAC, "MAC: TX: %d bytes\n", sz_data_tx_buffer);

	static uint8_t filenumber=0;
	
	char filename[10];
	sprintf(filename, "tx%02d.bin", filenumber);
	
	FILE *f=fopen(filename, "wb");
	if(!f)
		ERR(1, "could not open %s", filename);
	
	fwrite(tx_buffer, sz_data_tx_buffer*sizeof(uint8_t), 1, f);
	
	fclose(f);
	
	return filenumber++;
}
#endif

void mac_write(PERIPH_CB_WRITE_ARGUMENTS)
{
	(void)sz;
	
	MSG(MSG_PERIPH_MAC, "MAC: write 0x%x @0x%x\n", val, addr);
	
	if(addr==0x90907118 && val==0x90)
	{
#ifndef CONNECT_TO_TAP
		MSG(MSG_PERIPH_MAC, "MAC: TX packet saved to tx%02d.bin\n", save_tx());
#else
		//we need to send those 4 bytes before the actual packet but it looks like the Kernel
		//does not actually use them, so let's just leave them as zero
		uint16_t flags=0;
		uint16_t proto=0;
		if(write(fifo_tx, &flags, sizeof(uint16_t))<0)
			ERR(1, "write flags to fifo_tx");
		if(write(fifo_tx, &proto, sizeof(uint16_t))<0)
			ERR(1, "write proto to fifo_tx");
		if(write(fifo_tx, &tx_buffer, sz_data_tx_buffer*sizeof(uint8_t))<0)
			ERR(1, "write to fifo_tx");
		MSG(MSG_PERIPH_MAC, "MAC: TX packed sent to pipe\n");
		save(0x90907118, 0x10); //value after TX, read from real device
#endif
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

#ifndef CONNECT_TO_TAP
	char * filename=get_next_argument();
	
	printf("reading rx data from file %s\n", filename);
	
	FILE *f=fopen(filename, "rb");
	if(!f)
	{
		printf("could not open %s\n", filename);
		return;
	}
	
	uint16_t status_reg;
	if(fread(&status_reg, 2, 1, f)!=1)
	{
		printf("error reading status_reg\n");
		return;
	}
	
	
	if(fread(&sz_data_rx_buffer, 2, 1, f)!=1)
	{
		printf("error reading size\n");
		return;
	}
	
	if(fread(rx_buffer, 1, sz_data_rx_buffer, f)!=sz_data_rx_buffer)
	{
		printf("error reading data\n");
		return;
	}

	save(0x90907010, 0x3f40004+sz_data_rx_buffer);
	save(0x9090700c, status_reg);
	
	printf("saved %u bytes in MAC RX-buffer\n", sz_data_rx_buffer);
#else
	printf("error: You can't use command rx when compiled with -DCONNECT_TO_TAP\n");
#endif
}


#ifdef CONNECT_TO_TAP
void check_for_mac_rx(void)
{
	int nb_bytes;
	if(ioctl(fifo_rx, FIONREAD, &nb_bytes)<0)
		ERR(1, "ioctl");
	
	if(nb_bytes>0)
	{
		//these are in network order!
		uint16_t flags, proto;
		if(read(fifo_rx, &flags, sizeof(uint16_t))<0)
			ERR(1, "read flags");
		if(read(fifo_rx, &proto, sizeof(uint16_t))<0)
			ERR(1, "read proto");
		
		int nb_bytes_read=read(fifo_rx, rx_buffer, nb_bytes-4);
		if(nb_bytes_read>0)
		{
			MSG(MSG_PERIPH_MAC, "MAC: received %d bytes\n", nb_bytes_read);
			save(0x90907010, 0x3f40004+nb_bytes_read);
			switch(proto)
			{
				case 0x608:	save(0x9090700c, 0x382); break; //ARP
				case 0x8: save(0x9090700c, 0x482); break; //IPv4
				default: ERRX(1, "unknown proto %x\n", proto);
			}
		}
		else if(errno!=EAGAIN)
			ERR(1, "read");
	}
}
#endif
