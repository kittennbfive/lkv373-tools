#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "i2c.h"
#include "peripherals.h"
#include "verbosity.h"
#include "cmd_parser.h"

#define I2C_PRESCALER_REG 0x99c00000 //write 0xC8 for 100kHz (TODO: measure actual speed)
#define I2C_UNKNOWN1_REG 0x99c00004 //write 0x80 to enable I2C
#define I2C_DATA_TX_REG 0x99c00008
#define I2C_CTRL_REG 0x99c0000c
#define I2C_DATA_RX_REG 0x99c00010
#define I2C_STATUS_REG 0x99c00014

static uint32_t unknown1_reg=0x00;
static uint32_t data_tx_reg=0x00;
static uint32_t data_rx_reg=0x00;
static uint32_t status_reg=0x00;

typedef enum
{
	IDLE,
	READ,
	WRITE1,
	WRITE2
} i2c_state_t;

i2c_state_t i2c_state=IDLE;

//IT6604
static uint32_t it_reg_addr=0;
static uint8_t it_regs[256]={0};

#define SYS_STATE_REG 0x10
#define INT0_REG 0x13
#define INT0_MASK_REG 0x16
#define INT_CTRL_REG 0x19
#define HDCP_CTRL_REG 0x73
#define TRISTATE_CTRL_REG 0x89

void init_i2c_it(void)
{
	it_regs[0x02]=0x23;
	it_regs[0x03]=0x60;
	it_regs[0x04]=0xa3;
	it_regs[0x07]=0x0C;
	it_regs[0x08]=0b10011011;
	it_regs[0x09]=0b10011001;
	it_regs[0x0A]=0x2A;
	it_regs[0x0B]=0xA5;
	it_regs[0x11]=0x89;
	it_regs[0x16]=0x3F;
	it_regs[0x19]=0b00110000;
	it_regs[0x1A]=0b00010100;
	it_regs[0x1D]=0x30;
	it_regs[0x1E]=0xDA;
	it_regs[0x21]=0x10;
	it_regs[0x22]=0x80;
	it_regs[0x24]=0xB2;
	it_regs[0x25]=0x04;
	it_regs[0x26]=0x64;
	it_regs[0x27]=0x02;
	it_regs[0x28]=0xE9;
	it_regs[0x2A]=0x93;
	it_regs[0x2B]=0x1C;
	it_regs[0x2C]=0x16;
	it_regs[0x2D]=0x04;
	it_regs[0x2E]=0x56;
	it_regs[0x2F]=0x1F;
	it_regs[0x30]=0x49;
	it_regs[0x31]=0x1D;
	it_regs[0x32]=0x9F;
	it_regs[0x33]=0x1E;
	it_regs[0x34]=0x16;
	it_regs[0x35]=0x04;
	it_regs[0x3D]=0b10000000;
	it_regs[0x68]=0x04;
	it_regs[0x6B]=0x10;
	it_regs[0x6C]=0x03;
	it_regs[0x73]=0x20;
	it_regs[0x75]=0b01100000;
	it_regs[0x76]=0b11100100;
	it_regs[0x77]=0b00100000;
	it_regs[0x78]=0b11000001;
	it_regs[0x7E]=0b00000010;
	it_regs[0x87]=0b00001000;
	it_regs[0x89]=0x80;
	it_regs[0x93]=0x03;
	it_regs[0x94]=0x60;
	it_regs[0x95]=0x8F;
	it_regs[0x96]=0b00110000;
	it_regs[0xA7]=0x0A;
	it_regs[0xA8]=0x83;
	it_regs[0xFF]=0x04;
}
void i2c_write(PERIPH_CB_WRITE_ARGUMENTS)
{
	(void)sz;
	
	switch(addr)
	{
		case I2C_PRESCALER_REG:
			MSG(MSG_PERIPH_I2C, "I2C: prescaler set to 0x%x\n", val);
			break;
		
		case I2C_UNKNOWN1_REG:
				unknown1_reg=val;
			if(val==0x80)
				MSG(MSG_PERIPH_I2C, "I2C: enabled\n");
			break;
		
		case I2C_DATA_TX_REG:
			data_tx_reg=val;
			break;
		
		case I2C_CTRL_REG:
			switch(val)
			{
				case 0x10:
					if(i2c_state==WRITE1)
					{
						status_reg=0x41;
						it_reg_addr=data_tx_reg;
						i2c_state=WRITE2;
					}
					else if(i2c_state==WRITE2)
					{
						it_regs[it_reg_addr]=data_tx_reg;
						MSG(MSG_PERIPH_I2C, "I2C: IT: register 0x%x set to 0x%x\n", it_reg_addr, data_tx_reg);
						if(it_reg_addr==0x19 && data_tx_reg==0x31)
						{
							MSG(MSG_PERIPH_I2C, "I2C: IT: clearing INT-registers\n");
							it_regs[0x13]=0;
							it_regs[0x14]=0;
							it_regs[0x15]=0;
						}
					}
					else
						MSG(MSG_ALWAYS, "I2C: unexpected send byte command\n");
					
					break;
				
				case 0x20:
						MSG(MSG_ALWAYS, "I2C: unimpl CTRL==0x20 (read+ACK)\n");
					break;
				
				case 0x28:
					if(i2c_state==READ)
					{
						data_rx_reg=it_regs[it_reg_addr];
						MSG(MSG_PERIPH_I2C, "I2C: IT: read register 0x%x == 0x%x\n", it_reg_addr, data_rx_reg);
					}
					else
						MSG(MSG_ALWAYS, "I2C unexpected read byte +NACK command\n");
					break;
				
				case 0x40:
					status_reg=0x81;
					i2c_state=IDLE;
					break;
				
				case 0x90:
					status_reg=0x41;
					if(data_tx_reg&1)
						i2c_state=READ;
					else
						i2c_state=WRITE1;
						
					break;
				
				default:
					MSG(MSG_ALWAYS, "I2C: unknown command 0x%x written to CTRL_REG\n", val);
					break;
			}
			break;
		
		case I2C_DATA_RX_REG:
			MSG(MSG_ALWAYS, "I2C: trying to write to RX_REG, something is wrong!\n");
			break;
		
		case I2C_STATUS_REG:
			MSG(MSG_ALWAYS, "I2C: writing 0x%x to STATUS_REG\n", val);
			status_reg=val;
			break;
		
		default:
			MSG(MSG_ALWAYS, "I2C: write 0x%x to unknown register 0x%x\n", val, addr);
			break;
	}
}


