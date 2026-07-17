/*
** Copyright (c) 1995, 3Dfx Interactive, Inc.
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
** $Revision: 35$
** $Date: 10/25/00 4:56:48 AM$
*/

#ifndef __DDGLOBAL_H__
#define __DDGLOBAL_H__

#ifndef WINNT
#include "d3dhal.h"
#include "hw.h"
#include "header.h"
#include <stddef.h>
#endif

#ifdef DEBUG
#pragma  optimize ("",off)
#else
#pragma  optimize ("",on)
#endif

// The Global Structure is at the end of this file
#define FX_COLORMODEL_565    0x01
#define FX_COLORMODEL_555    0x02
#define FX_COLORMODEL_1555   0x04
#define FX_COLORMODEL_RGB    0x08
#define FX_COLORMODEL_BGR    0x10

#define FXCAPS_PALETTIZED_TEXTURES   1


/*
 * Control Panel Interface Stuff
 */

#define REGSTR_PATH_SW_3DFX_V2       "Software\\3Dfx Interactive\\Voodoo2"
#define REGSTR_PATH_SW_3DFX_V2_D3D   "Software\\3Dfx Interactive\\Voodoo2\\D3D"
// These two values should be equivelent
#define DEFUALT_GAMMA_FLOAT  1.2F
#define DEFAULT_GAMMA_DWORD  0x3FB9999A

/*
 * HW Initializtion
 */

#ifdef WINNT
typedef PDEV  NT9XDEVICEDATA;
#else
#define NT9XDEVICEDATA GLOBALDATA
#endif

// FX/Direct Draw Run Time Function Replacements

extern char *ddgetenv(NT9XDEVICEDATA *ppdev, const char *varname );
extern int ddatoi( const char *s );
extern double ddatof( const char *s );
extern float dd_itof_inv( int i );
extern unsigned long ddftol( float f );
extern int ddsscanf( const char *s, const char *fmt, FxU32 *result);
extern int ddxtoa( DWORD val, char *result);
extern int dditoa( int val, char *result);
extern int ddxtoi( DWORD val);
extern float ddpow( double x, double y);

// SST & Simulator Control
#ifdef WINNT
// note that the struct's are already declared as volatile in h3regs.h
// so no need to declare them volatile here
#if 1
#define ghw0    ((SstRegs *)(ppdev->p3DBase))
#define ghw     ((SstRegs *)(ppdev->p3DBase))
#define ghw3D   ((SstRegs *)(ppdev->p3DBase))
#else
#define ghw0    ((SstRegs *)(ppdev->pjBase+SST_3D_OFFSET))
#define ghw     ((SstRegs *)(ppdev->pjBase+SST_3D_OFFSET))
#define ghw3D   ((SstRegs *)(ppdev->pjBase+SST_3D_OFFSET))
#endif
#define ghw2D   ((SstGRegs  *)ppdev->pjH3Base)
#define ghwIO   ((SstIORegs *)ppdev->pjBase)
#define ghwAC   ((SstCRegs  *)ppdev->pjCmdAgpBase)
#else
#define ghw0    _DD(sst3DRegs)
#define ghw     _DD(sst3DRegs)
#define ghw3D   _DD(sst3DRegs)
#define ghw2D   _FF(lpGRegs)
#define ghwIO   _FF(lpIOregs)
#define ghwAC   _FF(lpCRegs)
#endif

#ifdef SST1_SIMULATOR
extern   void gdbg_init(void);
extern   void gsim_setFBsize(long, long);
extern   void dd_sethdc(long hdc);
extern   unsigned long *gsim_getfrontbuffer();
extern   unsigned long *gsim_getbackbuffer();
#define  GET_FRONTBUFFER  gsim_getfrontbuffer;
#define  GET_BACKBUFFER   gsim_getbackbuffer;
#define  GET_THIRDBUFFER  0
#define  GET_ZBUFFER      0
#define  GET_ALPHABUFFER  0
#endif

// Debug levels for trace control

#define DEBUG_APIENTRY       1
#define DEBUG_DDDETAILS      2
#define DEBUG_D3DDETALS      3
#define DEBUG_DDGORY         4
#define DEBUG_D3DGORY        5

#define Stringize(L)      #L
#define MakeString(M,L)   M(L)
#define MYLINE            MakeString(Stringize,__LINE__)
#define __FILELINE__      __FILE__"(" MYLINE ") "

