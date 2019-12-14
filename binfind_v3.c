//(c) 2019 by kitten_nb_five
//freenode #lkv373a
//licence: APGL v3 or later

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define VERSION 3

void print_usage(void)
{
	printf("bindiff [-p n] [-m n] [-r] file1 file2 for searching file2 inside file1\n-p n for showing partial matches from n bytes\n-m n to allow up to n different bytes\nwarning: for some arguments output can be huge!");
}

void error(char const * const msg)
{
	printf("err: %s\n", msg);
	exit(1);
} 

int main(int argc, char **argv)
{
	printf("This is binfind version %u (c) 2019 by kitten_nb_five.\n", VERSION);
	printf("This programm comes without any warranty.\n");
	if(argc<3)
	{
		print_usage();
		return 0;
	}
	
	unsigned int argv_offset=1;
	unsigned int partial=0;
	unsigned int allowed_mismatches=0;

	while(argv_offset<argc-2)
	{
		if(strcmp(argv[argv_offset],"-p")==0)
		{
			sscanf(argv[argv_offset+1],"%u",&partial);
			argv_offset+=2;
			if(partial==0)
				error("invalid argument for -p");
		}
		else if(strcmp(argv[argv_offset],"-m")==0)
		{
			sscanf(argv[argv_offset+1],"%u",&allowed_mismatches);
			argv_offset+=2;
			if(allowed_mismatches==0)
				error("invalid argument for -m");
		}
		else
			error("invalid argument");
	}

	unsigned int flag_match=0;
		
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
	
	if(size1<size2 && partial!=0)
		error("file 1 must be bigger than file 2 to make a complete match possible");

	unsigned int index1, index2;
	unsigned int search_index1;
	unsigned int nb_mismatch;
	
	for(index1=0; index1<size1; index1++)
	{
		search_index1=index1;
		index2=0;
		nb_mismatch=0;
		while(search_index1<size1 && index2<size2 && (data2[index2]==data1[search_index1] || ++nb_mismatch<=allowed_mismatches))
		{
			search_index1++;
			index2++;
		}
		if(index2==size2)
		{
			printf("match: file 2 is contained in file 1 @0x%08x ", index1);
			if(nb_mismatch)
				printf("(%u different bytes == %.4f%% of file 2)", nb_mismatch, 100.0*nb_mismatch/size2);
			printf("\n");
			flag_match=1;
		}
		else if(partial && (search_index1-index1)>=partial)
		{
			printf("partial match: first %u bytes of file 2 are inside file 1 @0x%08x", search_index1-index1, index1);
			if(nb_mismatch)
				printf("(%u different bytes == %.4f%% of file 2)", nb_mismatch, 100.0*nb_mismatch/size2);
			printf("\n");
			flag_match=1;
		}
	}
	
	if(!flag_match)
		printf("no match\n");
	
	free(data1);
	free(data2);
	
	return 0;
}

