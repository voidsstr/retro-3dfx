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
** $Date: 10/11/00 8:18:45 PM$
** NYI tiled texture
*/

#include "allocate.h"
#include "udiag.h"
#include "sstdiag.h"
#include <fximg.h>

#define MODNAME "h3replay"

extern int diagSwaps;		// a hack, we need to increment this

// compares just the register addresses, not the chip or wrap fields
// also have to worry about ALT regs
#define SAME_ADDR(addr,reg) (	\
	((addr&~0x3FFC00) == ((~0x3FFC00)&(FxU32)&reg)) || \
	((addr&~0x3FFC00) == ((~0x3FFC00)&(SST_3D_ALT_OFFSET-SST_3D_OFFSET+(FxU32)&reg))) )

#define TEX_OFFSET(addr,sst) (addr - SST_TEX_ADDRESS(sst))


#define XGETARG() opts[1] ? done = 1, ++opts : \
			  (--argc > 0) ? done = 1, *++argv : \
					 (char *)Xusage() \

// shadow register
static SstGRegs sGRegs;
// draw buffer swapping
static FxU32 lastColBuffer = 0xFFFFFFFF;
static int curColBuffer = 0;

static FxU32 texDataCount = 0;

// texture map
static int numTextures;
#define MAXTMAP 8096
static FxU32 oldBases[MAXTMAP], newBases[MAXTMAP];
static void initTex()
{
  numTextures = -1;
}
// lookup a texure, return index if found, -1 if not found
static int getTex(FxU32 base,FxU32 *newbase)
{
    int i;

    for (i=0; i<=numTextures; i++) {
      if (oldBases[i] == base) {
	*newbase =  newBases[i];
	gdbg_info(5, "Found new base %x for old base %x  %d !!\n",
		  *newbase,base,i);
	return i;
      }
    }
    return -1;
}

static void insertTex(FxU32 oldBase,FxU32 newBase)
{
  numTextures++;
  if (numTextures > MAXTMAP-1) {
    gdbg_error(MODNAME, "Too many textures to relocate !!\n");
    DIAG_FAIL();
  }
  newBases[numTextures] = newBase;
  oldBases[numTextures] = oldBase;
  gdbg_info(5, "Mapping new base %x for old base %x  %d !!\n",
	    newBase,oldBase,numTextures);
}
 
// secondary stream
#define MEOF 1
#define SEOF 2
static void *sinfname;
enum { SSINGLE, SDIRECT,SCMDFIFOONE };
static int sStream;

// options
static int ignoreSrcBaseAddr;
static int ignoreDstBaseAddr;
static FxU32 pOffset; // min offset of primary stream from mintrashmem
static int mburst,sburst;
static char *Xusage()
{
    gdbg_printf("Error Xusage::\n");
    gdbg_printf("\"-xI<second input file>\"\t\n");
    gdbg_printf("\"-xw<n>\"\tSecondary 0:single cmdfifo 1:direct 2:CMDFIFOONE \n");
    gdbg_printf("\"-xn<n>\"\tSecondary /Primary  burst.  Third decimal and first two decimal digits respectively\n");
    gdbg_printf("\"-xo<n>\"\tMin Primary stream offset from minTrashMem \n");
    DIAG_FAIL();
    return(0);
}

static void XParseOpts(int argc, char **argv)
{
    char *opts = 0;
    int burst;
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
	    case 'I':
	      sinfname = XGETARG();
	      GDBG_PRINTF("INFO: Secondary file name  %s\n",sinfname);
	      done = 1;
	      break; 
	    case 'w':
	      sscanf(XGETARG(), "%i", &sStream);
	      GDBG_PRINTF("INFO: secondary stream select %d\n",sStream);
	      done = 1;
	      break; 
	    case 'n':
	      sscanf(XGETARG(), "%i", &burst);
	      sburst = burst/100;
	      mburst = burst - sburst*100;
	      done = 1;
	      break; 
	    case 'o':
	      sscanf(XGETARG(), "%i", &pOffset);
	      GDBG_PRINTF("INFO: offset  %d\n",pOffset);
	      done = 1;
	      break; 
	    case 's':
	      sscanf(XGETARG(), "%i", &ignoreSrcBaseAddr);
	      GDBG_PRINTF("INFO: ignore Src Base  %d\n",ignoreSrcBaseAddr);
	      done = 1;
	      break; 
	    case 'd':
	      sscanf(XGETARG(), "%i", &ignoreDstBaseAddr);
	      GDBG_PRINTF("INFO: ignore Dst Base %d\n",ignoreDstBaseAddr);
	      done = 1;
	      break; 
	    default:
	      Xusage();
	    }
	    opts += 1;
	}
    }
}

