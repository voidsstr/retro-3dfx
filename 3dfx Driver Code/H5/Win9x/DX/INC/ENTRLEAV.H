/* -*-c++-*- */
/* $Header: entrleav.h, 3, 10/11/00 8:53:04 PM, Brent$ */
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
**
** $Revision: 3$
** $Date: 10/11/00 8:53:04 PM$
**
** $History: entrleav.h $
** 
** *****************  Version 3  *****************
** User: Andrew       Date: 6/16/99    Time: 4:41p
** Updated in $/devel/h5/Win9x/dx/inc
** Added some IS_16 so that entrleav.h can be used in 16 bit code
** 
** *****************  Version 2  *****************
** User: Andrew       Date: 6/15/99    Time: 5:11p
** Updated in $/devel/h5/Win9x/dx/inc
** Added SKIP_FLAGS and PUNT_EXP to reuse code with SLI_AA Slave and
** Master Mode
** 
** *****************  Version 1  *****************
** User: Michael      Date: 6/02/99    Time: 6:51a
** Created in $/devel/h5/Win9x/dx/inc
** Branch from h3
** 
** *****************  Version 22  *****************
** User: Stb_sjohnston Date: 4/07/99    Time: 2:48p
** Updated in $/devel/h3/win95/dx/inc
** Cleaner fix for Slashdot/Wax bug (PRS 4457) from Chris Edgington
** 
** *****************  Version 21  *****************
** User: Xingc        Date: 2/25/99    Time: 7:56p
** Updated in $/devel/h3/Win95/dx/inc
** Add DoMKTV( ) macros for  DD32 and minivdd
** 
** *****************  Version 20  *****************
** User: Stb_skephart Date: 2/11/99    Time: 8:32a
** Updated in $/devel/h3/win95/dx/inc
** 
** *****************  Version 19  *****************
** User: Andrew       Date: 8/18/98    Time: 2:02p
** Updated in $/devel/h3/Win95/dx/inc
** Changed UseDib fix to just reset the PDEV instead of removing whole
** call
** 
** *****************  Version 18  *****************
** User: Andrew       Date: 8/16/98    Time: 6:46p
** Updated in $/devel/h3/Win95/dx/inc
** Removed ResetInvariant from Use_DibEng path in Both FXLEAVE and
** FXLEAVE_NOHW.  In one case the BUSY bit was set so we should not be
** touching the HW and the other it was not set which was causing a
** problem with SW cursors.
** 
** *****************  Version 17  *****************
** User: Andrew       Date: 7/17/98    Time: 3:08p
** Updated in $/devel/h3/Win95/dx/inc
** Cleared BUSY bit if we take the HWC Exclusive early exit
** 
** *****************  Version 16  *****************
** User: Ken          Date: 7/15/98    Time: 3:48p
** Updated in $/devel/h3/win95/dx/inc
** restored (and optimized) FXENTER's dst_was_devbit case
** 
** *****************  Version 14  *****************
** User: Andrew       Date: 5/27/98    Time: 8:43a
** Updated in $/devel/h3/Win95/dx/inc
** Remove DidExclude dword and made flag
** 
** *****************  Version 13  *****************
** User: Andrew       Date: 5/20/98    Time: 12:30p
** Updated in $/devel/h3/Win95/dx/inc
** Changed DoIntersect to int from SHORT to remove warnings
** 
** *****************  Version 12  *****************
** User: Andrew       Date: 5/19/98    Time: 6:08p
** Updated in $/devel/h3/Win95/dx/inc
** found bug in cursor & rect intersection code
** 
** *****************  Version 11  *****************
** User: Michael      Date: 5/15/98    Time: 4:26p
** Updated in $/devel/h3/Win95/dx/inc
** Backout my most recent check in.
** 
** *****************  Version 10  *****************
** User: Michael      Date: 5/15/98    Time: 9:19a
** Updated in $/devel/h3/Win95/dx/inc
** Modify FXPUNT and FXLEAVE_NOACCESS to pass in lpDst.
** 
** *****************  Version 9  *****************
** User: Ken          Date: 5/13/98    Time: 4:22p
** Updated in $/devel/h3/win95/dx/inc
** added s/w cursor exclusion to macro-ized FXENTER
** 
** *****************  Version 8  *****************
** User: Ken          Date: 5/13/98    Time: 9:38a
** Updated in $/devel/h3/win95/dx/inc
** we now pay attention to SDATA_GDIFLAGS_HWC_EXCLUSIVE and 
** don't touch h/w in rendering routines if it's set
** 
** *****************  Version 7  *****************
** User: Ken          Date: 5/13/98    Time: 8:08a
** Updated in $/devel/h3/win95/dx/inc
** fast enter/leave turned on, working with SSB.
** also, bumped up the max # of device bitmap allocations by 4x
** 
** *****************  Version 5  *****************
** User: Ken          Date: 4/30/98    Time: 11:26a
** Updated in $/devel/h3/win95/dx/inc
** updated enter/leave to always check/set the busy bit on the 
** screen's pdevice, not the destination bitmap's pdevice ('cuz it might
** be a device bitmap.
** 
** *****************  Version 4  *****************
** User: Ken          Date: 4/27/98    Time: 5:35p
** Updated in $/devel/h3/win95/dx/inc
** complete (well, almost) enter/leave implementation for gdi32
** 
** *****************  Version 3  *****************
** User: Ken          Date: 4/24/98    Time: 1:33p
** Updated in $/devel/h3/win95/dx/inc
** first installment of FXENTER / FXLEAVE display driver drawing function
** work.  gdi32 only, no gdi16 yet.   works, but is slow, don't measure
** performance until all work is done, in about a week.
** also renamed fields in some blit parameter functions to be a bit more
** meaningful
** 
** *****************  Version 2  *****************
** User: Ken          Date: 4/23/98    Time: 5:45p
** Updated in $/devel/h3/win95/dx/inc
** working now, minus s/w cursor code
** 
** *****************  Version 1  *****************
** User: Ken          Date: 4/23/98    Time: 11:11a
** Created in $/devel/h3/win95/dx/inc
** common rendering function (2d) stuff
**
*/

//
// FILE: entrleav.h
//
// Preprocessor definitions for the GDI 16/32 common rendering routine
// infrastructure
// 

#ifndef _ENTRLEAV_H_
#define _ENTRLEAV_H_

#ifdef SLI_AA

#define SKIP_FLAGS (SDATA_GDIFLAGS_HWC_EXCLUSIVE | SDATA_GDIFLAGS_SLI_AA_SLAVE)

#ifdef IS_16
#define PUNT_EXP ((_FF(lpPDevice)->deFlags & (BUSY | PALETTE_XLAT)) || (_FF(gdiFlags) & SDATA_GDIFLAGS_SLI_AA_MASTER))
#else
#define PUNT_EXP ((*lpDriverData->lpDeFlags & (BUSY | PALETTE_XLAT)) || (_FF(gdiFlags) & SDATA_GDIFLAGS_SLI_AA_MASTER))
#endif

#else

#define SKIP_FLAGS (SDATA_GDIFLAGS_HWC_EXCLUSIVE)

#ifdef IS_16
#define PUNT_EXP (_FF(lpPDevice)->deFlags & (BUSY | PALETTE_XLAT))
#else
#define PUNT_EXP (*lpDriverData->lpDeFlags & (BUSY | PALETTE_XLAT))
#endif

#endif


#ifndef IS_16
int DoIntersect(int, int, int, int);

#undef STATIC_RIS
#ifdef STATIC_RIS
static void
ResetInvariantState(FxU32 resetMask)
{
    FxU32 cmdMask, cmdWords, dstFormat;
    CMDFIFO_PROLOG(cmdFifo);

    DEBUG_FIX;

    cmdWords = 0;
    cmdMask = 0;

    if (resetMask & RESET_PDEV)
    {
	if (_FF(ddPrimaryInTile))
	{
	    _FF(gdiDesktopStart) |= SSTG_IS_TILED;
	    dstFormat = _FF(screenFormat) & ~(SSTG_DST_LINEAR_STRIDE |
					      SSTG_DST_TILE_STRIDE);
	    dstFormat |= _FF(ddTileStride) << SSTG_DST_STRIDE_SHIFT;
	    _FF(screenFormat) = dstFormat;

       lpDst->deDeltaScan   = (WORD)_FF(ddTilePitch);
	}
	else
	{
	    // linear
	    _FF(gdiDesktopStart) &= ~(SSTG_IS_TILED);
	    dstFormat = _FF(screenFormat) & ~(SSTG_DST_LINEAR_STRIDE |
					      SSTG_DST_TILE_STRIDE);
	    dstFormat |= _FF(pitch) << SSTG_DST_STRIDE_SHIFT;
	    _FF(screenFormat) = dstFormat;

       lpDst->deDeltaScan   = (WORD)_FF(pitch);
	}

	// now change the dib engine's stride so that it can render to
	// the desktop
    } 

    if (resetMask & RESET_DST)
    {
	cmdWords += 5;
	cmdMask |= (clip0minBit | clip0maxBit | dstBaseAddrBit | dstFormatBit |
		    commandExBit);
    }

    if (resetMask & RESET_SRC)
    {
	cmdWords += 1;
	cmdMask |= srcBaseAddrBit;
    }
    
    if (cmdWords > 0)
    {
	cmdWords += 1;
	CMDFIFO_SETUP(cmdFifo);
	CMDFIFO_CHECKROOM(cmdFifo, cmdWords);
	SETPH(cmdFifo, SSTCP_PKT2 | cmdMask);
	if (resetMask & RESET_DST)
	{
	    SET(cmdFifo, _FF(lpGRegs)->clip0min, 0);
	    SET(cmdFifo, _FF(lpGRegs)->clip0max,
		(lpDriverData->bi.biHeight << 16) |
		lpDriverData->bi.biWidth);   
	    SET(cmdFifo, _FF(lpGRegs)->dstBaseAddr,
		lpDriverData->gdiDesktopStart);
	    SET(cmdFifo, _FF(lpGRegs)->dstFormat, lpDriverData->screenFormat);
	}
	if (resetMask & RESET_SRC)
	{
	    SET(cmdFifo, _FF(lpGRegs)->srcBaseAddr,
		lpDriverData->gdiDesktopStart);
	}
	if (resetMask & RESET_DST)
	{
	    SET(cmdFifo, _FF(lpGRegs)->commandEx, 0);
	}
	
	BUMP(cmdWords);

	CMDFIFO_EPILOG(cmdFifo);   
    }
}
#endif // #ifdef STATIC_RIS


extern char *fxEnterProcName;
extern FxU32 _SrcFormat;

#define FXENTER_RECT			1	// boolean true
#define FXENTER_NO_RECT			0	// boolean false	
#define FXENTER_NO_SRC			0	// null pointer

#define FXENTER_NO_SRCFORMAT		0	// boolean false
#define FXENTER_COMPUTE_SRCFORMAT	BIT(0)
#define FXENTER_COMPUTE_DSTBA_FORMAT	BIT(1)

// VC5 and VC6 automatically turn off the optimizer when they encounter
// any kind of embedded assembly jump instruction. Therefore, when we want
// to build with VC5 or VC6 and we don't want to take a performance hit 
// we must call a subfunction; however, I found that making the function
// inline makes the compiler copy the embedded assembly into the caller
// but does not turn off the optimizer, so we get almost the same output

#define INLINE_SETBUSY
#ifdef INLINE_SETBUSY

static _inline BOOL mySetBusy( VOID )
{
    // ebx set to pointer to deFlags by caller
    __asm
    {
        xor     eax, eax
        bts     WORD PTR [ebx], BUSY_BIT
        setc    al
    }
}

#define SET_BUSY(ptr)\
{\
    {_asm mov ebx, ptr};\
    if ( mySetBusy() )\
        goto Use_DibEng;\
}


#else

#define SET_BUSY(ptr)\
{\
    __asm\
    {\
        __asm mov ebx, ptr\
        __asm bts  WORD PTR [ebx], BUSY_BIT\
        __asm jc Use_DibEng\
    }\
}

#endif

#ifdef FXENTER_EXTERNAL_FUNCTION
FxU32 myFxEnter(FxU16 deFlags,
		FxU32 debugPuntExpr,
		LPDIBENGINE lpDst,
		LPDIBENGINE lpSrc,
		FxU32 flags,
		int rect1_valid,
		int r1_left, int r1_top, int r1_right, int r1_bottom,
		int rect2_valid,
		int r2_left, int r2_top, int r2_right, int r2_bottom);

#define FXENTER(procName, deFlags, debugPuntExpr, cmdFifo, lpDst, lpSrc, \
		flags, \
		rect1_valid, r1_left, r1_top, r1_right, r1_bottom, \
		rect2_valid, r2_left, r2_top, r2_right, r2_bottom ) \
{ \
    fxEnterProcName = procName; \
    _SrcFormat = myFxEnter(deFlags, debugPuntExpr, lpDst, lpSrc, flags, \
	 	          rect1_valid, r1_left, r1_top, r1_right, r1_bottom, \
		          rect2_valid, r2_left, r2_top, r2_right, r2_bottom); \
    if (bPunt) \
        goto Use_DibEng; \
    CMDFIFO_SETUP(cmdFifo); \
}
#endif // #ifdef FXENTER_EXTERNAL_FUNCTION

