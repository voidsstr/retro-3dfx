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
** $Revision: 3$
** $Date: 10/11/00 8:18:48 PM$
*/

#include "allocate.h"
#include "udiag.h"
#include "sstdiag.h"
#include <fximg.h>

// need some old defines from previous chips
#define SST_DRAWBUFFER_SHIFT    14
#define SST_DRAWBUFFER          (0x3<<SST_DRAWBUFFER_SHIFT)
#define SST_DRAWBUFFER_FRONT            (0<<SST_DRAWBUFFER_SHIFT)
#define SST_DRAWBUFFER_BACK             (1<<SST_DRAWBUFFER_SHIFT)
#define SST_SEQ_8_DOWNLD BIT(31)
#define SST1_FBIINIT3           0x0000021c

extern int diagSwaps;		// a hack, we need to increment this

// compares just the register addresses, not the chip or wrap fields
// also have to worry about ALT regs
#define SAME_ADDR(addr,reg) (	\
	((addr&~0x3FFC00) == ((~0x3FFC00)&(FxU32)&reg)) || \
	((addr&~0x3FFC00) == ((~0x3FFC00)&(SST_3D_ALT_OFFSET-SST_3D_OFFSET+(FxU32)&reg))) )

// what's a few globals amongst friends? 
// Answer: A bad idea
FxU32 curFbzMode;		// last seen fbzMode value
FxU32 curTexMode;		// last seen textureMode value
FxU32 curTexLOD;		// last seen tLOD value
FxU32 curTexLoadAddr;		// current relocation address
FxU32 lastTexBaseAddr = 0xFFFFFFFF;
FxU32 curTexBaseAddr = 0xFFFFFFFF;
FxU32 numTextures, oldBases[8096], newBases[8096], lodMins[8096];
FxBool noOriginSwap;


// lookup a texure, return index if found, -1 if not found
int lookupTex(FxU32 base)
{
    int i;

    for (i=0; i<(signed)numTextures; i++) {
	if (oldBases[i] == base)
	    return i;
    }
    return -1;
}

// here's some sample input to look at
// gd.120:        SET(0x10000110,      18321(0x00004791)) 0        FBZMODE
// gd.120:        GET(0x10000000,          0(0x00000000)) 0         STATUS

// translate an old SST-1 address to a H3 address
// returns 1 if it has done the write
int needs_idle;

