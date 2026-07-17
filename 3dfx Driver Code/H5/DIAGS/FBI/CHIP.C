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
** $Revision: 2$
** $Date: 10/11/00 8:10:05 PM$
*/

#include "udiag.h"
#include "sstdiag.h"

#ifdef CVG
#define MAX_CHIP	0xF
#elifdef H4
#define MAX_CHIP	0x7
#else
#define MAX_CHIP	0x3
#endif

#define CHIP_SHIFT	10
#define CHIP_FIELD	(0xF << CHIP_SHIFT)

SstRegs *sst;

void chip32test(volatile unsigned long *reg, unsigned long mask, int maskable)
{
    char addr[32];
    int n,chip,newreg;
    unsigned long val,got,shouldbe;

    sst_idle_really(sst);
    shouldbe = GET(*reg) & mask;	// get the current value

    for (n=5+iRandom(3); n>0; n--) {
	val = iRandom(mask) & mask;		// random value
	chip = iRandom(MAX_CHIP);	// random chip field
	gdbg_info(3,"testing chip field %d 0x%x\n",chip,chip);
	newreg = ((int)reg & ~CHIP_FIELD) | (chip << CHIP_SHIFT);
	SET(*(unsigned long *)newreg,val);

	sprintf(addr,"0x%x",reg);
	sst_idle_really(sst);
	got = GET(*reg) & mask;
	if ((chip == 0) || (chip & 1) || !maskable)
	    shouldbe = val&mask;
	DIAG_TESTREG32(addr,shouldbe,got);
	
    }
}

// GMT: define AUDIT if you want to keep a record of the writes

//#define AUDIT 1

#ifdef AUDIT

#define MAX_AUDITS 256

struct trail {
	unsigned long *addr;
	unsigned long data;
} auditTrail[MAX_AUDITS];

static int audit;

void record(unsigned long *a, unsigned long b)
{
	auditTrail[audit].addr = a;
	auditTrail[audit].data = b;
	audit++;
	audit &= (MAX_AUDITS-1);
}

#define RECORD(a,b) \
	auditTrail[audit].addr = &(a); \
	auditTrail[audit].data = b; \
	audit++; \
	audit &= (MAX_AUDITS-1);

void printAudit(void)
{
    int i;

    for (i=0; i<MAX_AUDITS; i++)
	printf("SET 0x%x 0x%x\n",auditTrail[i].addr,auditTrail[i].data);
    fflush(stdout);
}

#define TSET(a,b) {RECORD(*(chip+&a),b); SET(*(chip+&a),b);}

#else

#define TSET(a,b) SET(*(chip+&a),b)

#endif

// GMT: writing to texture space fixes the bug
//#define BUGFIX SET(*(long *)SST_TEX_ADDRESS(sst),0)
#define BUGFIX

// GSET randomly changes chips from 0 to 1
#define GSET(a,b) if (temp=(rands[k]&0x100)) BUGFIX;\
		chip^=temp; TSET(a,b)

void compare(volatile unsigned long *reg, unsigned long mask)
{
    char addr[32];
    unsigned long got,shouldbe;

    sprintf(addr,"0x%x",reg);
    got = GET(*reg) & mask;
    shouldbe =  diago.halInfo->csimLastRead & mask;
#ifdef AUDIT
    if (got != shouldbe) printAudit();
#endif
    DIAG_TESTREG32(addr,shouldbe,got);
}

