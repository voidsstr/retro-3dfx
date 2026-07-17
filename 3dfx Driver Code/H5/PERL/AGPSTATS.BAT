@rem = '--*-Perl-*--
@echo off
perl -S %0.bat %1 %2 %3 %4 %5 %6 %7 %8 %9
goto endofperl
@rem ';
#line 7

#
# agpstats.bat
# Run the agpmovereg program in the error statistics gathering mode.
#

if (@ARGV != 3) {
    die "\nUsage: agpstats <stats_file> <num_passes> <report_interval>\n";
}
else {
    $statsFile = $ARGV[0];
    $numPasses = $ARGV[1];
    $reportInterval = $ARGV[2];
}

# get current name 
$progName = substr($0,rindex($0,"\\")+1);  # basename($0)
$progName = substr($progName,rindex($progName,"/")+1);  # basename($0)
$progName =~ s/\.[bB][aA][tT]$//;          # remove trailing .bat, if any

# set env variables
$ENV{HAL_HW} = 1;
$ENV{HAL_VIDEO} = 0;
$ENV{SST_DUALHEAD} = 1;
$ENV{HAL_AGP} = 1;
$ENV{HAL_AGP_MEM} = 1024;
$ENV{GDBG_LEVEL} = 0;

# defaults
$AVENGER_BRINGUP_FREQ = 90;
$vendor = "SAMSUNG";
$gclk = $AVENGER_BRINGUP_FREQ;

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

$ENV{SSTH3_DRAMINIT1}=0x56c031;

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

#
# Run the diag
#

# I copied L:\tdfx\devel\h4\diags\trex\agpmovereg to agpmovereg2 on M: so
# I could use it here without disturbing the original program.
# 02-08-99

$cmd = "agpmovereg2 -W14 -h -xn -xe$statsFile -xi$reportInterval -p $numPasses";
$output = &Execute($cmd);

#
# Cleanup
#

unlink $outputPatternFile;

exit;

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
# execute a command and save it's output in a log file
#
# &Execute3($cmd,$log)
#
sub Execute3 {
    my ($cmd) = @_[0];
    my ($log) = @_[1];

    print "\n";
    print "##############################################################################\n";
    print "# $cmd\n";
    print "##############################################################################\n";
    print "\n";

    open(OUTPUT,"$cmd|")
	|| die "ERROR: can't fork diag: $!";
    $| = 1;    # make unbuffered
    open(LOG,"> $log")
	|| die "ERROR: can't open $log: $!";
    while (<OUTPUT>) {
	print $_;
	print LOG $_;
    }
    close OUTPUT;
    close LOG;
}


sub makePattern
{
    my($inputPatternFile,$outputPatternFile) = @_;
    my($PATTERN_MULTIPLIER,$patternArray,$i,$bgnd,$fgnd);

    # $PATTERN_MULTIPLIER is the number of cycles to duplicate each
    # pattern.

    $patternArray = [];

    # Specify the patterns. Each pattern will be used in the order
    # listed here.

    open PAT, "<$inputPatternFile" or die "Can't open $inputPatternFile\n";
    while (<PAT>) {
	chop;
	if (/PATTERN_MULTIPLIER/) {
	    ($PATTERN_MULTIPLIER) = /\s*PATTERN_MULTIPLIER\s+=\s+(\d+)/;
	}
	elsif (/\s*0x[A-Fa-f\d]+\s+0x[A-Fa-f\d]+/) {
	    ($bgnd,$fgnd) = split;
	    initPattern($patternArray,$bgnd,$fgnd);
	}
	elsif (!/^\s*$/) {
	    die "Unrecognized line in $inputPatternFile: $.\n";
	}
    }
    close(PAT);

    chdir("../diags/bringup");

    open PAT, ">$outputPatternFile" or die "Can't open $outputPatternFile\n";
    for ($i = 0; $i < @$patternArray; $i++) {
	writePattern(PAT,$patternArray,$i,$PATTERN_MULTIPLIER);
    }
    close(PAT);
}

# subroutine defs

# Write $iterations iterations of pattern $patterNum to the file
# $filePtr.

sub writePattern
{
    my($filePtr,$patternArray,$patternNum,$iterations) = @_;
    my($i);

    for ($i = 0; $i < $iterations; $i++) {
	printf $filePtr "$patternArray->[$patternNum]->{bgnd}\n";
	printf $filePtr "$patternArray->[$patternNum]->{fgnd}\n";
    }
    printf $filePtr "#\n";
}

#
# initPattern
# Write the specified pattern to patternArray. The pattern will be
# written in the order bgnd_string, fgnd_string.

sub initPattern
{
    my($patternArray,$bgnd_string,$fgnd_string) = @_;
    my($patternHash);

    $patternHash = {};
    $patternHash->{bgnd} = $bgnd_string;
    $patternHash->{fgnd} = $fgnd_string;
    push(@$patternArray,$patternHash);
}
