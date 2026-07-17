/********************************************************************************
	FIFOClasses.h

	Monroe Williams
	11/20/99
*/
#pragma once


#include <GraphicsPrivHwc.h>
#include <hdwr_res_mgr.h>
#include <hrm_fifo.h>
#include <hrm_mode.h>
#include <hrm_arbitration.h>

#include "HRM_Priv.h"

// MBW -- Turn this on to enable support for the AGP FIFO.
// MBW -- XXX -- Enable this in sync with the check in HRM_FIFO.c:hrmInitFifo()
#define AGP_FIFO 0

/* MBW -- I believe this code now works.
	NOTE:  Certain parts of the code (particularly text drawing) may not function
		correctly if you turn this switch off.
*/
#define FIFO_CACHE_SPECIAL_CASE 1

/* MBW -- Experimental.  If this is true, none of the fifo writing
	operations will actually bump the fifo until it needs to wrap or
	our FinishProc gets called.  
*/
#define DEFERRED_BUMP 0

/*
	MBW -- Enable this to do paranoid consistency checking wherever possible
	NOTE:  Don't turn this on unless you're actively debugging a problem with
		the fifo management code.  It will slow things down noticeably.
*/
#define PARANOID_FIFO_CHECKS 0

inline void bumpFifoLow(h3Info *board);
inline void bumpFifoLow(h3Info *board)
{
	__dcbst(board->fifo->fifoPtr,-4);
	__sync();
	board->regsFifo->cmdFifo0.bump = board->fifo->fifoPtr - board->fifo->lastBump;
	__sync();
	board->fifo->lastBump = board->fifo->fifoPtr;
}

inline void bumpFifoDeferred(h3Info *board);
inline void bumpFifoDeferred(h3Info *board)
{
	if(board->fifo->lastBump != board->fifo->fifoPtr)
	bumpFifoLow(board);
}

/********************************************************************************
	_FifoMakeRoom
		hrmBoard_t *board
		hrmFifoInfo *fifo
		const FxU32 blockSize
		
	Ensures there is enough contiguous empty space in the Fifo to accept a full packet.
*/

// MBW -- Change this to #if 1 to not wait for the fifo to wrap.
// Note that this change also requires a different version of hrmFifoWrap() 
// in HRM_FIFO.c in the hrm.
void hrmFifoWrap2(h3Info *h3info);
#if 1
inline void _FifoMakeRoom(h3Info *h3info, FxU32 blockSize, bool align)
{
	hrmBoard_t 	*board = h3info->board;
	hrmFifoInfo *fifo = h3info->fifo;
	UInt32		rdPtr;
	SInt32 		room = fifo->fifoRoom;
	UInt32 		alignAdjust = 0;
	
	if (align)
	{
		alignAdjust = (UInt32)(fifo->fifoPtr);
		alignAdjust = (-alignAdjust & 31) >> 2;
	}

	UInt32 totalSizeToClear = blockSize + 96 + alignAdjust;

	if (room < (totalSizeToClear))
	{
		/* If desired write won't fit in fifo, then wrap it */
		if ((fifo->fifoPtr + totalSizeToClear) >= fifo->fifoEnd)
		{
			// If we are using the deferred bump model, bump the fifo now.
			bumpFifoDeferred(h3info);
//			hrmFifoWrap(board);
			hrmFifoWrap2(h3info);
			room = fifo->fifoRoom;
			
			// After the wrap, the fifo will already be cache-aligned.
			alignAdjust = 0;
		} 
		
		// Wait for the fifo to drain until there's enough room
		while (room < totalSizeToClear)
		{
			rdPtr = h3info->regsFifo->cmdFifo0.readPtrL;
			if (rdPtr <= ((UInt32) fifo->fifoPtr & 0x00FFFFFF))
				room = fifo->fifoEnd - fifo->fifoPtr;
			else
				room = (rdPtr - ((UInt32) fifo->fifoPtr & 0x00FFFFFF) >> 2);
			
		}
	}
	
	room -= blockSize + alignAdjust;
	fifo->fifoRoom = room;
	
	if (alignAdjust)
	{
		while (alignAdjust--)
		{
			*fifo->fifoPtr++ = 0;
		}
		
		__dcbst((void *)fifo->fifoPtr,-4);
		__dcbz((void *)fifo->fifoPtr,0);
	}
}
#else

inline void _FifoMakeRoom(h3Info *h3info, const FxU32 blockSize, bool align)
{
	hrmBoard_t *board = h3info->board;
	hrmFifoInfo *fifo = h3info->fifo;
	FxU32 *rdPtr;
  
	UInt32 alignAdjust = 0;
	
	if(align)
	{
		alignAdjust = (UInt32)(fifo->fifoPtr);
		alignAdjust = (-alignAdjust & 31) >> 2;
	}

	/* This code is based on the 2D driver's FIFO management code.  It's far simpler than 
	 * what Glide uses in the fullscreen case, but probably not as efficent CPU-wise. */  
	if(fifo->fifoRoom < blockSize + alignAdjust)
	{
		/* If desired write won't fit in fifo, then wrap it */
		if((fifo->fifoPtr + blockSize + alignAdjust) > fifo->fifoEnd)
		{
			// If we are using the deferred bump model, bump the fifo now.
			bumpFifoDeferred(h3info);
	//		rdPtr = hrmFifoWrap(board);
			rdPtr = hrmFifoWrap2(h3info);
			
			// After the wrap, the fifo will already be cache-aligned.
			alignAdjust = 0;
		} 

		fifo->fifoRoom = fifo->fifoEnd - fifo->fifoPtr;
		while(fifo->fifoRoom < blockSize + alignAdjust)
		{
			rdPtr = (FxU32 *)hrmHwFifoPtr(board);
			if(rdPtr <= fifo->fifoPtr)
				fifo->fifoRoom = fifo->fifoEnd - fifo->fifoPtr;
			else
				fifo->fifoRoom = 0; 
		}
	}

	fifo->fifoRoom -= blockSize + alignAdjust;
	
	if(alignAdjust)
	{
		// Fill the remainder of the cache line with 0's
	//	while(alignAdjust--)
	//	{
	//		*fifo->fifoPtr++ = 0;
	//	}
		fifo->fifoPtr += alignAdjust;
		
		// store the cache line and prep the next one.		
		__dcbst((void *)fifo->fifoPtr,-4);
		__dcbz((void *)fifo->fifoPtr,0);
	}
}
#endif

#pragma mark
class FifoBase
{
protected:
	h3Info *mBoard;
	UInt32 mDepth;
	
	// Use these to call template functions with the proper store.
	class store8
	{
	public:
		
		typedef UInt32 srcType;
		typedef UInt32 *srcPtr;
		typedef UInt32 *dstPtr;
		
