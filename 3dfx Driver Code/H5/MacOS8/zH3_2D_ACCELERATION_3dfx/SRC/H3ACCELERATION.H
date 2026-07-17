#ifndef H3_ACCELERATION_H
#define H3_ACCELERATION_H

#include <GraphicsPrivHwc.h>
#include <hdwr_res_mgr.h>
#include <hrm_fifo.h>
#include <hrm_mode.h>
#include <cstddef>
#include <DCon.h>

#ifdef __cplusplus
extern "C" {
#endif

extern hrmHwFifoPtrPtr         _hrmHwFifoPtr;
extern hrmFifoWrapPtr          _hrmFifoWrap;
extern hrmGetTargetFifoInfoPtr _hrmGetTargetFifoInfo;

/* Mask values for CMDFIFO Packet Type 2 */
#define MASK_clip0min			(1UL << 3)
#define MASK_clip0max			(1UL << 4)
#define MASK_dstBaseAddr		(1UL << 5)
#define MASK_dstFormat			(1UL << 6)
#define MASK_srcColorkeyMin		(1UL << 7)
#define MASK_srcColorkeyMax		(1UL << 8)
#define MASK_dstColorkeyMin		(1UL << 9)
#define MASK_dstColorkeyMax		(1UL << 10)
#define MASK_bresError0			(1UL << 11)
#define MASK_bresError1			(1UL << 12)
#define MASK_rop				(1UL << 13)
#define MASK_srcBaseAddr		(1UL << 14)
#define MASK_commandEx			(1UL << 15)
#define MASK_lineStipple		(1UL << 16)
#define MASK_lineStyle			(1UL << 17)
#define MASK_pattern0alias		(1UL << 18)
#define MASK_pattern1alias		(1UL << 19)
#define MASK_clip1min			(1UL << 20)
#define MASK_clip1max			(1UL << 21)
#define MASK_srcFormat			(1UL << 22)
#define MASK_srcSize			(1UL << 23)
#define MASK_srcXY				(1UL << 24)
#define MASK_colorBack			(1UL << 25)
#define MASK_colorFore			(1UL << 26)
#define MASK_dstSize			(1UL << 27)
#define MASK_dstXY				(1UL << 28)
#define MASK_command			(1UL << 29)

#define PACKET_BLITS 1

#if PACKET_BLITS

#define SSTCP_PKT4_NOPCMD  ((0x48 << SSTCP_REGBASE_SHIFT))

#define CMDFIFO_BUILD_PK4( mask, regName, chip )\
    ( ((mask) << SSTCP_PKT4_MASK_SHIFT) | (chip << 11)\
    | (regName) | SSTCP_PKT4 )

#define WAXBUG_3DNOPFIX \
do { \
    MAKE_ROOM(2); \
  	SET_FIFO_LFB(*fifo->fifoPtr++,CMDFIFO_BUILD_PK4(1, SSTCP_PKT4_NOPCMD, 0)); \
  	SET_FIFO_LFB(*fifo->fifoPtr++,0); \
} while(0)

#define FIXW 8
#define FIXH 8
#define FIXOP ||

#define BLITPAT_START(numEntries) \
do { \
    MAKE_ROOM((numEntries) + 1); \
    SET_FIFO_LFB(*fifo->fifoPtr++,(SSTCP_PKT1_2D | SSTCP_PKT1 | SSTCP_INC | \
                      (0x40UL << SSTCP_REGBASE_SHIFT) | \
		 			  ((numEntries) << SSTCP_PKT1_NWORDS_SHIFT))); \
} while(0)

#define REG2D_BEGIN(nWords,mask) \
  { \
      FxU32 *fifoPtr; \
	    MAKE_ROOM(nWords+1); \
	    fifoPtr = fifo->fifoPtr; \
	    SET_FIFO_LFB(*fifoPtr++, 2 | (mask));

#define REG2D_DATA(reg,data) \
	SET_FIFO_LFB(*fifoPtr++, data)

#define REG2D_END \
      fifo->fifoPtr = fifoPtr; \
   }
   	
#define BLIT_START \
do { \
    *h3packetData++ = 2 | MASK_command; \
    *h3packetData++ = SSTG_NOP | SSTG_GO; \
	h3packetHeader = h3packetData; \
	*h3packetData++ = 2 | MASK_command; \
	cmdFlags = 0; \
} while(0)
#define BLIT_DATA(reg, value) \
do { \
	*h3packetData++ = (value); *h3packetHeader |= MASK_##reg; \
} while(0)
#define BLIT_CMD(value) \
do { \
	*h3packetData++ = (value); \
} while(0)

#else /* !PACKET_BLITS */

#define BLIT_START \
do { \
	cmdFlags = 0; \
} while(0)
#define BLIT_DATA(reg, value) \
do { \
	HWC_WAX_STORE(bInfo->regInfo, reg, (value)); \
} while(0) 
#define BLIT_CMD(value) \
do { \
	__eieio(); HWC_WAX_STORE(bInfo->regInfo, command, (value)); __eieio();\
} while(0)

#endif /* !PACKET_BLITS */

/* #define MAKEROOM(nWords)  H3FifoMakeRoom(fifo,bInfo,nWords) */

/* Function prototypes */
extern Int32  GetAcceleratedBitBlitProc (NQDDrawVars  *drawVars);
extern Int32  GetAcceleratedPatBlitProc (NQDDrawVars  *drawVars);
extern Int32  GetAcceleratedRgnBlitProc (NQDDrawVars  *drawVars);
extern Int32  GetAcceleratedPatRgnBlitProc (NQDDrawVars  *drawVars);
extern Int32  GetAcceleratedScaleBlitProc (NQDDrawVars  *drawVars);
extern Int32  GetAcceleratedLineBlitProc (NQDDrawVars  *drawVars);
extern Int32  GetAcceleratedSlabBlitProc (NQDDrawVars  *drawVars);

extern OSErr InitializeAccelerationHardware(void);
extern Int32  H3BlitFinish(long refCon);
extern void H3BlitFlush(hwcBoardInfo *bInfo);

extern void RgnBlitAcceleratedProc(NQDDrawVars *inDrawVars);

extern void  AcceleratedScreenToScreenBlitLL(NQDDrawVars  *drawVars, Rect *clipRect);
extern void  AcceleratedHostToScreenBlitLL(NQDDrawVars  *drawVars, Rect *clipRect);

//extern void RgnBlitMoveForward(NQDDrawVars	*inDrawVars);
extern void (*BlitMoveRect)(NQDDrawVars *drawVars, Rect *clipRect);


extern FxU32 dead;

#define H3FIFO_SIZE 4096

extern FxU32 h3packetBuffer[H3FIFO_SIZE + 64];
extern FxU32 *h3packetData, *h3packetHeader;

struct h3Info
{
  /* Rather than share this with the HRM, we now have our own copy. */
  hwcBoardInfo boardInfo;
  /* This is for legacy code reasons, and just points to the above struct. */
  hwcBoardInfo *bInfo;
  hrmFifoInfo *fifo;
  hrmBoard_t *board;
};
typedef struct h3Info h3Info;

extern h3Info h3_info[16];

/* Do this in a much faster way now (no loops, to cross-TOC function call, etc.). */
inline h3Info *FindH3Info(void *destPtr)
{
	if(h3_info[((FxU32)destPtr) >> 28].board)
		return &h3_info[((FxU32)destPtr) >> 28];
		
	return NULL;
}

#define P6FENCE __sync()

#if GDX_SWIZZLE_HACK
#define SET_FIFO_LFB(d, s)			setLfb(&(d),s)
#define SET_FIFO_HOST(d, s) 		setLfbHost(&(d),s)
#else
#define	SET_FIFO_LFB(d, s)			SET_FIFO(d,s)
#define SET_FIFO_HOST(d, s) (d = (s))
#endif

#if FAST_DISPATCH_HACK && 0
#undef SET_FIFO_LFB
#define SET_FIFO_LFB(d, s)			fastDispatch(&(d),s,*(FxU32 *)fifo->setLfb)

static void asm fastDispatch (FxU32 *dst, FxU32 src, FxU32 func);
static void asm fastDispatch2(FxU32 *dst, FxU32 src, FxU32 func)
{
	nop
	entry static fastDispatch
	mtctr r5
	bctr
}

#endif

#if 1
/* Full byte swap */
inline void  __swizzleWrite32_8(volatile FxU32 *d, FxU32 s)
{
#if 0
	if(__hackSwizzle != __swizzleWrite32_8) {
		dprintf("Called __swizzleWrite32_8 incorrectly!\n");
		__hackSwizzle(d, s);
		return;
	}
#endif	
#if PCI_COPYBACK
	if(((FxU32)d & 31) == 0) {
		__dcbst((void *)d,-4);
		__dcbz((void *)d,0);
	}
#endif	
	__stwbrx(s,(void *)d, 0);
}

/* Swap words */
inline void  __swizzleWrite32_16(volatile FxU32 *d, FxU32 s)
{
	FxU32 temp = (s >> 16) | (s << 16);
#if 0	
	if(__hackSwizzle != __swizzleWrite32_16) {
		dprintf("Called __swizzleWrite32_16 incorrectly!\n");
		__hackSwizzle(d, s);
		return;
	}
#endif	
#if PCI_COPYBACK
	if(((FxU32)d & 31) == 0) {
		__dcbst((void *)d,-4);
		__dcbz((void *)d,0);
	}
#endif	
	*d = temp;
}

/* Swap words and bytes */
inline void  __swizzleWrite16_32(volatile FxU32 *d, FxU32 s)
{
	FxU32 temp = (s >> 16) | (s << 16);
#if 0	
	if(__hackSwizzle != __swizzleWrite32_16) {
		dprintf("Called __swizzleWrite32_16 incorrectly!\n");
		__hackSwizzle(d, s);
		return;
	}
#endif	
#if PCI_COPYBACK
	if(((FxU32)d & 31) == 0) {
		__dcbst((void *)d,-4);
		__dcbz((void *)d,0);
	}
#endif	
	__stwbrx(temp,(void *)d, 0);
}

/* Pass through */
inline void  __swizzleWrite32_32(volatile FxU32 *d, FxU32 s)
{
#if 0
	if(__hackSwizzle != __swizzleWrite32_32) {
		dprintf("Called __swizzleWrite32_32 incorrectly!\n");
		__hackSwizzle(d, s);
		return;
	}
#endif
#if PCI_COPYBACK
	if(((FxU32)d & 31) == 0) {
		__dcbst((void *)d,-4);
		__dcbz((void *)d,0);
	}
#endif	
	*d = s;
}
#endif

#if PCI_COPYBACK
#define FIFO_CACHE_FLUSH(d)  __dcbst(d,-4)
#else
#define FIFO_CACHE_FLUSH(d)
#endif

#if 0
#define FIFO_SANITY_END \
  if(fifo->fifoPtr >= fifo->fifoEnd) {\
    dprintf("whoops!: %08lx %s %d\n",fifo->fifoPtr,__FILE__,__LINE__);\
  }

#define FIFO_SANITY_ROOM \
  if((fifo->fifoPtr + fifo->fifoRoom) > fifo->fifoEnd) {\
    dprintf("arghh!: %08lx %s %d\n",fifo->fifoPtr,__FILE__,__LINE__);\
  }
#else
#define FIFO_SANITY_END
#define FIFO_SANITY_ROOM
#endif

#if PCI_BUMP_N_GRIND
#define BUMP_N_GRIND \
{ \
  FIFO_CACHE_FLUSH(fifo->fifoPtr);\
  P6FENCE;\
  HWC_CAGP_STORE(bInfo->regInfo, cmdFifo0.bump, fifo->fifoPtr - fifo->lastBump);\
  P6FENCE;\
  FIFO_SANITY_END;\
  FIFO_SANITY_ROOM;\
  fifo->lastBump = fifo->fifoPtr;\
}	
#define SET_FIFO_NOSWAP(d, s)  setFifo8((FxU32 *)&(d),s)
#else
#define BUMP_N_GRIND
#define SET_FIFO_NOSWAP(d, s)  ((d) = (s))
#endif

#include <MacTypes.h>

inline void FastOffsetRect(Rect &r, SInt32 h, SInt32 v)
{
	r.left += h;
	r.right += h;
	r.top += v;
	r.bottom += v;
}

extern pascal OSErr  InitializeGraphicsAcceleration(const CFragInitBlock *inInitBlock);
extern OSErr  GraphicsAcceleration (OSType  selector, Ptr  params);
extern pascal OSErr  TerminateGraphicsAcceleration ();


extern FxU32 H3HwFifoPtr(H3FifoInfo *fifo, hwcBoardInfo *bInfo);
extern FxU32 *H3FifoWrap(H3FifoInfo *fifo, hwcBoardInfo *bInfo);

inline void _FifoMakeRoom(hrmBoard_t *board, hrmFifoInfo *fifo, const FxU32 blockSize)
{
  FxU32 wrapAddr = 0x00UL;
  FxU32 *rdPtr = (FxU32 *)0xdeadbeef;
  
	/* This code is based on the 2D driver's FIFO management code.  It's far simpler than 
	 * what Glide uses in the fullscreen case, but probably not as efficent CPU-wise. */
	 #if 0	 
	if((fifo->fifoPtr + fifo->fifoRoom) > fifo->fifoEnd ) {
	  dprintf("what THE fuck?: %08lx %08lx %d\n",fifo->fifoPtr + fifo->fifoRoom,fifo->fifoEnd,blockSize);
	  // Fix it, so we can figure out where it's getting fucked...
		fifo->fifoRoom = fifo->fifoEnd - fifo->fifoPtr;
	  if(fifo->fifoPtr > fifo->fifoEnd) {
	    fifo->fifoRoom = 0;
	  }
	}
  #endif
  
	if(fifo->fifoRoom < blockSize) {
	
	  #if 0
	  if(fifo->fifoPtr > fifo->fifoEnd) {
	    dprintf("OH SHIT!\n");
	  }
	  #endif
		// dprintf("_FifoMakeRoom(%d) @ %08lx %08lx\n",blockSize, fifo->fifoPtr, hrmHwFifoPtr(board));
		
		/* If desired write won't fit in fifo, then wrap it */
		if((fifo->fifoPtr + blockSize) > fifo->fifoEnd)
		{
			// dprintf("wrapping fifo: %d\n",blockSize);
			rdPtr = hrmFifoWrap(board);
			#if 0
			if((fifo->fifoPtr + 0) > fifo->fifoEnd) {
			  dprintf("WHAT the fuck?: %08lx %08lx\n",fifo->fifoPtr + 0,fifo->fifoEnd);
			}
			#endif
			// dprintf("fifo wrapped\n");
		} 
		#if 0
		  else {
		  dprintf("I think it'll fit..\n");
		}
		#endif
		fifo->fifoRoom = fifo->fifoEnd - fifo->fifoPtr;
		while(fifo->fifoRoom < blockSize)
		{
			rdPtr = (FxU32 *)hrmHwFifoPtr(board);
			if(rdPtr <= fifo->fifoPtr)
				fifo->fifoRoom = fifo->fifoEnd - fifo->fifoPtr;
			else
				fifo->fifoRoom = 0; //((FxU32)avengerHALData->fifoEnd - (FxU32)avengerHALData->fifoPtr);
				//avengerHALData->fifoRoom = rdPtr - avengerHALData->fifoPtr - 1;
			// dprintf("room: %d\n",fifo->fifoRoom);
		}
		#if 0
	  if((fifo->fifoPtr + fifo->fifoRoom) > fifo->fifoEnd ) {
	    dprintf("WHAT THE FUCK?: %08lx %08lx\n",fifo->fifoPtr + fifo->fifoRoom,fifo->fifoEnd);
	    // Fix it, so we can figure out where it's getting fucked...
	    fifo->fifoRoom = fifo->fifoEnd - fifo->fifoPtr;
    }
    #endif
    
	}
	#if 0
	if((fifo->fifoPtr + blockSize) > (fifo->fifoEnd + 8)) {
	  dprintf("what the FUCK?: %08lx %08lx %08lx\n",fifo->fifoPtr,fifo->fifoPtr + blockSize,fifo->fifoEnd + 8);
	} 

  if(fifo->fifoRoom > (fifo->fifoEnd - fifo->fifoStart)) {
    dprintf("Houston, we have a problem!: %08lx\n",fifo->fifoRoom);
  }

	if((fifo->fifoPtr + fifo->fifoRoom) > fifo->fifoEnd ) {
	  dprintf("what the FUCK2?: %08lx %08lx\n",fifo->fifoPtr + fifo->fifoRoom,fifo->fifoEnd);
	  // Fix it, so we can figure out where it's getting fucked...
	}
  #endif
	fifo->fifoRoom -= blockSize;

}

#define MAKE_ROOM(nWords) \
  FIFO_SANITY_ROOM; \
  FIFO_SANITY_END; \
  _FifoMakeRoom(board,fifo,nWords)

#if 1
extern void  __swizzleWrite32_8(volatile FxU32 *d, FxU32 s);
extern void  __swizzleWrite32_16(volatile FxU32 *d, FxU32 s);
extern void  __swizzleWrite16_32(volatile FxU32 *d, FxU32 s);
extern void  __swizzleWrite32_32(volatile FxU32 *d, FxU32 s);
#endif


#ifdef __cplusplus
}
#endif

#endif