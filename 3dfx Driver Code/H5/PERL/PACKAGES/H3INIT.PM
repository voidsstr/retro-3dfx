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

# Initialization routines for Banshee
# Based on common/src/tests/diags/h3_init.c rev 1.78

package h3init;
require Exporter;
@ISA = qw(Exporter);
@EXPORT = qw($IOBASE $MEMBASE0 $MEMBASE1 $ROMBASE
	     h3init h3initAgp h3initBlockWrite h3initSgram h3initTvOut h3initVga h3initVideo h3initGetRev
	     );
@EXPORT_OK = qw();

use h3defs;
use h3regs;
use pciwrap;
use pll;
use sgram;

require "mode_chart.pl";

#
# Find the card
#

sub h3init {
    my($AVENGER_GRX_FREQ,$pciInit0,$grxFreq);
    my($deviceID,$AVENGER_OEM_GRX_FREQ);

    print "h3init: begin\n";

    $AVENGER_GRX_FREQ = 143;
    $AVENGER_OEM_GRX_FREQ = 125;

    # locate board
    $deviceID = h3initMapBoard();

    # Set the clock frequency

    if (defined $ENV{SSTH3_GRXCLOCK}) {
	$grxFreq = $ENV{SSTH3_GRXCLOCK};
    }
    elsif ($deviceID == $h3def{SST_DEVICE_ID_H4}) {
	$grxFreq = $AVENGER_GRX_FREQ;
    }
    elsif ($deviceID == $h3def{SST_DEVICE_ID_H4_OEM}){
	$grxFreq = $AVENGER_OEM_GRX_FREQ;
    }

    # enable IO accesses
    CFGOUT32("PCICOMMAND", BIT(0));

    # You have to add a read wait-state before you can read
    # any registers under AGP

    $pciInit0 = (0x10 << $h3def{SST_PCI_LOWTHRESH_SHIFT}) |
	$h3def{SST_PCI_READ_WS} |
	    $h3def{SST_PCI_WRITE_WS} |
		$h3def{SST_PCI_DISABLE_IO} |
		    $h3def{SST_PCI_DISABLE_MEM} |
			(0x18 << $h3def{SST_PCI_FIFO_READ_THRESHOLD_SHIFT});
    OUT32("PCIINIT0", $pciInit0);

    # other init routines
    h4initPlls($grxFreq,$deviceID);
    h3initSgram();
    h3initVga();
    h3initTvOut("ChrontelSlave"); # Check this.
    h3initVideo();

    # Enable memory-mapped accesses
    CFGOUT32("PCICOMMAND", BIT(0)|BIT(1) );

    print "h3init: end\n";
}

#
# Map the board. This sub will look for either H4 or H4_OEM.
# Note that $MEMBASE0, $MEMBASE1, $IOBASE, and $ROMBASE are exported by this
# module.

sub h3initMapBoard {
    my($deviceID);

    # locate board
    if (FINDCARD(0x121a,$h3def{SST_DEVICE_ID_H4},0)) {
	$deviceID = $h3def{SST_DEVICE_ID_H4};
    }
    elsif (FINDCARD(0x121a,$h3def{SST_DEVICE_ID_H4_OEM},0)) {
	$deviceID = $h3def{SST_DEVICE_ID_H4_OEM};
    }
    else {
	die "h3init: Can't locate board!\n";
    }

    # map board
    $MEMBASE0 = MAPCARD(0x121a, $deviceID, 0x2000000, 0, 0);
    $MEMBASE1 = MAPCARD(0x121a, $deviceID, 0x2000000, 1, 0);

    # read base address registers
    $IOBASE = CFGIN32("IOBASEADDR") & 0xfff0;
    $ROMBASE = CFGIN32("ROMBASEADDR") & 0xfffffff0;
    
    printf "IOBASE   = 0x%x\n",$IOBASE;
    printf "MEMBASE0 = 0x%x\n",$MEMBASE0;
    printf "MEMBASE1 = 0x%x\n",$MEMBASE1;
    printf "ROMBASE  = 0x%x\n",$ROMBASE;
    return $deviceID;
}

#
# *** UNDER DEVELOPMENT ***
#
# configure AGP accesses
#