		enum
		{
			dstSize = 4,
			pixelFormat = SSTG_PIXFMT_8BPP,
			apertureDepth = 8
		};
		
		static void
		storeRaw(UInt32 *&dst, UInt32 src)
		{
			__stwbrx(src,(void *)dst++, 0);
		}
		
		static void
		moveRaw(UInt32 *&dst, UInt32 *&src)
		{
			__stwbrx(*src++,(void *)dst++, 0);
		}
	};

	class store16
	{
	public:

		typedef UInt32 srcType;
		typedef UInt32 *srcPtr;
		typedef UInt32 *dstPtr;

		enum
		{
			dstSize = 4,
			pixelFormat = SSTG_PIXFMT_16BPP,
			apertureDepth = 16
		};

		static void
		storeRaw(UInt32 *&dst, UInt32 src)
		{
			UInt32 temp = src;
			temp = __rlwinm(temp, 16, 0, 31);
			*dst++ = temp;
		}
		static void
		moveRaw(UInt32 *&dst, UInt32 *&src)
		{
			UInt32 temp = *src++;
			temp = __rlwinm(temp, 16, 0, 31);
			*dst++ = temp;
		}
	};

	class store32
	{
	public:

		typedef UInt32 srcType;
		typedef UInt32 *srcPtr;
		typedef UInt32 *dstPtr;

		enum
		{
			dstSize = 4,
			pixelFormat = SSTG_PIXFMT_32BPP,
			apertureDepth = 32
		};

		static void
		storeRaw(UInt32 *&dst, UInt32 src)
		{
			*dst++ = src;
		}

		static void
		moveRaw(UInt32 *&dst, UInt32 *&src)
		{
			*dst++ = *src++;
		}
	};
public:
	
	FifoBase(h3Info *board, UInt32 /*depth */ = 0)
	{
		mBoard = board;
#ifdef VOODOO4
		mDepth = 32;
#else
		mDepth = mBoard->depth;
#endif

#if AGP_FIFO
		if(mBoard->agpFIFO)
			mDepth = 8;
#endif

	}
	
	UInt32 setDepth(UInt32 depth)
	{
		// MBW -- Commenting this out to make sure getting depth from h3info works right.
		// Sometime when everything is checked in, I'll deprecate this function and make it
		// go away.
//		mDepth = depth;
		
		/* This should do the following transform:
			 8 -> SSTG_PIXFMT_8BPP  == 0x00010000
			16 -> SSTG_PIXFMT_16BPP == 0x00030000
			32 -> SSTG_PIXFMT_32BPP == 0x00050000
			
			This is done by shifting the depth left by 13, masking with 0x00060000, and adding 0x00010000.
		*/ 
		
		return(__rlwinm(depth, 13, 13, 14) + 0x00010000);
	}

	UInt32 *makeRoom(UInt32 wordCount, bool align = false)
	{
		// MBW -- XXX -- To round FIFO writes up to cache line size, do something here.
		_FifoMakeRoom(mBoard, wordCount, align);

#if PARANOID_FIFO_CHECKS
		// Use this to verify in bumpFifo() that the caller gave the correct word count.
		mBoard->fifo->bumpPos = mBoard->fifo->fifoPtr + wordCount;
#endif

		return(mBoard->fifo->fifoPtr);
	}
	
protected:
	static void preFlush(UInt32 *ptr)
	{
		// MBW -- XXX -- Make this better.  Please.
		if(((FxU32)ptr & 31) == 0) 
		{
			// Can be flush or store
			__dcbst((void *)ptr,-4);
			__dcbz((void *)ptr,0);
		}
	}

	void finishPacket(UInt32 *newFifoPtr)
	{
		mBoard->fifo->fifoPtr = newFifoPtr;
	}

	void bumpFifo(void)
	{
		// MBW -- XXX -- To round FIFO writes up to cache line size, do something here.

#if PARANOID_FIFO_CHECKS
		// MBW -- XXX -- TAKE THIS OUT -- make sure the caller actually took
		//									exactly what they asked for
		if(mBoard->fifo->bumpPos != mBoard->fifo->fifoPtr)
		{
			DebugStr("\pSomeone lied to makeRoom()...");
		}
#endif
		
#if !DEFERRED_BUMP
		bumpFifoLow(mBoard);
#endif
	}
	
	template <class storer>
	static typename storer::dstPtr store(typename storer::dstPtr dst, typename storer::srcPtr src, UInt32 count)
	{
		UInt32 i = ((-(UInt32)dst) & 31) / storer::dstSize;
		
		for(; i < count;)
		{
			count -= i;
			for(; i > 0; i--)
			{
				storer::moveRaw(dst, src);
			}
			i = 32 / storer::dstSize;
			// Can be flush or store
			__dcbst(dst, -storer::dstSize);
			__dcbz(dst, 0);
		}
		
		for(; count > 0; count--)
			storer::moveRaw(dst, src);
		
		__dcbst(dst, -storer::dstSize);
		
		return(dst);
	}

	template <class storer>
	static typename storer::dstPtr fill(typename storer::dstPtr dst, typename storer::srcType val, UInt32 count)
	{
		UInt32 i = ((-(UInt32)dst) & 31) / storer::dstSize;
		
		for(; i < count;)
		{
			count -= i;
			for(; i > 0; i--)
			{
				storer::storeRaw(dst, val);
			}
			i = 32 / storer::dstSize;
			// Can be flush or store
			__dcbst(dst, -storer::dstSize);
			__dcbz(dst, 0);
		}
		
		for(; count > 0; count--)
			storer::storeRaw(dst, val);
		
		__dcbst(dst, -storer::dstSize);

		return(dst);
	}
	
public:
	void storeNonPixelData(UInt32 *dst, UInt32 val)
	{
		preFlush(dst);
		
		// This should be more optimal than a switch statement.
		if(mDepth > 16)
			store32::storeRaw(dst, val);
		else if(mDepth == 16)	
			store16::storeRaw(dst, val);
		else
			store8::storeRaw(dst, val);
	}
	
	void storeNonPixelData(UInt32 *dst, float val)
	{
		UInt32 temp = *(UInt32*)&val;
		storeNonPixelData(dst, temp);
	}

	void storeNonPixelData(UInt32 *dst, int val)
	{
		UInt32 temp = *(UInt32*)&val;
		storeNonPixelData(dst, temp);
	}
	
	UInt32 *storeNonPixelData(UInt32 *dst, UInt32 *src, UInt32 count)
	{
		UInt32 *result;

		// This should be more optimal than a switch statement.
		if(mDepth > 16)
			result = store<store32>(dst, src, count);
		else if(mDepth == 16)	
			result = store<store16>(dst, src, count);
		else
			result = store<store8>(dst, src, count);

		return(result);
	}
	
