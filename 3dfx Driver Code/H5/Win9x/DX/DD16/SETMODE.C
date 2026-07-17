/* -*-c++-*- */
/* $Header: setmode.c, 12, 10/11/00 8:51:39 PM, Brent$ */
/*
** Copyright (c) 1995-1999, 3Dfx Interactive, Inc.
** All Rights Reserved.
**
** Portions Copyright (C) 1995 Microsoft Corporation.  All Rights Reserved.
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
** File name:   setmode.c
**
** Description: This file contains the device specific code for
**              setting and verifying the physical modes.  It also
**              contains the list of all possible supported modes.
**
** $Revision: 12$
** $Date: 10/11/00 8:51:39 PM$
**
** $History: setmode.c $
** 
** *****************  Version 94  *****************
** User: Dalev        Date: 8/20/99    Time: 4:38p
** Updated in $/devel/h5/Win9x/dx/dd16
** Added IS_LCD_MODE to the ModeList[] table for 960x720 & 720x480 modes.
** 
** *****************  Version 93  *****************
** User: Rbissell     Date: 8/07/99    Time: 3:11p
** Updated in $/devel/h5/Win9x/dx/dd16
** tvout merge from V3_OEM_100
** 
** *****************  Version 92  *****************
** User: Cwilcox      Date: 7/15/99    Time: 4:12p
** Updated in $/devel/h5/Win9x/dx/dd16
** Add runtime checks for Napalm.
** 
** *****************  Version 91  *****************
** User: Edwin        Date: 6/29/99    Time: 3:57p
** Updated in $/devel/h5/Win9x/dx/dd16
** Remove obsolete Banshee ifdefs.
** 
** *****************  Version 90  *****************
** User: Andrew       Date: 6/16/99    Time: 4:45p
** Updated in $/devel/h5/Win9x/dx/dd16
** Added SKIP_FLAGS
** 
** *****************  Version 89  *****************
** User: Andrew       Date: 6/04/99    Time: 4:10p
** Updated in $/devel/h5/Win9x/dx/dd16
** Added code to support UnitNumbers
** 
** *****************  Version 87  *****************
** User: Stb_srogers  Date: 5/19/99    Time: 9:04a
** Updated in $/devel/h3/win95/dx/dd16
** Adding in Compaq modes.
** 
** *****************  Version 86  *****************
** User: Edwin        Date: 5/18/99    Time: 1:12p
** Updated in $/devel/h3/Win95/dx/dd16
** Remove 24bpp modes for Napalm by adding if !defined(H5) in ModeList[]
** table.
** 
** *****************  Version 85  *****************
** User: Andrew       Date: 5/10/99    Time: 1:34p
** Updated in $/devel/h3/Win95/dx/dd16
** Changed PhysScreenAddr to RealregBase
** 
** *****************  Version 84  *****************
** User: Andrew       Date: 5/06/99    Time: 4:23p
** Updated in $/devel/h3/Win95/dx/dd16
** Modified to work with Trapping "C" Simulator
** 
** *****************  Version 83  *****************
** User: Cwilcox      Date: 4/26/99    Time: 4:57p
** Updated in $/devel/h3/Win95/dx/dd16
** Added tiled memory support for 32bpp modes, Napalm only.
** 
** *****************  Version 82  *****************
** User: Stb_mimhoff  Date: 4/26/99    Time: 1:14p
** Updated in $/devel/h3/win95/dx/dd16
** More work on PRS 5097... Slower PCs were still having the problem, so
** the HWSetPalette function was optimised...
** 
** *****************  Version 81  *****************
** User: Cshaw        Date: 4/20/99    Time: 12:24p
** Updated in $/devel/h3/Win95/dx/dd16
** Added Napalm's enabling of 2-pixel-per-clock rendering for 16bpp modes
** of "non-high" bandwidth requirements.  We'll need to tweak which modes
** are enabled after we get Silicon back.
** 
** *****************  Version 80  *****************
** User: Stb_mimhoff  Date: 4/19/99    Time: 2:13p
** Updated in $/devel/h3/win95/dx/dd16
** Missed checking in DEFINE of DPMS_MASK
** 
** *****************  Version 79  *****************
** User: Stb_mimhoff  Date: 4/19/99    Time: 12:31p
** Updated in $/devel/h3/win95/dx/dd16
** Fix PRS 5469...Changing resolutions overrides the Disable Monotor
** checkbox in the 3dfx TV control panel...To fix this, we simply restore
** the state of the DPMS bits before and after the modeset.
** 
** *****************  Version 78  *****************
** User: Stb_bseitsin Date: 4/09/99    Time: 12:37p
** Updated in $/devel/h3/win95/dx/dd16
** Added Napalm registers. Added ifdef H5.
** 
** *****************  Version 77  *****************
** User: Cwilcox      Date: 4/08/99    Time: 2:01p
** Updated in $/devel/h3/Win95/dx/dd16
** Added #ifdef LINEAR_ONLY to build without tiled mode.
** 
** *****************  Version 76  *****************
** User: Stb_mimhoff  Date: 4/08/99    Time: 10:35a
** Updated in $/devel/h3/win95/dx/dd16
** Imhoff - Fix for PRS 5097 ( At least this works on my system ) This
** read appears to be a wasted instruction and causes sparkle in
** resolutions 1920 or higher.
** 
** *****************  Version 75  *****************
** User: Pratt        Date: 2/28/99    Time: 4:50p
** Updated in $/devel/h3/Win95/dx/dd16
** added debug statements for tracing flow of LCD code.
** 
** *****************  Version 74  *****************
** User: Andrew       Date: 3/13/99    Time: 7:30p
** Updated in $/devel/h3/Win95/dx/dd16
** Added Code to Check DacMode before checking VSYNC
** 
** *****************  Version 73  *****************
** User: Xingc        Date: 3/11/99    Time: 10:20a
** Updated in $/devel/h3/Win95/dx/dd16
** Add PRE_MODE_CHANGE into HWSetMode()
** 
** *****************  Version 72  *****************
** User: Stb_srogers  Date: 3/09/99    Time: 1:26p
** Updated in $/devel/h3/win95/dx/dd16
** Adding 960x720 and 1280x960 modes into the list of Desktop modes.  We
** must do this to pass WHQL.  Fixes PRS 4936
** 
** *****************  Version 71  *****************
** User: Stb_srogers  Date: 3/06/99    Time: 4:42p
** Updated in $/devel/h3/win95/dx/dd16
** Renabling DD modes that are higher than 1600x1200
** 
** *****************  Version 70  *****************
** User: Stb_srogers  Date: 3/06/99    Time: 8:23a
** Updated in $/devel/h3/win95/dx/dd16
** Fix for PRS 4900, adding 960x720 and 1280x960 modes in for DDraw Only
** 
** *****************  Version 69  *****************
** User: Stb_srogers  Date: 3/06/99    Time: 7:33a
** Updated in $/devel/h3/win95/dx/dd16
** Removing 24 bit modes for 1792x1344 and 1856x1392
** 
** *****************  Version 68  *****************
** User: Stb_jcampbel Date: 3/05/99    Time: 2:27p
** Updated in $/devel/h3/win95/dx/dd16
** Remove certain desktop modes from list of valid D3D modes.
** 
** *****************  Version 67  *****************
** User: Stuartb      Date: 3/05/99    Time: 1:59p
** Updated in $/devel/h3/Win95/dx/dd16
** Do not enable TVOUT_MODEs if refresh rate is > 85hz.
** 
** *****************  Version 66  *****************
** User: Stb_srogers  Date: 3/02/99    Time: 3:56p
** Updated in $/devel/h3/Win95/dx/dd16
** Adding support for 960x720 & 1280x960 modes
** 
** *****************  Version 65  *****************
** User: Stb_srogers  Date: 2/23/99    Time: 4:33p
** Updated in $/devel/h3/win95/dx/dd16
** Removing 85 Hz for 2046x1536, changing pitch values for 16 bpp to tiled
** pitch
** 
** *****************  Version 64  *****************
** User: Stb_srogers  Date: 2/22/99    Time: 7:32a
** Updated in $/devel/h3/win95/dx/dd16
** Changes outward appearance of 2048 to 2046
** 
** *****************  Version 63  *****************
** User: Stb_srogers  Date: 2/18/99    Time: 4:23p
** Updated in $/devel/h3/win95/dx/dd16
** 
** *****************  Version 62  *****************
** User: Cwilcox      Date: 2/18/99    Time: 12:21p
** Updated in $/devel/h3/Win95/dx/dd16
** Final removal of tiled/linear promotion.
** 
** *****************  Version 61  *****************
** User: Stb_srogers  Date: 2/16/99    Time: 4:08p
** Updated in $/devel/h3/win95/dx/dd16
** 
** *****************  Version 60  *****************
** User: Stb_srogers  Date: 2/11/99    Time: 6:57p
** Updated in $/devel/h3/win95/dx/dd16
** 
** *****************  Version 59  *****************
** User: Andrew       Date: 2/11/99    Time: 4:42p
** Updated in $/devel/h3/Win95/dx/dd16
** Removed 2048,x1536x32 from 3DFX Modes.  Ifdef H4 around 2048,1536
** modes.  This mode was not in the MRD for H3.
** 
** *****************  Version 58  *****************
** User: Cwilcox      Date: 2/10/99    Time: 6:27p
** Updated in $/devel/h3/Win95/dx/dd16
** Fixed initialization of tiled primary for low resolution modes.
** 
** *****************  Version 57  *****************
** User: Cwilcox      Date: 2/10/99    Time: 3:52p
** Updated in $/devel/h3/Win95/dx/dd16
** Linear versus tiled promotion removal.
** 
** *****************  Version 56  *****************
** User: Andrew       Date: 2/07/99    Time: 4:44p
** Updated in $/devel/h3/Win95/dx/dd16
** Added some new modes to 3DFX table -- 1600x1024, 1920x1200, and
** 2048x1536.
** 
** *****************  Version 55  *****************
** User: Stb_srogers  Date: 1/29/99    Time: 7:06a
** Updated in $/devel/h3/win95/dx/dd16
** 
** *****************  Version 54  *****************
** User: Cwilcox      Date: 1/25/99    Time: 11:39a
** Updated in $/devel/h3/Win95/dx/dd16
** Minor modifications to remove compiler warnings.
** 
** *****************  Version 53  *****************
** User: Stuartb      Date: 1/21/99    Time: 10:10a
** Updated in $/devel/h3/Win95/dx/dd16
** Force 60hz refresh rate if flat panel is present, connected and active.
** 
** *****************  Version 52  *****************
** User: Andrew       Date: 1/15/99    Time: 9:20a
** Updated in $/devel/h3/Win95/dx/dd16
** Fix for Gateway Monitor
** 
** *****************  Version 51  *****************
** User: Andrew       Date: 1/08/99    Time: 8:26p
** Updated in $/devel/h3/Win95/dx/dd16
** Fixed load of palette to be in Vertical Retrace
** 
** *****************  Version 50  *****************
** User: Michael      Date: 12/31/98   Time: 9:32a
** Updated in $/devel/h3/Win95/dx/dd16
** STB's customized refresh rates and customer file naming conventions.
** STB's customizations are surrounded by "#ifdef INCSTBCUST".
** 
** *****************  Version 49  *****************
** User: Michael      Date: 12/30/98   Time: 9:30a
** Updated in $/devel/h3/Win95/dx/dd16
** Implement the 3Dfx/STB unified header.
** 
** *****************  Version 48  *****************
** User: Stuartb      Date: 12/21/98   Time: 4:34p
** Updated in $/devel/h3/Win95/dx/dd16
** Just tweaked LCD modes a bit.
** 
** *****************  Version 47  *****************
** User: Stuartb      Date: 12/21/98   Time: 11:38a
** Updated in $/devel/h3/Win95/dx/dd16
** Added initialization for tvout and flat panel modes.
** 
** *****************  Version 46  *****************
** User: Cwilcox      Date: 12/18/98   Time: 12:36p
** Updated in $/devel/h3/Win95/dx/dd16
** Cleaned up dd3dInOverlay values
** 
** *****************  Version 45  *****************
** User: Cwilcox      Date: 12/11/98   Time: 11:14a
** Updated in $/devel/h3/Win95/dx/dd16
** Redo previous checkin.
** 
** *****************  Version 43  *****************
** User: Andrew       Date: 12/10/98   Time: 12:37p
** Updated in $/devel/h3/Win95/dx/dd16
** Added set of flag for Tiled/Linear Flip
** 
** *****************  Version 42  *****************
** User: Martin       Date: 12/04/98   Time: 4:24p
** Updated in $/devel/h3/Win95/dx/dd16
** Blow away tv-out
** 
** *****************  Version 41  *****************
** User: Andrew       Date: 11/15/98   Time: 5:17p
** Updated in $/devel/h3/Win95/dx/dd16
** Added ifdef like to be able to build like GLOP
** 
** *****************  Version 40  *****************
** User: Stuartb      Date: 11/04/98   Time: 3:46p
** Updated in $/devel/h3/Win95/dx/dd16
** If booted to tvout, allow only valid tvout modes in HWTestMode.
** 
** *****************  Version 39  *****************
** User: Edwin        Date: 9/21/98    Time: 3:51p
** Updated in $/devel/h3/Win95/dx/dd16
** Use linear pitch in ModeList[] except for 16bpp.
** 
** *****************  Version 38  *****************
** User: Artg         Date: 8/28/98    Time: 11:17a
** Updated in $/devel/h3/Win95/dx/dd16
** changed the mode set to use banshee host instead of S3 968 path.
** 
** *****************  Version 37  *****************
** User: Andrew       Date: 8/26/98    Time: 12:24a
** Updated in $/devel/h3/Win95/dx/dd16
** removed 8x6 at 56 Hz
** 
** *****************  Version 36  *****************
** User: Andrew       Date: 8/20/98    Time: 10:13p
** Updated in $/devel/h3/Win95/dx/dd16
** Removed 24 & 32 bpp low rez modes and 720xY modes
** 
** *****************  Version 35  *****************
** User: Andrew       Date: 8/13/98    Time: 9:12p
** Updated in $/devel/h3/Win95/dx/dd16
** Changed the Horizontal refresh rate at 800x600 @ 120 as it was wrong
** 
** *****************  Version 34  *****************
** User: Andrew       Date: 7/27/98    Time: 1:23p
** Updated in $/devel/h3/Win95/dx/dd16
** Added scanline double for H3_BO and ability to double x & y
** independently.
** 
** *****************  Version 33  *****************
** User: Ken          Date: 7/24/98    Time: 10:38p
** Updated in $/devel/h3/win95/dx/dd16
** changes to allow 2d driver to run properly synchronized with an AGP
** command fifo (although video memory fifo is still used when the desktop
** has the focus, e.g., a fullscreen 3d app isn't in the foreground)
** 
** *****************  Version 32  *****************
** User: Edwin        Date: 7/21/98    Time: 4:03p
** Updated in $/devel/h3/Win95/dx/dd16
** Fix 1741, added bScanlineDouble to fixup cursor positions for "overlay
** double" modes, e.g. 320x200x16.
** 
** *****************  Version 31  *****************
** User: Andrew       Date: 7/21/98    Time: 2:44p
** Updated in $/devel/h3/Win95/dx/dd16
** Added new refreshs for 320x200, 320x240, 512x384, 400x300, and 1152x864
** 
** *****************  Version 30  *****************
** User: Andrew       Date: 7/13/98    Time: 5:24p
** Updated in $/devel/h3/Win95/dx/dd16
** Added a gammatable to be passed by Banshee Property Page
** 
** *****************  Version 29  *****************
** User: Andrew       Date: 7/11/98    Time: 8:17a
** Updated in $/devel/h3/Win95/dx/dd16
** Added gamma correction
** 
** *****************  Version 28  *****************
** User: Ken          Date: 7/01/98    Time: 6:36p
** Updated in $/devel/h3/win95/dx/dd16
** added 16bpp low res modes using video overlay stretching
**
*/

