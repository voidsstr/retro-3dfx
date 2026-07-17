/*
** Copyright (c) 1997-1998, 3Dfx Interactive, Inc.
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
** $Log:
**
**  1    3dfx      1.0         09/12/99 StarTeam VTS Administrator 
*/


#ifndef __H3G_H__
#define __H3G_H__

#ifndef VDDONLY         // the shared vdd defines are at the end of the file

// STB Begin Changes
// STB-SR 1/13/99 Adding code for bj
#ifdef _H2INC
#include "cmddefs.h"
#endif
// STB End Changes

// shared data structure field gdiFlags bit definition
#define SDATA_GDIFLAGS_CLEAN            0
#define SDATA_GDIFLAGS_2D_DIRTY         BIT(1)
#define SDATA_GDIFLAGS_SW_CURSOR        BIT(2)
#define SDATA_GDIFLAGS_HWC_EXCLUSIVE    BIT(3)
#define SDATA_GDIFLAGS_DST_WAS_DEVBIT   BIT(4)
#define SDATA_GDIFLAGS_SRC_WAS_DEVBIT   BIT(5)
#define SDATA_GDIFLAGS_CURSOR_ENABLED   BIT(6)
#define SDATA_GDIFLAGS_CURSOR_EXCLUDE   BIT(7)
#define SDATA_GDIFLAGS_CURSOR_IS_EXCLUDED   BIT(8)
#define SDATA_GDIFLAGS_DIB_CURSOR   BIT(9)
#define SDATA_GDIFLAGS_DIB_DST_CALL   BIT(10)
#define SDATA_GDIFLAGS_DIB_SRC_CALL   BIT(11)
#define SDATA_GDIFLAGS_KMTV_FLAG        BIT(12)
#ifdef SLI_AA
#define SDATA_GDIFLAGS_SLI_AA_MASTER BIT(13)
#define SDATA_GDIFLAGS_SLI_AA_SLAVE BIT(14)
#endif

// STB Begin Changes
#ifndef _H2INC
// STB End Changes

// simple user-level structure for managing the command region
// structure to hold local cache state of cmd Fifos.

typedef struct {
    DWORD outOfOrder;
    long roomToEnd;             // how much room is left until the end
    long roomToReadPtr; // how much room is left before we wait
    DWORD lastReadPtr;          // the last known position of the read ptr
    long size;                  // ring buffer size within total area in dword
    DWORD baseAddr;             // base address offset from chip base
    DWORD *startAddr;           // virtual address of CMD FIFO aperture
    DWORD curP;                 // pointer to current position in the fifo
} FIFOCACHE;

extern FIFOCACHE        fifocache0;
extern FIFOCACHE        *lpfifocache0;
extern SstGRegs *lph3g;
extern SstCRegs *lph3agp;
extern CmdFifo  *lph3cmdfifo0;
extern SstIORegs *lph3IORegs;

extern int dbgmsg;


/***************************************************************************
 ***************************************************************************/

#if !defined(_WIN32) && !defined(WIN32) && !defined(macintosh)

/* Escape functions */
#define MOUSETRAILS             39
#define DCICOMMAND              3075

/* Ternary raster operations */
#define PATCOPY             0x00F00021L
#define SRCCOPY             0x00CC0020L
#define BLACKNESS           0x00000042L
#define WHITENESS           0x00FF0062L

/* Icon/Cursor header */
typedef struct {
    int     xHotSpot, yHotSpot;
    int     cx, cy;
    int     cbWidth;
    BYTE    Planes, BitsPixel;
} CURSORSHAPE;


/***************************************************************************
 *
 * global driver data
 *
 ***************************************************************************/
#ifdef GBLDATA_IN_PDEV
extern GLOBALDATA FAR   *lpDriverData;
#else
extern GLOBALDATA       DriverData;     // in h3.c
#endif

// DDHALMODEINFO is declared in d3dddk\inc\ddrawi.h
//extern MODEINFO    ModeList[];     // in setmode.c

/***************************************************************************
 *
 *
 ***************************************************************************/

void FAR PASCAL _loadds BeginAccess(DIBENGINE FAR *pde, int left, int top, int right, int bottom, UINT flags);
void FAR PASCAL _loadds EndAccess(DIBENGINE FAR *pde, UINT flags);

/***************************************************************************
 *
 * functions in setmode.c
 *
 ***************************************************************************/

BOOL HWSetMode(int ModeNumber);
BOOL HWTestMode(int ModeNumber);
void HWBeginAccess(void);
void HWEndAccess(void);
BOOL HWSetPalette(int start, int count, DWORD FAR *colors);

/***************************************************************************
 *
 * functions in cursor.c
 *
 ***************************************************************************/

BOOL HWSetCursor(CURSORSHAPE FAR *lpCursor);
BOOL HWMoveCursor(int x, int y);

/***************************************************************************
 *
 * functions in DIBENG
 *
 ***************************************************************************/

extern DWORD FAR PASCAL CreateDIBPDevice(LPBITMAPINFOHEADER lpbi,
        DIBENGINE FAR *lpDevice, LPVOID lpBits, WORD dwFlags);

extern UINT  FAR PASCAL DIB_Enable (LPVOID, UINT, LPSTR, LPSTR, LPVOID);

extern UINT  FAR PASCAL DIB_Disable(DIBENGINE FAR *);

extern LONG  FAR PASCAL DIB_Control(DIBENGINE FAR *, UINT, LPVOID, LPVOID);

