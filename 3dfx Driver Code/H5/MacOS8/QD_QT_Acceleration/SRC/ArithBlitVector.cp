#include <NQDAcceleration.h>
#include "RegionParser.h"
#include "Utilities.h"

#include "ArithBlitVector.h"

#pragma mark ¥ Switches

// Uncomment this #define to have this code use the cachable space.
//#define ARITH_USE_CACHELINES

#pragma mark ¥ Prototypes

template <class variant> void  
VectorHostBlit(NQDDrawVars  *drawVars, Rect &origDstRect);

template <class variant> void  
VectorHostBlit16to32(NQDDrawVars  *drawVars, Rect &origDstRect);

// Turn on Altivec compilation
#pragma altivec_model on
#pragma scheduling altivec
#pragma altivec_vrsave on

#pragma mark ¥ Inline Functions

// Branchess unsigned "min"
// NOTE:  This may not work correctly if either x or y is greater than about 2^30.
inline UInt32 minBranchless(UInt32 x, UInt32 y)
{
	SInt32 temp = x - y;
	
	// Smear the sign bit, making "temp" into a mask, all ones iff y is greater.
	temp >>= 31;
	
	temp &= x ^ y;
	temp ^= y;
	
	return((UInt32)temp);
}

// Branchess unsigned "max"
// NOTE:  This may not work correctly if either x or y is greater than about 2^30.
inline UInt32 maxBranchless(UInt32 x, UInt32 y)
{
	SInt32 temp = x - y;
	
	// Smear the sign bit, making "temp" into a mask, all ones iff y is greater.
	temp >>= 31;
	
	temp &= x ^ y;
	temp ^= x;
	
	return((UInt32)temp);
}

inline void
Vec16to32(vector pixel src, vector unsigned char &dst1, vector unsigned char &dst2)
{
	vector unsigned char temp;
	
	temp = (vector unsigned char)vec_unpackh(src);
	dst1 = vec_or(vec_sl(temp, vec_splat_u8(3)), vec_sr(temp, vec_splat_u8(2)));

	temp = (vector unsigned char)vec_unpackl(src);
	dst2 = vec_or(vec_sl(temp, vec_splat_u8(3)), vec_sr(temp, vec_splat_u8(2)));
}

inline vector unsigned char
Vec16to32h(vector unsigned char src)
{
	vector unsigned char temp;
	
	temp = (vector unsigned char)vec_unpackh((vector pixel)src);
	return(vec_or(vec_sl(temp, vec_splat_u8(3)), vec_sr(temp, vec_splat_u8(2))));
}

inline vector unsigned char
Vec16to32l(vector unsigned char src)
{
	vector unsigned char temp;
	
	temp = (vector unsigned char)vec_unpackl((vector pixel)src);
	return(vec_or(vec_sl(temp, vec_splat_u8(3)), vec_sr(temp, vec_splat_u8(2))));
}

inline vector unsigned char
Vec32to16(vector unsigned char h, vector unsigned char l)
{
	return((vector unsigned char)vec_packpx((vector unsigned int)h, (vector unsigned int)l));
}

#pragma mark ¥ Classes

template <class opClass>
#pragma mark
struct Vector32to32
{
	enum
	{
		srcPixelsToBytes = 4,
		dstPixelsToBytes = 4,
		srcVectorCount = 1,
		scratchVectorCount = opClass::scratchVectorCount
		
	};
	
	static void setup(NQDDrawVars *drawVars, vector unsigned char *scratch)
	{
		opClass::setup(drawVars, scratch);
	}

	static vector unsigned char op(
			vector unsigned char dst,
			vector unsigned char *src,
			vector unsigned char *scratch)
	{
		return(opClass::op(dst, src[0], scratch));
	}
	
};

template <class opClass>
#pragma mark
struct Vector16to16
{
	enum
	{
		srcPixelsToBytes = 2,
		dstPixelsToBytes = 2,
		srcVectorCount = 1,
		scratchVectorCount = opClass::scratchVectorCount
	};
	
	static void setup(NQDDrawVars *drawVars, vector unsigned char *scratch)
	{
		opClass::setup(drawVars, scratch);
	}

	static vector unsigned char op(
			vector unsigned char dst,
			vector unsigned char *src,
			vector unsigned char *scratch)
	{
		return(Vec32to16(opClass::op(Vec16to32h(dst), Vec16to32h(src[0]), scratch), opClass::op(Vec16to32l(dst), Vec16to32l(src[0]), scratch)));
	}
	
};

