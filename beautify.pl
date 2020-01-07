#this tool "beautifies" the output of the objdump -D by transforming memory-writes to single lines of pseudo-C-code
#really much work in progress
#NO WARRANTY!
#(c) 2019 by kitten_nb_five
#freenode #lkv373a
#licence: APGL v3 or later

use strict;
use warnings;
use autodie;

use constant VERSION=>3;
use constant MAX_LINES=>20;
my $do_not_stop=1; #do not stop if no match for more than MAX_LINES lines

#####read stringfile
my @strings; #addr->str

open my $strf, '<', 'strings_all.txt';
while(($_=<$strf>))
{
	if(/^(?<addr>[[:xdigit:]]+): \d+: (?<str>.+)$/)
	{
		$strings[hex($+{addr})]=$+{str};
	}
}
close $strf;

####

open my $inp, '<', 'disassm.txt';

my $data;

$data.=$_ while(($_=<$inp>));

close $inp;

#remove everything before actual disassm
$data=~s/^.+?<\.data>://s;

#remove hexdump of instructions
$data=~s/^\s+([[:xdigit:]]+):\s+(?:[[:xdigit:]]+\s){2,4}\s+/$1:\t/gm;

#objump replaces several NOP with ..., remove these lines
$data=~s/^\s+\.{3}\n//gm;

my $found=0;
my $skipped_lines=0;
	