extern WORD     FAR PASCAL      DIB_DibBltExt(DIBENGINE FAR *, WORD, WORD, WORD,
        LPVOID, LPBITMAPINFO, LPDRAWMODE, LPINT, unsigned short);

extern UINT  FAR PASCAL DIB_BeginAccess     (DIBENGINE FAR *, int left,
        int top, int right, int bottom, WORD flags);

extern UINT  FAR PASCAL DIB_EndAccess       (DIBENGINE FAR *, WORD flags);

extern UINT  FAR PASCAL DIB_SetPaletteExt   (UINT start, UINT count,
        DWORD FAR *lpPalette, DIBENGINE FAR * pde);

extern UINT  FAR PASCAL DIB_BitBlt(DIBENGINE FAR *, WORD, WORD,
        DIBENGINE FAR *, WORD, WORD, WORD, WORD, DWORD, DIB_Brush8 FAR *,
        DRAWMODE FAR *);

extern UINT  FAR PASCAL DIB_StretchBlt (DIBENGINE FAR *, int, int, int,
        int, DIBENGINE FAR *, int, int, int, int, DWORD, DIB_Brush8 FAR *,
        DRAWMODE FAR *,RECT FAR *);

extern UINT  FAR PASCAL DIB_StretchDIBits (DIBENGINE FAR *, int, int,
        int, int, int, int, int, int, int, LPVOID, BITMAPINFO FAR *, LPVOID,
        DWORD, DIB_Brush8 FAR *, DRAWMODE FAR *,RECT FAR *);

extern UINT  FAR PASCAL DIB_DibToDevice(DIBENGINE FAR *, int, int, int,
        int, RECT FAR *, DRAWMODE FAR *, LPVOID, BITMAPINFO FAR *, LPVOID);

extern UINT  FAR PASCAL DIB_Output(DIBENGINE FAR *, int, int, POINT FAR *,
        DIB_Pen FAR *, DIB_Brush8 FAR *, DRAWMODE FAR *, RECT FAR *);

extern UINT  FAR PASCAL DIB_ExtTextOutExt (DIBENGINE FAR *pde, int x, int y,
        RECT FAR *Clip, LPSTR sz, UINT cb, LPVOID lpFont, DRAWMODE FAR *pdm,
        LPVOID xform, int FAR *pdx, RECT FAR *lpORect, UINT f, LPVOID DrawBitmap,
        LPVOID DrawRect);

extern UINT  FAR PASCAL DIB_ExtTextOut (DIBENGINE FAR *pde, int x, int y,
        RECT FAR *Clip, LPSTR sz, int cb, LPVOID lpFont, DRAWMODE FAR *pdm,
        LPVOID xform, int FAR *pdx, RECT FAR *lpORect, UINT f);

extern UINT  FAR PASCAL DIB_SetCursorExt (CURSORSHAPE FAR *lpCursor,
        DIBENGINE FAR *pde);

extern UINT  FAR PASCAL DIB_MoveCursorExt (int x, int y, DIBENGINE FAR * pde);

#endif // #if !defined(_WIN32) && !defined(WIN32)

/***************************************************************************
 *
 * DEBUG stuff
 *
 ***************************************************************************/

#define       DBG_VALIDATEMODE        0
#define       DBG_BITBLT              1
#define       DBG_COLORINFO           2
#define       DBG_CONTROL             3
#define       DBG_DISABLE             4
#define       DBG_ENABLE              5
#define       DBG_ENUMDFONTS          6
#define       DBG_ENUMOBJ             7
#define       DBG_OUTPUT              8
#define       DBG_PIXEL               9
#define       DBG_REALIzEOBJECT       10
#define       DBG_STRBLT              11
#define       DBG_SCANLR              12
#define       DBG_DEVICEMODE          13
#define       DBG_EXTTEXTOUT          14
#define       DBG_GETCHARWIDTH        15
#define       DBG_DEVICEBITMAP        16
#define       DBG_FASTBORDER          17
#define       DBG_SETATTRIBUTE        18
//  DIBBLT              19
#define       DBG_DEVICEBITMAPBITS    19
#define       DBG_CREATEDIBITMAP      20
//  DIBTODEVICE         21
#define       DBG_SETDIBITSTODEVICE   21
#define       DBG_SETPALETTE          22
#define       DBG_GETPALETTE          23
#define       DBG_SETPALETTETRANSLATE 24
#define       DBG_GETPALETTETRANSLATE 25
#define       DBG_UPDATECOLORS        26
#define       DBG_STRETCHBLT          27
#define       DBG_STRETCHDIBITS       28
#define       DBG_SELECTBITMAP        29
#define       DBG_BITMAPBITS          30
#define       DBG_REENABLE            31
#define       DBG_SAVESCREENBITMAP    32
#define       DBG_INQUIRE             33      
#define       DBG_SETCURSOR           34
#define       DBG_MOVECURSOR          35
#define       DBG_CHECkCURSOR         36
#define       DBG_DEBUG               MAX_GDI_FCN-1

// STB Begin Changes
#endif // #ifndef _H2INC
// STB End Changes

// GDI16 bit printing
#define DBGLVL_NORMAL     0x1   //debug level bit flags
#define DBGLVL_MEMMGR     0x2
#define DBGLVL_SSB        0x4
#define DBGLVL_CURSOR     0x8
#define DBGLVL_LCD        0x10
#define DBGLVL_TVOUT      0x20
#define DBGLVL_VPE        0x40
#define DBGLVL_ALL        0xFFFFFFFF

