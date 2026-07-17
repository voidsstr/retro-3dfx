use lib '.\packages';

use h3init;
use pciwrap;
use h3regs;
use h3defs;

h3init;
h3init::h3initVideoTiming(640, 480, 60);

WR32("DSTBASEADDR", 0x50000);
WR32("SRCFORMAT", 0x30500);
WR32("DSTFORMAT", 0x30500);
WR32("CLIP0MIN", 0);
WR32("CLIP0MAX", 480<<16 + 640);

WR32("VIDDESKTOPSTARTADDR", 0x50000);
WR32("VIDDESKTOPOVERLAYSTRIDE", 0x500);
#WR32("VIDPROCCFG", 0x40c81);
WR32("VIDPROCCFG", 0x40081);

