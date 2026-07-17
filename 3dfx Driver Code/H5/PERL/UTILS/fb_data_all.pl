#!/usr/local/bin/perl

use lib '.\packages';

use h3init;
use pciwrap;
use h3regs;
use h3defs;



        # Reinit
        h3init;
        h3init::h3initVideoTiming( 640,480, 75 );

        # Turn off video
        $vidProcCfg = 0;
        OUT32("VIDPROCCFG",$vidProcCfg);

$PCIVERBOSE = 1;

        # Disable block writes
        WR32("MISCINIT1", 0x07008001);

        # Clip window 0 (0,0) - (2k, 2k)
        WR32("CLIP0MIN", 0x00000000);
        WR32("CLIP0MAX", 0x08000800);

        WR32("COLORFORE", 0x00000000);
        WR32("COLORBACK", 0xffffffff);

        # set dst base, 16bpp, stride
        WR32("DSTBASEADDR", 0);
        WR32("DSTFORMAT", 0x00030000);
        WR32("DSTXY", 0x00000000);
        WR32("DSTSIZE", 0x0fff0008);

        WR32("PATTERN0ALIAS", 0x00ff00ff);
        WR32("PATTERN1ALIAS", 0x00ff00ff);

# Start Banshee setup for a pattern filled rect

$PCIVERBOSE = 0;

        while (1) {
                # rect mode, rop = f0
                WR32("COMMAND", 0xf0002105);
        }


$PCIVERBOSE = 0;
