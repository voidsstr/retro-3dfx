/********************************************************************************
	ArithBlit.cp
		
	Chall Fry
	Critical Path Software
	
*/

// Includes
#include <NQDAcceleration.h>
#include "RegionParser.h"
#include "Utilities.h"

#include "ArithBlitVector.h"

// Pragma Tricks
#pragma inline_max_size(1024)
#pragma inline_max_total_size(100000)

// Globals
static UInt32 gPinValue;

// Types

// The aritmetic functions are templatized over the source depth, the destination depth, and the QuickDraw
// transfer mode being used. This file is designed to be used with all combinations of 8, 16, and 32bpp
// soures and destinations.

#pragma mark
class Source8
{
public:
	static const UInt32	pixelSize = 8;
	static const UInt32	pixToBytesShift = 0;
	typedef UInt8	PixType;
	
	static UInt32	ConvertTo16(PixType *&inputPixPtr, UInt32 *sourceColorTable)
	{
		return sourceColorTable[*inputPixPtr];
	};

	static UInt32	ConvertTo32(PixType * const &inputPixPtr, UInt32 *sourceColorTable)
	{
		return sourceColorTable[*inputPixPtr];
	};
};

#pragma mark
class Source16
{
public:
	static const UInt32	pixelSize = 16;
	static const UInt32	pixToBytesShift = 1;
	typedef UInt16	PixType;
	

	static UInt32	ConvertTo16(PixType *&inputPixPtr, UInt32 *)
	{
		return *(UInt32 *) inputPixPtr;
	};

	static UInt32	ConvertTo32(const PixType * const &inputPixPtr, UInt32 *)
	{
		UInt32 inputPix = *inputPixPtr;
		UInt32 outputPix;

		outputPix = __rlwinm(           inputPix, 17- 8,  8, 12);
		outputPix = __rlwimi(outputPix, inputPix, 17-13, 13, 15);
		outputPix = __rlwimi(outputPix, inputPix, 22-16, 16, 20);
		outputPix = __rlwimi(outputPix, inputPix, 22-21, 21, 23);
		outputPix = __rlwimi(outputPix, inputPix, 27-24, 24, 28);
		outputPix = __rlwimi(outputPix, inputPix,    30, 29, 31);

		return outputPix;
	};
};

#pragma mark
class Source32
{
public:
	static const UInt32	pixelSize = 32;
	static const UInt32	pixToBytesShift = 2;
	typedef UInt32	PixType;
	
	static UInt32	ConvertTo16(PixType *&inputPixPtr, UInt32 *)
	{
		UInt32 outputPix;
	
		UInt32 inputPix = inputPixPtr[0];
		outputPix = __rlwinm(           inputPix, 8-1,		 1,	 5);
		outputPix = __rlwimi(outputPix, inputPix, 16-6,		 6,	10);
		outputPix = __rlwimi(outputPix, inputPix, 24-11,	11,	15);
		inputPix = inputPixPtr[1];
		outputPix = __rlwinm(           inputPix, 32+8-17,	17,	21);
		outputPix = __rlwimi(outputPix, inputPix, 32+16-22,	22,	26);
		outputPix = __rlwimi(outputPix, inputPix, 32+24-27,	27,	31);

		return outputPix;
	};
	
	static UInt32	ConvertTo32(PixType * const &inputPix, UInt32 *)
	{
		return *inputPix;
	};
};

#pragma mark
class Dest8
{
public:
	static const UInt32		pixelSize = 8;
	static const UInt32		pixToBytesShift = 0;
	typedef UInt32			ComputationType[4];

	static const UInt32	lowBitChannelMask = 0x01010100;
	static const UInt32	channelSize = 8;

	class	PixType
	{
	public:
		volatile SInt32 mData;
		
		operator SInt32()
		{
			return __lwbrx((void *) &mData, 0);
		}
		
		UInt32 operator= (const UInt32& value)
		{
			__stwbrx(value, (void *) &mData, 0);
			return value;
		}
	};

	static void PreparePinValue(NQDDrawVars *drawVars)
	{
		gPinValue = ((drawVars->rWt << 8) & 0xFF0000) | 
					(drawVars->gWt & 0xFF00) | 
					((drawVars->bWt >> 8) & 0xFF);
	}

