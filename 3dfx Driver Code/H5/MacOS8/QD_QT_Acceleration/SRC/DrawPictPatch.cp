/********************************************************************************
	 DrawPictPatch.cp
	 
	 
	 Monroe Williams
	 Critical Path Software
	 
*/
 
#pragma mark ¥ Notes

/*
	Random notes
	
	We only handle the case where the destination rect of DrawPicture is entirely contained
	on one screen.
	
	Theory of operation:
	
	This code will cache the bitmap data in certain types of pictures on the card.
	The cases in which it can operate are in practice quite common, but are actually a small subset
	of the ways in which Pictures can be used.  The only case in which this code will cache a picture
	is as follows:
	- the picture is being drawn entirely on one screen
	- the picture contains a single StdBits opcode, and does no other drawing
	
	This code does not attempt to parse picture data directly.  The picture data format is complex,
	ill-documented, and extensible, and verifying correct operation of code that attempted to
	parse it would be extremely difficult.
	
	A couple of other facts complicate things signifigantly.  
	
	One is that the "picSize" field does not accurately represent the size of the picture data.  
	The picSize field in a picture is only 16 bits -- not enough to represent the size of most 
	modern pictures -- and is therefore ignored, or at least not used as-is, by MacOS.  
	
	The other is that the usual way of determining the size of a picture -- GetHandleSize --
	is not usable during the composite benchmarks in MacBench.  MacBench uses "fake" handles
	(i.e. local pointers to pointers) instead of real MacOS Memory Manager handles for all of
	the pictures that it draws during its recorded benchmark tests.  This keeps us from using
	any memory manager routines on these "handles", and also prevents us from using the
	address of the master pointer of a PicHandle as an additional check for cache hit
	determination.
	
	The approach taken here is to actually call DrawPicture after replacing the QDProcs in the
	current port with local versions.  Most of the procs simply set a global noting that the 
	picture contains non-copybits opcodes and then call through to the original proc.  The 
	GetPicProc is used to calculate the actual size of the picture data, which is used after 
	DrawPicture returns as part of the cache entry matching data.
	
	A large part of the time taken up by DrawPicture is spent decompressing the picture data.
	
	
*/

#pragma mark ¥ Includes

#include <Quickdraw.h>
#include "NQDAcceleration.h"
#include "DynamicPatches.h"
#include "RegionParser.h"
#include "Utilities.h"
#include "DrawPictPatch.h"
#include "BitBlit.h"

#pragma mark ¥ Switches

// If this is set to 1, the code will use the stretch blitter to shrink.
#define DRAW_STRETCH_SHRINK 0

#pragma mark ¥ Types

struct PictCacheGlobalsPerBoard
{
};

enum
{
	MaxChecksum = 4096
};

#pragma mark ¥ Prototypes

template <class screenVariant>
static void PictCacheHitParsed(void *data, Rect &rect);

static void PurgeEntry(PictCacheEntry &entry);
static void PurgeAllEntries(void);
static PictCacheEntry *SearchCache(PicHandle pict);
static bool PreCache(PicHandle pict);
static void PostCache(PicHandle pict);
static UInt32 checksum(char *data, UInt32 length);
static void pictCacheNotifyProc(struct mmBlock_s *block, unsigned long code);
static void BlitterMove(h3Info *board, UInt32 to, UInt32 from, UInt32 size);

// The intercepts
static pascal void local_bitsProc(BitMap *srcBits, Rect *srcRect, Rect *dstRect, short mode, RgnHandle maskRgn);
static pascal void local_textProc(short byteCount, Ptr textBuf, Point numer, Point denom);
static pascal void local_lineProc(Point newPt);
static pascal void local_rectProc(GrafVerb verb, Rect *r);
static pascal void local_rRectProc(GrafVerb verb, Rect *r, short ovalWidth, short ovalHeight);
static pascal void local_ovalProc(GrafVerb verb, Rect *r);
static pascal void local_arcProc(GrafVerb verb, Rect *r, short startAngle, short arcAngle);
static pascal void local_polyProc(GrafVerb verb, PolyHandle poly);
static pascal void local_rgnProc(GrafVerb verb, RgnHandle rgn);
static pascal void local_commentProc(short kind, short dataSize, Handle dataHandle);
static pascal short local_txMeasProc(short byteCount, Ptr textAddr, Point *numer, Point *denom, FontInfo *info);
static pascal void local_getPicProc(Ptr dataPtr, short byteCount);
static pascal void local_putPicProc(Ptr dataPtr, short byteCount);
static pascal void local_opcodeProc(Rect *fromRect, Rect *toRect, short opcode, short version);
static pascal void local_stdPixProc(PixMap *src, Rect *srcRect, MatrixRecord *matrix, short mode, RgnHandle mask, PixMap *matte, Rect *matteRect, short flags);
static void local_glyphsProc(void *dataStream, ByteCount size);

// These are in the NQD library.

// Do all the BitMap * decoding that's built into CopyBits and fill out an NQDPixMap accordingly.
extern "C" void	BitsToPix( BitMap * inAnyMap, NQDPixMap * outPixMap, GDHandle scrnDev );

