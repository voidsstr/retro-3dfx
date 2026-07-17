/* $Header: fifomgr.h, 13, 10/11/00 8:53:06 PM, Brent$ */
/*
** Copyright (c) 1995-1999, 3Dfx Interactive, Inc.
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
** File Name: 	FIFOMGR.H
**
** Description: Hardware access macros for Voodoo3/Napalm.
**
** $Revision: 13$
** $Date: 10/11/00 8:53:06 PM$
**
*/

#ifndef __FIFOMGR_H__
#define __FIFOMGR_H__

/* 
   Macros:

   GDI 16-bit display driver: IS_16=1, IS_32=0, THUNK32=0
   GDI 32-bit minivdd driver: IS_16=0, IS_32=0, THUNK32=1
   DirectX drivers:           IS_16=0, IS_32=1, THUNK32=0
*/ 

#ifdef IS_16
  #define VOLATILE 
  #define INLINE __inline
  #define STDCALL
  #define DEBUG_FIX
#else // IS_16
  #define VOID void
  #define VOLATILE volatile
  #define INLINE   __inline
  #define STDCALL  __stdcall
  #include "shared.h" 
#endif // IS_16


//----------------------------------------------------------------------------------//
//                         COMMAND FIFO ACCESS MACROS
//----------------------------------------------------------------------------------//

// SETCF  - Send integer data using command fifo
// SETPH  - Send integer packet header using command fifo
// SETPD  - Send integer packet data using command fifo
// SET    - Send integer data to hardware using command fifo packet
// SETFCF - Send float data using command fifo
// SETFPD - Send float packet data using command fifo
// SETF   - Send float data to hardware using command fifo packet

#ifdef CMDFIFO

#ifndef DEBUGFIFO

#ifdef IS_16

  #define SETCF(hwPtr, data) \
          h3WRITE(NULL, ((FxU32 *)((hwPtr) + (hwIndex++ << 2))), (data))

  #undef  SETFCF  // no float transfers from 16-bit driver
  #undef  SETFPD  // no float transfers from 16-bit driver
  #undef  SETF    // no float transfers from 16-bit driver

#else // IS_16

  #define SETCF(hwPtr, data) \
          *((volatile FxU32 *)(hwPtr) + hwIndex++) = (data)

  #define SETFCF(hwPtr, data) \
          *((volatile float *)(hwPtr) + hwIndex++) = (data)

  #define SETFPD(hwPtr, hwReg, data ) \
          SETFCF((hwPtr), (data));

  #define SETF(hwPtr, hwReg, data) \
          SETFCF((hwPtr), (data));

#endif // IS_16

  #define SETPH(hwPtr, data) \
          SETCF((hwPtr), (data));

  #define SETPD(hwPtr, hwReg, data) \
          SETCF((hwPtr), (data));

  #define SET(hwPtr, hwReg, data) \
          SETCF((hwPtr), (data));

  #define SETC(hwPtr, hwReg, data) \
          SETCF((hwPtr), (data));

#endif // DEBUGFIFO

#endif // CMDFIFO

//----------------------------------------------------------------------------------//
//                         COMMAND FIFO ACCESS MACROS (debug)
//----------------------------------------------------------------------------------//

// SETCF  - Send integer data using command fifo
// SETPH  - Send integer packet header using command fifo
// SETPD  - Send integer packet data using command fifo
// SET    - Send integer data to hardware using command fifo packet
// SETFCF - Send float data using command fifo
// SETFPD - Send float packet data using command fifo
// SETF   - Send float data to hardware using command fifo packet

#ifdef CMDFIFO

#ifdef DEBUGFIFO
#define DF_INIT_FIFO_COUNT(n) _FF(fifoDwordCount) = n
#define DF_DECR_FIFO_COUNT    _FF(fifoDwordCount) -= 1
#else // DEBUGFIFO
#define DF_INIT_FIFO_COUNT(n) 
#define DF_DECR_FIFO_COUNT
#endif // DEBUGFIFO

#ifdef DEBUGFIFO

#ifdef IS_32