/*==========================================================================;
 *  public functions:
 *      HWSetMode
 *      HWTestMode
 *      HWSetPalette
 *      HWBeginAccess
 *      HWEndAccess
 *
 *  public data:
 *      ModeList
 ***************************************************************************/

#include <string.h>
#include "header.h"

#define Not_VxD
#include "minivdd.h"
#include "modelist.h"
#include "cursor.h"
#include <entrleav.h>
#include "dfpapi.h"
extern void FXWAITFORIDLE();

extern MODEINFO FAR * ModeList;
extern int nNumModes;
extern UINT GetFlatSel(void);
#ifdef SLI_AA
extern DISPLAYINFO DisplayInfo;
#endif

#ifdef WIN_CSIM
#undef IS_NAPALM
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
#endif

int bScanlineDouble = 0x0;   // fixup cursor positions if SCANLINE_DBL set

#define DPMS_MASK (SST_DAC_DPMS_ON_HSYNC | SST_DAC_DPMS_ON_VSYNC | SST_DAC_FORCE_VSYNC | SST_DAC_FORCE_HSYNC)

extern DWORD PLL2MHz(DWORD clock );
extern UINT GetFlatSel( void );
extern DWORD dwDeviceHandle;
DWORD grxFreq = 0;
DWORD grxClock = 0;