	UInt32 *fillNonPixelData(UInt32 *dst, UInt32 val, UInt32 count)
	{
		UInt32 *result;

		// This should be more optimal than a switch statement.
		if(mDepth > 16)
			result = fill<store32>(dst, val, count);
		else if(mDepth == 16)	
			result = fill<store16>(dst, val, count);
		else
			result = fill<store8>(dst, val, count);

		return(result);
	}
	
	void storePixelData(UInt32 *dst, UInt32 val)
	{
		preFlush(dst);
		store32::storeRaw(dst, val);
	}

	static UInt32 *storePixelData(UInt32 *dst, UInt32 *src, UInt32 count)
	{
		return(store<store32>(dst, src, count));
	}

	static UInt32 *fillPixelData(UInt32 *dst, UInt32 val, UInt32 count)
	{
		return(fill<store32>(dst, val, count));
	}

	hrmBoard_t *board(void)
	{
		return(mBoard->board);
	}
	hrmBoardInfo_t *bInfo(void)
	{
		return(mBoard->bInfo);
	}
	hrmFifoInfo *fifo(void)
	{
		return(mBoard->fifo);
	}
	
	/***********************************************************
		These are for code which wishes to roll its own special case, and handles
		flushing properly.
	***********************************************************/
	static void alignedPreFlush(UInt32 *ptr)
	{
#if PARANOID_FIFO_CHECKS
		if(((FxU32)ptr & 31) != 0) 
		{
			DebugStr("\palignedPreFlush() called when not aligned.");
		}
#endif
		// Can be flush or store
		__dcbst((void *)ptr,-4);
		__dcbz((void *)ptr,0);
	}
	
	void alignedStoreCmd(UInt32 *dst, UInt32 val, int depth = 0)
	{
#ifdef VOODOO4
		depth = 32;
#endif
#if AGP_FIFO
		if(mBoard->agpFIFO)
			depth = 8;
#endif

		if(depth != 0)
		{
			if(depth > 16)
				store32::storeRaw(dst, val);
			else if(depth == 16)	
				store16::storeRaw(dst, val);
			else
				store8::storeRaw(dst, val);
		}
		else
		{	
			if(mDepth > 16)
				store32::storeRaw(dst, val);
			else if(mDepth == 16)	
				store16::storeRaw(dst, val);
			else
				store8::storeRaw(dst, val);
		}
	}

	void alignedStorePixel(UInt32 *dst, UInt32 val, int /*depth*/ = 0)
	{
		store32::storeRaw(dst, val);
	}

	void finish(UInt32 *ptr)
	{
		// Commit this packet.
		finishPacket(ptr);
		
		// Update the FIFO
		bumpFifo();
	}
};


enum regName
{
	reg_clip0min = 0,
	reg_clip0max,
	reg_dstBaseAddr,
	reg_dstFormat,
	reg_srcColorkeyMin,
	reg_srcColorkeyMax,
	reg_dstColorkeyMin,
	reg_dstColorkeyMax,
	reg_bresError0,
	reg_bresError1,
	reg_rop,
	reg_srcBaseAddr,
	reg_commandEx,
	reg_lineStipple,
	reg_lineStyle,
	reg_pattern0alias,
	reg_pattern1alias,
	reg_clip1min,
	reg_clip1max,
	reg_srcFormat,
	reg_srcSize,
	reg_srcXY,
	reg_colorBack,
	reg_colorFore,
	reg_dstSize,
	reg_dstXY,
	reg_command,
	numRegs
};

#pragma mark
class Fifo2DRegs : public FifoBase
{
private:
	// We now keep the mask word in the first word of the buffer.
	// This makes for the same layout in memory and generates exactly the
	// same code, but it makes the packing algorithm more obviously correct.
	UInt32	mRegsBuffer[numRegs+1];


	template <class storer>
	void goSwap()
	{
		UInt32 regsCount = 0;
		UInt32 regsMask = mRegsBuffer[0];
		UInt32 *regsBuffer = mRegsBuffer+1;
		UInt32 tempBuffer[numRegs+1];
				
		// Pack the words into the top of the temp array
		UInt32 *packSrc, *packDst;
		packSrc = &regsBuffer[numRegs];
		packDst = &tempBuffer[numRegs];

		regsMask <<= (32 - numRegs);
		
		while(regsMask != 0)
		{
			UInt32 temp = __cntlzw(regsMask);
			packSrc -= temp;
			regsMask <<= temp + 1;
			*--packDst = *--packSrc;
			regsCount++;
		}
		
		// Put the packet header in the buffer
		*--packDst = (mRegsBuffer[0] << 3) | 2;
		
		// Write the packet to the fifo
		UInt32 *fifoPtr = makeRoom(regsCount + 1);
		fifoPtr = store<storer>(fifoPtr, packDst, regsCount + 1);
		
		// Commit this packet.
		finishPacket(fifoPtr);
	}	

#if 0
// Here's the original version of this function.
// It has been through several iterations, mRegsCount is no longer necessary.

	template <class storer>
	void goSwap(void)
	{
		// MBW -- I'm not sure about the best (i.e. most efficient) way to do this.
		//			We'll try this for now.
		UInt32 index = 0, mask = 1;
		UInt32 *fifoPtr;
		
		// Write the packet header
		fifoPtr = makeRoom(mRegsCount + 1);
		storer::store(fifoPtr++, (mRegsBuffer[0] << 3) | 2);
		
		// Write only those registers specified by the mask
		for(; index < 32; index++, mask <<= 1)
		{
			if(mask & mRegsBuffer[0])
			{
				storer::store(fifoPtr++, mRegsBuffer[index+1]);
			}
		}

		// Commit this packet.
		finishPacket(fifoPtr);
	}
#endif

public:	
	Fifo2DRegs(h3Info *board, UInt32 depth = 0)
		:FifoBase(board, depth)
	{
		reset();
	}
	
	void reset(void)
	{
		mRegsBuffer[0] = 0;
	}
	
	void queueNOP(void)
	{
		UInt32 *result = makeRoom(2);
		
		storeNonPixelData(result++, 
				CMDFIFO_BUILD_PK4(1, SSTCP_PKT4_NOPCMD, 0)
		);
		storeNonPixelData(result++, 0);
		
		finishPacket(result);
		
		bumpFifo();
	}
	
	void reg(regName reg, UInt32 value)
	{
		// Repeated calls to this function are not optimizing well.
		// mRegsMask gets loaded and stored for each call, and it
		// doesn't need to be.  Is there a storage class modifier that would
		// tell the compiler to be more aggressive about this?  (like the opposite
		// of "volatile".)

		
		mRegsBuffer[0] |= (1UL << reg);
		mRegsBuffer[reg+1] = value;
		
		if(reg == reg_commandEx)
		{
			// This variable is declared in Utilities.cp.
			extern bool gCommandExNeedsRestore;
			gCommandExNeedsRestore = true;
		}
	}
	