// try some harsh things - write a bunch of registers and then read them back
// we send them to different chip fields (just 0 or 1)
void chipRandom(SstRegs *sst)
{
    int i,j,k=0,temp;
    static int chip;
    static unsigned long rands[300];		// array of randoms

    if (rands[0] == 0)
    for (i=0; i<300; i++)			// init the array of randoms
	rands[i] = iRandom(0xFFFFFFFF);

    for (i=0; i<50; i++) {
	j = iRandom(13);
	if (k > 255) k = 0;
	switch(j) {
	    case 0:
		TSET(sst->r,j);
		TSET(sst->g,j);
		TSET(sst->b,j);
	    case 1:
		GSET(sst->fbzColorPath,rands[k]);
		k++;
	    case 2:
		GSET(sst->fogMode,rands[k]);
		k++;
	    case 3:
		GSET(sst->alphaMode,rands[k]);
		k++;
	    case 4:
	      if(diago.sliEnabled)
		{
		  GSET(sst->fbzMode,rands[k] & (~SST_YORIGIN));
		}
	      else
		{
		  GSET(sst->fbzMode,rands[k]);
		}

		k++;
	    case 5:
	      if(diago.sliEnabled)
		{
		  GSET(sst->lfbMode,rands[k] & (~SST_LFB_YORIGIN));
		}
	      else
		{
		  GSET(sst->lfbMode,rands[k]);
		}

		k++;
	    case 6:
		GSET(sst->clipLeftRight,rands[k]);
		k++;
	    case 7:
		GSET(sst->clipBottomTop,rands[k]);
		k++;
	    case 8:
		GSET(sst->fogColor,rands[k]);
		k++;

	    case 9:
		TSET(sst->vC.x,k);
		TSET(sst->vC.y,j);
	    case 10:
		GSET(sst->stipple,rands[k]);
		k++;
	    case 11:
		GSET(sst->c0,rands[k]);
		k++;
	    case 12:
		GSET(sst->c1,rands[k]);
		k++;
		break;
	    case 13:
		TSET(sst->vA.x,i);
		TSET(sst->vA.y,j);
		TSET(sst->vB.x,k);
		TSET(sst->vB.y,j);
	}
    }
    sst_idle_really(sst);

    // now read all the readable registers and verify against CSIM
    compare(&sst->fbzColorPath,0x3FFFFFFF);
    compare(&sst->fogMode,0x000000FF);
    compare(&sst->alphaMode,0xFFFFFFFF);
    compare(&sst->fbzMode,0x003F3FFF & (diago.sliEnabled ? (~SST_YORIGIN) : (~0)));
    compare(&sst->lfbMode,0x0001FFCF & (diago.sliEnabled ? (~SST_LFB_YORIGIN) : (~0)));

    compare(&sst->clipLeftRight,0x0FFF0FFF);
    compare(&sst->clipBottomTop,0x0FFF0FFF);
    compare(&sst->clipLeftRight1,0x0FFF0FFF);
    compare(&sst->clipBottomTop1,0x0FFF0FFF);
    compare(&sst->stipple,0xFFFFFFFF);
    compare(&sst->c0,0xFFFFFFFF);
    compare(&sst->c1,0xFFFFFFFF);
}

