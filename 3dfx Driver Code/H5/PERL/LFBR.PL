use lib '.\packages';

use h3init;
use h3regs;
use h3defs;
use pciwrap;


    # locate board
    $MEMBASE1 = MAPCARD(0x121a, 0x3, 0x2000000, 1, 0);

    $addr = hex($ARGV[0]);

    if ($ARGV[1]) {

        # Lots o' reads
        $end = hex($ARGV[1]);
        printf("%10x:", $addr);

        $count = 0;
        for (; $addr <= $end; $addr += 4) {
            $data = RD32($addr, 1);
            printf("%10x", $data);
            $count++;
            if ($count==4) {
                $count = 0;
                print("\n");
                if ($addr+4 <= $end) { printf("%10x:", $addr+4); }
            }
        }

     } else {
                
        # Single read
        $data = RD32($addr, 1);
        printf("Mem[%x] = %x\n", $addr, $data);
    }
