/*
** Copyright (c) 1999, 3Dfx Interactive, Inc.
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
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
**
** $Header: HRM_Overlay_Surface.c, 6, 10/11/00 8:35:13 PM, Brent$
** $Log: 
**  6    3dfx      1.1.1.3     10/11/00 Brent           Forced check in to enforce
**       branching.
**  5    3dfx      1.1.1.2     07/11/00 Critical Path   new source drop
**  4    3dfx      1.1.1.1     07/06/00 Critical Path   new source drop
**  3    3dfx      1.1.1.0     06/30/00 Critical Path   new source drop
**  2    MacOS Dev Tree1.1         02/03/00 Stephen Luce    Changing line endings
**       from Mac to DOS
**  1    MacOS Dev Tree1.0         01/28/00 Kenneth Dyke    
** $
** 
** 2     7/02/99 3:21p Kcd
** Header change.
**
*/

#include "DCon.h"

#include <3dfx.h>
#include <h3cinitdd_mac.h>
#include <h3cinit.h>
#include <h3regs.h>

#include <minihwc.h>
#include <hwcio.h>

#include "hrm_overlay_surface.h"

/*
________________________________________________________ Private Definitions ___
*/
                          
#define kosName "Overlay Surface"

typedef FxU32 DWORD;

#define GETtheOverlayAddr( x, y, addr)\
    (addr) = ((DWORD)((y) & 0x0fff) << 12)|(DWORD)((x) & 0x0fff)

#pragma mark
#if 0

short             osGetResourceInfo(
                          hrmResourceInfo_T *  outResInfo);

short             osRequestTermination( void );

OSErr             osSetVideoOverlay(
                          hrmMacOverlaySurface_t * inOvl);

void              osSetupVideoOverlaySurface(
                          FxU32                regBase,
                          FxU32                enable,
                          FxU32                stereo,
                          FxU32                leftSrcBaseAddr,
                          FxU32                rightSrcBaseAddr,
                          FxU32                srcStride,
                          FxU32                srcWidth,
                          FxU32                srcHeight,
                          FxU32                dstWidth,
                          FxU32                dstHeight,
                          FxU32                dstTop,
                          FxU32                dstLeft,
                          FxU32                filterMode,
                          FxU32                tiled,
                          FxU32                pixFmt,
                          FxU32                clutBypass,
                          FxU32                clutSelect,
                          FxU32                colorKey);
                          
DWORD             osGetScalingFactor(
                          DWORD                inDwSrc,
                          DWORD                inDwDst,
                          int                  inFracbits);



/*
__________________________________________________ osDispatch_OverlaySurface ___
*/

short
osDispatch_OverlaySurface(
  hrmCommand_t *       ioCmd)
{
  short                theSuccess = -1;
  
  switch( ioCmd->methodSelector ) {
    case HRM_GET_RESOURCE_INFO:
      theSuccess = osGetResourceInfo( &ioCmd->u.resInfo );
      break;
    
    case HRM_REQUEST_TERMINATION:
      theSuccess = osRequestTermination();
      break;
      
    case HRM_OVL_SET_SURFACE:
      theSuccess = osSetVideoOverlay( &ioCmd->u.macOverlaySurface );
      break;

    default:
      dprintf("omDispatch_OverlaySurface(): invalid method selector = %d, ( ioCmd @ 0x%08x )\n", ioCmd->methodSelector, ioCmd);
      break;
  }
  
  return theSuccess;   
}


/*
__________________________________________________________ omGetResourceInfo ___
*/

short
osGetResourceInfo(
  hrmResourceInfo_T *  outResInfo)
{
  short                theSuccess = 0;
  
  dprintf("omGetResourceInfo(): Get HRM_OVERLAY_SURFACE resource info\n");

  outResInfo->ioResourceTag = HRM_OVERLAY_SURFACE;
  outResInfo->outVersion = kHrmVersion;
  strcpy( outResInfo->outName, kosName );
  
  return theSuccess;
}



