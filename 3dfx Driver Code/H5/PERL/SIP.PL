#
# PROCESS MONITOR READ BACK
#

use lib './packages';

use h3init;
use pciwrap;
use h3regs;
use h3defs;
use pll;

$deviceID = h3init::h3initMapBoard();

$iters = 1000;

#
# Be sure GRX Clock is @ 125 Mhz
# First save it, so we can restore the current value at the end
# of the script.
#

$origPllCtrl1 = IN32("PLLCTRL1");
$grxFreq = 125;
h3init::h4initPlls($grxFreq,$deviceID);


#
# nand chain performance
#
$min = 0; $max = 0;
for($i=0; $i < $iters; $i++)
{
    $count = 0xfff;

    OUT32("SIPMONITOR", ($count << 16));
    OUT32("SIPMONITOR", ($count << 16));
    OUT32("SIPMONITOR", BIT(28) | ($count << 16));
     
    while($count != 0)
    {
       $count = (IN32("SIPMONITOR") >> 16) & 0xfff;
    }

    $it =  IN32("SIPMONITOR") & 0xffff;
    if($it < $min || !$min) {$min = $it;}
    if($it > $max) {$max = $it;} 

    $occurs[$it]++;
    #print "$it\n ";
}

printf("\n\n--- Process Values (NAND chain) ---\n");
printf("\n sip value: occurences (probability)\n");
$total = 0;
for ($i = 0; $i <= $max; $i++)
{
   if ($occurs[$i] != 0)
   {
       printf("  %4d: %4d (%g)\n", $i, $occurs[$i], $occurs[$i]/$iters);
       $total += $i * $occurs[$i];
   }
}
$E = $total / $iters;
$total = 0;
for ($i = 0; $i <= $max; $i++)
{
   if ($occurs[$i] != 0)
   {
       $total += ($i - $E)*($i - $E)*$occurs[$i];
   }
}
$VAR = $total / $iters;
$SD  = sqrt($VAR);

$total = 0; $N = 0;
for ($i = 0; $i <= $max; $i++)
{
   if (($occurs[$i] != 0) && ($i >= ($E - $SD)) && ($i <= ($E + $SD)))
   {
      $total += $i * $occurs[$i];
      $N += $occurs[$i];
   }

}
$avg = $total / $N;

printf("E   = %g\n", $E);
printf("VAR = %g\n", $VAR);
printf("SD  = %g\n", $SD);
printf("avg = %d (using %d values about E)\n", int($avg), $N);


#
# nor chain performance
#
@occurs = 0;
$min = 0; $max = 0;
for($i=0; $i < $iters; $i++)
{
    $count = 0xfff;
    OUT32("SIPMONITOR", BIT(29) | ($count << 16));
    OUT32("SIPMONITOR", BIT(29) | ($count << 16));
    OUT32("SIPMONITOR", BIT(29) | BIT(28) | ($count << 16));

    while($count != 0)
    {
       $count = (IN32("SIPMONITOR") >> 16) & 0xfff;
    }

    $it =  IN32("SIPMONITOR") & 0xffff;
    if($it < $min || !$min) {$min = $it;}
    if($it > $max) {$max = $it;} 

    $occurs[$it]++;
    #print "$it\n";
}

printf("\n--- Process Values (NOR chain) ---\n");
printf("\n sip value: occurences (probability)\n");
$total = 0;
for ($i = $0; $i <= $max; $i++)
{
   if ($occurs[$i] != 0)
   {
       printf("  %4d: %4d (%g)\n", $i, $occurs[$i], $occurs[$i]/$iters);
       $total += $i * $occurs[$i];
   }
}
$E = $total / $iters;
$total = 0;
for ($i = 0; $i <= $max; $i++)
{
   if ($occurs[$i] != 0)
   {
       $total += ($i - $E)*($i - $E)*$occurs[$i];
   }
}
$VAR = $total / $iters;
$SD  = sqrt($VAR);

$total = 0; $N = 0;
for ($i = 0; $i <= $max; $i++)
{
   if (($occurs[$i] != 0) && ($i >= ($E - $SD)) && ($i <= ($E + $SD)))
   {
      $total += $i * $occurs[$i];
      $N += $occurs[$i];
   }

}
$avg = $total / $N;

printf("E   = %g\n", $E);
printf("VAR = %g\n", $VAR);
printf("SD  = %g\n", $SD);
printf("avg = %d (using %d values about E)\n", int($avg), $N);

# Restore the original value of PLLCTRL1.

OUT32("PLLCTRL1",$origPllCtrl1);