DWORD GammaTable[] = {
	 0x00000000, // 0
	 0x00010101, // 1
	 0x00020202, // 2
	 0x00030303, // 3
	 0x00040404, // 4
	 0x00050505, // 5
	 0x00060606, // 6
	 0x00070707, // 7
	 0x00080808, // 8
	 0x00090909, // 9
	 0x000a0a0a, // 10
	 0x000b0b0b, // 11
	 0x000c0c0c, // 12
	 0x000d0d0d, // 13
	 0x000e0e0e, // 14
	 0x000f0f0f, // 15
	 0x00101010, // 16
	 0x00111111, // 17
	 0x00121212, // 18
	 0x00131313, // 19
	 0x00141414, // 20
	 0x00151515, // 21
	 0x00161616, // 22
	 0x00171717, // 23
	 0x00181818, // 24
	 0x00191919, // 25
	 0x001a1a1a, // 26
	 0x001b1b1b, // 27
	 0x001c1c1c, // 28
	 0x001d1d1d, // 29
	 0x001e1e1e, // 30
	 0x001f1f1f, // 31
	 0x00202020, // 32
	 0x00212121, // 33
	 0x00222222, // 34
	 0x00232323, // 35
	 0x00242424, // 36
	 0x00252525, // 37
	 0x00262626, // 38
	 0x00272727, // 39
	 0x00282828, // 40
	 0x00292929, // 41
	 0x002a2a2a, // 42
	 0x002b2b2b, // 43
	 0x002c2c2c, // 44
	 0x002d2d2d, // 45
	 0x002e2e2e, // 46
	 0x002f2f2f, // 47
	 0x00303030, // 48
	 0x00313131, // 49
	 0x00323232, // 50
	 0x00333333, // 51
	 0x00343434, // 52
	 0x00353535, // 53
	 0x00363636, // 54
	 0x00373737, // 55
	 0x00383838, // 56
	 0x00393939, // 57
	 0x003a3a3a, // 58
	 0x003b3b3b, // 59
	 0x003c3c3c, // 60
	 0x003d3d3d, // 61
	 0x003e3e3e, // 62
	 0x003f3f3f, // 63
	 0x00404040, // 64
	 0x00414141, // 65
	 0x00424242, // 66
	 0x00434343, // 67
	 0x00444444, // 68
	 0x00454545, // 69
	 0x00464646, // 70
	 0x00474747, // 71
	 0x00484848, // 72
	 0x00494949, // 73
	 0x004a4a4a, // 74
	 0x004b4b4b, // 75
	 0x004c4c4c, // 76
	 0x004d4d4d, // 77
	 0x004e4e4e, // 78
	 0x004f4f4f, // 79
	 0x00505050, // 80
	 0x00515151, // 81
	 0x00525252, // 82
	 0x00535353, // 83
	 0x00545454, // 84
	 0x00555555, // 85
	 0x00565656, // 86
	 0x00575757, // 87
	 0x00585858, // 88
	 0x00595959, // 89
	 0x005a5a5a, // 90
	 0x005b5b5b, // 91
	 0x005c5c5c, // 92
	 0x005d5d5d, // 93
	 0x005e5e5e, // 94
	 0x005f5f5f, // 95
	 0x00606060, // 96
	 0x00616161, // 97
	 0x00626262, // 98
	 0x00636363, // 99
	 0x00646464, // 100
	 0x00656565, // 101
	 0x00666666, // 102
	 0x00676767, // 103
	 0x00686868, // 104
	 0x00696969, // 105
	 0x006a6a6a, // 106
	 0x006b6b6b, // 107
	 0x006c6c6c, // 108
	 0x006d6d6d, // 109
	 0x006e6e6e, // 110
	 0x006f6f6f, // 111
	 0x00707070, // 112
	 0x00717171, // 113
	 0x00727272, // 114
	 0x00737373, // 115
	 0x00747474, // 116
	 0x00757575, // 117
	 0x00767676, // 118
	 0x00777777, // 119
	 0x00787878, // 120
	 0x00797979, // 121
	 0x007a7a7a, // 122
	 0x007b7b7b, // 123
	 0x007c7c7c, // 124
	 0x007d7d7d, // 125
	 0x007e7e7e, // 126
	 0x007f7f7f, // 127
	 0x00808080, // 128
	 0x00818181, // 129
	 0x00828282, // 130
	 0x00838383, // 131
	 0x00848484, // 132
	 0x00858585, // 133
	 0x00868686, // 134
	 0x00878787, // 135
	 0x00888888, // 136
	 0x00898989, // 137
	 0x008a8a8a, // 138
	 0x008b8b8b, // 139
	 0x008c8c8c, // 140
	 0x008d8d8d, // 141
	 0x008e8e8e, // 142
	 0x008f8f8f, // 143
	 0x00909090, // 144
	 0x00919191, // 145
	 0x00929292, // 146
	 0x00939393, // 147
	 0x00949494, // 148
	 0x00959595, // 149
	 0x00969696, // 150
	 0x00979797, // 151
	 0x00989898, // 152
	 0x00999999, // 153
	 0x009a9a9a, // 154
	 0x009b9b9b, // 155
	 0x009c9c9c, // 156
	 0x009d9d9d, // 157
	 0x009e9e9e, // 158
	 0x009f9f9f, // 159
	 0x00a0a0a0, // 160
	 0x00a1a1a1, // 161
	 0x00a2a2a2, // 162
	 0x00a3a3a3, // 163
	 0x00a4a4a4, // 164
	 0x00a5a5a5, // 165
	 0x00a6a6a6, // 166
	 0x00a7a7a7, // 167
	 0x00a8a8a8, // 168
	 0x00a9a9a9, // 169
	 0x00aaaaaa, // 170
	 0x00ababab, // 171
	 0x00acacac, // 172
	 0x00adadad, // 173
	 0x00aeaeae, // 174
	 0x00afafaf, // 175
	 0x00b0b0b0, // 176
	 0x00b1b1b1, // 177
	 0x00b2b2b2, // 178
	 0x00b3b3b3, // 179
	 0x00b4b4b4, // 180
	 0x00b5b5b5, // 181
	 0x00b6b6b6, // 182
	 0x00b7b7b7, // 183
	 0x00b8b8b8, // 184
	 0x00b9b9b9, // 185
	 0x00bababa, // 186
	 0x00bbbbbb, // 187
	 0x00bcbcbc, // 188
	 0x00bdbdbd, // 189
	 0x00bebebe, // 190
	 0x00bfbfbf, // 191
	 0x00c0c0c0, // 192
	 0x00c1c1c1, // 193
	 0x00c2c2c2, // 194
	 0x00c3c3c3, // 195
	 0x00c4c4c4, // 196
	 0x00c5c5c5, // 197
	 0x00c6c6c6, // 198
	 0x00c7c7c7, // 199
	 0x00c8c8c8, // 200
	 0x00c9c9c9, // 201
	 0x00cacaca, // 202
	 0x00cbcbcb, // 203
	 0x00cccccc, // 204
	 0x00cdcdcd, // 205
	 0x00cecece, // 206
	 0x00cfcfcf, // 207
	 0x00d0d0d0, // 208
	 0x00d1d1d1, // 209
	 0x00d2d2d2, // 210
	 0x00d3d3d3, // 211
	 0x00d4d4d4, // 212
	 0x00d5d5d5, // 213
	 0x00d6d6d6, // 214
	 0x00d7d7d7, // 215
	 0x00d8d8d8, // 216
	 0x00d9d9d9, // 217
	 0x00dadada, // 218
	 0x00dbdbdb, // 219
	 0x00dcdcdc, // 220
	 0x00dddddd, // 221
	 0x00dedede, // 222
	 0x00dfdfdf, // 223
	 0x00e0e0e0, // 224
	 0x00e1e1e1, // 225
	 0x00e2e2e2, // 226
	 0x00e3e3e3, // 227
	 0x00e4e4e4, // 228
	 0x00e5e5e5, // 229
	 0x00e6e6e6, // 230
	 0x00e7e7e7, // 231
	 0x00e8e8e8, // 232
	 0x00e9e9e9, // 233
	 0x00eaeaea, // 234
	 0x00ebebeb, // 235
	 0x00ececec, // 236
	 0x00ededed, // 237
	 0x00eeeeee, // 238
	 0x00efefef, // 239
	 0x00f0f0f0, // 240
	 0x00f1f1f1, // 241
	 0x00f2f2f2, // 242
	 0x00f3f3f3, // 243
	 0x00f4f4f4, // 244
	 0x00f5f5f5, // 245
	 0x00f6f6f6, // 246
	 0x00f7f7f7, // 247
	 0x00f8f8f8, // 248
	 0x00f9f9f9, // 249
	 0x00fafafa, // 250
	 0x00fbfbfb, // 251
	 0x00fcfcfc, // 252
	 0x00fdfdfd, // 253
	 0x00fefefe, // 254
	 0x00ffffff, // 255
   };


