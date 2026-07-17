/* -*-c++-*- */
/* $Header: bt868.c, 34, 10/24/00 7:21:13 AM, Leo Galway$ */
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
** File name:   bt868.c
**
** Description: BrookTree 868 TV Out support functions.
**
** $Revision: 34$
** $Date: 10/24/00 7:21:13 AM$
**
** $Log: 
**  34   3dfx      1.25.1.3.1.310/24/00 Leo Galway      Moved Brooktree part GUID
**       definition to Miniport\tv.h to keep consistent with Displays\tv.h file. 
**  33   3dfx      1.25.1.3.1.210/11/00 Brent           Forced check in to enforce
**       branching.
**  32   3dfx      1.25.1.3.1.109/04/00 Leo Galway      Use dderror.h status and
**       return codes in BT868_ProcessVideoParameters and BT868_ProcessRequest
**       functions (i.e. removed #ifdef MS_VIEW (for Whistler builds) define for
**       the status/return codes).
**  31   3dfx      1.25.1.3.1.009/01/00 Leo Galway      Use dderror.h error codes
**       for status codes and return codes in BT868_ProcessVideoParameters() and
**       BT868_ProcessRequest(). These changes have been contained within  #ifndef
**       MS_VIEW statements (for Whistler "in-box" driver builds only).
**  30   3dfx      1.25.1.3    06/02/00 Dan O'Connel    Sync. changes between Win9x
**       and Win2K versions of TvOut and DFP so that bt868.c and dfp.c are back to
**       being common source files.
**  29   3dfx      1.25.1.2    06/02/00 Dan O'Connel    Insure the
**       SST_VIDEOIN_TV_DATA_SCRAMBLE_DISABLE bit is clear when TvOut is enabled. 
**       Otherwise switching from  to DFP  to TvOut does not work.
**  28   3dfx      1.25.1.1    05/24/00 Dan O'Connel    Don't rely on the BIOS to
**       take the BT868 chip out of reset state.
**  27   3dfx      1.25.1.0    05/18/00 Dan O'Connel    Major clean up of DFP
**       support code. Restructures DFP code to simplify interfaces.
** 
**       Also let Win2K know that TvOut is a child device, and correct handling of
**       BIOS shared scratch registers to work on a multi-monitor system.
** 
**  26   3dfx      1.25        02/17/00 Dan O'Connel    Change Bt868 timings to
**       recover missing pixel columns on the right and left side of the image for
**       various resolutions and TV Screen Fit Sizes.  This addresses PRS 6384.
**  25   3dfx      1.24        02/07/00 Dan O'Connel    Fix boot to PAL TV in
**       WinNT4/Win2K and cleanup code.
**  24   3dfx      1.23        02/07/00 Dan O'Connel    Remove a lot of O/S
**       specific ifdefs from these files by changing the way IS_VOODOO3 and
**       IS_NAPALM macros are used.
**  23   3dfx      1.22        02/02/00 Dan O'Connel    Add code to access Bios
**       "Board Config" information to check for TvOut and DFP support.
**  22   3dfx      1.21        02/01/00 Russ Lind       changes to use
**       H3_MAPPEDADDRESS_INDICES
**  21   3dfx      1.20        01/28/00 Dan O'Connel    Add call to
**       Bt868_RefreshRegistry during system initialization which is necessary if
**       the system boots directly to TvOut.  Also sync up with changes made to
**       Win9x version that were necessary to startup on Napalm.
**  20   3dfx      1.19        01/10/00 Dan O'Connel    Remove remnants of Banshee
**       code.
**  19   3dfx      1.18        01/05/00 Dan O'Connel    Fix DCT300 MacroVision test
**       to pass when card is not TvOut capable on Win2K.
**  18   3dfx      1.17        01/05/00 Dan O'Connel    Correct Win9x
**       initialization problem that was occaisionally leaving TvOutActive set to
**       TRUE when TvOut was not in use.  Made the initialization more common
**       between Win9x and WinNT/Win2K.
**  17   3dfx      1.16        12/29/99 Dan O'Connel    Add macros to provide a
**       common mechanism to access BIOS Scratch Register 2 across the operating
**       systems.
**       Centralize all access to Scratch Register 2 for TvOut in the bt868.c file,
**       so that it will work for all operating systems.
**       Add support for Napalm BIOS in some places it was missing.
**       Some general cleanup and further steps to make code common between O/Ss.
** 
**  16   3dfx      1.15        12/21/99 Dan O'Connel    Cleanup Win9x code that
**       interface with BIOS via shared registers and NVRAM.  Also update code that
**       interfaces with BIOS to work with Napalm BIOS.
**  15   3dfx      1.14        12/13/99 Dan O'Connel    The wrong value was being
**       passed to BT868_CopyProtect resulting in MacroVision never being turned
**       off once it got turned on.
**  14   3dfx      1.13        12/13/99 Dan O'Connel    Changes necessary to make
**       these files common between WinNT/Win2K and Win9X.  These file will
**       eventually be shared across all three O/Ss.
**  13   3dfx      1.12        12/06/99 Dan O'Connel    Extensive but minor name
**       changes and type changes directed at making the Win9x code more common
**       with the WinNT/Win2K code.  There are no logic changes in this checkin.
**  12   3dfx      1.11        12/02/99 Dan O'Connel    Preliminary round of
**       changes to support new mechanism in 3dfx Tools to turn on and off
**       monitor/DFP/TvOut.
**  11   3dfx      1.10        11/30/99 Dan O'Connel    Corrections to pass DCT 267
**       "Macrovision Copy Protection for DVD" test.  Also cleanup handling of
**       IOCTL_VIDEO_HANDLE_VIDEOPARAMETERS to more closely match Win2K DDK
**       documentation.
**  10   3dfx      1.9         11/16/99 Dan O'Connel    Move handling of
**       VideoParameters data structure inside the miniport where Win2K defines
**       it's handling under IOCTL_VIDEO_HANDLEVIDEOPARAMETERS.
**  9    3dfx      1.8         11/10/99 Dan O'Connel    Port current DFP support
**       from WinNT4 to Win2K.
**  8    3dfx      1.7         11/04/99 Dan O'Connel    Restrict position and size
**       tweaks to the modes in which they are available.
**  7    3dfx      1.6         10/28/99 Dan O'Connel    PRS 6698 - Port mechanism
**       from Win9x that allows Windows to reboot with TvOut enabled if it was
**       shutdown with TvOut enabled.
**  6    3dfx      1.5         10/27/99 Dan O'Connel    The DFP feature has not
**       been moved into Win2K yet so avoid Win2K build problems by making
**       inclusion of DFP code dependent on O/S.
**  5    3dfx      1.4         10/26/99 Dan O'Connel    Turn on DFP if necessary
**       when turning off TvOut. This is temporary until 3dfx Tools is smart enough
**       to do this for the driver.
**  4    3dfx      1.3         10/20/99 Dan O'Connel    Correct problem where the
**       SST_SERPAR_TVOUTRESET_N bit of the vidSerialParallelPort register was not
**       initialized correctly.  The symptom was that the BT868 part was not
**       detected on some boards and the "3dfx TV" page would therefore not be
**       available to select TvOut.
**  3    3dfx      1.2         10/01/99 Dan O'Connel    Port "3dfx Tools" support,
**       TvOut support, and a few misc. bug fixes from WinNt4 to Win2K.  This
**       required a few change is this file to make the Compiler for Win2K happy
**       and to use the Win2K style IS_VOODOO3 macro.
**  2    3dfx      1.1         09/22/99 Dan O'Connel    Restructure and clean up
**       TvOut code, and port it to run on Ryan Bissell's Modularized Reference
**       Implementation of I2C.
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $History: bt868.c $
** 
** *****************  Version 1  *****************
** User: Doconnell    Date: 9/03/99    Time: 11:04a
** Created in $/devel/h5/WinNT/Src/Video/Miniport/h5
** Add TVOut support.
** 
** *****************  Version 12  *****************
** User: Doconnell    Date: 8/27/99    Time: 4:56p
** Updated in $/Releases/Voodoo3/V3_RT4/3dfx/devel/H3/WINNT/SRC/Video/Miniport/Voodoo3
** PRS 8199 Work with Edge Tools to correctly handle concurrent output to
** Monitor and CRT and various OEM specific options (Gateway).  Also port
** some fixes from Win9x to WinNT4 that have to do initializing and
** handling the NVRAM cache.
** 
** *****************  Version 11  *****************
** User: Doconnell    Date: 8/13/99    Time: 4:08p
** Updated in $/Releases/Voodoo3/V3_RT31/3dfx/devel/H3/WINNT/SRC/Video/Miniport/Voodoo3
** PRS 7926.  Port multiple TVOut changes from Win9X to WinNT4.  The most
** important change involves correctly storing data in EEPROM to share
** with TVOut configuration data with BIOS.  This involved changine I2C
** code to run with a slow EEPROM part.
** 
** *****************  Version 9  *****************
** User: Stb_doconnel Date: 7/07/99    Time: 5:23p
** Updated in $/releases/voodoo3/V3_OEM_100/3dfx/devel/h3/winnt/src/video/miniport/voodoo3
** PRS 6384 - Partial fix. Change timing in 640x480 mode to pick up 15
** pixels.
** 
** *****************  Version 8  *****************
** User: Stb_doconnel Date: 6/04/99    Time: 12:42p
** Updated in $/releases/voodoo3/V3_OEM_100/3dfx/devel/h3/winnt/src/video/miniport/voodoo3
** PRS 6386 Port connector override code from Win9x
** 
** *****************  Version 7  *****************
** User: Stb_doconnel Date: 5/25/99    Time: 5:02p
** Updated in $/releases/voodoo3/V3_OEM_100/3dfx/devel/h3/winnt/src/video/miniport/voodoo3
** PRS #6228 Remove code from Win9x that does not apply to WinNT
** 
** *****************  Version 6  *****************
** User: Stb_doconnel Date: 5/24/99    Time: 3:47p
** Updated in $/releases/voodoo3/V3_OEM_100/3dfx/devel/h3/winnt/src/video/miniport/voodoo3
** PRS 6227 Add connector override capability.
** 
** *****************  Version 5  *****************
** User: Stb_doconnel Date: 5/19/99    Time: 11:36a
** Updated in $/releases/voodoo3/V3_OEM_100/3dfx/devel/h3/winnt/src/video/miniport/voodoo3
** Cleanup unused trash code accidently left in file.
** 
** *****************  Version 4  *****************
** User: Stb_doconnel Date: 5/19/99    Time: 11:12a
** Updated in $/releases/voodoo3/V3_OEM_100/3dfx/devel/h3/winnt/src/video/miniport/voodoo3
** PRS 6176 Fix position adjustments after resolution change cause garble
** tv picture.
** 
** *****************  Version 3  *****************
** User: Stb_doconnel Date: 5/18/99    Time: 2:14p
** Updated in $/releases/voodoo3/V3_OEM_100/3dfx/devel/h3/winnt/src/video/miniport/voodoo3
** Force monitor on if Tv is forced off.
** 
** *****************  Version 2  *****************
** User: Stb_doconnel Date: 5/12/99    Time: 12:13p
** Updated in $/releases/voodoo3/V3_OEM_100/3dfx/devel/h3/winnt/src/video/miniport/voodoo3
** Clean up include of 3dfx.h
** 
** *****************  Version 1  *****************
** User: Stb_doconnel Date: 5/12/99    Time: 10:07a
** Created in $/releases/voodoo3/V3_OEM_100/3dfx/devel/h3/winnt/src/video/miniport/voodoo3
** 
** *****************  Version 26  *****************
** User: Stuartb      Date: 3/23/99    Time: 11:41a
** Updated in $/devel/h3/Win95/dx/minivdd
** Noted (fixed) problem in bt868_fixupNonStdModes where reading past end
** of array.
** 
** *****************  Version 25  *****************
** User: Stuartb      Date: 3/19/99    Time: 3:42p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added BT868_FixupVGA to assure we make the hires to VGA transition
** properly.  Cleaned up BT868_GetStatus so there is less video
** disturbance on call.  Added comments.
** 
** *****************  Version 24  *****************
** User: Stuartb      Date: 3/12/99    Time: 8:47a
** Updated in $/devel/h3/Win95/dx/minivdd
** Route bt868 DACs for composite out if so directed by BIOS.
** 
** *****************  Version 8  *****************
** User: Stuartb      Date: 3/08/99    Time: 9:53a
** Updated in $/Releases/Voodoo3/MT2/3Dfx/devel/H3/Win95/DX/minivdd
** Do not restore bt868 autoconfig mode from registry.  Mode is now
** determined from tvstd as saved in NVRAM or established by BIOS.
** 
** *****************  Version 7  *****************
** User: Stuartb      Date: 3/07/99    Time: 12:05p
** Updated in $/Releases/Voodoo3/MT2/3Dfx/devel/H3/Win95/DX/minivdd
** When changing major modes or tvstd, mark all BT868_RegisterShadow
** invalid.
** 
** *****************  Version 6  *****************
** User: Stuartb      Date: 3/03/99    Time: 5:04p
** Updated in $/Releases/Voodoo3/MT2/3Dfx/devel/H3/Win95/DX/minivdd
** leave DACs live to make it easier to reenable at shutdown time!
** 
** *****************  Version 20  *****************
** User: Stuartb      Date: 3/01/99    Time: 2:39p
** Updated in $/devel/h3/Win95/dx/minivdd
** Turn off genlock while downloading overscan registers.  Remove
** references to TV_STANDARD_MAJOR....
** 
** *****************  Version 19  *****************
** User: Stuartb      Date: 2/25/99    Time: 10:53a
** Updated in $/devel/h3/Win95/dx/minivdd
** Save tvstd in nvram if STB reference board.
** 
** *****************  Version 18  *****************
** User: Stuartb      Date: 2/23/99    Time: 2:36p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added code to access serial NVRAM if present.  Disable DAC A if H4 as
** it is not used and is shorted to gnd.
** 
** *****************  Version 17  *****************
** User: Stuartb      Date: 2/19/99    Time: 11:17a
** Updated in $/devel/h3/Win95/dx/minivdd
** If IS_H4, swap Y/C outputs.
** 
** *****************  Version 16  *****************
** User: Stuartb      Date: 2/18/99    Time: 11:42a
** Updated in $/devel/h3/Win95/dx/minivdd
** More work on PAL modes.  Turn off tv out DACs when disabled.
** 
** *****************  Version 15  *****************
** User: Stuartb      Date: 2/13/99    Time: 6:42p
** Updated in $/devel/h3/Win95/dx/minivdd
** First cut at PAL_M, PAL_N, PAL_NC support in tvout.
** 
** *****************  Version 14  *****************
** User: Stuartb      Date: 2/08/99    Time: 8:58a
** Updated in $/devel/h3/Win95/dx/minivdd
** Changes to fix lcd boot and simultaneous VMI & TV/LCD.
** 
** *****************  Version 13  *****************
** User: Agus         Date: 2/01/99    Time: 4:37p
** Updated in $/devel/h3/Win95/dx/minivdd
** Move IS_H4 macro define to h3vdd.h
** 
** *****************  Version 12  *****************
** User: Cwilcox      Date: 1/22/99    Time: 2:07p
** Updated in $/devel/h3/Win95/dx/minivdd
** Minor revisions to clean up compiler warnings.
** 
** *****************  Version 11  *****************
** User: Michael      Date: 1/04/99    Time: 1:20p
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
*/

/*
 * bt868.c - BrookTree TV encoder driver
 */



#ifdef WINNT

#include "dderror.h"

#include "miniport.h"
#include "ntddvdeo.h"
#include "video.h"
#include "h3.h"
#include "3dfx.h"

#define VDDONLY
#include "tv.h"
#undef  VDDONLY
#include "bt868.h"
#include "di_i2c.h"
#include "nvram.h"
#if (_WIN32_WINNT < 0x0500)
//for Win2K tvout.h is included by "ntddvdeo.h"
#include "tvout.h"
#endif
#include "fxioctl.h"

#define MSEC 1000
#define DELAY(usecs) VideoPortStallExecution(usecs)

#define IS_NAPALM_X(pcontext)   (0x06 <= pcontext->PCIDeviceID)
#define IS_VOODOO3_X(pcontext)  (0x05 >= pcontext->PCIDeviceID)

//  BEGIN lines copied from Win9x tvoutdef.h
#define BT868_ADDR           (0x88)
#define BT868_ALT_ADDR       (0x8A)
typedef struct
{
// don't save mode in registry anymore because it is made obsolete by changes between 640x480 and 800x600.
// Instead derive mode from tvStd on the fly.  DanO 5/18/99
    unsigned obsolete_mode:2;
    unsigned bright:3;
    unsigned filter:3;
    unsigned saturation:3;
    unsigned hpos:7;
    unsigned vpos:7;
    unsigned size:3;
    unsigned is_master:1;
    unsigned lastValueIndex:3;
    unsigned copyProtectOn:2;
    unsigned encoderInitAttempt:1;
    unsigned tvStd:5;    // this mirrors the BIOS scratch register
    unsigned cvbsOut:1;
    unsigned tvBoot:1;
}  TVOUT_CURSETUP;
//  END lines copied from Win9x tvoutdef.h

// the following macro is not implemented for WinNT
#undef IS_3DFX_REF_BOARD
#define IS_3DFX_REF_BOARD(pContext)  FALSE

#else //def WINNT

   #include "h3vdd.h"
   #include "h3.h"
   
   #define VDDONLY
   #include "tv.h"
   #include "devtable.h"
   #undef  VDDONLY
   #include "tvoutdef.h"
   #include "bt868.h"


   #include "i2c/di_i2c.h"
   #define MSEC 1000
   #define DELAY(usecs) CM_Yield(usecs, CM_YIELD_RESUME_EXEC)

   #include "nvram.h"

   #include "sagelcd.h"
   #include "time.h"

