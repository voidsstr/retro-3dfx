@rem = '--*-Perl-*--
@echo off
perl -S %0.bat %1 %2 %3 %4 %5 %6 %7 %8 %9
goto endofperl
@rem ';
#line 7

#
# agpmove.bat
# Run the agpmovereg diag using the specified pattern.
#

if (@ARGV != 2) {

    printf "\nUsage: agpmove <input_pattern_file> <number_of_passes_to_run_test>\n\n";
    printf "The first lines of <input_pattern_file> should be:\n";
    printf "PATTERN_MULTIPLIER = N\n";
    printf "where N = the number of times to duplicate each pattern.\n\n";
    printf "PRECHARGE_PATTERN = <HEX value>\n";
    printf "PRECHARGE_INTERVAL = N\n";
    printf "The optional precharge pattern will be asserted once between every N\n";
    printf "pairs of the background and foreground data\n\n";
    printf "The remaining lines of <input_pattern_file> should show the background and\n";
    printf "foreground HEX words of each pattern you want to run, ie:\n";
    printf "0x00000000 0xfffffffe\n\n";
    die;
}
else {
    $inputPatternFile = $ARGV[0];
    $numPasses = $ARGV[1];
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
$DEFAULT_FREQ = 90;
$DEFAULT_VENDOR = "GENERIC";

#
# set clock speeds
#

$gclk = $DEFAULT_FREQ;

# Set the pll frequency.

$ENV{SSTH3_SGRAM_VENDOR}=$DEFAULT_VENDOR;
$ENV{SSTH3_GRXCLOCK}=$gclk;

# Set dramInit1 env var

if ($gclk >= 100) {
    $ENV{SSTH3_DRAMINIT1}=0x530031;
}
else {
    $ENV{SSTH3_DRAMINIT1}=0x56c031;
}

#
# Produce the pattern file.
#

$pid = "$$";
$pid =~ s/-+//;
$outputPatternFile = "pattern_${pid}.txt";
# makePattern does a chdir to ../diags/bringup
makePattern($inputPatternFile,$outputPatternFile);

#
# Run the diag
#

$cmd = "agpmovereg -W14 -h -p $numPasses -xn -xf$outputPatternFile -E10000 -l agpmovereg_1.log";
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
    my($PRECHARGE_PATTERN,$PRECHARGE_MULTIPLIER);

    # $PATTERN_MULTIPLIER is the number of cycles to duplicate each
    # pattern.

    $patternArray = [];

    # Specify the patterns. Each pattern will be used in the order
    # listed here.
    $PRECHARGE_INTERVAL = -1; # Means don't do precharge.
    open PAT, "<$inputPatternFile" or die "Can't open $inputPatternFile\n";
    while (<PAT>) {
	chop;
	if (/^#/) {
	    ;
	}
	elsif (/PATTERN_MULTIPLIER/) {
	    ($PATTERN_MULTIPLIER) = /\s*PATTERN_MULTIPLIER\s+=\s+(\d+)/;
	}
	elsif (/PRECHARGE_PATTERN/) {
	    ($PRECHARGE_PATTERN) = /\s*PRECHARGE_PATTERN\s+=\s+(0x[A-Fa-f\d]+)/;
	}
	elsif (/PRECHARGE_INTERVAL/) {
	    ($PRECHARGE_INTERVAL) = /\s*PRECHARGE_INTERVAL\s+=\s+(\d+)/;
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
	writePattern(PAT,$patternArray,$i,$PATTERN_MULTIPLIER,$PRECHARGE_PATTERN,$PRECHARGE_INTERVAL);
    }
    close(PAT);
}

# subroutine defs

# Write $iterations iterations of pattern $patterNum to the file
# $filePtr. Insert the precharge pattern every $precharge_interval pairs of the
# background/foreground data. If $precharge_interval is <= 0, don't do a
# precharge.

sub writePattern
{
    my($filePtr,$patternArray,$patternNum,$iterations,$precharge_pat,$precharge_interval) = @_;
    my($i,$pi);

    $pi = $precharge_interval;
    for ($i = 0; $i < $iterations; $i++) {
	if (($pi == $precharge_interval) && ($pi > 0)) {
	    printf $filePtr "$precharge_pat\n";
	}
	$pi--;
	if ($pi <= 0) {
	    $pi = $precharge_interval;
	}
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
