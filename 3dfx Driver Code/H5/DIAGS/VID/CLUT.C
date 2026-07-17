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
** Last Edited: Mon Nov 17 10:55:25 1997 by psmith (Phil Smith (x2456)) on vlsi2
**
** $Revision: 2$ 
** $Date: 10/11/00 8:19:33 PM$ 
**
*/

#include "udiag.h"
#include "sstdiag.h"
#include "hsimio.h"
#include "h3asm.h"

#define MAX_BURST_SIZE  	64
#define MAX_OVERLAP_SIZE        16
#define CLUT_SIZE		512

static FxU32 clut[CLUT_SIZE];
static FxU16 portBase;
static SstIORegs *sstio;

void clutWrite(FxU32 addr, FxU32 data, int method)
{
  FxU32 dummy;

  if ( method < 0 )
    method = iRandom(1);

  data &= 0xFFFFFF;
  clut[addr] = data;

  GDBG_INFO(30,"writing 0x%x => clut[%d] using %s\n",
	    data,addr,method?"memory mapped":"i/o port");
  
  if ( method == 0 ) {	// i/o
    SET_IO((FxU16)(portBase+DACADDR),addr);
    dummy = GET_IO((FxU16)(portBase+DACADDR));	// prevent bursts
    SET_IO((FxU16)(portBase+DACDATA),data);
  } else {		// memory mapped
    SET(sstio->dacAddr,addr);
    dummy = GET(sstio->dacAddr);		// prevent bursts
    SET(sstio->dacData,data);
  }
}

FxU32 clutRead(FxU32 addr, int method)
{
  FxU32 data, dummy, d;

  if ( method < 0 )
    method = iRandom(1);
  
  if ( method == 0 ) {  // i/o
    SET_IO((FxU16)(portBase+DACADDR),addr);
    dummy = GET_IO((FxU16)(portBase+DACADDR));	// prevent bursts
    data = GET_IO((FxU16)(portBase+DACDATA));
  } else {		// memory mapped
    SET(sstio->dacAddr,addr);
    dummy = GET(sstio->dacAddr);		// prevent bursts
    data = GET(sstio->dacData);
  }

  data &= 0xFFFFFF;

  GDBG_INFO(30,"reading clut[%d] => 0x%x using %s\n",
	    addr,data,method?"memory mapped":"i/o port");

  d = data;
  if (diago.halInfo->hsim || diago.halInfo->hw) { // first check hardware/simulator
    if ( clut[addr] != d ) 
      GDBG_ERROR("clut","read clut[%d(0x%x)] = 0x%x, expected 0x%x from hsim/hw\n",
		 addr,addr,d,clut[addr]);
    d = diago.halInfo->csimLastRead & 0xFFFFFF;
  }

  if ( clut[addr] != d )
    GDBG_ERROR("clut","read clut[%d(0x%x)] = 0x%x, expected 0x%x from csim\n",
	       addr,addr,d,clut[addr]);

  return data;
}

void main (int argc, char **argv)
{
    SstRegs *sst;
    int i, n;
    int burstSize, startAddr;
    int method, order, op;
    FxU32 xref[CLUT_SIZE+MAX_OVERLAP_SIZE];

    // Init the Hardware
    sst = SST_BEGIN( argc, argv );
    sstio = (SstIORegs *)(SST_IO_ADDRESS(sst));
    portBase = diago.halInfo->boardInfo[SST_FAKE_ADDRESS_GET_BOARD(sst)].physPort & 0xFFFF;

    if (diago.deviceID == SST_DEVICE_ID_SST96 || diago.deviceID == SST_DEVICE_ID_SST1) {
	GDBG_ERROR("clut", "Can't run this diag on SST96 or SST1\n");
	DIAG_PASS(0);
	exit(0);
    }    
    
    while (DIAG_STARTPASS()) {

      // initialize clut to random values
      for ( i=0; i<CLUT_SIZE; i++ ) 
	clutWrite(i,iRandom(~0),-1);

      for (n=0; n<15; n++) {
	
	// randomly select burst length
	if ( iRandom(2) ) 
	  burstSize = rRandom( 1, CLUT_SIZE );
	else
	  burstSize = 1;
	
	// generate in-order or out-of-order burst
	startAddr = iRandom(CLUT_SIZE-burstSize);
	order = iRandom(1);
	if ( order ) {
	  for ( i=0; i<burstSize; i++ )
	    xref[i] = i;
	} else {
	  int t = burstSize + rRandom(0,MIN(burstSize,MAX_OVERLAP_SIZE));
	  scrambleRandom(t,xref);
	  xref[0] %= burstSize;
	  for ( i=1; i<t; i++ ) {
	    if ( xref[i] >= burstSize )     // access some addresses twice in a row
	      if ( iRandom(4) == 2 )
		xref[i] = xref[i-1];
	      else
		xref[i] %= burstSize;
	  }
	  burstSize = t;
	}

	// randomly read or write
	op = iRandom(1);
	method = iRandom(2) - 1;
	 GDBG_INFO(30,"%s %s burst of %d using %s accesses, starting at %d(0x%x)\n",
		   order?"in-order":"out-of-order",
		   op?"write":"read",
		   burstSize,
		   method==0?"i/o port":(method==1?"memory mapped":"mixed"),
		   startAddr,startAddr);

	if ( op ) {
	  for ( i=0; i<burstSize; i++ ) 
	    clutWrite(startAddr+xref[i],iRandom(~0),method);
	} else {
	  for ( i=0; i<burstSize; i++ ) 
	    clutRead(startAddr+xref[i],method);
	}
	  
      }

    }
    DIAG_PASS(0);
}



