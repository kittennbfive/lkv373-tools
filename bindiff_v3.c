#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//pipe into less -R for correct color handling

#define VERSION 4

#define min(a,b) ((a<b)?a:b)
#define max(a,b) ((a>b)?a:b)

#define COLOR(color, text) "\e["color"m"text"\e[0m"
#define RED "31"

#define BYTES_PER_LINE_DEF 8

void print_usage(void)
{
	printf("bindiff [-bpl n] [-r] file1 file2\n-bpl n for 2*n bytes per line\n-r for removing equal lines\n");
}

void error(char const * const msg)
{
	printf("err: %s\n", msg);
	exit(1);
}

unsigned int is_equal(unsigned char const * const data1, unsigned char const * const data2, const unsigned int index, const unsigned int bytes_per_line)
{
	unsigned int i;
	for(i=0; i<bytes_per_line; i++)
	{
		if(data1[index+i]!=data2[index+i])
			return 0;
	}
	return 1;
}

char mask_unprintable(const char c)
{
	if(c<0x20 || c>0x7E)
		return '.';
	else
		return c;
}

int main(int argc, char **argv)
{
	printf("This is bindiff version %u (c) 2019 by kitten_nb_five.\n", VERSION);
	printf("This programm comes without any warranty.\n\n");
	if(argc<3)
	{
		print_usage();
		return 0;
	}
	
	unsigned int argv_offset=1;
	
	unsigned int bytes_per_line=BYTES_PER_LINE_DEF;
	unsigned int remove_equal=0;
	
	while(argv_offset<argc-2)
	{
		if(strcmp(argv[argv_offset],"-bpl")==0)
		{
			sscanf(argv[argv_offset+1],"%u",&bytes_per_line);
			argv_offset+=2;
			if(bytes_per_line==0)
				error("invalid argument for -bpl");
		}
		else if(strcmp(argv[argv_offset],"-r")==0)
		{
			argv_offset++;
			remove_equal=1;
		}
		else
			error("invalid argument");
	}

	printf("comparing \"%s\" (left) and \"%s\" (right)\n", argv[argv_offset], argv[argv_offset+1]);
	
	FILE *file1=fopen(argv[argv_offset], "rb");
	if(!file1)
		error("could not open file 1");
	fseek(file1, 0, SEEK_END);
	unsigned int size1=ftell(file1);
	fseek(file1, 0, SEEK_SET);
	unsigned char * data1=malloc(size1);
	if(!data1)
		error("memory allocation for file 1 failed");
	if(!fread(data1, size1, 1, file1))
		error("reading file 1 failed");
	fclose(file1);

	FILE *file2=fopen(argv[argv_offset+1], "rb");
	if(!file2)
		error("could not open file 2");
	fseek(file2, 0, SEEK_END);
	unsigned int size2=ftell(file2);
	fseek(file2, 0, SEEK_SET);
	unsigned char * data2=malloc(size2);
	if(!data2)
		error("memory allocation for file 2 failed");
	if(!fread(data2, size2, 1, file2))
		error("reading file 2 failed");
	fclose(file2);

	unsigned int size=size1;
	
	if(size1!=size2)
	{
		size=max(size1, size2);
		printf("info: size missmatch: %u vs %u bytes\n", size1, size2);
	}
	
	unsigned int index=0;
	unsigned int bytes_in_line=0;
	unsigned int equal_line_removed=0;
	
	for(index=0; index<size; )
	{
		if(is_equal(data1, data2, index, bytes_per_line) && remove_equal)
		{
			index+=bytes_per_line;
			if(!equal_line_removed)
			{
				equal_line_removed=1;
				printf("...\n");
			}
			continue;
		}
		
		equal_line_removed=0;
		
		printf("0x%08x: ", index);
		for(bytes_in_line=0; bytes_in_line<bytes_per_line; bytes_in_line++)
		{
			if(index+bytes_in_line>=size1)
				printf("   ");
			else if(data1[index+bytes_in_line]!=data2[index+bytes_in_line])
				printf(COLOR(RED,"%02x "), data1[index+bytes_in_line]);
			else
				printf("%02x ", data1[index+bytes_in_line]);
		}
		
		printf("| ");
		for(bytes_in_line=0; bytes_in_line<bytes_per_line; bytes_in_line++)
		{
			if(index+bytes_in_line>=size2)
				printf("   ");
			else if(data1[index+bytes_in_line]!=data2[index+bytes_in_line])
				printf(COLOR(RED,"%02x "), data2[index+bytes_in_line]);
			else
				printf("%02x ", data2[index+bytes_in_line]);
		}
		
		printf(" ");
		
		for(bytes_in_line=0; bytes_in_line<bytes_per_line; bytes_in_line++)
		{
			if(index+bytes_in_line>=size1)
				printf(" ");
			else if(data1[index+bytes_in_line]!=data2[index+bytes_in_line])
				printf(COLOR(RED,"%c"), mask_unprintable(data1[index+bytes_in_line]));
			else
				printf("%c", mask_unprintable(data1[index+bytes_in_line]));
		}
		printf(" | ");
		for(bytes_in_line=0; bytes_in_line<bytes_per_line; bytes_in_line++)
		{
			if(index+bytes_in_line>=size2)
				printf(" ");
			else if(data1[index+bytes_in_line]!=data2[index+bytes_in_line])
				printf(COLOR(RED,"%c"), mask_unprintable(data2[index+bytes_in_line]));
			else
				printf("%c", mask_unprintable(data2[index+bytes_in_line]));
		}
		
		printf("\n");
		index+=bytes_per_line;
	}
	
	free(data1);
	free(data2);
	
	return 0;
}
