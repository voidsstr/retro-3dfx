# A perl script to run the glide3 conformance tests.
#
# On Win32, we've been using ActivePerl to run this. You can
# get it here: http://www.activestate.com/ActivePerl/default.htm
#


package ct; #conformance tests
use strict 'vars';

$ct::logFileName = "./runtests.log"; # save stdout of tests and report here

# default args applied to all self-checking tests
$ct::defaultSCTestArgs = "-wid 100 -hei 100 -xylimit 20";  

# Args applied to all image-based tests
$ct::defaultIBTestArgs = "-wid 50 -hei 50 -xylimit 20"; 

@ct::commandLineArgs = @ARGV;
@ct::testListList = ("self-checking", "image-based"); #default to all tests

$ct::useSubdirs = 0; # don't use subdirs in path by default

$ct::totalNumPasses = 0;
$ct::totalNumFailures  = 0;
@ct::failedTestList;
$ct::totalTestsRun = 0;

##
## The list of self-checking tests
## 
@ct::selfCheckingTests = (
  "state/color-combine",
  "state/alpha-combine",
  "state/depth-wbuffer-sc",
  "state/depth-zbuffer-sc",
  "state/depth-wbias",
  "state/depth-zbias",
  "state/lfb -cf ARGB",
  "state/lfb-region",
  "raster/ras-tri",
  "raster/ras-tri -aa", 
  "raster/ras-tri -cw",
  "raster/ras-tri -aa -cw",
  "raster/ras-tri-2",
  "raster/ras-tri-2 -wc",
  "raster/ras-tri-2 -cw",
  "raster/ras-tri-2 -wc -cw",
#  "raster/ras-point -aa",      
#  "raster/ras-tri-2 -aa",
#  "raster/ras-tri-2 -aa -cw",
#  "raster/ras-tri-2 -wc -aa",
#  "raster/ras-tri-2 -wc -aa -cw",
#  "raster/ras-line -aa",
#  "raster/ras-line -cw",
#  "raster/ras-line -cw -aa",
  "raster/ras-line",
  "raster/ras-point",
  "raster/ras-va -mode points",
  "raster/ras-va -mode lines",
  "raster/ras-va -mode line_strip",
  "raster/ras-va -mode triangles",
  "raster/ras-va -mode triangle_strip",
  "raster/ras-va -mode triangle_strip -wc",  # world coords
  "raster/ras-va -mode triangle_fan",
  "raster/ras-va -mode polygon",
  "raster/ras-va -contig -mode points",
  "raster/ras-va -contig -mode lines",
  "raster/ras-va -contig -mode line_strip",
  "raster/ras-va -contig -mode triangles",
  "raster/ras-va -contig -mode triangle_strip",
  "raster/ras-va -contig -mode triangle_fan",
  "raster/ras-va -mode points -aa",
  "raster/ras-va -mode points -cw",
  "raster/ras-va -mode points -aa -cw",
  "raster/ras-va -mode lines -aa",
  "raster/ras-va -mode lines -cw",
  "raster/ras-va -mode lines -aa -cw",
  "raster/ras-va -mode line_strip -aa",
  "raster/ras-va -mode line_strip -cw",
  "raster/ras-va -mode line_strip -aa -cw",
  "raster/ras-va -mode triangles -aa",
  "raster/ras-va -mode triangles -cw",
  "raster/ras-va -mode triangles -aa -cw",
  "raster/ras-va -mode triangle_strip -aa",
  "raster/ras-va -mode triangle_strip -cw",
  "raster/ras-va -mode triangle_strip -aa -cw",
  "raster/ras-va -mode triangle_strip -wc -aa",
  "raster/ras-va -mode triangle_strip -wc -cw",
  "raster/ras-va -mode triangle_strip -wc -aa -cw",
  "raster/ras-va -mode triangle_fan -aa",
  "raster/ras-va -mode triangle_fan -cw",
  "raster/ras-va -mode triangle_fan -aa -cw",
  "raster/ras-va -mode polygon -aa",
  "raster/ras-va -mode polygon -cw",
  "raster/ras-va -mode polygon -aa -cw",
  "raster/ras-va -contig -mode points -aa",
  "raster/ras-va -contig -mode points -cw",
  "raster/ras-va -contig -mode lines -aa",
  "raster/ras-va -contig -mode lines -cw",
  "raster/ras-va -contig -mode lines -aa -cw",
  "raster/ras-va -contig -mode line_strip -aa",
  "raster/ras-va -contig -mode line_strip -cw",
  "raster/ras-va -contig -mode line_strip -aa -cw",
  "raster/ras-va -contig -mode triangles -aa",
  "raster/ras-va -contig -mode triangles -cw",
  "raster/ras-va -contig -mode triangles -aa -cw",
  "raster/ras-va -contig -mode triangle_strip -aa",
  "raster/ras-va -contig -mode triangle_strip -cw",
  "raster/ras-va -contig -mode triangle_strip -aa -cw",
  "raster/ras-va -contig -mode triangle_fan -aa",
  "raster/ras-va -contig -mode triangle_fan -cw",
  "raster/ras-va -contig -mode triangle_fan -aa -cw",
  "raster/ras-va -contig -mode polygon -aa",
  "raster/ras-va -contig -mode polygon -cw",
  "raster/ras-va -contig -mode polygon -aa -cw", 
  "raster/ras-va -contig -mode polygon",
  "raster/ras-va-cont -mode strip",
  "raster/ras-va-cont -mode strip -aa",
  "raster/ras-va-cont -mode strip -cw",
  "raster/ras-va-cont -mode strip -aa -cw",
  "raster/ras-va-cont -mode fan",
  "raster/ras-va-cont -mode fan -aa",
  "raster/ras-va-cont -mode fan -cw",
  "raster/ras-va-cont -mode fan -aa -cw",
  "raster/ras-va-cont -contig -mode strip",
  "raster/ras-va-cont -contig -mode strip -aa",
  "raster/ras-va-cont -contig -mode strip -cw",
  "raster/ras-va-cont -contig -mode strip -aa -cw",
  "raster/ras-va-cont -contig -mode fan",
  "raster/ras-va-cont -contig -mode fan -aa",
  "raster/ras-va-cont -contig -mode fan -cw",
  "raster/ras-va-cont -contig -mode fan -aa -cw",
  "control/error-callback",
  "control/query",
  "control/query-state",
  "control/query-res",
  "control/simple-clear",
  "control/buffer-swap"    # also generates images
);