#define NUM_MODES (sizeof(ModeList) / sizeof(ModeList[0]) - 1)

/***************************************************************************
 *
 * BiosMode
 *
 * this is a table the tells us what BIOS mode to use, given a mode number
 * why do we have two tables? because we give a pointer to ModeList
 * to DirectDraw and it has to be a specific format....
 *
 ***************************************************************************/

UINT BiosMode[] = {
    0x13,            //0
    0x00,            //1
    0x00,            //2
    0x00,            //3
    0x101,            //4 640 x 8
    0x103,            //5 800 x 8
    0x105,           //6 1024 x 8
    0x107,           //7 1280 x 8
    0x00,            //8
    0x00,            //9
    0x00,            //10
    0x00,            //11
    0x00,            //12
    0x00,            //13
    0x00,            //14
    0x00,            //15
    0x00,            //16
    0x00,            //17
    0x00,            //18
    0x00,            //19
    0x00,            //20
    0x00,            //21
    0x00,            //22
    0x111,           //23 640 x 16
    0x114,           //24 800 x 16
    0x117,           //25 1024 x 16
    0x00,            //15
    0x00,            //16
    0x00,            //17
    0x00,            //18
    0x00,            //19
    0x112,           //20 640 x 24
    0x115,           //20 800 X 24
    0x118,           //20 1024 x 24
    0x00,            //20
    0x00,            //20
    0x00,            //20
};


