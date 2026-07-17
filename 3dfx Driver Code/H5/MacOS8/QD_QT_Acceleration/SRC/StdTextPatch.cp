/********************************************************************************
	 StdTextPatch.cp
	 
	 	Implements patches on StdText and StdTextMeas, caches fonts on 3dfx cards
	 	and renders the text strings using hardware acceleration.
	 
	 Chall Fry
	 Critical Path Software
	 
*/
 
// Includes
#include <Quickdraw.h>
#include "NQDAcceleration.h"
#include "DynamicPatches.h"
#include "Utilities.h"
#include "RegionParser.h"
#include "FSM.h"

#include "StdTextPatch.h"
#include "BitBlit.h"
 
//#define USE_3D_TEXT 1
 
// Macros
 
#define lmFractEnable *((UInt8 *) 0xBF4)

// Defaults for these are:
//#pragma inline_max_size(256)
//#pragma inline_max_total_size(10000)

#pragma inline_max_size(8192)
#pragma inline_max_total_size(1000000)
 
// Types


// This struct holds information that the text patches need to handle a drawing operation.
// It is generally created in the patch function, and passed into all the subordinate Text
// functions.
class StdTextInfo
{
public:
	static const	UInt32	kMaxStringSize = 512;

	// The arguments sent into StdText or StdTextMeas
	short		byteCount;
	UInt8		*textBuf;
	Point		origNumer;
	Point		origDenom;

	Fixed		matchHorizScale;
	Fixed		matchVertScale;
	Fixed		horizScaleFactor;

	// The font in the font cache we're using to do the drawing
	FontData	*curFont;
	
	// For AA text, the block that holds the antialiased text string, ready for blitting onscreen
	mmBlock_t	*aaBlitterBlock;
	UInt32		aaBlitterBlockRowBytes;
	UInt32		aaBlitterCmdValue;
	
	// Locations for drawing each character in the string
	SInt16		charPositions[kMaxStringSize];

	// The destination drawing surface, in QD terms (not including stuff about gDevice, or hardware)
	CGrafPort	*curPort;		// The port being drawn into
	BitMap		*portPixMap;	// The bitmap or pixmap pointed to by the port (usually points to the
								// main screen)
	PixMap		*curGDPixMap;	// The pixmap of the gDevice currently being drawn
	Rect		textRect;		// Bounding box for the glyphs
	Rect		destRect;		// Bounding box for drawing

	// The device currently being being drawn to; changes as we loop through the devices
	h3Info		*h3InfoPtr;	
	UInt32		fontBlockAddr;	// Address of the font block on current board
	UInt32		foreColor;
	UInt32 		backColor;
	UInt32		commandVal;
	
	
	bool		isRendering;	// TRUE if we are rendering the text characters; false if only measuring
	bool		isOffscreen;	// TRUE if drawing to offscreen gWorld
};

// Globals
UInt32		gInTextPatches = 0;

FontCache	gFonts;

GrafPort	gFontPort;
CGrafPtr	gColorFontPort;

// Function Declarations
short 		StdTextMeasPatch2(short byteCount, UInt8 *textBuf, Point *numer, 
					Point *denom, FontInfo *info);
void 		StdTextPatch2(short byteCount, UInt8 *textBuf, Point numer, Point denom);

Boolean 	StdTextHandleDevice(StdTextInfo	&info, GDHandle gd, Rect &deviceDestRect);
void 		GetQDColors(StdTextInfo &info, GDHandle gd);
UInt32		ColorForDevice(GDHandle gd, RGBColor &color);
template <class variant> Boolean StdTextAAPreDrawFn(StdTextInfo &info);
template <class variant> void StdTextDrawFn(void *blitData, Rect &rect);
template <class variant> void StdTextAADrawFn(void *blitData, Rect &rect);
void 		StdText3StepDrawFn(void *blitData, Rect &rect);
UInt32		CheckFontBlock(StdTextInfo &info);
void 		FontBlockNotifyProc(struct mmBlock_s *block, unsigned long code);

void 		StdTextQDDrawFn(void *blitData, Rect &rect);


/********************************************************************************
	TextCommonAccept
		short byteCount
		UInt8 *textBuf
		
	Performs basic tests to see if we can accelerate this text drawing operation. This
	includes checking arguments to the patch, inspecting the port, and finding/creating
	a cached font strike.
*/
inline Boolean	TextCommonAccept(StdTextInfo &info)
{
// Phase 1. Checks that don't require a grafport or pixmap

	if (!PatchCommonBlitAccept())
		return false;
	
	if (info.byteCount == 0 || (info.byteCount == 1 && !*info.textBuf) ||
			info.byteCount > StdTextInfo::kMaxStringSize)
		Punt(0);
		
	// Check to see if Aldus has modified the global width table
	if ((** (SInt32 **) 0xB10) == 'ALD5')
		Punt(0);
	
// Phase 2. Checks that require a grafport and pixmap
	GetPort((GrafPtr *) &info.curPort);
	if (!info.curPort)
		Punt(0);
		
	// Check the font's size. We can't handle fonts larger than about 100 points.
	UInt32 textSize = info.curPort->txSize;
	if (!textSize)
		textSize = GetDefFontSize();
	if (info.curPort->txSize > 100)
		Punt(0);
		
#ifndef USE_3D_TEXT
	// Check for antialiasing.
	if (textSize >= 12)
	{
		// Check for MacBench. This test looks for the file length of the current app's resource file.
		FCBRecPtr fcb;
		UTResolveFCB(LMGetCurApRefNum(), &fcb);
		if (fcb->fcbEOF < 5152300 || fcb->fcbEOF > 5152600)
		{
			// Not Macbench. The antialiasing test applies.  0x15E is a low-memory global used by the font manager.
			if (!((* (char *) 0x15E) & 0x02))
				return false;
		}
	}
#else
	SInt16 minSize;
	if (IsAntiAliasedTextEnabled(&minSize) && minSize <= textSize)
	{
		// AA text only handles certain modes.
		UInt32 mode = info.curPort->txMode;
		
		switch(mode)
		{
			case srcCopy:
			case srcOr:
			case srcXor:
			case srcBic:
			case notSrcCopy:
			case notSrcOr:
			case notSrcXor:
			case notSrcBic:
				// We can handle these cases
			break;
			
			default:
				// We can't handle anything else
				return false;
			break;
		}
	}
#endif
	
	// Get a pointer to the pixmap
	if (info.curPort->portVersion & 0xC000)
		info.portPixMap = (BitMap *) *info.curPort->portPixMap;
	else
		info.portPixMap = &((GrafPtr) info.curPort)->portBits;
		 
	// Is the text going to be drawn? Also, we can't insert ourselves into picture recordings.
	if (info.curPort->pnVis < 0 || info.curPort->picSave)
		Punt(0);
	
	if ((UInt32) info.portPixMap->baseAddr == (UInt32) LMGetMainDevice()[0][0].gdPMap[0][0].baseAddr)
	{
		// This text is going onscreen
		info.isOffscreen = false;
	} else if (info.h3InfoPtr = FindH3Info(info.portPixMap->baseAddr))
	{
		// This text is going offscreen, to a gworld that's in VRAM.
		info.isOffscreen = true;
	} else
		Punt(0);
		
	// Check the drawing mode
	// rcf Should be able to handle more modes
	if (info.curPort->txMode > 15 && info.curPort->txMode != grayishTextOr)
		Punt(0);
		
// Phase 3. Deal with the scaling factors.
	
	if (info.origNumer.h == info.origDenom.h)
	{
		info.horizScaleFactor = 1 << 16;
		info.matchHorizScale = 1 << 16;
	} else
	{
		// The horizScaleFactor is a precise horiz scaler for calculating character positions
		info.horizScaleFactor = ((info.origNumer.h << 16) / info.origDenom.h);

		// matchHorizScale and matchVertScale are imprecise measures of the scaling factors, used for
		// matching fonts 
		info.matchHorizScale = (info.horizScaleFactor + 0x1000) & 0xFFFFE000;
	}
	
	if (info.origNumer.v == info.origDenom.v)
	{
		info.matchVertScale = 1 << 16;
	} else
	{
		info.matchVertScale = (((info.origNumer.v << 16) / info.origDenom.v) + 0x1000) & 0xFFFFE000;
	}

// Phase 4: Find (or create) a font to do the drawing.
	if (!(info.curFont = gFonts.GetFont(info)))
		Punt(0);
		
	return true;
}

/********************************************************************************
	StdTextMeasPatch
		short byteCount
		UInt8 *textBuf
		Point *numer
		Point *denom
		FontInfo *info
		
	This is a patch on _StdTextMeas. The patch's intent is to replace the built-in
	functionality of StdTextMeas when asked to measure text that the StdText patch
	would choose to draw.
*/
short StdTextMeasPatch(short byteCount, UInt8 *textBuf, Point *numer, 
					Point *denom, FontInfo *fInfo)
{
	Fixed		pixelWidth;
	StdTextInfo	info;
	
	if (gInTextPatches)
		goto NotReentrant;
	gInTextPatches = 1;
	
	info.byteCount = byteCount;
	info.textBuf = textBuf;
	info.origNumer = *numer;
	info.origDenom = *denom;
	info.isRendering = false;
	
	// Make basic checks to see if we can do this operation
	if (!TextCommonAccept(info))
		goto Exit;
		
	*fInfo = info.curFont->fInfo;
	
	if (info.horizScaleFactor != 1)
	{
		fInfo->widMax = fInfo->widMax * info.horizScaleFactor >> 16;
	}
	if (info.matchVertScale != 1)
	{
		fInfo->ascent = fInfo->ascent * info.matchVertScale >> 16;
		fInfo->descent = fInfo->descent * info.matchVertScale >> 16;
		fInfo->leading = fInfo->leading * info.matchVertScale >> 16;
	}
	
	// Calculate the advance width of the text string
	pixelWidth = info.curFont->MeasureTextWidth(info);
	
	// The private area of QDGlobals has a field where the width of the text
	// string needs to be stored as a Fixed.
	char **currentA5Ptr;
	if (currentA5Ptr = (char **) LMGetCurrentA5())
	{
		*(SInt32 *) ((*currentA5Ptr) - sizeof(QDGlobals) + 0x22) = pixelWidth - 0x8000;
	}

	gInTextPatches = 0;
	return pixelWidth >> 16;	
	
Exit:
	gInTextPatches = 0;

NotReentrant:	
	return PatchStdTextMeas::PatchFn(byteCount, textBuf, numer, denom, fInfo);
}