##
## The list of image based tests.
##
@ct::imageBasedTests = (
 # "image based tests"
  "control/clear",
  "raster/simple-ras",
  "raster/simple-ras -aa",
  "raster/simple-ras -wc",
  "raster/simple-ras -wc -aa",
  "raster/simple-interp-alpha",
  "raster/simple-interp-depth",
  "raster/simple-interp-depth -z",
  "raster/simple-interp-rgb",
  "raster/simple-interp-rgb -aa",
  "raster/simple-interp-tex",
  "raster/simple-interp-tex -aa",
  "raster/simple-interp-tex -wc",
  "raster/simple-interp-tex -wc -aa",
  "texture/texture-lod",
  "texture/texture-bias",
  "texture/texture-mode",
  "texture/texture-wrap",
  "texture/texture-filter",
  "texture/texture-combine",
  "texture/texture-combine-detail",
  "texture/texture-combine-lod",
  "texture/texture-combine-multi",
  "texture/texture-load-mip",
  "texture/texture-load-level",
  "texture/texture-load-partial",
  "texture/texture-load-format",
  "texture/texture-load-palette",
  "texture/texture-load-simple",
  "texture/texture-load-multi",
  "texture/texture-load-ncc.exe",
  "texture/texture-interp",
  "texture/texture-interp -atc",
  "texture/texture-interp-large",
  "texture/texture-interp-large -atc",
  "texture/texture-interp-2tmu",
  "texture/texture-interp-2tmu -atc",
  "texture/texture-interp-2tmu -cw",
  "texture/texture-interp-2tmu -cw -atc",
  "texture/texture-interp-2tmu-large",
  "texture/texture-interp-2tmu-large -atc",
  "texture/texture-interp-3df",
  "state/backface-cull",
  "state/flex-coords",
  "state/flex-coords -wc",
  "state/flex-coords-depth-color",
  "state/flex-coords-depth-color -z",
  "state/flex-coords-depth-color -wc",
  "state/flex-coords-depth-color -wc -z",
  "state/flex-coords-texture",
  "state/flex-coords-texture -2tmus",
  "state/flex-coords-texture -wc",
  "state/flex-coords-texture -wc -2tmus",
  "state/flex-coords-texture-fog",
  "state/flex-coords-texture-fog -2tmus",
  "state/flex-coords-texture-fog -wc",
  "state/flex-coords-texture-fog -wc -2tmus",
  "state/color-constant",
  "state/color-dither",
  "state/color-chroma",
  "state/fog",
  "state/depth-wbuffer",
  "state/depth-zbuffer",
  "state/alpha-blend",
  "state/alpha-buffer",
  "state/alpha-test",
  "state/alpha-funky",
  "extensions/ext-chroma",
  "extensions/ext-fogcoord",
  "extensions/ext-palette",
  "extensions/ext-tex-chroma",
  "extensions/ext-texture-mirror",
  "control/buffer-swap",     # Also some self-checking
  "state/disable-effects",
);

