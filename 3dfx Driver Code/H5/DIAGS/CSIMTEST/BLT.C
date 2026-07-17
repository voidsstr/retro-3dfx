#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <h3.h>
#include <fximg.h>
#include <sstimage.h>

#ifndef M_PI
#define M_PI 3.141592F
#endif

#define FBITS SST_XY_FRACBITS
#define FONE (1<<FBITS)
#define MAXFRAC ((1<<FBITS)-1)
#define SST1 (1<<SST_XY_FRACBITS)

int mirror = 0;
long _s,_t;

// draw a rectangular blit
void draw_blt (int x, int y, int size, SstRegs *hw)
{
    long s = _s, t = _t;
    if (mirror) {		// hack: display it full screen
	s -= (400<<SST_ST_FRACBITS);
	t -= (400<<SST_ST_FRACBITS);
	size = 480;
    }
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

void
draw_quad(int x, int y, int size, int ang, int lod, SstRegs *hw)
{
    int xt,yt;
    float s2 = size/2.0F;
    float d1,d2;
    float s,c;
    float dsdx,dtdx,dsdy,dtdy;		// slopes in pure X,Y directions

    s = (float)sin(ang * (M_PI/180.0F));// compute sine,cosine of angle
    c = (float)cos(ang * (M_PI/180.0F));
    dsdx = (1<<(SST_ST_FRACBITS+lod))*c;// compute dsdx,dtdx slopes
    dtdx = (1<<(SST_ST_FRACBITS+lod))*s;
    SET(hw->dsdx, (int)dsdx);	// which are constant for entire quad
    SET(hw->dtdx, (int)dtdx);
    dsdy = -dtdx;
    dtdy = dsdx;

    // set initial s,t for top corner
    SET(hw->s, 0xFF<<SST_ST_FRACBITS);
    SET(hw->t, 0);
    d1 = s2 * (c+s);
    d2 = s2 * (c-s);
    if (c > s) {
	xt = x + (int)d2;		// x at the top
	yt = y - (int)d1;		// y at the top
	SET(hw->vA.x,xt<<SST_XY_FRACBITS);
	SET(hw->vA.y,yt<<SST_XY_FRACBITS);
	SET(hw->vB.x,x<<SST_XY_FRACBITS);
	SET(hw->vB.x,y<<SST_XY_FRACBITS);
	SET(hw->vC.x,x<<SST_XY_FRACBITS);
	SET(hw->vC.y,x<<SST_XY_FRACBITS);

	SET(hw->triangleCMD,0);		// GO****************

#if 0
	f = x - d1;			// x at leftmost corner
	xt = (int)(f * SST1);
	SET(hw->edgeStart.x,xt);
	SET(hw->edgeStart.dxdy,rs);	// same as right edge
	SET(hw->edgeStart.s, 0);	// reset S tex coordinate
	SET(hw->edgeStart.dsdy, (int)dsdy);	// and S,T slopes
	SET(hw->edgeStart.dtdy, (int)dtdy);
	yt = y + (int)d2;		// y at the rightmost corner
	SET_AND_GO(hw->yEnd,yt);	// GO****************

	f = x + d1;			// x at the rightmost corner
	xt = (int)(f * SST1);
	SET(hw->edgeEnd.x,xt);
	SET(hw->edgeEnd.dxdy,ls);	// same as left edge
#endif
    }
    else {				// angle is >= 45 degrees
#if 0
	m = (int)(-(1<<(SST_ST_FRACBITS+lod))/s);
	SET(hw->edgeStart.dsdy, m);
	SET(hw->edgeStart.dtdy, 0);
	yt = y + (int)d2;		// y at the rightmost corner
	SET_AND_GO(hw->yEnd,yt);	// GO****************

	f = x + d1;			// x at the rightmost corner
	xt = (int)(f * SST1);
	SET(hw->edgeEnd.x,xt);
	SET(hw->edgeEnd.dxdy,ls);	// same as left edge
	yt = y - (int)d2;
	SET_AND_GO(hw->yEnd,yt);	// GO****************

	f = x - d1;			// x at leftmost corner
	xt = (int)(f * SST1);
	SET(hw->edgeEnd.x,xt);
	SET(hw->edgeEnd.dxdy,rs);	// same as right edge
	SET(hw->edgeStart.s, 0);	// reset S tex coordinate
	SET(hw->edgeStart.dsdy, 0);	// and S slope
	m = (int)((1<<(SST_ST_FRACBITS+lod))/c);
	SET(hw->edgeStart.dtdy, m);
    yt = y + (int)d1;			// y at the bottom
    SET_AND_GO(hw->yEnd,yt);		// GO****************
#endif
    }
}

int
main (int argc, char **argv)  
{
    int n,count,texMode,test,dit2;
    long x,y,dx,dy,lod;
    SstRegs *hw;
    ImgInfo img;

    if (argc < 3) {
	printf ("Usage: blt <count> <filename> [<x>] [<y>] [test]\n");
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
    //if (!fxHalInitVideo(hw, GR_RESOLUTION_1280x1024, GR_REFRESH_60Hz, NULL))
	printf("fxHalInitVideo failed!\n");
    fflush(stdout);

    if (getenv("SST_DITHER2"))
	dit2 = SST_DITHER2x2;
    else dit2 = 0;
    sstMipMapInit();

    test = 0;
    x = y = 0;

    count = atoi(argv[1]);
    if (argc > 3) x = atoi(argv[3]);
    if (argc > 4) y = atoi(argv[4]);
    if (argc > 5) test = atoi(argv[5]);

    if (count < 0) {		// negative count implies 8-bit
	count = -count;
//#define RGB332 1
#ifdef RGB332
	texMode = SST_RGB332;
#else
	texMode = SST_YIQ422;
#endif
    }
    else
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
    texMode |= SST_TC_REPLACE;
texMode |= SST_TCLAMPS | SST_TCLAMPT;
    SET(hw->textureMode, texMode);	// must set this before download
    if (SST_T8BIT(texMode)) {
	unsigned char *tex8;
#define RGB332
#ifdef RGB332
	tex8 = sstDither332(NULL,(unsigned int *)img.any.data,n,n);
#else
	printf("Converting");
	tex8 = nccRGBtoYIQ(img.any.width,img.any.height,
				(unsigned int *)img.any.data,.01F);
	// NccTable is at end of texture
	sstSetNccTable(hw,0,(NccTable *)(tex8+n*img.any.height));
#endif
	sstDownLoadTexture(hw,0,0,texMode,0,8-lod,8-lod,1, (unsigned long *)tex8);
    }
    else {
	sstDither565((unsigned short *)img.any.data,(unsigned int *)img.any.data,n,n);
	sstDownLoadTexture(hw,0,0,texMode,0,8-lod,8-lod,2, (unsigned long *)img.any.data);
    }
    // NOTE: sstDownLoadTexture sets texBaseAddr properly

    // set slopes
    SET(hw->dsdx, 1<<(SST_ST_FRACBITS+lod));		// +1
    SET(hw->dtdx, 0);
    SET(hw->dwdx, 0);

    SET(hw->dsdy, 0);
    SET(hw->dtdy, 1<<(SST_ST_FRACBITS+lod));		// +1
    SET(hw->dwdy, 0);
    SET(hw->w, (1<<SST_W_FRACBITS));

    SET(hw->tLOD, SST_TLOD_MINMAX_INT(lod,lod));
    SET(hw->fbzMode, SST_RGBWRMASK | SST_ENDITHER | dit2);
#ifdef CVG
SET(hw->fbzMode, SST_RGBWRMASK | SST_ENDITHER | SST_DRAWBUFFER_BACK | dit2);
#else // H3
SET(hw->fbzMode, SST_RGBWRMASK | SST_ENDITHER | dit2);
#endif
    SET(hw->fbzColorPath, SST_RGBSEL_TREXOUT | SST_ENTEXTUREMAP | SST_PARMADJUST);
    if (test >= 6) {		// if test larger than 5, then implies mirroring
	test -= 6;
	SET(hw->tLOD, SST_TLOD_MINMAX_INT(lod,lod) | SST_TMIRRORS | SST_TMIRRORT);
	mirror = 1;
    }

    x <<= SST_XY_FRACBITS;
    y <<= SST_XY_FRACBITS;
    _s = 0;
    _t = 0;

    switch (test) {
	case 0:
	    while (count-- > 0) {
		draw_blt(x,y,img.any.width,hw);
	    }
	    break;
	case 2:			// scroll vertical
	    dx = 0;
	    dy = FONE/4;
	case 1:			// scroll horizontal
	    if (test == 1) dx = FONE/4, dy=0;
//x += dx;
	    while (count-- > 0) {
		draw_blt(x,y,img.any.width,hw);
		x += dx;
		y += dy;
	    }
	    break;
	case 3:
	    x += (long)(img.any.width/1.3F);
	    y += (long)(img.any.width/1.3F);
	    dx = 1;
	    dy = 1;
	    while (count-- > 0) {
		draw_quad(x,y,img.any.width,dx,lod,hw);
		dx += dy;
	    }
	    break;
	case 4:				// get larger
	case 5:				// get smaller
	    dy = 1;
	    if (test==4) dy = -dy;
	    dx = (1<<6);			// initial mag
	    while (count-- > 0) {
		SET(hw->dsdx, dx<<(SST_ST_FRACBITS+lod-6));
		SET(hw->dtdy, -dx<<(SST_ST_FRACBITS+lod-6));
		draw_blt(x,y,img.any.width * (1<<6)/dx,hw);
		dx += dy;
	    }
	    break;

	default:
	    break;
    }
    fxHalShutdown(hw);
    return 0;
}
