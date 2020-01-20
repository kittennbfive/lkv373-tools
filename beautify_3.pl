#this tool "beautifies" the code even more. use it on output of beautify_2.pl
#really much work in progress
#NO WARRANTY!
#(c) 2020 by kitten_nb_five
#freenode #lkv373a
#licence: APGL v3 or later

use strict;
use warnings;
use autodie;

use constant VERSION=>1;

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

open my $inp, '<', 'disassm_block2_beautified2.txt';

my @data;

while(($_=<$inp>))
{
	chomp($_);
	push @data, $_;
}

close $inp;

print "read ",scalar(@data)," lines\n";

#add string when pointer is loaded to register
foreach $_ (@data)
{
	if($_=~/\$r\d+=0x([[:xdigit:]]+)/ && $strings[hex($1)])
	{
		print "found $1\n";
		$_.=" //\"".substr($strings[hex($1)], 0, 50)."\"";
	}
	elsif($_=~/\$r\d+=0x([[:xdigit:]]+)/ && $strings[hex($1)+0x50000]) #block2
	{
		print "found b2: $1\n";
		$_.=" //[b2] \"".substr($strings[hex($1)+0x50000], 0, 50)."\"";
	}
}

open my $out, '>', 'disassm_block2_beautified3.txt';
print $out join("\n", @data);
close $out;
