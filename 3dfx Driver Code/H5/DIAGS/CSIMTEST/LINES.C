#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <h3.h>

int rop;

void draw_line (long x1, long y1, long x2, long y2, SstGRegs *hw)
{
    SET(hw->srcXY,(y1<<16) | (x1&0xFFFF));
    SET(hw->dstXY,(y2<<16) | (x2&0xFFFF));
    SET(hw->command,  SSTG_LINE | SSTG_GO | (rop<<SSTG_ROP0_SHIFT));
}

void draw_fline (double x1, double y1, double x2, double y2, SstGRegs *hw)
{
    draw_line((long)x1, (long)y1, (long)x2, (long)y2, hw);
}

void draw_polyline (long x2, long y2, SstGRegs *hw)
{
    SET(hw->dstXY,(y2<<16) | (x2&0xFFFF));
    SET(hw->command, SSTG_POLYLINE | SSTG_EN_LINESTIPPLE | SSTG_GO | (rop<<SSTG_ROP0_SHIFT));
}

void draw_polyfline (double x2, double y2, SstGRegs *hw)
{
    draw_polyline((long)x2, (long)y2, hw);
}

unsigned long erase, hole;


float twopi = 2.0F * 3.14159F;

void draw_spokes_out(long cx, long cy, long rad, int spokes, SstGRegs *hwg)
{
    int i;

    for (i=0; i<spokes; i++) {
	draw_fline (
		    cx+hole*cos(twopi*i/spokes),
		    cy+hole*sin(twopi*i/spokes),
		    cx+rad*cos(twopi*i/spokes),
		    cy+rad*sin(twopi*i/spokes),
		    hwg);
	if (erase) {
	    draw_fline (
		    cx+hole*cos(twopi*i/spokes),
		    cy+hole*sin(twopi*i/spokes),
		    cx+rad*cos(twopi*i/spokes),
		    cy+rad*sin(twopi*i/spokes),
		    hwg);
	}
    }
}

void draw_spokes_in(long cx, long cy, long rad, int spokes, SstGRegs *hwg)
{
    int i;

    for (i=0; i<spokes; i++) {
	draw_fline (
		    cx+rad*cos(twopi*i/spokes),
		    cy+rad*sin(twopi*i/spokes),
		    cx+hole*cos(twopi*i/spokes),
		    cy+hole*sin(twopi*i/spokes),
		    hwg);
	if (erase) {
	    draw_fline (
		    cx+rad*cos(twopi*i/spokes),
		    cy+rad*sin(twopi*i/spokes),
		    cx+hole*cos(twopi*i/spokes),
		    cy+hole*sin(twopi*i/spokes),
		    hwg);
	}
    }
}

void draw_circles(long cx, long cy, long rad, int spokes, SstGRegs *hwg)
{
    int i,n;

    SET(hwg->lineStipple,0x50ff);
    for (n=1; n<rad; n += 10) {
	// reload the line style
	SET(hwg->lineStyle,0xF00);
	SET(hwg->srcXY,(cy<<16) | (cx+n));
	for (i=0; i<spokes; i++) {
	    draw_polyfline(cx+n*cos(twopi*(i+1)/spokes) + 0.5,
			  cy+n*sin(twopi*(i+1)/spokes) + 0.5,
			  hwg);
	}
    }
}

void draw_shrinking(long cx, long cy, long r, int spokes, SstGRegs *hwg)
{
    int i;
    float rad = (float)r;

    for (;rad > 0; rad -= .10327F) {
	for (i=0; i<spokes; i++) {
	    draw_fline (
		    cx+hole*cos(twopi*i/spokes),
		    cy+hole*sin(twopi*i/spokes),
		    cx+rad*cos(twopi*i/spokes),
		    cy+rad*sin(twopi*i/spokes),
		    hwg);
	}

	for (i=0; i<spokes; i++) {
	    draw_fline (
		    cx+hole*cos(twopi*i/spokes),
		    cy+hole*sin(twopi*i/spokes),
		    cx+rad*cos(twopi*i/spokes),
		    cy+rad*sin(twopi*i/spokes),
		    hwg);
	}
    }
}