#ifndef LAUNCH_REG_1
#define LAUNCH_REG_1  0X80 >> 2
#endif // #ifndef LAUNCH_REG_1

// cursor defines 
#define CURSOR_BMP_STRIDE (32UL * 4UL)
#define EXCLUSION_BMP_STRIDE (32UL * 4UL)
#define SWCURSOR_SIZE (32UL*32UL*4UL)
#define SWCURSOR_HEIGHT (32UL)
#define SWCURSOR_WIDTH  (32UL)

#define CURSOR_LEFT _FF(CursorPosX)
#define CURSOR_RIGHT _FF(CursorPosX) + SWCURSOR_WIDTH

#define CURSOR_TOP _FF(CursorPosY)
#define CURSOR_BOTTOM _FF(CursorPosY) + SWCURSOR_HEIGHT

// STB Begin Changes
#ifndef _H2INC
// STB End Changes
#ifdef TRACE
  #ifdef THUNK32
    // GDI32 bit printing
    #define DPF dprintf1
    VOID FAR dprintf(char FAR *str,...);
    VOID FAR dprintf1(DWORD GDI_fcn,DWORD dbg_level, char FAR *str,...);
  #else 
    #define DPF Msg
    extern void __cdecl _loadds Msg(DWORD DbgLvl, LPSTR szFormat, ...);
  #endif
#else
    #define DPF         1 ? (void)0 : (void)
#endif

BOOL SetDriverInfo (BOOL reset);
BOOL invokeSetInfo(BOOL reset, DWORD modeidx);

#if !defined(THUNK32) && !defined(macintosh)

// brush styles.
// pain to get from windows.h with some ifdefs.
#ifdef IS_16
#define  BS_SOLID           0
#define  BS_NULL            1
#define  BS_HOLLOW          1
#define  BS_HATCHED         2
#define  BS_PATTERN         3
#define  BS_INDEXED         4
#define  BS_DIBPATTERN      5
#define  BS_DIBPATTERNPT    6
#define  BS_PATTERN8X8      7
#define  BS_DIBPATTERN8X8   8
#endif // #ifdef IS_16
#define HIGHWORD 0xFFFF0000
#define  LOWWORD 0x0000FFFF

/* macros for determining rop components */
/* The following macros RopDst,RopSrc, RopPat uses the high word of
   the 32 bit rop.
        
         The following exerpt explains how the macros work.
        
        P  S  D  |        PSo     DPSoo
        _________|       ______   _____
           |
        0  0  0  |          0       0
        0  0  1  |          0       1
        0  1  0  |          1       1
        0  1  1  |          1       1
        1  0  0  |          1       1
        1  0  1  |          1       1
        1  1  0  |          1       1
        1  1  1  |          1       1

        Operation index:  00FC    00FE


In this case, "PSo" has the operation index 00FC (read from bottom up);
"DPSoo" has the index 00FE.  These values define the location of the
corresponding raster operation code in the following table.  The "PSo"
operation is in line 252 (FC hex) of the table; "DPSoo" in line 254
(hex FE).
*/

//Uses the high word.
// ie  00f00021 patcopy,  macros use 00f0
#define RopUsesSrc(rop) (0 != (0x33 & ( rop ^ (rop >> 2))))
#define RopUsesPat(rop) (0 != (0x0F & ( rop ^ (rop >> 4))))
#define RopUsesDst(rop) (0 != (0x55 & ( rop ^ (rop >> 1))))
#define SRCCOPYROP 0xCC000000

#define CSIM_REG_BASE                           (unsigned long far *) 0x10000000

#define PHYS_SCREEN_BASE_ADDR                   0x0L
#define PHYS_CMDFIFO_BASE_ADDR                  0x02ff000L
#define CMDFIFO_SIZE                                    0xffff
//#define NON_MODAL_LFB                                 0x1000000
#define NON_MODAL_LFB                                   SST_RAW_LFB_OFFSET

// SUGGESTION:  might want to keep lph3g in driver data
extern SstGRegs  * lph3g;
extern SstGRegs h3g;
extern SstCRegs * lph3agp;
extern CmdFifo  * lph3cmdfifo0;

extern int PacketCount;
extern DWORD LastPtr;

extern void far sprintf1(char far *result, char far *str, ...) ;
extern void far dprintf(char far *str,...);
extern void far mysprintf(char far *result, char far * str,int far *param);

extern void  FAR _loadds h3WRITE (unsigned long far * baseaddr,
                                  DWORD * myregs, DWORD data);
extern unsigned long FAR _loadds h3READ(unsigned long far *baseaddr, 
                                        DWORD  * myregs);
extern int setinvariantreg();

extern int setinvariantreg();
extern void _loadds makeRoom(DWORD * cp, unsigned long n);

#define LFBBASE lfbBase
#define REG_BASE 0xffffffff

#ifdef CRASHTEST
extern FxU32 doCT;
#endif // #ifdef CRASHTEST

#define WORDBIT(n)                      (1<<(n))
#define BIT(n)              (1UL<<(n))

// for ddraw heap odd/even tile alignment
#define NOSLOP 0
#define HASSLOP 1


#ifdef STOP
        #define INT_3 __asm int 3;
#else // #ifdef STOP    
        #define INT_3 ;
        
#endif // #ifdef STOP