extern void mySetCF(NT9XDEVICEDATA * ppdev, FxU32 *hwPtr, FxU32 hwIndex, FxU32 data);
extern void mySetPH(NT9XDEVICEDATA * ppdev, FxU32 *hwPtr, FxU32 hwIndex, FxU32 data);

  #define SETCF(hwPtr, data) \
          mySetCF(ppdev, hwPtr, hwIndex++, (data)); \
          DF_DECR_FIFO_COUNT;

  #define SETPH(hwPtr, data) \
          mySetPH(ppdev, hwPtr, hwIndex++, (data));

  #define SETFCF(hwPtr, data) \
          mySetFCF(ppdev, hwPtr, hwIndex++, (data)); \
          DF_DECR_FIFO_COUNT;

  __inline void mySetFCF(NT9XDEVICEDATA * ppdev, FxU32 *hwPtr, FxU32 hwIndex, float data)
  {
    mySetCF(ppdev, hwPtr, hwIndex, *(FxU32 *)&data) ;
  }

#else // IS_32

extern void mySetCF(FxU32 hwPtr, FxU32 hwIndex, FxU32 data);
extern void mySetPH(FxU32 hwPtr, FxU32 hwIndex, FxU32 data);

  #define SETCF(hwPtr, data) \
          mySetCF(hwPtr, hwIndex++, (data)); \
          DF_DECR_FIFO_COUNT;

  #define SETPH(hwPtr, data) \
          mySetPH(hwPtr, hwIndex++, (data));

#endif // IS_32

  #define SETPD(hwPtr, hwReg, data) \
          SETCF((hwPtr), (data));

  #define SET(hwPtr, hwReg, data) \
          SETCF((hwPtr), (data));

  #define SETC(hwPtr, hwReg, data) \
          SETCF((hwPtr), (data));

#ifdef IS_16

  #undef  SETFCF  // no float transfers from 16-bit driver
  #undef  SETFPD  // no float transfers from 16-bit driver
  #undef  SETF    // no float transfers from 16-bit driver

#else // IS_16

  #define SETFPD(hwPtr, hwReg, data ) \
          SETFCF((hwPtr), (data));

  #define SETF(hwPtr, hwReg, data) \
          SETFCF((hwPtr), (data));

#endif // IS_16

#endif // DEBUGFIFO

#endif // CMDFIFO

//----------------------------------------------------------------------------------//
//                         DIRECT WRITE ACCESS MACROS
//----------------------------------------------------------------------------------//

// SETDW  - Direct write of integer data.
// SETFDW - Direct write of float data. 
// GET    - Direct read of integer data.

#ifdef IS_16

  #define SETDW(hwReg, data) \
          h3WRITE(NULL, (FxU32 *)&(hwReg), (data));

  #define GET(hwPtr) \
          h3READ(NULL, (FxU32 *)&(hwPtr))

  #undef  SETFDW  // no float transfers from 16-bit driver

#else // IS_16

  #define SETDW(hwReg, data) \
          (*(volatile FxU32 *)&(hwReg)) = (data);

  #define SETFDW(hwReg, data) \
          (*(volatile float *)&(hwReg)) = (data);

  #define GET(hwPtr) \
          (*(volatile FxU32 *)&(hwPtr))

#endif // IS_16

#ifndef CMDFIFO

  /* Make command fifo access macros use direct writes! */

  #define SETPH(hwPtr,data)

  #define SETPD(hwPtr, hwReg, data) \
          SETDW((hwReg), (data))

  #define SETFPD(hwPtr, hwReg, data) \
          SETFDW((hwReg), (data))
      
  #define SET(hwPtr, hwReg, data) \
          SETDW((hwReg), (data))
  
  #define SETC(hwPtr, hwReg, data) \
          SETDW((hwReg), (data))

  #define SETF(hwPtr, hwReg, data) \
          SETFDW((hwReg), (data))

#endif // !CMDFIFO


//----------------------------------------------------------------------------------//
//                         16,32-BIT COMMON MACROS
//----------------------------------------------------------------------------------//

  #define BUMP(n) // as nothing

  #define LOAD_CMDFIFO_RDPTR(p) \
          GET(((SstCRegs *) p)->PRIMARY_CMDFIFO.readPtrL )


