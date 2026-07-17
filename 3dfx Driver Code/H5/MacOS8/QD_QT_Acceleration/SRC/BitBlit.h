#include <NQDAcceleration.h>
#include "RegionParser.h"

#pragma mark ¥ Prototypes

RegionBlitProc PickScreenBlitVariantLL(UInt32 srcDepth, UInt32 dstDepth, NQDDrawVars *drawVars);
RegionBlitProc PickHostBlitVariantLL(UInt32 srcDepth, UInt32 dstDepth, NQDDrawVars *drawVars);

/* This function should be instantiated with one of the 'variant' classes below,
	or a class defined by the caller that has appropriately named members.
	
	Note that the bounds rects for the source and dest pixmaps are not given.  This function
	assumes that srcRect and dstRect are given in coordinate spaces that have the base
	address of the source and dest pixmaps pointing to the pixel at (0, 0).  The
	caller of this function can simulate this by adjusting the source and dest
	rects before passing them in.  See AcceleratedHostToScreenBlitLL() for an example.
	
	Other assumptions:
		- dstRect is pre-clipped to the destination bounds
		- the destination is on the board
		- the source is NOT on the board
		- you've picked the correct variant for the src and dst depth
		- transfer mode and coloration are cases the code can handle (see 
			GetAcceleratedBitBlitProc() for current limitations)
*/
template <class variant>  
void doHostBlit(
	h3Info *board,
	UInt32 srcBase,
	SInt32 srcRowBytes,
	Rect &srcRect,
	UInt32 dstBase,
	SInt32 dstRowBytes,
	Rect &dstRect,
	UInt32 mode,
	UInt32 *scaleTable,
	UInt32 foreColor,
	UInt32 backColor,
	Int32 colorizeFlag = 0,
	bool customROP = false);


/* This function should be instantiated with one of the 'variant' classes below,
	or a class defined by the caller that has appropriately named members.
	
	Note that the bounds rects for the source and dest pixmaps are not given.  This function
	assumes that srcRect, dstRect, and dstClipped are given in coordinate spaces that have the 
	base address of the source and dest pixmaps pointing to the pixel at (0, 0).  The
	caller of this function can simulate this by adjusting the source and dest
	rects before passing them in.  See AcceleratedScreenToScreenBlitLL() for an example.
	
	This function _does_ handle stretched blits.  It will optionally align the fifo so that
	the clip1_cmd_go() special case can be used for further calls from the region parser.
	See AcceleratedScreenToScreenBlitLL() for an example of this, too.
	
	Other assumptions:
		- dstRect is pre-clipped to the destination bounds
		- source and destination are both on the board
		- you've picked the correct variant for the src and dst depth
		- transfer mode and coloration are cases the code can handle (see 
			GetAcceleratedBitBlitProc() for current limitations)
*/
template <class variant>  void 
doScreenBlit(
	h3Info *board,
	UInt32 srcBase,
	SInt32 srcRowBytes,
	Rect &srcRect,
	UInt32 dstBase,
	SInt32 dstRowBytes,
	Rect &dstRect,
	Rect &dstClipped,
	bool backwardsX,
	UInt32 mode,
	UInt32 *scaleTable,
	UInt32 foreColor,
	UInt32 backColor,
	bool alignFifo,
	Int32 colorizeFlag = 0);


template <class xfer>
inline void cacheLineLoop(typename xfer::dstPtr &dst, typename xfer::srcPtr &src, UInt32 *refcon)
{
	// This loop will be constant counted, so with a repeat count of 8 or less, Codewarrior
	// will unroll this loop completely and replace it with n copies of the loop's core code.

	const UInt32 dstStride = sizeof(dst[0]) * xfer::dstStep;
	for (int i = 32 / dstStride; i > 0; --i)
	{
		xfer::move(dst, src, refcon);
		dst += xfer::dstStep;
		src += xfer::srcStep;
	}
}

/* This function should be instantiated with the 'xfer' or 'xferInvert' member class
	from one of the 'variant' classes below, or a class defined by the caller which
	has appropriately named members.
	
	refcon is drawVars->scaleTable for cases that need it.
*/
template <class xfer>
void pushDataToFifo(
		h3Info *board, 
		xfer::srcPtr src, 
		UInt32 srcRowBytes,
		UInt32 hwLongCount,
		UInt32 lineCount,
		UInt32 *refcon);


#pragma mark -
#pragma mark READ ME
/******
	If you've just started looking at this file for the first time, you are no doubt wondering
	just what the hell this mess is.  I'll explain.  Hear me out.
	
	On a previous project of this type that I worked on, the transfer loops were the
	ugliest part of the code.  In particular, most of the functions were almost identical
	in overall form (including cache management foo), and this made maintenance... interesting.
	
	This code is my attempt to factor three major, interleaved parts of the code into orthogonal 
	pieces, while at the same time making the generated code as optimal as possible.  The three
	pieces are:
	
	- the low-level blit loop (bit swizzling, knowledge of pointer bumps)
	- cache management (handling cache line transfers and alignment issues)
	- blit setup (decoding NQDDrawVars, various calculations, running the blitter)
	
	The basic approach I've taken here is to use template functions.  This code
	also relies heavily on inline functions and the optimizer (particularly loop unrolling)
	to make the code runtime-efficient.  I've spent much time analyzing the output of the
	disassembler and tweaking the source to make the optimizer do what it does here.
	
	Each of the "variant*" classes is intended to embody all of the knowledge of how to do
	a particular blit.  
	
	The ones in the "partial variants" section are incomplete, and intended to be used as base 
	classes for the others.  Naming maps to functionality as follows:
	
	- variantSrcXX:			the source (srcPixMap) has depth XX
	- variantHWInputXX:		the blitter is fed with data of depth XX
	- variantDstXX:			the destination (dstPixMap) has depth of XX
	- variantBitBlt:		some common elements shared by all blits that don't do depth conversion
	
	Each "variant*" class contains (or inherits) a class called "xfer", which contains all
	of the information the blit loop (pushDataToFifo<variant>()) needs to do the transfer.
	This information was originally just part of each class, instead of being encapsulated
	in an 'xfer' class, and this resulted in several identical copies of some of the blit
	loops being generated.  (The copies had different names but identical code.)  The 'xfer'
	classes give the compiler enough information to avoid doing this.
	
	Objects of these classes should never be instantiated -- their only purpose in life is 
	to be used as template arguments for the template functions in this file.  If you were
	to instantiate one, it would not contain any data (the only members are typedefs, enums,
	and static functions).
		
******/