template <class opClass>
#pragma mark
struct Vector32to16
{
	enum
	{
		srcPixelsToBytes = 4,
		dstPixelsToBytes = 2,
		srcVectorCount = 2,
		scratchVectorCount = opClass::scratchVectorCount
		
	};
	
	static void setup(NQDDrawVars *drawVars, vector unsigned char *scratch)
	{
		opClass::setup(drawVars, scratch);
	}

	static vector unsigned char op(
			vector unsigned char dst,
			vector unsigned char *src,
			vector unsigned char *scratch)
	{
		
		return(Vec32to16(opClass::op(Vec16to32h(dst), src[0], scratch), opClass::op(Vec16to32l(dst), src[1], scratch)));
	}
	
};

template <class opClass>
#pragma mark
struct Vector16to32
{
	enum
	{
		srcPixelsToBytes = 2,
		dstPixelsToBytes = 4,
		srcVectorCount = 1,
		scratchVectorCount = opClass::scratchVectorCount
		
	};
	
	static void setup(NQDDrawVars *drawVars, vector unsigned char *scratch)
	{
		opClass::setup(drawVars, scratch);
	}

	static vector unsigned char op(
			vector unsigned char dst,
			vector unsigned char *src,
			vector unsigned char *scratch)
	{
		return(opClass::op(dst, Vec16to32h(src[0]), scratch));
	}
	
};

#pragma mark
struct VecAddOver
{
	enum
	{
		scratchVectorCount = 1
	};

	static void setup(NQDDrawVars */*drawVars*/, vector unsigned char */*scratch*/)
	{
		// No scratch needed
	}
	
	static vector unsigned char op(
			vector unsigned char dst,
			vector unsigned char src,
			vector unsigned char */*scratch*/)
	{
		return(vec_add(dst, src));
	}
};

#pragma mark
struct VecSubOver
{
	enum
	{
		scratchVectorCount = 1
	};

	static void setup(NQDDrawVars */*drawVars*/, vector unsigned char */*scratch*/)
	{
		// No scratch needed
	}
	
	static vector unsigned char op(
			vector unsigned char dst,
			vector unsigned char src,
			vector unsigned char */*scratch*/)
	{
		return(vec_sub(dst, src));
	}
};

#pragma mark
struct VecAdMin
{
	enum
	{
		scratchVectorCount = 1
	};

	static void setup(NQDDrawVars */*drawVars*/, vector unsigned char */*scratch*/)
	{
		// No scratch needed
	}
	
	static vector unsigned char op(
			vector unsigned char dst,
			vector unsigned char src,
			vector unsigned char */*scratch*/)
	{
		return(vec_min(src, dst));
	}
};

#pragma mark
struct VecAdMax
{
	enum
	{
		scratchVectorCount = 1
	};

	static void setup(NQDDrawVars */*drawVars*/, vector unsigned char */*scratch*/)
	{
		// No scratch needed
	}
	
	static vector unsigned char op(
			vector unsigned char dst,
			vector unsigned char src,
			vector unsigned char */*scratch*/)
	{	
		return(vec_max(src, dst));
	}
};

#pragma mark
struct VecAddPin
{
	enum
	{
		scratchVectorCount = 1
	};

	static void setup(NQDDrawVars *drawVars, vector unsigned char *scratch)
	{
		// Construct the op color
		UInt32 color = __rlwnm(drawVars->rWt, 8, 8, 15);
		color = __rlwimi(color, drawVars->gWt, 0, 16, 23);
		color = __rlwimi(color, drawVars->bWt, 32 - 8, 24, 31);
		
		// load the color into a vector register
		vector unsigned long temp;
		temp = vec_lde(0, &color);
		
		// and splat it to all elelemts of the vector
		temp = vec_splat(vec_perm(temp, temp, vec_lvsl(0, &color)), 0);
		
		scratch[0] = (vector unsigned char)temp;
	}
	
	static vector unsigned char op(
			vector unsigned char dst,
			vector unsigned char src,
			vector unsigned char *scratch)
	{
		return(vec_min(vec_adds(dst, src), scratch[0]));
	}
};

#pragma mark
struct VecSubPin
{
	enum
	{
		scratchVectorCount = 1
	};