//----------------------------------------------------------------------------------//
//                         COMMAND FIFO MANAGEMENT MACROS
//----------------------------------------------------------------------------------//

// CMDFIFO_PROLOG    - create local fifo pointer and local fifo count
// CMDFIFO_SETUP     - transfer global fifo pointer to local, zero fifo count
// CMDFIFO_EPILOG    - update global fifo pointer and count from locals, zero fifo count
// CMDFIFO_SAVE      - same as CMDFIFO_EPILOG, without zero fifo count
// CMDFIFO_RELOAD    - same as CMDFIFO_SETUP, with no differences
// CMDFIFO_CHECKROOM - check for space in command fifo using locals fifo count
// CMDFIFO_CHECKBUMP - check whether fencing needs to occur, to prevent bump overflow

#ifdef CMDFIFO
  #define CMDFIFO_CHECKBUMP(hwPtr, n)            \
          CMDFIFOBUMP += (n);                    \
          if (CMDFIFOBUMP >= 0xffff)             \
          {                                      \
            P6FENCE;                             \
            CMDFIFOBUMP = (n);                   \
            if (_FF(doAgpCF))                    \
              MYAGPEPILOG(ppdev, (DWORD)hwPtr);  \
          }

#ifdef IS_32

  #define CMDFIFO_PROLOG(hwPtr)                  \
          FxU32 *(hwPtr)= (FxU32 *)CMDFIFOPTR;   \
          FxU32 hwIndex=0;

  #ifdef AGP_CMDFIFO
    #define CMDFIFO_EPILOG(hwPtr)                \
            hwPtr += hwIndex;                    \
            CMDFIFOPTR = (FxU32)hwPtr;           \
            CMDFIFOSPACE -= hwIndex;             \
            if (_FF(doAgpCF))                    \
              MYAGPEPILOG(ppdev, (DWORD)hwPtr);  \
            hwIndex = 0;
  #else // AGP_CMDFIFO
    #define CMDFIFO_EPILOG(hwPtr)                \
            hwPtr += hwIndex;                    \
            CMDFIFOPTR = (FxU32)hwPtr;           \
            CMDFIFOSPACE -= hwIndex;             \
            hwIndex = 0;
  #endif // AGP_CMDFIFO

  #define CMDFIFO_SAVE(hwPtr)                    \
          hwPtr += hwIndex;                      \
          CMDFIFOSPACE -= hwIndex;               \
          CMDFIFOPTR = (FxU32)hwPtr;
  
  #define CMDFIFO_RELOAD( hwPtr )                \
          hwPtr = (FxU32 *)CMDFIFOPTR;           \
          hwIndex = 0;

  #define CMDFIFO_CHECKROOM(hwPtr, n)            \
          CMDFIFO_CHECKBUMP(hwPtr, n)            \
          CMDFIFOSPACE -= hwIndex;               \
          hwPtr += hwIndex;                      \
          hwIndex = 0;                           \
          DF_INIT_FIFO_COUNT(n);                 \
          if ((n) > CMDFIFOSPACE)                \
          {                                      \
            fifo_MakeRoom(ppdev, &(hwPtr), (n)); \
          }