	/*	NOTE:  The caller should only pass a _compile-time constant_ as the 'depth' argument.
			If the depth isn't available as a compile-time constant, leave it
			blank.  This is an optimizer trick.

		NOTE: Most callers should use the "go()" function below instead of this function.
	*/ 
	void goNoReset(UInt32 depth = 0)
	{
#ifdef VOODOO4
	depth = 32;
#endif
#if AGP_FIFO
		if(mBoard->agpFIFO)
			depth = 8;
#endif

		if(mRegsBuffer[0] != 0)
		{
			// This should be more optimal than a switch statement.
			if(depth == 0)
			{
				if(mDepth > 16)
					goSwap<store32>();
				else if(mDepth == 16)	
					goSwap<store16>();
				else
					goSwap<store8>();
			}
			else
			{
				if(depth > 16)
					goSwap<store32>();
				else if(depth == 16)	
					goSwap<store16>();
				else
					goSwap<store8>();
			}

		}

		// If this is the last packet in a group, bump the fifo.
		bumpFifo();
	}

	/*	NOTE:  The caller should only pass a _compile-time constant_ as the 'depth' argument.
			If the depth isn't available as a compile-time constant, leave it
			blank.  This is an optimizer trick.
	*/ 
	void go(UInt32 depth = 0)
	{
		goNoReset(depth);
		
		// Reset our state in case the caller wants to do it again.
		reset();
	}

private:
	template <class storer>
	UInt32 *go1regSwap(UInt32 *fifoPtr, regName reg, UInt32 value)
	{
		preFlush(fifoPtr);
		storer::storeRaw(fifoPtr, (((1UL << reg)) << 3) | 2);
		preFlush(fifoPtr);
		storer::storeRaw(fifoPtr, value);
		
		return(fifoPtr);
	}
	
public:
	void go1reg(regName reg, UInt32 value, UInt32 depth = 0)
	{
#ifdef VOODOO4
	depth = 32;
#endif
#if AGP_FIFO
		if(mBoard->agpFIFO)
			depth = 8;
#endif

		UInt32 *fifoPtr = makeRoom(2);

		// This should be more optimal than a switch statement.
		if(depth == 0)
		{
			if(mDepth > 16)
				fifoPtr = go1regSwap<store32>(fifoPtr, reg, value);
			else if(mDepth == 16)	
				fifoPtr = go1regSwap<store16>(fifoPtr, reg, value);
			else
				fifoPtr = go1regSwap<store8>(fifoPtr, reg, value);
		}
		else
		{
			if(depth > 16)
				fifoPtr = go1regSwap<store32>(fifoPtr, reg, value);
			else if(depth == 16)	
				fifoPtr = go1regSwap<store16>(fifoPtr, reg, value);
			else
				fifoPtr = go1regSwap<store8>(fifoPtr, reg, value);
		}
		
		// Commit this packet
		finishPacket(fifoPtr);
		bumpFifo();		
	}
	
	void alignFifo(void)
	{
#if FIFO_CACHE_SPECIAL_CASE
		UInt32 *fifoPtr = makeRoom(0, true);
		
	#if PARANOID_FIFO_CHECKS
		if(((FxU32)fifoPtr & 31) != 0) 
		{
			DebugStr("\palignFifo() didn't manage to align the fifo.");
		}
	#endif
	
#endif
	}

	void endAlignFifo(void)
	{
#if FIFO_CACHE_SPECIAL_CASE	
		// Pad the remainder of this cache line with zeroes.	
		UInt32 *fifoPtr = mBoard->fifo->fifoPtr;
		UInt32 i = (UInt32)(fifoPtr);
		i = (-i & 31) >> 2;
		
		// If we didn't happen to end at the end of a cache line...
		if(i != 0)
		{
			// The dcbz's _should_ take care of this, but I'm not convinced they are.
			for(;i > 0; i--)
			{
				*fifoPtr++ = 0;
			}

			finishPacket(fifoPtr);
			bumpFifo();					
		}
#endif
	}

private:	
	template <class storer>
	inline UInt32 *specialCase3Regs_go(UInt32 *fifoPtr, UInt32 regsMask, UInt32 reg1, UInt32 reg2, UInt32 reg3)
	{
#if !FIFO_CACHE_SPECIAL_CASE
		preFlush(fifoPtr);
#endif
		storer::storeRaw(fifoPtr, (regsMask << 3) | 2);
#if !FIFO_CACHE_SPECIAL_CASE
		preFlush(fifoPtr);
#endif
		storer::storeRaw(fifoPtr, reg1);
#if !FIFO_CACHE_SPECIAL_CASE
		preFlush(fifoPtr);
#endif
		storer::storeRaw(fifoPtr, reg2);
#if !FIFO_CACHE_SPECIAL_CASE
		preFlush(fifoPtr);
#endif
		storer::storeRaw(fifoPtr, reg3);
		
		return(fifoPtr);
	}

	inline void specialCase3Regs_go(UInt32 regsMask, UInt32 reg1, UInt32 reg2, UInt32 reg3, UInt32 depth = 0)
	{
		UInt32 *fifoPtr;
#if FIFO_CACHE_SPECIAL_CASE
		fifoPtr = mBoard->fifo->fifoPtr;
		bool firstHalf = (((UInt32)fifoPtr & 31) == 0);
		
		if(firstHalf)
		{
			fifoPtr = makeRoom(8);
			preFlush(fifoPtr);
		}
#else
		fifoPtr = makeRoom(4);
#endif		
#ifdef VOODOO4
	depth = 32;
#endif
#if AGP_FIFO
		if(mBoard->agpFIFO)
			depth = 8;
#endif
		if(depth == 0)
		{
			if(mDepth > 16)
				fifoPtr = specialCase3Regs_go<store32>(fifoPtr, regsMask, reg1, reg2, reg3);
			else if(mDepth == 16)	
				fifoPtr = specialCase3Regs_go<store16>(fifoPtr, regsMask, reg1, reg2, reg3);
			else
				fifoPtr = specialCase3Regs_go<store8>(fifoPtr, regsMask, reg1, reg2, reg3);
		}
		else
		{
			if(depth > 16)
				fifoPtr = specialCase3Regs_go<store32>(fifoPtr, regsMask, reg1, reg2, reg3);
			else if(depth == 16)	
				fifoPtr = specialCase3Regs_go<store16>(fifoPtr, regsMask, reg1, reg2, reg3);
			else
				fifoPtr = specialCase3Regs_go<store8>(fifoPtr, regsMask, reg1, reg2, reg3);
		}
		
		finishPacket(fifoPtr);

#if FIFO_CACHE_SPECIAL_CASE
		if(!firstHalf)
		{
			// Commit this packet
			bumpFifo();		
		}
#else
		// Commit this packet
		bumpFifo();		
#endif
	}	

public:

	/* These are special cases for various blits which end up repeatedly setting
		just 3 registers, including the command register to initiate a blit.
		Due to the way the command fifo works, we are able to save a fair amount
		of bus bandwidth by only flushing a cache line after every other blit.
		(Each blit needs to write 4 longs to the fifo, and a cache line is 8
		longs.  The general-case code would need to __dcbst twice per cache line,
		whereas this special case only does it once per cache line.)
		
		NOTE:  The caller should only pass a _compile-time constant_ as the 'depth' argument.
		If the depth isn't available as a compile-time constant, leave the depth
		argument blank.  This is an optimizer trick.
		
		NOTE:  Before calling this function, you must call either alignFifo() or go(*, true), 
		which cache-aligns the fifo pointer.  For best results, you should then call 
		blah_blah_blah_cmd_go() many times in a row (as per SlabBlit).  After you're done, you 
		need to call endAlignFifo() before doing anything else with the fifo.  Note that 
		H3BlitFinish will take care of this for you, so if you aren't doing anything else 
		afterwards, it's safe to just fall out of your blit proc.
	*/
	inline void dstSize_dstXY_cmd_go(UInt32 dstSize, UInt32 dstXY, UInt32 cmd, UInt32 depth = 0)
	{
		specialCase3Regs_go(
			(1UL << reg_dstSize) | (1UL << reg_dstXY) | (1UL << reg_command),
			dstSize,
			dstXY,
			cmd,
			depth);
	}
	
	inline void clip1_cmd_go(UInt32 clip1min, UInt32 clip1max, UInt32 cmd, UInt32 depth = 0)
	{
		specialCase3Regs_go(
			(1UL << reg_clip1min) | (1UL << reg_clip1max) | (1UL << reg_command),
			clip1min,
			clip1max,
			cmd,
			depth);
	}	
	
	inline void srcXY_dstXY_cmd_go(UInt32 srcXY, UInt32 dstXY, UInt32 cmd, UInt32 depth = 0)
	{
		specialCase3Regs_go(
			(1UL << reg_srcXY) | (1UL << reg_dstXY) | (1UL << reg_command),
			srcXY,
			dstXY,
			cmd,
			depth);
	}	

	inline void dstSize_dstAddr_cmd_go(UInt32 dstSize, UInt32 dstAddr, UInt32 cmd, UInt32 depth = 0)
	{
		specialCase3Regs_go(
			(1UL << reg_dstSize) | (1UL << reg_dstBaseAddr) | (1UL << reg_command),
			dstAddr,
			dstSize,
			cmd,
			depth);
	}
};

// Code for dealing with the 3D registers.
#ifdef VOODOO4

// 3D register names enum.
enum reg3Name
{
	reg3_status = 0,
	reg3_intrCtrl,
	reg3_vertexAx,
	reg3_vertexAy,
	reg3_vertexBx,
	reg3_vertexBy,
	reg3_vertexCx,
	reg3_vertexCy,
	reg3_startR,
	reg3_startG,
	reg3_startB,
	reg3_startZ,
	reg3_startA,
	reg3_startS,
	reg3_startT,
	reg3_startW,
	reg3_dRdX,
	reg3_dGdX,
	reg3_dBdX,
	reg3_dZdX,
	reg3_dAdX,
	reg3_dSdX,
	reg3_dTdX,
	reg3_dWdX,
	reg3_dRdY,
	reg3_dGdY,
	reg3_dBdY,
	reg3_dZdY,
	reg3_dAdY,
	reg3_dSdY,
	reg3_dTdY,
	reg3_dWdY,
	reg3_triangleCMD,
	reg3_fvertexAx = 0x22,
	reg3_fvertexAy,
	reg3_fvertexBx,
	reg3_fvertexBy,
	reg3_fvertexCx,
	reg3_fvertexXy,
	reg3_fstartR,
	reg3_fstartG,
	reg3_fstartB,
	reg3_fstartZ,
	reg3_fstartA,
	reg3_fstartS,
	reg3_fstartT,
	reg3_fstartW,
	reg3_fdRdX,
	reg3_fdGdX,
	reg3_fdBdX,
	reg3_fdZdX,
	reg3_fdAdX,
	reg3_fdSdX,
	reg3_fdTdX,
	reg3_fdWdX,
	reg3_fdRdY,
	reg3_fdGdY,
	reg3_fdBdY,
	reg3_fdZdY,
	reg3_fdAdY,
	reg3_fdSdY,
	reg3_fdTdY,
	reg3_fdWdY,
	reg3_ftriangleCMD,
	reg3_fbzColorPath,
	reg3_fogMode,
	reg3_alphaMode,
	reg3_fbzMode,
	reg3_lfbMode,
	reg3_clipLeftRight,
	reg3_clipTopBottom,
	reg3_nopCMD,
	reg3_fastFillCMD,
	reg3_swapBufferCMD,
	reg3_fogColor,
	reg3_zaColor,
	reg3_chromaKey,
	reg3_chromaRange,
	reg3_userIntrCMD,
	reg3_stipple,
	reg3_color0,
	reg3_color1,
	reg3_fbiPixelsIn,
	reg3_fbiChromaFail,
	reg3_fbiZfuncFail,
	reg3_fbiAfuncFail,
	reg3_fbiPixelsOut,
	reg3_fogTable,
	reg3_renderMode = 0x78,
	reg3_stencilMode,
	reg3_stencilOp,
	reg3_colBufferAddr,
	reg3_colBufferStride,
	reg3_auxBufferAddr,
	reg3_auxBufferStride,
	reg3_fbiStencilTestFail,
	reg3_clipLeftRight1,
	reg3_clipTopBottom1,
	reg3_combineMode,
	reg3_sliCtrl,
	reg3_aaCtrl,
	reg3_chipMask,
	reg3_leftDesktopBuf,
	reg3_swapPending = 0x93,
	reg3_leftOverlayBuf,
	reg3_rightOverlayBuf,
	reg3_fbiSwapHistory,
	reg3_fbiTrianglesOut,
	reg3_sSetupMode,
	reg3_sVx,
	reg3_sVy,
	reg3_sARGB,
	reg3_sRed,
	reg3_sGreen,
	reg3_sBlue,
	reg3_sAlpha,
	reg3_sVz,
	reg3_sWb,
	reg3_sWtmu0,
	reg3_sS_W0,
	reg3_sT_W0,
	reg3_sWtmu1,
	reg3_sS_Wtmu1,
	reg3_sT_Wtmu1,
	reg3_sDrawTriCMD,
	reg3_sBeginTriCMD,
	reg3_textureMode = 0xC0,
	reg3_tLOD,
	reg3_tDetail,
	reg3_texBaseAddr,
	reg3_texBaseAddr_1,
	reg3_texBaseAddr_2,
	reg3_texBaseAddr_3_8,
	reg3_texStride,
	reg3_trexInit1,
	num3Regs
};

