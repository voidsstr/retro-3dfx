/********************************************************************************
	Utliities.h
		
	Common types, global defines, and inline functions for QuickDraw acceleraiton.
*/
#pragma once

// Includes
#include "hdwr_res_mgr.h"
#include "hrm_fifo.h"
#include "hrm_mem.h"
#include "hrm_prefs.h"
#include <cstddef>
#include "DCon.h"
#include "h3GDefs.h"
#include "minihwc.h"
#include "VoodooRegs.h"
#include "StdTextPatch.h"

#include "NQDAcceleration.h"

// rcf Should be in FifoClasses.h
#include "hwcio.h"

#pragma mark // Crap

#define SSTCP_PKT4_NOPCMD  ((0x48 << SSTCP_REGBASE_SHIFT))

#define CMDFIFO_BUILD_PK4( mask, regName, chip )\
    ( ((mask) << SSTCP_PKT4_MASK_SHIFT) | (chip << 11)\
    | (regName) | SSTCP_PKT4 )

#pragma mark // Constants

const UInt32	kQDScratchSize	= 0x00040000;	// 256K of scratch memory
const UInt32	H3FIFO_SIZE 	= 4096;

const UInt32 	kROPSource		= 0x000000CC << SSTG_ROP0_SHIFT;
const UInt32 	kROPDest		= 0x000000AA << SSTG_ROP0_SHIFT;
const UInt32 	kROPPattern 	= 0x000000F0 << SSTG_ROP0_SHIFT;

	// Masks relevant bits for source and dest addr registers from a 32 bit address 
const UInt32	kBaseAddrOffsetMask = 0x03FFFFFF;

#pragma mark // Types

#if UNIVERSAL_INTERFACES_VERSION < 0x332
enum
{
	useDistantHdwrMemBit		= 4,
	useLocalHdwrMemBit			= 5,

	useDistantHdwrMem			= 1L << useDistantHdwrMemBit,
	useLocalHdwrMem				= 1L << useLocalHdwrMemBit
};
#endif

typedef struct h3Info
{
	hrmBoardInfo_t	*bInfo;
	hrmFifoInfo		*fifo;
	hrmBoard_t		*board;

	// used only to keep track of the swap mode so that the FifoClasses can
	// do the right thing.
	UInt32 			depth;		

	// Quickdraw Scratch Space on this board
	mmBlock_t		*scratchSpace;
	
	// Array of Font Cache memory block pointers
	mmBlock_t		*fontCacheBlocks[FontCache::kNumFonts];
	
	UInt32			boardFlags;			// For now, 1 indicates a valid board
	
	// Resources on the card
	VoodooIORegs	*regsIO;
	VoodooFifoRegs	*regsFifo;
	Voodoo2DRegs	*regs2D;
	Voodoo3DRegs	*regs3D;
	
	bool			agpFIFO;
	
	UInt32			prefsIdx;
	
	bool			hasArbitration;
	bool			holdArbitration;
	bool			is66MHz;			// TRUE iff in 66MHz PCI slot
	
} h3Info;

#include "FIFOClasses.h"

#pragma mark // Global Data

extern h3Info					h3_info[16];

// Quickdraw transfer mode tables. Maps a QD transfer mode in to a ROP mode
extern const UInt32 gModeToRop8[16];
extern const UInt32 gModeToRop[16];
extern const UInt32 gModeToRop1[16];
extern const UInt32 gModeToRop1InvertXfer[16];
extern const SInt32	gModeToRop1SwapColors[16];

extern bool gCommandExNeedsRestore;

// Memory block used to hold stretch bits source data
extern mmBlock_t	*gStretchBitsBlock;

// If true, the picture caching code is trying a cache fill
extern bool		gPictCacheFillAttempt;

// Set to TRUE when a new NQD operation is entered; allows blit code to do
// once-per-operation setups, and then launch multiple blits.
extern bool gNewNQDOperation;

// Set to true if we are running on a machine with Altivec.
extern bool gAltivecAvailable;

