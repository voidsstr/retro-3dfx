#!/usr/local/bin/perl

use lib '.\packages';

use h3init;
use pciwrap;
use h3regs;
use h3defs;

h3init::h3initMapBoard;

$debug = 0;

$i2c_enable = BIT(23);
$i2c_sc_wr = BIT(24);
$i2c_sd_wr = BIT(25);
$i2c_sc_rd =  BIT(26);
$i2c_sd_rd =  BIT(27);

$device = 0x40;  # 40 or 42 ??

###
### Initialize
###

# enable i2c
printf("Before enable\n") if ($debug);
&SPOUT( (&SPIN() & ~$i2c_enable) | $i2c_sc_wr | $i2c_sd_wr);  # disable i2c; pull sd and sc high
until ( (&SPIN() & $i2c_enable) == 0 ) {print("Wait for i2c to be disabled\n") if ($debug);}
&SPOUT( &SPIN() | $i2c_enable );  # enable i2c while sd and sc are high
until ( &SPIN() & $i2c_enable ) {print("Wait for i2c to be enabled\n") if ($debug);}

printf("After enable\n") if ($debug);

# stop
&SPOUT( &SPIN() & ~$i2c_sc_wr ); # pull the clk to low before any data transition
until ( (&SPIN() & $i2c_sc_rd) == 0 ) {&SPOUT( &SPIN() | $i2c_sd_wr );} # wait until the clk is low, and then release the data to 1

printf("After 1\n") if ($debug);

while ( (&SPIN() & $i2c_sd_rd) == 0 ) {&ToggleClock(); print("Wait for slave to release sd\n") if ($debug);} # while the data is tied to low by the slave, keep toggling the clk
&SendStop();

printf("After stop\n") if ($debug);

###
### perform i2c operations
###

# Read back VID
#@dmr = &ReadRegs(0x25); # Version Id Register
#printf "Version ID = 0x%x\n",$dmr[0];

# reg 2
&WriteRegs( 0x02, 0xde); # s-video
print("ACK: reg 2 initialized\n") if ($debug);

# reg 3
&WriteRegs( 0x03, 0x23); # analog control 2
print("ACK: reg 3 initialized\n") if ($debug);

# reg 6
&WriteRegs( 0x06, 0xf0); # hsync begins
print("ACK: reg 6 initialized\n") if ($debug);

# reg 7
&WriteRegs( 0x07, 0x30); # hsync ends
print("ACK: reg 7 initialized\n") if ($debug);

# reg 8
&WriteRegs( 0x08, 0x48); # 60 Hz field control
print("ACK: reg 8 initialized\n") if ($debug);

# reg 9
&WriteRegs( 0x09, 0x80); # s-video, no peaking
print("ACK: reg 9 initialized\n") if ($debug);

# reg a
&WriteRegs( 0x0a, 0x80); #
print("ACK: reg a initialized\n") if ($debug);

# reg b
&WriteRegs( 0x0b, 0x47); #
print("ACK: reg b initialized\n") if ($debug);

# reg c
&WriteRegs( 0x0c, 0x40); #
print("ACK: reg c initialized\n") if ($debug);

# reg d
&WriteRegs( 0x0d, 0x00); #
print("ACK: reg d initialized\n") if ($debug);

# reg e
&WriteRegs( 0x0e, 0x01); #
print("ACK: reg e initialized\n") if ($debug);

# reg f
&WriteRegs( 0x0f, 0x2e); #
print("ACK: reg f initialized\n") if ($debug);

# reg 10
&WriteRegs( 0x10, 0x00); #
print("ACK: reg 10 initialized\n") if ($debug);

# reg 11
&WriteRegs( 0x11, 0x85); #
print("ACK: reg 11 initialized\n") if ($debug);

# reg 12
&WriteRegs( 0x12, 0x81); #
print("ACK: reg 12 initialized\n") if ($debug);

# reg 13
&WriteRegs( 0x13, 0x21); #
print("ACK: reg 13 initialized\n") if ($debug);

# reg 14
&WriteRegs( 0x14, 0x00); #
print("ACK: reg 14 initialized\n") if ($debug);

# reg 83
&WriteRegs( 0x83, 0x01); #
print("ACK: reg 83 initialized\n") if ($debug);

# reg 84
&WriteRegs( 0x84, 0x40); #
print("ACK: reg 84 initialized\n") if ($debug);

# reg 86
&WriteRegs( 0x86, 0x40); #
print("ACK: reg 86 initialized\n") if ($debug);

# reg 87
&WriteRegs( 0x87, 0x01); #
print("ACK: reg 87 initialized\n") if ($debug);

