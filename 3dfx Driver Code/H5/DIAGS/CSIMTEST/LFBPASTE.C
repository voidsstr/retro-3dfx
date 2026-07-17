#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <h3.h>
#include <fximg.h>
#include <sstimage.h>

int lfbMode;

// draw a rectangular blit
void draw_blt16 (int x, int y, ImgInfo *img, SstRegs *hw)
{
    int dy, sizeX;
    unsigned short *lfb, *imgdata;

    sizeX = img->any.width;
    dy = 1<<SST_LFB_ADDR_Y_SHIFT;

    // base address of linear frame buffer is 4 Meg
    lfb = (unsigned short *) SST_LFB_ADDRESS(hw);
    // add in x,y offset
    lfb += (y<<SST_LFB_ADDR_Y_SHIFT) + x;
    imgdata = (unsigned short *)img->any.data;
    SET(hw->lfbMode, lfbMode);
    for (y=0; y<(signed)img->any.height; y++) {
	for (x=0; x<sizeX; x++) {
	    SET16(lfb[x],*imgdata);
	    imgdata++;
	}
	lfb += dy;
    }
}

// draw a rectangular blit with 32-bit writes
void draw_blt32 (int x, int y, ImgInfo *img, SstRegs *hw)
{
    int dy, sizeX;
    unsigned long *lfb, *imgdata;

    sizeX = img->any.width;
    if ((lfbMode&SST_LFB_FORMAT) == SST_LFB_565) {
	sizeX >>= 1;
	dy = 1<<(SST_LFB_ADDR_Y_SHIFT-1);
    }
    else dy = 1<<SST_LFB_ADDR_Y_SHIFT; 

    // base address of linear frame buffer is 4 Meg
    lfb = (unsigned long *) SST_LFB_ADDRESS(hw);
    // add in x,y offset
    lfb += (y<<SST_LFB_ADDR_Y_SHIFT) + x;
    imgdata = (unsigned long *)img->any.data;
    SET(hw->lfbMode, lfbMode);
    for (y=0; y<(signed)img->any.height; y++) {
	for (x=0; x<sizeX; x++) {
	    SET(lfb[x],*imgdata);
	    imgdata++;
	}
	lfb += dy;
    }
}

int
main (int argc, char **argv)  
{
    int count,test,lanes=0;
    long x,y;
    SstRegs *hw;
    ImgInfo img;

argc++;
    if (argc < 3) {
	printf ("Usage: lfbpaste <filename> [<x>] [<y>] [test] [lanes]\n");
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

    test = 0;
    x = y = 0;
    count = 1;
argv--;
    imgReadFile(argv[2],&img);
    if (argc > 3) x = atoi(argv[3]);
    if (argc > 4) y = atoi(argv[4]);
    if (argc > 5) test = atoi(argv[5]);
    if (argc > 6) lanes = atoi(argv[6])<<SST_LFB_RGBALANES_SHIFT;

    lfbMode = SST_LFB_8888;
    SET(hw->fbzMode, SST_RGBWRMASK | SST_ENDITHER);

    switch (test) {
	case 1:		// 32-bit 565,565
	    lfbMode = SST_LFB_565;
	    lfbMode |= lanes;
	    img.any.data = (unsigned char *) sstDither565(NULL,
				(unsigned int *)img.any.data,
				img.any.width,img.any.height);
	    while (count-- > 0) {
		draw_blt32(x,y,&img,hw);
	    }
	    break;
	case 0:		// 32-bit 8888
	    lfbMode = SST_LFB_8888;
	    lfbMode |= lanes;
	    while (count-- > 0) {
		draw_blt32(x,y,&img,hw);
	    }
	    break;
	case 2:		// 16-bit 565
	    lfbMode = SST_LFB_565;
	    lfbMode |= lanes;
	    img.any.data = (unsigned char *) sstDither565(NULL,
				(unsigned int *)img.any.data,
				img.any.width,img.any.height);
	    while (count-- > 0) {
		draw_blt16(x,y,&img,hw);
	    }
	default:
	    break;
    }
    fxHalShutdown(hw);
    return 0;
}