	static void setup(NQDDrawVars *drawVars, vector unsigned char *scratch)
	{
		// Construct the op color
		UInt32 color = __rlwnm(drawVars->rWt, 8, 8, 15);
		color = __rlwimi(color, drawVars->gWt, 0, 16, 23);
		color = __rlwimi(color, drawVars->bWt, 32 - 8, 24, 31);
		
		// load the color into a vector register
		vector unsigned long temp;
		temp = vec_lde(0, &color);
		
		// and splat it to all elelemts of the vector
		temp = vec_splat(vec_perm(temp, temp, vec_lvsl(0, &color)), 0);
		
		scratch[0] = (vector unsigned char)temp;
	}
	
	static vector unsigned char op(
			vector unsigned char dst,
			vector unsigned char src,
			vector unsigned char *scratch)
	{	
		return(vec_max(vec_subs(dst, src), scratch[0]));
	}
};

#pragma mark
struct VecBlend
{
	enum
	{
		scratchVectorCount = 2
	};

	static void setup(NQDDrawVars *drawVars, vector unsigned char *scratch)
	{
		union
		{
			vector unsigned char v;
			UInt16 a[8];
		} x;
		
		scratch[0] = vec_splat_u8(0);
		
		x.a[0] = 0;
		x.a[1] = drawVars->rWt >> 1;
		x.a[2] = drawVars->gWt >> 1;
		x.a[3] = drawVars->bWt >> 1;
		x.a[4] = 0;
		x.a[5] = drawVars->rWt >> 1;
		x.a[6] = drawVars->gWt >> 1;
		x.a[7] = drawVars->bWt >> 1;
		scratch[1] = x.v;

//		x.a[0] = 0;
//		x.a[1] = drawVars->notRWt >> 1;
//		x.a[2] = drawVars->notGWt >> 1;
//		x.a[3] = drawVars->notBWt >> 1;
//		x.a[4] = 0;
//		x.a[5] = drawVars->notRWt >> 1;
//		x.a[6] = drawVars->notGWt >> 1;
//		x.a[7] = drawVars->notBWt >> 1;
//		scratch[2] = x.v;
		
		
	}

	static vector unsigned char op(
			vector unsigned char dst,
			vector unsigned char src,
			vector unsigned char *scratch)
	{		
	
		vector signed short srcHigh = (vector signed short)vec_mergeh(scratch[0], src);
		vector signed short dstHigh = (vector signed short)vec_mergeh(scratch[0], dst);
		vector signed short srcLow = (vector signed short)vec_mergel(scratch[0], src);
		vector signed short dstLow = (vector signed short)vec_mergel(scratch[0], dst);
				
		return(vec_packsu(
			vec_mradds((vector signed short)scratch[1], vec_sub(srcHigh, dstHigh), dstHigh),
			vec_mradds((vector signed short)scratch[1], vec_sub(srcLow, dstLow), dstLow)
		));
	}
};

#pragma mark
struct VecBlendHalf
{
	enum
	{
		scratchVectorCount = 1
	};

	static void setup(NQDDrawVars */*drawVars*/, vector unsigned char */*scratch*/)
	{
		// No scratch needed
	}

	static vector unsigned char op(
			vector unsigned char dst,
			vector unsigned char src,
			vector unsigned char */*scratch*/)
	{
		return(vec_avg(src, dst));
	}
};


#pragma mark -