int xlate(SstRegs *sst, FxU32 *_addr, FxU32 *data)
{
    FxU32 addr = *_addr;
    int ar, bpt, i, w;
    static FxU32 textureCounter=0;  ///Counts number of textures downloaded
    
    if (addr & 0x00800000) {		// if a write to SST-1 texture space
	FxU32 tmu, lod, s, t;
	static FxU32 lodlast;

	bpt = SST_T8BIT(curTexMode) ? 1 : 2;
	ar = (curTexLOD & SST_LOD_ASPECT)>>SST_LOD_ASPECT_SHIFT;

	tmu = (addr >> 21) & 0x3;	// grab TREX number from bits 22:21
	if (tmu > 1) {
	    GDBG_ERROR("replay","write to TMU %d encountered\n",tmu);
	    DIAG_FAIL();
	}
	lod = (addr >> 17) & 0xF;	// grab the LOD from bits 20:17
	i = lookupTex(lastTexBaseAddr);
	if (i < 0) {
	    GDBG_ERROR("xlate", "texture lookup failed\n");
	    DIAG_FAIL();
	}

	if (lod < lodMins[i]) {
	    gdbg_printf("WARNING: non-monotonic LODs downloaded, redefining texture\n");
	    newBases[i] = 0xFFFFFFFF;
	}
	if (newBases[i] == 0xFFFFFFFF) {	// if switched to a new texture
	  char textureName[32];

	  sprintf(textureName, "texture %d", textureCounter++);
	  curTexLoadAddr=allocate((sstLinearMipMapSize(lod, ar, bpt * 8, FXFALSE, FXFALSE, FXFALSE) * 4 / 3)+8, 
				  textureName, normalPlacement);

	  //Every once in a while, print out info on where
	  //the texture maps are located
	  if(textureCounter==10 || ((textureCounter % 50) == 0))
	    memoryMap();
	  
	  lodMins[i] = lod;			// assume largest size LOD loaded first
	  lodlast = lod;
	  curTexBaseAddr = curTexLoadAddr - sstLinearMipMapOffset(lod, ar, bpt * 8, FXFALSE,
								  FXFALSE, FXFALSE, FXFALSE);
	  curTexBaseAddr = ((curTexBaseAddr + 15) & ~15) & SST_TEXTURE_ADDRESS;
	  gdbg_info(5,"new texture relocated from 0x%x to 0x%x, lodmin=%d ar=%d bpt=%d\n",
		    lastTexBaseAddr<<3, curTexBaseAddr,
		    lod, ar, bpt);
	  SET(sst->texBaseAddr,curTexBaseAddr);
	  needs_idle = 1;
	  newBases[i] = curTexBaseAddr;
	}
	if (lod != lodlast) {
	    gdbg_info(6,"    new lod = %d\n",lod);
	    lodlast = lod;
	}

	w = 1 << (8-lod);
	if (!(curTexLOD & SST_LOD_S_IS_WIDER)) {// if S is the short side
	    w >>= ar;				// reduce the width
	    if (w < 1) w = 1;
	}

	if ((bpt==1) && (curTexMode & SST_SEQ_8_DOWNLD)) {
	    s = addr & 0x0FF;
	}
	else {
	    s = (addr >> 1) & 0xFF;
	}
	t = (addr >> 9) & 0xFF;
//	gdbg_info(9,"\tlod=%d  s,t=%d,%d\n",lod,s,t);
	addr = SST_TEX_ADDRESS(sst) + sstLinearMipMapOffset(lod, ar, bpt * 8, FXFALSE,
							    FXFALSE, FXFALSE, FXFALSE);

	addr += (s + t * w) * bpt;
	curTexLoadAddr += 4;
	if (diago.halInfo->hsim & HSIM_TREX_BACKDOOR_TEXWRITES)
	if (needs_idle && diago.writeFifo)
	    sst_idle_really(sst);
	needs_idle = 0;

	if (addr & 2) {
	    SET16(*(unsigned long *)addr,(FxU16) *data);
	}
	else
	    SET(*(unsigned long *)addr,*data);
	// return without modifying *_addr
	return 1;
    }
    else if (addr & 0x00400000) {	// if a write to SST-1 3D lfb space
	addr += SST_LFB_OFFSET - 0x00400000UL;
	// shift the Y coordinate over by 1 bit (YUCK) assumes 16-bit LFBs
	addr = ( addr & ~(0x7FF<<11) ) | ((addr & (0x7FF<<11))<<1);
    }
    else {				// else just a register write
	addr += SST_3D_OFFSET;		// convert to H3 address
	// NOTE: we must relocate all textures !!!
	if (SAME_ADDR(addr,sst->texBaseAddr)) {
	    *data &=  0x0007FFFF;
	    lastTexBaseAddr = *data;
	    i = lookupTex(lastTexBaseAddr);
	    if (i >= 0)
	        curTexBaseAddr = newBases[i];
	    else {
		oldBases[numTextures] = lastTexBaseAddr;
		newBases[numTextures] = 0xFFFFFFFF;
		numTextures++;
	    }

	    *data = curTexBaseAddr;
	}
	if (SAME_ADDR(addr,sst->textureMode)) {	// we need to shadow this
	    curTexMode = *data;
	}
	if (SAME_ADDR(addr,sst->tLOD)) {
	    curTexLOD = *data;
	}
	needs_idle = 1;
    }
    *_addr = addr;
    return 0;
}

