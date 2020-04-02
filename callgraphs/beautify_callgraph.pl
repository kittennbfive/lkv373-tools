#This is part of a programm for making callgraphs for the firmware of the LKV373
#This programm comes WITHOUT ANY WARRANTY.
#licence: AGPL version 3 or later
#(c) 2019-2020 by kitten_nb_five

use strict;
use warnings;
use autodie;
use File::Slurp;

use known_funcs;

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
