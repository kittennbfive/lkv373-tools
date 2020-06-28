/*
This file is part of the simulator for the LKV373
(c) 2020 by kitten_nb_five
freenode #lkv373a

licence: AGPL v3 or later

THIS PROGRAM COMES WITHOUT ANY WARRANTY!
*/

/*
THIS IS HIGHLY EXPERIMENTAL STUFF!

This code needs to be run with superuser-priviledges, so i say it again:
NO WARRANTY!

This is quite hacked together, i am a beginner, so please be nice. :-)

This code creates the virtual network device ("TAP") and
forwards data from/to the simulator using fifos
Start this first (needs root) and then start the simulator
One the firmware (on the simulator) has done it's init you can then
point ping or wget or even a browser to 192.168.168.55
Beware: Its REALLY slow!

If it does not work check that you don't have a (real) network interface
that uses the same IP as this code.
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <unistd.h>
#include <signal.h>
#include <sys/select.h>
#include "my_err.h"

#define max(a,b) (a>b?a:b)

int make_tap(char * const name)
{
	struct ifreq ifr;
	int fd;

	if((fd=open("/dev/net/tun", O_RDWR))<0)
		ERR(1, "open /dev/net/tun");

	memset(&ifr, 0, sizeof(ifr));
	 
	ifr.ifr_flags=IFF_TAP; //|IFF_NO_PI;
	
	if(ioctl(fd, TUNSETIFF, &ifr)<0)
		ERR(1, "ioctl");

	//yeah, i know, but i was not able to make it work with proper C-code. It works, so what?
	char buf[100];
	sprintf(buf, "ip addr add 192.168.168.50/8 dev %s", ifr.ifr_name);
	system(buf);
	sprintf(buf, "ip link set %s up", ifr.ifr_name);
	system(buf);
	
	strcpy(name, ifr.ifr_name);
	return fd;
}

#define SZ_BUFFER 2000
#define NAME_FIFO_1 "fifo_net1" //from tap to sim - WRITE to there
#define NAME_FIFO_2 "fifo_net2" //from sim to tap - READ from there
#define NAME_FIFO_SIGNAL "fifo_signal"

int fifo_signal;

void sigint(int sig)
{
	int dummy=42;
	write(fifo_signal, &dummy, sizeof(int));
}

int main(void)
{
	if(geteuid()!=0)
	{
		printf("WARNING: You are not root. This program will probably fail.\n");
	}
	
	signal(SIGINT, &sigint);
	printf("registered handler for Ctrl+C\n");
	
	char tapname[10];
	int tap=make_tap(tapname);
	
	printf("tap %s created\n", tapname);
	
	uint8_t *buffer=malloc(SZ_BUFFER);
	if(!buffer)
		ERR(1, "malloc");

	//again, not really elegant but it works...
	system("mkfifo "NAME_FIFO_1);
	system("chmod 666 "NAME_FIFO_1);

	system("mkfifo "NAME_FIFO_2);
	system("chmod 666 "NAME_FIFO_2);
	
	system("mkfifo "NAME_FIFO_SIGNAL);

	int fifo1=open(NAME_FIFO_1, O_RDWR);
	if(fifo1<0)
		ERR(1, "open fifo1");
		
	int fifo2=open(NAME_FIFO_2, O_RDWR);
	if(fifo2<0)
		ERR(1, "open fifo2");
	
	fifo_signal=open(NAME_FIFO_SIGNAL, O_RDWR);
	if(fifo_signal<0)
		ERR(1, "open fifo_signal");
	
	printf("fifos created and opened, permissions set, ready for data...\n");
	
	printf("You may now start the simulator.\n");
	
	while(1)
	{
		
		fd_set set_read;
		FD_ZERO(&set_read);
		FD_SET(fifo2, &set_read);
		FD_SET(tap, &set_read);
		FD_SET(fifo_signal, &set_read);
		
		select(max(max(fifo1, fifo2), max(tap, fifo_signal))+1, &set_read, NULL, NULL, NULL);

		if(FD_ISSET(fifo_signal, &set_read))
		{
			printf("received SIGINT, exiting...\n");
			break;
		}

		if(FD_ISSET(tap, &set_read))
		{
			printf("reading from TAP\n");
			int nb=read(tap, buffer, SZ_BUFFER);
			if(nb<0)
				ERR(1, "read tap");

			if(write(fifo1, buffer, nb)<0)
				ERR(1, "write fifo1");
			printf("%d bytes written to fifo1\n", nb);
		}
		
		if(FD_ISSET(fifo2, &set_read))
		{
			printf("reading from fifo2\n");
			int nb=read(fifo2, buffer, SZ_BUFFER);
			if(nb<0)
				ERR(1, "read fifo2");
			
			if(write(tap, buffer, nb)<0)
				ERR(1, "write tap");
			printf("%d bytes written to TAP\n", nb);
		}
	}
	
	close(fifo1);
	close(fifo2);
	close(fifo_signal);
	
	system("rm "NAME_FIFO_1);
	system("rm "NAME_FIFO_2);
	system("rm "NAME_FIFO_SIGNAL);
	
	free(buffer);
	
	printf("cleaned up everything, bye.\n");
	return 0;
}