/*
___________________________________________________ osRequestAllTerminations ___

*/

short
osRequestTermination( void )
{
  return 0;
}


/*
__________________________________________________________ omGetResourceInfo ___
*/

OSErr
osSetVideoOverlay(
  hrmMacOverlaySurface_t * inOvl)
{
  hrmBoard_t *         theBoard;
  FxU32                theFilterMode;
  FxU32                thePixFormat;
  FxU32                theColorKey;
  short                theSuccess = 0;

  theBoard = hrmIdentifyTarget( inOvl->inRefNum );

  if ( theBoard == 0 ) {
      theSuccess = -1;
  }

  switch ( inOvl->inSrcPixFormat ) {
    case HRM_OVERLAY_SRC_PIXEL_RGB565U:
      thePixFormat = SST_OVERLAY_PIXEL_RGB565U;
      break;
      
    case HRM_OVERLAY_SRC_PIXEL_YUV411:
      thePixFormat = SST_OVERLAY_PIXEL_YUV411;
      break;

    case HRM_OVERLAY_SRC_PIXEL_YUYV422:
      thePixFormat = SST_OVERLAY_PIXEL_YUYV422;
      break;

    case HRM_OVERLAY_SRC_PIXEL_UYVY422:
      thePixFormat = SST_OVERLAY_PIXEL_UYVY422;
      break;

    case HRM_OVERLAY_SRC_PIXEL_RGB565D:
      thePixFormat = SST_OVERLAY_PIXEL_RGB565D;
      break;
      
    default:
      theSuccess = -1;
      break;      
  }

  switch ( inOvl->inDstPixFormat ) {
    case HRM_OVERLAY_DST_PIXEL_PAL8:
      theColorKey = inOvl->inColorKey.blue;
      break;
      
    case HRM_OVERLAY_DST_PIXEL_RGB555:
      theColorKey = ((inOvl->inColorKey.red << 10) & 0x7C00 ) |
                      ((inOvl->inColorKey.green << 5) & 0x03E0 ) |
                        ((inOvl->inColorKey.blue >> 3) & 0x001F ) ;
      break;
      
    case HRM_OVERLAY_DST_PIXEL_RGB32:
      theColorKey = ((inOvl->inColorKey.red << 16) & 0xFF0000 ) |
                      ((inOvl->inColorKey.green << 8) & 0xFF00 ) |
                          inOvl->inColorKey.blue;
      break;

    default:
      theSuccess = -1;
      break;      
  }

  switch ( inOvl->inDstFilter ) {
    case HRM_OVERLAY_FILTER_POINT:
      theFilterMode = SST_OVERLAY_FILTER_POINT;
      break;

    case HRM_OVERLAY_FILTER_2X2:
      theFilterMode = SST_OVERLAY_FILTER_2X2;
      break;
      
    case HRM_OVERLAY_FILTER_4X4:
      theFilterMode = SST_OVERLAY_FILTER_4X4;
      break;

    case HRM_OVERLAY_FILTER_BILINEAR:
      theFilterMode = SST_OVERLAY_FILTER_BILINEAR;
      break;

    default:
      theSuccess = -1;
      break;      
  }

  if ( theSuccess == 0 ) {
    osSetupVideoOverlaySurface(  theBoard->boardInfo.regInfo.ioPortBase,
                                 inOvl->inEnable,
                                 inOvl->inIsStereo,
                                 inOvl->inSrcBaseAddr,
                                 inOvl->inSrcRightBaseAddr,
                                 inOvl->inSrcStride,
                                 inOvl->inSrcWidth,
                                 inOvl->inSrcHeight,
                                 inOvl->inDstRect.right - inOvl->inDstRect.left,
                                 inOvl->inDstRect.bottom - inOvl->inDstRect.top,
                                 inOvl->inDstRect.top,
                                 inOvl->inDstRect.left,
                                 theFilterMode,
                                 false,
                                 thePixFormat,
                                 true,
                                 0,
                                 theColorKey );

    if ( !inOvl->inEnable ) {
      inOvl->inSrcRightBaseAddr = inOvl->inSrcBaseAddr = 0;
    }
    
    inOvl->inSrcBaseAddr &= 0x00FFFFFF;
    inOvl->inSrcRightBaseAddr &= 0x00FFFFFF;

    HWC_SST_STORE( theBoard->boardInfo.regInfo, leftOverlayBuf, inOvl->inSrcBaseAddr);
    HWC_SST_STORE( theBoard->boardInfo.regInfo, rightOverlayBuf, inOvl->inSrcRightBaseAddr);
    HWC_SST_STORE( theBoard->boardInfo.regInfo, swapbufferCMD, 0x01);
    HWC_SST_STORE( theBoard->boardInfo.regInfo, swapbufferCMD, 0x01);
    
  }

  return theSuccess;
}