	static UInt32 GetPixels(PixType *&pixPtr, ComputationType &pixels, UInt32 *destColorTable)
	{
		UInt32 eightBitPixels = *(UInt32 *) pixPtr;
		
		pixels[0] = destColorTable[eightBitPixels >> 24 & 0xFF];
		pixels[1] = destColorTable[eightBitPixels >> 16 & 0xFF];
		pixels[2] = destColorTable[eightBitPixels >>  8 & 0xFF];
		pixels[3] = destColorTable[eightBitPixels >>  0 & 0xFF];
	
		return eightBitPixels;
	}
	
	static UInt32 InverseLookup(UInt32 compPixel, ITabPtr inverseTable)
	{
		UInt32 index = ((compPixel >> 12) & 0x0F00) | ((compPixel >> 8) & 0x00F0) |
				((compPixel >> 4) & 0x000F);
		return inverseTable->iTTable[index];
	}

	template <class Source>
	static void GetSourcePixels(typename Source::PixType *&inputPixPtr, ComputationType &destPix,
			UInt32 *sourceColorTable)
	{
		destPix[0] = Source::ConvertTo32(inputPixPtr, sourceColorTable);
		destPix[1] = Source::ConvertTo32(inputPixPtr + 1, sourceColorTable);
		destPix[2] = Source::ConvertTo32(inputPixPtr + 2, sourceColorTable);
		destPix[3] = Source::ConvertTo32(inputPixPtr + 3, sourceColorTable);
		inputPixPtr += 4;
	}

	template <class Transfer> 
	static UInt32 PerformTransfer(ComputationType &srcComp, ComputationType &dstComp, 
			UInt32 &extraData, ITabPtr inverseTable)
	{
		return InverseLookup(Transfer::op(srcComp[0], dstComp[0], extraData), inverseTable) << 24 |
			   InverseLookup(Transfer::op(srcComp[1], dstComp[1], extraData), inverseTable) << 16 |
			   InverseLookup(Transfer::op(srcComp[2], dstComp[2], extraData), inverseTable) <<  8 |
			   InverseLookup(Transfer::op(srcComp[3], dstComp[3], extraData), inverseTable) <<  0;
	}
};

#pragma mark
class Dest16
{
public:
	static const UInt32	pixelSize = 16;
	static const UInt32	pixToBytesShift = 1;
	typedef UInt32	ComputationType;

	static const UInt32	lowBitChannelMask = 0x84208420;
	static const UInt32	channelSize = 5;

	class	PixType
	{
	public:
		volatile SInt32 mData;
		
		operator SInt32()
		{
			return __rlwinm(mData, 16, 0, 31);
		}

		UInt32 operator= (const UInt32& value)
		{
			mData = __rlwinm(value, 16, 0, 31);
			return value;
		}
	};
	
	static void PreparePinValue(NQDDrawVars *drawVars)
	{
		gPinValue = ((drawVars->rWt >> 1) & 0x7C00) | 
					((drawVars->gWt >> 6) & 0x03E0) | 
					((drawVars->bWt >> 11) & 0x001F);
		gPinValue |= gPinValue << 16;
	}

	static UInt32 GetPixels(PixType *&pixPtr, ComputationType &pixels, UInt32 *)
	{
		pixels = *(UInt32 *) pixPtr;
		return pixels;
	}

	template <class Source>
	static void GetSourcePixels(typename Source::PixType *&inputPixPtr, ComputationType &destPix,
			UInt32 *sourceColorTable)
	{
		destPix = Source::ConvertTo16(inputPixPtr, sourceColorTable);
		inputPixPtr += 2;
	}

	template <class Transfer> 
	static UInt32 PerformTransfer(ComputationType &srcComp, ComputationType &dstComp, 
			UInt32 &extraData, ITabPtr)
	{
		return Transfer::op(srcComp, dstComp, extraData);
	}

};

#pragma mark
class Dest32
{
public:
	static const UInt32	pixelSize = 32;
	static const UInt32	pixToBytesShift = 2;
	typedef UInt32	ComputationType;
	typedef UInt32	PixType;
	
	static const UInt32	lowBitChannelMask = 0x01010100;
	static const UInt32	channelSize = 8;

