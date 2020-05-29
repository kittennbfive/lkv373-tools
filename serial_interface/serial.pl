use strict;
use warnings FATAL=>'all';
use Device::SerialPort;

=pod
simple UI to interface custom firmware of LKV373

written in Perl for easy hackability using regular expressions (parsers in C are a pain in the butt...)

(c) 2020 kitten_nb_five

version 0.01 - early release for weekend

licence: AGPL v3 or later

NO WARRANTY!

commands:
sz,addr=value ->write value to addr using sz byte(s) memory-access (1 or 2 or 4)
sz,addr? ->read value from addr using sz byte(s) memory-access (1 or 2 or 4)
read|file $file ->read commands from file
quit|exit ->exit immediately

Commands in $file can have the syntax specified above or direct output of the simulator (write only at this point, byte-size is calculated from value).
Lines starting with # or // are ignored.
=cut

my $interface='/dev/ttyUSB0';

my $delay_ms=5; #min 5ms

my $port=new Device::SerialPort($interface, undef, undef) || die "can't open $interface";

$port->databits(8);
$port->baudrate(115200);
$port->parity("none");
$port->stopbits(1);
$port->handshake("none");
$port->write_settings;
$port->purge_rx();
$port->purge_tx();

my $run=1;
while($run)
{
	print ">";
	my $line=<STDIN>;
	chomp($line);
	
	if($line eq "quit" || $line eq "exit")
	{
		print "Bye...\n";
		$run=0;
	}
	elsif($line=~/^(?:file|read) (.+)/)
	{
		my $r=open my $fh, '<', $1;
		if(!$r)
		{
			print "can't open file $1!\n";
			next;
		}
		print "reading file $1...\n";
		while(($_=<$fh>))
		{
			chomp;
			next if(/^#/ || m!^//! || /^\s*$/);
			last if(parse($_));
		}
		close $fh;
	}
	else
	{
		parse($line);
	}
}

sub parse
{
	my $l=shift;
	if($l=~/(\d),(?:0x)?([[:xdigit:]]+)=(?:0x)?([[:xdigit:]]+)/)
	{
		my ($sz, $addr, $val)=($1, hex($2), hex($3));
		write_addr($sz, $addr, $val);
	}
	elsif($l=~/\w+: write 0x([[:xdigit:]]+) \@0x([[:xdigit:]]+)/)
	{
		my ($val, $addr)=(hex($1), hex($2));
		my $sz;
		if($val)
		{
			$sz=int(log($val)/log(2));
			$sz+=(8-$sz%8);
			$sz/=8;
			$sz=4 if($sz==3);
		}
		else
		{
			$sz=1;
		}
		write_addr($sz, $addr, $val);
	}
	elsif($l=~/(\d),(?:0x)?([[:xdigit:]]+)\?/)
	{
		my ($sz, $addr)=($1, hex($2));
		read_addr($sz, $addr);
	}
	else
	{
		print "unknown command: \"$l\"\n";
		return 1;
	}
	
		
	return 0;
}

sub sleep_ms
{
	select(undef, undef, undef, shift(@_)/1000);
}

sub read_all_data
{
	my ($read_bytes, $raw, $data, $count);
	
	$count=0;
	
	do
	{
		($read_bytes, $raw)=$port->read(255);
		$data.=$raw;
		if($read_bytes==0)
		{
			$count++;
		}
		
	} while($count<10);
	
	return $data;
}

sub read_addr
{
	my ($size, $addr)=(shift, shift);
	
	my $cmd=pack("CCLL", ord('?'), $size, $addr, 0x01020304);
	
	$port->write($cmd);
	
	sleep_ms($delay_ms);
	
	my $return=read_all_data();
	
	my ($status, $val)=unpack("A2L", $return);
	
	if($status ne "ok")
	{
		die "read_addr: $status\n";
	}
	
	printf("(%d) 0x%08x==0x%x\n", $size, $addr, $val);
	
	return $val;
}

sub write_addr
{
	my ($size, $addr, $val)=(shift, shift, shift);
	
	my $cmd=pack("CCLL", ord('='), $size, $addr, $val);
	
	$port->write($cmd);
	
	sleep_ms($delay_ms);
	
	my $return=read_all_data();
	
	my $status=unpack("A2", $return);
	
	if($status ne "ok")
	{
		die "write_addr: $status\n";
	}
	
	printf("(%d) 0x%x written to 0x%08x\n", $size, $val, $addr);
}
