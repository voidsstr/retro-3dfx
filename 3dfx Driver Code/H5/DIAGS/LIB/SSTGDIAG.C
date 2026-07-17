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
** $Date: 10/11/00 8:12:03 PM$
*/

#include <assert.h>

#include "allocate.h"
#include "udiag.h"
#include "sstdiag.h"
#include "hsimio.h"
#include "h3gdefs.h"

//----------------------------------------------------------------------
// this file has routines to emulate all the functions in the 2D GUI
// engine's pixel pipeline
//
// HACK NOTE: we use the CSIM simulator's register structure as a shadow
// we reach into it to read the state of the 2D regs
//----------------------------------------------------------------------


//----------------------------------------------------------------------
// init the screen array to some random values
//----------------------------------------------------------------------
void sstg_init_random_screen(SstGRegs *sstg, FxU32 screen[MAXSCREEN][MAXSCREEN])
{
    int x,y, dbg300, dbg198, dbg199;
    unsigned int saveseed = getSeed(), saveCET;
    FxU32 col;

    sst_idle_really(diago.sst);
    setSeed(999);		// always generate the same screen
    gdbg_info( 2, "Initializing the screen to random colors ... " );
    csimVideo(CSIM_PRIVATE(diago.sstCSIM),FXFALSE);	// disable video
    saveCET = diago.checkEveryTriangle;
    diago.checkEveryTriangle = 1;
    dbg198 = GDBG_GET_DEBUGLEVEL(198);
    dbg199 = GDBG_GET_DEBUGLEVEL(199);
    dbg300 = GDBG_GET_DEBUGLEVEL(300);
    GDBG_SET_DEBUGLEVEL(198,GDBG_GET_DEBUGLEVEL(301));
    GDBG_SET_DEBUGLEVEL(199,GDBG_GET_DEBUGLEVEL(301));
	
    if (diago.halInfo->hw) {
      CsimPrivate *cp = CSIM_PRIVATE(diago.sstCSIM);
      int start = csimPixelAddress(diago.sstCSIM,CSIM_BUF_2D_DST,0,0);
      int end = csimPixelAddress(diago.sstCSIM,CSIM_BUF_2D_DST,diago.xmaxscreen,diago.ymaxscreen);
      
      for ( y = 0; y < diago.ymaxscreen; y++ ) {
	for ( x = 0; x < diago.xmaxscreen; x++ ) {
	  screen[y][x] = col = iRandom(0xFFFFFFFF);
	  if (dbg300)
	    gdbg_info_more(300,"init random screen %d,%d = 0x%x\n",x,y,col);
	  csimWritePixel(diago.sstCSIM, CSIM_BUF_2D_DST, x, y, col);
	}
      }

      HW_MEM_WR2(start,(FxU8*)(cp->memory+start),end-start);

    } else {

      for ( y = 0; y < diago.ymaxscreen; y++ ) {
	for ( x = 0; x < diago.xmaxscreen; x++ ) {
	  screen[y][x] = col = iRandom(0xFFFFFFFF);
	  if (dbg300)
	    gdbg_info_more(300,"init random screen %d,%d = 0x%x\n",x,y,col);
	  DIAG_FORCE_PIXEL(CSIM_BUF_2D_DST, x, y, col);
	}
      }

    }

    setSeed(saveseed);
    diago.checkEveryTriangle = saveCET;
    GDBG_SET_DEBUGLEVEL(198,dbg198);
    GDBG_SET_DEBUGLEVEL(199,dbg199);
    gdbg_info_more( 2, "done.\n" );
    csimVideo(CSIM_PRIVATE(diago.sstCSIM),FXTRUE);	// enable video
}

//
// allocate a private memory array the same size as the csim's
//
// optionally, initialize the shadow memory by calling 
// sstg_init_random_memory()
//
FxU8 *sstg_alloc_shadow_memory(SstRegs *sst, int initialize)
{
  FxU8 *p;
  int size = CSIM_PRIVATE(diago.sstCSIM)->memorySizeInBytes;

  p = (FxU8 *) calloc( size, sizeof(FxU8) );

  if ( p == NULL ) {
    GDBG_ERROR("sstg_alloc_shadow_memory","unable to allocate 0x%x bytes\n",size);
    DIAG_FAIL();
  }

  return p;
}

  
//----------------------------------------------------------------------
// init the memory array to some random values
// rolls through 107 (prime number) random values, makes diag startup
// run much faster
//----------------------------------------------------------------------
void sstg_init_random_memory(FxU8 *mem)
{
    FxU32 i, j;
    FxU32 saveseed, dbg198, dbg199;
    CsimPrivate *cp = CSIM_PRIVATE(diago.sstCSIM);
    FxU32 col, randoms[107];
    FxU32 start, stop;
    FxBool firstTime;
    FxU32 nrandoms = sizeof(randoms)/sizeof(FxU32);

    gdbg_info( 2, "Initializing the memory to random colors ... " );
    csimVideo(cp,FXFALSE);		// disable video

    // always generate the same screen
    saveseed = getSeed();
    setSeed(999);

    // don't echo memory accesses
    dbg198 = GDBG_GET_DEBUGLEVEL(198);
    dbg199 = GDBG_GET_DEBUGLEVEL(199);
    GDBG_SET_DEBUGLEVEL(198,GDBG_GET_DEBUGLEVEL(301));
    GDBG_SET_DEBUGLEVEL(199,GDBG_GET_DEBUGLEVEL(301));

    // initialize the pool of random frame buffer values
    for ( i=0; i<nrandoms; i++ )
      randoms[i] = iRandom(~0x0UL);

    // initialize "real" and "shadow" memory
    if(diago.halInfo->hw) 
      {
	j=0;
	firstTime = FXTRUE;
	
	while(getEmptyRange(&start, &stop, firstTime))
	  {
	    for(i=start; i<stop; i+=4)
	      {
		col = randoms[j++];
		
		if ( j >= nrandoms )
		  j = iRandom(nrandoms);
	    
		csimWriteMem32(cp, i, col);
	      }
	    
	    HW_MEM_WR2(start,(FxU8*)(cp->memory+start), stop - start);
	    if ( mem )
	      memcpy(mem,(FxU8*)(cp->memory+start), stop - start);

	    firstTime = FXFALSE;
	  }
      }
    else
      {
	j=0;
	firstTime = FXTRUE;
	
	while(getEmptyRange(&start, &stop, firstTime))
	  {
	    for(i=start; i<stop; i+=4)
	      {
		col = randoms[j++];
		
		if ( j >= nrandoms )
		  j = iRandom(nrandoms);

		DIAG_FORCE_MEM(i, col, 4);
		if (mem)
		  writeMem32((char *)mem, i, col, cp->memorySizeInBytes, "sstg");		
	      }	    	    
	    firstTime = FXFALSE;
	  }
      }	      

    // restore seed and debug levels
    setSeed(saveseed);
    GDBG_SET_DEBUGLEVEL(198,dbg198);
    GDBG_SET_DEBUGLEVEL(199,dbg199);

    gdbg_info_more( 2, "done.\n" );
    csimVideo(cp,FXTRUE);		// enable video
}


    
int
sstg_test_screen_pixel(FxI32 x, FxI32 y, FxU32 color)
{
    return DIAG_TEST_PIXEL(CSIM_BUF_2D_DST, x, y, color);
}


//
// read a pixel from memory (either diag's or the simulator's)
// if (memory == NULL) use private CSIM memory, otherwise treat
// "memory" as the pointer to board memory
//
FxU32
sstg_get_screen_color(FxI32 x, FxI32 y, FxU8 *memory)
{
    SstGRegs *sstg = &CSIM_PRIVATE(diago.sstCSIM)->gui;
    CsimPrivate *cp = CSIMG_PRIVATE(sstg);
    FxU32 data;

    if (!ONSCREEN(x, y))
    {
	GDBG_ERROR("sstg_get_screen_color",
		   "(x, y) = (%d,%d) not onscreen!\n", x, y);
	return DIAG_INCERROR();
    }

    if (!memory)
	memory = (FxU8 *) cp->memory;

    data = readPixel(diago.sstCSIM,CSIM_BUF_2D_DST, x, y, memory, "screen");

    return sstg_destination_mask(data);
}
    
// given a srcFormat and a width and height, calculate the exact
// memory footprint 
// the rectangle takes up in board memory (byte address of last pixel of
// last line - byte address of first pixel of first line)
//
static void fault() 
{
    *(int *)0 = 0;
}


FxU32
sstg_src_sizeInBytes(int buffer, FxU32 srcBaseAddr, FxU32 srcFormat, FxU32 w, FxU32 h)
{
    FxU32 bitspp = 0;
    FxU32 bytespp = 0;
    FxU32 returnval;
    FxU32 top, bot;

    SstGRegs *hack_sstg = &CSIM_PRIVATE(diago.sstCSIM)->gui;
    FxU32 hack1 = hack_sstg->srcFormat;
    FxU32 hack2 = hack_sstg->srcBaseAddr;
    FxU32 hack3 = hack_sstg->dstSize;
    FxU32 hack4 = hack_sstg->srcSize;
    hack_sstg->srcFormat = srcFormat;	// backdoor it
    hack_sstg->srcBaseAddr = srcBaseAddr;
    // make sure there's room for at least one pixel because
    // sblt and others are going to go ahead and access the 
    // pixel at (0,0) regardless of w and h
    if ( w == 0 ) 
      w = 1;
    hack_sstg->dstSize = (h << 16) | w;	// needed for blt packed src size calc
    hack_sstg->srcSize = (h << 16) | w;	// needed for sblt packed src size calc
    top = csimPixelAddress(diago.sstCSIM, buffer, w, h);
    bot = csimPixelAddress(diago.sstCSIM, buffer, 0, 0);
    hack_sstg->srcFormat = hack1;		// restore
    hack_sstg->srcBaseAddr = hack2;
    hack_sstg->dstSize = hack3;
    hack_sstg->srcSize = hack4;

    returnval = top - bot;
    
    gdbg_info(211, "sstg_src_sizeInBytes: srcFormat=0x%08x, w=%d, h=%d",
	      srcFormat, w, h);
    gdbg_info_more(211, ", footprint = %d\n", returnval);

    return returnval;
}


//
// choose a random source rectangle from a "surface" of up to size,
// "diago.size" by "diago.size" make sure that it
// doesn't go past the end of memory
// 
//