	static void PreparePinValue(NQDDrawVars *drawVars)
	{
		gPinValue = ((drawVars->rWt << 8) & 0xFF0000) | 
					(drawVars->gWt & 0xFF00) | 
					((drawVars->bWt >> 8) & 0xFF);
	}

	static UInt32 GetPixels(PixType *&pixPtr, ComputationType &pixels, UInt32 *)
	{
		pixels = *pixPtr;
		return pixels;
	}
	
	template <class Source>
	static void GetSourcePixels(typename Source::PixType *&inputPixPtr, ComputationType &destPix,
			UInt32 *sourceColorTable)
	{
		destPix = Source::ConvertTo32(inputPixPtr, sourceColorTable);
		++inputPixPtr;
	}

	template <class Transfer> 
	static UInt32 PerformTransfer(ComputationType &srcComp, ComputationType &dstComp, 
			UInt32 &extraData, ITabPtr)
	{
		return Transfer::op(srcComp, dstComp, extraData);
	}
	
};

#pragma mark -
#pragma mark ¥ Transfer Classes

// These classes act as function objects; each class performs a certain type of Quickdraw transfer
// type. These classes are templatized over the destination bit depth; the template argument is used to 
// determine the channel mask and channel size.

template<class Dest> class TransferAddOver
{
public:
	static UInt32 op(UInt32 srcPix, UInt32 dstPix, const UInt32)
	{
		UInt32 result = srcPix + dstPix;		
		return result - (((srcPix ^ dstPix) ^ result) & Dest::lowBitChannelMask);
	}
};

template<class Dest> class TransferSubOver
{
public:
	static UInt32 op(UInt32 srcPix, UInt32 dstPix, const UInt32)
	{
		UInt32 result = dstPix - srcPix;
		return result + (((srcPix ^ dstPix) ^ result) & Dest::lowBitChannelMask);
	}
};

template<class Dest> class TransferAddPin
{
public:
	static UInt32 op(UInt32 srcPix, UInt32 dstPix, const UInt32 pinValue)
	{
		UInt32 result = srcPix + dstPix;
		UInt32 carries = (((srcPix ^ dstPix) ^ result) & Dest::lowBitChannelMask);
		
		result -= carries;
		UInt32 difference = pinValue - result;
		carries |= ((result ^ pinValue) ^ difference) & Dest::lowBitChannelMask;
		carries -= carries >> Dest::channelSize;
		
		return (result & ~carries) | (pinValue & carries);
	}
};

template<class Dest> class TransferSubPin
{
public:
	static UInt32 op(UInt32 srcPix, UInt32 dstPix, const UInt32 pinValue)
	{
		UInt32 result = dstPix - srcPix;
		UInt32 carries = (((srcPix ^ dstPix) ^ result) & Dest::lowBitChannelMask);
		
		result += carries;
		UInt32 difference = result - pinValue;
		carries |= ((result ^ pinValue) ^ difference) & Dest::lowBitChannelMask;
		carries -= carries >> Dest::channelSize;
		
		return (result & ~carries) | (pinValue & carries);
	}
};

template<class Dest> class TransferAdMax
{
public:
	static UInt32 op(UInt32 srcPix, UInt32 dstPix, const UInt32)
	{
		UInt32 result = dstPix - srcPix;
		UInt32 maxMask = ((srcPix ^ dstPix) ^ result) & Dest::lowBitChannelMask;
		maxMask -= maxMask >> Dest::channelSize;
		return (srcPix & maxMask) | (dstPix & ~maxMask);
	}
};

template<class Dest> class TransferAdMin
{
public:
	static UInt32 op(UInt32 srcPix, UInt32 dstPix, const UInt32)
	{
		UInt32 result = dstPix - srcPix;
		UInt32 maxMask = ((srcPix ^ dstPix) ^ result) & Dest::lowBitChannelMask;
		maxMask -= maxMask >> Dest::channelSize;
		return (srcPix & ~maxMask) | (dstPix & maxMask);
	}
};

template<class Dest> class TransferBlend50
{
public:
	static UInt32 op(UInt32 srcPix, UInt32 dstPix, const UInt32)
	{
		UInt32 result = dstPix + srcPix;
		return (result - ((srcPix ^ dstPix) & Dest::lowBitChannelMask)) >> 1;
	}
};

