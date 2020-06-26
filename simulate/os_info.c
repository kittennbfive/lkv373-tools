/*
This file is part of the simulator for the LKV373
(c) 2020 by kitten_nb_five
freenode #lkv373a

licence: AGPL v3 or later

THIS PROGRAM COMES WITHOUT ANY WARRANTY!
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "os_info.h"

#include "simulate.h"
#include "verbosity.h"
#include "os_common.h"
#include "my_err.h"

#include "errorcodes.c"

void os_init(const pos_t pos)
{
	if(pos==START)
		MSG(MSG_OS_CALLS, "OSInit() called\n");
	else
		MSG(MSG_OS_CALLS, "OSInit() finished\n");
}

void os_start(const pos_t pos)
{
	if(pos==START)
		MSG(MSG_OS_CALLS, "OSStart() called - MULTITASKING WILL START\n");
	else
		MSG(MSG_OS_CALLS, "OSStart() finished\n");
}

void task_create(const pos_t pos) //not called so why bother?
{
	(void)pos;
	MSG(MSG_OS_CALLS, "os_info.c: OSTaskCreate() unimpl\n");
}

void task_create_ext(const pos_t pos)
{
	if(pos==START)
	{
		uint32_t task=get_register(0);
		uint32_t prio=get_register(3);
		uint32_t id=get_register(4);
		
		MSG(MSG_OS_CALLS, "OSTaskCreateExt(code @0x%x, prio 0x%x [%s], ID 0x%x [%s])\n", task, prio, get_task_name_from_priority(prio), id, get_task_name_from_id(id));
	}
	else
	{
		uint32_t r=get_register(0);
		MSG(MSG_OS_CALLS, "OSTaskCreateExt returned %u (%s)\n", r, errcodes[r]);
	}
}

void sem_create(const pos_t pos)
{
	if(pos==START)
	{
		uint32_t cnt=get_register(0);
		MSG(MSG_OS_CALLS, "[%s]: OSSemCreate(cnt==%u)\n", get_task_name_from_id(get_current_task_id()), cnt);
	}
	else
	{
		uint32_t r=get_register(0);
		int8_t i;
		if(r)
			i=(r-0xd0304)/36;
		else
			i=-1;
		MSG(MSG_OS_CALLS, "[%s]: OSSemCreate returned 0x%x (index %d of OSEventTbl)\n", get_task_name_from_id(get_current_task_id()), r, i);
	}
}

void sem_accept(const pos_t pos)
{
	if(pos==START)
	{
		uint32_t ptr=get_register(0);
		MSG(MSG_OS_CALLS, "[%s]: OSSemAccept(0x%x)\n", get_task_name_from_id(get_current_task_id()), ptr);
	}
	else
	{
		uint32_t r=get_register(0);
		MSG(MSG_OS_CALLS, "[%s]: OSSemAccept returned %u\n", r);
	}
}

//Because of multitasking there can be another call to OSSemPend before the first returns, so we need a LIFO/stack to keep track of the pointers
//to where the returncode is
#define PERR_MAX 5

static uint32_t perr[PERR_MAX];
static uint8_t nb_perr;

static void push(const uint32_t ptr)
{
	if(nb_perr<PERR_MAX)
	{
		perr[nb_perr++]=ptr;
	}
	else
		ERRX(1, "perr is full");
}

static uint32_t pop(void)
{
	if(nb_perr)
	{
		return perr[--nb_perr];
	}
	else
		ERRX(1, "trying to read from empty stack");
}

void sem_pend(const pos_t pos)
{
	if(pos==START)
	{
		uint32_t ecb=get_register(0);
		uint32_t timeout=get_register(1);
		uint32_t perr=get_register(2);
		push(perr);
		MSG(MSG_OS_CALLS, "[%s]: OSSemPend(ECB @0x%x, timeout=%u, perr=0x%x)\n", get_task_name_from_id(get_current_task_id()), ecb, timeout, perr);
	}
	else
	{
		uint32_t perr=pop();
		uint8_t r=get_byte(perr);
		MSG(MSG_OS_CALLS, "[%s]: OSSemPend returned %u [%s]\n", get_task_name_from_id(get_current_task_id()), r, errcodes[r]);
	}
}

void sem_post(const pos_t pos)
{
	if(pos==START)
	{
		uint32_t ecb=get_register(0);
		MSG(MSG_OS_CALLS, "[%s]: OSSemPost(ECB @0x%x)\n", get_task_name_from_id(get_current_task_id()), ecb);
	}
	else
	{
		uint8_t r=get_register(0);
		MSG(MSG_OS_CALLS, "[%s]: OSSemPost returned %u [%s]\n", get_task_name_from_id(get_current_task_id()), r, errcodes[r]);
	}
}

void task_suspend(const pos_t pos)
{
	if(pos==START)
	{
		uint16_t prio=get_register(0);
		MSG(MSG_OS_CALLS, "[%s]: OSTaskSuspend(prio=0x%x [%s])\n", get_task_name_from_id(get_current_task_id()), prio, get_task_name_from_priority(prio));
	}
	else
	{
		uint8_t r=get_register(0);
		MSG(MSG_OS_CALLS, "[%s]: OSTaskSuspend returned %u [%s]\n", get_task_name_from_id(get_current_task_id()), r, errcodes[r]);
	}
}

void task_switch(const pos_t pos)
{
	if(pos==START)
	{
		;
	}
	else
	{
		MSG(MSG_OS_CALLS, "---switched to Task %s---\n", get_task_name_from_id(get_current_task_id()));
	}
}

void time_get(const pos_t pos)
{
	if(pos==START)
	{
		;
	}
	else
	{
		uint32_t r=get_register(0);
		MSG(MSG_OS_CALLS, "[%s]: OSTimeGet returned %u\n", get_task_name_from_id(get_current_task_id()), r);
	}
}

void q_create(const pos_t pos)
{
	if(pos==START)
	{
		uint16_t start=get_register(0);
		uint16_t size=get_register(1);
		MSG(MSG_OS_CALLS, "[%s]: OSQCreate(start=0x%x, size=0x%x)\n", get_task_name_from_id(get_current_task_id()), start, size);
	}
	else
	{
		uint32_t r=get_register(0);
		int8_t i;
		if(r)
			i=(r-0xd0304)/36;
		else
			i=-1;
		MSG(MSG_OS_CALLS, "[%s]: OSQCreate returned 0x%x (index %d of OSEventTbl)\n", get_task_name_from_id(get_current_task_id()), r, i);
	}
}


//TODO, seem unused??
void time_set(const pos_t pos)
{ (void)pos; MSG(MSG_OS_CALLS, "------------->time_set\n"); }

void ev_pend_multi(const pos_t pos)
{ (void)pos; MSG(MSG_OS_CALLS, "------------->ev_pend_multi\n"); }

void flag_accept(const pos_t pos)
{ (void)pos; MSG(MSG_OS_CALLS, "------------->flag_accept\n"); }

void flag_create(const pos_t pos)
{ (void)pos; MSG(MSG_OS_CALLS, "------------->flag_create\n"); }

void flag_block(const pos_t pos)
{ (void)pos; MSG(MSG_OS_CALLS, "------------->flag_block\n"); }



/*
void ...(const pos_t pos)
{
	if(pos==START)
	{
		MSG(MSG_OS_CALLS, "[%s]: ...\n", get_task_name_from_id(get_current_task_id()), ...);
	}
	else
	{
		MSG(MSG_OS_CALLS, "[%s]: ... returned %u [%s]\n", get_task_name_from_id(get_current_task_id()), ...);
	}
}
*/