void
sstg_random_src_rect(int buffer, FxU32 srcFormat, FxU32 srcBase,
		     long *xsOut, long *ysOut, long *wOut, long *hOut)
{
    long w, h;
    long max_xy, max_wh;
    FxU32 maxArea, maxMemoryAddress;
    FxU32 base = (srcBase&SSTG_BASEADDR) >> SSTG_BASEADDR_SHIFT;
    char buf[18], buf2[18];
    int did_1x1 = 0;

    maxMemoryAddress = getMaxExpansion((srcBase&SSTG_BASEADDR) >> SSTG_BASEADDR_SHIFT);
    maxArea = maxMemoryAddress - ((srcBase&SSTG_BASEADDR) >> SSTG_BASEADDR_SHIFT);

    // Restrict w, h to positive values <= diago.tsize or the max the hw can
    // support, whichever is smaller. The gui registers are signed values, so
    // restrict size of numbers to num register bits-1.

    max_xy = SST_MASK(SSTG_XY_SIZE - 1);
    max_wh = (diago.tsize > max_xy) ? max_xy : diago.tsize;
    
    // Generate a value between 1 and max_wh.

    w = iRandom(max_wh - 1) + 1;
    h = iRandom(max_wh - 1) + 1;    

    // find a surface size that fits between srcBaseAddr and the end of
    // memory
    
    while ( base+sstg_src_sizeInBytes(buffer, srcBase, srcFormat, w, h) >= maxMemoryAddress)
    {
	if (iRandom(3) == 0)
	    w -= 1;
	else
	    h -= 1;

	if (w == 0)
	{
	    w = 1;
	    h -= 1;
	}
	if (h == 0)
	{
	    h = 1;
	    w -= 1;
	}

	if (w == 0)
	    w = 1;

	if ((w == 1) && (h == 1))
	{
	    if (!did_1x1)
		did_1x1 = 1;
	    else
	    {
		did_1x1 = 2;
		w = 0;
		h = 0;
		break;
	    }
	}
    }

    // we now have a surface size in (w,h) that we can choose a subrectangle
    // from.  either use the entire surface or a random subrectangle 

    gdbg_info(112, "sstg_random_src_rect: size (%d,%d), ", w, h);
    gdbg_info(112,"sstg_random_src_rect: max mem = 0x%x\n",
	      ((srcBase&SSTG_BASEADDR) >> SSTG_BASEADDR_SHIFT)
	      +sstg_src_sizeInBytes(buffer, srcBase, srcFormat, w, h));

    if (did_1x1 == 2)
    {
	*xsOut = 0;
	*ysOut = 0;
	*wOut = 0;
	*hOut = 0;
    }
    else if (iRandom(2) == 0)
    {
	*xsOut = 0;
	*ysOut = 0;
	*wOut = w;
	*hOut = h;
    }
    else
    {
	*xsOut = iRandom(w - 1);
	*ysOut = iRandom(h - 1);
	*wOut = iRandom((w - *xsOut) - 1) + 1;
	*hOut = iRandom((h - *ysOut) - 1) + 1;

	// don't allow too many 0 widths or heights
	if (iRandom(40) == 0)
	    *wOut = 0;
	if (iRandom(40) == 0)
	    *hOut = 0;
    }

    if ( *wOut == 0 )
	strcpy(buf,"zero width");
    else
	sprintf(buf,"%d",*xsOut + *wOut - 1);

    if ( *hOut == 0 )
	strcpy(buf2,"zero height");
    else
	sprintf(buf2,"%d",*ysOut + *hOut - 1);

    gdbg_info_more(112, "subrectangle (%d,%d) to (%s,%s)\n",
		     *xsOut, *ysOut, buf, buf2);
}


// right now only implements random source base address that doesn't
// overlap with the destination (overlap == 0)
// allowing overlap (overlap == 1) is TBD
//
// This dumb ass function doesn't use all of its arguments!
FxU32 sstg_random_srcBaseAddr(SstGRegs *sstg, FxU32 srcFormat, FxU32 destFormat,
			      FxU32 destBA, FxU32 overlap)
{
  FxI32 width, height;
  FxU32 addr;

  if ( overlap == 1 )
    GDBG_ERROR("sstg_random_srcBaseAddr","overlap=1 is nyi\n");

  //Pick a surface size
  width = iRandom(diago.tsize);
  height = iRandom(diago.tsize);

  addr = sstg_random_srcBaseAddr2(destFormat, destBA, width, height);

  SET(sstg->srcBaseAddr, addr);
  return(addr);
}


FxU32 sstg_random_srcBaseAddr2(FxU32 destFormat, FxU32 destBA, FxU32 dstWidth, FxU32 dstHeight)
{
  FxI32 size;
  FxU32 addr;
  char name[32];
  static FxU32 srcCounter=0;
      
  size = dstWidth * dstHeight * 4;

  //Allocate the memory
  sprintf(name, "rand_src %x", srcCounter++);
  if(!allocateWorker(&addr, size, name, randomPlacement, FXFALSE))
    { //Ran out of frame buffer memory	  
      GDBG_INFO(1, "Warning! Ran out of frame buffer memory! Unallocating non-buffers %s(%d)\n",
		__FILE__, __LINE__);
      memoryMap();
      
      //Unallocate everything that isn't locked (i.e. the buffers)
      unallocateAll();
      
      //Try to reallocate; this time die on failure.
      addr=allocate(size, name, randomPlacement);
    }

  if(diago.ytiled == 1 || diago.ytiled == 3)
    addr |= SSTG_IS_TILED;
  
  return(addr);
}


//
// write a pixel to memory (only the given memory ptr, no implicit csim
// memory write
//
int
sstg_set_screen_color(FxI32 x, FxI32 y, FxU8 *memory, FxU32 data)
{
    SstGRegs *sstg = &CSIM_PRIVATE(diago.sstCSIM)->gui;
    CsimPrivate *cp = CSIMG_PRIVATE(sstg);
    
    if (!ONSCREEN(x, y))
    {
	GDBG_ERROR("sstg_set_screen_color",
		   "(x,y) = (%d,%d) not onscreen!\n", x, y);
	return DIAG_INCERROR();
    }

    if (!memory)
    {
	GDBG_ERROR("sstg_set_screen_color",
		   "can't set simulator's screen through this function!\n");
	return DIAG_INCERROR();
    }

    writePixel(diago.sstCSIM,CSIM_BUF_2D_DST, x, y, data, memory, "screen");

    return(0);
}


//----------------------------------------------------------------------
// generate a totally random command (except for the actual command)
// if a diag doesn't want a bit set, it should clear it afterwards
//----------------------------------------------------------------------
void sstg_random_command_bits(FxU32 *command, FxU32 *commandX)
{
    *command = *commandX = 0;

    if (diago.updatexy) {		// -U (update dstXY)
	if (iRandom(1)) *command ^= SSTG_UPDATE_DSTX;
	if (iRandom(1)) *command ^= SSTG_UPDATE_DSTY;
    }
    if (diago.lstipple) {		// -L (enable line stipple)
	if (iRandom(2)) *command ^= SSTG_EN_LINESTIPPLE;
    }
    if (diago.loddither) {		// -M (monochrome pattern)
	if (iRandom(1)) *command ^= SSTG_MONO_PATTERN;
    }
    if (diago.bilinear < 0) {		// -B (random blit direction)
	if (iRandom(1)) *command ^= SSTG_XDIR;
	if (iRandom(1)) *command ^= SSTG_YDIR;
    }
    if (diago.multiTexBaseAddr) {	// -T (transparent monochrome)
	if (iRandom(1)) *command ^= SSTG_TRANSPARENT;
    }
#if COLORTRANSLUT
    if (diago.lodbias) {		// -m (enable colorTransLut)
	if (iRandom(2)) *command ^= SSTG_EN_CLUT88;
    }
#endif
    if (diago.perspective) {		// -P (pattern offsets)
	*command |= (iRandom(SSTG_X_PATOFFSET>>SSTG_X_PATOFFSET_SHIFT)<<SSTG_X_PATOFFSET_SHIFT);
	*command |= (iRandom(SSTG_Y_PATOFFSET>>SSTG_Y_PATOFFSET_SHIFT)<<SSTG_Y_PATOFFSET_SHIFT);
    }
    if (diago.clamp) {			// -c (clipSelect)
	if (iRandom(1)) *command ^= SSTG_CLIPSELECT;
    }

    if (diago.srcKey) {			// -k (enable source color key)
	if (iRandom(3)) *commandX ^= SSTG_EN_SRC_COLORKEY_EX;
    }
    if (diago.dstKey) {			// -K (enable dest color key)
	if (iRandom(3)) *commandX ^= SSTG_EN_DST_COLORKEY_EX;
    }
    if (diago.vsync) {			// -v (wait for vsync)
#ifdef H3_A0
      { 
	static i=0;
        if (i++ == 0)
	  GDBG_INFO(0,"HACK -- not setting SSTG_WAIT_FOR_VSYNC_EX due to hw bug\n");
      }
#else
	if (iRandom(3)) *commandX ^= SSTG_WAIT_FOR_VSYNC_EX;
#endif
    }
    if (diago.perspective) {		// -P (pattern force row0)
	if (iRandom(3)) *commandX ^= SSTG_PAT_FORCE_ROW0;
    }
    
    //If running in 16bpp, randomly choose between making the MSB non-writeable
    if(diago.dstFormat == 3)
      {
	if(iRandom(1))
	  *commandX ^= SSTG_PRESERVE_MSB;	
      }
}

//This checks to see if an idle is necessary before changing the
//SSTG_PRESERVE_MSB bit in the commandExtra register
void sstgCheckForIdle(SstRegs *sst, FxU32 commandExtra)
{
  static lastCommandExtra=0; 

  if((commandExtra & SSTG_PRESERVE_MSB) != (lastCommandExtra & SSTG_PRESERVE_MSB))
    {
      sst_idle_really(sst);
    }
  
  lastCommandExtra = commandExtra;
}

void sstg_print_stuff(FxU32 command, FxU32 commandX, FxU32 rop, FxU32 srcFormat, FxU32 cFore, FxU32 cBack)
{
    static char *_bpp[] = {"1","8","15", "16","24","32","422Y","411Y"};
    static char *_pack[] = {"Pack-Man","Pack-8","Pack-16","Pack-32"};

    gdbg_info(3,"rops = %02x %02x %02x %02x    srcFormat = %sbpp %s\n",
			(rop>>16)&0xFF,(rop>>8)&0xFF,(rop>>0)&0xFF,(command>>SSTG_ROP0_SHIFT)&0xFF,
			_bpp[(srcFormat&SSTG_SRC_FORMAT)>>SSTG_SRC_FORMAT_SHIFT],
			_pack[(srcFormat&SSTG_SRC_PACK)>>SSTG_SRC_PACK_SHIFT]
			);
    gdbg_info(4,"cfore = 0x%x    cback= 0x%x\n", cFore,cBack);
    gdbg_info(5,"command opts = 0x%x 0x%x",command, commandX);
    if (command & SSTG_UPDATE_DSTX) gdbg_info_more(5," x+=w");
    if (command & SSTG_UPDATE_DSTY) gdbg_info_more(5," y+=h");
    if (command & SSTG_EN_LINESTIPPLE) gdbg_info_more(5," ENls");
    if (command & SSTG_MONO_PATTERN) gdbg_info_more(5," MONOP");
    if (command & SSTG_XDIR) gdbg_info_more(5," X-");
    if (command & SSTG_YDIR) gdbg_info_more(5," Y-");
    if (command & SSTG_TRANSPARENT) gdbg_info_more(5," TRANS");
#if COLORTRANSLUT
    if (command & SSTG_EN_CLUT88) gdbg_info_more(5," ENclut88");
#endif
    if (command & SSTG_CLIPSELECT) gdbg_info_more(5," clip1");

    if (commandX & SSTG_EN_SRC_COLORKEY_EX) gdbg_info_more(5," ENskey");
    if (commandX & SSTG_EN_DST_COLORKEY_EX) gdbg_info_more(5," ENdkey");
    if (commandX & SSTG_WAIT_FOR_VSYNC_EX) gdbg_info_more(5," Vsync");
    if (commandX & SSTG_PAT_FORCE_ROW0) gdbg_info_more(5," patForceRow0");
    gdbg_info_more(5,"\n");
}

void split555(FxU32 *r, FxU32 *g, FxU32 *b, FxU32 col)
{
    *r = (col>>10) & 0x1F;
    *g = (col>>5) & 0x1F;
    *b = (col>>0) & 0x1F;
}

void split565(FxU32 *r, FxU32 *g, FxU32 *b, FxU32 col)
{
    *r = (col>>11) & 0x1F;
    *g = (col>>5) & 0x3F;
    *b = (col>>0) & 0x1F;
}

void split888(FxU32 *r, FxU32 *g, FxU32 *b, FxU32 col)
{
    *r = (col>>16) & 0xFF;
    *g = (col>>8) & 0xFF;
    *b = (col>>0) & 0xFF;
}

