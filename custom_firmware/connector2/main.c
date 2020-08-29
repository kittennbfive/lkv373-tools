/*
(c) 2020 by kitten_nb_five
freenode #lkv373a

licence: AGPL v3 or later

THIS CODE COMES WITHOUT ANY WARRANTY!
*/

#include <stdlib.h>
#include <stdio.h>

#include "sspc.h"
#include "gpio.h"
#include "parser.h"
#include "uart.h"
#include "i2c_ite.h"

#define SZ_BUF 10

int main(void)
{
	gpio_init(); //for reset button
	setbuf(stdout, NULL); //otherwise printf("<no \r\n here>") will not show up (immediatly)!

	if(is_reset_button_pressed())
			go_into_bootloader();

	unsigned char buf[SZ_BUF];
	unsigned char i;
	unsigned char c;

	ite_i2c_init();

	while(1)
	{
		if(is_reset_button_pressed())
			go_into_bootloader();
		
		do
		{
			c=inbyte();
		} while(c!='=' && c!='?' && c!='Q' && c!='I');
		
		buf[0]=c;
		
		for(i=1; i<SZ_BUF; i++)
		{
			buf[i]=inbyte();
		}
		
		parse(buf);
	}

	return 0;
}