sub h3initAgp {
    # check whether AGP is enabled
    $data = CFGIN32("PCISTATUS");
    $agpEnabled = 0;
    if (data & PCISTATUS_CAPLIST) {
	$capptr = CFGIN32("CAPPTR");
	while ($capptr != 0) {
	    $data = CFGIN32($capptr);
	    $agpEnabled = 1
		if (($data & $h3def{CAPID_MASK}) >> $h3def{CAPID_SHIFT}  == $h3def{AGP_ID});
	    $capptr = ($data & $h3def{NEXTPTR_MASK}) >> $h3def{NEXTPTR_SHIFT};
	}
    }

    # configure AGP
    if ($agpEnabled) {
	$agpStatus = CFGIN32("AGPSTATUS");
	if ($agpStatus & $h3def{AGPSTATUS_RES0} || 
	    $agpStatus & $h3def{AGPSTATUS_RES1} || 
	    $agpStatus & $h3def{AGPSTATUS_RES2} ) {
	    printf("h3initAgp: ***Config Error*** AGP Status Reserved field not 0\n");
	}

	printf("h3initAgp: ***Config Error*** AGP Status 4G not 1\n")
	    if (($agpStatus & $h3def{AGPSTATUS_4G}) == 0);
	printf("h3initAgp: ***Config Error*** AGP Status SBA not 1\n")
	    if (($agpStatus & $h3def{AGPSTATUS_SBA}) == 0);
	printf("h3initAgp: ***Config Error*** AGP Status RATE not set\n")
	    if (($agpStatus & $h3def{AGPSTATUS_RATE}) == 0);

	$rqDepth = ($agpStatus & $h3def{AGPSTATUS_RQDEPTH}) >> $h3def{AGPSTATUS_RQDEPTH_SHIFT};
	$sysRqDepth = (exists $ENV{SYSMEMCTRL_RQDEPTH}) ? &atoi($ENV{SYSMEMCTRL_RQDEPTH}) : 0x7;
	$actual = ($sysRqDepth > $rqDepth) ? $rqDepth : $sysRqDepth;
	printf("h3initAgp: AGP RQDEPTH %d\n",$actual);
	$agpCmd = ($actual << $h3def{AGPCMD_RQDEPTH_SHIFT}) & $h3def{AGPCMD_RQDEPTH} |
	    $h3def{AGPCMD_SBAENABLE} | $h3def{AGPCMD_4G} | $h3def{AGPCMD_ENABLE};
	if ( ($agpStatus & $h3def{AGPSTATUS_RATE}) >= 2 ) {
	    $agpCmd |= (2 & $h3def{AGPCMD_RATE});
	} else {
	    $agpCmd |= (1 & $h3def{AGPCMD_RATE});
	}
	CFGOUT32("AGPCMD",$agpCmd);
	$rdata = CFGIN32("AGPCMD");
	if ($agpCmd != $rdata) {
	    printf("h3initAgp: ***Config Error*** Unexpected AGPCMD 0x%x. Expecting 0x%x\n",
		   $rdata,$agpCmd);
	}
    }

}

#
# configure memory controller and SGRAMs
#
# usage: h3initSgram()
#
# The type of SGRAM is specified with the SSTH3_SGRAM_VENDOR environment
# variable. Legal values are ETRON, GLINK, MOSYS, SAMSUNG, SIEMENS, and TOSHIBA
# The default value is GENERIC.
#
# Note: This function clobbers any previous contents of register:
#       tmuGbeInit
#