DWORD VDDCall(DWORD myEAX,      // in
         DWORD VDDmagicNumber, // in
              DWORD myECX,      // in
              DWORD myEDX,      // in
              LPVOID esdi);     // in

#endif // #ifndef THUNK32

// STB Begin Changes
#endif // #ifndef _H2INC
// STB End Changes

#endif // #ifndef VDDONLY

// STB Begin Changes
#ifndef _H2INC
// STB End Changes

typedef enum _vmi_config         //TV-OUT: values for dwVmiTv in global data 
{
   H3VMI_NOT_USED,               //VMI not used
   H3VMI_VIDEO_IN,               //VMI used for video-in
   H3VMI_TV_OUT                  //VMI used for tv-out
   
} VmiConfig;

typedef enum _vdd_codes
{
    H3VDD_GET_HW_INFO,
    H3VDD_SET_VIDEO_MODE,
    H3VDD_GET_FN_TABLE32,
    H3VDD_DISABLE_GDI_DESKTOP,
    H3VDD_I2C_WRITE,
    H3VDD_I2C_READ,
    H3VDD_I2C_MULTI_WRITE,
    H3VDD_AGP_SERVICE,
    H3VDD_GET_BIOS_VERSION,
    H3VDD_GET_FLAT_ADDRESS,
#ifdef SLI_AA
    H3VDD_CLEAR_SLAVE_BIT,
    H3VDD_PCI_OP,
#endif
        H3VDD_TVOUT_MIN,          //TV-OUT
    H3VDD_GET_TVSTATUS = H3VDD_TVOUT_MIN,
    H3VDD_SET_TVSTANDARD,        
    H3VDD_SET_TVCONTROL,
    H3VDD_SET_TVPOSITION,
    H3VDD_SET_TVSIZE,
    H3VDD_SET_SPECIAL,
    H3VDD_GET_TVPOSITION_CTRL,
    H3VDD_GET_TVSIZE_CTRL,
    H3VDD_GET_TVPIC_CTRL,
    H3VDD_GET_FILTER_CTRL,
    H3VDD_DISABLE_TV,
    H3VDD_GET_REGISTRY_INFO,
    H3VDD_SET_REGISTRY_INFO,
    H3VDD_GET_STANDARD,
    H3VDD_GET_SIZE_CAPS,
    H3VDD_GET_POS_CAPS,
    H3VDD_GET_FILTER_CAPS,
    H3VDD_GET_PIC_CAPS,
    H3VDD_GET_TVINIT_STATUS,
    H3VDD_MACROVISION_ON,
    H3VDD_MACROVISION_OFF,
    H3VDD_TVOUT_MAX,
    H3VDD_LINEAR_MAP_OFFSET,
	H3VDD_FLATPNL_PHYSICAL,     // get flat panel width, height and refresh rate
	H3VDD_RW_REGISTER,          // generic io register I/O
	H3VDD_FLATPNL_CTRL,         // enable or disable panel link
   H3VDD_GET_TVNVRAM_STATUS,
   H3VDD_ALLOW_CRT_WITH_TV,    // user wants CRT to stay on when output is TV (user)
   H3VDD_ALLOW_PAL_AND_CRT,     // allow CRT and PAL TV to be active at the same time (system override)
    H3VDD_MOVE_SCREEN,			// jmccartney added these 4 for moving and resizing the screen
	H3VDD_SET_MON_POS_REGISTRY,
	H3VDD_RESTORE_POS,
	H3VDD_RESIZE_SCREEN,
	H3VDD_FLATPNL_EDID,        // return the flat panel edid
    H3VDD_GET_GTF_TIMINGS	   // DYNAMIC MODE TABLE - Calculate GTF timing parameters in 32 bit MiniVDD code

} VddCallCode;


#define IS_AGP_CARD       (0x00000001L)        //Card has AGP capabilities 
#define IS_GART_AVAILABLE (0x00000002L)        //VMM Gart Functions are present
#define IS_AGP_READY      (IS_AGP_CARD|IS_GART_AVAILABLE) //We can do AGP Functions on this card 

#define P6_INTELCPU_WITH_KNI	(0x80000000L)	// for cpuType

#ifdef SLI_AA
#define SLI_AA_REGULAR_DEVICE (0x00)
#define SLI_AA_MASTER_DEVICE (0x01)
#define SLI_AA_SLAVE_DEVICE (0x02)
#define HWINFO_SST_MAX_NUM_CHIPS (0x04)
#else
#define HWINFO_SST_MAX_NUM_CHIPS (0x01)
#endif

#define HWINFO_SST_IOREGS_INDEX (0x00)
#define HWINFO_SST_CMDFIFOREGS_INDEX (0x01)
#define HWINFO_SST_2DREGS_INDEX (0x02)
#define HWINFO_SST_3DREGS_INDEX (0x03)
#define HWINFO_SST_MAX_CHIP_INDEX (0x04)
#define HWINFO_SST_MAX_BASE_INDEX ((HWINFO_SST_MAX_CHIP_INDEX) * (HWINFO_SST_MAX_NUM_CHIPS))

