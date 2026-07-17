#
# PROCESS MONITOR EXCERCISE
#

# Usage: perl sipper.pl [-excel]
#      -excel: enables excel output

### overhead

use lib './packages';
use h3init;
use pciwrap;
use h3regs;
use h3defs;

### init board

h3init::h3initMapBoard;

### init program

$iters = 1000;
$clks = "050 060 070 080 090 100 110 120";
$passes = 1;
$settled = 15;

### main program

foreach $pass (1..$passes) {
   foreach $clk (split(" ", $clks)) {
      ### clocks
      $mclk = $clk;
      $gclk = $clk;
      $ENV{"SSTH3_GRXCLOCK"} = $mclk;
      $ENV{"SSTH3_MEMCLOCK"} = $gclk;
      h3init::h3initPlls($gclk, $mclk);
      ### loop on sip movement
      $avg = 0;
      $lastavg = 0;
      $steady = 0;
      $loops = 0;
      while ($steady < $settled) {
         $loops++;
         $min = 0;
         $max = 0;
         $tot = 0;
         for($i=0; $i < $iters; $i++) {
            $refcnt = 0xfff;
            OUT32("SIPMONITOR", ($refcnt << 16));
            OUT32("SIPMONITOR", ($refcnt << 16));
            OUT32("SIPMONITOR", BIT(28) | ($refcnt << 16));
            while($refcnt != 0) {
               $refcnt = (IN32("SIPMONITOR") >> 16) & 0xfff;
            }
            $sip =  IN32("SIPMONITOR") & 0xffff;
            $tot =$tot + $sip;
            if($sip < $min || !$min) {$min = $sip;}
            if($sip > $max) {$max = $sip;}
         }
         $avg = int($tot/$iters);
         print "#sipper (nand) moving: Pass = $pass, Clk = $clk, Min = $min, Max = $max, Avg = $avg\n";
         $steady = ($avg == $lastavg)? $steady+1 : 0;
         $lastavg = $avg;
      }
      $rec_avg{$pass,$clk} = $avg;
      $rec_loops{$pass,$clk} = $loops;
   }
}

$clktot = 0;
$clkcnt = 0;
$tot = 0;
$min = 0;
$max = 0;

print "!sipper summary:\n";
foreach $pass (1..$passes) {
   $lastclk = 0;
   $lastavg = 0;
   foreach $clk (split(" ", $clks)) {
      $clkcnt++;
      $avg = $rec_avg{$pass,$clk};
      $lastclk = $clk if (!$lastclk);
      $lastavg = $avg if (!$lastavg);
      $dclk = $clk-$lastclk;
      $pclk = ($dclk)? int (100*($dclk)/$clk) : 0;
      $davg = $lastavg-$avg;
      $pavg = ($davg)? (100*($davg)/$avg) : 0;
      $pavg =~ s/(\.\d\d)\d*/$1/;
      print "! Pass = $pass, Clk = $clk, Avg = $avg, %clk-delta = $pclk, %avg-delta = $pavg\n";
      $lastclk = $clk;
      $lastavg = $rec_avg{$pass,$clk};
      if ($pass == $passes) {
         $clktot = $clktot + $clk;
         $tot = $tot + $avg;
         if($avg < $min || !$min) {$min = $avg;}
         if($avg > $max) {$max = $avg;}
      }
   }
}

$clkavg = int($clktot/$clkcnt);
$avg = int($tot/$clkcnt);

print "Final Pass Averages: AvgClk = $clkavg, minavg = $min, maxavg = $max, avgavg = $avg\n";

$mclk = 100;
$gclk = 100;
$ENV{"SSTH3_GRXCLOCK"} = $mclk;
$ENV{"SSTH3_MEMCLOCK"} = $gclk;
h3init::h3initPlls($gclk, $mclk);

