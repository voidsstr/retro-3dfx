#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <h3.h>

#define BIAS1 ((float)(1<<19))
#define BIAS3 ((float)(3<<18))

main (int argc, char **argv)
{
    int n,count;
    long x,y;
    SstRegs *hw;

    if (argc < 4) {
	printf ("Usage: tris <count> <x> <y>\n");
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
    SET(hw->fbzColorPath,SST_PARMADJUST);

// GMT: for now a simple hardcoded strip to fan to strip case
for (n=0; n<count; n++) {
    SET(hw->sSetupMode,SST_SETUP_RGB | (count < 0 ? SST_SETUP_FAN: 0));

// you can define this to be BIAS1 or BIAS3 to test out
// older snapping code
#define BIAS 0
    SETF(hw->sVx,(float)x+BIAS);
    SETF(hw->sVy,(float)y+BIAS);
    SETF(hw->sRed,1.0F);
    SETF(hw->sGreen,2.0F);
    SETF(hw->sBlue,3.0F);
    SETF(hw->sAlpha,4.0F);
    SET(hw->sBeginTriCMD,0);		// BEGIN 1

    SETF(hw->sVx,(float)x+BIAS);
    SETF(hw->sVy,y+10.01F+BIAS);
    SETF(hw->sRed,255.0F);
    SETF(hw->sGreen,2.0F);
    SETF(hw->sBlue,3.0F);
    SETF(hw->sAlpha,4.0F);
    SET(hw->sDrawTriCMD,0);		// DRAW 2

    SETF(hw->sVx,x+9.99F+BIAS);
    SETF(hw->sVy,(float)y+BIAS);
    SETF(hw->sRed,1.0F);
    SETF(hw->sGreen,255.0F);
    SETF(hw->sBlue,3.0F);
    SETF(hw->sAlpha,4.0F);
    SET(hw->sDrawTriCMD,0);		// DRAW 3

    SETF(hw->sVx,x+10.1212F+BIAS);
    SETF(hw->sVy,y+10.55555F+BIAS);
    SETF(hw->sRed,1.0F);
    SETF(hw->sGreen,2.0F);
    SETF(hw->sBlue,255.0F);
    SETF(hw->sAlpha,4.0F);
    SET(hw->sDrawTriCMD,0);		// DRAW 4

    SET(hw->sSetupMode,SST_SETUP_RGB | (count >= 0 ? SST_SETUP_FAN: 0));
    SETF(hw->sVx,x+10.0F+BIAS);
    SETF(hw->sVy,y+20.0F+BIAS);
    SETF(hw->sRed,155.0F);
    SETF(hw->sGreen,255.0F);
    SETF(hw->sBlue,55.0F);
    SETF(hw->sAlpha,4.0F);
    SET(hw->sDrawTriCMD,0);		// DRAW 5

    SETF(hw->sVx,x+10.0F+BIAS);
    SETF(hw->sVy,y+30.0F+BIAS);
    SET(hw->sARGB,0x05FFFEFC);
    SET(hw->sDrawTriCMD,0);		// DRAW 6

    SET(hw->sSetupMode,SST_SETUP_RGB | (count < 0 ? SST_SETUP_FAN: 0));
    SETF(hw->sVx,x+20.0F+BIAS);
    SETF(hw->sVy,y+20.0F+BIAS);
    SETF(hw->sRed,5.0F);
    SETF(hw->sGreen,5.0F);
    SETF(hw->sBlue,5.0F);
    SETF(hw->sAlpha,4.0F);
    SET(hw->sDrawTriCMD,0);		// DRAW 7

    SETF(hw->sVx,x+20.0F+BIAS);
    SETF(hw->sVy,y+30.0F+BIAS);
    SETF(hw->sRed,255.0F);
    SETF(hw->sGreen,5.0F);
    SETF(hw->sBlue,5.0F);
    SETF(hw->sAlpha,4.0F);
    SET(hw->sDrawTriCMD,0);		// DRAW 8

    SET(hw->sDrawTriCMD,0);		// 0 area
}

    fxHalShutdown(hw);
    return 0;
}