sub h3initSgram {
    @_ == 0 or print "h3initSgram: WARNING, vendor type must be specified via SSTH3_SGRAM_VENDOR env var\n";

    my ($xDramInit0,$sgramMode,$sgramMask,$sgramColor);
    my ($sgramName,$dramInit1,$dramInit0,$grxFreq);
    my ($miscInit1,$sdram,$nbanks,$size);
    my ($AVENGER_BRINGUP_FREQ,$temp);
    
    $AVENGER_BRINGUP_FREQ = 100.0;

    # check for sgram type from env variable

    if ( exists $ENV{SSTH3_SGRAM_VENDOR} ) {
	$sgramName = $ENV{SSTH3_SGRAM_VENDOR};
    } else {
	$sgramName = "GENERIC";
    }
    $sgramName =~ tr/a-z/A-Z/;

    # read strap bits

    $dramInit1 = IN32("DRAMINIT1");
    $dramInit1 &= $h3def{SST_MCTL_TYPE_SDRAM};  # preserve strap bit

    $dramInit0 = IN32("DRAMINIT0");
    $dramInit0 &= $h3def{SST_SGRAM_NUM_CHIPSETS} | $h3def{SST_SGRAM_TYPE};  # don't overwrite strapping bits

    $sdram = $dramInit1 & $h3def{SST_MCTL_TYPE_SDRAM};
			    
    if ( $sdram ) {  
	$nbanks = 1;
	$size = 16;
	print "Memory configuration is $nbanks banks x $size Mbit $sgramName SDRAM parts = ",$nbanks*$size," Mbytes\n";
	# clear num chipset field to avoid a minor performance hit when using SDRAM
	$dramInit0 &= ~$h3def{SST_SGRAM_NUM_CHIPSETS};
    } else {
	$nbanks = ( ($dramInit0 & $h3def{SST_SGRAM_NUM_CHIPSETS}) == 0 ) ? 1 : 2;
	if ( ($dramInit0&$h3def{SST_SGRAM_TYPE}) == $h3def{SST_SGRAM_TYPE_8MBIT} ) {
	    $size = 8;
	} elsif ( ($dramInit0&$h3def{SST_SGRAM_TYPE}) == $h3def{SST_SGRAM_TYPE_16MBIT} ) {
	    $size = 16;
	} else {
	    die "h3initSgram: invalid sgram type = ", 
	    ($dramInit0&$h3def{SST_SGRAM_TYPE})<<$h3def{SST_SGRAM_TYPE_SHIFT};
	}
	print "Memory configuration is $nbanks banks x $size Mbit $sgramName SGRAM parts = ",$nbanks*$size/2," Mbytes\n";
    }

    # turn on memory refresh
    if ( exists $ENV{SSTH3_DRAMINIT1} ) {  # env variable overrides value
	$dramInit1 |= &atoi($ENV{SSTH3_DRAMINIT1}) & ~$h3def{SST_MCTL_TYPE_SDRAM}; # don't overwrite strapping bits
    } else {
	$grxFreq = ( exists $ENV{SSTH3_GRXCLOCK} ) ? $ENV{SSTH3_GRXCLOCK} : $AVENGER_BRINGUP_FREQ;
	if ( $grxFreq >= 100 ) {
            $dramInit1 |= 8<<$h3def{SST_SGRAM_OFLOP_DEL_ADJ_SHIFT};
	    $dramInit1 |= $h3def{SST_SGRAM_CLK_NODELAY};
	    $dramInit1 |= $h3def{SST_DRAM_REFRESH_EN};
	    $dramInit1 |= (0x18 << $h3def{SST_DRAM_REFRESH_VALUE_SHIFT}) & $h3def{SST_DRAM_REFRESH_VALUE};  
	} else {
	    $dramInit1 |= 2<<$h3def{SST_SGRAM_OFLOP_DEL_ADJ_SHIFT};
	    $dramInit1 |= $h3def{SST_SGRAM_DEL_CLK_INVERT};
	    $dramInit1 |= $h3def{SST_SGRAM_USE_INV_SAMPLE};
	    $dramInit1 |= $h3def{SST_DRAM_REFRESH_EN};
	    $dramInit1 |= (0x4 << $h3def{SST_SGRAM_CLK_ADJ_SHIFT}) & $h3def{SST_SGRAM_CLK_ADJ};
	    $dramInit1 |= (0x18 << $h3def{SST_DRAM_REFRESH_VALUE_SHIFT}) & $h3def{SST_DRAM_REFRESH_VALUE};  
	}
    }
    
    OUT32("DRAMINIT1", $dramInit1);
    printf "dramInit1 = 0x%x\n",$dramInit1;

    # get config info
    print "$sgramName\n";
    ($xDramInit0,$sgramMode,$sgramMask,$sgramColor) = &calcSgramParams($sgramName,$sdram,$size);
    printf "0x%x 0x%x 0x%x 0x%x \n",$xDramInit0,$sgramMode,$sgramMask,$sgramColor;
    $xDramInit0 = &atoi($ENV{SSTH3_DRAMINIT0}) if ( exists $ENV{SSTH3_DRAMINIT0} ); # env variable overrides value
    $xDramInit0 &= ~($h3def{SST_SGRAM_NUM_CHIPSETS} | $h3def{SST_SGRAM_TYPE}); # don't overwrite strapping bits
					   
    $dramInit0 |= $xDramInit0;

    OUT32("DRAMINIT0", $dramInit0);
    $temp = IN32("DRAMINIT0");
    printf("h3initSgram: Error setting draminit0 to 0x%08x, read 0x%08x\n",$dramInit0,$temp)
	if ($temp != $dramInit0);

    # load sgram mode register
    $sgramMode = &atoi($ENV{SSTH3_SGRAMMODE}) if ( exists $ENV{SSTH3_SGRAMMODE} ); # env variable overrides value
    OUT32("DRAMDATA",$sgramMode);
    OUT32("DRAMCOMMAND",0x10d);

    if ( ! $sdram ) {
	# load sgram mask register
	$sgramMask = &atoi($ENV{SSTH3_SGRAMMASK}) if ( exists $ENV{SSTH3_SGRAMMASK} ); # env variable overrides value
	OUT32("DRAMDATA",$sgramMask);	
	OUT32("DRAMCOMMAND",0x10e);
	
	# load sgram color register
	$sgramColor = &atoi($ENV{SSTH3_SGRAMCOLOR}) if ( exists $ENV{SSTH3_SGRAMCOLOR} ); # env variable overrides value
	OUT32("DRAMDATA",$sgramColor);	
	OUT32("DRAMCOMMAND",0x10f);
    }

    # disable block write for SDRAM

    if ( $sdram ) {
	$miscInit1 = IN32("MISCINIT1");
	$miscInit1 |= $h3def{SST_DISABLE_2D_BLOCK_WRITE};
	OUT32("MISCINIT1",$miscInit1);
    }

    # disable texture cache CAM
    $tmuGbeInit = (0x5 << $h3def{SST_TGI_READLOW_THRESH_SHIFT}) |
	(0x5 << $h3def{SST_TGI_READHIGH_THRESH_SHIFT}) |
	    (0x5 << $h3def{SST_TGI_PIXELHIGH_THRESH_SHIFT});
    OUT32("TMUGBEINIT",$tmuGbeInit);
}