// Create a scaletable for indexed sources.
extern "C" UInt32 *	MakeScaleTable(	NQDPixMapPtr pix, 
									Int32 colorizeFlag, 
									RGBColor * foreColor, 
									RGBColor * backColor, 
									Int32 dstShift );

#pragma mark ¥ Globals

bool				gInDrawPictPatch = false;

PictCacheGlobals	gPictCache;


/********************************************************************************
	DrawPictPatchInit		
	
	Set up the globals in this file.
*/
void DrawPictPatchInit(void)
{
	SetStdCProcs(&gPictCache.stdProcs);
	SetStdCProcs(&gPictCache.localProcs);
	
	gPictCache.localProcs.textProc = NewQDTextProc(local_textProc);
	gPictCache.localProcs.lineProc = NewQDLineProc(local_lineProc);
	gPictCache.localProcs.rectProc = NewQDRectProc(local_rectProc);
	gPictCache.localProcs.rRectProc = NewQDRRectProc(local_rRectProc);
	gPictCache.localProcs.ovalProc = NewQDOvalProc(local_ovalProc);
	gPictCache.localProcs.arcProc = NewQDArcProc(local_arcProc);
	gPictCache.localProcs.polyProc = NewQDPolyProc(local_polyProc);
	gPictCache.localProcs.rgnProc = NewQDRgnProc(local_rgnProc);
	gPictCache.localProcs.bitsProc = NewQDBitsProc(local_bitsProc);
	gPictCache.localProcs.commentProc = NewQDCommentProc(local_commentProc);
	gPictCache.localProcs.txMeasProc = NewQDTxMeasProc(local_txMeasProc);
	gPictCache.localProcs.getPicProc = NewQDGetPicProc(local_getPicProc);
	gPictCache.localProcs.putPicProc = NewQDPutPicProc(local_putPicProc);
	gPictCache.localProcs.opcodeProc = NewQDOpcodeProc(local_opcodeProc);
	gPictCache.localProcs.newProc1 = (UniversalProcPtr)NewStdPixProc((StdPixProcPtr)local_stdPixProc); /* StdPix bottleneck -- see ImageCompression.h */
	gPictCache.localProcs.glyphsProc = NewQDStdGlyphsProc(local_glyphsProc);	/* was newProc2; now used in Unicode text drawing */

}

/********************************************************************************
	DrawPictPatchTerminate	
	Clean up after the DrawPict patch
*/
void DrawPictPatchTerminate(void)
{
	PurgeAllEntries();
}

