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
** $Date: 10/11/00 8:11:17 PM$
*/

#include "udiag.h"
#include "sstdiag.h"

FxU32 srcFormat;

static FxU32 screen[MAXSCREEN][MAXSCREEN];
static FxU32 host_pixels[MAXSCREEN*MAXSCREEN];

static FxU32 greg_swap(FxU32 srcFormat, FxU32 data)
{
    if (srcFormat & SSTG_HOST_BYTE_SWIZZLE) {
        data = (data>>24) | ((data>>8)&0xFF00) | ((data<<8)&0xFF0000) | (data<<24);
    }
    if (srcFormat & SSTG_HOST_WORD_SWIZZLE) {
        data = (data>>16) | (data<<16);
    }
    return data;
}
 


// GREG - I want really slow data coming in!!!!
// this routine assumes 32-bit packed host blit data
// where stride==dstSize rounded to byte(align==0), word(align==1) or dword(align==3)
// it's main loop is centered around pixel counting,
// the inner loops decode multiple pixels out of a DWORD and also count pixels
void sendBltDataSlowly(SstGRegs *sstg, FxU32 align, FxU32 byte,
                       int xd, int yd, int w, int h, int ydir,
                       FxU32 srcFormat,
                       FxU32 host_pixels[])
{
    int i, x, size, pixels_sent, pixels_to_send, stalled;
    int xc,yc, n=0, L=0;
    FxU32 next_addr, addr, col;
    FxU32 pack = srcFormat & SSTG_SRC_PACK;
    FxI32 yInc = (ydir == 0) ? 1 : -1;
 
    addr = byte;
 
    addr &= ~3;
 
    align -= 1;
    i = 8*(addr&3);                             // starting bit in 1st dword
    pixels_to_send = w*h;                       // total # of pixels to send
    pixels_sent = 0;
    xc = xd;
    yc = yd;
    stalled = 0;
 
    switch(srcFormat & SSTG_SRC_FORMAT)
    {
      case SSTG_PIXFMT_1BPP:
          size = 1;
          while (pixels_sent < pixels_to_send)
          {     // for each pixel
              if (pixels_sent == 0 || i==0)
              {
                  col = iRandom(0xFFFFFFFF);    // get a random DWORD
                  SET(sstg->launch[L++], col);  // send it to chip
    // GREG - wait a while before sending launch data
#ifdef HAL_HSIM
    if ( diago.halInfo->hsim && !stalled) PCI_STALL(300);
    stalled = 1;
#endif
                  col = greg_swap(srcFormat, col);           // swap bytes/words
                  if (L > rRandom(27,31))       // if randomly close to end
                      L = iRandom(2);           // wrap to near beginning
              }
              if (ONSCREEN(xc,yc))
              {         // expand into pixels
                  host_pixels[n++] = ((col>>(i&~7)) >> (7 - i & 7)) & 1;
                  gdbg_info(7,"host_pixels[%d] = 0x%x\n",n-1,host_pixels[n-1]);
              }
              pixels_sent++;
              i += size;
              if (++xc >= xd + w)
              {         // done with this row
                  i = (i + align) & ~align;
                  xc = xd;
                  yc += yInc;
              }
              i &= 31;
          }
          break;
 
      case SSTG_PIXFMT_8BPP:
          size = 8;
          goto merge;
 
      case SSTG_PIXFMT_15BPP:
      case SSTG_PIXFMT_16BPP:
          size = 16;
      merge:
          if (i & (size-1))
              gdbg_error("hblt", "bad alignment\n");
          while (pixels_sent < pixels_to_send)
          {     // for each pixel
              if (pixels_sent == 0 || i==0)
              {
                  col = iRandom(0xFFFFFFFF);    // get a random DWORD
                  SET(sstg->launch[L++], col);  // send it to chip
    // GREG - wait a while before sending launch data
#ifdef HAL_HSIM
    if ( diago.halInfo->hsim && !stalled) PCI_STALL(300);
    stalled = 1;
#endif
                  col = greg_swap(srcFormat, col);           // swap bytes/words
                  if (L > rRandom(20,30))           // if randomly close to end
                      L = iRandom(2);           // wrap to near beginning
              }
              if (ONSCREEN(xc,yc))
              {         // expand into pixels
                  host_pixels[n++] = (col>>i) & (0xFFFFFFFF>>(32-size));
                  gdbg_info(7,"host_pixels[%d] = 0x%x\n",n-1,host_pixels[n-1]);
              }
              pixels_sent++;
              i += size;
              if (++xc >= xd + w)
              {         // done with this row
                  i = (i + align) & ~align;
                  xc = xd;
                  yc += yInc;
              }
              i &= 31;
          }
          break;
 
      case SSTG_PIXFMT_24BPP:
      {
          FxU32 last;                           // holds the last DWORD
 
          while (pixels_sent < pixels_to_send)
          {     // for each pixel
              if (pixels_sent == 0 || i>8)
              {
                  col = iRandom(0xFFFFFFFF);    // get a random DWORD
                  SET(sstg->launch[L++], col);  // send it to chip
    // GREG - wait a while before sending launch data
#ifdef HAL_HSIM
    if ( diago.halInfo->hsim && !stalled) PCI_STALL(300);
    stalled = 1;
#endif
                  col = greg_swap(srcFormat, col);           // swap bytes/words
                  if (L > rRandom(24,31))       // if randomly close to end
                      L = iRandom(2);           // wrap to near beginning
              }
              if (L <= 1 && pixels_sent==0)
              {
                  last = col;
                  if (i > 8)
                      continue;
              }
 
              switch(i)
              {
                case 0:
                case 32:
                    if (ONSCREEN(xc,yc))
                    {
                        host_pixels[n++] = col & 0xFFFFFF;
                        gdbg_info(7,"host_pixels[%d] = 0x%x\n",
                                  n-1,host_pixels[n-1]);
                    }
                    i = 0;
                    break;
                case 8:
                case 16:
                case 24:
                    if (ONSCREEN(xc,yc))
                    {
                        host_pixels[n++] = 0xFFFFFF & ((col<<(32-i)) |
                                                                (last>>i));
                        gdbg_info(7,"host_pixels[%d] = 0x%x\n",
                                  n-1,host_pixels[n-1]);
                    }
                    break;
                default:
                    GDBG_ERROR("hblt", "internal error in 24bpp i=%d\n",i);
              }
              pixels_sent++;
              i += 24;
              if (++xc >= xd + w)
              {         // done with this row
                  xc = xd;
                  yc += yInc;
                  i = (i + align) & ~align;
              }
              if (i > 32)
                  i -= 32;
              last = col;
          }
          break;
      }
 
// Stolen from sendBltData0(sstg, byte, 0, xd,yd, w,h, ydir, srcFormat,
          case SSTG_PIXFMT_32BPP:
	    // for each row of the blit
	    for (yc = yd; (ydir == 0) ? yc < yd+h : yc > yd-h; yc += yInc)
	    {
	        int i,b,x;
 
              for (x=0; x < w; x++)
              {
                  col = iRandom(0xFFFFFFFF);
                  SET(sstg->launch[8], col);
    // GREG - wait a while before sending launch data
#ifdef HAL_HSIM
    if ( diago.halInfo->hsim && !stalled) PCI_STALL(300);
    stalled = 1;
#endif
                  col = greg_swap(srcFormat, col);
                  if (ONSCREEN(xd+x,yc))
                      host_pixels[n++] = col;
              }
	    }
          break;
 
// Stolen from sendBltData0(sstg, addr, 0, xd,yd, w,h, ydir, srcFormat,
          case SSTG_PIXFMT_422YUV:
	    next_addr = addr;
	    // for each row of the blit
	    for (yc = yd; (ydir == 0) ? yc < yd+h : yc > yd-h; yc += yInc)
	    {
	        addr = next_addr;
	        next_addr = addr % 4;
              if (addr & 2)
              {
                  col = iRandom(0xFFFFFFFF);
                  SET(sstg->launch[4], col);
    // GREG - wait a while before sending launch data
#ifdef HAL_HSIM
    if ( diago.halInfo->hsim && !stalled) PCI_STALL(300);
    stalled = 1;
#endif
                  col = greg_swap(srcFormat, col);
                  if (ONSCREEN(xd,yc))
                      host_pixels[n++] = sstg_unpack_yuv(col, 1);
                  x = 1;
              }
              else
                  x = 0;
              for (x=x; x < w; x+=2)
              {
                  col = iRandom(0xFFFFFFFF);
                  SET(sstg->launch[5], col);
    // GREG - wait a while before sending launch data
#ifdef HAL_HSIM
    if ( diago.halInfo->hsim && !stalled) PCI_STALL(300);
    stalled = 1;
#endif
                  col = greg_swap(srcFormat, col);
                  for (i=0; i<2; i++)
                  {
                      if (ONSCREEN(xd+x+i,yc) && (x+i<w))
                      {
                          host_pixels[n++] = sstg_unpack_yuv(col, i);
                      }
                  }
              }
            }
          break;
 
// Stolen from sendBltData0(sstg, addr, 0, xd,yd, w,h, ydir, srcFormat,
          case SSTG_PIXFMT_422UYV:
            next_addr = addr;
            // for each row of the blit
            for (yc = yd; (ydir == 0) ? yc < yd+h : yc > yd-h; yc += yInc)
            {
                addr = next_addr;

              if (addr & 2)
              {
                  col = iRandom(0xFFFFFFFF);
                  SET(sstg->launch[4], col);
    // GREG - wait a while before sending launch data
#ifdef HAL_HSIM
    if ( diago.halInfo->hsim && !stalled) PCI_STALL(300);
    stalled = 1;
#endif
                  col = greg_swap(srcFormat, col);
                  if (ONSCREEN(xd,yc))
                      host_pixels[n++] = sstg_unpack_uyv(col, 1);
                  x = 1;
              }
              else
                  x = 0;
              for (x=x; x < w; x+=2)
              {
                  col = iRandom(0xFFFFFFFF);
                  SET(sstg->launch[5], col);
    // GREG - wait a while before sending launch data
#ifdef HAL_HSIM
    if ( diago.halInfo->hsim && !stalled) PCI_STALL(300);
    stalled = 1;
#endif
                  col = greg_swap(srcFormat, col);
                  for (i=0; i<2; i++)
                  {
                      if (ONSCREEN(xd+x+i,yc) && (x+i<w))
                      {
                          host_pixels[n++] = sstg_unpack_uyv(col, i);
                      }
                  }
              }
            }
          break;
 
      default:
          GDBG_ERROR("hblt", "invalid source format\n");
          break;
    }
}
 


