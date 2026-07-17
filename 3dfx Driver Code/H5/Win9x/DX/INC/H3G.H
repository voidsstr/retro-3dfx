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
** $
**
** 
** 23    9/08/99 4:21p Cwilcox
** Added protection for H2INC, after merge with \devel\h5\incsrc.
** 
** 22    9/03/99 10:36a Xingc
** Add VDD_VMI_FUNCTION IoControl code
** 
** 21    8/30/99 4:05p Cwilcox
** Added additional Napalm device IDs.
** 
** 20    8/25/99 2:13p Lpost
** V3TV code merge into H5
** 
** 19    8/07/99 3:10p Rbissell
** tvout merge from V3_OEM_100
** 
** 18    8/06/99 4:39p Andrew
** Added Glide PCI Function
** 
** 17    7/28/99 3:48p Andrew
** Changed some comments
** 
** 16    7/26/99 5:50p Andrew
** Removed some unused members from SaveMemory
** 
** 15    7/16/99 4:34p Cwilcox
** Removed obsolete heap*Start variables.
** 
** 14    7/16/99 12:50p Andrew
** Added code to pass through 4 pointers to our base address instead of
** one
** 
** 13    7/09/99 8:22a Andrew
** Added a new structure to pass the Chip Info and Memory Info to the
** Mini-Vdd from DD32
** 
** 12    7/08/99 4:57p Cwilcox
** Remove unused Napalm device IDs.
** 
** 
** 11    7/08/99 1:05p Cwilcox
** Added runtime checking for Napalm versus Voodoo3.
** 
** 10    7/07/99 2:34p Andrew
** Added New Napalm Device ID's and new pointer for DibEngine work around
** 
** 9     7/01/99 6:38p Dalev
** Added 'VDD_EXTRN_PLD' control code.
** 
** 8     6/25/99 4:24p Andrew
** Added a Define for Regular Device
** 
** 7     6/25/99 9:22a Andrew
** Added new structure to save memory configuration
** 
** 6     6/22/99 5:17p Cwilcox
** Added device identification macros and defines.
** 
** 5     6/17/99 3:17p Andrew
** Removed HAL_CSIM
** 
** 4     6/15/99 5:16p Andrew
** Added defines and structures for keeping track of SLI Master and Slave
** in DD16 code
** 
** 3     6/04/99 4:15p Andrew
** Added new members to structures to support SLI/AA
** 
** 2     6/03/99 9:04a Andrew
** Added SLI_MASTER and SLI_SLAVE defines
** 
** 1     6/02/99 6:52a Michael
** Branch from h3
** 
** 73    5/17/99 1:41p Stb_lpost
** V3TV Video Capture fixes for E3 Demo
** 
** 72    5/14/99 1:33p Andrew
** Added Fake/Real MemBase0,1 to make managing Real/Fake Address easier to
** manage in WIN_SIM
** 
** 71    5/13/99 4:12p Andrew
** Changed Real on lfb to more accurate reflect it's use
** 
** 70    5/06/99 4:29p Andrew
** Added code for new Windows "C" Simulator
** 
** 69    5/03/99 2:56p Stb_lpost
** V3TV VBI fixes for VBI capture
** 
** 68    4/26/99 7:58a Stb_lpost
** V3TV - VBI Capture and Connection fixes
** 
** 67    4/08/99 5:20p Xingc
** Add several varaibles in KMVTBUFF structure for BOB_INTERLEAVE support
** 
** 66    4/06/99 12:41a Andrew
** Added structure to MAP_FLAT UnGlideContext
** 
** 65    3/31/99 10:47a Pratt
** added debug levels for TVout, LCD and VPE
** 
** 64    3/22/99 5:39p Xingc
** Change KMTV_SYNC into GET_DIB_ADDR
** 
** 63    2/26/99 3:23p Xingc
** Add  KMVTBUFF structure to deal with two different commands
** 
** 62    2/25/99 7:59p Xingc
** Add SDATA_GDIFLAGS_KMTV_FLAG for KMVT.
** 
** 61    2/25/99 12:49p Stuartb
** Added FPFLAG_CAPABLE.
** 
** 60    2/18/99 2:55p Stuartb
** Added xlcd FPFLAGS for filter mode setting.
** 
** 59    2/18/99 11:47a Stuartb
** Added FPFLAGS bits to select xlcd filter configuration.
** 
** 58    2/09/99 9:54a Stb_srogers
** 
** 57    2/08/99 2:10p Ken
** added cpu/OS detection of pentium III (katmai) processors, and added
** katmai-optimized d3d texture download
** 
** 56    1/21/99 5:34p Andrew
** Added some Device IO Ctl Functions
** 
** 55    1/18/99 2:06p Adrians
** Removed obsolete variables and associated code.
** 
** 54    1/14/99 3:45p Stuartb
** Defined H3VDD_FLATPNL_CTRL.
** 
** 53    1/12/99 2:35p Stuartb
** Added H3VDD_RW_REGISTER, helper call.
** 
** 52    1/06/99 5:09p Stuartb
** Changed misleading define H3_VDD_GET_BIOS_SCRATCH_REG to something
** meaningful.
** 
** 51    1/05/99 10:46a Andrew
** Added new service and new structure for get the bios version
** 
** 50    12/18/98 5:30p Stuartb
** Added definest for minivdd call to get flat panel description.
** 
** 49    11/30/98 6:52p Peter
** query for possibly re-mapped base address
** 
** 48    11/24/98 8:31a Andrew
** Added AGPCaps and flags to hwinfo
** 
** 47    11/16/98 8:35p Andrew
** Modified HwInfo to have PCI[0] PCI[8] and PCI[2C].
** 
** 46    11/11/98 9:32a Stuartb
** First cut at tvout copy protection.  Will only work under Win98.
** 
** 45    9/16/98 11:06a Stuartb
** Added additional tvout functionality.
** 
** 44    9/10/98 2:42p Stuartb
** TVOUT work in progress.
** 
** 43    8/27/98 11:35a Artg
** h4 csim support
** 
** 41    7/27/98 1:26p Andrew
** Added a field to dtSurface to set Half Mode bit in vidProccfg and
** double Y in vidScreenSize
** 
** 40    7/23/98 4:04p Ken
** added agp command fifo.   not added to NT build.  currently not
** functional on non-win98 systems without running a special enable apg
** script first, see Ken for that.   agp command fifo is enabled by
** setting the environment variable ACF=1 , all other settings disable it.
** Only turned on interatively in debugger in InitFifo call.   
** 
** 39    6/23/98 10:13a Stuartb
** Added I2C multibyte writes.
** 
** 38    5/27/98 8:44a Andrew
** Added new bits
** 
** 37    5/19/98 6:08p Andrew
** New bit defines
** 
** 36    5/05/98 1:20p Stuartb
** Adding defines for i2c functions.
** 
** 35    5/01/98 4:25p Artg
** sw cursor stuff
** 
** 34    4/28/98 4:53p Artg
** added define dbglvl_cursor = 0x8
** 
** 33    4/28/98 3:54p Michael
** change Msg() to include a new parameter that will allow for selective
** debug message output.
** 
** 32    4/27/98 7:40p Suninn
** add NOSLOP HASSLOP #defines
** 
** 31    4/27/98 7:48a Ken
** fixed multi-defined launch_reg_1 warning
** 
** 30    4/26/98 9:05p Artg
** forgot swcursor_size
** 
** 29    4/26/98 6:01p Artg
** added defines for swcursor support
** 
** 
** 28    4/24/98 1:33p Ken
** first installment of FXENTER / FXLEAVE display driver drawing function
** work.  gdi32 only, no gdi16 yet.   works, but is slow, don't measure
** performance until all work is done, in about a week.
** also renamed fields in some blit parameter functions to be a bit more
** meaningful
** 
** 27    4/22/98 5:26p Dow
** Added gdi bit for hwc exclusive mode.
** 
** 26    4/22/98 2:56p Andrew
** Removed unneeded global reference
** 
** 25    4/03/98 10:50a Artg
** added DBG_DEBUG to function signature list
** 
** 24    4/02/98 10:51p Artg
** put support for user controlable debug level.
** 
** 23    3/31/98 11:42a Miriam
** 16/32 bit print macro definition.
*/
// WARNING, WARNING WILL ROBINSON
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