# reg 88
&WriteRegs( 0x88, 0xc0); #
print("ACK: reg 88 initialized\n") if ($debug);

# reg 90
&WriteRegs( 0x90, 0x40); #
print("ACK: reg 90 initialized\n") if ($debug);

# reg 91
&WriteRegs( 0x91, 0x08); #
print("ACK: reg 91 initialized\n") if ($debug);

# reg 92
&WriteRegs( 0x92, 0xaa); #
print("ACK: reg 92 initialized\n") if ($debug);

# reg 93
&WriteRegs( 0x93, 0x80); #
print("ACK: reg 93 initialized\n") if ($debug);

# reg 96
&WriteRegs( 0x96, 0xd0); #
print("ACK: reg 96 initialized\n") if ($debug);

# reg 97
&WriteRegs( 0x97, 0x02); #
print("ACK: reg 97 initialized\n") if ($debug);

# reg 98
&WriteRegs( 0x98, 0x0b); #
print("ACK: reg 98 initialized\n") if ($debug);

# reg 99
&WriteRegs( 0x99, 0x00); #
print("ACK: reg 99 initialized\n") if ($debug);

# reg 9a
&WriteRegs( 0x9a, 0xf0); #
print("ACK: reg 9a initialized\n") if ($debug);

# reg 9c
&WriteRegs( 0x9c, 0x80); #
print("ACK: reg 9c initialized\n") if ($debug);

# reg 9d
&WriteRegs( 0x9d, 0x02); #
print("ACK: reg 9d initialized\n") if ($debug);

# reg 9e
&WriteRegs( 0x9e, 0xf0); #
print("ACK: reg 9e initialized\n") if ($debug);

# reg 9f
&WriteRegs( 0x9f, 0x00); #
print("ACK: reg 9f initialized\n") if ($debug);

# reg a0
&WriteRegs( 0xa0, 0x01); #
print("ACK: reg a0 initialized\n") if ($debug);

# reg a1
&WriteRegs( 0xa1, 0x00); #
print("ACK: reg a1 initialized\n") if ($debug);

# reg a2
&WriteRegs( 0xa2, 0x00); #
print("ACK: reg a2 initialized\n") if ($debug);

# reg a4
&WriteRegs( 0xa4, 0x80); #
print("ACK: reg a4 initialized\n") if ($debug);

# reg a5
&WriteRegs( 0xa5, 0x40); #
print("ACK: reg a5 initialized\n") if ($debug);

# reg a6
&WriteRegs( 0xa6, 0x40); #
print("ACK: reg a6 initialized\n") if ($debug);

# reg a8
&WriteRegs( 0xa8, 0x80); #
print("ACK: reg a8 initialized\n") if ($debug);

# reg a9
&WriteRegs( 0xa9, 0x04); #
print("ACK: reg a9 initialized\n") if ($debug);

# reg ac
&WriteRegs( 0xac, 0x40); #
print("ACK: reg ac initialized\n") if ($debug);

# reg ad
&WriteRegs( 0xad, 0x02); #
print("ACK: reg ad initialized\n") if ($debug);

# reg b0
&WriteRegs( 0xb0, 0x04); #
print("ACK: reg b0 initialized\n") if ($debug);

# reg b1
&WriteRegs( 0xb1, 0x04); #
print("ACK: reg b1 initialized\n") if ($debug);

# reg b2
&WriteRegs( 0xb2, 0x04); #
print("ACK: reg b2 initialized\n") if ($debug);

# reg b3
&WriteRegs( 0xb3, 0x04); #
print("ACK: reg b3 initialized\n") if ($debug);

# reg b4
&WriteRegs( 0xb4, 0x01); #
print("ACK: reg b4 initialized\n") if ($debug);


# reg b8
&WriteRegs( 0xb8, 0x20); #
print("ACK: reg b8 initialized\n") if ($debug);

# reg b9
&WriteRegs( 0xb9, 0x10); #
print("ACK: reg b9 initialized\n") if ($debug);

# reg ba
&WriteRegs( 0xba, 0x10); #
print("ACK: reg ba initialized\n") if ($debug);

# reg bb
&WriteRegs( 0xbb, 0x00); #
print("ACK: reg bb initialized\n") if ($debug);

# reg bc
&WriteRegs( 0xbc, 0x20); #
print("ACK: reg bc initialized\n") if ($debug);

# reg bd
&WriteRegs( 0xbd, 0x10); #
print("ACK: reg bd initialized\n") if ($debug);

# reg be
&WriteRegs( 0xbe, 0x10); #
print("ACK: reg be initialized\n") if ($debug);