#ifndef WINNT
#ifdef DEBUG
#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
#define DISPDBG(arg)
#define STATEDBG(level)
#define RIP(x)
#define ASSERTDD(x,y)
#else
#define DISPDBG(arg)      Msg arg
#define STATEDBG(level)   _DD(DD_DebugLevel) = (level)
#define RIP(x)            { Msg(ppdev,0,x); DebugBreak(); }
#define ASSERTDD(x,y)     if(! (x)) RIP(y)
#endif
#else
#define DISPDBG(arg)
#define STATEDBG(level)
#define RIP(x)
#define ASSERTDD(x,y)
#endif  // DEBUG
#endif  // WINNT

#ifdef FXTRACE

// define a win9x DDPRINT here if you want it
// don't make it incompatible with NT this time!
// so that probably means you need to do something on the
// order of the D3DPRINT function as declared in d3global.h
#define DDPRINT         1 ? (void)0 : (void)
#define DDDBGLVL        250

#else

#if defined(WINNT) && DBG
#define DDPRINT         DebugPrint
#define DDDBGLVL        2
#else
#define DDPRINT         1 ? (void)0 : (void)
#define DDDBGLVL        250
#endif

#if defined(WINNT) && (_WIN32_WINNT < 0x0500)
// define D3DPRINT for NT4 because of the moron adding D3DPRINT statements in the ddraw code
#define D3DPRINT        1 ? (void)0 : (void)

#define DLSURFACE       16
#endif

#endif // FXTRACE

#ifdef DBG
#define ASSERTDD2(x)      ASSERTDD(x,"Assertion failed: " __FILELINE__)
#else
#define ASSERTDD2(x)
#endif



