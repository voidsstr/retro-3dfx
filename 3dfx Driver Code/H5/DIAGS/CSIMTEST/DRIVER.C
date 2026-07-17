#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <h3.h>

void draw_rect (int x, int y, int w, int h, SstGRegs *sstg)
{
    SET(sstg->dstXY, (y<<16) | x);
    SET(sstg->dstSize,  (h<<16) | w);
    SET(sstg->command,  SSTG_RECTFILL | SSTG_GO | 
		(SSTG_ROP_SRC<<SSTG_ROP0_SHIFT));
}

main (int argc, char **argv)
{
    int count;
    long x,y,w,h;
    FxU32 *yuv,*cp;
    SstRegs *sst;
    SstGRegs *sstg;
    SstCRegs *sstc;

    if (argc < 6) {
	printf ("Usage: rect <count> <x> <y> <width> <height>\n");
	exit (1);
    }

    // 0x10000000 is a hardcoded address for now
    // NOTE: most of the macros work off the 3D base address
    sst = (SstRegs *)(0x10000000+SST_3D_OFFSET);
    csimInitDriver(4096*1024, malloc(4096*1024), (volatile FxU32 *)SST_BASE_ADDRESS(sst));

    count = atoi(argv[1]);
    x = atoi(argv[2]);
    y = atoi(argv[3]);
    w = atoi(argv[4]);
    h = atoi(argv[5]);

    // get a pointer to CMDAGP registers and draw some YUV pixels
    sstc = (SstCRegs *)SST_CMDAGP_ADDRESS(sst);
    SET(sstc->yuvBaseAddr,x*16);
    yuv = (FxU32 *)SST_YUV_ADDRESS(sst);
    SET(yuv[0],0xFFFFFFFF);		// Y plane
    SET(yuv[1],0x55555555);
    SET(yuv[2],0xaaaaaaaa);
    SET(yuv[0x40000],0xFFFFFFFF);	// U plane
    SET(yuv[0x40001],0x55555555);
    SET(yuv[0x40002],0xaaaaaaaa);
    SET(yuv[0x80000],0xFFFFFFFF);	// V plane
    SET(yuv[0x80001],0x55555555);
    SET(yuv[0x80002],0xaaaaaaaa);

    // test out some raw LFB writes
    cp = (FxU32 *)(SST_BASE_ADDRESS(sst) + SST_RAW_LFB_OFFSET);
    SET(cp[0],0x0);
    SET(cp[1],0x1);
    SET(cp[2],0x2);
    SET(cp[3],0x3);

    // get a pointer to the 2D regs and draw a rectangle
    sstg = (SstGRegs *)SST_GUI_ADDRESS(sst);	// get 2d address
    SET(sstg->dstFormat,SSTG_PIXFMT_32BPP | 640*4);
    SET(sstg->colorFore,0x12345678);
    SET(sstg->clip0min,0x00000000);
    SET(sstg->clip0max,0xFFFFFFFF);

    while (count-- > 0) {
	draw_rect(x,y,w,h,sstg);
    }

#define CMDFIFO_START 0x03ff000
    // initialize a command fifo
    fxHalInitCmdFifo(sst,
			0,		// which fifo, either 0 or 1
			CMDFIFO_START,	// starting fifo address
			0x1000,		// fifo size
			0,		// directExec mode (CVG only)
			0,		// manually bumping mode
			0); 		// disable agp

    // very primitive code to draw the same rectangle again
    cp = (FxU32 *)(SST_BASE_ADDRESS(sst) + SST_RAW_LFB_OFFSET + CMDFIFO_START);
    SET(cp[0], (0x7000000 << SSTCP_PKT2_MASK_SHIFT) | SSTCP_PKT2);
    SET(cp[1], (h<<16) | w);
    SET(cp[2], (y<<16) | x);
    SET(cp[3],  SSTG_RECTFILL | SSTG_GO | (SSTG_ROP_SRC<<SSTG_ROP0_SHIFT));

    // this waits for idle and executes everything in the CMD FIFO
    fxHalIdleNoNop(sst);

    fxHalShutdown(sst);
    return 0;
}