/********************************************************************************
	DrawPictPatch
		PicHandle myPicture
		const Rect *dstRect
		
	This is our patch on DrawPicture().  
	
*/
void DrawPictPatch(PicHandle myPicture, Rect *dstRect)
{
	bool			cardSpecificDest = false;
	
	// If our patch gets called reentrantly (which can happen), we shouldn't process the
	// parameters twice, however we do need to call through to the OS.
	if (gInDrawPictPatch)
	{
		PatchDrawPicture::PatchFn(myPicture, dstRect);
		return;
	}
	gInDrawPictPatch = true;
	
	// Check to see if we're turned off
	if (!PatchCommonBlitAccept())
		goto Callthrough;
	
	// Get the dest port
	GetPort((GrafPtr *) &gPictCache.port);
	if (!gPictCache.port)
		goto Callthrough;
		
	// Bail if we're recording a picture
	if (gPictCache.port->picSave)
		goto Callthrough;

	// Bail if custom grafProcs are in place.
	if(gPictCache.port->grafProcs != nil)
		goto Callthrough;

	// Is this a new or old style port?
	if (gPictCache.port->portVersion & 0xC000)
	{
		gPictCache.dstPixMap = *gPictCache.port->portPixMap;
		
		if (gPictCache.dstPixMap->baseAddr == LMGetMainDevice()[0]->gdPMap[0]->baseAddr)
		{
			// This is a normal blit to the screen
		}
		else if(FindH3Info(gPictCache.dstPixMap->baseAddr) != nil)
		{
			// This is a blit to somewhere on one of our cards
			cardSpecificDest = true;
		}
		else
		{
			// This is not a destination we should deal with
			goto Callthrough;
		}
	} 
	else
	{
		if (((GrafPtr) gPictCache.port)->portBits.baseAddr == LMGetMainDevice()[0]->gdPMap[0]->baseAddr)
		{
			// This is a normal blit to the screen
		}
		else if(FindH3Info(((GrafPtr) gPictCache.port)->portBits.baseAddr) != nil)
		{
			// This is a blit to somewhere on one of our cards
			cardSpecificDest = true;
		}
		else
		{
			// This is not a destination we should deal with
			goto Callthrough;
		}

		gPictCache.fakeDstPixMap = LMGetMainDevice()[0][0].gdPMap[0][0];
		gPictCache.fakeDstPixMap.bounds = ((GrafPtr)gPictCache.port)->portBits.bounds;
		gPictCache.dstPixMap = &gPictCache.fakeDstPixMap;
	}

	// Loop through each device in the DeviceList
	GDHandle	gd, oldGD;
	oldGD = GetGDevice();
	
	if(cardSpecificDest)
	{
		gd = oldGD;
	}
	else
	{
		gd = GetDeviceList();
	}
	
	// Stash the original dstRect so we can use it later.
	gPictCache.dstRect = *dstRect;

	// Offset the dest rect into global coordinates
	Rect dstRectGlobal = *dstRect;
	FastOffsetRect(dstRectGlobal, -gPictCache.dstPixMap->bounds.left, -gPictCache.dstPixMap->bounds.top);
	
	gPictCache.device = nil;

	for (; gd != 0; cardSpecificDest?(gd = nil):(gd = GetNextDevice(gd)))
	{
		// Is the gDevice active?
		if (gd[0][0].gdFlags & 0x8000)
		{
			// Does this gDevice intersect the destRect?

			if (FastSectRect2(dstRectGlobal, gd[0][0].gdRect))
			{
				if(gPictCache.device == nil)
				{
					// This is the first screen the dstRect intersects.
					gPictCache.device = gd;
				}
				else
				{
					// The dstRect intersected more than one screen.
					goto Callthrough;
				}
			}	
		}
	}
	
	if(gPictCache.device != nil)
	{
		// Figure out if the picture is being drawn on one of our screens.
		gPictCache.board = FindH3Info(gPictCache.device[0]->gdPMap[0]->baseAddr);
		
		if(gPictCache.board == nil)
		{
			// Drawing on a screen that doesn't belong to us.
			goto Callthrough;
		}
		
		if (gPreferences->m2D[gPictCache.board->prefsIdx]->disableFlags & kDisableDrawPictPatch)
			goto Callthrough;

		
		// See if this picture is already cached
		PictCacheEntry *hit = SearchCache(myPicture);
		
		if(hit != nil)
		{
			// This is a cache hit.  Draw the picture from the cache.
			RgnParserParams params;
			BitsToPix((BitMap*)gPictCache.dstPixMap, &gPictCache.dstPix, gPictCache.device);
			
			params.blitFunction = nil;
						
			switch(gPictCache.dstPix.pixelSize)
			{
				case 8:		
					switch(hit->cacheDepth)
					{
						case 1:		params.blitFunction = PictCacheHitParsed<variant1to8>;		break;
						case 8:		params.blitFunction = PictCacheHitParsed<variantBitBlt8>;		break;
					}
				break;

				case 16:		
					switch(hit->cacheDepth)
					{
						case 1:		params.blitFunction = PictCacheHitParsed<variant1to16>;	break;
						case 16:	params.blitFunction = PictCacheHitParsed<variantBitBlt16>;	break;
					}
				break;

				case 32:		
					switch(hit->cacheDepth)
					{
						case 1:		params.blitFunction = PictCacheHitParsed<variant1to32>;		break;
						case 32:	params.blitFunction = PictCacheHitParsed<variantBitBlt32>;	break;
					}
				break;
			}
			
			if(params.blitFunction == nil)
			{
				// Something has gone horribly wrong.  Purge this entry and bail.
				PurgeEntry(*hit);
				goto Callthrough;
			}
			
			params.blitDataPtr = (void*)hit;
			params.parseTopToBottom = true;
			params.parseLeftToRight = true;
			
			SetGDevice(gPictCache.device);
			
			GetArbitration(gPictCache.board);
			
			RegionParserDirect(params, gPictCache.port->visRgn, gPictCache.port->clipRgn, nil, *dstRect);
			
			// Wait for all blits to finish.
			while(!H3BlitFinish((long)(gPictCache.board)))
				;

			SetGDevice(oldGD);
		}
		else
		{
			// This is a cache miss.  We may want to put this picture in the cache.
			if(PreCache(myPicture))
			{
				// Looks promising.  Try and cache the picture.
				
				// Draw the picture with our custom procs and see if it pans out.
				bool usingTrickPort = false;
				CGrafPort trickPort;
				RgnHandle saveVis, saveClip;
				
				if (gPictCache.port->portVersion & 0xC000)
				{
					// New-style CGrafPort.  Just replace the grafProcs.
					gPictCache.port->grafProcs = &gPictCache.localProcs;
				}
				else
				{
					// old-style GrafPort
					/*
						If DrawPict is called in an old-style port, the StdBitsProc will be called
						with a 1-bit source.  This is bad.  To avoid this, we need to create a
						CGrafPort that will work in place of the real current port.
					*/
					usingTrickPort = true;
					OpenCPort(&trickPort);

					trickPort.grafProcs = &gPictCache.localProcs;
					trickPort.portRect = gPictCache.port->portRect;
					trickPort.portPixMap[0]->bounds = gPictCache.dstPixMap->bounds;

					saveVis = trickPort.visRgn;
					trickPort.visRgn = gPictCache.port->visRgn;

					saveClip = trickPort.clipRgn;
					trickPort.clipRgn = gPictCache.port->clipRgn;
					
					SetPort((GrafPtr)&trickPort);
				}
				
				PatchDrawPicture::PatchFn(myPicture, dstRect);
				
				if(usingTrickPort)
				{
					// Clean up the trick grafport
					SetPort((GrafPtr)gPictCache.port);
					trickPort.visRgn = saveVis;
					trickPort.clipRgn = saveClip;
					CloseCPort(&trickPort);
				}
				else
				{
					// Restore the grafProcs in the real port
					gPictCache.port->grafProcs = nil;
				}
				
				// Figure out whether it worked or not, and set up the actual cache entry.
				PostCache(myPicture);
			}
		}
		
		gInDrawPictPatch = false;
		return;
	}	

Callthrough:
	// Generally, if we get here, it means we are choosing to not attempt to cache or draw this picture.
	gInDrawPictPatch = false;
	PatchDrawPicture::PatchFn(myPicture, dstRect);
}

