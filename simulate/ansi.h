/*
This file is part of the simulator for the LKV373
(c) 2019-2020 by kitten_nb_five
freenode #lkv373a

licence: AGPL v3 or later

THIS PROGRAM COMES WITHOUT ANY WARRANTY!
*/

#ifndef __ANSI_H__
#define __ANSI_H__

#define COLOR(color, text) "\033["color"m"text"\033[0m"

#define SET_COLOR "\033[%sm"
#define RESET_COLOR "\033[0m"

#define WHITE "0"
#define RED "31"
#define GREEN "32"
#define YELLOW "93" //"1;33"
#define BLUE "34"
#define GREY "37"

#define CLEAR_SCREEN "\033[2J\033[1;1H"

#define HIDE_CURSOR "\033[?25l"
#define SHOW_CURSOR "\033[?25h"

#endif
