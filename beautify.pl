#this tool "beautifies" the output of the objdump -D by transforming memory-writes to single lines of pseudo-C-code
#really much work in progress
#NO WARRANTY!
#(c) 2019 by kitten_nb_five
#freenode #lkv373a
#licence: APGL v3 or later

use strict;
use warnings;
use autodie;

use constant VERSION=>1;

open my $inp, '<', 'dis_block2_5000.txt'; #change this to match your file

my $data;

$data.=$_ while(($_=<$inp>));

close $inp;

#remove everything before actual disassm
$data=~s/^.+?<\.data>://s;

#remove addresses and hexdump of instructions
$data=~s/^\s+[[:xdigit:]]+:\s+(?:[[:xdigit:]]+\s){2,4}\s+//gm;

my $found=0;
my $skipped_lines=0;
	
do
{
	#print "-"x5,"\n";
	#print "\"",substr($data, 0, 500),"...\"";

	$found=0;
	
	#[addr]|=imm (with register and "or")
	if($data=~s/^sethi (?<reg_addr>\$r\d+),#0x(?<addr_h>[[:xdigit:]]+)
ori (?&reg_addr),(?&reg_addr),#0x(?<addr_low>[[:xdigit:]]+)
movi(?:55|) (?<reg_imm>\$r\d+),#0x(?<imm>[[:xdigit:]]+)
lwi (?<reg_read_val>\$r\d+),\[(?&reg_addr)\+#0x(?<offset>[[:xdigit:]]+)\]
or (?&reg_read_val),(?&reg_read_val),(?&reg_imm)
swi (?&reg_read_val),\[(?&reg_addr)\+#0x(?&offset)\]
//)
	{
		$found=1;
		$skipped_lines=0;
		printf("(1) *%08x |= 0x%08x\n", hex($+{addr_h})<<12|hex($+{addr_low})+hex($+{offset}), hex($+{imm}));
	}

	#[addr]=imm
	elsif($data=~s/^sethi (?<reg_addr>\$r\d+),#0x(?<addr_h>[[:xdigit:]]+)
(?:(?:ori (?&reg_addr),(?&reg_addr),#0x(?<addr_low>[[:xdigit:]]+))
|)movi(?:55|) (?<reg_imm>\$r\d+),#0x(?<imm>[[:xdigit:]]+)
swi (?&reg_imm),\[(?&reg_addr)\+#0x(?<offset>[[:xdigit:]]+)\]
//)
	{
		$found=1;
		$skipped_lines=0;
		printf("(2) *%08x = 0x%08x\n", hex($+{addr_h})<<12|hex($+{addr_low}//0)+hex($+{offset}), hex($+{imm}));
	}

	#[addr]&=-imm (imm<0)
	elsif($data=~s/^sethi (?<reg_addr>\$r\d+),#0x(?<addr_h>[[:xdigit:]]+)
ori (?&reg_addr),(?&reg_addr),#0x(?<addr_low>[[:xdigit:]]+)
movi(?:55|) (?<reg_imm>\$r\d+),#-(?<imm>[[:xdigit:]]+)
lwi (?<reg_read_val>\$r\d+),\[(?&reg_addr)\+#0x(?<offset>[[:xdigit:]]+)\]
and (?&reg_read_val),(?&reg_read_val),(?&reg_imm)
swi (?&reg_read_val),\[(?&reg_addr)\+#0x(?&offset)\]
//)
	{
		$found=1;
		$skipped_lines=0;
		printf("(3) *%08x &= 0x%08x\n", hex($+{addr_h})<<12|hex($+{addr_low})+hex($+{offset}), (2<<31)-1-$+{imm});
	}
	
	#[addr]|=imm (with "ori" and immediate)
	elsif($data=~s/^sethi (?<reg_addr>\$r\d+),#0x(?<addr_h>[[:xdigit:]]+)
ori (?&reg_addr),(?&reg_addr),#0x(?<addr_low>[[:xdigit:]]+)
lwi (?<reg_read_val>\$r\d+),\[(?&reg_addr)\+#0x(?<offset>[[:xdigit:]]+)\]
ori (?&reg_read_val),(?&reg_read_val),#0x(?<imm>[[:xdigit:]]+)
swi (?&reg_read_val),\[(?&reg_addr)\+#0x(?&offset)\]
//)
	{
		$found=1;
		$skipped_lines=0;
		printf("(4) *%08x |= 0x%08x\n", hex($+{addr_h})<<12|hex($+{addr_low})+hex($+{offset}), hex($+{imm}));
	}
	
	#no match for instruction(s) at beginn of data stream
	#skip a line (but print it out) and retry
	#if no success after 20 lines stop 
	else
	{
		$skipped_lines++;
		if($skipped_lines<20)
		{
			$data=~s/^(.+?)\n//;
			print "$1\n";
			$found=1;
		}
	}
} while($found);