RegionBlitProc PickArithBlitVectorVariantLL(UInt32 srcDepth, UInt32 dstDepth, NQDDrawVars *drawVars)
{
	RegionBlitProc result = nil;
	
	switch(dstDepth)
	{
		case 32:		
		switch(srcDepth)
		{
			case 32:
			switch(drawVars->mode)
			{
				case addOver:		result = VectorHostBlit< Vector32to32<VecAddOver> >;		break;
				case subOver:		result = VectorHostBlit< Vector32to32<VecSubOver> >;		break;
				case adMin:			result = VectorHostBlit< Vector32to32<VecAdMin> >;			break;
				case adMax:			result = VectorHostBlit< Vector32to32<VecAdMax> >;			break;
				case addPin:		result = VectorHostBlit< Vector32to32<VecAddPin> >;			break;
				case subPin:		result = VectorHostBlit< Vector32to32<VecSubPin> >;			break;
				case blend:
					if( ((drawVars->rWt >= 0x7f7f) && (drawVars->rWt <= 0x8080)) &&
						((drawVars->gWt >= 0x7f7f) && (drawVars->gWt <= 0x8080)) &&
						((drawVars->bWt >= 0x7f7f) && (drawVars->bWt <= 0x8080)))
					{
						result = VectorHostBlit< Vector32to32<VecBlendHalf> >;
					}
					else
					{
						result = VectorHostBlit< Vector32to32<VecBlend> >;
					}
				break;
			}
			break;

			case 16:
			switch(drawVars->mode)
			{
				case addOver:		result = VectorHostBlit16to32<VecAddOver>;		break;
				case subOver:		result = VectorHostBlit16to32<VecSubOver>;		break;
				case adMin:			result = VectorHostBlit16to32<VecAdMin>;			break;
				case adMax:			result = VectorHostBlit16to32<VecAdMax>;			break;
				case addPin:		result = VectorHostBlit16to32<VecAddPin>;			break;
				case subPin:		result = VectorHostBlit16to32<VecSubPin>;			break;
				case blend:
					if( ((drawVars->rWt >= 0x7f7f) && (drawVars->rWt <= 0x8080)) &&
						((drawVars->gWt >= 0x7f7f) && (drawVars->gWt <= 0x8080)) &&
						((drawVars->bWt >= 0x7f7f) && (drawVars->bWt <= 0x8080)))
					{
						result = VectorHostBlit16to32<VecBlendHalf>;
					}
					else
					{
						result = VectorHostBlit16to32<VecBlend>;
					}
				break;
			}
			break;
		}
		break;
		case 16:		
		switch(srcDepth)
		{
			case 32:
			switch(drawVars->mode)
			{
				case addOver:		result = VectorHostBlit< Vector32to16<VecAddOver> >;		break;
				case subOver:		result = VectorHostBlit< Vector32to16<VecSubOver> >;		break;
				case adMin:			result = VectorHostBlit< Vector32to16<VecAdMin> >;			break;
				case adMax:			result = VectorHostBlit< Vector32to16<VecAdMax> >;			break;
				case addPin:		result = VectorHostBlit< Vector32to16<VecAddPin> >;			break;
				case subPin:		result = VectorHostBlit< Vector32to16<VecSubPin> >;			break;
				case blend:
					if( ((drawVars->rWt >= 0x7f7f) && (drawVars->rWt <= 0x8080)) &&
						((drawVars->gWt >= 0x7f7f) && (drawVars->gWt <= 0x8080)) &&
						((drawVars->bWt >= 0x7f7f) && (drawVars->bWt <= 0x8080)))
					{
						result = VectorHostBlit< Vector32to16<VecBlendHalf> >;
					}
					else
					{
						result = VectorHostBlit< Vector32to16<VecBlend> >;
					}
				break;
			}
			break;

			case 16:
			switch(drawVars->mode)
			{
				case addOver:		result = VectorHostBlit< Vector16to16<VecAddOver> >;		break;
				case subOver:		result = VectorHostBlit< Vector16to16<VecSubOver> >;		break;
				case adMin:			result = VectorHostBlit< Vector16to16<VecAdMin> >;			break;
				case adMax:			result = VectorHostBlit< Vector16to16<VecAdMax> >;			break;
				case addPin:		result = VectorHostBlit< Vector16to16<VecAddPin> >;			break;
				case subPin:		result = VectorHostBlit< Vector16to16<VecSubPin> >;			break;
				case blend:
					if( ((drawVars->rWt >= 0x7f7f) && (drawVars->rWt <= 0x8080)) &&
						((drawVars->gWt >= 0x7f7f) && (drawVars->gWt <= 0x8080)) &&
						((drawVars->bWt >= 0x7f7f) && (drawVars->bWt <= 0x8080)))
					{
						result = VectorHostBlit< Vector16to16<VecBlendHalf> >;
					}
					else
					{
						result = VectorHostBlit< Vector16to16<VecBlend> >;
					}
				break;
			}
			break;
		}
		break;
	}

	return(result);
}


