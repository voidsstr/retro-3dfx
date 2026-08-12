/******************************Module*Header*******************************\
* Module Name: hw.h
*
* All the hardware specific driver file stuff.  Parts are mirrored in
* 'hw.inc'.
*
* Copyright (c) 1992-1996 Microsoft Corporation
* Copyright (c) 1998 3Dfx Interactive, Inc.
*
\**************************************************************************/

////////////////////////////////////////////////////////////////////////
// Chip equates:

#define STATUS_1                        0x03DA
#define DISPLAY_MODE_INACTIVE           0x001
#define VBLANK_ACTIVE                   0x008

#define CRTC_INDEX                      0x03D4
#define CRTC_DATA                       0x03D5

#define S3R8                            0x038
#define S3R9                            0x039
#define S3R1                            0x031
#define S3R5                            0x035

#define CR39                            0x039
#define CR4C                            0x04C
#define CR4D                            0x04D

#define REG_UNLOCK_1                    0x048
#define CPUA_BASE                       0x001

#define SYSCTL_UNLOCK                   0x0A0
#define SYSCTL_LOCK                     0x000

#define SYS_CNFG                        0x040
#define LAW_CTL                         0x058
#define EX_SCTL_2                       0x051
#define EX_DAC_CT                       0x055

#define MISC_1                          0x03A

//////////////////////////////////////////////////////////////////////
// PowerPC considerations
//
// The PowerPC does not guarantee that I/O to separate addresses will
// be executed in order.  However, the PowerPC guarantees that
// output to the same address will be executed in order.
//
// Consequently, we use the following synchronization macros.  They
// are relatively expensive in terms of performance, so we try to avoid
// them whereever possible.
//
// CP_EIEIO() 'Ensure In-order Execution of I/O'
//    - Used to flush any pending I/O in situations where we wish to
//      avoid out-of-order execution of I/O to separate addresses.
//
// CP_MEMORY_BARRIER()
//    - Used to flush any pending I/O in situations where we wish to
//      avoid out-of-order execution or 'collapsing' of I/O to
//      the same address.  We used to have to do this separately for
//      the Alpha because unlike the PowerPC it did not guarantee that
//      output to the same address will be exectued in order.  However,
//      with the move to kernel-mode, on Alpha we are now calling HAL
//      routines for every port I/O which ensure that this is not a
//      problem.

#if defined(_PPC_)

    // On PowerPC, CP_MEMORY_BARRIER doesn't do anything.

    #define CP_EIEIO()          MEMORY_BARRIER()
    #define CP_MEMORY_BARRIER()

#elif defined(_ALPHA_)

    // On Alpha, since we must do all non-frame-buffer I/O through HAL
    // routines, which automatically do memory-barriers, we don't have
    // to do memory barriers ourselves (and should not, because it's a
    // performance hit).

    #define CP_EIEIO()
    #define CP_MEMORY_BARRIER()

#else

    // On other systems, both CP_EIEIO and CP_MEMORY_BARRIER don't do anything.

    #define CP_EIEIO()          MEMORY_BARRIER()
    #define CP_MEMORY_BARRIER() MEMORY_BARRIER()

#endif

////////////////////////////////////////////////////////////////////
// Macros for accessing accelerator registers:

////////////////////////////////////////////////////////////////////////
// Unsafe direct access macros
//
// These are macros for directly accessing Banshee's accelerator
// registers.  They should be used with care, because they always
// ignore memory barriers:

#define CP_WRITE_DIRECT(pjBase, addr, dw)                           \
    WRITE_REGISTER_ULONG((BYTE*) (pjBase) + (addr), (ULONG) (dw))

#define CP_WRITE_DIRECT_BYTE(pjBase, addr, j)                       \
    WRITE_REGISTER_UCHAR((BYTE*) (pjBase) + (addr), (UCHAR) (j))

#define CP_READ_REGISTER(p)                                         \
    READ_REGISTER_ULONG((BYTE*) (p))