#undef IS_3DFX_REF_BOARD
// the following macro returns true if it's a Voodoo3 reference board that doesn't have an NVRAM
#define IS_3DFX_REF_BOARD(pContext)  ((0x05 >= pContext->dwVendorDeviceID) && ((pContext->dwSubSystemID & 0xff0000) < 0x00300000))

#define IS_NAPALM_X(pcontext)   (IS_NAPALM(pcontext->dwVendorDeviceID))
#define IS_VOODOO3_X(pcontext)  (IS_VOODOO3(pcontext->dwVendorDeviceID))
#endif //def WINNT
#include "bt868regs.h"
#include "bios.h"
#include "dfp.h"

#ifdef WINNT
//
// For WinNT we don't use the CRT 'min' function because that would drag in
// unwanted CRT baggage.
//
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#else
#define MIN(a, b) min(a, b)
#endif  //def WINNT

#ifdef WINNT
// forward definitions
int BT868_Disable(BT868CONTEXT pContext);
int BT868_SetSize( BT868CONTEXT pContext, PTVSETSIZE pTvSetSize);
int BT868_SetPicControl ( BT868CONTEXT pContext, PTVSETCAP pTvSetPicControl);
int BT868_CopyProtect (BT868CONTEXT pContext, int setting);
#else
FxI32 SetConnectionType(BT868CONTEXT pContext, PTVSETSTANDARD ptvstd);
#endif

BT868_Regs Bt868_RegShadow;

// Bt868 default (autoconfig) PLL settings, 8.16 numbers
// fclk = (13500000 * pllint.pllfract) / 6  (36.0 = (13.5 * 16) / 6)
FxU32 AutoCfgFclk[] = {0x0c880e, 0x0d1c72, 0x113b14, 0x100000};

// top 16 bits of HposNormal is actually vscale 13:8 for the selected mode
FxU32 HposNormal[4] = {0x140155, 0x1001b0, 0x1c024d, 0x1601f0};  // Voodoo3 values

// srogers - 7/17/99 in Win9x, copied from Win9x by DanO 7/23/99
// This is the old 2nd line of Clk Delay.  It was being excluded since the
// number was being && with 0xF.  So we're changing it to 0xF.  This
// should fix the blue tinge on TVOut grayscales by making tmugbeinit = 0xF0FF0.
// This fixes PRS 7401.
//                          {0x10, 0x10, 0x10, 0x10}};
const FxU8 ClkDly[4] = {0x1F, 0x1F, 0x1F, 0x1F};  // Voodoo3 values
const FxU8 HExtentNormal[4] = {0x5c, 0xe8, 0x00, 0x68};  // Voodoo3 values

const FxU8 VposNormal[] = {75, 104, 88, 95};
const FxI8 HPosScale[] = {50, 50, 50, 50};
const FxI8 VPosScale[] = {50, 50, 50, 50};
const FxI8 SIZE_XLATE[] = {0, 1, -1, 2, 3};

#define FX_RESOLUTION(h,v) ((((FxU32)(h)) << 16) | ((FxU32)(v)))

#define OSCAN_STEPS   1
#if OSCAN_STEPS

typedef struct
{
    FxI8 hPosOfst;
    FxI8 vPosOfst;
    FxU8 regVals[32];
}  BT868_SUBMODE;


const FxU8 BT868_720x480_NTSC[] =
{
   0x00,
   0x00,
   0x02,
   0x00,
   0x01,
   0xD0,
   0xD0,
   0x80,
   0x92,
   0x5A,
   0x11,
   0x13,
   0xF2,
   0x26,
   0x00,
   0x68,
   0x85,
   0x03,
   0x0D,
   0x24,
   0xE0,
   0x06,
   0x00,
   0x50,
   0x20,
   0x32,
   0x0C,
   0x0A,
   0xE5,
   0x76,
   0x79,
   0x44,
   0x85,
   0xA7,
   0xFD,
   0x64,
   0x21,
   0x00,
   0x00,
   0x00,
   0x00,
   0x00,
   0x00,
   0x00,
   0x01,
   0x80,
   0x40,    //DIS_FF enabled so flicker filter 'off'
   0xC0,
   0xC0,
   0x18,
   0x00,
   0x00,
   0x00,
   0x00,
   0x00
};

const FxU8 BT868_720x576_PAL[] =
{
   0x00,
   0x00,
   0x02,
   0x00,
   0x01,
   0xF0,
   0xD0,
   0x82,
   0x9C,
   0x5A,
   0x31,
   0x16,
   0x22,
   0xA6,
   0x00,
   0x78,
   0x93,
   0x03,
   0x71,
   0x2A,
   0x40,
   0x0A,
   0x00,
   0x50,
   0x55,
   0x55,
   0x0C,
   0x24,
   0xF0,
   0x59,
   0x82,
   0x49,
   0x8C,
   0x8E,
   0xB0,
   0xE6,
   0x28,
   0x00,
   0x01,
   0x00,
   0x00,
   0x00,
   0x00,
   0x00,
   0x01,
   0x80,
   0x40,    //DIS_FF enabled so flicker filter 'off'
   0xC0,
   0xC0,
   0x18,
   0x00,
   0x00,
   0x00,
   0x00,
   0x00
};

/*
These are overscan registers 0x76 through 0xb4 inclusive.  They are arranged as:
[MAJOR_MODE][OVERSCAN_SETTING].
*/
const BT868_SUBMODE Overscan[4][4] = {
// major mode 0, 640x480 NTSC
{{ -24,  -24, 0x80, 0x80, 0x7c, 0x8a, 0x50, 0x35, 0x18, 0xe9, 0x26, 0x00, 0x20,
              0x8c, 0x03, 0x22, 0x2f, 0xe0, 0x36, 0x48, 0x51, 0xe9, 0xa2, 0x0b,
              0x0a, 0xe5, 0x76, 0x79, 0x44, 0x85, 0x00, 0x00, 0x00, 0x23},
 {   0,  -16, 0xc0, 0x80, 0x80, 0x90, 0x58, 0x59, 0x1c, 0xe0, 0x26, 0x00, 0x20,
              0x8c, 0x03, 0x37, 0x3a, 0xe0, 0x36, 0x8f, 0x52, 0x7b, 0x15, 0x0c,
              0x0a, 0xe5, 0x76, 0x79, 0x44, 0x85, 0xed, 0x25, 0xb4, 0x21},
 {  16,   12, 0x40, 0x80, 0x8a, 0x9a, 0x68, 0xa1, 0x24, 0xd1, 0x27, 0x00, 0x20,
              0x8c, 0x03, 0x61, 0x51, 0xe0, 0x36, 0x1f, 0x55, 0xa1, 0xfa, 0x0c,
              0x0a, 0xe5, 0x75, 0x79, 0x44, 0x85, 0x7c, 0x1a, 0x61, 0x1f},
 {  48,   32, 0xc0, 0x80, 0x92, 0xa6, 0x78, 0xe9, 0x2b, 0xc3, 0x27, 0x00, 0x20,
              0x8c, 0x03, 0x8b, 0x68, 0xe0, 0x36, 0xae, 0x57, 0xc7, 0xdf, 0x0d,
              0x0a, 0xe5, 0x75, 0x78, 0x44, 0x85, 0xb6, 0xd6, 0x5a, 0x1d}},

// major mode 1, 640x480 PAL
{{ -64,  -32, 0x90, 0x80, 0x7c, 0x94, 0x4e, 0x4b, 0x17, 0x20, 0xa6, 0x00, 0xe8, 
              0x1a, 0x0b, 0x0d, 0x24, 0xe0, 0x36, 0xe1, 0x4a, 0xab, 0xaa, 0x0b, 
              0x24, 0xf0, 0x59, 0x83, 0x49, 0x8c, 0xca, 0x03, 0x3d, 0x2b},
 { -40,  -48, 0xc0, 0x80, 0x7e, 0x98, 0x54, 0x65, 0x1b, 0x18, 0xa6, 0x00, 0xe8, 
              0x12, 0x0b, 0x1c, 0x2c, 0xe0, 0x36, 0xa6, 0x4b, 0x00, 0x00, 0x0c, 
              0x24, 0xf0, 0x59, 0x82, 0x49, 0x8c, 0xcb, 0x8a, 0x09, 0x2a},
 {0000, 0000, 0x80, 0x80, 0x8e, 0xa8, 0x6c, 0xd3, 0x2e, 0xf2, 0x27, 0x00, 0xc0, 
              0x0a, 0x0b, 0x71, 0x5a, 0xe0, 0x36, 0x00, 0x50, 0x55, 0x55, 0x0d, 
              0x24, 0xf0, 0x58, 0x81, 0x49, 0x8c, 0x50, 0x63, 0xd5, 0x25},
 {  20, 0000, 0xb0, 0x80, 0x90, 0xac, 0x72, 0xef, 0x2e, 0xf2, 0x27, 0x00, 0xd8, 
              0x1a, 0x0b, 0x71, 0x5a, 0xe0, 0x36, 0x00, 0x50, 0xab, 0xaa, 0x0d, 
              0x24, 0xf0, 0x58, 0x81, 0x49, 0x8c, 0xb2, 0x28, 0xe9, 0x24}},

// major mode 2, 800x600 NTSC
{{ -88,  -32, 0x90, 0x20, 0xa2, 0xb6, 0x92, 0xbd, 0x18, 0xe8, 0x38, 0x00, 0x48, 
              0x1e, 0x03, 0xad, 0x3c, 0x58, 0x3a, 0xc1, 0x59, 0x24, 0x54, 0x0f, 
              0x0a, 0xe5, 0x75, 0x78, 0x43, 0x85, 0x78, 0xc0, 0x91, 0x1a},
 { -64,  000, 0xf0, 0x20, 0xaa, 0xbe, 0x9e, 0xf3, 0x1d, 0xde, 0x38, 0x00, 0x48, 
              0x1e, 0x03, 0xcb, 0x4c, 0x58, 0x3a, 0x95, 0x5b, 0x00, 0x00, 0x10, 
              0x0a, 0xe5, 0x74, 0x78, 0x43, 0x85, 0x17, 0x5d, 0x74, 0x19},
 {  32, 0000, 0x00, 0x20, 0xbe, 0xd6, 0xc2, 0x8b, 0x22, 0xd4, 0x3a, 0x00, 0x80, 
              0x52, 0x03, 0xee, 0x5e, 0x58, 0x3a, 0xb7, 0x9d, 0xf0, 0xe6, 0x11, 
              0x0a, 0xe5, 0x74, 0x77, 0x43, 0x85, 0x00, 0x00, 0xc0, 0x16},
 {  64, 0000, 0x80, 0x20, 0xc6, 0xe0, 0xd2, 0xd3, 0x20, 0xd8, 0x3a, 0x00, 0xc0,
              0x40, 0x03, 0xdf, 0x56, 0x58, 0x3a, 0xcd, 0x9c, 0x15, 0xcc, 0x12, 
              0x0a, 0xe5, 0x74, 0x77, 0x43, 0x85, 0xab, 0xaa, 0xaa, 0x15}},

// major mode 3, 800x600 PAL
{{ -48,  -24, 0x50, 0x20, 0x9c, 0xba, 0x86, 0xa9, 0x19, 0x1c, 0xb8, 0x00, 0xe8,
              0x90, 0x03, 0x99, 0x33, 0x58, 0x3a, 0x0c, 0x52, 0x1c, 0xc7, 0x0e,
              0x24, 0xf0, 0x57, 0x80, 0x48, 0x8c, 0x26, 0xb2, 0x22, 0x22},
 { -32,  -24, 0x90, 0x20, 0xa0, 0xc0, 0x8c, 0xcd, 0x1d, 0x14, 0xb8, 0x00, 0xe8,
              0x90, 0x03, 0xad, 0x3d, 0x58, 0x3a, 0x12, 0x53, 0xe4, 0x38, 0x0f,
              0x24, 0xf0, 0x57, 0x80, 0x48, 0x8c, 0x2e, 0x8d, 0x23, 0x21},
 {  24,    0, 0x60, 0x20, 0xb0, 0xd2, 0xa6, 0x45, 0x29, 0xfc, 0x39, 0x00, 0xe8, 
              0x90, 0x03, 0xee, 0x5f, 0x58, 0x3a, 0x66, 0x96, 0xab, 0xaa, 0x10, 
              0x24, 0xf0, 0x57, 0x80, 0x48, 0x8c, 0x74, 0x4f, 0x44, 0x1e},
 {  60,   20, 0xd0, 0x20, 0xb8, 0xdc, 0xb4, 0x83, 0x2f, 0xf1, 0x39, 0x00, 0xe8,
              0x90, 0x03, 0x11, 0x73, 0x58, 0x3b, 0x31, 0x98, 0xc7, 0x71, 0x11,
              0x24, 0xf0, 0x57, 0x7f, 0x48, 0x8c, 0x23, 0xd8, 0xea, 0x1c}}};
#endif

  // In the following tables hsynoffset is typically set to values from 0 to -32, and
  // hsynwidth is set to 0x1f.
const FxU8 register6E[4][5] =
{{0xe8, 0xe8, 0xe8, 0xe8, 0xe8},   // NTSC 640X480 - 5 sizes largest through smallest
 {0xf0, 0xf0, 0xf0, 0xf0, 0x00},   // PAL 640X480  - 5 sizes largest through smallest
 {0xd8, 0xd8, 0xfc, 0xfc, 0xf8},   // NTSC 800X600 - 5 sizes largest through smallest
 {0x00, 0x00, 0x00, 0x00, 0x00}};  // PAL 800X600  - 5 sizes largest through smallest

const FxU8 register70[4][5] =
{{0xdf, 0xdf, 0xdf, 0xdf, 0xdf},   // NTSC 640X480 - 5 sizes largest through smallest
 {0xdf, 0xdf, 0xdf, 0xdf, 0x1f},   // PAL 640X480  - 5 sizes largest through smallest
 {0xdf, 0xdf, 0xdf, 0xdf, 0xdf},   // NTSC 800X600 - 5 sizes largest through smallest
 {0x1f, 0x1f, 0x1f, 0x1f, 0x1f}};  // PAL 800X600  - 5 sizes largest through smallest


enum
{
    NTSC_OFF,
    NTSC_ON,
    PAL_OFF,
    PAL_ON
};

FxU8 MacroVisionRegsPAL[][18] = {
{0x05, 0x57, 0x20, 0x40, 0x6e, 0x7e, 0xf4, 0x51, 0x0f,
 0xf1, 0x05, 0xd3, 0x78, 0xa2, 0x25, 0x54, 0xa5, 0x00},
{0x05, 0x57, 0x20, 0x40, 0x6e, 0x7e, 0xf4, 0x51, 0x0f,
 0xf1, 0x05, 0xd3, 0x78, 0xa2, 0x25, 0x54, 0xa5, 0x63}};
FxU8 MacroVisionRegsNTSC[][18] = {
{0x0f, 0xfc, 0x20, 0xd0, 0x6f, 0x0f, 0x00, 0x00, 0x0c,   // OFF
 0xf3, 0x09, 0xbd, 0x67, 0xb5, 0x90, 0xb2, 0x7d, 0x00},
{0x0f, 0xfc, 0x20, 0xd0, 0x6f, 0x0f, 0x00, 0x00, 0x0c,   // no color stripe
 0xf3, 0x09, 0xbd, 0x67, 0xb5, 0x90, 0xb2, 0x7d, 0x63},
{0x0f, 0xfc, 0x20, 0xd0, 0x6f, 0x0f, 0x00, 0x00, 0x0c,   // 2 line color stripe
 0xf3, 0x09, 0xbd, 0x6c, 0x31, 0x92, 0x32, 0xdd, 0xe3},
{0x0f, 0xfc, 0x20, 0xd0, 0x6f, 0x0f, 0x00, 0x00, 0x0c,   // 4 line color stripe
 0xf3, 0x09, 0xbd, 0x66, 0xb5, 0x90, 0xb2, 0x7d, 0xe3}};

#ifndef WINNT
#pragma VxD_LOCKED_CODE_SEG
#pragma VxD_LOCKED_DATA_SEG
#endif



/*----------------------------------------------------------------------
Function name:  vpTvStdToBiosStd

Description:    given VP_TV_STANDARD_THING (from win98 ddk) return BIOS_XX value

Information:    PAL GDHIB are all considered 'generic' PAL.

Return:         FxU8
----------------------------------------------------------------------*/
FxU8 vpTvStdToBiosStd (int vpTvStd)
{
    ULONG dwPal = VP_TV_STANDARD_PAL_B | VP_TV_STANDARD_PAL_G | VP_TV_STANDARD_PAL_D | VP_TV_STANDARD_PAL_H | VP_TV_STANDARD_PAL_I;

    if (vpTvStd & VP_TV_STANDARD_PAL_M)
        return (BIOS_PAL_M);
    else if (vpTvStd & VP_TV_STANDARD_PAL_N)
        return (BIOS_PAL_N);
    else if (vpTvStd & VP_TV_STANDARD_PAL_NC)
        return (BIOS_PAL_Nc);
    else if (vpTvStd & dwPal)
        return (BIOS_PAL);
    else
        return (BIOS_NTSC);
}