//
// OUT parameter structure of H3VDD_GET_HW_INFO
//
typedef struct _hwinfo
{
#ifdef SLI_AA
    FxU32 dwDevNode;            // New Devnode;
    FxU32 diUnitNumber;         // here is the Unit Number
    FxU32 dwSpecialNumber;      // Index into table
    FxU32 dwType;               // Is this a Master or a Slave
    FxU32 dwNum;                // Number of Units
#endif
    FxU32 regBase[HWINFO_SST_MAX_BASE_INDEX];              // CPU linear address of h/w membase0
    FxU32 lfbBase;              // CPU linear address of h/w membase1
#ifdef WIN_CSIM
    FxU32 RealregBase;              // CPU linear address of h/w membase0
    FxU32 ReallfbBase;              // CPU linear address of h/w membase1
    FxU32 FakeregBase;              // CPU linear address of Fake h/w membase0
    FxU32 FakelfbBase;              // CPU linear address of Fake h/w membase1
#endif
    FxU32 ioBase;               // CPU linear address of h/w iobase (membase2)
    FxU32 memSizeInMB;          // size in megabytes of board memory
    FxU32 VendorDeviceID;       // Vendor Device ID's
    FxU32 RevisionID;           // Revision ID's
    FxU32 PLDRevisionID;
    FxU32 SSID;                 // Subsystem Id's
    FxU32 AGPCaps;              // AGP Capabilities
    FxU32 cpuType;		// CPU type / capabilities
} HwInfo;

#ifndef macintosh
//
// OUT parameter structure of H3VDD_GET_BIOS_VERSION
//
#define MAX_BIOS_VERSION_STRING (32)
typedef struct _hwgetbiosversion
{
    BYTE bBIOSVersion[MAX_BIOS_VERSION_STRING];              // BIOS Version String
} HWGETBIOSVERSION;
#endif

typedef struct _hwgetflataddress
{
   FxU32 * pData;
   DWORD FlatAddress; 
} HWGETFLATADDRESS;

#ifdef SLI_AA
#define H3G_PCI_READ 0x00
#define H3G_PCI_WRITE 0x01
typedef struct _hwpciop {
   DWORD dwOp;             // Operation Read or Write?
   DWORD dwFunc;           // Which Unit?
   DWORD dwOffset;         // Offset [0..255]
   DWORD dwValue;          // Value to Read into or Write from
} HWPCIOP;
#endif

//
// IN parameter sub-structure of H3VDD_SET_VIDEO_MODE
//
typedef struct _dtSurface
{
    FxU32 enable;               // 1=enable desktop surface (DS), 1=disable
    FxU32 tiled;                // 0=DS linear, 1=tiled
    FxU32 pixFmt;               // pixel format of DS (vidProcCfg register)
    FxU32 clutBypass;           // bypass clut for DS?
    FxU32 clutSelect;           // 0=lower 256 CLUT entries, 1=upper 256
    FxU32 startAddress;         // board address of beginning of DS
    FxU32 stride;               // distance between scanlines of the DS, in
#ifdef H3_B0
    FxU32 scanlinedouble;       // B0 Scanline Double Mode
#endif
} dtSurface;


//
// IN parameter sub-structure of H3VDD_SET_VIDEO_MODE
//
typedef struct _ovSurface
{
    FxU32 enable;               // 1=enable Overlay surface (OS), 1=disable
    FxU32 stereo;               // 1=enable OS stereo, 0=disable
    FxU32 horizScaling;         // 1=enable horizontal scaling, 0=disable
    FxU32 dudx;                 // horizontal scale factor (ignored if not
                                // scaling)
    FxU32 verticalScaling;      // 1=enable vertical scaling, 0=disable
    FxU32 dvdy;                 // vertical scale factor (ignored if not
                                // scaling)
    FxU32 filterMode;           // duh
    FxU32 tiled;                // 0=OS linear, 1=tiled
    FxU32 pixFmt;               // pixel format of OS
    FxU32 clutBypass;           // bypass clut for OS?
    FxU32 clutSelect;           // 0=lower 256 CLUT entries, 1=upper 256
    FxU32 startAddress;         // board address of beginning of OS
    FxU32 stride;               // distance between scanlines of the OS, in

} ovSurface;

//=======================================================================================
// DYNAMIC MODE TABLE
//=======================================================================================

#define CRTC_TABLE_SIZE				24

//
// Parameter Structure for CRTC
//
typedef struct _TIMING_PARAMS
{
    FxU32 width;                // width in pixels
    FxU32 height;               // height in pixels
    FxU32 refresh;              // refresh rate in Hz
    FxU32 HTotal;    			// Make sure to use only 32 bit variables in this or any
    FxU32 HSyncStart;    		// structure else the Mini VDD may read them incorrectly 
    FxU32 HSyncEnd;    
    FxU32 VTotal;    
    FxU32 VSyncStart;    
    FxU32 VSyncEnd;    
    FxU32 CRTCflags;    
    FxU32 PixelClock;    
    FxU32 CharWidth;    
    FxU32 UseGTF;    
    FxU32 UseAltTiming;    
    FxU32 Checksum; 
    FxU8  AltTiming[CRTC_TABLE_SIZE];   			

} TIMING_PARAMS;

#define HTOTAL_FIELD				1
#define HSYNCSTART_FIELD   			2
#define HSYNCEND_FIELD   			3
#define VTOTAL_FIELD				4
#define VSYNCSTART_FIELD   			5
#define VSYNCEND_FIELD   			6
#define CRTCFLAG_FIELD   			7
#define PIXCLOCK_FIELD   			8
#define REFRESH_FIELD   			9
#define CHARWIDTH_FIELD   			10
#define CHECKSUM_FIELD   			11

#define ALT_CHECKSUM_FIELD   		CRTC_TABLE_SIZE - 2