#define FXENTER_MACRO
#ifdef FXENTER_MACRO
# define FXENTER(procName, myDeFlags, debugPuntExpr, cmdFifo, lpDst, lpSrcIn, \
		flags, \
		rect1_valid, r1_left, r1_top, r1_right, r1_bottom, \
		rect2_valid, r2_left, r2_top, r2_right, r2_bottom ) \
{ \
    FxU32 srcFormat, srcAddr, dstFormat, dstAddr;\
    FxU32 srcIsDevBit, dstIsDevBit;\
    FxU32 doSecondRect;\
    LPDIBENGINE mylpSrc = (LPDIBENGINE) 0; \
    FxU32 *pDeFlags;\
    \
    if (lpSrcIn)\
	mylpSrc = lpSrcIn;\
\
    if (PUNT_EXP)\
        goto Use_DibEng;\
    if ( ! (myDeFlags & VRAM))\
        goto Use_DibEng;\
\
    pDeFlags = lpDriverData->lpDeFlags;\
    SET_BUSY(pDeFlags);\
\
    dstIsDevBit = myDeFlags & OFFSCREEN;\
\
    srcIsDevBit = mylpSrc &&\
	( ! ((mylpSrc->deFlags & (VRAM | OFFSCREEN)) ^ (VRAM | OFFSCREEN)));\
\
    if (_FF(gdiFlags) != 0)\
    {\
        if (_FF(gdiFlags) & SKIP_FLAGS)\
            {\
           *lpDriverData->lpDeFlags &= ~BUSY;\
            return TRUE;\
            }\
\
	if (_FF(gdiFlags) & SDATA_GDIFLAGS_2D_DIRTY)\
	{\
	    ResetInvariantState(RESET_DST | RESET_SRC | RESET_PDEV);\
           _FF(gdiFlags) &= ~ (SDATA_GDIFLAGS_2D_DIRTY |\
			       SDATA_GDIFLAGS_DST_WAS_DEVBIT |\
			       SDATA_GDIFLAGS_SRC_WAS_DEVBIT);\
	}\
\
	if (_FF(gdiFlags & SDATA_GDIFLAGS_DST_WAS_DEVBIT) && !dstIsDevBit)\
	{\
		ResetInvariantState(RESET_DST);\
		_FF(gdiFlags) &= ~ SDATA_GDIFLAGS_DST_WAS_DEVBIT;\
	}\
\
	if (_FF(gdiFlags & SDATA_GDIFLAGS_SRC_WAS_DEVBIT) && !srcIsDevBit)\
	{\
	    ResetInvariantState(RESET_SRC);\
	    _FF(gdiFlags) &= ~ SDATA_GDIFLAGS_SRC_WAS_DEVBIT;\
	}\
\
   	if (_FF(gdiFlags) & SDATA_GDIFLAGS_SW_CURSOR)\
	{\
            doSecondRect = 1;\
                if (rect1_valid)\
                {\
                    if (DoIntersect((int)r1_left, (int)r1_top, (int)r1_right, (int)r1_bottom))\
                    {\
                        FxBeginAccess32(CURSOREXCLUDE);\
                        doSecondRect = 0;\
                    }\
                }\
                if (rect2_valid && doSecondRect)\
		{\
         if (DoIntersect((int)r2_left, (int)r2_top, (int)r2_right, (int)r2_bottom))\
			  {\
			  FxBeginAccess32(CURSOREXCLUDE);\
		     }\
                 }\
	  }\
    }\
\
    CMDFIFO_SETUP(cmdFifo);\
\
    if (srcIsDevBit)\
    {\
	srcFormat = _FF(screenFormat) & ~(SSTG_DST_LINEAR_STRIDE |\
					  SSTG_DST_TILE_STRIDE);\
	srcFormat |= mylpSrc->deDeltaScan & SSTG_DST_LINEAR_STRIDE;\
	srcAddr = mylpSrc->deBitsOffset - lpDriverData->lfbBase;\
\
	CMDFIFO_CHECKROOM(cmdFifo, 3);\
	SETPH(cmdFifo, SSTCP_PKT2 | srcBaseAddrBit | srcFormatBit);\
	SET(cmdFifo, _FF(lpGRegs)->srcBaseAddr, srcAddr);\
	SET(cmdFifo, _FF(lpGRegs)->srcFormat, srcFormat);\
	BUMP(3);\
\
	_FF(gdiFlags) |= SDATA_GDIFLAGS_SRC_WAS_DEVBIT;\
        if (flags & FXENTER_COMPUTE_SRCFORMAT)\
            _SrcFormat = srcFormat; \
    }\
    else if (flags & FXENTER_COMPUTE_SRCFORMAT)\
    {\
	_SrcFormat = _FF(screenFormat);\
    }\
\
    if (dstIsDevBit)\
    {\
	dstAddr = lpDst->deBitsOffset - lpDriverData->lfbBase;\
	dstFormat = _FF(screenFormat) & SSTG_DST_FORMAT;\
	dstFormat |= SSTG_SRC_PACK_SRC;\
	dstFormat |= lpDst->deDeltaScan & SSTG_SRC_LINEAR_STRIDE;\
	\
	CMDFIFO_CHECKROOM(cmdFifo, 5);\
	SETPH(cmdFifo, (SSTCP_PKT2 | clip0minBit | clip0maxBit |\
			dstBaseAddrBit | dstFormatBit));\
	SET(cmdFifo, _FF(lpGRegs)->clip0min, 0);\
	SET(cmdFifo, _FF(lpGRegs)->clip0max, R32(lpDst->deHeight,\
						 lpDst->deWidth));\
	SET(cmdFifo, _FF(lpGRegs)->dstBaseAddr, dstAddr);\
	SET(cmdFifo, _FF(lpGRegs)->dstFormat, dstFormat);\
	BUMP(5);\
	\
	_FF(gdiFlags) |= SDATA_GDIFLAGS_DST_WAS_DEVBIT;\
\
	if (flags & FXENTER_COMPUTE_DSTBA_FORMAT)\
	{\
	    CurrentDstAddr = dstAddr;\
	    CurrentDstFormat = dstFormat;\
	}\
    }\
    else\
    {\
	if (flags & FXENTER_COMPUTE_DSTBA_FORMAT)\
	{\
	    CurrentDstAddr = _FF(gdiDesktopStart);\
	    CurrentDstFormat = _FF(screenFormat);\
	}\
    }\
    if (rect1_valid) \
        _FF(lastY) = r1_bottom; \
}

