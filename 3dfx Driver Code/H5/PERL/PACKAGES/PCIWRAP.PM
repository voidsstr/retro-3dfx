#
# Copyright (c) 1998, 3Dfx Interactive, Inc.
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
# the contents of this file may not be disclosed to third parties, copied or
# duplicated in any form, in whole or in part, without the prior written
# permission of 3Dfx Interactive, Inc.
#
# RESTRICTED RIGHTS LEGEND:
# Use, duplication or disclosure by the Government is subject to restrictions
# as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
# and Computer Software clause at DFARS 252.227-7013, and/or in similar or
# successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
# rights reserved under the Copyright Laws of the United States.
#
#
# $Revision: $
# $Date: $
#
#

#----------------------------------------------------------------------
#                    PCI wrapper routines
#----------------------------------------------------------------------

package pciwrap;
require Exporter;
@ISA = qw(Exporter);
@EXPORT = qw(FINDCARD MAPCARD UNMAPCARD
	     OUT32 OUT16 OUT8 
	     IN32 IN16 IN8
	     CFGOUT32 CFGIN32
	     RD32 RD16 RD8
	     WR32 WR16 WR8
	     $PCIVERBOSE
	     $IOBASE $MEMBASE0 $MEMBASE1 $ROMBASE
	     );
@EXPORT_OK = qw();

use Pci;
use h3regs;
use h3init;

#
# Copyright (c) 1998, 3Dfx Interactive, Inc.
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
# the contents of this file may not be disclosed to third parties, copied or
# duplicated in any form, in whole or in part, without the prior written
# permission of 3Dfx Interactive, Inc.
#
# RESTRICTED RIGHTS LEGEND:
# Use, duplication or disclosure by the Government is subject to restrictions
# as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
# and Computer Software clause at DFARS 252.227-7013, and/or in similar or
# successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
# rights reserved under the Copyright Laws of the United States.
#
# --------------------------------------------------------------------------
#
#

$fmt = "   %-8s 0x%08x 0x%08x %s\n";
$debug = 0;
$PCIVERBOSE = $ENV{PCIVERBOSE} if ( defined $ENV{PCIVERBOSE} );

#
# The $addr argument can be either a string or a numeric value.
# If $addr is a string, it is used as a key to look up the 
# corresponding numerical value in the %h3reg hash table.
#

#
# routines for finding, mapping, and unmapping a card
#

sub FINDCARD {
    my($vendorId,$devId,$card) = @_;
    my($flag) = $debug ? 1 : Pci::find($vendorId,$devId,$card);
    printf "   FINDCARD(0x%x,0x%x,0x%x) = 0x%x\n",$vendorId,$devId,$card,$flag if ($PCIVERBOSE);
    return $flag;
}

sub MAPCARD {
    my($vendorId,$devId,$size,$base,$card) = @_;
    my($addr) = $debug ? 0 : Pci::mapCard($vendorId,$devId,$size,$base,$card);
    printf "   MAPCARD(0x%x,0x%x,0x%x,0x%x,0x%x) = 0x%x\n",$vendorId,$devId,$size,$base,$card,$addr
	if ($PCIVERBOSE);
    return $addr;
}

sub UNMAPCARD {
    my($card,$size) = @_;
    Pci::close($card,$size) if ( ! $debug);
    printf "   UNMAPCARD(0x%x,0x%x)\n",$card,$size
	if ($PCIVERBOSE);
}

#
# port i/o access routines
#
# usage:    $OUT8($addr,$data)
#           $OUT16($addr,$data)
#           $OUT32($addr,$data)
#           $data = $IN8($addr)
#           $data = $IN16($addr)
#           $data = $IN32($addr)
#
sub OUT32 { 
    my($addr,$data) = @_; 
    $addr = REGADDR($addr) if ( $addr !~ /^[0-9]*$/ ); 
    printf $fmt,"OUT32",$IOBASE+$addr,$data,$h3portx{$addr} if ($PCIVERBOSE);
    Pci::outpd($IOBASE+$addr,$data) if (!$debug); 
}

sub OUT16 { 
    my($addr,$data) = @_; 
    $addr = REGADDR($addr) if ( $addr !~ /^[0-9]*$/ ); 
    printf $fmt,"OUT16",$IOBASE+$addr,$data,$h3portx{$addr} if ($PCIVERBOSE);
    Pci::outpw($IOBASE+$addr,$data) if (!$debug); 
}

sub OUT8 { 
    my($addr,$data) = @_; 
    $addr = REGADDR($addr) if ( $addr !~ /^[0-9]*$/ ); 
    printf $fmt,"OUT8",$IOBASE+$addr,$data,$h3portx{$addr} if ($PCIVERBOSE);
    Pci::outp($IOBASE+$addr,$data) if (!$debug); 
}

sub IN32  { 
    my($addr) = @_; 
    $addr = REGADDR($addr) if ( $addr !~ /^[0-9]*$/ ); 
    my $data = ($debug) ? 0 : Pci::inpd($IOBASE+$addr); 
    printf $fmt,"IN32",$IOBASE+$addr,$data,$h3portx{$addr} if ($PCIVERBOSE);
    return $data;
}

sub IN16  { 
    my($addr) = @_; 
    $addr = REGADDR($addr) if ( $addr !~ /^[0-9]*$/ ); 
    my $data = ($debug) ? 0 : Pci::inpw($IOBASE+$addr); 
    printf $fmt,"IN16",$IOBASE+$addr,$data,$h3portx{$addr} if ($PCIVERBOSE);
    return $data;
}