#else // IS_32

  #define CMDFIFO_PROLOG(hwPtr)                  \
          FxU32 (hwPtr);                         \
          FxU32 hwIndex;

  #define CMDFIFO_SETUP(hwPtr)                   \
          (hwPtr) = CMDFIFOPTR;                  \
          hwIndex = 0;

  #ifdef AGP_CMDFIFO
    #define CMDFIFO_EPILOG(hwPtr)                \
            hwPtr += (hwIndex << 2);             \
            CMDFIFOSPACE -= hwIndex;             \
            CMDFIFOPTR = hwPtr;                  \
            if (_FF(doAgpCF))                    \
              MYAGPEPILOG(ppdev, (DWORD)hwPtr);  \
            hwIndex = 0;
  #else // AGP_CMDFIFO
    #define CMDFIFO_EPILOG(hwPtr)                \
            hwPtr += (hwIndex << 2);             \
            CMDFIFOSPACE -= hwIndex;             \
            CMDFIFOPTR = hwPtr;                  \
            hwIndex = 0;
  #endif // AGP_CMDFIFO

  #define CMDFIFO_SAVE(hwPtr)                    \
          hwPtr += (hwIndex << 2);               \
          CMDFIFOSPACE -= hwIndex;               \
          CMDFIFOPTR = hwPtr;

  #define CMDFIFO_RELOAD(hwPtr)                  \
          (hwPtr) = CMDFIFOPTR;                  \
          hwIndex = 0;

  #define CMDFIFO_CHECKROOM(hwPtr, n)            \
          CMDFIFO_CHECKBUMP(hwPtr, n)            \
          CMDFIFOSPACE -= hwIndex;               \
          hwPtr += (hwIndex << 2);               \
          hwIndex = 0;                           \
          DF_INIT_FIFO_COUNT(n);                 \
          if( ((DWORD)(n))>CMDFIFOSPACE )        \
          {                                      \
            hwPtr = fifo_MakeRoom((hwPtr), (n)); \
          }

#endif // IS_32

#else // CMDFIFO

  /* Make command fifo management macros do nothing! */

  #define CMDFIFO_PROLOG(hwptr)
  #define CMDFIFO_EPILOG(hwptr)
  #define CMDFIFO_SETUP(hwPtr)
  #define CMDFIFO_SAVE(hwPtr)
  #define CMDFIFO_RELOAD(hwPtr)
  #define CMDFIFO_CHECKROOM(hwPtr, n)

#endif // CMDFIFO

//----------------------------------------------------------------------------------//
//                         COMMAND FIFO PACKET MACROS                 
//----------------------------------------------------------------------------------//

// CMDFIFO_BUILD_PK1       - build a command FIFO packet #1, with 3D registers, for all chips
// CMDFIFO_BUILD_PK1CHIP   - build a command FIFO packet #1, with 3D registers, for specified chips
// CMDFIFO_BUILD_2DPK1     - build a command FIFO packet #1, with 2D registers
// CMDFIFO_BUILD_IOPK1     - build a command FIFO packet #1, with I/O registers
// CMDFIFO_BUILD_PK1R      - build a command FIFO packet #1, with 3D registers, specified by offset
// CMDFIFO_BUILD_PK2       - build a command FIFO packet #2
// CMDFIFO_BUILD_PK3       - build a command FIFO packet #3
// CMDFIFO_BUILD_PK4       - build a command FIFO packet #4
// CMDFIFO_BUILD_PK5       - build a command FIFO packet #5
// CMDFIFO_BUILD_PK6_0     - build a command FIFO packet #6, word #0
// CMDFIFO_BUILD_PK6_1     - build a command FIFO packet #6, word #1
// CMDFIFO_BUILD_PK6_2     - build a command FIFO packet #6, word #2
// CMDFIFO_BUILD_PK6_3     - build a command FIFO packet #6, word #3
// CMDFIFO_BUILD_PK6_4     - build a command FIFO packet #6, word #4