#pragma mark -

/********************************************************************************
	PictCacheHitParsed
		PictCacheEntry *entry
		Rect &rect

	Called through RegionParserDirect to draw an entry from the picture cache.
	
*/
template <class screenVariant>
static void PictCacheHitParsed(void *data, Rect &rect)
{
	PictCacheEntry *entry = (PictCacheEntry*)data;
	Rect adjDstClip = rect;
	FastOffsetRect(adjDstClip, -gPictCache.dstPix.bounds.left, -gPictCache.dstPix.bounds.top);

	if(gNewNQDOperation)
	{
		Rect adjDstRect = entry->mappedDstRect;
		MapRect(&adjDstRect, &entry->picFrame, &gPictCache.dstRect);
		FastOffsetRect(adjDstRect, -gPictCache.dstPix.bounds.left, -gPictCache.dstPix.bounds.top);	
		
		/* Colorized 1-bit sources need to have the colors redone when the
			destination changes (depth or ctSeed).  The original RGB colors
			are stored in the cache entry. 
		*/
		if(screenVariant::srcPixelSize == 1)
		{
			// Reconstruct the necessary destination-specific colors
			entry->scale1bit[0] = Color2Index(&entry->rgbScale0);
			entry->scale1bit[1] = Color2Index(&entry->rgbScale1);
			entry->foreColor = Color2Index(&entry->rgbFore);
			entry->backColor = Color2Index(&entry->rgbBack);
		}
		
		doScreenBlit<screenVariant>(
			gPictCache.board,
			(UInt32)entry->cacheBlock->start,
			entry->cacheRowBytes,
			entry->cacheRect,
			(UInt32)gPictCache.dstPix.baseAddr,
			gPictCache.dstPix.rowBytes,
			adjDstRect,
			adjDstClip,
			false,
			entry->mode,
			entry->scale1bit,
			entry->foreColor,
			entry->backColor,
			true,
			entry->colorizeFlag);
		
		gNewNQDOperation = false;
	}
	else
	{
		Fifo2DRegs		blitter(gPictCache.board);
		
		// This is declared in BitBlit.cp
		extern UInt32	gBitBltCmdValue;
		
		blitter.clip1_cmd_go(
			(adjDstClip.left)  | ((adjDstClip.top) << 16),
			(adjDstClip.right) | ((adjDstClip.bottom) << 16),
			gBitBltCmdValue);
	}
}


/********************************************************************************
	PurgeEntry

	Purge one entry from the picture cache.
	
*/
static void PurgeEntry(PictCacheEntry &entry)
{
	if(entry.cacheBlock)
	{
		hrmFreeBlock(entry.cacheBlock);
		entry.cacheBlock = nil;
	}
	entry.inUse = false;
}

/********************************************************************************
	PurgeAllPictures

	Completely empty the picture cache.
	
*/
static void PurgeAllEntries(void)
{
	int i;
	for(i=0; i < PictCacheSize; i++)
	{
		if(gPictCache.entries[i].inUse)
		{
			PurgeEntry(gPictCache.entries[i]);
		}
	}
}