/*----------------------------------------------------------------------
Function name:  HWTestMode

Description:    This function is called to verify a mode can be done.

Information:
    This function checks to see if there is enough VRAM,
    the hardware is capable and present, and any thing else.

    This function needs to be "safe" it can't disturb the
    hardware or change the current display mode.

Return:         BOOL    TRUE for success, FALSE for failure.
----------------------------------------------------------------------*/
BOOL HWTestMode(int ModeNumber)
{
    MODEINFO *pMode;
    FxU32 memRequired;
	int i;

    if (ModeNumber < 0 || ModeNumber >= nNumModes)
        return FALSE;

    pMode = &ModeList[ModeNumber];

    if (_FF(TotalVRAM) == 0)
    {
	//
	// we're here because windows is querying a mode w/out having
	// actually loaded the full DRV on the h/w, so let's set our
	// memory size to the max possible, to "allow" all the potentially
	// possible modes
	//
        _FF(TotalVRAM) = 16L*1024L*1024L;
    }

    if (_FF(TotalVRAM) == 0)
    {
        // cant determine the VRAM size.
        return FALSE;
    }

    memRequired = pMode->lPitch * pMode->dwHeight;
    memRequired += _FF(ddLinearHeapStart);

    if (memRequired > _FF(TotalVRAM))
    {
        // not enough VRAM.
        return FALSE;
    }

	// Fix for PRS #8449 - V3CPQ: 2046x1536 displays, but should not.	
	if(!(IS_NAPALM) && ( _FF(customerNumber) == 8 ))
	{
		// When we get here, many of the variables that are used to access
		// memory mapped IO are not initialized, so we set up enough of them
		// here to obtain the Graphics Clock Speed.
		if ( grxFreq == 0 )
		{
		HwInfo MyHwInfo;
		HwInfo * pHwInfo = &MyHwInfo;
		UINT FlatSel = GetFlatSel();


		   	if ( lph3IORegs == 0 )
			{

			    VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle,
			    H3VDD_GET_HW_INFO, 0, pHwInfo);

			    if (pHwInfo->ioBase == 0)
			         return TRUE;

                for (i=0; i<HWINFO_SST_MAX_BASE_INDEX; i++) 
                  _FF(regBase[i]) = pHwInfo->regBase[i];

                lph3IORegs = (SstIORegs *)(_FF(regBase[HWINFO_SST_IOREGS_INDEX]));
			}

			grxClock = GET(lph3IORegs->pllCtrl1);
		   	grxFreq = PLL2MHz(grxClock) / 10000;
		}

		// The hardware is "jittery" at this mode at higher refreshes than 60 when the
		// Graphics clock is running at 126MHz (it's 126 and not 125 is to give us a margin
		// just in case).
		if ( pMode->dwWidth >= 2046 && pMode->dwHeight >= 1536 && pMode->wVert > 60 )
		{
	   		if ( grxFreq <= 126 )
	   			return FALSE;
		}

	}  // if ( customerNumber == 8 )

	if ( IS_VOODOO3 )		   	// DYNAMIC MODE TABLE
	{
		// Voodoo3 can't do this mode at 32 bit depths.
		if ( pMode->dwWidth >= 2046 && pMode->dwHeight >= 1536 && pMode->dwBPP > 16 )
	   		return FALSE;
	}

    return TRUE;
}


