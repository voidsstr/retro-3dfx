#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <h3.h>
#include <fxos.h>

#define XY_ONE (1<<SST_XY_FRACBITS)

void draw_point (int x, int y, SstRegs *hw)
{
    x <<= SST_XY_FRACBITS;
    y <<= SST_XY_FRACBITS;
    SET(hw->vA.x,x);
    SET(hw->vA.y,y);
    SET(hw->vB.x,x+XY_ONE);
    SET(hw->vB.y,y);
    SET(hw->vC.x,x+XY_ONE);
    SET(hw->vC.y,y+XY_ONE);
    SET(hw->triangleCMD,0);
}

main (int argc, char **argv)
{
    int n,count;
    long x,y;
    SstRegs *hw;

    if (argc < 4) {
	printf ("Usage: points <count> <x> <y>\n");
	exit (1);
    }

    fxHalInit(0);
    printf("found %d boards\n",fxHalNumBoardsInSystem());
#if 1
    count = atoi(argv[1]);
    timer(0);
    for (n=count; n>0; n--)
	fxHalNumBoardsInSystem();
    fprintf(stdout,"%.2f seconds for %d calls\n",timer(1),count);
    fflush(stdout);
#endif
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

#ifdef CVG
    SET(hw->fbzMode,SST_RGBWRMASK | SST_DRAWBUFFER_FRONT);
#else // H3
    SET(hw->fbzMode,SST_RGBWRMASK);
#endif

    // set command, dx, r,g,b
    SET(hw->r, 0xFF<<SST_RGBA_FRACBITS);	// set the color
    SET(hw->g, 0x80<<SST_RGBA_FRACBITS);
    SET(hw->b, 0x01<<SST_RGBA_FRACBITS);

    for (n = count>>3; n; n--) {
	draw_point(x,y,hw);
	draw_point(x,y,hw);
	draw_point(x,y,hw);
	draw_point(x,y,hw);
	draw_point(x,y,hw);
	draw_point(x,y,hw);
	draw_point(x,y,hw);
	draw_point(x,y,hw);
    }
    count &= 7;
    while (count-- > 0) {
	draw_point(x,y,hw);
    }

    x = (x+(y<<SST_LFB_ADDR_Y_SHIFT))*2 + SST_LFB_OFFSET+(int)hw;
    GET16(*(FxU16 *)x);
    fxHalShutdown(hw);
    return 0;
}