typedef struct _DDGLOBAL 
{
  SstRegs *sst3DRegs;

  int    DD_DebugLevel;
  BOOL   fSingleMonitor;
  int    WaitOnVsync;
  ULONG  overlayFilter;

  ULONG  fbiWidth, fbiHeight;
  ULONG  startTREXMem;                  // SST_TEX_ADDRESS(ghw)
  ULONG  totalSizeTREXMem;
  ULONG  fbiColorModel;                 // RGB or BGR
  ULONG  fxCaps;                        // Capability bits
  ULONG  overlaySurfaceCnt;
  ULONG  visibleOverlaySurf;

  ULONG  clip1min;
  ULONG  clip1max;
  ULONG  clipLeftRight;
  ULONG  clipBottomTop;

  DWORD  bufferUsage;

  DWORD  dd3DInOverlay;                  // Direct3D promoted to overlay?
  DWORD  ddSurfaceFlippedFrom;          // DirectDraw surface flipped from
  DWORD  ddSurfaceFlippedTo;            // DirectDraw surface flipped to
  DWORD  ddAcceleratorUsed;             // DirectDraw application is accelerated

#if (USE_NT5_DDMEMMGR) || !defined(WINNT)
  FXSURFACEDATA* shrinkSurface1;
  FXSURFACEDATA* shrinkSurface2;
  FXSURFACEDATA* txtrSurface1;
  BOOL  f32ShrinkOvl;
#endif

  VIDEO_IN_DATA  VidInData;
  VIDEO_SRC_DATA VidSrcData;
  VIDEO_DST_DATA VidDstData;
  RECT rOvlDst;

#if (_WIN32_WINNT >= 0x0500) || !defined WINNT
  BOOL fUpdateOverlay;
  BANDWIDTH_DATA H3BandWidth;
  VPORTFLIPRECORD gsVportFlip;
#endif

#ifndef WINNT
  BOOL regDebug;
  I2C_DEV I2CDev[MAX_I2C_DEVICES];
#endif

  //V3TV data
#if (_WIN32_WINNT >= 0x0500) || !defined(WINNT)
  DWORD  dwVBIHeight;
  BOOL   fWDMVXDActive;
  BOOL	 bVideoPortActive;
  BOOL	 bUseWDMScaling;
  BOOL	 bIgnoreWDM;
  BOOL	 bVMIPLDpresent;
  BOOL   bVMIPLDinUse;
  DWORD  dwVMIPLDmode;
  RECT	 rWDMSrc;
  DWORD  dwAdjustForEav;
  DWORD  dwSavedSize;
  DWORD  dwSavedStart;
  BOOL	 bScaleWDMinUse;
#endif

#if /*defined(SLI_AA) &&*/ (!defined(WINNT) || (_WIN32_WINNT >= 0x0500))      // commented out - so this code would compile on non Napalm hardware (voodoo 3) - jhunter@3dfx.com

// SLI and AA configuration settings

#define SINGLE_CHIP_NOSLI_AA_DISABLED   0
#define SINGLE_CHIP_NOSLI_AA_2SAMPLE    1
#define DUAL_CHIP_SLI_2WAY_AA_DISABLED  2
#define DUAL_CHIP_SLI_2WAY_AA_2SAMPLE   3
#define DUAL_CHIP_NOSLI_AA_4SAMPLE      4
#define QUAD_CHIP_SLI_4WAY_AA_DISABLED  5
#define QUAD_CHIP_SLI_4WAY_AA_2SAMPLE   6
#define QUAD_CHIP_SLI_2WAY_AA_4SAMPLE   7
#define QUAD_CHIP_NOSLI_AA_8SAMPLE      8

#define SINGLE_CHIP__ABOVE_1280			0
#define ANALOG_SLI_ABOVE_1280				1
#define DIGITAL_SLI_ABOVE_1280			2

  DWORD  ddSLIAAConfiguration;          // SLI/AA configuration
  DWORD  ddSLIAAAnalog;                 // SLI/AA analog (1) versus digital (0)
  DWORD  ddSLIModeEnabled;              // SLI enable flag
  DWORD  ddSLIModeRequested;            // SLI request flag
  DWORD  ddSLINumberScanlines;          // SLI number of scanlines
  DWORD  ddSLINumberWays;               // SLI 2-way (2) versus 4-way (4)
  DWORD  ddAAModeEnabled;               // AA enable flag
  DWORD  ddAAModeRequested;             // AA request flag
  DWORD  ddAANumberSamples;             // AA number of samples
  DWORD  ddAAPrimaryStart;              // AA mode, offset of primary (AA)
  DWORD  ddAAZbufferStart;              // AA mode, offset of zbuffer (AA)
  DWORD  dwlog2BandHeight;              // log2 BandHeight
  DWORD  dwlog2NumChips;                // log2 NumChips
  DWORD  dwMaxHeight;      // Maximum Band Height
  DWORD  dwMaxLinearHeight;   // Maximum Linear Band Height

  DWORD  ddHostcursorAndStart;          // Start of And Bitmap in Host Memory
  DWORD  ddHostcursorXorStart;          // Start of Xor Bitmap in Host Memory
  DWORD  ddHostcursorSrcStart;          // Scratch Buffer
  DWORD  ddHostcursorExclusionStart;    // Save Under
  DWORD  ddHostcursorExclusionSave;     // Old Save Under
  DWORD  ddOldHostcursorAndStart;       // Old Start of And Bitmap in Host Memory
  DWORD  ddOldHostcursorXorStart;       // Old Start of Xor Bitmap in Host Memory
  DWORD  ddOldHostcursorSrcStart;       // Old Scratch Buffer
  DWORD  ddOldHostcursorExclusionStart; // Old Save Under
#endif

#ifdef Z_ACCESS_OPT
  DWORD  ddFlipsWithoutZClear;
  DWORD  ddEnableZClearOpt;
#endif

} DDGLOBAL;

#ifdef SLI_AA
#define DUAL_CHIP_DIGITAL_SLI_CUTOFF (280000000)
#define DUAL_CHIP_DIGITAL_AA_CUTOFF (300000000)
#define DUAL_CHIP_ANALOG_SLI_CUTOFF (320000000)
#define DUAL_CHIP_ANALOG_AA_CUTOFF (320000000)
#endif


#ifdef RD_ABORT_ERROR
#ifndef WINNT
#define MODIFY_SLI_READ(Device, Request) {\
   if (_DD(ddSLIModeEnabled))\
      { \
      if (Request == ENABLE_SLI_READ) \
         _FF(dwSLIMode) = H3VDD_ENABLE_SLI_READ;\
      else \
         _FF(dwSLIMode) = H3VDD_DISABLE_SLI_READ;\
      Modify_SLI_Read(Device, Request);\
      } \
   }