/********************************************************************************
	StdTextPatch
		short byteCount
		UInt8 *textBuf
		Point numer
		Point denom
		
	This is a patch on _StdText. The patch looks for text drawing operations that can
	be accelerated, and handles the drawing of the text in those instances. Note that
	this function will in some instances replace the OS functionality (that is, do all
	the drawing, and never pass the call on through to the OS) and in some instances
	partially replace the OS by handling the drawing on some monitors and not on others.
	
	This function:
		Checks to see if the draw operation can be accelerated by us
		Measures the width of the text, and determines the containment rect for drawing
		Loops through all the devices in the system, drawing the text on all 3dfx cards
		Calls through to the OS if necessary, to draw the text to non-3dfx cards
*/
void StdTextPatch(short byteCount, UInt8 *textBuf, Point numer, Point denom)
{
	StdTextInfo	info;
	Rect		deviceDestRect;
	UInt32		deviceHandledMask = 0;
	UInt32		curDeviceBit;
	GDHandle	gd;
	bool		unhandledDevices = false;

	if (gInTextPatches)
		goto NotReentrant;
	gInTextPatches = 2;

	info.byteCount = byteCount;
	info.textBuf = textBuf;
	info.origNumer = numer;
	info.origDenom = denom;
	info.isRendering = true;
	
	// Make basic checks to see if we can do this operation
	if (!TextCommonAccept(info))
		goto Exit;

// rcf
//	Debugger();

	// Calculate the advance width of the text string
	SInt32 pixelWidth = info.curFont->MeasureTextWidth(info) >> 16;

	// Determine the text rect based on the width and the current pen position
	info.textRect.top = info.curPort->pnLoc.v - info.curFont->ascent;
	info.textRect.left = info.curPort->pnLoc.h;
	info.textRect.bottom = info.curPort->pnLoc.v + info.curFont->descent;
	
	info.textRect.right = info.textRect.left + pixelWidth;
	
	// Adjust for a first character with a negative LSB.
	CharMetrics metrics = info.curFont->metrics[textBuf[0]];	
	if (metrics.GetLeft() < 0)
		info.textRect.left += metrics.GetLeft();

	// Adjust the text rect for the glyph width of the last characters, which may
	// be larger than their advance width
	
	metrics = info.curFont->metrics[textBuf[byteCount - 1]];
	SInt32 glyphEdge = info.curPort->pnLoc.h + info.charPositions[byteCount - 1] + metrics.GetGlyph();

	// Characters in a bitmap font with a positive LSB work a little differently
	if((info.curFont->fontFlags & kFontIsOutline) == 0)
	{
		if(metrics.GetLeft() > 0)
			glyphEdge -= metrics.GetLeft();
	}
		
	if(glyphEdge > info.textRect.right)
	{
		info.textRect.right = glyphEdge;
	}
		
	if (byteCount > 1)
	{
		metrics = info.curFont->metrics[textBuf[byteCount - 2]];
		glyphEdge = info.curPort->pnLoc.h + info.charPositions[byteCount - 2] + metrics.GetGlyph();

		// Characters in a bitmap font with a positive LSB work a little differently
		if((info.curFont->fontFlags & kFontIsOutline) == 0)
		{
			if(metrics.GetLeft() > 0)
				glyphEdge -= metrics.GetLeft();
		}

		if (glyphEdge > info.textRect.right)
			info.textRect.right = glyphEdge;
	}
	
	info.destRect = info.textRect;

	// MBW -- XXX -- I believe this is not necessary.
#if 0	
	// Set the dest rect to be outset one pixel from the text rect. This is done to include
	// the 'whitespace' area around the text
	if (info.curFont->fInfo.leading >= 2)
	{
		FastSetRect(info.destRect, info.textRect.top - 1, info.textRect.left - 1,
				info.textRect.bottom + 1, info.textRect.right + 1);
	} else
	{
		FastSetRect(info.destRect, info.textRect.top, info.textRect.left - 1,
				info.textRect.bottom, info.textRect.right + 1);
	}
#endif
	
	if (info.isOffscreen)
	{
		CGrafPtr	port;
		GetGWorld(&port, &gd);		
		if (FastSectRect(info.destRect, info.portPixMap->bounds, deviceDestRect))
			unhandledDevices = !StdTextHandleDevice(info, gd, deviceDestRect);
	} else
	{
	// Loop through the all display devices
	GDHandle	saveGD = GetGDevice();
	for (curDeviceBit = 1, gd = GetDeviceList(); gd != 0; curDeviceBit <<= 1, gd = GetNextDevice(gd))
	{
		// Is the gDevice active?
		if (gd[0][0].gdFlags & 0x8000)
		{
			// Get the device bounds Rect, and offset it into local coordinates
			Rect deviceRectLocal = gd[0][0].gdRect;
			FastOffsetRect(deviceRectLocal, info.portPixMap->bounds.left, info.portPixMap->bounds.top);

			// Does this gDevice intersect the dest Rect?
			if (FastSectRect(info.destRect, deviceRectLocal, deviceDestRect))
			{
					// Attempt to draw the portion of the text string that intersects the current
					// gDevice. Note that we don't yet know if h3InfoPtr is nonzero.
				info.h3InfoPtr = FindH3Info(gd[0][0].gdPMap[0][0].baseAddr);
					if (!StdTextHandleDevice(info, gd, deviceDestRect))
				{
					// There are gDevices that intersect the dest rect that we can't draw to.
					unhandledDevices = true;
					continue;
				}
				
				deviceHandledMask |= curDeviceBit;
			}
		}
	}
	SetGDevice(saveGD);
	}

	if (!unhandledDevices)
	{
		// We handled the entire operation. Now, we need to advance the pen position
		info.curPort->pnLoc.h += pixelWidth;
		gInTextPatches = 0;
		return;
	} else if (deviceHandledMask)
	{
		// There were both handled and unhandled devices. Turn off the 3dfx devices
		for (curDeviceBit = 1, gd = GetDeviceList(); gd != 0; curDeviceBit <<= 1, gd = GetNextDevice(gd))
		{
			// Disable the gDevice
			if (deviceHandledMask & curDeviceBit)
				gd[0][0].gdFlags &= 0x7FFF;
		}
		
		// Now, call through to Quickdraw, which will draw on the non-3dfx screens.
		PatchStdText::PatchFn(byteCount, textBuf, numer, denom);
		
		// Turn or gDevices back on, and exit
		// Now, turn the devices back on
		for (curDeviceBit = 1, gd = GetDeviceList(); gd != 0; curDeviceBit <<= 1, gd = GetNextDevice(gd))
		{
			if (deviceHandledMask & curDeviceBit)
				gd[0][0].gdFlags |= 0x8000;
		}
		gInTextPatches = 0;		
		return;		
	}
		
Exit:
	gInTextPatches = 0;
	
NotReentrant:
	PatchStdText::PatchFn(byteCount, textBuf, numer, denom);
}

/********************************************************************************
	StdTextHandleDevice
		StdTextInfo	&info
		GDHandle gd
		Rect &deviceDestRect
		
	
*/
Boolean StdTextHandleDevice(StdTextInfo	&info, GDHandle gd, Rect &deviceDestRect)
{
	bool			doInvert;
	RgnParserParams params;

	// Check that we can draw to this card
	if (!info.h3InfoPtr || info.h3InfoPtr->fifo->exclusiveMode ||
			!(info.fontBlockAddr = CheckFontBlock(info)))
		return false;
	
	// Check the preferences
	if (gPreferences->m2D[info.h3InfoPtr->prefsIdx]->disableFlags & kDisableTextPatches)
		return false;
	
	// Check that all the characters used in this string are rendered into the cache
	for (UInt32 index = 0; index < info.byteCount; ++index)
		info.curFont->CheckCharRendered(info, info.textBuf[index]);
	
	// Set the current GDevice so that Color2Index will work.
	info.curGDPixMap = *gd[0][0].gdPMap;
	SetGDevice(gd);
	GetQDColors(info, gd);
	UInt32 mode = info.curPort->txMode & 7;
	info.commandVal = ModeToROP1(mode, info.foreColor, info.backColor, doInvert);
	info.commandVal |= SSTG_TRANSPARENT;
	
	if (mode == notSrcXor)
		info.commandVal = gModeToRop1[srcXor];
			
	// rcf Set blitFunction to be the antialiased blit here
#ifdef USE_3D_TEXT
	info.aaBlitterBlock = 0;
	if (info.curFont->fontFlags & kFontIsAntiAliased)
	{
		switch(mode)
		{
			case srcCopy:
			case srcOr:
			case srcXor:	
			case srcBic:
			case notSrcCopy:
			case notSrcOr:
			case notSrcXor:	
			case notSrcBic:
				// We can handle these cases
			break;
			
			default:
				// We can't handle anything else
				return false;
			break;
		}
		
		// Antialiasing to an 8bpp destination is a nightmare. The 3D hardware can't do it.
		if (info.h3InfoPtr->depth < 16)
			return false;
		else if (info.h3InfoPtr->depth == 16)
		{
			if (!StdTextAAPreDrawFn<variantDst16>(info))
				return false;
			params.blitFunction = StdTextAADrawFn<variantDst16>;
		}
		else 
		{
			if (!StdTextAAPreDrawFn<variantDst32>(info))
				return false;
			params.blitFunction = StdTextAADrawFn<variantDst32>;
		}
	} else
#endif
	{
		// Need to use the 3 step process on modes where we'd need to invert
		if (doInvert)
		{
			if (!CheckScratchSpace(info.h3InfoPtr))
				return false;
			
			params.blitFunction = StdText3StepDrawFn;
		} else
		{
			if (info.h3InfoPtr->depth < 16)
				params.blitFunction = StdTextDrawFn<variantDst8>;
			else if (info.h3InfoPtr->depth == 16)
				params.blitFunction = StdTextDrawFn<variantDst16>;
			else
				params.blitFunction = StdTextDrawFn<variantDst32>;
		}
	}
	
	// Shield the cursor 
	if (!info.isOffscreen)
	{
		Point pt = { info.portPixMap->bounds.top, info.portPixMap->bounds.left };
		ShieldCursor(&deviceDestRect, pt);
	}
	
	GetArbitration(info.h3InfoPtr);
	
	// Call the region parser, which will in turn call our draw fn
	params.blitDataPtr = &info;
	params.parseTopToBottom = true;
	params.parseLeftToRight = true;
	RegionParserDirect(params, info.curPort->visRgn, info.curPort->clipRgn, 
			0, deviceDestRect);
	
	// Call the finish proc, and then unshield the cursor
	while (!H3BlitFinish((long) info.h3InfoPtr)) ;
	
#ifdef USE_3D_TEXT
	if ((info.curFont->fontFlags & kFontIsAntiAliased) && (info.aaBlitterBlock != nil))
	{
		hrmFreeBlock(info.aaBlitterBlock);
		info.aaBlitterBlock = nil;
	}
#endif

	if (!info.isOffscreen)
		ShowCursor();
	
	return true;
}

#pragma mark -
#pragma mark ¥ Drawing Functions

#if 0
/********************************************************************************
	StdTextQDDrawFn
		void *blitData
		Rect &rect
		
	For testing purposes only.
*/
void StdTextQDDrawFn(void *blitData, Rect &rect)
{
	StdTextInfo	&info = *(StdTextInfo *) blitData;
	FontData	&curFont = *info.curFont;
	
	FontInfo fInfo;
	StdTxMeas(info.byteCount, info.textBuf, &info.origNumer, &info.origDenom, &fInfo);

	PixMap	pix = **gColorFontPort->portPixMap;
	pix.baseAddr = (char *) info.fontBlockAddr;
	pix.rowBytes = info.curFont->imageRowBytes | 0x8000;

#if 0
	Rect srcBounds = {0, 0, 32000, 32000 };
	pix.bounds = srcBounds;
	pix.pmVersion = 1;
	pix.packType = 0;
	pix.hRes = pix.vRes = 72.0;
	pix.pixelType = k8IndexedPixelFormat;
	pix.pixelSize = 8;
	pix.cmpCount = 1;
	pix.cmpSize = 8;
	pix.planeBytes = 0;
	pix.pmTable = gColorFontPort->portPixMap[0][0].pmTable;
#endif	

	Rect srcRect;
	Rect dstRect = info.textRect;
	UInt32 charHeight = (curFont.ascent + curFont.descent);

	for (UInt32 count = 0; count < info.byteCount; ++count)
	{
		char	ch = info.textBuf[count];
		srcRect.top = (ch / 16) * charHeight;
		srcRect.bottom = srcRect.top + charHeight;
		srcRect.left = (ch % 16) * curFont.widMax;
		srcRect.right = srcRect.left + curFont.widMax;
		
		dstRect.left = info.textRect.left + info.charPositions[count];
		dstRect.right = dstRect.left + curFont.widMax;
		CopyBits((BitMap *) &pix, info.portPixMap, &srcRect, &dstRect, srcCopy, 0);
	}	
}
#endif

/********************************************************************************
	StdTextAAPreDrawFn
		void *blitData
		
	
*/

