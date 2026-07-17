/********************************************************************************
	 StdBitsPatch.cp
	 
	 
	 Chall Fry
	 Critical Path Software
	 
*/
 
// Includes
#include <Quickdraw.h>
#include "NQDAcceleration.h"
#include "DynamicPatches.h"
#include "Utilities.h"
#include "DrawPictPatch.h"
 
// Types

typedef struct CopyBitsInfo
{
	// Source pixmap
	PixMap		*srcPixMap;
	UInt32		srcBaseAddr;
	UInt32		srcPixelSize;
	
	// Dest pixmap
	PixMap		*dstPixMap;

} CopyBitsInfo, *CopyBitsInfoPtr;

// Globals
bool		gInStdBitsPatch;

// Memory block used to hold stretch bits source data
mmBlock_t	*gStretchBitsBlock;

// If true, the picture caching code is trying a cache fill
bool		gPictCacheFillAttempt;

// Function Declarations
void SetNQDColors(GDHandle gd, NQDDrawVars &drawVars, bool colorize);
inline UInt32 ColorForDevice(GDHandle gd, RGBColor &color);

// These are in the NQD library.

// Do all the BitMap * decoding that's built into CopyBits and fill out an NQDPixMap accordingly.
extern "C" void	BitsToPix( BitMap * inAnyMap, NQDPixMap * outPixMap, GDHandle scrnDev );

// Create a scaletable for indexed sources.
extern "C" UInt32 *	MakeScaleTable(	NQDPixMapPtr pix, 
									Int32 colorizeFlag, 
									RGBColor * foreColor, 
									RGBColor * backColor, 
									Int32 dstShift );