#pragma mark -
#pragma mark ¥ partial variants

#pragma mark
struct variantSrc4
{
	enum
	{
		srcPixelSize = 4
	};	
};

#pragma mark
struct variantSrc8
{
	enum
	{
		srcPixelSize = 8
	};	
};

#pragma mark
struct variantSrc16
{
	enum
	{
		srcPixelSize = 16
	};
};

#pragma mark
struct variantSrc32
{
	enum
	{
		srcPixelSize = 32
	};
};

#pragma mark
struct variantHWInputBase
{
	// calculate the starting pixel offset for the hardware
	static UInt32 
	hwLeftCount(	UInt32 /*srcX*/,
					UInt32 srcDataStart) 
	{
		return(srcDataStart & 3);
	};
	
	/* adjust the src start and end pointers and return the "long count" value
		that will be passed to the hardware.
	 */
	static UInt32 
	hwLongCount(	UInt32 &srcDataStart,
					UInt32 srcDataEnd) 
	{
		// Back up the source data pointer to a long boundary
		srcDataStart &= ~3;
		
		// Push the src end forward to a long boundary
		srcDataEnd = (srcDataEnd + 3) & ~3;
		
		// return value is the number of longs between the pointers.
		return((srcDataEnd - srcDataStart) >> 2);
	};

	struct xferInvert
	{
		typedef UInt32 *srcPtr;
		typedef UInt32 *dstPtr;

		enum
		{
			srcStep = 1,
			dstStep = 1
		};
		
		static inline void
		move(dstPtr dst, srcPtr src, UInt32*)
		{
			dst[0] = ~(src[0]);
		}

		static UInt32
		srcToFifoBytes(UInt32 srcBytes)
		{
			return(srcBytes);
		}
	};
};

#pragma mark
struct variantHWInput1
	: public variantHWInputBase
{
	enum
	{
		srcPixFmt = SSTG_PIXFMT_1BPP,
		srcPixelSize = 1
	};
	
	// calculate the starting pixel offset for the hardware
	static UInt32 
	hwLeftCount(	UInt32 srcX,
					UInt32 srcDataStart)
	{
		return((srcX & 7) | ((srcDataStart & 3) << 3));
	};
};

#pragma mark
struct variantHWInput8
	: public variantHWInputBase 
{
	enum
	{
		srcPixFmt = SSTG_PIXFMT_8BPP
#ifdef VOODOO4
					| SSTG_HOST_BYTE_SWIZZLE
#endif
	};	
};

#pragma mark
struct variantHWInput16
	: public variantHWInputBase 
{
	enum
	{
		srcPixFmt = SSTG_PIXFMT_16BPP
#ifdef VOODOO4
					| SSTG_HOST_WORD_SWIZZLE
#endif
	};	
};


#pragma mark
struct variantHWInput32
	: public variantHWInputBase
{
	enum
	{
		srcPixFmt = SSTG_PIXFMT_32BPP
	};	

	// MBW -- Sometimes the pixels in a 32-bit source aren't long aligned.
	// 		This causes several problems if we don't force things here.
	
	// calculate the starting pixel offset for the hardware
	static UInt32 
	hwLeftCount(	UInt32 /*srcX*/,
					UInt32 /*srcDataStart*/) 
	{
		return(0);
	};
	
	/* adjust the src start and end pointers and return the "long count" value
		that will be passed to the hardware.
	 */
	static UInt32 
	hwLongCount(	UInt32 &srcDataStart,
					UInt32 srcDataEnd) 
	{		
		// return value is the number of longs between the pointers.
		return((srcDataEnd - srcDataStart) >> 2);
	};
};


#pragma mark
struct variantDst1xfer
{
	enum
	{
		dstPixFmt = SSTG_PIXFMT_8BPP,
		srcPixelSize = 1,
		dstPixelSize = 1,
		dstPixelMask = 0x00000001
	};
	
	static UInt32 modeToRop(UInt32 mode)
	{
		return(::ModeToROP(8, mode));
	}
};

#pragma mark
struct variantDst8
{
	enum
	{
		dstPixFmt = SSTG_PIXFMT_8BPP,
		dstPixelSize = 8,
		apertureDepth = 8,
		// Used for comparing ForeColor/BackColor for transparency
		dstPixelMask = 0x000000FF
	};
	
	static UInt32 modeToRop(UInt32 mode)
	{
		return(::ModeToROP(dstPixelSize, mode));
	}
};

