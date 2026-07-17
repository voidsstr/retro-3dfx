#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <h3.h>
#include <fximg.h>
#include <sstimage.h>

#define FBITS SST_XY_FRACBITS
#define FONE (1<<FBITS)
#define MAXFRAC ((1<<FBITS)-1)
#define SST1 (1<<SST_XY_FRACBITS)

long _s,_t;

// draw a rectangular blit
void draw_blt (int x, int y, int size, int mag, SstRegs *hw)
{
    long s = _s, t = _t;

    SET(hw->fastfillCMD,mag);
    size *= mag;
    SET(hw->s, s);
    SET(hw->t, t);
    SET(hw->vA.x,x);
    SET(hw->vA.y,y);
    SET(hw->vB.x,x);
    SET(hw->vB.y,y+(size<<SST_XY_FRACBITS));
    SET(hw->vC.x,x+(size<<SST_XY_FRACBITS));
    SET(hw->vC.y,y+(size<<SST_XY_FRACBITS));
    SET(hw->triangleCMD,0xFFFFFFFF);

    SET(hw->s, s);
    SET(hw->t, t);
    SET(hw->vB.x,x+(size<<SST_XY_FRACBITS));
    SET(hw->vB.y,y);
    SET(hw->triangleCMD,0);
    SET(hw->swapbufferCMD,0);
}

int
main (int argc, char **argv)  
{
    int n,count,texMode,test,dit2,aref;
    long x,y,dx,dy,lod,mag;
    SstRegs *hw;
    ImgInfo img;

    if (argc < 3) {
	printf ("Usage: chroma <count> <filename> [mag] [<x>] [<y>] [test]\n");
	exit (1);
    }
    imgReadFile(argv[2],&img);

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

    if (getenv("SST_DITHER2"))
	dit2 = SST_DITHER2x2;
    else dit2 = 0;
    sstMipMapInit();

    test = 0;
    mag = 32;
    x = y = 0;

    count = atoi(argv[1]);
    if (argc > 3) mag = atoi(argv[3]);
    if (argc > 4) x = atoi(argv[4]);
    if (argc > 5) y = atoi(argv[5]);
    if (argc > 6) test = atoi(argv[6]);

    texMode = SST_RGB565;

    if (test < 0) {		// negative test implies bilinear
	test = -test;
	texMode |= SST_TMAGFILTER | SST_TMINFILTER;
    }
    if (img.any.width != img.any.height) {
	printf ("error: image is not square: %d x %d\n",
		    img.any.width,img.any.height);
//	exit (2);
    }
    if (img.any.width == 256) lod = 0;
    else if (img.any.width == 128) lod = 1;
    else if (img.any.width == 64) lod = 2;
    else if (img.any.width == 32) lod = 3;
    else if (img.any.width == 16) lod = 4;
    else if (img.any.width == 8) lod = 5;
    else if (img.any.width == 4) lod = 6;
    else if (img.any.width == 2) lod = 7;
    else if (img.any.width == 1) lod = 8;
    else {
	printf("error: image size is not a power of 2 - %d\n",img.any.width);
	fxHalShutdown(hw);
	exit (3);
    }
    n = lod & 0xF;			// just take lower nibble
    n = 1 << (8-n);			// compute size from LOD
    texMode |= SST_TC_REPLACE | SST_TCA_REPLACE;
    SET(hw->textureMode, texMode);	// must set this before download
    sstDither565((unsigned short *)img.any.data,(unsigned int *)img.any.data,n,n);
    sstDownLoadTexture(hw,0,0,texMode,0,8-lod,8-lod,2, (unsigned long *)img.any.data);
    // NOTE: sstDownLoadTexture sets texBaseAddr properly

    // set slopes
    SET(hw->dsdx, (1<<(SST_ST_FRACBITS+lod))/mag);		// +1
    SET(hw->dtdx, 0);
    SET(hw->dwdx, 0);

    SET(hw->dsdy, 0);
    SET(hw->dtdy, (1<<(SST_ST_FRACBITS+lod))/mag);		// +1
    SET(hw->dwdy, 0);
    SET(hw->w, (1<<SST_W_FRACBITS));
    SET(hw->chromaKey, 0x0000FF);

if (count < 0) {
    count = -count;
    // this creates circular outlines
    SET(hw->chromaRange, SST_ENCHROMAKEY_TMU | SST_ENCOLORSUBSTITUTION);
    aref = 0x50;
    if (getenv("AREF")) sscanf(getenv("AREF"),"%i",&aref);

    SET(hw->alphaMode, (aref<<SST_ALPHAREF_SHIFT) | SST_ENALPHAFUNC | SST_ALPHAFUNC_GT);
}
else {
    SET(hw->chromaRange, SST_ENCHROMAKEY_TMU);
    // this blends into background, looks better without alphafunc
    SET(hw->alphaMode, SST_ENALPHABLEND |
//		(SST_A_SRCALPHA<<SST_RGBSRCFACT_SHIFT) |
		(SST_A_ONE<<SST_RGBSRCFACT_SHIFT) |
		(SST_AOM_SRCALPHA<<SST_RGBDSTFACT_SHIFT));
}

    SET(hw->tLOD, SST_TLOD_MINMAX_INT(lod,lod));
    // single : SET(hw->fbzMode, SST_RGBWRMASK | SST_ENDITHER | SST_ENCHROMAKEY | dit2);
#ifdef CVG
    SET(hw->fbzMode, SST_RGBWRMASK | SST_ENDITHER | SST_ENCHROMAKEY | SST_DRAWBUFFER_BACK | dit2);
#else // H3
    SET(hw->fbzMode, SST_RGBWRMASK | SST_ENDITHER | SST_ENCHROMAKEY | dit2);
#endif
    SET(hw->fbzColorPath, SST_RGBSEL_TREXOUT | SST_ASEL_TREXOUT | SST_ENTEXTUREMAP | SST_PARMADJUST);

    SET(hw->c0, 0x800000);
    SET(hw->c1, 0x800000);
    SET(hw->clipLeftRight, (mag*img.any.width+32)<<0);
    SET(hw->clipBottomTop, (mag*img.any.height+32)<<0);

    x <<= SST_XY_FRACBITS;
    y <<= SST_XY_FRACBITS;
    _s = 0;
    _t = 0;

    switch (test) {
	case 0:
	    while (count-- > 0) {
		draw_blt(x,y,img.any.width,mag,hw);
	    }
	    break;
	case 2:			// scroll vertical
	    dx = 0;
	    dy = FONE/4;
	case 1:			// scroll horizontal
	    if (test == 1) dx = FONE/4, dy=0;
	    while (count-- > 0) {
		draw_blt(x,y,img.any.width,mag,hw);
		x += dx;
		y += dy;
	    }
	    break;
	case 3:				// get larger
	case 4:				// get smaller
	    dy = 1;
	    if (test==4) dy = -dy;
	    dx = (1<<6);			// initial mag
	    while (count-- > 0) {
		SET(hw->dsdx, dx<<(SST_ST_FRACBITS+lod-6));
		SET(hw->dtdy, -dx<<(SST_ST_FRACBITS+lod-6));
		draw_blt(x,y,img.any.width * (1<<6)/dx,mag,hw);
		dx += dy;
	    }
	    break;

	default:
	    break;
    }
    fxHalShutdown(hw);
    return 0;
}