// This is actually declared in InstallAcceleration.cp.
extern HRMPrefsTable			*gPreferences;

#pragma mark // Function prototypes

Int32	GetAcceleratedBitBlitProc (NQDDrawVars  *drawVars);
Int32	GetAcceleratedStretchBlitProc (NQDDrawVars  *drawVars, bool &isHostBlit, bool shrink);
Int32	GetAcceleratedDitherBlitProc(NQDDrawVars  *drawVars);
Int32	GetAcceleratedPatBlitProc (NQDDrawVars  *drawVars);
Int32	GetAcceleratedLineBlitProc (NQDDrawVars  *drawVars);
Int32	GetAcceleratedSlabBlitProc (NQDDrawVars  *drawVars);

OSErr	InitializeAccelerationHardware(void);
Int32	H3BlitFinish(long refCon);
Int32 	H3SlabBlitFinish(long refCon);
Int32 	H3LineBlitFinish(long refCon);
void 	ScratchSpaceNotifyProc(struct mmBlock_s *block, UInt32 code);
void 	StdBitsPatch(BitMap *srcBits, Rect *srcRect, Rect *dstRect,
				short mode, RgnHandle maskRgn);

extern "C" UInt32 	NQDProtectCursor(Rect *theRect, Point offsetPoint);
extern "C" void		NQDUnprotectCursor(UInt32  protectionID);

// For filling in the comment field in memory blocks.
void strcpy(char *dst, char *src);
void strcat(char *dst, char *src);

#pragma mark // Class Definitions
/*----------------------------------------------------------------------------*\
	==> UStateLocalBuffer <==
	
	This class creates a object  that creates a buffer that goes away when
	the stack deallocates the object, for good clean up.  It also aligns
	the the pointer to a long word for good access	
\*----------------------------------------------------------------------------*/
class UStateLocalBuffer
{
public:
		UStateLocalBuffer(UInt32 numLongs);
		~UStateLocalBuffer();

		UInt32 *Get();
private:

	UInt32 		mBasePtr;
};


#pragma mark -
#pragma mark // Inline Functions
#pragma mark -
#pragma mark // General purpose

/********************************************************************************
	NumLeadingZeroBytes
		UInt32 leftMask
		
	This routine returns the number of zero bytes of zero bits from leading
	a mask.

	This roytine is identical in function to the above, but has a different
	name to make reading slab blit code easier.
	
*/
inline UInt32 NumLeadingZeroBytes(register UInt32 mask)
{
	return (__cntlzw(mask) >> 3);
}

/********************************************************************************
	NumTrailingZeroBytes
		UInt32 leftMask
		
	This roytine is identical in function to the above, but has a different
	name to make reading slab blit code easier.
*/
inline UInt32 NumTrailingZeroBytes(UInt32 mask)
{
	return (__cntlzw(__lwbrx(&mask, 0)) >> 3);
}

/********************************************************************************
	SWAP
		C&x
		C&y
		
	
*/
template <class C> inline void SWAP(C&x, C&y)
{
	C temp;
	temp = x;
	x = y;
	y = temp;
}

/********************************************************************************
	MIN/MAX
		C x
		C y
		
	
*/
template <class C>
inline typename C MIN(C x, C y)
{
	return (x < y) ? (x) : (y);
}
template <class C>
inline typename C MAX(C x, C y)
{
	return (x > y) ? (x) : (y);
}

/**********************************************************************************
	FastSetRect
	
	Sets the coordinates of a rectangle to the given values.
*/
inline void FastSetRect(Rect &rect, SInt16 top, SInt16 left, SInt16 bottom, SInt16 right)
{
	rect.top = top;
	rect.left = left;
	rect.bottom = bottom;
	rect.right = right;
}

/********************************************************************************
	FastOffsetRect
		Rect &r
		SInt32 h
		SInt32 v
		
	Positive h and v move the rect down and to the right.
*/
inline void FastOffsetRect(Rect &r, SInt32 h, SInt32 v)
{
	r.left += h;
	r.right += h;
	r.top += v;
	r.bottom += v;
}