void chipRandomChroma(SstRegs *sst)
{
    int n, sub01;
    long x,y;
    FxU32 t0c0,t0c1,t1c0,t1c1;		// constant color regs
    SstRegs *fbi, *tmu0, *tmu1;

    // get pointers to these individual chips
    fbi  = SST_CHIP(sst,1);
    tmu0 = SST_TREX(sst,0);
    tmu1 = SST_TREX(sst,1);

    // disable chroma junk, enable constant color registers
    // also setup TCUs to add 3 of the 4 constant color registers together
    // as follows t1c0+t1c1- (t0c0 or t0c1)

    // (other+local) * 1 + 0
    SET(tmu1->textureMode,SST_TC_MONE|SST_TCA_MONE |
			  SST_TC_SUB_CLOCAL|SST_TCA_SUB_CLOCAL);
    // other=chromaRange, local=chromaKey
    SET(tmu1->combineMode,SST_CM_DISABLE_CHROMA_SUBSTITUTION | SST_CM_USE_COMBINE_MODE |
			SST_CM_TC_OTHERSELECT_CR_RGB | SST_CM_TC_LOCALSELECT_CK_RGB |
			SST_CM_TC_INVERT_OTHER_X | SST_CM_TC_INVERT_LOCAL_X |
			SST_CM_TCA_OTHERSELECT_CR_A  | SST_CM_TCA_LOCALSELECT_CK_A |
			SST_CM_TCA_INVERT_OTHER_X | SST_CM_TCA_INVERT_LOCAL_X );

    // (other-local) * 1 + 0
    SET(tmu0->textureMode,SST_TC_MONE|SST_TCA_MONE |
			  SST_TC_SUB_CLOCAL|SST_TCA_SUB_CLOCAL);

    // and finally set up FBI to pass texture RGB,A right thru w/o modification
    SET(sst->fbzColorPath, SST_RGBSEL_TREXOUT | SST_ASEL_TREXOUT | SST_ENTEXTUREMAP);
    SET(sst->fbzMode, SST_RGBWRMASK | drawbufferRandom());

    // now randomly set the constant color registers and render a pixel
    for (n=0; n<100; n++) {			// do 100 tests
	FxI32 r,g,b,a, good;

	t0c0 = colRandom32();
	t0c1 = colRandom32();
	t1c0 = colRandom32();
	t1c1 = colRandom32();

	sub01 = iRandom(1);
	// compute (t1c0+t1c1) - t0c1
	if (sub01) {
	    // other=previous(TMU1), local= -chromaKey
	    SET(tmu0->combineMode,SST_CM_DISABLE_CHROMA_SUBSTITUTION | SST_CM_USE_COMBINE_MODE |
			SST_CM_TC_OTHERSELECT_OTHER_TRGB | SST_CM_TCA_OTHERSELECT_OTHER_TA |
			SST_CM_TC_INVERT_OTHER_X | SST_CM_TCA_INVERT_OTHER_X |
			SST_CM_TC_LOCALSELECT_CK_RGB | SST_CM_TCA_LOCALSELECT_CK_A|
			SST_CM_TC_INVERT_LOCAL_ZERO_MINUS_X | SST_CM_TCA_INVERT_LOCAL_ZERO_MINUS_X);
	}
	// compute (t1c0+t1c1) - t0c0
	else {
	    // other= -chromaRange, local=previous(TMU1)
	    SET(tmu0->combineMode,SST_CM_DISABLE_CHROMA_SUBSTITUTION | SST_CM_USE_COMBINE_MODE |
			SST_CM_TC_OTHERSELECT_CR_RGB | SST_CM_TCA_OTHERSELECT_CR_A |
			SST_CM_TC_INVERT_OTHER_ZERO_MINUS_X | SST_CM_TCA_INVERT_OTHER_ZERO_MINUS_X |
			SST_CM_TC_LOCALSELECT_OTHER_TRGB | SST_CM_TCA_LOCALSELECT_OTHER_TA |
			SST_CM_TC_INVERT_LOCAL_X | SST_CM_TCA_INVERT_LOCAL_X);
	}

	r = iRandom(2);
	gdbg_info(3,"case %d\n",r);
	switch (r) {
	    case 0:	// simple writes
		SET(tmu0->chromaKey,t0c1);
		SET(tmu0->chromaRange,t0c0);
		SET(tmu1->chromaRange,t1c0);
		SET(tmu1->chromaKey,t1c1);
		break;
	    case 1:	// broadcast writes to all chips, then override tmu1
		SET(sst->chromaRange,t0c0);
		SET(sst->chromaKey,t0c1);
		SET(tmu1->chromaKey,t1c1);
		SET(tmu1->chromaRange,t1c0);
		break;
	    case 2:	// broadcast writes to all chips, then override tmu0
		SET(sst->chromaKey,t1c1);
		SET(sst->chromaRange,t1c0);
		SET(tmu0->chromaKey,t0c1);
		SET(tmu0->chromaRange,t0c0);
		break;
	}
	// randomly trash FBI(only) registers with zeroes
	if (iRandom(1)) {
	    gdbg_info(3,"trash FBI only\n");
	    SET(fbi->chromaKey,0);
	    SET(fbi->chromaRange,0);
	    SET(fbi->combineMode,0);
	}
	if (sub01)
	    gdbg_info(3,"t1c0+t1c1-t0c1 = %08x + %08x - %08x\n",t1c0,t1c1,t0c1);
	else
	    gdbg_info(3,"t1c0+t1c1-t0c0 = %08x + %08x - %08x\n",t1c0,t1c1,t0c0);

	// render the pixel
	xyRandom(&x,&y);			// pick random x,y
	x <<= SST_XY_FRACBITS;
	y <<= SST_XY_FRACBITS;
	SET(sst->vA.x,x);
	SET(sst->vA.y,y);
	SET(sst->vB.x,x+XY_ONE);
	SET(sst->vB.y,y);
	SET(sst->vC.x,x+XY_ONE);
	SET(sst->vC.y,y+XY_ONE);
	SET(sst->triangleCMD,0);
	x >>= SST_XY_FRACBITS;
	y >>= SST_XY_FRACBITS;

	sst_idle(sst);				// wait for the command to complete
						// now predict the result
	a = (t1c0 >> 24) & 0xFF;		// get t1c0
	r = (t1c0 >> 16) & 0xFF;
	g = (t1c0 >>  8) & 0xFF;
	b = (t1c0 >>  0) & 0xFF;

	a += (t1c1 >> 24) & 0xFF;		// add t1c1
	r += (t1c1 >> 16) & 0xFF;
	g += (t1c1 >>  8) & 0xFF;
	b += (t1c1 >>  0) & 0xFF;

	if (a > 0xFF) a = 0xFF;			// clamp to 8 bits
	if (r > 0xFF) r = 0xFF;
	if (g > 0xFF) g = 0xFF;
	if (b > 0xFF) b = 0xFF;

	if (sub01) {
	    a -= (t0c1 >> 24) & 0xFF;		// subtract t0c1
	    r -= (t0c1 >> 16) & 0xFF;
	    g -= (t0c1 >>  8) & 0xFF;
	    b -= (t0c1 >>  0) & 0xFF;
	}
	else {
	    a -= (t0c0 >> 24) & 0xFF;		// subtract t0c0
	    r -= (t0c0 >> 16) & 0xFF;
	    g -= (t0c0 >>  8) & 0xFF;
	    b -= (t0c0 >>  0) & 0xFF;
	}

	if (a > 0xFF) a = 0xFF;			// clamp to 8 bits
	if (r > 0xFF) r = 0xFF;
	if (g > 0xFF) g = 0xFF;
	if (b > 0xFF) b = 0xFF;
	if (a < 0x00) a = 0x00;			// clamp to 8 bits
	if (r < 0x00) r = 0x00;
	if (g < 0x00) g = 0x00;
	if (b < 0x00) b = 0x00;

	good = (a<<24) | (r<<16) | (g<<8) | b;		// build final color
	good = sst_argb_form_result(good,0,0,0);	// convert to native form
	DIAG_TEST_PIXEL(diago.curdrawbuffer,x,y,good);	// and test
    }
}