/*----------------------------------------------------------------------
Function name:  getTvStdInfo

Description:    given BIOS_XX tvStd return bt868 auto-config mode and 
                VP_TV_STANDARD_THING

Information:    PAL GDHIB are all considered 'generic' PAL.

Return:         void
----------------------------------------------------------------------*/
void getTvStdInfo (BT868CONTEXT pContext, FxU32 biosStd, FxU8 *autoCfg, FxU32 *vpTvStd)
{
#ifdef WINNT
    PH3_MEMBASE0 sstIoRegs = (PH3_MEMBASE0) pContext->MappedAddress[SST_IO_REGS_INDEX];
#else
	SstIORegs *sstIoRegs = (SstIORegs *)pContext->RegBase[HWINFO_SST_IOREGS_INDEX];
#endif
    ULONG xres = sstIoRegs->vidScreenSize & 0xfff;
    ULONG dwPal = VP_TV_STANDARD_PAL_B | VP_TV_STANDARD_PAL_G | VP_TV_STANDARD_PAL_D | VP_TV_STANDARD_PAL_H | VP_TV_STANDARD_PAL_I;

    if (autoCfg)
    {
      switch (biosStd)
      {
         case BIOS_NTSC:
         case BIOS_PAL_M:
            switch (xres)
            {
               case 720:
               case 800:
                  *autoCfg = 2;
                  break;

               default:
                  *autoCfg = 0;
            }
            break;

              case BIOS_PAL:
            case BIOS_PAL_N:
            case BIOS_PAL_Nc:
            switch (xres)
            {
               case 720:
               case 800:
                  *autoCfg = 3;
                  break;

               default:
                  *autoCfg = 1;
            }
            break;
      }
    }        

    if (vpTvStd)
    {
        if (biosStd == BIOS_PAL_M)
            *vpTvStd = VP_TV_STANDARD_PAL_M;
        else if (biosStd == BIOS_PAL_N)
            *vpTvStd = VP_TV_STANDARD_PAL_N;
        else if (biosStd == BIOS_PAL_Nc)
            *vpTvStd = VP_TV_STANDARD_PAL_NC;
        else if (biosStd == BIOS_PAL)
            *vpTvStd = dwPal;
        else
            *vpTvStd = VP_TV_STANDARD_NTSC_M;
    }
}

static int bt_write (I2CCONTEXT pContext, I2CKEY i2ckey, int reg, FxU8 data)
{
   int result = I2C_SUCCESS;
   I2CKEY key;

   if (i2ckey == I2C_NOTAKEY) //do we need to provide our own key?
   {
      key = i2c_getaccess(pContext, I2C_TVENCODER, I2C_NORMALSPEED);
      if (key == I2C_NOTAKEY)
         return FXFALSE;
   }
   else
      key = i2ckey;  //caller has provided us with a key already

   result &= i2c_start(pContext, key);
   result &= i2c_sendbyte(pContext, key, BT868_ADDR);
   result &= i2c_sendbyte(pContext, key, (FxU8)reg);
   result &= i2c_sendbyte(pContext, key, data);
   result &= i2c_stop(pContext, key);

   if (i2ckey == I2C_NOTAKEY) //revoke the key if I created it
      i2c_endaccess(pContext, key);

   if (reg >= 0x76 && reg < 0xd8)
        ((char *)&Bt868_RegShadow)[(reg - 0x76) / 2] = data;

   return (result ? FXTRUE : FXFALSE);
}



static int bt_read(I2CCONTEXT pContext, I2CKEY i2ckey, FxU8* pbyte)
{
   int result = I2C_SUCCESS;
   I2CKEY key;

   if (NULL == pbyte)
      return FXFALSE;

   if (i2ckey == I2C_NOTAKEY) //do we need to provide our own key?
   {
      key = i2c_getaccess(pContext, I2C_TVENCODER, I2C_NORMALSPEED);
      if (key == I2C_NOTAKEY)
         return FXFALSE;
   }
   else
      key = i2ckey;  //caller has provided us with a key already

   result &= i2c_start(pContext, key);
   result &= i2c_sendbyte(pContext, key, BT868_ADDR);
   result &= i2c_sendbyte(pContext, key, 0xC4);
   result &= i2c_start(pContext, key);
   result &= i2c_sendbyte(pContext, key, BT868_ADDR | 0x01);
   result &= i2c_readbyte(pContext, key, pbyte, 0);
   result &= i2c_stop(pContext, key);

   if (i2ckey == I2C_NOTAKEY) //revoke the key if I created it
      i2c_endaccess(pContext, key);

   return (result ? FXTRUE : FXFALSE);
}



/*----------------------------------------------------------------------
Function name:  BT868_GetStatus

Description:    Return the status of the BT868.

Information:

Return:         INT     The status word or,
                        -1 for failure.
----------------------------------------------------------------------*/
int BT868_GetStatus (BT868CONTEXT pContext, int estatus)
{
#ifdef WINNT
   PH3_MEMBASE0 sstIoRegs = (PH3_MEMBASE0) pContext->MappedAddress[SST_IO_REGS_INDEX];
#else
   SstIORegs *sstIoRegs = (SstIORegs *)pContext->RegBase[HWINFO_SST_IOREGS_INDEX];
#endif
   I2CKEY key;
   FxU8 bRegValue;
   TVOUT_CURSETUP *btSetup = (void *)&pContext->tvOutData;
   FxU8 readData;
   FxU32 vidInFmt = sstIoRegs->vidInFormat;

   // insure the BT868 is not being held in reset
   sstIoRegs->vidSerialParallelPort |= SST_SERPAR_TVOUTRESET_N;

#ifndef WINNT
   // srogers 7/14/99 Since the 380 board has both i2c clock and data lines pulled
   // high with the same resistor, we end up detecting that the BT868 is present
   // when it is not.  Here we are excluding board numbers 4B-4F from the check,
   // since they should not return true.
   // srogers 8/13/99 Found out that Gateway was taking a 380 board.  So now we check
   // the SSID between 4a and 50, but exclude the 0x124f board, Compaq 3500.
   // srogers 9/21/99 Since Napalm would have a different ID I'm encasing this check
   // in IS_VOODOO3 so it doesn't corrupt Napalm code.
   if(IS_VOODOO3_X(pContext))
   {
     if( (((pContext->dwSubSystemID >> 16 )&0xFF) > 0x004A) &&
       (((pContext->dwSubSystemID >> 16 )&0xFF) < 0x0050) &&
       ((pContext->dwSubSystemID >> 16 ) != 0x124F) )
       return (-1);
   }

#endif

   // test if BIOS supports TvOut
   if ((IS_NAPALM_X(pContext)) && ((pContext->biosBoardConfigInfo & BIOS_BOARDCONFIG_TVOUTSUPPORT)==0))
       return(-1);

   sstIoRegs->vidInFormat |= SST_VIDEOIN_TVOUT_ENABLE;
 
   key = i2c_getaccess(pContext, I2C_TVENCODER, I2C_NORMALSPEED);
   if (key == I2C_NOTAKEY)
      return -1;
   i2c_stop(pContext, key);
   i2c_endaccess(pContext, key);  //just wanted the key to do the STOP condition.

   if (estatus == 1)
   {
       bRegValue = (FxU8) (BT86X_REGxBA_CHECKSTAT | (btSetup->is_master ? 0 : BT86X_REGxBA_SLAVER));

       if (IS_VOODOO3_X(pContext))
       {
#ifdef WINNT
         switch (pContext->PCISubSystemID)
#else
         switch ((pContext->dwSubSystemID & 0x00FF0000) >> 16)
#endif
         {
            // The Voodoo3 3500TV has a different DAC configuration.
            case 0x00000060:
            case 0x00000061:
            case 0x00000062:
               break;

            default:
               bRegValue |= BT86X_REGxBA_DACDISA;  //disable unused DAC A
               break;
         }
       }

       if (FXFALSE == bt_write(pContext, I2C_NOTAKEY, REGxBA, bRegValue))
       {
           sstIoRegs->vidInFormat = vidInFmt;
           return (-1);
       }

       // @RBISSELL, A short delay here will improve the accuracy
       // of this connector detection scheme.
       DELAY(125*MSEC);
   }

   bRegValue = ((estatus << 6) | btSetup->is_master);   // Set estatus
   if (FXFALSE == bt_write(pContext, I2C_NOTAKEY, REGxC4, bRegValue))
   {
       sstIoRegs->vidInFormat = vidInFmt;
       return (-1);
   }

   // @RBISSELL, A short delay here will improve the accuracy
   // of this connector detection scheme.
   DELAY(125*MSEC);
   
   if (FXFALSE == bt_read(pContext, I2C_NOTAKEY, &bRegValue))
   {
       sstIoRegs->vidInFormat = vidInFmt;
       return (-1);
   }
   readData = bRegValue;

   if (estatus == 1)
   {
      bRegValue = (FxU8) (btSetup->is_master ? 0 : BT86X_REGxBA_SLAVER);

       if (IS_VOODOO3_X(pContext))
       {
#ifdef WINNT
         switch (pContext->PCISubSystemID)
#else
         switch ((pContext->dwSubSystemID & 0x00FF0000) >> 16)
#endif
         {
            // The Voodoo3 3500TV has a different DAC configuration.
            case 0x00000060:
            case 0x00000061:
            case 0x00000062:
               break;

            default:
               bRegValue |= BT86X_REGxBA_DACDISA;  //disable unused DAC A
               break;
         }
       }

       if (FXFALSE == bt_write(pContext, I2C_NOTAKEY, REGxBA, bRegValue))
       {
           sstIoRegs->vidInFormat = vidInFmt;
           return (-1);
       }
   }

   sstIoRegs->vidInFormat = vidInFmt;
   return (readData);
}


/*----------------------------------------------------------------------
Function name:  bt868_position

Description:    Handle x,y positioning.

Information:
    This is awful!  There's no reasonable bt868 handle for
    vertical positioning.  After all, it doesn't have a field
    store and it can't show us the data before it arrives!  So
    we will use the CRT timing totals to move vertical.  The 
    horizontal is controlled via bt868 regs (9a:80).

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
static int bt868_position (BT868CONTEXT pContext, int xpos, int ypos)
{
   I2CKEY key;
   TVOUT_CURSETUP *tvOutData = (void *)&pContext->tvOutData;
#ifdef WINNT
   PH3_MEMBASE0 sstIoRegs = (PH3_MEMBASE0) pContext->MappedAddress[SST_IO_REGS_INDEX];
#else
   SstIORegs *sstIoRegs = (SstIORegs *)pContext->RegBase[HWINFO_SST_IOREGS_INDEX];
#endif
   ULONG yres = (sstIoRegs->vidScreenSize >> 12) & 0xfff;
   ULONG xres = sstIoRegs->vidScreenSize & 0xfff;
   FxU8 mode;
   BT868_SUBMODE const *oscan = 0;
   FxU8 v_scale;

    // position tweaks are only available in some modes
    switch (FX_RESOLUTION(xres,yres))
    {
    // this is a list of modes we can handle
    case FX_RESOLUTION(640,480):
    case FX_RESOLUTION(800,600):
        break;

    default:
        return(0);
    }

   // get mode corresponding to tvStd.
   getTvStdInfo (pContext, tvOutData->tvStd, &mode, 0);

   v_scale = (FxU8)(HposNormal[mode] >> 16);

   key = i2c_getaccess(pContext, I2C_TVENCODER, I2C_NORMALSPEED);
   if (key == I2C_NOTAKEY)
      return -1;

   if (OSCAN_STEPS && SIZE_XLATE[tvOutData->size] >= 0)
      oscan = &Overscan[mode][SIZE_XLATE[tvOutData->size]];
   if (xpos >= 0 && xpos <= 100)
   {
      tvOutData->hpos = xpos;
      xpos -= 50;     // make bipolar
      xpos = HposNormal[mode] + ((xpos * HPosScale[mode]) / 50);
      if (oscan)
      {
         xpos += (oscan->hPosOfst * 2);
         v_scale = oscan->regVals[18] & 0x3f;
      }
      bt_write(pContext, key, REGx80, (FxU8) (xpos & 0xff));
      bt_write(pContext, key, REGx9A, (FxU8) (v_scale | ((xpos & 0x300) >> 2)));
   }
   if (ypos >= 0 && ypos <= 100)
   {
      tvOutData->vpos = ypos;
      ypos -= 50;     // make bipolar
      ypos = VposNormal[mode] + ((ypos * VPosScale[mode]) / 50);
      if (oscan)
         ypos += oscan->vPosOfst;
      sstIoRegs->vidTvOutBlankVCount = ((ypos + yres) << 16) | ypos;
   }

   xpos = HExtentNormal[mode];
   sstIoRegs->vidTvOutBlankHCount = ((xpos + xres) << 16) | xpos;

   // position tweaks
   switch (FX_RESOLUTION(xres,yres))
   {
      case FX_RESOLUTION(640,480):
         switch (tvOutData->tvStd)
         {
            case BIOS_NTSC:
            case BIOS_PAL_M:
               switch (SIZE_XLATE[tvOutData->size])
               {
                  case -1:
                  sstIoRegs->vidTvOutBlankHCount = 0x02D00050;
                  break;

                  default:
                     sstIoRegs->vidTvOutBlankHCount = 0x02DC005C;
                     break;
               }
               break;

            default:
               switch (SIZE_XLATE[tvOutData->size])
               {
                  case 1:
                     sstIoRegs->vidTvOutBlankHCount = 0x036000E0;
                     break;

                  default:
                     sstIoRegs->vidTvOutBlankHCount = 0x035800D8;
                     break;
               }
               break;
         }
         break;


         case FX_RESOLUTION(800,600):
            switch (tvOutData->tvStd)
            {
               case BIOS_NTSC:
               case BIOS_PAL_M:
               switch (SIZE_XLATE[tvOutData->size])
               {
                  case 2:
                     sstIoRegs->vidTvOutBlankHCount = 0x03300010;
                     break;

                  default:
                     sstIoRegs->vidTvOutBlankHCount = 0x03200000;
                     break;
               }
               break;

               default:
                  sstIoRegs->vidTvOutBlankHCount = 0x03680048;
                  break;
            }
            break;
   }

   i2c_endaccess(pContext, key);

   return (0);
}


/*----------------------------------------------------------------------
Function name:  BT868_GetPosition

Description:    Get current horizontal/vertical position.

Information:

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
int BT868_GetPosition (PTVCURPOS pTvCurPos, void *tvOutData)
{
    pTvCurPos->dwCurLeft = ((TVOUT_CURSETUP *)tvOutData)->hpos;
    pTvCurPos->dwCurTop = ((TVOUT_CURSETUP *)tvOutData)->vpos;
    pTvCurPos->dwCurRight = 0;
    pTvCurPos->dwCurBottom = 0;
    return (0);
}


/*----------------------------------------------------------------------
Function name:  BT868_SetPosition

Description:    Set position.

Information:

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
#ifdef WINNT
int BT868_SetPosition(BT868CONTEXT pContext, PTVSETPOS pTvSetPosition)
#else
int BT868_SetPosition(PTVSETPOS pTvSetPosition, BT868CONTEXT pContext)
#endif
{
    bt868_position (pContext, pTvSetPosition->dwLeft, pTvSetPosition->dwTop);
    return(0);
}


/*----------------------------------------------------------------------
Function name:  tvstdToNVRAM

Description:    Update tv standard in nvram.  It is written to location 0.

Information:    Returns -1 on failure, 0 if OK or not an STB card.

Return:         int
----------------------------------------------------------------------*/
FxI32 tvstdToNVRAM (BT868CONTEXT pContext, FxU8 nvstd)
{

	if (!IS_3DFX_REF_BOARD(pContext))
	{
		return (nvramWrite(pContext, NV_TVSTANDARD, 1, &nvstd));
	}
	else
		return (0);
}


/*----------------------------------------------------------------------
Function name:  BT868_SetStandard

Description:    Set TV type.

Information:

Return:         INT     result of the BT868_Enable call. (TRUE if TvOut active)
----------------------------------------------------------------------*/
#ifdef WINNT
int BT868_SetStandard (BT868CONTEXT pContext, PTVSETSTANDARD pTvSetStandard)
{
	FxU8 mode;
	FxU8 ucTemp;

    mode = vpTvStdToBiosStd (pTvSetStandard->dwStandard);
    // write to nvram
    if (tvstdToNVRAM (pContext, mode) < 0)
        VideoDebugPrint((0,"tvstdToNVRAM failed\n"));
    ((TVOUT_CURSETUP *)&pContext->tvOutData)->tvStd = mode;

    if (IS_VOODOO3_X(pContext))
    {
        READSCRATCHREGISTER2(pContext,ucTemp);
        //mask off old standard
        ucTemp = ucTemp & ~BIOS_TVSTD_MASK;
        //or in new standard
        ucTemp = (ucTemp | mode);
        WRITESCRATCHREGISTER2 (pContext, ucTemp);
    }
    else
    {
        // maintain napalm style shared register 
        // tv standard is not in napalm shared register, nothing to do.
    }

    // if the tv is active make the new standard active.
    if (pContext->tvOutActive)
        return(BT868_Enable(pContext, -1));
    return (FXFALSE);

}
#else
int BT868_SetStandard (PTVSETSTANDARD pTvSetStandard, BT868CONTEXT pContext)
{
	FxU8 mode;
	FxU8 ucTemp;

  
   switch (pTvSetStandard->dwSubFunc)
   {
      case QUERYSETSTANDARD:
         switch (pTvSetStandard->dwStandard)
         {
            case 0:
               pContext->tvOutActive = FXTRUE;
               return (BT868_Enable (pContext, -1));

            default:
               mode = vpTvStdToBiosStd (pTvSetStandard->dwStandard);
               // write to nvram
               if (tvstdToNVRAM (pContext, mode) < 0)
            	   Debug_Printf (VNAME "tvstdToNVRAM failed\n");
               ((TVOUT_CURSETUP *)&pContext->tvOutData)->tvStd = mode;

               if (IS_VOODOO3_X(pContext))
               {
                   // maintain Voodoo3 style shared register
                   READSCRATCHREGISTER2(pContext,ucTemp);
                   //mask off old standard
                   ucTemp = ucTemp & ~BIOS_TVSTD_MASK;
                   //or in new standard
                   ucTemp = (ucTemp | mode);
                   WRITESCRATCHREGISTER2 (pContext, ucTemp);
               }
               else
               {
                   // maintain napalm style shared register 
                   // tv standard is not in napalm shared register, nothing to do.
               }

               if (pContext->tvOutActive)
                  return (BT868_Enable (pContext, -1));
               return (FXFALSE);
         }
         break;

      case QUERYSETOVERRIDE:
         SetConnectionType(pContext, pTvSetStandard);
         return FXTRUE;
   }

   return FXFALSE;
}
#endif