#define ALT_HTOTAL					0
#define ALT_MISC_OUTPUT				16
#define ALT_SR1						17
#define ALT_PLLCTRL0_LOW			18
#define ALT_PLLCTRL0_HIGH			19
#define ALT_DACMODE					20

#define BPP_8_CAP_FLAG				0x00000002
#define BPP_16_CAP_FLAG				0x00000004
#define BPP_24_CAP_FLAG				0x00000008
#define BPP_32_CAP_FLAG				0x00000010

#define DDRAW_CAPABLE 				0x00000100
#define TVOUT_DESKTOP_CAPABLE 		0x00000200
#define TVOUT_DDRAW_CAPABLE 		0x00000400
#define DFP_DESKTOP_CAPABLE 		0x00000800
#define DFP_DDRAW_CAPABLE 			0x00001000
#define NTSC_CAPABLE 				0x00002000
#define PAL_CAPABLE 				0x00004000

#define DESKTOP_CAPABLE 			0x40000000
#define ALL_MODES_CAPABLE 			0x80000000

#define DESKTOP_CAPABLE_STR 		"DESKTOP"
#define DDRAW_CAPABLE_STR 			"DDRAW"
#define TVOUT_DESKTOP_CAPABLE_STR 	"TVO_DESK"
#define TVOUT_DDRAW_CAPABLE_STR 	"TVO_DD"
#define DFP_DESKTOP_CAPABLE_STR 	"DFP_DESK"
#define DFP_DDRAW_CAPABLE_STR 		"DFP_DD"
#define NTSC_CAPABLE_STR 			"NTSC"
#define PAL_CAPABLE_STR 			"PAL"
#define BPP_CAP_STR 				"BPP"

#define TIMINGS_KEY_STR 			"\\TIMINGS"
#define TIMINGS_VALUE_STR 			""
#define DFP_TIMINGS_VALUE_STR 	   	"AltDFP"
#define SUPPORT_VALUE_STR 			"Supported"
#define HERTZ_KEY_STR				"Hz"
#define USE_GTF_VALUE_STR 			"UseGTF"
#define OPT_REFRESH_LIMIT_STR 		"OptimalRefreshLimit"
#define OPT_NONEDID_LIMIT_STR 		"OptimalNonEDIDLimit"

#define MAX_EDID_LIMIT				-2
#define MAX_REFRESH_RATE			255
#define MAX_NONEDID_RATE			85
#define MIN_DESKTOP_WIDTH			640
#define MIN_DESKTOP_HEIGHT			480

#define DEFAULT_CHAR_WIDTH			8
// Default CRTC Flags = NON-DOUBLE SCANNED, NON-INTERLACED, H SYNC POLARITY NEGATIVE, V SYNC POSITIVE. 
#define DEFAULT_CRTC_FLAGS			4
		
#define DECIMAL 					10
#define HEX 						16

#define MAX_NUMERIC_STRING_SIZE 	36
#define MAX_TIMING_STRING_SIZE 		132
#define MAX_CAP_STRING_SIZE 		132
#define	MAX_TIMINGS_REG_KEY_LEN 	MAX_VMM_REG_KEY_LEN + sizeof(TIMINGS_KEY_STR)

//=======================================================================================
// END DYNAMIC MODE TABLE
//=======================================================================================
// IN parameter structure of H3VDD_SET_VIDEO_MODE
//
typedef struct _vid_Proc_Config
{
#ifdef SLI_AA
   FxU32 diUnitNumber;          // Do Unit Number 
#endif
//    FxU32 changeVideoMode;      // change video mode timings
    FxU32 width;                // width in pixels
    FxU32 height;               // height in pixels
    FxU32 refresh;              // refresh rate in HZ
//    FxU32 changeDesktop;        // change current desktop surface settings?
//    dtSurface desktopSurface;   // new desktop surface data
//    FxU32 changeOverlay;        // change current overlay surface settings?
//    ovSurface overlaySurface;   // new overlay surface data
    TIMING_PARAMS TimingParams;	// DYNAMIC MODE TABLE
} VidProcConfig;

#ifdef SLI_AA
// GLOBALDATA Information that we need to save when we enter SLI/AA mode
typedef struct _conf_save {
    FxU32               fifoStart;
    FxU32               fifoSize;
    DWORD               deBitsOffset;
} CONF_SAVE, * PCONF_SAVE;

typedef struct _sli_aa_chipinfo {
   DWORD dwChips;       // dwChips: Number of chips in multi-chip configuration (1-4)
   DWORD dwsliEn;       // dwsliEn: Sli is to be enabled (0,1)
   DWORD dwaaEn;        // dwaaEn: Anti-aliasing is to be enabled (0,1)
   DWORD dwaaSampleHigh;  // dwaaSampleHigh: 0->Enable 2-sample AA, 1->Enable 4-sample AA
   DWORD dwsliAaAnalog;   // dwsliAaAnalog: 0->Enable digital SLI/AA, 1->Enable analog Sli/AA
   DWORD dwsli_nlines;    // dwsli_nLines: Number of lines owned by each chip in SLI (2-128)
   DWORD dwCfgSwapAlgorithm; // Swap Buffer Algorithm
}  SLI_AA_CHIPINFO, * PSLI_AA_CHIPINFO;