template <class variant>
Boolean StdTextAAPreDrawFn(StdTextInfo	&info)
{
	FontData	&curFont = *info.curFont;
	Fifo2DRegs	blitter(info.h3InfoPtr);
	Fifo3DRegs 	blitter3(info.h3InfoPtr, variant::dstPixelSize);
	UInt32		destAddr;
	UInt32		destRowBytes;
	Rect		rect = info.textRect;
	UInt32		mode = info.curPort->txMode & 7;
	bool		invertSouce = false;
	
	enum
	{
		drawCopy,
		drawOr,
		drawXor		
	} drawAlgorithm = drawOr;
	
	const UInt32 destPixToByteShift = variant::dstPixelSize >> 4;

	// Translate the textRect to the actual drawn area
	// MBW -- XXX -- Is this correct?
	rect.left--;
	rect.top--;
	rect.right++;
	rect.bottom++;
	
	UInt32 foreground3d = info.foreColor;
	
	if(variant::dstPixelSize == 16)
	{
		// We need a 32-bit version of the foreground color to pass to the 3d engine.		
		foreground3d = __rlwinm(			  info.foreColor, 17-8,   8, 12);
		foreground3d = __rlwimi(foreground3d, info.foreColor, 17-13, 13, 15);
		foreground3d = __rlwimi(foreground3d, info.foreColor, 22-16, 16, 20);
		foreground3d = __rlwimi(foreground3d, info.foreColor, 22-21, 21, 23);
		foreground3d = __rlwimi(foreground3d, info.foreColor, 27-24, 24, 28);
		foreground3d = __rlwimi(foreground3d, info.foreColor,    30, 29, 31);
	}
	
	// Note: For some modes, the foreground and background colors need to be swapped.
	// 		This is already taken care of by ModeToROP before this function is called.
	switch(mode)
	{
		case srcCopy:
			drawAlgorithm = drawCopy;
		break;
		case srcOr:
		break;
		case srcXor:
			drawAlgorithm = drawXor;	
		break;
		case srcBic:
		break;
		case notSrcCopy:
			drawAlgorithm = drawCopy;
		break;
		case notSrcOr:
			invertSouce = true;
		break;
		case notSrcXor:
			drawAlgorithm = drawXor;	
			invertSouce = true;
		break;
		case notSrcBic:
			invertSouce = true;
		break;
	}
	
	if (info.isOffscreen)
	{
		// Offset the rect to 'global' coordinates; so that the pixel at baseAddr is { 0, 0 }.
		FastOffsetRect(rect, - info.portPixMap->bounds.left, - info.portPixMap->bounds.top);

		// If we're drawing to an offscreen gWorld, the portPixMap has the correct base address.
		destAddr = (UInt32) info.portPixMap->baseAddr & kBaseAddrOffsetMask; 
		destRowBytes = info.portPixMap->rowBytes & 0x3FFF;
	} else 
	{
		// Offset the rect to be relative to the topleft of the current gDevice. Note
		// that in the case of monitor mirroring, this may not be the topleft of the screen.
		FastOffsetRect(rect, - (info.curGDPixMap->bounds.left + info.portPixMap->bounds.left),
				- (info.curGDPixMap->bounds.top + info.portPixMap->bounds.top));

		// For onscreen drawing, the gDevice has the correct baseAddr
		destAddr =  (UInt32) info.curGDPixMap->baseAddr & kBaseAddrOffsetMask; 	
		destRowBytes = info.curGDPixMap->rowBytes & 0x3FFF;
	}
	
	// Calcuate the byte address of the topleft of the dest rect. This allows us to keep srcXY and dstXY
	// set to 0 for all the blits.
	destAddr += (rect.top * destRowBytes) + (rect.left << destPixToByteShift);

	// Step 1. Allocate and prepare buffers. 
	
	// These blocks must be of SCRATCH priority; we can't have them bumping out the font
	// cache block we're using.
	UInt32 alphaRowBytes = 1 << (32 - __cntlzw(rect.right - rect.left));
	
	// Just in case we somehow get something higher than it is wide...
	{
		UInt32 height = 1 << (32 - __cntlzw(rect.bottom - rect.top));
		if(height > alphaRowBytes)
			alphaRowBytes = height;
	}
	
	// Using textures smaller than 256 wide complicates the address calculations,
	// so we always use a texture at least that big.
	if (alphaRowBytes < 256)
		alphaRowBytes = 256;
		
	UInt32 bufferWidth = rect.right - rect.left;
	UInt32 bufferHeight = rect.bottom - rect.top;
	UInt32 bufferRowBytes = ((bufferWidth + 31) & ~31) << destPixToByteShift;
	
	// Round up to cache line alignment.  (may not be strictly necessary, but seems like a good idea.)
	UInt32 bufferSize = ((bufferRowBytes * bufferHeight) + 31) & ~31;
	UInt32 alphaSize = ((alphaRowBytes * bufferHeight) + 31) & ~31;
	UInt32 altSize = 0;
	UInt32 altRowBytes = 0;

	if(drawAlgorithm == drawOr)
	{
		// use the alt buffer for compositing
		altRowBytes = bufferRowBytes;
		altSize = bufferSize;
	}
	else if(drawAlgorithm == drawXor)
	{
		// Use the alt buffer as the source color (other) texture.
		altRowBytes = alphaRowBytes << destPixToByteShift;
		altSize = ((altRowBytes * bufferHeight) + 31) & ~31;
	}

	UInt32 bufferBase = nil;
	if(alphaSize + bufferSize + altSize <= kQDScratchSize)
	{
		// Use the QD scratch
		if(!CheckScratchSpace(info.h3InfoPtr))
		{
			return(false);
		}
		
		bufferBase = info.h3InfoPtr->scratchSpace->start;
		
		// We don't need to deallocate anything when we're done.
		info.aaBlitterBlock = nil;
	}
	else
	{
		// Too big for QD scratch.  Allocate a new block just for this blit.
		info.aaBlitterBlock = hrmAllocateBlock(
					info.h3InfoPtr->board, 
					alphaSize + bufferSize + altSize,
					HRM_MEMF_LINEAR, 
					HRM_MEM_PRI_SCRATCH);
		
		if(info.aaBlitterBlock == nil)
			return(false);

		bufferBase = info.aaBlitterBlock->start;
	}
		
	bufferBase &= kBaseAddrOffsetMask;

	UInt32 altBase = bufferBase + bufferSize;
	UInt32 alphaBase = altBase + altSize;
	
	if(drawAlgorithm == drawXor)
	{
		// Copy the destination pixels to the main buffer.
		blitter.reg(reg_srcBaseAddr, destAddr);
		blitter.reg(reg_srcFormat, destRowBytes | variant::dstPixFmt);
		blitter.reg(reg_srcXY, 0 | (0 << 16));
		blitter.reg(reg_dstBaseAddr, bufferBase);
		blitter.reg(reg_dstFormat, bufferRowBytes | variant::dstPixFmt);
		blitter.reg(reg_dstXY, 0 | (0 << 16));
		blitter.reg(reg_dstSize, bufferWidth | (bufferHeight << 16)); 
		blitter.reg(reg_command, SSTG_BLT | SSTG_GO | (kROPSource));
		blitter.go(variant::apertureDepth);
	}
	else // if((drawAlgorithm == drawOr) || (drawAlgorithm == drawSrc))
	{
		// Fill the main buffer with the background color
		blitter.reg(reg_colorFore, info.backColor);
		blitter.reg(reg_dstBaseAddr, bufferBase);
		blitter.reg(reg_dstFormat, bufferRowBytes | variant::dstPixFmt);	
		blitter.reg(reg_dstXY, 0 | (0 << 16));
		blitter.reg(reg_dstSize, bufferWidth | (bufferHeight << 16)); 
		blitter.reg(reg_command, SSTG_RECTFILL | SSTG_GO | (kROPSource));
		blitter.go(variant::apertureDepth);
	}
	
	if((drawAlgorithm == drawOr) || (drawAlgorithm == drawXor))
	{
		// Copy the destination pixels into the alt buffer
		blitter.reg(reg_srcBaseAddr, destAddr);
		blitter.reg(reg_srcFormat, destRowBytes | variant::dstPixFmt);
		blitter.reg(reg_srcXY, 0 | (0 << 16));
		blitter.reg(reg_dstBaseAddr, altBase);
		blitter.reg(reg_dstFormat, altRowBytes | variant::dstPixFmt);
		blitter.reg(reg_dstXY, 0 | (0 << 16));
		blitter.reg(reg_dstSize, bufferWidth | (bufferHeight << 16)); 
		blitter.reg(reg_command, SSTG_BLT | SSTG_GO | (kROPSource));
		blitter.go(variant::apertureDepth);
	}

	// Step 2. Use the 2D engine to composite all the characters into the 8bpp alpha buffer.
	
	// Fill the alpha buffer with the background color
	blitter.reg(reg_colorFore, 0xFFFFFFFF);
	blitter.reg(reg_dstBaseAddr, alphaBase);
	blitter.reg(reg_dstFormat, alphaRowBytes | SSTG_PIXFMT_8BPP);	
	blitter.reg(reg_dstXY, 0 | (0 << 16));
	blitter.reg(reg_dstSize, bufferWidth | (bufferHeight << 16)); 
	blitter.reg(reg_command, SSTG_RECTFILL | SSTG_GO | (kROPSource));
	blitter.go(variant::apertureDepth);

	// Set up static registers
	blitter.reg(reg_srcBaseAddr, info.fontBlockAddr & kBaseAddrOffsetMask);
	blitter.reg(reg_srcFormat, curFont.imageRowBytes | SSTG_PIXFMT_8BPP);
//	blitter.reg(reg_dstBaseAddr, alphaBase);
//	blitter.reg(reg_dstFormat, alphaRowBytes | SSTG_PIXFMT_8BPP); 
	UInt32 srcCharSpacing = curFont.widMax;
	UInt32 charHeight = (curFont.ascent + curFont.descent) << 16;
	blitter.reg(reg_dstSize, srcCharSpacing | charHeight);
	blitter.go(variant::apertureDepth);
	UInt32 *fifoPtr = blitter.makeRoom(4 * info.byteCount, true);
	UInt32 index = 0;
	UInt8 curChar;
	SInt32 xOffset = info.curPort->pnLoc.h - info.textRect.left;
	
	// MBW -- XXX -- I'm offsetting all characters down and to the right one.  Is this correct?
#define BLIT_ONE_CHAR(dst)\
		curChar = info.textBuf[index];\
		blitter.alignedStoreCmd(dst, (((1UL << reg_srcXY) | (1UL << reg_dstXY) | (1UL << reg_command)) << 3) | 2, variant::apertureDepth);\
		blitter.alignedStoreCmd(dst + 1, (curChar & 0xF) * srcCharSpacing | (curChar >> 4) * charHeight, variant::apertureDepth);\
		blitter.alignedStoreCmd(dst + 2, ((info.charPositions[index] + 1 + xOffset) & 0x0000FFFF) | 0x00010000, variant::apertureDepth);\
		blitter.alignedStoreCmd(dst + 3,	\
				SSTG_BLT | SSTG_GO | 		\
				(kROPSource & kROPDest), 	\
				variant::apertureDepth);
	
	// This loop puts an even number of blits into the fifo.
	// (one cache line worth per iteration)
	for (UInt32 count = info.byteCount >> 1; count; --count)
	{
		__dcbz(fifoPtr, 0);
		
		BLIT_ONE_CHAR(fifoPtr);		
		index++;
		
		BLIT_ONE_CHAR(fifoPtr + 4);
		index++;
		
		__dcbst(fifoPtr, 0);
		fifoPtr += 8;
	}	
	
	// At this point, the fifo pointer is still cache-line aligned.
	
	// If there are an odd number of blits total, we need to do one more at the end.
	if (info.byteCount & 1)
	{		
		__dcbz(fifoPtr, 0);
		
		BLIT_ONE_CHAR(fifoPtr);
		index++;

		__dcbst(fifoPtr, 0);
		fifoPtr += 4;
	}
#undef BLIT_ONE_CHAR
	blitter.finish(fifoPtr);
	
	// Wait for the 2d section of the chip to finish the queued blits.
	while (info.h3InfoPtr->regsFifo->cmdFifo0.depth) ;
		
	while (info.h3InfoPtr->regs2D->status & (SST_BUSY | SST_GUI_BUSY | SST_CMD0_BUSY)) ;
	

	// Step 3. Use the 3D blitter to draw the text into the main and alt buffers,
	// with proper colorization

	// Invalidate the current 3d hardware context ID.
	info.h3InfoPtr->board->lastContextID = -1;

	// Set up the destination surface
	blitter3.setupDest((variant::dstPixelSize == 32 ? (SST_RM_32BPP) : (SST_RM_15BPP)) |
			SST_RM_YORIGIN_SELECT | SST_RM_RGB_WMASK,
			bufferBase, 
			bufferRowBytes);
	
	if(drawAlgorithm == drawOr)
	{
		// Set up the secondary destination surface so the characters will be
		// drawn to both Temp and Mask at the same time.
		blitter3.setupSecondaryDest(altBase);
	}

	// Set up the pixel pipeline
	blitter3.setupPipeline(
			// reg3_FBI(reg3_combineMode)
			((drawAlgorithm == drawXor)?
				SST_CM_CC_OTHERSELECT_TRGB:			// c_other select "Texture RGB"
				SST_CM_CC_OTHERSELECT_C1_RGB)|		// c_other select "Color 1 RGB"
			SST_CM_CC_LOCALSELECT_ZERO |			// c_local (output not used)
			SST_CM_CC_MSELECT_7_C1_RGB |			// cc_mselect_7 (output not used)
			SST_CM_CCA_OTHERSELECT_TA |				// a_other select "Texture alpha"
			SST_CM_CCA_LOCALSELECT_ZERO |			// a_local (output not used)
			SST_CM_USE_COMBINE_MODE,				// use these selections, not fbzMode contents
			
			// reg3_TREX(reg3_combineMode)
			((drawAlgorithm == drawXor)?
				SST_CM_TC_OTHERSELECT_OTHER_TRGB:	// tc_other "Other texture RGB"
				SST_CM_TC_OTHERSELECT_CR_RGB)|		// tc_other (output not used)
			SST_CM_TC_LOCALSELECT_CK_RGB |			// tc_local (output not used)
			SST_CM_TC_MSELECT_7_ZERO |				// tc_mselect_7 (output not used)
			SST_CM_TCA_OTHERSELECT_LOCAL_TA |		// ta_other select "Local texture alpha"
			SST_CM_TCA_LOCALSELECT_CK_A |			// ta_local (output not used)
			(invertSouce?0:SST_CM_TCA_INVERT_OTHER_ONE_MINUS_X)| // tca_invert_other (our alpha texture is inverted)
			0,

			// reg3_fbzColorPath
			SST_CC_MONE |						// cc_mselect select constant 1
			SST_CCA_MONE |						// cca_mselect select constant 1
			SST_ENTEXTUREMAP |
			(2 << SST_TRIANGLE_ITERATOR_COLUMN_BAND_CONTROL_SHIFT),
			
			// reg3_fbzMode
			SST_ENRECTCLIP |
			SST_RGBWRMASK |
			//SST_ENDITHER |		// Don't dither text.
			0,
			
			// reg3_alphaMode
			SST_ENALPHABLEND |
			(SST_A_SRCALPHA << SST_RGBSRCFACT_SHIFT) |
			(SST_AOM_SRCALPHA << SST_RGBDSTFACT_SHIFT) |
			0,
			
			// reg3_fogMode
			0,
			
			// reg3_color0
			0,
			
			// reg3_color1
			foreground3d
	);

	// Set up the source texture
	UInt32 lod = __cntlzw(alphaRowBytes) - 20;
	blitter3.setupTexture(
			alphaBase, 
			lod,

			// reg3_TREX(reg3_textureMode)
			SST_A8 |
			SST_TCLAMPS |
			SST_TCLAMPT |
			SST_TC_MONE |			// tc_mselect select constant 1
			SST_TCA_MONE |			// tca_mselect select constant 1
			0);
	
	if(drawAlgorithm == drawXor)
	{
		blitter3.setupOtherTexture(
				altBase,
				lod,
				
				// reg3_TREX1(reg3_textureMode)
				((variant::dstPixelSize == 16)?(SST_ARGB1555):(SST_ARGB8888)) |
				SST_TCLAMPS |
				SST_TCLAMPT |
				SST_TC_MONE |			// tc_mselect select constant 1
				SST_TCA_MONE |			// tca_mselect select constant 1
				0,
				
				// reg3_TREX1(reg3_combineMode)
				SST_CM_TC_OTHERSELECT_LOCAL_TRGB |		// tc_other "Local texture RGB"
				SST_CM_TC_LOCALSELECT_CK_RGB |			// tc_local (output not used)
				SST_CM_TC_MSELECT_7_ZERO |				// tc_mselect_7 (output not used)
				SST_CM_TCA_OTHERSELECT_CR_A |			// ta_other (output not used)
				SST_CM_TCA_LOCALSELECT_CK_A |			// ta_local (output not used)
				SST_CM_TC_INVERT_OTHER_ONE_MINUS_X |	// tc_invert_other (inverts the colors in the texture)
				0);
				
	}
			
	// Build a type 3 (triangle) packet.
	UInt32 *result = blitter3.makeRoom((17 * info.byteCount) + 2);
	blitter3.storeNonPixelData(result++, 
		SSTCP_PKT3 |
		SSTCP_PKT3_BDDDDD |
		(0x00000004 << SSTCP_PKT3_NUMVERTEX_SHIFT) |
		(0x00000020 << SSTCP_PKT3_PMASK_SHIFT) |
		(0x00000000 << SSTCP_PKT3_SMODE_SHIFT)
	);
	
	float srcMult = (256.0 / alphaRowBytes);
	
	float offset = 0.5;
	
	// Top right
	blitter3.storeNonPixelData(result++, (float) (bufferWidth));					// reg3_sVx
	blitter3.storeNonPixelData(result++, (float) 0);								// reg3_sVy
	blitter3.storeNonPixelData(result++, (bufferWidth + offset) * srcMult);			// reg3_sS_W0
	blitter3.storeNonPixelData(result++, (offset * srcMult));						// reg3_sT_W0
	
	// Top left
	blitter3.storeNonPixelData(result++, (float) 0);								// reg3_sVx
	blitter3.storeNonPixelData(result++, (float) 0);								// reg3_sVy
	blitter3.storeNonPixelData(result++, (offset * srcMult));						// reg3_sS_W0
	blitter3.storeNonPixelData(result++, (offset * srcMult));						// reg3_sT_W0
	
	// Bottom right
	blitter3.storeNonPixelData(result++, (float) bufferWidth);						// reg3_sVx
	blitter3.storeNonPixelData(result++, (float) bufferHeight);						// reg3_sVy
	blitter3.storeNonPixelData(result++, (bufferWidth + offset) * srcMult);			// reg3_sS_W0
	blitter3.storeNonPixelData(result++, (bufferHeight + offset) * srcMult);		// reg3_sT_W0

	// Bottom left
	blitter3.storeNonPixelData(result++, (float) 0);								// reg3_sVx
	blitter3.storeNonPixelData(result++, (float) bufferHeight);						// reg3_sVy
	blitter3.storeNonPixelData(result++, (offset * srcMult));						// reg3_sS_W0
	blitter3.storeNonPixelData(result++, (bufferHeight + offset) * srcMult);		// reg3_sT_W0
	
	// 3D NOP command (keeps subsequent 2d blits from being hosed)
	blitter3.storeNonPixelData(result++, CMDFIFO_BUILD_PK4(1, SSTCP_PKT4_NOPCMD, 0));
	blitter3.storeNonPixelData(result++, 0);
	blitter3.finish(result);
	
	if(drawAlgorithm == drawOr)
	{
		// Turn off the strange things we did to aactl.  (They can do bad things to the 3d code later.)
		blitter3.cleanupSecondaryDest();
	}
	
	// Wait for the 3d section of the chip to finish the queued blits.
	while (info.h3InfoPtr->regsFifo->cmdFifo0.depth) ;
		
	while (info.h3InfoPtr->regs2D->status & (SST_BUSY | SST_FBI_BUSY | SST_TMU_BUSY | SST_CMD0_BUSY)) ;


	if(drawAlgorithm == drawOr)
	{
		// Step 4. XOR the destination pixels onto the alt buffer
		blitter.reg(reg_srcBaseAddr, destAddr);
		blitter.reg(reg_srcFormat, destRowBytes | variant::dstPixFmt);
		blitter.reg(reg_srcXY, 0);
		blitter.reg(reg_dstBaseAddr, altBase);
		blitter.reg(reg_dstFormat, altRowBytes | variant::dstPixFmt);
		blitter.reg(reg_dstXY, 0);
		blitter.reg(reg_dstSize, bufferWidth | (bufferHeight << 16));
		blitter.reg(reg_command, SSTG_BLT | SSTG_GO | (SSTG_ROP_XOR << SSTG_ROP0_SHIFT));
		blitter.go(variant::apertureDepth);
		
		// Sometimes the foreground and background colors are the same.  This screws our srcOr
		// drawing algorithm, so we check for it here.
		if(info.foreColor != info.backColor)
		{
			// Step 5. XOR the destination pixels onto the mask buffer, with dest colorkeying
			blitter.reg(reg_dstColorkeyMin, info.backColor);
			blitter.reg(reg_dstColorkeyMax, info.backColor);
			blitter.reg(reg_rop, (SSTG_ROP_ZERO << 16) | (SSTG_ROP_XOR << 8) | SSTG_ROP_ZERO);
			blitter.reg(reg_commandEx, SSTG_EN_DST_COLORKEY_EX);
		//	blitter.reg(reg_srcBaseAddr, dstAddr);
		//	blitter.reg(reg_srcFormat, dstRowBytes | variant::dstPixFmt);
		//	blitter.reg(reg_srcXY, 0);
			blitter.reg(reg_dstBaseAddr, bufferBase);
			blitter.reg(reg_dstFormat, bufferRowBytes | variant::dstPixFmt);
		//	blitter.reg(reg_dstXY, 0);
		//	blitter.reg(reg_dstSize, bufferWidth | (bufferHeight << 16));
			blitter.reg(reg_command, SSTG_BLT | SSTG_GO | (SSTG_ROP_XOR << SSTG_ROP0_SHIFT));
			blitter.go(variant::apertureDepth);

	
			// Step 6. Copy the mask to the temp buffer, with source colorkeying
			blitter.reg(reg_srcColorkeyMin, 0);
			blitter.reg(reg_srcColorkeyMax, 0);
			blitter.reg(reg_rop, (SSTG_ROP_ZERO << 16) | (SSTG_ROP_ZERO << 8) | SSTG_ROP_DST);
			blitter.reg(reg_commandEx, SSTG_EN_SRC_COLORKEY_EX);
			blitter.reg(reg_srcBaseAddr, bufferBase);
			blitter.reg(reg_srcFormat, bufferRowBytes | variant::dstPixFmt);
			blitter.reg(reg_srcXY, 0);
			blitter.reg(reg_dstBaseAddr, altBase);
			blitter.reg(reg_dstFormat, altRowBytes | variant::dstPixFmt);
		//	blitter.reg(reg_dstXY, 0);
		//	blitter.reg(reg_dstSize, bufferWidth | (bufferHeight << 16));
			blitter.reg(reg_command, SSTG_BLT | SSTG_GO | (SSTG_ROP_DST << SSTG_ROP0_SHIFT));
			blitter.go(variant::apertureDepth);
		}
		
		// Step 7. Prepare for region parsing.
		blitter.reg(reg_srcBaseAddr, altBase);
		blitter.reg(reg_srcFormat, altRowBytes | variant::dstPixFmt);
		info.aaBlitterCmdValue = SSTG_BLT | SSTG_CLIPSELECT | SSTG_GO | (SSTG_ROP_XOR << SSTG_ROP0_SHIFT);
	}
	else // if((drawAlgorithm == drawXor) || (drawAlgorithm == drawSrc))
	{
		// Step 7. Prepare for region parsing.
		blitter.reg(reg_srcBaseAddr, bufferBase);
		blitter.reg(reg_srcFormat, bufferRowBytes | variant::dstPixFmt);
		info.aaBlitterCmdValue = SSTG_BLT | SSTG_CLIPSELECT | SSTG_GO | (SSTG_ROP_SRC << SSTG_ROP0_SHIFT);
	}

	blitter.reg(reg_srcXY, 0);
	blitter.reg(reg_dstBaseAddr, destAddr);
	blitter.reg(reg_dstFormat, destRowBytes | variant::dstPixFmt);
	blitter.reg(reg_dstSize, bufferWidth | (bufferHeight << 16));
	blitter.reg(reg_dstXY, 0);
	blitter.reg(reg_commandEx, 0);
	blitter.go(variant::apertureDepth);
	
	blitter.alignFifo();
	
	return true;
}