#else
#define ENABLE_SLI_READ     IOCTL_3DFX_ENABLE_SLI_READ
#define DISABLE_SLI_READ    IOCTL_3DFX_DISABLE_SLI_READ
#define MODIFY_SLI_READ(Device, Request) \
        { \
          if (_DD(ddSLIModeEnabled)) \
            Modify_SLI_Read(Device, Request); \
        }
#endif
#else
#ifdef WINNT
#define ENABLE_SLI_READ (0)
#define DISABLE_SLI_READ (0)
#endif
#define MODIFY_SLI_READ(Device, Request)
#endif

#define VidInData  _DD(VidInData)
#define VidSrcData _DD(VidSrcData)
#define VidDstData _DD(VidDstData)
#define rOvlDst    _DD(rOvlDst)

#define MM_DD(a)  (((DDGLOBAL *)(ppdev->pddglobal))->a)

  #if defined(WINNT) || defined(GBLDATA_IN_PDEV)
    #define DD_ENTRY_SETUP(value)   \
      PDEV  *(ppdev);               \
      ppdev = (PDEV *)value->dhpdev;
  #else // win9x
    #ifdef KMVP
        void DoKMTV(NT9XDEVICEDATA * ppdev);
        #define DD_ENTRY_SETUP(value)       \
            NT9XDEVICEDATA * ppdev = (NT9XDEVICEDATA *)value->dwReserved3;\
            ppdev->gdiFlags |= SDATA_GDIFLAGS_2D_DIRTY; \
            if (ppdev->gdiFlags & SDATA_GDIFLAGS_KMTV_FLAG) DoKMTV(ppdev);
    #else
        #define DD_ENTRY_SETUP(value)       \
        NT9XDEVICEDATA * ppdev = (NT9XDEVICEDATA *)value->dwReserved3;\
        ppdev->gdiFlags |= SDATA_GDIFLAGS_2D_DIRTY;
    #endif
  #endif  // winnt

  // Macros to fetch hardware offset based on local surface.

  #ifdef WINNT
    #if ENABLE_RECONFIG_VIDMEM
      #define GET_HW_ADDR(p)       ((FXSURFACEDATA*)((p)->lpGbl->dwReserved1))->hwPtr
      #define GET_AAHW_ADDR(p)     ((FXSURFACEDATA*)((p)->lpGbl->dwReserved1))->AAhwPtr
      #define SET_HW_ADDR(p,addr)  ((FXSURFACEDATA*)((p)->lpGbl->dwReserved1))->hwPtr=addr; 
    #else
      #define GET_HW_ADDR(p)       (p)->lpGbl->fpVidMem
      #define GET_AAHW_ADDR(p)     (p)->lpGbl->fpVidMem
      #define SET_HW_ADDR(p,addr)  (p)->lpGbl->fpVidMem=addr; 
    #endif
  #else
    #define GET_HW_ADDR(p)         ((FXSURFACEDATA*)((p)->lpGbl->dwReserved1))->hwPtr
    #define GET_AAHW_ADDR(p)       ((FXSURFACEDATA*)((p)->lpGbl->dwReserved1))->AAhwPtr
    #define SET_HW_ADDR(p,addr)    ((FXSURFACEDATA*)((p)->lpGbl->dwReserved1))->hwPtr=addr; 
  #endif

  // Macros to fetch hardware offset based on surface handle.

  #if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
    #define GET_HW_OFFSET(h)       ((FXSURFACEDATA*)((TXTRHNDL_PTR(h))->surfData))->hwPtr
    #define GET_AAHW_OFFSET(h)     ((FXSURFACEDATA*)((TXTRHNDL_PTR(h))->surfData))->AAhwPtr
  #endif

  #ifdef WINNT
  #define DD_CHECKDIRTY \
    (ppdev->DriverData.gdiFlags & SDATA_GDIFLAGS_2D_DIRTY)

  #define _DD(Field) (((DDGLOBAL*)(ppdev->DriverData.pddglobal))->Field)
  #define _DS(Field) (ppdev->DriverData.Field)
  #else
  #define DD_CHECKDIRTY \
    (ppdev->gdiFlags & SDATA_GDIFLAGS_2D_DIRTY)

  #define _DD(Field) (((DDGLOBAL*)(ppdev->pddglobal))->Field)
  #define _DS(Field) (ppdev->Field)
  #endif // winnt

  #define FXBUSYWAIT(ppdev)           do{;}while(FXGETBUSYSTATUS(ppdev)!=DD_OK)

  #define BLTFMT(stride, pixFmt, bltDstFormat)                                   \
              (bltDstFormat) =    (pixFmt) |                                     \
              ( ( (stride) << SSTG_DST_STRIDE_SHIFT ) & SSTG_DST_LINEAR_STRIDE ) \

  #define BLTXY(left, top, bltxy) \
              (bltxy) = ( (left) & 0xFFFUL) | ( ( (top) & 0xFFFUL) << 16 )

  #define BLTSIZE(width, height, bltsize) \
              (bltsize) = ( (width) & 0x1FFFUL) | ( ( (height) & 0x1FFFUL) << 16)


  #define BLTCLIP(x, y, clip) \
    (clip) = ( ((y) & 0xFFF) << 16) | ((x) & 0xFFF)

  #define CLIPLEFTRIGHT(left, right, clipLeftRight) \
    (clipLeftRight) = ( ( (left) & 0xFFF ) << SST_CLIPLEFT_SHIFT) \
           | ( ( (right) & 0xFFF ) << SST_CLIPRIGHT_SHIFT)

  #define CLIPBOTTOMTOP(bottom, top, clipBottomTop) \
    (clipBottomTop) = ( ( (bottom) & 0xFFF ) << SST_CLIPBOTTOM_SHIFT) \
           | ( ( (top) & 0xFFF ) << SST_CLIPTOP_SHIFT)

  #define INVALID_PIXELFORMAT 0xFFFF
  #define GETPIXELFORMAT(byte, format)                  \
              switch(byte)                              \
              {                                         \
                case 1:                                 \
                  (format) = SSTG_PIXFMT_8BPP;          \
                break;                                  \
                case 2:                                 \
                  (format) = SSTG_PIXFMT_16BPP;         \
                break;                                  \
                case 3:                                 \
                  (format) = SSTG_PIXFMT_24BPP;         \
                break;                                  \
                case 4:                                 \
                  (format) = SSTG_PIXFMT_32BPP;         \
                break;                                  \
                default:                                \
                  (format) = INVALID_PIXELFORMAT;       \
                break;                                  \
              }
  
  #define OVERLAYDISABLE                           \
      vidProcCfg = GET(ghwIO->vidProcCfg);         \
      vidProcCfg &= ~( SST_OVERLAY_STEREO_EN    |  \
                  SST_USE_ALPHA_BIT             |  \
                  SST_CHROMA_EN                 |  \
                  SST_CHROMA_INVERT             |  \
                  SST_OVERLAY_EN                |  \
                  SST_VIDEOIN_AS_OVERLAY        |  \
                  SST_OVERLAY_CLUT_BYPASS       |  \
                  SST_OVERLAY_CLUT_SELECT       |  \
                  SST_OVERLAY_HORIZ_SCALE_EN    |  \
                  SST_OVERLAY_VERT_SCALE_EN     |  \
                  SST_OVERLAY_FILTER_MODE       |  \
                  SST_OVERLAY_PIXEL_FORMAT      |  \
                  SST_OVERLAY_TILED_EN          |  \
                  SST_OVERLAY_DEINTERLACE_EN    |  \
                  0 );                             \
      SETDW(ghwIO->vidProcCfg, vidProcCfg);

  #ifdef WINNT
  #define GETPRIMARY            ((ULONG)ppdev->ulScreenOffset & ~SSTG_IS_TILED)
  #define GETPRIMARYBYTEDEPTH   ppdev->cjPelSize
  #else
  #define GETPRIMARY ((ULONG) ( (_DS(gdiDesktopStart) & (~SSTG_IS_TILED) )+ _DS(LFBBASE) ))
  #define GETPRIMARYBYTEDEPTH (_FF(bpp) >> 3)
  #endif
  #define GETYUVBYTEDEPTH         16 >> 3;