typedef struct _sli_aa_meminfo {
   DWORD dwTotalMemory;       // In MB
   DWORD dwTileMark;          // In MB
   DWORD dwTileCmpMark;       // In MB
   DWORD dwaaSecondaryColorBufBegin;    // In MB
   DWORD dwaaSecondaryDepthBufBegin;    // In MB
   DWORD dwaaSecondaryDepthBufEnd;    // In MB
   DWORD dwBpp;                        // Bits Per Pixel
}  SLI_AA_MEMINFO, * PSLI_AA_MEMINFO; 

typedef struct sli_aa_request {
   SLI_AA_CHIPINFO ChipInfo;
   SLI_AA_MEMINFO MemInfo;
} SLI_AA_REQUEST, * PSLI_AA_REQUEST;

#endif

typedef enum _agp_calls
{
    RESERVE,
    COMMIT,
    UNCOMMIT,
    FREE
} AgpService_t;

//
// IN/OUT parameter structure of H3VDD_AGP_SERVICE
//
typedef struct _agp_service
{
    FxU32 service;

    union _params
    {
        struct 
        {
            FxU32 devNode;      // IN: devnode of graphics device 
            FxU32 nPages;       // IN: # of pages to reserve
            FxU32 gartLinAddr;  // OUT: base cpu addr of gart reserved range
            FxU32 gartPhysAddr; // OUT: base gart phys addr to be mapped ...
                                //      ... to gartLinAddr
        } reserve;

        struct 
        {
            FxU32 gartLinAddr;  // IN: cpu base lin addr to commit
            FxU32 pageOffset;   // IN: offset from base to begin commit
            FxU32 nPages;       // IN: # of pages to commit
            FxU32 gartPhysAddr; // OUT: gart phys addr mapped to gartLinAddr
            FxU32 retval;       // OUT: 0 = failure, non-0 = success
        } commit;

        struct
        {
            FxU32 gartLinAddr;  // IN: cpu base lin addr to uncommit
            FxU32 pageOffset;   // IN: offset from base to begin uncommit
            FxU32 nPages;       // IN: # of pages to uncommit
        } uncommit;

        struct
        {
            FxU32 gartLinAddr;  // IN: cpu base lin addr to free
        } free;
    } params;
} AgpSrvc;

typedef struct {
  FxU32
    mapAddr,
    remapAddr;
} MapInfo;

// IN - OUT parameter for H3VDD_FLATPNL_PHYSICAL
// CAUTION: this is mirrored in qmodes.h!!!

typedef struct {
	FxU32 maxWidth;
	FxU32 maxHeight;
	FxU32 refreshRate;
	FxU32 fpFlags;
}  FlatPnlPhysical;

#define FPFLAG_ENABLED            BIT(0)   // is currently enabled (OUT)
#define FPFLAG_PRESENT            BIT(1)   // panel & controller connected (OUT)
#define FPFLAG_PRESENT_AT_BOOT    BIT(2)   // panel & controller connected 
										   // at boot time (OUT)
#define FPFLAG_ENABLE             BIT(3)   // try to enable panel (IN)
#define FPFLAG_DISABLE            BIT(4)   // try to disable panel (IN)
#define FPFLAG_GET_DIMS           BIT(5)   // read the EDID to get panel's
										   // physical dimensions (IN)
#define FPFLAG_SETRES             BIT(6)   // current desktop resolution too hi
                                           // (OUT)
#define FPFLAG_SETREFR            BIT(7)   // current refresh rate wrong  (OUT)

// filter bits
// 00   bilinear    01   gausian
// 10   crisp       11   extra crisp
#define FPFLAG_FILTER_SOFT        BIT(8)   // use these 2 bits together (IN)
#define FPFLAG_FILTER_HARD        BIT(9)
#define FPFLAG_CAPABLE            BIT(10)  // board has controller & BIOS

#define FPFLAG_SCALING            BIT(11)  // panel can scale

// IN - OUT parameter for H3VDD_RW_REGISTER

typedef struct
{
	FxU32 regRWflags;
	FxU32 regAddr;
	FxU32 regValue;
}  SstIoRegRW;

#define RWFLAG_WRITE_REG          BIT(0)   // write regValue to regAddr (IN)
#define RWFLAG_WRITE_BYTE         BIT(1)   // write byte



#define UPDATE_IMASK (1)
#define AGP_WARMUP (2)
#define GET_DIB_ADDR (3)
#define VIDEO_SYS_BUF (4)
#define VDD_V3TV_WDMSCALE (5)

#ifdef SLI_AA
#define SLI_AA_ENABLE (6)
#define SLI_AA_DISABLE (7)
#endif
#define VDD_EXTRN_PLD (8)

#define VDD_V3TV_WDMQUERYACTIVE (9)
#define VDD_VMI_FUNCTION (10)

#define SHRINKBLT_SIZE  17

