@rem = '--*-Perl-*--
@echo off
perl -S %0.bat %1 %2 %3 %4 %5 %6 %7 %8 %9
goto endofperl
@rem ';
#line 7
################################################################################
# 
# Automated script for testing chips with speed vectors
# 1. run memdiag.pl
# 2. run approprite CRC version of shortdiags testlist
#
# $Revision: 5 $
#
################################################################################

# get current name 
$progName = substr($0,rindex($0,"\\")+1);  # basename($0)
$progName = substr($progName,rindex($progName,"/")+1);  # basename($0)
$progName =~ s/\.[bB][aA][tT]$//;          # remove trailing .bat, if any

# defaults
$DEFAULT_FREQ = 90;
$DEFAULT_VENDOR = "GENERIC";

# parse command-line arguments
$cmdline = $progName . " " . join(" ",@ARGV);

if ( @ARGV == 0 ) {
    $vendor = $DEFAULT_VENDOR;
    $gclk = $DEFAULT_FREQ;
}
elsif ( @ARGV == 1 ) {
    $vendor = shift @ARGV;
    $gclk = $DEFAULT_FREQ;
} elsif ( @ARGV == 2 ) {
    $vendor = shift @ARGV;
    $gclk = shift @ARGV;
} else {
    &Usage();
    exit 1;
}

$ENV{SSTH3_SGRAM_VENDOR}=$vendor;
$ENV{SSTH3_GRXCLOCK}=$gclk;
if ($gclk >= 100) {
    $ENV{SSTH3_DRAMINIT1}=0x530031;
}
else {
    $ENV{SSTH3_DRAMINIT1}=0x56c031;
}

# 
# Run 2 passes of memdiag, and use it's output to see what our memType
# and memSize are. We need this info to select a crc file.
#

$output = &Execute("perl memdiag.pl -2");
die "$progName: memdiag failed\n" if ($output !~ /passes/);

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
    print "usage: $progName [sgram_vendor] [gclk] \n";
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