# reg bf
&WriteRegs( 0xbf, 0x00); #
print("ACK: reg bf initialized\n") if ($debug);

# reg 80
&WriteRegs( 0x80, 0x90); #
print("ACK: reg 80 initialized\n") if ($debug);


for ($q=0; $q<=0x09; $q++) {
    @dd = &ReadRegs($q);
    printf "reg[0x%x] = 0x%x\n",$q,$dd[0];
    ($dd) = &ReadRegs($q);
    printf "reg[0x%x] = 0x%x\n",$q,$dd;
}

for ($q=0xa0; $q<=0xbf; $q++) {
    @dd = &ReadRegs($q);
    printf "reg[0x%x] = 0x%x\n",$q,$dd[0];
    ($dd) = &ReadRegs($q);
    printf "reg[0x%x] = 0x%x\n",$q,$dd;
}

###
### finished
###

print "\n";
print "*** i2c initialization completed ***\n";

################################################################################
################################################################################
################################################################################
###                                                                          ###
###                      i2c library of operations                           ###
###                                                                          ###
################################################################################
################################################################################
################################################################################

###
### read one or more registers 
###
### @vals = &ReadRegs(@addrs)
###

sub ReadRegs {
    my @addrs = @_;
    my @data;
    my $n = @_;
    my $a;

    foreach $a ( @_ ) {

	printf "ReadRegs: reading reg[0x%x]\n",$a if ( $debug );
	
	&SendStart;
	&SendByte( $device );  # device + W
	&SendByte( $a & ~BIT(6) );        # send address, clear AutoInc bit
	
	&SendStart;
	&SendByte( $device | 0x1 );  # device + R
	$d = &RecvByte(1);                # read data (last in burst)

	printf "ReadRegs: reg[0x%x] = 0x%x\n",$a,$d if ( $debug );
	push @data, $d;

	&SendStop;
    }

    print "ReadRegs: reads complete\n" if ($debug);

    return @data;
}

###
### write one or more addr/data pairs
###
### &WriteRegs($addr1,$data1,$addr2,$data2,...)
###

sub WriteRegs {
    my (@args) = @_;
    my ($a,$d);

    die "WriteRegs: odd number of arguments\n" if ( @args % 2 == 1 );

    &SendStart;
    &SendByte( $device );  # device + W

    while (@_) {
	$a = shift @_;
	$d = shift @_;

	$a &= ~BIT(6);   # clear AutoInc bit

	printf "WriteRegs: writing reg[0x%x] = 0x%x\n",$a,$d if ($debug);
	&SendByte( $a );
	&SendByte( $d );
	printf "WriteRegs: wrote reg[0x%x] = 0x%x\n",$a,$d if ($debug);
    }

    &SendStop;

    print "WriteRegs: writes complete\n" if ($debug);
}

###
### print state of vidSerialParallelPort register
###
### &PrintReg($portVal)
###

sub PrintReg {
    my($v) = @_;
    
    printf "i2c %s    write sc,sd=%x,%x     read sc,sd=%x,%x\n",
    $v & $i2c_enable ? "enabled" : "disabled",
    $v & $i2c_sc_wr ? 1 : 0,
    $v & $i2c_sd_wr ? 1 : 0,
    $v & $i2c_sc_rd ? 1 : 0,
    $v & $i2c_sd_rd ? 1 : 0  if ($debug);
}

###
### send a byte of data over i2c
###
### &SendByte($byte)
###

sub SendByte {
    my ($byte) = @_;
    my ($i, $bit);
    
    for ($i=7; $i>=0; $i=$i-1) {
	$bit = ($byte >> $i) & 0x1;

	print "SendByte: writing bit $i = $bit\n" if ($debug);
	
	if ( $bit ) {
	    &SPOUT(&SPIN() | $i2c_sd_wr);  # pull sd high when sc is low to transfer a 1
	    until ( &SPIN() & $i2c_sd_rd ) { print "SendByte: waiting for sc to go high\n"; }
	} else {
	    &SPOUT(&SPIN() & ~$i2c_sd_wr); # pull sd low when sc is low to transfer a 0
	    until ( (&SPIN() & $i2c_sd_rd) == 0 ) { print "SendByte: waiting for sd to go high\n"; }
	}
	
	&ToggleClock();

	print "SendByte: wrote bit $i = $bit\n" if ($debug);
    }
	
    print "SendByte: before ack\n" if ($debug);
    &WaitForAck();
    print "SendByte: after ack\n" if ($debug);
    
}

###
### receive a byte of data over i2c
###
### $byte = &RecvByte($lastOne)
###
### where $lastOne selects whether the master issues an ACK
###