/********************************************************************************
	FastEqualRect
		Rect &r1
		Rect &r2
		
	Returns true iff the two rects are equal
*/
inline bool FastEqualRect(Rect &r1, Rect &r2)
{
	UInt32 *p1 = (UInt32*)&r1;
	UInt32 *p2 = (UInt32*)&r2;
	
	return((p1[0] == p2[0]) && (p1[1] == p2[1]));
}

/**********************************************************************************
	FastSectRect
		Rect *source
		Rect *source2
		Rect *dest
		
	Calculates the intersection of two rectangles. Sets dest to the intersection.
	Returns TRUE if the intersection is non-null. Otherwise, dest is undefined on exit.
*/
inline Boolean FastSectRect(const Rect &source, const Rect &source2, Rect &dest)
{
	SInt32 destTop = source.top;
	SInt32 destLeft = source.left;
	SInt32 destBottom = source.bottom;
	SInt32 destRight = source.right;
	
	if (source2.top > destTop)
		destTop = source2.top;
	if (source2.bottom < destBottom)
		destBottom = source2.bottom;
	if (source2.left > destLeft)
		destLeft = source2.left;
	if (source2.right < destRight)
		destRight = source2.right;
		
	// Why cast to UInt16? Because, it makes CW not add extraneous extsh instructions
	dest.top = (UInt16) destTop;
	dest.left = (UInt16) destLeft;
	dest.bottom = (UInt16) destBottom;
	dest.right = (UInt16) destRight;
	
	return (destTop < destBottom) && (destLeft < destRight);
}

/**********************************************************************************
	FastSectRect2
		Rect *source
		Rect *source2
		
	Returns TRUE if the intersection of two rectangles is non-null.
*/
inline Boolean FastSectRect2(const Rect &source, const Rect &source2)
{
	Rect dest;
	
	/* MBW -- This could probably be done more efficiently, but 
		the logic is making my head hurt right now.
	*/
	return(FastSectRect(source, source2, dest));
}


#pragma mark -
#pragma mark // Board functions

/********************************************************************************
	CheckScratchSpace
		h3Info &whichBoard
		
	Checks to see if the scratch space is allocated, attempts to allocate it if it isn't.
	Will return TRUE if you have scratch space to work with, or FALSE if you don't.
	
	You should call this function in an Accept proc before setting up a blit that requires
	the scratch space. You then have to use an alternate method or pass on the blit if you
	don't get space.
*/
inline bool	CheckScratchSpace(h3Info *whichBoard)
{
	// See if we have scratch space already allocated
	if (!whichBoard->scratchSpace)
	{
		// Try to allocate our scratch space
		whichBoard->scratchSpace = hrmAllocateBlock(whichBoard->board, kQDScratchSize, HRM_MEMF_LINEAR, HRM_MEM_PRI_IMPORTANT);
		if (!whichBoard->scratchSpace)
			return false;
			
		// It's important that we get the scratch space, but not so important that we keep it.
		whichBoard->scratchSpace->priority = HRM_MEM_PRI_SCRATCH;
		
		// Fix up the scratch space memory record
		whichBoard->scratchSpace->userData = whichBoard;
		whichBoard->scratchSpace->notifyProc = ScratchSpaceNotifyProc;
		
		// Fill in the comment field
		strcpy(whichBoard->scratchSpace->comment, "QuickDraw Scratch");
	}
	
	return true;
}

/********************************************************************************
	TouchCommandEx
		void
		
	
*/
inline void TouchCommandEx(void)
{
	gCommandExNeedsRestore = true;
}

/********************************************************************************
	FindH3Info
		void *destPtr
		
	
*/
inline h3Info *FindH3Info(void *destPtr)
{
	if(h3_info[((FxU32)destPtr) >> 28].bInfo)
		return &h3_info[((FxU32)destPtr) >> 28];
		
	return NULL;
}

