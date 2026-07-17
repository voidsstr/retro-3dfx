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
** $Date: 10/11/00 8:19:15 PM$
*/

// ss version checked in on 6/10/96 works with theese args except screen 
//    check at end - JM
// reset -R 1 1 -Y 0x6420 -t-100


#include "udiag.h"
#include "sstdiag.h"
#include "stwtri.h"

// always return outside a triangle, this is used for testing a triangle
// that should have been aborted due to chip reset
int insideTriangleNOT(Triangle *t, int x, int y, unsigned long col)
{
    return 0;
}

void
main (int argc, char **argv)
{
    int i,n, tsize;
    static Triangle t1,t2;
    SstRegs *sst;

    sst = SST_BEGIN(argc,argv);
    t1.next = NULL;
    t2.next = NULL;
    tsize = diago.tsize;
    if (tsize < 0) tsize = -tsize;
 
  while (DIAG_STARTPASS()) {			// for each pass

    gdbg_info(0, "reset.c:  starting diag pass\n");

    // use the 16-bit 565 RGB texture format, size = 4x4, in replace mode
    t1.tex.tMode = (diago.perspective?SST_TPERSP_ST:0) | 
			SST_RGB565 | SST_TC_REPLACE | SST_TCA_REPLACE;
    texRandomTextureMap(sst,diago.trex,0, 4,4,&t1.tex);
    t2.tex = t1.tex;
    SET(sst->fbzColorPath, SST_RGBSEL_TREXOUT | SST_ENTEXTUREMAP |
		(diago.adjust?SST_PARMADJUST:0));

    for (n=0; n<20; n++) {			// do 20 tests
	if (diago.rgb == 16)
	    SET(sst->fbzMode, SST_RGBWRMASK | drawbufferRandom() |
		(texRandomZAbuffer(sst,t.tex.tMode) ? SST_ENALPHABUFFER : 0));
	else
	    SET(sst->fbzMode, SST_RGBWRMASK | drawbufferRandom());

    again1:
        t1.vA.x = rRandom(tsize*XY_ONE,(diago.xmaxscreen-tsize)*XY_ONE);
        t1.vA.y = rRandom(tsize*XY_ONE,(diago.ymaxscreen-tsize)*XY_ONE);
	randomTriangle1(&t1,diago.tsize,1);	// pick random triangle
	randomSt1418Triangle(&t1);		// with random texcoords
	if (diago.perspective)			// if testing perspective
	    randomW230Triangle(&t1);		// generate random W

	areaTriangle(&t1);			// compute the area (before setup)
	if (!setupStwTriangle(&t1))		// setup STW slopes
	    goto again1;			// reject bad slopes
	sortTriangle(&t1);			// sort it
	printStwTriangle(4,&t1);
	printStwTriangleSlopes(5,&t1);

	// NOTE: sub-pixel parameter adjustment is OFF so we may end up with
	//	 color overflows/underflows etc
	drawStwTriangle(sst,&t1);

    again2:
        t2.vA.x = rRandom(tsize*XY_ONE,(diago.xmaxscreen-tsize)*XY_ONE);
        t2.vA.y = rRandom(tsize*XY_ONE,(diago.ymaxscreen-tsize)*XY_ONE);
	randomTriangle1(&t2,diago.tsize,1);	// pick random triangle
	randomSt1418Triangle(&t2);		// with random texcoords
	if (diago.perspective)			// if testing perspective
	    randomW230Triangle(&t2);		// generate random W

	areaTriangle(&t2);			// compute the area (before setup)
	if (!setupStwTriangle(&t2))		// setup STW slopes
	    goto again2;			// reject bad slopes
	sortTriangle(&t2);			// sort it
	if (t1.vA.y <= t2.vC.y && t1.vC.y >= t2.vA.y) { // cheap intersect
	    int xl1, xr1, xl2, xr2;
	    xl1 = t1.vA.x;
	    if (t1.vB.x < xl1) xl1 = t1.vB.x;
	    if (t1.vC.x < xl1) xl1 = t1.vC.x;
	    xr1 = t1.vA.x;
	    if (t1.vB.x > xr1) xr1 = t1.vB.x;
	    if (t1.vC.x > xr1) xr1 = t1.vC.x;

	    xl2 = t2.vA.x;
	    if (t2.vB.x < xl2) xl2 = t2.vB.x;
	    if (t2.vC.x < xl2) xl2 = t2.vC.x;
	    xr2 = t2.vA.x;
	    if (t2.vB.x > xr2) xr2 = t2.vB.x;
	    if (t2.vC.x > xr2) xr2 = t2.vC.x;

	    if (xl1 <= xr2 && xr1 >= xr2) goto again2;
	}

	printStwTriangle(4,&t2);
	printStwTriangleSlopes(5,&t2);
	drawStwTriangle(sst,&t2);

#define TMU_RESET_LOOP_CNT 100
	// reset the TMU which should abort the 1st triangle and the 2nd
	// because it should be in the setup unit
	gdbg_info(0, "Bringing TMU into reset.    trexInit1=0x%08x\n", 
	    		(diago.trexInit1 | (BIT(19) | BIT(20))));
	for (i=0; i< TMU_RESET_LOOP_CNT; i++)
	    SET(sst->trexInit1, diago.trexInit1 | (BIT(19) | BIT(20)));
	gdbg_info(0, "Bringing TMU out of reset.  trexInit1=0x%08x\n", 
	    		(diago.trexInit1 & ~(BIT(19) | BIT(20))));
	for (i=0; i< TMU_RESET_LOOP_CNT; i++)
	    SET(sst->trexInit1, diago.trexInit1 & ~(BIT(19) | BIT(20)));


	// reset FBI to clear its fifos and triangle engine
#define FBI_GRX_RESET BIT(1)
#define FBI_PCIFIFO_RESET BIT(2)
#if 1
	gdbg_info(0, "reset.c:  resetting FBI\n");

	for (i=0; i< TMU_RESET_LOOP_CNT; i++)
	    SET(sst->fbiInit0, GET(sst->fbiInit0) | FBI_GRX_RESET |
		 	FBI_PCIFIFO_RESET);
	for (i=0; i< TMU_RESET_LOOP_CNT; i++)
	    SET(sst->fbiInit0, GET(sst->fbiInit0) & ~FBI_GRX_RESET);
	for (i=0; i< TMU_RESET_LOOP_CNT; i++)
	    SET(sst->fbiInit0, GET(sst->fbiInit0) & ~FBI_PCIFIFO_RESET);
#endif

//	gdbg_info(0, "Bringing TMU out of reset (2nd time).  trexInit1=0x%08x\n", 
//	    		(diago.trexInit1 & ~(BIT(19) | BIT(20))));
//	for (i=0; i< TMU_RESET_LOOP_CNT; i++)
//	    SET(sst->trexInit1, diago.trexInit1 & ~(BIT(19) | BIT(20)));


	gdbg_info(0, "reset.c:  before first sst_idle_really(sst) call\n");
	//sst_idle(sst);				// wait for the command to complete
	sst_idle_really(sst);

	checkTriangle(&t2,1,1,diago.adjust, insideTriangleNOT,1,0,0);

	// now draw it and make sure it works, first reset the TREX registers
	SET(sst->textureMode,SST_TC_PASS | SST_TCA_PASS);	// broadcast pass thru
	SET(SST_TREX(sst,diago.trex)->textureMode,t2.tex.tMode);
	SET(sst->tLOD,t2.tex.tLOD);
	SET(sst->texBaseAddr,(t2.tex.mip[4].addr - sstMipMapOffset[0][4]*2)>>3);
	drawStwTriangle(sst,&t2);
	checkTriangle(&t2,1,1,diago.adjust, insideStTriangle,0,0,0);
	eraseTriangle(sst,&t1,1,0,0);		// erase the triangle
	eraseTriangle(sst,&t2,1,0,0);		// erase the triangle

	gdbg_info(0, "reset.c:  before second sst_idle_really(sst) call\n");
	sst_idle_really(sst);
    }
  }

done:
	gdbg_info(0, "reset.c:  before done sst_idle_really(sst) call\n");
	//sst_idle(sst);				// wait for the command to complete
	sst_idle_really(sst);
  gdbg_info(0, "reset.c:  before DIAG_PASS(1) call\n");
  DIAG_PASS(1);					// check for black screen

}
