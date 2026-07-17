#include <stdio.h>
#include <stdlib.h>

#include <h3.h>

void
main (int argc, char **argv)
{
    long color=99;
    SstRegs *sst;
    SstGRegs *hwg;

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

    SET(sst[-1],color);				// illegal
    SET(*(char *)(1+(long)sst),color);		// unaligned
    SET(*(char *)(2+(long)sst),color);		// unaligned
    SET(*(char *)(3+(long)sst),color);		// unaligned
    SET(*(char *)(900+(long)sst),color);	// bad reg
    SET(*(char *)(2900+(long)sst),color);	// bad reg
    SET(*(char *)(3900+(long)sst),color);	// bad reg

    SET_FBI_0(sst->s,23);
    SET_FBI(sst->fbzMode,1);
    SET_0(sst->textureMode,0xFFFFFFFF);		// TREX0

    SET_1(sst->textureMode,0xDEADBEEF);		// TREX1
    SETF(SST_TMU(sst,2)->s,1.23F);		// TREX2


//    gsim_putpixel(0,1,2,23);
//    color = gsim_getpixel(1,2,0xFFFFFF);
//    GDBG_INFO((1,"info test color = %d %f\n",color,2.0F));

    color = GET(sst->s);
    GET16(sst->textureMode);
    
    SET(*(char *)(SST_RAW_LFB_OFFSET+SST_BASE_ADDRESS(sst)),0xafb);	// non-modal LFB

    hwg = SSTG_CHIP(sst);		// get 2d address
    SET(hwg->colorPattern[23],23);	// good writes
#if COLORTRANSLUT
    SET(hwg->colorTransLut[4],4);
#endif
    SET(hwg->launch[0],1);
    SET(hwg->launch[31],2);

    SET(hwg->colorPattern[-1],3);	// last launch area
#if COLORTRANSLUT
    SET(hwg->colorTransLut[256],0xbad);
#endif
    SET(hwg->colorPattern[0],3);
    SET(hwg->colorPattern[63],4);
    SET(hwg->colorPattern[64],0xdeadbeef);

    fxHalShutdown(sst);
}