template <class variant> void  
VectorHostBlit(NQDDrawVars  *drawVars, Rect &origDstRect)
{	
	Rect srcRect, dstRect;
		
	srcRect = origDstRect;
	FastOffsetRect(srcRect, 
			-drawVars->dstRect.left + drawVars->srcRect.left - drawVars->srcPixMap.bounds.left, 
			-drawVars->dstRect.top + drawVars->srcRect.top - drawVars->srcPixMap.bounds.top);
	
	dstRect = origDstRect;
	FastOffsetRect(dstRect, -drawVars->dstPixMap.bounds.left, -drawVars->dstPixMap.bounds.top);

	UInt32 srcBase = (UInt32)drawVars->srcPixMap.baseAddr;
	SInt32 srcRowBytes = drawVars->srcPixMap.rowBytes;
	UInt32 dstBase = (UInt32)drawVars->dstPixMap.baseAddr;
	SInt32 dstRowBytes = drawVars->dstPixMap.rowBytes;

#ifdef ARITH_USE_CACHELINES
	// Move the destination address into cacheable space.
	dstBase = (dstBase & 0xF3FFFFFF) | 0x06000000;
#endif

	// We'd better not be called with anything but the correct mode and no colorization.
//	UInt32 mode = drawVars->mode & 0x0000003f,
//	UInt32 foreColor = drawVars->foreColor,
//	UInt32 backColor = drawVars->backColor);
	
	// We don't currently do indexed modes.
//	UInt32 *scaleTable = drawVars->scaleTable,
		
	SInt32  height, width;

  	height = dstRect.bottom - dstRect.top;
  	width = dstRect.right - dstRect.left;

	if (srcRowBytes < 0) 
	{
		srcRowBytes = -srcRowBytes;
		dstRowBytes = -dstRowBytes;
	}
	
	// Figure out the addresses of the top-left corner of both source and dest
	srcBase += (srcRect.left * variant::srcPixelsToBytes) + (srcRect.top * srcRowBytes);
	dstBase += (dstRect.left * variant::dstPixelsToBytes) + (dstRect.top * dstRowBytes);
		
	unsigned char *srcPtr, *dstPtr;
	UInt32 dstByteWidth = width * variant::dstPixelsToBytes;
	vector unsigned char scratch[variant::scratchVectorCount];
	
	variant::setup(drawVars, scratch);
	
#ifdef ARITH_USE_CACHELINES
	vector unsigned char dstSwizzle;
	
	if(variant::dstPixelsToBytes == 2)
	{
		// This is the proper permute vector to swizzle or unswizzle 16-bit pixels
		// being accessed through the 32-bit aperture.
		dstSwizzle = (vector unsigned char)(
						 2,  3,  0,  1, 
						 6,  7,  4,  5, 
						10, 11,  8,  9, 
						14, 15, 12, 13);
	}
#endif
	
	vector bool char zero, one;
	// "zero" is all zero bits
	zero = (vector bool char)vec_splat_u8(0);
	
	// "one" is all one bits
	one = (vector bool char)vec_splat_s8(-1);

	while(height-- > 0)
	{
		vector unsigned char src[variant::srcVectorCount];
		vector unsigned char dst;
		vector unsigned char src1, src2, work, srcPerm;
		vector bool char mask;
		
		srcPtr = (unsigned char*)srcBase;
		dstPtr = (unsigned char*)dstBase;
		
#ifdef ARITH_USE_CACHELINES
		__dcbt(dstPtr, 0);
#endif
		
		// Set up mask as firstMask for this line
		mask = vec_perm(zero, one, vec_lvsr(0, dstPtr));
		
		// Set up the permute for loading the source
		SInt32 shiftLeftBytes;
		
		if(variant::srcVectorCount == 2)
		{
			shiftLeftBytes = (((UInt32)srcPtr) & 0xF) - (((UInt32)dstPtr << 1) & 0x1F);
			
			if(shiftLeftBytes < -15)
			{
				srcPtr -= 16;
			}
		}
		else
		{
			shiftLeftBytes = ((UInt32)srcPtr & 0xF) - ((UInt32)dstPtr & 0xF);
		}
		
		if( shiftLeftBytes <= 0 )
		{
			// The source will need to be shifted right to align with the dest
			srcPerm = vec_lvsr(-shiftLeftBytes, (unsigned char *)0);
			
			// The initial contents of src2 don't matter.  Assign something so that the compiler doesn't complain.
			src2 = (vector unsigned char)zero;
		}
		else
		{
			// The source will need to be shifted left to align with the dest
			srcPerm = vec_lvsl(shiftLeftBytes, (unsigned char *)0);

			// Load the first part of the source
			src2 = vec_ld(0, (vector unsigned char*)srcPtr);
			srcPtr += 16;
		}
		
		unsigned char * dstEnd = dstPtr + dstByteWidth;
		
		// Change dstPtr to point at the beginning of the vector.
		dstPtr = (unsigned char *)((UInt32)dstPtr & ~0x0F);
		
		while(dstEnd - dstPtr > 0)
		{
			// First/last mask case
			
			// Load the next part of the source;
			for(int i=0 ;i < variant::srcVectorCount; i++)
			{
				src1 = src2;
				src2 = vec_ld(0, (vector unsigned char*)srcPtr);
				srcPtr += 16;
			
				// Permute the source to align with the dest
				src[i] = vec_perm(src1, src2, srcPerm);
			}
			
			// Load the destination
			dst = vec_ld(0, (vector unsigned char*)dstPtr);

#ifdef ARITH_USE_CACHELINES
			if(variant::dstPixelsToBytes != 4)
			{
				// Unswizzle the destination
				dst = vec_perm(dst, dst, dstSwizzle);
			}
#endif			
			// Perform the operation
			work = variant::op(dst, src, scratch);
			
			// Create the end mask, if necessary
			mask = vec_and(mask, vec_perm(one, zero, vec_lvsl( 16 - minBranchless(dstEnd - dstPtr, 16L) , (unsigned char *)0)));
			
			// Apply the mask for this part of the destination
			dst = vec_sel(dst, work, mask);
			
#ifdef ARITH_USE_CACHELINES
			if(variant::dstPixelsToBytes != 4)
			{
				// Reswizzle the destination
				dst = vec_perm(dst, dst, dstSwizzle);
			}
#endif
			// Write back the result
			vec_st(dst, 0, (vector unsigned char*)dstPtr);
			
			dstPtr += 16;
			
#ifdef ARITH_USE_CACHELINES
			__dcbt(dstPtr, 0);
			__dcbf(dstPtr, -32);
#endif

			mask = one;

			// Simplified main loop case, doesn't have to worry about first/last mask.
			SInt32 count = (dstEnd - dstPtr);
			count >>= 4;
			while(count-- > 0)
			{

				// Load the next part of the source;
				for(int i=0 ;i < variant::srcVectorCount; i++)
				{
					src1 = src2;
					src2 = vec_ld(0, (vector unsigned char*)srcPtr);
					srcPtr += 16;
				
					// Permute the source to align with the dest
					src[i] = vec_perm(src1, src2, srcPerm);
				}
					
				// Load the destination
				dst = vec_ld(0, (vector unsigned char*)dstPtr);

#ifdef ARITH_USE_CACHELINES
				if(variant::dstPixelsToBytes != 4)
				{
					// Unswizzle the destination
					dst = vec_perm(dst, dst, dstSwizzle);
				}
#endif				

				// Perform the operation
				dst = variant::op(dst, src, scratch);
				
#ifdef ARITH_USE_CACHELINES
				if(variant::dstPixelsToBytes != 4)
				{
					// Reswizzle the destination
					dst = vec_perm(dst, dst, dstSwizzle);
				}
#endif
				// Write back the result
				vec_st(dst, 0, (vector unsigned char*)dstPtr);
				
				dstPtr += 16;

#ifdef ARITH_USE_CACHELINES
				__dcbt(dstPtr, 0);
				__dcbf(dstPtr, -32);
#endif
			}
		}
		
		
#ifdef ARITH_USE_CACHELINES
		__dcbf(dstPtr, 0);
#endif	
	
		srcBase += srcRowBytes;
		dstBase += dstRowBytes;
	}
}

