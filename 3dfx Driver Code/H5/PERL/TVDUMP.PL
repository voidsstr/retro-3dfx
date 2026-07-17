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

$device = 0xec;  # 11101100   lsb is zero where R/Wn value is stored

# stop
#SPOUT(SPIN() & ~$i2c_sc_wr);  # pull sc low
#until ( (SPIN() & $i2c_sc_rd) == 0 ) {}
&SPOUT( &SPIN() | $i2c_sd_wr );
&SendStop() if ( (&SPIN() & $i2c_sc_rd) == 0 );

# enable i2c
printf("Before enable\n") if ($debug);
&SPOUT( (&SPIN() & ~$i2c_enable) | $i2c_sc_wr | $i2c_sd_wr);  # disable i2c; pull sd and sc high
until ( (&SPIN() & $i2c_enable) == 0 ) {print("Wait for i2c to be disabled\n") if ($debug);}
until ( &SPIN() & $i2c_sc_rd ) {print("Wait for sc to go high\n") if ($debug);}
until ( &SPIN() & $i2c_sd_rd ) {print("Wait for sd to go high\n") if ($debug);}


&SPOUT( &SPIN() | $i2c_enable );  # enable i2c while sd and sc are high
until ( &SPIN() & $i2c_enable ) {print("Wait for i2c to be enabled\n") if ($debug);}

printf("After enable\n") if ($debug);
# IDF
if ( $dowrites ) {
    $dd=0xff;
    for ($q=0; $q<=0x18; $q++) {
	&WriteRegs($q,$dd);
	printf "writing reg[0x%x] = 0x%x\n",$q,$dd;
    }
}

for ($q=0; $q<=0x25; $q++) {
    @dd = &ReadRegs($q);
    printf "reg[0x%x] = 0x%x\n",$q,$dd[0];
}
$q = 0x25;
@dd = &ReadRegs($q);
printf "reg[0x%x] = 0x%x\n",$q,$dd[0];
exit;

# Read back DMR
@dmr = &ReadRegs(0x00); # Version Id Register
printf "DMR = 0x%x\n",$dmr[0];

# DMR
&WriteRegs( 0x00, 0x6a); #01101010
print("ACK: DMR init completed\n") if ($debug);

# FFR
&WriteRegs( 0x01, 0x01); #00000001, moderate flicker filter
print("ACK: FFR init completed\n") if ($debug);

# VBW
&WriteRegs( 0x03, 0x00); #00000000, low bandwidth
print("ACK: VBW init completed\n") if ($debug);

# IDF
&WriteRegs( 0x04, 0x05); # 12-bit multipliexed "I" scheme
print("ACK: IDF init completed\n") if ($debug);

# CM
&WriteRegs( 0x06, 0x00); # slave, neg edge, 1x clock
print("ACK: CM init completed\n") if ($debug);

# SAV
&WriteRegs( 0x07, 0xe4); # ~200 blank + 28 pipe cycles
print("ACK: SAV init completed\n") if ($debug);

# PO
&WriteRegs( 0x08, 0x00); # no overflow for SAV, HP, and VP
print("ACK: PO init completed\n") if ($debug);

# BLR
&WriteRegs( 0x09, 0x7f); # black level = 127
print("ACK: BLR init completed\n")  if ($debug);

# HCR
&WriteRegs( 0x0a, 0x00); # 0 horizontal shift
print("ACK: HCR init completed\n") if ($debug);

# VPR
&WriteRegs( 0x0b, 0x00); # 0 vertical shift
print("ACK: VPR init completed\n") if ($debug);

# SR
&WriteRegs( 0x0d, 0x00); # active low syncs
print("ACK: SR init completed\n") if ($debug);

# PMR
&WriteRegs( 0x0e, 0x03); # remove reset
print("ACK: PMR init completed\n") if ($debug);

print "\n";
print "*** i2c initialization completed ***\n";

# Read back VID
$vid = &ReadRegs(0x25); # Version Id Register
printf "VID = 0x%x\n",$vid;

# Read back BLR
$blr = &ReadRegs(0x09); # Black level
printf "BLR = 0x%x\n",$blr;

# Read back CIV
$civ1 = &ReadRegs(0x22); # CIV
printf "CIV22 = 0x%x\n",$civ1;

# Read back CIV
$civ2 = &ReadRegs(0x23); # CIV
printf "CIV23 = 0x%x\n",$civ2;

# Read back CIV
$civ3 = &ReadRegs(0x024); # CIV
printf "CIV24 = 0x%x\n",$civ3;

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

sub PrintReg {
    my($v) = @_;
    
    printf "i2c %s    write sc,sd=%x,%x     read sc,sd=%x,%x\n",
    $v & $i2c_enable ? "enabled" : "disabled",
    $v & $i2c_sc_wr ? 1 : 0,
    $v & $i2c_sd_wr ? 1 : 0,
    $v & $i2c_sc_rd ? 1 : 0,
    $v & $i2c_sd_rd ? 1 : 0  if ($debug);
}

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

sub WaitForAck {
    local $ack;

    &SPOUT(&SPIN() | $i2c_sd_wr);  # release sd
    $ack = ! &ToggleClock();
    
    die "WaitForAck: slave failed to acknowledge\n" if ( ! $ack );
    
    return $ack;
}

sub SendStop {

    die "SendStop: clock already high\n" if ( &SPIN() & $i2c_sc_rd );
 	
    &SPOUT(&SPIN() & ~$i2c_sd_wr); # pull sd low when sc is low
    until ( (&SPIN() & $i2c_sd_rd) == 0 ) { }
   
    &SPOUT(&SPIN() | $i2c_sc_wr); # pull sc high
    until ( &SPIN() & $i2c_sc_rd ) { }
   
    &SPOUT(&SPIN() | $i2c_sd_wr); # pull sd high when sc is high
    until ( &SPIN() & $i2c_sd_rd ) { }
   
}
    
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

sub SPIN {
    my $v;
    select undef, undef, undef, 0.1;      # sleep for 0.1 seconds
    $v = IN32("VIDSERIALPARALLELPORT");
    &PrintReg($v) if ( $debug );
    return $v;
}




