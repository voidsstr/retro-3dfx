/*
** Copyright (c) 1997, 3Dfx Interactive, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of 3Dfx Interactive, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
** rights reserved under the Copyright Laws of the United States.
**
** $Revision: 2$
** $Date: 10/11/00 8:11:16 PM$
*/

#include "udiag.h"
#include "sstdiag.h"
#include <math.h>

#define IND_LINE(x1,y1,x2,y2) \
SET(sstg->srcXY, ((y1) << 16) | ((x1) & 0xFFFF)); \
SET(sstg->command, 0x6 | ((ropcode & 0xFF) << 24)); \
SET(sstg->launch[launchc++ % 10], ((y2) << 16) | ((x2) & 0xFFFF))

#define POL_LINE(x1,y1) \
SET(sstg->launch[launchc++ % 10], ((y1) << 16) | ((x1) & 0xFFFF))

#define NOP() \
SET(sstg->command, 0); \
SET(sstg->command, 0x100)

#define IDLE(s) \
	  while (GET(s->status) & 0x200)

#define RECTANGLE(x1,y1,x2,y2) \
SET(sstg->dstSize, ((y1) << 16) | ((x1) & 0xFFFF)); \
SET(sstg->launch[launchc++ % 10], ((y2) << 16) | ((x2) & 0xFFFF))

#define SBLIT(sx1,sy1,dx1,dy1,dw,dh)\
SET(sstg->dstSize, ((dh) << 16) | ((dw) & 0xFFFF)); \
SET(sstg->dstXY, ((dy1) << 16) | ((dx1) & 0xFFFF)); \
SET(sstg->launch[launchc++ % 10], ((sy1) << 16) | ((sx1) & 0xFFFF))

#define STRETCH_BLIT(sx1,sy1,dx1,dy1,dw,dh,sw,sh)\
SET(sstg->dstSize, ((dh) << 16) | ((dw) & 0xFFFF)); \
SET(sstg->srcSize, ((sh) << 16) | ((sw) & 0xFFFF)); \
SET(sstg->dstXY, ((dy1) << 16) | ((dx1) & 0xFFFF)); \
SET(sstg->launch[launchc++ % 10], ((sy1) << 16) | ((sx1) & 0xFFFF))


#define HBLIT_START(x,y,ox,oy,sx,sy)\
SET(sstg->dstXY, ((y) << 16) | ((x) & 0xFFFF)); \
SET(sstg->srcXY, 0)

#define XGETARG() opts[1] ? done = 1, ++opts : \
			  (--argc > 0) ? done = 1, *++argv : \
					 (char *)Xusage()

static FxU32 charnum = 0;
static FxU32 charline = 1;
static FxU32 srcformat = 1;
static FxU32 dstformat = 3;
static FxU32 do_polylines = 0;
static FxU32 do_indlines = 0;
static FxU32 do_rectangles = 0;
static FxU32 do_polygons = 0;
static FxU32 do_screenblits = 0;
static FxU32 do_stretchblits = 0;
static FxU32 do_hostblits = 0;
static FxU32 pack_src = 0;
static FxU32 iters = 256;
static FxU32 stride = 1024;
static FxU32 srcsize = 10;
static FxU32 prim_width = 10;
static FxU32 prim_height = 10;
static FxU32 ropcode = 0xcccc;
static FxU32 reverse_blt = 0;
static FxU32 src_colorkey = 0;
static FxU32 dst_colorkey = 0;
static FxU32 align = 0;
static FxU32 transparent_mono = 0;
static FxU32 mono_pattern = 0;
static FxU32 tiled = 0;
static char *primitive;

char *
Xusage()
{
  gdbg_printf("\n");
  gdbg_printf("\"-xb #\"\tprimitive size\n");
  gdbg_printf("\"-xc #\"\tnumber of loops\n");
  gdbg_printf("\"-xd #\"\tdst format 1=>8, 3=>16, 4=>24, 5=>32\n");
  gdbg_printf("\"-xh\"\tHost blits\n");
  gdbg_printf("\"-xi\"\tIndependent lines\n");
  gdbg_printf("\"-xk\"\tsrc colorkey\n");
  gdbg_printf("\"-xK\"\tdst colorkey\n");
  gdbg_printf("\"-xl\"\tPolylines\n");
  gdbg_printf("\"-xM\"\tMono Pattern\n");
  gdbg_printf("\"-xo #\"\tRop\n");
  gdbg_printf("\"-xp\"\tpacked src\n");
  gdbg_printf("\"-xr\"\tRectangle fill\n");
  gdbg_printf("\"-xs #\"\tsrc format 0=>1, 1=>8, 3=>16, 4=>24, 5=>32\n");
  gdbg_printf("\"-xt\"\tScreen to Screen blits\n");
  gdbg_printf("\"-xT\"\tTransparent mono\n");
  gdbg_printf("\"-xu\"\tTiled memory\n");
  gdbg_printf("\"-xw #\"\tstride in bytes\n");
  gdbg_printf("\"-xz #\"\tStretch blits w/ src size\n");
  gdbg_printf("\n");
  exit(1);
  return(0);
}

