#
#

use lib './packages';

use h3init;
use pciwrap;
use h3regs;

#$PCIVERBOSE = 1;

#
# Initialize the device (set up globals)
#
    h3init;

    while (1) {
    for($i =0 ;$i < 0x3f; $i = $i + 4)
    {
        WR32($i, $i, 1);
    }
    for ($i = 0; $i < 0x3f; $i = $i + 4) {
        $data = RD32( $i, 1);
#        printf " i = 0x%lx , data = 0x%lx\n", $i, $data;

        }
     }