#
# configure and wake up VGA controller
#
sub h3initVga {
    my($vgaInit0,$miscInit1);

    OUT32("VGAINIT0", BIT(8) | BIT(9) );	# wake-up select, disable legacy registers

    # wake up VGA motherboard style

    $vgaInit0 = IN32("VGAINIT0");
    $vgaInit0 = ($vgaInit0 & 0xfeff) | BIT(8) | BIT(9) | BIT(6);
    OUT32("VGAINIT0", $vgaInit0);

    OUT32("VGAINIT1", 0x0);


    OUT8(0xc3, 0x1);


    $miscInit1 = IN32("MISCINIT1");
    $miscInit1 |= BIT(0);               # clear CLUT Invert Address
    OUT32("MISCINIT1",$miscInit1);


}

#
# configure 2D block write parameters
#
# usage: h3initBlockWrite(enable,threshhold)
#
sub h3initBlockWrite {
    ($enable,$thresh) = @_;
    my($miscInit1,$sdram,$temp);

    # read current config, clear block write fields
    $miscInit1 = IN32("MISCINIT1");
    $miscInit1 &= ~($h3def{SST_BLOCK_WRITE_THRESH} | $h3def{SST_DISABLE_2D_BLOCK_WRITE});

    # don't allow block writes to be turned on with SDRAM
    $sdram = RD32("DRAMINIT1") & $h3def{SST_MCTL_TYPE_SDRAM};
    if ( $sdram && $enable ) {
	print "h3inintBlockWrite: cannot enable block writes with SDRAM !!!\n";
	$enable = 0;
    }

    # set fields
    if ($enable) {
	$miscInit1 |= ($thresh << $h3def{SST_BLOCK_WRITE_THRESH_SHIFT}) & $h3def{SST_BLOCK_WRITE_THRESH};
    } else {
	$miscInit1 |= $h3def{SST_DISABLE_2D_BLOCK_WRITE};
    }

    # display settings
    printf "miscInit1 block write thresh=0x%x\n",
    ($miscInit1&$h3def{SST_BLOCK_WRITE_THRESH})>>$h3def{SST_BLOCK_WRITE_THRESH_SHIFT};
    printf "miscInit1 block writes %s\n",
    $miscInit1&$h3def{SST_DISABLE_2D_BLOCK_WRITE} ? "DISABLED" : "ENABLED";
    
    # update register
    OUT32("MISCINIT1", $miscInit1);
    $temp = IN32("MISCINIT1");
    printf("Error setting miscInit1 to 0x%08x, read 0x%08x\n",$miscInit1,$temp)
	if ($temp != $miscInit1);
}

