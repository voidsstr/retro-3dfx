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

# do a 2d blit!

# set clip regs to 640x480
        WR32( "CLIP0MIN", 0);
        WR32( "CLIP0MAX", 0x01e00280);

        # set dstbase addr to 0
        WR32( 0x10, 0);

        # set dstFormat to 16bpp, 640*2 (bytes) stride
        WR32( 0x14, 0x00030000 + 640*2);

        # set commandExtra to zero
        WR32( "COMMANDEX", 0);

        # set foreground color to 0x1234
        WR32( "COLORFORE", 0x1234);

        # draw a 100x100 solid rectangle
        WR32( "DSTXY", 0);
        WR32( "DSTSIZE", 0x00640064);
        WR32( "COMMAND", 0xcc000105);

