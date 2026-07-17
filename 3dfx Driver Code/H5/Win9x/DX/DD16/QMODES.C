/* $Header: qmodes.c, 11, 10/11/00 8:51:33 PM, Brent$ */
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
** File name:   qmodes.c
**
** Description: QueryMode and supporting functions.
**
** $Revision: 11$ 
** $Date: 10/11/00 8:51:33 PM$
**
** $History: qmodes.c $
** 
** *****************  Version 36  *****************
** User: Rbissell     Date: 8/07/99    Time: 3:11p
** Updated in $/devel/h5/Win9x/dx/dd16
** tvout merge from V3_OEM_100
** 
** *****************  Version 34  *****************
** User: Stb_rbissell Date: 5/14/99    Time: 9:47p
** Updated in $/devel/h3/win95/dx/dd16
** tvout and dfp changes
** 
** *****************  Version 33  *****************
** User: Andrew       Date: 5/10/99    Time: 1:34p
** Updated in $/devel/h3/Win95/dx/dd16
** Changed PhysScreenAddr to RealregBase
** 
** *****************  Version 32  *****************
** User: Andrew       Date: 5/06/99    Time: 4:23p
** Updated in $/devel/h3/Win95/dx/dd16
** Modified to work with Trapping "C" Simulator
** 
** *****************  Version 31  *****************
** User: Stb_mimhoff  Date: 4/19/99    Time: 12:18p
** Updated in $/devel/h3/win95/dx/dd16
** When running TV Out and shutting down the monitor, it is generally
** preferred to shut down the VSYNC, not the HSYNC...Halting the VSYNC is
** the same action that we do in DPMS - Suspend.
** 
** *****************  Version 30  *****************
** User: Andrew       Date: 3/13/99    Time: 7:29p
** Updated in $/devel/h3/Win95/dx/dd16
** Added code to check DACMODE before checking VSYNC
** 
** *****************  Version 29  *****************
** User: Stuartb      Date: 2/25/99    Time: 12:48p
** Updated in $/devel/h3/Win95/dx/dd16
** Added table to reference individual board caps bt subsysID.  Added
** FPFLAG_CAPABLE.
** 
** *****************  Version 28  *****************
** User: Stuartb      Date: 2/23/99    Time: 3:06p
** Updated in $/devel/h3/Win95/dx/dd16
** A change to pass FPFLAG_FILTER_XX got lost.
** 
** *****************  Version 27  *****************
** User: Stuartb      Date: 2/18/99    Time: 2:54p
** Updated in $/devel/h3/Win95/dx/dd16
** Added xlcd FPFLAGS for filter mode setting.
** 
** *****************  Version 26  *****************
** User: Andrew       Date: 2/11/99    Time: 11:17a
** Updated in $/devel/h3/Win95/dx/dd16
** Added AGP cap Query
** 
** *****************  Version 25  *****************
** User: Stuartb      Date: 2/02/99    Time: 5:34p
** Updated in $/devel/h3/Win95/dx/dd16
** Minor change to LCD_CTRL.  Don't tweak hardware unless we enable or
** disable panel (eg. if only request for status).
** 
** *****************  Version 24  *****************
** User: Stuartb      Date: 2/01/99    Time: 2:18p
** Updated in $/devel/h3/Win95/dx/dd16
** Once again, when disabling analog crt do not set SST_DAC_FORCE_VSYNC
** lest we pend on VRETRACE which will never happen.
**  
** 
** *****************  Version 23  *****************
** User: Stuartb      Date: 1/29/99    Time: 9:50a
** Updated in $/devel/h3/Win95/dx/dd16
** When disabling analog CRT don't force syncs as this stops VRETRACE
** toggling.
** 
** *****************  Version 22  *****************
** User: Stuartb      Date: 1/28/99    Time: 4:34p
** Updated in $/devel/h3/Win95/dx/dd16
** Reinstate broadcastMonitor change on tv/lcd enable/disable.
** 
** *****************  Version 21  *****************
** User: Cwilcox      Date: 1/25/99    Time: 11:38a
** Updated in $/devel/h3/Win95/dx/dd16
** Minor modifications to remove compiler warnings.
** 
** *****************  Version 20  *****************
** User: Stuartb      Date: 1/21/99    Time: 11:18a
** Updated in $/devel/h3/Win95/dx/dd16
** Added FPFLAG_SETRES and FPFLAG_SETREFR for app code.  Removed
** broadcastMonitorChange calls.
** 
** *****************  Version 19  *****************
** User: Stuartb      Date: 1/14/99    Time: 3:46p
** Updated in $/devel/h3/Win95/dx/dd16
** Changed flat panel & tvout enable/disable philosophy.  No longer
** requires booting to that device to enable.
** 
** *****************  Version 18  *****************
** User: Stuartb      Date: 1/12/99    Time: 2:27p
** Updated in $/devel/h3/Win95/dx/dd16
** Added QUERY_ANALOG_MONITOR & modified QUERY_LCDCTRL.
** 
** *****************  Version 17  *****************
** User: Andrew       Date: 1/08/99    Time: 8:26p
** Updated in $/devel/h3/Win95/dx/dd16
** Fixed load of palette to be in Vertical Retrace
** 
** *****************  Version 16  *****************
** User: Stuartb      Date: 1/08/99    Time: 3:34p
** Updated in $/devel/h3/Win95/dx/dd16
** Added QUERY_LCDCTRL for control panel flat panel ops.
** 
** *****************  Version 15  *****************
** User: Bob          Date: 1/05/99    Time: 5:14p
** Updated in $/devel/h3/Win95/dx/dd16
** Altered nesting of header files so that tv.h is not inside qmodes.h.
** 
** This is a sharing issue with NT.
** 
** *****************  Version 14  *****************
** User: Andrew       Date: 1/05/99    Time: 10:52a
** Updated in $/devel/h3/Win95/dx/dd16
** Added new function to retrieve bios version string
** 
** *****************  Version 13  *****************
** User: Michael      Date: 12/29/98   Time: 2:36p
** Updated in $/devel/h3/Win95/dx/dd16
** Implement the 3Dfx/STB unified header.
** 
** 12    10/22/98 10:33a Stuartb
** Added TvOut extern function defs.  These were previously in tvout.h but
** DX6 now has a real tvout.h that we need to include.
** 
** 11    10/16/98 3:37p Michael
** In QUERYSETGAMMA, check default_gamma_flag before assign_gammaramp().
** 
** 10    9/16/98 5:35p Michael
** Fred's (igx) assign_gammaramp() implementation and a minor gamma fix.
** 
** 9     9/16/98 11:06a Stuartb
** Added additional tvout functionality.
** 
** 7     9/10/98 2:53p Stuartb
** Added TVOUT ExtEscapes.
** 
** 6     8/27/98 6:21p Andrew
** Added Gamma Correction
** 
** 5     8/21/98 7:34p Michael
** The GammaRamp implementation broke the 3Dfx properties page.  Fix made
** in QUERYSETGAMMA to initialize gamma_ramp structure.
** 
** 4     7/13/98 5:25p Andrew
** Changed to support a GammaTable
** 
** 3     7/11/98 8:15a Andrew
** Added Gamma Code
** 
** 2     5/12/98 9:34a Andrew
** Updates to add TV out and fix minor bugs
** 
** 1     4/22/98 2:48p Andrew
** Query Mode Function
** 
*/