typedef struct
{
	uint32_t PC_start;
	uint32_t PC_ret;
	char * func_name;
	void (*callback)(const pos_t pos);
} OS_func_t;

const OS_func_t list[]=
{
	{ 0x53e4, 0x57e8, "OSInit()", &os_init },
	{ 0x1120, 0x1176, "OSStart()", &os_start },
	{ 0x4308, 0x43b2, "OSTaskCreate()", &task_create },
	{ 0x442c, 0x44fa, "OSTaskCreateExt()", &task_create_ext },
	{ 0x3d44, 0x3da0, "OSSemCreate()", &sem_create },
	{ 0x3d14, 0x3d42, "OSSemAccept()", &sem_accept },
	{ 0x3ea8, 0x3f9c, "OSSemPend()", &sem_pend },
	{ 0x4028, 0x4090, "OSSemPost()", &sem_post },
	{ 0x1bf4, 0x1cc2, "OSTaskSuspend()", &task_suspend },
	{ 0x58ca, 0x5988, "OS_TASK_SW()", &task_switch },
	{ 0x4b58, 0x4b74, "OSTimeGet()", &time_get },
	{ 0x4b76, 0x4b94, "OSTimeSet()", &time_set },
	{ 0x1978, 0x1bbc, "OSEventPendMulti", &ev_pend_multi },
	{ 0x2138, 0x21fa, "OSFlagAccept", &flag_accept },
	{ 0x21fc, 0x225c, "OSFlagCreate", &flag_create },
	{ 0x2338, 0x23cc, "OS_FlagBlock", &flag_block },
	{ 0x36f4, 0x37a2, "OSQCreate", &q_create },
	
	
	
	{0, 0, NULL, NULL}
};

void os_info(const uint32_t PC)
{
	uint8_t i;
	for(i=0; list[i].PC_start; i++)
	{
		if(list[i].PC_start==PC)
		{
			list[i].callback(START);
			break;
		}
		else if(list[i].PC_ret==PC)
		{
			list[i].callback(END);
			break;
		}
	}
}
