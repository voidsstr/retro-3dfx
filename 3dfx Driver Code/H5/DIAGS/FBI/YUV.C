/*
** Copyright (c) 1995, 3Dfx Interactive, Inc.
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
** Last Edited: Wed Jul  2 00:11:07 1997 by psmith (Phil Smith (x2456)) on vlsi2
**
** $Revision: 2$ 
** $Date: 10/11/00 8:10:52 PM$ 
**
*/

#include "udiag.h"
#include "sstdiag.h"
#include "hsimio.h"

#define ADDR(x,y)    ( (((x)<<SST_YUV_ADDR_X_SHIFT) | ((y)<<SST_YUV_ADDR_Y_SHIFT)) << 1)

static FxU32 memAddr(FxU32 base, FxU32 stride, int depth, int x, int y, int tiled )
{
  FxU32 addr;
  if ( tiled )
    addr = tiledAddress(base,stride,depth,x,y);
  else
    addr = base + y*stride + x*depth;
  return addr;
}

#define MAX_BURST_SIZE  64
#define YUV_MEM_SIZE    (1024*1024)

static struct { 
  FxU8 y[2], u, v;
} mem[1024][1024];

void main (int argc, char **argv)
{
    SstRegs *sst;
    SstCRegs *sstc;
    CsimPrivate    *cpriv;
    FxU8 *yuv;
    int base, stride, tiled, nbytes;
    int x, y, i, j, n, t;
    int burstSize;
    int dummy, w, h, r;
    FxU32 amask, dmask;
    FxU32 yuvBaseAddr, yuvStride;
    FxU32 addr, data;
    FxU32 d;

    // Init the Hardware
    sst = SST_BEGIN( argc, argv );
    sstc = (SstCRegs *)(SST_CMDAGP_ADDRESS(sst));
    cpriv = CSIM_PRIVATE(diago.sstCSIM);
    yuv = (FxU8 *) SST_YUV_ADDRESS(sst);

    if (diago.deviceID == SST_DEVICE_ID_SST96 || diago.deviceID == SST_DEVICE_ID_SST1) {
	GDBG_ERROR("yuv", "Can't run this diag on SST96 or SST1\n");
	DIAG_PASS(0);
	exit(0);
    }    
    
    while (DIAG_STARTPASS()) {

      // load the config register at the beginning of each pass
      do {
	base = rRandom(16*1024,cpriv->memorySizeInBytes) & (~0xF);   // 16-byte aligned
	base &= SST_YUV_BASE_ADDR>>SST_YUV_BASE_ADDR_SHIFT;

	tiled = diago.ytiled;
	if ( tiled < 0 )
	  tiled = iRandom(1);

	if ( tiled ) {
	  stride = rRandom(1,SST_YUV_TILE_STRIDE>>SST_YUV_STRIDE_SHIFT);
	  sstBoundingBox(base,cpriv->memorySizeInBytes,stride,2,&dummy,&w,&h,&r);
	} else {
	  stride = rRandom(16,SST_YUV_LINEAR_STRIDE>>SST_YUV_STRIDE_SHIFT) & ~0xF;
	  w = MIN(1024,stride/2);
	  h = (cpriv->memorySizeInBytes - base) / stride;
	}
	if ( w > 1024 )
	  w = 1024;
	if ( h > 1024 )
	  h = 1024;

      } while ( w == 0 || h == 0 );

      yuvBaseAddr = base<<SST_YUV_BASE_ADDR_SHIFT;
      SET(sstc->yuvBaseAddr,base);

      yuvStride = stride<<SST_YUV_STRIDE_SHIFT;
      if ( tiled )
	yuvStride |= SST_YUV_MEMORY_TYPE;
      else
	yuvStride &= ~SST_YUV_MEMORY_TYPE;
      SET(sstc->yuvStride,yuvStride);

      GDBG_INFO(1,"base=0x%x, tiled=%d, stride=%d, w,h=%d,%d\n",base,tiled,stride,w,h);

      for (n=0; n<100; n++) {
	
	// randomly select 8, 16, or 32-bit writes
	nbytes = 1 << iRandom(2);

	amask = ~(nbytes-1);
	dmask = SST_MASK(nbytes*8);

	// randomly select y, u, or v (force y if not enough memory for u,v)
	t = (w<2 || h<2) ? 0 : iRandom(2);

	// randomly select burst length
	if ( iRandom(5) )
	  burstSize = rRandom(1, MAX_BURST_SIZE);
	else 
	  burstSize = 1;
	
	// generate a random starting point
	if ( t == 0 ) {
	  x = iRandom(w-1) & amask;
	  y = iRandom(h-1);
	  if ( x + burstSize*nbytes > w )
	    burstSize = (w-x)/nbytes;
	} else {
	  x = iRandom(w/2-1) & amask;
	  y = iRandom(h/2-1);
	  if ( x + burstSize*nbytes > w/2 )
	    burstSize = (w/2-x)/nbytes;
	}  

	// generate a burst of address/data pairs
	GDBG_INFO(1,"%s burst of %d %dbit %s, starting at x,y=%d,%d(0x%x,0x%x)\n",
		  tiled ? "Tiled":"Linear",burstSize,nbytes*8,(t==0?"Y":(t==1?"U":"V")),x,y,x,y);
	
	x -= nbytes;
	for ( i=0; i<burstSize; i++ ) {
	  x += nbytes;
	  addr = (t<<20) | (x<<SST_YUV_ADDR_X_SHIFT) | (y<<SST_YUV_ADDR_Y_SHIFT);
	  data = iRandom(~0U) & dmask;

	  GDBG_INFO(20,"x,y=%d,%d(0x%x,0x%x), addr=0x%x, data=0x%x\n",x,y,x,y,addr,data);

	  for ( j=0; j<nbytes; j++ ) {
	    d = (FxU8) ((data>>(j*8)) & 0xFF);
	    switch (t) {
	    case 0: // y
	      mem[(x+j)/2][y].y[(x+j)%2] = (FxU8) d;
	      break;
	    case 1: // u
	      mem[x+j][2*y].u = (FxU8) d;
	      mem[x+j][2*y+1].u = (FxU8) d;
	      break;
	    case 2: // v
	      mem[x+j][2*y].v = (FxU8) d;
	      mem[x+j][2*y+1].v = (FxU8) d;
	      break;
	    }
	  }
	  
	  switch (nbytes) {
	  case 1: {FxU8  *a = (FxU8  *) &yuv[addr]; SET8 (*a, (FxU8) data);} break;
	  case 2: {FxU16 *a = (FxU16 *) &yuv[addr]; SET16(*a, (FxU16)data);} break;
	  case 4: {FxU32 *a = (FxU32 *) &yuv[addr]; SET  (*a, (FxU32)data);} break;
	  }

	}

      }
      
      // wait for writes to complete
      sst_idle_really(sst);

      // check memory contents
      for ( x=0; x<w; x+=2 ) {
	for ( y=0; y<h; y++ ) {
	  addr = memAddr(base,stride,2,x,y,tiled);
	  d = mem[x/2][y].y[0] | (mem[x/2][y].u<<8) | (mem[x/2][y].y[1]<<16) | (mem[x/2][y].v<<24);
	  DIAG_TEST_MEM(addr,d,4);
	  if ( d != 0 ) 
	    DIAG_FORCE_MEM(addr,0,4);
	}
      }
      
      // clean memory contents for next pass
      memset(mem,0,sizeof(mem));

    }

    DIAG_PASS(0);
}
