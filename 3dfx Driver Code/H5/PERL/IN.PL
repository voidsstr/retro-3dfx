#
#

use lib './packages';

use h3init;
use pciwrap;
use h3regs;
use h3defs;
use pll;

h3init::h3initMapBoard();

$addr = hex(shift @ARGV);
$data = IN32($addr);
printf "%lx  ==  %lx\n", $addr, $data;