/********************************************************************************
	StdTextAADrawFn
		void *blitData
		Rect &rect
		
	
*/
template <class variant>
void StdTextAADrawFn(void *blitData, Rect &rect)
{
	StdTextInfo	&info = *(StdTextInfo *) blitData;
	FontData	&curFont = *info.curFont;
	Fifo2DRegs	blitter(info.h3InfoPtr);

	// Determine the source offset
	Int32 top = rect.top - (info.textRect.top - 1);
	Int32 left = rect.left - (info.textRect.left - 1);
	
	blitter.clip1_cmd_go(
		(left) | (top << 16),
		(left + (rect.right - rect.left)) | ((top + (rect.bottom - rect.top)) << 16),
		info.aaBlitterCmdValue,
		variant::apertureDepth);
}

/********************************************************************************
	StdTextDrawFn
		void *blitData		Pointer to the StdTextInfo struct, from StdTextPatch
		Rect &rect			Rect to draw to
		
	This function is called by the region parser, and it draws the entire text string,
	clipping the text to the rect passed in from the parser. It also handles clearing the 
	background in transfer modes that requre it.
	
	Note that no matter what the Quickdraw transfer mode is, this function only uses
	ROP modes that don't modify the background pixels while drawing text. This means
	that most modes use the transparent bit for rendering, others use src XOR dest. Transfer
	modes that require that the background be modified need to do so OUTSIDE of the glyph
	drawing process.
*/
template <class variant>
void StdTextDrawFn(void *blitData, Rect &rect)
{
	StdTextInfo	&info = *(StdTextInfo *) blitData;
	FontData	&curFont = *info.curFont;
	Fifo2DRegs	blitter(info.h3InfoPtr);
	UInt32		commandVal = SSTG_BLT | SSTG_GO | SSTG_CLIPSELECT | info.commandVal;
	
	// rcf test
//	FrameRect(&rect);
	
	if (info.isOffscreen)
	{
		// Offset the rect to 'global' coordinates; so that the pixel at baseAddr is { 0, 0 }.
		FastOffsetRect(rect, - info.portPixMap->bounds.left, - info.portPixMap->bounds.top);

		// If we're drawing to an offscreen gWorld, the portPixMap has the correct base address.
		blitter.reg(reg_dstBaseAddr, (UInt32) info.portPixMap->baseAddr & kBaseAddrOffsetMask); 
	} else 
	{
	// Offset the rect to be relative to the topleft of the current gDevice. Note
	// that in the case of monitor mirroring, this may not be the topleft of the screen.
	FastOffsetRect(rect, - (info.curGDPixMap->bounds.left + info.portPixMap->bounds.left),
			- (info.curGDPixMap->bounds.top + info.portPixMap->bounds.top));
	
		// For onscreen drawing, the gDevice has the correct baseAddr
		blitter.reg(reg_dstBaseAddr, (UInt32) info.curGDPixMap->baseAddr & kBaseAddrOffsetMask); 	
	}
	
	// Set up registers
	blitter.reg(reg_clip1min, rect.left | ((long) rect.top << 16));
	blitter.reg(reg_clip1max, rect.right | ((long) rect.bottom << 16));
	blitter.reg(reg_srcBaseAddr, info.fontBlockAddr & kBaseAddrOffsetMask);
	blitter.reg(reg_srcFormat, curFont.imageRowBytes | SSTG_PIXFMT_1BPP);
	blitter.reg(reg_dstFormat, (info.curGDPixMap->rowBytes & 0x3FFF) | variant::dstPixFmt); 
	
	// Certain transfer modes reqire that we fill in the background of the text area,
	// since the characters (and the rects around them) don't cover the entire rect.
	UInt32	mode = info.curPort->txMode & 7;
	if (mode == srcCopy || mode == notSrcCopy)
	{
		// SrcCopy needs the background filled with the backColor
		blitter.reg(reg_dstXY, rect.left | ((long) rect.top << 16));
		blitter.reg(reg_dstSize, rect.right - rect.left | ((rect.bottom - rect.top) << 16));
		blitter.reg(reg_colorFore, info.backColor);
		blitter.reg(reg_command, SSTG_RECTFILL | SSTG_GO | gModeToRop[srcCopy]);
		blitter.go(variant::apertureDepth);
	} else if (mode == notSrcXor)
	{
		// NotSrcXor need the entire rect xor'ed. The glyphs themselves are then
		// xor'ed back to their original state.
		blitter.reg(reg_dstXY, rect.left | ((long) rect.top << 16));
		blitter.reg(reg_dstSize, rect.right - rect.left | ((rect.bottom - rect.top) << 16));
		blitter.reg(reg_command, SSTG_BLT | SSTG_GO | 
				((~kROPDest & 0xFF) << SSTG_ROP0_SHIFT));
		blitter.go(variant::apertureDepth);
	}
	
	// Note that at this point, we may or may not have registers in our blitter object
	blitter.reg(reg_colorFore, info.foreColor);
	blitter.reg(reg_colorBack, info.backColor);
		
	SInt32 destXValue = info.curPort->pnLoc.h - (info.curGDPixMap->bounds.left +
			info.portPixMap->bounds.left);
	UInt32 destYValue = (info.curPort->pnLoc.v - (info.curGDPixMap->bounds.top +
			info.portPixMap->bounds.top) - curFont.ascent) << 16;
	UInt32 srcCharSpacing = curFont.widMax;
	UInt32 charHeight = (curFont.ascent + curFont.descent) << 16;

	blitter.reg(reg_dstSize, srcCharSpacing | charHeight);
	blitter.go(variant::apertureDepth);
	
	UInt32 *fifoPtr = blitter.makeRoom(4 * info.byteCount, true);
	UInt32 index = 0;
	UInt8 curChar;
		
#define BLIT_ONE_CHAR(dst)\
		curChar = info.textBuf[index];\
		blitter.alignedStoreCmd(dst, (((1UL << reg_srcXY) | (1UL << reg_dstXY) | (1UL << reg_command)) << 3) | 2, variant::apertureDepth);\
		blitter.alignedStoreCmd(dst + 1, (curChar & 0xF) * srcCharSpacing | (curChar >> 4) * charHeight, variant::apertureDepth);\
		blitter.alignedStoreCmd(dst + 2, ((destXValue + info.charPositions[index]) & 0x0000FFFF) | destYValue, variant::apertureDepth);\
		blitter.alignedStoreCmd(dst + 3, commandVal, variant::apertureDepth);
	
	// This loop puts an even number of blits into the fifo.
	// (one cache line worth per iteration)
	for (UInt32 count = info.byteCount >> 1; count; --count)
	{
		__dcbz(fifoPtr, 0);
		
		BLIT_ONE_CHAR(fifoPtr);		
		index++;
		
		BLIT_ONE_CHAR(fifoPtr + 4);
		index++;
		
		__dcbst(fifoPtr, 0);
		fifoPtr += 8;
	}	
	
	// At this point, the fifo pointer is still cache-line aligned.
	
	// If there are an odd number of blits total, we need to do one more at the end.
	if (info.byteCount & 1)
	{		
		__dcbz(fifoPtr, 0);
		
		BLIT_ONE_CHAR(fifoPtr);
		index++;

		__dcbst(fifoPtr, 0);
		fifoPtr += 4;
	}
	
#undef BLIT_ONE_CHAR

	blitter.finish(fifoPtr);
}