/*----------------------------------------------------------------------
Function name:  mul64

Description:    multiply a u32 by a u32 and return an __int64

Information:

Return:         __int64
----------------------------------------------------------------------*/
__int64 mul64 (FxU32 prodA, FxU32 prodB)
{
#ifdef WINNT
    __int64 term1;
    __int64 term2;
    term1 = prodA;
    term2 = prodB;
    return(term1*term2);

#else
    _asm
	{
		mov eax,[prodA]
		mul [prodB]
	}
#endif
}

/*----------------------------------------------------------------------
Function name:  div64

Description:    divide a __int64 by a u32 and return an u32

Information:

Return:         __int64
----------------------------------------------------------------------*/
FxU32 div64 (FxU32 *dividend, FxU32 divisor)
{
    FxU32 retval;

    _asm
    {
        mov edx,[dividend]
        mov eax,[edx]
        mov edx,[edx+4]
        div [divisor]
        mov [retval],eax
    }
    return (retval);
}

/*----------------------------------------------------------------------
Function name:  bt868_fixupNonStdModes

Description:    for little used PAL M, Nc and N fixup subcarrier, etc

Information:

Return:         nadda
----------------------------------------------------------------------*/
void bt868_fixupNonStdModes (BT868CONTEXT pContext, int extMode)
{
    TVOUT_CURSETUP *btSetup = (void *)&pContext->tvOutData;
    FxU32 subc;
    FxU32 clk, i, reg;
    __int64 uli;
    FxU8 palSetup[] = {0xf0, 0x57, 0x80, 0x48, 0x8c};
    FxU8 mode;

    // get mode corresponding to tvStd.
    getTvStdInfo (pContext, btSetup->tvStd, &mode, 0);

    // select subc depending on NTSC or PAL
    subc = (mode & 1) ? 4433619 : 3579545;

    switch (btSetup->tvStd)
    {
        case BIOS_PAL_M:
            subc = 3575611;
            bt_write(pContext, I2C_NOTAKEY, REGxA2, 0x2A);
            break;
        case BIOS_PAL_N:
            subc = 4433619;
            bt_write(pContext, I2C_NOTAKEY, REGxA2, 0x2E);
            break;
        case BIOS_PAL_Nc:
            subc = 3582056;
            bt_write(pContext, I2C_NOTAKEY, REGxA2, 0x24);
            break;
        default:        // no need to do anything
            return;
    }

    // setup sync_amp, burst_amp, mcr-y, mcb-y and mcy for PAL

    for (reg = 0xa4, i = 0; reg <= 0xac; reg += 2)
        bt_write (pContext, I2C_NOTAKEY, reg, palSetup[i++]);


    // subcarrier processing

    // master clk = (13500000 * PLL_INT.PLL_FRACT) / (65536 * 6)
    clk = (Bt868_RegShadow.regA0.ucPLL_INT << 16)  | 
             (Bt868_RegShadow.reg9E.ucPLL_FRACT << 8) |
          (Bt868_RegShadow.reg9C.ucPLL_FRACT);
    uli = mul64 (clk, 13500000);
    clk = div64 ((void *)&uli, 65536 * 6);
    // subc incr = (subcarrier_freq * 0x100000000) / master_clk
    uli = mul64 (0xffffffff, subc);
    clk = div64 ((void *)&uli, clk);

    bt_write(pContext, I2C_NOTAKEY, REGxAE, (FxU8) clk);        // subc mpx lsb
    bt_write(pContext, I2C_NOTAKEY, REGxB0, (FxU8) (clk >> 8));
    bt_write(pContext, I2C_NOTAKEY, REGxB2, (FxU8) (clk >> 16));
    bt_write(pContext, I2C_NOTAKEY, REGxB4, (FxU8) (clk >> 24));  // subc mpx msb

}

#ifndef WINNT
void BT868_StateOfCRT(BT868CONTEXT pContext, int State)
{
   FxU32 DacModeData;
   SstIORegs *sstIoRegs = (SstIORegs *)pContext->RegBase;

   DacModeData = sstIoRegs->dacMode;

   if (State)
   {
      if (pContext->dwAllowCRTwithTV || !pContext->tvOutActive)
      {
         DacModeData &= ~(SST_DAC_DPMS_ON_VSYNC | SST_DAC_DPMS_ON_HSYNC);
         sstIoRegs->dacMode = DacModeData;
      }
   }
   else
   {
      DacModeData |= SST_DAC_DPMS_ON_VSYNC;
      sstIoRegs->dacMode = DacModeData;
   }
}
#endif

/*----------------------------------------------------------------------
Function name:  BT868_Enable

Description:    Enable the TV device.

Information:

Return:         INT     FXTRUE if success,
                        FXFALSE if failure.
----------------------------------------------------------------------*/
int BT868_Enable (BT868CONTEXT pContext, int vgaMode)
{
    TVOUT_CURSETUP *btSetup = (void *)&pContext->tvOutData;
#ifdef WINNT
    int value;
    PH3_MEMBASE0 sstIoRegs = (PH3_MEMBASE0) pContext->MappedAddress[SST_IO_REGS_INDEX];
#else
	SstIORegs *sstIoRegs = (SstIORegs *)pContext->RegBase[HWINFO_SST_IOREGS_INDEX];
#endif
    FxU8 autoCfg;
    ULONG i;
    volatile FxU32 *pdwTvClkDly = &sstIoRegs->tmuGbeInit;

    TVSETCAP TvSetPicControl;
    ULONG xres = sstIoRegs->vidScreenSize & 0xfff;
    ULONG yres = (sstIoRegs->vidScreenSize >> 12) & 0xfff;
    TVSETSIZE tvSize;
    FxU8 bRegValue;

	//Setup as slave for TV-Out operation mode
    ULONG dwVidInFormat = //SST_VIDEOIN_VSYNC_POLARITY_LOW  |
                         //SST_VIDEOIN_HSYNC_POLARITY_LOW   |
                           SST_VIDEOIN_G4_FOR_POSEDGE       |   // brooktree mode
                           SST_VIDEOIN_GENLOCK_ENABLE       |   // use brooktree's clock
                                 SST_VIDEOIN_NOT_USE_VGA_TIMING   |   // use brooktree's syncs
                           SST_VIDEOIN_TVOUT_ENABLE         |   // aka H3_VMI_MODE_TV
                           SST_VIDEOIN_GENLOCK_SOURCE_TV    |   // for H4
                                 0;

    ULONG dwVidInMask =  // SST_VIDEOIN_VSYNC_POLARITY_LOW   |
                        // SST_VIDEOIN_HSYNC_POLARITY_LOW   |
                           SST_VIDEOIN_G4_FOR_POSEDGE       |   // brooktree mode
                           SST_VIDEOIN_GENLOCK_ENABLE       |   // use brooktree's clock
                                 SST_VIDEOIN_NOT_USE_VGA_TIMING   |   // use brooktree's syncs
                           SST_VIDEOIN_TVOUT_ENABLE         |   // aka H3_VMI_MODE_TV
                           SST_VIDEOIN_GENLOCK_SOURCE_TV    |   // for H4
                                 0;
    if (IS_NAPALM_X(pContext))
        // Setup mask to clear this bit. This bit formats data for DFPs if on.
        dwVidInMask |=     SST_VIDEOIN_TV_DATA_SCRAMBLE_DISABLE;
  


    if (xres > 800)
    {
        BT868_Disable (pContext);
#ifdef WINNT
        // force monitor to be on
        pContext->monitorActive = TRUE;
#endif
        return (FXFALSE);
    }

    // the following will be overidden later.  This temporarily gives a clock
    // to the bt868 so we can program it.
    sstIoRegs->vidInFormat &= ~dwVidInMask;
    sstIoRegs->vidInFormat |= SST_VIDEOIN_TVOUT_ENABLE;
    DELAY(20);     // delay 20uS for clock to settle

    btSetup->is_master = vgaMode < 0;
    getTvStdInfo (pContext, btSetup->tvStd, &autoCfg, 0);
        bt_write(pContext, I2C_NOTAKEY, REGxBA, BT86X_REGxBA_SRESET);

    // invalidate Register Shadow
    for (i = 0; i < sizeof(Bt868_RegShadow); i++)
        ((FxU8 *)&Bt868_RegShadow)[i] = 0;

    bt_write(pContext, I2C_NOTAKEY, REGxB8, autoCfg);

   // RYAN@990708, Needed for macrovision 7.1 compliance.
   switch (autoCfg)
   {
      // these numbers are bt869 autoconf numbers
      case 0: pContext->tvPixelClock = 1792; break;
      case 1: pContext->tvPixelClock = 1888; break;
      case 2: pContext->tvPixelClock = 2464; break;
      case 3: pContext->tvPixelClock = 2304; break;
   }

   switch (FX_RESOLUTION(xres,yres))
   {
      case FX_RESOLUTION(720,480):
         pContext->tvPixelClock = 1744;
         for (i=0x6C; i <= 0xD8; i+=2)
         {
            if (i == 0xB8)
               continue;
            else
               bt_write(pContext, I2C_NOTAKEY, i, BT868_720x480_NTSC[(i-0x6C)/2]);
         }
         break;

      case FX_RESOLUTION(720,576):
         pContext->tvPixelClock = 1776;
         for (i=0x6C; i <= 0xD8; i+=2)
         {
            if (i == 0xB8)
               continue;
            else
               bt_write(pContext, I2C_NOTAKEY, i, BT868_720x576_PAL[(i-0x6C)/2]);
         }     
         break;
   }

#ifdef WINNT
      switch (pContext->PCISubSystemID)
#else
      switch ((pContext->dwSubSystemID & 0x00FF0000) >> 16)
#endif
      {
         // The Voodoo3 3500TV has a different DAC configuration.
         case 0x00000060:
         case 0x00000061:
         case 0x00000062:
            bt_write (pContext, I2C_NOTAKEY, REGxBA, (FxU8) ((btSetup->cvbsOut ? 0x06 : 0x01)));
            break;

         default:
            bt_write (pContext, I2C_NOTAKEY, REGxBA, BT86X_REGxBA_DACDISA);       // disable DAC A
            break;
      }
      //Bt868_RegShadow.regBA |= 0x01;
      *pdwTvClkDly = (*pdwTvClkDly & ~0xf8000) | ((ClkDly[autoCfg] & 15) << 16L);

   if (ClkDly[autoCfg] & 0x10)
       dwVidInFormat |= SST_VIDEOIN_G4_FOR_POSEDGE;

   // setup DAC routing
   bt_write(pContext, I2C_NOTAKEY, REGxCE, (FxU8) (btSetup->cvbsOut ? 0 : 0x24)); 
   bt_write(pContext, I2C_NOTAKEY, REGxC6, 0); 
   if (btSetup->is_master)
   {
      bt_write(pContext, I2C_NOTAKEY, REGxC4, 0x01);   // enable pins
      bt_write(pContext, I2C_NOTAKEY, REGx70, 0x1f);   // set hsynwidth to 0x1f, and clear high part of hsynoffset
      bt_write(pContext, I2C_NOTAKEY, REGx6E, 0);   // set hsynoffset to zero
   }

    //maintain register shared with BIOS
    READSCRATCHREGISTER2(pContext,bRegValue);
    // turn on tv active bit
    if (IS_VOODOO3_X(pContext))
    {
        //maintain voodoo3 style register shared with BIOS
        bRegValue |= BIOS_TVOUT_ACTIVE;
    }
    else
    {
        // maintain napalm style shared register
        bRegValue |= BIOS_CRx1E_TVACTIVE;
    }
    WRITESCRATCHREGISTER2(pContext, bRegValue);

   // size tweaks
   switch (FX_RESOLUTION(xres,yres))
   {
      // we provide hard-coded (fixed) centering for these modes.
      case FX_RESOLUTION(720,480):
      case FX_RESOLUTION(720,576):
         break;

      default:
         tvSize.dwOverScan = btSetup->size;
#ifdef WINNT
         BT868_SetSize (pContext, &tvSize);
#else
         BT868_SetSize (&tvSize, pContext);
#endif
   }

   // position tweaks
   switch (FX_RESOLUTION(xres,yres))
   {
      case FX_RESOLUTION(720,480):
         // we provide hard-coded (fixed) centering for this mode.
         sstIoRegs->vidTvOutBlankHCount = 0x03140044;
         sstIoRegs->vidTvOutBlankVCount = 0x02000020;
         break;

      case FX_RESOLUTION(720,576):
         // we provide hard-coded (fixed) centering for this mode.
         sstIoRegs->vidTvOutBlankHCount = 0x03280058;
         sstIoRegs->vidTvOutBlankVCount = 0x02800028;
         break;

      case FX_RESOLUTION(640,400):
         // we provide hard-coded (fixed) centering for this mode.
         if (btSetup->tvStd == BIOS_NTSC)
            bt868_position (pContext, 70, 90);
         else
            bt868_position (pContext, 60, 70);
         break;
         
      default:
         break;
   }

    TvSetPicControl.dwCap = TV_BRIGHTNESS;
    TvSetPicControl.dwStep = btSetup->bright;
#ifdef WINNT
    BT868_SetPicControl (pContext, &TvSetPicControl);
#else
	BT868_SetPicControl (&TvSetPicControl, pContext);
#endif

    TvSetPicControl.dwCap = TV_SATURATION;
    TvSetPicControl.dwStep = btSetup->saturation;
#ifdef WINNT
    BT868_SetPicControl (pContext, &TvSetPicControl);
#else
	BT868_SetPicControl (&TvSetPicControl, pContext);
#endif

    TvSetPicControl.dwCap = TV_FLICKER;
    TvSetPicControl.dwStep = btSetup->filter;
#ifdef WINNT
    BT868_SetPicControl (pContext, &TvSetPicControl);
#else
	BT868_SetPicControl (&TvSetPicControl, pContext);
#endif

    sstIoRegs->vidInFormat |= (dwVidInFormat & dwVidInMask);

    // @RYAN, This is a better place to do this.
     BT868_CopyProtect (pContext, btSetup->copyProtectOn);

    pContext->tvOutActive = FXTRUE;

#ifdef WINNT
    // commit present state to the registry
    value = 1;
    VideoPortSetRegistryParameters(pContext,
            L"TV.enabled",
            &value,
            sizeof(int));

#else
   // this feature has not been ported to WinNT
   if (!pContext->dwAllowPALandCRT)
   {
      // we can't have the CRT turned on for PAL modes.
      BT868_StateOfCRT(pContext, (btSetup->tvStd == BIOS_NTSC));
   }
#endif

    return (FXTRUE);
}