/********************************************************************************
	SearchCache
		PicHandle pict

	Search for a match in the picture cache.
	
*/
static PictCacheEntry *SearchCache(PicHandle pict)
{
	PictCacheEntry *result = nil;
	Rect picFrame = pict[0]->picFrame;
	short picSize = pict[0]->picSize;
		
	gPictCache.shrunkenMismatch = PictCacheSize;
	gPictCache.firstFreeEntry = PictCacheSize;
	
	// MBW -- Linear search.  Can we make this better?
	int i;
	for(i=0; i < PictCacheSize; i++)
	{
		PictCacheEntry *cur = &(gPictCache.entries[i]);
		if(cur->inUse)
		{
			// This entry in the cache is valid.
			if(cur->board != gPictCache.board)
			{
				// This entry is for a different board
				continue;
			}
			
			// Make sure the entry is compatible with the current state of the screen
			switch(cur->cacheDepth)
			{
				case 1:
					// This entry can be used at any depth
				break;
				
				case 8:
					if(	(gPictCache.device[0]->gdPMap[0]->pixelSize == cur->screenDepth) && 
						(gPictCache.device[0]->gdPMap[0]->pmTable[0]->ctSeed == cur->ctSeed))
					{
						// Depth and color table match.  This will do.
					}
					else
					{
						// This entry can't be used with the current screen depth and color table.
						PurgeEntry(*cur);
						continue;
					}
				break;
				
				case 16:
				case 32:
					if(gPictCache.device[0]->gdPMap[0]->pixelSize == cur->screenDepth)
					{
						// Depth and color table match.  This will do.
					}
					else
					{
						// This entry can't be used with the current screen depth.
						PurgeEntry(*cur);
						continue;
					}
				break;
			}
			
			// See if the picture fits the description of the one in the cache.

			// Cheap checks first.
			if(cur->picSize != picSize)
				continue;
			if(!FastEqualRect(cur->picFrame, picFrame))
				continue;
			
			// When we cached the picture, we made a note of where the end opcode was.
			// See if this picture has an end opcode in the same place.
			if(cur->type2pict)
			{
				UInt16 *opcodes = (UInt16*)(*pict + 1);
				if(opcodes[cur->endOpcodeOffset] != 0x00FF)
					continue;
			}
			else
			{
				UInt8 *opcodes = (UInt8*)(*pict + 1);
				if(opcodes[cur->endOpcodeOffset] != 0xFF)
					continue;
			}
			
			// Finally, the most expensive check -- checksum the picture data.
			if(cur->checksum != checksum((char*)(*pict + 1), cur->checksumLength))
			{
				// Hopefully we won't hit this case very often...
				continue;
			}
			
			// This is the right picture.  If this cache entry has a shrink constraint, see if it matches.
			if(cur->hShrink || cur->vShrink)
				gPictCache.shrunkenMismatch = i;

			Rect curDstRect = cur->mappedDstRect;
			MapRect(&curDstRect, &cur->picFrame, &gPictCache.dstRect);
			
			if((curDstRect.right - curDstRect.left) < (cur->srcRect.right - cur->srcRect.left))
			{
				// Drawing the picture at this size requires a horizontal shrink.
				if(!cur->hShrink)
					continue;
				if((curDstRect.right - curDstRect.left) != cur->hShrinkSize)
					continue;
			}
			else
			{
				// Drawing the picture at this size does not require a horizontal shrink
				if(cur->hShrink)
					continue;
			}
			
			if((curDstRect.bottom - curDstRect.top) < (cur->srcRect.bottom - cur->srcRect.top))
			{
				// Drawing the picture at this size requires a vertical shrink.
				if(!cur->vShrink)
					continue;
				if((curDstRect.bottom - curDstRect.top) != cur->vShrinkSize)
					continue;
			}
			else
			{
				// Drawing the picture at this size does not require a horizontal shrink
				if(cur->vShrink)
					continue;
			}
			
			// If we got here, we have a match.  Return it.
			gPictCache.shrunkenMismatch = PictCacheSize;
			result = cur;
			break;
		}
		else
		{
			// Stash the index of a free entry so the replacement code can use it later.
			if(gPictCache.firstFreeEntry == PictCacheSize)
			{
				gPictCache.firstFreeEntry = i;
			}
		}
	}
	
	return(result);
}

/********************************************************************************
	PreCache
		PicHandle pict

	Determine whether a picture might be cachable and do some setup.
*/
static bool PreCache(PicHandle pict)
{
	// See if this picture is worth caching, and do some preparation.
	
	Rect picFrame = pict[0]->picFrame;
	short picSize = pict[0]->picSize;
	
	// If the picture is too small, don't bother.
	{
		if(	((picFrame.right - picFrame.left) <= 0) ||
			((picFrame.bottom - picFrame.top) <= 0))
		{
			// For some reason, negative sizes seem to happen from time to time.
			return(false);
		}
	}
	
	// Check the first couple of opcodes to see whether this is a type 2 picture
	UInt16 *opcodes = (UInt16*)(*pict + 1);
	if((opcodes[0] == 0x0011) && (opcodes[1] == 0x02FF))
	{
		// Type 2 picture;
		gPictCache.current.type2pict = true;
	}
	else if(((UInt8*)opcodes)[0] == 0x11)
	{
		// Type 1 picture;
		gPictCache.current.type2pict = false;
	}
	else
	{
		// WTF?
		return(false);
	}

	// Set up for the cache attempt.
	gPictCache.current.inUse = false;
	gPictCache.current.cacheBlock = nil;
	gPictCache.current.picSize = picSize;
	gPictCache.current.picFrame = picFrame;
	gPictCache.current.banded = false;
	gPictCache.current.bandOffset = 0;

	gPictCache.allowCache = false;
	gPictCache.preventCache = false;
	gPictCache.realPictSize = 0;

	return(true);
}

