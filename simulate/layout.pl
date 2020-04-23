#! /usr/bin/perl

=pod
This file is part of the simulator for the LKV373
(c) 2019-2020 by kitten_nb_five
freenode #lkv373a

licence: AGPL v3 or later
=cut

#THIS IS A HACK AND DOES NOT WORK PROPERLY! might be due to https://github.com/jordansissel/xdotool/issues/176
#TODO: IMPROVE

use strict;
use warnings;
use autodie;

if(scalar(@ARGV)<2)
{
	die "need arguments";
}

my $command=shift(@ARGV);
my $file=shift(@ARGV).".layout";

my @titles=qw/disassembly registers memory stack breakpoints UART/; #keep this synchronized with C code

if($command eq "save")
{
	open(my $out, '>', $file);
	
	foreach my $window (@titles)
	{
		my $result=`xdotool search --name \"$window\" getwindowgeometry`;
		$result=~/Position: (\d+),(\d+)/;
		my ($posx, $posy)=($1, $2);
		$result=~/Geometry: (\d+)x(\d+)/;
		my ($sizex, $sizey)=($1, $2);
		
		$result=`xwininfo -name \"$window\" | grep \"Map State:\"`;
		if($result=~/IsViewable/)
		{
			print $out "$window==windowmove $posx $posy windowsize $sizex $sizey\n";
		}
	}
	
	close($out);
}
elsif ($command eq "load")
{
	open(my $inp, '<', $file);
	my %data;
	while(($_=<$inp>))
	{
		chomp;
		my ($title, $possize)=split(/==/, $_);
		$data{$title}=$possize;
	}
	
	close($inp);
	
	foreach my $window (@titles)
	{
		if($data{$window})
		{
			system("xdotool search --name \"$window\" ".$data{$window}." windowactivate");
		}
	}
}
else
{
	die "unknown command";
}

