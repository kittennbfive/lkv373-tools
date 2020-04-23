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
#include <sys/ioctl.h>	/* ioctl, TIOCGWINSZ */
#include <fcntl.h> //open
#include <unistd.h>	//close
#include <stdarg.h> //variadic

#include "window.h"
#include "my_err.h"

#define SZ_BUF 250
#define SZ_PATH_PTS 20

static int window_list[NB_WINDOW_MAX];
static int nb_window;

static void windows_cleanup(void)
{
	uint8_t i;
	for(i=0; i<nb_window; i++)
	{
		close(window_list[i]);
	}
	system("rm "NAME_FIFO);
	system("stty sane");
}

void window_init(void)
{
	nb_window=0;
	atexit(&windows_cleanup);
	
	system("mkfifo "NAME_FIFO);
	
	//change settings for main window for non-blocking event read
	int r=fcntl(0, F_SETFL, O_NONBLOCK);
	if(r==-1)
		ERR(1, "fcntl");
	
	//this is actually not needed for main
	//to make backspace work icanon is needed but that makes input blocking until enter is pressed
	//system("stty raw -echo opost isig icrnl -ctlecho");
	
	//make output unbuffered
	setbuf(stdout, NULL);
}

int open_new_window(char const * const title, uint16_t * const rows, uint16_t * const cols)
{
	char buffer[SZ_BUF];
	
	sprintf(buffer, CMD_TERMINAL" /bin/sh -c \"tty>%s; stty -echo raw opost; while [ -p %s ]; do sleep 0.5; done; \"", title, NAME_FIFO, NAME_FIFO);
	system(buffer);
	
	FILE *f=fopen(NAME_FIFO, "r");
	if(!f)
		ERR(1, "failed to open pipe %s", NAME_FIFO);

	char path_pts[SZ_PATH_PTS];	
	fgets(path_pts, SZ_PATH_PTS, f);
	fclose(f);
	path_pts[strlen(path_pts)-1]='\0'; //remove newline
	
	int fd=open(path_pts, O_RDWR|O_NONBLOCK);
	if(!fd)
		ERR(1, "open pts");
	
	strcpy(buffer, "\033[?1049h\033[22;0;0t"); //tput smcup
	if(write(fd, buffer, strlen(buffer))==-1)
		ERR(1, "write pts init 1");
	
	strcpy(buffer, "\033[H\033[2J\033[3J"); //tput clear	
	if(write(fd, buffer, strlen(buffer))==-1)
		ERR(1, "write pts init 2");
	
	if(nb_window>NB_WINDOW_MAX)
		ERRX(1, "no more space in window_list");
	window_list[nb_window++]=fd;
	
	get_window_size(fd, rows, cols);
	
	return fd;
}

void get_window_size(const int fd, uint16_t * const rows, uint16_t * const cols)
{
	struct winsize ws;

	if (ioctl(fd, TIOCGWINSZ, &ws) < 0)
		ERR(1, "ioctl failed on fd %d", fd);
 
	if(rows)
		*rows=ws.ws_row;
	
	if(cols)
		*cols=ws.ws_col;
}

int win_printf(const int fd, char const * const fmt, ...)
{
	char buffer[SZ_BUF];
	va_list ap;
	va_start(ap, fmt);
	vsprintf(buffer, fmt, ap);
	va_end(ap);
	
	return write(fd, buffer, strlen(buffer));
}

static uint32_t process_input_buffer(char const * const buf, const uint32_t sz_buf, uint8_t * const events, const uint32_t sz_events)
{
	uint32_t i, j;
	
	i=0;
	j=0;
	
	while(i<sz_buf && j<sz_events)
	{
		if(buf[i]=='\033')
		{
			if(memcmp(&buf[i], "\033[A", 3)==0)
			{
				events[j++]=EV_SCROLL_UP;
				i+=3;
			}
			else if(memcmp(&buf[i], "\033[B", 3)==0)
			{
				events[j++]=EV_SCROLL_DOWN;
				i+=3;
			}
			else if(memcmp(&buf[i], "\033[D", 3)==0)
			{
				events[j++]=EV_KEY_LEFT;
				i+=3;
			}
			else if(memcmp(&buf[i], "\033[C", 3)==0)
			{
				events[j++]=EV_KEY_RIGHT;
				i+=3;
			}
			else if(memcmp(&buf[i], "\033[5~", 4)==0)
			{
				events[j++]=EV_PAGE_UP;
				i+=4;
			}
			else if(memcmp(&buf[i], "\033[6~", 4)==0)
			{
				events[j++]=EV_PAGE_DOWN;
				i+=4;
			}
			else
			{
				printf("process_input_buffer: unknown escape seq!\n");
				i++;
			}
		}
		else
		{
			//copy ASCII
			events[j++]=buf[i++];
		}
	}
	
	return j;
}

uint32_t read_input(const int fd, uint8_t * const buf, const uint32_t sz_buf)
{
	char buffer[SZ_BUF];
	
	int ret=read(fd, buffer, SZ_BUF);
	
	if(ret==-1)
		return 0;
	
	uint32_t nb_ev;
	nb_ev=process_input_buffer(buffer, ret, buf, sz_buf);
	
	return nb_ev;
}

uint32_t wait_for_input(const int fd, uint8_t * const buf, const uint32_t sz_buf) //blocking
{
	char buffer[SZ_BUF];
	
	int ret;
	do
	{
		ret=read(fd, buffer, SZ_BUF);
		if(ret==-1)
			sleep(0.1);
	} while(ret==-1);
	
	if(ret==-1)
		return 0;
	
	uint32_t nb_ev;
	nb_ev=process_input_buffer(buffer, ret, buf, sz_buf);
	
	return nb_ev;
}

void cmd_layout(PROTOTYPE_ARGS_HANDLER)
{
	(void)cmd; (void)nb_args;
	
	char buf[SZ_BUF];
	
	char *arg0, *arg1;
	arg0=get_next_argument();
	arg1=get_next_argument();
	
	sprintf(buf, "./layout.pl %s %s", arg0, arg1);
	system(buf);
}