/********************************************************************************
	StdBitsPatch
		BitMap *srcBits
		Rect *srcRect
		Rect *dstRect
		short mode
		RgnHandle maskRgn
		
	
*/
void StdBitsPatch(BitMap *srcBits, Rect *srcRect, Rect *dstRect,
		short mode, RgnHandle maskRgn)
{
	NQDDrawVars 	drawVars;
	CopyBitsInfo	cbInfo;
	UInt32			deviceHandledMask = 0;
	UInt32			curDeviceBit;
	Boolean			unhandledDevices = false;
	Rect			rgnMinRect;
	bool			colorize = false;
	bool			hStretch = false;
	bool			vStretch = false;
	bool			hShrink = false;
	bool			vShrink = false;
	bool			dither = false;
	bool			ditherFill = false;
	bool			stretch = false;
	
	/* This variable will be set if the destination base address is not equal to the
		base address of the main screen.  This disables the device loop and allows
		dithered or stretched drawing to an offscreen gWorld.
	*/
	bool			cardSpecificDest = false;
	
	// If our patch gets called reentrantly (which can happen), we shouldn't process the
	// parameters twice, however we do need to call through to the OS.
	if (gInStdBitsPatch)
	{
		PatchStdBits::PatchFn(srcBits, srcRect, dstRect, mode, maskRgn);
		return;
	}
	gInStdBitsPatch = true;
	
// These checks are simply for timesaving, as it's faster to make the checks up front.
// The functionality could be moved into the stretch blit accept proc.

	// Check to see if we're turned off
	if (!PatchCommonBlitAccept())
		goto Callthrough;
	
	// Check for ditherCopy
	if (gPictCacheFillAttempt && (mode == ditherCopy))
	{
		// In many cases, we can dither and fill the cache at the same time.
		ditherFill = true;
	}
	
	// Check to see if this is a stretch blit
	if (srcRect->bottom - srcRect->top < dstRect->bottom - dstRect->top)
		vStretch = true;
	else if (srcRect->bottom - srcRect->top > dstRect->bottom - dstRect->top)
		vShrink = true;

	if (srcRect->right - srcRect->left < dstRect->right - dstRect->left)
		hStretch = true;
	else if(srcRect->right - srcRect->left > dstRect->right - dstRect->left)
		hShrink = true;
	
	if (hShrink || vShrink || hStretch || vStretch)
	{
		// This is some form of stretch/shrink blit.
		stretch = true;

		// Check the transfer mode
		if ((mode >= patCopy) && (mode != transparent) && (!ditherFill))
			goto Callthrough;
		
		// For now, don't allow shrinking unless we're filling the picture cache.
		if ((hShrink || vShrink) && !gPictCacheFillAttempt)
		{
			goto Callthrough;
		}		
	}
	
	if (mode == ditherCopy)
	{
		// This is our custom dither blit.
		dither = true;
	}
	
	if (stretch && dither && (!ditherFill))
	{
		// We can't stretch and dither at the same time.
		goto Callthrough;
	}
	
	if (!stretch && !dither)
	{
		// We're not stretching or dithering.  
		
		// If this is an attempt at a cache fill, we still want to try the two-step stretch blit.
		
		if(!gPictCacheFillAttempt)
		{
			// Not trying to fill the picture cache.  let the normal NQD hooks do their work.
			goto Callthrough;
		}
	}

	// Get the dest port
	GetPort((GrafPtr *) &drawVars.port);
	if (!drawVars.port)
		goto Callthrough;
		
	// Bail if we're recording a picture
	if (drawVars.port->picSave)
		goto Callthrough;
		
	// We catch colorization below.

	// Is this a new or old style port?
	if (drawVars.port->portVersion & 0xC000)
	{
		cbInfo.dstPixMap = *drawVars.port->portPixMap;
		
		if (cbInfo.dstPixMap->baseAddr == LMGetMainDevice()[0]->gdPMap[0]->baseAddr)
		{
			// This is a normal blit to the screen
		}
		else if (FindH3Info(cbInfo.dstPixMap->baseAddr) != nil)
		{
			// This is a blit to somewhere on one of our cards
			cardSpecificDest = true;
		}
		else
		{
			// This is not a destination we should deal with
			goto Callthrough;
		}
		
		// Set up the colors for MakeScaleTable
		drawVars.foreRGB = drawVars.port->rgbFgColor;
		drawVars.backRGB = drawVars.port->rgbBkColor;

	} 
	else
	{
		if (((GrafPtr) drawVars.port)->portBits.baseAddr == LMGetMainDevice()[0]->gdPMap[0]->baseAddr)
		{
			// This is a normal blit to the screen
		}
		else if (FindH3Info(((GrafPtr) drawVars.port)->portBits.baseAddr) != nil)
		{
			// This is a blit to somewhere on one of our cards
			cardSpecificDest = true;
		}
		else
		{
			// This is not a destination we should deal with
			goto Callthrough;
		}

		cbInfo.dstPixMap = GetGDevice()[0]->gdPMap[0];
		
		// Set up the colors for MakeScaleTable
		GetForeColor(&drawVars.foreRGB);
		GetBackColor(&drawVars.backRGB);
	}
			
	// Analyze the source bitmap pointer, and determine what kind of bitmap it is.
	BitsToPix( srcBits, &drawVars.srcPixMap, GetGDevice() );
	
	drawVars.mode = drawVars.unmappedMode = mode;
	
	// Check for colorization
	if(dither && !ditherFill)
	{
		// Only dither 32-bit sources.
		if(drawVars.srcPixMap.pixelSize != 32)
			goto Callthrough;

		// Any colorization will kill our dithering code
		if ((drawVars.port->rgbBkColor.red | drawVars.port->rgbBkColor.green | drawVars.port->rgbBkColor.blue) == 0 && 
			(drawVars.port->rgbFgColor.red & drawVars.port->rgbFgColor.green & drawVars.port->rgbFgColor.blue) == 0xFFFF)
		{
			goto Callthrough;
		}	
	}
	else if(mode == transparent)
	{
		// The blit is never colorized in this case.  The colors are only used for
		// transparency determination.  Fall through.
	}
	else
	{
		// Arithmetic transfer modes only.
		
		if(drawVars.srcPixMap.pixelSize == 1)
		{
			// BitBlit handles all colorization of 1-bit sources.
		}
		else
		if ((drawVars.port->rgbFgColor.red | drawVars.port->rgbFgColor.green | drawVars.port->rgbFgColor.blue) == 0 && 
			(drawVars.port->rgbBkColor.red & drawVars.port->rgbBkColor.blue & drawVars.port->rgbBkColor.green) == 0xFFFF)
		{
			// This blit is not colorized; the foreground and background are black and white, respectively.  
			// Fall through.
		}
		else
		if ((drawVars.port->rgbBkColor.red | drawVars.port->rgbBkColor.green | drawVars.port->rgbBkColor.blue) == 0 && 
			(drawVars.port->rgbFgColor.red & drawVars.port->rgbFgColor.green & drawVars.port->rgbFgColor.blue) == 0xFFFF)
		{
			// The blit is inverted; the foreground and background are white and black, respectively.
			// For boolean op transfers, we can handle this by rearranging the modes.
			switch(drawVars.unmappedMode)
			{
				case srcCopy:		drawVars.mode = notSrcCopy;		break;
				case srcOr:			drawVars.mode = srcBic;			break;
				case srcXor:		drawVars.mode = notSrcXor;		break;
				case srcBic:		drawVars.mode = srcOr;			break;
				case notSrcCopy:	drawVars.mode = srcCopy;		break;
				case notSrcOr:		drawVars.mode = notSrcBic;		break;
				case notSrcXor:		drawVars.mode = srcXor;			break;
				case notSrcBic:		drawVars.mode = notSrcOr;		break;
			}
		} 
		else
		{
			// The blit is colorized
			if(drawVars.srcPixMap.pixelSize > 8)
			{
				// The source is a direct pixmap.  We don't have a good way to handle this case.
				goto Callthrough;
			}
			else
			{
				// The source is indexed.  For some transfer modes, we can handle this by 
				// building the scale table properly.
				switch(drawVars.unmappedMode)
				{
					case srcCopy:
					case srcXor:
					case notSrcCopy:
					case notSrcXor:
						colorize = true;
					break;
					
					default:
						goto Callthrough;
					break;
				}
			}
		}

	}		

	// General setup of the NQDDrawVars struct
	drawVars.colorizeFlag = 0;
	drawVars.hasMask = 0;
	drawVars.origSrcRect = *srcRect;
	drawVars.origDstRect = *dstRect;
	drawVars.hBump = 4;
	drawVars.vertDir = 1;
	drawVars.blitProc = 0;
	drawVars.trimResult = 0;
	drawVars.xferSrcShift = drawVars.srcPixMap.shift;

	// Fix the colorize flag for 1-bit sources
	if(drawVars.srcPixMap.pixelSize == 1)
	{
	 	if(	(drawVars.srcPixMap.pmTable != nil) &&
	 		(drawVars.srcPixMap.pmTable[0] != nil) &&
	 		(drawVars.srcPixMap.pmTable[0]->ctSeed != 1))
	 	{
	 		// The mask isn't using the standard 1-bit color table (0 = white, 1 = black).
	 		// Have QuickDraw build a ScaleTable for this blit.
			colorize = true;
	 	}
	 	else
	 	{
			drawVars.colorizeFlag = 1;
		}
	}
	
	// Region
	drawVars.combineMask = 0;
	drawVars.rgnA = maskRgn;
	drawVars.rgnB = drawVars.port->visRgn;
	drawVars.rgnC = drawVars.port->clipRgn;
	
	// Rects
	rgnMinRect = drawVars.origDstRect;
	if (drawVars.rgnA && *drawVars.rgnA)
	{
		if (!FastSectRect(rgnMinRect, drawVars.rgnA[0][0].rgnBBox, rgnMinRect))
			goto Callthrough;
		if (drawVars.rgnA[0][0].rgnSize > 10)
			drawVars.trimResult = 1;
	}
	if (drawVars.rgnB && *drawVars.rgnB)
	{
		if (!FastSectRect(rgnMinRect, drawVars.rgnB[0][0].rgnBBox, rgnMinRect))
			goto Callthrough;
		if (drawVars.rgnB[0][0].rgnSize > 10)
			drawVars.trimResult = 1;
	}
	if (drawVars.rgnC && *drawVars.rgnC)
	{
		if (!FastSectRect(rgnMinRect, drawVars.rgnC[0][0].rgnBBox, rgnMinRect))
			goto Callthrough;
		if (drawVars.rgnC[0][0].rgnSize > 10)
			drawVars.trimResult = 1;
	}	
	
	// Special setup for AcceleratedHostShrinkBlitLL
	// We'll need the original srcBits to put the shrink case on the board.
	// We can use the reserved fields in the drawVars struct because only 
	// our code will ever see this struct.
	drawVars.reserved1 = (UInt32)srcBits;
	
	// Loop through each device in the DeviceList
	GDHandle	gd, oldGD;
	
	oldGD = GetGDevice();
	
	if(cardSpecificDest)
	{
		curDeviceBit = 1;
		gd = oldGD;
	}
	else
	{
		curDeviceBit = 1;
		gd = GetDeviceList();
	}
	
	short origMode = drawVars.mode;
		
	for (; gd != nil; cardSpecificDest ? (gd = nil) : (curDeviceBit <<= 1, gd = GetNextDevice(gd)))
	{
		short dstMode = origMode;
		
		// Is the gDevice active?
		if (gd[0][0].gdFlags & 0x8000)
		{
			// Does this gDevice intersect the destRect?
			Rect deviceRectLocal = gd[0][0].gdRect;
			
			// Offset the device rect into local coordinates
			FastOffsetRect(deviceRectLocal, cbInfo.dstPixMap->bounds.left, cbInfo.dstPixMap->bounds.top);

			if (FastSectRect(rgnMinRect, deviceRectLocal, drawVars.minRect))
			{
				// If we're drawing to this device, but the prefs for this device are turned off,
				// skip it, and remember there's unhandled devices out there.
				// It just so happens that all of the blits done in this patch (stretch and dither)
				//	are potentially non-bit-accurate, so we do both "disabled" checks here.
				h3Info *h3InfoDst = FindH3Info(gd[0][0].gdPMap[0][0].baseAddr);
				if (!h3InfoDst || (gPreferences->m2D[h3InfoDst->prefsIdx]->disableFlags & 
						(kDisableStdBitsPatch | kBitAccurateOnly)))
				{
					unhandledDevices = true;
					continue;
				}
				
				// Set the current GDevice so that Color2Index will work.
				SetGDevice(gd);

				// Set up the dest pixmap
				if (cardSpecificDest)
				{
					BitsToPix( (BitMap*)(cbInfo.dstPixMap), &drawVars.dstPixMap, gd);
				}
				else
				{
				BitsToPix( (BitMap*)(*gd[0][0].gdPMap), &drawVars.dstPixMap, gd);
				}
				
				// The easy way to convert from pixelsize to pixel->byte shift
				drawVars.xferDstShift = drawVars.dstPixMap.shift;

				// Modify the destpixmap's bounds to be local coordinates
				drawVars.dstPixMap.bounds = deviceRectLocal;
				
				// Determine the portion of the source that applies to this dest Rect
				drawVars.srcRect = drawVars.dstRect = drawVars.minRect;
				MapRect(&drawVars.srcRect, &drawVars.origDstRect, &drawVars.origSrcRect);
				
				// Figure out whether this is a ditherFill case we can handle.
				if (ditherFill)
				{
					if (drawVars.dstPixMap.pixelSize < 16)
					{
						// The dither should be visible as such.
													
						if (hStretch || vStretch)
						{
							// In this case, we would dither into the cache and then stretch to the
							// screen, which would look wrong.  Skip this device.
							unhandledDevices = true;
							continue;
						}
					}
					else
					{
						// A dither to this depth has no visible effect.  Strip the dither bit from the mode we use here.
						dstMode &= ~ditherCopy;
					}
				}
				
				drawVars.mode = dstMode;
				
				// Set up the ScaleTable to point to _something_, just in case.
				UInt32 fakeScale[2];
				drawVars.scaleTable = fakeScale;
				
				SetNQDColors(gd, drawVars, colorize);
				
				if (stretch || ditherFill || gPictCacheFillAttempt)
				{
					bool isHostBlit;
					
					// Call the BitBlit proc, as if we were actually NQD making the call.
					if (GetAcceleratedStretchBlitProc(&drawVars, isHostBlit, hShrink || vShrink))
					{
						// Allocate memory to hold the image
						UInt32 imagePixelSize;
						UInt32 imageSize;
						h3Info *h3info = FindH3Info(drawVars.dstPixMap.baseAddr);
						Rect scratchRect = drawVars.srcRect;
						
						if(isHostBlit)
						{
							// Figure out the depth of the scratch space we'll be using
							if(drawVars.srcPixMap.pixelSize == 1)
								imagePixelSize = 1;
							else
								imagePixelSize = drawVars.dstPixMap.pixelSize;
							
							if(gPictCacheFillAttempt)
							{
								// Allocate a block big enough for the entire source, instead of the
								// current srcRect.
								scratchRect = drawVars.srcPixMap.bounds;
								
								// If we're shrinking in x or y, make the scratch the size of the destination in that dimension.
								if(hShrink)
								{
									scratchRect.left = drawVars.origDstRect.left;
									scratchRect.right = drawVars.origDstRect.right;
								}
								if(vShrink)
								{
									scratchRect.top = drawVars.origDstRect.top;
									scratchRect.bottom = drawVars.origDstRect.bottom;
								}
							}
							else
							{
								// If we're shrinking in x or y, make the scratch the size of the destination in that dimension.
								if(hShrink)
								{
									scratchRect.left = drawVars.dstRect.left;
									scratchRect.right = drawVars.dstRect.right;
								}
								if(vShrink)
								{
									scratchRect.top = drawVars.dstRect.top;
									scratchRect.bottom = drawVars.dstRect.bottom;
								}
							}
							
							// If we're shrinking, make sure the scratch space isn't 1-bit.
							if(hShrink || vShrink)
								imagePixelSize = drawVars.dstPixMap.pixelSize;
							
							// Make sure we leave room for possible slop on either side.
							imageSize = scratchRect.right - scratchRect.left;
							imageSize *= imagePixelSize;
							imageSize += 31;	// Round rowbytes up to a long boundary
							imageSize >>= 3;
							imageSize += 4;		// and add one long for good measure
							imageSize *= (scratchRect.bottom - scratchRect.top);
							
							if(gPictCacheFillAttempt)
							{
								if(!gPictCache.current.banded)
								{
									// Picture cache blocks have a different priority and are relocatable.
									gStretchBitsBlock = 
										hrmAllocateBlock(
											h3info->board, 
											imageSize, 
											HRM_MEMF_LINEAR | HRM_MEMF_RELOCATABLE, 
											HRM_MEM_PRI_CACHE);
								}
								else
								{
									// This is not the first band.  The cache block has already been reallocated.
									// Just use it again.
									gStretchBitsBlock = gPictCache.current.cacheBlock;
								}
							}
							else if((h3info->scratchSpace != NULL) &&
								(imageSize <= h3info->scratchSpace->size))
							{
								// If the scratch space is there and is big enough, use it.
								gStretchBitsBlock = h3info->scratchSpace;
							}
							else
							{
								gStretchBitsBlock = hrmAllocateBlock(h3info->board, imageSize, HRM_MEMF_LINEAR, 
										HRM_MEM_PRI_SCRATCH);
							}
						}
												
						if (!isHostBlit || (gStretchBitsBlock != nil))
						{
							// Shield the cursor 
							Point pt = { deviceRectLocal.top, deviceRectLocal.left };
							UInt32 id = NQDProtectCursor(&drawVars.dstRect, pt);
						
							// Call the blit proc, and then the finish proc
							drawVars.blitProc(&drawVars);
							while(!H3BlitFinish(drawVars.refCon))
								;
							
							if(isHostBlit)
							{
								if((gStretchBitsBlock != nil) && (gStretchBitsBlock != h3info->scratchSpace))
								{
									// Deallocate the memory block
									hrmFreeBlock(gStretchBitsBlock);
								}
								gStretchBitsBlock = nil;
							}
														
							// Unshield the cursor
							NQDUnprotectCursor(id);
				
							deviceHandledMask |= curDeviceBit;
						}
					} 
					else
					{
						unhandledDevices = true;
					}
				}
				else
				{
					// If we're not stretching, we'd better be dithering.  If not, our logic is broken.

					if (GetAcceleratedDitherBlitProc(&drawVars))
					{
						// Shield the cursor 
						Point pt = { deviceRectLocal.top, deviceRectLocal.left };
						UInt32 id = NQDProtectCursor(&drawVars.dstRect, pt);
					
						// Call the blit proc, and then the finish proc
						drawVars.blitProc(&drawVars);
						while(!H3BlitFinish(drawVars.refCon))
							;
												
						// Unshield the cursor
						NQDUnprotectCursor(id);
			
						deviceHandledMask |= curDeviceBit;
					} 
					else
					{
						unhandledDevices = true;
					}
				}
			}
		}
	}
	
	SetGDevice(oldGD);
	
	// Determine the method we use for exiting, based on whether we drew the image on all, some, or none of the 
	// displays involved.
	if (cardSpecificDest || !unhandledDevices)
	{
		// We completed the job. Just return to the application
		gInStdBitsPatch = false;
		return;
	} else if (deviceHandledMask)
	{
		// The operation is partially completed. We need to turn off the gDevices we completed on
		// and call through
		for (curDeviceBit = 1, gd = GetDeviceList(); gd != 0; curDeviceBit <<= 1, gd = GetNextDevice(gd))
		{
			// Disable the gDevice
			if (deviceHandledMask & curDeviceBit)
				gd[0][0].gdFlags &= 0x7FFF;
		}
		
		// Call through, so that the OS can handle the operation on devices we can't
		PatchStdBits::PatchFn(srcBits, srcRect, dstRect, mode, maskRgn);
		
		// Now, turn the devices back on
		for (curDeviceBit = 1, gd = GetDeviceList(); gd != 0; curDeviceBit <<= 1, gd = GetNextDevice(gd))
		{
			if (deviceHandledMask & curDeviceBit)
				gd[0][0].gdFlags |= 0x8000;
		}

		gInStdBitsPatch = false;
		return;
	}
	

Callthrough:
	// Generally, if we get here, it means we are choosing to not attempt to handle this QD operation.
	gInStdBitsPatch = false;
	PatchStdBits::PatchFn(srcBits, srcRect, dstRect, mode, maskRgn);
}

