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

int main(void)
{
	gpio_init(); //for reset button
	setbuf(stdout, NULL); //otherwise printf("<no \r\n here>") will not show up (immediatly)!
	
	printf("\r\n");
	printf("Hello World from newlib!\r\n");

	while(1)
	{
		if(is_reset_button_pressed())
			go_into_bootloader();
	}

	return 0;
}