/***************************************************************************
* I N C L U D E S
****************************************************************************/

#include <string.h>
#include "header.h"

#define Not_VxD
#include "minivdd.h"

#define Not_VxD
#include <vmm.h>

#define MIDL_PASS     // suppress 32-bit only #pragma pack(push)

#pragma warning (disable: 4047 4704)
#include <configmg.h>
#pragma warning (default: 4047 4704)

#include "h3g.h"
#include "modelist.h"
#include "tv.h"
#include "qmodes.h"
#include "gramp.h"
#include "dfpapi.h"

extern DISPLAYINFO DisplayInfo;
extern DWORD GammaTable[256];
extern DWORD dwDeviceHandle;

extern void TVOutStatus( LPQIN lpQIN, LPQTVSTATUS lpOutput );
extern void TVOutConStatus( LPQIN lpQIN, LPTVCONSTATUS lpOutput );
extern void TVOutGetPicCap( LPQIND lpQIN, LPTVCAPDATA lpOutput );
extern void TVOutGetFilterCap( LPQIND lpQIN, LPTVCURCAP lpOutput );
extern void TVOutGetPosCap( LPQIN lpQIN, LPTVPOSCAP lpOutput );
extern void TVOutGetSizeCap( LPQIN lpQIN, LPTVSIZECAP lpOutput );
extern void TVOutGetSpecialCap( LPQIND lpQIN, LPVOID lpOutput );
extern void TVOutGetStandard( LPQIN lpQIN, LPTVGETSTANDARD lpOutput );
extern void TVOutGetPicControl( LPQIND lpQIN, LPTVCURCAP lpOutput );
extern void TVOutGetFilterControl( LPQIND lpQIN, LPTVCURCAP lpOutput );
extern void TVOutGetPosControl( LPQIN lpQIN, LPTVCURPOS lpOutput );
extern void TVOutGetSizeControl( LPQIN lpQIN, LPTVCURSIZE lpOutput );
extern void TVOutGetSpecialCtl( LPQIND lpQIN, LPVOID lpOutput );
extern void TVOutGetConStatus( LPQIN lpQIN, LPTVCONSTATUS lpOutput );
extern void TVOutSetStandard( LPTVSETSTANDARD lpQIN );
extern void TVOutSetPicControl( LPTVSETCAP lpQIN );
extern void TVOutSetFilterControl( LPTVSETCAP lpQIN );
extern void TVOutSetPosControl( LPTVSETPOS lpQIN );
extern void TVOutSetSizeControl( LPTVSETSIZE lpQIN );
extern void TVOutSetSpecial( LPTVSETSPECIAL lpQIN );
extern void TVOutSetConStatus( LPTVSETCONNECTOR lpQIN );
extern void TVOutCommitReg( LPQIN lpQIN );
extern void TVOutRefreshMem( LPQIN lpQIN );
extern void TVOutDisable( );
extern void TVOutEnable( LPQIN lpQIN );


typedef struct   // this needs to move, but not today
{
	FxU8 busTypeIsAGP;    // 'A' == AGP, \000 == PCI
	FxU8 ramTypeIsSGRAM;  // 'G' == SGRAM, \000 == SDRAM
	FxU16 speed;          // in motorcycles per second
	FxU8 hasTVOUT;        // BOOL
	FxU8 hasLCD;          // BOOL
	FxU8 biosSize;        // in KB
}  V3_BOARD_DESCRIPTION;