#if !defined(_WIN32) && !defined(WIN32)

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

#ifndef THUNK32

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
    H3VDD_GET_OEM_BOARD_NAME,
    H3VDD_GET_CHIP_NAME,
    H3VDD_GET_FLAT_ADDRESS,
#ifdef SLI_AA
    H3VDD_CLEAR_SLAVE_BIT,
    H3VDD_PCI_OP,
#endif
    H3VDD_INITFIFO,
    H3VDD_ENABLE_SLI_READ,
    H3VDD_DISABLE_SLI_READ,
    H3VDD_SLI_READ_NOT_IN_USE,
    H3VDD_AGP_FORCE_RATE,

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
	H3VDD_DFP_REQUEST,          // generic DFP interface
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
    H3VDD_GET_GTF_TIMINGS,	   // DYNAMIC MODE TABLE - Calculate GTF timing parameters in 32 bit MiniVDD code
    H3VDD_SET_ALT_DISPLAYS

} VddCallCode;


#define IS_AGP_CARD       (0x00000001L)        //Card has AGP capabilities 
#define IS_GART_AVAILABLE (0x00000002L)        //VMM Gart Functions are present
#define IS_PRIMARY_DISPLAY (0x00000004L)        //On Banshee AGP would not work a one Gateway if it was not Primary
#define IS_AGP_READY      (IS_AGP_CARD|IS_GART_AVAILABLE|IS_PRIMARY_DISPLAY) //We can do AGP Functions on this card 