template <class variant> void  
VectorHostBlit16to32(NQDDrawVars  *drawVars, Rect &origDstRect)
{	
	const int srcPixelsToBytes = 2;
	const int dstPixelsToBytes = 4;
	
	Rect srcRect, dstRect;
		
	srcRect = origDstRect;
	FastOffsetRect(srcRect, 
			-drawVars->dstRect.left + drawVars->srcRect.left - drawVars->srcPixMap.bounds.left, 
			-drawVars->dstRect.top + drawVars->srcRect.top - drawVars->srcPixMap.bounds.top);
	
	dstRect = origDstRect;
	FastOffsetRect(dstRect, -drawVars->dstPixMap.bounds.left, -drawVars->dstPixMap.bounds.top);

	UInt32 srcBase = (UInt32)drawVars->srcPixMap.baseAddr;
	SInt32 srcRowBytes = drawVars->srcPixMap.rowBytes;
	UInt32 dstBase = (UInt32)drawVars->dstPixMap.baseAddr;
	SInt32 dstRowBytes = drawVars->dstPixMap.rowBytes;
	
#ifdef ARITH_USE_CACHELINES
	// Move the destination address into cacheable space.
	dstBase = (dstBase & 0xF3FFFFFF) | 0x06000000;
#endif
	
	// We'd better not be called with anything but the correct mode and no colorization.
//	UInt32 mode = drawVars->mode & 0x0000003f,
//	UInt32 foreColor = drawVars->foreColor,
//	UInt32 backColor = drawVars->backColor);
	
	// We don't currently do indexed modes.
//	UInt32 *scaleTable = drawVars->scaleTable,
		
	SInt32  height, width;

  	height = dstRect.bottom - dstRect.top;
  	width = dstRect.right - dstRect.left;

	if (srcRowBytes < 0) 
	{
		srcRowBytes = -srcRowBytes;
		dstRowBytes = -dstRowBytes;
	}
	
	// Figure out the addresses of the top-left corner of both source and dest
	srcBase += (srcRect.left * srcPixelsToBytes) + (srcRect.top * srcRowBytes);
	dstBase += (dstRect.left * dstPixelsToBytes) + (dstRect.top * dstRowBytes);
		
	UInt32 dstByteWidth = width * dstPixelsToBytes;
	vector unsigned char scratch[variant::scratchVectorCount];
	
	variant::setup(drawVars, scratch);

	vector bool char zero, one;
	// "zero" is all zero bits
	zero = (vector bool char)vec_splat_u8(0);
	
	// "one" is all one bits
	one = (vector bool char)vec_splat_s8(-1);
		

	while(height-- > 0)
	{
		vector unsigned char src;
		vector unsigned char dst;
		vector unsigned char src1, src2, work, srcPerm;
		vector bool char mask;
		
		unsigned char *srcPtr = (unsigned char*)srcBase;
		unsigned char *dstPtr = (unsigned char*)dstBase;
		
#ifdef ARITH_USE_CACHELINES
		__dcbt(dstPtr, 0);
#endif

		// Set up mask as firstMask for this line
		mask = vec_perm(zero, one, vec_lvsr(0, dstPtr));
		
		SInt32 shiftLeftBytes = (((UInt32)srcPtr) & 0xF) - (((UInt32)dstPtr >> 1) & 0x7);
		
		if(shiftLeftBytes > 15)
		{
			srcPtr += 16;
		}
		else if(shiftLeftBytes < -15)
		{
			srcPtr -= 16;
		}
		
		if( shiftLeftBytes <= 0 )
		{
			// The source will need to be shifted right to align with the dest
			srcPerm = vec_lvsr(-shiftLeftBytes, (unsigned char *)0);
			
			// The initial contents of src2 don't matter.  Assign something so that the compiler doesn't complain.
			src2 = (vector unsigned char)zero;
		}
		else
		{
			// The source will need to be shifted left to align with the dest
			srcPerm = vec_lvsl(shiftLeftBytes, (unsigned char *)0);

			// Load the first part of the source
			src2 = vec_ld(0, (vector unsigned char*)srcPtr);
			srcPtr += 16;
		}
		
		unsigned char * dstEnd = dstPtr + dstByteWidth;
		
		// Change dstPtr to point at the beginning of the vector.
		dstPtr = (unsigned char *)((UInt32)dstPtr & ~0x0F);
		
		UInt32 count = (0xF + dstEnd - dstPtr) >> 4;
		while(count-- != 0)
		{
			// Load the next part of the source;
			src1 = src2;
			src2 = vec_ld(0, (vector unsigned char*)srcPtr);
			srcPtr += 16;
			
			// Permute the source to align with the dest
			src = vec_perm(src1, src2, srcPerm);

			// Load the destination
			dst = vec_ld(0, (vector unsigned char*)dstPtr);
			
			// Perform the operation
			work = variant::op(dst, Vec16to32h(src), scratch);
			
			// Create the end mask, if necessary
			mask = vec_and(mask, vec_perm(one, zero, vec_lvsl( 16 - minBranchless(dstEnd - dstPtr, 16L) , (unsigned char *)0)));
			
			// Apply the mask for this part of the destination
			work = vec_sel(dst, work, mask);
			
			// Write back the result
			vec_st(work, 0, (vector unsigned char*)dstPtr);
			
			dstPtr += 16;

#ifdef ARITH_USE_CACHELINES
			__dcbt(dstPtr, 0);
			__dcbf(dstPtr, -32);
#endif
			
			mask = one;

			// Bail out if we're done.
			if(count-- == 0)
				break;
			
			// Load the destination
			dst = vec_ld(0, (vector unsigned char*)dstPtr);
			
			// Perform the operation
			work = variant::op(dst, Vec16to32l(src), scratch);
			
			// Create the end mask, if necessary
			mask = vec_and(mask, vec_perm(one, zero, vec_lvsl( 16 - minBranchless(dstEnd - dstPtr, 16L) , (unsigned char *)0)));
			
			// Apply the mask for this part of the destination
			work = vec_sel(dst, work, mask);
			
			// Write back the result
			vec_st(work, 0, (vector unsigned char*)dstPtr);
			
			dstPtr += 16;

#ifdef ARITH_USE_CACHELINES
			__dcbt(dstPtr, 0);
			__dcbf(dstPtr, -32);
#endif

			mask = one;
		}
		
		
#ifdef ARITH_USE_CACHELINES
		__dcbf(dstPtr, 0);
#endif
		
		srcBase += srcRowBytes;
		dstBase += dstRowBytes;
	}
}