const V3_BOARD_DESCRIPTION V3_Board_Description[] = {
// ASSY DWG     BUS   RAM  SPD  TV?  LCD? BIOS-KB    BIOS filenm  SubSystemID
{0},                                            //                0x0000
{0},                                            //                0x0001
{0},                                            //                0x0002
/* REF1     */ {'A',  000, 183, 000, 000,  32}, // K V33K5R1.ROM  0x0003
/* REF2     */ {'A',  'G', 143, 000, 000,  32}, // K V32KR2.ROM   0x0004
/* REF2     */ {'A',  'G', 166, 000, 000,  32}, // K V33KR2.ROM   0x0005
/* REF2     */ {'A',  'G', 183, 000, 000,  32}, // K V33K5R2.ROM  0x0006
/* REF3     */ {000,  000, 143, 000, 000,  32}, // K V3P769.ROM   0x0007
/* REF3     */ {000,  000, 166, 000, 000,  32}, // K V33KR3.ROM   0x0008
/* REF3     */ {000,  000, 183, 000, 000,  32}, // K V33K5R3.ROM  0x0009
/* REF4     */ {000,  'G', 143, 000, 000,  32}, // K V32KR4.ROM   0x000A
/* REF4     */ {000,  'G', 166, 000, 000,  32}, // K V33KR4.ROM   0x000B
/* REF4     */ {000,  'G', 183, 000, 000,  32}, // K V33K5R4.ROM  0x000C
/* REF1     */ {'A',  000, 143, 'Y', 'Y',  64}, // K V32KR1E.ROM  0x000D
/* REF1     */ {'A',  000, 166, 'Y', 'Y',  64}, // K V33KR1E.ROM  0x000E
/* REF1     */ {'A',  000, 183, 'Y', 'Y',  64}, // K V33K5R1E.ROM 0x000F
/* REF2     */ {'A',  'G', 143, 'Y', 'Y',  64}, // K V32KR2E.ROM  0x0010
/* REF2     */ {'A',  'G', 166, 'Y', 'Y',  64}, // K V33KR2E.ROM  0x0011
/* REF2     */ {'A',  'G', 183, 'Y', 'Y',  64}, // K V33K5R2E.ROM 0x0012
/* REF3     */ {000,  000, 143, 'Y', 'Y',  64}, // K V32KR3E.ROM  0x0013
/* REF3     */ {000,  000, 166, 'Y', 'Y',  64}, // K V33KR3E.ROM  0x0014
/* REF3     */ {000,  000, 183, 'Y', 'Y',  64}, // K V33K5R3E.ROM 0x0015
/* REF4     */ {000,  'G', 143, 'Y', 'Y',  64}, // K V32KR4E.ROM  0x0016
/* REF4     */ {000,  'G', 166, 'Y', 'Y',  64}, // K V33KR4E.ROM  0x0017
/* REF4     */ {000,  'G', 183, 'Y', 'Y',  64}, // K V33K5R4E.ROM 0x0018
{0},                                            //                0x0019
{0},                                            //                0x001A
{0},                                            //                0x001B
{0},                                            //                0x001C
{0},                                            //                0x001D
{0},                                            //                0x001E
{0},                                            //                0x001F
{0},                                            //                0x0020
{0},                                            //                0x0021
{0},                                            //                0x0022
{0},                                            //                0x0023
{0},                                            //                0x0024
{0},                                            //                0x0025
{0},                                            //                0x0026
{0},                                            //                0x0027
{0},                                            //                0x0028
{0},                                            //                0x0029
{0},                                            //                0x002A
{0},                                            //                0x002B
{0},                                            //                0x002C
{0},                                            //                0x002D
{0},                                            //                0x002E
{0},                                            //                0x002F
/* 773  364 */ {'A',  000, 143, 000, 000,  32}, // K V3P773.ROM   0x0030
/* 774  364 */ {'A',  000, 143, 000, 000,  32}, // K V3P774.ROM   0x0031
/* 765  365 */ {'A',  000, 143, 000, 'Y',  64}, // K V3P765.ROM   0x0032
/* 778  369 */ {'A',  000, 143, 000, 'Y',  64}, // K V3P778.ROM   0x0033
/* 772  364 */ {'A',  000, 143, 'Y', 000,  64}, // K V3P772.ROM   0x0034
/* 777  369 */ {'A',  000, 143, 'Y', 'Y',  64}, // K V3P777.ROM   0x0035
/* 769  366 */ {000,  000, 143, 000, 000,  32}, // K V3P769.ROM   0x0036
/* 762  364 */ {'A',  000, 166, 000, 000,  32}, // K V3P762.ROM   0x0037
/* 763  364 */ {'A',  000, 166, 000, 000,  32}, // K V3P763.ROM   0x0038
/* 776  369 */ {'A',  000, 166, 000, 'Y',  64}, // K V3P776.ROM   0x0039
/* 761  364 */ {'A',  000, 166, 'Y', 000,  64}, // K V3P761.ROM   0x003A
/* 775  369 */ {'A',  000, 166, 'Y', 'Y',  64}, // K V3P775.ROM   0x003B
/* 771  368 */ {'A',  'G', 183, 'Y', 'Y',  64}, // K V3P771.ROM   0x003C
/* 766  366 */ {000,  000, 143, 'Y', 'Y',  64}, // K V3P766.ROM   0x003D
/* 753  361 */ {'A',  000, 143, 000, 'Y',  64}, // K V3P753.ROM   0x003E
/* 755  368 */ {'A',  'G', 166, 'Y', 'Y',  64}, // K V3P755.ROM   0x003F
/* 757  361 */ {'A',  000, 143, 000, 000,  32}, // K V3P757.ROM   0x0040
/* 758  368 */ {'A',  'G', 166, 'Y', 000,  64}, // K V3P758.ROM   0x0041
/* 759  368 */ {'A',  'G', 166, 000, 'Y',  64}, // K V3P759.ROM   0x0042
/* 760  368 */ {'A',  'G', 166, 000, 000,  32}, // K V3P760.ROM   0x0043
/* 767  366 */ {000,  000, 143, 'Y', 000,  64}, // K V3P767.ROM   0x0044
/* 768  366 */ {000,  000, 143, 000, 'Y',  64}, // K V3P768.ROM   0x0045
/* 779  368 */ {'A',  'G', 183, 000, 'Y',  64}, // K V3P779.ROM   0x0046
/* 780  368 */ {'A',  000, 183, 'Y', 000,  64}, // K V3P780.ROM   0x0047
/* 781  368 */ {'A',  'G', 183, 000, 000,  32}, // K V3P781.ROM   0x0048
/* 797  369 */ {'A',  000, 183, 'Y', 'Y',  64}, // K V3P797.ROM   0x0049
/* 798  369 */ {'A',  000, 183, 000, 'Y',  64}, // K V3P797.ROM   0x004A
/* 806  380 */ {'A',  'G', 143, 000, 000,  64}, // K V3P806.ROM   0x004B
{0},                                            //                0x004C
/* 804  380 */ {'A',  'G', 143, 000, 000,  32}, // K V3P804.ROM   0x004D
{0},                                            //                0x004E
/* 808  369 */ {'A',  000, 183, 'Y', 000,  64}, // K V3P808.ROM   0x004F
{0},                                            //                0x0050
/* 819  383 */ {'A',  'G', 143, 000, 000,  32}, // K V3P819.ROM   0x0051
/* 820  380 */ {'A',  'G', 125, 000, 000,  32}, // K V3P820.ROM   0x0052
/* 826  382 */ {000,  'G', 143, 000, 000,  64}, // K V3P826.ROM   0x0053
/* 827  382 */ {000,  'G', 125, 000, 000,  64}, // K V3P827.ROM   0x0054
/* 828  382 */ {000,  'G', 143, 000, 000,  64}, // K V3P828.ROM   0x0055
/* 852  382 */ {000,  000, 143, 000, 000,  64}, // K V3P852.ROM   0x0056
/* 855  395 */ {000,  000, 166, 000, 000,  64}, // K V3P855.ROM   0x0057
{0},                                            //                0x0058
{0},                                            //                0x0059
{0},                                            //                0x005A
{0},                                            //                0x005B
{0},                                            //                0x005C
{0},                                            //                0x005D
{0},                                            //                0x005E
{0},                                            //                0x005F
/* 784  371 */ {'A',  000, 183, 'Y', 000,  64}, // K V3P808.ROM   0x0060
/* 794  371 */ {'A',  000, 183, 'Y', 000,  64}, // K V3P808.ROM   0x0061
/* 795  371 */ {'A',  000, 183, 'Y', 000,  64}, // K V3P808.ROM   0x0062
{0},                                            //                0x0063
{0},                                            //                0x0064
{0},                                            //                0x0065
{0},                                            //                0x0066
{0},                                            //                0x0067
{0},                                            //                0x0068
{0},                                            //                0x0069
{0},                                            //                0x006A
{0},                                            //                0x006B
{0},                                            //                0x006C
{0},                                            //                0x006D
{0},                                            //                0x006E
{0},                                            //                0x006F
/* ???  ??? */ {'A',  'G', 143, 'Y', 000,  32}, // ? ??????.ROM   0x0070 some Gateway thing
/* ???  ??? */ {'A',  000, 166, 'Y', 000,  64}, // ? ??????.ROM   0x0071
/* ???  ??? */ {'A',  'G', 143, 000, 000,  32}, // ? ??????.ROM   0x0072
/* ???  ??? */ {'A',  'G', 166, 000, 000,  32}, // ? ??????.ROM   0x0073
/* ???  ??? */ {'A',  'G', 143, 000, 000,  32}, // ? ??????.ROM   0x0074
/* ???  ??? */ {'A',  'G', 166, 000, 000,  32}, // ? ??????.ROM   0x0075
/* ???  ??? */ {'A',  000, 166, 000, 000,  32}, // ? ??????.ROM   0x0076
/* ???  ??? */ {'A',  000, 143, 000, 000,  32}, // ? ??????.ROM   0x0077
/* ???  ??? */ {'A',  000, 143, 000, 000,  32}  // ? ??????.ROM   0x0078
};