#pragma mark
struct variantDst16
{
	enum
	{
		dstPixFmt = SSTG_PIXFMT_16BPP,
		dstPixelSize = 16,
		apertureDepth = 16,
		// Used for comparing ForeColor/BackColor for transparency
		dstPixelMask = 0x00007FFF
	};
	
	static UInt32 modeToRop(UInt32 mode)
	{
		return(::ModeToROP(dstPixelSize, mode));
	}
};

#pragma mark
struct variantDst32
{
	enum
	{
		dstPixFmt = SSTG_PIXFMT_32BPP,
		dstPixelSize = 32,
		apertureDepth = 32,
		// Used for comparing ForeColor/BackColor for transparency
		dstPixelMask = 0x00FFFFFF
	};
	
	static UInt32 modeToRop(UInt32 mode)
	{
		return(::ModeToROP(dstPixelSize, mode));
	}
};

#pragma mark
struct variantBitBlt
{
	
	struct xfer
	{
		typedef UInt32 *srcPtr;
		typedef UInt32 *dstPtr;

		enum
		{
			srcStep = 1,
			dstStep = 1
		};
		
		static inline void
		move(dstPtr dst, srcPtr src, UInt32*)
		{
			dst[0] = src[0];
		}

		static UInt32
		srcToFifoBytes(UInt32 srcBytes)
		{
			return(srcBytes);
		}
	};
};


#pragma mark -
#pragma mark ¥ full variants

#pragma mark
struct variantBitBlt8 
	: public variantBitBlt, public variantSrc8 , public variantHWInput8, public variantDst8 {};

#pragma mark
struct variantBitBlt8Scaled 
	: public variantBitBlt8 
{
	// This is an 8->8 blit that needs to be remapped as per the scale table.
	struct xfer
	{
		typedef UInt32 *srcPtr;
		typedef UInt32 *dstPtr;
		
		enum
		{
			srcStep = 1,
			dstStep = 1
		};
		
		static inline void
		move(dstPtr dst, srcPtr src, UInt32 *refcon)
		{			
			// The __rlwinm both extracts one pixel from the source word and compensates for the
			// table entries being 32 bits.  We add 3 to the table to get the lsb of each entry.
			UInt8 *table = ((UInt8*)refcon) + 3;
			
			UInt32 temp = src[0];
			UInt32 t0 = table[__rlwinm(temp, 10, 22, 29)];
			UInt32 t1 = table[__rlwinm(temp, 18, 22, 29)];
			UInt32 t2 = table[__rlwinm(temp, 26, 22, 29)];
			UInt32 t3 = table[__rlwinm(temp,  2, 22, 29)];
			
			t1 = __rlwimi(t1, t0, 8, 16, 23);
			t3 = __rlwimi(t3, t2, 8, 16, 23);
			
			dst[0] = __rlwimi(t3, t1, 16, 0, 15);
		}

		static UInt32
		srcToFifoBytes(UInt32 srcBytes)
		{
			return(srcBytes);
		}
	};
};

#pragma mark
struct variantBitBlt16 
	: public variantBitBlt, public variantSrc16, public variantHWInput16, public variantDst16 {};

#pragma mark
struct variantBitBlt32 
	: public variantBitBlt, public variantSrc32, public variantHWInput32, public variantDst32 {};

#pragma mark
struct variant1to8 
	: public variantBitBlt, public variantHWInput1, public variantDst8 
{
	enum
	{
		srcPixFmt = SSTG_PIXFMT_1BPP
#ifdef VOODOO4
					| SSTG_HOST_BYTE_SWIZZLE
#endif
	};
};

#pragma mark
struct variant1to16 
	: public variantBitBlt, public variantHWInput1, public variantDst16
{
	enum
	{
		srcPixFmt = SSTG_PIXFMT_1BPP 
#ifdef VOODOO4
					| SSTG_HOST_BYTE_SWIZZLE 
#else
					| SSTG_HOST_BYTE_SWIZZLE | SSTG_HOST_WORD_SWIZZLE
#endif
	};
};


#pragma mark
struct variant1to32 
	: public variantBitBlt, public variantHWInput1, public variantDst32
{
	enum
	{
		srcPixFmt = SSTG_PIXFMT_1BPP | SSTG_HOST_BYTE_SWIZZLE
	};
};

#pragma mark
struct variant1Xfer8 
	: public variantBitBlt, public variantHWInputBase, public variantDst1xfer 
{
	enum
	{
		apertureDepth = 8,
		srcPixFmt = SSTG_PIXFMT_8BPP
#ifdef VOODOO4
					| SSTG_HOST_BYTE_SWIZZLE
#endif
	};
};

#pragma mark
struct variant1Xfer16 
	: public variantBitBlt, public variantHWInputBase, public variantDst1xfer
{
	enum
	{
		apertureDepth = 16,
		srcPixFmt = SSTG_PIXFMT_8BPP 
#ifdef VOODOO4
					| SSTG_HOST_BYTE_SWIZZLE 
#else
					| SSTG_HOST_BYTE_SWIZZLE | SSTG_HOST_WORD_SWIZZLE
#endif
	};
};


#pragma mark
struct variant1Xfer32 
	: public variantBitBlt, public variantHWInputBase, public variantDst1xfer
{
	enum
	{
		apertureDepth = 32,
		srcPixFmt = SSTG_PIXFMT_8BPP | SSTG_HOST_BYTE_SWIZZLE
	};
};