/*
_________________________________________________ osSetupVideoOverlaySurface ___

This code is inspired from "ddovl32.c"


enable,               // 1=enable Overlay surface (OS), 0=disable
stereo,               // 1=enable OS stereo, 0=disable
leftSrcBaseAddr,      // base address for the left overlay buffer (source)
rightSrcBaseAddr,     // base address for the right overlay buffer (unused if not stereo)
srcStride,
srcWidth,             // width of the source overlay buffer in pixels (in overlay surface)
srcHeight,            // height of the source overlay buffer in pixels (in overlay surface)
dstWidth,             // width of the destination overlay buffer in pixels (on desktop surface)
dstHeight,            // height of the destination overlay buffer in pixels (on desktop surface)
dstTop,               // top coordinate of the destination overlay buffer (on desktop surface)
dstLeft,              // left coordinate of the destination overlay buffer (on desktop surface)
filterMode,           //
tiled,                // 0=OS linear, 1=tiled
pixFmt,               // pixel format of OS
clutBypass,           // bypass clut for OS?
clutSelect,           // 0=lower 256 CLUT entries, 1=upper 256

*/

void
osSetupVideoOverlaySurface(
    FxU32 regBase,
    FxU32 enable,
    FxU32 stereo,
    FxU32 /*leftSrcBaseAddr*/,
    FxU32 /*rightSrcBaseAddr*/,
    FxU32 srcStride,
    FxU32 srcWidth,
    FxU32 srcHeight,
    FxU32 dstWidth,
    FxU32 dstHeight,
    FxU32 dstTop,
    FxU32 dstLeft,
    FxU32 filterMode,
    FxU32 tiled,
    FxU32 pixFmt,
    FxU32 clutBypass,
    FxU32 clutSelect,
    FxU32 colorKey)
{
    FxU32 theVidProcCfg = IGET32(vidProcCfg);
 
    theVidProcCfg &= ~(SST_OVERLAY_EN |
                       SST_CHROMA_EN |
                       SST_OVERLAY_TILED_EN |
                       SST_OVERLAY_STEREO_EN |  
                       SST_OVERLAY_HORIZ_SCALE_EN |
                       SST_OVERLAY_VERT_SCALE_EN |
                       SST_OVERLAY_TILED_EN |
                       SST_OVERLAY_PIXEL_FORMAT |
                       SST_OVERLAY_FILTER_MODE |
                       SST_OVERLAY_CLUT_BYPASS |
                       SST_OVERLAY_CLUT_SELECT);
    if (enable) {
        theVidProcCfg |= SST_OVERLAY_EN | SST_CHROMA_EN;
    } else {
        ISET32(vidProcCfg, theVidProcCfg);
        return;
    }
    
    if (stereo)
        theVidProcCfg |= SST_OVERLAY_STEREO_EN;

    if (tiled)
        theVidProcCfg |= SST_OVERLAY_TILED_EN;

    theVidProcCfg |= pixFmt;

    if (clutBypass)
        theVidProcCfg |= SST_OVERLAY_CLUT_BYPASS;

    if (clutSelect)
        theVidProcCfg |= SST_OVERLAY_CLUT_SELECT;

    if (srcWidth < dstWidth) {
        FxU32 theDudx;
        FxU32 theDudxOffsetSrcWidth;
        
        theVidProcCfg |= SST_OVERLAY_HORIZ_SCALE_EN;
        theDudx = osGetScalingFactor( srcWidth, dstWidth, 20);
        if (tiled)
        {
            theDudxOffsetSrcWidth = (((srcWidth * 2) << 7) << SST_OVERLAY_FETCH_SIZE_SHIFT)|(DWORD) 0; //Dudx offset = 0
        }
        else
        {
            theDudxOffsetSrcWidth = ((srcWidth * 2) << SST_OVERLAY_FETCH_SIZE_SHIFT)|(DWORD) 0; //Dudx offset = 0
        }

        ISET32(vidOverlayDudx, theDudx);
        ISET32(vidOverlayDudxOffsetSrcWidth, theDudxOffsetSrcWidth);
    }
    
    if (srcHeight < dstHeight) {
        FxU32 theDvdy;
        
        theVidProcCfg |= SST_OVERLAY_VERT_SCALE_EN;
        theDvdy = osGetScalingFactor(srcHeight, dstHeight, 20);
        ISET32(vidOverlayDvdy, theDvdy);
        ISET32(vidOverlayDvdyOffset, 0);
    }

    theVidProcCfg |= filterMode;

    {
        FxU32 theStride;

        // change only the overlay portion of the vidDesktopOverlayStride register
        //
        theStride = IGET32(vidDesktopOverlayStride);
        theStride &= ~(SST_OVERLAY_LINEAR_STRIDE | SST_OVERLAY_TILE_STRIDE);
        srcStride <<= SST_OVERLAY_STRIDE_SHIFT;
        if (tiled)
            srcStride &= SST_OVERLAY_TILE_STRIDE;
        else
            srcStride &= SST_OVERLAY_LINEAR_STRIDE;
        theStride |= srcStride;

        ISET32(vidDesktopOverlayStride, theStride);
    }

    {
        FxU32 theChromaMin;
        FxU32 theChromaMax;
        
        theChromaMin = theChromaMax = colorKey;
    
        ISET32(vidChromaMin, theChromaMin);
        ISET32(vidChromaMax, theChromaMax);
    }
    
    {
        FxU32 thePosition;
        
        GETtheOverlayAddr( dstLeft, dstTop, thePosition);
        ISET32(vidOverlayStartCoords, thePosition);
        
        GETtheOverlayAddr( dstLeft + dstWidth, dstTop + dstHeight, thePosition);
        ISET32(vidOverlayEndScreenCoord, thePosition);
    }


    ISET32(vidProcCfg, theVidProcCfg);
}