int sstg_colorkey(FxU32 col, FxU32 fmt, FxU32 min, FxU32 max)
{
    FxU32 r,g,b, y, u, v;
    FxU32 rmin, gmin, bmin, ymin, umin, vmin;
    FxU32 rmax, gmax, bmax, ymax, umax, vmax;

    switch (fmt & SSTG_SRC_FORMAT) {
	case SSTG_PIXFMT_1BPP:
		return 0;
	case SSTG_PIXFMT_8BPP:
		col &= 0xFF;
		min &= 0xFF;
		max &= 0xFF;
		return (col >= min) && (col <= max);
	case SSTG_PIXFMT_15BPP:
		split555(&r,&g,&b, col);
		split555(&rmin,&gmin,&bmin, min);
		split555(&rmax,&gmax,&bmax, max);
		break;
	case SSTG_PIXFMT_16BPP:
		split565(&r,&g,&b, col);
		split565(&rmin,&gmin,&bmin, min);
		split565(&rmax,&gmax,&bmax, max);
		break;
	case SSTG_PIXFMT_24BPP:
	case SSTG_PIXFMT_32BPP:
		split888(&r,&g,&b, col);
		split888(&rmin,&gmin,&bmin, min);
		split888(&rmax,&gmax,&bmax, max);
		break;
        case SSTG_PIXFMT_422YUV:
        case SSTG_PIXFMT_422UYV:
		split888(&y,&u,&v, col);
		split888(&vmin,&umin,&ymin, min);
		split888(&vmax,&umax,&ymax, max);
		r = y;
		g = u;
		b = v;
		rmin = ymin;
		rmax = ymax;
		gmin = umin;
		gmax = umax;
		bmin = vmin;
		bmax = vmax;
		break;

	default:
		GDBG_ERROR("sstg_colorkey", "invalid pixel format\n");
    }
    return (r >= rmin) && (r <= rmax) &&
	   (g >= gmin) && (g <= gmax) &&
	   (b >= bmin) && (b <= bmax);
}

int sstg_src_colorkey(FxU32 col)
{
    SstGRegs *sstg = &CSIM_PRIVATE(diago.sstCSIM)->gui;
    if (sstg->commandEx & SSTG_EN_SRC_COLORKEY_EX) {
	return sstg_colorkey(col,sstg->srcFormat,
				sstg->srcColorkeyMin,
				sstg->srcColorkeyMax);
    }
    return 0;
}

int sstg_dst_colorkey(FxU32 col)
{
    SstGRegs *sstg = &CSIM_PRIVATE(diago.sstCSIM)->gui;
    if (sstg->commandEx & SSTG_EN_DST_COLORKEY_EX) {
	return sstg_colorkey(col,sstg->dstFormat,
				sstg->dstColorkeyMin,
				sstg->dstColorkeyMax);
    }
    return 0;
}

// choose random background and colorKey colors so that we get decent
// pass/fail distribution, it also sets these colors (but not colorFore)
// NOTE: minor bug in that it calls ckeyRandom888 even in 565 format
// returns the rop that will get selected
int sstg_random_colors(SstGRegs *sstg, FxU32 cfore, FxU32 *cback, FxU32 cdest)
{
    int rop;

    *cback = colRandom32();			// pick a random color
    SET(sstg->colorBack, *cback);		// now set the color
    if (diago.srcKey) {
	FxU32 srcKeyMin, srcKeyMax;
	if (iRandom(1))	{			// if we want to pass
	    ckeyRandom888(1,1,1, cfore,&srcKeyMin,&srcKeyMax);
	    gdbg_info(6,"src ckey pass: 0x%x 0x%x 0x%x\n",cfore,srcKeyMin,srcKeyMax);
	}
	else {
	    srcKeyMin = colRandom32();		// this will normally fail
	    srcKeyMax = colRandom32();
	}
	SET(sstg->srcColorkeyMin, srcKeyMin);
	SET(sstg->srcColorkeyMax, srcKeyMax);
	rop = 2 * sstg_colorkey(cfore, 0, srcKeyMin, srcKeyMax);
    }
    else rop = 0;
    if (diago.dstKey) {
	FxU32 dstKeyMin, dstKeyMax;
	if (iRandom(1))				// if we want to pass
	    ckeyRandom888(1,1,1, cdest,&dstKeyMin,&dstKeyMax);
	else {					// this willl normally fail
	    dstKeyMin = colRandom32();
	    dstKeyMax = colRandom32();
	}
	SET(sstg->dstColorkeyMin, dstKeyMin);
	SET(sstg->dstColorkeyMax, dstKeyMax);
	rop += sstg_colorkey(cdest, 0, dstKeyMin, dstKeyMax);
    }
    return rop;
}

//----------------------------------------------------------------------
// pick a rop based on colorkey results, and then return it
//----------------------------------------------------------------------
FxU32 sstg_pick_rop(FxU32 cmdops, FxU32 cmdXops,
			FxU32 rop, FxU32 csrc, FxU32 cdst, int skey)
{
    int n, dkey;
    SstGRegs *sstg = &CSIM_PRIVATE(diago.sstCSIM)->gui;

    if (diago.checkEveryTriangle || !diago.writeFifo)
    if (sstg->commandEx != cmdXops)
	GDBG_ERROR("sstg_pick_rop", "cmdXops 0x%x != chip's commandEx register 0x%x\n",
			cmdXops, sstg->commandEx);

    dkey = sstg_dst_colorkey(cdst);
    n = skey*2 + dkey - 1;
    if (n < 0)
	rop = cmdops >> SSTG_ROP0_SHIFT;
    else
	rop = rop >> (n*8);
    rop &= 0xFF;
    gdbg_info(9,"diag picking rop[%d] = %02x\n",n+1,rop);
    return rop;
}

// XXX we should limit the rop loop to the size of the destination format???

//----------------------------------------------------------------------
// apply a binary/ternary rasterop to 2/3 colors and return the result
//----------------------------------------------------------------------
FxU32 sstg_rop2(FxU32 rop, FxU32 src, FxU32 dst)
{
    FxU32 i,psd, res = 0;

    src = (src<<1) | (src>>31);		// rotate left one bit
    for (i=0; i<32; i++) {
	psd = (src&2) | (dst&1);
	res |= ((rop >> psd) & 1) << i;
	src = (src>>1) | (src<<31);	// rotate right one bit
	dst >>= 1;
    }
    return res;
}

FxU32 sstg_rop3(FxU32 rop, FxU32 pat, FxU32 src, FxU32 dst)
{
    FxU32 i,psd, res = 0;
	
    src = (src<<1) | (src>>31);		// rotate left one bit
    pat = (pat<<2) | (pat>>30);		// rotate left two bits
    for (i=0; i<32; i++) {
	psd = (pat&4) | (src&2) | (dst&1);
	res |= ((rop >> psd) & 1) << i;
	pat = (pat>>1) | (pat<<31);	// rotate right one bit
	src = (src>>1) | (src<<31);	// rotate right one bit
	dst >>= 1;
    }
    return res;
}

//----------------------------------------------------------------------
// waits for idle
// convenience routine - turms off debug levels for 3D nop command
//----------------------------------------------------------------------
void sstg_idle(SstRegs *sst)
{
    int x;
#ifdef GDBG_INFO_ON
    unsigned char gdSave[GDBG_MAX_LEVELS];
#endif

#ifdef GDBG_INFO_ON
    // for practical reasons we disable debug info during NOP command
    // except during hardware simulation or writing the command fifo
    if (!(diago.halInfo->hsim|diago.writeFifo))
    for (x=105; x<GDBG_MAX_LEVELS; x++) {		// and debug levels
	gdSave[x] = GDBG_GET_DEBUGLEVEL(x);
	GDBG_SET_DEBUGLEVEL(x,0);
    }
#endif
    sst_idle(sst);
#ifdef GDBG_INFO_ON
    if (!(diago.halInfo->hsim|diago.writeFifo))
    for (x=105; x<GDBG_MAX_LEVELS; x++)		// and debug levels
	GDBG_SET_DEBUGLEVEL(x,gdSave[x]);
#endif
}

//----------------------------------------------------------------------
// set the pattern registers to random values
//----------------------------------------------------------------------
void sstg_setpattern_random(SstGRegs *sstg)
{
    FxU32 i,pat;

    for (i=0; i<64; i++) {
	pat = colRandom32();
	if (i==0 && iRandom(1))
	    SET(sstg->pattern0alias,pat);
	else if (i==1 && iRandom(1))
	    SET(sstg->pattern1alias,pat);
	else
	    SET(sstg->colorPattern[i],pat);
    }
}

//----------------------------------------------------------------------
// lookup a pattern entry and return it
//----------------------------------------------------------------------
FxU32 sstg_getpattern(int x, int y, int *visible)
{
    SstGRegs *sstg = &CSIM_PRIVATE(diago.sstCSIM)->gui;
    FxU32 cmd = sstg->command;

    // first fetch the pattern offset and bias X,Y
    x = (x+((cmd & SSTG_X_PATOFFSET)>>SSTG_X_PATOFFSET_SHIFT)) & 7;
    y = (y+((cmd & SSTG_Y_PATOFFSET)>>SSTG_Y_PATOFFSET_SHIFT)) & 7;
    if (sstg->commandEx & SSTG_PAT_FORCE_ROW0) y = 0;

    // now test to see if the pattern is monochrome
    *visible = 1;
    if (cmd & SSTG_MONO_PATTERN) {
	FxU32 col = sstg->colorPattern[y < 4 ? 0 : 1];
	if (cmd & SSTG_TRANSPARENT)
		*visible = col & (1<<((7-x) + ((y&3)*8)));
	return (col & (1<<((7-x) + ((y&3)*8)))) ? sstg->colorFore : sstg->colorBack;
    }

    // else pattern is in the format of the destination
    switch(sstg->dstFormat & SSTG_DST_FORMAT) {
	case SSTG_PIXFMT_32BPP:
	    return sstg->colorPattern[y*8+x];

	case SSTG_PIXFMT_24BPP:		// UGLY!!!
	    y = (y*8+x)*3;			// get byte index
	    x = y % 4;				// which byte
	    y = y / 4;				// which word
	    if (x < 2)
		return 0xFFFFFF & (sstg->colorPattern[y]>>(x*8));
	    else
		return 0xFFFFFF & ((sstg->colorPattern[y]>>(x*8)) |
			(sstg->colorPattern[y+1]<<((4-x)*8)));

	case SSTG_PIXFMT_16BPP:
	case SSTG_PIXFMT_15BPP:
	    return 0xFFFF & (sstg->colorPattern[(y*8+x)/2] >> ((x&1)*16));

	case SSTG_PIXFMT_8BPP:
	    return 0xFF & (sstg->colorPattern[(y*8+x)/4] >> ((x&3)*8));
	default:
	    GDBG_ERROR("sstg_getpattern","invalid destination format\n");
    }

    return(0xdefaced);
}

#if COLORTRANSLUT
//----------------------------------------------------------------------
// set the color translation table to random values
//----------------------------------------------------------------------
void sstg_setclut_random(SstGRegs *sstg)
{
    FxU32 i,col;

    for (i=0; i<256; i++) {
	col = colRandom32();
	SET(sstg->colorTransLut[i],col);
    }
}
#endif