// set a random stencil mode
void randomStencils(SstRegs *sst)
{
    int s = iRandom(4);

    gdbg_info(4,"switching to random stencil case %d\n",s);
    switch(s) {
	case 0:		// increment all pixels to count DC
	    SET(sst->stencilMode, SST_STENCIL_ENABLE | SST_STENCIL_FUNC | SST_STENCIL_WMASK);
	    SET(sst->stencilOp, (SST_SOP_KEEP<<SST_STENCIL_SFAIL_OP_SHIFT) |
			(SST_SOP_INC<<SST_STENCIL_ZFAIL_OP_SHIFT) |
			(SST_SOP_INC<<SST_STENCIL_ZPASS_OP_SHIFT));
	    break;
	case 1:		// disable totally
	    SET(sst->stencilMode, 0);
	    break;
	case 2:		// increment all pixels that pass the zbuffer
	    SET(sst->stencilMode, SST_STENCIL_ENABLE | SST_STENCIL_FUNC | SST_STENCIL_WMASK);
	    SET(sst->stencilOp, (SST_SOP_KEEP<<SST_STENCIL_SFAIL_OP_SHIFT) |
			(SST_SOP_KEEP<<SST_STENCIL_ZFAIL_OP_SHIFT) |
			(SST_SOP_INC<<SST_STENCIL_ZPASS_OP_SHIFT));
	    break;
	case 3:		// write a pixel every OTHER time!!! (NEQ to ref)
	    SET(sst->stencilMode, SST_STENCIL_ENABLE | SST_SFUNC_LT | SST_SFUNC_GT |
				(0x23 << SST_STENCIL_REF_SHIFT) |
				SST_STENCIL_MASK | SST_STENCIL_WMASK);
	    SET(sst->stencilOp, (SST_SOP_NEG<<SST_STENCIL_SFAIL_OP_SHIFT) |
			(SST_SOP_KEEP<<SST_STENCIL_ZFAIL_OP_SHIFT) |
			(SST_SOP_REPLACE<<SST_STENCIL_ZPASS_OP_SHIFT));
	    break;
	case 4:		// stencil fail
	    SET(sst->stencilMode, SST_STENCIL_ENABLE);
    }
}