#pragma mark
#endif


/*
_________________________________________________________ osGetScalingFactor ___

This code is straight out of the "ddovl32.c"

*/

UInt32
osGetScalingFactor(
	UInt32			dwSrc,
	UInt32			dwDst,
	UInt32			fracbits,
	double 			&scalingFactor)
{
	float fScale;

	if ((dwSrc >= dwDst) || (dwDst == 0))
	{
		scalingFactor = 1;
		return(0x000fffff); //No scale-up
	}

	fScale = (double)dwSrc/(double)dwDst;
	scalingFactor = fScale;
	fScale *= 1 << fracbits;
	
	return((UInt32)(fScale));
}


/*
_________________________________________________ osSetupVideoOverlaySurface ___

This code is inspired from "ddovl32.c"


enable,               // 1=enable Overlay surface (OS), 0=disable
stereo,               // 1=enable OS stereo, 0=disable
leftSrcBaseAddr,      // base address for the left overlay buffer (source)
rightSrcBaseAddr,     // base address for the right overlay buffer (unused if not stereo)
srcStride,
srcWidth,             // width of the source overlay buffer in pixels (in overlay surface)
srcHeight,            // height of the source overlay buffer in pixels (in overlay surface)
dstWidth,             // width of the destination overlay buffer in pixels (on desktop surface)
dstHeight,            // height of the destination overlay buffer in pixels (on desktop surface)
dstTop,               // top coordinate of the destination overlay buffer (on desktop surface)
dstLeft,              // left coordinate of the destination overlay buffer (on desktop surface)
filterMode,           //
tiled,                // 0=OS linear, 1=tiled
pixFmt,               // pixel format of OS
clutBypass,           // bypass clut for OS?
clutSelect,           // 0=lower 256 CLUT entries, 1=upper 256

*/

