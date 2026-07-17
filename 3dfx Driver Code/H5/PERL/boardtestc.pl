@rem = '--*-Perl-*--
@echo off
perl -S %0.bat %1 %2 %3 %4 %5 %6 %7 %8 %9
goto endofperl
@rem ';
#line 7
################################################################################
# 
# Automated script for testing chips with speed vectors
# 1. run 3dvideo.pl
# 2. run memdiag.pl
# 3. run approprite CRC version of shortdiags testlist
#
# $Revision: 2 $
#
################################################################################

# get current name 
$progName = substr($0,rindex($0,"\\")+1);  # basename($0)
$progName = substr($progName,rindex($progName,"/")+1);  # basename($0)
$progName =~ s/\.[bB][aA][tT]$//;          # remove trailing .bat, if any

# defaults
$AVENGER_BRINGUP_FREQ = 90;

# parse command-line arguments
$cmdline = $progName . " " . join(" ",@ARGV);

if ( @ARGV == 1 ) {
    $vendor = shift @ARGV;
    $gclk = $AVENGER_BRINGUP_FREQ;
} elsif ( @ARGV == 2 ) {
    $vendor = shift @ARGV;
    $gclk = shift @ARGV;
} else {
    &Usage();
    exit 1;
}

#while (@ARGV) {
#    $_ = shift @ARGV;
#    if ( /^$/ ) { }
#    elsif ( /^$/ ) { }
#    else { }
#}

#
# set clock speeds
#

# Keep fxHalInitRegisters from clobbering draminit1.
$ENV{HAL_NOINIT}=1;

# Set the pll frequency.

&Execute("perl gpll.pl $gclk");

$ENV{SSTH3_SGRAM_VENDOR}=$vendor;
$ENV{SSTH3_GRXCLOCK}=$gclk;

# As of 12-03-98, these values work for SGRAM and SDRAM.
# Don't write out draminit1 to avoid clobbering sdram strap value.
# The h3init code will pick up the env var value, but leave the strap
# bits alone.

$ENV{SSTH3_DRAMINIT1}=0x564031;

# Do write out draminit0 with numBanks = 1 to make fbiMemSize calculate
# the sdram size correctly. This requires that h3init::h3initSgram no
# longer clear the SST_SGRAM_NUM_CHIPSETS bit either. This will
# cause some inefficiency in sdram access until it is properly fixed.

# We don't need to write out the dramInit0 value for sgram.

$output = &Execute("perl rd.pl DRAMINIT1");

if ($output =~ /SDRAM/) {
    # sdram
    $ENV{SSTH3_DRAMINIT0}=0xc17a9e9;
    &Execute("perl wr.pl draminit0 0xc17a9e9");
}
else {
    # For sgram, don't set numBanks so we can do 8MB sgram boards.
    $ENV{SSTH3_DRAMINIT0}=0x817a9e9;
}

#
# init the chip
#
$output = &Execute("perl 3dvideo.pl");

# 
# determine memory type and size
#
if ( $output =~ /SGRAM/ ) {
    $memType = "sgram";
    if ( $output =~ /4 Mbytes/ ) {
	$memSize = 4;
    } elsif ( $output =~ /8 Mbytes/ ) {
	$memSize = 8;
    } elsif ( $output =~ /16 Mbytes/ ) {
	$memSize = 16;
    } else {
	die "$progName: unable to determine memory size\n";
    }
} elsif ( $output =~ /SDRAM/ ) {
    $memType = "sdram";
    $memSize = 16;
} else {
    die "$progName: unable to determine memory type\n";
}

print "$progName: detected $memSize MBytes of $memType memory\n";

$agp = 1;  # assume AGP board

# run memdiag -- 2 passes
$output = &Execute("perl memdiag.pl -2");
die "$progName: memdiag failed\n" if ($output !~ /passes/);

# run diags
chdir("../diags/bringup");

if ( $agp ) {
    $ENV{HAL_AGP}=1;
    $ENV{HAL_AGP_MEM}=1024;
    die "$progName: ERROR, 4Meg boards not supported yet\n" if ( $memSize == 4 );
    $testlist = "boardcrc" . ( $memType eq "sdram" ? "sd" : $memSize );
    if ( &Execute2("rundiags -f $testlist","Command Stream '.*' FAILED") ) {
	die "$progName: speed vectors failed\n";
    }
}

# get sip data
chdir("../../perl");
$output = &Execute("perl sip_short.pl");

print "$progName: chip passed !\n";

exit 0;

# print usage message
sub Usage {
    print "usage: $progName sgram_vendor [gclk] \n";
}

#
# execute a command and return its output
#
# $output = &Execute($cmd)
#
sub Execute {
    my ($cmd) = @_[0];
    my ($output) = "";

    print "\n";
    print "##############################################################################\n";
    print "# $cmd\n";
    print "##############################################################################\n";
    print "\n";

    open(OUTPUT,"$cmd|")
	|| die "ERROR: can't fork diag: $!";
    $| = 1;    # make unbuffered
    while (<OUTPUT>) {
	$output .= $_;
	print;
    }
    close OUTPUT;
    return $output;
}

#
# execute a command and return pass/fail status
#
# $fail = &Execute2($cmd,$failStr)
#
# execute $cmd
# return 1 if $failStr appears in the output, otherwise 0
#
sub Execute2 {
    my ($cmd) = @_[0];
    my ($failStr) = @_[1];
    my ($fail) = 0;

    print "\n";
    print "##############################################################################\n";
    print "# $cmd\n";
    print "##############################################################################\n";
    print "\n";

    open(OUTPUT,"$cmd|")
	|| die "ERROR: can't fork diag: $!";
    $| = 1;    # make unbuffered
    while (<OUTPUT>) {
	$fail = 1 if ( /$failStr/ );
	print;
    }
    close OUTPUT;
    return $fail;
}

__END__
:endofperl