#ifdef IS_32

  #define PH1_SIZE  1
  #define PH3_SIZE  1
  #define PH4_SIZE  1
  #define PH5_SIZE  2

  #define CMDFIFO_BUILD_PK1(n, inc, regName, chip) \
    (((n) << 16) | ((inc) << 15) | ((offsetof(SstRegs, regName) / 4) << 3) | SSTCP_PKT1)

  #define CMDFIFO_BUILD_PK1CHIP(n, inc, regName, chip) \
    (((n) << 16) | ((inc) << 15) | ((offsetof(SstRegs, regName) / 4) << 3) | SSTCP_PKT1 | (chip << 11))

  #define CMDFIFO_BUILD_2DPK1(n, inc, regName, chip) \
    (((n) << 16) | ((inc) << 15) | ((offsetof(SstGRegs, regName) / 4) << 3) | SSTCP_PKT1 | SSTCP_PKT1_2D)

  #define CMDFIFO_BUILD_IOPK1(n, inc, regName, chip) \
    (((n) << 16) | ((inc) << 15) | ((offsetof(SstIORegs, regName) / 4) << 3) | SSTCP_PKT1 )

  #define CMDFIFO_BUILD_PK1R(n, inc, reg, chip) \
    (((n) << 16) | ((inc) << 15) | (reg << 3) | SSTCP_PKT1 )

  #define CMDFIFO_BUILD_PK2(mask)\
    ((mask) | SSTCP_PKT2 )

  #define CMDFIFO_BUILD_PK3(cmd, noVert, param, packed) \
    (((packed) << 28) | ((param & 0xF0000) << 6) | ((param & 0xFF) << 10) | (noVert << 6) | (cmd << 3) | SSTCP_PKT3 )

  #define CMDFIFO_BUILD_PK4(mask, regName, chip) \
    (((mask) << SSTCP_PKT4_MASK_SHIFT) | (chip << 11) | ((offsetof(SstRegs, regName) / 4) << 3) | SSTCP_PKT4 )

  #define CMDFIFO_BUILD_PK5(n, byteEnableW2, byteEnableWN, memPort) \
    ((n << SSTCP_PKT5_NWORDS_SHIFT) | (byteEnableW2 << SSTCP_PKT5_BYTEN_W2_SHIFT) \
    | (byteEnableWN << SSTCP_PKT5_BYTEN_WN_SHIFT) | (memPort) | SSTCP_PKT5 )

  #define CMDFIFO_BUILD_PK6_0(n, memPort) \
    ((n << SSTCP_PKT6_NBYTES_SHIFT) | (memPort) | SSTCP_PKT6 )

  #define CMDFIFO_BUILD_PK6_1(physAddrLow) \
    ((physAddrLow) & SSTCP_PKT6_SRC_BASELOW )

  #define CMDFIFO_BUILD_PK6_2(physAddrHi, srcStride, srcWidth) \
    ((((physAddrHi)<<SSTCP_PKT6_SRC_BASEHIGH_SHIFT) & SSTCP_PKT6_SRC_BASEHIGH) \
    | (((srcStride)<<SSTCP_PKT6_SRC_STRIDE_SHIFT) & SSTCP_PKT6_SRC_STRIDE) \
    |  ((srcWidth) & SSTCP_PKT6_SRC_WIDTH))

  #define CMDFIFO_BUILD_PK6_3(dstAddr) \
    ((dstAddr) & SSTCP_PKT6_FRAME_BUFFER_OFFSET )

  #define CMDFIFO_BUILD_PK6_4(dstStride) \
    (((dstStride) & SSTCP_PKT6_DST_STRIDE) )

#endif // IS_32


//----------------------------------------------------------------------------------//
//                         WRITE COMBINE FLUSH MACRO
//----------------------------------------------------------------------------------//

#undef P6FENCE

#ifdef WRITECOMBINE

/* P6 fencing */

#pragma warning (disable: 4704)
static void INLINE _p6fencer( void )
{
  volatile WORD fenceVar;

#if defined(THUNK32)
#ifdef DEBUG
  // Debug Fix
  __asm
  {
     movzx   ebp, bp;
     mov     eax, [ebp];
     movzx   eax, ax;
     mov     [ebp], eax;
  }
#endif //#ifdef DEBUG
#endif //#if defined(THUNK32)

  __asm
  {
    xchg ax, fenceVar;
  }
}
#pragma warning (default: 4704)

  #define P6FENCE _p6fencer()

#else // WRITECOMBINE

  #define P6FENCE

#endif // WRITECOMBINE


//----------------------------------------------------------------------------------//
//                         AGP COMMAND FIFO MACROS
//----------------------------------------------------------------------------------//

#ifdef AGP_CMDFIFO

#ifdef IS_32

  #define BUMPAGP(arg1,arg2) bumpAgp(arg1,arg2)
  #define FLUSHAGP(arg1) flushAgp(arg1)
  #define MYWRAPAGP(arg1,arg2) myWrapAgp(arg1,arg2)
  #define MYAGPEPILOG(arg1,arg2) myAgpEpilog(arg1,arg2)

#else // IS_32

  #define BUMPAGP(arg1,arg2) bumpAgp(arg2)
  #define FLUSHAGP(arg1) flushAgp()
  #define MYWRAPAGP(arg1,arg2) myWrapAgp(arg2)
  #define MYAGPEPILOG(arg1,arg2) myAgpEpilog(arg2)