//----------------------------------------------------------------------
// return whether a pixel would be clipped or not: 1=>clipped
//----------------------------------------------------------------------
int sstg_clipped(int x, int y)
{
    FxU32 clipMin, clipMax;
    SstGRegs *sstg = &CSIM_PRIVATE(diago.sstCSIM)->gui;

    if (sstg->command & SSTG_CLIPSELECT) {	// first select clip regs
	clipMin = sstg->clip1min;
	clipMax = sstg->clip1max;
    }
    else {
	clipMin = sstg->clip0min;
	clipMax = sstg->clip0max;
    }

    return  x < (signed)LOWORD(clipMin) || !(x < (signed)LOWORD(clipMax)) ||
	    y < (signed)HIWORD(clipMin) || !(y < (signed)HIWORD(clipMax));
}

//----------------------------------------------------------------------
// mask a color down to the appropriate destination format size
//----------------------------------------------------------------------
FxU32 sstg_destination_mask(FxU32 color)
{
    static FxU32 _mask[] = {0x00, 0xFF, 0xFFFF, 0xFFFF, 0xFFFFFF, 0xFFFFFFFF, 0xFF,0xFF};
    SstGRegs *sstg = &CSIM_PRIVATE(diago.sstCSIM)->gui;	// get the real one

    return color & _mask[(sstg->dstFormat & SSTG_DST_FORMAT) >> SSTG_DST_FORMAT_SHIFT];
}

//----------------------------------------------------------------------
// draw a line, choose randomly between GO and LAUNCH modes
//----------------------------------------------------------------------
void sstg_drawline(SstGRegs *sstg, int x1,int y1, int x2,int y2, FxU32 cmd)
{
    if (iRandom(1)) {				// command+GO
	SET(sstg->srcXY,(y1<<16) | (x1 & 0xFFFF));
	SET(sstg->dstXY,(y2<<16) | (x2 & 0xFFFF));
	SET(sstg->command, cmd | SSTG_GO);
    }
    else {					// LAUNCH
	SET(sstg->srcXY,(y1<<16) | (x1 & 0xFFFF));
	SET(sstg->command, cmd);
	SET(sstg->launch[1],(y2<<16) | (x2 & 0xFFFF));
    }
}

//----------------------------------------------------------------------
// draw a rectangle, choose randomly between GO and LAUNCH modes
//----------------------------------------------------------------------
void sstg_drawrect(SstGRegs *sstg, int x, int y, int w, int h, FxU32 cmd)
{
    if (iRandom(1)) {				// command+GO
	SET(sstg->dstXY,(y<<16) | (x & 0xFFFF));
	SET(sstg->dstSize,(h<<16) | w);
	SET(sstg->command, cmd | SSTG_RECTFILL | SSTG_GO);
    }
    else {					// LAUNCH
	SET(sstg->dstSize,(h<<16) | w);
	SET(sstg->command, cmd | SSTG_RECTFILL);
	SET(sstg->launch[iRandom(31)],(y<<16) | (x & 0xFFFF));
    }
}

//----------------------------------------------------------------------
// draw a scr-to-scr blt, choose randomly between GO and LAUNCH modes
//----------------------------------------------------------------------
void sstg_drawblt(SstGRegs *sstg, int xs, int ys, int xd, int yd, int w, int h, FxU32 cmd, FxU32 *cmdXops)
{
    cmd &= ~(SSTG_XDIR | SSTG_YDIR);		// clear out the direction bits
    if (xs < xd) {				// compute new direction bits
	cmd |= SSTG_XDIR;			// right to left
	*cmdXops &= ~(SSTG_EN_SRC_COLORKEY_EX | SSTG_EN_DST_COLORKEY_EX);
	SET(sstg->commandEx, *cmdXops);
	xs += w-1;
	xd += w-1;
    }
    if (ys < yd) {
	cmd |= SSTG_YDIR;
	ys += h-1;
	yd += h-1;
    }

    if (iRandom(1)) {				// command+GO
	SET(sstg->srcXY,(ys<<16) | (xs & 0xFFFF));
	SET(sstg->dstXY,(yd<<16) | (xd & 0xFFFF));
	SET(sstg->dstSize,(h<<16) | w);
	SET(sstg->command, cmd | SSTG_BLT | SSTG_GO);
    }
    else {					// LAUNCH
	SET(sstg->command, cmd | SSTG_BLT);
	SET(sstg->dstSize,(h<<16) | w);
	SET(sstg->dstXY,(yd<<16) | (xd & 0xFFFF));
	SET(sstg->launch[0],(ys<<16) | (xs & 0xFFFF));
    }
}

//----------------------------------------------------------------------
// draw a scr-to-scr blt, choose randomly between GO and LAUNCH modes,
// let caller choose XDIR and YDIR
//----------------------------------------------------------------------
void sstg_draw_R_blt(SstGRegs *sstg, int xs, int ys, int xd, int yd, int w, int h, FxU32 cmd)
{
    if ((w * h) != 0)
    {
	if (cmd & SSTG_XDIR)
	{
	    xs += w-1;
	    xd += w-1;
	}

	if (cmd & SSTG_YDIR)
	{
	    ys += h-1;
	    yd += h-1;
	}
    }

    if (iRandom(1)) {				// command+GO
	SET(sstg->srcXY,(ys<<16) | (xs & 0xFFFF));
	SET(sstg->dstXY,(yd<<16) | (xd & 0xFFFF));
	SET(sstg->dstSize,(h<<16) | w);
	SET(sstg->command, cmd | SSTG_BLT | SSTG_GO);
    }
    else {					// LAUNCH
	SET(sstg->command, cmd | SSTG_BLT);
	SET(sstg->dstSize,(h<<16) | w);
	SET(sstg->dstXY,(yd<<16) | (xd & 0xFFFF));
	SET(sstg->launch[0],(ys<<16) | (xs & 0xFFFF));
    }
}


//----------------------------------------------------------------------
// draw a scr-to-scr STRETCH blt, choose randomly between GO and LAUNCH modes,
// let caller choose XDIR and YDIR
//----------------------------------------------------------------------
void sstg_draw_S_blt(SstGRegs *sstg, int xs, int ys, int sw, int sh,
			int xd, int yd, int dw, int dh, FxU32 cmd)
{

    if (iRandom(1)) {				// command+GO
	SET(sstg->srcXY,(ys<<16) | (xs & 0xFFFF));
	SET(sstg->dstXY,(yd<<16) | (xd & 0xFFFF));
	SET(sstg->srcSize,(sh<<16) | sw);
	SET(sstg->dstSize,(dh<<16) | dw);
	SET(sstg->command, cmd | SSTG_STRETCH_BLT | SSTG_GO);
    }
    else {					// LAUNCH
	SET(sstg->command, cmd | SSTG_STRETCH_BLT);
	SET(sstg->srcSize,(sh<<16) | sw);
	SET(sstg->dstSize,(dh<<16) | dw);
	SET(sstg->dstXY,(yd<<16) | (xd & 0xFFFF));
	SET(sstg->launch[0],(ys<<16) | (xs & 0xFFFF));
    }
}


//----------------------------------------------------------------------
// check one pixel at (xc,yc) given a bunch of chip state
// this routine simulates most of the pixel pipe
// it returns the final color of the pixel
//----------------------------------------------------------------------
FxU32 sstg_check_pixel(long xc, long yc, FxU32 cmdops, FxU32 cmdXops,
			FxU32 rop, FxU32 csrc, FxU32 cdest, int srcKey)
{
    int vis;
    FxU32 cpat, temp;
    FxU32 originalCDest;

    if (!diago.checkEveryTriangle)
	return 0;
    
    originalCDest = cdest;
    cpat = sstg_getpattern(xc,yc,&vis);		// get the pattern output
    if (vis && !sstg_clipped(xc,yc)) {		// check visibility 
	temp = sstg_pick_rop(cmdops, cmdXops, rop, csrc,cdest,srcKey);
	cdest = sstg_rop3(temp,cpat,csrc,cdest);
    }
    cdest = sstg_destination_mask(cdest);

    //Preserve the MSB if supporting 1555
    if(cmdXops & SSTG_PRESERVE_MSB)
      {
	cdest = (cdest & 0x7FFF) | (originalCDest & 0x8000);
      }

    gdbg_info(8,"checking %d,%d 0x%x\n",xc,yc,cdest);
    DIAG_TEST_PIXEL(CSIM_BUF_2D_DST,xc,yc,cdest); // test the pixel
    return cdest;
}

void sstg_bres_setup(SstGRegs *sstg, DiagBresEdge *pe, int x1, int y1, int x2, int y2)
{
    int adx,ady, dx,dy;

    pe->cmd = sstg->command;

    // setup the line, compute deltas and sort it
    adx = dx = x2 - x1;
    ady = dy = y2 - y1;
    pe->dx = dx;
    pe->dy = dy;
    if (adx < 0) adx = -adx;
    if (ady < 0) ady = -ady;

    if (adx >= ady) {			// Xmajor
	pe->err = 2*ady - adx;
	pe->einc1 = 2*ady;
	pe->einc2 = 2*(ady-adx);
	pe->xinc1 = ISIGN(dx);
	pe->xinc2 = pe->xinc1;
	pe->yinc1 = 0;
	pe->yinc2 = ISIGN(dy);
    }
    else {				// Ymajor
	pe->err = 2*adx - ady;
	pe->einc1 = 2*adx;
	pe->einc2 = 2*(adx-ady);
	pe->xinc1 = 0;
	pe->xinc2 = ISIGN(dx);
	pe->yinc1 = ISIGN(dy);
	pe->yinc2 = pe->yinc1;
    }
    pe->x = x1;
    pe->y = y1;

    // if we are setting up a line, then apply bresError0 and reversible options
    if ((pe->cmd & SSTG_COMMAND) != SSTG_POLYFILL) {
	if (sstg->bresError0 & 0x80000000)
	    pe->err = SIGN_EXTEND(sstg->bresError0,16);
    }
}

void sstg_bres_iterate(DiagBresEdge *pe)
{
    int test;

    if ((pe->cmd & SSTG_REVERSIBLE) &&
	(((pe->dx > 0) && ((pe->dy>0) || pe->yinc1)) || 
	((pe->dx < 0) && ((pe->dy>0) && pe->xinc1))) )
	test = pe->err <= 0;
    else
	test = pe->err < 0;

    if (test) {				// iterate the line
	pe->x += pe->xinc1;		// major axis
	pe->y += pe->yinc1;
	pe->err += pe->einc1;
    }
    else {				// major and minor axis
	pe->x += pe->xinc2;
	pe->y += pe->yinc2;
	pe->err += pe->einc2;
    }
}