#
# configure tv-out interface
#
# usage: h3initTvOut("mode")
#
# where mode is one of {ChrontelSlave,ChrontelMaster,BtSlave,BtMaster}
#
sub h3initTvOut {
    my($mode) = @_;
    my($miscInit0,$tmuGbeInit,$vidSerialParallelPort);

    if ($mode =~ /^ChrontelSlave$/) {

	# vid in format chrontel digital tv, no genlock
        OUT32("VIDINFORMAT", $h3def{SST_VIDEOIN_TVOUT_ENABLE});

	# Deassert the vmi device's chip select.
	$vidSerialParallelPort = $h3def{SST_SERPAR_VMI_CS_N};
        OUT32("VIDSERIALPARALLELPORT", $vidSerialParallelPort);

	# Clear all of the delay fields.
	$miscInit0 = IN32("MISCINIT0");
	$miscInit0 &= ~($h3def{SST_TVOUT_BLANK_DELAY} | $h3def{SST_TVOUT_SYNC_DELAY} |
			$h3def{SST_MONITOR_BLANK_DELAY});
	OUT32("MISCINIT0",$miscInit0);

	# Invert the tv clk out
	$tmuGbeInit = IN32("TMUGBEINIT");
	$tmuGbeInit |= $h3def{SST_TVOUT_CLK_INVERT};
        OUT32("TMUGBEINIT",$tmuGbeInit);

    } elsif ($mode =~ /^ChrontelMaster$/) {
	die "h3initTvOut: mode \"$mode\" not implemented yet";
    } elsif ($mode =~ /^BtSlave$/) {
	die "h3initTvOut: mode \"$mode\" not implemented yet";
    } elsif ($mode =~ /^BtMaster$/) {
	die "h3initTvOut: mode \"$mode\" not implemented yet";
    } else {
	die "h3initTvOut: invalid mode = \"$mode\"";
    }
}

#
# configure video processor
#
sub h3initVideo {
    my($vidProcCfg);

    $vidProcCfg = IN32("VIDPROCCFG");
    $vidProcCfg |= $h3def{SST_VIDEO_PROCESSOR_EN};

    $vidProcCfg |= 
	$h3def{SST_DESKTOP_CLUT_BYPASS} | $h3def{SST_OVERLAY_CLUT_BYPASS};

    OUT32("VIDPROCCFG",$vidProcCfg);
    printf("h3initVideo: Video Processor ENABLED\n");
}

#
# configure PLL for GRX clock
#
# h4initPlls($grxFreq)
# 

sub h4initPlls {
    @_ == 2 or die "h4initPlls: Wrong number of arguments: @_\n";

    my($grxFreq,$deviceID) = @_;
    my($w_wedge,$n,$m,$k,$actual,$pllCtrl1);

    if ($deviceID == $h3def{SST_DEVICE_ID_H4}) {
	# calcPllParams actually ignores $w_wedge.
	$w_wedge = -1;
	($n,$m,$k) = calcPllParams($grxFreq,$w_wedge);
    }
    elsif ($deviceID == $h3def{SST_DEVICE_ID_H4_OEM}){
	# calcOEMPllParams also wedges k to 1.
	if ($grxFreq > 141) {
	    die "h4initPlls: This is an OEM part and can only be set to a max frequency of 141 MHz.\n";
	}
	$w_wedge = 11;
	($n,$m,$k) = calcOEMPllParams($grxFreq,$w_wedge);
    }
    else {
	die "h4initPlls: Unrecognized device ID: $deviceID\n";
    }
    $actual = 14.31818 * ($n + 2) / ($m + 2) / 2**$k;
    printf "GRX clock: freq=%.4f, actual=%.4f, n=%d, m=%d, k=%d\n",
        $grxFreq, $actual, $n, $m, $k;
    $pllCtrl1 = IN32("PLLCTRL1");
    $pllCtrl1 &= $h3def{SST_PLL_TEST};    # preserve the test state
    $pllCtrl1 |= ($k<<$h3def{SST_PLL_K_SHIFT}) & $h3def{SST_PLL_K};
    $pllCtrl1 |= ($m<<$h3def{SST_PLL_M_SHIFT}) & $h3def{SST_PLL_M};
    $pllCtrl1 |= ($n<<$h3def{SST_PLL_N_SHIFT}) & $h3def{SST_PLL_N};
    OUT32("PLLCTRL1",$pllCtrl1);
}