extern int di_ValidateModeList(void);	// DYNAMIC MODE TABLE
int setupPalette();
void TvoutAllowCRTWithTV(FxU32 State);

// When disabling a monitor, it is generally preferred to shut down the VSYNC,
// not the HSYNC... Halting the VSYNC is the same action that we do in DPMS - Suspend.
#define MONITOR_DPMS_MASK ( SST_DAC_DPMS_ON_VSYNC )

/*----------------------------------------------------------------------
Function name:  QueryMode

Description:    This Control Subfunction is used to handle all of
                the queries defined in qmodes.h.
Information:

Return:         INT     TDFXACK for Success or TDFXERR for Failure.
----------------------------------------------------------------------*/
int QueryMode(LPQIN lpQIN, LPVOID lpOutput)
{
   LPQGETAGPCAPS lpGetAGPCaps;
   LPQVERSION lpQVersion;
   LPQNUMMODE lpQNumMode;
   LPQMODE lpQMode;
#ifdef REAL_NET
   LPQMODE_EXT lpQModeExt;
   LPQMODE_CURRENT lpQModeCurrent;
#endif
   LPQDEVNODE lpQDevNode;
   LPQGETGAMMA lpQGetGamma;
   LPQSETGAMMA lpQSetGamma;
   SstIoRegRW sstIO;
   DWORD dwStatus;
   int i;
   int nReturn = TDFXERR;
   GLOBALDATA *glbData = (void *)&_FF(lpPDevice);

   switch (lpQIN->dwSubFunc)
      {
      case QUERYVERSION:
         lpQVersion = (LPQVERSION)lpOutput;
         lpQVersion->dwMajor = QUERYMODE_MAJOR;
         lpQVersion->dwMinor = QUERYMODE_MINOR;
         nReturn = TDFXACK;
         break;

      case QUERYNUMMODES:
         lpQNumMode = (LPQNUMMODE)lpOutput;
         for (i=0; 0 != ModeList[i].dwWidth; i++)
            ;
         lpQNumMode->dwNum = i;
         nReturn = TDFXACK;
         break;

      case QUERYMODES:
         lpQMode = (LPQMODE)lpOutput;

		   di_ValidateModeList();  	// Re-validate the Mode List array - DYNAMIC MODE TABLE 

         for (i=0; 0 != ModeList[i].dwWidth; i++)
            {
            lpQMode->dwX = ModeList[i].dwWidth;
            lpQMode->dwY = ModeList[i].dwHeight;
            lpQMode->dwBpp = ModeList[i].dwBPP;
            if (IS_VALID_MODE == (ModeList[i].dwFlags & IS_VALID_MODE))
               lpQMode->dwValid = QUERY_MODE_VALID;
            else
               lpQMode->dwValid = 0x0L;
            lpQMode->dwRef = (DWORD)ModeList[i].wVert;
            lpQMode++;
            }
         nReturn = TDFXACK;
         break;

#ifdef REAL_NET
      case QUERYMODESEXT:
         lpQModeExt = (LPQMODE_EXT)lpOutput;

		   di_ValidateModeList();  	// Re-validate the Mode List array - DYNAMIC MODE TABLE 

         for (i=0; 0 != ModeList[i].dwWidth; i++)
            {
            lpQModeExt->dwX = ModeList[i].dwWidth;
            lpQModeExt->dwY = ModeList[i].dwHeight;
            lpQModeExt->dwBpp = ModeList[i].dwBPP;
            if (IS_VALID_MODE == (ModeList[i].dwFlags & IS_VALID_MODE))
               lpQModeExt->dwValid = QUERY_MODE_VALID;
            else
               lpQModeExt->dwValid = 0x0L;
            lpQModeExt->dwRef = (DWORD)ModeList[i].wVert;
            lpQModeExt->dwRef100 = (DWORD)ModeList[i].wVert100;
            lpQModeExt++;
            }
         nReturn = TDFXACK;
         break;

      case QUERYCURRENTMODE:
         lpQModeCurrent = (LPQMODE_CURRENT)lpOutput;

         lpQModeCurrent->dwX = ModeList[_FF(ModeNumber)].dwWidth;
         lpQModeCurrent->dwY = ModeList[_FF(ModeNumber)].dwHeight;
         lpQModeCurrent->dwBpp = ModeList[_FF(ModeNumber)].dwBPP;
         lpQModeCurrent->dwRef = (DWORD)ModeList[_FF(ModeNumber)].wVert100;

         nReturn = TDFXACK;
         break;
#endif

      case QUERYDEVNODE:
         lpQDevNode = (LPQDEVNODE)lpOutput;
         lpQDevNode->dwDevNode = DisplayInfo.diDevNodeHandle;
         nReturn = TDFXACK;
         break;

      case QUERYGETGAMMA:
         lpQGetGamma = (LPQGETGAMMA)lpOutput;
         lpQGetGamma->dwRed = _FF(dwRed);
         lpQGetGamma->dwGreen = _FF(dwGreen);
         lpQGetGamma->dwBlue = _FF(dwBlue);
         for (i=0; i<256; i++)
            lpQGetGamma->GammaTable[i] = GammaTable[i];
         nReturn = TDFXACK;
         break;

      case QUERYSETGAMMA:
         lpQSetGamma = (LPQSETGAMMA)lpQIN;
         _FF(dwRed) = lpQSetGamma->dwRed;
         _FF(dwGreen) = lpQSetGamma->dwGreen;
         _FF(dwBlue) = lpQSetGamma->dwBlue;
         for (i=0; i<256; i++)
            {
            GammaTable[i] = lpQSetGamma->GammaTable[i];
            assign_gammaramp(i,
                 (int)(GammaTable[i] & 0xff),          /* red */
                 ((int)(GammaTable[i]>>8) & 0xff),      /* green */
                 ((int)(GammaTable[i]>>16) & 0xff),     /* blue */
                 0);                                    /*for desktop */
            }
         setupPalette();
         nReturn = TDFXACK;
         break;

      case QUERYSETOVERLAYGAMMA:
         lpQSetGamma = (LPQSETGAMMA)lpQIN;
         for (i=0; i<256; i++)
            {
            assign_gammaramp(i,
                 (int)(lpQSetGamma->GammaTable[i] & 0xff),          /* red */
                 ((int)(lpQSetGamma->GammaTable[i]>>8) & 0xff),      /* green */
                 ((int)(lpQSetGamma->GammaTable[i]>>16) & 0xff),     /* blue */
                 1);                                                /* overlay */
            } 
         set_gammaramp(1);
         nReturn = TDFXACK;
         break;

      case QUERYGETAGPCAPS:
         lpGetAGPCaps = (LPQGETAGPCAPS)lpOutput;
         lpGetAGPCaps->dwAGPCaps = _FF(AGPCaps);            
         nReturn = TDFXACK;
         break;

      case QUERYGETBIOSVERSION:
         dwStatus = VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle, H3VDD_GET_BIOS_VERSION, 0, lpOutput);
         if (0 == dwStatus)
            nReturn = TDFXACK;
         else
            nReturn = TDFXERR;
         break;

      case QUERYGETOEMBOARDNAME:
         dwStatus = VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle, H3VDD_GET_OEM_BOARD_NAME, 0, lpOutput);
         if (0 == dwStatus)
            nReturn = TDFXACK;
         else
            nReturn = TDFXERR;
         break;

      case QUERYGETCHIPNAME:
         dwStatus = VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle, H3VDD_GET_CHIP_NAME, 0, lpOutput);
         if (0 == dwStatus)
            nReturn = TDFXACK;
         else
            nReturn = TDFXERR;
         break;

      //TV-Out functions----------------------------------------------------------//
      
      case QUERYTVAVAIL:
         TVOutStatus( lpQIN, (LPQTVSTATUS)lpOutput );
         nReturn = TDFXACK;
         break;
      case QUERYTVSENSE:
         TVOutConStatus( lpQIN, (LPTVCONSTATUS)lpOutput );
         nReturn = TDFXACK;
         break;
      case QUERYGETPICCAP:
         TVOutGetPicCap( (LPQIND)lpQIN, (LPTVCAPDATA)lpOutput );
         nReturn = TDFXACK;
         break;
      case QUERYGETFILTERCAP:
         TVOutGetFilterCap( (LPQIND)lpQIN, (LPTVCURCAP)lpOutput );
         nReturn = TDFXACK;
         break;
      case QUERYGETPOSCAP:
         TVOutGetPosCap( lpQIN, (LPTVPOSCAP)lpOutput );
         nReturn = TDFXACK;
         break;
      case QUERYGETSIZECAP:
         TVOutGetSizeCap( lpQIN, (LPTVSIZECAP)lpOutput );
         nReturn = TDFXACK;
         break;
      case QUERYGETSPECIALCAP:
         TVOutGetSpecialCap( (LPQIND)lpQIN, (LPVOID)lpOutput );
         nReturn = TDFXACK;
         break;
      case QUERYGETSTANDARD:
      case QUERYGETOVERRIDE:
         TVOutGetStandard( lpQIN, (LPTVGETSTANDARD)lpOutput );
         nReturn = TDFXACK;
         break;
      case QUERYGETPICCONTROL:
         TVOutGetPicControl( (LPQIND)lpQIN, (LPTVCURCAP)lpOutput );
         nReturn = TDFXACK;
         break;
      case QUERYGETFILTERCONTROL:
         TVOutGetFilterControl( (LPQIND)lpQIN, (LPTVCURCAP)lpOutput );
         nReturn = TDFXACK;
         break;
      case QUERYGETPOSCONTROL:
         TVOutGetPosControl( lpQIN, (LPTVCURPOS)lpOutput );
         nReturn = TDFXACK;
         break;
      case QUERYGETSIZECONTROL:
         TVOutGetSizeControl( lpQIN, (LPTVCURSIZE)lpOutput );
         nReturn = TDFXACK;
         break;
      case QUERYGETSPECIAL:
         TVOutGetSpecialCtl( (LPQIND)lpQIN, (LPVOID)lpOutput );
         nReturn = TDFXACK;
         break;
      case QUERYGETCONSTATUS:
         TVOutGetConStatus( lpQIN, (LPTVCONSTATUS)lpOutput );
         nReturn = TDFXACK;
         break;
      case QUERYSETOVERRIDE:
      case QUERYSETSTANDARD:
         TVOutSetStandard( (LPTVSETSTANDARD)lpQIN );
         nReturn = TDFXACK;
         break;
      case QUERYSETPICCONTROL:
         TVOutSetPicControl( (LPTVSETCAP)lpQIN );
         nReturn = TDFXACK;
         break;
      case QUERYSETFILTERCONTROL:
         TVOutSetFilterControl( (LPTVSETCAP)lpQIN );
         nReturn = TDFXACK;
         break;
      case QUERYSETPOSCONTROL:
         TVOutSetPosControl( (LPTVSETPOS)lpQIN );
         nReturn = TDFXACK;
         break;
      case QUERYSETSIZECONTROL:
         TVOutSetSizeControl( (LPTVSETSIZE)lpQIN );
         nReturn = TDFXACK;
         break;
      case QUERYSSETSPECIAL:
         TVOutSetSpecial( (LPTVSETSPECIAL)lpQIN );
         nReturn = TDFXACK;
         break;
      case QUERYSETCONSTATUS:
         TVOutSetConStatus( (LPTVSETCONNECTOR)lpQIN );
         nReturn = TDFXACK;
         break;
      case QUERYCOMMITREG:
         TVOutCommitReg( lpQIN );
         nReturn = TDFXACK;
         break;
      case QUERYREFRESH:
         TVOutRefreshMem( lpQIN );
         nReturn = TDFXACK;
         break;
      case QUERYDISABLETV:
         TVOutDisable( );
         nReturn = TDFXACK;
         break;
      case QUERYENABLETV:
         TVOutEnable( lpQIN );
         nReturn = TDFXACK;
         break;

      //End TV-Out section----------------------------------------------------------//

	  case QUERY_LCDCTRL:
#ifdef DEBUG
         _asm int 3;  // this interface is obsolete and should not be used.
#endif
         nReturn = TDFXACK;
         break;

	 case QUERY_ANALOG_MONITOR:
		 sstIO.regRWflags = 0;
		 sstIO.regAddr = (DWORD)&((SstIORegs FAR *)0L)->dacMode;
		 VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle,
										H3VDD_RW_REGISTER, 0, &sstIO);
		 if (!(sstIO.regValue & MONITOR_DPMS_MASK))
			 ((QGETSET_MONITOR_CTL *)lpQIN)->monitorStatus = MONITOR_IS_ENABLED;
		 else
			 ((QGETSET_MONITOR_CTL *)lpQIN)->monitorStatus = 0;


       if (_FF(dwTvoActive) && (_FF(dwTvoStd) != VP_TV_STANDARD_NTSC_M) && !(_FF(allowPALCRT)))
       {
          ((QGETSET_MONITOR_CTL *)lpQIN)->monitorStatus = 0;                 //say the monitor is off
          ((QGETSET_MONITOR_CTL *)lpQIN)->monitorControl = DISABLE_MONITOR;  //turn the monitor off
       }

		 sstIO.regRWflags = RWFLAG_WRITE_REG;
		 if (((QGETSET_MONITOR_CTL *)lpQIN)->monitorControl & ENABLE_MONITOR)
       {
			 sstIO.regValue &= ~MONITOR_DPMS_MASK;
          TvoutAllowCRTWithTV(1UL);
       }
		 else if (((QGETSET_MONITOR_CTL *)lpQIN)->monitorControl & DISABLE_MONITOR)
       {
			 sstIO.regValue |= MONITOR_DPMS_MASK;
          TvoutAllowCRTWithTV(0UL);
       }
		 else
			 break;

		 VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle,
										H3VDD_RW_REGISTER, 0, &sstIO);
		 break;

   default:
         nReturn = TDFXERR;
         break;
      }

   return nReturn;
} 