//----------------------------------------------------------------------
// check pixels along a line that was drawn against a background of 0's
// returns the new lineStyle register
//----------------------------------------------------------------------
FxU32 
sstg_checkline(SstGRegs *sstg, int x1,int y1,int x2,int y2, FxU32 cmd, FxU32 cmdX,
		FxU32 cfore, FxU32 cback, FxU32 style, int neighbors)
{
    int run,xmajor;
    DiagBresEdge line;
    FxU32 solid, monotrans, repeat, ipos, fpos, lssize;

    sstg = &CSIM_PRIVATE(diago.sstCSIM)->gui;
    if (diago.checkEveryTriangle || !diago.writeFifo)
    if ((sstg->command & ~SSTG_GO) != cmd)
	GDBG_ERROR("sstg_checkline", "cmd 0x%x != chip's command register 0x%x\n",
			cmd, sstg->command);

    solid = ! (sstg->command & SSTG_EN_LINESTIPPLE);
    if (!solid) {
	monotrans = sstg->command & SSTG_TRANSPARENT;
	repeat = (style & SSTG_LSREPEAT)>>SSTG_LSREPEAT_SHIFT;
	ipos = (style & SSTG_LSPOS_INT)>>SSTG_LSPOS_INT_SHIFT;
	fpos = (style & SSTG_LSPOS_FRAC)>>SSTG_LSPOS_FRAC_SHIFT;
	lssize = (style & SSTG_LSSIZE)>>SSTG_LSSIZE_SHIFT;
	ipos = ipos % (lssize+1);
    }

    cfore = sstg_destination_mask(cfore);
    sstg_bres_setup(&CSIM_PRIVATE(diago.sstCSIM)->gui, &line, x1,y1,x2,y2);
    xmajor = line.yinc1==0;
    run = 1;				// now iterate the line
    while (run) {
	gdbg_info(6,"----test line pixel(%d,%d)\n",line.x,line.y);

	// check for done, if POLYLINE then skip the last pixel
	if (xmajor ? line.x==x2 : line.y==y2) {
	    run = 0;
	    if ((cmd & SSTG_COMMAND) == SSTG_POLYLINE) continue;
	}

	// if pixel is clipped out, check for 0, else normal check
	if (sstg_clipped(line.x,line.y))
	    DIAG_TEST_PIXEL(CSIM_BUF_2D_DST,line.x,line.y, 0);
	else if (solid)
	    sstg_check_pixel(line.x,line.y,cmd,cmdX, sstg->rop,cfore,0,0);
	else {
	    if (sstg->lineStipple & (1<<ipos))	// stipple is '1'
		sstg_check_pixel(line.x,line.y,cmd,cmdX, sstg->rop,cfore,0,0);
	    else if (monotrans)	{		// stipple is '0' => transparent
		gdbg_info(8,"checking %d,%d 0x%x\n",line.x,line.y,0);
		DIAG_TEST_PIXEL(CSIM_BUF_2D_DST, line.x,line.y, 0);
	    }
	    else			// opaque with background
		sstg_check_pixel(line.x,line.y,cmd,cmdX, sstg->rop,cback,0,0);
	}
	if (!solid) {
	    if (fpos == repeat) {	// if equal to repeat
		fpos = 0;
		ipos++;			// then increment integer pos
		if (ipos > lssize)
		    ipos = 0;		// modulo lineStipple size
	    }
	    else fpos++;		// increment fractional position
	}

	// test surrounding pixels if requested and color is not zero
	if (neighbors && cfore != 0) {
	    if (xmajor) {			// XMAJOR
		DIAG_TEST_PIXEL(CSIM_BUF_2D_DST,line.x,line.y-1,0);
		DIAG_TEST_PIXEL(CSIM_BUF_2D_DST,line.x,line.y+1,0);
	    }
	    else {				// YMAJOR
		DIAG_TEST_PIXEL(CSIM_BUF_2D_DST,line.x-1,line.y,0);
		DIAG_TEST_PIXEL(CSIM_BUF_2D_DST,line.x+1,line.y,0);
	    }
	}

	sstg_bres_iterate(&line);
    }
    if (!solid) {			// return data to register
	style &= ~(SSTG_LSPOS_INT | SSTG_LSPOS_FRAC);
	style |= fpos << SSTG_LSPOS_FRAC_SHIFT;
	style |= ipos << SSTG_LSPOS_INT_SHIFT;
    }

    // check that POLYLINEs skip the last pixel, cliprect doesn't matter 
    if ((cmd & SSTG_COMMAND) == SSTG_POLYLINE && cfore != 0 && neighbors)
	DIAG_TEST_PIXEL(CSIM_BUF_2D_DST,x2,y2,0);
    return style;
}


void sstg_random_poly(Polygon *poly, long xd, long yd,
		      FxBool firstPoint)	// use xd, yd as first point?
{
    int n, lei,rei;

    poly->spanArraysValidFlag = 0;

    // generate 1st vertex
    if (!firstPoint)
	xyRandom(&poly->leftEdges[0].x,&poly->leftEdges[0].y);
    else
    {
	poly->leftEdges[0].x = xd;
	poly->leftEdges[0].y = yd;
    }

    do
      {
	poly->xl = poly->xr = poly->leftEdges[0].x;
	poly->yb = poly->yt = poly->leftEdges[0].y;

    // generate 2nd vertex
	if (iRandom(4)==0) {			// 1/4 of time generate a flat top
	  poly->rightEdges[0].x = poly->leftEdges[0].x + iRandom(diago.tsize);
	  poly->rightEdges[0].y = poly->leftEdges[0].y;
	  // generate 3rd vertex
	  poly->leftEdges[1].x = poly->leftEdges[0].x + rRandom(-diago.tsize,diago.tsize);
	  poly->leftEdges[1].y = poly->leftEdges[0].y + iRandom(diago.tsize);
	}
	else {
	  poly->rightEdges[0].x = poly->leftEdges[0].x + rRandom(-diago.tsize,diago.tsize);
	  poly->rightEdges[0].y = poly->leftEdges[0].y + iRandom(diago.tsize);
	  // generate 3rd vertex
	  do {
	    poly->leftEdges[1].x = poly->leftEdges[0].x + rRandom(-diago.tsize,diago.tsize);
	    poly->leftEdges[1].y = poly->leftEdges[0].y + iRandom(diago.tsize);
	  } while (poly->leftEdges[1].y == poly->leftEdges[0].y);
	}
	lei = 1;
	rei = 0;
	// keep the bbox up to date
	if (poly->xl > poly->leftEdges[lei].x)
	  poly->xl = poly->leftEdges[lei].x;
	if (poly->xr < poly->leftEdges[lei].x)
	  poly->xr = poly->leftEdges[lei].x;
	if (poly->xl > poly->rightEdges[rei].x)
	  poly->xl = poly->rightEdges[rei].x;
	if (poly->xr < poly->rightEdges[rei].x)
	  poly->xr = poly->rightEdges[rei].x;
	poly->yt = poly->leftEdges[1].y;

	for (n=3; n<poly->numVerts; n++) {
	  // gen a new left edge
	  if (poly->leftEdges[lei].y <= poly->rightEdges[rei].y) {
	    poly->leftEdges[lei+1].x = poly->leftEdges[lei].x + rRandom(-diago.tsize,diago.tsize);
	    poly->leftEdges[lei+1].y = poly->leftEdges[lei].y + iRandom(diago.tsize);
	    if (iRandom(4)==0)			// 1/4 of time generate a flat edge
	      poly->leftEdges[lei+1].y = poly->leftEdges[lei].y;
		
	    lei++;
	    if (poly->xl > poly->leftEdges[lei].x)
	      poly->xl = poly->leftEdges[lei].x;
	    if (poly->xr < poly->leftEdges[lei].x)
	      poly->xr = poly->leftEdges[lei].x;
	  }
	  else {	// gen a new right edge
	    poly->rightEdges[rei+1].x = poly->rightEdges[rei].x + rRandom(-diago.tsize,diago.tsize);
	    poly->rightEdges[rei+1].y = poly->rightEdges[rei].y + iRandom(diago.tsize);
	    if (iRandom(4)==0)			// 1/4 of time generate a flat edge
	      poly->rightEdges[rei+1].y = poly->rightEdges[rei].y;
	    rei++;
	    if (poly->xl > poly->rightEdges[rei].x)
	      poly->xl = poly->rightEdges[rei].x;
	    if (poly->xr < poly->rightEdges[rei].x)
	      poly->xr = poly->rightEdges[rei].x;
	  }
	}
	// keep the bbox up to date
	if (poly->yt < poly->leftEdges[lei].y)
	  poly->yt = poly->leftEdges[lei].y;
	if (poly->yt < poly->rightEdges[rei].y)
	  poly->yt = poly->rightEdges[rei].y;
	poly->nLeft = lei+1;
	poly->nRight = rei+1;
      } while(!sstg_is_valid_polygon(poly));
}

//This function makes sure that a polygon doesn't having
//crossing edges and left edges that are on the right side.
FxBool sstg_is_valid_polygon(Polygon *poly)
{
  int leftIndex, rightIndex;

  //Variables for edge vectors
  int leftDX, leftDY;   //Vector from left Vertex n to left Vertex n+1
  int rightDX, rightDY; //Vector from left Vertex n to right Vertex m

  sstg_print_poly(poly);

  GDBG_INFO(187, "Left side\n");
  for(leftIndex=0;leftIndex < poly->nLeft; leftIndex++)
    {
      GDBG_INFO(187, "poly->leftEdges[%d]=(%d,%d)\n", leftIndex, 
		poly->leftEdges[leftIndex].x,
		poly->leftEdges[leftIndex].y);
    }

  GDBG_INFO(187, "Right side\n");
  for(rightIndex=0;rightIndex < poly->nRight; rightIndex++)
    {
      GDBG_INFO(187, "poly->rightEdges[%d]=(%d,%d)\n", rightIndex, 
		poly->rightEdges[rightIndex].x,
		poly->rightEdges[rightIndex].y);
    }

  //Go through all left edges and make sure "right" vertices are
  //actually on the right side of the edge
  for(leftIndex=0; leftIndex < poly->nLeft-1; leftIndex++)
    {
      //Be sloppy and check all right vertices
      for(rightIndex=0; rightIndex < poly->nRight; rightIndex++)
	{
	  //Only compare the right vertex against the left edges
	  //that overlap the right vertex in the y coordinate
	  if(poly->leftEdges[leftIndex].y   > poly->rightEdges[rightIndex].y ||
	     poly->leftEdges[leftIndex+1].y < poly->rightEdges[rightIndex].y)
	    continue;
	  
	  leftDX = poly->leftEdges[leftIndex+1].x - poly->leftEdges[leftIndex].x;
	  leftDY = poly->leftEdges[leftIndex+1].y - poly->leftEdges[leftIndex].y;
	  
	  //Horizontal left edge case
	  assert(leftDY >= 0);
	  if(leftDY == 0)
	    {
	      if(poly->leftEdges[leftIndex].x   >= poly->rightEdges[rightIndex].x ||
		 poly->leftEdges[leftIndex+1].x >= poly->rightEdges[rightIndex].x)
		{
		  return(FXFALSE);
		}
	    }

	  rightDX = poly->rightEdges[rightIndex].x - poly->leftEdges[leftIndex].x;
	  rightDY = poly->rightEdges[rightIndex].y - poly->leftEdges[leftIndex].y;
	  
	  //Check to see that right vertex is on the right side
	  //This test is the result of taking the cross product of left and right
	  if((leftDX*rightDY) > (leftDY*rightDX))
	    {
	      return(FXFALSE);	  
	    }
	}
    }

  //Make sure extremum of polygon are ok
  for(rightIndex=0; rightIndex < poly->nRight; rightIndex++)
    {
      if(poly->rightEdges[rightIndex].y < poly->leftEdges[0].y &&
	 poly->rightEdges[rightIndex].x <= poly->leftEdges[0].x)
	{
	  return(FXFALSE);
	}

      if(poly->rightEdges[rightIndex].y > poly->leftEdges[poly->nLeft-1].y &&
	 poly->rightEdges[rightIndex].x <= poly->leftEdges[poly->nLeft-1].x)
	{
	  return(FXFALSE);	
	}
    }

  
  return(FXTRUE);
}