#pragma mark
struct variant4to8
	: public variantSrc4, public variantHWInput8, public variantDst8
{
	struct xfer
	{
		typedef UInt16 *srcPtr;
		typedef UInt32 *dstPtr;
		
		enum
		{
			srcStep = 1,
			dstStep = 1
		};
		
		static inline void
		move(dstPtr dst, srcPtr src, UInt32 *refcon)
		{
			// The __rlwinm both extracts one pixel from the source word and compensates for the
			// table entries being 32 bits.  We add 3 to the table to get the lsb of each entry.
			UInt8 *table = ((UInt8*)refcon) + 3;

			UInt32 temp = src[0];			
			UInt32 t0 = table[__rlwinm(temp, 22, 26, 29)];
			UInt32 t1 = table[__rlwinm(temp, 26, 26, 29)];
			UInt32 t2 = table[__rlwinm(temp, 30, 26, 29)];
			UInt32 t3 = table[__rlwinm(temp,  2, 26, 29)];
			
			t1 = __rlwimi(t1, t0, 8, 16, 23);
			t3 = __rlwimi(t3, t2, 8, 16, 23);
			
			dst[0] = __rlwimi(t3, t1, 16, 0, 15);
		}
		
		static UInt32
		srcToFifoBytes(UInt32 srcBytes)
		{
			return(srcBytes << 1);
		}
	};
	
	// calculate the starting pixel offset for the hardware
	static UInt32 
	hwLeftCount(	UInt32 /*srcX*/,
					UInt32 &/*srcDataStart*/) 
	{
		// We'll always make the alignment come out right.
		return(0);
	};
	
	/* adjust the src start and end pointers and return the "long count" value
		that will be passed to the hardware.
	 */
	static UInt32 
	hwLongCount(	UInt32 &srcDataStart,
					UInt32 srcDataEnd) 
	{
		// Leave the source pointer alone.
				
		// Leave the src end alone, too.

		// return value is twice the number of bytes between the pointers, converted to words.
		return((((srcDataEnd - srcDataStart) << 1) + 3) >> 2);
	};
};

template <> inline void 
#pragma mark
cacheLineLoop<variant4to8::xfer>
(
	variant4to8::xfer::dstPtr &dst,	// UInt32*
	variant4to8::xfer::srcPtr &src,	// UInt16*
	UInt32 *refcon
)
{
	// The __rlwinm both extracts one pixel from the source word and compensates for the
	// table entries being 32 bits.  We add 3 to the table to get the lsb of each entry.
	UInt8 *table = ((UInt8*)refcon) + 3;
	
	for(int i = 4; i > 0; i--)
	{
		UInt32 temp = ((UInt32*)src)[0];			
		
		dst[0] = __rlwimi(	
			__rlwimi(	
				table[__rlwinm(temp, 18, 26, 29)], 
				table[__rlwinm(temp, 14, 26, 29)], 
				8, 16, 23), 
			__rlwimi(	
				table[__rlwinm(temp, 10, 26, 29)], 
				table[__rlwinm(temp, 6, 26, 29)], 
				8, 16, 23), 
			16, 0, 15);
		
		dst[1] = __rlwimi(	
			__rlwimi(	
				table[__rlwinm(temp, 2, 26, 29)], 
				table[__rlwinm(temp, 30, 26, 29)], 
				8, 16, 23), 
			__rlwimi(	
				table[__rlwinm(temp, 26, 26, 29)], 
				table[__rlwinm(temp, 22, 26, 29)], 
				8, 16, 23), 
			16, 0, 15);

		src += variant4to8::xfer::srcStep * 2;
		dst += variant4to8::xfer::dstStep * 2;
	}
}

#pragma mark
struct variant4to16
	: public variantSrc4, public variantHWInput16, public variantDst16
{
	struct xfer
	{
		typedef UInt8 *srcPtr;
		typedef UInt32 *dstPtr;
		
		enum
		{
			srcStep = 1,
			dstStep = 1
		};
		
		static inline void
		move(dstPtr dst, srcPtr src, UInt32 *refcon)
		{
			// The __rlwinm both extracts one pixel from the source word and compensates for the
			// table entries being 32 bits.  We add 1 to the table to get the lsh of each entry.
			UInt16 *table = ((UInt16*)refcon) + 1;

			UInt32 temp = src[0];
			UInt32 src1 = table[__rlwinm(temp, 29, 27, 30)];
			UInt32 src2 = table[__rlwinm(temp,  1, 27, 30)];

			dst[0] = __rlwimi(src2, src1, 16, 0, 15);

		}
		
		static UInt32
		srcToFifoBytes(UInt32 srcBytes)
		{
			return(srcBytes << 2);
		}
	};
	
	// calculate the starting pixel offset for the hardware
	static UInt32 
	hwLeftCount(	UInt32 /*srcX*/,
					UInt32 &/*srcDataStart*/) 
	{
		// We'll always make the alignment come out right.
		return(0);
	};
	
	/* adjust the src start and end pointers and return the "long count" value
		that will be passed to the hardware.
	 */
	static UInt32 
	hwLongCount(	UInt32 &srcDataStart,
					UInt32 srcDataEnd) 
	{
		// Leave the source pointer alone.
				
		// Leave the src end alone, too.

		// return value is 4x the number of bytes between the pointers, converted to words.
		return((((srcDataEnd - srcDataStart) << 2) + 3) >> 2);
	};
};