bool i2c_read(PERIPH_CB_READ_ARGUMENTS)
{
	(void)sz;
	
	switch(addr)
	{
		case I2C_UNKNOWN1_REG:
			(*val)=unknown1_reg;
			return true;
			break;
		
		case I2C_DATA_RX_REG:
			(*val)=data_rx_reg;
			return true;
			break;
		
		case I2C_STATUS_REG:
			(*val)=status_reg;
			return true;
			break;
		
		default:
			MSG(MSG_ALWAYS, "I2C: trying to read from unknown register or not implemented 0x%x\n", addr);
			return false;
	}
	
}

void hdmi(PROTOTYPE_ARGS_HANDLER)
{
	ARGS_HANDLER_UNUSED;
	
	uint8_t step=atoi(get_next_argument());
	
	switch(step)
	{
		case 1: 
			it_regs[0x10]=0xAA;
			it_regs[0x19]=0x30;
			it_regs[0x13]=0x0C;
			MSG(MSG_ALWAYS, "I2C: set it_regs for step 1\n");
			break;

		case 2: 
			it_regs[0x10]=0xAE;
			it_regs[0x13]=0x04;
			MSG(MSG_ALWAYS, "I2C: set it_regs for step 2\n");
			break;
		
		case 3: 
			it_regs[0x10]=0xAF;
			it_regs[0x13]=0x21;
			it_regs[0x16]=0x3F;
			it_regs[0x3C]=0x00;
			it_regs[0x89]=0x80;
			it_regs[0x73]=0x31;
			MSG(MSG_ALWAYS, "I2C: set it_regs for step 3\n");
			break;
		
		case 99:
			it_regs[0x10]=0xaf;
			it_regs[0x16]=0xf;
			it_regs[0x3c]=0x00;
			it_regs[0x20]=0x00;
			it_regs[0x89]=0xff;
			it_regs[0x19]=0x30;
			it_regs[0x73]=0x1;
			it_regs[0x13]=0x4;
			it_regs[0x5a]=0x78;
			it_regs[0x5b]=0x80;
			it_regs[0x5c]=0x2c;
			it_regs[0x5d]=0x0;
			it_regs[0x5e]=0x58;
			it_regs[0x5f]=0x65;
			it_regs[0x60]=0x44;
			it_regs[0x61]=0x38;
			it_regs[0x62]=0x29;
			it_regs[0x63]=0x4;
			it_regs[0x58]=0x8;
			it_regs[0x64]=0x17;
			it_regs[0x59]=0x98;
			it_regs[0x65]=0x3;
			it_regs[0x73]=0x31;
			break;
		
		default:
			MSG(MSG_ALWAYS, "I2C: hdmi: invalid step\n");
			break;
	}
}