void
hrmSetVideoOverlay(
	hrmBoard_t	*theBoard,
	UInt32		enable,
	UInt32		stereo,
	UInt32		leftSrcBaseAddr,
	UInt32		rightSrcBaseAddr,
	UInt32		srcStride,
	UInt32		srcWidth,
	UInt32		srcHeight,
	UInt32		dstWidth,
	UInt32		dstHeight,
	SInt32		dstTop,
	SInt32		dstLeft,
	UInt32		filterMode,
	UInt32		tiled,
	UInt32		pixFmt,
	UInt32		clutBypass,
	UInt32		clutSelect,
	UInt32		colorKey)
{
	UInt32 regBase = theBoard->boardInfo.regInfo.ioPortBase;
	UInt32 theVidProcCfg = IGET32(vidProcCfg);
	theVidProcCfg &= ~(SST_OVERLAY_EN |
	                   SST_CHROMA_EN |
	                   SST_OVERLAY_TILED_EN |
	                   SST_OVERLAY_STEREO_EN |  
	                   SST_OVERLAY_HORIZ_SCALE_EN |
	                   SST_OVERLAY_VERT_SCALE_EN |
	                   SST_OVERLAY_TILED_EN |
	                   SST_OVERLAY_PIXEL_FORMAT |
	                   SST_OVERLAY_FILTER_MODE |
	                   SST_OVERLAY_CLUT_BYPASS |
	                   SST_OVERLAY_CLUT_SELECT);

	// Disable the overlay while we mess with all the registers that control it.
	ISET32(vidProcCfg, theVidProcCfg);

	if (enable) 
	{	    
		// Figure out the pixel size of the overlay source data.
		UInt32 overlayPixelBytes = 2;
		switch(pixFmt)
		{
			case SST_OVERLAY_PIXEL_RGB32U:
				overlayPixelBytes = 4;
			break;
		}
		
	    theVidProcCfg |= SST_OVERLAY_EN | SST_CHROMA_EN;
	    
	    if(theVidProcCfg & SST_VIDEO_2X_MODE_EN)
	    {
	    	// We're running in 2x mode.  Bilinear filtering won't work.
	    	switch(filterMode)
	    	{
	    		case SST_OVERLAY_FILTER_BILINEAR:
	    			filterMode = SST_OVERLAY_FILTER_POINT; 
	    		break;
	    	}
	    }

		if (stereo)
		    theVidProcCfg |= SST_OVERLAY_STEREO_EN;

		if (tiled)
		    theVidProcCfg |= SST_OVERLAY_TILED_EN;

		theVidProcCfg |= pixFmt;

		if (clutBypass)
		    theVidProcCfg |= SST_OVERLAY_CLUT_BYPASS;

		if (clutSelect)
		    theVidProcCfg |= SST_OVERLAY_CLUT_SELECT;

		double xScale = 1, yScale = 1;		// desired x and y scaling factors
		double scaleStart, scaleEnd;		// start and end of the scale in the _source_, including fractional pixel positions.
	    UInt32 overlayStartAdjust = 0;		// overlay start adjust in vidOverlayStartCoords
		UInt32 theDudxOffsetSrcWidth = 0;
		
		if(srcWidth == dstWidth)
		{
			if(dstLeft < 0)
			{
				// The overlay is clipped by the left edge of the screen
				// Fudge it.  Let the stretch case deal with pixel alignment.
				srcWidth -= 2;
				leftSrcBaseAddr += 4;
				rightSrcBaseAddr += 4;
			}
		}

		if(dstWidth <= 0)
		{
			// This is bad, but what can we do about it?
		}
		else if (srcWidth < dstWidth)
		{

			theVidProcCfg |= SST_OVERLAY_HORIZ_SCALE_EN;
			xScale = (double)srcWidth/(double)dstWidth;

			// Start the x offset of the overlay in the middle of the first pixel
			scaleStart = 0;
			
			// and end in the middle of the last pixel
			scaleEnd = srcWidth - 0.5;

			if(dstLeft < 0)
			{
				// The overlay is clipped by the left edge of the screen
				
				// Figure out where the left edge of the overlay needs to fall in the source.
				scaleStart -= dstLeft * xScale;
				
				// Adjust all of the necessary parameters
		    	overlayStartAdjust |= (((UInt32)scaleStart) & 3) << SST_OVERLAY_XADJ_SHIFT;
				leftSrcBaseAddr += (((UInt32)scaleStart) & ~1) << 1;
				rightSrcBaseAddr += (((UInt32)scaleStart) & ~1) << 1;
				
				// If the desired start isn't on an even YUV pixel pair boundary, things get off by one
				// pixel.  Fudge it so that the right edge of the stretch comes out even.
				switch(pixFmt)
				{
					case SST_OVERLAY_PIXEL_YUV411:
					case SST_OVERLAY_PIXEL_YUYV422:
					case SST_OVERLAY_PIXEL_UYVY422:
						if(((UInt32)scaleStart) & 1)
						{
							scaleStart = floor(scaleStart) - 1;
						}
					break;
				}
				
				// Adjust the start and width of the destination to match.
				dstWidth += dstLeft;
				dstLeft = 0;
			}
			Rect deviceRect = ((GDHandle)(theBoard->boardInfo.hMon))[0]->gdRect;
			UInt32 screenWidth = deviceRect.right - deviceRect.left;
			if((dstLeft + dstWidth) > screenWidth)
			{
				// The overlay is clipped by the right edge of the screen.
				scaleEnd -= (dstLeft + dstWidth - screenWidth) * xScale;
				srcWidth -= (dstLeft + dstWidth - screenWidth) * xScale;
				dstWidth -= dstLeft + dstWidth - screenWidth;
			}

			UInt32 theDudx = ((scaleEnd - scaleStart) / dstWidth) * (1 << 20);
			
			if(theDudx & 0xFFF00000)
			{
				// Whoops.  Prevent something really bad from happening.
				theDudx = 0x000fffff;
			}
			
			theDudxOffsetSrcWidth |= ((UInt32)(scaleStart * (1 << 19))) & 0x0007FFFF;

			ISET32(vidOverlayDudx, theDudx);

			// Only set the filter mode if we are actually stretching.
			theVidProcCfg |= filterMode;
		}

			if (tiled)
			{
			    theDudxOffsetSrcWidth |= (((srcWidth * overlayPixelBytes) << 7) << SST_OVERLAY_FETCH_SIZE_SHIFT);
			}
			else
			{
			    theDudxOffsetSrcWidth |= ((srcWidth * overlayPixelBytes) << SST_OVERLAY_FETCH_SIZE_SHIFT);
			}

		// Always write the number of bytes to fetch for each scanline, whether or not
		// we're stretching.
		ISET32(vidOverlayDudxOffsetSrcWidth, theDudxOffsetSrcWidth);

		if(srcHeight == dstHeight)
		{
			if(dstTop < 0)
			{
		    	// The overlay is clipped by the top edge of the screen
		    	overlayStartAdjust |= (((UInt32)-dstTop) & 3) << SST_OVERLAY_YADJ_SHIFT;
		    	leftSrcBaseAddr += ((UInt32)-dstTop) * srcStride;
		    	rightSrcBaseAddr += ((UInt32)-dstTop) * srcStride;
		    	
		    	// Adjust the start and height of the destination to match.
		    	dstHeight += dstTop;
		    	dstTop = 0;
			}
		}
		else if (srcHeight < dstHeight) 
		{
		    UInt32 theDvdyOffset = 0;
		    
		    theVidProcCfg |= SST_OVERLAY_VERT_SCALE_EN;
		    yScale = (double)srcHeight/(double)dstHeight;
		    
			// Start the y offset of the overlay in the middle of the first pixel
			scaleStart = 0;

			// and end in the middle of the last pixel
			scaleEnd = srcHeight - 0.5;

		    if(dstTop < 0)
		    {
		    	// The overlay is clipped by the top edge of the screen
		    	
		    	// Figure out where in the source the left edge of the overlay needs to fall.
		    	scaleStart -= dstTop * yScale;
		    	
		    	// Adjust all of the necessary parameters
		    	overlayStartAdjust |= (((UInt32)scaleStart) & 3) << SST_OVERLAY_YADJ_SHIFT;
		    	leftSrcBaseAddr += ((UInt32)scaleStart) * srcStride;
		    	rightSrcBaseAddr += ((UInt32)scaleStart) * srcStride;
		    	
		    	// Adjust the start and height of the destination to match.
		    	dstHeight += dstTop;
		    	dstTop = 0;
		    }

			UInt32 theDvdy = ((scaleEnd - scaleStart) / dstHeight) * (1 << 20);
			
			if(theDvdy & 0xFFF00000)
			{
				// Whoops.  Prevent something really bad from happening.
				theDvdy = 0x000fffff;
			}
			
	    	theDvdyOffset = ((UInt32)(scaleStart * (1 << 19))) & 0x0007FFFF;
		    
		    ISET32(vidOverlayDvdy, theDvdy);
		    ISET32(vidOverlayDvdyOffset, theDvdyOffset);

			// Only set the filter mode if we are actually stretching.
			theVidProcCfg |= filterMode;
		}


		{
		    UInt32 theStride;

		    // change only the overlay portion of the vidDesktopOverlayStride register
		    //
		    theStride = IGET32(vidDesktopOverlayStride);
		    theStride &= ~(SST_OVERLAY_LINEAR_STRIDE | SST_OVERLAY_TILE_STRIDE);
		    srcStride <<= SST_OVERLAY_STRIDE_SHIFT;
		    if (tiled)
		        srcStride &= SST_OVERLAY_TILE_STRIDE;
		    else
		        srcStride &= SST_OVERLAY_LINEAR_STRIDE;
		    theStride |= srcStride;

		    ISET32(vidDesktopOverlayStride, theStride);
		}

		{
		    UInt32 theChromaMin;
		    UInt32 theChromaMax;
		    
		    theChromaMin = theChromaMax = colorKey;

		    ISET32(vidChromaMin, theChromaMin);
		    ISET32(vidChromaMax, theChromaMax);
		}

		{
		    UInt32 thePosition;
		    
		    GETtheOverlayAddr( dstLeft, dstTop, thePosition);
		    ISET32(vidOverlayStartCoords, thePosition | overlayStartAdjust);
		    
		    GETtheOverlayAddr( dstLeft + dstWidth, dstTop + dstHeight, thePosition);
		    ISET32(vidOverlayEndScreenCoord, thePosition);
		}

	leftSrcBaseAddr &= 0x00FFFFFF;
	rightSrcBaseAddr &= 0x00FFFFFF;

	HWC_SST_STORE( theBoard->boardInfo.regInfo, leftOverlayBuf, leftSrcBaseAddr);
	HWC_SST_STORE( theBoard->boardInfo.regInfo, rightOverlayBuf, rightSrcBaseAddr);
		HWC_SST_STORE( theBoard->boardInfo.regInfo, swapbufferCMD, 0x00);
		HWC_SST_STORE( theBoard->boardInfo.regInfo, swapbufferCMD, 0x00);

	    ISET32(vidProcCfg, theVidProcCfg);
	} 

}