typedef void (*ArithBlitLoopFn)(NQDDrawVars *drawVars, SInt32 numLines, UInt32 srcRowAddr, UInt32 destRowAddr,
		UInt32 destStartMask, UInt32 destEndMask, SInt32 numLongs, UInt32 *sourceColorTable,
		UInt32 *destColorTable);

ArithBlitLoopFn gArithBlitLoopFn;

// Function Declarations

void MakeColorTable(NQDPixMap &pixMap, UInt32 *table, UInt32 expansionDepth);
void ArithBlitSetup(NQDDrawVars *drawVars, Rect &dstRect);

template <class Source, class Dest, class Transfer>
	void ArithBlitLoop(NQDDrawVars *drawVars, SInt32 numLines, UInt32 srcRowAddr, UInt32 destRowAddr,
			UInt32 destStartMask, UInt32 destEndMask, SInt32 numLongs, UInt32 *sourceColorTable,
			UInt32 *destColorTable);


#pragma mark -
#pragma mark ¥ Loop Functions

/********************************************************************************
	MakeColorTable
		NQDPixMap &pixMap
		UInt32 *table
		UInt32 expansionDepth
		
	Generates a color table for turning 8 bit source indexes into 16 or 32 bit color
	values.
*/
inline void MakeColorTable(NQDPixMap &pixMap, UInt32 *table, UInt32 expansionDepth)
{
	ColorSpec *ctTablePtr = pixMap.pmTable[0][0].ctTable;
	
	if (expansionDepth == 16)
	{
		for (UInt32 index = 0; index < 256; ++index)
		{
			table[index] = (((UInt32) ctTablePtr[index].rgb.red >> 1) & 0x7C00) |
					(((UInt32) ctTablePtr[index].rgb.green >> 6) & 0x03E0) |
					(((UInt32) ctTablePtr[index].rgb.blue >> 11) & 0x001F);
		}
	} else
	{
		for (UInt32 index = 0; index < 256; ++index)
		{
			table[index] = (((UInt32) ctTablePtr[index].rgb.red << 8) & 0xFF0000) |
					(((UInt32) ctTablePtr[index].rgb.green) & 0xFF00) |
					(((UInt32) ctTablePtr[index].rgb.blue >> 8) & 0xFF);
		}
	}
}


/********************************************************************************
	ArithBlitSetup
		NQDDrawVars *drawVars
		Rect &dstRect
		
	Performs non-templatized setup for an arithmetic blit, and calls the template
	function with lots of parameters. Anything in the blit loop that doesn't need
	to be templatized should be put here to save code size.
*/
void ArithBlitSetup(NQDDrawVars *drawVars, Rect &dstRect)
{
	UInt32 destPixelSize = drawVars->dstPixMap.pixelSize;
	UInt32 destPixToByteShift = destPixelSize >> 4;
	UInt32 srcPixelSize = drawVars->srcPixMap.pixelSize;
	UInt32 srcPixToByteShift = srcPixelSize >> 4;

	UInt32 destRowAddr = ((UInt32) drawVars->dstPixMap.baseAddr & 0xF3FFFFFF) + 0x06000000 +
			(dstRect.top - drawVars->dstPixMap.bounds.top) * drawVars->dstPixMap.rowBytes +
			((dstRect.left - drawVars->dstPixMap.bounds.left) << destPixToByteShift);
	UInt32 srcRowAddr = (UInt32) drawVars->srcPixMap.baseAddr + drawVars->srcPixMap.rowBytes *
			(dstRect.top - drawVars->dstRect.top + drawVars->srcRect.top - drawVars->srcPixMap.bounds.top) +
			((dstRect.left - drawVars->dstRect.left + drawVars->srcRect.left - 
			drawVars->srcPixMap.bounds.left) << srcPixToByteShift);
		
	// Caclulate masks for the left and right edges of the dest. These mask out the partial longs
	// on the left and right sides of the dest rect.
	UInt32 destStartMask = 0xFFFFFFFFU >> ((destRowAddr & 3) << 3);
	UInt32 destEndMask = ~(0xFFFFFFFFU >> ((destRowAddr + ((dstRect.right - dstRect.left) << 
			destPixToByteShift) & 3) << 3));

	// numLongs is the # of longs per line, minus the left and right masked longs.
	SInt32 numLongs = (dstRect.right - dstRect.left) >> (2 - destPixToByteShift);
	if (!numLongs && (destStartMask & destEndMask))
	{
		destStartMask = destStartMask & destEndMask;
		destEndMask = 0;
	} else if ((destStartMask | destEndMask) == -1)
		--numLongs;
		
	// Set up the dest pointer so that it is long aligned. The first long transferred will be masked
	// by the above mask value. Subtract the same number of PIXELS from the source, so that source to 
	// dest alignment is maintained.
	UInt32 destStartBytes = destRowAddr & 3;
	destRowAddr -= destStartBytes;
	srcRowAddr -= (destStartBytes >> destPixToByteShift) << srcPixToByteShift;
	
	// rcf For 8 bit sources, need to set up the color table. For 8 bit dests, need to setup
	// color table and inverse table
	UInt32 sourceColorTable[256], destColorTable[256];
	if (srcPixelSize == 8)
		MakeColorTable(drawVars->srcPixMap, sourceColorTable, destPixelSize);
	if (destPixelSize == 8)
		MakeColorTable(drawVars->dstPixMap, destColorTable, srcPixelSize);
	
	gArithBlitLoopFn(drawVars, dstRect.bottom - dstRect.top, srcRowAddr, destRowAddr, 
			destStartMask, destEndMask, numLongs, sourceColorTable, destColorTable);
}