/********************************************************************************
	StdText3StepDrawFn
		void *blitData
		Rect &rect
		
	Used for the notSrcOr and notSrcBic transfer modes. Handles drawing in this mode
	in 3 stages. First, the destination pixels are XOR'ed with the foreground color
	and stored in the scratch area. Second, the characters are drawn to the dest in 
	the foreground color in (essentially) srcOr mode. Third, the scratch area is XOR'ed
	back onto the dest. This has the effect of painting the area outside of the glyphs
	with the foreground color, while leaving the area inside the glyphs untouched. Note
	that in notSrcBic mode, the colors get swapped by ModeToRop before we see them, and
	the 'foreground' color will be QD's background color.
*/
void StdText3StepDrawFn(void *blitData, Rect &rect)
{
	StdTextInfo	&info = *(StdTextInfo *) blitData;
	Fifo2DRegs	scratchBlitter(info.h3InfoPtr);
	Fifo2DRegs	finishBlitter(info.h3InfoPtr);
	UInt32		rectWidth = rect.right - rect.left;
	UInt32 		linesPerChunk;

	// Prepare registers for XOR'ing the dest into the scratch area, and then XOR'ing them back
	scratchBlitter.reg(reg_dstBaseAddr, (UInt32) info.h3InfoPtr->scratchSpace->start & kBaseAddrOffsetMask); 
	finishBlitter.reg(reg_srcBaseAddr, (UInt32) info.h3InfoPtr->scratchSpace->start & kBaseAddrOffsetMask);

	if (info.curGDPixMap->pixelSize == 8)
	{
		scratchBlitter.reg(reg_srcFormat, (info.curGDPixMap->rowBytes & 0x3FFF) | SSTG_PIXFMT_8BPP);
		scratchBlitter.reg(reg_dstFormat, rectWidth | SSTG_PIXFMT_8BPP); 
		finishBlitter.reg(reg_srcFormat, rectWidth | SSTG_PIXFMT_8BPP);
		finishBlitter.reg(reg_dstFormat, (info.curGDPixMap->rowBytes & 0x3FFF) | SSTG_PIXFMT_8BPP);
		linesPerChunk =  kQDScratchSize / rectWidth;
	} else if (info.curGDPixMap->pixelSize == 16)
	{
		scratchBlitter.reg(reg_srcFormat, (info.curGDPixMap->rowBytes & 0x3FFF) | SSTG_PIXFMT_16BPP);
		scratchBlitter.reg(reg_dstFormat, (rectWidth << 1) | SSTG_PIXFMT_16BPP); 
		finishBlitter.reg(reg_srcFormat, (rectWidth << 1) | SSTG_PIXFMT_16BPP);
		finishBlitter.reg(reg_dstFormat, (info.curGDPixMap->rowBytes & 0x3FFF) | SSTG_PIXFMT_16BPP);
		linesPerChunk =  kQDScratchSize / (rectWidth << 1);
	} else
	{
		scratchBlitter.reg(reg_srcFormat, (info.curGDPixMap->rowBytes & 0x3FFF) | SSTG_PIXFMT_32BPP);
		scratchBlitter.reg(reg_dstFormat, (rectWidth << 2) | SSTG_PIXFMT_32BPP); 
		finishBlitter.reg(reg_srcFormat, (rectWidth << 2) | SSTG_PIXFMT_32BPP);
		finishBlitter.reg(reg_dstFormat, (info.curGDPixMap->rowBytes & 0x3FFF) | SSTG_PIXFMT_32BPP);
		linesPerChunk =  kQDScratchSize / (rectWidth << 2);
	}
	
	if (linesPerChunk > 2000)
		linesPerChunk = 2000;
	
	// The blit to the scratch area needs to use the pattern as a solid, to XOR with a color
	scratchBlitter.reg(reg_colorFore, info.foreColor);
	scratchBlitter.reg(reg_pattern0alias, 0xFFFFFFFF);
	scratchBlitter.reg(reg_pattern1alias, 0xFFFFFFFF);

	// DeviceRect is device coordinate transform; that is, it changes local QD coords into coords
	// for the XY registers
	Rect deviceRect = { 0, 0, 0, 0 };
	if (info.isOffscreen)
	{
		// Offset the rect to 'global' coordinates; so that the pixel at baseAddr is { 0, 0 }.
		FastOffsetRect(deviceRect, - info.portPixMap->bounds.left, - info.portPixMap->bounds.top);

		// If we're drawing to an offscreen gWorld, the portPixMap has the correct base address.
		scratchBlitter.reg(reg_srcBaseAddr, (UInt32) info.portPixMap->baseAddr & kBaseAddrOffsetMask);
		finishBlitter.reg(reg_dstBaseAddr, (UInt32) info.portPixMap->baseAddr & kBaseAddrOffsetMask); 
	} else 
	{
		// Offset the rect to be relative to the topleft of the current gDevice. Note
		// that in the case of monitor mirroring, this may not be the topleft of the screen.
		FastOffsetRect(deviceRect, - (info.curGDPixMap->bounds.left + info.portPixMap->bounds.left),
				- (info.curGDPixMap->bounds.top + info.portPixMap->bounds.top));

		// For onscreen drawing, the gDevice has the correct baseAddr
		scratchBlitter.reg(reg_srcBaseAddr, (UInt32) info.curGDPixMap->baseAddr & kBaseAddrOffsetMask);
		finishBlitter.reg(reg_dstBaseAddr, (UInt32) info.curGDPixMap->baseAddr & kBaseAddrOffsetMask); 
	}
		
	// This loop cuts the input rectangle into bands of a size small enough that each band can
	// be saved off into the Scratch area
	Rect dividedRect = rect;
	while (dividedRect.top < rect.bottom)
	{
		// DividedRect holds the largest chunk we can handle at once
		dividedRect.bottom = dividedRect.top + linesPerChunk;
		if (dividedRect.bottom > rect.bottom)
			dividedRect.bottom = rect.bottom;
	
		Rect screenRect = dividedRect;
		FastOffsetRect(screenRect, deviceRect.left, deviceRect.top);

		// Blit the screen dest XOR the background color into the scratch area
		scratchBlitter.reg(reg_dstXY, 0);
		scratchBlitter.reg(reg_srcXY, screenRect.left | ((long) screenRect.top << 16));
		scratchBlitter.reg(reg_dstSize, screenRect.right - screenRect.left |
				((screenRect.bottom - screenRect.top) << 16));
		scratchBlitter.reg(reg_command, SSTG_BLT | SSTG_GO | SSTG_MONO_PATTERN | 
				((kROPSource ^ kROPPattern) & 0xFF000000));
		scratchBlitter.goNoReset();
		
		if (info.h3InfoPtr->depth < 16)
			StdTextDrawFn<variantDst8>(blitData, dividedRect);
		else if (info.h3InfoPtr->depth == 16)
			StdTextDrawFn<variantDst16>(blitData, dividedRect);
		else
			StdTextDrawFn<variantDst32>(blitData, dividedRect);
		dividedRect.top = dividedRect.bottom;
		
		// Now, blit the scratch area XOR the screen dest back to the screen dest
		finishBlitter.reg(reg_dstXY, screenRect.left | ((long) screenRect.top << 16));
		finishBlitter.reg(reg_srcXY, 0);
		finishBlitter.reg(reg_dstSize, screenRect.right - screenRect.left |
				((screenRect.bottom - screenRect.top) << 16));
		finishBlitter.reg(reg_command, SSTG_BLT | SSTG_GO | ((kROPSource ^ kROPDest) & 0xFF000000));
		finishBlitter.goNoReset();
	}			
}

