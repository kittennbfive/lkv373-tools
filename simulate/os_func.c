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

#include "cmd_parser.h"
#include "os_common.h"

//OFFSETS FOR NORMAL FIRMWARE - BLOCK2 ONLY
//NOT VALID FOR BOOTLOADER!

//----------------------------------------------------------------------
//TASK CONTROL BLOCKS

#define POS_TCBTABLE 0xd0b74
#define NB_ENTRIES_TCBTABLE 22 //22
#define SIZEOF_TCBTABLE 92

#define TCB_WORD(entry, offset) get_word(POS_TCBTABLE+(entry)*SIZEOF_TCBTABLE+offset)
#define TCB_HALFWORD(entry, offset) get_halfword(POS_TCBTABLE+(entry)*SIZEOF_TCBTABLE+offset)
#define TCB_BYTE(entry, offset) get_byte(POS_TCBTABLE+(entry)*SIZEOF_TCBTABLE+offset)

#define SZ_NAME 15

void show_all_tcb(void)
{
	uint8_t i, j;
	uint32_t stack_top, stack_bottom;
	uint32_t task_id;
	uint32_t sp, addr_ipc, ipc;
	
	
	char name[SZ_NAME];
	
	for(i=0; i<NB_ENTRIES_TCBTABLE; i++)
	{
		printf("entry %d @0x%x:\n", i, POS_TCBTABLE+i*SIZEOF_TCBTABLE);

		task_id=TCB_HALFWORD(i, 0x12);
		
		if(!task_id)
		{
			printf("  (empty)\n");
			continue;
		}
		
		stack_top=TCB_WORD(i, 0x0);
		stack_bottom=TCB_WORD(i, 0x8);
		
		printf("  top of stack: 0x%x\n", stack_top);
		printf("  used defined data: 0x%x\n", TCB_WORD(i, 0x4));
		printf("  bottom of stack: 0x%x\n", stack_bottom);
		printf("  (stack size: 0x%x)\n", stack_top-stack_bottom);
		printf("  Task ID: 0x%x (%s)\n", task_id, get_task_name_from_id(task_id));
		printf("  prev TCB: 0x%x\n", TCB_WORD(i, 0x14));
		printf("  next TCB: 0x%x\n", TCB_WORD(i, 0x18));
		printf("  event ctrl block: 0x%0x\n", TCB_WORD(i, 0x1c));
		
		j=0;
		do
		{
			name[j]=TCB_BYTE(i, 0x4c+j);
		} while(name[j++] && j<SZ_NAME);
		name[j]='\0';
		printf("  name: %s\n", name);
		
		sp=get_word(stack_top);
		addr_ipc=stack_top+8;
		if((sp&7)==0) //don't ask me why this is done, i just copied from the disassembly. must be some alignment/ABI-stuff
			addr_ipc+=4;
		ipc=get_word(addr_ipc);
		printf("  sp: 0x%x\n", sp);
		printf("  IPC: 0x%x\n", ipc);
	}	
}

//----------------------------------------------------------------------
//READY LIST
#define POS_RDYTBL 0xcfefc

void decode_rdy_tbl(void)
{
	uint8_t priority;
	
	for(priority=0; priority<64; priority++)
	{
		if((get_byte(POS_RDYTBL+priority/8)>>(priority%8))&1)
			printf("Task with priority 0x%x (%s) is ready to run\n", priority, get_task_name_from_priority(priority));
	}
	
}

//----------------------------------------------------------------------
//OSEventTbl
#define POS_OSEVENTTBL 0xd0304
#define NB_ENTRIES_OSEVENTTBL 60
#define SIZEOF_OSEVENTTBL 36

#define OS_EVENT_TBL_SIZE 8
#define OS_EVENT_NAME_SIZE 17

#include "priotable.c"