/********************************************************************************
	LoadNQDPattern
		NQDDrawVars *drawVars
		h3Info &h3InfoDst
		
	Loads a 8x8 (or smaller) pattern from the NQD expanded pattern data into
	the Voodoo3's pattern registers.
*/
inline void LoadNQDPattern(NQDDrawVars *drawVars, h3Info &h3InfoDst)
{
	UInt32	patternBuffer[64];
	UInt32	*patBufPtr = patternBuffer;
	UInt32	hPos = 0;
	UInt32	vPos = 0;
	UInt32	dstPixelSize = drawVars->dstPixMap.pixelSize;

	FifoSetPattern	setpat(&h3InfoDst);
	setpat.setDepth(dstPixelSize);

	if ((drawVars->patHMask + 4) != dstPixelSize || (drawVars->patVMask + 4) !=
			drawVars->patRowBytes << 3)
	{
		for (UInt32 vertIndex = 0; vertIndex < 8; ++vertIndex)
		{
			for (UInt32 horizIndex = 0; horizIndex < dstPixelSize >> 2; ++horizIndex)
			{
				*patBufPtr++ = drawVars->patExData[hPos + vPos];
				hPos = (hPos + 4) & drawVars->patHMask;
			}
			vPos = (vPos + drawVars->patRowBytes) & drawVars->patVMask;
		}

		setpat.load(dstPixelSize << 1, patternBuffer);
	} else
	{
		setpat.load(dstPixelSize << 1, drawVars->patExData);
	}
}

#pragma mark -
#pragma mark // Accepting Blits

/********************************************************************************
	CapsLockDown
		
	Debugging function. Turns off all accel operations when caps lock is down.
	Useful for quickly comparing accelerated vs. non-accelerated blits, to ensure
	bug-compatibility with Quickdraw.
*/
inline bool CapsLockDown()
{
#if DEBUG_MODE
	return !!((*(UInt32 *) 0x178) & 0x02);
#else
	return false;
#endif
}

/********************************************************************************
	CommonBlitAccept
		NQDDrawVars &drawVars
		h3Info **h3InfoDst
		
	Checks common to all the NQD hooks are placed here, as well as housekeeping
	functions needed for every blit operation.
*/
inline bool CommonBlitAccept(NQDDrawVars &drawVars, h3Info **h3InfoDstPtr)
{
	// Get the destination board for the blit
	h3Info *h3InfoDst = FindH3Info(drawVars.dstPixMap.baseAddr);
	
	// Make sure we found a board we can accelerate
	if(!h3InfoDst || h3InfoDst->fifo->exclusiveMode)
		return false;
	
	// We only handle 8, 16, or 32bpp dests
	if (drawVars.dstPixMap.pixelSize < 8)
		return false;
	
	// For debugging, always bail if the caps lock keys is down
	if (CapsLockDown())
		return false;
	
	// We are starting a new operation
	gNewNQDOperation = true;
	*h3InfoDstPtr = h3InfoDst;

	return true;
}

/********************************************************************************
	PatchCommonBlitAccept
		
	
*/
inline bool PatchCommonBlitAccept()
{	
	// For debugging, always bail if the caps lock keys is down
	if (CapsLockDown())
		return false;
	
	// We are starting a new operation
	gNewNQDOperation = true;

	return true;
}

#pragma mark -
#pragma mark /// Mode To ROP helper funcs

/********************************************************************************
	ModeToROP
		UInt32 bitDepth
		
	
*/
inline UInt32 ModeToROP(UInt32 bitDepth, Int32 mode)
{
	if (bitDepth == 8)
		return gModeToRop8[mode];
	else
		return gModeToRop[mode];
}