void sstg_print_poly(Polygon *poly)
{
    int n;
    int lei, rei;		// left and right edge indicies
    Edge *ple,*pre;		// left and right edge pointers

    gdbg_info(2,"\n");
    gdbg_info(2,"random poly, %d vertices (%d,%d), bbox= %d,%d to %d,%d\n",
		poly->numVerts, poly->nLeft, poly->nRight,
		poly->xl,poly->yb, poly->xr,poly->yt);
    gdbg_info(3,"     LEFT      RIGHT\n");
    gdbg_info(3,"---------  ---------\n");

    gdbg_info(3,"%4d,%4d\n", poly->leftEdges[0].x,poly->leftEdges[0].y);
    lei = 0;
    ple = poly->leftEdges;
    if (ple->y == poly->rightEdges[0].y) {
	rei = 0;
	pre = &poly->rightEdges[rei];
	gdbg_info(3,"           %4d,%4d %s\n",
			pre->x,pre->y,
			ple->y==pre->y? "flat" : "");
    }
    else {
	rei = -1;
	pre = ple;
    }

    for (n=rei+2; n<poly->numVerts; n++) {
	if (ple->y <= pre->y) {
	    ple = &poly->leftEdges[++lei];
	    gdbg_info(3,"%4d,%4d            %s\n",
			ple->x,ple->y,
			ple->y==poly->leftEdges[lei-1].y? "flat" : "");
	}
	else {
	    pre = &poly->rightEdges[++rei];
	    gdbg_info(3,"           %4d,%4d %s\n",
			pre->x,pre->y,
			pre->y==poly->rightEdges[rei-1].y? "flat" : "");
	}
    }
}

void sstg_draw_poly(SstGRegs *sstg, Polygon *poly, FxU32 cmdops,
		    FxBool reversible)
{
    int n, ilaunch;
    int lei, rei;		// left and right edge indicies
    Edge *ple,*pre;		// left and right edge pointers

    poly->cmd = cmdops | SSTG_POLYFILL;
    
    if (reversible && iRandom(1))
	poly->cmd |= SSTG_REVERSIBLE;

    ilaunch = 0;
    lei = 0;
    ple = poly->leftEdges;
    if (ple->y == poly->rightEdges[0].y) {		// flat top
	rei = 0;
	pre = &poly->rightEdges[rei];
	SET(sstg->command, poly->cmd);
	SET(sstg->srcXY,(ple->y<<16) | (ple->x & 0xFFFF));// write srcXY,dstXY
	SET(sstg->dstXY,(pre->y<<16) | (pre->x & 0xFFFF));
    }
    else {
	rei = -1;
	pre = ple;
	SET(sstg->srcXY,(ple->y<<16) | (ple->x & 0xFFFF));
	SET(sstg->command, poly->cmd | SSTG_GO);
    }

    for (n=rei+2; n<poly->numVerts; n++) {
	if (ple->y <= pre->y) {
	    ple = &poly->leftEdges[++lei];
	    SET(sstg->launch[ilaunch++],(ple->y<<16) | (ple->x & 0xFFFF));
	}
	else {
	    pre = &poly->rightEdges[++rei];
	    SET(sstg->launch[ilaunch++],(pre->y<<16) | (pre->x & 0xFFFF));
	}
	ilaunch &= 31;
    }
    // if the bottom is not flat, repeat the last vertex
    if (ple->y != pre->y) {
	if (ple->y <= pre->y) {
	    SET(sstg->launch[ilaunch++],(pre->y<<16) | (pre->x & 0xFFFF));
	}
	else {
	    SET(sstg->launch[ilaunch++],(ple->y<<16) | (ple->x & 0xFFFF));
	}
    }
}

// rasterize a polygon edge into spanArray, store the minimum X for each span
//
// NOTE: any part of the polygon which is off the screen is ignored
//
static void sstg_rasterize_poly_edge(Edge *p1, Edge *p2, int spanArray[])
{
    DiagBresEdge line;

    GDBG_INFO(9, "\tsstg_rasterize_poly_edge: (%d,%d) -> (%d,%d)\n",
	      p1->x, p1->y, p2->x, p2->y);

    sstg_bres_setup(&CSIM_PRIVATE(diago.sstCSIM)->gui,
			&line, p1->x,p1->y,p2->x,p2->y);
    while (1) {
	if (line.y == p2->y)
	    break;
	if (line.yinc1==0 && line.x==p2->x)
	    break;
	if ( ONSCREEN(line.x,line.y) && line.x < spanArray[line.y] ) {
	    spanArray[line.y] = line.x;
	    gdbg_info(9,"\tspan[%d] = %d\n",line.y,line.x);
	}
	else
	  {
	    if(line.x < spanArray[line.y])
	      {
		if(line.x <= 0)
		  {
		    spanArray[line.y]=0;
		    gdbg_info(9,"\t<0 span[%d] = 0\n",line.y);
		  }
		else if(line.x >= diago.xmaxscreen)
		  {
		    spanArray[line.y]=diago.xmaxscreen;
		    gdbg_info(9,"\t>= max span[%d] = %d\n",line.y, diago.xmaxscreen);
		  }
	      }
	  }

	sstg_bres_iterate(&line);
    }
}

// go thru a polygon's edges and mark the left and right extents of the
// polygon for each span, this is done by rasterizing the lines for the edges
// and storing the results in poly->spanLeftCheck and spanRightCheck
//
// NOTE: any part of the polygon which is off the screen is ignored
//
static void sstg_recheck_poly(Polygon *poly)
{
    int n;
    Edge *p1,*p2;

    // first reset the span arrays
    for (n=0; n<MAX_SPANS; n++) {
	    poly->spanLeftCheck[n] = 0xFFFFFF;
	    poly->spanRightCheck[n] = 0xFFFFFF;
    }

    // now rasterize the left edges
    gdbg_info(8,"rasterizing left edge\n");
    for (n=1; n<poly->nLeft; n++) {
	p1 = &poly->leftEdges[n-1];
	p2 = &poly->leftEdges[n];
	sstg_rasterize_poly_edge(p1,p2,poly->spanLeftCheck);
    }
    p1 = &poly->leftEdges[poly->nLeft-1];
    p2 = &poly->rightEdges[poly->nRight-1];
    if (p1->y < p2->y)
	sstg_rasterize_poly_edge(p1,p2,poly->spanLeftCheck);

    // now rasterize the right edges
    gdbg_info(8,"rasterizing right edge\n");
    p1 = &poly->leftEdges[0];
    p2 = &poly->rightEdges[0];
    if (p1->y < p2->y)
	sstg_rasterize_poly_edge(p1,p2,poly->spanRightCheck);

    for (n=1; n<poly->nRight; n++) {
	p1 = &poly->rightEdges[n-1];
	p2 = &poly->rightEdges[n];
	sstg_rasterize_poly_edge(p1,p2,poly->spanRightCheck);
    }
    p1 = &poly->rightEdges[poly->nRight-1];
    p2 = &poly->leftEdges[poly->nLeft-1];
    if (p1->y < p2->y)
	sstg_rasterize_poly_edge(p1,p2,poly->spanRightCheck);
    poly->spanArraysValidFlag = 1;
}

// return 1 if (x,y) is inside a polygon, 0 if it's outside
// this is really easy once we have the check arrays
int sstg_inside_poly(Polygon *poly, int x, int y)
{
    if ( ! ONSCREEN(x,y) ) {   
      // span arrays only consider onscreen portion of polygon
      // offscreen coordinates are not supported
      GDBG_ERROR("sstg_inside_poly","point (%d,%d) is OFF the screen\n",x,y);
      return 0;
    }

    if (!poly->spanArraysValidFlag) {		// if checkArrays are invalid
	sstg_recheck_poly(poly);		// then rebuild them
    }

    return (poly->spanLeftCheck[y] <= x) && (x < poly->spanRightCheck[y]);
}


#define SWAP(pointA, pointB) \
{\
    FxI32 tmp;\
    tmp = pointA##x; pointA##x = pointB##x; pointB##x = tmp; \
    tmp = pointA##y; pointA##y = pointB##y; pointB##y = tmp; \
}

FxU32 sstg_area(FxI32 p1x, FxI32 p1y,
		FxI32 p2x, FxI32 p2y,
		FxI32 p3x, FxI32 p3y)
{
    FxI32 crossp = ((p2x-p1x)*(p3y-p1y)) - ((p2y-p1y)*(p3x-p1x));
    return abs(crossp) / 2;
}


//
// returns 1 if the 3 points are colinear, 0 if they're not
//
int 
sstg_colinear(FxI32 p1x, FxI32 p1y,
	      FxI32 p2x, FxI32 p2y,
	      FxI32 p3x, FxI32 p3y)
{
    double m, b, a, c, d;
    
    if ((p1x == p2x) && (p2x == p3x))
	return 1;
    
    // find line equation for p1p2, y=mx+b
    if(p2x != p1x)
      m = (double)(p2y - p1y) / (double)(p2x - p1x);
    else
      m = (double)(p2y - p1y) * 1e9;

    b = (double)p1y - (m * p1x);

    // find a, b, c in the form ax + by + c = 0
    a = m;
    c = b;
    b = -1.0;

    // apply distance formula from point to line
    if(a*a + b*b > 0)
      d = fabs((a*p3x) + (b*p3y) + c) / sqrt(a*a + b*b);
    else
      d = fabs((a*p3x) + (b*p3y) + c) * 1e9;

    if (d <= 2.0)
	return 1;

    return 0;
}

void sstg_draw_triangle(SstGRegs * sstg,
			FxI32 p1x, FxI32 p1y,
			FxI32 p2x, FxI32 p2y,
			FxI32 p3x, FxI32 p3y,
			FxU32 cmdops)
{
    FxI32 crossp;

    gdbg_info(3, "triang: (%d,%d), (%d,%d), (%d,%d)\n",
	      p1x, p1y, p2x, p2y, p3x, p3y);
    
    // sort
    if (p1y > p2y)
	SWAP(p1, p2);
    if (p1y > p3y)
	SWAP(p1, p3);
    if (p2y > p3y)
	SWAP(p2, p3);
    
    // flat top
    if ((p1y == p2y) && (p1x > p2x))
    {
	SWAP(p1, p2);
    }
    else
    {
	// make p2 be the left edge

	if ((p2x <= p1x) && (p3x > p2x))
	    ;//p2 is left edge
	else if ((p2x >= p1x) && (p3x < p2x))
	{
	    // p3 is left edge
	    SWAP(p2,p3);
	    // now p2 is left edge
	}
	else
	{
	    // the sign of the cross product of p1p2 and p1p3 tells us
	    // the "direction" of the turn from p1p2 to p2p3
	    crossp = ((p2x-p1x)*(p3y-p1y) - (p2y-p1y)*(p3x-p1x));
	    if (crossp > 0)
		SWAP(p2, p3);
	}
    }
    
    p1x &= 0xFFFF;
    p2x &= 0xFFFF;
    p3x &= 0xFFFF;

    SET(sstg->srcXY, (p1y << 16) | p1x);
    
    if (p1y == p2y)
    {
	// flat top
	SET(sstg->dstXY, (p2y << 16) | p2x);
	SET(sstg->command, SSTG_POLYFILL | cmdops);
	SET(sstg->launch[iRandom(31)], (p3y << 16) | p3x);
	SET(sstg->launch[iRandom(31)], (p3y << 16) | p3x);
    }
    else
    {
	SET(sstg->command, SSTG_POLYFILL | SSTG_GO | cmdops);
	SET(sstg->launch[iRandom(31)], (p2y << 16) | p2x);
	SET(sstg->launch[iRandom(31)], (p3y << 16) | p3x);
	if (p2y > p3y)
	    SET(sstg->launch[iRandom(31)], (p2y << 16) | p2x);
	else if (p2y < p3y)
	    SET(sstg->launch[iRandom(31)], (p3y << 16) | p3x);
    }
}

    
void
sstg_draw_poly_rectangle(SstGRegs *sstg, FxI32 baseX, FxI32 baseY,
		    FxI32 width, FxI32 height, FxU32 cmdops)
{
    FxI32 middleX;
    FxI32 middleY;

    if ((width < 3) || (height < 3))
    {
	GDBG_ERROR("sstg_draw_poly_rectangle", "width and height must be > 3\n");
    }

    middleX = rRandom(baseX + 1, baseX + width - 2);
    middleY = rRandom(baseY + 1, baseY + height - 2);

    gdbg_info(2, "sstg_draw_poly_rectangle: base:(%d,%d), w/h:(%d,%d)\n",
	      baseX, baseY, width, height);
    gdbg_info(2, "\t\tmiddle point: (%d,%d)\n", middleX, middleY);
    
    sstg_draw_triangle(sstg,
		       baseX,		  baseY,
		       baseX + width - 1, baseY,
		       middleX		, middleY,
		       cmdops);
    sstg_draw_triangle(sstg,
		       baseX + width - 1, baseY,
		       middleX		, middleY,
		       baseX + width - 1, baseY + height - 1,
		       cmdops);
    sstg_draw_triangle(sstg,
		       middleX		, middleY,
		       baseX + width - 1, baseY + height - 1,
		       baseX		, baseY + height - 1,
		       cmdops);
    sstg_draw_triangle(sstg,
		       middleX		, middleY,
		       baseX		, baseY,
		       baseX		, baseY + height - 1,
		       cmdops);
}

