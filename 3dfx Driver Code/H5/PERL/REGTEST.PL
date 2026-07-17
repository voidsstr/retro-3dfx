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

        for($i = 0; $i < 0xb0; $i = $i + 4)
        {

                $status = IN32( $i);
                printf "register %x =  %lx\n", $i , $status;
        }

        for($i = 0xe0; $i < 0xff; $i = $i + 4)
        {

                $status = IN32( $i);
                printf "register %x =  %lx\n", $i , $status;
        }

#
# 2D registers
#
        WR32( 0x10003c, 0xdeadbeef);
        WR32( 0x200140, 0xbabecafe);

        $status = RD32( 0x10003c);
        printf "register 2D =  %lx\n", $status;


        $status = RD32(  0x200140);
        printf "register 3D =  %lx\n", $status;

        WR32( 0x200104, 0xffffffff);
        $status = RD32( 0x200104);
        printf "register 3D (default) =  %lx\n", $status;