#pragma mark -
#pragma mark ¥ Font Data

/********************************************************************************
	FontData::Initialize
		
	Initialized a FontData struct to a stable state. Generally, this means clearing
	the bitfields to 0.
*/
void FontData::Initialize()
{
	measuredChars.Clear();
	fractMeasuredChars.Clear();
	renderedChars.Clear();
	
	fontFlags = 0;
}

/********************************************************************************
	FontData::SetupFontData
		StdTextInfo &info
		
	Returns true if set up correctly.
	
	This function analyzes a font, and determines several basic numbers which will
	be needed to calculate width information and generate cached glyphs for the font.
*/
bool FontData::SetupFontData(StdTextInfo &info)
{
	// Set up some basic info about the font
	mNumer = info.origNumer;
	mDenom = info.origDenom;
	mHorizScale = (((info.origNumer.h << 16) / info.origDenom.h) + 0x1000) & 0xFFFFE000;
	mVertScale = (((info.origNumer.v << 16) / info.origDenom.v) + 0x1000) & 0xFFFFE000;
	
	bool isOutline = IsOutline(mNumer, mDenom);
	if (isOutline)
		fontFlags |= kFontIsOutline;
		
	if (GetOutlinePreferred())
	{
		if (isOutline)
			fontFlags |= kFontApprovedOutlinePrefOn  | kFontCheckedOutlinePrefOn;
		else
			fontFlags |= kFontApprovedOutlinePrefOn  | kFontCheckedOutlinePrefOn |
						 kFontApprovedOutlinePrefOff | kFontCheckedOutlinePrefOff;
	} else
	{
		if (!isOutline)
			fontFlags |= kFontApprovedOutlinePrefOff | kFontCheckedOutlinePrefOff;
		else
			fontFlags |= kFontApprovedOutlinePrefOn  | kFontCheckedOutlinePrefOn |
						 kFontApprovedOutlinePrefOff | kFontCheckedOutlinePrefOff;
	}
	
	// rcf Here, we should call FMSwapFont, to get the post font manager scaling factors
	// 
	
	// The the FontInfo struct, we need to pass back from TxMeas
	GetFontInfo(&fInfo);
	
	// Now, set up our private fields
	ascent = fInfo.ascent;
	descent = fInfo.descent;

	// Store the font ID, font size, and face of this font
	mFont = info.curPort->txFont;
	if (mFont == systemFont) mFont = GetSysFont();
	if (mFont == applFont) mFont = GetAppFont();
	if (mFont == systemFont) mFont = GetSysFont();
	mSize = info.curPort->txSize;
	if (!mSize) mSize = GetDefFontSize();
	if (!mSize) mSize = 12;
	mFace = info.curPort->txFace & 0x7F;
	
	// rcf Need to munge charWidthMax
	UInt32	charWidthMax = fInfo.widMax;
	
	if ((fontFlags & kFontIsOutline) == 0)
	{
		// This is a bitmap font.  Adjust charWidthMax accordingly.

		FMInput sfInput;
		sfInput.family = mFont;
		sfInput.size = mSize;
		sfInput.face = mFace;
		sfInput.needBits = false;
		sfInput.device = 0;
		sfInput.numer = mNumer;
		sfInput.denom = mDenom;
		FMOutput *sfOutput = FMSwapFont(&sfInput);
							
		// Now, determine the spacing modifiers QD will be applying to the font
		// Italic, bold, outline and shadow are handled here; underline is
		// handled later.
		if (sfOutput->curStyle & italic && sfOutput->italicPixels)
		{
			charWidthMax += (((sfOutput->italicPixels * sfOutput->descent) >> 4) *
							sfOutput->numer.h) / sfOutput->denom.h;
			charWidthMax += (((sfOutput->italicPixels * (sfOutput->ascent - 1)) >> 4) *
							sfOutput->numer.h) / sfOutput->denom.h;
		}
		
		if (sfOutput->curStyle & bold && sfOutput->boldPixels)
			charWidthMax += sfOutput->boldPixels;
			
		if (sfOutput->curStyle & (outline | shadow))
		{
			charWidthMax += 1 + sfOutput->shadowPixels + 1;
		}
	}
	
#ifdef USE_3D_TEXT
	// rcf Determine whether this font is antialiased.
	SInt16 minSize;
	if (IsAntiAliasedTextEnabled(&minSize) && minSize <= mSize)
	{
		// Antialiased. Chararacters are still arranged in a 16x16 grid.
		imageRowBytes = charWidthMax * 16;
		fontFlags |= kFontIsAntiAliased;
	} else
#endif
	{
		// Not antialiased. Characters are arranged in a 16x16 grid. 
		// Rowbytes is max. width * 16, divided by 8 to turn bits into bytes
		imageRowBytes = charWidthMax * (16 / 8);
	}
	widMax = charWidthMax;
		
	
	return true;	
}

/********************************************************************************
	FontData::IsMatch
		short inFont
		short inSize
		short inFace
		Fixed horizScale
		Fixed vertScale
		
	Determine whether the given font description would be a match for our font.
*/
inline bool FontData::IsMatch(short inFont, short inSize, short inFace, Fixed horizScale, Fixed vertScale,
					bool inAntiAliased)
{
	// Check everything else here
	if (inFont == mFont &&
			inSize     == mSize &&
			inFace     == mFace &&
			horizScale == mHorizScale &&
			vertScale  == mVertScale &&
			inAntiAliased == !!(fontFlags & kFontIsAntiAliased))
	{
		if (GetOutlinePreferred())
		{
			// Have we checked whether this font uses an outline font with OutlinePreferred on?
			if (!(fontFlags & kFontCheckedOutlinePrefOn))
			{
				// See if it'd be an outline font now. If not, we can use this (bitmap)
				// cached font in both states of OutlinePreferred.
				if (!IsOutline(mNumer, mDenom))
					fontFlags |= kFontApprovedOutlinePrefOn;
					
				fontFlags |= kFontCheckedOutlinePrefOn;
			}

			// If outlinePreferred is on, but we're not approved for it, no match
			if (!(fontFlags & kFontApprovedOutlinePrefOn))
				return false;
		} else
		{
			// OutlinePreferred is off. Have we checked whether this font uses a bitmap font
			// with OutlinePreferred off?
			if (!(fontFlags & kFontCheckedOutlinePrefOff))
			{
				// See if it'd be an bitmap font now. If not, we can use this (outline)
				// cached font in both states of OutlinePreferred.
				if (IsOutline(mNumer, mDenom))
					fontFlags |= kFontApprovedOutlinePrefOff;
					
				fontFlags |= kFontCheckedOutlinePrefOff;
			}

			// If outlinePreferred is off, but we're not approved for it, no match
			if (!(fontFlags & kFontApprovedOutlinePrefOff))
				return false;
		}
	
		return true;
	}
	
	return false;
}

/********************************************************************************
	FontData::MeasureTextWidthInternal
		StdTextInfo &info
		bool fractEnable
		bool extraSpace
		bool isRendering
		Fixed spExtra = 0
		Fixed chExtra = 0
		
	Private, inner loop for measuring text width. The fractEnable, extraSpace and 
	isRendering flags should be passed in as constants. This will make most of the
	if statements inside the loop disappear.
*/
inline Fixed FontData::MeasureTextWidthInternal(StdTextInfo &info, bool fractEnable, bool extraSpace, 
					bool isRendering, bool scaledText, Fixed spExtra, Fixed chExtra)
{
	Fixed	curWidth = 0x8000;
	Fixed	horizScaleFactor = info.horizScaleFactor >> 8;
	
	for (UInt32 index = 0; index < info.byteCount; ++index)
	{
		UInt8 curChar = info.textBuf[index];
		
		// Check that we have computed width info for this character
		if (fractEnable)
		{
			if (!fractMeasuredChars.CheckBit(curChar))
				SetCharWidth(info, curChar);
		} else
		{
			if (!measuredChars.CheckBit(curChar))
				SetCharWidth(info, curChar);
		}
					
		if (isRendering)
		{
			info.charPositions[index] = (curWidth >> 16) + metrics[curChar].GetLeft();
		}

		if (extraSpace)
		{
			if (curChar == ' ')
				curWidth += spExtra;
			else
				curWidth += chExtra;
		} 
		
		if (scaledText)
		{
			if (fractEnable)
				curWidth += (metrics[curChar].GetFract() >> 8) * horizScaleFactor;
			else
				curWidth += (metrics[curChar].GetAdvance() >> 8) * horizScaleFactor;
		} else
		{
			if (fractEnable)
				curWidth += metrics[curChar].GetFract();
			else
				curWidth += metrics[curChar].GetAdvance();
		}
	}

	return curWidth;
}


/********************************************************************************
	FontData::MeasureTextWidth2nd
		StdTextInfo &info
		bool fractEnable
		bool extraSpace
		Fixed spExtra
		Fixed chExtra
		
	The function MeasureTextWidthInternal has 4 booleans, and gets inlined 16 times. 
	This function simplifies the decision tree for MeasureTextWidth, by handling
	the inner 2 decisions (whether the scale factor is equal to 1, and whether we
	are rendering the text).
*/
inline Fixed FontData::MeasureTextWidth2nd(StdTextInfo &info, bool fractEnable, bool extraSpace, 
					Fixed spExtra, Fixed chExtra)
{
	if (info.horizScaleFactor != 1 << 16)
	{
		if (info.isRendering)
			return MeasureTextWidthInternal(info, fractEnable, extraSpace, 1, 1, spExtra, chExtra);
		else
			return MeasureTextWidthInternal(info, fractEnable, extraSpace, 0, 1, spExtra, chExtra);
	} else
	{
		if (info.isRendering)
			return MeasureTextWidthInternal(info, fractEnable, extraSpace, 1, 0, spExtra, chExtra);
		else
			return MeasureTextWidthInternal(info, fractEnable, extraSpace, 0, 0, spExtra, chExtra);
	}
}

