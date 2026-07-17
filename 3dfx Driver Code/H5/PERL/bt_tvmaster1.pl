#!/usr/local/bin/perl

use lib '.\packages';

use h3init;
use pciwrap;
use h3regs;
use h3defs;

h3init::h3initMapBoard;

$debug = 1;

$i2c_enable = BIT(23);
$i2c_sc_wr = BIT(24);
$i2c_sd_wr = BIT(25);
$i2c_sc_rd =  BIT(26);
$i2c_sd_rd =  BIT(27);

$device = 0x88;  # lsb is zero where R/Wn value is stored


&SPOUT( &SPIN() | $i2c_enable );  # enable i2c while sd and sc are high
until ( &SPIN() & $i2c_enable ) {print("Wait for i2c to be enabled\n") if ($debug);}
printf("After enable\n") if ($debug);

&SendStop;
printf("After stop is sent\n") if ($debug);

# auto config to mode 0
&WriteRegs( 0xb8, 0x00); # 
print("ACK: auto config initialized\n") if ($debug);

&WriteRegs( 0xc4, 0x01); # 
print("ACK: en_out initialized\n") if ($debug);

&WriteRegs( 0xc6, 0x00); # 
print("ACK: en_blko = 0 initialized; BEFORE RESET\n") if ($debug);



#############################
# 
# &WriteRegs( 0xc4, 0x04); # 
# print("ACK: Color bar initialized\n") if ($debug);

# 
# &WriteRegs( 0xc4, 0x01); # 
# print("ACK: Color bar disabled and en_out initialized\n") if ($debug);

# soft reset + master  mode
# &WriteRegs( 0xba, 0x80); # 
# print("ACK: soft reset + master mode\n") if ($debug);

# 
# &WriteRegs( 0x9c, 0x0e); # 
# print("ACK: pll fract[7:0] initialized\n") if ($debug);

# 
# &WriteRegs( 0x9e, 0x88); # 
# print("ACK: pll fract[15:8] initialized\n") if ($debug);

# 
# &WriteRegs( 0xa0, 0x0c); # 
# print("ACK: en_xclk, by_pll, pll int initialized\n") if ($debug);

# 
# &WriteRegs( 0xb6, 0x00); # 
# print("ACK: b6 initialized\n") if ($debug);
# 
# &WriteRegs( 0xba, 0x00); # 
# print("ACK: ba initialized\n") if ($debug);
# 
# &WriteRegs( 0xc6, 0x00); # 
# print("ACK: c6 initialized\n") if ($debug);
# 
# &WriteRegs( 0xc6, 0xc0); # 
# print("ACK: c6 initialized\n") if ($debug);
# 
# &WriteRegs( 0xc8, 0x00); # 
# print("ACK: c8 initialized\n") if ($debug);
# 
# &WriteRegs( 0xca, 0x00); # 
# print("ACK: ca initialized\n") if ($debug);
# 
# &WriteRegs( 0xcc, 0x00); # 
# print("ACK: cc initialized\n") if ($debug);
# 
# &WriteRegs( 0xce, 0x00); # 
# print("ACK: ce initialized\n") if ($debug);
# 
# &WriteRegs( 0xd0, 0x00); # 
# print("ACK: d0 initialized\n") if ($debug);
# 
# &WriteRegs( 0xd2, 0x00); # 
# print("ACK: d2 initialized\n") if ($debug);
# 
# &WriteRegs( 0xd4, 0x00); # 
# print("ACK: d4 initialized\n") if ($debug);
# 
# &WriteRegs( 0xd6, 0x00); # 
# print("ACK: d6 initialized\n") if ($debug);


#############################
# soft reset + master  mode
#&WriteRegs( 0xba, 0x80); # 
#print("ACK: soft reset + master mode\n") if ($debug);



print "\n";
print "*** i2c initialization completed ***\n";

sub ReadRegs {
    my @addrs = @_;
    my @data;
    my $n = @_;
    my $a;

    foreach $a ( @_ ) {

	printf "ReadRegs: reading reg[0x%x]\n",$a if ( $debug );
	
#	&SendStart;
#	&SendByte( $device );  # device + W
#	&SendByte( $a & ~BIT(6) );        # send address, clear AutoInc bit
	
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

#    getc();

    &SendStart;
    &SendByte( $device );  # device + W

    while (@_) {
	$a = shift @_;
	$d = shift @_;

#	$a &= ~BIT(6);   # clear AutoInc bit

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

    until ( &SPIN() & $i2c_sd_rd ) { # toggle the clock until the data line is released
 
    &SPOUT(&SPIN() | $i2c_sd_wr);  # release sd

    &SPOUT(&SPIN() & ~$i2c_sc_wr);  # pull sc low
    until ( (&SPIN() & $i2c_sc_rd) == 0 ) {}

    &SPOUT(&SPIN() | $i2c_sc_wr); # release sc
    until ( &SPIN() & $i2c_sc_rd ) {} # wait for sc to go hi

    &SPOUT(&SPIN() & ~$i2c_sc_wr);  # pull sc low
    until ( (&SPIN() & $i2c_sc_rd) == 0 ) {}

    printf("toggle the clock until the data line is released\n");
    }

    if ( &SPIN() & $i2c_sc_rd ) {
    &SPOUT(&SPIN() & ~$i2c_sc_wr);  # pull sc low
    until ( (&SPIN() & $i2c_sc_rd) == 0 ) {}
    }
 	
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



