#!/usr/local/bin/perl
## 
##

$minfreq = 80;
$maxfreq = 130;

if ($#ARGV > 0) {
        $minfreq = $ARGV[0];
        $maxfreq = $ARGV[1];
}

   $ENV{"SSTH3_GRXCLOCK"} = 50;

print "MEMCLK\n";
   $init = "stress2d -p 10 -r"; 

for ($clk = $minfreq; $clk < $maxfreq; $clk+=5) {
   $ENV{"SSTH3_MEMCLOCK"} = $clk;

   $fail = &run_test($init);
   if($fail)
   {
      print "$clk Fail\n";
   }
   else
   {
      print "$clk Pass\n";
   }

}


   $ENV{"SSTH3_MEMCLOCK"} = 100;
#   $init = "stress7 -FZ -D -d-1 -N4096 -p 10"; 
   $init = "perf -O 6 -p 20"; 

print "GRXCLK\n";
for ($clk = $minfreq; $clk < $maxfreq; $clk+=5) {
   $ENV{"SSTH3_GRXCLOCK"} = $clk;

    
   $fail = &run_test($init);
   if($fail)
   {
      print "$clk Fail\n";
   }
   else
   {
      print "$clk Pass\n";
   }

}

exit;



sub exec_cmd {
    local($cmd) = @_;
    open (CMD, "$cmd |") || die "can't execute a command: $cmd";
    while (<CMD>) {print;}
    close (CMD);
}

sub run_test {
    local ($cmd) = @_;
    local ($error);

#    $cmd = "simple -p 10";

    open(CMD, "$cmd |") || die "can't execute a command: $cmd";

    $error = 0;
    while (<CMD>) {
    $error = 1 if /ERROR/;
    }

    close(CMD);
    $error;
}

