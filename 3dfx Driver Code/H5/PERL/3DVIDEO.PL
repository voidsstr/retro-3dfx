#
# This is an example file on calling the 
# subroutine set_video_timing.
#
#

use lib './packages';

use h3init;
use pciwrap;
use h3regs;
use h3defs;

#
# Initialize the device (set up globals)
#
        h3init;

#
# Set the timing to the desired resolution
#
        h3init::h3initVideoTiming (640, 480, 60);
#        h3init::h3initVideoTiming (800, 600, 75);