/********************************************************************************
	PostCache
		PicHandle pict

	Do some post-processing after trying to cache a picture.
*/
static void PostCache(PicHandle pict)
{
	// Do the final steps of caching the picture.
	if(!gPictCache.current.inUse || gPictCache.preventCache)
	{
		/* Something happened while drawing the picture that prevents us from
			caching it.  Don't put this entry in the cache.
		*/
		if(gPictCache.current.cacheBlock != nil)
		{
			// The cache block was allocated.  Deallocate it.
			PurgeEntry(gPictCache.current);
		}
		return;
	}
	
	if(gPictCache.current.cacheBlock == nil)
	{
		// Something went very wrong.
		return;
	}
	
	// Now that we have the real size of the picture, calculate the checksum.
	if(gPictCache.current.type2pict)
		gPictCache.current.checksumLength = gPictCache.realPictSize - 2;
	else
		gPictCache.current.checksumLength = gPictCache.realPictSize - 1;
	
	// If the picture is large, there's not much point in checksumming _all_ the data.
	if(gPictCache.current.checksumLength > MaxChecksum)
	{
		gPictCache.current.checksumLength = MaxChecksum;
	}
	
	char *opcodes = (char*)(*pict + 1);
	gPictCache.current.checksum = checksum(opcodes, gPictCache.current.checksumLength);
	
	// Stash the offset of the end opcode
	if(gPictCache.current.type2pict)
	{
		gPictCache.current.endOpcodeOffset = ((gPictCache.realPictSize - 2) >> 1);
	}
	else
	{
		gPictCache.current.endOpcodeOffset = (gPictCache.realPictSize) - 1;
	}
	
	// Locate a cache entry to be replaced.  
	UInt32 index = PictCacheSize;
	
	if(gPictCache.current.hShrink || gPictCache.current.vShrink)
	{
		// If the cache search found a shrunk entry for this picture that didn't have matching
		// shrink constraints, replace that entry.
		index = gPictCache.shrunkenMismatch;
	}
		
	if(index == PictCacheSize)
	{
		index = gPictCache.firstFreeEntry;
		
		// look for a free cache entry
	for(index = 0; index < PictCacheSize; index++)
	{
		if(!gPictCache.entries[index].inUse)
			break;
	}
	}
	
	// MBW -- XXX -- Is a completely random replacement policy really good enough?
	// No free cache entries.  Replace one at random.
	if(index == PictCacheSize)
	{
		index = ((UInt32) Random() % PictCacheSize);
	}
	
	// Purge the old entry if necessary
	if(gPictCache.entries[index].inUse)
	{
		PurgeEntry(gPictCache.entries[index]);
	}
	
	// Put this entry into the cache.
	gPictCache.entries[index] = gPictCache.current;
	
	// Set up the refcon and purge callback for the cache block
	gPictCache.entries[index].cacheBlock->userData = (void*)(gPictCache.entries + index);
	gPictCache.entries[index].cacheBlock->notifyProc = pictCacheNotifyProc;
	gPictCache.entries[index].cacheBlock->priority = HRM_MEM_PRI_CACHE;
	strcpy(gPictCache.entries[index].cacheBlock->comment, "Picture Cache Block");
	
	// Stash the other matching information for this entry
	gPictCache.entries[index].board = gPictCache.board;
	gPictCache.entries[index].screenDepth = gPictCache.device[0]->gdPMap[0]->pixelSize;
	if(gPictCache.entries[index].screenDepth == 8)
	{
		gPictCache.entries[index].ctSeed = gPictCache.device[0]->gdPMap[0]->pmTable[0]->ctSeed;
	}
}

static UInt32 checksum(char *data, UInt32 length)
{
	UInt32 result = 0;
	UInt32 *p = (UInt32*)data;
	
	// This is a really cheap excuse for a checksum.  
	// Feel free to drop in a better algorithm at any time.
	// Just pick something that's not too slow.
	// Also, make sure this function doesn't call the memory manager.
	
	for(int i = length / 4; i != 0; i--)
	{
		// xor the data with the running checksum
		result ^= *p++;
		// and rotate the running total by 5 bits.  
		// Why 5 bits?  5 is relatively prime to 32
		result = __rlwinm(result, 5, 0, 31);
	}
	
	// Take care of leftover bytes
	if(length & 0x03)
	{
		// Mask off the bits in the final word that are past the end of the data.
		// Note the big-endian dependency here.
		result ^= (*p & (0xFFFFFFFF << ((4 - (length & 0x03)) << 3)));
	}
	
	return(result);
}

static void pictCacheNotifyProc(struct mmBlock_s *block, unsigned long code)
{
	PictCacheEntry *entry = (PictCacheEntry*)block->userData;
	
	if (code == HRM_MEM_NOTIFY_DISPOSE)
	{
		// This entry has gone away.
		entry->cacheBlock = nil;
		entry->inUse = false;
	}
}

/********************************************************************************
	BlitterMove
		h3Info *board
		UInt32 to
		UInt32 from
		UInt32 size

	Shovel data around on the board (just like BlockMove), using the blitter.
*/
static void BlitterMove(h3Info *board, UInt32 to, UInt32 from, UInt32 size)
{
	Fifo2DRegs		blitter(board);

	const UInt32 maxWidth = 1024;
	const UInt32 maxHeight = 1024;
	UInt32 blockSize;
	UInt32 blockHeight;
	UInt32 blockWidth;
	bool moveBackwards = (from < to);
		
	if(moveBackwards)
	{
		// Run the whole thing backwards
		to += size;
		from += size;
	}

	// Move large blocks as much as possible
	while(size != 0)
	{
		blockHeight = size / maxWidth;
		
		if(blockHeight != 0)
		{
			// This will be a block of one or more maximum width lines
			blockWidth = maxWidth;
			
			// The largest block we will move is maxWidth x maxHeight
			if(blockHeight > maxHeight)
				blockHeight = maxHeight;
		}
		else
		{
			// One-line block at the end
			blockHeight = 1;
			blockWidth = size;
		}
		
		blockSize = blockWidth * blockHeight;

		if(moveBackwards)
		{
			to -= blockSize;
			from -= blockSize;
		}
		
		blitter.reg(reg_dstBaseAddr, to & 0x03FFFFFF);
		blitter.reg(reg_dstFormat, SSTG_PIXFMT_8BPP | maxWidth);
		blitter.reg(reg_srcBaseAddr, from & 0x03FFFFFF);
		blitter.reg(reg_srcFormat, SSTG_PIXFMT_8BPP | maxWidth);
		blitter.reg(reg_srcXY, moveBackwards?((blockWidth - 1) | ((blockHeight - 1) << 16)):(0));
		blitter.reg(reg_dstSize, blockWidth | (blockHeight << 16));
		blitter.reg(reg_dstXY, moveBackwards?((blockWidth - 1) | ((blockHeight - 1) << 16)):(0));

		blitter.reg(reg_command, 
			(moveBackwards?(SSTG_XDIR | SSTG_YDIR):(0)) | 
			SSTG_BLT |
			SSTG_ROP_SRC << SSTG_ROP0_SHIFT |
			SSTG_GO);
			
		blitter.go();
		
		if(!moveBackwards)
		{
			to += blockSize;
			from += blockSize;
		}

		size -= blockSize;
	}
	
}