/*----------------------------------------------------------------------
Function name:  bppToPixfmt

Description:    Translate a bits per pixel into a video processor
                desktop pixel format setting.
Information:

Return:         FxU32   The pixel format setting or,
                        0 for failure.
----------------------------------------------------------------------*/
FxU32
bppToPixfmt(FxU32 bpp)
{
    switch (bpp)
    {
      case 8:
	  return SST_DESKTOP_PIXEL_PAL8;
      case 16:
	  return SST_DESKTOP_PIXEL_RGB565;
      case 24:
	  return SST_DESKTOP_PIXEL_RGB24;
      case 32:
	  return SST_DESKTOP_PIXEL_RGB32;

      default:
	     DPF(DBGLVL_NORMAL, "bppToPixfmt: bad bpp value: %d\n", bpp);
	     return 0;
    }
}

/*----------------------------------------------------------------------
Function name:  HWSetMode

Description:    This function is called to set the mode.
                
Information:
    Only a mode that as passed HWTestMode will be get this far.

    This function needs to fill out the following globals.

    If the mode is a linear mode
        DriverData.TotalVRAM        ; total amount of VRAM
        DriverData.ScreenAddress    ; *physical* address of frame buffer
        DriverData.ScreenSel        ; zero

    If the mode is a banked mode
        DriverData.TotalVRAM        ; total amount of VRAM
        DriverData.ScreenAddress    ; zero
        DriverData.ScreenSel        ; selector to VFlatD virtual frame buffer.

Return:         BOOL    TRUE for success, FALSE for failure
----------------------------------------------------------------------*/
#pragma optimize("", off)
void VX900T_Fixup(int ModeNumber);

