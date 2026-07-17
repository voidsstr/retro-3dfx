/********************************************************************************
	 DrawPictPatch.h
	 
	 
	 Monroe Williams
	 Critical Path Software
	 
*/

#include "Utilities.h"

#pragma once

enum
{
	PictCacheSize = 128
};

struct PictCacheEntry
{
	bool		inUse;
	h3Info		*board;
	
	// Information about the cached pixmap
	mmBlock_t	*cacheBlock;
	UInt32		cacheDepth;
	UInt32		cacheRowBytes;
	Rect		cacheRect;
	
	// Information needed to draw the cached pixmap with doScreenBlit<>()
	short		mode;
	Int32		colorizeFlag;
	UInt32		scale1bit[2];
	UInt32		foreColor;
	UInt32		backColor;
	RGBColor	rgbScale0;
	RGBColor	rgbScale1;
	RGBColor	rgbFore;
	RGBColor	rgbBack;
	Rect		srcRect;
	Rect		mappedDstRect;
	
	// Information needed to deal with banded pictures.
	short		unmappedMode;
	Rect		dstRect;
	bool		banded;
	UInt32		bandOffset;
	
	// Information about the screen at the time this entry was filled.
	UInt32		screenDepth;
	UInt32 		ctSeed;
	
	// Information about the picture that was cached
	UInt32		picSize;
	Rect		picFrame;
	UInt32		checksumLength;
	UInt32		checksum;
	bool		type2pict;
	UInt32		endOpcodeOffset;
	
	bool		hShrink;
	bool		vShrink;
	UInt32		hShrinkSize;
	UInt32		vShrinkSize;
};

struct PictCacheGlobals
{
	CGrafPtr		port;
	GDHandle		device;
	h3Info			*board;
	PixMap			*dstPixMap;
	PixMap			fakeDstPixMap;
	Rect			dstRect;
	NQDPixMap		dstPix;

	CQDProcs		localProcs;
	CQDProcs		stdProcs;
	bool			allowCache;
	bool			preventCache;
	UInt32			realPictSize;
	UInt32			shrunkenMismatch;
	UInt32			firstFreeEntry;
	
	
	// This is the entry that we are currently considering caching
	PictCacheEntry	current;
	
	// This is the actual cache
	PictCacheEntry	entries[PictCacheSize];
};

#pragma mark ¥ Globals

extern PictCacheGlobals	gPictCache;

#pragma mark ¥ Prototypes

void DrawPictPatchInit(void);
void DrawPictPatch(PicHandle myPicture, Rect *dstRect);
void DrawPictPatchTerminate(void);