// stream control
static int tempwf;
static void disableFifo() {
  // reach into privies..
  CSIM_PRIVATE(diago.sstCSIM)->inCmdFifoExecMode = 1;
}
static void enableFifo() {
  CSIM_PRIVATE(diago.sstCSIM)->inCmdFifoExecMode = 0;
}

static void pushS(int direct)
{
  if (direct) {
    tempwf = diago.writeFifo; diago.writeFifo = 0; 
  }
  else
  switch(sStream) {
  case SSINGLE: break;
  case SDIRECT: tempwf = diago.writeFifo; diago.writeFifo = 0; break;
  case SCMDFIFOONE: hb_selectFifo(1); break;
  }
  if (!diago.writeFifo && tempwf)
    disableFifo();
}
static void popS(int direct) 
{
  if (!diago.writeFifo && tempwf)
    enableFifo();
  if (direct) {
    diago.writeFifo = tempwf;
  }
  else 
  switch(sStream) {
  case SSINGLE: break;
  case SDIRECT: diago.writeFifo = tempwf;break;
  case SCMDFIFOONE: hb_restoreFifo(); break;
  }
}

static void pushDirect() { pushS(1); }
static void popDirect() { popS(1); }

// buffer placement 
#define CEIL(x,y)      ( ((x)+(y)-1) / (y) )  // divide x/y, rounding up
#define MAX_TILE_STRIDE_SLOP     2  // stride will be a max of 2 tiles larger than reqd
#define MAX_LINEAR_STRIDE_SLOP  64  // stride will be a max of 64 bytes larger than reqd
#define MAX_BASE_ADDR_SLOP      (SST_TILE_SIZE-1)  // max of one tile