do
{
	$found=0;
	
	#[addr]|=imm (with register and "or")
	if($data=~s/^(?<addr>[[:xdigit:]]+):\tsethi (?<reg_addr>\$r\d+),#0x(?<addr_h>[[:xdigit:]]+)
[[:xdigit:]]+:\tori \k<reg_addr>,\k<reg_addr>,#0x(?<addr_low>[[:xdigit:]]+)
[[:xdigit:]]+:\tmovi(?:55|) (?<reg_imm>\$r\d+),#0x(?<imm>[[:xdigit:]]+)
[[:xdigit:]]+:\tlwi (?<reg_read_val>\$r\d+),\[\k<reg_addr>\+#0x(?<offset>[[:xdigit:]]+)\]
[[:xdigit:]]+:\tor \k<reg_read_val>,\k<reg_read_val>,\k<reg_imm>
[[:xdigit:]]+:\tswi \k<reg_read_val>,\[\k<reg_addr>\+#0x\k<offset>\]
//)
	{
		$found=1;
		$skipped_lines=0;
		printf("%08x: *%08x |= 0x%08x\n", hex($+{addr}), hex($+{addr_h})<<12|hex($+{addr_low})+hex($+{offset}), hex($+{imm}));
	}

	#[addr]=imm
	elsif($data=~s/^(?<addr>[[:xdigit:]]+):\tsethi (?<reg_addr>\$r\d+),#0x(?<addr_h>[[:xdigit:]]+)
(?:(?:[[:xdigit:]]+:\tori \k<reg_addr>,\k<reg_addr>,#0x(?<addr_low>[[:xdigit:]]+))
|)[[:xdigit:]]+:\tmovi(?:55|) (?<reg_imm>\$r\d+),#0x(?<imm>[[:xdigit:]]+)
[[:xdigit:]]+:\tswi \k<reg_imm>,\[\k<reg_addr>\+#0x(?<offset>[[:xdigit:]]+)\]
//)
	{
		$found=1;
		$skipped_lines=0;
		printf("%08x: *%08x = 0x%08x\n", hex($+{addr}), hex($+{addr_h})<<12|hex($+{addr_low}//0)+hex($+{offset}), hex($+{imm}));
	}

	#[addr]&=-imm (imm<0)
	elsif($data=~s/^(?<addr>[[:xdigit:]]+):\tsethi (?<reg_addr>\$r\d+),#0x(?<addr_h>[[:xdigit:]]+)
[[:xdigit:]]+:\tori \k<reg_addr>,\k<reg_addr>,#0x(?<addr_low>[[:xdigit:]]+)
[[:xdigit:]]+:\tmovi(?:55|) (?<reg_imm>\$r\d+),#-(?<imm>[[:xdigit:]]+)
[[:xdigit:]]+:\tlwi (?<reg_read_val>\$r\d+),\[\k<reg_addr>\+#0x(?<offset>[[:xdigit:]]+)\]
[[:xdigit:]]+:\tand \k<reg_read_val>,\k<reg_read_val>,\k<reg_imm>
[[:xdigit:]]+:\tswi \k<reg_read_val>,\[\k<reg_addr>\+#0x\k<offset>\]
//)
	{
		$found=1;
		$skipped_lines=0;
		printf("%08x: *%08x &= 0x%08x\n", hex($+{addr}), hex($+{addr_h})<<12|hex($+{addr_low})+hex($+{offset}), (2<<31)-1-$+{imm});
	}
	
	#[addr]|=imm (with "ori" and immediate)
	elsif($data=~s/^(?<addr>[[:xdigit:]]+):\tsethi (?<reg_addr>\$r\d+),#0x(?<addr_h>[[:xdigit:]]+)
[[:xdigit:]]+:\tori \k<reg_addr>,\k<reg_addr>,#0x(?<addr_low>[[:xdigit:]]+)
[[:xdigit:]]+:\tlwi (?<reg_read_val>\$r\d+),\[\k<reg_addr>\+#0x(?<offset>[[:xdigit:]]+)\]
[[:xdigit:]]+:\tori \k<reg_read_val>,\k<reg_read_val>,#0x(?<imm>[[:xdigit:]]+)
[[:xdigit:]]+:\tswi \k<reg_read_val>,\[\k<reg_addr>\+#0x\k<offset>\]
//)
	{
		$found=1;
		$skipped_lines=0;
		printf("%08x: *%08x |= 0x%08x\n", hex($+{addr}), hex($+{addr_h})<<12|hex($+{addr_low})+hex($+{offset}), hex($+{imm}));
	}
	
	#delay-loop using increment
	elsif($data=~s/^(?<addr>[[:xdigit:]]+):\tmovi(?:55|) (?<reg_cmp>\$r\d+),#0x(?<imm_cmp>[[:xdigit:]]+)
[[:xdigit:]]+:\tmovi(?:55|) (?<reg_cnt>\$r\d+),#0x0
(?<addr_addi>[[:xdigit:]]+):\taddi \k<reg_cnt>,\k<reg_cnt>,#0x1
[[:xdigit:]]+:\tbne \k<reg_cmp>,\k<reg_cnt>,0x\k<addr_addi>
//)
	{
		$found=1;
		$skipped_lines=0;
		printf("%08x: some_delay_inc(%u)\n", hex($+{addr}), hex($+{imm_cmp}));
	}
	
	#delay-loop using decrement
	elsif($data=~s/^(?<addr>[[:xdigit:]]+):\tmovi(?:55|) (?<reg_cnt>\$r\d+),#0x(?<imm_cmp>[[:xdigit:]]+)
(?<addr_addi>[[:xdigit:]]+):\taddi \k<reg_cnt>,\k<reg_cnt>,#-1
[[:xdigit:]]+:\tbnez \k<reg_cnt>,0x\k<addr_addi>
//)
	{
		$found=1;
		$skipped_lines=0;
		printf("%08x: some_delay_decr(%u)\n", hex($+{addr}), hex($+{imm_cmp}));
	}
	
	#printf()
	elsif($data=~s/^(?<addr>[[:xdigit:]]+):\tsethi \$r1,#0x(?<addr_h>[[:xdigit:]]+)
[[:xdigit:]]+:\tmovi \$r0,#0xfa
[[:xdigit:]]+:\tori \$r1,\$r1,#0x(?<addr_l>[[:xdigit:]]+)
[[:xdigit:]]+:\tsethi (?<reg_addr>\$r\d+),#0x6
[[:xdigit:]]+:\tori \k<reg_addr>,\k<reg_addr>,#0x5f0
[[:xdigit:]]+:\tjral(?:5|) (?:\$lp,|)\k<reg_addr>
//)
	{
		$found=1;
		$skipped_lines=0;
		my $str_addr=hex($+{addr_h})<<12|hex($+{addr_l});
		printf("%08x: printf(\"%s\") //str_addr is 0x%08x\n", hex($+{addr}), substr($strings[$str_addr], 0, 50), $str_addr);
	}

	#wait for UART ready
	elsif($data=~s/^(?<addr>[[:xdigit:]]+):\tsethi (?<reg_addr>\$r\d+),#0x99600
[[:xdigit:]]+:\tori \k<reg_addr>,\k<reg_addr>,#0x8
(?<addr_jmp>[[:xdigit:]]+):\tlwi(?:450|) (?<reg_status>\$r\d+),\[\k<reg_addr>\]
[[:xdigit:]]+:\tandi (?<reg_dst>\$r\d+),\k<reg_status>,#0x10
[[:xdigit:]]+:\tbnez(?:38|) \k<reg_dst>,0x\k<addr_jmp>
//)
	{
		$found=1;
		$skipped_lines=0;
		printf("%08x: wait_until_uart_ready()\n", hex($+{addr}));
	}
	
	#uart_putchar
	elsif($data=~s/^(?<addr>[[:xdigit:]]+):\tsethi (?<reg_addr>\$r\d+),#0x99600
[[:xdigit:]]+:\tmovi (?<reg_char>\$r\d+),#0x(?<char>[[:xdigit:]]+)
[[:xdigit:]]+:\tswi(?:450|) \k<reg_char>,\[\k<reg_addr>\]
//)
	{
		$found=1;
		$skipped_lines=0;
		my $c=hex($+{char});
		printf("%08x: uart_putchar(0x%x) //('%c')\n", hex($+{addr}), $c, $c);
	}
	
	#jump using register
	elsif($data=~s/^(?<addr>[[:xdigit:]]+):\tsethi (?<reg_addr>\$r\d+),#0x(?<addr_h>[[:xdigit:]]+)
[[:xdigit:]]+:\tori \k<reg_addr>,\k<reg_addr>,#0x(?<addr_low>[[:xdigit:]]+)
[[:xdigit:]]+:\tjr(?:5|) \k<reg_addr>
//)
	{
		$found=1;
		$skipped_lines=0;
		printf("%08x: goto 0x%08x\n", hex($+{addr}), hex($+{addr_h})<<12|hex($+{addr_low}));
	}
	
	#jump and link using register
	elsif($data=~s/^(?<addr>[[:xdigit:]]+):\tsethi (?<reg_addr>\$r\d+),#0x(?<addr_h>[[:xdigit:]]+)
[[:xdigit:]]+:\tori \k<reg_addr>,\k<reg_addr>,#0x(?<addr_low>[[:xdigit:]]+)
[[:xdigit:]]+:\tjral (?:\$lp,|)\k<reg_addr>
//)
	{
		$found=1;
		$skipped_lines=0;
		printf("%08x: call 0x%08x\n", hex($+{addr}), hex($+{addr_h})<<12|hex($+{addr_low}));
	}
	
	#jump directly
	elsif($data=~s/^(?<addr>[[:xdigit:]]+):\tj 0x(?<addr_dest>[[:xdigit:]]+)
//)
	{
		$found=1;
		$skipped_lines=0;
		printf("%08x: goto 0x%08x\n", hex($+{addr}), hex($+{addr_dest}));
	}
	
	#jump and link directly 
	elsif($data=~s/^(?<addr>[[:xdigit:]]+):\tjal 0x(?<addr_dest>[[:xdigit:]]+)
//)
	{
		$found=1;
		$skipped_lines=0;
		printf("%08x: call 0x%08x\n", hex($+{addr}), hex($+{addr_dest}));
	}
	
	#load to register (possibly with offset)
	elsif($data=~s/^(?<addr>[[:xdigit:]]+):\tsethi (?<reg_addr>\$r\d+),#0x(?<addr_h>[[:xdigit:]]+)
[[:xdigit:]]+:\tori \k<reg_addr>,\k<reg_addr>,#0x(?<addr_low>[[:xdigit:]]+)
[[:xdigit:]]+:\tlwi (?<reg_dest>\$r\d+),\[\k<reg_addr>\+#0x(?<offset>[[:xdigit:]]+)\]
//)
	{
		$found=1;
		$skipped_lines=0;
		printf("%08x: %s=*0x%08x\n", hex($+{addr}), $+{reg_dest}, hex($+{addr_h})<<12|hex($+{addr_low})+hex($+{offset}));
	}
	
	#store from register (possibly with offset)
	#TODO
	
	
	
	#set register
	elsif($data=~s/^(?<addr>[[:xdigit:]]+):\tsethi (?<reg>\$r\d+),#0x(?<imm_h>[[:xdigit:]]+)
[[:xdigit:]]+:\tori \k<reg>,\k<reg>,#0x(?<imm_low>[[:xdigit:]]+)
//)
	{
		$found=1;
		$skipped_lines=0;
		printf("%08x: %s=0x%08x\n", hex($+{addr}), $+{reg}, hex($+{imm_h})<<12|hex($+{imm_low}));
	}
	
=pod
	elsif($data=~s/^(?<addr>[[:xdigit:]]+):\t
[[:xdigit:]]+:
//)
	{
		$found=1;
		$skipped_lines=0;
		printf("%08x: \n", hex($+{addr}), );
	}
=cut

	#no match for instruction(s) at beginn of data stream
	#skip a line (but print it out) and retry
	#if no success after MAX_LINES lines stop 
	else
	{
		$skipped_lines++;
		if($skipped_lines<MAX_LINES || $do_not_stop)
		{
			$data=~s/^(.+?)\n//;
			print "$1\n";
			$found=1;
		}
	}
} while($found && length($data)>10); #yes, the length-thing is a hack but it works...

