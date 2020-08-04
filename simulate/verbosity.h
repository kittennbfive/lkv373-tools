/*
This file is part of the simulator for the LKV373
(c) 2019-2020 by kitten_nb_five
freenode #lkv373a

licence: AGPL v3 or later

THIS PROGRAM COMES WITHOUT ANY WARRANTY!
*/

#ifndef __VERBOSITY_H__
#define __VERBOSITY_H__
#include <stdbool.h>

#include "cmd_parser.h"

#define MSG_PC (1U<<0) //unimplemented, output would be too big. implement if needed for debug, should be easy
#define MSG_CALL (1U<<1) //jal (jump and link)
#define MSG_RET (1U<<2) //ret
#define MSG_IRET (1U<<3) //iret
#define MSG_GOTO (1U<<4) //j (jump) or conditional branch
#define MSG_IRQ (1U<<5) //interrupt request from peripheral
#define MSG_INT (1U<<6) //actual interrupt service routine being entered
#define MSG_PERIPH_CPE (1U<<7)
#define MSG_PERIPH_PMU (1U<<8)
#define MSG_PERIPH_TMR1 (1U<<9)
#define MSG_PERIPH_INTC (1U<<10)
#define MSG_PERIPH_UART (1U<<11)
#define MSG_PERIPH_SSPC (1U<<12)
#define MSG_PERIPH_MAC (1U<<13)
#define MSG_PERIPH_GPIO (1U<<14)
#define MSG_CONNECTOR (1U<<15) //serial connector to real
#define MSG_OS_CALLS (1U<<16) //calls to functions from OS and results
#define MSG_PERIPH_UNKNOWN (1U<<17)
#define MSG_PERIPH_I2C (1U<<18)
#define MSG_HOOKS (1U<<19)

#define MSG_MEM (1U<<29) //memory writes

#define MSG_ALWAYS (1U<<30) //print always, used for init-messages, errors, ...
#define MSG_VARIOUS (1U<<31) //currently unused

void cmd_set_verbosity(PROTOTYPE_ARGS_HANDLER);

void message_printf(const uint32_t msg_type, char const * const fmt, ...);

#define MSG(msg, fmt, ...) message_printf(msg, fmt, ##__VA_ARGS__)

#endif