//KMVT command fifo buffer
typedef struct{
    DWORD dwSize;
    DWORD data[SHRINKBLT_SIZE+1];
    DWORD dwStatus;
    DWORD dwVPAddr0;
    DWORD dwVPAddr1;
    DWORD dwVPAddr2;
    DWORD dwVPStride;
    DWORD dwVBILines;
	
	// V3TV
    DWORD fpVidMem[3];
    DWORD dwVideoLines;			// video data
	DWORD dwVideoLinesOrig;
    DWORD dwVideoSurfacePitch;	// surface pitch
	DWORD dwVBILinesOrig;
    DWORD dwVidInFormat;
    DWORD dwCounter;
	DWORD dwDataWidth;
	DWORD dwDataHeight;

	//PLD 656 variables
    BOOL bVMIPLDinUse;
    BOOL bLastFieldEven;
    BOOL bLastIntrVBI;
    BOOL bVBIcropped;
    DWORD dwLastBufferFilled;
    BOOL  bSawVBIintr;

	// other need info
	BOOL  bDoublePitch;
	DWORD dwPLDVidLines;
	BOOL  bUsingEAV;
	BOOL  bUseReset;

} KMVTBUFF;
//value for dwStatus
#define BUFF1_IN_USE    0x0000
#define BUFF2_IN_USE    0x0001 
#define BUFF3_IN_USE    0x0002
#define BUFFER_IN_USE_MASK    0x3
#define ONE_BUFFER      0x0100 
#define TWO_BUFFERS     0x0200 
#define THREE_BUFFERS   0x0300
#define VP_BUFF_MASK    0x0300
#define VP_BUFF_SHIFT   8 

#define BOB_ON          0x10000
#define INTERLEAVE_ON   0x20000

typedef struct dioc_data {
   DWORD dwDevNode;           // Device that we are interested in
   DWORD dwSpare;             // Spare DWORD currently used to pass IMASK
   } DIOC_DATA, * PDIOC_DATA;

// STB Begin Changes
#endif //#ifndef _H2INC
// STB End Changes



// DEVICE IDS

#define SST_VENDOR_DEVICE_ID_H4        0x0005121a // Voodoo3
#define SST_VENDOR_DEVICE_ID_H5_6      0x0006121a // Napalm
#define SST_VENDOR_DEVICE_ID_H5_7      0x0007121a // Napalm
#define SST_VENDOR_DEVICE_ID_H5_8      0x0008121a // Napalm
#define SST_VENDOR_DEVICE_ID_H5_9      0x0009121a // Napalm
#define SST_VENDOR_DEVICE_ID_H5_A      0x000A121a // Napalm2
#define SST_VENDOR_DEVICE_ID_H5_B      0x000B121a // Napalm2
#define SST_VENDOR_DEVICE_ID_H5_C      0x000C121a // Napalm2
#define SST_VENDOR_DEVICE_ID_H5_D      0x000D121a // Napalm2
#define SST_VENDOR_DEVICE_ID_H5_E      0x000E121a // Reserved
#define SST_VENDOR_DEVICE_ID_H5_F      0x000F121a // Reserved

// DEVICE IDENTIFICATION MACROS

#ifndef _H2INC

#ifndef THUNK32

#ifndef macintosh

// display driver, DirectDraw, Direct3D version

#define IS_VOODOO3 \
        (_FF(VendorDeviceID) == SST_VENDOR_DEVICE_ID_H4)

#ifdef WIN_CSIM

#define IS_NAPALM 1

#else  // WIN_CSIM

#define IS_NAPALM \
 ((_FF(VendorDeviceID) == SST_VENDOR_DEVICE_ID_H5_6) || \
  (_FF(VendorDeviceID) == SST_VENDOR_DEVICE_ID_H5_7) || \
  (_FF(VendorDeviceID) == SST_VENDOR_DEVICE_ID_H5_8) || \
  (_FF(VendorDeviceID) == SST_VENDOR_DEVICE_ID_H5_9) || \
  (_FF(VendorDeviceID) == SST_VENDOR_DEVICE_ID_H5_A) || \
  (_FF(VendorDeviceID) == SST_VENDOR_DEVICE_ID_H5_B) || \
  (_FF(VendorDeviceID) == SST_VENDOR_DEVICE_ID_H5_C) || \
  (_FF(VendorDeviceID) == SST_VENDOR_DEVICE_ID_H5_D) || \
  (_FF(VendorDeviceID) == SST_VENDOR_DEVICE_ID_H5_E) || \
  (_FF(VendorDeviceID) == SST_VENDOR_DEVICE_ID_H5_F))

#endif // WIN_CSIM

#else

// macintosh

#define IS_VOODOO3(_id) \
  ((_id == 4) || (_id == 5))

#define IS_NAPALM(_id) \
  ((_id == 6) || (_id == 7) || (_id == 8) || (_id == 9))  
  

#endif

#else  // THUNK32

// minivdd version

#define IS_VOODOO3(deviceID) \
        ((deviceID) == SST_VENDOR_DEVICE_ID_H4)

#ifdef WIN_CSIM

#define IS_NAPALM(deviceID) 1

#else  // WIN_CSIM

#define IS_NAPALM(deviceID) \
 (((deviceID) == SST_VENDOR_DEVICE_ID_H5_6) || \
  ((deviceID) == SST_VENDOR_DEVICE_ID_H5_7) || \
  ((deviceID) == SST_VENDOR_DEVICE_ID_H5_8) || \
  ((deviceID) == SST_VENDOR_DEVICE_ID_H5_9) || \
  ((deviceID) == SST_VENDOR_DEVICE_ID_H5_A) || \
  ((deviceID) == SST_VENDOR_DEVICE_ID_H5_B) || \
  ((deviceID) == SST_VENDOR_DEVICE_ID_H5_C) || \
  ((deviceID) == SST_VENDOR_DEVICE_ID_H5_D) || \
  ((deviceID) == SST_VENDOR_DEVICE_ID_H5_E) || \
  ((deviceID) == SST_VENDOR_DEVICE_ID_H5_F))

#endif // WIN_CSIM

#endif // THUNK32

#endif // _H2INC

#endif //#ifdef __H3G_H__