// options
// -T : tests TMU chromarange chip fields (run in -5 or -7 modes to test alpha)
// -Z : tests random chip fields
void
main (int argc, char **argv)
{
    int n;

    sst = SST_BEGIN(argc,argv);

    while (DIAG_STARTPASS())			// for each pass
    if (diago.zeroLodFrac) {			// bang on chip fields randomly
      for (n=0; n<5; n++) {			// do 5 tests
	gdbg_info(2,"test random chip fields\n");
	chipRandom(sst);
      }
    }
    else if (diago.multiTexBaseAddr) {
	gdbg_info(2,"test random TMU.chromaKey,Range chip fields\n");
	chipRandomChroma(sst);
    }
    else {
      for (n=0; n<5; n++) {			// do 5 tests
	gdbg_info(2,"test fbzColorPath\n");
	chip32test(&sst->fbzColorPath,0x3FFFFFFF,0);
	gdbg_info(2,"test fogMode\n");
	chip32test(&sst->fogMode,0x000000FF,1);
	gdbg_info(2,"test alphaMode\n");
	chip32test(&sst->alphaMode,0xFFFFFFFF,1);
	gdbg_info(2,"test fbzMode\n");
	chip32test(&sst->fbzMode,0x003F3FFF & (diago.sliEnabled ? (~SST_YORIGIN) : (~0)),1);
	gdbg_info(2,"test lfbMode\n");
	chip32test(&sst->lfbMode,0x0001FFCF & (diago.sliEnabled ? (~SST_LFB_YORIGIN) : (~0)),1);

	gdbg_info(2,"test clipLeftRight\n");
	chip32test(&sst->clipLeftRight,0x0FFF0FFF,1);
	gdbg_info(2,"test clipBottomTop\n");
	chip32test(&sst->clipBottomTop,0x0FFF0FFF,1);
	gdbg_info(2,"test clipLeftRight1\n");
	chip32test(&sst->clipLeftRight1,0x0FFF0FFF,1);
	gdbg_info(2,"test clipBottomTop1\n");
	chip32test(&sst->clipBottomTop1,0x0FFF0FFF,1);

	gdbg_info(2,"test stipple\n");
	chip32test(&sst->stipple,0xFFFFFFFF,1);
	gdbg_info(2,"test c0\n");
	chip32test(&sst->c0,0xFFFFFFFF,1);
	gdbg_info(2,"test c1\n");
	chip32test(&sst->c1,0xFFFFFFFF,1);

	// NOTE: we cannot write pixel counter registers so we cannot
	//	 test them here
      }
    }
    DIAG_PASS(0);
}
