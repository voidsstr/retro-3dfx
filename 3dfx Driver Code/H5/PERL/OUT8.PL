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
$data = hex(shift @ARGV);
OUT8($addr,$data);

