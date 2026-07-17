use lib './packages';

use h3init;
use h3regs;
use h3defs;
use pciwrap;
use diags;

#
# Initialize the device
#

$PCIVERBOSE=1;

    # locate board
    h3init::h3initMapBoard();
   $gclk = $ARGV[0];
   $mclk = $ARGV[1];
        $ENV{"SSTH3_GRXCLOCK"} = $gclk;
        $ENV{"SSTH3_MEMCLOCK"} = $mclk;

    h3init::h3initPlls($gclk, $mclk);
    print ("Gclk = $ARGV[0]\nMclk = $ARGV[1]\n");