/*----------------------------------------------------------------------
Function name:  BT868_GetPicControl

Description:    Get the picture control (Brightness, Saturation, Flicker).

Information:    WinNT and Win9x interface is different because WinNT assumes
                the tvPicValue parameter is output only.

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
#ifdef WINNT
int BT868_GetPicControl (PTVCURCAP tvPicValue, void *tvOutData, ULONG dwCap)
{
    // switch on Capability Requested
    switch (dwCap)
    {
        tvPicValue->dwCap = dwCap;
#else
int BT868_GetPicControl (PTVCURCAP tvPicValue, void *tvOutData)
{
    // switch on Capability Requested
	switch (tvPicValue->dwCap)
    {
#endif
        case TV_BRIGHTNESS:
            tvPicValue->dwStep = ((TVOUT_CURSETUP *)tvOutData)->bright;
            break;
        case TV_SATURATION:
            tvPicValue->dwStep = ((TVOUT_CURSETUP *)tvOutData)->saturation;
            break;
        case TV_FLICKER:
            tvPicValue->dwStep = ((TVOUT_CURSETUP *)tvOutData)->filter;
            break;
    }
    return (0);
}


/*----------------------------------------------------------------------
Function name:  BT868_GetFilterControl

Description:    Get the filter control (Flicker).

Information:

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
#ifdef WINNT
// notused by WINNT 3dfx Tools uses QUERYGETPICCONTROL for flicker filter
//int BT868_GetFilterControl (PTVCURCAP tvFilterValue, void *tvOutData, ULONG dwCap)
//{
//    // switch on Capability Requested
//    switch (dwCap)
//    {
//        tvFilterValue->dwCap = dwCap;
//       case TV_FLICKER:
//            tvFilterValue->dwStep = ((TVOUT_CURSETUP *)tvOutData)->filter;
//            break;
//    }
//    return (0);
//}
#else
int BT868_GetFilterControl (PTVCURCAP tvPicValue, void *tvOutData)
{
	tvPicValue->dwStep = ((TVOUT_CURSETUP *)tvOutData)->filter;
	return (0);
}
#endif

/*----------------------------------------------------------------------
Function name:  BT868_GetSizeControl

Description:    Get the horizontal/vertical contorl.

Information:

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
#ifdef WINNT
int BT868_GetSizeControl (BT868CONTEXT pContext, PTVCURSIZE tvSizeValue)
{
    TVOUT_CURSETUP *tvOutData = (void *)&pContext->tvOutData;

    tvSizeValue->dwCurHorInput = tvSizeValue->dwCurHorOutput = tvOutData->size;
    tvSizeValue->dwCurVerInput = tvSizeValue->dwCurVerOutput = tvOutData->size;
    return (0);
}
#else
int BT868_GetSizeControl (PTVCURSIZE tvPicValue, void *tvData)
{
	TVOUT_CURSETUP *tvOutData = tvData;

	tvPicValue->dwCurHorInput = tvPicValue->dwCurHorOutput = tvOutData->size;
	tvPicValue->dwCurVerInput = tvPicValue->dwCurVerOutput = tvOutData->size;
	return (0);
}
#endif

/*----------------------------------------------------------------------
Function name:  BT868_SetPicControl

Description:    Set the picture control (Brightness, Saturation, Flicker)

Information:

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
#ifdef WINNT
int BT868_SetPicControl (BT868CONTEXT pContext, PTVSETCAP pTvSetPicControl)
#else
int BT868_SetPicControl (PTVSETCAP pTvSetPicControl, BT868CONTEXT pContext)
#endif
{
   static const ULONG STEP_XLATE[] = { 0, 3, 2, 1, 4 };
   ULONG dwStep;
   TVOUT_CURSETUP *tvOutData = (void *)&pContext->tvOutData;
#ifdef WINNT
   PH3_MEMBASE0 sstIoRegs = (PH3_MEMBASE0) pContext->MappedAddress[SST_IO_REGS_INDEX];
#else
   SstIORegs *sstIoRegs = (SstIORegs *)pContext->RegBase[HWINFO_SST_IOREGS_INDEX];
#endif
   FxU8 bRegValue;
 
   
   switch (pTvSetPicControl->dwCap)
   {
      case TV_BRIGHTNESS:
         tvOutData->bright = pTvSetPicControl->dwStep;
         bt_write(pContext, I2C_NOTAKEY, REGxCA, (FxU8) (MIN( pTvSetPicControl->dwStep, 7) | 0xc0));
         break;
         
      case TV_SATURATION:
         tvOutData->saturation = pTvSetPicControl->dwStep;
         bt_write(pContext, I2C_NOTAKEY, REGxCC, (FxU8) (MIN( pTvSetPicControl->dwStep, 7) | 0xc0));
         break;               
               
      case TV_FLICKER:
         tvOutData->filter = pTvSetPicControl->dwStep;
         dwStep = STEP_XLATE[ MIN(pTvSetPicControl->dwStep, 4) ];
         bRegValue = (dwStep == 4) ? (FxU8)BT86X_REGxC8_DISFFILT : (FxU8)((dwStep << 3) | dwStep);
         bRegValue |= BT86X_REGxC8_DISYFLPF;
         bt_write(pContext, I2C_NOTAKEY, REGxC8, bRegValue);
         break;
               
      case TV_HUE:
      case TV_CONTRAST:
      case TV_GAMMA:
      case TV_SHARPNESS:
      default:
         return TV_CONTROL_UNSUPPORTED;
   }
   
   return(0);
}

#ifdef notused  //3dfx Tools uses QUERYSETPICCONTROL for flicker filter
/*----------------------------------------------------------------------
Function name:  BT868_SetFilterControl

Description:    Set the filter control (Flicker)

Information:

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
int BT868_SetFilterControl (BT868CONTEXT pContext, PTVSETCAP pTvSetFilterControl)
{
   static const ULONG STEP_XLATE[] = { 0, 3, 2, 1, 4 };
   ULONG dwStep;
   TVOUT_CURSETUP *tvOutData = (void *)&pContext->tvOutData;
#ifdef WINNT
   PH3_MEMBASE0 sstIoRegs = (PH3_MEMBASE0) pContext->MappedAddress[SST_IO_REGS_INDEX];
#else
   SstIORegs *sstIoRegs = (SstIORegs *)pContext->RegBase[HWINFO_SST_IOREGS_INDEX];
#endif
   FxU8 bRegValue;
 
   
   switch (pTvSetFilterControl->dwCap)
   {
      case TV_FLICKER:
         tvOutData->filter = pTvSetFilterControl->dwStep;
         dwStep = STEP_XLATE[ MIN(pTvSetFilterControl->dwStep, 4) ];
         bRegValue = (dwStep == 4) ? (FxU8)BT86X_REGxC8_DISFFILT : (FxU8)((dwStep << 3) | dwStep);
         bRegValue |= BT86X_REGxC8_DISYFLPF;
         bt_write(pContext, I2C_NOTAKEY, REGxC8, bRegValue);
         break;
               
      case TV_CHROMA:
      case TV_LUMA:
      default:
         return TV_CONTROL_UNSUPPORTED;
   }
   
   return(0);
}
#endif //def notused

/*----------------------------------------------------------------------
Function name:  BT868_SetSize

Description:    Set the picture size.

Information:

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
#ifdef WINNT
int BT868_SetSize(BT868CONTEXT pContext, PTVSETSIZE pTvSetSize)
#else
int BT868_SetSize(PTVSETSIZE pTvSetSize, BT868CONTEXT pContext)
#endif
{
#ifdef WINNT
    PH3_MEMBASE0 sstIoRegs = (PH3_MEMBASE0) pContext->MappedAddress[SST_IO_REGS_INDEX];
#else
	SstIORegs *sstIoRegs = (SstIORegs *)pContext->RegBase[HWINFO_SST_IOREGS_INDEX];
#endif
    TVOUT_CURSETUP *btSetup = (void *)&pContext->tvOutData;
    FxU32 vidInFmt = sstIoRegs->vidInFormat;
    ULONG xres = sstIoRegs->vidScreenSize & 0xfff;
    ULONG yres = (sstIoRegs->vidScreenSize >> 12) & 0xfff;
    int reg, i;
    const FxU8 *regdat;
    FxU8 mode;

    // size tweaks are only available in some modes
    switch (FX_RESOLUTION(xres,yres))
    {
    // this is a list of modes we can handle
    case FX_RESOLUTION(640,480):
    case FX_RESOLUTION(800,600):
        break;

    default:
        return(0);
    }

    // get mode corresponding to tvStd.
    getTvStdInfo (pContext, btSetup->tvStd, &mode, 0);

    // We have a problem.  When we blast the BT868 registers and we are in
    // master mode we drive Avenger crazy.  Syncs and clocks go every which
    // way.  So, even though it looks atrocious, disable genlock until register
    // load is done.

    if ((reg = SIZE_XLATE[pTvSetSize->dwOverScan]) >= 0)
    {
        regdat = Overscan[mode][reg].regVals;

        // RYAN@TVOUT, resizing affects our Macrovision support.
        pContext->tvPixelClock = ((((FxU16)regdat[8] & 0x000F) << 8) | ((FxU16)regdat[0] & 0x00FF));

        // check to see if we can skip the load
        for (reg = 0x76, i = 0; reg <= 0xb4; reg += 2, i++)
        {
            if (reg == 0x80)   // bt868_position will set this for us
                continue;
            if (((FxU8 *)&Bt868_RegShadow)[(reg - 0x76) / 2] != regdat[i])
                break;
        }
        if (reg < 0xb4)
        {
            sstIoRegs->vidInFormat = 0;  // genlock off
            bt_write(pContext, I2C_NOTAKEY, REGxB8, mode);
            for (reg = 0x76; reg <= 0xb4; reg += 2)
                bt_write(pContext, I2C_NOTAKEY, reg, *regdat++); 
            sstIoRegs->vidInFormat = vidInFmt;   // back
        }
    }
    else   // normal, ie. middle size
    {
        // RYAN@990708, Needed for macrovision 7.1 compliance.
        switch (mode)
        {
          // these numbers are bt869 autoconf numbers
          case 0: pContext->tvPixelClock = 1792; break;
          case 1: pContext->tvPixelClock = 1888; break;
          case 2: pContext->tvPixelClock = 2464; break;
          case 3: pContext->tvPixelClock = 2304; break;
        }

        // do the auto config and done
        bt_write (pContext, I2C_NOTAKEY, REGxB8, mode);

        // since we counted on the bt868 to setup the PLL regs we need to update
        // the shadow to what it really is FBO bt868_fixupNonStdModes
        Bt868_RegShadow.reg9C.ucPLL_FRACT = (UCHAR)AutoCfgFclk[mode];
        Bt868_RegShadow.reg9E.ucPLL_FRACT = (UCHAR)(AutoCfgFclk[mode] >> 8);
        Bt868_RegShadow.regA0.ucPLL_INT = (UCHAR)(AutoCfgFclk[mode] >> 16);
        Bt868_RegShadow.regA0.ucBY_PLL = Bt868_RegShadow.regA0.ucEN_XCLK = 0;
    }
    btSetup->size = pTvSetSize->dwOverScan;

    // Set hsynoffset and hsynwidth specifically for each mode
    bt_write(pContext, I2C_NOTAKEY, REGx6E, register6E[mode][btSetup->size]);   // set part of hsynoffset
    bt_write(pContext, I2C_NOTAKEY, REGx70, register70[mode][btSetup->size]);   // set hsynwidth and top part of hsynoffset

    // since we may have overwritten registers, rerun the following, it's cheap
    bt868_position (pContext, btSetup->hpos, btSetup->vpos);
    BT868_CopyProtect (pContext, btSetup->copyProtectOn);
    bt868_fixupNonStdModes (pContext, btSetup->tvStd);
    return(0);
}


/*----------------------------------------------------------------------
Function name:  BT868_GetStandard

Description:    Get the TV type.

Information:

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
#ifdef WINNT
int BT868_GetStandard (BT868CONTEXT pContext, LPTVGETSTANDARD lpOutput)
{
    ULONG vpTvStd;

    getTvStdInfo (pContext, ((TVOUT_CURSETUP *)&pContext->tvOutData)->tvStd, 0,
                                                                &vpTvStd);
    lpOutput->dwStandard = vpTvStd;
    return (0);                                              
}

FxI32 GetConnectionTypeFromNVRAM(BT868CONTEXT pContext, PTVGETOVERRIDE ptvstd)
{
    ptvstd->dwConnectorType = (FxU8)nvramRead(pContext, NV_CONNECTOR);
    return (0);
}
#else
// Win9x uses hacked GetStandard structure to return Connection Type.
FxU8 GetConnectionTypeFromNVRAM(BT868CONTEXT pContext, PTVSETSTANDARD ptvstd)
{

    ptvstd->dwStandard = 0;

	if (!IS_3DFX_REF_BOARD(pContext))
        ptvstd->dwStandard = nvramRead(pContext, NV_CONNECTOR);

	return (0);
}


/*----------------------------------------------------------------------
Function name:  BT868_GetStandard

Description:    Get the TV type.

Information:

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
int BT868_GetStandard (BT868CONTEXT pContext, LPTVSETSTANDARD lpOutput)
{
	ULONG vpTvStd;

   // MAJOR HACK-O-RAMA!!! WE'RE OVERLOADING "GET STANDARD"
   switch (lpOutput->dwSubFunc)
   {
      case QUERYGETSTANDARD:
         getTvStdInfo (pContext, ((TVOUT_CURSETUP *)&pContext->tvOutData)->tvStd, 0, &vpTvStd);
	      lpOutput->dwStandard = vpTvStd;
         break;

      default:
      case QUERYGETOVERRIDE:
         GetConnectionTypeFromNVRAM(pContext, lpOutput);
         break;
   }

	return (0);
}
#endif

/*----------------------------------------------------------------------
Function name:  BT868_GetSizeCap

Description:    Get the Min/Max size capabilities.

Information:

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
int BT868_GetSizeCap (BT868CONTEXT pContext, LPTVSIZECAP lpOutput)
{
    lpOutput->dwMaxHorInput = 800;
    lpOutput->dwMaxVerInput = 600;
    lpOutput->dwMaxHorOutput = 800;
    lpOutput->dwMaxVerOutput = 600;
    lpOutput->dwMinHorInput = 640;
    lpOutput->dwMinVerInput = 480;
    lpOutput->dwMinHorOutput = 640;
    lpOutput->dwMinVerOutput = 480;
    lpOutput->dwHorStepSize = 1;
    lpOutput->dwVerStepSize = 1;
    return (0);
}


/*----------------------------------------------------------------------
Function name:  BT868_GetPosCap

Description:    Get the position capabilities.

Information:

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
int BT868_GetPosCap (BT868CONTEXT pContext, LPTVPOSCAP lpOutput)
{
    lpOutput->dwMaxLeft = 0;
    lpOutput->dwMaxRight = 800;
    lpOutput->dwHorGranularity = 1;
    lpOutput->dwMaxTop = 0;
    lpOutput->dwMaxBottom = 600;
    lpOutput->dwVGAGranularity = 8;
    return (0);
}


#ifndef WINNT
// not used in WINNT, 3dfx Tools uses QUERYGETPICCAP for flicker filter
/*----------------------------------------------------------------------
Function name:  BT868_GetFilterCap

Description:    Get the filter capabilities.

Information:

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
int BT868_GetFilterCap (BT868CONTEXT pContext, LPTVCAPDATA lpOutput)
{
    switch (lpOutput->dwCap)
    {
        case TV_FLICKER:
            lpOutput->dwNumSteps = 5;
            break;
        default:
            lpOutput->dwNumSteps = 0;
    }
    return (0);
}
#endif //ndef WINNT

/*----------------------------------------------------------------------
Function name:  BT868_GetPicCap

Description:    Get the picture capabilities.

Information:

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
int BT868_GetPicCap (BT868CONTEXT pContext, LPTVCAPDATA lpOutput)
{
    switch (lpOutput->dwCap)
    {
        case TV_BRIGHTNESS:
        case TV_SATURATION:
            lpOutput->dwNumSteps = 8;
            break;
        case TV_FLICKER:
            lpOutput->dwNumSteps = 5;
            break;
        default:
            lpOutput->dwNumSteps = 0;
    }
    return (0);
}

#ifdef WINNT
int BT868_SetSpecial(BT868CONTEXT pContext, PTVSETSPECIAL pTvSetSpecial)
{
    //not currently used
    return (0);
}
#else
#endif


/*----------------------------------------------------------------------
Function name:  BT868_Disable

Description:    Disable the TV device.

Information:

Return:         INT     0 is always returned
----------------------------------------------------------------------*/
int BT868_Disable(BT868CONTEXT pContext)
{
#ifdef WINNT
   int value;
   PH3_MEMBASE0 sstIoRegs = (PH3_MEMBASE0) pContext->MappedAddress[SST_IO_REGS_INDEX];
#else
   SstIORegs *sstIoRegs = (SstIORegs *)pContext->RegBase[HWINFO_SST_IOREGS_INDEX];
#endif
   FxU8 bRegValue;

   ULONG dwVidInMask =  // SST_VIDEOIN_VSYNC_POLARITY_LOW   |
                        // SST_VIDEOIN_HSYNC_POLARITY_LOW   |
                           SST_VIDEOIN_GENLOCK_ENABLE       |
                                 SST_VIDEOIN_NOT_USE_VGA_TIMING   |
                           SST_VIDEOIN_TVOUT_ENABLE         |   // aka H3_VMI_MODE_TV
                           SST_VIDEOIN_GENLOCK_SOURCE_TV    |   // for H4
                                 0;

//   disable DACs when not using TV Out
   bt_write(pContext, I2C_NOTAKEY, REGxC4, 0x00);
   bt_write(pContext, I2C_NOTAKEY, REGxBA, 0x10);

   sstIoRegs->vidInFormat &= ~dwVidInMask;

   pContext->tvOutActive = FXFALSE;

// next line is temp code until 3dfx Tools works better ???
   if ( isPanelActive( pContext ) )
       panelOn(pContext);

#ifdef WINNT
   // commit present state to the registry
   value = 0;
   VideoPortSetRegistryParameters(pContext,
            L"TV.enabled",
            &value,
            sizeof(int));
#endif

    // let the BIOS know the TV is s'posed to be off
    READSCRATCHREGISTER2(pContext,bRegValue);
    // turn off tv active bit
    if (IS_VOODOO3_X(pContext))
    {
        //maintain voodoo3 style register shared with BIOS
        bRegValue &= ~BIOS_TVOUT_ACTIVE;
    }
    else
    {
        // maintain napalm style shared register
        bRegValue &= ~BIOS_CRx1E_TVACTIVE;
    }
    WRITESCRATCHREGISTER2(pContext, bRegValue);

#ifndef WINNT
   BT868_StateOfCRT(pContext, 1);
#endif

   return(0);
}

/* 
Formatting for save/restore of parameters.  This will be used by all TV
encoders, not just the bt.  This'll get moved eventually.
*/

#ifdef WINNT
wchar_t *TvOutParms[] = {
    L"TV.mode",             // this registry parameter is obsolete DanO 5/19/99
    L"TV.brightness",
    L"TV.saturation",
    L"TV.filter",
    L"TV.hposition",
    L"TV.vposition",
    L"TV.size",
    };
#else
char *TvOutParms[] = {
	"mode",             // this registry parameter is obsolete for Win9x DanO 12/6/99
	"brightness",
	"saturation",
	"filter",
	"hposition",
	"vposition",
	"size",
	};
#endif

#define NUM_TVPARMS   (sizeof(TvOutParms) / sizeof(TvOutParms[0]))