#define RED_NAME  "SSTH3_RGAMMA"
#define BLUE_NAME  "SSTH3_BGAMMA"
#define GREEN_NAME  "SSTH3_GGAMMA"

#define FRACMULT (10000L)
#define RANGE(val, low, hi) ((low <= val) && (val <= hi))
#define EPILSON (10)
#define SUB(a,b) (((a) > (b)) ? (a)-(b) : (b)-(a))

DWORD gamma(WORD wValue, DWORD gamma1, DWORD g255);
DWORD ipow(DWORD a, DWORD x);
DWORD logex(DWORD x);
char *ddgetenv( const char *varname );
DWORD atof100x( const char *s );
DWORD idiv(DWORD x, DWORD y, DWORD fracmult);
DWORD imult(DWORD x, DWORD y, DWORD fracmult);


/*----------------------------------------------------------------------
Function name:  LoadGamma

Description:    Program the DAC.

Information:

Return:         VOID
----------------------------------------------------------------------*/
void LoadGamma(void)
{
#ifdef WIN_CSIM
   SstIORegs FAR *lph3IORegs = (SstIORegs *)_FF(regRealBase);
#endif
   DWORD rgamma;
   DWORD ggamma;
   DWORD bgamma;
   DWORD rgamma1;
   DWORD rg255;
   DWORD ggamma1;
   DWORD gg255;
   DWORD bgamma1;
   DWORD bg255;
   DWORD color;
   DWORD garbage;
   int i;
   LPSTR lpStr;

   lpStr = ddgetenv(RED_NAME);
   if (NULL != lpStr)
      rgamma = atof100x(lpStr);
   else
      rgamma = 100;

   lpStr = ddgetenv(GREEN_NAME);
   if (NULL != lpStr)
      ggamma = atof100x(lpStr);
   else
      ggamma = 100;

   lpStr = ddgetenv(BLUE_NAME);
   if (NULL != lpStr)
      bgamma = atof100x(lpStr);
   else
      bgamma = 100;

   // Clamp these babies
   if (rgamma < 43)
      rgamma = 43;
   else if (rgamma > 400)
      rgamma = 400;
    
   if (ggamma < 43)
      ggamma = 43;
   else if (ggamma > 400)
      ggamma = 400;
    
   if (bgamma < 43)
      bgamma = 43;
   else if (bgamma > 400)
      bgamma = 400;

   rgamma1 = idiv(FRACMULT, rgamma * FRACMULT/100, FRACMULT);
   rg255 = ipow((DWORD)255 * FRACMULT, rgamma1);
   ggamma1 = idiv(FRACMULT, ggamma * FRACMULT/100, FRACMULT);
   gg255 = ipow((DWORD)255 * FRACMULT, ggamma1);
   bgamma1 = idiv(FRACMULT, bgamma * FRACMULT/100, FRACMULT);
   bg255 = ipow((DWORD)255 * FRACMULT, bgamma1);

   // wait for vblank b4 update dac entries to prevent sparkle
   //
   // Make sure Hsync and Vsync are toggling before we check them
   if  (!(GET(lph3IORegs->dacMode) & (SST_DAC_DPMS_ON_VSYNC|SST_DAC_FORCE_VSYNC|SST_DAC_DPMS_ON_HSYNC|SST_DAC_FORCE_HSYNC)))
      {
       while (!((GET(lph3IORegs->status) & SST_VRETRACE) ^ (_FF(ddMiscFlags) & DDMF_VSYNC_POLARITY_MASK)))
	      ;
      }

   for (i=0; i<256; i++)
      {
      // try to reduce math since gamma is time consuming
      color = gamma(i, bgamma1, bg255);
      if (bgamma1 == ggamma1)
         color =  color | (color & 0xFF) << 8;
      else
         color =  color | gamma(i, ggamma1, gg255) << 8;

      if (bgamma1 == rgamma1)
         color =  color | (color & 0xFF) << 16;
      else
         color = color | gamma(i, rgamma1, rg255) << 16;

   	SETDW(lph3IORegs->dacAddr, i);
	   garbage = GET(lph3IORegs->dacAddr);
   	SETDW(lph3IORegs->dacData, color);
	   garbage = GET(lph3IORegs->dacData);
      }
}