BOOL 
HWSetMode(int ModeNumber)
{
    VidProcConfig vpc;
    VidProcConfig *pVpc = &vpc;
	DWORD dacMode;
	DWORD dpmsBits;


    // ensure only modes with SCANLINE_DBL bit set turns this on
    bScanlineDouble = 0x0;    // no cursor positions fixup

    //
    // mode number -1 means restore the video mode present when windows
    // was booting (VGA mode 3 is assumed here)
    //
    if (ModeNumber == -1)
    {
        _asm mov ax,3       ;; go back to text mode.
        _asm int 10h
        return TRUE;
    }

    //
    // verify that the mode number is in range
    //
    if (ModeNumber < 0 || ModeNumber >= NUM_MODES)
        return FALSE;

    VDDCall(VDD_PRE_MODE_CHANGE, dwDeviceHandle, 0, 0, 0);

#ifdef  S3_INIT
    videosetmode(ModeNumber, 0);
    return TRUE;
#endif // #ifdef S3_INIT

    pVpc->changeVideoMode = 1;
    pVpc->width   = ModeList[ModeNumber].dwWidth;

    pVpc->height  = ModeList[ModeNumber].dwHeight;
	pVpc->refresh = ModeList[ModeNumber].wVert;

   // @RBISSELL, I've removed the "_FF(TvoActive)" check from the following
   //            if statement, because the Voodoo3 is actually in slave mode,
   //            so that the bt868 is providing the signals.  As such, it really
   //            doesn't matter what we think the refresh rate is for TVOUT.
   //            As it was, this was keeping us from displaying certain modes
   //            on the TV, because we didn't have 60Hz versions of those modes
   //            in the table.
	
	//=======================================================================================
	// DYNAMIC MODE TABLE begins
	//=======================================================================================

    if (DFPisPanelActive() && 
        (ModeList[ModeNumber].dwDFPWidth != 0))
    {
        pVpc->TimingParams.width  = ModeList[ModeNumber].dwDFPWidth;
        pVpc->TimingParams.height = ModeList[ModeNumber].dwDFPHeight;
    }
    else
    {
	    pVpc->TimingParams.width = ModeList[ModeNumber].dwWidth;
	    pVpc->TimingParams.height = ModeList[ModeNumber].dwHeight;
    }
	pVpc->TimingParams.refresh = pVpc->refresh;

	pVpc->TimingParams.HTotal = ModeList[ModeNumber].HTotal;
	pVpc->TimingParams.HSyncStart = ModeList[ModeNumber].HSyncStart;
	pVpc->TimingParams.HSyncEnd = ModeList[ModeNumber].HSyncEnd;
	pVpc->TimingParams.VTotal = ModeList[ModeNumber].VTotal;
	pVpc->TimingParams.VSyncStart = ModeList[ModeNumber].VSyncStart;
	pVpc->TimingParams.VSyncEnd = ModeList[ModeNumber].VSyncEnd;
	pVpc->TimingParams.CRTCflags = ModeList[ModeNumber].CRTCflags;
	pVpc->TimingParams.PixelClock = ModeList[ModeNumber].PixelClock;
	pVpc->TimingParams.CharWidth = ModeList[ModeNumber].CharWidth;
	pVpc->TimingParams.UseGTF = ModeList[ModeNumber].UseGTF;

    if (ModeList[ModeNumber].dwFlags & INTERLACED) 
	{
		// Half the vertical timings for interlaced modes
		// NOTE: This is not fully tested as the Voodoo3 HW does not support interlaced
		pVpc->TimingParams.VTotal /= 2;
		pVpc->TimingParams.VSyncStart /= 2;
		pVpc->TimingParams.VSyncEnd /= 2;
	}

	pVpc->TimingParams.UseAltTiming = ModeList[ModeNumber].UseAltTiming;
 	_fmemcpy( pVpc->TimingParams.AltTiming, ModeList[ModeNumber].AltTiming, CRTC_TABLE_SIZE );

	//=======================================================================================
	// DYNAMIC MODE TABLE ends
	//=======================================================================================

    if (ModeList[ModeNumber].dwFlags & SCANLINE_DBL)
    {
       bScanlineDouble = CURSOR_DBL_Y; // fixup cursor positions for this mode
	}

    // for now, always enable this mode as the desktop, and disable the
    // overlay
    //
    pVpc->changeDesktop = 1;
    pVpc->desktopSurface.enable = 1;
    pVpc->desktopSurface.tiled = _FF(ddPrimaryInTile) ? 1: 0;

    pVpc->desktopSurface.pixFmt = bppToPixfmt(ModeList[ModeNumber].dwBPP);
    pVpc->desktopSurface.clutBypass = 0;
    pVpc->desktopSurface.clutSelect = 0;
    pVpc->desktopSurface.startAddress = _FF(gdiDesktopStart) & (~SSTG_IS_TILED);

    pVpc->desktopSurface.stride = ModeList[ModeNumber].lPitch;
    if (pVpc->desktopSurface.tiled)
	{
       pVpc->desktopSurface.stride = _FF(ddTileStride);
	}

    pVpc->desktopSurface.scanlinedouble = (ModeList[ModeNumber].dwFlags & SCANLINE_DBL) ? TRUE : FALSE;

    // right now, disable the overlay surface
    //
    pVpc->changeOverlay = 1;
    pVpc->overlaySurface.enable = 0;
    pVpc->overlaySurface.stereo	= 0;
    pVpc->overlaySurface.horizScaling = 0;
    pVpc->overlaySurface.dudx = 0;
    pVpc->overlaySurface.verticalScaling = 0;
    pVpc->overlaySurface.dvdy = 0;
    pVpc->overlaySurface.filterMode = 0;
    pVpc->overlaySurface.tiled = 0;
    pVpc->overlaySurface.pixFmt	= 0;
    pVpc->overlaySurface.clutBypass = 0;
    pVpc->overlaySurface.clutSelect = 0;
    pVpc->overlaySurface.startAddress = 0;
    pVpc->overlaySurface.stride	= 0;

	dacMode = GET(lph3IORegs->dacMode);
#ifdef SLI_AA
    pVpc->diUnitNumber = DisplayInfo.diUnitNumber;
#endif
    VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle, H3VDD_SET_VIDEO_MODE, 0, pVpc);
    VX900T_Fixup(ModeNumber);

	// Imhoff - Fix PRS 5469... Changing resolutions overrides the "Disable Monitor"
	// checkbox in the 3dfx TV control panel... To fix this, we simply save and 
	// restore the state of the DPMS bits before and after the modeset.
	if ( _FF(dwTvoActive) )
	{
		dpmsBits = dacMode & DPMS_MASK;
		dacMode = GET(lph3IORegs->dacMode);
		dacMode &= ~DPMS_MASK;
		dacMode |= dpmsBits;
   		SETDW(lph3IORegs->dacMode, dacMode);
	}

    return TRUE;
}

#pragma optimize("", on)


