#!/usr/local/bin/perl

use lib '.\packages';

use h3init;
use pciwrap;
use h3regs;
use h3defs;

h3init::h3initMapBoard;

OUT32("VIDINFORMAT", (IN32("VIDINFORMAT") | BIT(8) | BIT(6) | BIT(5))); # broktree config and active low syncs
OUT32("VIDINFORMAT", (IN32("VIDINFORMAT") & ~BIT(16) & ~BIT(17))); # clear genlock
OUT32("VIDINFORMAT", (IN32("VIDINFORMAT") | BIT(16) | BIT(17))); # set genlock
#OUT32("VIDINFORMAT", (IN32("VIDINFORMAT") | BIT(16) )); # set genlock

$hfrontporchwidth = 144;
$hbackporchstart = 640 + $hfrontporchwidth;

$vfrontporchwidth = 44;
$vbackporchstart = 480 + $vfrontporchwidth;

OUT32("VIDINXDECIMDELTAS", (($hbackporchstart << 16) | $hfrontporchwidth)); 
OUT32("VIDINYDECIMDELTAS", (($vbackporchstart << 16) | $vfrontporchwidth)); 


$i2c_enable = BIT(23);
$i2c_sc_wr = BIT(24);
$i2c_sd_wr = BIT(25);
$i2c_sc_rd =  BIT(26);
$i2c_sd_rd =  BIT(27);

$device = 0x88;  #   lsb is zero where R/Wn value is stored

# HCR
#&WriteRegs( 0x0a, 0x23); # 10 pixels for horizontal shift, still need to get adjusted!!!
#print("ACK: HCR init completed\n") if ($debug);


# CIVC
#&WriteRegs( 0x21, 0x0); # set ACIV bit to 0 to bypass the oscillator, and use ROM
#print("ACK: Use ROM, and bypass the oscillator for master mode completed\n") if ($debug);

# Write to FSCI registers for ROM lookup; for 640x480 NTSC
#&WriteRegs( 0x18, 0x2); 
#&WriteRegs( 0x19, 0x5); 
#&WriteRegs( 0x1a, 0x2); 
#&WriteRegs( 0x1b, 0x4); 
#&WriteRegs( 0x1c, 0x9); 
#&WriteRegs( 0x1d, 0x2); 
#&WriteRegs( 0x1e, 0x4); 
#&WriteRegs( 0x1f, 0x9); 

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