/*----------------------------------------------------------------------
Function name:  gamma

Description:    This function computes (wValue/255)^(1/dwGamma)
                * 255 + .5 which is what we use to calculate
                gamma correction.

Information:

Return:         DWORD   The calculation noted above.
----------------------------------------------------------------------*/
DWORD gamma(WORD wValue, DWORD gamma1, DWORD g255)
{
   DWORD dwReturn;

   dwReturn = ((imult(idiv(ipow((DWORD)wValue * FRACMULT, gamma1), g255, FRACMULT), 255 * FRACMULT, FRACMULT) + 5 * FRACMULT/10)/FRACMULT);

   return dwReturn;
}


/*----------------------------------------------------------------------
Function name:  ipow

Description:    This function computes a^x by using the series
                exponential a^x = 1 + x * ln a + (x * ln a)^2/2!
                + (x * ln a) ^ 3/3! + ...

Information:    This is calculated to 4 decimal places

Return:         DWORD   The result of the calculation.
----------------------------------------------------------------------*/
DWORD ipow(DWORD a, DWORD x)
{
   DWORD dRet;
   DWORD dOldRet;
   DWORD xlogea;
   DWORD xnew;
   DWORD xfac;
   int n;
   int nCount=1000;

   if (a < EPILSON)
      return 0;
   xlogea = imult(x, logex(a), FRACMULT);
   n = 1;
   xfac = 1*FRACMULT;
   dRet = 1 * FRACMULT;
   dOldRet = 2;
   while ((SUB(dRet,dOldRet) > EPILSON) && (nCount-- > 0))
      {
      dOldRet = dRet;
      xnew = xlogea/n;
      xfac = imult(xfac, xnew, FRACMULT);
      n += 1;
      dRet = dRet + xfac;
      }

   return dRet;
}