/********************************************************************************
	ArithBlitLoop
		NQDDrawVars *drawVars
		SInt32 numLines
		UInt32 srcRowAddr
		UInt32 destRowAddr
		UInt32 destStartMask
		UInt32 destEndMask
		SInt32 numLongs
		UInt32 *sourceColorTable
		UInt32 *destColorTable
		
	This is the big function. This function is templatized over 3 classes, which will
	produce 3x3x7=63 template expansions of this function. All the functions this function
	calls should get inlined into this function. This function loops through every long
	of the dest, and calls functions to read in the source and dest pixels, combine them
	according to the transfer mode, and write out the completed pixels.
*/
template <class Source, class Dest, class Transfer>
void ArithBlitLoop(NQDDrawVars *drawVars, SInt32 numLines, UInt32 srcRowAddr, UInt32 destRowAddr,
		UInt32 destStartMask, UInt32 destEndMask, SInt32 numLongs, UInt32 *sourceColorTable,
		UInt32 *destColorTable)
{
	ITabPtr inverseTable = drawVars->invTable;
	UInt32 extraData = gPinValue;

	// For each line...
	while (numLines-- > 0)
	{
		Dest::PixType			*destAddr = (Dest::PixType *) destRowAddr;
		Source::PixType			*srcAddr  = (Source::PixType *) srcRowAddr;
		Dest::ComputationType	srcComp, dstComp;
		UInt32					srcLong, dstLong;
		
		Dest::GetSourcePixels<Source>(srcAddr, srcComp, sourceColorTable);
		dstLong = Dest::GetPixels(destAddr, dstComp, destColorTable);
		srcLong = Dest::PerformTransfer<Transfer>(srcComp, dstComp, extraData, inverseTable);
		*destAddr = (dstLong & ~destStartMask) | (srcLong & destStartMask);
		++destAddr;
				
		// Handle the bulk of the line a cache line at a time
		for (SInt32 numCacheLines = numLongs >> 3; numCacheLines > 0; --numCacheLines)
		{			
			// This loop unrolls completely
			for (UInt32 index = 0; index < 8; ++index)
			{
				Dest::GetSourcePixels<Source>(srcAddr, srcComp, sourceColorTable);
				Dest::GetPixels(destAddr, dstComp, destColorTable);
				*destAddr++ = Dest::PerformTransfer<Transfer>(srcComp, dstComp, extraData, inverseTable);
			}
				
			__dcbf(destAddr, -36);
		}
		
		// Handle the rest of the line
		for (SInt32 numExtraLongs = numLongs & 7; numExtraLongs > 0; --numExtraLongs)
		{
			Dest::GetSourcePixels<Source>(srcAddr, srcComp, sourceColorTable);
			Dest::GetPixels(destAddr, dstComp, destColorTable);
			*destAddr++ = Dest::PerformTransfer<Transfer>(srcComp, dstComp, extraData, inverseTable);
		}
		
		// Handle the last partial long
		if (destEndMask)
		{
			Dest::GetSourcePixels<Source>(srcAddr, srcComp, sourceColorTable);
			dstLong = Dest::GetPixels(destAddr, dstComp, destColorTable);
			srcLong = Dest::PerformTransfer<Transfer>(srcComp, dstComp, extraData, inverseTable);
			*destAddr = (dstLong & ~destEndMask) | (srcLong & destEndMask);
		}
		
		__dcbf(destAddr, -64);
		__dcbf(destAddr, -32);
		__dcbf(destAddr, 0);
		destRowAddr += drawVars->dstPixMap.rowBytes;
		srcRowAddr += drawVars->srcPixMap.rowBytes;
	}
}

