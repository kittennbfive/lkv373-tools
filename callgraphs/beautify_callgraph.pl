#This is part of a programm for making callgraphs for the firmware of the LKV373
#This programm comes WITHOUT ANY WARRANTY.
#licence: AGPL version 3 or later
#(c) 2019-2020 by kitten_nb_five

use strict;
use warnings;
use autodie;
use File::Slurp;

#block 1
my %known_funcs_bl1=(
	'65f0'=>'printf()',
	'6ad8'=>'printf2()',
	'156b0'=>'helper_printf_strcpy??',
	'344c0'=>'helper_printf',
	'3484c'=>'helper_printf',
	'2cf50'=>'uart_putchar()',
	'6400'=>'console_main?',
	'35668'=>'programm_memory()?',
	'14df8'=>'putstr()?',
	'14edc'=>'some_output_func()',
	'1595c'=>'strlen(r0)?',
	'70cc'=>'post_http()?',
	'15754'=>'strcmp()',
	'730c'=>'httpd_respond_to_request',
	'5c24'=>'trigger_reboot()',
	'5ef0'=>'bootloader?'
);

#block 2
my %known_funcs_bl2=(
	'1b018'=>'os_create_task(r1=callback)',
	'7bf4'=>'some_init_register_main()',
	'de0'=>'call_init_and_stop()',
	'39ddc'=>'enable_interrupts()',
	'3a944'=>'wait_for_uart_ready()',
	'75c0'=>'TF_MAIN_Task',
	'6564'=>'TF_AV_Task',
	'cbd8'=>'TF_Console_Task',
	'ee54'=>'TF_HTTP_Task',
	'7330'=>'TF_TFVEP_Task',
	'cdc0'=>'some_output_func()?',
	'324f8'=>'uart_putchar()',
	'36f4'=>'create_pipe()??',
	'197e4'=>'print_console_header()'
);

my $hash_ref;

print "beautifying callgraph...\n";

my $calls=read_file('calls.txt');

if ($calls!~s/#file is (.+)//)
{
	print "error: couldn't determine name of image file\n";
	exit(1);
}

if($1 eq "block1.bin")
{
	print "using data for block1\n";
	$hash_ref=\%known_funcs_bl1;
}
elsif($1 eq "block2.bin")
{
	print "using data for block2\n";
	$hash_ref=\%known_funcs_bl2;
}
else
{
	print "couldn't determine block from image file $1\n";
	exit(1);
}

$calls=~s/digraph calls \{//;
$calls=~s/\}//;

my $calls_beautified="digraph calls {\n";

my $cnt=0;

foreach (keys %{$hash_ref})
{
	if($calls=~/func_$_[\s;]/)
	{
		$calls_beautified.="\tfunc_$_ [label=\"$hash_ref->{$_}\\n(func_$_)\"]\n";
		$cnt++;
	}
}

$calls_beautified.=$calls;

$calls_beautified.="}\n";

write_file('calls_beautified.txt', $calls_beautified);

print "$cnt known functions found\n";
