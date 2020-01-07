#this tool "beautifies" the code even more. use it on output of beautify(1).pl
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

open my $inp, '<', 'disassm_beautified.txt';

my @data;

while(($_=<$inp>))
{
	chomp($_);
	push @data, $_;
}

close $inp;

print "read ",scalar(@data)," lines\n";

#find direct use of UDR -> uart_putchar()
foreach $_ (@data)
{
	$_=~s/\*99600000 = 0x000000([[:xdigit:]]{2})/"uart_putchar(0x$1) \/\/\'".chr(hex($1))."\'"/e;
}


#beautify register loads where sethi and ori are not directly following each other
my $i;
my %data;
for $i (0..$#data)
{
	my $l=$data[$i];
	
	if($l=~/sethi \$(?<reg>r\d+),#0x(?<val_h>[[:xdigit:]]+)/)
	{
		$data{$+{reg}}={ val_h=>$+{val_h}, isethi=>$i };
		#print "sethi found, reg is $+{reg}, val_h is $+{val_h}\n";
	}
	elsif($l=~/(?<addr>[[:xdigit:]]+):\tori \$(?<reg>r\d+),\$\k<reg>,#0x(?<val_l>[[:xdigit:]]+)/)
	{
		#print "ori found, reg is $+{reg}, val_l is $+{val_l}\n";
		if($data{$+{reg}})
		{
			#print "data found!";
			my $reg=$+{reg};
			my $isethi=$data{$reg}->{'isethi'};
			my $val=hex($data{$reg}->{'val_h'})<<12|hex($+{val_l});
			my $addr=hex($+{addr});
			$data[$i]=sprintf("%08x:\t\$%s=0x%08x", $addr, $reg, $val);
			
			$data[$isethi]='('.$data[$isethi].')';
			
			delete $data{$+{reg}};
		}
	}
	elsif($l=~/\$(?<reg>r\d+)/)
	{
		#print "reset for $+{reg}, line is $l\n";
		delete $data{$+{reg}} if($data{$+{reg}});
	}
	
}

undef %data;

#beautify calls to printf 1 (fmt=r1)
my $isvalidr1;
my $straddr;
for $i (0..$#data)
{
	my $l=$data[$i];

	#print "$l\n";

	if($l=~/\$r1=0x(?<addr>[[:xdigit:]]+)/)
	{		
		if($strings[hex($+{addr})])
		{
			#print "string found for this addr\n";
			$isvalidr1=1;
			$straddr=hex($+{addr});
		}
		else
		{
			#print "no string for this addr\n";
		}
	}
	elsif($l=~/\$r1,/)
	{
		#print "reset r1\n";
		$isvalidr1=0;
	}

	if($l=~/\$(?<reg>r\d+)=0x000065f0/)
	{
		$data{$+{reg}}=1;
		#print "addr 1 found, reg is $+{reg}\n";
	}

	elsif($l=~/(?<addr>[[:xdigit:]]+):\tjral(?:5|) (?:\$lp,|)\$(?<reg>r\d+)/)
	{
		#print "jral found, reg is $+{reg}\n";
		if($data{$+{reg}})
		{
			#print "this is a printf-call!\n";
			
			if($isvalidr1)
			{
				#print "r1 has string addr\n";
				
				$data[$i]=sprintf("%08x:\tprintf1(\"%s\")", hex($+{addr}), $strings[$straddr]);
			}
			else
			{
				#print "printf call with invalid r1\n";
				$data[$i].=" //possible printf-call but r1 has no valid string-addr";
			}
			
		}
	}
	elsif($l=~/(?<addr>[[:xdigit:]]+):\tcall 0x000065f0/)
	{
		#print "direct call found\n";
	
		if($isvalidr1)
		{
			#print "r1 has string addr\n";
			
			$data[$i]=sprintf("%08x:\tprintf1(\"%s\")", hex($+{addr}), $strings[$straddr]);
		}
		else
		{
			#print "printf call with invalid r1\n";
			$data[$i].=" //possible printf-call but r1 has no valid string-addr";
		}
	}
	elsif($l=~/\$(?<reg>r\d+)/)
	{
		if($data{$+{reg}})
		{
			#print "reset for $+{reg}, line is $l\n";
			delete $data{$+{reg}};
		}
	}
	
}

undef %data;

#beautify calls to printf 2 (fmt=r2)
$straddr=0;
my $isvalidr2;
for $i (0..$#data)
{
	my $l=$data[$i];

	if($l=~/\$r2=0x(?<addr>[[:xdigit:]]+)/)
	{		
		if($strings[hex($+{addr})])
		{
			#print "string found for this addr\n";
			$isvalidr2=1;
			$straddr=hex($+{addr});
		}
		#else
		#{
		#	print "no string for this addr\n";
		#}
	}
	elsif($l=~/\$r2,/)
	{
		#print "reset r2\n";
		$isvalidr2=0;
	}

	if($l=~/\$(?<reg>r\d+)=0x00006ad8/)
	{
		$data{$+{reg}}=1;
		#print "addr 2 found, reg is $+{reg}\n";
	}
	elsif($l=~/(?<addr>[[:xdigit:]]+):\tjral(?:5|) (?:\$lp,|)\$(?<reg>r\d+)/)
	{
		#print "jral found, reg is $+{reg}\n";
		if($data{$+{reg}})
		{
			#print "this is a printf-call!\n";
			
			if($isvalidr2)
			{
				#print "r2 has string addr\n";
				
				$data[$i]=sprintf("%08x:\tprintf2(\"%s\")", hex($+{addr}), $strings[$straddr]);
			}
			else
			{
				#print "printf call with invalid r2\n";
				$data[$i].=" //possible printf-call but r2 has no valid string-addr";
			}
			
		}
	}
	elsif($l=~/(?<addr>[[:xdigit:]]+):\tcall 0x00006ad8/)
	{
		#print "direct call found\n";
	
		if($isvalidr2)
		{
			#print "r2 has string addr\n";
			
			$data[$i]=sprintf("%08x:\tprintf2(\"%s\")", hex($+{addr}), $strings[$straddr]);
		}
		else
		{
			#print "printf call with invalid r2\n";
			$data[$i].=" //possible printf-call but r2 has no valid string-addr";
		}
	}
	elsif($l=~/\$(?<reg>r\d+)/)
	{
		if($data{$+{reg}})
		{
			#print "reset for $+{reg}, line is $l\n";
			delete $data{$+{reg}};
		}
	}
	
}


open my $out, '>', 'disassm_beautified2.txt';
print $out join("\n", @data);
close $out;