template <> inline void 
#pragma mark
cacheLineLoop<variant4to16::xfer>
(
	variant4to16::xfer::dstPtr &dst,	// UInt32*
	variant4to16::xfer::srcPtr &src,	// UInt8*
	UInt32 *refcon
)
{
	// The __rlwinm both extracts one pixel from the source word and compensates for the
	// table entries being 32 bits.  We add 1 to the table to get the lsh of each entry.
	UInt16 *table = ((UInt16*)refcon) + 1;
	
	for(int i = 2; i > 0; i--)
	{
		UInt32 temp = ((UInt32*)src)[0];			

		dst[0] = __rlwimi(
			table[__rlwinm(temp,  9, 27, 30)], 
			table[__rlwinm(temp,  5, 27, 30)], 
			16, 0, 15);
		dst[1] = __rlwimi(
			table[__rlwinm(temp, 17, 27, 30)], 
			table[__rlwinm(temp, 13, 27, 30)], 
			16, 0, 15);
		dst[2] = __rlwimi(
			table[__rlwinm(temp, 25, 27, 30)], 
			table[__rlwinm(temp, 21, 27, 30)], 
			16, 0, 15);
		dst[3] = __rlwimi(
			table[__rlwinm(temp,  1, 27, 30)], 
			table[__rlwinm(temp, 29, 27, 30)], 
			16, 0, 15);
		
		dst += variant4to16::xfer::dstStep * 4;
		src += variant4to16::xfer::srcStep * 4;
	}
}

#pragma mark
struct variant4to32
	: public variantSrc4, public variantHWInput32, public variantDst32
{
	struct xfer
	{
		typedef UInt8 *srcPtr;
		typedef UInt32 *dstPtr;
		
		enum
		{
			srcStep = 1,
			dstStep = 2
		};
		
		static inline void
		move(dstPtr dst, srcPtr src, UInt32 *refcon)
		{
			UInt32 temp = src[0];
			
			dst[0] = refcon[__rlwinm(temp, 28, 28, 31)];
			dst[1] = refcon[__rlwinm(temp,  0, 28, 31)];
		}
		
		static UInt32
		srcToFifoBytes(UInt32 srcBytes)
		{
			return(srcBytes << 3);
		}
	};
	
	// calculate the starting pixel offset for the hardware
	static UInt32 
	hwLeftCount(	UInt32 /*srcX*/,
					UInt32 &/*srcDataStart*/) 
	{
		// We'll always make the alignment come out right.
		return(0);
	};
	
	/* adjust the src start and end pointers and return the "long count" value
		that will be passed to the hardware.
	 */
	static UInt32 
	hwLongCount(	UInt32 &srcDataStart,
					UInt32 srcDataEnd) 
	{
		// Leave the source pointer alone.
				
		// Leave the src end alone, too.

		// return value is 8x the number of bytes between the pointers, converted to words.
		return((((srcDataEnd - srcDataStart) << 3) + 3) >> 2);
	};
};

// MBW -- XXX -- This could be improved with more attention to input alignment.
#pragma mark
struct variant8to16
	: public variantSrc8, public variantHWInput16, public variantDst16
{
	struct xfer
	{
		typedef UInt8 *srcPtr;
		typedef UInt32 *dstPtr;
		
		enum
		{
			srcStep = 2,
			dstStep = 1
		};
		
		static inline void
		move(dstPtr dst, srcPtr src, UInt32 *refcon)
		{
			UInt32 src1 = refcon[src[0]];
			UInt32 src2 = refcon[src[1]];
			dst[0] = __rlwimi(src2, src1, 16, 0, 15);
		}

		static UInt32
		srcToFifoBytes(UInt32 srcBytes)
		{
			return(srcBytes << 1);
		}
	};
	
	// calculate the starting pixel offset for the hardware
	static UInt32 
	hwLeftCount(	UInt32 /*srcX*/,
					UInt32 &/*srcDataStart*/) 
	{
		// We'll always make the alignment come out right.
		return(0);
	};
	
	/* adjust the src start and end pointers and return the "long count" value
		that will be passed to the hardware.
	 */
	static UInt32 
	hwLongCount(	UInt32 &srcDataStart,
					UInt32 srcDataEnd) 
	{
		// Leave the source pointer alone.
				
		// Leave the src end alone, too.

		// return value is half the number of bytes between the pointers, rounded up.
		return((1 + srcDataEnd - srcDataStart) >> 1);
	};
};

template <> inline void 
#pragma mark
cacheLineLoop<variant8to16::xfer>
(
	variant8to16::xfer::dstPtr &dst,	// UInt32*
	variant8to16::xfer::srcPtr &src,	// UInt8*
	UInt32 *refcon
)
{
	// The __rlwinm both extracts one pixel from the source word and compensates for the
	// table entries being 32 bits.  We add 1 to the table to get the lsh of each entry.
	UInt16 *table = ((UInt16*)refcon) + 1;
	
	for(int i = 4; i > 0; i--)
	{
		UInt32 temp = ((UInt32*)src)[0];			

		dst[0] = __rlwimi(
			table[__rlwinm(temp, 17, 23, 30)], 
			table[__rlwinm(temp,  9, 23, 30)], 
			16, 0, 15);
		dst[1] = __rlwimi(
			table[__rlwinm(temp,  1, 23, 30)], 
			table[__rlwinm(temp, 25, 23, 30)], 
			16, 0, 15);
		
		dst += variant8to16::xfer::dstStep * 2;
		src += variant8to16::xfer::srcStep * 2;
	}
}