/********************************************************************************
	FontData::MeasureTextWidth
		StdTextInfo &info
		
	Measures the width of a string of text.
*/
Fixed FontData::MeasureTextWidth(StdTextInfo &info)
{
	Fixed	spExtra = info.curPort->spExtra;
	Fixed	chExtra = 0;
	
	if (info.curPort->portVersion & 0xC000)
		chExtra = (Fixed) info.curPort->chExtra << 4;

	if (spExtra || chExtra)
	{
		if (lmFractEnable)
		{
			return MeasureTextWidth2nd(info, 1, 1, spExtra, chExtra);
		} else
		{
			return MeasureTextWidth2nd(info, 0, 1, spExtra, chExtra);
		}
	} else
	{
		if (lmFractEnable)
		{
			return MeasureTextWidth2nd(info, 1, 0);
		} else
		{
			return MeasureTextWidth2nd(info, 0, 0);
		}
	}
}

/********************************************************************************
	FontData::SetCharWidthBitmap
		StdTextInfo &info
		UInt8 curChar
		
	
*/
void FontData::SetCharWidthBitmap(StdTextInfo &info, UInt8 curChar)
{
#pragma unused (info)

	SInt32	left = 0, right = 0;
	SInt16	widthOffset = -1;
	SInt8	width, offset;
	
	FMInput sfInput;
	sfInput.family = mFont;
	sfInput.size = mSize;
	sfInput.face = mFace;
	sfInput.needBits = false;
	sfInput.device = 0;
	sfInput.numer = mNumer;
	sfInput.denom = mDenom;
	FMOutput *sfOutput = FMSwapFont(&sfInput);
	
//	mSFOutput = *sfOutput;
	
	FMetricRec fMetric;
	FontMetrics(&fMetric);
	WidthTable *wTab = (WidthTable *) *fMetric.wTabHandle;
		
	FontRec **fontH = (FontRec **) sfOutput->fontHandle;
// rcf Can't die here
	if (!fontH)
		return;
	if (!*fontH)
	{
		LoadResource((Handle) fontH);
		if (!*fontH)
			return;
	}
	
	// Get the addr of the width/offset table
	UInt16 *woTable = (UInt16 *) ((UInt32) *fontH + 16) + fontH[0][0].owTLoc;
	if (fontH[0][0].nDescent > 0)
		woTable += ((UInt32) fontH[0][0].nDescent) << 16;
		
	SInt16	kernMax = fontH[0][0].kernMax;
	
	// Now, determine the spacing modifiers QD will be applying to the font
	// Italic, bold, outline and shadow are handled here; underline is
	// handled later.
	if (sfOutput->curStyle & italic && sfOutput->italicPixels)
	{
		SInt32 temp = (((sfOutput->italicPixels * sfOutput->descent) >> 4) *
				sfOutput->numer.h) / sfOutput->denom.h;
		right += temp + (((sfOutput->italicPixels * (sfOutput->ascent - 1)) >> 4) *
				sfOutput->numer.h) / sfOutput->denom.h;
		left -= temp;
	}
	
	if (sfOutput->curStyle & bold && sfOutput->boldPixels)
		right += sfOutput->boldPixels;
		
	if (sfOutput->curStyle & (outline | shadow))
	{
		left -= 1;
		right += sfOutput->shadowPixels + 1;
	}
	
// rcf Should just calc and store the left and right offsets in SetupFontData, for speed.

	// Get the advance width for this character
	SInt32 advance = ((((wTab->tabData[curChar] >> 8) * wTab->hOutput) + 0x8000) >> 16) & 0xFF;

	// Get the width and lsb for this character
	if (curChar >= fontH[0][0].firstChar && curChar <= fontH[0][0].lastChar)
		widthOffset = woTable[curChar - fontH[0][0].firstChar];
	if (widthOffset == -1)
		widthOffset = woTable[fontH[0][0].lastChar - fontH[0][0].firstChar + 1];
	width = widthOffset & 0xFF;
	offset = ((SInt8) (widthOffset >> 8));
	
	width = ((width * wTab->hOutput) + 0x0080) >> 8;
	offset = (((offset * wTab->hOutput) + 0x0080) >> 8) + fontH[0][0].kernMax;		
	if (offset < 0)
		width -= offset;
		
	// Add in the left and right modifiers that QD-controlled styles will add to the font
	offset += left;
	width += right;
	
	// For underlined text, QD will tell us the width of the character, not
	// including the underline. We, however, need the width including the underline, which
	// is the entire character.
	if (sfOutput->curStyle & underline)
	{
		// Remove a 'gap' on the left side of the character.
		if (offset > 0)
		{
			width += offset;
			offset = 0;
		}
		
		// Remove a 'gap' on the right side of the character.
		if (width < advance)
			width = advance;
	}
	
	CharMetrics &met = metrics[curChar];
	met.Clear();
	if (width)
		met.SetGlyph(width);
	else
		met.SetGlyph(0);
	met.SetLeft(offset);
	met.SetAdvance(advance);
 
}

/********************************************************************************
	FontData::SetCharWidthOutline
		StdTextInfo &info
		UInt8 curChar
		
	
*/
void FontData::SetCharWidthOutline(StdTextInfo &info, UInt8 curChar)
{
	Fixed	advanceWidth;
	Fixed	lsbWidth;
	Rect	charBounds;
	short	yMax, yMin;
	
	// Do we need the entire set of measurements, or simply the advance width?
	// (We only measure fixed or fractional advance width, depending on lmFractEnable setting
	// when called)
	if (fractMeasuredChars.CheckBit(curChar) || measuredChars.CheckBit(curChar))
	{
		// rcf Might want to used munged numer/denom instead, ca depend
		OutlineMetrics(1, &curChar, info.origNumer, info.origDenom, &yMax, &yMin,
				&advanceWidth, 0, 0);
				
		if (lmFractEnable)
		{
			metrics[curChar].SetFract(advanceWidth);
		} else 
		{
			// The fractional width is saved as an offset from the advance width. So, to change
			// the advance width, remember the (entire) fractional width, modify the advance width,
			// and re-set the fractional width. This will cause the fractional with to be offset
			// from the new advance width
			Fixed saveFract = metrics[curChar].GetFract();
			metrics[curChar].SetAdvance(advanceWidth >> 16);
			metrics[curChar].SetFract(saveFract);
		}
		
	} else
	{
		OutlineMetrics(1, &curChar, info.origNumer, info.origDenom, &yMax, &yMin, 
				&advanceWidth, &lsbWidth, &charBounds);

		SInt32 glyphWidth = charBounds.right - charBounds.left;
		lsbWidth >>= 16;
		
		if (info.curPort->txFace & underline)
		{
			if (lsbWidth > 0)
			{
				glyphWidth += lsbWidth;
				lsbWidth = 0;
			}
			
			if ((advanceWidth >> 16) - lsbWidth > glyphWidth)
			{
				glyphWidth = (advanceWidth >> 16) - lsbWidth;
			}
		}
		
		if (glyphWidth < 0)
			glyphWidth = 0;

		metrics[curChar].Clear();
		metrics[curChar].SetGlyph(glyphWidth);
		metrics[curChar].SetLeft(lsbWidth);
		metrics[curChar].SetAdvance(advanceWidth >> 16);
		metrics[curChar].SetFract(advanceWidth);
	}
}

/********************************************************************************
	FontData::SetCharWidth
		StdTextInfo &info
		UInt8 curChar
		
	
*/
void FontData::SetCharWidth(StdTextInfo &info, UInt8 curChar)
{
	if (fontFlags & kFontIsOutline)
	{
		SetCharWidthOutline(info, curChar);
		if (lmFractEnable)
			fractMeasuredChars.SetBit(curChar);
		else
			measuredChars.SetBit(curChar);
	} else
	{
		SetCharWidthBitmap(info, curChar);
		fractMeasuredChars.SetBit(curChar);
		measuredChars.SetBit(curChar);
	}
}

/********************************************************************************
	FontData::RenderChar
		StdTextInfo &info
		UInt8 curChar
		
	Draws the indicated character into our offscreen graphics port, and copies that
	character onto ALL boards that have a font block for that font.
*/
void FontData::RenderChar(StdTextInfo &info, UInt8 curChar)
{
	FontData 	&curFont = *info.curFont;
	GrafPort	*localPort;
	char		localPortBits[16*120];
	Rect		sourceRect, destRect;
	UInt32		charHeight = curFont.ascent + curFont.descent;
	Rect		boundsRect = { 0, 0, charHeight, 16 << 3 };
	Boolean		isAntiAliased = false;
	
	if (curFont.fontFlags & kFontIsAntiAliased)
	{
		localPort = (GrafPtr) gColorFontPort;
		isAntiAliased = true;
	} else
	{
		localPort = &gFontPort;
		localPort->portBits.baseAddr = (Ptr) localPortBits;
		localPort->portBits.rowBytes = 16;
		FastSetRect(localPort->portBits.bounds, 0, 0, charHeight, 16 << 3);
	}

	localPort->pnLoc.v = curFont.ascent;
	localPort->pnLoc.h = ((curFont.widMax * (curChar & 0xF)) % 8) - curFont.metrics[curChar].GetLeft();
	
	localPort->txFont = curFont.mFont;
	localPort->txFace = curFont.mFace;
	localPort->txSize = curFont.mSize;

	// Switch to the font grafport, and draw the char
	SetPort(localPort);
	EraseRect(&boundsRect);
	PatchStdText::PatchFn(1, &curChar, curFont.mNumer, curFont.mDenom);
	SetPort((GrafPtr) info.curPort);
	
	// Move the char's glyph up onto the board
	sourceRect.top = 0;
	sourceRect.left = (curFont.widMax * (curChar & 0xF)) % 8;
	sourceRect.bottom = charHeight;
	sourceRect.right = sourceRect.left + curFont.widMax;
	destRect.top = (curChar >> 4) * charHeight;
	destRect.left = curFont.widMax * (curChar & 0x0F);
	destRect.bottom = destRect.top + charHeight;
	destRect.right = destRect.left + curFont.widMax;

	for (UInt32 index = 0; index < 16; ++index)
	{
		// Is there a board at this memory address?
		if ((h3_info[index].boardFlags & 1) && h3_info[index].fontCacheBlocks)
		{
			mmBlock_t *fontBlockPtr = h3_info[index].fontCacheBlocks[curFont.fontIndex];
			if (fontBlockPtr)
			{
				if (isAntiAliased)
				{
					doHostBlit<variantBitBlt8>(&h3_info[index], (long) gColorFontPort->portPixMap[0][0].baseAddr,
								gColorFontPort->portPixMap[0][0].rowBytes & 0x3FFF, sourceRect, 
								fontBlockPtr->start, curFont.imageRowBytes,
								destRect, srcCopy, 0, 0, 0);
				} else
				{
					if (h3_info[index].depth == 8)
					{
						doHostBlit<variant1Xfer8>(&h3_info[index], (long) localPortBits, 16, 
								sourceRect, fontBlockPtr->start, curFont.imageRowBytes,
								destRect, srcOr, 0, 0, 0);
					} else if (h3_info[index].depth == 16)
					{
						doHostBlit<variant1Xfer16>(&h3_info[index], (long) localPortBits, 16, 
								sourceRect, fontBlockPtr->start, curFont.imageRowBytes,
								destRect, srcOr, 0, 0, 0);
					} else
					{
						doHostBlit<variant1Xfer32>(&h3_info[index], (long) localPortBits, 16, 
								sourceRect, fontBlockPtr->start, curFont.imageRowBytes,
								destRect, srcOr, 0, 0, 0);
					}
				}
			}
		}
	}
	
	// Mark this character as being rendered
	renderedChars.SetBit(curChar);
}

#pragma mark -
#pragma mark ¥ Font Cache

/********************************************************************************
	FontCache::FontCache
		
	Static constructor for global fontCache.
*/
FontCache::FontCache()
{
	// Put all the fonts in the free list
	for (UInt32 index = 0; index < kNumFonts; ++index)
	{
		fonts[index].next = &fonts[index + 1];
		fonts[index].fontIndex = index;
	}
	fonts[kNumFonts - 1].next = 0;
	
	usedList.next = usedList.prev = 0;
	freeList.prev = 0;
	freeList.next = &fonts[0];
}