void
XParseOpts(int argc, char **argv)
{
  char *opts = 0;
  FxBool aopt = 0;
  FxU32 bopt = 0;
  FxBool done;
  
  while ((--argc > 0) && (**++argv))
    {
		if (argv[0][0] != '-')
		  continue;
		if (argv[0][1] != 'x')
		  continue;
		
		/* now parse all extended parameters */
		done = 0;
		opts = &argv[0][2];
		if (*opts == '\0')
		  Xusage();
		
		while (!done && *opts)
		  {
			 switch (*opts)
				{
				case 'a': align = 1;
				  break;
				case 'b':
				  sscanf(XGETARG(), "%i", &prim_width);
				  break;
				case 'B':
				  sscanf(XGETARG(), "%i", &prim_height);
				  break;
				case 'c':
				  sscanf(XGETARG(), "%i", &iters);
				  break;
				case 'd':
				  sscanf(XGETARG(), "%i", &dstformat);
				  break;
				case 'h':
				  gdbg_printf("INFO: Host to Screen blits\n");
				  do_hostblits = 1;
				  primitive = "Hblt";
				  break;
				case 'i':
				  gdbg_printf("INFO: Independent lines\n");
				  do_indlines = 1;
				  primitive = "Iline";
				  break;
				case 'k': src_colorkey = 1;
				  break;
				case 'K': dst_colorkey = 1;
				  break;
				case 'l':
				  gdbg_printf("INFO: Poly lines\n");
				  do_polylines = 1;
				  primitive = "Pline";
				  break;
				case 'M': mono_pattern = 1;
				  break;
				case 'o':
				  sscanf(XGETARG(), "%i", &ropcode);
				  break;
				case 'p':
				  pack_src = 1;
				  break;
				case 'P':
				  gdbg_printf("INFO: Polygon fill\n");
				  do_polygons = 1;
				  primitive = "Poly";
				  break;
				case 'r':
				  gdbg_printf("INFO: Rectangle fill\n");
				  do_rectangles = 1;
				  primitive = "Rect";
				  break;
				case 'R':
				  reverse_blt = 1;
				  break;
				case 's':
				  sscanf(XGETARG(), "%i", &srcformat);
				  break;				  
				case 't':
				  gdbg_printf("INFO: Screen to Screen blits\n");
				  do_screenblits = 1;
				  primitive = "Blt";
				  break;
				case 'T': transparent_mono = 1;
				  break;
				case 'u':
				  tiled = 1;
				  break;
				case 'w':
				  sscanf(XGETARG(), "%i", &stride);
				  break;
				case 'z':
				  gdbg_printf("INFO: Stretch blits\n");
				  do_stretchblits = 1;
				  primitive = "StretchBlt";
				  sscanf(XGETARG(), "%i", &srcsize);
				  break;
				default:
				  Xusage();
				}
			 
			 opts += 1;
		  }
    }
}

void
random_range(FxU32 xbase, FxU32 ybase, FxI32 xrange, FxI32 yrange, FxU32 *x, FxU32 *y)
{
  FxU32 axes;
  FxU32 minor;
  FxU32 sign;
  FxU32 xoff;
  FxU32 yoff;

  if (xrange < 0) { /* set distance */
	 /* Pick xmajor or ymajor */
	 if (iRandom(4) & 1) xrange = -xrange;
	 *x = xbase + xrange;
	 if ((*x > diago.xmaxscreen) || (*x < 0)) {
		*x -= xbase;
		*x = -*x + xbase;
	 }
  }
  else *x = xbase + iRandom(xrange);

  if (yrange < 0) {
	 if (iRandom(4) & 1) yrange = -yrange;
	 *y = ybase + yrange;
	 if ((*y > diago.ymaxscreen) || (*y < 0)) {
		*y -= ybase;
		*y = -*y + ybase;
	 }
  }
  else *y = ybase + iRandom(yrange);
}