// MBW -- XXX -- This could be improved with more attention to input alignment.
#pragma mark
struct variant8to32
	: public variantSrc8, public variantHWInput32, public variantDst32
{
	struct xfer
	{
		typedef UInt8 *srcPtr;
		typedef UInt32 *dstPtr;
		
		enum
		{
			srcStep = 1,
			dstStep = 1
		};
		
		static inline void
		move(dstPtr dst, srcPtr src, UInt32 *refcon)
		{
			dst[0] = refcon[src[0]];
		}

		static UInt32
		srcToFifoBytes(UInt32 srcBytes)
		{
			return(srcBytes << 2);
		}
	};
	
	// calculate the starting pixel offset for the hardware
	static UInt32 
	hwLeftCount(	UInt32 /*srcX*/,
					UInt32 &/*srcDataStart*/) 
	{
		// Since we're writing longs, the alignment always comes out even.
		return(0);
	};

	/* adjust the src start and end pointers and return the "long count" value
		that will be passed to the hardware.
	 */
	static UInt32 
	hwLongCount(	UInt32 &srcDataStart,
					UInt32 srcDataEnd) 
	{
		// Leave the source pointer alone.
		
		// Leave the src end alone, too.
		
		// return value is the number of bytes between the pointers.
		return(srcDataEnd - srcDataStart);
	};
};

template <> inline void 
#pragma mark
cacheLineLoop<variant8to32::xfer>
(
	variant8to32::xfer::dstPtr &dst,	// UInt32*
	variant8to32::xfer::srcPtr &src,	// UInt8*
	UInt32 *refcon
)
{
	for(int i = 2; i > 0; i--)
	{
		UInt32 temp = ((UInt32*)src)[0];			

		dst[0] = refcon[__rlwinm(temp,  8, 24, 31)], 
		dst[1] = refcon[__rlwinm(temp, 16, 24, 31)], 
		dst[2] = refcon[__rlwinm(temp, 24, 24, 31)], 
		dst[3] = refcon[__rlwinm(temp,  0, 24, 31)], 
		
		dst += variant8to32::xfer::dstStep * 4;
		src += variant8to32::xfer::srcStep * 4;
	}
}


#pragma mark
struct variant16to32HW
	: public variantSrc16, public variantHWInput16, public variantDst32
{
	
	struct xfer
	{
		typedef UInt32 *srcPtr;
		typedef UInt32 *dstPtr;

		enum
		{
			srcStep = 1,
			dstStep = 1
		};
		
		/* MBW -- It appears that 15->32 doesn't work correctly on this board,
			so we're using 16->32.  This means that we need to munge the
			pixels a bit.
			
			                    11 1111 1111 2222 2222 2233
			bit:    0123 4567 8901 2345 6789 0123 4567 8901

			before: xrrr rrgg gggb bbbb xrrr rrgg gggb bbbb
			         123 4512 3451 2345  123 4512 3451 2345
	 
			after:  rrrr rggg gggb bbbb rrrr rggg gggb bbbb
			        1234 5123 4511 2345 1234 5123 4511 2345
		*/
		static inline void
		move(dstPtr dst, srcPtr src, UInt32*)
		{
			
			UInt32 temp = src[0];
			
			// get the big parts
			temp = __rlwimi(temp, temp,  1,  0,  9);
			temp = __rlwimi(temp, temp,  1, 16, 25);
			
			// fix the lsb of the green channel
			temp = __rlwimi(temp, temp,	 32 - 5, 10, 10);
			temp = __rlwimi(temp, temp,	 32 - 5, 26, 26);
			
			dst[0] = temp;
		}

		static UInt32
		srcToFifoBytes(UInt32 srcBytes)
		{
			return(srcBytes);
		}
	};
	
	enum
	{
		srcPixFmt = SSTG_PIXFMT_16BPP | SSTG_HOST_WORD_SWIZZLE
	};

	// calculate the starting pixel offset for the hardware
	static UInt32 
	hwLeftCount(	UInt32 /*srcX*/,
					UInt32 srcDataStart) 
	{
		// take the bottom bit of the original start address.
		return(srcDataStart & 2);
	};

};

// MBW -- XXX -- This could be improved with more attention to input alignment.
#pragma mark
struct variant16to32
	: public variantSrc16, public variantHWInput32, public variantDst32
{
	
	struct xfer
	{
		typedef UInt16 *srcPtr;
		typedef UInt32 *dstPtr;

		enum
		{
			srcStep = 1,
			dstStep = 1
		};
		
		static inline void
		move(dstPtr dst, srcPtr src, UInt32*)
		{
			UInt32 in = src[0];
			UInt32 out;
			
			out = __rlwinm(     in, 17-8,   8, 12);
			out = __rlwimi(out, in, 17-13, 13, 15);
			out = __rlwimi(out, in, 22-16, 16, 20);
			out = __rlwimi(out, in, 22-21, 21, 23);
			out = __rlwimi(out, in, 27-24, 24, 28);
			out = __rlwimi(out, in,    30, 29, 31);
			
			dst[0] = out;
		}

		static UInt32
		srcToFifoBytes(UInt32 srcBytes)
		{
			return(srcBytes << 1);
		}
	};
	

	// calculate the starting pixel offset for the hardware
	static UInt32 
	hwLeftCount(	UInt32 /*srcX*/,
					UInt32 &/*srcDataStart*/) 
	{
		// The source pointer is already aligned.
		return(0);
	};
	
	/* adjust the src start and end pointers and return the "long count" value
		that will be passed to the hardware.
	 */
	static UInt32 
	hwLongCount(	UInt32 &srcDataStart,
					UInt32 srcDataEnd) 
	{
		// Leave the source pointer alone
		
		// Leave the source end pointer alone, too
		
		// return value is the number of halfwords between the pointers, rounded up.
		return((1 + srcDataEnd - srcDataStart) >> 1);
	};
};