// here's some sample input to look at
// gd.120:        SET(0x10000110,      18321(0x00004791)) 0        FBZMODE
// gd.120:        GET(0x10000000,          0(0x00000000)) 0         STATUS

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
    int startTime,lastTime,endTime;
    int guardband;
    FxU32 rmode;
    FxI32 cmdCount,cmdLimit,cmdCountTotal, triangleCount;
    FxU32 addr, data,lfbAddr,texAddr,texStart;
    SstRegs *sst;
    FILE *inf,*sinf,*minf;
    int neof = 0;
    int doline;
    int needs_idle = 1;
    int burst;
    SstCRegs *sstc;
    SstIORegs *sstio;
    SstGRegs *sstg;

    sst = SST_BEGIN(argc,argv);
    sstc = (SstCRegs *)SST_CMDAGP_ADDRESS(sst);
    sstio = (SstIORegs *)SST_IO_ADDRESS(sst);
    sstg = SSTG_CHIP(sst);
    lfbAddr = SST_LFB_ADDRESS(sst);
    texAddr = SST_TEX_ADDRESS(sst);
    //
    sinfname = NULL;
    ignoreSrcBaseAddr = 1;
    ignoreDstBaseAddr = 1;
    pOffset = 0;
    mburst = 1; sburst =1 ;
    initTex();
    sStream = SSINGLE;
    XParseOpts(argc,argv);
    // Print Out Option Description
    if ( diago.printOpts )
    {
	gdbg_printf( "replayd option description:\n" );
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
	// diago.rectangular
	gdbg_printf( " -r -> Relocate \n");

        DIAG_FAIL();
    }

    if (diago.infile_name == NULL) {
	gdbg_error(MODNAME, "no input file specified\n");
	DIAG_FAIL();
    }
    else {
	minf = fopen(diago.infile_name,"r");
	neof |= MEOF;
	if (minf == NULL) {
	  gdbg_error(MODNAME, "could not open input file '%s'\n",
		     diago.infile_name);
	  DIAG_FAIL();
	}
    }
    if (sinfname != NULL) {
      sinf = fopen(sinfname,"r");
      neof |= SEOF;
      if (sinf == NULL) {
	gdbg_error(MODNAME, "could not open secondary input file '%s'\n",sinfname);
	DIAG_FAIL();
      }
    }
    else sinf = NULL;


    if (sStream == SCMDFIFOONE && diago.whichFifo < 2) {
	gdbg_error(MODNAME, "Second stream cannot run.\n");
	DIAG_FAIL();
    }

    if (diago.rectangular) {
      //todo: check limits, noop log writes
      pushDirect();
      
      sGRegs.dstBaseAddr = allocate(diago.ymaxscreen * diago.xmaxscreen * 4 + MAX_BASE_ADDR_SLOP,
				    "wax", normalPlacement);      
      sGRegs.srcBaseAddr = sGRegs.dstBaseAddr;
      SET(sstg->dstBaseAddr,sGRegs.dstBaseAddr);
      SET(sstg->srcBaseAddr,sGRegs.srcBaseAddr);
      GDBG_INFO(0,"+WAX src addr,stride=0x%08x,0x%08x \n",sGRegs.srcBaseAddr,0xdead);
      GDBG_INFO(0,"+WAX dst addr,stride=0x%08x,0x%08x \n",sGRegs.dstBaseAddr,0xdead);

      popDirect();
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
    startTime = lastTime = DIAG_TIME();
    gdbg_info(1,"starting time = %d ns\n",startTime);

    // send NOP to make sure cmdfifo0 trigerred first.
    sst_idle_really(sst);
    inf = minf;
    burst = 0;

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
    while (neof) {
      doline = 0;
      switch(neof) {
      case MEOF:
	inf = minf;
	if (EOF != fscanf(inf,"%1000s",token))
	  doline = 1;
	else neof = 0;
	break;
      case SEOF:
	inf = sinf;
	if (EOF != fscanf(inf,"%1000s",token))
	  doline = 1;
	else neof = 0;
	break;
      case (SEOF|MEOF):
	if (burst <= 0) {
	  inf = iRandom(1) ? sinf : minf;
	  burst = (inf == sinf) ? sburst : mburst;
	}
	if (EOF != fscanf(inf,"%1000s",token))
	  doline = 1;
	else neof = (inf == sinf) ? MEOF : SEOF ;
	burst--;
	break;
      }
      if (doline) {
	if (diago.multiTexBaseAddr)		// if random stencils
	    if (iRandom(1000)==0)
		randomStencils(sst);

	  if (inf == sinf) {
	    pushS(0);
	  }

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
		if (SST_IS_RAW_LFB_ADDR(addr-SST_BASE_ADDRESS(sst))) {
		    static int once;
		    if (!once) {
			gdbg_printf("WARNING: RAW LFB Not supported %x ...\n", addr);
		    }
		    once = 1;
		}

		if (addr >= (FxU32)SST_YUV_ADDRESS(sst)) {
		}
		if (addr < (FxU32)SST_CMDAGP_ADDRESS(sst)) {
		    static int once;
		    if (!once) {
			gdbg_printf("WARNING: funky address 0x%x encountered ...\n", addr);
			gdbg_printf("WARNING: most likely cause is an Antique file, "
					"I recommend the -A option\n",addr);
		    }
		    once = 1;
		}
		if (SAME_ADDR(addr,sst->nopCMD)) {
		    if (diago.perspective && (data != 0)) {
			gdbg_printf("WARNING: forcing nopCMD data to 0\n");
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
		if (SAME_ADDR(addr,SST_TMU(sst,0)->textureMode)) {	// TMU0
		    data &= ~0x80000000;		// turn off old SEQ8 download bit
		}
		if (SAME_ADDR(addr,SST_TMU(sst,1)->textureMode)) {	// TMU1
		    data &= ~0x80000000;		// turn off old SEQ8 download bit
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

		// If SLI is enabled, don't allow y-origin swapping
		if(diago.sliEnabled && SAME_ADDR(addr, sst->fbzMode))
		  data &= ~(SST_YORIGIN);

		// if relocating buffers
		if (diago.rectangular) {
		  // gd.120:	       SET(0x10100010,          0(0x00000000)) G	DSTBASEADDR
		  // gd.120:	       SET(0x10100034,          0(0x00000000)) G	SRCBASEADDR
		  // Yuk !
		  if (ignoreDstBaseAddr && addr == (FxU32)&(sstg->dstBaseAddr))  
		    gdbg_printf("Ignoring dstbaseaddr  %x\n",data);
		  else if (ignoreSrcBaseAddr && (addr == (FxU32)&(sstg->srcBaseAddr)))
		    gdbg_printf("Ignoring srcbaseaddr  %x\n",data);
		  else if (addr == (FxU32)&(sst->colBufferAddr)) {
		    // Ignore secondary stream. Cannot have 2 streams changing colBufferAddr
		    // Todo: filter out all 3d writes in secondary stream
		    if (inf == minf) { 
		      // assume double ??
		      if (lastColBuffer != data && lastColBuffer != 0xFFFFFFFF) {
			// swap
			SET(*(unsigned long *)addr,diagfb.colBufferAddr[curColBuffer]);
			SET(*(unsigned long *)addr,diagfb.colBufferAddrSecondary[curColBuffer] |
			    SST_BUFFER_BASE_SELECT);
			gdbg_info(5,"Emulating draw buffer change %x curColBuffer %d\n",data,
				  curColBuffer);
			curColBuffer = !curColBuffer;
		      }
		      else 
			gdbg_info(5,"Ignoring colbufferaddr  %x\n",data);
		      lastColBuffer = data;
		    }
		  }
		  else if (addr == (FxU32)&(sst->auxBufferAddr))  
		    gdbg_info(5,"Ignoring auxbufferaddr  %x\n",data);
		  else if (addr == (FxU32)&(sst->auxBufferStride))  
		    gdbg_info(5,"Ignoring auxbufferstride  %x\n",data);
		  else if (addr == (FxU32)&(sst->colBufferStride))  
		    gdbg_info(5,"Ignoring colbufferstride  %x\n",data);
		  else if (addr == (FxU32)&(sst->leftOverlayBuf))  
		    gdbg_info(5,"Ignoring leftoverlaybuf  %x\n",data);
		  // strobe *bufferStride registers into colBufferStride[*]
		  else  if (SAME_ADDR(addr,sst->texBaseAddr1) 
			    || SAME_ADDR(addr,sst->texBaseAddr2) 
			    || SAME_ADDR(addr,sst->texBaseAddr38) ) {
		    gdbg_info(5, "Ignoring texBaseAddrN %x=> %x\n",addr,data);
		  }
		  else {
		    if (SAME_ADDR(addr,sst->texBaseAddr) 
			&& (data != 0xFFFFFFFF)) {
		      FxU32 newbase;
		      // assumes wrapping texture buffer from replay traces
		      //todo: if input texbase addresses wraps...trash texbase mapping table
		      if (getTex(data,&newbase) != -1) {
			data = newbase;
			gdbg_info(5, "Found new texture old %x cur %x\n",
				  data,newbase);
		      }
		      else {
			FxU32 newbase;
			static FxU32 textureCounter=0;
			char textureName[32];

			//Allocate space in the frame buffer for the texture
			sprintf(textureName, "texture %d", textureCounter++);
			newbase=allocate(0x30000, textureName, normalPlacement);
			
			//Every once in a while, print out info on where
			//the texture maps are located
			if(textureCounter==10 || ((textureCounter % 50) == 0))
			  memoryMap();			

			//Mask off the bits for the texture base address
			data &= SST_TEXTURE_ADDRESS;

			//Add this texture to the texture table
			insertTex(data, newbase);
			gdbg_info(5, "Relocating texbaseaddr %x to %x\n", data, newbase);

			//Write down that this texture hasn't had any data downloaded
			texDataCount = 0;

			//sst_idle_really(sst);

			//Set the new translated texture base address
			SET(sst->texBaseAddr, newbase);
			needs_idle = 1;
			goto eol; // wait for TEX			
		      }
		    }
		    
		    if (addr == (FxU32)&(sst->clipLeftRight)) {
		      gdbg_info(5,"Checking  clipleftright  %x\n",data);
		      if ((data & SST_MASK(16)) >  (FxU32)diago.xmaxscreen) {
			gdbg_error(MODNAME, "ClipLeftRight too big.\n");
			DIAG_FAIL();
		      }
		    }
		    if (addr == (FxU32)&(sst->clipBottomTop)) {
		      gdbg_info(5,"Checking  clipBottomTop  %x\n",data);
		      if ((data & SST_MASK(16)) >  (FxU32)diago.ymaxscreen) {
			gdbg_error(MODNAME, "ClipBottomTop too big\n");
			DIAG_FAIL();
		      }
		    }
		    if (addr == (FxU32)&(sst->colBufferStride)) {
		      if (data != diagfb.colBufferStride[0]) {
			gdbg_info(5, "WARNING Col Buffer stride mismatch 0x%x 0x%x\n",data,
				   diagfb.colBufferStride[curColBuffer]);
		      }
		      goto eol;
		    }
		    if (addr == (FxU32)&(sst->auxBufferStride)) {
		      if (data != diagfb.auxBufferStride) {
			gdbg_info(5, "WARNING Aux Buffer stride mismatch 0x%x 0x%x\n",data,
				   diagfb.auxBufferStride);
		      }
		      goto eol;
		    }
		    SET(*(unsigned long *)addr,data);
		  }
		}  		//for if (diago.rectangular)
		// not relocating buffers
		else {
		  if (addr == (FxU32)&(sst->colBufferStride)) {
		    if (data != diagfb.colBufferStride[0]) {
		      gdbg_error(MODNAME, " Col Buffer stride mismatch 0x%x 0x%x. Use -r.\n",
				 data,diagfb.colBufferStride[curColBuffer]);
		      DIAG_FAIL();
		    }
		  }
		  if (addr == (FxU32)&(sst->auxBufferStride)) {
		    if (data != diagfb.auxBufferStride) {
		      gdbg_error(MODNAME, " Aux Buffer stride mismatch 0x%x 0x%x. Use -r \n",
				 data,diagfb.auxBufferStride);
		      DIAG_FAIL();
		    }
		  }
		  SET(*(unsigned long *)addr,data);
		}
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
	    texStart += 4;				// then it still is sequential
	    texDataCount = (TEX_OFFSET(texStart,sst) > texDataCount) ? 
	      TEX_OFFSET(addr,sst):texDataCount;
	    SET(*(unsigned long *)texStart,data);
	}
	else if (strcmp(token,"TEX")==0) {	// if starting at a new address
	  fscanf(inf,"%x %x",&addr,&data);
	  gdbg_info(5,"new tex offset %x %x\n",addr,data);
	  texStart = addr;
	  texDataCount =  TEX_OFFSET(addr,sst) + 4;
	  if (diago.halInfo->hsim & HSIM_TREX_BACKDOOR_TEXWRITES)
	    if (needs_idle && (diago.writeFifo & 0x1)) {
	      sst_idle_really(sst);
	      needs_idle = 0;
	    }
	  SET(*(unsigned long *)addr,data);
	}
    eol:
	fscanf(inf,"%[^\n]s",token);
	if (inf == sinf) 
	  popS(0);
      }
    }
    sst_idle_really(sst);
    endTime = DIAG_TIME();
    gdbg_info(1,"EOF: time since last measurement = %d ns (%.2f msec)\n",
		endTime - lastTime, (endTime - lastTime)/1e06);
    gdbg_info(1,"EOF: total time since start      = %d ns (%.2f msec)\n",
		endTime - startTime, (endTime - startTime)/1e06);

    gdbg_printf("Total commands executed: %6d\n",cmdCount);
    gdbg_printf("Total triangles rendered:%6d\n",triangleCount);

    gdbg_printf("total pixels drawn: %d\n",
		CSIMG_PRIVATE(&CSIM_PRIVATE(diago.sstCSIM)->gui)->pixelsOut2d);

    DIAG_PASS(0);
    return;

eof_error:
    gdbg_error("parse","unexpected end of file\n");
    DIAG_FAIL();
}

