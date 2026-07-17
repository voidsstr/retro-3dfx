#
# This is an example file on calling the 
# subroutine set_video_timing.
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


#
# Set the timing to the desired resolution
#
        h3init::h3initVideoTiming (640, 480, 60);



#
# Fix up blank so it's off
#

#        OUT8(0xd4, 0x11);
#        $aa = IN8(0xd5);

#        OUT8 (0xd5, $aa & 0x7f);

        #
        # Hblank
        #
#        OUT8 (0xd4, 0x8002);
#        OUT8 (0xd4, 0x8003);

        #
        # Vblank
        #
#        OUT8 (0xd4, 0x8015);



#  exit;
$i =0;

#  while(1)
  {

   for($dacaddr= 0; $dacaddr < 0xff; $dacaddr = $dacaddr + 1)
   {
      OUT32("DACADDR", 0x0);
      IN32("DACADDR");
      OUT32("DACDATA", 0xffffff);
      IN32("DACDATA");

    }

    $i = $i + 0x00000f;

  }