# For debugging, add tests here and then run with -debugtests
@ct::debugTests = (
  "control/buffer-swap",
);


##
##  Utility functions
##

# Show the usage of this script
sub ShowUsage {   
  print <<EOM;
A perl script for running the Glide conformance tests. At the moment, 
some tests are image based and some tests have been converted to be 
self-checking. See the arguments below for info on how to run the 
sets of tests. 

Environment variables:

  GLIDE_CONFORM_OUTPUTDIR - where the images and conform.log file
                            will end up. Defaults to '.'. If you set
                            it, make sure the directory exists.

The output of this script, and the stdout output of the tests that are
run, are logged in $ct::logFileName.

EOM
  print "Usage: perl runtests.pl [-h] [-ta \"args\"] [-itests] [-stests] [-all] [-dirs] ";
  print "[-file <name>]\n";
  print "        -h        : Print this usage information.\n";
  print "        -ta \"args\": Pass \"args\" to tests. Double quotes are required.\n"; 
  print "                      These arguments replace the default test args.\n";
  print "        -itests   : Run the image-based tests.\n";
  print "        -stests   : Run the self-checking tests.\n";
  print "        -all      : Run all the tests. (default)\n";
  print "        -dirs     : Prepend the tests subdirectory to its name. \n";
  print "                    Use this if you're running from conform/tests\n";
  print "        -file <name> : use args from file <name>\n";
  exit 0;
}

#Default to image based args
$ct::globalTestArgs = $ct::defaultIBTestArgs;

# Process the command line arguments
sub ProcessCommandLineArgs {
  my @args = @_;
  while ($_ = $args[0], /^-/) {
    shift @args;
    last if /^--$/;
    if (/^-h/)     { ShowUsage }
    if (/^-ta/) { $ct::globalTestArgs = $args[0] }
    if (/^-itests/) { 
      @ct::testListList = ("image-based");
      $ct::globalTestArgs = $ct::defaultIBTestArgs;
    } 
    if (/^-stests/) { 
      @ct::testListList = ("self-checking");
      $ct::globalTestArgs = $ct::defaultSCTestArgs;
    }
    if (/^-debugtests/) { 
      @ct::testListList = ("debug-tests");
      $ct::globalTestArgs = $ct::defaultIBTestArgs;
    }
    if (/^-all/) { @ct::testListList = ("self-checking", "image-based") }
    if (/^-dirs/) { $ct::useSubDirs = 1 }
    if (/^-file/) { 
      $ct::globalTestArgs = "-f " . $args[0];  
    }
  }
}


