//extracts strings WITH ADDRESS out of binary
//compile+run 2 times and copy the results together to strings_all.txt
//(c) 2020 kitten_nb_five
//freenode #lkv373a
//NO WARRANTY
//licence: AGPL v3 or later

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void error(char const * const msg)
{
	printf("err: %s\n", msg);
	exit(1);
}

unsigned int is_printable(const char c)
{
	if(c=='\r' || c=='\n' || c=='\t' || (c>=0x20 && c<=0x7e))
		return 1;
	else
		return 0;
}

#define LENGTH_MIN 4

//part 1
//#define START_ADDR 0x3B974
//#define END_ADDR 0x42EAA

//part 2
#define START_ADDR 0xA1DC8
#define END_ADDR 0xD915C

int main(void)
{
	
	FILE *file=fopen("lkv373.bin", "rb");
	if(!file)
		error("could not open file");
	fseek(file, 0, SEEK_END);
	unsigned int size=ftell(file);
	fseek(file, 0, SEEK_SET);
	unsigned char * data=malloc(size);
	if(!data)
		error("memory allocation for file failed");
	if(!fread(data, size, 1, file))
		error("reading file failed");
	fclose(file);
	
	if(START_ADDR>=size || END_ADDR>=size)
		error("invalid addr");
	
	
	unsigned char * buf=malloc(50000);
	if(!buf)
		error("memory allocation for buf failed");
	
	
	unsigned int i,j,k;
	i=START_ADDR;
	while(i<END_ADDR)
	{
		j=0;
		while(i+j<END_ADDR && is_printable(data[i+j]))
			j++;
		
		if(j>=LENGTH_MIN && data[i+j]==0)
		{
			memcpy(buf, data+i, j+1);
			for(k=0; k<j; k++)
			{
				if(buf[k]=='\r' || buf[k]=='\n')
					buf[k]=' ';
			}
			printf("%08x: %u: %s\n", i, j, buf);
		}
		
		if(j)
			i+=j;
		else
			i++;
	}
	
	free(buf);
	free(data);
	return 0;
}
