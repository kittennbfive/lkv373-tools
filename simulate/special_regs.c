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
#include <stdbool.h>

#include "special_regs.h"
#include "my_err.h"
#include "cmd_parser.h"

static const sr_t special_regs[]=
{
	{0x0, "CPU_VER"},
	{0x8, "ICM_CFG"},
	{0x10, "DCM_CFG"},
	{0x18, "MMU_CFG"},
	{0x20, "MSC_CFG"},
	{0x1, "CORE_ID"},
	{0x28, "FUCOP_EXIST"},
	{0x80, "PSW"},
	{0x81, "IPSW"},
	{0x82, "P_IPSW"},
	{0x89, "IVB"},
	{0x91, "EVA"},
	{0x92, "P_EVA"},
	{0x99, "ITYPE"},
	{0x9a, "P_ITYPE"},
	{0xa1, "MERR"},
	{0xa9, "IPC"},
	{0xaa, "P_IPC"},
	{0xab, "OIPC"},
	{0xb2, "P_P0"},
	{0xba, "P_P1"},
	{0xc0, "INT_MASK"},
	{0xc8, "INT_PEND"},
	{0x100, "MMU_CTL"},
	{0x108, "L1_PPTB"},
	{0x110, "TLB_VPN"},
	{0x118, "TLB_DATA"},
	{0x120, "TLB_MISC"},
	{0x128, "VLPT_IDX"},
	{0x130, "ILMB"},
	{0x138, "DLMB"},
	{0x140, "CACHE_CTL"},
	{0x148, "HSMP_SADDR"},
	{0x149, "HSMP_EADDR"},
	{0x180, "BPC0"},
	{0x188, "BPA0"},
	{0x190, "BPAM0"},
	{0x198, "BPV0"},
	{0x1a0, "BPCID0"},
	{0x181, "BPC1"},
	{0x189, "BPA1"},
	{0x191, "BPAM1"},
	{0x199, "BPV1"},
	{0x1a1, "BPCID1"},
	{0x182, "BPC2"},
	{0x18a, "BPA2"},
	{0x192, "BPAM2"},
	{0x19a, "BPV2"},
	{0x1a2, "BPCID2"},
	{0x183, "BPC3"},
	{0x18b, "BPA3"},
	{0x193, "BPAM3"},
	{0x19b, "BPV3"},
	{0x1a3, "BPCID3"},
	{0x184, "BPC4"},
	{0x18c, "BPA4"},
	{0x194, "BPAM4"},
	{0x19c, "BPV4"},
	{0x1a4, "BPCID4"},
	{0x185, "BPC5"},
	{0x18d, "BPA5"},
	{0x195, "BPAM5"},
	{0x19d, "BPV5"},
	{0x1a5, "BPCID5"},
	{0x186, "BPC6"},
	{0x18e, "BPA6"},
	{0x196, "BPAM6"},
	{0x19e, "BPV6"},
	{0x1a6, "BPCID6"},
	{0x187, "BPC7"},
	{0x18f, "BPA7"},
	{0x197, "BPAM7"},
	{0x19f, "BPV7"},
	{0x1a7, "BPCID7"},
	{0x1a8, "EDM_CFG"},
	{0x1b0, "EDMSW"},
	{0x1b8, "EDM_CTL"},
	{0x1c0, "EDM_DTR"},
	{0x1c8, "BPMTC"},
	{0x1d0, "DIMBR"},
	{0x1f0, "TECR0"},
	{0x1f1, "TECR1"},
	{0x200, "PFMC0"},
	{0x201, "PFMC1"},
	{0x202, "PFMC2"},
	{0x208, "PFM_CTL"},
	{0x280, "DMA_CFG"},
	{0x288, "DMA_GCSW"},
	{0x290, "DMA_CHNSEL"},
	{0x298, "DMA_ACT"},
	{0x2a0, "DMA_SETUP"},
	{0x2a8, "DMA_ISADDR"},
	{0x2b0, "DMA_ESADDR"},
	{0x2b8, "DMA_TCNT"},
	{0x2c0, "DMA_STATUS"},
	{0x2c8, "DMA_2DSET"},
	{0x2c9, "DMA_2DSCTL"},
	{0x220, "PRUSR_ACC_CTL"},
	{0x228, "FUCOP_CTL"},
	{0x178, "SDZ_CTL"},
	{0x179, "MISC_CTL"},
	{0, NULL}
};

static uint32_t sr[111]={0}; 

char * get_special_reg_name(const uint16_t sridx)
{
	uint16_t i;
	uint8_t found=0;
	for(i=0; special_regs[i].name; i++)
	{
		if(special_regs[i].id==sridx)
		{
			found=1;
			break;
		}
	}
	
	if(found)
		return special_regs[i].name;
	else
		return "UNKNOWN REG";
}

void write_to_special_reg(const uint16_t sridx, const uint32_t val)
{
	uint16_t i;
	uint8_t found=0;
	for(i=0; special_regs[i].name; i++)
	{
		if(special_regs[i].id==sridx)
		{
			found=1;
			break;
		}
	}
	
	if(!found)
		ERRX(1, "sr %u not found", sridx);
	
	sr[i]=val;
	
	//printf("value %x written to SR %s\n", val, special_regs[i].name);
	
	
	if(special_regs[i].id==SR_PROC_STATUS_WORD)
	{
		if(val&PSW_GIE)
			printf("INTERRUPTS ENABLED\n");
		else
			printf("INTERRUPTS DISABLED\n");
	}
	
}

uint32_t read_from_special_reg(const uint16_t sridx)
{
		uint16_t i;
	uint8_t found=0;
	for(i=0; special_regs[i].name; i++)
	{
		if(special_regs[i].id==sridx)
		{
			found=1;
			break;
		}
	}
	
	if(!found)
		ERRX(1, "sr %u not found", sridx);
	
	//printf("SR %s read, returning %x\n", special_regs[i].name, sr[i]);
	
	return sr[i];
}

#if 0
/******************************************************************************
 * ir0: PSW (Processor Status Word Register)
 * ir1: IPSW (Interruption PSW Register)
 * ir2: P_IPSW (Previous IPSW Register)
 *****************************************************************************/
#define PSW_offGIE		0	/* Global Interrupt Enable */
#define PSW_offINTL		1	/* Interruption Stack Level */
#define PSW_offPOM		3	/* Processor Operation Mode, User/Superuser */
#define PSW_offBE		5	/* Endianness for data memory access, 1:MSB, 0:LSB */
#define PSW_offIT		6	/* Enable instruction address translation */
#define PSW_offDT		7	/* Enable data address translation */
#define PSW_offIME		8	/* Instruction Machine Error flag */
#define PSW_offDME		9	/* Data Machine Error flag */
#define PSW_offDEX		10	/* Debug Exception */
#define PSW_offHSS		11	/* Hardware Single Stepping */
#define PSW_offIFCON		15	/* Hardware Single Stepping */
/* bit 12:31 reserved */
#endif

void special_reg_setgie(const uint8_t en)
{
	uint32_t psw=read_from_special_reg(SR_PROC_STATUS_WORD);
	
	if(en)
		psw|=PSW_GIE;
	else
		psw&=~PSW_GIE;
	
	write_to_special_reg(SR_PROC_STATUS_WORD, psw);
}

bool get_gie(void)
{
	uint32_t psw=read_from_special_reg(SR_PROC_STATUS_WORD);
	
	return psw&PSW_GIE;
}

/*
void cmd_write_sr(PROTOTYPE_ARGS_HANDLER)
{
	ARGS_HANDLER_UNUSED;
	
	
}
*/
