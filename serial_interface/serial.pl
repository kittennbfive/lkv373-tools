use strict;
use warnings FATAL=>'all';
use Device::SerialPort;

=pod
simple UI to interface custom firmware of LKV373

written in Perl for easy hackability using regular expressions (parsers in C are a pain in the butt...)

(c) 2020 kitten_nb_five

version 0.03

licence: AGPL v3 or later

NO WARRANTY!

NEEDS FIRMWARE CONNECTOR_2_ ON THE 373!

commands:
sz,addr=value ->write value to addr using sz byte(s) memory-access (1 or 2 or 4)
sz,addr? ->read value from addr using sz byte(s) memory-access (1 or 2 or 4)
q $n $a $b $c $d -> set JPEG quality to $n (1-100, dez) with unknown bytes written as $a-$d (hex)
i2c r $reg -> read register $reg (hex) from ITE-chip
i2c w $reg $val -> write $val (hex) to register $reg (hex) in ITE-chip 
read|file $file ->read commands from file
quit|exit ->exit immediately

Commands in $file must have the same syntax as specified above. Lines starting with # or // are ignored.
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
	elsif($line eq "purge")
	{
		$port->purge_rx();
		$port->purge_tx();
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
			#sleep_ms(150);
		}
		close $fh;
	}
	else
	{
=pod
		#test i2c
		$line=~s/tx/1,0x99c00008/;
		$line=~s/rx/1,0x99c00010/;
		$line=~s/ctrl/1,0x99c0000c/;
		$line=~s/sta/1,0x99c00014/;
=cut
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
=pod
	#this works but is prone to errors, like is 42 a 8 bit or a small 32 bit number?? this might produce strange bugs inside the TF680
	elsif($l=~/\w+: write 0x([[:xdigit:]]+) (?:\@|to )0x([[:xdigit:]]+)/)
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
=cut
	elsif($l=~/(\d),(?:0x)?([[:xdigit:]]+)\?/)
	{
		my ($sz, $addr)=($1, hex($2));
		read_addr($sz, $addr);
	}
	#q 50 e 0 f 0
	#4,0x9090c008=6
	elsif($l=~/q\s+(\d+)\s+(?:0x)?([[:xdigit:]]+)\s+(?:0x)?([[:xdigit:]]+)\s+(?:0x)?([[:xdigit:]]+)\s+(?:0x)?([[:xdigit:]]+)/)
	{
		my ($q, $a, $b, $c, $d)=($1, hex($2), hex($3), hex($4), hex($5));
		
		set_qual($q, $a, $b, $c, $d);

		$port->purge_rx();
	}
	elsif($l=~/i2c r (?:0x)?([[:xdigit:]]+)/)
	{
		i2c('r', hex($1));
	}
	elsif($l=~/i2c w (?:0x)?([[:xdigit:]]+) (?:0x)?([[:xdigit:]]+)/)
	{
		i2c('w', hex($1), hex($2));
	}
	elsif($l eq 'd')
	{
		print read_all_data();
	}
	elsif($l=~/^dump (?:0x)?([[:xdigit:]]+) (\d+)(?:$|\s*>\s*(.+))/)
	{
		hexdump($1, $2, $3);
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
	my ($size, $addr, $silent)=(shift, shift, shift//undef);
	
	return undef if($addr==0x9090a80c); #what on earth is this???
	
	my $cmd=pack("CCLL", ord('?'), $size, $addr, 0x01020304);
	
	$port->write($cmd);
	
	sleep_ms($delay_ms);
	
	my $return=read_all_data();
	
	my ($status, $val)=unpack("A2L", $return);
	
	if($status ne "ok")
	{
		die "read_addr: $status\n";
	}
	
	printf("(%d) 0x%08x==0x%x\n", $size, $addr, $val) if(!$silent);
	
	return $val;
}

sub write_addr
{
	my ($size, $addr, $val)=(shift, shift, shift);
	
	return if($addr==0x9090a80c);
	
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

sub hexdump
{
	my ($addr_start, $sz, $file)=(hex(shift), shift, shift);
	
	my $addr;
	my $cnt=0;

	my $fh;
	
	if($file)
	{
		my $r=open $fh, '>', $file;
		if(!$r)
		{
			print "can't open file $1!\n";
			return;
		}
		print "dumping to file $file...";
	}
	else
	{
		 $fh=*stdout;
	}
			
	for($addr=$addr_start; $addr<$addr_start+$sz;)
	{
		print $fh sprintf("%08x: ", $addr);
		for($cnt=0; $cnt<16 && $addr<$addr_start+$sz; $cnt++, $addr++)
		{
			print $fh sprintf("%02x ", read_addr(1, $addr, 1));
		}
		print $fh "\n";
	}
	
	if($file)
	{
		close $fh;
		print "finished\n";
	}
}

sub set_qual
{
	my ($q, $a, $b, $c, $d)=(shift, shift, shift, shift, shift);
	
	my $cmd=pack("CCCCCCL", ord('Q'), $q, $a, $b, $c, $d, 0);
	
	$port->write($cmd);
	
	sleep_ms(250);
	
	my $return=read_all_data();
=pod
	hacked to see error messages
	my $status=unpack("A2", $return);
	
	if($status ne "ok")
	{
		print "set_qual: \"$return\"\n";
	}
=cut
	print $return;

	print "Q set to $q\n";
}

sub i2c
{
	my ($rw, $reg, $val)=(shift, shift, shift//undef);
	
	if($rw eq 'r')
	{
		my $cmd=pack("CCCCCCL", ord('I'), ord('r'), $reg, 0, 0, 0, 0); #we MUST send 10 bytes!!
		
		$port->write($cmd);
	
		sleep_ms(5);
	
		my $return=read_all_data();

		my ($status, $regval)=unpack("A2C", $return);
		
		if($status ne "ok")
		{
			die "i2c read: \"$return\"\n";
		}
		
		printf("I2C_ITE: reg 0x%x is 0x%x\n", $reg, $regval);
	}
	else
	{
		my $cmd=pack("CCCCCCL", ord('I'), ord('w'), $reg, $val, 0, 0, 0);
		
		$port->write($cmd);
	
		sleep_ms(5);
	
		my $return=read_all_data();

		my $status=unpack("A2", $return);
		
		if($status ne "ok")
		{
			die "i2c write: \"$return\"\n";
		}
		
		printf("I2C_ITE: 0x%x written to 0x%x\n", $val, $reg);
	}
}

		
