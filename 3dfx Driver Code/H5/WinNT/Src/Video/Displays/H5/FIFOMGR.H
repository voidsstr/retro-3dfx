/*
** Copyright (c) 1997-98, 3Dfx Interactive, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of 3Dfx Interactive, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
** rights reserved under the Copyright Laws of the United States.
**
*/
#ifndef __FIFOMGR_H__
#define __FIFOMGR_H__

/* ************************************************************

   FifoMgr.h contains the cmdfifo macros that manage the fullness
   and the writes/reads to the command fifo.

   ************************************************************  */

#ifdef USE_D3D_CODE
#define CMDFIFO_ROOMTOEND \
  (ULONG)(((ULONG)ppdev->fifoData.fifoEnd - H3_FIFO_END_ADJUST) - (ULONG)ppdev->fifoData.fifoPtr)
#endif

#ifdef CSIM

static __inline void halStore32f(volatile void *addr, float data)
{
  FxU32 d;

  *(float *)&d = data;
  halStore32(addr,d);
}

#define SETDW(hwRegister,data)    halStore32((void *)&(hwRegister),(data))
#define SETFDW(hwRegister,data)   halStore32f((void *)&(hwRegister),(data))
#undef GET
#define GET(hwPtr)                halLoad32((void *)&(hwPtr))

#else

#define SETDW(hwRegister,data)    hwRegister = (data); H3PRINTF((ppdev, "  SETDW  %s = %08lX\r\n", #hwRegister, (data)))
#define SETFDW(hwRegister,data)   *(float *)&(hwRegister) = (data); H3PRINTF((ppdev, "  SETFDW  %s = %08lX\r\n", #hwRegister, (data)))
#undef GET
#define GET(hwPtr)                hwPtr

#endif

//---------------------------------------------------------------
// DirectX
//---------------------------------------------------------------

#if defined(DEBUGFIFO) && defined(DBG) && defined(H3_FIFO)
  #define FD_INIT_FIFO_COUNT(n) _FF(fifoDwordCount) = n
#endif

  #define PH1_SIZE  1
  #define PH3_SIZE  1
  #define PH4_SIZE  1
  #define PH5_SIZE  2

  #define CMDFIFO_BUILD_PK1( n, inc, regName, chip )\
    ( ((n) << 16) | ((inc) << 15) \
    | ((offsetof(SstRegs, regName) / 4) << 3) | SSTCP_PKT1 )

  #define CMDFIFO_BUILD_2DPK1( n, inc, regName, chip )\
    ( ((n) << 16) | ((inc) << 15)  | SSTCP_PKT1_2D\
    | ( (offsetof(SstGRegs, regName) / 4)  << 3) | SSTCP_PKT1 )

  #define CMDFIFO_BUILD_IOPK1( n, inc, regName, chip )\
    ( ((n) << 16) | ((inc) << 15) \
    | ((offsetof(SstIORegs, regName) / 4) << 3) | SSTCP_PKT1 )

  #define CMDFIFO_BUILD_PK1R( n, inc, reg, chip )\
    ( ((n) << 16) | ((inc) << 15) | (reg << 3) | SSTCP_PKT1 )

  #define CMDFIFO_BUILD_PK2( mask)\
    ( (mask) | SSTCP_PKT2 )

  #define CMDFIFO_BUILD_PK3( cmd, noVert, param, packed )\
    ( ((packed) << 28) | ((param & 0xF0000) << 6)\
    | ((param & 0xFF) << 10) | (noVert << 6) | (cmd << 3) | SSTCP_PKT3 )

  #define CMDFIFO_BUILD_PK4( mask, regName, chip )\
    ( ((mask) << SSTCP_PKT4_MASK_SHIFT) | (chip << 11)\
    | ((offsetof(SstRegs, regName) / 4) << 3) | SSTCP_PKT4 )

  #define CMDFIFO_BUILD_PK5( noWords, byteEnableW2, byteEnableWN, memPort )\
    ( (noWords << SSTCP_PKT5_NWORDS_SHIFT) | (byteEnableW2 << SSTCP_PKT5_BYTEN_W2_SHIFT) \
    | (byteEnableWN << SSTCP_PKT5_BYTEN_WN_SHIFT) | (memPort) | SSTCP_PKT5 )


#ifdef CMDFIFO