template <> inline void 
#pragma mark
cacheLineLoop<variant16to32::xfer>
(
	variant16to32::xfer::dstPtr &dst,	// UInt32*
	variant16to32::xfer::srcPtr &src,	// UInt16*
	UInt32 */*refcon*/
)
{
	// The __rlwinm both extracts one pixel from the source word and compensates for the
	// table entries being 32 bits.  We add 1 to the table to get the lsh of each entry.
	for(int i = 4; i > 0; i--)
	{
		UInt32 in = ((UInt32*)src)[0];			
		UInt32 out;

		out = __rlwinm(     in, 16+17-8,   8, 12);
		out = __rlwimi(out, in, 16+17-13, 13, 15);
		out = __rlwimi(out, in, 16+22-16, 16, 20);
		out = __rlwimi(out, in, 16+22-21, 21, 23);
		out = __rlwimi(out, in, 16+27-24, 24, 28);
		out = __rlwimi(out, in, 16+   30, 29, 31);
		
		dst[0] = out;

		out = __rlwinm(     in, 17-8,   8, 12);
		out = __rlwimi(out, in, 17-13, 13, 15);
		out = __rlwimi(out, in, 22-16, 16, 20);
		out = __rlwimi(out, in, 22-21, 21, 23);
		out = __rlwimi(out, in, 27-24, 24, 28);
		out = __rlwimi(out, in,    30, 29, 31);
		
		dst[1] = out;
		
		dst += variant16to32::xfer::dstStep * 2;
		src += variant16to32::xfer::srcStep * 2;
	}
}

#pragma mark
struct variant32to16
	: public variantSrc32, public variantHWInput16, public variantDst16
{
	struct xfer
	{	
		typedef UInt32 *srcPtr;
		typedef UInt32 *dstPtr;
		
		enum
		{
			srcStep = 2,
			dstStep = 1
		};
		
		static inline void
		move(dstPtr dst, srcPtr src, UInt32*)
		{
			UInt32 in1 = src[0];
			UInt32 in2 = src[1];
			UInt32 out;
			
			out = __rlwinm(     in1, 8-1,		 1,	 5);
			out = __rlwimi(out, in1, 16-6,		 6,	10);
			out = __rlwimi(out, in1, 24-11,		11,	15);
			out = __rlwimi(out, in2, 32+8-17,	17,	21);
			out = __rlwimi(out, in2, 32+16-22,	22,	26);
			out = __rlwimi(out, in2, 32+24-27,	27,	31);
			
			dst[0] = out;
		}
		
		static UInt32
		srcToFifoBytes(UInt32 srcBytes)
		{
			return(srcBytes >> 1);
		}
	};
	
	// calculate the starting pixel offset for the hardware
	static UInt32 
	hwLeftCount(	UInt32 /*srcX*/,
					UInt32 &/*srcDataStart*/) 
	{
		// The source pointer is already aligned.
		return(0);
	};

	/* adjust the src start and end pointers and return the "long count" value
		that will be passed to the hardware.
	 */
	static UInt32 
	hwLongCount(	UInt32 &srcDataStart,
					UInt32 srcDataEnd) 
	{
		// Leave the source pointer alone
		
		// Leave the source end pointer alone, too
		
		// return value is the number of doubles between the pointers (rounded up)
		return((7 + srcDataEnd - srcDataStart) >> 3);
	};
};

#pragma mark -
#pragma mark ¥ functions

/*
	storeCacheLines<>()
	
	This function takes care of the hard part of cacheable transfers.
	It expects the 'xfer' class to have the following members:
	
	xfer::srcPtr -- the type of the "src" argument to xfer::move
	xfer::dstPtr -- the type of the "dst" argument to xfer::move
	xfer::move(src, dst, refcon) -- a function that moves one quantum of data from *src to *dst
	xfer::srcStep -- the amount to add to a xfer::srcPtr after calling xfer::move
	xfer::dstStep -- the amount to add to a xfer::dstPtr after calling xfer::move
	
	The value of the "count" argument is the number of longs that will be put into the
	fifo.  If one call to xfer::move() puts more than 4 bytes into the fifo, the caller must
	ensure that the value of count is evenly divisible by the number of longs that xfer::move()
	stores.  Failing to do this will result in undefined behavior.
	
	This function calls xfer::move() repeatedly, incrementing the source and destination pointers 
	each time as specified by the srcStep and dstStep.
	
	Note that the loop unroller has a field day with this function.
*/

