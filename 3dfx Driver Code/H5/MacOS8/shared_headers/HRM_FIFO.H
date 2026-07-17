#ifndef __HRM_FIFO_H__
#define __HRM_FIFO_H__ 1


#include "3dfx.h"
#include "HDWR_Res_Mgr.h"

#ifdef __cplusplus
extern "C" {
#endif

/* This file describes the FIFO extension stuff. It is "public" to internal
   3Dfx software, but "private" to third parties. */
   
typedef struct hrmFifoInfo_s
{
	FxU32 exclusiveMode;
	FxU32 cacheActive;
	
  /* Basic command fifo characteristics. These should be
   * considered logically const after their initialization.
   */

  FxU32* fifoStart;    /* Virtual address of start of fifo */
  FxU32* fifoEnd;      /* Virtual address of fba fifo */
  FxU32  fifoOffset;   /* Offset from hw base to fifo start */
  FxU32  fifoSize;     /* Size in bytes of the fifo */
  FxU32  fifoJmpHdr[2];/* Type0 packet for jmp to fifo start
                          only first DWORD is used for memory
                          fifo--both are used for AGP FIFO
                        */
  
  FxU32* fifoPtr;      /* Current write pointer into fifo */
  FxU32  fifoRead;     /* Last known hw read ptr. 
                        * This is the sli master, if enabled.
                        */

  /* Fifo checking information. In units of usuable bytes until
   * the appropriate condition.
   */
  FxU32  fifoRoom;     /* Space until next fifo check */

  /* These are only valid for the fullscreen case */
  FxI32  roomToReadPtr;/* Bytes until last known hw ptr */
  FxI32  roomToEnd;    /* # of bytes until last usable address before fifoEnd */

  /*FxBool lfbLockCount; /* Have we done an lfb lock? Count of the locks. */
  /*FxBool
  /*  autoBump;                 /* Are we auto bumping (aka hole counting?) */
  FxU32
    *lastBump,                /* Last ptr where we bumped. */
    *bumpPos;                 /* Nex place to bump */
  FxU32
    bumpSize;                 /* # of DWORDS per bump */

  void  	(*setLfb)(volatile FxU32 *d, FxU32 s);
  void		(*setLfbHost)(volatile FxU32 *d, FxU32 s);
} hrmFifoInfo;

/* Given to k3DfxExecuteFifo */
typedef struct hrmExecuteFifoRequest_s
{
  /* NB: These fields must be first to support old drivers that do not
   * support multiple fifo types.  
   */
  FxU32
    fifoPtr,        /* Linear address of command stream to execute */
    fifoSize,       /* Size of command stream (DWORDS) */
    statePtr,       /* Linear address of persistent state buffer */
    stateSize;      /* Size of persistent state buffer (DWORDS) */
  
  FxU32
    fifoType,       /* One of HWCEXT_FIFO_XXXX */
    contextID;
    
  /* These are only necessary if fifoType != HWCEXT_FIFO_HOST */
  FxU32
    fifoOffset,     /* HW relative address of command stream */
    stateOffset,    /* HW relative address of state buffer */
    sentinalOffset, /* HW relative address of buffer to write serialNumber */
    serialNumber;   /* Client specified value to write to indicate that the
                       * current command stream has been committed by the hw.
                       */
} hrmExecuteFifoRequest_t;

typedef void (*hrmFifoMakeRoomPtr)(hrmBoard_t *board, const FxU32 blockSize);
typedef FxU32 (*hrmHwFifoPtrPtr)(hrmBoard_t *board);
typedef FxU32 *(*hrmFifoWrapPtr)(hrmBoard_t *board);
typedef hrmFifoInfo * (*hrmGetTargetFifoInfoPtr)(hrmBoard_t *board);
typedef short (*hrmFifoUpdatePtr)( hrmFifoUpdate_t *fifoUpdate );
typedef short (*hrmEnableFifo2DPtr)( short inRefNum );
typedef short (*hrmDisableFifo2DPtr)( short inRefNum );
typedef unsigned long (*hrmAllocWinContextPtr)(hrmBoard_t *board);
typedef void (*hrmFreeWinContextPtr)(hrmBoard_t *board, unsigned long contextID);
typedef short (*hrmExecuteWinFifoPtr)(hrmBoard_t *board, hrmExecuteFifoRequest_t *fifoReq);
typedef short (*hrmGetAGPInfoPtr)(hrmBoard_t *board, FxU32 *agpLAddr, FxU32 *agpPAddr, FxU32 *agpSize);

void hrmInitFifo(hrmBoard_t *board, FxBool doGetFifoInfo);
void hrmDisableFifo(hrmBoard_t *board);

// Fifo 
hrmFifoInfo *hrmGetTargetFifoInfo(hrmBoard_t *board);
FxU32 hrmHwFifoPtr(hrmBoard_t *board);
FxU32 *hrmFifoWrap(hrmBoard_t *board);
short hrmFifoUpdate( hrmFifoUpdate_t *fifoUpdate );
short hrmEnableFifo2D( short inRefNum );
short hrmDisableFifo2D( short inRefNum );
unsigned long hrmAllocWinContext(hrmBoard_t *board);
void hrmFreeWinContext(hrmBoard_t *board, unsigned long contextID);
short hrmExecuteWinFifo(hrmBoard_t *board, hrmExecuteFifoRequest_t *fifoReq);
short hrmGetAGPInfo(hrmBoard_t *board, FxU32 *agpLAddr, FxU32 *agpPAddr, FxU32 *agpSize);

#ifdef __cplusplus
}
#endif

#endif