/*----------------------------------------------------------------------
Function name:  HWBeginAccess

Description:    This function is called to make sure the framebuffer
                can be written to if your hardware has blitter mode
                and a framebuffer mode, this function needs to wait
                for the blitter to finish and enable framebufffer
                mode.
                
Information:    This routine is empty!  BeginAccess handled
                elsewhere.

Return:         VOID
----------------------------------------------------------------------*/
void HWBeginAccess()
{
    //
    // ********************* INSERT CODE HERE ***********************
    //
    // because the is a sample we just fake it, or if your hardware
    // does not need to do anything you can fake it too.

    //
    // ********************* INSERT CODE HERE ***********************
    //
}


/*----------------------------------------------------------------------
Function name:  HWEndAccess

Description:    Compliment of HWBeginAccess.
                
Information:    This routine is empty!  EndAccess handled
                elsewhere.

Return:         VOID
----------------------------------------------------------------------*/
void HWEndAccess()
{
    //
    // ********************* INSERT CODE HERE ***********************
    //
    // because the is a sample we just fake it, or if your hardware
    // does not need to do anything you can fake it too.

    //
    // ********************* INSERT CODE HERE ***********************
    //
}


/*----------------------------------------------------------------------
Function name:  HWSetPalette

Description:    Program the HW's VGA DAC
                
Information:    This will need to change if the DAC is not VGA
                compatible or you want to enable 8bit DAC support.

Return:         BOOL    TRUE is always returned.   

----------------------------------------------------------------------*/

#define NOINVERTEDDAC
#undef DUMPDAC

#define CHECK_PALETTE
#ifdef CHECK_PALETTE
WORD CheckPalette=0;
#endif

void LoadPalette(DWORD DacAddr, FxU32 FAR *  pColor, WORD FlatSel, WORD Start, WORD End);

#pragma warning (disable: 4704)
BOOL HWSetPalette(int start, int count, DWORD FAR *colors)
{
#ifdef WIN_CSIM
  SstIORegs FAR *lph3IORegs = (SstIORegs *)_FF(regRealBase);
#endif
  FxU32 color[256];
  FxU32 data;
  int index;
  WORD wRed;
  WORD wGreen;
  WORD wBlue;
  WORD FlatSel=GetFlatSel(); 
  int j;
  #define MAX_RETRY (10)

  // This fixes a problem with Turok.

  if (_FF(gdiFlags) & SKIP_FLAGS)
    return TRUE;

  // Apply gamma correction.

  for (index = start; index < (start + count); index++)
  {
	 color[index] = colors[index-start];
    if (8 == ModeList[_FF(ModeNumber)].dwBPP)
    {
      wRed   = (WORD)((color[index] & 0x00FF0000) >> 16);
      wGreen = (WORD)((color[index] & 0x0000FF00) >> 8);
      wBlue  = (WORD)(color[index] & 0x000000FF);
    }
    else
      wRed = wGreen = wBlue = index;

    // Gamma correct, and convert RGB to BGR.

    color[index] = (GammaTable[wRed]   & 0x000000FF) | 
                   (GammaTable[wGreen] & 0x0000FF00) |
                   (GammaTable[wBlue]  & 0x00FF0000);

#ifndef NOINVERTEDDAC
    color[index] = ~color[index];
#endif

  }

  // Make sure Hsync and Vsync are toggling before waiting on vertical retrace.

  if (!(GET(lph3IORegs->dacMode) & (SST_DAC_DPMS_ON_VSYNC|SST_DAC_FORCE_VSYNC|SST_DAC_DPMS_ON_HSYNC|SST_DAC_FORCE_HSYNC)))
     {
      // if in vtrace wait until out
      while ((GET(lph3IORegs->status) & SST_VRETRACE) ^ (_FF(ddMiscFlags) & DDMF_VSYNC_POLARITY_MASK))
         ;
   
      // if in active time then wait for start of vtrace
      while (!((GET(lph3IORegs->status) & SST_VRETRACE) ^ (_FF(ddMiscFlags) & DDMF_VSYNC_POLARITY_MASK)))
         ;

      // should be start of vtrace
      }

  // Download DAC data ("C" Code).
#ifndef C_CODE
  LoadPalette((DWORD)&lph3IORegs->dacAddr, &color[start], FlatSel, start, start+count);
#else
  for (index = start; index < (start + count); index++)
  {
    // Write DAC address.
    SETDW(lph3IORegs->dacAddr, index);

    // Read DAC address, to avoid bursting.
    data = GET(lph3IORegs->dacAddr);

    // Write DAC data.
    SETDW(lph3IORegs->dacData, color[index]);

    // Read DAC data, to avoid bursting.
    data = GET(lph3IORegs->dacData);
  }
#endif

#ifdef CHECK_PALETTE
   if (CheckPalette)
      {
      for (index = start; index < (start+count); index++)
         {
         // Write DAC address.
         SETDW(lph3IORegs->dacAddr, index);

         // Read DAC address, to avoid bursting.
         data = GET(lph3IORegs->dacAddr);

         // Display DAC data.
         // Read DAC data
         // On V3 there is a hand shaking problem between the
         // PCI clocks and the Video Clocks 
         for (j=0; j<MAX_RETRY; j++)
            {
            data = GET(lph3IORegs->dacData);
            if (data == color[index])
               break;
            }

       if (data != color[index])
            {
            DPF(DBGLVL_NORMAL, "dac index %x, got data = 0x%08lx expected = 0x%08lx", index, data, color[index]);
            _asm {int 03};
            }
         }
      }
#endif

#ifdef DUMPDAC
  for (index = 0; index < 256; index++)
  {
    // Write DAC address.
    SETDW(lph3IORegs->dacAddr, index);

    // Read DAC address, to avoid bursting.
    data = GET(lph3IORegs->dacAddr);

    // Display DAC data.
    DPF(DBGLVL_ALL, "dac index %x, data = 0x%08lx", index, GET(lph3IORegs->dacData));
  }
#endif
      
  return TRUE;
}

#pragma warning (default: 4704)