#endif // #ifdef FXENTER_MACRO


#ifdef FXENTER_INLINE_OR_STATIC_FUNCTION
FxU32 __inline
myFxEnter(FxU16 deFlags,
	  FxU32 debugPuntExpr,
	  LPDIBENGINE lpDst,
	  LPDIBENGINE lpSrc,
	  FxU32 flags,
	  int rect1_valid,
	  int r1_left, int r1_top, int r1_right, int r1_bottom,
	  int rect2_valid,
	  int r2_left, int r2_top, int r2_right, int r2_bottom);
{ \
    FxU32 srcFormat, srcAddr, dstFormat, dstAddr;\
    FxU32 srcIsDevBit, dstIsDevBit;\
    LPDIBENGINE mylpSrc = (LPDIBENGINE) 0; \
    FxU32 *pDeFlags;\
    \
    if (lpSrcIn)\
	mylpSrc = lpSrcIn;\
\
    if (*lpDriverData->lpDeFlags & (BUSY | PALETTE_XLAT))\
        goto Use_DibEng;\
    if ( ! (myDeFlags & VRAM))\
        goto Use_DibEng;\
\
    pDeFlags = lpDriverData->lpDeFlags;\
    SET_BUSY(pDeFlags);\
\
    dstIsDevBit = myDeFlags & OFFSCREEN;\
\
    srcIsDevBit = mylpSrc &&\
	( ! ((mylpSrc->deFlags & (VRAM | OFFSCREEN)) ^ (VRAM | OFFSCREEN)));\
\
    if (_FF(gdiFlags) != 0)\
    {\
	if (_FF(gdiFlags) & SDATA_GDIFLAGS_2D_DIRTY)\
	{\
	    ResetInvariantState(RESET_DST | RESET_SRC | RESET_PDEV);\
           _FF(gdiFlags) &= ~ (SDATA_GDIFLAGS_2D_DIRTY |\
			       SDATA_GDIFLAGS_DST_WAS_DEVBIT |\
			       SDATA_GDIFLAGS_SRC_WAS_DEVBIT);\
	}\
\
	if (_FF(gdiFlags & SDATA_GDIFLAGS_DST_WAS_DEVBIT))\
	{\
	    if (!dstIsDevBit ||	(flags & FXENTER_COMPUTE_DSTBA_FORMAT))\
	    {\
		ResetInvariantState(RESET_DST);\
		_FF(gdiFlags) &= ~ SDATA_GDIFLAGS_DST_WAS_DEVBIT;\
	    }\
	}\
\
	if (_FF(gdiFlags & SDATA_GDIFLAGS_SRC_WAS_DEVBIT) && !srcIsDevBit)\
	{\
	    ResetInvariantState(RESET_SRC);\
	    _FF(gdiFlags) &= ~ SDATA_GDIFLAGS_SRC_WAS_DEVBIT;\
	}\
	\
	if (_FF(gdiFlags) & SDATA_GDIFLAGS_SW_CURSOR)\
	{\
	}\
    }\
\
    CMDFIFO_SETUP(cmdFifo);\
\
    if (srcIsDevBit)\
    {\
	srcFormat = _FF(screenFormat) & ~(SSTG_DST_LINEAR_STRIDE |\
					  SSTG_DST_TILE_STRIDE);\
	srcFormat |= mylpSrc->deDeltaScan & SSTG_DST_LINEAR_STRIDE;\
	srcAddr = mylpSrc->deBitsOffset - lpDriverData->lfbBase;\
\
	CMDFIFO_CHECKROOM(cmdFifo, 3);\
	SETPH(cmdFifo, SSTCP_PKT2 | srcBaseAddrBit | srcFormatBit);\
	SET(cmdFifo, _FF(lpGRegs)->srcBaseAddr, srcAddr);\
	SET(cmdFifo, _FF(lpGRegs)->srcFormat, srcFormat);\
	BUMP(3);\
\
	_FF(gdiFlags) |= SDATA_GDIFLAGS_SRC_WAS_DEVBIT;\
        if (flags & FXENTER_COMPUTE_SRCFORMAT)\
            _SrcFormat = srcFormat; \
    }\
    else if (flags & FXENTER_COMPUTE_SRCFORMAT)\
    {\
	_SrcFormat = _FF(screenFormat);\
    }\
\
    if (dstIsDevBit)\
    {\
	dstAddr = lpDst->deBitsOffset - lpDriverData->lfbBase;\
	dstFormat = _FF(screenFormat) & SSTG_DST_FORMAT;\
	dstFormat |= SSTG_SRC_PACK_SRC;\
	dstFormat |= lpDst->deDeltaScan & SSTG_SRC_LINEAR_STRIDE;\
	\
	CMDFIFO_CHECKROOM(cmdFifo, 5);\
	SETPH(cmdFifo, (SSTCP_PKT2 | clip0minBit | clip0maxBit |\
			dstBaseAddrBit | dstFormatBit));\
	SET(cmdFifo, _FF(lpGRegs)->clip0min, 0);\
	SET(cmdFifo, _FF(lpGRegs)->clip0max, R32(lpDst->deHeight,\
						 lpDst->deWidth));\
	SET(cmdFifo, _FF(lpGRegs)->dstBaseAddr, dstAddr);\
	SET(cmdFifo, _FF(lpGRegs)->dstFormat, dstFormat);\
	BUMP(5);\
	\
	_FF(gdiFlags) |= SDATA_GDIFLAGS_DST_WAS_DEVBIT;\
\
	if (flags & FXENTER_COMPUTE_DSTBA_FORMAT)\
	{\
	    CurrentDstAddr = dstAddr;\
	    CurrentDstFormat = dstFormat;\
	}\
    }\
    else\
    {\
	if (flags & FXENTER_COMPUTE_DSTBA_FORMAT)\
	{\
	    CurrentDstAddr = _FF(gdiDesktopStart);\
	    CurrentDstFormat = _FF(screenFormat);\
	}\
    }\
}