/********************************************************************************
	ColorForDevice
		GDHandle gd
		RGBColor &color
		
	Generates a 32-bit word containing the pixel value that corresponds to
	the specified RGB color.  For 8 and 16 bit depths, the pixel value is 
	replicated to fill the word.
	
********************************************************************************/
inline UInt32 ColorForDevice(GDHandle gd, RGBColor &color)
{
	UInt32 result = 0;
	switch(gd[0]->gdPMap[0]->pixelSize)
	{
		case 8:
			result = Color2Index(&color);

			// Replicate the result in all 4 bytes.
			result |= result << 8;
			result |= result << 16;
		break;

		case 16:
			result  = ((UInt32)color.blue  >> 11) & 0x0000001F;
			result |= ((UInt32)color.green >>  6) & 0x000003E0;
			result |= ((UInt32)color.red   >>  1) & 0x00007C00;

			// Replicate in both words.
			result |= result << 16;
		break;

		case 32:
			result  = ((UInt32)color.blue >> 8) & 0x000000FF;
			result |= ((UInt32)color.green    ) & 0x0000FF00;
			result |= ((UInt32)color.red  << 8) & 0x00FF0000;
			
		break;
	}
	
	return(result);
}

/********************************************************************************
	SetNQDColors
		GDHandle gd
		NQDDrawVars &drawVars
		
	
*/
void SetNQDColors(GDHandle gd, NQDDrawVars &drawVars, bool colorize)
{	
	// set foreground and background colors
	drawVars.foreColor = ColorForDevice(gd, drawVars.port->rgbFgColor);
	drawVars.backColor = ColorForDevice(gd, drawVars.port->rgbBkColor);
	
	// Stash the inverse table pointer from the device in drawVars
	drawVars.invTable = NULL;
	if(gd[0]->gdITable && gd[0]->gdITable[0])
	{
		drawVars.invTable = gd[0]->gdITable[0];
	}
	
	// Stash the destination color table in drawVars
	drawVars.colorTable = NULL;
	
	// MBW -- XXX -- Check ctFlags
	if(drawVars.dstPixMap.pmTable && drawVars.dstPixMap.pmTable[0] &&
			(drawVars.dstPixMap.pmTable[0]->ctSize == 255))
		drawVars.colorTable = &(drawVars.dstPixMap.pmTable[0]->ctTable[0]);

	/* Build a scale table if we need one (src depth <= 8 and (dst depth != src 
		depth _or_ ctSeeds don't match _or_ we're colorizing)). This code should 
		also set drawVars->bMustScale to true iff srcDepth == dstDepth and we built 
		a scaletable.
	*/
	
	static RGBColor rgbBlack = {0, 0, 0};
	static RGBColor rgbWhite = {-1, -1, -1};
	
	drawVars.bMustScale = 0;
	if(drawVars.srcPixMap.pixelSize <= 8)
	{
		if( (drawVars.srcPixMap.pixelSize != drawVars.dstPixMap.pixelSize) ||
			(drawVars.srcPixMap.pmTable[0]->ctSeed != drawVars.dstPixMap.pmTable[0]->ctSeed) ||
			colorize)
		{	
			if(drawVars.srcPixMap.pixelSize == drawVars.dstPixMap.pixelSize)
				drawVars.bMustScale = 1;
			
			RGBColor *foreColorize = &rgbBlack;
			RGBColor *backColorize = &rgbWhite;

			if(colorize)
			{
				switch(drawVars.unmappedMode)
				{
					case srcCopy:
						foreColorize = &drawVars.foreRGB;
						backColorize = &drawVars.backRGB;
					break;
					case transparent:
					case srcXor:
						foreColorize = &rgbBlack;
						backColorize = &rgbWhite;
					break;
					case notSrcCopy:
						drawVars.mode = srcCopy;
						foreColorize = &drawVars.backRGB;
						backColorize = &drawVars.foreRGB;
					break;
					case notSrcXor:
						drawVars.mode = srcXor;
						foreColorize = &rgbWhite;
						backColorize = &rgbBlack;
					break;
					
					// MBW -- These four modes, when colorized, actually behave more
					// like arithmetic modes.  I don't think our blitter can actually
					// handle them.
#if 0
					case srcOr:
						foreColorize = &drawVars.foreRGB;
						backColorize = &rgbWhite;
					break;
					case srcBic:
						drawVars.mode = srcOr;
						foreColorize = &drawVars.foreRGB;
						backColorize = &drawVars.backRGB;
					break;
					case notSrcOr:
						drawVars.mode = srcOr;
						foreColorize = &rgbWhite;
						backColorize = &drawVars.foreRGB;
					break;
					case notSrcBic:
						drawVars.mode = srcBic;
						foreColorize = &rgbWhite;
						backColorize = &drawVars.backRGB;
					break;
#endif
				}
			}

			drawVars.scaleTable = MakeScaleTable(	
										&drawVars.srcPixMap, 
										colorize, 
										foreColorize, 
										backColorize, 
										drawVars.dstPixMap.shift );
		}
	}
				
}