#
#
# set the video Timing
# (x_res, y_res, refresh_rate);
#
# This subroutine sets up the video timing for Banshee.
# This sub requires mode_chart.pl, and gets %mode_num from
# there.
#

sub h3initVideoTiming
{
    my($x_res,$y_res,$refresh_rate,@reg_values);
    my($rev,$vidProcCfg,$vgainit0,$v,$i,$j);

    #
    # Load in the set of video modes supported by Banshee
    #

    $x_res          = $_[0];
    $y_res          = $_[1];
    $refresh_rate	= $_[2];

    printf "Mode set: %sx%s @%s Hz\n", $x_res, $y_res, $refresh_rate; 

    if (! exists $mode_num{"$x_res $y_res $refresh_rate"})
    {
        printf "Resolution not supported!\n";
	exit;
    }

    @reg_values = split (' ', $mode_num{"$x_res $y_res $refresh_rate"});

#
# MISC REGISTERS
# This register gets programmed first since the Mono/ Color
# selection needs to be made.
#
# Sync polarities
# Also force the programmable clock to be used with bits 3&2
#

    OUT8(0x0C2, hex($reg_values[16]) | BIT(0) );

#
# SEQ REGISTERS
# set stop mode in the sequencer (reset)
#
#
    OUT16( 0x0C4, ( 0x0 << 8) | 0x0 );

#
# CRTC REGISTERS
# First Unlock CRTC, then program them 
#


    OUT16( 0x0D4,  (0x00 <<8) | 0x11 );
    OUT16( 0x0D4,  (hex($reg_values[0]) <<8) | 0x0 );
    OUT16( 0x0D4,  (hex($reg_values[1]) <<8) | 0x1 );
    OUT16( 0x0D4,  (hex($reg_values[2]) <<8) | 0x2 );
    OUT16( 0x0D4,  (hex($reg_values[3]) <<8) | 0x3 );
    OUT16( 0x0D4,  (hex($reg_values[4]) <<8) | 0x4 );
#
# The problem with 800x600 is due to a
# synthesys bug (Which A1 does not use)
#

    $rev = h3initGetRev();
    if ( $rev == 1 && ( hex($reg_values[14]) & 0x80) ) {
	OUT16( 0x0D4,  ((hex($reg_values[5]) & 0xe0) <<8) | 0x5 );
    } else {
	OUT16( 0x0D4,  (hex($reg_values[5]) <<8) | 0x5 );
    }

    OUT16( 0x0D4,  (hex($reg_values[6]) <<8) | 0x6 );
    OUT16( 0x0D4,  (hex($reg_values[7]) <<8) | 0x7 );
    OUT16( 0x0D4,  (hex($reg_values[8]) <<8) | 0x9 );
    OUT16( 0x0D4,  (hex($reg_values[9]) <<8) | 0x10 );
    OUT16( 0x0D4,  (hex($reg_values[10]) <<8) | 0x11 );
    OUT16( 0x0D4,  (hex($reg_values[11]) <<8) | 0x12 );
    OUT16( 0x0D4,  (hex($reg_values[12]) <<8) | 0x15 );
    OUT16( 0x0D4,  (hex($reg_values[13]) <<8) | 0x16 );

#
# Remove BLANK signal from inverted boards....
#

    if( $rev == 1 )
    {
      OUT16( 0x0D4,  (hex($reg_values[14]) <<8) | 0x101a );
      OUT16( 0x0D4,  (hex($reg_values[15]) <<8) | 0x101b );
    }
    else
    {
      OUT16( 0x0D4,  (hex($reg_values[14]) <<8) | 0x1a );
      OUT16( 0x0D4,  (hex($reg_values[15]) <<8) | 0x1b );
    }

#
# Enable Sync Outputs
#
    OUT16( 0x0D4,  (0x80 <<8) | 0x17 );

#
# VIDCLK (32 bit access only!)
# Set the Video Clock to the correct frequency
#

    OUT32( "PLLCTRL0",
          (hex($reg_values[19])<<8) | hex($reg_values[18]));

#
# DACMODE (32 bit access only!)
# (sets up 1x mode or 2x mode)
#
    OUT32( "DACMODE",  hex($reg_values[20]));
    $vidProcCfg = IN32("VIDPROCCFG");
    $vidProcCfg &= ~$h3def{SST_VIDEO_2X_MODE_EN};
    $vidProcCfg |= $h3def{SST_VIDEO_2X_MODE_EN} if ( hex($reg_values[20]) );
    OUT32("VIDPROCCFG",$vidProcCfg);

#
# SEQ REGISTERS
# set run mode in the sequencer (not reset)
#
# make sure bit 5 == 0 (i.e., screen on)
#
    OUT16( 0x0C4, ((hex($reg_values[17]) & ~BIT(5))<<8) | 0x1 );
    OUT16( 0x0C4, ( 0x3 << 8) | 0x0 );

#
# turn off VGA screen refresh here
#
    $vgainit0 = IN32("VGAINIT0");
    OUT32("VGAINIT0",$vgainit0 | BIT(12));

#
#  Make Sure Attribute Index register is Initialized 
#
    IN8(  0x0DA);

@vgaattr = (0x00, 0x00, 0x00, 0x00, 0x00, 
            0x00, 0x00, 0x00, 0x00, 0x00, 
            0x00, 0x00, 0x00, 0x00, 0x00, 
            0x00, 0x01, 0x00, 0x0F, 0x00);


  for ($v=0; $v <= 19 ; $v++) {
    OUT8( 0x0C0, $v  );
    OUT8( 0x0C0, $vgaattr[$v]);
  } 

    OUT8( 0x0C0, 0x34  );

    OUT8( 0x0DA, 0x00 );

#
# Initialize VIDEO res & proc mode info...
#

    if($x_res < 639) { # half-mode is used 
	OUT32( "VIDSCREENSIZE", ($y_res  << 13) | (($x_res) & 0xfff) );
	$vidProcCfg = IN32("VIDPROCCFG");
	$vidProcCfg |= $h3def{SST_HALF_MODE}; # set half mode bit
	OUT32("VIDPROCCFG",$vidProcCfg);
    }
    else {
	$vidProcCfg = IN32("VIDPROCCFG");
	$vidProcCfg &= ~$h3def{SST_HALF_MODE}; # clear half mode bit
	OUT32("VIDPROCCFG",$vidProcCfg);
	OUT32( "VIDSCREENSIZE", ($y_res  << 12) | ($x_res & 0xfff) );
    }
    OUT32( "VIDOVERLAYSTARTCOORDS",  0 );
    OUT32( "VIDOVERLAYENDCOORDS", ($y_res  << 12) | ($x_res & 0xfff) );
#    OUT32( "VIDPROCCFG",  0 );


#
# Load CLUTs with an inverted ramp (undo inversion with new hardware!)
#


    for( $i=0; $i<0x1ff; $i=$i+1 ) {
	OUT32("DACADDR", $i);        # assumes CLUT Invert Address, miscInit1[0], is set
	IN32("DACADDR");
	$j = $i & 0xff;
        OUT32("DACDATA", (($j<<16) | ($j<<8) | $j));
	IN32("DACDATA");
    }

}

#
# read and return revision ID
#
sub h3initGetRev {
    my $rev;

    $rev = CFGIN32("REVISIONID") & 0xff;

    return $rev;
}

1;

#
# convert a string to a number
#
sub atoi {
    my($str)=@_;
    my($val);

    $val = $str;
    $val = hex($str) if ( $str =~ /0x/ || $str =~ /0X/ );

    return $val;
}
