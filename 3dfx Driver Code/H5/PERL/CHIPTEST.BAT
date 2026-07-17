@rem = '--*-Perl-*--
@echo off
perl -S %0.bat %1 %2 %3 %4 %5 %6 %7 %8 %9
goto endofperl
@rem ';
#line 7
################################################################################
#
# Automated script for testing chips with speed vectors
# 0. run sgram.pl
# 1. run 3dvideo.pl
# 2. run memdiag.pl
# 3. run approprite CRC version of shortdiags testlist
#
################################################################################
# get current name 
$progName = substr($0,rindex($0,"\\")+1);  # basename($0)
$progName = substr($progName,rindex($progName,"/")+1);  # basename($0)
$progName =~ s/\.[bB][aA][tT]$//;          # remove trailing .bat, if any

# defaults
$AVENGER_BRINGUP_FREQ = 100;

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
$ENV{SSTH3_SGRAM_VENDOR}=$vendor;
$ENV{SSTH3_GRXCLOCK}=$gclk;

#
# New memory timings
# DRAMINIT1 = 0x284031
#       SST_DRAM_REFRESH_EN             0x1
#       SST_DRAM_REFRESH_VALUE          0x18
#       SST_SGRAM_USE_INV_SAMPLE        0x1
#       SST_SGRAM_CLK_ADJ               0x8
#       SST_SGRAM_OFLOP_DEL_ADJ         0x2

$ENV{SSTH3_DRAMINIT1}=0x284031;

#
# force SGRAM
#
$output = &Execute("perl sgram.pl");

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

# determine PCI or AGP
#$output = &Execute("cap");
#if ( $output =~ /AGP/ ) {
#    $agp = 1;
#}
#die "$progName: must be run on an AGP board\n" if ( ! $agp );

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
    $testlist = "shortcrc" . ( $memType eq "sdram" ? "sd" : $memSize );
    if ( &Execute2("rundiags -f $testlist","Command Stream '.*' FAILED") ) {
	die "$progName: speed vectors failed\n";
    }
}

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