#define CP_READ_REGISTER_BYTE(p)                                    \
    READ_REGISTER_UCHAR((BYTE*) (p))


#ifdef H3_FIFO
#  ifdef CSIM
    // CSIM is so slow we'll always be busy so lie for testing.
    #define H3_FIFO_BUSY(ppdev, pjBase, level) FALSE
#  else
     #define H3_FIFO_BUSY(ppdev, pjBase, level)          \
        (((GET_ABSOLUTE((BYTE*) pjBase) & SST_PCIFIFO_FREE) - level) < 0)
#  endif
#else
    // No fifo, so it's never busy.
    #define H3_FIFO_BUSY(ppdev, pjH3Base, level) FALSE
#endif


#ifdef SLI_AA
typedef struct _DDGLOBAL DDGLOBAL;

__inline BOOL H3_GP_BUSY(PDEV *ppdev, BYTE *pjBase)
{
  DWORD     dwStatus;


  P6_FENCE;

  dwStatus = GET2(((SstIORegs *)ppdev->pjBase)->status);

  // If we are in SLI Mode
  if (ppdev->DriverData.ddMultiChipConfig)
  {
    int       i;
    int       nNumChips;
    SstIORegs *pIORegs;


    nNumChips = ppdev->DriverData.dwNumUnits;

    for (i = 1; i < nNumChips; i++)
    {
      pIORegs = (SstIORegs *)ppdev->DriverData.regBase[i * HWINFO_SST_MAX_CHIP_INDEX + HWINFO_SST_IOREGS_INDEX];
      dwStatus |= GET2(pIORegs->status);
    }
  }

  return (dwStatus & (SST_BUSY | SST_GUI_BUSY | SST_CMD0_BUSY));
}
#else
// Works with either board base or 2D base

__inline BOOL H3_GP_BUSY(PDEV* ppdev, BYTE* pjBase)
{
    P6_FENCE;
//    CP_EIEIO();
    return(GET_ABSOLUTE((BYTE*) pjBase) & (SST_BUSY | SST_GUI_BUSY | SST_CMD0_BUSY));
}
#endif

#if DBG

    VOID vH3GpWait(PDEV*, BYTE*);

    #define H3_GP_WAIT(ppdev, pjBase)                 \
        vH3GpWait((ppdev), (pjBase))

#else

/* retro3dfx: bounded (was unbounded) — see FXBUSYWAIT note in DDGLOBAL.H. */
#if ENABLE_LOG_FILE
    #define H3_GP_WAIT(ppdev, pjBase)                                          \
        do {                                                                   \
          ULONG _retroGpSpin = 0;                                              \
          while (H3_GP_BUSY(ppdev, pjBase)) {                                  \
            if (++_retroGpSpin >= 100000000UL) {                               \
              retroLogForce((PDEV *)(ppdev),                                   \
                            "retro3dfx H3GpWait WEDGE-BREAK@100M\r\n");        \
              break;                                                           \
            }                                                                  \
          }                                                                    \
        } while (0)
#else
    #define H3_GP_WAIT(ppdev, pjBase)                     \
        do {;} while (H3_GP_BUSY(ppdev, pjBase))
#endif

#endif

/* retro3dfx: bounded replacement for the raw `while(H3_GP_BUSY(ppdev,
   ppdev->pjH3Base));` spins in the 2D BitBlt / DrvCopyBits path (BITBLT.C).
   Those waited for the 2D engine to idle before reading framebuffer bits; if
   the accelerator is stuck (e.g. the shared command FIFO is mid-D3D-present),
   the raw spin hangs win32k -> GDI BitBlt / a client SCREENSHOT never returns.
   Bounded at 100M reads (~100s, unreachable in healthy operation) with a ring
   log on breach, matching the DDraw/3D wedge-breakers. Behaviourally identical
   otherwise. */