sub IN8  { 
    my($addr) = @_; 
    $addr = REGADDR($addr) if ( $addr !~ /^[0-9]*$/ ); 
    my $data = ($debug) ? 0 : Pci::inp($IOBASE+$addr); 
    printf $fmt,"IN8",$IOBASE+$addr,$data,$h3portx{$addr} if ($PCIVERBOSE);
    return $data;
}

#
# config access routines
#
# usage:    $CFGOUT32($addr,$data)
#           $data = $CFGIN32($addr)
#
sub CFGOUT32 {
    my($addr,$data) = @_; 
    $addr = CFGADDR($addr) if ( $addr !~ /^[0-9]*$/ ); 
    printf $fmt,"CFGOUT32",$addr,$data,$h3cfgx{$addr} if ($PCIVERBOSE);
    Pci::writecfg($addr,$data) if (!$debug); 
}
    
sub CFGIN32 {
    my($addr) = @_; 
    $addr = CFGADDR($addr) if ( $addr !~ /^[0-9]*$/ ); 
    my $data = ($debug) ? 0 : Pci::readcfg($addr); 
    printf $fmt,"CFGIN32",$addr,$data,$h3cfgx{$addr} if ($PCIVERBOSE);
    return $data;
}
    
#
# memory-mapped access routines
#
# usage:    $WR8($addr,$data,$base)
#           $WR16($addr,$data,$base)
#           $WR32($addr,$data,$base)
#           $data = $RD8($addr,$base)
#           $data = $RD16($addr,$base)
#           $data = $RD32($addr,$base)
#
# the third argument, $base, is optional.  if it is omitted or is zero, the first
# memory base address is used, otherwise the second memory address is used.
#
sub WR32 {
    my($addr,$data,$base) = @_;
    $addr = REGADDR($addr) if ( $addr !~ /^[0-9]*$/ ); 
    $base = (@_==2 || $base==0) ? $MEMBASE0 : $MEMBASE1;
    printf $fmt,"WR32",$base+$addr,$data,REGNAME($addr,$base) if ($PCIVERBOSE);
    Pci::writed($base+$addr,$data) if (!$debug); 
}
    
sub WR16 {
    my($addr,$data,$base) = @_;
    $addr = REGADDR($addr) if ( $addr !~ /^[0-9]*$/ ); 
    $base = (@_==2 || $base==0) ? $MEMBASE0 : $MEMBASE1;
    printf $fmt,"WR16",$base+$addr,$data,REGNAME($addr,$base) if ($PCIVERBOSE);
    Pci::writew($base+$addr,$data) if (!$debug); 
}
    
sub WR8 {
    my($addr,$data,$base) = @_;
    $addr = REGADDR($addr) if ( $addr !~ /^[0-9]*$/ ); 
    $base = (@_==2 || $base==0) ? $MEMBASE0 : $MEMBASE1;
    printf $fmt,"WR8",$base+$addr,$data,REGNAME($addr,$base) if ($PCIVERBOSE);
    Pci::write($base+$addr,$data) if (!$debug); 
}

sub RD32 { 
    my($addr,$base) = @_;
    $addr = REGADDR($addr) if ( $addr !~ /^[0-9]*$/ ); 
    $base = (@_==1 || $base==0) ? $MEMBASE0 : $MEMBASE1;
    my $data = ($debug) ? 0 : Pci::readd($base+$addr); 
    printf $fmt,"RD32",$base+$addr,$data,REGNAME($addr,$base) if ($PCIVERBOSE);
    return $data;
}

sub RD16 { 
    my($addr,$base) = @_;
    $addr = REGADDR($addr) if ( $addr !~ /^[0-9]*$/ ); 
    $base = (@_==1 || $base==0) ? $MEMBASE0 : $MEMBASE1;
    my $data = ($debug) ? 0 : Pci::readw($base+$addr); 
    printf $fmt,"RD16",$base+$addr,$data,REGNAME($addr,$base) if ($PCIVERBOSE);
    return $data;
}

sub RD8 { 
    my($addr,$base) = @_;
    $addr = REGADDR($addr) if ( $addr !~ /^[0-9]*$/ ); 
    $base = (@_==1 || $base==0) ? $MEMBASE0 : $MEMBASE1;
    my $data = ($debug) ? 0 : Pci::read($base+$addr); 
    printf $fmt,"RD8",$base+$addr,$data,REGNAME($addr,$base) if ($PCIVERBOSE);
    return $data;
}


#
# translate register name to register address
#
sub PORTADDR {
    my($name) = @_;
    die "unknown port $name" 
	if ( ! exists $h3port{$name} );
    return $h3reg{$name};
}

sub REGADDR {
    my($name) = @_;
    die "unknown register $name" 
	if ( ! exists $h3reg{$name} );
    return $h3reg{$name};
}

sub CFGADDR {
    my($name) = @_;
    die "unknown config register $name" 
	if ( ! exists $h3cfg{$name} );
    return $h3cfg{$name};
}

#
# translate register address to register name
#
# usage:  $name = REGNAME($addr,$base)
#
# $base is optional.  if $base != $MEMBASE0, then the null string
# is returned.
#
sub REGNAME {
    my($addr,$base) = @_;
    # only return a name for MEMBASE0 addresses
    return (@_==1 || $base==$MEMBASE0) ? $h3regx{$addr} : "";
}

1;


