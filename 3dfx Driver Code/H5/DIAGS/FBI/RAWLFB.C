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
** Last Edited: Mon Aug 25 17:19:02 1997 by psmith (Phil Smith (x2456)) on vlsi2
**
** $Revision: 2$ 
** $Date: 10/11/00 8:10:23 PM$ 
**
*/

#include "udiag.h"
#include "sstdiag.h"
#include "hsimio.h"

static FxU32 memAddr(FxU32 base, FxU32 stride, int depth, int x, int y, int tiled )
{
  FxU32 addr;
  
  if ( tiled )
    addr = tiledAddress(base,stride,depth,x,y);
  else
    addr = base + y*stride + x*depth;
   
  return addr;
}

#define MAX_BURST_SIZE  	64
#define MAX_READ_OVERLAP_SIZE   16

void main (int argc, char **argv)
{
    SstRegs        *sst;
    SstIORegs      *sstio;
    CsimPrivate    *cpriv;
    FxU8 *lfb;
    int tBase, tStride, aStride;
    int x, y, xmax, ymax, i, m, n, rows, maxBurst;
    int width, height;
    FxU32 amask, dmask;
    int tiled, burstSize, readSize, nbytes;
    FxU32 lfbMemoryConfig;
    FxU32 a, d;
    FxI32 xref[MAX_BURST_SIZE+MAX_READ_OVERLAP_SIZE];
    FxU32 waddr[MAX_BURST_SIZE], raddr[MAX_BURST_SIZE];
    FxU32 data[MAX_BURST_SIZE];

    // Don't let this diag be run with command fifo on
    for(m=0; m<argc; m++)
      {
	if(!strncmp(argv[m], "-W", 2))
	  {
	    GDBG_INFO(0, "\n");
	    GDBG_INFO(0, "\n");
	    GDBG_INFO(0, "\n");
	    GDBG_INFO(0, "Warning: Stripping this arg \"%s\"!\n", argv[m]);
	    GDBG_INFO(0, "\n");
	    GDBG_INFO(0, "\n");
	    GDBG_INFO(0, "\n");
	    for(n=m; n<argc-1; n++)
	      argv[n] = argv[n+1];
	    
	    argc--;
	    m--;
	  }	
      }

    // Init the Hardware
    sst = SST_BEGIN( argc, argv );
    sstio = (SstIORegs *)(SST_IO_ADDRESS(sst));
    lfb = (FxU8 *)(SST_BASE_ADDRESS(sst) + SST_RAW_LFB_OFFSET);
    cpriv = CSIM_PRIVATE(diago.sstCSIM);

    if(diago.sliEnabled)
      {
	GDBG_INFO(0, "Warning! rawlfb doesn't work with SLI enabled.\n");
	GDBG_INFO(0, "Just passing the diag and quitting\n");
	DIAG_PASS(0);
	exit(0);
      }

    if (diago.deviceID == SST_DEVICE_ID_SST96 || diago.deviceID == SST_DEVICE_ID_SST1) {
	GDBG_ERROR("rawlfb", "Can't run this diag on SST96 or SST1\n");
	DIAG_PASS(0);
	exit(0);
    }    
    
    if ( diago.writeFifo ) {
      GDBG_ERROR("rawlfb","CMDFIFO packetizer doesn't support raw lfb writes\n");
      DIAG_PASS(0);
      exit(0);
    }

    while (DIAG_STARTPASS()) {

      xmax = ymax = 0;

      for (n=0; n<100; n++) {
	
	// load the config register at the beginning of each pass,
	// then occassionally afterward
	if ( iRandom(3) == 0 || n == 0 ) {

	  if ( iRandom(3) == 0 ) {	    
	    // try to hit some large y-values

	    //We can't run in commandFifo mode, so write to bottom page
	    tBase = 0;
	    tStride = 1;
	    aStride = SST_RAW_LFB_ADDR_STRIDE_1K>>SST_RAW_LFB_ADDR_STRIDE_SHIFT;
	  } else {
	    tBase = rRandom(0, (cpriv->memorySizeInBytes - 1) / SST_TILE_SIZE);			    
	    tStride = rRandom(1,SST_RAW_LFB_TILE_STRIDE>>SST_RAW_LFB_TILE_STRIDE_SHIFT);
	    aStride = rRandom(1,SST_RAW_LFB_ADDR_STRIDE_MAX>>SST_RAW_LFB_ADDR_STRIDE_SHIFT);
	  }

	  lfbMemoryConfig = (SST_RAW_LFB_TILE_BEGIN_PAGE_MUNGE(tBase)) |
	    (tStride<<SST_RAW_LFB_TILE_STRIDE_SHIFT) |
	    (aStride<<SST_RAW_LFB_ADDR_STRIDE_SHIFT) ;
	  SET(sstio->lfbMemoryConfig,lfbMemoryConfig);

	  tBase *= SST_TILE_SIZE;
	  aStride = 1<<(aStride+10);
	  
	  // find max x-value for tiled accesses
	  width = MIN(aStride,tStride * SST_TILE_WIDTH);
	  
	  // find max y-value for tiled accesses
	  height = MIN( ((cpriv->memorySizeInBytes - 1) - tBase) / aStride,
			((cpriv->memorySizeInBytes - 1) - tBase) / (tStride * SST_TILE_SIZE) * SST_TILE_HEIGHT );
	  if ( height > (SST_RAW_LFB_ADDR/aStride) )
	    height = (SST_RAW_LFB_ADDR/aStride);
	}

	// randomly select 8, 16, or 32-bit writes
	if ( diago.writeFifo )
	  nbytes = 4;                  // CMDFIFO packetizer doesn't handle 8-bit writes
	else
	  nbytes = 1 << iRandom(2);

	amask = ~(nbytes-1);
	dmask = SST_MASK(nbytes*8);

	// select linear or tiled access
	tiled = diago.ytiled;
	if ( tiled < 0 )
	  tiled = iRandom(1);
	
	// randomly select burst length
	if ( tiled )
	  maxBurst = (width*height)/nbytes;
	else
	  maxBurst = MIN(tBase,cpriv->memorySizeInBytes - 1) / nbytes;

	if ( maxBurst == 0 ) {   // try again
	  n--;
	  continue;
	}

	if ( iRandom(2) ) 
	  burstSize = rRandom( 1, maxBurst < MAX_BURST_SIZE ? maxBurst : MAX_BURST_SIZE );
	else
	  burstSize = 1;
	
	// generate a burst of address/data pairs
	if ( tiled ) {
	  do {
	    x = iRandom(width-1) & amask;
	    y = iRandom(height-1);
	    rows = (x+burstSize*nbytes) / width;
	  } while ( y+rows >= height  );
	  
	  for ( i=0; i<burstSize; i++ ) {
	    xmax = MAX(xmax,x);
	    ymax = MAX(ymax,y);
	    waddr[i] = memAddr(tBase,aStride,1,x,y,0);
	    raddr[i] = memAddr(tBase,tStride,1,x,y,1);
	    data[i] = iRandom(~0U) & dmask;
	    GDBG_INFO(20,"tiled: x,y=%d,%d(0x%x,0x%x), write addr=0x%x, phys addr=0x%x, data=0x%x\n",
		      x,y,x,y,waddr[i],raddr[i],data[i]);
	    x += nbytes;
	    if ( x >= width ) { x = 0; y++; }
	  }
	  
	} else {
	  FxU32 ceiling = tBase < (cpriv->memorySizeInBytes - 1) ? tBase : (cpriv->memorySizeInBytes - 1);
	  a = iRandom( ceiling - burstSize*nbytes - 1 ) & amask;
	  
	  for ( i=0; i<burstSize; i++ ) {
	    raddr[i] = waddr[i] = a;
	    data[i] = iRandom(~0U) & dmask;
	    GDBG_INFO(20,"linear: addr=0x%x, data=0x%x\n",waddr[i],data[i]);
	    a += nbytes;
	  }
	}
	
	// perform the burst of writes
	for ( i=0; i<burstSize; i++ ) {
	  switch (nbytes) {
	  case 1: {FxU8  *a = (FxU8  *) &lfb[waddr[i]]; SET8 (*a, (FxU8) data[i]);} break;
	  case 2: {FxU16 *a = (FxU16 *) &lfb[waddr[i]]; SET16(*a, (FxU16)data[i]);} break;
	  case 4: {FxU32 *a = (FxU32 *) &lfb[waddr[i]]; SET  (*a, (FxU32)data[i]);} break;
	  }
	}  
	
	// read back in random order, reading some addresses twice
	readSize = burstSize + rRandom(0,MIN(burstSize,MAX_READ_OVERLAP_SIZE));
	scrambleRandom(readSize,xref);
	for ( i=1; i<readSize; i++ ) {
	  if ( xref[i] >= burstSize )     // read some addresses twice in a row
	    if ( iRandom(4) == 2 )
	      xref[i] = xref[i-1];
	}
	GDBG_INFO(20,"burstSize=%d, readSize=%d\n",burstSize,readSize);
	for ( i=0; i<readSize; i++ ) {
	  xref[i] %= burstSize;
	  GDBG_INFO(20,"xref[%d] = %d\n",i,xref[i]);
	}

	// RAW LFBs are supposed to complete immediately, so start reading right away
	for ( i=0; i<readSize; i++ ) {
	  m = xref[i];
	      
	  switch (nbytes) {
	  case 1: d = GET8 (lfb[waddr[m]]); break;
	  case 2: d = GET16(lfb[waddr[m]]); break;
	  case 4: d = GET  (lfb[waddr[m]]); break;
	  }
	  if (diago.halInfo->hsim || diago.halInfo->hw) { // first check hardware/simulator
	    DIAG_COMPARE_MEM("hsim/hw",waddr[m],d,data[m]);
	    d =  diago.halInfo->csimLastRead;
	  }
	  DIAG_COMPARE_MEM("csim",waddr[m],d,data[m]);	  // then check software simulator
	}

	// also check memory contents via backdoor accesses
	for ( i=0; i<readSize; i++ ) {
	  m = xref[i];
	  DIAG_TEST_MEM(raddr[m],data[m],nbytes);
	}

      }
      if ( diago.ytiled )   // only relevant for tiled or mixed linear/tiled
	GDBG_INFO(1,"rawlfb:  max tiled coords (x,y) = %d,%d\n",xmax,ymax);
    }
    DIAG_PASS(0);
}