// IGX-NVH 01.07.00
#if 1
#define CMDFIFO_CHECKBUMP(n)                              \
      pfifoData->ulCmdFifoBump += (n);                    \
      if (pfifoData->ulCmdFifoBump >= 0xffff)             \
      {                                                   \
        P6_FENCE;                                         \
        pfifoData->ulCmdFifoBump = n;                     \
      }
#else
#define CMDFIFO_CHECKBUMP(n)
#endif
// END_IGX_NVH
  
#ifdef USE_D3D_CODE
  #define CMDFIFO_CHECKROOM(hwPtr,n) \
  { \
    ppdev->fifoData.fifoPtr = hwPtr; \
    ppdev->fifoData.roomToEnd = CMDFIFO_ROOMTOEND; \
    ASSERTDD((ULONG)(ppdev->fifoData.roomToEnd) <= CMDFIFO_ROOMTOEND, __FILELINE__ "roomToEnd wrong"); \
    H3MakeRoom((ppdev),&(hwPtr),(n)); \
    H3PRINTF((ppdev, "  CmdFifo_CheckRoom, hwPtr = %08lX, count = %ld\r\n", hwPtr, n)); \
  }
#else
#if defined(DEBUGFIFO) && defined(DBG) && defined(H3_FIFO)
  #define CMDFIFO_CHECKROOM(hwPtr,n) \
  { \
    ppdev->fifoData.fifoPtr = hwPtr; \
    H3MakeRoom((ppdev),&(hwPtr),n); \
    FD_INIT_FIFO_COUNT(n); \
  }
#else
  #define CMDFIFO_CHECKROOM(hwPtr,n) \
  { \
    ppdev->fifoData.fifoPtr = hwPtr; \
    H3MakeRoom((ppdev),&(hwPtr),n); \
    H3PRINTF((ppdev, "  CmdFifo_CheckRoom, hwPtr = %08lX, count = %ld\r\n", hwPtr, n)); \
  }
#endif
#endif // USE_D3D_CODE

#if defined(DEBUGFIFO) && defined(DBG) && defined(H3_FIFO)
  #define BUMP( n ) \
  { \
    hwPtr += hwIndex; \
    hwIndex = 0; \
  }
#else
  #define BUMP(n)
#endif

#if defined(DEBUGFIFO) && defined(DBG) && defined(H3_FIFO)
  #define LOAD_CMDFIFO_RDPTR()   GET2( ((CmdFifo *)(ppdev->pjFifoBase))->readPtrL )

  #if defined(CRASH)
    #define BREAK_STALL()
    #define POLL_STALL() PollStall();
  #else
    #define BREAK_STALL()
    #define POLL_STALL()
  #endif

//  #ifdef DEBUGFIFO

    // fifo debugging
    #define FD_DEC_FIFO_COUNT     _FF(fifoDwordCount) -= 1
    extern  void mySetCF(PDEV *ppdev, FxU32 *hwPtr, FxU32 hwIndex, FxU32 data);
    extern  void mySetPH(PDEV *ppdev, FxU32 *hwPtr, FxU32 hwIndex, FxU32 data);
    extern  void mySetDW(FxU32  hwPtr, FxU32 data);

    __inline void mySetFCF(PDEV *ppdev, FxU32 *hwPtr, FxU32 hwIndex, float data)
    {
      mySetCF(ppdev, hwPtr, hwIndex, *(FxU32 *)&data) ;
    }

//    #define SETDW(hwRegister, data)  mySetDW((FxU32)&(hwRegister), data)

  #define SETCF(hwPtr, data)\
  {\
    mySetCF(ppdev, hwPtr, hwIndex, (data));\
    hwIndex += 1;\
    FD_DEC_FIFO_COUNT;\
  }

  #define SETFCF(hwPtr, data)\
  {\
    mySetFCF(ppdev, hwPtr, hwIndex, (data));\
    hwIndex += 1;\
    FD_DEC_FIFO_COUNT;\
  }
#endif  // DEBUGFIFO && DBG && H3_FIFO

   // copy cmdfifo write pointer to local variable
#ifdef USE_D3D_CODE
  #define CMDFIFO_PROLOG(hwPtr) \
    ULONG *hwPtr = ppdev->fifoData.fifoPtr; \
    H3PRINTF((ppdev, "  CmdFifo_Prolog hwPtr = %08lX\r\n", hwPtr)); \
    ASSERTDD((ULONG)(ppdev->fifoData.roomToEnd) <= CMDFIFO_ROOMTOEND, __FILELINE__ "roomToEnd wrong")
