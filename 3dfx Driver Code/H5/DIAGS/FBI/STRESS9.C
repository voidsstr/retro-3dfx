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
** $Revision: 4$
** $Date: 10/11/00 8:10:40 PM$
*/

#include "udiag.h"
#include "sstdiag.h"

#ifdef CVG
#include "cvgasm.h"
#else
#include "h3asm.h"
#endif

// these keep track of whether or not we have sent these values to the chip
// for the current triangle
int sentVAx, sentVAy;
int sentVBx, sentVBy;
int sentVCx, sentVCy;

void newTriangle(Triangle *t, int onscreen, int i)
{
    randomStressTriangle(t, diago.tsize, onscreen, i);
    sentVAx = 0;
    sentVAy = 0;
    sentVBx = 0;
    sentVBy = 0;
    sentVCx = 0;
    sentVCy = 0;
}

void flushTriangle(SstRegs *sst, Triangle *t)
{
    if (!sentVAx) if (iRandom(1)) SET(sst->vA.x,t->vA.x); else SETF(sst->FvA.x,t->vA.fx);
    if (!sentVAy) if (iRandom(1)) SET(sst->vA.y,t->vA.y); else SETF(sst->FvA.y,t->vA.fy);
    if (!sentVBx) if (iRandom(1)) SET(sst->vB.x,t->vB.x); else SETF(sst->FvB.x,t->vB.fx);
    if (!sentVBy) if (iRandom(1)) SET(sst->vB.y,t->vB.y); else SETF(sst->FvB.y,t->vB.fy);
    if (!sentVCx) if (iRandom(1)) SET(sst->vC.x,t->vC.x); else SETF(sst->FvC.x,t->vC.fx);
    if (!sentVCy) if (iRandom(1)) SET(sst->vC.y,t->vC.y); else SETF(sst->FvC.y,t->vC.fy);
}

#define SETW(a,w) SET(a,(unsigned long)(w>>(SST_W64_FRACBITS-SST_W_FRACBITS)))

