#This is part of a programm for making calllists for the firmware of the LKV373
#This programm comes WITHOUT ANY WARRANTY.
#licence: AGPL version 3 or later
#(c) 2019-2020 by kitten_nb_five

use strict;
use warnings;
use autodie;
use File::Slurp;

use known_funcs;

my $hash_ref;

print "beautifying...\n";

my $calls=read_file('call_list.txt');

if ($calls!~s/#file is (.+?)\n//)
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

print scalar(keys(%{$hash_ref}))," functions in database\n";

my $cnt=0;

foreach (keys %{$hash_ref})
{
	if($calls=~/to $_/)
	{
		$calls=~s/to $_/to $_ ($hash_ref->{$_})/g;
		$cnt++;
	}
}

write_file('call_list_beautified.txt', $calls);

print "$cnt known functions found\n";
