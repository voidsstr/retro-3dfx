#
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
# Initialize the device (set up globals)
#
        h3init::h3initMapBoard();

        $membase0_cfg = CFGIN32(0x10) & 0xfffffff0;
        $rombase_cfg = $membase0_cfg + 0x800000;
        CFGOUT32(0x30, $rombase_cfg | 0x1);

        $rombase = 0x800000;

        $addr = hex(shift @ARGV);
        $len  = hex(shift @ARGV);


        for ($i = $addr; $i < $addr + $len; $i = $i + 0x1)
        {

             if(($i & 0xf) == 0x0)
             {
                printf "%4.4x :", $i;
             }

             $data = RD8($rombase + $i);
             printf "%2.2x ", $data;
             if(($i & 0xf) == 0xf)
             {
                printf "\n";
             }
        }


