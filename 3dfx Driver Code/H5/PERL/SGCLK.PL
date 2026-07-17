#!/usr/local/bin/perl


print STDERR "The 8MB crc values for this script are not up to date.\n";
print STDERR "Fix it if you need to test an 8MB board.\n\n";

# input delay step value
$i_step = 2;
# output delay list
@odels = (5, 2);
# frequency list
@freqlist = (90, 125, 143, 166, 175);
$schmoo_step = 2;
# tests to use with a 16M frame buffer
$test{"large_fb"}[0] = "perf -O6 -y2 -w 1600 -f -xr1.1 -xR0.6 -p 10 -s 101 +k 0x2b6549f3";
$test{"large_fb"}[1] = "perf -O6 -y2 -w 1600 -f -xr1.0 -xR0.5 -p 10 +k 0x193ff5fe";
# tests to use with an 8M frame buffer
$test{"small_fb"}[0] = "perf -O6 -y2 -w 800 -f -xr1.1 -xR0.6 -p 10 -s 101 +k 0xcb31cd1d";
$test{"small_fb"}[1] = "perf -O6 -y2 -w 800 -f -xr1.0 -xR0.5 -p 10 +k 0xd3de6b84";
# tests to use with an SDRAM board
$test{"sdram"}[0] = "perf -O6 -y2 -w 1600 -f -xr1.1 -xR0.6 -p 10 -s 101 +k 0x2b6549f3";
$test{"sdram"}[1] = "perf -O6 -y2 -w 1600 -f -xr1.0 -xR0.5 -p 10 +k 0x193ff5fe";
##
## conservative values:
##
$schmoo_begin = 175;

##
## intenal settings:
##
$ENV{"HAL_VIDEO"} = 0;
$ENV{"HAL_NOINIT"} = 1;
$ENV{"HAL_CSIMIO"} = 0;
$schmoo = 0;
$schmoo_step = 2;
$aggressive = 0;
$board = "$$";
$board =~ s/-+//;
$outfile = "sgclk_${board}.log";
$header = '             /----- input adjustments ---\  /output adj\ 
clk_freq   nodel inv_smp inv_clk del  nodel del draminit1 pass/fail board';
($program = $0)=~s/.*\///;