template <class xfer> inline 
typename xfer::dstPtr 
storeCacheLines(
	typename xfer::dstPtr dst, 
	typename xfer::srcPtr src, 
	UInt32 count, 
	UInt32 *refcon)
{
	// dstStride is the number of bytes the destination needs to move for each
	// call to xfer::move().
	const UInt32 dstStride = sizeof(dst[0]) * xfer::dstStep;
	
	// i starts out as the number of bytes we need to move before we hit
	// destination cache line alignment.
	UInt32 i = ((-(UInt32)dst) & 31);
	
	// Determine the number of full cachelines to flush
	UInt32 numFullCacheLines = ((count << 2) - i) / 32;
	
	if (i < (count << 2))
	{
		// Count is now the number of longs in the last partial cache line
		count = ((count << 2) - i - (numFullCacheLines * 32)) >> 2;
	
		// This loop gets us to cache line alignment
		for (; i > 0; i -= dstStride)
		{
			xfer::move(dst, src, refcon);
			dst += xfer::dstStep;
			src += xfer::srcStep;
		}
		__dcbst(dst, -dstStride);
		__dcbz(dst, 0);
		__dcbt(src, 32);
		
		// This loop does the bulk of the transfer; one cacheline per iteration
		for (;numFullCacheLines > 0; --numFullCacheLines)
		{
			__dcbz(dst, 32);
			__dcbt(src, 32);
			
			cacheLineLoop<xfer>(dst, src, refcon);
			
			__dcbst(dst, -64);
		}
		__dcbst(dst, -32);
	}
	
	// Get the last partial cache line.
	for (; count ; count -= (dstStride / 4))
	{
		xfer::move(dst, src, refcon);
		dst += xfer::dstStep;
		src += xfer::srcStep;
	}	
	
	return(dst);
}

#pragma mark storeCacheLines<variant4to32::xfer>

// Due to sub-byte source addressing issues, the 4->32 case needs special handling.
// This is a specialization of the storeCacheLines template that handles this case.
template <> inline 
typename variant4to32::xfer::dstPtr 
storeCacheLines<variant4to32::xfer>(
		typename variant4to32::xfer::dstPtr dst, // UInt32
		typename variant4to32::xfer::srcPtr src, // UInt8
		UInt32 count, 
		UInt32 *refcon)
{
	// dstStride is the number of bytes the destination needs to move for each
	// call to variant4to32::xfer::move().
	const UInt32 dstStride = sizeof(dst[0]) * variant4to32::xfer::dstStep;
	bool odd = false;
	UInt32 leftover = 0;
	UInt32 temp;
	
	// i starts out as the number of bytes we need to move before we hit
	// destination cache line alignment.
	SInt32 i = ((-(UInt32)dst) & 31);
	
	// Determine the number of full cachelines to flush
	SInt32 numFullCacheLines = ((count << 2) - i) / 32;
		
	if (i < (count << 2))
	{
		// Count is now the number of longs in the last partial cache line
		count = ((count << 2) - i - (numFullCacheLines * 32)) >> 2;
	
		// This loop gets us to cache line alignment
		for (; i > 4; i -= 8)
		{
			variant4to32::xfer::move(dst, src, refcon);
			dst += variant4to32::xfer::dstStep;
			src += variant4to32::xfer::srcStep;
		}
		
		// Handle a possible leftover half-byte of source
		if(i > 0)
		{
			leftover = src[0];
			dst[0] = refcon[__rlwinm(leftover, 28, 28, 31)];
			src++;
			dst++;
			odd = true;
		}
		
		__dcbst(dst, -dstStride);
		__dcbz(dst, 0);
		__dcbt(src, 32);
		
		// This loop does the bulk of the transfer; one cacheline per iteration
		if(odd)
		{
			for (;numFullCacheLines > 0; --numFullCacheLines)
			{
				__dcbz(dst, 32);
				__dcbt(src, 32);
				
				temp = ((UInt32*)src)[0];

				dst[0] = refcon[__rlwinm(leftover,  0, 28, 31)], 
				dst[1] = refcon[__rlwinm(temp,  4, 28, 31)], 
				dst[2] = refcon[__rlwinm(temp,  8, 28, 31)], 
				dst[3] = refcon[__rlwinm(temp, 12, 28, 31)], 
				dst[4] = refcon[__rlwinm(temp, 16, 28, 31)], 
				dst[5] = refcon[__rlwinm(temp, 20, 28, 31)], 
				dst[6] = refcon[__rlwinm(temp, 24, 28, 31)], 
				dst[7] = refcon[__rlwinm(temp, 28, 28, 31)], 

				leftover = temp;
				
				dst += 8;
				src += 4;
				
				__dcbst(dst, -64);
			}
		}
		else
		{
			for (;numFullCacheLines > 0; --numFullCacheLines)
			{
				__dcbz(dst, 32);
				__dcbt(src, 32);
				
				temp = ((UInt32*)src)[0];			

				dst[0] = refcon[__rlwinm(temp,  4, 28, 31)], 
				dst[1] = refcon[__rlwinm(temp,  8, 28, 31)], 
				dst[2] = refcon[__rlwinm(temp, 12, 28, 31)], 
				dst[3] = refcon[__rlwinm(temp, 16, 28, 31)], 
				dst[4] = refcon[__rlwinm(temp, 20, 28, 31)], 
				dst[5] = refcon[__rlwinm(temp, 24, 28, 31)], 
				dst[6] = refcon[__rlwinm(temp, 28, 28, 31)], 
				dst[7] = refcon[__rlwinm(temp,  0, 28, 31)], 
				
				dst += 8;
				src += 4;
				
				__dcbst(dst, -64);
			}
		}
		__dcbst(dst, -32);
	}
	
	i = count;
	
	if(odd && (i > 0))
	{
		dst[0] = refcon[__rlwinm(leftover, 0, 28, 31)];
		dst++;
		i -= (dstStride / 4);
	}
	
	// Get the last partial cache line.
	for (; i > 0 ; i -= (dstStride / 4))
	{
		variant4to32::xfer::move(dst, src, refcon);
		dst += variant4to32::xfer::dstStep;
		src += variant4to32::xfer::srcStep;
	}	
	
	return(dst);
}