/********************************************************************************
	ModeToROP1
		UInt32 bitDepth
		NQDDrawVars *drawVars
		UInt32 &colorFore
		UInt32 &colorBack
		bool &invertXfer
	
	This function does a fair bit more than ModeToROP().  To get all of the
	1-bit transfer modes right, it:
	
	- Returns a value to be bitwise ORed with reg_command.  This includes the
		ROP mode and possibly the SSTG_TRANSPARENT bit, which makes zero bits
		in the source transparent when set.
	- Sets up the correct values to be put into the reg_colorFore and reg_colorBack
		registers.  The caller should set colorFore to drawVars->foreColor
		and colorBack to drawVars->backColor before calling this function.
		They may come back changed.
	- Returns a bool (invertXfer) which, if true, means that the transfer proc
		needs to invert (bitwise negate) the pixel data before sending it to
		the chip.
	
*/
inline UInt32 ModeToROP1(	UInt32 mode,
							UInt32 &colorFore,
							UInt32 &colorBack,
							bool &invertXfer)
{
	if(gModeToRop1SwapColors[mode] == 0)
	{
		// The colors are already correct.
	}
	else if(gModeToRop1SwapColors[mode] == 1)
	{
		// The colors need to be swapped.
		SWAP(colorFore, colorBack);
	}
	else // if(gModeToRop1SwapColors[mode] == 2)
	{
		// ___Xor/not___Xor mode
		colorFore = 0xFFFFFFFF;
		colorBack = 0x00000000;
	}

	// Since the hardware can only make '0' bits transparent, 
	// the 'not___Or' and 'not___Bic' modes need an inverting transfer proc.
	// We don't use this for not___Copy, since we can get the same result by 
	// swapping the colors.
	invertXfer = (gModeToRop1InvertXfer[mode] != 0);
	
	return gModeToRop1[mode];
}

// This is the even simpler special case for when drawVars->patSolid is true.
inline UInt32 ModeToROPSolid(	UInt32 mode,
								NQDDrawVars *drawVars,
								UInt32 &color)
{
	// If this would be a source-invert mode, invert the pattern data.
	UInt32 patData = drawVars->patData[0] ^ gModeToRop1InvertXfer[mode];
	UInt32 rop = gModeToRop1[mode];
	
	// Minimize branching.
	UInt32 colors[3];	
	colors[0] = drawVars->foreColor;
	colors[1] = drawVars->backColor;
	colors[2] = 0xFFFFFFFF;
	color = colors[gModeToRop1SwapColors[mode]];
	
	if((rop & SSTG_TRANSPARENT) && (patData == 0))
	{
		// This is a non-blit.  Do nothing.
		return(0);
	}
	
	// Strip off the SSTG_TRANSPARENT bit and return the rop.
	return(rop & SSTG_ROP0);
}

#pragma mark -
/********************************************************************************
	Punt
		
	This routine is called when we choose NOT to do an acceleration.  This will
	help us figure out why we didn't do it.
	
	Right now we only do file, line and mode, but since we have all the draw
	vars, we can do more.
*/
#if DEBUG_MODE
	#if PUNT_LOGGING

		#define Punt(vars) {_Punt(vars, __FILE__, __LINE__); return false;}
		#include "AsyncLogger.h"
		#include <cstdio>
		using std::sprintf;

		inline void _Punt(NQDDrawVars *drawVars, char *file, long line)
		{
			// Log a bunch of information
			#if DCON_LOGGING
				// Put the info to the debug console
				dprintf("%s # %d, Mode: %d\n", file, line, drawVars->mode);
			#endif
			#if APP_LOGGING
				// Put the info to the normal printf incase we have an app running.
				printf("%s # %d, Mode: %d\n", file, line, drawVars->mode);
			#endif	
			#if MB_LOGGING
				// We have neither and app nor DCON so put the reason to macs bugs.
				char	string[256];
				
				// Okay, we don't want to drag in the standard libraries so let.s
				sprintf(string, "%s line %d mode %d ; g", file, line, drawVars->mode);
				c2pstr(string);
				DebugStr((StringPtr) string);
			#endif
			#if NQD_VARS_DUMP
				// We will spill the whole NQDDrawVars to a file so we can GeneralEdit it later.
				WriteToAsyncLog((Ptr)drawVars, sizeof(NQDDrawVars));
			#endif
		}

	#else
		#define Punt(vars)		{return false;}
	#endif	
#else
	#define Punt(vars)		{return false;}

#endif

#pragma mark -

void GetArbitration(h3Info *board);
void ReleaseArbitration(h3Info *board);