#pragma mark ¥ Intercepts
#pragma mark -

static pascal void local_bitsProc(BitMap *srcBits, Rect *srcRect, Rect *dstRect, short mode, RgnHandle maskRgn)
{
	if(gPictCache.current.inUse)
	{
		// We've already seen one stdBits opcode this picture.

		// See if this might be a banded picture we can handle.
		
		// I don't even want to think about the shrink + banded case.
		if(gPictCache.current.hShrink || gPictCache.current.vShrink)
			goto Callthrough;
		
		// The dithering case could get ugly, too.
		if(mode & ditherCopy)
			goto Callthrough;
		
		// The source rects must be just right.
		if(	(gPictCache.current.srcRect.bottom != srcRect->top) ||
			(gPictCache.current.srcRect.right != srcRect->right) ||
			(gPictCache.current.srcRect.left != srcRect->left))
		{
			goto Callthrough;
		}
		
		// The dest rects must be just right as well.
		if(	(gPictCache.current.dstRect.bottom != dstRect->top) ||
			(gPictCache.current.dstRect.right != dstRect->right) ||
			(gPictCache.current.dstRect.left != dstRect->left))
		{
			goto Callthrough;
		}
		
		// Make sure we can allocate enough additional space on the board for this band.
		{
			mmBlock_t	*newCache;
			UInt32 newCacheSize = gPictCache.current.cacheRowBytes;
			newCacheSize *= srcRect->bottom - gPictCache.current.srcRect.top;

			newCache = 
				hrmAllocateBlock(
					gPictCache.current.board->board, 
					newCacheSize, 
					HRM_MEMF_LINEAR | HRM_MEMF_RELOCATABLE, 
					HRM_MEM_PRI_CACHE);
			
			if(newCache == nil)
			{
				// Not enough space on the board.  Bail.
				goto Callthrough;
			}
			
			// The allocation succeeded.

			// Copy the existing data to the new block.
			gPictCache.current.bandOffset = gPictCache.current.cacheRowBytes * 
					(gPictCache.current.srcRect.bottom - gPictCache.current.srcRect.top);
			
			BlitterMove(gPictCache.current.board, 
						newCache->start, 
						gPictCache.current.cacheBlock->start, 
						gPictCache.current.bandOffset);
			
			// Dispose of the old block
			hrmFreeBlock(gPictCache.current.cacheBlock);
			
			// and keep track of the new block
			gPictCache.current.cacheBlock = newCache;
		}
		
		// If we got here, we will try to accumulate further bands of a banded picture.
		gPictCache.current.banded = true;
	}
	
	if((maskRgn != nil) && (maskRgn[0] != nil) && (maskRgn[0]->rgnSize > 10))
	{
		// Complex mask region.  Just say no.
		goto Callthrough;
	}
	
	// This is the call that attempts to cache the picture.
	gPictCacheFillAttempt = true;	
	StdBitsPatch(srcBits, srcRect, dstRect, mode, maskRgn);
	gPictCacheFillAttempt = false;

	if(gPictCache.current.inUse)
	{
		Rect mappedDstRect;
		
		// Transform dstRect back to the coordinate space of the picture.
		mappedDstRect = *dstRect;
		MapRect(&mappedDstRect, &gPictCache.dstRect, &gPictCache.current.picFrame);

		if(!gPictCache.current.banded)
		{
			gPictCache.current.srcRect = *srcRect;
			gPictCache.current.mappedDstRect = mappedDstRect;
		
			// This may be the first band of a banded picture.  Stash some information just in case.
			gPictCache.current.unmappedMode = mode;
			gPictCache.current.dstRect = *dstRect;
		}	
		else
		{
			// This is not the first band of a banded picture.  Accumulate the rects.
			gPictCache.current.cacheRect.bottom += srcRect->bottom - gPictCache.current.srcRect.bottom;
			gPictCache.current.srcRect.bottom = srcRect->bottom;
			gPictCache.current.mappedDstRect.bottom = mappedDstRect.bottom;			
			gPictCache.current.dstRect.bottom = dstRect->bottom;
		}
	}
	else
	{
		// If we failed to fill the cache entry, get out of the way.
		gPictCache.preventCache = true;
		gPictCache.port->grafProcs = nil;
	}
	return;
	
Callthrough:

	gPictCache.preventCache = true;
	gPictCache.port->grafProcs = nil;

	CallQDBitsProc(gPictCache.stdProcs.bitsProc, srcBits, srcRect, dstRect, mode, maskRgn);
}