void
main (int argc, char **argv)
{
    int j,k,n;
    FxU32 fbzMode, fogTable[FOG_TABLE_SIZE];
    static Triangle t;
    SstRegs *sst;

    sst = SST_BEGIN(argc,argv);
    if (!diago.diff) {
	gdbg_error("stress9","must run with -D option, forcing -D\n");
	diago.diff = 1;
    }
    if (diago.zeroLodFrac && !diago.hasAuxBuffer) {
	gdbg_error("stress9","must run without -Z option when no zbuffer present, turning off -Z\n");
	diago.zeroLodFrac = 0;
    }

    //Make sure that the subsample jitter is 0,0. Otherwise, there can be goofy
    //mismatches between the csim and rtl/hw. This occurs because this diag only
    //sends down partial triangles
    if(diago.aaEnabled)
      SET(sst->aaCtrl, SST_AA_CONTROL_AA_ENABLE);


#if !defined(CVG) && !defined(H4)
    if (diago.zeroLodFrac)  {
      SstIORegs *sstio = (SstIORegs *) SST_IO_ADDRESS(sst);
      int sdram = GET(sstio->dramInit1) & SST_MCTL_TYPE_SDRAM;
      if ( sdram ) {
	GDBG_INFO(0,"stress9: SDRAM detected, disabling depth buffering\n");
	diago.zeroLodFrac = 0;
      }
    }
#endif

    // setup some reasonable starting modes
    fbzMode = SST_RGBWRMASK;
    if (diago.zeroLodFrac)			// enable Zbuffer
	fbzMode |= SST_ENDEPTHBUFFER | SST_ZAWRMASK | SST_ZFUNC_GT;
    SET(sst->fbzMode, fbzMode);
    SET(sst->fbzColorPath, SST_RGBSEL_RGBA);

    // send random (but valid) register writes to the chip
    while (DIAG_STARTPASS())  {			// for each pass
      randomFloatTriangle(&t,diago.tsize,1);
      newTriangle(&t,1,1);
      sst_random_fog_table(sst,fogTable);

	if (diago.checkEveryTriangle || !diago.diff)
	    DIAG_DIFFSCREEN(diago.xmaxscreen-1,diago.ymaxscreen-1);
      for (j=0; j<5; j++)			// do 5 rounds
      for (n=0; n<200; n++) {			// 200 random writes
	switch(rRandom(0,C1)&~3) {		// random register address
	  case VA_X:
		sentVAx = 1;
		SET(sst->vA.x,t.vA.x);
	  	break;
	  case VA_Y:
		sentVAy = 1;
		SET(sst->vA.y,t.vA.y);
	  	break;
	  case VB_X:
		sentVBx = 1;
		SET(sst->vB.x,t.vB.x);
	  	break;
	  case VB_Y:
		sentVBy = 1;
		SET(sst->vB.y,t.vB.y);
	  	break;
	  case VC_X:
		sentVCx = 1;
		SET(sst->vC.x,t.vC.x);
	  	break;
	  case VC_Y:
		sentVCy = 1;
		SET(sst->vC.y,t.vC.y);
	  	break;
	  case R:
		SET(sst->r,t.vA.r);
	  	break;
	  case G:
		SET(sst->g,t.vA.g);
	  	break;
	  case B:
		SET(sst->b,t.vA.b);
	  	break;
	  case A:
		SET(sst->a,t.vA.a);
	  	break;
	  case Z:
		SET(sst->z,(FxU32)(t.vA.z64>>(SST_Z64_SIZE-SST_Z_16BPP_SIZE)));
	  	break;
	  case W:
		SETW(sst->w,t.vA.w);
	  	break;
	  case DRDX:
		SET(sst->drdx,t.drdx);
	  	break;
	  case DGDX:
		SET(sst->dgdx,t.dgdx);
	  	break;
	  case DBDX:
		SET(sst->dbdx,t.dbdx);
	  	break;
	  case DADX:
		SET(sst->dadx,t.dadx);
	  	break;
	  case DZDX:
		SET(sst->dzdx,(FxU32)(t.dzdx64>>(SST_Z64_SIZE-SST_Z_16BPP_SIZE)));
	  	break;
	  case DWDX:
		SETW(sst->dwdx,t.dwdx);
	  	break;
	  case DRDY:
		SET(sst->drdy,t.drdy);
	  	break;
	  case DGDY:
		SET(sst->dgdy,t.dgdy);
	  	break;
	  case DBDY:
		SET(sst->dbdy,t.dbdy);
	  	break;
	  case DADY:
		SET(sst->dady,t.dady);
	  	break;
	  case DZDY:
		SET(sst->dzdy,(FxU32)(t.dzdy64>>(SST_Z64_SIZE-SST_Z_16BPP_SIZE)));
	  	break;
	  case DWDY:
		SETW(sst->dwdy,t.dwdy);
	  	break;

	  case INTRCTRL:	// increase probability of triangle command
	  case TRIANGLECMD:
		flushTriangle(sst,&t);

		//setPixelsPerClock toggles between 1 and 2 pixels per clock rendering
		//if appropriate (i.e. --pixelsPerClock <= 0)
		setPixelsPerClock(sst);		

		SET(sst->triangleCMD,t.area);
		newTriangle(&t,1,-1);
		break;
	  case FVA_X:
		sentVAx = 1;
		SETF(sst->FvA.x,t.vA.fx);
		break;
	  case FVA_Y:
		sentVAy = 1;
		SETF(sst->FvA.y,t.vA.fy);
	  	break;
	  case FVB_X:
		sentVBx = 1;
		SETF(sst->FvB.x,t.vB.fx);
	  	break;
	  case FVB_Y:
		sentVBy = 1;
		SETF(sst->FvB.y,t.vB.fy);
	  	break;
	  case FVC_X:
		sentVCx = 1;
		SETF(sst->FvC.x,t.vC.fx);
	  	break;
	  case FVC_Y:
		sentVCy = 1;
		SETF(sst->FvC.y,t.vC.fy);
	  	break;
	  case FR:
		SETF(sst->Fr,t.vA.fr);
	  	break;
	  case FG:
		SETF(sst->Fg,t.vA.fg);
	  	break;
	  case FB:
		SETF(sst->Fb,t.vA.fb);
	  	break;
	  case FA:
		SETF(sst->Fa,t.vA.fa);
	  	break;
	  case FZ:
		SETF(sst->Fz,t.vA.fz);
	  	break;
	  case FW:
		SETF(sst->Fw,t.vA.fw);
	  	break;
	  case FDRDX:
		SETF(sst->Fdrdx,t.fdrdx);
	  	break;
	  case FDGDX:
		SETF(sst->Fdgdx,t.fdgdx);
	  	break;
	  case FDBDX:
		SETF(sst->Fdbdx,t.fdbdx);
	  	break;
	  case FDADX:
		SETF(sst->Fdadx,t.fdadx);
	  	break;
	  case FDZDX:
		SETF(sst->Fdzdx,t.fdzdx);
	  	break;
	  case FDWDX:
		SETF(sst->Fdwdx,t.fdwdx);
	  	break;
	  case FDRDY:
		SETF(sst->Fdrdy,t.fdrdy);
	  	break;
	  case FDGDY:
		SETF(sst->Fdgdy,t.fdgdy);
	  	break;
	  case FDBDY:
		SETF(sst->Fdbdy,t.fdbdy);
	  	break;
	  case FDADY:
		SETF(sst->Fdady,t.fdady);
	  	break;
	  case FDZDY:
		SETF(sst->Fdzdy,t.fdzdy);
	  	break;
	  case FDWDY:
		SETF(sst->Fdwdy,t.fdwdy);
	  	break;

	  case STATUS:		// increase probability of triangle command
	  case FTRIANGLECMD:
		flushTriangle(sst,&t);

		//setPixelsPerClock toggles between 1 and 2 pixels per clock rendering
		//if appropriate (i.e. --pixelsPerClock <= 0)
		setPixelsPerClock(sst);

		SETF(sst->FtriangleCMD,(float)t.area);
		newTriangle(&t,1,-1);
		break;
	  case FBZCOLORPATH:
	  	{
		    unsigned long ccu;
		    while (!goodCcuPath(ccu=iRandom(0xFFFFFFFF),FXTRUE));

		    //Don't allow sub-pixel correction with AA
		    if(diago.aaEnabled)
		      ccu &= ~(SST_PARMADJUST);

		    SET(sst->fbzColorPath,ccu);
		}
		break;
	  case FOGMODE:
		SET(sst->fogMode,iRandom(0x00003FFF));
		break;
	  case ALPHAMODE:
	  	{
		    unsigned long amode;
		    while (!goodAlphaMode(amode=iRandom(0xFFFFFFFF),fbzMode));
		}
		break;
	  case FBZMODE:
		SET(sst->fbzMode, fbzMode | drawbufferRandom());
		break;
	  case LFBMODE:
		{
		    unsigned long lmode;
		    do {
			lmode = iRandom(0xFFFFFFFF);
		    } while (!goodLfbMode(lmode));
		    SET(sst->lfbMode,lmode);
		}
		break;
	  case CLIPLEFTRIGHT:
		{
		    int temp;
		    temp = iRandom(diago.xmaxscreen);
		    temp = (iRandom(temp)<<16) | temp;
		    SET(sst->clipLeftRight,temp);
		}
		break;
	  case CLIPBOTTOMTOP:
		{
		    int temp;
		    temp = iRandom(diago.ymaxscreen);
		    temp = (iRandom(temp)<<16) | temp;
		    SET(sst->clipBottomTop,temp);
		}
		break;

	  case NOPCMD:
		SET(sst->nopCMD,iRandom(0xFFFFFFFF));
		break;
	  case FASTFILLCMD:
		{
		    int x,y,w,h;
		again:
		    xyRandom(&x,&y);			// pick random x,y onscreen
		    do {				// and random width and height
			w = iRandom(17);
			h = iRandom(15);
		    } while (w * h > 500);
		    if (!ONSCREEN(x+w-1,y+h-1))
			goto again;
		    SET(sst->clipLeftRight, (x<<16) | (x+w));
		    SET(sst->clipBottomTop, (y<<16) | (y+h));
		    SET(sst->fastfillCMD,iRandom(0xFFFFFFFD));
		}
		break;
	  case SWAPBUFFERCMD:
		if (!diago.videoTest)
		    DIAG_SWAPBUFFER();
		break;
	  case FOGCOLOR:
		SET(sst->fogColor,iRandom(0xFFFFFFFF));
		break;
	  case ZACOLOR:
		SET(sst->zaColor,iRandom(0xFFFFFFFF));
		break;
	  case CHROMAKEY:
		SET(sst->chromaKey,iRandom(0xFFFFFFFF));
		break;

	  case STIPPLE:
		SET(sst->stipple,iRandom(0xFFFFFFFF));
		break;
	  case C0:
		SET(sst->c0,iRandom(0xFFFFFFFF));
		break;
	  case C1:
		SET(sst->c1,iRandom(0xFFFFFFFF));
		break;

	  default:
		// random FOG table writes
		switch (iRandom(2)) {
		  case 0:	// FOG TABLE write
			for (k=rRandom(1,3); k>0; k--) {
			    FxU32 foge = sst_random_fog_table_entry();
			    foge |= sst_random_fog_table_entry()<<16;
			    SET(sst->fogTable[iRandom(31)],foge);
			}
		  default:
			n--;
			break;
		}
	  	break;
	}
      }
    }
    DIAG_PASS(0);
}