# Run a list of tests
sub RunTestList {
  my @testList = @_;
  my $lastLine = "";
  my $lastTestName;

  foreach my $test (@testList) {
    my @path = split ('/', $test); # only works for 'subdir/testname'
    $lastTestName = $path[1];

    my $command = "$path[1] $ct::globalTestArgs";

    # Check if we want to prepend the subdirectory to the testname
    if ($ct::useSubDirs != 0) {
       $lastTestName = "$path[0]/$path[1]";
       $command = "$test $ct::globalTestArgs";
    }

    my $os = "==>: $command\n";
    print $os; print LOG $os; $os = "";
  
    open (TEST, "$command |") or die "couldn't run \"$command\"";
    while (<TEST>) {
      print $_;
      print LOG $_;
      $lastLine = $_; # save the last line of output
    }
    close (TEST);
  
    # This is icky. There's undoubtedly a better way...
    chop $lastLine;
    if ($lastLine =~ /Test Completed/) {
      ++$ct::totalTestsRun; # only if we succesfully ran
    }

    if ($lastLine =~ /reported/) { # If we reported any passes/fails...
      my $passes = $lastLine;
      my $fails = $lastLine;
      # I suck at perl regexps and string parsing
      $passes =~ s/^.*reported //;
      $passes =~ s/ .*$//;
      $fails =~ s/^.* and //;
      $fails =~ s/ .*$//;
      if ($fails != 0) {
        # If the test had some failures, add it to a list of failed tests
        push @ct::failedTestList, $lastTestName;
      }
      $ct::totalNumPasses += $passes;
      $ct::totalNumFailures += $fails;
      $ct::failsByTest{$lastTestName} = $fails;
      $ct::passesByTest{$lastTestName} = $passes;
    }
  }
}

##
## The real work starts here!
##

sub RunTheTests {

  # Open the log file for appending.
  open (LOG, ">>$ct::logFileName") 
    or die "couldn't open log file \"$ct::logFileName\"";

  my $now = localtime;

  # Record what we're doing.
  my $os = "";
  $os .= "\n";
  $os .= "#################################################################\n";
  $os .= "##\n";
  $os .= "## Run started: $now\n";
  $os .= "##  Command line args: @ct::commandLineArgs\n";
  $os .= "##  Global test args : $ct::globalTestArgs\n";
  $os .= "##\n";
  $os .= "#################################################################\n";

  print $os; print LOG $os; $os = "";
 
  foreach my $testList (@ct::testListList) {
    if ($testList =~ /self-checking/) {
      $os .= "\n##\n";
      $os .= "## Self Checking Tests\n";
      $os .= "##\n";
      print $os; print LOG $os; $os = "";
      RunTestList (@ct::selfCheckingTests);
    }
    if ($testList =~ /image-based/) {
      $os .= "\n##\n";
      $os .= "## Image Based Tests\n";
      $os .= "##\n";
      print $os; print LOG $os; $os = "";
      RunTestList (@ct::imageBasedTests);
    }
    if ($testList =~ /debug-tests/) {
      $os .= "\n##\n";
      $os .= "## Debug Tests\n";
      $os .= "##\n";
      print $os; print LOG $os; $os = "";
      RunTestList (@ct::debugTests);
    }
  }

  
  #print "Image Based Tests\n";
  #RunTestList (@ct::imageBasedTests);
  
  $now = localtime;
  
  $os .= "\n";
  $os .= "#################################################################\n";
  $os .= "##\n";
  $os .= "## Run completed: $now\n";
  $os .= "##\n";
  $os .= "## $ct::totalTestsRun tests were run.\n";
  $os .= "## $ct::totalNumPasses passes were reported.\n";
  $os .= "## $ct::totalNumFailures failures were reported.\n";
  
  if ($ct::totalNumFailures > 0) {
    $os .= "##\n";
    $os .= "## The following tests reported failures:\n";
    foreach my $testName (@ct::failedTestList) {
      $os .= "##   $testName [$ct::failsByTest{$testName} failures, ";
      $os .= "$ct::passesByTest{$testName} passes]\n";
    }
  }
  $os .= "##\n";
  $os .= "#################################################################\n";

  print $os; print LOG $os; $os = "";
  close (LOG);
}


# Run 'em!
ProcessCommandLineArgs @ARGV;
RunTheTests;

 
