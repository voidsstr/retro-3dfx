#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <h3.h>

void draw_rect (int x, int y, int w, int h, SstGRegs *hwg)
{
    SET(hwg->dstXY, (y<<16) | x);
    SET(hwg->dstSize, (h<<16) | w);
    SET(hwg->command,  SSTG_RECTFILL | SSTG_GO | 
		(SSTG_ROP_SRC<<SSTG_ROP0_SHIFT));
}

main (int argc, char **argv)
{
    int count;
    long x,y,w,h;
    SstRegs *hw;
    SstGRegs *hwg;

    if (argc < 6) {
	printf ("Usage: rect <count> <x> <y> <width> <height>\n");
	exit (1);
    }

    fxHalInit(0);
    hw = fxHalMapBoard(0);
    if (!fxHalInitRegisters(hw))
	printf("fxHalInitRegisters failed!\n");
    fflush(stdout);
    if (!fxHalInitGamma(hw, 1.4F))
	printf("fxHalInitGamma failed!\n");
    fflush(stdout);
    if (!fxHalInitVideo(hw, GR_RESOLUTION_640x480, GR_REFRESH_60Hz, NULL))
    //if (!fxHalInitVideo(hw, GR_RESOLUTION_1280x1024, GR_REFRESH_60Hz, NULL))
	printf("fxHalInitVideo failed!\n");
    fflush(stdout);

    count = atoi(argv[1]);
    x = atoi(argv[2]);
    y = atoi(argv[3]);
    w = atoi(argv[4]);
    h = atoi(argv[5]);

    hwg = SSTG_CHIP(hw);		// get 2d address
    SET(hwg->dstFormat,SSTG_PIXFMT_32BPP | 640*4);
    SET(hwg->colorFore,0x12345678);
    SET(hwg->clip0min,0x00000000);
    SET(hwg->clip0max,0xFFFFFFFF);

    while (count-- > 0) {
	draw_rect(x,y,w,h,hwg);
    }

    fxHalShutdown(hw);
    return 0;
}
