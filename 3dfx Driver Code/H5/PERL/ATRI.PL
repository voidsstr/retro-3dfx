#
#

use lib './packages';

use h3init;
use pciwrap;
use h3regs;

$PCIVERBOSE = 1;
#
# Initialize the device (set up globals)
#
        h3init;

        $td_base = 0x200000;

        $pixels_out = RD32("FBIPIXELSOUT");
        $pixels_in  = RD32("FBIPIXELSIN");

        print "pixels_in = $pixels_in,   pixels_out = $pixels_out\n";

	# c_xstride: 640
	WR32($td_base +0x1f0,  0x280);
	# fbzmode: enable rgbwrites
#        WR32($td_base +0x110,  0x200);
        WR32("FBZMODE", 0x200);
	# fbzcolorpath: select iterated RGB
	WR32($td_base +0x104,  0x0);

	WR32($td_base + 0x08,0x00001b66); # vA.x
	WR32($td_base + 0x0c,0x0000029e); # vA.y
	WR32($td_base + 0x10,0x000020e2); # vB.x
	WR32($td_base + 0x14,0x0000059f); # vB.y
	WR32($td_base + 0x18,0x00001de0); # vC.x
	WR32($td_base + 0x1c,0x00000b19); # vC.y
	WR32($td_base + 0x20,0x00058ce8); # R
	WR32($td_base + 0x24,0x000eacde); # G
	WR32($td_base + 0x28,0x000850ef); # B
	WR32($td_base + 0x40,0x00000de6); # DRDX
	WR32($td_base + 0x44,0x00000004); # DGDX
	WR32($td_base + 0x48,0x00000baa); # DBDX
	WR32($td_base + 0x60,0x00000a4f); # DRDY
	WR32($td_base + 0x64,0xffffe8e5); # DGDY
	WR32($td_base + 0x68,0xffffffc9); # DBDY
												 
	# triangle command
	WR32($td_base +0x20*4, 0x0027121a);

   $i=0;
	while ($i<3) {
	  if (!(RD32($td_base) & 0x80)) {
                 $i++;
	  }
	}

        $pixels_out = RD32("FBIPIXELSOUT");
        $pixels_in  = RD32("FBIPIXELSIN");
        $fbz = RD32("FBZMODE");

        print "Done drawing triangle ...\n";
        print "pixels_in = $pixels_in,   pixels_out = $pixels_out\n";
        print "fbzmode = $fbz\n";
        