// Functions used to target registers on particular chips.
inline reg3Name reg3_FBI(reg3Name reg)
{
	return((reg3Name)(reg + 0x100));
}
inline reg3Name reg3_TREX(reg3Name reg)
{
	return((reg3Name)(reg + 0x600));
}
inline reg3Name reg3_TREX1(reg3Name reg)
{
	return((reg3Name)(reg + 0x400));
}

#pragma mark
class Fifo3DRegs : public FifoBase
{
private:
public:
	void idleFifo(void)
	{
		__sync();
		
		for(int i=0; i < 3; i++)
		{
			UInt32 depth = mBoard->regsFifo->cmdFifo0.depth;
			UInt32 status = mBoard->regs2D->status;
			if (depth != 0) 
			{
				i = 0;
			}
			if ((status & (SST_BUSY | SST_GUI_BUSY | SST_CMD0_BUSY)) != 0)
			{
				i = 0;
			}
			__eieio();
		}
	}
	
	Fifo3DRegs(h3Info *board, UInt32 depth = 0)
		:FifoBase(board, depth)
	{
	}	

	/* The 'reg()' method in this class uses the direct register model.
		Unlike FifoRegs::reg(), this method should not be used unless you
		are experimenting with new usages.  Once you have your code working,
		you should write a function like setupDest() or setupPipeline() that
		makes proper use of the FIFO.
	*/   
	void reg(reg3Name reg, UInt32 value)
	{
		__stwbrx(value, (void*)(mBoard->board->boardInfo.regInfo.sstBase), reg * 4);
	}
	
	void reg(reg3Name reg, float value)
	{
		UInt32 temp = *(UInt32*)&value;
		this->reg(reg, temp);
	}
	
	void reg(reg3Name reg, SInt32 value)
	{
		UInt32 temp = *(UInt32*)&value;
		this->reg(reg, temp);
	}
	
	void reg(reg3Name reg, int value)
	{
		UInt32 temp = *(UInt32*)&value;
		this->reg(reg, temp);
	}
	
	void go(void)
	{
		// This class does everything elsewhere.
	}
	
	void go(reg3Name reg, UInt32 value)
	{
		this->reg(reg, value);
		
		this->go();
	}
	
	inline UInt32 build_pk4(UInt32 mask, reg3Name regName)
	{
		// NOTE:  regName may also include the "chipMask" field at the
		//  proper relative offset.
		return(
    		(mask << SSTCP_PKT4_MASK_SHIFT) | 
			(regName << SSTCP_REGBASE_SHIFT) | 
			SSTCP_PKT4
		);
	}
	
	void setupDest(
			UInt32 renderMode, 
			UInt32 colBufferAddr, 
			UInt32 colBufferStride)
	{
		UInt32 *result = makeRoom(11 + 3);
		
		storeNonPixelData(result++, build_pk4(0x1b7b, reg3_renderMode) );
		storeNonPixelData(result++, renderMode);		// reg3_renderMode
		storeNonPixelData(result++, 0);					// reg3_stencilMode
		storeNonPixelData(result++, colBufferAddr);		// reg3_colBufferAddr
		storeNonPixelData(result++, colBufferStride);	// reg3_colBufferStride
		storeNonPixelData(result++, 0);					// reg3_auxBufferAddr
		storeNonPixelData(result++, 0);					// reg3_auxBufferStride
		storeNonPixelData(result++, 0);					// reg3_clipLeftRight1
		storeNonPixelData(result++, 0);					// reg3_clipTopBottom1
		storeNonPixelData(result++,	0);					// reg3_sliCtrl
		storeNonPixelData(result++,	0);					// reg3_aaCtrl
		
		// Set up the clipping registers
		storeNonPixelData(result++, build_pk4(0x03, reg3_clipLeftRight) );
		storeNonPixelData(result++,	(0 << 16) | 2048);	// reg3_clipLeftRight
		storeNonPixelData(result++,	(0 << 16) | 2048);	// reg3_clipTopBottom

		finishPacket(result);
		bumpFifo();
	}

	void setupSecondaryDest(UInt32 colBufferAddr)
	{
		UInt32 *result = makeRoom(2 + 2);

		// Set up secondary color buffer base address
		storeNonPixelData(result++, build_pk4(0x01, reg3_colBufferAddr) );
		storeNonPixelData(result++, colBufferAddr | SST_BUFFER_BASE_SELECT);		// reg3_colBufferAddr
		
		// Set up aaCtl to draw to both buffers with no X/Y offset
		storeNonPixelData(result++, build_pk4(0x01, reg3_aaCtrl) );
		storeNonPixelData(result++,	SST_AA_CONTROL_AA_ENABLE);			// reg3_aaCtrl

		finishPacket(result);
		bumpFifo();
	}

	void cleanupSecondaryDest(void)
	{
		UInt32 *result = makeRoom(2);

		// Set up aaCtl to draw to only the primary buffer.
		storeNonPixelData(result++, build_pk4(0x01, reg3_aaCtrl) );
		storeNonPixelData(result++,	0);			// reg3_aaCtrl

		finishPacket(result);
		bumpFifo();
	}

	// MBW -- XXX -- This method has had no testing.	Beware.
	void setupLFB(
			UInt32 lfbMode)
	{
		UInt32 *result = makeRoom(2);
		
		storeNonPixelData(result++, build_pk4(0x1, reg3_lfbMode) );
		storeNonPixelData(result++, lfbMode);			// reg3_lfbMode
		
		finishPacket(result);
		bumpFifo();
	}

