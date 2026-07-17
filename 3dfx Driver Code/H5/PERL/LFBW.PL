use lib '.\packages';

use h3init;
use h3regs;
use h3defs;
use pciwrap;

    # locate board
    $MEMBASE1 = MAPCARD(0x121a, 0x3, 0x2000000, 1, 0);

    $addr = hex($ARGV[0]);
    $data = hex($ARGV[1]);

$PCI_VERBOSE=1;

    if ($ARGV[2]) {

        # Lots o' writes
        $end = hex($ARGV[2]);
        for (; $addr <= $end; $addr += 4) {
            WR32($addr, $data, 1);
        }

     } else {
                
        # Single write
        WR32($addr, $data, 1);
    }