#else
#if defined(DEBUGFIFO) && defined(DBG) && defined(H3_FIFO)
  #define CMDFIFO_PROLOG( hwPtr )\
    ULONG *hwPtr = ppdev->fifoData.fifoPtr; \
    FxU32 hwIndex = 0
#else
  #define CMDFIFO_PROLOG(hwPtr) \
    ULONG *hwPtr = ppdev->fifoData.fifoPtr; \
    H3PRINTF((ppdev, "  CmdFifo_Prolog hwPtr = %08lX\r\n", hwPtr))
#endif
#endif // USE_D3D_CODE


#if (_WIN32_WINNT >= 0x0500) && defined (AGP_CMDFIFO)

  // update global variables with internal count
  // reset internal count
#ifdef USE_D3D_CODE
  #define CMDFIFO_EPILOG(hwPtr) \
    ppdev->fifoData.fifoPtr = hwPtr; \
    ppdev->fifoData.roomToEnd = CMDFIFO_ROOMTOEND; \
    if ( ppdev->doAGPFifo == TRUE ) \
        NTAGPEPILOG(ppdev, (DWORD) hwPtr); \
    ASSERTDD((ULONG)(ppdev->fifoData.roomToEnd) <= CMDFIFO_ROOMTOEND, __FILELINE__ "roomToEnd wrong"); \
    H3PRINTF((ppdev, "  CmdFifo_Epilog hwPtr = %08lX\r\n", hwPtr))
#else
#if defined(DEBUGFIFO) && defined(DBG) && defined(H3_FIFO)
#if 1
  #define CMDFIFO_EPILOG( hwPtr )\
    hwPtr += hwIndex;\
    hwIndex = 0;\
    ppdev->fifoData.fifoPtr = hwPtr \
    if ( ppdev->doAGPFifo == TRUE ) \
        NTAGPEPILOG(ppdev, (DWORD) hwPtr)
#else
  #define CMDFIFO_EPILOG( hwPtr )\
    _FF(cmdFifoSpace) -= hwIndex;\
    hwPtr += hwIndex;\
    hwIndex = 0;\
    ppdev->fifoData.fifoPtr = hwPtr; \
    if ( ppdev->doAGPFifo == TRUE ) \
        NTAGPEPILOG(ppdev, (DWORD) hwPtr)
#endif
#else
  #define CMDFIFO_EPILOG(hwPtr) \
    ppdev->fifoData.fifoPtr = hwPtr; \
    if ( ppdev->doAGPFifo == TRUE ) \
        NTAGPEPILOG(ppdev, (DWORD) hwPtr); \
    H3PRINTF((ppdev, "  CmdFifo_Epilog hwPtr = %08lX\r\n", hwPtr))
#endif
#endif // USE_D3D_CODE

#else  // AGP_CMDFIFO

  // update global variables with internal count
  // reset internal count
#ifdef USE_D3D_CODE
  #define CMDFIFO_EPILOG(hwPtr) \
    ppdev->fifoData.fifoPtr = hwPtr; \
    ppdev->fifoData.roomToEnd = CMDFIFO_ROOMTOEND; \
    ASSERTDD((ULONG)(ppdev->fifoData.roomToEnd) <= CMDFIFO_ROOMTOEND, __FILELINE__ "roomToEnd wrong"); \
    H3PRINTF((ppdev, "  CmdFifo_Epilog hwPtr = %08lX\r\n", hwPtr))
#else
#if defined(DEBUGFIFO) && defined(DBG) && defined(H3_FIFO)
#if 1
  #define CMDFIFO_EPILOG( hwPtr )\
    hwPtr += hwIndex;\
    hwIndex = 0;\
    ppdev->fifoData.fifoPtr = hwPtr
#else
  #define CMDFIFO_EPILOG( hwPtr )\
    _FF(cmdFifoSpace) -= hwIndex;\
    hwPtr += hwIndex;\
    hwIndex = 0;\
    ppdev->fifoData.fifoPtr = hwPtr