static FxU32 swap(FxU32 srcFormat, FxU32 data)
{
    if (srcFormat & SSTG_HOST_BYTE_SWIZZLE) {
	data = (data>>24) | ((data>>8)&0xFF00) | ((data<<8)&0xFF0000) | (data<<24);
    }
    if (srcFormat & SSTG_HOST_WORD_SWIZZLE) {
	data = (data>>16) | (data<<16);
    }
    return data;
}

// compute and return a byte address for a pixel
FxU32 sstg_compute_blit_address(FxU32 srcFormat, FxU32 base, int x, int y, int xsize)
{
    int align, stride;

    // packing field: 	0 => stride = bits(14:0)
    //			else stride = src_width*bpp rounded up to packing
    switch(srcFormat & SSTG_SRC_PACK) {
	case SSTG_SRC_PACK_SRC:	align = -1;
				stride = (srcFormat & SSTG_SRC_LINEAR_STRIDE) >> SSTG_SRC_STRIDE_SHIFT;
				break;
	// XXX for stretch blit, source width is NOT dest. size but is srcSize!!!
	case SSTG_SRC_PACK_8:	align = 0;
				break;
	case SSTG_SRC_PACK_16:	align = 1;
				break;
	case SSTG_SRC_PACK_32:	align = 3;
				break;
    }
    // stride = (source width * bpp)/8 rounded up to bytes
    switch(srcFormat & SSTG_SRC_FORMAT) {
	case SSTG_PIXFMT_1BPP: 	if (align >= 0)
				    stride = (((xsize+7)>>3) + align) & ~align;
				return y*stride + (x>>3);
//	case SSTG_PIXFMT_4BPP: 	if (align >= 0)
//				    stride = (((xsize+1)>>1) + align) & ~align;
//				return y*stride + (x>>1);
	case SSTG_PIXFMT_8BPP: 	if (align >= 0)
				    stride = (xsize + align) & ~align;
				return y*stride + x;
	case SSTG_PIXFMT_15BPP:
	case SSTG_PIXFMT_16BPP:	if (align >= 0) {
				    if (align == 0)
					GDBG_ERROR("csimComputeAddress.16",
						"invalid packing field: 8\n");
				    stride = (xsize*2 + align) & ~align;
				}
				else if (stride & 1)
				    GDBG_ERROR("sstg_compute_blit_address.16",
						"stride is not multiple of 2 bytes\n");
				return y*stride + x*2;
	case SSTG_PIXFMT_422YUV:
	case SSTG_PIXFMT_422UYV:if (align >= 0) {
				    if (align < 3)
					GDBG_ERROR("sstg_compute_blit_address.YUV",
						"invalid packing field: 8 or 16\n");
				    stride = (xsize*2 + align) & ~align;
				}
				else if (stride & 3)
				    GDBG_ERROR("sstg_compute_blit_address.YUV",
						"stride is not multiple of 4 bytes\n");
				return y*stride + x*2;
				
				

	case SSTG_PIXFMT_24BPP:	if (align >= 0)
				    stride = (xsize*3 + align) & ~align;
				return y*stride + x*3;
	case SSTG_PIXFMT_32BPP:	if (align >= 0) {
				    if (align < 3)
					GDBG_ERROR("sstg_compute_blit_address.32",
						"invalid packing field: 8 or 16\n");
				    stride = (xsize*4 + align) & ~align;
				}
				else if (stride & 3)
				    GDBG_ERROR("sstg_compute_blit_address.32",
						"stride is not multiple of 4 bytes\n");
				return y*stride + x*4;
    }

    return(0xDEADFAD);
}



//
// unpack a packed YUYV word into a YUV word, with Y0 (x == 0) or Y1 (x == 1)
// 
FxU32 sstg_unpack_yuv(FxU32 data, FxU32 x)
{
    FxU32 y, u, v;
    
    y = (data >> ((x & 1) ? 16 : 0)) & 0xFF;
    u = (data >> 8) & 0xFF;
    v = (data >> 24) & 0xFF;
    return (y << 16) | (u << 8) | v;
}

//
// unpack a packed UYVY word into a YUV word, with Y0 (x == 0) or Y1 (x == 1)
// 
FxU32 sstg_unpack_uyv(FxU32 data, FxU32 x)
{
    FxU32 y, u, v;
    
    y = (data >> ((x & 1) ? 24 : 8)) & 0xFF;
    u = (data >> 0) & 0xFF;
    v = (data >> 16) & 0xFF;
    return (y << 16) | (u << 8) | v;
}




// new way

// this routine assumes pack:0 (manual stride rounded to dword)
// (KMW: note, stride is not rounded to dword, but is truncated to
// pixel format size)
// GMT: I know this routine could probably be combined with the other one below,
// but in this case I think it's good to have two very different code segments
// sending blit data.  This routine is organized around a main per-row loop.
// the other routine is organized around a pixel counting loop.
void sendBltData0(SstGRegs *sstg, FxU32 byte, FxU32 bit,
		  int xd, int yd, int w, int h, int ydir, FxU32 srcFormat,
		  FxU32 host_pixels[])
{
    int yc, n=0;
    FxU32 addr;
    FxU32 stride, next_addr;
    FxI32 yEnd, yInc;

    stride = srcFormat & SSTG_SRC_LINEAR_STRIDE;
    next_addr = byte;

    if (!ydir)
    {
	// positive y direction
	yEnd = yd+h;
	yInc = 1;
    }
    else
    {
	// negative y direction
	yEnd = yd-h;
	yInc = -1;
    }
    

    // for each row of the blit
    for (yc = yd;
	 (ydir == 0) ? yc < yd+h : yc > yd-h;
	 yc += yInc)
    {
	int i,b,x;
	FxU32 col;

	addr = next_addr;
	next_addr = (addr + (stride % 4)) % 4;

	switch(srcFormat & SSTG_SRC_FORMAT)
	{
	  case SSTG_PIXFMT_1BPP:
	      if ((addr & 3) || (bit & 7))
	      {
		  col = iRandom(0xFFFFFFFF);
		  SET(sstg->launch[0], col);
		  col = swap(srcFormat, col);
		  b = (addr & 3) * 8 + (bit & 7);  // compute starting bit 
		  i = 32-b;			// compute pixels to produce
		  if (w < i)
		      i = w;
		  gdbg_info(7,"skipping %d+%d=%d bits\n",(addr&3)*8,
			    bit & 7, b);
		  for (x=0; x<i; x++)
		  {
		      if (ONSCREEN(xd+x,yc))
		      {
			  host_pixels[n++] = ((col>>(b&~7)) >>
					      (7 - b & 7)) & 1;
			  gdbg_info(7,"host_pixels[%d] = 0x%x\n",
				    n-1,host_pixels[n-1]);
		      }
		      b++;
		  }
	      }
	      else
		  x = 0;
	      for (x=x; x < w; x+=32)
	      {
		  col = iRandom(0xFFFFFFFF);
		  SET(sstg->launch[1], col);
		  col = swap(srcFormat, col);
		  for (i=0; i<32; i++)
		  {
		      if (ONSCREEN(xd+x+i,yc) && (x+i<w))
		      {
			  host_pixels[n++] = ((col>>(i&~7)) >>
					      (7 - i & 7)) & 1;
			  gdbg_info(7,"host_pixels[%d] = 0x%x\n",
				    n-1,host_pixels[n-1]);
		      }
		  }
	      }
	      break;

	  case SSTG_PIXFMT_8BPP:
	      if (addr & 3)
	      {
		  col = iRandom(0xFFFFFFFF);
		  SET(sstg->launch[2], col);
		  col = swap(srcFormat, col);
		  col >>= (addr&3)*8;
		  i = 4-(addr&3);	      // number of pixels in this dword
		  if (w < i)
		      i = w;		// clamp to the width
		  for (x=0; x<i; x++)
		  {
		      if (ONSCREEN(xd+x,yc))
		      {
			  host_pixels[n++] = col&0xFF;
			  gdbg_info(7,"host_pixels[%d] = 0x%x\n",
				    n-1,host_pixels[n-1]);
		      }
		      col >>= 8;
		  }
	      }
	      else
		  x = 0;
	      for (x=x; x < w; x+=4)
	      {
		  col = iRandom(0xFFFFFFFF);
		  SET(sstg->launch[3], col);
		  col = swap(srcFormat, col);
		  for (i=0; i<4; i++)
		  {
		      if (ONSCREEN(xd+x+i,yc) && (x+i<w))
		      {
			  host_pixels[n++] = col&0xFF;
			  gdbg_info(7,"host_pixels[%d] = 0x%x\n",
				    n-1,host_pixels[n-1]);
		      }
		      col >>= 8;
		  }
	      }
	      break;

	  case SSTG_PIXFMT_15BPP:
	  case SSTG_PIXFMT_16BPP:
	      if (addr & 2)
	      {
		  col = iRandom(0xFFFFFFFF);
		  SET(sstg->launch[4], col);
		  col = swap(srcFormat, col);
		  if (ONSCREEN(xd,yc))
		      host_pixels[n++] = col >> 16;
		  x = 1;
	      }
	      else
		  x = 0;
	      for (x=x; x < w; x+=2)
	      {
		  col = iRandom(0xFFFFFFFF);
		  SET(sstg->launch[5], col);
		  col = swap(srcFormat, col);
		  for (i=0; i<2; i++)
		  {
		      if (ONSCREEN(xd+x+i,yc) && (x+i<w))
		      {
			  host_pixels[n++] = col;
		      }
		      col >>= 16;
		  }
	      }
	      break;

	  case SSTG_PIXFMT_24BPP:
	  {
	      int left_over=0;
	      FxU32 old;

	      gdbg_info(7,"addr = %d(0x%x)\n",addr,addr);
	      x = 0;
	      if (addr & 3)
	      {
		  col = iRandom(0xFFFFFFFF);
		  SET(sstg->launch[6], col);
		  col = swap(srcFormat, col);
		  addr &= 3;
		  if (addr == 1)
		  {
		      if (ONSCREEN(xd,yc))
		      {
			  host_pixels[n++] = col>>8;
			  gdbg_info(7,"host_pixels[%d] = 0x%x\n",
				    n-1,host_pixels[n-1]);
		      }
		      x = 1;
		  }
		  else
		  {
		      left_over = 4 - addr;
		      old = col >> (addr*8);
		  }
	      }
	      for (x=x; x < w; x++)
	      {
		  col = iRandom(0xFFFFFFFF);
		  SET(sstg->launch[7], col);
		  col = swap(srcFormat, col);
		  switch(left_over)
		  {
		    case 0:
			if (ONSCREEN(xd+x,yc))
			{
			    host_pixels[n++] = col & 0xFFFFFF;
			    gdbg_info(7,"host_pixels[%d] = 0x%x\n",
				      n-1,host_pixels[n-1]);
			}
			left_over = 1;
			old = col >> 24;
			break;
		    case 1:
			if (ONSCREEN(xd+x,yc))
			{
			    host_pixels[n++] = ((col&0xFFFF)<<8) | old;
			    gdbg_info(7,"host_pixels[%d] = 0x%x\n",
				      n-1,host_pixels[n-1]);
			}
			left_over = 2;
			old = col >> 16;
			break;
		    case 2:
			if (ONSCREEN(xd+x,yc))
			{
			    host_pixels[n++] = ((col&0xFF)<<16) |
				(old & 0xFFFF);
			    gdbg_info(7,"host_pixels[%d] = 0x%x\n",
				      n-1,host_pixels[n-1]);
			}
			x++;
			if (ONSCREEN(xd+x,yc) && (x<w))
			{
			    host_pixels[n++] = (col>>8) & 0xFFFFFF;
			    gdbg_info(7,"host_pixels[%d] = 0x%x\n",
				      n-1,host_pixels[n-1]);
			}
			left_over = 0;
			old = 0xbad00bad;
			break;
		    default:
			GDBG_ERROR("hblt"," internal error in 24bpp\n");
		  }
	      }
	      break;
	  }

	  case SSTG_PIXFMT_32BPP:
	      for (x=0; x < w; x++)
	      {
		  col = iRandom(0xFFFFFFFF);
		  SET(sstg->launch[8], col);
		  col = swap(srcFormat, col);
		  if (ONSCREEN(xd+x,yc))
		      host_pixels[n++] = col;
	      }
	      break;

	  case SSTG_PIXFMT_422YUV:
	      if (addr & 2)
	      {
		  col = iRandom(0xFFFFFFFF);
		  SET(sstg->launch[4], col);
		  col = swap(srcFormat, col);
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
		  col = swap(srcFormat, col);
		  for (i=0; i<2; i++)
		  {
		      if (ONSCREEN(xd+x+i,yc) && (x+i<w))
		      {
			  host_pixels[n++] = sstg_unpack_yuv(col, i);
		      }
		  }
	      }
	      break;

	  case SSTG_PIXFMT_422UYV:
	      if (addr & 2)
	      {
		  col = iRandom(0xFFFFFFFF);
		  SET(sstg->launch[4], col);
		  col = swap(srcFormat, col);
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
		  col = swap(srcFormat, col);
		  for (i=0; i<2; i++)
		  {
		      if (ONSCREEN(xd+x+i,yc) && (x+i<w))
		      {
			  host_pixels[n++] = sstg_unpack_uyv(col, i);
		      }
		  }
	      }
	      break;
	      
	  default:
	      GDBG_ERROR("hblt", "invalid source format\n");
	      break;
	}
    }
}