#if ENABLE_LOG_FILE
#define RETRO_GP_SPIN(ppdev)                                                  \
    do {                                                                      \
      ULONG _retroBltSpin = 0;                                                \
      while (H3_GP_BUSY((ppdev), (ppdev)->pjH3Base)) {                        \
        if (++_retroBltSpin >= 100000000UL) {                                 \
          retroLogForce((PDEV *)(ppdev),                                      \
                        "retro3dfx BitBlt-GPSpin WEDGE-BREAK@100M\r\n");       \
          break;                                                              \
        }                                                                     \
      }                                                                       \
    } while (0)
#else
#define RETRO_GP_SPIN(ppdev)                                                  \
    do { while (H3_GP_BUSY((ppdev), (ppdev)->pjH3Base)) ; } while (0)
#endif

/////////////////////////////////////////////////////////////////
// DirectDraw stuff

__inline BOOL VBLANK_IS_ACTIVE(BYTE* pjH3Base)
{
    CP_EIEIO();
    return(!GET_ABSOLUTE((BYTE*) (pjH3Base)+H3_2DREG_OFFSET(status)) & SST_VRETRACE);
}

__inline ULONG GET_SCANLINE(BYTE* pjBase)
{
    CP_EIEIO();
    return(GET_ABSOLUTE((pjBase) + FIELDOFFSET(SstIORegs,vidCurrentLine)));
}

//////////////////////////////////////////////////////////////////////

// The H3_PACKXY macro is used for line drawing, and is safe for
// negative 'x' values:

#define H3_PACKXY(x, y)        (((y) << 16) | (x) & 0xffff)

// This one isn't safe for negative 'x' values:

#define H3_PACKXY_FAST(x, y)  (((y) << 16) | (x))

//////////////////////////
// H3_TRANSFER_DWORD_ALIGNED - Dword transfers using memory-mapped transfers.
//
// Source must be dword aligned!

#define H3_TRANSFER_DWORD_ALIGNED(ppdev, pjH3Base, p, c)                    \
{                                                                           \
             ULONG   mcd             = (c);                                 \
             ULONG UNALIGNED* mpdSrc = (ULONG*) (p);                        \
    ASSERTDD((((ULONG) p) & 3) == 0, "Transfer not dword aligned");         \
    ASSERTDD((c) > 0, "Can't have a zero transfer count");                  \
                                                                            \
    CP_MEMORY_BARRIER();                                                    \
    do {                                                                    \
		SET_PKT1_REG(pjH3Base, launch, *mpdSrc++);                            \
    } while (--mcd);                                                        \
}

//////////////////////////
// H3_TRANSFER_DWORD - Dword transfers using memory-mapped transfers.
//
// Source does not have to be dword aligned.

#define H3_TRANSFER_DWORD(ppdev, pjH3Base, p, c)                            \
{                                                                           \
             ULONG   mcd             = (c);                                 \
             ULONG UNALIGNED* mpdSrc = (ULONG*) (p);                        \
    ASSERTDD((c) > 0, "Can't have a zero transfer count");                  \
                                                                            \
    CP_MEMORY_BARRIER();                                                    \
    do {                                                                    \
		SET_PKT1_REG(pjH3Base, launch, *mpdSrc++);                            \
    } while (--mcd);                                                        \
}

/////////////////////////////////////////////////////////////////////////////
// Trapezoid macros
//

#define CONVERT_POINT_TO_INTEGER( ulIPoint, pptfx, xOffset, yOffset )  	\
	ulIPoint =  H3_PACKXY(  (pptfx->x >> 4) + xOffset,                  \
	                        (pptfx->y >> 4) + yOffset  )

#define GET_NEXT_VERTEX( pptfx, lDelta )                                               \
{                                                                    \
        pptfx = (POINTFIX*) ((BYTE*) pptfx + lDelta);                \
        if (pptfx < pptfxFirst)                                      \
            pptfx = pptfxLast;                                       \
        else if (pptfx > pptfxLast)                                  \
            pptfx = pptfxFirst;                                      \
}

// moved ioctl defines and structures to an include file that can
// be shared by the miniport and display driver
#include "fxioctl.h"