#endif // IS_32

  void BUMPAGP(NT9XDEVICEDATA * ppdev, FxU32 nWords);
  void FLUSHAGP(NT9XDEVICEDATA * ppdev);
  void MYWRAPAGP(NT9XDEVICEDATA * ppdev, FxU32 hwPtr);
  void MYAGPEPILOG(NT9XDEVICEDATA * ppdev, FxU32 hwPtr);

#endif // AGP_CMDFIFO

#define AGP_FLUSH

#ifdef IS_32
#ifdef CMDFIFO
#ifdef AGP_CMDFIFO

  #undef AGP_FLUSH
  #define AGP_FLUSH                                        \
          {                                                \
            if (_FF(doAgpCF))                              \
              MYAGPEPILOG(ppdev, (DWORD)CMDFIFOPTR);       \
          }

#endif // AGP_CMDFIFO
#endif // CMDFIFO
#endif // IS_32


//----------------------------------------------------------------------------------//
//                         COMMAND FIFO INLINE FUNCTIONS
//----------------------------------------------------------------------------------//

// fifo_Wrap         - wrap back to the beginning of the command fifo
// fifo_MakeRoom     - make room for data in the command fifo

#define FIFO_MIN(A,B) ((A) < (B) ? (A) : (B))
#ifdef CMDFIFO

#ifdef IS_32

  static INLINE FxU32* fifo_Wrap(GLOBALDATA * ppdev, FxU32 *hwPtr )
   {
   FxU32 *rdPtr;
#if defined(SLI_AA) && !defined(WIN_CSIM)
   int i;
   int nNumChips;
   FxU32 *rdPtrMin;

   // If SLI or AA is enabled then check all Chip's CMDFIFO
   if (_FF(gdiFlags) & SDATA_GDIFLAGS_SLI_AA_MASTER)
      nNumChips = _FF(dwNumUnits);
   else 
      nNumChips = 1;

   rdPtrMin = (FxU32 *)CMDFIFOEND;
   for (i=0; i<nNumChips; i++)
      {
      do
         {
         rdPtr = (FxU32*)(LOAD_CMDFIFO_RDPTR(_FF(regBase[i * HWINFO_SST_MAX_NUM_CHIPS + HWINFO_SST_CMDFIFOREGS_INDEX])) + CMDFIFOOFFSET);
         } 
      while( (rdPtr > hwPtr) || (rdPtr == (FxU32 *)CMDFIFOSTART) );
      rdPtrMin = FIFO_MIN(rdPtrMin, rdPtr);
      }
   rdPtr = rdPtrMin;
#else
    do
    {
      rdPtr = (FxU32*)(LOAD_CMDFIFO_RDPTR(ghwAC) + CMDFIFOOFFSET);
    } while( (rdPtr > hwPtr) || (rdPtr == (FxU32 *)CMDFIFOSTART) );
#endif

    *(volatile FxU32 *)(hwPtr) = CMDFIFOJMP;
    #ifdef AGP_CMDFIFO
      if (_FF(doAgpCF))
        *((volatile FxU32 *)(hwPtr)+1) = CMDFIFOJMP2;
    #endif // AGP_CMDFIFO

    P6FENCE;

    #ifdef AGP_CMDFIFO
      if (_FF(doAgpCF))
        MYWRAPAGP(ppdev, (DWORD)hwPtr);
    #endif // AGP_CMDFIFO

    hwPtr = (FxU32 *)CMDFIFOSTART;

    CMDFIFOSPACE = (FxU32)(rdPtr - hwPtr) - 1;
  
    return( hwPtr );
  }

  static INLINE void fifo_MakeRoom(GLOBALDATA * ppdev, FxU32 **hwPtr, FxU32 n )
  {
    FxU32 *rdPtr;
    FxU32 resetIdleCount = 0;
#if defined(SLI_AA) && !defined(WIN_CSIM)
    int i;
    int nNumChips;
    FxU32 dwCmdFifoSpace;
    FxU32 dwCmdFifoMin;
#endif
    
    if( (*hwPtr + n) > (FxU32 *)CMDFIFOEND )
      *hwPtr = fifo_Wrap(ppdev, *hwPtr);
    
#if defined (SLI_AA) && !defined(WIN_CSIM)
   if (_FF(gdiFlags) & SDATA_GDIFLAGS_SLI_AA_MASTER)
      nNumChips = _FF(dwNumUnits);
   else 
      nNumChips = 1;

   dwCmdFifoMin = CMDFIFOEND - CMDFIFOSTART + 1;
   for (i=0; i<nNumChips; i++)
      {
      dwCmdFifoSpace = CMDFIFOSPACE;
      while (dwCmdFifoSpace < n)
         {
         rdPtr = (FxU32*)(LOAD_CMDFIFO_RDPTR(_FF(regBase[i * HWINFO_SST_MAX_NUM_CHIPS + HWINFO_SST_CMDFIFOREGS_INDEX])) + CMDFIFOOFFSET);

         if (rdPtr <= *hwPtr)
            {
            dwCmdFifoSpace = (FxU32)((FxU32 *)CMDFIFOEND - *hwPtr);
            }
         else
            {
            dwCmdFifoSpace = (FxU32)(rdPtr - *hwPtr) - 1;
            }
         }
      dwCmdFifoMin = FIFO_MIN(dwCmdFifoMin, dwCmdFifoSpace);
      }
    CMDFIFOSPACE = dwCmdFifoMin;
#else
    while (CMDFIFOSPACE < n)
    {
      rdPtr = (FxU32*)(LOAD_CMDFIFO_RDPTR(ghwAC) + CMDFIFOOFFSET);

      if (rdPtr <= *hwPtr)
      {
        CMDFIFOSPACE = (FxU32)((FxU32 *)CMDFIFOEND - *hwPtr);
      }
      else
      {
        CMDFIFOSPACE = (FxU32)(rdPtr - *hwPtr) - 1;
      }
    }
#endif
  }