// this routine assumes packed host blit data
// where stride==dstSize rounded to byte(align==0), word(align==1) or dword(align==3)
// it's main loop is centered around pixel counting, 
// the inner loops decode multiple pixels out of a DWORD and also count pixels
void sendBltDataPacked(SstGRegs *sstg, FxU32 align, FxU32 byte, 
		       int xd, int yd, int w, int h, int ydir,
		       FxU32 srcFormat,
		       FxU32 host_pixels[])
{
    int i, size, pixels_sent, pixels_to_send;
    int xc,yc, n=0, L=0;
    FxU32 addr, col;
    FxU32 pack = srcFormat & SSTG_SRC_PACK;
    FxI32 yInc = (ydir == 0) ? 1 : -1;
    
    addr = byte;
    
    if (pack == SSTG_SRC_PACK_16)
	addr &= ~1;
    else if (pack == SSTG_SRC_PACK_32)
	addr &= ~3;
    
    align -= 1;
    i = 8*(addr&3);				// starting bit in 1st dword
    pixels_to_send = w*h;			// total # of pixels to send
    pixels_sent = 0;
    xc = xd;
    yc = yd;

    switch(srcFormat & SSTG_SRC_FORMAT)
    {
      case SSTG_PIXFMT_1BPP:
	  size = 1;
	  while (pixels_sent < pixels_to_send)
	  {	// for each pixel
	      if (pixels_sent == 0 || i==0)
	      {
		  col = iRandom(0xFFFFFFFF);	// get a random DWORD
		  SET(sstg->launch[L++], col);	// send it to chip
		  col = swap(srcFormat, col);		// swap bytes/words
		  if (L > rRandom(27,31))       // if randomly close to end
		      L = iRandom(2);		// wrap to near beginning
	      }
	      if (ONSCREEN(xc,yc))
	      {		// expand into pixels
		  host_pixels[n++] = ((col>>(i&~7)) >> (7 - i & 7)) & 1;
		  gdbg_info(7,"host_pixels[%d] = 0x%x\n",n-1,host_pixels[n-1]);
	      }
	      pixels_sent++;
	      i += size;
	      if (++xc >= xd + w)
	      {		// done with this row
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
	  {	// for each pixel
	      if (pixels_sent == 0 || i==0)
	      {
		  col = iRandom(0xFFFFFFFF);	// get a random DWORD
		  SET(sstg->launch[L++], col);	// send it to chip
		  col = swap(srcFormat, col);		// swap bytes/words
		  if (L > rRandom(20,30))	    // if randomly close to end
		      L = iRandom(2);		// wrap to near beginning
	      }
	      if (ONSCREEN(xc,yc))
	      {		// expand into pixels
		  host_pixels[n++] = (col>>i) & (0xFFFFFFFF>>(32-size));
		  gdbg_info(7,"host_pixels[%d] = 0x%x\n",n-1,host_pixels[n-1]);
	      }
	      pixels_sent++;
	      i += size;
	      if (++xc >= xd + w)
	      {		// done with this row
		  i = (i + align) & ~align;
		  xc = xd;
		  yc += yInc;
	      }
	      i &= 31;
	  }
	  break;

      case SSTG_PIXFMT_24BPP:
      {
	  FxU32 last;				// holds the last DWORD

	  while (pixels_sent < pixels_to_send)
	  {	// for each pixel
	      if (pixels_sent == 0 || i>8)
	      {
		  col = iRandom(0xFFFFFFFF);	// get a random DWORD
		  SET(sstg->launch[L++], col);	// send it to chip
		  col = swap(srcFormat, col);		// swap bytes/words
		  if (L > rRandom(24,31))	// if randomly close to end
		      L = iRandom(2);		// wrap to near beginning
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
	      {		// done with this row
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

      case SSTG_PIXFMT_32BPP:
	  sendBltData0(sstg, byte, 0, xd,yd, w,h, ydir, srcFormat,
		       host_pixels);
	  break;

      case SSTG_PIXFMT_422YUV:
      case SSTG_PIXFMT_422UYV:
	  sendBltData0(sstg, addr, 0, xd,yd, w,h, ydir, srcFormat,
		       host_pixels);
	  break;

      default:
	  GDBG_ERROR("hblt", "invalid source format\n");
	  break;
    }
}

//
// returns "true" IFF src and dst are compatible blit source / destination
// formats
//
int sstg_compatible_src_dst(FxU32 src, FxU32 dst)
{
    src &= SSTG_SRC_FORMAT;
    dst &= SSTG_DST_FORMAT;
    return  !(((dst == SSTG_PIXFMT_8BPP)
	       && ((src != SSTG_PIXFMT_1BPP) && (src != SSTG_PIXFMT_8BPP)))
	      ||
	      ((dst != SSTG_PIXFMT_8BPP)
	       && (src == SSTG_PIXFMT_8BPP)));
}

#define SRCFORMAT (srcFormat & SSTG_SRC_FORMAT)

//
// returns "true" IFF the given source format and source packing are
// compatible
//
int
sstg_compatible_srcfmt_pack(FxU32 srcFormat, FxU32 pack)
{
    srcFormat &= SSTG_SRC_FORMAT;
    pack &= SSTG_SRC_PACK;
    
    switch (srcFormat)
    {
      case SSTG_PIXFMT_1BPP:
      case SSTG_PIXFMT_8BPP:
      case SSTG_PIXFMT_24BPP:
	  return 1;
	  
      case SSTG_PIXFMT_15BPP:
      case SSTG_PIXFMT_16BPP:
	  if (pack == SSTG_SRC_PACK_8)
	      return 0;
	  else
	      return 1;

      case SSTG_PIXFMT_32BPP:
      case SSTG_PIXFMT_422YUV:
      case SSTG_PIXFMT_422UYV:
	  if ((pack == SSTG_SRC_PACK_8) || (pack == SSTG_SRC_PACK_16))
	      return 0;
	  else
	      return 1;

      default:
	  GDBG_ERROR("sstg_compatible_srcfmt_pack", "bad srcFormat");
    }

    return(0);
}



FxU32 sstg_random_srcFormat(FxU32 dstFormat, int src_is_tiled)
{
    FxU32 srcFormat, stride, pack;
    do
    {
	do
	{
	    srcFormat = iRandom(9);
	} while ((srcFormat == 6) || (srcFormat == 7));
		
	srcFormat <<= SSTG_SRC_FORMAT_SHIFT;
	// no 15bpp support, make it be 16bpp
	if (srcFormat == SSTG_PIXFMT_15BPP)
	    srcFormat = SSTG_PIXFMT_16BPP;
    }
    while (!sstg_compatible_src_dst(srcFormat, dstFormat));

    if (diago.flip)
    {		// command line override
	srcFormat = diago.flip;

	if (src_is_tiled &&
	    ((srcFormat & SSTG_SRC_PACK) != SSTG_SRC_PACK_SRC))
	{
	    GDBG_ERROR("sstg_rancom_srcFormat",
		       "illegal source packing (%d) when tiled\n",
		       (srcFormat & SSTG_SRC_PACK)>>SSTG_SRC_PACK_SHIFT);
	    DIAG_FAIL();
	}

	if (!sstg_compatible_src_dst(srcFormat, dstFormat))
	{
	    GDBG_ERROR("sstg_random_srcFormat",
		       "incompatible src and destination formats\n");
	    DIAG_FAIL();
	}
    }
    // also pick a random source stride and source packing

    stride = iRandom(diago.xmaxscreen*4);
    if (!diago.flip)
	pack = iRandom(3);
    else
	pack = (diago.flip & SSTG_SRC_PACK) >> SSTG_SRC_PACK_SHIFT;

    if (iRandom(3) == 0)
	stride = iRandom(diago.tsize);

    if (src_is_tiled)
    {
	pack = SSTG_SRC_PACK_SRC>>SSTG_SRC_PACK_SHIFT;
	stride = CEIL(stride,SST_TILE_WIDTH);
    }
    else
    {
	if ((SRCFORMAT == SSTG_PIXFMT_15BPP) ||
	    (SRCFORMAT == SSTG_PIXFMT_16BPP))
	{
	    stride &= ~1;
	    if (pack == 1) pack = 0;
	}
	else if ((SRCFORMAT == SSTG_PIXFMT_32BPP) ||
		 (SRCFORMAT == SSTG_PIXFMT_422YUV) ||
		 (SRCFORMAT == SSTG_PIXFMT_422UYV))
	{
	    stride &= ~3;
	    if (pack == 1) pack = 0;
	    if (pack == 2) pack = 3;
	}
    }

    srcFormat &= ~SSTG_SRC_PACK;
    srcFormat |= pack<<SSTG_SRC_PACK_SHIFT;
    srcFormat |= stride;

    return srcFormat;
}
