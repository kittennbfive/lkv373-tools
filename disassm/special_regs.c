/*
This file is part of a disassembler for Andestech NDS32.

(c) 2019-2020 kitten_nb_five

THIS WORK COMES WITHOUT ANY WARRANTY and is released under the AGPL version 3 or later
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "special_regs.h"

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

		