void
main (int argc, char **argv)
{
    char token[1024];
    int startTime,lastTime,endTime,i;
    int guardband;
    FxU32 rmode;
    FxI32 cmdCount,cmdLimit,cmdCountTotal, triangleCount;
    FxU32 addr, data, iaddr,lfbAddr,texAddr;
    FxU8 *sst1_fbiInit3;
    SstRegs *sst;
    FILE *inf;

    //Go through arguments to find hacked arguments
    noOriginSwap = FXFALSE;
    for(i=0; i<argc; i++)
      {
	int j;
	
	if(!strcmp(argv[i], "--noOriginSwap"))
	  {
	    noOriginSwap = FXTRUE;

	    argc--;
	    for(j=i; j<argc; j++)
	      argv[j] = argv[j+1];
	  }
      }

    sst = SST_BEGIN(argc,argv);
    lfbAddr = SST_LFB_ADDRESS(sst);
    texAddr = SST_TEX_ADDRESS(sst);
    sst1_fbiInit3 = (FxU8 *)sst + SST1_FBIINIT3;  // address of sst1's fbiInit3 register

    if(diago.randomPlacement || diago.randomCmdFifoPlacement)
      {
	GDBG_ERROR("replay::main", 
		   "Can't use --diago.randomPlacement or --diago.randomCmdFifoPlacement\n");
	DIAG_FAIL();
      }

    // Print Out Option Description
    if ( diago.printOpts )
    {
	gdbg_printf( "replay option description:\n" );
	gdbg_printf( " --noOriginSwap -> Mask out y-origin flip from trace\n");
	gdbg_printf( " -A -> replay Antique trace files (SST-1)\n");
	gdbg_printf( " -B -> force LOD bias = 0.5\n");
	gdbg_printf( " -c -> enable guard band clipping (default=20/100)\n");
	gdbg_printf( " -d -> delta triangle count between DIFF screens\n");
	gdbg_printf( " -E -> set the error limit\n");
	gdbg_printf( " -M -> forcfully disable LOD dithering\n");
	gdbg_printf( " -O -> initialize color buffer to specified color\n");
	gdbg_printf( " -P -> performance mode, force swap and nops to 0\n");
	gdbg_printf( " -q -> output movie files\n");
	gdbg_printf( " -S -> enable Stencil planes to count DC\n");
	gdbg_printf( " -T -> enable Stencil planes to random modes\n");
	gdbg_printf( " -t -> set guard band clipping to #/100 of screen size\n");
	gdbg_printf( " -Z -> initialize Zbuffer to 0xFFFF\n");
	

        DIAG_FAIL();
    }

    if (diago.infile_name == NULL) {
	gdbg_error("main", "no input file specified\n");
	DIAG_FAIL();
    }
    else {
	inf = fopen(diago.infile_name,"r");
	if (inf == NULL) {
	    gdbg_error("main", "could not open input file '%s'\n",
			diago.infile_name);
	    DIAG_FAIL();
	}
    }

    // HACK: use other options to do special things here
    if (guardband = diago.clamp) {
	guardband = diago.tsize;
	if (guardband < 0)
	    guardband = rRandom(1,-guardband);
    }
    if (guardband) {
	int xmin, xmax, ymin, ymax;
	if (guardband > 100) guardband = 100;
	gdbg_info(1,"Enabling Guard Band Clipping to %d%% of screen\n",guardband);
	// GMT: set 1st clip rect to guard band, 2nd to even pixels
	// then monitor writes to clip rectangles and throw them away
 	rmode = GET(sst->renderMode);
	rmode |= SST_RM_ENGUARDBAND;
	SET(sst->renderMode, rmode);
	xmin = diago.xmaxscreen*(100-guardband)/200;
	xmax = diago.xmaxscreen*(100+guardband)/200;
	ymin = diago.ymaxscreen*(100-guardband)/200;
	ymax = diago.ymaxscreen*(100+guardband)/200;
 	SET(sst->clipLeftRight, (xmin<<16) | xmax);
	SET(sst->clipBottomTop, (ymin<<16) | ymax);
	// round the guard band to even numers
	xmin &= ~1;			// round down
	ymin &= ~1;
	xmax = (xmax + 1) & ~1;		// round up
	ymax = (ymax + 1) & ~1;
 	SET(sst->clipLeftRight1, (xmin<<16) | xmax);
	SET(sst->clipBottomTop1, (ymin<<16) | ymax);
    }

    if (diago.zeroLodFrac || diago.option) {
	diago.checkEveryTriangle = 1;	// allows DIAG_FORCE_* to function
	csimVideo(CSIM_PRIVATE(diago.sstCSIM),FXFALSE);     // disable video
	// -Z option clears zbuffer to 0xFFFF
	if (diago.zeroLodFrac) {
	    DIAG_FORCE_RECT(CSIM_BUF_3D_AUX1,0,0,diago.xmaxscreen,diago.ymaxscreen,
				diago.rgb==32 ? 0xFFFFFF : 0xFFFF);
	}
	// -O clears both color buffers to value
	if (diago.option) {
	    DIAG_FORCE_RECT(CSIM_BUF_3D_FRONT,0,0,diago.xmaxscreen,diago.ymaxscreen,diago.option);
	    DIAG_FORCE_RECT(CSIM_BUF_3D_BACK,0,0,diago.xmaxscreen,diago.ymaxscreen,diago.option);
	}
	csimVideo(CSIM_PRIVATE(diago.sstCSIM),FXTRUE);     // enable video
    }
    diago.checkEveryTriangle = 0;	// forces DIFFSCREEN at end
    cmdCount = cmdCountTotal = triangleCount = 0;
    cmdLimit = diago.drawbuffer;
    curTexLoadAddr = diago.texMemStart;
    gdbg_info(5,"textures start at 0x%x\n",curTexLoadAddr);
    startTime = lastTime = DIAG_TIME();
    gdbg_info(1,"starting time = %d ns\n",startTime);

    // if 32bpp mode and -S then enable stencils to count depth complexity
    if (diago.rgb < 32) diago.multiTexBaseAddr = diago.tsplit = 0;
    if (diago.multiTexBaseAddr)		// if random stencils
	diago.tsplit = 1;		// enable basic stencils
    if (diago.tsplit) {
	SET(sst->stencilMode, SST_STENCIL_ENABLE | SST_STENCIL_FUNC | SST_STENCIL_WMASK);
	SET(sst->stencilOp, (SST_SOP_KEEP<<SST_STENCIL_SFAIL_OP_SHIFT) |
			(SST_SOP_INC<<SST_STENCIL_ZFAIL_OP_SHIFT) |
			(SST_SOP_INC<<SST_STENCIL_ZPASS_OP_SHIFT));
    }

    // now parse the input file
    while (EOF != fscanf(inf,"%1000s",token)) {
	if (diago.multiTexBaseAddr)		// if random stencils
	    if (iRandom(1000)==0)
		randomStencils(sst);

	if (strcmp(token,"gd.120:")==0) {
	    if (fscanf(inf,"%1000s",token) == EOF) goto eof_error;
	    if (strncmp(token,"SET(",4)==0) {	// a SET 32-bit
		sscanf(token+4,"%i",&addr);
		if (token[15]=='-') {		// hack
		    sscanf(token+15,"%*d(%i",&data);
		}
		else {
		    // we read the HEX value to make it easy to hand edit
#ifdef __unix__
		    if (fscanf(inf,"%1000s",token) == EOF) goto eof_error;
		    sscanf(token,"%*d(%i",&data); // for some reason this works
#else
		    fscanf(inf,"%*d(%i",&data);	// and this core dumps on UNIX
#endif
		}

		// If SLI is enabled, don't allow y-origin swapping
		if(diago.sliEnabled && SAME_ADDR(addr, sst->fbzMode))
		  data &= ~(SST_YORIGIN);

		if (diago.randomZA) {		// process ANTIQUE files
		    if (addr == 0x10e20000) {
			static int once;
			if (!once)
			    gdbg_printf("WARNING: skipping writes to PACKER bugfix address\n");
			once = 1;
			goto eol;
		    }

		    if (xlate(sst,&addr, &data)) goto eol;	// XLATE!!!
		    iaddr = SST_FAKE_ADDRESS_GET_OFFSET(addr);
		    if (SST_IS_3D_ADDR(iaddr) || SST_IS_3D_ALT_ADDR(iaddr))
		    if (addr & (FxU32)SST_TMU(0,2)) {	// if chip field has TMU 2
			static int once;
			if (!once) {
			    gdbg_printf("WARNING: skipping writes to TMU 2\n");
			}
			once = 1;
			addr &= (FxU32)SST_TMU(0,2);	// mask off TMU 2
#if 1
			if ((addr&0x1C00) == 0) // if no other chips
			    goto eol;		// then skip the write
						// else write remaining chips
#endif
		    }
		    if (SAME_ADDR(addr,sst->fbzMode)) {

		      //Mask out y origin swap bit if appropriate
		      if(noOriginSwap)
			data &= ~SST_YORIGIN;

			// if the DRAWBUFFER field has changed, emulate it!!!
			if ((curFbzMode ^ data) & SST_DRAWBUFFER) {
			    FxU32 temp = (data & SST_DRAWBUFFER) >> SST_DRAWBUFFER_SHIFT;
			    gdbg_info(4,"emulating SST_DRAWBUFFER change\n");
			    SET(sst->colBufferAddr,diagfb.colBufferAddr[temp]);
			    SET(sst->colBufferAddr,diagfb.colBufferAddrSecondary[temp] | SST_BUFFER_BASE_SELECT);
			    SET(sst->colBufferStride,diagfb.colBufferStride[temp]);			    
			}
			curFbzMode = data;
		    }
		}
		else if (addr < (FxU32)SST_CMDAGP_ADDRESS(sst)) {
		    static int once;
		    if (!once) {
			gdbg_printf("WARNING: funky address 0x%x encountered ...\n", addr);
			gdbg_printf("WARNING: most likely cause is an Antique file, "
					"I recommend the -A option\n",addr);
		    }
		    once = 1;
		}
		if (SAME_ADDR(addr,SST_TMU(sst,0)->textureMode)) {	// TMU0
		    data &= ~0x80000000;		// turn off old SEQ8 download bit
		}
		if (SAME_ADDR(addr,SST_TMU(sst,1)->textureMode)) {	// TMU1
		    data &= ~0x80000000;		// turn off old SEQ8 download bit
		}
		if (SAME_ADDR(addr,*sst1_fbiInit3)) {	// fbiInit3
		    gdbg_printf("WARNING: skipping write of 0x%x to fbiInit3\n",data);
		    goto eol;
		}
		if (SAME_ADDR(addr,sst->nopCMD)) {
		    if (diago.perspective && (data != 0)) {
			gdbg_printf("WARNING: forcing nopCMD data to 0\n");
			data = 0;
		    }
		}

		if(SAME_ADDR(addr, sst->lfbMode))
		   {
		     if(noOriginSwap)
		       data &= ~SST_LFB_YORIGIN;
		   }

		if (SAME_ADDR(addr,sst->swapbufferCMD)) {	// hack
		    gdbg_info(3,"emulating swapbufferCMD\n");
		    if (!(data & SST_SWAP_DONT_SWAP)) {		// if really swapping
			FxU32 temp;
			diagSwaps++;				// DIAG_TESTSWAP checks this on exit
			temp = diagfb.colBufferAddr[0];
			diagfb.colBufferAddr[0] = diagfb.colBufferAddr[1];
			diagfb.colBufferAddr[1] = temp;

			temp = diagfb.colBufferAddrSecondary[0];
			diagfb.colBufferAddrSecondary[0] = diagfb.colBufferAddrSecondary[1];
			diagfb.colBufferAddrSecondary[1] = temp;

			temp = diagfb.colBufferStride[0];
			diagfb.colBufferStride[0] = diagfb.colBufferStride[1];
			diagfb.colBufferStride[1] = temp;

			temp = (curFbzMode & SST_DRAWBUFFER)>>SST_DRAWBUFFER_SHIFT;

			SET(sst->leftOverlayBuf,diagfb.colBufferAddr[0]);
			SET(sst->swapBufferPend,0x0);
			SET(sst->colBufferAddr,diagfb.colBufferAddr[temp]);
			SET(sst->colBufferAddr,diagfb.colBufferAddrSecondary[temp] | SST_BUFFER_BASE_SELECT);
			SET(sst->colBufferStride,diagfb.colBufferStride[temp]);
		    }
		    endTime = DIAG_TIME();
		    gdbg_info(1,"swapbuffer, time since last measurement = %d ns (%.2f msec)\n",
				endTime - lastTime, (endTime - lastTime)/1e06);
		    lastTime = endTime;
		    if (diago.perspective && (data != 0)) {
			gdbg_printf("WARNING: forcing swapbufferCMD data to 0\n");
			data = 0;
		    }
		}
 
		// -B forces LOD bias to 0.5
 		if (diago.bilinear < 0) {
		    if (SAME_ADDR(addr,sst->tLOD)) {
			data &= ~SST_LODBIAS;
			data |= 2 << SST_LODBIAS_SHIFT;
		    }
		}
 		if (diago.loddither) {
		    if (SAME_ADDR(addr,sst->textureMode)) {
			data &= ~SST_TLODDITHER;
		    }
		}
		// if testing guardband discard all SETs to clip rect regs
		if (guardband) {
		    if (SAME_ADDR(addr,sst->clipLeftRight) ||
			SAME_ADDR(addr,sst->clipBottomTop) ||
			SAME_ADDR(addr,sst->clipLeftRight1) ||
			SAME_ADDR(addr,sst->clipBottomTop1))
		    {
			gdbg_printf("Ignoring clip register write: %x\n",data);
			goto eol;
		    }
		}

		SET(*(unsigned long *)addr,data);
		if ((addr >= lfbAddr && addr < texAddr) ||
		    SAME_ADDR(addr,sst->triangleCMD) ||
		    SAME_ADDR(addr,sst->FtriangleCMD))
		{
		    cmdCount++;
		    cmdCountTotal++;
		    gdbg_info(3,"command #%d\n",cmdCountTotal);
		    if (SAME_ADDR(addr,sst->triangleCMD) ||
			SAME_ADDR(addr,sst->FtriangleCMD)) {
			if (triangleCount == 0) {
			endTime = DIAG_TIME();
			gdbg_info(1,"1st triangle, time since last measurement = %d ns (%.2f msec)\n",
				endTime - lastTime, (endTime - lastTime)/1e06);
			lastTime = endTime;
			}
			triangleCount++;
		    }
		}
		if (cmdLimit > 0) {
		    if (cmdCount >= cmdLimit) {
			cmdCount = 0;
			if (diago.diff)
			    DIAG_DIFFSCREEN(diago.xmaxscreen-1,diago.ymaxscreen-1);
		    }
		}
	    }
	    else if (strncmp(token,"SET16(",6)==0) {
		sscanf(token+6,"%i",&addr);
		if (token[17]=='-') {		// hack
		    sscanf(token+17,"%*d(%i",&data);
		}
		else {
#ifdef __unix__
		    if (fscanf(inf,"%1000s",token) == EOF) goto eof_error;
		    sscanf(token,"%*d(%i",&data);
#else
		    fscanf(inf,"%*d(%i",&data);
#endif
		}
		SET16(*(unsigned long *)addr,(unsigned short)data);

		// the only valid 16-bit writes are LFB accesses
		if (cmdLimit > 0) {
		    cmdCount++;
		    if (cmdCount >= cmdLimit) {
			cmdCount = 0;
			if (diago.diff)
			     DIAG_DIFFSCREEN(diago.xmaxscreen-1,diago.ymaxscreen-1);
		    }
		}
	    }
	    else if (strncmp(token,"GET(",4)==0) {
		FxU32 gaddr;
		sscanf(token+4,"%i",&gaddr);
		gdbg_printf("WARNING: GET not implemented, skipping get of 0x%x\n",gaddr);
	    }
	    else if (strncmp(token,"GET16(",6)==0) {
		FxU32 gaddr;
		sscanf(token+4,"%i",&gaddr);
		gdbg_printf("WARNING: GET16 not implemented, skipping get of 0x%x\n",gaddr);
	    }
	}
	else if (strcmp(token,"T+4")==0) {	// special compact texture writes
	    fscanf(inf,"%x",&data);		// if it was sequential
	    addr += 4;				// then it still is sequential
	    if (diago.randomZA)			// then re-translate it
		if (xlate(sst,&addr,&data)) goto eol;
	    SET(*(unsigned long *)addr,data);
	}
	else if (strcmp(token,"TEX")==0) {	// if starting at a new address
	    fscanf(inf,"%x %x",&addr,&data);
	    if (diago.randomZA)			// then re-translate it
		if (xlate(sst,&addr,&data)) goto eol;
	    SET(*(unsigned long *)addr,data);
	}
    eol:
	fscanf(inf,"%[^\n]s",token);
    }

    sst_idle_really(sst);
    endTime = DIAG_TIME();
    gdbg_info(1,"EOF: time since last measurement = %d ns (%.2f msec)\n",
		endTime - lastTime, (endTime - lastTime)/1e06);
    gdbg_info(1,"EOF: total time since start      = %d ns (%.2f msec)\n",
		endTime - startTime, (endTime - startTime)/1e06);

    gdbg_info(1,"Total commands executed: %6d\n",cmdCount);
    gdbg_info(1,"Total triangles rendered:%6d\n",triangleCount);

    DIAG_PASS(0);
    return;

eof_error:
    gdbg_error("parse","unexpected end of file\n");
    DIAG_FAIL();
}