#ifdef WINNT
int BT868_CommitTvRegistry(BT868CONTEXT pContext) {
    int value;

        // Note: the "TV.mode" value in the registry is now obsolete. DanO 5/19/99

        value = ((TVOUT_CURSETUP *)&pContext->tvOutData)->bright;
        VideoPortSetRegistryParameters(pContext,
                           TvOutParms[1],
                           &value,
                           sizeof(int));
        value = ((TVOUT_CURSETUP *)&pContext->tvOutData)->saturation;
        VideoPortSetRegistryParameters(pContext,
                           TvOutParms[2],
                           &value,
                           sizeof(int));
        value = ((TVOUT_CURSETUP *)&pContext->tvOutData)->filter;
        VideoPortSetRegistryParameters(pContext,
                           TvOutParms[3],
                           &value,
                           sizeof(int));
        value = ((TVOUT_CURSETUP *)&pContext->tvOutData)->hpos;
        VideoPortSetRegistryParameters(pContext,
                           TvOutParms[4],
                           &value,
                           sizeof(int));
        value = ((TVOUT_CURSETUP *)&pContext->tvOutData)->vpos;
        VideoPortSetRegistryParameters(pContext,
                           TvOutParms[5],
                           &value,
                           sizeof(int));
        value = ((TVOUT_CURSETUP *)&pContext->tvOutData)->size;
        VideoPortSetRegistryParameters(pContext,
                           TvOutParms[6],
                           &value,
                           sizeof(int));

    return (0);
}

int BT868_RefreshTvRegistry(BT868CONTEXT pContext) {
    int value;

        // Note: the "TV.mode" value in the registry is now obsolete. DanO 5/19/99

        // get the value from the registry
        if (VideoPortGetRegistryParameters(pContext,
                                            TvOutParms[1],
                                            FALSE,
                                            H3RegistryCallback,
                                            &value) == NO_ERROR) 
            // set the value
            ((TVOUT_CURSETUP *)&pContext->tvOutData)->bright = value;

        // get the value from the registry
        if (VideoPortGetRegistryParameters(pContext,
                                            TvOutParms[2],
                                            FALSE,
                                            H3RegistryCallback,
                                            &value) == NO_ERROR) 
            // set the value
            ((TVOUT_CURSETUP *)&pContext->tvOutData)->saturation = value;

        // get the value from the registry
        if (VideoPortGetRegistryParameters(pContext,
                                            TvOutParms[3],
                                            FALSE,
                                            H3RegistryCallback,
                                            &value) == NO_ERROR) 
            // set the value
            ((TVOUT_CURSETUP *)&pContext->tvOutData)->filter = value;

        // get the value from the registry
        if (VideoPortGetRegistryParameters(pContext,
                                            TvOutParms[4],
                                            FALSE,
                                            H3RegistryCallback,
                                            &value) == NO_ERROR) 
            // set the value
            ((TVOUT_CURSETUP *)&pContext->tvOutData)->hpos = value;

        // get the value from the registry
        if (VideoPortGetRegistryParameters(pContext,
                                            TvOutParms[5],
                                            FALSE,
                                            H3RegistryCallback,
                                            &value) == NO_ERROR) 
            // set the value
            ((TVOUT_CURSETUP *)&pContext->tvOutData)->vpos = value;

        // get the value from the registry
        if (VideoPortGetRegistryParameters(pContext,
                                            TvOutParms[6],
                                            FALSE,
                                            H3RegistryCallback,
                                            &value) == NO_ERROR) 
            // set the value
            ((TVOUT_CURSETUP *)&pContext->tvOutData)->size = value;


    return (0);

}
#else
/*----------------------------------------------------------------------
Function name:  tvOutGetNextValue

Description:    Return TV capability values.

Information:    can not fail, given name, return current value.

Return:         INT     one of upto seven values.
----------------------------------------------------------------------*/
int tvOutGetNextValue (char *valueName, TVOUT_CURSETUP *tvOutData)
{
	FxU8 i, idx;

	tvOutData->lastValueIndex = tvOutData->lastValueIndex >= (NUM_TVPARMS - 1)
								? 0 : tvOutData->lastValueIndex + 1;
	idx = tvOutData->lastValueIndex;
    for (i = 0; valueName[i] = TvOutParms[idx][i]; i++) ;   // strcpy
	switch (idx)
	{
//obsolete		case 0: return (tvOutData->mode);
		case 1: return (tvOutData->bright);
		case 2: return (tvOutData->saturation);
		case 3: return (tvOutData->filter);
		case 4: return (tvOutData->hpos);
		case 5: return (tvOutData->vpos);
		case 6: return (tvOutData->size);
//		case 7: return (tvOutData->tvStd);
	}
	return (FXFALSE); // Line inserted here to satisfy -WX compiler option.
}


/*----------------------------------------------------------------------
Function name:  tvOutSetNextValue

Description:    Set TV capability values.

Information:    This shouldn't ever fail.  It should be called
                right after GetNext so LastValueIndex should point
                to the parm.  However, we check to make sure and
                return FXFALSE if the parameter name doesn't match
                what we expect.

Return:         INT     FXTRUE if success,
                        FXFALSE if failure.
----------------------------------------------------------------------*/
int tvOutSetNextValue (FxI8 *valueName, int value, TVOUT_CURSETUP *tvOutData)
{
	FxI8 idx = tvOutData->lastValueIndex;
	FxI8 *c = TvOutParms[idx];
	int i = 0;

	while (valueName[i] == c[i] && (c[i] && valueName[i]))    // strcmp
		i++;
	if (!c[i] && !valueName[i])
	{
		switch (idx)
		{
			case 0:
//obsolete				tvOutData->mode = value;
				return (FXTRUE);
			case 1:
				tvOutData->bright = value;
				return (FXTRUE);
			case 2:
				tvOutData->saturation = value;
				return (FXTRUE);
			case 3:
				tvOutData->filter = value;
				return (FXTRUE);
			case 4:
				tvOutData->hpos = value;
				return (FXTRUE);
			case 5:
				tvOutData->vpos = value;
				return (FXTRUE);
			case 6:
				tvOutData->size = value;
				return (FXTRUE);
		}
	}
	return (FXFALSE);
}
#endif //def WINNT

/*----------------------------------------------------------------------
Function name:  BT868_CopyProtect

Description:    Enable or Disable Macrovision encoding on bt869

Information:    setting = 0 means off, setting = 1 or 2 or 3 means on.

Return:         INT     1 or 0 --> 1 means sucess
----------------------------------------------------------------------*/
int BT868_CopyProtect (BT868CONTEXT pContext, int setting)
{
    BT868CONTEXT  regBase = pContext;
    FxU16 reg;
    FxU8 *regdat;
    FxU8 mode;
    FxU8 regvalue;
    FxU16 regvalue10;
    TVOUT_CURSETUP *btSetup = (void *)&pContext->tvOutData;

    // get mode corresponding to tvStd.
    getTvStdInfo (pContext, ((TVOUT_CURSETUP *)pContext->tvOutData)->tvStd, &mode, 0);

    setting &= 3;
    ((TVOUT_CURSETUP *)pContext->tvOutData)->copyProtectOn = setting;
    if (mode & 1)   // PAL
        regdat = MacroVisionRegsPAL[!!setting];
    else
        regdat = MacroVisionRegsNTSC[setting];

    for (reg = 0xda; reg <= 0xfc; reg += 2, regBase = NULL)
    {
        if (!bt_write(pContext, I2C_NOTAKEY, reg, *regdat++))
            break;
    }

    if (setting && (btSetup->tvStd == BIOS_NTSC))
    {
      //regvalue = (5.5 * pContext->tvPixelClock) / 63.5555
      //regvalue = (55000 * pContext->tvPixelClock) / 635555
      regvalue10 = (FxU16)(((FxU32)(550000 * pContext->tvPixelClock)) / (FxU32)635555);
      if ((regvalue10 % 10) >= 5)
         regvalue = (regvalue10 / 10) +1;  // round up
      else
         regvalue = (regvalue10 / 10);     // round down
      bt_write(pContext, I2C_NOTAKEY, REGx7C, regvalue); // HBURST_BEGIN

      //regvalue = ((8.0 * pContext->tvPixelClock) / 63.5555) - 128
      //regvalue = ((80000 * pContext->tvPixelClock) / 635555) - 128
      //regvalue = ((800000 * pContext->tvPixelClock) / 635555)  - 1280
      regvalue10 = (FxU16)((((FxU32)(800000 * pContext->tvPixelClock)) / (FxU32)635555) - 1280);
      if ((regvalue10 % 10) >= 5)
         regvalue = (regvalue10 / 10) +1;  // round up
      else
         regvalue = (regvalue10 / 10);     // round down
      bt_write(pContext, I2C_NOTAKEY, REGx7E, regvalue); // HBURST_END
    }

    if (reg != 0xfe)
      return 0;

    return 1;
}

#ifdef WINNT
int GetExtendedStatus (BT868CONTEXT  pContext)
{
    int tvStatus;

        tvStatus = BT868_GetStatus (pContext, 0);
        if (tvStatus == 0xffffffff)    // error
            return (tvStatus);
        tvStatus = (tvStatus << 8) | 
            BT868_GetStatus (pContext, 1);
        tvStatus = (tvStatus << 8) | 
            BT868_GetStatus (pContext, 2);

        tvStatus &= ~0x80;
        if (pContext->tvOutActive)
            tvStatus |= 0x80;
        return (tvStatus);
}
#else
/*----------------------------------------------------------------------
Function name:  BT868_FixupVGA

Description:    Set VGA CRTC registers for full screen DOS mode 3

                The timings in this routine must change whenever the BIOS
                is changed.
                
                My understanding is this routine must restore the
                timing registers to what the BIOS expects for the current
                mode (NTSC or PAL).  Because control is going to pass back
                to the BIOS without a mode set being done.

                The timings can't be restored to what they were when Windows
                started up because the Tv mode (NTSC or PAL) may have changed.

Information:    Returns 0

Return:         int
----------------------------------------------------------------------*/

int BT868_FixupVGA (BT868CONTEXT pContext, int unused)
{
	FxU8 dummy;
	FxU16 voodoo3_m3_625_regs[] = \
		{0x7100, 0x8003, 0x5804, 0x8005, 0x6f06, 0x3e07, 0xf010, 0};
	FxU16 voodoo3_m3_525_regs[] = \
		{0x5d00, 0x8003, 0x5504, 0x8005, 0x5606, 0x3e07, 0xf010, 0};
	FxU16 napalm_m3_625_regs[] = \
		{0x7100, 0x8003, 0x5804, 0xa005, 0x6f06, 0x3e07, 0xf010, 0};
	FxU16 napalm_m3_525_regs[] = \
		{0x7300, 0x8003, 0x6904, 0xa005, 0xe806, 0x1f07, 0xb010, 0};
	FxU16 *crtcRegs;
    FxU8 mode;
	
    // get mode corresponding to tvStd.
    getTvStdInfo (pContext, ((TVOUT_CURSETUP *)pContext->tvOutData)->tvStd, &mode, 0);

    if(IS_VOODOO3_X(pContext))
        crtcRegs = (mode & 1) ? voodoo3_m3_625_regs : voodoo3_m3_525_regs;
    else
        crtcRegs = (mode & 1) ? napalm_m3_625_regs : napalm_m3_525_regs;

	outpw ((FxU16)(pContext->IoBase + 0xc4), 0x0101);   // select 8 bit characters
	dummy = inp ((FxU16)(pContext->IoBase + 0xba));     // select address register
	outp ((FxU16)(pContext->IoBase + 0xc0), 0x33);      // select pixel panning reg
	outp ((FxU16)(pContext->IoBase + 0xc0), 0x00);      // and set it to 0

	outpw ((FxU16)(pContext->IoBase + 0xd4), 0x0c11);   // unlock VGA registers
	while (*crtcRegs)
		outpw ((FxU16)(pContext->IoBase + 0xd4), *crtcRegs++);
	outpw ((FxU16)(pContext->IoBase + 0xd4), 0x8c11);   // relock VGA registers
	return (0);
}

// We'll need this when we have more than one encoder to support

#include "nultvout.c"

const TVOUT_DEV_CALLS Bt868DevCalls = {
	BT868_GetStatus,
	BT868_GetPosition,
	BT868_SetStandard,
	BT868_SetPicControl,
	BT868_SetPosition,
	BT868_SetSize,
	NullTV_SetSpecial,
	BT868_Disable,
	BT868_GetPicControl,
	BT868_GetFilterControl,
	BT868_GetSizeControl,
	BT868_Enable,
    BT868_GetStandard,
    BT868_GetSizeCap,
    BT868_GetPosCap,
    BT868_GetFilterCap,
    BT868_GetPicCap,
	BT868_CopyProtect,
	BT868_FixupVGA
};


const TVOUT_DEV_CALLS NullTVDevCalls = {
	NullTV_GetStatus,
	NullTV_GetPosition,
	NullTV_SetStandard,
	NullTV_SetPicControl,
	NullTV_SetPosition,
	NullTV_SetSize,
	NullTV_SetSpecial,
	NullTV_Disable,
	NullTV_GetPicControl,
	NullTV_GetFilterControl,
	NullTV_GetSizeControl,
	NullTV_Enable,
    NullTV_GetStandard,
    NullTV_GetSizeCap,
    NullTV_GetPosCap,
    NullTV_GetFilterCap,
    NullTV_GetPicCap,
	NullTV_CopyProtect,
	NullTV_FixupVGA
};
#endif



FxU8 DetectTVConnection(BT868CONTEXT pContext)
{
   int Status;
   FxU8 bDetectMask;
   FxU8 bDetectSVID;
   FxU8 bDetectCVBS;
   FxU8 bDefault;

#ifdef WINNT
   switch (pContext->PCISubSystemID)
#else
   switch ((pContext->dwSubSystemID & 0x00FF0000) >> 16)
#endif
   {
      // The Voodoo3 3500TV has a different DAC configuration.
      case 0x00000060:
      case 0x00000061:
      case 0x00000062:
         bDefault = TV_TYPE_SVIDEO;
         bDetectMask = 0xE0;
         bDetectSVID = 0x60;  // DACs B and C are SVIDEO
         bDetectCVBS = 0x80;  // DAC A is COMPOSITE
         break;

      default:
         bDefault = TV_TYPE_COMPOSITE;
         bDetectMask = 0x60;
         bDetectSVID = 0x60; // DAC B and C are SVIDEO
         bDetectCVBS = 0x40; // could also be 0x20, but doesn't matter because of bDefault.
         break;
   }


   Status = BT868_GetStatus(pContext, 1);

   switch (Status & bDetectMask) // mask the detection bits
   {
      case 0x00:  // no connection detected
         return 0;

      default:
         if ((Status & bDetectMask) == bDetectSVID)
            return TV_TYPE_SVIDEO;
         else
         {
            if ((Status & bDetectMask) == bDetectCVBS)
               return TV_TYPE_COMPOSITE;
            else
               return bDefault;
         }
   }
}

FxU8 GetBiosTVStandardFromNVRAM(BT868CONTEXT pContext)
{
  if (!IS_3DFX_REF_BOARD(pContext))
    return (FxU8)nvramRead(pContext, NV_TVSTANDARD);

	return (FxU8)(0);
}




#ifdef WINNT
FxI32 SetConnectionType(BT868CONTEXT pContext, PTVSETOVERRIDE ptvstd)
#else
FxI32 SetConnectionType(BT868CONTEXT pContext, PTVSETSTANDARD ptvstd)
#endif
{
    FxU8 bStatusValue;
    FxU8 bDacRoutingCVBS;
    FxU8 bDacRoutingSVID;
    FxU8 bDacEnableCVBS;
    FxU8 bDacEnableSVID;
    FxU8 connType;
    TVOUT_CURSETUP *btSetup = (void *)&pContext->tvOutData;

#ifdef WINNT
    connType = (FxU8)ptvstd->dwConnectorType;
#else
    connType = (FxU8)ptvstd->dwStandard;
#endif
    nvramWrite(pContext, NV_CONNECTOR, 1, &connType);

    READSCRATCHREGISTER2(pContext,bStatusValue);

    if (IS_NAPALM_X(pContext))
        // Clear "TV Type Attached" bits
        bStatusValue &= ~BIOS_CRx1E_TVCONNECTED;

#ifdef WINNT
    switch (pContext->PCISubSystemID)
#else
    switch ((pContext->dwSubSystemID & 0x00FF0000) >> 16)
#endif
    {
    // The Voodoo3 3500TV has a different DAC configuration.
    case 0x00000060:
    case 0x00000061:
    case 0x00000062:
        bDacEnableSVID = 0x01;
        bDacEnableCVBS = 0x06;
        bDacRoutingSVID = 0x24;
        bDacRoutingCVBS = 0x24;
        break;

    default:
        bDacEnableSVID = 0x01;
        bDacEnableCVBS = 0x01;
        bDacRoutingSVID = 0x24;
        bDacRoutingCVBS = 0x00;
        break;
    }


    switch (connType)
    {
    case TV_TYPE_SVIDEO:
        if (IS_VOODOO3_X(pContext))
            bStatusValue &= ~BIOS_TVOUT_COMPOSITE;
        else
            bStatusValue |= BIOS_CRx1E_SVIDEO;
        btSetup->cvbsOut = 0;
        // setup DAC routing
        bt_write(pContext, I2C_NOTAKEY, REGxBA, bDacEnableSVID);
        bt_write(pContext, I2C_NOTAKEY, REGxCE, bDacRoutingSVID);
        break;

    case TV_TYPE_COMPOSITE:
        if (IS_VOODOO3_X(pContext))
            bStatusValue |= BIOS_TVOUT_COMPOSITE;
        else
            bStatusValue |= BIOS_CRx1E_COMPOSITE;
        btSetup->cvbsOut = 1;
        // setup DAC routing
        bt_write(pContext, I2C_NOTAKEY, REGxBA, bDacEnableCVBS);
        bt_write(pContext, I2C_NOTAKEY, REGxCE, bDacRoutingCVBS);
        break;

    case 0x00:  // Auto Connector Detection
    default:
        switch (DetectTVConnection(pContext))
        {
        case TV_TYPE_SVIDEO:
            if (IS_VOODOO3_X(pContext))
                bStatusValue &= ~BIOS_TVOUT_COMPOSITE;
            else
                bStatusValue |= BIOS_CRx1E_SVIDEO;
            btSetup->cvbsOut = 0;
            // setup DAC routing
            bt_write(pContext, I2C_NOTAKEY, REGxBA, bDacEnableSVID);
            bt_write(pContext, I2C_NOTAKEY, REGxCE, bDacRoutingSVID);
            break;

        default:
        case TV_TYPE_COMPOSITE:
            if (IS_VOODOO3_X(pContext))
                bStatusValue |= BIOS_TVOUT_COMPOSITE;
            else
                bStatusValue |= BIOS_CRx1E_COMPOSITE;
            btSetup->cvbsOut = 1;
            // setup DAC routing
            bt_write(pContext, I2C_NOTAKEY, REGxBA, bDacEnableCVBS);
            bt_write(pContext, I2C_NOTAKEY, REGxCE, bDacRoutingCVBS);
            break;
        }
        break;
    }

    WRITESCRATCHREGISTER2(pContext, bStatusValue);
    return (0);
}