#pragma mark -
#pragma mark ¥ Picker
/********************************************************************************
	PickArithBlitVariantDepth
		UInt32 transferType
		
	The picker is templatized, to make the code easier. The PickArithBlitVariantLL
	function determine the proper source and dest bit depths, and then calls this
	function to pick the proper blit function based on the transfer type.
*/
template<class Source, class Dest>
inline RegionBlitProc PickArithBlitVariantDepth(NQDDrawVars *drawVars)
{
	ArithBlitLoopFn result = nil;

	switch (drawVars->mode)
	{
	case addOver: result = ArithBlitLoop<Source, Dest, TransferAddOver<Dest> >; break;
	case subOver: result = ArithBlitLoop<Source, Dest, TransferSubOver<Dest> >; break;
	case adMax:   result = ArithBlitLoop<Source, Dest, TransferAdMax<Dest> >; break;
	case adMin:   result = ArithBlitLoop<Source, Dest, TransferAdMin<Dest> >; break;

	case addPin:
		result = ArithBlitLoop<Source, Dest, TransferAddPin<Dest> >; 
		Dest::PreparePinValue(drawVars);
		break;

	case subPin:
		result = ArithBlitLoop<Source, Dest, TransferSubPin<Dest> >; 
		Dest::PreparePinValue(drawVars);
		break;

	case blend: 
		if (drawVars->rWt == 0x7FFF && drawVars->gWt == 0x7FFf && drawVars->bWt == 0x7FFF)
		{
			result =  ArithBlitLoop<Source, Dest, TransferBlend50<Dest> >;
		}
		break;
	}
	
	if (result)
	{
		gArithBlitLoopFn = result;
		return ArithBlitSetup;
	} else
		return nil;
}

/********************************************************************************
	PickArithBlitVariantLL
		NQDDrawVars *drawVars
		UInt32 srcDepth
		UInt32 dstDepth
		
	Blit function picker. Chooses the proper blit function to perform the requested blit.
*/
RegionBlitProc PickArithBlitVariantLL(NQDDrawVars *drawVars, UInt32 srcDepth, UInt32 dstDepth)
{
	if (srcDepth == 8)
	{
		if (dstDepth < 16)
		{
			return PickArithBlitVariantDepth<Source8, Dest8>(drawVars);
		} else if (dstDepth == 16)
		{
			return PickArithBlitVariantDepth<Source8, Dest16>(drawVars);
		} else
		{
			return PickArithBlitVariantDepth<Source8, Dest32>(drawVars);
		}
	} else if (srcDepth == 16)
	{
		if (dstDepth < 16)
		{
			return PickArithBlitVariantDepth<Source16, Dest8>(drawVars);
		} else if (dstDepth == 16)
		{
			return PickArithBlitVariantDepth<Source16, Dest16>(drawVars);
		} else
		{
			return PickArithBlitVariantDepth<Source16, Dest32>(drawVars);
		}
	} else if (srcDepth == 32)
	{
		if (dstDepth < 16)
		{
			return PickArithBlitVariantDepth<Source32, Dest8>(drawVars);
		} else if (dstDepth == 16)
		{
			return PickArithBlitVariantDepth<Source32, Dest16>(drawVars);
		} else
		{
			return PickArithBlitVariantDepth<Source32, Dest32>(drawVars);
		}
	}

	return nil;
}

#pragma mark-
#pragma mark ¥ Scratch Area