#define FOURCC_YUY2   0x32595559
#define FOURCC_YUV2   0x32565559
#define FOURCC_Y211   0x31313259
#define FOURCC_UYVY   0x59565955
#define FOURCC_VBPL   0x4C504256 //This is VBPL a 3DFX flavored CLPL, U,V planes has 1024 byte pitch

#if !defined(WINNT) || (_WIN32_WINNT >= 0x0500)
#define SIMULATE_YV12     1
#endif

#define MAKE_FOURCC( ch0, ch1, ch2, ch3 )                       \
        ( (DWORD)(BYTE)(ch0) | ( (DWORD)(BYTE)(ch1) << 8 ) |    \
        ( (DWORD)(BYTE)(ch2) << 16 ) | ( (DWORD)(BYTE)(ch3) << 24 ) )

#define FOURCC_RAW8	    MAKE_FOURCC('R','A','W','8')
#ifdef SIMULATE_YV12
#define FOURCC_YV12     MAKE_FOURCC('Y','V','1','2')
#endif


#undef GETENV
#define GETENV(a) ddgetenv(ppdev,a)

#define GETOVERLAYADDR( x, y, addr)\
    (addr) = ((DWORD)((y) & 0x0fff) << 12)|(DWORD)((x) & 0x0fff)

//use for both win9x and winnt
#if defined(WINNT) && (_WIN32_WINNT < 0x0500)
#define VSYNC_IRQ_DISABLE SETDW(ghw0->intrCtrl, (GET(ghw0->intrCtrl) & ~H3_VSYNC_INT_ENABLE))
#define VSYNC_IRQ_ENABLE  SETDW(ghw0->intrCtrl, (GET(ghw0->intrCtrl) | H3_VSYNC_INT_ENABLE))
#else
int UpdateIMask(NT9XDEVICEDATA * ppdev, DWORD dwIMask);
#define VSYNC_IRQ_DISABLE \
   UpdateIMask(ppdev, GET(ghw0->intrCtrl) & ~H3_VSYNC_INT_ENABLE);\
   SETDW(ghw0->intrCtrl, (GET(ghw0->intrCtrl) & ~H3_VSYNC_INT_ENABLE))