static pascal void local_textProc(short byteCount, Ptr textBuf, Point numer, Point denom)
{
	gPictCache.preventCache = true;
	gPictCache.port->grafProcs = nil;
	
	CallQDTextProc(gPictCache.stdProcs.textProc, byteCount, textBuf, numer, denom);
}

static pascal void local_lineProc(Point newPt)
{
	gPictCache.preventCache = true;
	gPictCache.port->grafProcs = nil;
	
	CallQDLineProc(gPictCache.stdProcs.lineProc, newPt);
}

static pascal void local_rectProc(GrafVerb verb, Rect *r)
{
	gPictCache.preventCache = true;
	gPictCache.port->grafProcs = nil;
	
	CallQDRectProc(gPictCache.stdProcs.rectProc, verb, r);
}

static pascal void local_rRectProc(GrafVerb verb, Rect *r, short ovalWidth, short ovalHeight)
{
	gPictCache.preventCache = true;
	gPictCache.port->grafProcs = nil;
	
	CallQDRRectProc(gPictCache.stdProcs.rRectProc, verb, r, ovalWidth, ovalHeight);
}

static pascal void local_ovalProc(GrafVerb verb, Rect *r)
{
	gPictCache.preventCache = true;
	gPictCache.port->grafProcs = nil;
	
	CallQDOvalProc(gPictCache.stdProcs.ovalProc, verb, r);
}

static pascal void local_arcProc(GrafVerb verb, Rect *r, short startAngle, short arcAngle)
{
	gPictCache.preventCache = true;
	gPictCache.port->grafProcs = nil;
	
	CallQDArcProc(gPictCache.stdProcs.arcProc, verb, r, startAngle, arcAngle);
}

static pascal void local_polyProc(GrafVerb verb, PolyHandle poly)
{
	gPictCache.preventCache = true;
	gPictCache.port->grafProcs = nil;
	
	CallQDPolyProc(gPictCache.stdProcs.polyProc, verb, poly);
}

static pascal void local_rgnProc(GrafVerb verb, RgnHandle rgn)
{
	gPictCache.preventCache = true;
	gPictCache.port->grafProcs = nil;
	
	CallQDRgnProc(gPictCache.stdProcs.rgnProc, verb, rgn);
}

static pascal void local_commentProc(short kind, short dataSize, Handle dataHandle)
{
	// Comments don't matter, since the default commentProc ignores them.
//	gPictCache.preventCache = true;
//	gPictCache.port->grafProcs = nil;
	
	CallQDCommentProc(gPictCache.stdProcs.commentProc, kind, dataSize, dataHandle);
}

static pascal short local_txMeasProc(short byteCount, Ptr textAddr, Point *numer, Point *denom, FontInfo *info)
{
	gPictCache.preventCache = true;
	gPictCache.port->grafProcs = nil;
	
	return(CallQDTxMeasProc(gPictCache.stdProcs.txMeasProc, byteCount, textAddr, numer, denom, info));
}

static pascal void local_getPicProc(Ptr dataPtr, short byteCount)
{
	// Add up the actual size of the picture data
	gPictCache.realPictSize += byteCount;
	
	CallQDGetPicProc(gPictCache.stdProcs.getPicProc, dataPtr, byteCount);
}

static pascal void local_putPicProc(Ptr dataPtr, short byteCount)
{
	CallQDPutPicProc(gPictCache.stdProcs.putPicProc, dataPtr, byteCount);
}

static pascal void local_opcodeProc(Rect *fromRect, Rect *toRect, short opcode, short version)
{
	CallQDOpcodeProc(gPictCache.stdProcs.opcodeProc, fromRect, toRect, opcode, version);
}

static pascal void local_stdPixProc(PixMap *src, Rect *srcRect, MatrixRecord *matrix, short mode, RgnHandle mask, PixMap *matte, Rect *matteRect, short flags)
{
	if(gPictCache.current.inUse)
	{
		// We've already seen one stdBits opcode this picture.  We can't cache another.
		goto Callthrough;
	}
	
	// Make sure StdPix knows we want it to call StdBits
	CallStdPixProc(gPictCache.stdProcs.newProc1, src, srcRect, matrix, mode, mask, matte, matteRect, flags | callOldBits);

	// If anything failed, get out of the way.
	if(!gPictCache.current.inUse)
	{
		gPictCache.preventCache = true;
	}
	
	if(gPictCache.preventCache)
	{
		gPictCache.port->grafProcs = nil;
	}
	return;
	
Callthrough:
	gPictCache.preventCache = true;
	gPictCache.port->grafProcs = nil;
	
	CallStdPixProc(gPictCache.stdProcs.newProc1, src, srcRect, matrix, mode, mask, matte, matteRect, flags);
}

static void local_glyphsProc(void *dataStream, ByteCount size)
{
	gPictCache.preventCache = true;
	gPictCache.port->grafProcs = nil;
	
	CallQDStdGlyphsProc(gPictCache.stdProcs.glyphsProc, dataStream, size);
}