	void setupPipeline(
			UInt32 fbzCombineMode,
			UInt32 trexCombineMode,
			UInt32 fbzColorPath,
			UInt32 fbzMode,
			UInt32 alphaMode = 0,
			UInt32 fogMode = 0,
			UInt32 color0 = 0,
			UInt32 color1 = 0)
	{
		UInt32 *result = makeRoom(13 + 2 + 5 + 3 + 2);
		
		// 12 3D NOP commands directed to the TREX chips, 
		// in case the previous client was using 2PPC rendering.
		storeNonPixelData(result++, 
				((0x600 + reg3_nopCMD) << SSTCP_REGBASE_SHIFT) |
				(12 << SSTCP_PKT1_NWORDS_SHIFT) |
				SSTCP_PKT1);

		for(int i=0; i < 12; i++)
		{
			storeNonPixelData(result++, 0x00);
		}

		storeNonPixelData(result++, build_pk4(0x01, reg3_FBI(reg3_combineMode)) );
		storeNonPixelData(result++, fbzCombineMode);		// reg3_combineMode
		
		storeNonPixelData(result++, build_pk4(0x0F, reg3_fbzColorPath));
		storeNonPixelData(result++, fbzColorPath);			// reg3_fbzColorPath
		storeNonPixelData(result++, fogMode);				// reg3_fogMode
		storeNonPixelData(result++, alphaMode);				// reg3_alphaMode
		storeNonPixelData(result++, fbzMode);				// reg3_fbzMode
		
		storeNonPixelData(result++, build_pk4(0x03, reg3_color0));
		storeNonPixelData(result++, color0);				// reg3_color0
		storeNonPixelData(result++, color1);				// reg3_color1

		storeNonPixelData(result++, build_pk4(0x1, reg3_TREX(reg3_combineMode)) );
		storeNonPixelData(result++, trexCombineMode);		// reg3_combineMode
		
		finishPacket(result);
		bumpFifo();
	}
	
	void setupTexture(
			UInt32 baseAddr,
			UInt32 lod,
			UInt32 textureMode)
	{
		UInt32 offset = 0;
		
		// We always use large textures, for ease of calculations.
		// The base address of the texture always needs to be calculated as if
		// it started at lod 0 (2k x 2k), even if that lod doesn't really exist.
		switch(lod)
		{
			case 0:
				offset += 2048 * 2048;
			/* FALLTHROUGH */

			case 1:
				offset += 1024 * 1024;
			/* FALLTHROUGH */
				
			case 2:
				offset += 512 * 512;
			/* FALLTHROUGH */
			
			case 3: // 256x256
				// do nothing
			break;
		}
		
		switch(textureMode & SST_TFORMAT)
		{
			case SST_ARGB8332:
			case SST_AYIQ8422:
			case SST_RGB565:
			case SST_ARGB1555:
			case SST_ARGB4444:
			case SST_AI88:
			case SST_AP88:
				// two-byte pixels
				offset *= 2;
			break;
			
			case SST_ARGB8888:
				// four-byte pixels
				offset *= 4;
			break;
			
			default:
				// one-byte pixels
			break;
		}
		
		baseAddr += offset;
		
		UInt32 *result = makeRoom(6);
		
		storeNonPixelData(result++, build_pk4(0xB, reg3_textureMode) );
		storeNonPixelData(result++, textureMode);							// reg3_textureMode
		storeNonPixelData(result++, (lod << 2) | (lod << 8) | SST_TBIG);	// reg3_tLOD
		storeNonPixelData(result++, SST_TEXTURE_MUNGE_ADDRESS(baseAddr));	// reg3_texBaseAddr
		
		// 3D NOP command
		storeNonPixelData(result++, 
				CMDFIFO_BUILD_PK4(1, SSTCP_PKT4_NOPCMD, 0)
		);
		storeNonPixelData(result++, 0);
		
		finishPacket(result);
		bumpFifo();
	}

	void setupOtherTexture(
			UInt32 baseAddr,
			UInt32 lod,
			UInt32 textureMode,
			UInt32 trex1CombineMode)
	{
		UInt32 offset = 0;
		
		// We always use large textures, for ease of calculations.
		// The base address of the texture always needs to be calculated as if
		// it started at lod 0 (2k x 2k), even if that lod doesn't really exist.
		switch(lod)
		{
			case 0:
				offset += 2048 * 2048;
			/* FALLTHROUGH */

			case 1:
				offset += 1024 * 1024;
			/* FALLTHROUGH */
				
			case 2:
				offset += 512 * 512;
			/* FALLTHROUGH */
			
			case 3: // 256x256
				// do nothing
			break;
		}
		
		switch(textureMode & SST_TFORMAT)
		{
			case SST_ARGB8332:
			case SST_AYIQ8422:
			case SST_RGB565:
			case SST_ARGB1555:
			case SST_ARGB4444:
			case SST_AI88:
			case SST_AP88:
				// two-byte pixels
				offset *= 2;
			break;
			
			case SST_ARGB8888:
				// four-byte pixels
				offset *= 4;
			break;
			
			default:
				// one-byte pixels
			break;
		}
		
		baseAddr += offset;
		
		UInt32 *result = makeRoom(8);
		
		storeNonPixelData(result++, build_pk4(0xB, reg3_TREX1(reg3_textureMode)) );
		storeNonPixelData(result++, textureMode);							// reg3_textureMode
		storeNonPixelData(result++, (lod << 2) | (lod << 8) | SST_TBIG);	// reg3_tLOD
		storeNonPixelData(result++, SST_TEXTURE_MUNGE_ADDRESS(baseAddr));	// reg3_texBaseAddr
		
		storeNonPixelData(result++, build_pk4(0x1, reg3_TREX1(reg3_combineMode)) );
		storeNonPixelData(result++, trex1CombineMode);		// reg3_combineMode

		// 3D NOP command
		storeNonPixelData(result++, 
				CMDFIFO_BUILD_PK4(1, SSTCP_PKT4_NOPCMD, 0)
		);
		storeNonPixelData(result++, 0);

		finishPacket(result);
		bumpFifo();
	}
	
	// MBW -- XXX -- This method was written for an experiment, and I'm not sure it works correctly.
	void setupTextureTiled(
			UInt32 baseAddr,
			UInt32 stride,
			UInt32 lod,
			UInt32 textureMode)
	{
		
		UInt32 *result = makeRoom(4);
		
		storeNonPixelData(result++, build_pk4(0xB, reg3_textureMode) );
		storeNonPixelData(result++, textureMode);							// reg3_textureMode
		storeNonPixelData(result++, (lod << 2) | (lod << 8) | SST_TBIG);	// reg3_tLOD
		storeNonPixelData(result++, 										// reg3_texBaseAddr
				SST_TEXTURE_MUNGE_ADDRESS(baseAddr) |
				(stride << SST_TEXTURE_TILESTRIDE_SHIFT) |
				SST_TEXTURE_IS_TILED);
		
		finishPacket(result);
		bumpFifo();
	}

