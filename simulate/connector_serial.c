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
#include <string.h>
#include <stdbool.h>
#define __USE_MISC
#include <unistd.h>
#include <fcntl.h>

#include "connector_serial.h"
#include "verbosity.h"
#include "my_err.h"

#define SERIALPORT "/dev/ttyUSB0"

#define WAIT_MS 5

static int fd;

static void cleanup(void)
{
	close(fd);
}

void init_connector(void)
{
	system("stty -F "SERIALPORT" 115200 cs8 -cstopb -crtscts"); //thats the lazy solution...

	fd=open(SERIALPORT,  O_RDWR | O_NOCTTY | O_NDELAY);
	if(fd==-1)
		ERR(1, "open serial");
	
	atexit(&cleanup);
	
	MSG(MSG_ALWAYS, "init_connector: ok, port is %s\n", SERIALPORT);
}

uint32_t saved=0;

void con_setval(const uint8_t sz, const uint32_t addr, const uint32_t val)
{
	char buf[10];
	char buf2[10];
	
	MSG_NO_DEDUP(MSG_CONNECTOR, "con_setval: (%u) set 0x%x to 0x%x\n", sz, addr, val);
	
	if(addr==0x9090a80c)
	{
		usleep(WAIT_MS*1000);
		return;
	}
	
	if(addr==0x9090c208)
		saved=val;
		
	buf[0]='=';
	buf[1]=sz;
	memcpy(&buf[2], &addr, sizeof(uint32_t));
	memcpy(&buf[6], &val, sizeof(uint32_t));

	int r=write(fd, buf, 10);
	if(r<0)
		ERR(1, "write");
	
	usleep(WAIT_MS*1000);
	
	r=read(fd, buf2, 10);
	if(r!=-1)
	{
		if(memcmp(buf2, "ok", 2))
		{
			buf2[r]='\0';
			ERRX(1, "setval failed: %s", buf2);
		}
	}
	else
		ERRX(1, "read returned -1");
		
	usleep(WAIT_MS*1000);
}

uint32_t con_getval(const uint8_t sz, const uint32_t addr)
{
	char buf[10];
	char buf2[10];
	
	if(addr==0x9090c20c)
		return saved;
	
	buf[0]='?';
	buf[1]=sz;
	memcpy(&buf[2], &addr, sizeof(uint32_t));
	memcpy(&buf[6], "\x01\x02\x03\x04", 4);
	
	int r=write(fd, buf, 10);
	if(r<0)
		ERR(1, "write");
	
	usleep(WAIT_MS*1000);
	
	uint32_t val;
	
	r=read(fd, buf2, 10);
	if(r!=-1)
	{
		if(memcmp(buf2, "ok", 2))
			ERRX(1, "getval failed");
		memcpy(&val, &buf2[2], sizeof(uint32_t));
	}
	else
		ERRX(1, "read returned -1");
	
	MSG(MSG_CONNECTOR, "con_getval: (%u) 0x%x == 0x%x\n", sz, addr, val);
	
	return val;
}

