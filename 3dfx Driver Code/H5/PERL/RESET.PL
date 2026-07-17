#
# read a list of registers, divide them into fields, and display the fields
#
use lib './packages';

use h3init;
use pciwrap;

h3init::h3initMapBoard;
print "\n";

#
# Set bit 19 of Miscinit1 to clean up cmd fifo
#
WR32("MISCINIT1",0x0008000);


WR32("MISCINIT0",0xf3);
WR32("MISCINIT0",0x00);


WR32("MISCINIT1",0x00);