	// This method does a rectangular blit using a tristrip.
	void drawRect(
			UInt32 lod,
			Rect &srcRect,
			Rect &dstRect,
			Rect &clipRect)
	{
		// Texture coordinates are always scaled so that texture coordinates
		// are in the range [0, 256).  Adjust accordingly.
		float srcMult = 1.0 / (8 >> lod) ;

		float xStretch = (srcRect.right - srcRect.left);
		xStretch /= (dstRect.right - dstRect.left);
		xStretch *= srcMult;
		
		float yStretch = (srcRect.bottom - srcRect.top);
		yStretch /= (dstRect.bottom - dstRect.top);
		yStretch *= srcMult;
		
		float srcXOffset = srcRect.left - (dstRect.left * xStretch);
		float srcYOffset = srcRect.top - (dstRect.top * yStretch);
		
		UInt32 *result = makeRoom(1 + (4 * 4) + 2);

		// Build a type 3 (triangle) packet.
		storeNonPixelData(result++, 
			SSTCP_PKT3 |
			SSTCP_PKT3_BDDDDD |
			(0x00000004 << SSTCP_PKT3_NUMVERTEX_SHIFT) |
			(0x00000020 << SSTCP_PKT3_PMASK_SHIFT) |
			(0x00000000 << SSTCP_PKT3_SMODE_SHIFT)
		);
		
		// Top right
		storeNonPixelData(result++, (float)clipRect.right);						// reg3_sVx
		storeNonPixelData(result++, (float)clipRect.top);						// reg3_sVy
		storeNonPixelData(result++, srcXOffset + (clipRect.right * xStretch));	// reg3_sS_W0
		storeNonPixelData(result++, srcYOffset + (clipRect.top * yStretch));	// reg3_sT_W0
		
		// Top left
		storeNonPixelData(result++, (float)clipRect.left);						// reg3_sVx
		storeNonPixelData(result++, (float)clipRect.top);						// reg3_sVy
		storeNonPixelData(result++, srcXOffset + (clipRect.left * xStretch));	// reg3_sS_W0
		storeNonPixelData(result++, srcYOffset + (clipRect.top * yStretch));	// reg3_sT_W0
		
		// Bottom right
		storeNonPixelData(result++, (float)clipRect.right);						// reg3_sVx
		storeNonPixelData(result++, (float)clipRect.bottom);					// reg3_sVy
		storeNonPixelData(result++, srcXOffset + (clipRect.right * xStretch));	// reg3_sS_W0
		storeNonPixelData(result++, srcYOffset + (clipRect.bottom * yStretch));	// reg3_sT_W0

		// Bottom left
		storeNonPixelData(result++, (float)clipRect.left);						// reg3_sVx
		storeNonPixelData(result++, (float)clipRect.bottom);					// reg3_sVy
		storeNonPixelData(result++, srcXOffset + (clipRect.left * xStretch));	// reg3_sS_W0
		storeNonPixelData(result++, srcYOffset + (clipRect.bottom * yStretch));	// reg3_sT_W0

		// 3D NOP command
		storeNonPixelData(result++, 
				CMDFIFO_BUILD_PK4(1, SSTCP_PKT4_NOPCMD, 0)
		);
		storeNonPixelData(result++, 0);

		finishPacket(result);
		bumpFifo();
	}
	
};
#endif // VOODOO4

/*********************************************************************
	class FifoSetPattern
	
	This class is a special case of the "type 1" 2D register write.
	It is used to set up the colorPattern registers.  This class does
	not bump the fifo when it's done, since it assumes that you will
	do some blits with the pattern you just loaded, which will take
	care of that.
	
	Users of this class should do ONE of the following:
	- call load() with the address of the pattern and let the class 
		write the pattern to the fifo.
	- call fill() with a fill value and let the class write the pattern
		to the fifo.
 *********************************************************************/
#pragma mark
class FifoSetPattern : public FifoBase
{
private:
	UInt32 *start(UInt32 wordCount)
	{
		UInt32 *result = makeRoom(wordCount +1);
		
		storeNonPixelData(result++, 
				SSTCP_PKT1 | 
				SSTCP_PKT1_2D | 
				SSTCP_INC |
				(0x40UL << SSTCP_REGBASE_SHIFT) |
				((wordCount) << SSTCP_PKT1_NWORDS_SHIFT)
		);
		
		return(result);
	}
	
public:
	FifoSetPattern(h3Info *board, UInt32 depth = 0)
		:FifoBase(board, depth)
	{
	}
	
	void load(UInt32 wordCount, UInt32 *src)
	{
		UInt32 *dst = start(wordCount);
		
		dst = storePixelData(dst, src, wordCount);

		// Commit this packet.
		finishPacket(dst);
		bumpFifo();
	}
	
	void fill(UInt32 wordCount, UInt32 fillValue)
	{
		UInt32 *dst = start(wordCount);

		dst = fillPixelData(dst, fillValue, wordCount);

		// Commit this packet.
		finishPacket(dst);
		bumpFifo();
	}
};

/*********************************************************************
	class FifoBlitData1
	
	This class is used for writing blit data using type 1 packets.
	The type 1 packet built by this class specifies that successive
	words of the blit data are written to the first word of the
	launch area.  
	
 *********************************************************************/
#pragma mark
class FifoBlitData1 : public FifoBase
{
public:
	FifoBlitData1(h3Info *board, UInt32 depth = 0)
		:FifoBase(board, depth)
	{
	}
	
	UInt32 *start(UInt32 wordCount)
	{
		UInt32 *result = makeRoom(wordCount + 1);

		storeNonPixelData(result++, 
				SSTCP_PKT1 | 
				SSTCP_PKT1_2D | 
				// NOTE: don't increment.  This is intentional.
				(0x20UL << SSTCP_REGBASE_SHIFT) |
		 		(wordCount << SSTCP_PKT1_NWORDS_SHIFT)
		);
		
		return(result);
	}
	
private:
};

/*********************************************************************
	class FifoBlitData5
	
	This class is used for writing data using type 5 packets.
	
	NOTE: The fifo we use is currently about 262144 (256k) bytes.
	For performance reasons, it's probably best not to write packets
	bigger than 128k (so that the card can be reading one while you're
	writing the other).  Attempting to write a packet bigger than
	the fifo will cause BadThings to happen (probably a hang in MakeRoom()).  
	You have been warned.
	
 *********************************************************************/
#pragma mark
class FifoBlitData5 : public FifoBase
{
public:
	FifoBlitData5(h3Info *board, UInt32 depth = 0)
		:FifoBase(board, depth)
	{
	}
	
	UInt32 *start(	UInt32 destination, 
					UInt32 wordCount, 
					UInt32 firstBytes = 0, 
					UInt32 lastBytes = 0)
	{
		UInt32 *result = makeRoom(wordCount + 2);

		storeNonPixelData(result++, 
				SSTCP_PKT5 | 
				SSTCP_PKT5_LFB |
				((firstBytes << SSTCP_PKT5_BYTEN_W2_SHIFT) & SSTCP_PKT5_BYTEN_W2) |
				((lastBytes << SSTCP_PKT5_BYTEN_WN_SHIFT) & SSTCP_PKT5_BYTEN_WN) |
		 		((wordCount << SSTCP_PKT5_NWORDS_SHIFT) & SSTCP_PKT5_NWORDS));
		 		
		storeNonPixelData(result++, destination & SSTCP_PKT5_BASEADDR);
		
		return(result);
	}
private:
};