##
## parse input args
##
while ($arg = shift(@ARGV)) {
    if ($arg =~ /-b/) {
	$board = shift @ARGV;
	$outfile = "sglogs/sgclk_${board}.log";
    } elsif ($arg =~ /-s$/) {
	$schmoo = 1;
	if ($#ARGV > -1) {
	    $fixedinit = hex(shift @ARGV);
	} else {
	    &usage;
	}
    } elsif ($arg =~ /-a/) {
	$aggressive = 1;
    } elsif ($arg =~ /-ss/) {
	($#ARGV < 0) && &usage;
	$schmoo_step = shift @ARGV;
    } elsif ($arg =~ /-sb/) {
	($#ARGV < 0) && &usage;
	$schmoo_begin = shift @ARGV;
    } elsif ($arg =~ /-d0/) {
	($#ARGV < 0) && &usage;
	$override_draminit0 = &get_possible_hex(shift @ARGV);
    } elsif ($arg =~ /-od/) {
	@odels = ();
	while (($ARGV[0] !~ /-[a-z]/) && ($#ARGV > -1)) {
	    push(@odels, &get_possible_hex(shift @ARGV));
	}
    } elsif ($arg =~ /-f/) {
	@freqlist = ();
	while (($ARGV[0] !~ /-/) && ($#ARGV > -1)) {
	    push(@freqlist, &get_possible_hex(shift @ARGV));
	}
    } elsif ($arg =~ /-is/) {
	($#ARGV < 0) && &usage;
	$i_step = &get_possible_hex(shift @ARGV);
    } else {
	&usage;
    }
}


open (OUTFILE, "> $outfile") || die "can't open $outfile for writing";
&myprint ("outfile = $outfile\n");
## cause commands to get flushed out to file immediately:
$oldh = select(OUTFILE);
$|=1;
select($oldh);


##
## detect starting conditions
##
$draminit0 = &do_rd("DRAMINIT0");
if (($draminit0 & 0xc000000) != 0xc000000) {
    $small_frame_buffer = 1;
}
$default_draminit0 = &make_draminit0;
$override_draminit0 && ($default_draminit0 = $override_draminit0);
$draminit0 = ($draminit0 & 0xc000000) | (0x3ffffff & $default_draminit0);
&exec_cmd("perl wr.pl DRAMINIT0 $draminit0");

myprint ("initial values:\n");
open (CMD, "perl rd.pl draminit0 |") || "can't exec perl rd.pl draminit0";
while (<CMD>) {myprint ("  $_") if /DRAM/;}
close(CMD);
open (CMD, "perl rd.pl draminit1 |") || "can't exec perl rd.pl draminit1";
while (<CMD>) {
    if (/DRAMINIT1 = (\S+)/) {
	$initial_draminit1 = hex($1);
	myprint ("  $_") if /DRAM/;
    }
}
close(CMD);
#$force_sdram && ($initial_draminit1 |= 0x40000000);
$type_sdram = ($initial_draminit1 & 0x40000000) ? 1 : 0;

if ($type_sdram) {
    $testtype = "sdram";
} elsif ($small_frame_buffer) {
    $testtype = "small_fb";
} else { 
    $testtype = "large_fb";
}

# Initialization stuff that used to be handled by running at16{d} before
# this program.

$ENV{SSTH3_SGRAM_VENDOR} = "SAMSUNG";
$ENV{SSTH3_GRXCLOCK} = 90;

# Don't write out draminit1 to avoid clobbering sdram strap value.
# The h3init code will pick up the env var value, but leave the strap
# bits alone.

$ENV{SSTH3_DRAMINIT1}=0x56c031;

#
# init the chip
#
$output = &exec_cmd("perl 3dvideo.pl");

##
## first, heat up part a lot by running perf -p 20, @ 120MHz
##
$clk = 120; $cmd = "perl gpll.pl $clk"; &exec_cmd($cmd);
$cmd = "perf -O5 -y2 -w 1600 -p 40 +k 0x9ad8d0e8"; &exec_cmd($cmd);
$passes = 1;
myprint ("\n");


##
## schmoo
##
if ($schmoo) {
    myprint ("Varying frequency with a constant DramInit1\n");
    for ($clk=$schmoo_begin; $clk<240; $clk+=$schmoo_step) {
	$cmd = "perl gpll.pl $clk";
	&exec_cmd($cmd);
	$fail = &run_test($fixedinit);
	$outstr = &make_result($fixedinit>>13, $fail, $clk);
	myprint ("$outstr\n");
	last if $fail;
    }
    goto done;
} 


##
## find DramInit1 settings
##
foreach $clk (@freqlist) {
    $cmd = "perl gpll.pl $clk";
    &exec_cmd($cmd);
    &myprint ("$header\n");  
    
    foreach $o_del_adj (@odels) {
	$o_mindel = ($o_del_adj == -1) ? 1 : 0;
	for ($i_mindel=1; $i_mindel>=0; $i_mindel--)
	{
	    if ($i_mindel) {
		$idel_max = 1;
	    } else {
		$idel_max = 16;
	    }
	    for ($otherbits=0; $otherbits<($i_mindel?1:4); $otherbits++)
	    {
		$i_del_otherbits = ($otherbits<<1) | $i_mindel;
		for ($i_del_adj=0; $i_del_adj<($i_mindel?1:$idel_max); $i_del_adj+=$i_step)
		{
		    $init = ($i_del_adj << 3) | $i_del_otherbits;
		    $init = $init | ($o_del_adj << 7) | ($o_mindel << 11);
		    $fail = &run_test($init);
		    $outstr = &make_result($init, $fail, $clk);
		    myprint ("$outstr\n");
		}

	    }
	}
    }
}


done:
close(OUTFILE);
myprint ("outfile was $outfile\n");
exit;



##
## SUBROUTINES
##

sub make_result {
    local($init, $fail, $clk) = @_;
    local($result);

    $passfail   = $fail ? "FAIL" : "ok";
    $nodelay    = ($init & 1) ? 1 : 0;
    $inv_sample = ($init & 2) ? 1 : 0;
    $inv_clk    = ($init & 4) ? 1 : 0;
    $del_amt    = ($init >> 3) & 0xf;
    $o_delay    = ($init >> 7) & 0xf;
    $o_trans    = ($init >> 11) & 1;

    $result = sprintf "[%3d MHz]  %3d  %6d  %5d  %4d  %5d %4d %8x %6s     $board",
    $clk, $nodelay, $inv_sample, $inv_clk, $del_amt, $o_trans, $o_delay, $newdram, $passfail;

    $result;
}

sub exec_cmd {
    local($cmd) = @_;
    open (CMD, "$cmd |") || die "can't execute a command: $cmd";
    while (<CMD>) {;}
    close (CMD);
}

sub run_test {
    local ($init) = @_;
    local ($error);

    $newdram = $schmoo ? $init : ($initial_draminit1 & ~0x1ffe000) | $init<<13;
#    $newdram = ($initial_draminit1 & ~0x1ffe000) | $init<<13;
    $cmd= "perl wr.pl DRAMINIT1 $newdram";
    open (CMD, "$cmd |") || die "can't execute a command: $cmd";
    while (<CMD>) {;}
    close (CMD);

    $cmd = $test{$testtype}[$other];
    $other = !$other;


    open(CMD, "$cmd |") || die "can't execute a command: $cmd";
    $error = 0;
    while (<CMD>) {
	if (/ERROR/||/error/) {
	    $error = 1; break;
	}
    }
    close(CMD);
    $error;
}

sub usage {
    print STDERR "$program [-b board] [-f list] [-s val] [-ss num] [-sb num] [-od numlist]\n";
    print STDERR "         [-is num] [-d0 num] [-a]\n";
    print STDERR "   -b <board>       generate logfile as \"sglogs/sgclk_{board}.log\"\n";
    print STDERR "   -f <freqlist>    list of frequencies to use for DramInit1 analysis\n";
    print STDERR "                    default: (@freqlist)\n";
    print STDERR "   -s <val>         do a schmoo using <val> as the DramInit1 value.\n";
    print STDERR "   -ss <num>        stepping (in MHz) for the schmoo; default=$schmoo_step\n";
    print STDERR "   -sb <num>        schmoo begin frequency; default = 175MHz\n";
    print STDERR "   -od <numlist>    list of delays to use for the output clock.\n";
    print STDERR "                    default list: (@odels); -1 = no delay\n";
    print STDERR "   -is <num>        input delay stepping; default = $i_step\n";
    print STDERR "   -d0 <num>        default setting for DramInit0 (in hex)\n";
    print STDERR "   -a               use aggressive DramInit0 timing.\n";
    print        "\n";
    print        "This program autodetects the memory size and type of the board and then\n";
    print        "runs a series of diags to stress the board out at various frequencies\n";
    print        "\n";
    print        "The default mode of this script is to vary DramInit1 timings using a\n";
    print        "variety of frequencies (@freqlist).\n";
    print        "\n";
    print        "An alternate mode is to ramp (shmoo) frequency up using a fixed DramInit1\n";
    print        "value. This selected using the -s flag.\n";
    print        "\n";
    exit;
}

sub myprint {
    print OUTFILE @_;
    print @_;
}

sub myprintf {
    printf OUTFILE @_;
    printf @_;
}

sub do_rd {
    ($reg) = @_;
    open (CMD, "perl rd.pl $reg |") || die "can't execute rd.pl $reg";
    while (<CMD>) {
	if (/$reg = (\S+)/) {
	    $value = hex($1);
	}
    }
    close (CMD);
    $value;
}

sub get_possible_hex {
    local ($val) = @_;
    if ($str =~ /0x/ || $str =~ /a-f/ || $str =~ /A-F/) {
	$val = hex($val);
    }
    $val;
}

## some work to do here still
sub make_draminit0 {
    $tRRD = 3;
    $tRCD = 3;
    $tRP = 4;
    $tRASmin = 9;
    $tRC = 12;
    
    $tCAS = 3;
    $tDQR = 1;

    $tBWC = 2; 
    $tWL = 2; 
    $tWL = 2; 

    $val  = ($tRRD-1) & 0x3;
    $val |= (($tRCD-1) & 0x3)<<2;
    $val |= (($tRP-1) & 0x3)<<4;
    $val |= (($tRASmin-1) & 0xf)<<6;
    $val |= (($tRC-1) & 0xf)<<10;

## aggressive:   10_0101_1110_0101 = 25e5
#    $tRRD = 2; 01
#    $tRCD = 2; 01
#    $tRP = 3; 10
#    $tRASmin = 8; 0111
#    $tRC = 10; 1001
#    $tBWC = 1; 0
#    $tWL = 1; 0
#    $tBWL = 1; 0
#    $tRL = 1; 0
    $agg = 0x03a5e5;
## conservative:
#    $tRRD = 3;
#    $tRCD = 3;
#    $tRP = 4;
#    $tRASmin = 9;
#    $tRC = 12;
    $cons = 0x5fae29;

    $val = $aggressive ? $agg : $cons;
}