//----------------------------------------------------------------------
// draw a host-to-scr blt, 
//----------------------------------------------------------------------
void sstg_drawHblt(SstGRegs *sstg, int xs, int ys, int xd, int yd, int w, int h, FxU32 cmd)
{
    FxU32 addr, bit, byte;
    FxU32 ydir;
    
    SET(sstg->command, cmd | SSTG_HOST_BLT);

    // old: SET(sstg->srcXY,(ys<<16) | (xs & 0xFFFF));
    // new: convert xs, ys to byte/bit position
    //
    addr = sstg_compute_blit_address(srcFormat, 0, xs, ys, w);
    if ((srcFormat & SSTG_SRC_FORMAT) == SSTG_PIXFMT_1BPP)
    {
	byte = addr % 4;
	bit = (byte * 8) + (xs % 8);

	gdbg_info(7, "hblt.exe: starting bit position: %d\n", bit);
	// should ignore everything above bit 31
	SET(sstg->srcXY, (iRandom(0xFFFFFFFF) & ~31) | bit);
    }
    else
    {
	// color format -- just send the byte address
	byte = addr & 3;
	gdbg_info(7, "hblt.exe: starting byte position: %d\n", byte);
	// should ignore everything above bit 2
	SET(sstg->srcXY, (iRandom(0xFFFFFFFF) & ~3) | byte);
    }
    

    SET(sstg->dstXY,(yd<<16) | (xd & 0xFFFF));
    SET(sstg->dstSize,(h<<16) | w);

    ydir = cmd & SSTG_YDIR;

    sendBltDataSlowly(sstg, 32, byte, xd,yd, w,h, ydir, srcFormat, host_pixels);
}