void triangle(SstGRegs *sstg)
{
  FxU32 x0, y0, x1, y1, x2, y2;
  float l0, theta0, l_perp, perp_intersect, int_x, int_y, min_size;
  FxU32 launchc;

  min_size = sqrt((float)(2.0*prim_width));
  random_range((2*min_size), (2*min_size), diago.xmaxscreen - (4*min_size), diago.ymaxscreen - (4*min_size), &x0, &y0);
  l0 = (float)(min_size + iRandom(min_size));
  theta0 = (float)iRandom(360);
  x1 = x0 + (FxU32)(l0 * sin(2.0*3.1415926*theta0/360.0));
  y1 = y0 + (FxU32)(l0 * cos(2.0*3.1415926*theta0/360.0));
  l0 = sqrt((x1-x0)*(x1-x0) + (y1-y0)*(y1-y0));
  l_perp = ((float)2.0*prim_width)/l0;
  perp_intersect = ((float)iRandom(1000))/1000.0;
  int_x = x0*(1.0-perp_intersect) + x1*(perp_intersect);
  int_y = y0*(1.0-perp_intersect) + y1*(perp_intersect);
  x2 = (FxU32)(int_x + 0.5 + (l_perp * sin(2.0*3.1415926*(theta0+90.0)/360.0)));
  y2 = (FxU32)(int_y + 0.5 + (l_perp * cos(2.0*3.1415926*(theta0+90.0)/360.0)));
  gdbg_printf ("Poly (%d,%d) (%d, d) (%d,%d) l0=%f, lp=%f\n", x0, y0, x1, y1, x2, y2, l0, l_perp);

  if ( (y0<y1) && (y0<y2) ) {
    SET(sstg->dstXY, (y0 << 16) | (x0 & 0xFFFF));
    SET(sstg->srcXY, (y0 << 16) | (x0 & 0xFFFF));
    SET(sstg->launch[launchc++ % 10], (y1 << 16) | (x1 & 0xFFFF));
    SET(sstg->launch[launchc++ % 10], (y2 << 16) | (x2 & 0xFFFF));
    if (y1>y2) SET(sstg->launch[launchc++ % 10], (y1 << 16) | (x1 & 0xFFFF));
    else SET(sstg->launch[launchc++ % 10], (y2 << 16) | (x2 & 0xFFFF));
  } else if (y1<y2) {
    SET(sstg->dstXY, (y1 << 16) | (x1 & 0xFFFF));
    SET(sstg->srcXY, (y1 << 16) | (x1 & 0xFFFF));
    SET(sstg->launch[launchc++ % 10], (y2 << 16) | (x2 & 0xFFFF));
    SET(sstg->launch[launchc++ % 10], (y0 << 16) | (x0 & 0xFFFF));
    if (y0>y2) SET(sstg->launch[launchc++ % 10], (y0 << 16) | (x0 & 0xFFFF));
    else SET(sstg->launch[launchc++ % 10], (y2 << 16) | (x2 & 0xFFFF));
  } else {
    SET(sstg->dstXY, (y2 << 16) | (x2 & 0xFFFF));
    SET(sstg->srcXY, (y2 << 16) | (x2 & 0xFFFF));
    SET(sstg->launch[launchc++ % 10], (y0 << 16) | (x0 & 0xFFFF));
    SET(sstg->launch[launchc++ % 10], (y1 << 16) | (x1 & 0xFFFF));
    if (y1>y0) SET(sstg->launch[launchc++ % 10], (y1 << 16) | (x1 & 0xFFFF));
    else SET(sstg->launch[launchc++ % 10], (y0 << 16) | (x0 & 0xFFFF));
  }

}