#ifdef WINNT	// OLD CPUID CODE
#define P6_INTELCPU_WITH_KNI	(0x80000000L)	// for cpuType
#endif

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
    FxU32 cpuType;	        	// CPU type / capabilities
	FxU32 dwVIACoreLogic; 	    // VIA Core Logic flags
} HwInfo;

//
// OUT parameter structure of H3VDD_GET_BIOS_VERSION
//
#define MAX_BIOS_VERSION_STRING (32)
typedef struct _hwgetbiosversion
{
    BYTE bBIOSVersion[MAX_BIOS_VERSION_STRING];              // BIOS Version String
} HWGETBIOSVERSION;

typedef struct _hwgetoemboardname
{
    BYTE bOEMBoardName[MAX_BIOS_VERSION_STRING];              	// OEM board name String
} HWGETOEMBOARDNAME;

typedef struct _hwgetchipname
{
    BYTE bChipName[MAX_BIOS_VERSION_STRING];              	// Chip name String
} HWGETCHIPNAME;

typedef struct _hwgetflataddress
{
   FxU32 * pData;
   DWORD FlatAddress; 
} HWGETFLATADDRESS;

typedef struct _hwagpforcerate {
   DWORD dwRate;
   } HWAGPFORCERATE;


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
    FxU32 UseAltTiming;    //Note: use of altTimings for DFPs is obsolete. DanO 6/7/00
    FxU32 Checksum; 
#ifdef REAL_NET
    FxU32 refresh100;
#endif
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
    FxU32 changeVideoMode;      // change video mode timings
    FxU32 width;                // width in pixels
    FxU32 height;               // height in pixels
    FxU32 refresh;              // refresh rate in HZ
    FxU32 changeDesktop;        // change current desktop surface settings?
    dtSurface desktopSurface;   // new desktop surface data
    FxU32 changeOverlay;        // change current overlay surface settings?
    ovSurface overlaySurface;   // new overlay surface data
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
#define ENABLE_SLI_READ (11)
#define DISABLE_SLI_READ (12)
#define HOT_KEY_FLAG (13)
#define HOT_KEY_OFF_FLAG (14)
#ifdef REAL_NET
#define CLEAR_VCOUNT (1000)
#define DISABLE_VCOUNT ((CLEAR_VCOUNT)+1)
#define ENABLE_VCOUNT ((CLEAR_VCOUNT)+2)
#define RETURN_VCOUNT ((CLEAR_VCOUNT)+3)
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
   DWORD dwModifier;          // Spare Modifier for Toggle Key
   DWORD dwOffset;			  // Offset (used for HOT_KEY)
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