static int maxHeight = -1;

char *
Xusage()
{
    gdbg_printf("\n");
    gdbg_printf("\"-xh #\"\toverrides -t with random height 0 <= h <= # (default use -t)\n");
    gdbg_printf("\n");
    exit(1);
    return(0);
}

/* myParseOpts
 *
 * look for "-x" options and interpret them for this test
 *
 */

#define XGETARG() opts[1] ? done = 1, ++opts : \
			  (--argc > 0) ? done = 1, *++argv : \
					 (char *)Xusage()

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
	      case 'h':
		  sscanf(XGETARG(), "%i", &maxHeight);
		  break;

	      default:
		  Xusage();
	    }
	    
	    opts += 1;
	}
    }
}


void
main (int argc, char **argv)
{
    int j,n, monotrans;
    long xs,ys, xd,yd, w,h, xc,yc;
    FxU32 max, min;
    FxU32 rop, cmdops,cmdXops;
    FxU32 cfore,cback,cdest;
    SstRegs *sst;
    SstGRegs *sstg;
    FxI32 ycStart, ycEnd, ycInc;

    sst = SST_BEGIN2d(argc,argv);
    sstg = SSTG_CHIP(sst);

    XParseOpts(argc, argv);

    SET(sstg->clip0min, 0x00000000);
    SET(sstg->clip0max,(diago.ymaxscreen<<16) | diago.xmaxscreen);

    while (DIAG_STARTPASS())			// for each pass
    for (n=0; n<256; n++) {
	if ((n % 7) == 0) {			// init pattern every 7 times
	    sstg_setpattern_random(sstg);
	}
	if (diago.clamp) {			// if random clipping
	// GREG
	    min = (iRandom(87)<<16) | iRandom(99);
	    max = ((diago.ymaxscreen-iRandom(25)-1)<<16) | (diago.xmaxscreen-iRandom(50)-1);
	    SET(sstg->clip0min, min);
	    SET(sstg->clip0max, max);
	    SET(sstg->clip1min, min);
	    SET(sstg->clip1max, max);
	}

	for (j=0; j<2; j++) {			// and do a number of tests
	    int pix = 0;

	    xyRandom(&xs,&ys);			// NOTE: source is on screen
	    do {				// generate random width height
		w = rRandom(1,diago.tsize);
		if (maxHeight < 0)
		    h = rRandom(1,diago.tsize);
		else
		    h = iRandom(maxHeight);
	    } while (!ONSCREEN(xs+w-1, ys+h-1));// until it's entirely onscreen

	    xyRandom(&xd,&yd);		// anywhere
	// GREG
	    yd = (min >> 16) - (h>>1);

	    // now pick a random source format (valid with destination)
	    // 8bpp dest requires 1 or 8 bpp source
	    // 16,24,32 bpp dest require anything but 8bpp
	    // host blit never has tiled source, so pass in 0
	    diago.dstFormat <<= SSTG_DST_FORMAT_SHIFT;
	    srcFormat = sstg_random_srcFormat(diago.dstFormat, 0);
	    srcFormat &= ~(SSTG_SRC_PACK);
	    srcFormat |= SSTG_SRC_PACK_32;
	    diago.dstFormat >>= SSTG_DST_FORMAT_SHIFT;
	    
	    // also pick random byte/word swizzle
	    if (diago.bilinear) {		// -b (byte/word swapping)
		if (iRandom(1)) srcFormat |= SSTG_HOST_BYTE_SWIZZLE;
		if (iRandom(1)) srcFormat |= SSTG_HOST_WORD_SWIZZLE;
	    }
	    SET(sstg->srcFormat, srcFormat);

#define SRCFORMAT (srcFormat & SSTG_SRC_FORMAT)
	    sstg_random_command_bits(&cmdops,&cmdXops);
	    if (SRCFORMAT == SSTG_PIXFMT_1BPP)
		cmdXops &= ~SSTG_EN_SRC_COLORKEY_EX;	// disable source colorkey
	    if (j&1) cmdXops |= SSTG_EN_DST_COLORKEY_EX;	// 1/2 the time, enable dstcolorkey
	    else cmdXops &= ~SSTG_EN_DST_COLORKEY_EX;		// other half, disable dstcolorkey
	    cfore = iRandom(0xFFFFFFFF);
	    SET(sstg->colorFore, cfore);
	    cdest = ONSCREEN(xd,yd) ? screen[yd][xd] : 0;
	    rop = sstg_random_colors(sstg, cfore,&cback,cdest);
	    if (diago.rectangular) {		// if random rops
		cmdops |= iRandom(0xFF) << SSTG_ROP0_SHIFT;
		rop = iRandom(0xFFFFFF);	// then pick totally random
	    }
	    else if (rop == 0) 			// else use SRC
		cmdops |= SSTG_ROP_SRC << SSTG_ROP0_SHIFT;
	    else
		rop = SSTG_ROP_SRC << (rop-1)*8;

	// GREG
	    cmdops &= ~SSTG_YDIR;

	    SET(sstg->rop, rop);		// set the rop
	    SET(sstg->commandEx, cmdXops);
	    sstgCheckForIdle(sst, cmdXops);

	    gdbg_info(2,"\n");
	    gdbg_info(2,"xs,ys = %d,%d    xd,yd = %d,%d    w,h = %d,%d    pox,y = %d,%d\n",
			xs,ys, xd,yd, w,h,
			((cmdops & SSTG_X_PATOFFSET)>>SSTG_X_PATOFFSET_SHIFT) & 7,
			((cmdops & SSTG_Y_PATOFFSET)>>SSTG_Y_PATOFFSET_SHIFT) & 7);
	    sstg_print_stuff(cmdops, cmdXops, rop, srcFormat, cfore, cback);

	    sstg_drawHblt(sstg,xs,ys, xd,yd,w,h, cmdops);
	    sstg_idle(sst);			// wait for the command to complete
	    monotrans = (cmdops & SSTG_TRANSPARENT) &&
			((srcFormat & SSTG_SRC_FORMAT) == SSTG_PIXFMT_1BPP);

	    if (cmdops & SSTG_YDIR)
	    {
		// negative y direction
		ycStart = yd + 1;
		ycEnd = yd - h;
		ycInc = -1;		
	    }
	    else
	    {
		// positive y direction
		ycStart = yd - 1;
		ycEnd = yd + h;
		ycInc = 1;
	    }
	    
	    // check the entire rectangle
	    for (yc = ycStart;
		 ycInc > 0 ? yc <= ycEnd : yc >= ycEnd;
		 yc += ycInc)
	    for (xc = xd-1; xc <= xd+w; xc++)	// with a 1 pixel border
	    if (ONSCREEN(xc,yc)) {
		cdest = screen[yc][xc];		// compute predicted result
		if (xc < xd || xc >= xd+w ||
		    ((ycInc == 1) && ((yc < yd) || (yc >= yd+h))) ||
		    ((ycInc == -1) && ((yc > yd) || (yc <= yd-h))))
		{
		    // if outside rect, then unchanged
		    cdest = sstg_destination_mask(cdest);
		    gdbg_info(8,"checking %d,%d 0x%x (outside)\n",xc,yc,cdest);
		    DIAG_TEST_PIXEL(CSIM_BUF_2D_DST,xc,yc,cdest); // test the pixel
		}
		else {
		    FxU32 csrc = host_pixels[pix++];
		    int srcKey = sstg_src_colorkey(csrc);
		    
		    if (monotrans && !(csrc&1)) {
			gdbg_info(8,"checking %d,%d 0x%x (monotrans)\n",xc,yc,cdest);
			DIAG_TEST_PIXEL(CSIM_BUF_2D_DST,xc,yc,cdest); // test the pixel
			continue;
		    }
		    GDBG_INFO(8,"converting color 0x%x\n",csrc);
		    // NOTE: call into csim to convert colors
		    csrc = csimColorConvert(&CSIM_PRIVATE(diago.sstCSIM)->gui, csrc);

		    cdest = sstg_check_pixel(xc,yc,cmdops,cmdXops, rop,csrc,cdest,srcKey);
		    screen[yc][xc] = cdest;	// update our shadow screen
		}
	    }

	    // now verify that dstXY got updated properly
	    if (diago.updatexy && diago.checkEveryTriangle) {
		FxU32 new, expected;

		if ( (cmdops & SSTG_GO) == 0  &&  (w*h) == 0 ) {
		  GDBG_INFO(0,"skipping update\n");
		  expected = (xd & 0x1FFF) + ((yd & 0x1FFF)<<16);
		} else {
		  expected = (cmdops & SSTG_UPDATE_DSTX ? xd+w : xd) & 0x1FFF;
		  expected += ((cmdops & SSTG_UPDATE_DSTY ? yd+h : yd)& 0x1FFF)<<16;
		}
		new = GET(sstg->dstXY);
		DIAG_TESTREG32("dstXY",expected,new);
	    }
	}
	if (DIAG_EXCEEDED_PIXEL_LIMIT())
	    break;
    }
    DIAG_PASS(0);
}