sub RecvByte {
    my ($lastOne) = @_;
    my $byte, $bit, $i;
    
    $byte = 0; 

    for ($i=7; $i>=0; $i=$i-1) {

	print "RecvByte: reading bit $i\n" if ($debug);
	
	$bit = &ToggleClock();
	$byte |= $bit << $i;
	
	print "RecvByte: read bit $i = $bit\n" if ($debug);
	
    }

    print "RecvByte: before ack\n" if ($debug);
    if ( $lastOne ) {
	# master does not ack on last read
	&SPOUT(&SPIN() | $i2c_sd_wr);  # pull sd high
	until ( &SPIN() & $i2c_sd_rd ) {}
	&ToggleClock();
    } else {
	# master does ack on intermediate reads
	&SPOUT(&SPIN() & ~$i2c_sd_wr); # pull sd low
	until ( (&SPIN() & $i2c_sd_rd) == 0 ) { }
	&ToggleClock();
    }
    print "RecvByte: before ack\n" if ($debug);
    
    return $byte;
    
}

###
### wait for ACK bit, returns value of ACK
###
### $ack = &WaitForAck()
###

sub WaitForAck {
    local $ack;

    &SPOUT(&SPIN() | $i2c_sd_wr);  # release sd
    $ack = ! &ToggleClock();
    
    die "WaitForAck: slave failed to acknowledge\n" if ( ! $ack );
    
    return $ack;
}

###
### send the stop condition
###
### &SendStop()
###

sub SendStop {

    die "SendStop: clock already high\n" if ( &SPIN() & $i2c_sc_rd );
 	
    &SPOUT(&SPIN() & ~$i2c_sd_wr); # pull sd low when sc is low
    until ( (&SPIN() & $i2c_sd_rd) == 0 ) { }
   
    &SPOUT(&SPIN() | $i2c_sc_wr); # pull sc high
    until ( &SPIN() & $i2c_sc_rd ) { }
   
    &SPOUT(&SPIN() | $i2c_sd_wr); # pull sd high when sc is high
    until ( &SPIN() & $i2c_sd_rd ) { }
   
}
    
###
### send the start condition
###
### &SendStart()
###

sub SendStart {

    die "SendStart: clock high but data low!\n" if ( (&SPIN() & $i2c_sc_rd) && ((&SPIN() & $i2c_sd_rd) == 0 ));

    if ( (&SPIN() & $i2c_sc_rd) == 0 ) {

	&SPOUT(&SPIN() | $i2c_sd_wr); # pull sd high when sc is low
	until ( &SPIN() & $i2c_sd_rd ) { }
	
	&SPOUT(&SPIN() | $i2c_sc_wr); # pull sc high
	until ( &SPIN() & $i2c_sc_rd ) { }
   
    }

    &SPOUT(&SPIN() & ~$i2c_sd_wr); # pull sd low when sc is high
    until ( (&SPIN() & $i2c_sd_rd) == 0 ) { }
   
    &SPOUT(&SPIN() & ~$i2c_sc_wr); # pull sc low
    until ( (&SPIN() & $i2c_sc_rd) == 0 ) { }
   
   
}

###
### advance the clock one cycle
###
### &toggleClock()
###
    
sub ToggleClock {   
    local $data;  # data value when clock is high

    die "ToggleClock: clock already high\n" if ( &SPIN() & $i2c_sc_rd );

    &SPOUT(&SPIN() | $i2c_sc_wr); # release sc
    until ( &SPIN() & $i2c_sc_rd ) {} # wait for sc to go hi

    $data = &SPIN() & $i2c_sd_rd ? 1 : 0;

    &SPOUT(&SPIN() & ~$i2c_sc_wr);  # pull sc low
    until ( (&SPIN() & $i2c_sc_rd) == 0 ) {}

    return $data;
}

###
### write the contents of the vidSerialParallelPort register
###
### &SPOUT($val)
###

sub SPOUT {
    my ($d) = @_;
    if ( $debug ) {
	print "--> writing ";
	&PrintReg($d);
#	getc;
    }
    select undef, undef, undef, 0.1;      # sleep for 0.1 seconds
    OUT32("VIDSERIALPARALLELPORT", $d);
}

###
### read the contents of the vidSerialParallelPort register
###
### $val = &SPIN()
###

sub SPIN {
    my $v;
    select undef, undef, undef, 0.1;      # sleep for 0.1 seconds
    $v = IN32("VIDSERIALPARALLELPORT");
    &PrintReg($v) if ( $debug );
    return $v;
}




