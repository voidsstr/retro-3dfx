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
      h3init::h3initVideoTiming (1280,1024,60);