#else // IS_32

  static INLINE FxU32 fifo_Wrap(FxU32 hwPtr)
  {
    FxU32 rdPtr;
#if defined (SLI_AA) && !defined(WIN_CSIM)
   int i;
   int nNumChips;
   FxU32 rdPtrMin;
#endif

    DEBUG_FIX;

#if defined(SLI_AA) && !defined(WIN_CSIM)
   // If SLI or AA is enabled then check all Chip's CMDFIFO
   if (_FF(gdiFlags) & SDATA_GDIFLAGS_SLI_AA_MASTER)
      nNumChips = (int)_FF(dwNumUnits);
   else 
      nNumChips = 1;

   rdPtrMin = CMDFIFOEND;
   for (i=0; i<nNumChips; i++)
      {
      do
         {
         rdPtr = LOAD_CMDFIFO_RDPTR(_FF(regBase[i * HWINFO_SST_MAX_NUM_CHIPS + HWINFO_SST_CMDFIFOREGS_INDEX])) + CMDFIFOOFFSET;
         } 
      while( (rdPtr > hwPtr) || (rdPtr == CMDFIFOSTART) );
      rdPtrMin = FIFO_MIN(rdPtrMin, rdPtr);
      }
   rdPtr = rdPtrMin;
#else
    do
    {
      rdPtr = LOAD_CMDFIFO_RDPTR(_FF(lpCRegs)) + CMDFIFOOFFSET;
    } while ((rdPtr > hwPtr) || (rdPtr == CMDFIFOSTART));
#endif

    #ifndef THUNK32
      h3WRITE(NULL, (DWORD *)(hwPtr), CMDFIFOJMP); 
      #ifdef AGP_CMDFIFO
        if (_FF(doAgpCF))
		  h3WRITE(NULL, ((DWORD *)(hwPtr))+1, CMDFIFOJMP2); 
      #endif // AGP_CMDFIFO 
    #else // !THUNK32
      *(volatile FxU32 *)(hwPtr) = CMDFIFOJMP;
      #ifdef AGP_CMDFIFO
        if (_FF(doAgpCF))
		  *((volatile FxU32 *)(hwPtr)+1) = CMDFIFOJMP2;
      #endif // AGP_CMDFIFO 
    #endif // !THUNK32

    P6FENCE;

    #ifdef AGP_CMDFIFO
      if (_FF(doAgpCF))
	    MYWRAPAGP(ppdev, (DWORD)hwPtr);
    #endif // AGP_CMDFIFO 

    hwPtr = CMDFIFOSTART;

    CMDFIFOSPACE = ((rdPtr - hwPtr)/4) - 1;

    return (hwPtr);
  }

  static INLINE FxU32 fifo_MakeRoom( DWORD hwPtr, FxU32 n )
  {
    DWORD rdPtr;
#if defined (SLI_AA) && !defined(WIN_CSIM)
   int i;
   int nNumChips;
   FxU32 dwCmdFifoSpace;
   FxU32 dwCmdFifoMin;
#endif

    DEBUG_FIX;

    if ((hwPtr + (n*4)) >= CMDFIFOEND)
    {
      hwPtr = fifo_Wrap(hwPtr);
    }

#if defined (SLI_AA) && !defined(WIN_CSIM)
    
   if (_FF(gdiFlags) & SDATA_GDIFLAGS_SLI_AA_MASTER)
      nNumChips = (int)_FF(dwNumUnits);
   else 
      nNumChips = 1;

   dwCmdFifoMin = CMDFIFOEND - CMDFIFOSTART + 1;
   for (i=0; i<nNumChips; i++)
      {
      dwCmdFifoSpace = CMDFIFOSPACE;
      while (dwCmdFifoSpace < n)
         {
         rdPtr = LOAD_CMDFIFO_RDPTR(_FF(regBase[i * HWINFO_SST_MAX_NUM_CHIPS + HWINFO_SST_CMDFIFOREGS_INDEX])) + CMDFIFOOFFSET;
         if (rdPtr <= hwPtr)
            {
            if ((dwCmdFifoSpace = (FxU32)((CMDFIFOEND - hwPtr))/4) < n)
               {
               // Don't need a wrap per chip ... only on first
               if (0 == i)
                  hwPtr = fifo_Wrap(hwPtr);
               }
            }
         else 
            {
            dwCmdFifoSpace = (FxU32)((rdPtr - hwPtr)/4) - 1;
            }
         }
      dwCmdFifoMin = FIFO_MIN(dwCmdFifoMin, dwCmdFifoSpace);
      }
   CMDFIFOSPACE = dwCmdFifoMin;
#else       
    while (CMDFIFOSPACE < n)
    {
      rdPtr = LOAD_CMDFIFO_RDPTR(_FF(lpCRegs)) + CMDFIFOOFFSET;
      if (rdPtr <= hwPtr)
      {
        if ((CMDFIFOSPACE = (FxU32)((CMDFIFOEND - hwPtr))/4) < n)
        {
          hwPtr = fifo_Wrap(hwPtr);
        }
      }
      else 
      {
        CMDFIFOSPACE = (FxU32)((rdPtr - hwPtr)/4) - 1;
      }
    }
#endif

    return (hwPtr);
  }

#endif // IS_32

#endif // CMDFIFO


//----------------------------------------------------------------------------------//
//                         CLEAR COMMANDEX REGISTER MACROS
//----------------------------------------------------------------------------------//

#ifdef THUNK32

#ifdef CMDFIFO

  #define CLEAR_COMMAND_EX(cf)                   \
          if (_FF(ClearCommandEx))               \
          {                                      \
            CMDFIFO_CHECKROOM(cf, 2);            \
            SETPH(cf, SSTCP_PKT2 | commandExBit);\
            SET(cf, _FF(lpGRegs->commandEx), 0); \
            BUMP(2);                             \
            _FF(ClearCommandEx) = 0;             \
          }

#else // CMDFIFO

  #define CLEAR_COMMAND_EX(cf)                   \
          if (_FF(ClearCommandEx))               \
          {                                      \
            SET(cf, _FF(lpGRegs->commandEx), 0); \
            _FF(ClearCommandEx)=0;               \
          }

#endif // CMDFIFO

#endif // THUNK32

#endif //__FIFOMGR_H__
