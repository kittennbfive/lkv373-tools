/*
This is EXPERIMENTAL CODE for receiving video data from the 373 WITH PATCHED FIRMWARE and show some informations.

This is not intended as a real receiver but for fuzzing around in the config of the 373 and observe any effects.

(c) 2020 by kitten_nb_five
freenode #lkv373a

licence: AGPL v3 or later

THIS PROGRAM COMES WITHOUT ANY WARRANTY!

Version 0.02

You need to run this code as root.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include "my_err.h"

#define SKIP_DAMAGED

#define SZ_BUF 1024*1024

#define SZ_FRAME_MAX 1024*1024


void parse_dqt(unsigned char const * const data, const unsigned int sz)
{
	unsigned int i, j;
	for(i=0; i<sz-64;)
	{
		fprintf(stderr, "DQT: dst %u:\n", data[i]&0x0f);
		i++;
		for(j=0; j<64; j++)
			fprintf(stderr, "%3u, ", data[i+j]);
		fprintf(stderr, "\n");
		i+=64;	
	}
}

void parse_sof0(unsigned char const * const data, const unsigned int sz)
{
	(void)sz;
	
	unsigned short sz_y=data[1]*256+data[2];
	unsigned short sz_x=data[3]*256+data[4];
	
	unsigned char nb_images_comp=data[5];
	
	fprintf(stderr, "picture is %u x %u - %u components\n", sz_x, sz_y, nb_images_comp);
}

void parse(unsigned char const * const data, const unsigned int fsz)
{
		
	unsigned int i;
	bool stop=false;
	
	for(i=2;i<fsz-4;)
	{
		unsigned short segment=data[i]<<8|data[i+1];
		unsigned int sz=data[i+2]*256+data[i+3];
		
		if(sz==0)
			ERRX(1, "sz==0");
		
		//fprintf(stderr, "0x%x: 0x%x sz %u\n", i, segment, sz);
		
		switch(segment)
		{
			case 0xffe0: //APP0 - JFIF tag
				break;
			case 0xffdb: //DQT - definition of quantification tables
				parse_dqt(&data[i+4], sz);
				break;
			case 0xffc0: //SOF0 - baseline DCT - non differential, Huffman coding
				parse_sof0(&data[i+4], sz);
				break;
			case 0xffc4: //DHT - definition of Huffman tables
				break;
			case 0xffda: //SOS - start of scan
				stop=true;
				break;
			
			default:
				fprintf(stderr, "unknown segment 0x%x", segment);
				//ERRX(1, "unknown segment 0x%x", segment);
				break;
		}
		
		if(stop)
			break;
		
		i+=sz+2;
	}
}
volatile int run=1;

void sigint(int sig)
{
	(void)sig;
	run=0;
}

void usage(void)
{
	printf("./receive $interface > $file OR ./receive $interface | $other_tool\n");
	printf("try ./receive $interface | ffplay -\n");
	printf("VLC does not like the data.\n");
	exit(0);
}

int main(int argc, char *argv[])
{
	int sock;
	unsigned char buf[SZ_BUF];
	int rx_size;

	if(argc!=2)
		usage();

	if(geteuid()!=0)
	{
		fprintf(stderr, "WARNING: You are not root. This program will probably fail.\n");
	}
	
	sock=socket(AF_PACKET, SOCK_DGRAM, htons(0x0800));
	if(sock<0)
		ERR(1, "could not open socket");
	
	//we need promiscuous mode to listen to multicast without hassle
	struct ifreq ifr;
	memset(&ifr, 0, sizeof(struct ifreq));
	strcpy(ifr.ifr_name, argv[1]);
	if(ioctl(sock, SIOCGIFFLAGS, &ifr)<0)
		ERR(1, "ioctl get");
	ifr.ifr_flags|=IFF_PROMISC;
	if(ioctl(sock, SIOCSIFFLAGS, &ifr)<0)
		ERR(1, "ioctl set");
	
	
	if(setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, argv[1], IFNAMSIZ)<0)
		ERR(1, "could not bind to device");
	
	fprintf(stderr, "listening on %s\n", argv[1]);

	bool start_of_frame=false;
	bool end_of_frame=false;
	bool marker_sof_received=false; //start of frame
	bool write_output=false; //wait for start of frame before beginn to write
	unsigned short frame_previous=0;
	unsigned short block_previous=0;
	
	unsigned char framedata[SZ_FRAME_MAX];
	unsigned int sz_frame=0;
	
	
	signal(SIGINT, &sigint);
	
	while(run)
	{		
		rx_size=recv(sock, buf, SZ_BUF, 0);
		if(rx_size==48+1) //This should be 48 bytes but there is a trailing byte as seen on Wireshark
		{
			//fprintf(stderr, "SOF marker received\n");
			marker_sof_received=true;
			continue;
		}
		else if(rx_size==1052)
		{
			if(buf[2]!=4 || buf[3]!=0x1c)
			{
				fprintf(stderr, "ignoring unknown block\n"); //You may want to comment this if you have other traffic on your interface
				continue;
			}
		}
		
		unsigned short frame=buf[28]<<8|buf[29];
		unsigned short block=buf[30]<<8|buf[31];
		if(buf[30]&(1<<7))
		{
			start_of_frame=false;
			end_of_frame=true;
		}
		else if(block==0)
		{
			start_of_frame=true;
			end_of_frame=false;
			write_output=true;
			sz_frame=0;
		}
		else
		{
			start_of_frame=false;
			end_of_frame=false;
		}

		if(!end_of_frame)
			fprintf(stderr, "receiving frame %05u block %05u\r", frame, block);

		if(!start_of_frame && marker_sof_received)
			fprintf(stderr,  "unexpected SOF marker received\n");

		if(start_of_frame && frame!=frame_previous+1 && frame_previous!=0xffff)
			fprintf(stderr, "discontinuity: last frame was %u, next is %u\n", frame_previous, frame);
		
		if(!start_of_frame && !end_of_frame && block!=block_previous+1)
		{
			fprintf(stderr, "discontinuity: frame %u last block was %u, next is %u\n", frame, block_previous, block);
#ifdef SKIP_DAMAGED
			write_output=false;
#endif
		}
		
		frame_previous=frame;
		block_previous=block;
		
		marker_sof_received=false;
		
		memcpy(&framedata[sz_frame], buf+28+4, 1020);
		sz_frame+=1020;
		
		if(end_of_frame && write_output)
		{
			parse(framedata, sz_frame);

			fwrite(framedata, sz_frame, 1, stdout);
		}
	}
	
	close(sock);
	
	fprintf(stderr, "\nBye.\n");
	
	return 0;
}