#define VSYNC_IRQ_ENABLE \
    UpdateIMask(ppdev, GET(ghw0->intrCtrl) | H3_VSYNC_INT_ENABLE);\
    SETDW(ghw0->intrCtrl, (GET(ghw0->intrCtrl) | H3_VSYNC_INT_ENABLE))
#endif

// some #define.  Need to think where to put them = SS
#define DR(hwPtr, register)	*((volatile DWORD *)hwPtr + \
        (offsetof(SstRegs, register) / 4))

#ifdef WINNT
#define INVALID_ADDRESS     1
#endif

#ifndef WINNT
#define RESTORE_BUSY(pFlags, flags)\
{\
   _asm  mov   ebx, pFlags\
   _asm  mov   ax, flags\
   _asm  mov   WORD PTR [ebx], ax\
}
#endif

///#ifndef WINNT  NT need this function too
// Remove ifdef WIN_CSIM once the w9x csim toggles the vert retrace status bit for us.
#ifdef WIN_CSIM
#define WAIT_ON_VSYNC()\
  while ( !( (_FF(fakeVerticalBlank)) ^ (_FF(ddMiscFlags) & DDMF_VSYNC_POLARITY_MASK)) )\
  {\
    if(_FF(fakeVerticalBlank))\
    {\
         _FF(fakeVerticalBlank)=0;\
    }\
    else\
    {\
         _FF(fakeVerticalBlank)=1;\
    }\
  }
#else
#define WAIT_ON_VSYNC()\
	if  (!(GET(ghwIO->dacMode) & (SST_DAC_DPMS_ON_VSYNC|SST_DAC_FORCE_VSYNC|SST_DAC_DPMS_ON_HSYNC|SST_DAC_FORCE_HSYNC)))\
  		while ( !((GET(ghwIO->status) & SST_VRETRACE) ^ (_FF(ddMiscFlags) & DDMF_VSYNC_POLARITY_MASK)) )
#endif // WIN_CSIM

#define NUM_DEVICES 16
extern NT9XDEVICEDATA * pDevices[NUM_DEVICES];

#define LEFT_OVERLAYBUF_EVENFIELD   BIT(31)


#ifndef WINNT
#define BUSY 0x10
#define LOW_POWER_MODE(flag) ((((flag) & BUSY) && (0xFFFFFFFF == GET(ghwIO->pllCtrl1))) ? (TRUE) : (FALSE))
#endif

#endif  // __DDGLOBAL_H__
