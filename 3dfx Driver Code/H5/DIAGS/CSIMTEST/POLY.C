#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <h3.h>

// draw the poly from the H3 manual
void draw_poly (SstGRegs *sstg, int flag)
{
if (flag) {	// use GO bit
    SET(sstg->srcXY, 0x00010004);
    SET(sstg->command,SSTG_POLYFILL | SSTG_GO | (SSTG_ROP_SRC<<SSTG_ROP0_SHIFT));
    SET(sstg->launch[0], 0x00040002);
    SET(sstg->launch[1], 0x0001000a);
}
else {		// use separate srcXY dstXY
    SET(sstg->srcXY, 0x00010004);
    SET(sstg->dstXY, 0x0001000a);
    SET(sstg->command,SSTG_POLYFILL | (SSTG_ROP_SRC<<SSTG_ROP0_SHIFT));
    SET(sstg->launch[1], 0x00040002);
}
    SET(sstg->launch[2], 0x0003000b);
    SET(sstg->launch[3], 0x0006000b);
    SET(sstg->launch[4], 0x00060003);
    SET(sstg->launch[5], 0x00060001);
    SET(sstg->launch[6], 0x00080002);
    SET(sstg->launch[7], 0x0008000d);
    SET(sstg->launch[8], 0x000b0005);
    SET(sstg->launch[9], 0x00080008);
    SET(sstg->launch[10], 0x000b0005);
}

main (int argc, char **argv)
{
    int count,flag=0;
    SstRegs *sst;
    SstGRegs *sstg;

    if (argc < 2) {
	printf ("Usage: poly <count> [flag]\n");
	exit (1);
    }

    fxHalInit(0);
    sst = fxHalMapBoard(0);
    if (!fxHalInitRegisters(sst))
	printf("fxHalInitRegisters failed!\n");
    fflush(stdout);
    if (!fxHalInitGamma(sst, 1.4F))
	printf("fxHalInitGamma failed!\n");
    fflush(stdout);
    if (!fxHalInitVideo(sst, GR_RESOLUTION_640x480, GR_REFRESH_60Hz, NULL))
    //if (!fxHalInitVideo(sst, GR_RESOLUTION_1280x1024, GR_REFRESH_60Hz, NULL))
	printf("fxHalInitVideo failed!\n");
    fflush(stdout);

    count = atoi(argv[1]);
    if (argc > 2) flag = atoi(argv[2]);

    sstg = SSTG_CHIP(sst);		// get 2d address
    SET(sstg->dstFormat,SSTG_PIXFMT_32BPP | 640*4);
    SET(sstg->colorFore,0x12345678);
    SET(sstg->clip0min,0x00000000);
    SET(sstg->clip0max,0xFFFFFFFF);

    while (count-- > 0) {
	draw_poly(sstg,flag);
    }

    fxHalShutdown(sst);
    return 0;
}