/*----------------------------------------------------------------------
Function name:  logex

Description:    This functions computes ln x.  We use the formula:
                ln (1+x)/(1-x) = 2(x + x^3/3 + x^5/5 + x^7/7 + ...)
                where we set y=(1+x)/(1-x) and solve for x giving
                x=(y-1)/y+1)

Information:    This is calculated to 6 decimal places

Return:         DWORD   The result of the calculation.
----------------------------------------------------------------------*/
DWORD logex(DWORD x)
{
   DWORD xfac;
   DWORD xPart;
   DWORD xsqrd;
   DWORD xnum;
   DWORD dRet;
   DWORD dOldRet;
   int nCount = 1000;

   x = x * 100;
   xfac = idiv((x - 1 * FRACMULT*100L), (x + 1 * FRACMULT*100L), FRACMULT*100L);
   xsqrd = imult(xfac, xfac, FRACMULT*100L);

   dRet = xfac;
   dOldRet = 2*EPILSON;
   xPart = xfac;
   xnum = 3 * FRACMULT*100L;
   while ((SUB(dRet, dOldRet) > EPILSON) && (nCount-- > 0))
      {
      dOldRet = dRet;
      xPart = imult(xPart, xsqrd, FRACMULT*100L);
      dRet = dRet + idiv(xPart,xnum, FRACMULT*100L);
      xnum += (2 * FRACMULT*100L); 
      }

   dRet = dRet/100;
   dRet = dRet * 2;

   return dRet;
}


/*----------------------------------------------------------------------
Function name:  ddgetenv

Description:    Extract environment information from WIN.INI or
                the registry.
Information:

Return:         char *  to valid data or,
                        NULL if failure.
----------------------------------------------------------------------*/
#define  PRSIZE  50      // GetProfileString return buffer
char    rstr[ PRSIZE ];
char    DevNodeKey[MAX_VMM_REG_KEY_LEN];
char *ddgetenv( const char *varname )
{
  char    nstr[]="\0";
  char    *lpnull;
  char    *lprstr;
  DWORD   DevNode;


  lprstr = rstr;
  lpnull = nstr;

  // First check in [3dfx] section of WIN.INI
  if( GetProfileString( "3Dfx", varname, lpnull, lprstr, PRSIZE ) )
  {
    return( lprstr );
  }

  DevNode = _FF(DevNode);

  // convert devnode to a registry key
  // when successful, this thing returns a string something like
  // "System\CurrentControlSet\Services\Class\DISPLAY\XXXX"
  //
  if (CR_SUCCESS == CM_Get_DevNode_Key(DevNode,
                                        NULL,
                                        (PFARVOID)DevNodeKey,
                                        sizeof(DevNodeKey),
                                        CM_REGISTRY_SOFTWARE))
  {
    typedef struct _H3_SEARCH_TYPE
    {
      HKEY  hkey;
      char  *pszSubkey;
    } H3_SEARCH_TYPE;

    static const H3_SEARCH_TYPE H3SearchOrder[] =
    {
      { HKEY_CURRENT_USER,  "\\Glide" },
      { HKEY_CURRENT_USER,  NULL    },
      { HKEY_LOCAL_MACHINE, "\\Glide" },
      { HKEY_LOCAL_MACHINE, NULL    },
    };
    const H3_SEARCH_TYPE *pSearchLoc;
    HKEY  hkey;
    DWORD type;
    DWORD length;
    ULONG DevNodeKeyLength;


    // save original length of DevNodeKey
    DevNodeKeyLength = strlen(DevNodeKey);

    // loop over possible registry locations
    for (pSearchLoc = &H3SearchOrder[0];
         pSearchLoc < &H3SearchOrder[sizeof(H3SearchOrder)/sizeof(H3SearchOrder[0])];
         pSearchLoc++)
    {
      // if we have a non NULL subkey
      // tack the subkey to the end of the DevNodeKey
      if (NULL != pSearchLoc->pszSubkey)
        strcat(DevNodeKey, pSearchLoc->pszSubkey);

      // attempt to open the key
      if (ERROR_SUCCESS == RegOpenKey(pSearchLoc->hkey,
                                        DevNodeKey,
                                        &hkey))
      {
        // the key exists so attempt to read the value of varname
        length = sizeof(rstr);
        if (ERROR_SUCCESS == RegQueryValueEx(hkey,
                                             varname,
                                             0,
                                             &type,
                                             lprstr,
                                             &length))
        {
          // the value exists so check it's type
          // if it's a string then return it
          // otherwise loop to the next search location
          //
          // we could put a switch statement here
          // and convert other types to strings
          if (REG_SZ == type)
          {
            RegCloseKey(hkey);
            return lprstr;
          }
        }

        RegCloseKey(hkey);
      }

      // restore the original DevNodeKey, in case we tacked on a subkey above
      DevNodeKey[DevNodeKeyLength] = '\0';
    }
  }

  // Return NULL if not found
  return( NULL );
}


/*----------------------------------------------------------------------
Function name:  atof100x

Description:    Perform an ASCII to Float x 100.

Information:

Return:         DWORD   the converted value.
----------------------------------------------------------------------*/
DWORD atof100x( const char *s )
{
   DWORD left, sign, mult;
   int i;

   for( i=0; s[i] == ' ' || s[i] == '\n' || s[i] == '\t'; i++)
     ;

   if (s[i] == '+' || s[i] == '-')
     sign = (s[i++] == '+') ? 1 : -1;
   else
     sign = 1; 
 
   for (left = 0; s[i] >= '0' && s[i] <= '9'; i++)
     left = (left * 10) + (s[i] - '0');

   left *= 100;
   if (s[i++] != '.')
     return( sign * left);
     
   // Here's where we get the bottom 2
   for (mult = 10; s[i] >= '0' && s[i] <= '9' && mult > 0; i++, mult /= 10)
     left = left + (mult * (s[i] - '0'));

   return(sign * left);
}
