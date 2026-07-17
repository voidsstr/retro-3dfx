use lib './packages';

use h3init;
use h3regs;
use h3defs;
use pciwrap;
use diags;

h3init::h3initMapBoard();

# clear sdram bit
$dramInit1 = IN32("DRAMINIT1");
$dramInit1 &= ~$h3def{SST_MCTL_TYPE_SDRAM};
OUT32("DRAMINIT1",$dramInit1);