#define FXENTER(procName, deFlags, debugPuntExpr, cmdFifo, lpDst, lpSrc, \
		flags, \
		rect1_valid, r1_left, r1_top, r1_right, r1_bottom, \
		rect2_valid, r2_left, r2_top, r2_right, r2_bottom ) \
{ \
    fxEnterProcName = procName; \
    _SrcFormat = myFxEnter(deFlags, debugPuntExpr, lpDst, lpSrc, flags, \
	 	          rect1_valid, r1_left, r1_top, r1_right, r1_bottom, \
		          rect2_valid, r2_left, r2_top, r2_right, r2_bottom); \
    if (bPunt) \
        goto Use_DibEng; \
    CMDFIFO_SETUP(cmdFifo); \
}
#endif // #ifdef FXENTER_INLINE_OR_STATIC_FUNCTION

#if 0
#define FXLEAVE(procName, cmdFifo, lpDst) \
    CMDFIFO_EPILOG(cmdFifo); \
\
ResetInvariantState(RESET_SRC | RESET_DST); \
    *lpDriverData->lpDeFlags &= ~BUSY;\
    return TRUE; \
\
Use_DibEng: \
    if (_FF(gdiFlags) & SDATA_GDIFLAGS_2D_DIRTY) \
    { \
        ResetInvariantState(RESET_PDEV | RESET_SRC | RESET_DST); \
	_FF(gdiFlags) &= ~ SDATA_GDIFLAGS_2D_DIRTY; \
    } \