/********************************************************************************
	FontCache::Init
		
	Allocates memory for the font cache's block handles, on boards that are present
	in the system. Each board in the system has an array of kNumFonts block handles
	which point to the on-board location of the font's glyph data.
*/
void FontCache::Init()
{
	GrafPtr oldPort;
	CTabHandle ct = (CTabHandle) NewHandle(sizeof(ColorTable) + sizeof (ColorSpec) * 256);
	ct[0][0].ctFlags = 0;
	ct[0][0].ctSize = 255;
	for (UInt32 index = 0; index < 256; ++index)
	{
		ct[0][0].ctTable[index].value = index;
		ct[0][0].ctTable[index].rgb.red = ct[0][0].ctTable[index].rgb.green = 
				ct[0][0].ctTable[index].rgb.blue = index << 8 | index;
	}
	CTabChanged(ct);
	
	GetPort(&oldPort);
	OpenPort(&gFontPort);
	Rect portRect = { 0, 0, 128, 128 };
	NewGWorld(&gColorFontPort, 8, &portRect, ct, 0, 0);
	LockPixels(GetGWorldPixMap(gColorFontPort));
	SetPort(oldPort);
	SetRect(&gFontPort.portRect, 0, 0, 32767, 32767);
	SetRectRgn(gFontPort.visRgn, -32768, -32768, 32767, 32767);
	gFontPort.txMode = 0;
	gColorFontPort->txMode = 0;
}

/********************************************************************************
	FontCache::Terminate
		
	Clean up the font cache blocks, prepare for termination of the code fragment.
*/
void FontCache::Terminate()
{
	// QuickDraw keeps track of the ports that have been opened. If we terminate
	// and don't close the port, QD gets very unhappy.
	ClosePort(&gFontPort);
	ClosePort((GrafPtr) gColorFontPort);
	
	for (UInt32 index = 0; index < 16; ++index)
 	{
 		if ((h3_info[index].boardFlags & 1))
		{
			for (UInt32 fontIndex = 0; fontIndex < kNumFonts; ++fontIndex)
			{
				if (h3_info[index].fontCacheBlocks[fontIndex])
				{
					hrmFreeBlock(h3_info[index].fontCacheBlocks[fontIndex]);
				}
			}
		}
 	}
}

/********************************************************************************
	FontCache::NewFontData
		
	Pull a fontData struct off the free list, add it to the used list, and clear out
	the data in the struct;
*/
FontData *FontCache::NewFontData()
{
	FontData *curFont;

	// Are there any fontDatas on the free list?
	if (freeList.next)
	{
		// Pop the font off the free list...
		curFont = freeList.next;
		freeList.next = curFont->next;
		
		// ... And add it to the used list
		curFont->next = usedList.next;
		curFont->prev = 0;
		if (usedList.next)
			usedList.next->prev = curFont;
		else
			usedList.prev = curFont;
		usedList.next = curFont;
		
		curFont->Initialize();
		return curFont;
	} else
	{
		static int fontReplaceCounter = 0;
		if (fontReplaceCounter > 10)
		{
			fontReplaceCounter = 0;

			// All the fonts are currently in use (the free list is empty)
			UInt32 fontToReplace = (((UInt32) Random()) * FontCache::kNumFonts) >> 16;
			if (fontToReplace >= FontCache::kNumFonts)
				fontToReplace = FontCache::kNumFonts - 1;
			
			curFont = &fonts[fontToReplace];
			curFont->Initialize();
			return curFont;
		} else
			++fontReplaceCounter;
		
		return 0;
	}
}

/********************************************************************************
	FontCache::RemoveFontData
		FontData *inData
		
	Removes a fontData struct from the used list, and puts it on the free list
*/
void FontCache::RemoveFontData(FontData *inData)
{
	// Unlink the fontData from the used list
	if (inData->prev)
		inData->prev->next = inData->next;
	else
		usedList.next = inData->next;
	if (inData->next)
		inData->next->prev = inData->prev;
	else
		usedList.prev = inData->prev;
	
	inData->next = freeList.next;
	freeList.next = inData;
}

/********************************************************************************
	FontCache::GetFont
		StdTextInfo &info
		
	Returns a pointer to a font in the font cache that can be used to draw the given
	text size, style, fontID, and scaling parameters.
	
	First, this function looks through the list of fonts in the cache. If no fontData matched,
	this function tries to add this text font, style, and size to the cache.
*/
FontData *FontCache::GetFont(StdTextInfo &info)
{
	CGrafPtr	curPort = info.curPort;
	FontData 	*curFont;
	bool		found = false;
	
	// Determine the true font id and font size. The multiple if statements are because
	// of weirdness where SysFontSize can be zero, or GetSysFont can return applFont, etc.
	SInt16	actualSize = curPort->txSize;
	SInt16	actualFontID = curPort->txFont;
	if (!actualSize)	actualSize = LMGetSysFontSize();
	if (!actualSize)	actualSize = 12;
	if (actualFontID == systemFont) actualFontID = GetSysFont();
	if (actualFontID == applFont) actualFontID = GetAppFont();
	if (actualFontID == systemFont) actualFontID = GetSysFont();
	
	bool	inAntiAliased = false;
#ifdef USE_3D_TEXT
	SInt16	minSize;
	if (IsAntiAliasedTextEnabled(&minSize) && minSize <= actualSize)
		inAntiAliased = true;
#endif
	
	// Search through the list of fonts in use
	for (curFont = GetFirst(); curFont != 0; curFont = GetNext(curFont))
	{
		if (found = curFont->IsMatch(actualFontID, actualSize, curPort->txFace & 0x7F, 
				info.matchHorizScale, info.matchVertScale, inAntiAliased))
			break;		
	}
	
	if (!found)
	{
		// Double check to see if this is a roman script font
		if (FontToScript(curPort->txFont) != smRoman)
			return 0;
			
		// Acquire an unused FontData struct if possible
		if (curFont = NewFontData())
		{
			// Attempt to add the font to our cache
			if (!curFont->SetupFontData(info))
			{
				// Couldn't add the font
				RemoveFontData(curFont);
				return 0;
			}
		}	
	}
	
	return curFont;
}

#pragma mark -
#pragma mark ¥ Font Memory Blocks
/********************************************************************************
	CheckFontBlock
		StdTextInfo &info
		
	The font block is a bitmap, containing the glyphs for all the characters in a font.
	Every board in the system has its own copy of the font block for a particular font.
	
	This function makes sure the font block for the font we're drawing with is actually
	present on the board we'll be drawing on. Note that that if the font block is 
	already on another card, we attempt to copy the existing font data over. This is
	because we only have 1 bit mask showing which characters are imaged for a font, which
	means that all boards that have the font block allocated, need to have the same
	characters imaged into it.
*/
UInt32	CheckFontBlock(StdTextInfo &info)
{
	UInt16		fontIndex = info.curFont->fontIndex;
	mmBlock_t	*fontBlock = info.h3InfoPtr->fontCacheBlocks[fontIndex];
	bool		dataOnOtherCard = false;
	GrafPort	&localPort = gFontPort;

	if (!fontBlock)
	{
		// Need to allocate the font block on this card.
		fontBlock = hrmAllocateBlock(info.h3InfoPtr->board, (info.curFont->ascent + info.curFont->descent)
				* info.curFont->imageRowBytes * 16, HRM_MEMF_LINEAR | HRM_MEMF_RELOCATABLE, 
				HRM_MEM_PRI_CACHE);
				
		if (!fontBlock)
			return 0;
		fontBlock->notifyProc = FontBlockNotifyProc;
		fontBlock->userData = (void *) info.curFont;
		strcpy(fontBlock->comment, "Font Cache Block");
			
		// Does this font block already exist on another card?
		for (UInt32 index = 0; index < 16; ++index)
		{
			// Is there a board at this memory address?
			if ((h3_info[index].boardFlags & 1) && (h3_info[index].fontCacheBlocks[fontIndex]))
			{
				// Copy the font data from the other card to this one
				
				// rcf slow, slow, slow. Also, it won't work on Voodoo3, because of swizzling.
				// Should work fine on voodoo 4.
				BlockMoveDataUncached((void *) h3_info[index].fontCacheBlocks[fontIndex]->start,
						(void *) fontBlock->start, 
						(info.curFont->ascent + info.curFont->descent)
						* info.curFont->imageRowBytes * 16);
				
				dataOnOtherCard = true;
				break;
			}
		}
		
		info.h3InfoPtr->fontCacheBlocks[fontIndex] = fontBlock;

		if (!dataOnOtherCard)
		{
			// Blank out the font block to white. The font block is a 16x16 grid of characters.
			Fifo2DRegs		blitter(FindH3Info((void *) fontBlock->start));
			blitter.reg(reg_colorFore, 0x00000000);
			blitter.reg(reg_dstBaseAddr, (FxU32) fontBlock->start & kBaseAddrOffsetMask);
			blitter.reg(reg_dstFormat, info.curFont->imageRowBytes | SSTG_PIXFMT_8BPP);	

			blitter.reg(reg_dstSize, info.curFont->imageRowBytes | 
					((info.curFont->ascent + info.curFont->descent) << 20)); // Shift 4 to mult. by 16, shift 16 for field alignment
			blitter.reg(reg_dstXY, 0);
			blitter.reg(reg_command, SSTG_RECTFILL | SSTG_GO | kROPSource);
			blitter.go();

#if 0		
			// No glyph data on other card. Blank out the font block to white.
			// Note that we always treat this block as 1bpp, even if the block is antialiased (8bpp).
			localPort.portBits.baseAddr = (Ptr) fontBlock->start;
			localPort.portBits.rowBytes = info.curFont->imageRowBytes;
			FastSetRect(localPort.portBits.bounds, 0, 0, (info.curFont->ascent + info.curFont->descent) * 16,
					info.curFont->imageRowBytes << 3);
			SetPort(&localPort);
			EraseRect(&localPort.portBits.bounds);
			SetPort((GrafPtr) info.curPort);
#endif			
			// Also, zero out the RenderedChars array
			gFonts.fonts[fontIndex].renderedChars.Clear();
		}
	}
	
	return fontBlock->start;
}

/********************************************************************************
	FontBlockNotifyProc
		struct mmBlock_s *block
		unsigned long code
		
	This is the HRM memory manager callback proc to tell us when a font block
	is being deleted.
*/
void FontBlockNotifyProc(struct mmBlock_s *block, unsigned long code)
{
	if (code == HRM_MEM_NOTIFY_DISPOSE)
	{
		// Find the board that this block was on
		h3Info	*h3InfoPtr = FindH3Info((void *) block->start);
		
		// Find the fontData that this block belongs to
		FontData 	*curFont = (FontData *) block->userData;
		
		// Zero out the block's ptr in the font cache block list
		h3InfoPtr->fontCacheBlocks[curFont->fontIndex] = 0;
	}
}


#pragma mark -



/********************************************************************************
	ColorForDevice
		GDHandle gd
		RGBColor &color
		
	
*/
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
	GetQDColors
		GDHandle gd
		NQDDrawVars &drawVars
		
	Sets the foreColor and backColor in the info struct correctly.
*/
void GetQDColors(StdTextInfo &info, GDHandle gd)
{
	RGBColor	foreColor, backColor;
	
	if (info.curPort->portVersion & 0xC000)
	{
		// set foreground and background colors
		foreColor = info.curPort->rgbFgColor;
		backColor = info.curPort->rgbBkColor;
	} else
	{
		GetForeColor(&foreColor);
		GetBackColor(&backColor);
	}
	
	// In grayishTextOr mode, the foreground color is a 50% blend between the
	// port's fore and back colors.
	if (info.curPort->txMode == grayishTextOr)
	{
		foreColor.red = ((UInt32) foreColor.red + backColor.red) >> 1;
		foreColor.green = ((UInt32) foreColor.green + backColor.green) >> 1;
		foreColor.blue = ((UInt32) foreColor.blue + backColor.blue) >> 1;
	}

	info.foreColor = ColorForDevice(gd, foreColor);
	info.backColor = ColorForDevice(gd, backColor);
}

/********************************************************************************
	StdTextPatch
		short byteCount
		UInt8 *textBuf
		Point numer
		Point denom
		
	Bypass versions of StdTextPatch and StdTextMeasPatch
*/
short StdTextMeasPatch2(short byteCount, UInt8 *textBuf, Point *numer, 
					Point *denom, FontInfo *info)
{
	// This version just bypasses everything.
	return PatchStdTextMeas::PatchFn(byteCount, textBuf, numer, denom, info);
}

void StdTextPatch2(short byteCount, UInt8 *textBuf, Point numer, Point denom)
{
	// This version just bypasses everything.
	PatchStdText::PatchFn(byteCount, textBuf, numer, denom);
}
