#
#

use lib './packages';

use h3init;
use pciwrap;
use h3regs;
use h3defs;
use pll;

if (@ARGV != 1) {
    die "Usage: perl gpll.pl <freq>\n";
}
else {
    $grxFreq = $ARGV[0];
}

$deviceID = h3init::h3initMapBoard();

h3init::h4initPlls($grxFreq,$deviceID);