#pragma mark -
#pragma mark ¥ Scratch area
//#pragma mark -

#if 0

#pragma mark
struct VecBlend
{
	/* This assumes that opcolor is different from the others -- that it
		has the full 16 bits per channel.
	*/
	static vector unsigned char op(
			vector unsigned char dst,
			vector unsigned char src,
			vector unsigned char opColor)
	{
		vector unsigned char zero = vec_splat_u8(0);
		
		vector signed short srcHigh = (vector signed short)vec_mergeh(zero, src);
		vector signed short dstHigh = (vector signed short)vec_mergeh(zero, dst);
		vector signed short srcLow = (vector signed short)vec_mergel(zero, src);
		vector signed short dstLow = (vector signed short)vec_mergel(zero, dst);
		
		srcHigh = vec_sub(srcHigh, dstHigh);
		srcLow = vec_sub(srcLow, dstLow);
		
		srcHigh = vec_add(dstHigh, vec_pack(vec_mule(
		
		return(vec_packsu(srcHigh, srcLow));
	}
};

/*
	This version isn't quite right...
struct VecBlend
{
	static vector unsigned char op(
			vector unsigned char dst,
			vector unsigned char src,
			vector unsigned char opColor)
	{
		vector unsigned char negopColor = vec_sub(vec_splat_u8(-1), opColor);
		
		return(vec_perm( 
				(vector unsigned char)vec_add(vec_mule(src, opColor), vec_mule(dst, negopColor)),
				(vector unsigned char)vec_add(vec_mulo(src, opColor), vec_mulo(dst, negopColor)),
				(vector unsigned char)(0, 16, 2, 18, 4, 20, 6, 22, 8, 24, 10, 26, 12, 28, 14, 30)));
	}
};
*/


void testall(int size,
			vector unsigned char *src,
			vector unsigned char *dst,
			vector unsigned char opColor);

void testme(int size,
			vector unsigned char *src,
			vector unsigned char *dst,
			vector unsigned char opColor);


template <class variant>
void testme(int size,
			vector unsigned char *src,
			vector unsigned char *dst,
			vector unsigned char opColor)
{
	for(int i = 0; i < size; i++)
		dst[i] = variant::op(src[i], dst[i], opColor);
}


void testall(int size,
			vector unsigned char *src,
			vector unsigned char *dst,
			vector unsigned char opColor)
{
	testme<VecAddOver>(size, src, dst, opColor);
	testme<VecSubOver>(size, src, dst, opColor);
	testme<VecAdMin>(size, src, dst, opColor);
	testme<VecAdMax>(size, src, dst, opColor);
	testme<VecAddPin>(size, src, dst, opColor);
	testme<VecSubPin>(size, src, dst, opColor);
	testme<VecBlend>(size, src, dst, opColor);
	testme<VecBlendHalf>(size, src, dst, opColor);
}

#endif