typedef struct __attribute__((__packed__))
{
    uint8_t    OSEventType;                    /* Type of event control block (see OS_EVENT_TYPE_xxxx)    */
    uint8_t align[3];
    uint32_t	OSEventPtr; /*actually a pointer but sizeof==4, not 8 as on a modern PC! */ /* Pointer to message or queue structure                   */
    uint16_t   OSEventCnt;                     /* Semaphore Count (not used if other EVENT type)          */
    uint8_t    OSEventGrp;                     /* Group corresponding to tasks waiting for event to occur */
    uint8_t    OSEventTbl[OS_EVENT_TBL_SIZE];  /* List of tasks waiting for event to occur                */
    char    OSEventName[OS_EVENT_NAME_SIZE];
} os_event_t;

char *get_type_str(const uint8_t t)
{
	switch(t)
	{
		case 0: return "OS_EVENT_TYPE_UNUSED";
		case 1: return "OS_EVENT_TYPE_MBOX";
		case 2: return "OS_EVENT_TYPE_Q";
		case 3: return "OS_EVENT_TYPE_SEM";
		case 4: return "OS_EVENT_TYPE_MUTEX";
		case 5: return "OS_EVENT_TYPE_FLAG";
		
		case 100: return "OS_TMR_TYPE";
		
		default: return "unknown!";
	}  
}

void parse_os_event(const uint8_t index)
{
	os_event_t ev;
	
	//yeah...
	uint8_t i;
	for(i=0; i<SIZEOF_OSEVENTTBL; i++)
		*((uint8_t*)&ev+i)=get_byte(POS_OSEVENTTBL+index*SIZEOF_OSEVENTTBL+i);
		
	uint8_t type=ev.OSEventType;
		if(type==0)
		{
			printf("(empty)\n");
			return;
		}
		
		printf("OSEventType: %u==%s\n", type, get_type_str(type));
		printf("OSEventPtr: 0x%x\n", ev.OSEventPtr);
		printf("OSEventCnt: %u\n", ev.OSEventCnt);
		printf("OSEventName: %s\n", ev.OSEventName);
		
		uint8_t grp=ev.OSEventGrp;
		if(grp==0)
		{
			printf("no tasks waiting for this event\n");
			return;
		}
		
		printf("OSEventGrp: %u\n", grp);
		printf("Prio of tasks waiting for this event: ");
		uint8_t y=OSUnMapTbl[ev.OSEventGrp];
		uint8_t j;
		for(j=0; j<OS_EVENT_TBL_SIZE; j++)
		{
			if(ev.OSEventTbl[j]==0)	
				continue;
			uint8_t x=OSUnMapTbl[ev.OSEventTbl[y]];
			uint8_t prio=(y<<3)+x;			
			printf("0x%x (%s)", prio, get_task_name_from_priority(prio));
		}
		printf("\n");
}

void decode_os_ev_tbl(void)
{
	uint8_t i;
	for(i=0; i<NB_ENTRIES_OSEVENTTBL; i++)
	{
		printf("OSEventTbl[%u] @0x%x:\n", i, POS_OSEVENTTBL+i*SIZEOF_OSEVENTTBL);
		parse_os_event(i);
		printf("\n");
	}
}

//----------------------------------------------------------------------
//OSTCBCur

void show_curr_task(void)
{
	uint16_t task_id=get_current_task_id();
	printf("ID of current task is 0x%x (%s)\n", task_id, get_task_name_from_id(task_id));
}

//----------------------------------------------------------------------
//DISPATCHER

void show_os_info(PROTOTYPE_ARGS_HANDLER)
{
	ARGS_HANDLER_UNUSED;
	
	char *show_what=get_next_argument();
	
	if(!strcmp(show_what, "tcb"))
		show_all_tcb();
	else if(!strcmp(show_what, "rdy"))
		decode_rdy_tbl();
	else if(!strcmp(show_what, "evtbl"))
		decode_os_ev_tbl();
	else if(!strcmp(show_what, "curr"))
		show_curr_task();
	else
		printf("unknown: %s\n", show_what);
}