\
    return TRUE;
#else // #if 1
#ifdef KMVP
void DoKMTV(GLOBALDATA * ppdev);
#define FXLEAVE(procName, cmdFifo, lpDst) \
    CMDFIFO_EPILOG(cmdFifo); \
    if ( (SDATA_GDIFLAGS_SW_CURSOR | SDATA_GDIFLAGS_CURSOR_IS_EXCLUDED) == (_FF(gdiFlags) & (SDATA_GDIFLAGS_SW_CURSOR | SDATA_GDIFLAGS_CURSOR_IS_EXCLUDED)))\
    {\
        FxEndAccess32(CURSOREXCLUDE);\
    }\
    if (_FF(gdiFlags) & SDATA_GDIFLAGS_KMTV_FLAG) DoKMTV(lpDriverData);\
    *lpDriverData->lpDeFlags &= ~BUSY;\
    return TRUE; \
\
Use_DibEng: \
    bPunt = TRUE;\
    if (_FF(gdiFlags) & SDATA_GDIFLAGS_2D_DIRTY) \
    { \
        ResetInvariantState(RESET_PDEV); \
    } \
\
    return TRUE;
#define FXLEAVE2(procName, cmdFifo, lpDst, retval) \
    CMDFIFO_EPILOG(cmdFifo); \
    if ( (SDATA_GDIFLAGS_SW_CURSOR | SDATA_GDIFLAGS_CURSOR_IS_EXCLUDED) == (_FF(gdiFlags) & (SDATA_GDIFLAGS_SW_CURSOR | SDATA_GDIFLAGS_CURSOR_IS_EXCLUDED)))\
    {\
        FxEndAccess32(CURSOREXCLUDE);\
    }\
    if (_FF(gdiFlags) & SDATA_GDIFLAGS_KMTV_FLAG) DoKMTV(lpDriverData);\
    *lpDriverData->lpDeFlags &= ~BUSY;\
    return retval; \