#endif
#else
  #define CMDFIFO_EPILOG(hwPtr) \
    ppdev->fifoData.fifoPtr = hwPtr; \
    H3PRINTF((ppdev, "  CmdFifo_Epilog hwPtr = %08lX\r\n", hwPtr))
#endif
#endif // USE_D3D_CODE

#endif // AGP_CMDFIFO


#ifdef USE_D3D_CODE
  #define CMDFIFO_SAVE(hwPtr) \
    ppdev->fifoData.fifoPtr = hwPtr; \
    ppdev->fifoData.roomToEnd = CMDFIFO_ROOMTOEND; \
    H3PRINTF((ppdev, "  CmdFifo_Save hwPtr = %08lX\r\n", hwPtr))
#else
#if defined(DEBUGFIFO) && defined(DBG) && defined(H3_FIFO)
  #define CMDFIFO_SAVE(hwPtr)\
    _FF(cmdFifoSpace) -= hwIndex;\
    hwPtr += hwIndex;\
    ppdev->fifoData.fifoPtr = hwPtr;
#else
  #define CMDFIFO_SAVE(hwPtr) \
    ppdev->fifoData.fifoPtr = hwPtr; \
    H3PRINTF((ppdev, "  CmdFifo_Save hwPtr = %08lX\r\n", hwPtr))
#endif
#endif // USE_D3D_CODE


#if defined(DEBUGFIFO) && defined(DBG) && defined(H3_FIFO)
  #define CMDFIFO_RELOAD(hwPtr)\
    hwPtr = ppdev->fifoData.fifoPtr;\
    hwIndex = 0;
#else
  #define CMDFIFO_RELOAD(hwPtr) \
    hwPtr = ppdev->fifoData.fifoPtr; \
    H3PRINTF((ppdev, "  CmdFifo_Reload hwPtr = %08lX\r\n", hwPtr))
#endif


#ifdef CSIM

#define SETPH(hwPtr,data)               halStore32((void *)(hwPtr++),(data))
#define SETPD(hwPtr,hwRegister,data)    halStore32((void *)(hwPtr++),(data))
#define SETFPD(hwPtr,hwRegister,data)   halStore32f((void *)(hwPtr++),(data))

#else // ndef CSIM

#if defined(DEBUGFIFO) && defined(DBG) && defined(H3_FIFO)
  #define SETPH( hwPtr, data )\
  {\
    mySetPH(ppdev, hwPtr, hwIndex, data);\
    hwIndex += 1;\
    FD_DEC_FIFO_COUNT;\
  }
#else
  #define SETPH(hwPtr,data) \
  { \
    H3PRINTF((ppdev, "  SETPH  %08lX = %08lX\r\n", hwPtr, (data))); \
    *hwPtr++ = (data); \
  }
#endif

#if defined(DEBUGFIFO) && defined(DBG) && defined(H3_FIFO)
  #define SETPD( hwPtr, hwRegister, data )\
  {\
    SETCF( hwPtr, data );\
  }
#else
  #define SETPD(hwPtr,hwRegister,data) \
  { \
    H3PRINTF((ppdev, "  SETPD  %08lX = %08lX (%s)\r\n", hwPtr, (data), #hwRegister)); \
    *hwPtr++ = (data); \
  }
#endif

#if defined(DEBUGFIFO) && defined(DBG) && defined(H3_FIFO)
  #define SETFPD( hwPtr, hwRegister, data )\
  {\
    SETCF( hwPtr, data );\
  }
#else
  #define SETFPD(hwPtr,hwRegister,data) \
  { \
    H3PRINTF((ppdev, "  SETFPD  %08lX = %f (%s)\r\n", hwPtr, (data), #hwRegister)); \
    *(float *)hwPtr++ = (data); \
  }
#endif

#endif // ndef CSIM

#else // ndef CMDFIFO

#define CMDFIFO_CHECKROOM(hwPtr,n)
#define BUMP(n)
#define CMDFIFO_PROLOG(hwPtr)
#define CMDFIFO_EPILOG(hwPtr)
#define CMDFIFO_SAVE(hwPtr)
#define CMDFIFO_RELOAD(hwPtr)
#define SETPH(hwPtr,data)

#define SETPD(hwPtr,hwRegister,data)    SETDW((hwRegister),(data))
#define SETFPD(hwPtr,hwRegister,data)   SETFDW((hwRegister),(data))

#endif  // CMDFIFO

#endif //__FIFOMGR_H__