#define SST_VENDOR_DEVICE_SSID_H5_1    0x0001121a // ???
#define SST_VENDOR_DEVICE_SSID_H5_2    0x0002121a // Napalm 5500 AGP
#define SST_VENDOR_DEVICE_SSID_H5_3    0x0003121a // Napalm 5500 PCI
#define SST_VENDOR_DEVICE_SSID_H5_4    0x0004121a // Napalm 4500 AGP
#define SST_VENDOR_DEVICE_SSID_H5_5    0x0005121a // Napalm 4500 PCI
#define SST_VENDOR_DEVICE_SSID_H5_6    0x0006121a // ???
#define SST_VENDOR_DEVICE_SSID_H5_7    0x0007121a // ???
#define SST_VENDOR_DEVICE_SSID_H5_8    0x0008121a // ???
#define SST_VENDOR_DEVICE_SSID_H5_9    0x0009121a // ???
#define SST_VENDOR_DEVICE_SSID_H5_A    0x000A121a // ???
#define SST_VENDOR_DEVICE_SSID_H5_B    0x000B121a // ???
#define SST_VENDOR_DEVICE_SSID_H5_C    0x000C121a // ???
#define SST_VENDOR_DEVICE_SSID_H5_D    0x000D121a // ???
#define SST_VENDOR_DEVICE_SSID_H5_E    0x000E121a // ???
#define SST_VENDOR_DEVICE_SSID_H5_F    0x000F121a // ???

// DEVICE IDENTIFICATION MACROS

#ifndef _H2INC

#ifndef THUNK32

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

// Is Napalm2?
#define IS_DAYTONA \
 ((_FF(VendorDeviceID) == SST_VENDOR_DEVICE_ID_H5_A) || \
  (_FF(VendorDeviceID) == SST_VENDOR_DEVICE_ID_H5_B) || \
  (_FF(VendorDeviceID) == SST_VENDOR_DEVICE_ID_H5_C) || \
  (_FF(VendorDeviceID) == SST_VENDOR_DEVICE_ID_H5_D) || \
  (_FF(VendorDeviceID) == SST_VENDOR_DEVICE_ID_H5_E) || \
  (_FF(VendorDeviceID) == SST_VENDOR_DEVICE_ID_H5_F))

// Is AGP Capable Napalm
#define IS_AGP_READY_NAPALM \
 ((_FF(SSID) == SST_VENDOR_DEVICE_SSID_H5_4))

// DX version
#define IS_DX5  (_FF(ddRunTimeVersion) >= 0x0500)
#define IS_DX6  (_FF(ddRunTimeVersion) >= 0x0600)
#define IS_DX7  (_FF(ddRunTimeVersion) >= 0x0700)
#define IS_DX8  (_FF(ddRunTimeVersion) >= 0x0800)

#endif // WIN_CSIM

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

// Is napalm2
#define IS_DAYTONA(deviceID) \
 (((deviceID) == SST_VENDOR_DEVICE_ID_H5_A) || \
  ((deviceID) == SST_VENDOR_DEVICE_ID_H5_B) || \
  ((deviceID) == SST_VENDOR_DEVICE_ID_H5_C) || \
  ((deviceID) == SST_VENDOR_DEVICE_ID_H5_D) || \
  ((deviceID) == SST_VENDOR_DEVICE_ID_H5_E) || \
  ((deviceID) == SST_VENDOR_DEVICE_ID_H5_F))

// Is AGP Capable Napalm
#define IS_AGP_READY_NAPALM(deviceID) \
 (((deviceID) == SST_VENDOR_DEVICE_SSID_H5_4))

#endif // WIN_CSIM

#endif // THUNK32

#endif // _H2INC

#endif //#ifdef __H3G_H__