FxU32 GetTvoutInfoNVRAM(BT868CONTEXT pContext)
{
#ifdef WINNT
   TVGETOVERRIDE Connector;
#else
   TVSETSTANDARD Connector;
#endif

   // we have the same sob story for connector detection; the BIOS
   // doesn't do this if a CRT is connected, so sometimes the connector
   // type reported by the BIOS is bogus; so if the type in the NVRAM
   // is "autodetect", detect the connection for ourselves.
   if (!GetConnectionTypeFromNVRAM(pContext, &Connector))
   {
#ifdef WINNT
      switch (Connector.dwConnectorType)
#else
// Win9x still uses hacked standard structure to return connector type.
      switch (Connector.dwStandard)
#endif
      {
         case TV_TYPE_SVIDEO:
            return TV_TYPE_SVIDEO;

         case TV_TYPE_COMPOSITE:
            return TV_TYPE_COMPOSITE;
            
         default:
            switch (DetectTVConnection(pContext))
            {
               case TV_TYPE_SVIDEO:
                  return TV_TYPE_SVIDEO;

               case TV_TYPE_COMPOSITE:
                  return TV_TYPE_COMPOSITE;

               default:
                  return  0; // meaning "no tv connected"
                  break;
            }
      }
   }

   return 0; // default to "no tv connected"
}

/*----------------------------------------------------------------------
Function name:  BT868_SetupTvOut

Description:    Initialize TV Out if the hardware is available.

Information:    

Return:         Nothing
----------------------------------------------------------------------*/
FxBool BT868_SetupTvOut(BT868CONTEXT pContext) 
{
#ifdef WINNT
  PH3_MEMBASE0 sstIoRegs = (PH3_MEMBASE0) pContext->MappedAddress[SST_IO_REGS_INDEX];
  TVOUT_CURSETUP *btSetup = (void *)&pContext->tvOutData;
  TVGETOVERRIDE Connector;
  int value;
  BOOLEAN OverrideTV = FALSE;
#else
  SstIORegs *sstIoRegs = (SstIORegs *)pContext->RegBase[HWINFO_SST_IOREGS_INDEX];
  TVOUT_CURSETUP *btSetup = (void *)&pContext->tvOutData;
  TVSETSTANDARD Connector;
#endif
  FxU8 bootStat;
  FxU8 biosStd;

    btSetup->encoderInitAttempt = 1;

#ifdef protoDfpBoard
    // insure the BT868 is not being held in reset
    sstIoRegs->vidSerialParallelPort |= SST_SERPAR_TVOUTRESET_N;
#endif  //def protoDfpBoard

    // BT868_GetStatus with estatus=0 returns the ID and the Version packed in a byte.
    // Assume that we never support a BT868 with version equal to zero.  This fixes a problem
    // with Velocity 100/200 boards where I2C routines happily read status from non-existing
    // BT868 without returning an error.

    // check for BT868 chip present
    if (BT868_GetStatus (pContext, 0) <= 0)
    {
        return FALSE;
    }

    // set some defaults that will be overwritten by app later
    btSetup->size = 2;   // middle size
    btSetup->hpos = 50;  // h centre
    btSetup->vpos = 50;  // v centre

#ifdef WINNT
    //??? the equivilant of this routine should also be done for Win9x.???TBD
    // Reload values from the registry in case we are booting directly to TvOut
    BT868_RefreshTvRegistry(pContext);
#endif

// read register shared with BIOS
    READSCRATCHREGISTER2(pContext,bootStat);
    if (IS_VOODOO3_X(pContext))
    {
        btSetup->tvStd = bootStat & BIOS_TVSTD_MASK;
        btSetup->cvbsOut = !!(bootStat & BIOS_TVOUT_COMPOSITE);
        btSetup->tvBoot = !!(bootStat & BIOS_TVOUT_ACTIVE);
    }
    else
    {
        btSetup->tvStd = BIOS_NTSC;  // default to NTSC
        btSetup->cvbsOut = !!(bootStat & BIOS_CRx1E_COMPOSITE);
        btSetup->tvBoot = !!(bootStat & BIOS_CRx1E_TVACTIVE);
    }
#ifdef WINNT
    pContext->tvOutActive = (BOOLEAN) btSetup->tvBoot;
#else
    pContext->tvOutActive = btSetup->tvBoot;
#endif

// Override what shared BIOS scratch register says

    // we can't always trust what the BIOS says the TV standard is in the shared scratch reg.
    // like when we boot to a CRT.  So always go by what the NVRAM says, instead.
    biosStd = GetBiosTVStandardFromNVRAM(pContext);

    // if a tv standard is specified in NVRAM then use it.
    if (biosStd)
    {
      // TV Standard is not stored in BIOS ScratchRegister2 for Napalm.
      if (IS_VOODOO3_X(pContext))
      {
          bootStat &= ~BIOS_TVSTD_MASK;
          bootStat |= (biosStd & BIOS_TVSTD_MASK);
      }
      else
      {
          // Clear "TV Type Attached" bits
          bootStat &= ~BIOS_CRx1E_TVCONNECTED;
      }
 		
      btSetup->tvStd = biosStd & BIOS_TVSTD_MASK;
    }


    // we have the same sob story for connector detection; the BIOS
    // doesn't do this if a CRT is connected, so sometimes the connector
    // type reported by the BIOS is bogus; so if the type in the NVRAM
    // is "autodetect", detect the connection for ourselves.
    if (!GetConnectionTypeFromNVRAM(pContext, &Connector))
    {
#ifdef WINNT
      switch (Connector.dwConnectorType)
#else
// Win9x uses hacked tvstandard structure
      switch (Connector.dwStandard)
#endif
#ifdef WINNT
      {
         case TV_TYPE_SVIDEO:
            btSetup->cvbsOut = 0;
            break;

         case TV_TYPE_COMPOSITE:
            btSetup->cvbsOut = 1;
            break;
            
         default:
            switch (DetectTVConnection(pContext))
            {
               case TV_TYPE_SVIDEO:
                  btSetup->cvbsOut = 0;
                  break;

               default:
               case TV_TYPE_COMPOSITE:
                  btSetup->cvbsOut = 1;
                  break;
            }
      }
      // let the BIOS know what connector type we are using.

      if (IS_VOODOO3_X(pContext))
      {
          //mask off old connector type
          bootStat = (bootStat & ~BIOS_TVOUT_COMPOSITE);
          //or in new connector type
          if (btSetup->cvbsOut == 1)
              bootStat = (bootStat | BIOS_TVOUT_COMPOSITE);
      }
      else
      {
          //mask off old connector type
          bootStat = (bootStat & ~BIOS_CRx1E_TVCONNECTED);
          //or in new connector type
          if (btSetup->cvbsOut == 1)
              bootStat = (bootStat | BIOS_CRx1E_COMPOSITE);
          else
              bootStat = (bootStat | BIOS_CRx1E_SVIDEO);
      }

#else
      {
         case TV_TYPE_SVIDEO:
            if (IS_VOODOO3_X(pContext))
                bootStat &= ~BIOS_TVOUT_COMPOSITE;
            else
                bootStat |= BIOS_CRx1E_SVIDEO;
            btSetup->cvbsOut = 0;
            break;

         case TV_TYPE_COMPOSITE:
            if (IS_VOODOO3_X(pContext))
                bootStat |= BIOS_TVOUT_COMPOSITE;
            else
                bootStat |= BIOS_CRx1E_COMPOSITE;
            btSetup->cvbsOut = 1;
            break;
            
         default:
            switch (DetectTVConnection(pContext))
            {
               case TV_TYPE_SVIDEO:
                  if (IS_VOODOO3_X(pContext))
                      bootStat &= ~BIOS_TVOUT_COMPOSITE;
                  else
                      bootStat |= BIOS_CRx1E_SVIDEO;
                  btSetup->cvbsOut = 0;
                  break;

               default:
               case TV_TYPE_COMPOSITE:
                  if (IS_VOODOO3_X(pContext))
                      bootStat |= BIOS_TVOUT_COMPOSITE;
                  else
                      bootStat |= BIOS_CRx1E_COMPOSITE;
                  btSetup->cvbsOut = 1;
                  break;
            }
      }
#endif
    }


#ifdef WINNT
    // if we didn't boot to the TV, but the user shut down while
    // on the TV last time, we want to go there now, UNLESS one of
    // the following is true:
    //    --There is no TV connected (@TODO: Implement this!)
    //    --The only reason we were on the TV last time was because
    //      no monitor was present, but had there been, we would have been
    //      on the monitor
    if (!pContext->tvOutActive)
    {
        // get the value from the registry
        if (VideoPortGetRegistryParameters(pContext,
                L"TV.enabled",
                FALSE,
                H3RegistryCallback,
                &value) != NO_ERROR) 
            // default to "off"
            value = 0;

        // if "TV.enabled" == TRUE
        if (value)
        {
            // hmm... registry says use the TV.  If the reason for this was
            // because no monitor was present last time, then we should ignore
            // this, because there IS a monitor present THIS time.

            // get the value from the registry
            if (VideoPortGetRegistryParameters(pContext,
                    L"TV.priorbootTV",
                    FALSE,
                    H3RegistryCallback,
                    &value) != NO_ERROR) 
                // default to "no"
                value = 0;


            pContext->tvOutActive = value ? FALSE : TRUE;
        }
        else
            pContext->tvOutActive = FALSE;

        // if the last time we booted up, there was a tv, but this time:
        //    --there is no tv
        //    --the "connector" setting is set to "auto"
        // then we should ignore the registry and boot up to the CRT.
        if (pContext->tvOutActive)
        {
            pContext->tvOutActive = !!GetTvoutInfoNVRAM(pContext);
            if (!pContext->tvOutActive)
            {
                OverrideTV = TRUE; // don't commit to registry
            }
        }
    }

    // commit our decisions back to the registry
    value = OverrideTV ? 1 : pContext->tvOutActive;  // are we presently active?
    VideoPortSetRegistryParameters(pContext,
            L"TV.enabled",
            &value,
            sizeof(int));

    value = OverrideTV ? 0 : btSetup->tvBoot;  // did we boot to TV this time?
    VideoPortSetRegistryParameters(pContext,
            L"TV.priorbootTV",
            &value,
            sizeof(int));

#endif

    // now let the BIOS know what we decided, so that DOS boxen will work right.
    WRITESCRATCHREGISTER2(pContext, bootStat);

    return(TRUE);  // result has no meaning for Win9x

}

#ifdef WINNT
/*----------------------------------------------------------------------
Function name:  BT868_ProcessVideoParameters

Description:    Process the IOCTL_VIDEO_HANDLE_VIDEOPARAMETERS request

Information:    
  
Return:         0 for sucess, -1 for failure
----------------------------------------------------------------------*/
int BT868_ProcessVideoParameters (BT868CONTEXT pContext, PVIDEO_REQUEST_PACKET RequestPacket)
{
    union
    {
        TVSETSTANDARD tvSetStd;
        TVSETCAP tvSetCap;
        TVSETPOS tvSetPos;
        TVSETSIZE tvSetSize;
        TVGETSTANDARD tvGetStd;
        TVCURCAP tvCurCap;
        TVCURPOS tvCurPos;
        TVCURSIZE tvCurSize;
        TVSETSPECIAL tvSetSpecial;
    }  ioBuf;
    ULONG tvStatus;
    ULONG dwPal    = VP_TV_STANDARD_PAL_B | VP_TV_STANDARD_PAL_D |  \
            VP_TV_STANDARD_PAL_H | VP_TV_STANDARD_PAL_I;
    TVPACKET requestPkt;
    ULONG vpTvStd;
    VIDEOPARAMETERS  videoParametersIn;
    PVIDEOPARAMETERS  pVideoParametersOut = RequestPacket->OutputBuffer;
    TVOUT_CURSETUP *btSetup = (void *)&pContext->tvOutData;


    if (sizeof(VIDEOPARAMETERS) >  RequestPacket->InputBufferLength)
    {
        VideoDebugPrint((0,"VideoParameters input buffer too small\n"));
		return (ERROR_INSUFFICIENT_BUFFER);
    }

    // check the GUID
    if (memcmp (&vpguid, &((PVIDEOPARAMETERS)(RequestPacket->InputBuffer))->Guid, sizeof(vpguid)))
		return (ERROR_INVALID_PARAMETER);
  

    if (RequestPacket->OutputBufferLength < sizeof(VIDEOPARAMETERS))
    {
        VideoDebugPrint((0,"VideoParameters buffer size mismatch\n"));
		return (ERROR_INVALID_PARAMETER);
    }

    // Create local copy of input buffer because caller may use same buffer for both input and output.
    memcpy( &videoParametersIn, RequestPacket->InputBuffer, sizeof(VIDEOPARAMETERS) );


    // if no tvOut and a set command
    if ((!pContext->tvOutCapable) && (videoParametersIn.dwCommand != VP_COMMAND_GET))
		return (ERROR_DEV_NOT_EXIST); 

    RequestPacket->StatusBlock->Information = sizeof(VIDEOPARAMETERS);


    if (videoParametersIn.dwCommand == VP_COMMAND_GET)
    {
        memset (RequestPacket->OutputBuffer, 0, sizeof(VIDEOPARAMETERS));
        pVideoParametersOut->Guid = videoParametersIn.Guid;
        pVideoParametersOut->dwCommand = videoParametersIn.dwCommand;

        if (!pContext->tvOutCapable)
        {   // no tvout adapter, apparently
			return (NO_ERROR);
        }

        // GetStatus with estatus=0 returns the ID and the Version packed in a byte.
        tvStatus = BT868_GetStatus(pContext,0);

        if (((tvStatus >> 5) & 7) == 1)    // is a BT869
        {
            pVideoParametersOut->dwCPType = \
                    VP_CP_TYPE_MACROVISION | VP_CP_TYPE_APS_TRIGGER;
            pVideoParametersOut->dwCPStandard = dwPal | VP_TV_STANDARD_NTSC_M;
        }

        pVideoParametersOut->dwFlags = VP_FLAGS_TV_MODE |
                VP_FLAGS_TV_STANDARD |
                VP_FLAGS_FLICKER |
                VP_FLAGS_OVERSCAN |
                VP_FLAGS_MAX_UNSCALED |
                VP_FLAGS_POSITION |
                VP_FLAGS_BRIGHTNESS |
                VP_FLAGS_CONTRAST |    // actually chroma
                VP_FLAGS_COPYPROTECT;

        pVideoParametersOut->dwMode = 0;
        if ((pContext->monitorActive) || isPanelActive( pContext ) )
            pVideoParametersOut->dwMode |= VP_MODE_WIN_GRAPHICS;
        if (pContext->tvOutActive)
        {
            pVideoParametersOut->dwMode |= VP_MODE_TV_PLAYBACK;
            getTvStdInfo (pContext, btSetup->tvStd, 0, &vpTvStd);
            pVideoParametersOut->dwTVStandard = vpTvStd;
        }
        else
            pVideoParametersOut->dwTVStandard = VP_TV_STANDARD_WIN_VGA;

        pVideoParametersOut->dwMaxUnscaledX = 800;
        pVideoParametersOut->dwMaxUnscaledY = 600;


        pVideoParametersOut->dwAvailableModes = VP_MODE_WIN_GRAPHICS | VP_MODE_TV_PLAYBACK;
        pVideoParametersOut->dwAvailableTVStandard = dwPal | VP_TV_STANDARD_NTSC_M |
                VP_TV_STANDARD_PAL_NC | VP_TV_STANDARD_PAL_N | VP_TV_STANDARD_PAL_M;

        BT868_GetPicControl (&ioBuf.tvCurCap, &pContext->tvOutData, TV_FLICKER);
        pVideoParametersOut->dwFlickerFilter = ioBuf.tvCurCap.dwStep * 250;

        BT868_GetPosition( &ioBuf.tvCurPos, &pContext->tvOutData);
        pVideoParametersOut->dwPositionX = ioBuf.tvCurPos.dwCurLeft;
        pVideoParametersOut->dwPositionY = ioBuf.tvCurPos.dwCurTop;

        BT868_GetPicControl (&ioBuf.tvCurCap, &pContext->tvOutData, TV_BRIGHTNESS);
        if ((pVideoParametersOut->dwBrightness = ioBuf.tvCurCap.dwStep * 14) == 98)
            pVideoParametersOut->dwBrightness = 100;

        BT868_GetPicControl (&ioBuf.tvCurCap, &pContext->tvOutData, TV_SATURATION);
        if ((pVideoParametersOut->dwContrast = ioBuf.tvCurCap.dwStep * 14) == 98)
            pVideoParametersOut->dwContrast = 100;

        BT868_GetSizeControl( pContext, &ioBuf.tvCurSize);
        pVideoParametersOut->dwOverScanX = ioBuf.tvCurSize.dwCurHorOutput * 250;
        pVideoParametersOut->dwOverScanY = ioBuf.tvCurSize.dwCurVerOutput * 250;

		return (NO_ERROR);
    }
    else if (videoParametersIn.dwCommand == VP_COMMAND_SET)
    {
        if (videoParametersIn.dwFlags & VP_FLAGS_TV_MODE)
        {
#ifdef notimplemented
            _asm {int 3};
            if (videoParametersIn.dwMode == VP_MODE_WIN_GRAPHICS)
            {
                // set flicker filter and overscan optimal for Windows desktop.
                _asm {int 3};  //??? not implemented
            }
            else
            {
                // set flicker filter and overscan optimal for video playback.
                _asm {int 3};  //??? not implemented
            }
#endif
        }
        else if (videoParametersIn.dwFlags & VP_FLAGS_FLICKER)
        {
            ioBuf.tvSetCap.dwCap = TV_FLICKER;
            ioBuf.tvSetCap.dwStep = videoParametersIn.dwFlickerFilter / 250;
            BT868_SetPicControl (pContext, &ioBuf.tvSetCap);
        }
        else if (videoParametersIn.dwFlags & VP_FLAGS_TV_STANDARD)
        {
            if (videoParametersIn.dwTVStandard == 0)
            {
                // Warning :  Ignore setting the standard to zero, to be compatible with Win9x kludge.
				return (NO_ERROR);
            }
            ioBuf.tvSetStd.dwStandard = videoParametersIn.dwTVStandard;
            BT868_SetStandard (pContext, &ioBuf.tvSetStd);
        }
        else if (videoParametersIn.dwFlags & VP_FLAGS_POSITION)
        {
            bt868_position (pContext, videoParametersIn.dwPositionX, videoParametersIn.dwPositionY);
        }
        else if (videoParametersIn.dwFlags & VP_FLAGS_OVERSCAN)
        {
            ioBuf.tvSetSize.dwOverScan =  \
                    (videoParametersIn.dwOverScanX + videoParametersIn.dwOverScanY) / 500;
            if (ioBuf.tvSetSize.dwOverScan > (2000 / 400))
				return (ERROR_INVALID_PARAMETER);

            BT868_SetSize(pContext, &ioBuf.tvSetSize);
        }
        else if (videoParametersIn.dwFlags & VP_FLAGS_BRIGHTNESS)
        {
            ioBuf.tvSetCap.dwCap = TV_BRIGHTNESS;
            ioBuf.tvSetCap.dwStep = videoParametersIn.dwBrightness / 14;
            BT868_SetPicControl (pContext, &ioBuf.tvSetCap);
        }
        else if (videoParametersIn.dwFlags & VP_FLAGS_CONTRAST)
        {
            ioBuf.tvSetCap.dwCap = TV_SATURATION;
            ioBuf.tvSetCap.dwStep = videoParametersIn.dwContrast / 14;
            BT868_SetPicControl (pContext, &ioBuf.tvSetCap);
        }
        else if (videoParametersIn.dwFlags & VP_FLAGS_COPYPROTECT)
        {
            switch (videoParametersIn.dwCPCommand)
            {
            case VP_CP_CMD_ACTIVATE:
                pContext->copyProtectKey = videoParametersIn.dwCPKey;  // remember this key

                if (BT868_CopyProtect (pContext, videoParametersIn.bCP_APSTriggerBits & 3))
                {
                    VideoDebugPrint((0,"Macrovision On okay\n"));
                }
                else
					return (ERROR_INVALID_PARAMETER);

                break;
            case VP_CP_CMD_DEACTIVATE:
                // don't disable copy protect unless key matches
                if (pContext->copyProtectKey == videoParametersIn.dwCPKey)
                {
                    if (BT868_CopyProtect (pContext, 0))
                    {
                        VideoDebugPrint((0,"Macrovision Off okay\n"));
                    }
                    else
						return (ERROR_INVALID_PARAMETER);
                }
                break;
            case VP_CP_CMD_CHANGE:
                // don't change copy protect unless key matches
                if (pContext->copyProtectKey == videoParametersIn.dwCPKey)
                {
                    if (BT868_CopyProtect (pContext, videoParametersIn.bCP_APSTriggerBits & 3))
                    {
                        VideoDebugPrint((0,"Macrovision changed okay\n"));
                    }
                    else
						return (ERROR_INVALID_PARAMETER);

                }
            }
        }
		return (NO_ERROR);
    }
    else
		return (ERROR_INVALID_PARAMETER);
}