void
main (int argc, char **argv)
{
  FxU32 startTime, endTime;
  SstRegs *sst;
  SstGRegs *sstg;
  FxU32 n, i;
  FxU32 x1, y1, x2, y2;
  FxU32 launchc;
  FxU32 sign;
  FxU32 pixelsout;
  FxU32 sx, sy, dx, dy;
  FxU32 dwords;

  sst = SST_BEGIN2d(argc, argv);
  sstg = SSTG_CHIP(sst);
  stride = diago.xmaxscreen * 2;
  XParseOpts(argc, argv);

  SET(sstg->clip0min, 0);
  SET(sstg->clip1min, 0);
  sstg_setpattern_random(sstg);

  // Set mono pattern to all ones, so we don't get bogus #s with this!
  SET(sstg->pattern0alias, 0xffffffff);
  SET(sstg->pattern1alias, 0xffffffff);

  SET(sstg->commandEx, (dst_colorkey<<1) & src_colorkey);
  SET(sstg->colorFore, 0xeeeeee);
  SET(sstg->colorBack, 0xaaaaaa);
  SET(sstg->rop, (ropcode << 24) | (ropcode << 16) | (ropcode << 8) |
	 (ropcode) );

  if (pack_src==1) {
	switch (srcformat) {
		case 0: pack_src = 1; break;
		case 1: pack_src = 1; break;
		case 3: pack_src = 2; break;
		case 4: pack_src = 1; break;
		default: pack_src = 3;
	}
  }

  SET(sstg->srcFormat, stride | (srcformat << 16) | (pack_src << 22));
  SET(sstg->dstFormat, stride | (dstformat << 16));
  SET(sstg->clip0max, (4*diago.ymaxscreen<<16) | 4*diago.xmaxscreen);
  SET(sstg->clip1max, (4*diago.ymaxscreen<<16) | 4*diago.xmaxscreen);

  SET(sstg->srcBaseAddr, (tiled) ? 0x80000000 : 0);
  SET(sstg->dstBaseAddr, (tiled) ? 0x80000000 : 0);

  /* Initial code */
  if (do_rectangles)
	 SET(sstg->command, 0x5 | ((ropcode & 0xFF) << 24) | (mono_pattern<<13) | (transparent_mono<<16) | (1<<10));
  else if (do_polygons)
	 SET(sstg->command, 0x8 | ((ropcode & 0xFF) << 24) | (mono_pattern<<13) | (transparent_mono<<16));
  else if (do_screenblits) {
	 SET(sstg->command, 0x1 | ((ropcode & 0xFF) << 24) | (reverse_blt<<14) | (mono_pattern<<13) | (transparent_mono<<16) | (1<<10));
	 sstg_init_random_memory(NULL);
  }
  else if (do_stretchblits) {
	 SET(sstg->command, 0x2 | ((ropcode & 0xFF) << 24) | (mono_pattern<<13) | (transparent_mono<<16) | (1<<10));
	 sstg_init_random_memory(NULL);
  }
  else if (do_hostblits) {
	 SET(sstg->command, 0x3 | 0x400 | ((ropcode & 0xFF) << 24) | (mono_pattern<<13) | (transparent_mono<<16) | (1<<10));
	 HBLIT_START(0,0,0,0,prim_width,prim_height);
  }

  IDLE(sst);
  startTime = DIAG_TIME();
  gdbg_printf ("Beginning %d primitives @ %d\n", iters, startTime);

  while (DIAG_STARTPASS()) {
	 for (n = 0; n < iters; n++) {
		gdbg_printf ("primitive %d\n", n);
		if (do_indlines) {
		  random_range(0, 0, diago.xmaxscreen - prim_width, 
							diago.ymaxscreen - prim_height, &x1, &y1);
		  sign = (iRandom(4) & 1) ? -1 : 1;
		  random_range(x1, y1, sign*(prim_width -1), -sign*(prim_height -1), &x2, &y2);
		  IND_LINE(x1, y1, x2, y2);
		}
		else if (do_polylines) {
		  if (!n) {
			 random_range(0, 0, diago.xmaxscreen - prim_width, 
							  diago.ymaxscreen - prim_height,&x1, &y1);
			 sign = (iRandom(4) & 1) ? -1 : 1;
			 random_range(x1, y1, sign*(prim_width - 1), -sign*(prim_height - 1), 
							  &x2, &y2);
			 IND_LINE(x1, y1, x2, y2);
			 SET(sstg->command, 0x07 | ((ropcode & 0xFF) << 24));
		  }
		  else {
			 sign = (iRandom(4) & 1) ? -1 : 1;
			 random_range(x2, y2, sign*prim_width, -sign*prim_height, &x1, &y1);
			 if (x1 > diago.xmaxscreen) x2 = x2 - (x1 - x2); else x2 = x1;
			 if (x1 < 0) x2 = x2 + (x2 - x1); else x2 = x1;
			 if (y1 > diago.ymaxscreen) y2 = y2 - (y1 - y2); else y2 = y1;
			 if (y1 < 0) y2 = y2 + (y2 - y1); else y2 = y1;
			 POL_LINE(x2, y2);
		  }
		}
		else if (do_rectangles) {
		  random_range(0, 0, diago.xmaxscreen - prim_width, 
							diago.ymaxscreen - prim_height, &x2, &y2);
		  sign = (iRandom(4) & 1) ? -1 : 1;
		  RECTANGLE(prim_width, prim_height, x2, y2);		  
		}
		else if (do_screenblits) {
		  random_range(0, 0, diago.xmaxscreen - prim_width,
							diago.ymaxscreen - prim_height, &sx, &sy);
		  if (pack_src) { sx = sx&0xfffffff0; sy = sy&0xfffffff0; };
		  random_range(0, 0, diago.xmaxscreen - prim_width,
							diago.ymaxscreen - prim_height, &dx, &dy);
		  if (align) {
			sx &= 0xfff0;
			dx &= 0xff80;
		  }

		  SBLIT(sx, sy, dx, dy, prim_width, prim_height);
		}
		else if (do_stretchblits) {
		  random_range(0, 0, diago.xmaxscreen - srcsize,
							diago.ymaxscreen - srcsize, &sx, &sy);
		  random_range(0, 0, diago.xmaxscreen - prim_width,
							diago.ymaxscreen - prim_height, &dx, &dy);
		  STRETCH_BLIT(sx, sy, dx, dy, prim_width, prim_height, srcsize, srcsize);
		}
		else if (do_hostblits) {
		  if (prim_width==9) {
		      if (charnum>70) {
			   SET(sstg->dstXY, ((charline++) << 20) );
			   charnum = 0;
		      } else charnum++;
		  } else {
		      random_range(0, 0, diago.xmaxscreen - prim_width,
					 diago.ymaxscreen - prim_height, &dx, &dy);
		      SET(sstg->dstXY, (dy<<16) | (dx & 0xFFFF));
		 }
		 SET(sstg->dstSize, (prim_height << 16) | (prim_width & 0xFFFF));

		 if (pack_src) {
		    switch (srcformat) {
		       case 0: dwords = (((((prim_width-1)>>3)+1)*prim_height-1)>>2) + 1;
		       break;
		       case 1: dwords = ((prim_width*prim_height-1)>>2) + 1;
		       break;
		       case 3: dwords = ((prim_width*2*prim_height-1)>>2) + 1;
		       break;
		       case 4: dwords = ((prim_width*3*prim_height-1)>>2) + 1;
		       break;
		       case 5: dwords = prim_width*prim_height;
		       break;
		    }
		 } else {
		    switch (srcformat) {
		       case 0: dwords = (((prim_width-1)>>5)+1)*prim_height;
		       break;
		       case 1: dwords = (((prim_width-1)>>2)+1)*prim_height;
		       break;
		       case 3: dwords = (((prim_width-1)>>2)*2+1)*prim_height;
		       break;
		       case 4: dwords = (((prim_width-1)>>2)*3+1)*prim_height;
		       break;
		       case 5: dwords = prim_width*prim_height;
		       break;
		    }
		 }

		 for (i=0; i<dwords; i++) {
		     SET(sstg->launch[launchc++ % 10], iRandom(-1));
		 }
		}
		else if (do_polygons) {
		  triangle(sstg);
		}
	 }
  }
  NOP();
  IDLE(sst);
  endTime = DIAG_TIME();
  gdbg_printf ("end of %d primitives at time = %d ns\n", iters,  endTime);
  gdbg_printf ("%d primitives in %d ns, %.2fM primitives/sec\n",
					iters, endTime-startTime,iters*1e3/(endTime-startTime));

  pixelsout = CSIM_PRIVATE(diago.sstCSIM)->pixelsOut2d;
  gdbg_printf ("Fill rate = %.2fM pixels/sec\n", 
					pixelsout*1e3/(endTime-startTime));

  gdbg_printf ("EXCEL: %s-%s-u%d,%d,%d,%f,%f\n", primitive, (tiled) ? "tiled" :
					"linear", dstformat, prim_width, dstformat, 
					iters*1e3/(endTime-startTime),
					pixelsout*1e3/(endTime-startTime));
  DIAG_PASS(0);
}
