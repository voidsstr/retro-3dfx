#!/usr/local/bin/perl

use lib '.\packages';

use h3init;
use pciwrap;
use h3regs;
use h3defs;

h3init::h3initMapBoard;

$debug = 1;

$i2c_enable = BIT(18);
$i2c_sc_wr = BIT(19);
$i2c_sd_wr = BIT(20);
$i2c_sc_rd =  BIT(21);
$i2c_sd_rd =  BIT(22);

$device = 0xa0;  # monitor device id according to ddc spec

# stop
#SPOUT(SPIN() & ~$i2c_sc_wr);  # pull sc low
#until ( (SPIN() & $i2c_sc_rd) == 0 ) {}
&SPOUT( &SPIN() | $i2c_sd_wr ); # I don't understand this !!!
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

for ($q=0; $q<=0; $q++) {
    @dd = &ReadRegs($q);
    printf "reg[0x%x] = 0x%x\n",$q,$dd[0];
    ($dd) = &ReadRegs($q);
    printf "reg[0x%x] = 0x%x\n",$q,$dd;
}

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

    for ($i=127; $i>=0; $i=$i-1) {

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