int BT868_ProcessRequest(BT868CONTEXT pContext, PVIDEO_REQUEST_PACKET RequestPacket)
{
    int status;
    ULONG tvStatus;
    UCHAR ucTemp;

    if (sizeof(ULONG) >  RequestPacket->InputBufferLength)
        {
        VideoDebugPrint((0,"TVPACKET input buffer too small\n"));
		return (ERROR_INSUFFICIENT_BUFFER);
        }

                  
    switch (((PTVPACKET)RequestPacket->InputBuffer)->tvPacketFunc)
    {
    case tvGetStatus:
	
		status = ERROR_INVALID_PARAMETER;

        if (RequestPacket->OutputBufferLength < (RequestPacket->StatusBlock->Information = sizeof(ULONG)))
          {
          VideoDebugPrint((0,"TVSTATUS buffer size mismatch\n"));
          break;
          }
        tvStatus = GetExtendedStatus (pContext);

		if (tvStatus == ERROR_INVALID_PARAMETER)
        {
            ((ULONG *)RequestPacket->OutputBuffer)[0] = 0;
            break;
        }
        ((ULONG *)RequestPacket->OutputBuffer)[0] = tvStatus;

		status = NO_ERROR;
        break;
    case tvSetStandard:

        if ((sizeof(ULONG)+sizeof(TVSETSTANDARD)) >  RequestPacket->InputBufferLength)
          {
          VideoDebugPrint((0,"TVSETSTANDARD buffer size mismatch\n"));
		  status = ERROR_INVALID_PARAMETER;
          break;
          }
        status = \
            BT868_SetStandard (pContext, &((PTVPACKET)RequestPacket->InputBuffer)->tvOptData.tvStandard);
        break;
    case tvSetConnOverride:
        if ((sizeof(ULONG)+sizeof(TVSETOVERRIDE)) >  RequestPacket->InputBufferLength)
          {
          VideoDebugPrint((0,"TVSETOVERRIDE buffer size mismatch\n"));
		  status = ERROR_INVALID_PARAMETER;
          break;
          }
        status = \
            SetConnectionType (pContext, &((PTVPACKET)RequestPacket->InputBuffer)->tvOptData.tvConnOverride);
        break;
    case tvSetPicCtrl:
        if ((sizeof(ULONG)+sizeof(TVSETCAP)) >  RequestPacket->InputBufferLength)
          {
          VideoDebugPrint((0,"TVSETCAP buffer size mismatch\n"));
		  status = ERROR_INVALID_PARAMETER;
          break;
          }
        status =  \
        BT868_SetPicControl (pContext, &((PTVPACKET)RequestPacket->InputBuffer)->tvOptData.tvCapability);
        break;
#ifdef notused  //3dfx Tools uses QUERYSETPICCONTROL for flicker filter
    case tvSetFilterCtrl:
        _asm {int 3};
        if ((sizeof(ULONG)+sizeof(TVSETCAP)) >  RequestPacket->InputBufferLength)
          {
          VideoDebugPrint((0,"TVSETCAP buffer size mismatch\n"));
		  status = ERROR_INVALID_PARAMETER;
          break;
          }
        status =  \
        BT868_SetFilterControl (pContext, &((PTVPACKET)RequestPacket->InputBuffer)->tvOptData.tvCapability);
        break;
#endif  //def notused
    case tvSetPositionCtrl:
        if ((sizeof(ULONG)+sizeof(TVSETPOS)) >  RequestPacket->InputBufferLength)
          {
          VideoDebugPrint((0,"TVSETPOS buffer size mismatch\n"));
		  status = ERROR_INVALID_PARAMETER;
          break;
          }
        status =  \
        BT868_SetPosition (pContext, &((PTVPACKET)RequestPacket->InputBuffer)->tvOptData.tvPosition);
        break;
    case tvSetSizeCtrl:
        if ((sizeof(ULONG)+sizeof(TVSETSIZE)) >  RequestPacket->InputBufferLength)
          {
          VideoDebugPrint((0,"TVSETSIZE buffer size mismatch\n"));
		  status = ERROR_INVALID_PARAMETER;
          break;
          }
        status = \
        BT868_SetSize (pContext, &((PTVPACKET)RequestPacket->InputBuffer)->tvOptData.tvSize);
        break;
    case tvSetSpecial:
#ifdef WINNT
        if ((sizeof(ULONG)+sizeof(TVSETSPECIAL)) >  RequestPacket->InputBufferLength)
          {
          VideoDebugPrint((0,"TVSETSPECIAL buffer size mismatch\n"));
		  status = ERROR_INVALID_PARAMETER;
          break;
          }
        status =  \
        BT868_SetSpecial(pContext, &((PTVPACKET)RequestPacket->InputBuffer)->tvOptData.tvSpecial);
        break;
#endif
    case tvDisable:
        status = BT868_Disable (pContext);
        break;
    case tvEnable:
        status = BT868_Enable (pContext, -1);
        break;
    case tvGetPositionCtrl:
        if (RequestPacket->OutputBufferLength < (RequestPacket->StatusBlock->Information = sizeof(TVCURPOS)))
          {
          VideoDebugPrint((0,"TVCURPOS buffer size mismatch\n"));
		  status = ERROR_INVALID_PARAMETER;
          break;
          }
        status =  \
        BT868_GetPosition ((PTVCURPOS)RequestPacket->OutputBuffer, &pContext->tvOutData);
        break;
    case tvGetPicCtrl:
        if (RequestPacket->OutputBufferLength < (RequestPacket->StatusBlock->Information = sizeof(TVCURCAP)))
          {
          VideoDebugPrint((0,"TVPIC_CTRL buffer size mismatch\n"));
		  status = ERROR_INVALID_PARAMETER;
          break;
          }
        status =  \
        BT868_GetPicControl ((PTVCURCAP)RequestPacket->OutputBuffer,
                          &pContext->tvOutData,
                          ((PTVPACKET)RequestPacket->InputBuffer)->tvOptData.tvCurCap.dwCap);
        break;
#ifdef notused  //3dfx Tools uses QUERYGETPICCONTROL for flicker filter
    case tvGetFilterCtrl:
        _asm {int 3};
        if (RequestPacket->OutputBufferLength < (RequestPacket->StatusBlock->Information = sizeof(TVCURCAP)))
          {
          VideoDebugPrint((0,"TVPIC_CTRL buffer size mismatch\n"));
		  status = ERROR_INVALID_PARAMETER;
          break;
          }
        status =  \
        BT868_GetFilterControl ((PTVCURCAP)RequestPacket->OutputBuffer,
                                &pContext->tvOutData,
                                ((PTVPACKET)RequestPacket->InputBuffer)->tvOptData.tvCurCap.dwCap);
        break;
#endif  //def notused
    case tvRefreshRegistry:
        status = BT868_RefreshTvRegistry(pContext);
        break;
    case tvCommitRegistry:
        status = BT868_CommitTvRegistry(pContext);
        break;
    case tvGetStandard:
        if (RequestPacket->OutputBufferLength < (RequestPacket->StatusBlock->Information = sizeof(TVGETSTANDARD)))
          {
          VideoDebugPrint((0,"TVGETSTANDARD buffer size mismatch\n"));
		  status = ERROR_INVALID_PARAMETER;
          break;
          }
        status = BT868_GetStandard (pContext, (PTVGETSTANDARD)RequestPacket->OutputBuffer);
        break;
    case tvGetConnOverride:
        if (RequestPacket->OutputBufferLength < (RequestPacket->StatusBlock->Information = sizeof(TVGETOVERRIDE)))
          {
          VideoDebugPrint((0,"TVGETOVERRIDE buffer size mismatch\n"));
		  status = ERROR_INVALID_PARAMETER;
          break;
          }
        status = GetConnectionTypeFromNVRAM (pContext, (PTVGETOVERRIDE)RequestPacket->OutputBuffer);
        break;
    case tvGetSizeCaps:
        if (RequestPacket->OutputBufferLength < (RequestPacket->StatusBlock->Information = sizeof(TVSIZECAP)))
          {
          VideoDebugPrint((0,"TVSIZECAP buffer size mismatch\n"));
		  status = ERROR_INVALID_PARAMETER;
          break;
          }
        status = BT868_GetSizeCap (pContext, (PTVSIZECAP)RequestPacket->OutputBuffer);
        break;
    case tvGetPositionCaps:
        if (RequestPacket->OutputBufferLength < (RequestPacket->StatusBlock->Information = sizeof(TVPOSCAP)))
          {
          VideoDebugPrint((0,"TVPOSCAP buffer size mismatch\n"));
		  status = ERROR_INVALID_PARAMETER;
          break;
          }
        status = BT868_GetPosCap (pContext, (LPTVPOSCAP)RequestPacket->OutputBuffer);
        break;
#ifdef notused  //3dfx Tools uses QUERYGETPICCAP for flicker filter
    case tvGetFilterCaps:
        if (RequestPacket->OutputBufferLength < (RequestPacket->StatusBlock->Information = sizeof(TVCAPDATA)))
          {
          VideoDebugPrint((0,"TVCAPDATA buffer size mismatch\n"));
		  status = ERROR_INVALID_PARAMETER;
          break;
          }
        status = BT868_GetFilterCap (pContext, (LPTVCAPDATA)RequestPacket->OutputBuffer);
        break;
#endif  //def notused
    case tvGetPicCaps:
        if (RequestPacket->OutputBufferLength < (RequestPacket->StatusBlock->Information = sizeof(TVCAPDATA)))
          {
          VideoDebugPrint((0,"TVCAPDATA buffer size mismatch\n"));
		  status = ERROR_INVALID_PARAMETER;
          break;
          }
        status = BT868_GetPicCap (pContext, (LPTVCAPDATA)RequestPacket->OutputBuffer);
        break;
    case tvGetSizeCtrl:
        if (RequestPacket->OutputBufferLength < (RequestPacket->StatusBlock->Information = sizeof(TVCURSIZE)))
          {
          VideoDebugPrint((0,"TVSIZECTRL buffer size mismatch\n"));
		  status = ERROR_INVALID_PARAMETER;
          break;
          }
        status = \
        BT868_GetSizeControl (pContext, (PTVCURSIZE)RequestPacket->OutputBuffer);
        break;
    case tvMacrovisionOn:
        if (sizeof(int) >  RequestPacket->InputBufferLength)
          {
          VideoDebugPrint((0,"Macrovision On buffer size mismatch\n"));
		  status = ERROR_INVALID_PARAMETER;
          break;
          }
        status = \
        BT868_CopyProtect (pContext, *(int *)RequestPacket->InputBuffer);
        break;
    case tvMacrovisionOff:
        status = BT868_CopyProtect (pContext, 0);
        break;
    }


#ifdef MS_VIEW
// Not currently chasing down all called functions for returned values.
if ((status != 0) && (status != NO_ERROR)) // same thing, just for semantics
{
    status = ERROR_INVALID_PARAMETER;
}

#endif

return (status);

}


/*----------------------------------------------------------------------
Function name:  BT868_SetActiveState

Description:    Enable or Disable the TV device as selected by value of alternate
                display device mask passed in.

Information:

Return:         FxU32     STB_FUNCTION_TV if success,
                          0 if the current mode does not allow completion.
----------------------------------------------------------------------*/
FxU32 BT868_SetActiveState (BT868CONTEXT pContext, FxU32 activeStateMask)
{
#ifdef WINNT
    PH3_MEMBASE0 sstIoRegs = (PH3_MEMBASE0) pContext->MappedAddress[SST_IO_REGS_INDEX];
#else
	SstIORegs *sstIoRegs = (SstIORegs *)pContext->RegBase[HWINFO_SST_IOREGS_INDEX];
#endif
    ULONG xres = sstIoRegs->vidScreenSize & 0xfff;
    ULONG yres = (sstIoRegs->vidScreenSize >> 12) & 0xfff;
    ULONG result = STB_FUNCTION_TV;


    if (pContext->tvOutCapable)
    {
        // the card is tvOut Capable, check if TvOut is supposed to be on.
        if ((activeStateMask & STB_FUNCTION_TV) == STB_FUNCTION_TV)
        { // TvOut should be turned on
            // check if TvOut if off and needs to be turned on
            if (!pContext->tvOutActive)
            {
                // TvOut is off and needs to be turned on Check if the current mode is valid for TvOut
                switch (FX_RESOLUTION(xres,yres))
                {
                // a complete list of mode that are legal for TvOut
                case FX_RESOLUTION(640,480):
                case FX_RESOLUTION(720,480):
                    //???            case FX_RESOLUTION(720,576):
                case FX_RESOLUTION(800,600):
                    // the current mode is valid for TvOut turn it on.
                    if (! BT868_Enable (pContext, -1))
                        result = 0;
                    break;

                default:
                    // The current mode is not valid for TvOut.
                    // Turn on tvOutActive flag so that next SetMode command from "3dfx Tools"
                    // will force the TV on.  If "3dfx Tools" fails to do the set mode our state 
                    // will be left inconsistant.
                    pContext->tvOutActive = TRUE;
                    result = 0;
                }
            }

        }
        else
        {// TvOut should be turned off
            // check if TvOut is on and needs to be turned off
            if (pContext->tvOutActive)
                BT868_Disable (pContext);

        }
    }
    return(result);
}
#endif  //def WINNT

#undef FX_RESOLUTION