// GMT - this isn't fully working yet, too little time....
void draw_grid(long cx, long cy, long rad, int spokes, SstGRegs *hwg)
{
    int i;
    float a,d1,d2,c,s;

    spokes /= 2;
    for (a = 0.0F; a < twopi/4.0F; a+= twopi/4.0F/90.0F) {
	c = (float)cos(a);
	s = (float)sin(a);
	for (i= -spokes; i<=spokes; i++) {
	    draw_fline (
		    cx-rad * c - rad*i/spokes * s,
		    cy-rad * s + rad*i/spokes * c,
		    cx+rad * c - rad*i/spokes * s,
		    cy+rad * s + rad*i/spokes * c,
		    hwg);
		    /*
	    draw_fline (
		    cx-d1,
		    cy-d2,
		    cx+d1,
		    cy+d2,
		    hwg);
		    */
	}

	c = (float)cos(a);
	s = (float)sin(a);
	for (i= -spokes; i<=spokes; i++) {
	    d1 = rad * c - rad*i/spokes * s;
	    d2 = rad * s + rad*i/spokes * c;
	    draw_fline (
		    cx-d1,
		    cy+d2,
		    cx+d1,
		    cy+d2,
		    hwg);
	    draw_fline (
		    cx+d2,
		    cy-d1,
		    cx+d2,
		    cy+d1,
		    hwg);
	}
    }
}


main (int argc, char **argv)
{
    char *token;
    int count,test;
    long x1,y1, x2,y2;
    SstRegs *hw;
    SstGRegs *hwg;
    
    if (argc < 6) {
     printf("Usage: %s <count> <x1|cx> <y1|cy> <x2|rad> <y2|spokes> [test]\n",
	    argv[0]);
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

    test = hole = 0;
    count = atoi(argv[1]);
    x1 = atoi(argv[2]);
    y1 = atoi(argv[3]);
    x2 = atoi(argv[4]);
    y2 = atoi(argv[5]);
    if (argc > 6) test = atoi(argv[6]);
    if (token=getenv("HOLE"))
	sscanf(token,"%d",&hole);

    hwg = SSTG_CHIP(hw);		// get 2d address
    SET(hwg->dstFormat,SSTG_PIXFMT_32BPP | 640*4);
    SET(hwg->colorFore,0x12345678);
    SET(hwg->colorBack,0xFFFFFFFF);
    SET(hwg->clip0min,0x00000000);
    SET(hwg->clip0max,0xFFFFFFFF);
    SET(hwg->lineStipple,0xFFFF);
    SET(hwg->lineStyle,0x0);
    rop = SSTG_ROP_SRC;
    
    erase = 0;
    switch (test) {
	case 0:
	    while (count-- > 0)
		draw_line (x1,y1,x2,y2,hwg);
	    break;
	case 2:	erase++;
	    rop = SSTG_ROP_XOR;
	case 1: 
	    while (count-- > 0)
		draw_spokes_out (x1,y1,x2,y2,hwg);
	    break;
	case 4:	erase++;
	    rop = SSTG_ROP_XOR;
	case 3:
	    while (count-- > 0) {
		draw_spokes_in (x1,y1,x2,y2,hwg);
	    }
	    break;
	case 5:
	    while (count-- > 0) {
		draw_circles (x1,y1,x2,y2,hwg);
	    rop = SSTG_ROP_XOR;
	    }
	    break;
	case 6:
	    rop = SSTG_ROP_XOR;
	    while (count-- > 0)
		draw_shrinking (x1,y1,x2,y2,hwg);
	    break;
	case 7:
	    while (count-- > 0)
		draw_grid (x1,y1,x2,y2,hwg);
	    break;
	case 8:
	    while (count-- > 0)
		draw_grid (x1,y1,x2,y2,hwg);
	    break;
	default:
	    break;
    }

    fxHalShutdown(hw);
    return 0;
}

