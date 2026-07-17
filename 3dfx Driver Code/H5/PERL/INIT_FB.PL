use lib '.\packages';

use h3init;
use h3regs;
use h3defs;
use pciwrap;

$PCI_VERBOSE=1;

 h3init::h3initMapBoard();

$startaddr = 0x0;
$screen_x = 512;
$screen_y = 384;
$refresh = 60;

# Set video timing
 h3init::h3initVideoTiming ($screen_x, $screen_y, $refresh);

# fullscreen desktop, no overlay
$vidProcCfg = IN32("VIDPROCCFG");
$vidProcCfg &= ~$h3def{SST_OVERLAY_EN};
$vidProcCfg |= $h3def{SST_DESKTOP_EN};
$vidProcCfg |= $h3def{SST_DESKTOP_PIXEL_RGB32};
OUT32("VIDPROCCFG",$vidProcCfg);

$pixel_size = 4;
$stride = $screen_x * $pixel_size;
$vidDesktopOverlayStride = IN32("VIDDESKTOPOVERLAYSTRIDE");
$vidDesktopOverlayStride &= ~$h3def{SST_DESKTOP_LINEAR_STRIDE};
$vidDesktopOverlayStride |= $stride << $h3def{SST_DESKTOP_STRIDE_SHIFT};
OUT32("VIDDESKTOPOVERLAYSTRIDE",$vidDesktopOverlayStride);

$vidDesktopStartAddr = $startaddr;
OUT32("VIDDESKTOPSTARTADDR",$vidDesktopStartAddr);

#
# draw a pattern into memory
#

$data0 = 0xff0000; # try 32-bpp first
$data1 = 0x00ff00; # try 32-bpp first
$data2 = 0x0000ff; # try 32-bpp first
$endy = $screen_y;
$endx = $screen_x;
$endx0 = $endx / 3;
$endx1 = $endx * 2 / 3;
$startx = 0; #$endx/2;
$starty = 0; #$endy/2;

printf("endx0 = %d\n", $endx0);
printf("endx1 = %d\n", $endx1);
printf("stride = %d\n", $stride);

for($y = $starty; $y < $endy; $y += 1) {
    for($x = $startx; $x < $endx; $x += 1) {
        $addr = $startaddr + ($stride * $y) + ($x * $pixel_size);
	if(($x >= 0) && ($x < $endx0)) {
	    $data = $data0;
	} elsif (($x >= $endx0) && ($x < $endx1)) {
	    $data = $data1;
	} else {
 	    $data = $data2;
	}
#	$data = 0xffff00 if ( $y % 2 == 1 );
	WR32($addr, $data, 1);
    }
    printf("y = %d  x = %d addr is %x %x  %x  %x\n", $y, $x, $addr, $data0, $data1, $data2);
}