\
Use_DibEng: \
    bPunt = TRUE;\
    if (_FF(gdiFlags) & SDATA_GDIFLAGS_2D_DIRTY) \
    { \
        ResetInvariantState(RESET_PDEV); \
    } \
\
    return retval;
#else
#define FXLEAVE(procName, cmdFifo, lpDst) \
    CMDFIFO_EPILOG(cmdFifo); \
    if ( (SDATA_GDIFLAGS_SW_CURSOR | SDATA_GDIFLAGS_CURSOR_IS_EXCLUDED) == (_FF(gdiFlags) & (SDATA_GDIFLAGS_SW_CURSOR | SDATA_GDIFLAGS_CURSOR_IS_EXCLUDED)))\
    {\
        FxEndAccess32(CURSOREXCLUDE);\
    }\
    *lpDriverData->lpDeFlags &= ~BUSY;\
    return TRUE; \
\
Use_DibEng: \
    bPunt = TRUE;\
    if (_FF(gdiFlags) & SDATA_GDIFLAGS_2D_DIRTY) \
    { \
        ResetInvariantState(RESET_PDEV); \
    } \
\
    return TRUE;
#define FXLEAVE2(procName, cmdFifo, lpDst, retval) \
    CMDFIFO_EPILOG(cmdFifo); \
    if ( (SDATA_GDIFLAGS_SW_CURSOR | SDATA_GDIFLAGS_CURSOR_IS_EXCLUDED) == (_FF(gdiFlags) & (SDATA_GDIFLAGS_SW_CURSOR | SDATA_GDIFLAGS_CURSOR_IS_EXCLUDED)))\
    {\
        FxEndAccess32(CURSOREXCLUDE);\
    }\
    *lpDriverData->lpDeFlags &= ~BUSY;\
    return retval; \
\
Use_DibEng: \
    bPunt = TRUE;\
    if (_FF(gdiFlags) & SDATA_GDIFLAGS_2D_DIRTY) \
    { \
        ResetInvariantState(RESET_PDEV); \
    } \
\
    return retval;

#endif      //KMVP
#endif // #if 1




//
// FXLEAVE_NOHWACCESS
//
// this is used when there's a top level "early punt" function that
// runs through a bunch of test before calling one of several
// h/w accelerated sub-functions
//

#define FXLEAVE_NOHWACCESS(procName, retval) \
Use_DibEng: \
    if (_FF(gdiFlags) & SDATA_GDIFLAGS_2D_DIRTY) \
    { \
        ResetInvariantState(RESET_PDEV); \
    } \
\
    return retval;
    
#define FXPUNT_NORMAL		1
#define FXPUNT_EXTTEXTEXT	2

#define FXPUNT(puntVal)	\
{ \
	bPunt = puntVal; \
	goto Use_DibEng; \
}

#endif   // ifndef IS_16

#endif // #ifndef _ENTRLEAV_H_
