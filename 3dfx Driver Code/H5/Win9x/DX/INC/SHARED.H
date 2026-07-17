/* $Header: shared.h, 35, 10/11/00 8:53:13 PM, Brent$ */
/*
** Copyright (c) 1997-1999, 3Dfx Interactive, Inc.
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
** File name:   shared.h
**
** Description: GLOBALDATA structure and supporting stuff.
**
** $Revision: 35$
** $Date: 10/11/00 8:53:13 PM$
**
** $History: shared.h $
** 
** *****************  Version 1  *****************
** User: Skephart     Date: 11/12/99   Time: 4:27p
** Created in $/DEVEL/h5/Win9x/DX/inc
** 
** *****************  Version 23  *****************
** User: Xingc        Date: 9/03/99    Time: 10:34a
** Updated in $/devel/h5/Win9x/dx/inc
** move definitions of PFNVMIREAD and PFNVMIWRITE into vmipld.h, delete
** pfnVMIPLDRead and pfnVMIPLDWrite in GLOBALDATA structure.
** 
** *****************  Version 22  *****************
** User: Xingc        Date: 8/30/99    Time: 2:03p
** Updated in $/devel/h5/Win9x/dx/inc
** Add DosActive and dwRelaxedOverlayOwnerMode for full BOS Box overlay
** issues.
** 
** *****************  Version 21  *****************
** User: Lpost        Date: 8/25/99    Time: 2:13p
** Updated in $/devel/h5/Win9x/dx/inc
** V3TV code merge into H5
** 
** *****************  Version 20  *****************
** User: Andrew       Date: 8/23/99    Time: 6:58p
** Updated in $/devel/h5/Win9x/dx/inc
** Added new members to fix Hostbase cursor for flip code
** 
** *****************  Version 19  *****************
** User: Cwilcox      Date: 8/23/99    Time: 3:19p
** Updated in $/devel/h5/Win9x/dx/inc
** Added secondary buffer heap variables.
** 
** *****************  Version 18  *****************
** User: Rbissell     Date: 8/07/99    Time: 3:10p
** Updated in $/devel/h5/Win9x/dx/inc
** tvout merge from V3_OEM_100
** 
** *****************  Version 17  *****************
** User: Msmith       Date: 8/06/99    Time: 2:19p
** Updated in $/devel/h5/Win9x/dx/inc
** changed srcAddrOff to a DWORD instead of int to fix an alignment issue.
** 
** *****************  Version 16  *****************
** User: Msmith       Date: 7/30/99    Time: 6:15p
** Updated in $/devel/h5/Win9x/dx/inc
** Checked in for Bob J. changes for multi-mon support.
** 
** *****************  Version 15  *****************
** User: Cwilcox      Date: 7/22/99    Time: 9:48a
** Updated in $/devel/h5/Win9x/dx/inc
** Added AAhwPtr, AAlfbPtr, AAlPitch, and AAheapID to FXSURFACEDATA.
** 
** *****************  Version 14  *****************
** User: Cwilcox      Date: 7/19/99    Time: 11:26a
** Updated in $/devel/h5/Win9x/dx/inc
** Added state variables to track new DirectDraw heap offsets.
** 
** *****************  Version 13  *****************
** User: Cwilcox      Date: 7/16/99    Time: 4:31p
** Updated in $/devel/h5/Win9x/dx/inc
** Removed obsolete heap*Start variables.
** 
** *****************  Version 12  *****************
** User: Andrew       Date: 7/16/99    Time: 1:03p
** Updated in $/devel/h5/Win9x/dx/inc
** changed regBase from single dword to arrary to support sparse mapping
** 
** *****************  Version 11  *****************
** User: Cshaw        Date: 7/13/99    Time: 2:25p
** Updated in $/devel/h5/Win9x/dx/inc
** Added runtime support for Napalm's multitexturing (NAPALM_CU).
** 
** *****************  Version 10  *****************
** User: Xingc        Date: 7/09/99    Time: 5:28p
** Updated in $/devel/h5/Win9x/dx/inc
** Add savriables in BLTPARAM structure and GLOBALDATA struncture to
** support NAPALM overlay shrink using 3D HW.
** 
** *****************  Version 9  *****************
** User: Andrew       Date: 7/09/99    Time: 8:25a
** Updated in $/devel/h5/Win9x/dx/inc
** New member to hold number of master + slaves
** 
** *****************  Version 8  *****************
** User: Xingc        Date: 6/30/99    Time: 5:25p
** Updated in $/devel/h5/Win9x/dx/inc
** Add dwShrinkSurfAdd1 and dwShrinkSurAdd2 for two overlay shrink
** surfaces
** 
** *****************  Version 7  *****************
** User: Dalev        Date: 6/30/99    Time: 4:38p
** Updated in $/devel/h5/Win9x/dx/inc
** Added 'PLDRevisionID' to the 'GLOBALDATA' struc.
** 
** *****************  Version 6  *****************
** User: Andrew       Date: 6/25/99    Time: 4:24p
** Updated in $/devel/h5/Win9x/dx/inc
** Added a Address to call back to Switch the Cursor to a Host Base Cursor
** 
** *****************  Version 5  *****************
** User: Andrew       Date: 6/17/99    Time: 2:37p
** Updated in $/devel/h5/Win9x/dx/inc
** Removed some unused variables
** 
** *****************  Version 4  *****************
** User: Andrew       Date: 6/15/99    Time: 5:17p
** Updated in $/devel/h5/Win9x/dx/inc
** Added dwType to keep track of SLI AA Master/Slave
** 
** *****************  Version 3  *****************
** User: Cwilcox      Date: 6/04/99    Time: 1:35p
** Updated in $/devel/h5/Win9x/dx/inc
** Added flip_pending variable.
** 
** *****************  Version 2  *****************
** User: Edwin        Date: 6/02/99    Time: 2:45p
** Updated in $/devel/h5/Win9x/dx/inc
** Remove ifdef MM, multi-monitor support is always enabled.
** 
** *****************  Version 1  *****************
** User: Michael      Date: 6/02/99    Time: 6:52a
** Created in $/devel/h5/Win9x/dx/inc
** Branch from h3
** 
** *****************  Version 128  *****************
** User: Cwilcox      Date: 5/26/99    Time: 1:31p
** Updated in $/devel/h3/Win95/dx/inc
** Added AGP texture download back into TOT.
** 
** *****************  Version 127  *****************
** User: Cwilcox      Date: 5/24/99    Time: 4:46p
** Updated in $/devel/h3/Win95/dx/inc
** Removed AGP texture download.
** 
** *****************  Version 126  *****************
** User: Stb_lpost    Date: 5/17/99    Time: 1:41p
** Updated in $/devel/h3/win95/dx/inc
** V3TV Video Capture fixes for E3 Demo
** 
** *****************  Version 125  *****************
** User: Andrew       Date: 5/14/99    Time: 1:33p
** Updated in $/devel/h3/Win95/dx/inc
** Added Fake/Real MemBase0,1 to make managing Real/Fake Address easier to
** manage in WIN_SIM
** 
** *****************  Version 124  *****************
** User: Andrew       Date: 5/13/99    Time: 4:12p
** Updated in $/devel/h3/Win95/dx/inc
** Changed Real on lfb to more accurate reflect it's use
** 
** *****************  Version 123  *****************
** User: Andrew       Date: 5/10/99    Time: 1:34p
** Updated in $/devel/h3/Win95/dx/inc
** Added regRealBase and lfbRealBase for Windows "C" Simulator real
** linear/virtual address
** 
** *****************  Version 122  *****************
** User: Cwilcox      Date: 5/03/99    Time: 4:38p
** Updated in $/devel/h3/Win95/dx/inc
** Changes to allow tiled/linear switching.
** 
** *****************  Version 121  *****************
** User: Cwilcox      Date: 5/03/99    Time: 12:25p
** Updated in $/devel/h3/Win95/dx/inc
** Removed obsolete variables.
** 
** *****************  Version 120  *****************
** User: Stb_echilds  Date: 4/28/99    Time: 3:21p
** Updated in $/devel/h3/win95/dx/inc
** Added flag to use for OS Version detection.
** 
** *****************  Version 119  *****************
** User: Cwilcox      Date: 4/26/99    Time: 9:53a
** Updated in $/devel/h3/Win95/dx/inc
** Added heapID to FXSURFACEDATA structure.
** 
** *****************  Version 118  *****************
** User: Cwilcox      Date: 4/21/99    Time: 9:56a
** Updated in $/devel/h3/Win95/dx/inc
** Removed support for FOURCC_VBPL format.
** 
** *****************  Version 117  *****************
** User: Cshaw        Date: 4/20/99    Time: 12:19p
** Updated in $/devel/h3/Win95/dx/inc
** Added ModeUses2PixPerClkRender to GLOBAL struct.
** 
** *****************  Version 116  *****************
** User: Cwilcox      Date: 4/14/99    Time: 1:00p
** Updated in $/devel/h3/Win95/dx/inc
** Removed obsolete rectangular memory manager.
** 
** *****************  Version 115  *****************
** User: Stb_bseitsin Date: 4/09/99    Time: 12:38p
** Updated in $/devel/h3/win95/dx/inc
** Added Napalm registers. Added ifdef H5.
** 
** *****************  Version 114  *****************
** User: Stb_sjohnston Date: 4/07/99    Time: 2:47p
** Updated in $/devel/h3/win95/dx/inc
** Cleaner fix for Slashdot/Wax bug (PRS 4457) from Chris Edgington
** 
** *****************  Version 113  *****************
** User: Andrew       Date: 4/06/99    Time: 12:41a
** Updated in $/devel/h3/Win95/dx/inc
** Added UnGlideContext
** 
** *****************  Version 112  *****************
** User: Andrew       Date: 4/01/99    Time: 11:52a
** Updated in $/devel/h3/Win95/dx/inc
** Changed some shared names 
** 
** *****************  Version 111  *****************
** User: Cwilcox      Date: 3/26/99    Time: 4:23p
** Updated in $/devel/h3/Win95/dx/inc
** Major modification to memory allocation and management, fix PRS #4229.
** 
** *****************  Version 110  *****************
** User: Xingc        Date: 3/22/99    Time: 5:40p
** Updated in $/devel/h3/Win95/dx/inc
** Add pDebFlags in OVLBLTPARMS
** 
** *****************  Version 109  *****************
** User: Xingc        Date: 3/22/99    Time: 2:24p
** Updated in $/devel/h3/Win95/dx/inc
** Add ovlXScale, ovlYScale and ovlCurAddr for TVPCI support
** 
** *****************  Version 108  *****************
** User: Cwilcox      Date: 3/19/99    Time: 10:09a
** Updated in $/devel/h3/Win95/dx/inc
** Removed unused variables in driver data structure.
** 
** *****************  Version 107  *****************
** User: Andrew       Date: 3/17/99    Time: 2:40p
** Updated in $/devel/h3/Win95/dx/inc
** Reverted to old version to remove SWAPHANG workaround
** 
** *****************  Version 105  *****************
** User: Stb_srogers  Date: 3/02/99    Time: 10:21p
** Updated in $/devel/h3/Win95/dx/inc
** Removed "#ifdef INCSTBCUST" so that nightly builds allow OEM mode
** culling
** 
** *****************  Version 104  *****************
** User: Cwilcox      Date: 3/01/99    Time: 1:18p
** Updated in $/devel/h3/Win95/dx/inc
** Fix PRS #4159, disallow locks on stale surfaces.
** 
** *****************  Version 103  *****************
** User: Xingc        Date: 2/25/99    Time: 7:49p
** Updated in $/devel/h3/Win95/dx/inc
** Rename PhysAddress into KMVTBuff for KMVT support
** 
** *****************  Version 102  *****************
** User: Martin       Date: 2/24/99    Time: 12:21p
** Updated in $/devel/h3/Win95/dx/inc
** Workaround an H2inc abomination.  Remove conditional compilation and
** h2inc'ify resultant header file.
** 
** *****************  Version 101  *****************
** User: Martin       Date: 2/20/99    Time: 5:04p
** Updated in $/devel/h3/Win95/dx/inc
** Globals for agp texture downloads
** 
** *****************  Version 100  *****************
** User: Xingc        Date: 2/16/99    Time: 5:45p
** Updated in $/devel/h3/Win95/dx/inc
** Enable KMVP flag for WIN9X only
** 
** *****************  Version 99  *****************
** User: Xingc        Date: 2/16/99    Time: 4:39p
** Updated in $/devel/h3/Win95/dx/inc
** Add OVLBLTPARAMS structure into GLOBALDATA to support overlay shrink in
** KMVT
** 
** *****************  Version 98  *****************
** User: Stb_pzheng   Date: 2/10/99    Time: 3:32p
** Updated in $/devel/h3/win95/dx/inc
** 
** *****************  Version 97  *****************
** User: Stb_srogers  Date: 2/09/99    Time: 9:56a
** Updated in $/devel/h3/win95/dx/inc
** 
** *****************  Version 96  *****************
** User: Ken          Date: 2/08/99    Time: 2:10p
** Updated in $/devel/h3/win95/dx/inc
** added cpu/OS detection of pentium III (katmai) processors, and added
** katmai-optimized d3d texture download
** 
** *****************  Version 95  *****************
** User: Adrians      Date: 1/18/99    Time: 2:06p
** Updated in $/devel/h3/Win95/dx/inc
** Removed obsolete variables and associated code.
** 
** *****************  Version 94  *****************
** User: Michael      Date: 1/12/99    Time: 12:52p
** Updated in $/devel/h3/Win95/dx/inc
** Add the STB customer specific code.  All STB added code is surrounded
** by "#ifdef INCSTBCUST/#endif"
** 
** 93    12/21/98 11:39a Stuartb
** Added dwTvoActive (tvout) and dwLcdActive (flat panel) vars to shared
** data.
** 
** 92    12/18/98 12:34p Cwilcox
** Cleaned up dd3dInOverlay values
** 
** 91    12/14/98 1:47p Agus
** Disable/turn off DVDHACK
** 
** 90    12/11/98 11:12a Cwilcox
** Redo previous checkin.
** 
** 88    12/10/98 12:16p Andrew
** Added a bit to DDMF flags which is the tile bit or bit 31
** 
** 87    12/10/98 12:04p Agus
** Turn off AA_HEAPHACK define, added DVDHACK define
** 
** 86    12/02/98 9:07a Michael
** Add DWORD ClearSawpCount for PRS 3351
** 
** 85    11/24/98 8:31a Andrew
** Added AGPCaps to shared.h
** 
** 84    11/22/98 8:48p Andrew
** Changes to support Multi-Monitor
** 
** 83    11/16/98 8:36p Andrew
** Added some PCI space variable to return Device, Revision, SSID.
** 
** 82    10/29/98 2:13p Martin
** Modification of cmdFifo macros to support future changes for
** super-sampling AA.
** 
** 81    10/16/98 3:25p Michael
** Add default_gamma_flag for the GrammaRamp.
** 
** 80    10/06/98 8:58a Andrew
** Changed the Global Data structure to support the new way we draw
** cursors
** 
** 79    9/23/98 4:56p Agus
** Changed FXCLPLDATA to FXVBPLDATA
** 
** 78    9/21/98 4:01p Edwin
** Label name change: DDMF_8BPP_PRIMARY to DDMF_16BPP_PRIMARY.  This is
** added along with changes to other files so we promote desktop to tile
** mode in 16bpp only.
** 
** 77    9/17/98 4:45p Artg
** added fake vertical blank
** 
** 76    9/10/98 2:41p Stuartb
** TVOUT work in progress.
** 
** 75    9/02/98 12:47p Ken
** when disabling/enabling, restore the mode active at the time of the
** disable, not the current registry setting.  fixes mtm1 title screen,
** PRS #2481
** 
** 74    10/30/98 5:51p Suninn
** add tripple buffer flag in ddMiscFlags
** 
** 73    9/01/98 4:40p Miriam
** New flip code. 
** 
** 71    8/28/98 4:48p Suninn
** add 8BPP flag to ddMiscFlag
** 
** 70    8/26/98 12:04p Adrians
** Fix to enable DX6 to compile.
** 
** 69    8/26/98 12:02p Artg
** added h4 csim support
** 
** 68    8/25/98 3:56p Edwin
** Replace WIN32 flags with WINNT for overlay shrink members so 16 and 32
** bit build of DriverData has same size. Remove selFifo, pad memclk to
** dword.
** 
** 67    8/23/98 1:05a Adrians
** Fix for build after AA SuperSample checkin.
** 
** 65    8/20/98 3:36p Michael
** Add GammaRamp API structure definition.
** 
** 64    8/18/98 12:37p Agus
** Added support for VoodooBanshee CLPL planar YUV where Y,U,V planes have
** fixed 1024 byte pitch.
** 
** 63    8/12/98 6:19p Suninn
** shrink overlay
** 
** 62    8/07/98 4:44p Suninn
** overlay shrink fix for overfly
** 
** 61    7/31/98 10:34p Miriam
** D3D & Glide cooperation. Allow each API to set/reset state to indicate
** that  the HW state has been changed.
** 
** 60    7/30/98 2:02p Ken
** determine memory type at boot (FirstEnable in Enable1) and set a shared
** memory flag in ddMiscFlags to indicate SDRAM or SGRAM
** 
** 59    7/29/98 11:18a Suninn
** add shrink management in surfacedata
** 
** 58    7/24/98 7:17p Miriam
** AGP command fifo only enabled for D3D.
** 
** 57    7/23/98 4:04p Ken
** added agp command fifo.   not added to NT build.  currently not
** functional on non-win98 systems without running a special enable apg
** script first, see Ken for that.   agp command fifo is enabled by
** setting the environment variable ACF=1 , all other settings disable it.
** Only turned on interatively in debugger in InitFifo call.   
** 
** 56    7/18/98 6:41p Ken
** added ability to use cmdfifo1 as the primary command fifo, #define
** PRIMARY_CMDFIFO at the top of inc\shared.h
** 
** 55    7/11/98 8:20a Andrew
** added gamma correction variables
** 
** 54    7/08/98 12:17p Agus
** Added directdraw overlay register field to save all directraw overlay
** registers ' state
** 
** 53    7/02/98 11:02p Edwin
** Add DDMF_ENABLE_DEVICEBITMAPS to fix defect 1733. This flag is used to
** enable and disable device bitmaps support when DDRAW is running.
** 
** 52    6/30/98 4:31p Agus
** Added dwOvlOffset in GLOBALDATA for manual autoflipping usage
** 
** 51    6/30/98 11:19a Suninn
** add ddResetOverfly & related field to fix overfly whql test
** 
** 50    6/29/98 2:31p Edwin
** Fix defect 1882 by adding fpRepaintScreen field.
** 
** 49    6/17/98 11:56p Edwin
** Added fields for rectangular memory manager.
** 
** 48    6/16/98 4:48p Russ
** Added ddMiscFlags to GLOBALDATA
** Added DDMF_VSYNC_POLARITY defines
**
** 47    6/16/98 9:57a Suninn
** add overlayAddr field to surfaceData
**
** 46    6/11/98 5:49p Suninn
** add overlay management fields
**
** 45    6/01/98 1:47p Ken
** added isSdram to globaldata, settable through control1() calls
** (probably should put this in the registry), changed d3d ddiClear to
** look at this to determine whether or not to issue a dithered clear (to
** force block writes on/off)
**
** 44    5/29/98 3:01p Andrew
** Changed HotSpotX & HotSpotY to SHORTs.
**
** 43    5/27/98 8:45a Andrew
** Remved didExclude, move gdiFlags to know position and changed cursor to
** signed ints.
**
** 42    5/22/98 6:01p Suninn
** primary surface data is in shared to avoid memory leak
**
** 41    5/20/98 5:18p Russ
** added DevNode field to GLOBALDATA
**
** 40    5/20/98 2:48p Suninn
** add ddprimarysurfacedata field
**
** 39    5/15/98 4:27p Michael
** Add flat32 pointer to screen's pdevice.
**
** 38    5/15/98 9:49a Suninn
** tile activation field
**
** 37    5/13/98 8:08a Ken
** fast enter/leave turned on, working with SSB.
** also, bumped up the max # of device bitmap allocations by 4x
**
** 36    5/08/98 12:24p Agus
** Added refresh & memclk in shared global
**
** 35    5/01/98 4:25p Artg
** sw cursor stuff
**
** 34    4/30/98 11:26a Ken
** updated enter/leave to always check/set the busy bit on the
** screen's pdevice, not the destination bitmap's pdevice ('cuz it might
** be a device bitmap.
**
** 33    4/29/98 5:58p Artg
** added fields for last packet data info
**
** 32    4/29/98 5:52p Ken
** added lpPDevice32
**
** 31    4/26/98 5:59p Artg
** added extra fields for sw cursor
**
** 30    4/15/98 4:59p Suninn
** move dwordCount into shared.h
**
** 29    4/14/98 11:17a Ken
** added mode12 virtualization
**
** 28    4/02/98 10:51p Artg
** put support for user controlable debug level.
**
** 27    3/31/98 11:41a Miriam
** Global data to debug the command fifo.
*/
/***************************************************************************
 *
 * DriverData
 *
 * this structure contains all the globals of the driver, it is shared
 * between the 16 bit gdi, 32 bit gdi and 32 bit DirectDraw driver.
 *
 ***************************************************************************/
#ifndef __SHARED_H__
#define __SHARED_H__

// NOTE - if you change PRIMARY_CMDFIFO here you MUST change the
// same equate in FIFOMGR.INC as well!!!
#define PRIMARY_CMDFIFO cmdFifo0

#define Number_HotKeys 3		// 4 hotkeys available (we only use 2 at the moment)

#if !defined(WINNT)
#define KMVP    1           //enable KMVP (or KMVT) functions
                            //in 32DD and minivdd
#endif

// STB Begin Changes
// STB-SR 1/13/99 Adding code for bj
#ifdef _H2INC
typedef unsigned char   BYTE;
typedef unsigned short  WORD;
typedef unsigned long   DWORD;
typedef unsigned long   ULONG;
#define LONG            long
#define SHORT           short

typedef unsigned long   FxU32;

typedef struct tagBITMAPINFOHEADER
{
    DWORD   biSize;
    LONG    biWidth;
    LONG    biHeight;
    WORD    biPlanes;
    WORD    biBitCount;
    DWORD   biCompression;
    DWORD   biSizeImage;
    LONG    biXPelsPerMeter;
    LONG    biYPelsPerMeter;
    DWORD   biClrUsed;
    DWORD   biClrImportant;
} BITMAPINFOHEADER;

#endif // #ifdef _H2INC
// STB End Changes

#ifndef WINNT
#include "cpu.h"	/* NEW CPUID CODE */
#endif

#ifndef VDDONLY		/* to end of file */

// STB Begin Changes
#ifndef _H2INC
// STB End Changes

#include "h3regs.h"

// THUNK32 is define for 32 bit gdi specific stuff
#ifdef THUNK32
   #pragma pack( 2 )
#endif

// STB Begin Changes
#endif // ifndef _H2INC
// STB End Changes


// WARNING, WARNING WILL ROBINSON
#define DDMINI_VERSION  0x0100
#define MAX_GDI_FCN 40

// For now we will only write into the main cmdFifo when in 16 bit land.
#ifdef IS_16
#define CMDFIFOBASE           _FF( mainFifo.base          )
#define CMDFIFOSTART          _FF( mainFifo.start         )
#define CMDFIFOEND            _FF( mainFifo.end           )
#define CMDFIFOPTR            _FF( mainFifo.ptr           )
#define CMDFIFOOFFSET         _FF( mainFifo.offset        )
#define CMDFIFOSPACE          _FF( mainFifo.space         )
#define CMDFIFOJMP            _FF( mainFifo.jmp           )
#define CMDFIFOJMP2           _FF( mainFifo.jmp2          )
#define CMDFIFOEPILOGPTR      _FF( mainFifo.epilogPtr     )
#define CMDFIFOUNBUMPEDWORDS  _FF( mainFifo.unbumpedWords )
#define CMDFIFOBUMP           _FF( mainFifo.bump          )
#else
#define CMDFIFOBASE           (DWORD)(((DWORD *)_FF( cmdFifoBasePtr ))[ 0 ])
#define CMDFIFOSTART          (DWORD)(((DWORD *)_FF( cmdFifoBasePtr ))[ 1 ])
#define CMDFIFOEND            (DWORD)(((DWORD *)_FF( cmdFifoBasePtr ))[ 2 ])
#define CMDFIFOPTR            (DWORD)(((DWORD *)_FF( cmdFifoBasePtr ))[ 3 ])
#define CMDFIFOOFFSET         (DWORD)(((DWORD *)_FF( cmdFifoBasePtr ))[ 4 ])
#define CMDFIFOSPACE          (DWORD)(((DWORD *)_FF( cmdFifoBasePtr ))[ 5 ])
#define CMDFIFOJMP            (DWORD)(((DWORD *)_FF( cmdFifoBasePtr ))[ 6 ])
#define CMDFIFOJMP2           (DWORD)(((DWORD *)_FF( cmdFifoBasePtr ))[ 7 ])
#define CMDFIFOEPILOGPTR      (DWORD)(((DWORD *)_FF( cmdFifoBasePtr ))[ 8 ])
#define CMDFIFOUNBUMPEDWORDS  (DWORD)(((DWORD *)_FF( cmdFifoBasePtr ))[ 9 ])
#define CMDFIFOBUMP           (DWORD)(((DWORD *)_FF( cmdFifoBasePtr ))[ 10])
#endif

#define CMDFIFO_BUMPTHRESH    0x100

#ifdef SLI_AA
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

// Be REAL careful about changing this structure!!!!
// When changing this sturcture YOU MUST change the corresponding macros
// for accessing the actual variables.
typedef struct {
  DWORD         base;             // cmdFifoBasePtr[0] frame buffer fifo base
  DWORD         start;            // cmdFifoBasePtr[1] virtual address of fifo base

// STB Begin Changes
#ifndef _H2INC
// STB End Changes

  DWORD         end;              // cmdFifoBasePtr[2]
  DWORD         ptr;              // cmdFifoBasePtr[3] next fifo entry
  DWORD         offset;           // cmdFifoBasePtr[4] (start - base)

// STB Begin Changes
#else 
  DWORD         FXend;            // cmdFifoBasePtr[2]
  DWORD         FXptr;            // cmdFifoBasePtr[3] next fifo entry
  DWORD         FXoffset;         // cmdFifoBasePtr[4] (start - base)
#endif
// STB End Changes

  DWORD         space;            // cmdFifoBasePtr[5]

// STB Begin Changes
#ifndef _H2INC
// STB End Changes

  DWORD         jmp;              // cmdFifoBasePtr[6]

// STB Begin Changes
#else 
  DWORD         FXjmp;            // cmdFifoBasePtr[6]
#endif
// STB End Changes

  DWORD         jmp2;             // cmdFifoBasePtr[7]
  DWORD         epilogPtr;        // cmdFifoBasePtr[8]
  DWORD         unbumpedWords;    // cmdFifoBasePtr[9]
  DWORD         bump;			  // cmdFifoBasePtr[10]
} FXCMDFIFO;

typedef struct {
  // private agp allocation information -- on AGP boards, we can set aside
  // some AGP allocation from the system for our private use in the
  // driver.
  // 
  DWORD					linAddr;      // CPU linear base of agp allocation
  DWORD					physAddr;	    // GART mapping of agpLinAddr
  DWORD					sizeInB;	    // # of Bytes of agp allocation
} FXAGPINFO;
  
typedef struct {
  DWORD         hwPtr;
  DWORD         lfbPtr;
  DWORD         overlayShrinkFlag;
  DWORD         doShrink;
  DWORD         shrinkWidth;
  DWORD         shrinkHeight;
  DWORD         pixelFormat;
  DWORD         inFlipChain;
  DWORD         overlayAddr;
  DWORD         lPitch;
  DWORD         surfaceLevel;
  DWORD         heapID;
/*#ifdef SLI_AA*/
  DWORD         AAhwPtr;
  DWORD         AAlfbPtr;
  DWORD         AAlPitch;
  DWORD         AAheapID;
/*#endif*/
#ifndef WINNT
  DWORD         shrinkSrcWidth;
  DWORD         shrinkSrcHeight;
  DWORD         shrinkSrcX;
  DWORD         shrinkSrcY;
#endif // WINNT
} FXSURFACEDATA;

// defines for ddMiscFlags bit flags

#define DDMF_ENABLE_DEVICEBITMAPS	BIT(1)
#define DDMF_MEMTYPE_SDRAM		BIT(2)
#define DDMF_16BPP_PRIMARY		BIT(3)
#define DDMF_TRIPPLE_BUFFER		BIT(4)
#define DDMF_DRIVER_DISABLED		BIT(5)
#define DDMF_VSYNC_POLARITY_BIT		6
#define DDMF_VSYNC_POLARITY_MASK	BIT(DDMF_VSYNC_POLARITY_BIT)
	

#if ( DIRECTDRAW_VERSION < 0x0600 )
// Gamma ramp API structure definition
typedef struct _DDGAMARAMP {
    WORD        red[256];
    WORD        green[256];
    WORD        blue[256];
} DDGAMMARAMP;

// STB Begin Changes
#ifndef _H2INC
// STB End Changes

#ifndef _WIN32
typedef DDGAMMARAMP FAR * LPDDGAMMARAMP;
#endif

// STB Begin Changes
#endif // ifndef _H2INC
// STB End Changes

#endif

typedef struct _OVLBLTPARAMS {
  DWORD bltDstFormat;
  DWORD clip1min;
  DWORD clip1max;
  DWORD bltSrcFormat;
  DWORD bltSrcSize;
  DWORD bltSrcXY;
  DWORD bltDstSize;
  DWORD bltDstXY;

  DWORD txtrbltDstFormat;  //for texture shrink
  DWORD txtrclip1max;
  DWORD txtrbltDstSize;
  union{
  float max_x;
  DWORD dstWidth;
  };
  union{
  float max_y;
  DWORD dstHeight;
  };
  DWORD dstPitch;
  DWORD tLOD;
  DWORD srcAddrOff;       // 8/6/99 mls changed from int to DWORD to fix alignment problems
  union{
  float max_s;
  DWORD maxs;
  };    
  union{
  float max_t;
  DWORD maxt;
  };    

  WORD  wShrinkFlags;
  WORD  *pDebAddr;
} OVLBLTPARAMS;

//Flags for wShrinkFlags

#define OVL_SHRINK      0x0001  // need shrink overlay surface
#define OVL_BUSY        0x0002  // blt is busy, kernel shrink blt is pending

#define MAX_GLIDE_STATES			256

typedef struct
{
    FxU32   procID;                   // Process ID for Glide 
    FxU32   lostContext;              // Lost Context flag for glide
	FxU32   EntryActive;              // State of this node, is it active or not ( Not needed when 
                                      // linked list is implemented )
	FxU32   NextStateNode;            // Next State Node ( Reserved for linked list implementation )
} FXGLIDESTATEINFO;

typedef struct
{
    // lpPDevice *must* be first (DIBLINK.ASM) uses it.
#ifdef _WIN32
    DWORD               lpPDevice;
    DWORD               *lpDeFlags; // gdi32's flat ptr to lpPDevice
#else

// STB Begin Changes
#ifndef _H2INC
// STB End Changes

    DIBENGINE FAR       *lpPDevice;
    DWORD               *lpDeFlags; // gdi32's flat ptr to lpPDevice

// STB Begin Changes
#else
    DWORD       lpPDevice;
    DWORD       lpDeFlags; // gdi32's flat ptr to lpPDevice
#endif // ifndef _H2INC
// STB End Changes

#endif

    // Move to put at a known location so that it is easy to
    // get at from thunk16.asm. If you move gdiFlags then change
    // thunk16 offset.
    //
    // gdiFlags is a GDI driver entrypoint "fast path" check
    // if gdiFlags is zero, then we need not be concerned with things like
    // restoring invariant 2d registers, worrying about s/w cursors, etc.
    // + DD will set the SDATA_GDIFLAGS_2D_DIRTY flag when it clobbers any of
    //   gdi's invariant 2d registers
    // + GDI will set SDATA_GDIFLAGS_SW_CURSOR if there's a s/w cursor enabled
    //
    DWORD               gdiFlags;       // for gdi fast-path test

    DWORD               dwVersion;
    DWORD               fIsDisplay;

    // screen's flat32 pdevice
    DWORD               lpPDevice32;

    DWORD               HWTotalVRAM;      // total VRAM on host csim card.

    // info about the current mode
    DWORD               ModeNumber;
    DWORD               TotalVRAM;      // total VRAM on card.
    DWORD               ScreenSel;      // selector to screen.
    DWORD               ScreenAddress;  // base addess of screen.
#ifdef KMVP
    DWORD               KMVTBuff;       // buff addess in kmtv.c.
#else
    DWORD               PhysAddress;    // base addess of screen.
#endif
    DWORD               VideoBase;      // base of video memory.
    union
    {
    WORD                memclk;         // memory clock in MHz.
    DWORD               dwFiller1;      // ask compiler to reserve space to
    };                                  // ... align bi on dword boundary

    // bitmapinfo for current mode
    BITMAPINFOHEADER    bi;
    DWORD               color_table[256];


    // mouse cursor stuff
    DWORD               fHardwareCursor;
    DWORD               fMouseTrailsOn;
    SHORT               CursorPosX;
    SHORT               CursorPosY;
    SHORT               LastCursorPosX;
    SHORT               LastCursorPosY;
    WORD                cursorBusy;
    WORD                cursorMissed;
#ifdef SLI_AA
    DWORD               CurrentSurface;
    DWORD               CursorSurface;
    DWORD               FlipCount;
    DWORD               CursorFlipCount;
#endif

    SHORT               HotspotX;
    SHORT               HotspotY;

    DWORD               SWcursorFormat;
    DWORD               SWcursorAndStart;                // Start of And Bitmap
    DWORD               SWcursorXorStart;                // Start of Xor Bitmap
    DWORD               SWcursorSrcStart;                // Start of Src Bitmap
    DWORD               SWcursorExclusionStart;          // SaveUnder stuff
#ifdef SLI_AA
    DWORD               HostcursorAndStart;                // Start of And Bitmap
    DWORD               HostcursorXorStart;                // Start of Xor Bitmap
    DWORD               HostcursorSrcStart;                // Start of Src Bitmap
    DWORD               HostcursorExclusionStart;          // SaveUnder stuff
#endif
    DWORD               SWcursorSize;                    // Size of And, Xor, Dst, Exl Bitmap

    // Device Specific Information
    DWORD VendorDeviceID;              // Device Vendor ID
    DWORD RevisionID;                  // Revision ID
    DWORD PLDRevisionID;
    DWORD SSID;                        // Subsystem ID's
    DWORD AGPCaps;                      // AGP Capabilities
#ifdef SLI_AA
    DWORD dwType;                      // Master or Slave
    DWORD dwSwitchCursor;             // Call at to Promote to Overlay Time to use Host Base Cursor
    DWORD dwSLIMode;                   // State of SLI, Enabled, Disabled, Not in Use
#endif
    DWORD dwNumUnits;                  // Number of Master + Slaves

	DWORD dwDefaultClock;              // Default clock rate as set by BIOS

#ifdef IGXHWC
    DWORD               HWCursorPatAddr;
#endif //IGXHWC
    DWORD               FontCacheAddr;  // should be IGX
    DWORD               FontCacheSize;

    // 2d driver info
    WORD                hres;
    WORD                vres;
    WORD                bpp;
    WORD				        refresh;
    DWORD               pitch;
    DWORD               screenFormat;
    DWORD               PhysOffscreenAddr;   // physical addr of offscreen mem
    DWORD               PhysScreenAddr;      // physical addr of  Display.

    DWORD               FifoAddress;         // physical addr off fifo start
//#if defined(H5)
    DWORD               ModeUses2PixPerClkRender; // This is set at modeset time.
//#endif
    //
    // memory organization
    //

    FxU32               vgaStart;
    FxU32               vgaSize;
    FxU32               fifoStart;
    FxU32               fifoSize;
    FxU32               cursorStart;
    FxU32               cursorSize;
    FxU32               stretchBltStart;
    FxU32               stretchBltSize;

    FxU32               ddLinearHeapStart;
    FxU32               ddLinearHeapSize;
    FxU32               ddSecondaryHeapStart;
    FxU32               ddSecondaryHeapSize;
    FxU32               ddTiledHeapStart;
    FxU32               ddTiledHeapSize;
    FxU32               gdiDesktopStart;
    FxU32               gdiDesktopSize;

    FxU32               ddNumHeap;
    FxU32               ddNumColorBuff;
    FxU32               ddExclusiveMode;
    FxU32               ddPrimaryInTile;
    FxU32               ddTiledHeapActive;

    FxU32               ddLinearHeap1Start;
    FxU32               ddLinearHeap2Start;
    FxU32               ddLinearHeap3Start;
    FxU32               ddLinearHeap4Start;
    FxU32               ddTiledHeap0Start;
    FxU32               ddTiledHeap1Start;
    FxU32               ddTiledHeap2Start;
    FxU32               ddExtraMemorySize;             // Extra memory heap size
    FxU32               ddExtraMemoryHeap;             // Extra memory heap index

    FxU32               secondaryFrontBuffer;
    FxU32               secondaryBackBuffer;
    FxU32               secondaryZBuffer;
    FxU32               secondaryThirdBuffer;

    DWORD               dd3DSurfaceCount;				// Direct3D surface count
    FxU32               ddTileStride;					// Tile width of primary surface
    FxU32               ddTileHeight;                   // Tile height of primary surface
    FxU32               ddTilePitch;					// Tile pitch of primary surface
    DWORD               ddTileMark;						// Divides memory into linear and tiled

    DWORD               lastOverlayAddress;
    DWORD               cmdFifoBasePtr;                 // Point to one of the guys below ...

    DWORD               vidPixelBufThold;          // Holds tweaked value of vidPixelBufThold
    DWORD				flip_pending;

    #define             NUM_AUX_FIFO_BUFS   8
    FXCMDFIFO           mainFifo;                       // main command fifo or
    FXCMDFIFO           auxFifo[ NUM_AUX_FIFO_BUFS ];   // auxilliary fifos for super sampling AA

    //  ClearCommandEx indicates that the CommandEx must be cleared.
    DWORD               ClearCommandEx;

    // Glide lost context array
    FXGLIDESTATEINFO    GlideContext [MAX_GLIDE_STATES];     

    // h/w memory base register bases.  These are host linear addresses
    DWORD               regBase[HWINFO_SST_MAX_BASE_INDEX];      // for direct register writes. h/w membase0
    DWORD               lfbBase;      // for LFBs and cmdfifo writes. h/w membase1
    DWORD               ioBase;       // for init, mode setting, VGA access

#ifdef WIN_CSIM
    DWORD               regRealBase;  // When using Win "C" Simulator this is Real Linear Address of Registers
    DWORD               lfbRealBase;  // When using Win "C" Simulator this is Real Linear Address of lfb
    DWORD               regFakeBase;  // When using Win "C" Simulator this is Fake Linear Address of reg
    DWORD               lfbFakeBase;  // When using Win "C" Simulator this is Fake Linear Address of lfb
#endif

    FXAGPINFO           agpMain;                      // info for main cmd fifo
    FXAGPINFO           agpAux[ NUM_AUX_FIFO_BUFS ];  // info for aux super-sampling AA cmd fifo

    DWORD               doAgpCF;      // dynamically switch between agp cf & video command fifo
    DWORD               enableAGPCF;  // registry key to enable/disable 

    // DD and D3D status & h/w state flags
    // + when glide changes 3D state, D3D needs to resend its state
    // + when D3D changes state, glide command buffer flushes need to
    //      be prefaced with the required state
    DWORD               pD3colbuff;      
    DWORD               pD3context;
    DWORD               pD3auxbuff;
    DWORD               pD3changed;

    DWORD               ddRunTimeVersion;
    DWORD               ddMiscFlags;

    //
    // ddraw, d3d internal, adapter-specific global info.
    //
    DWORD               hSharedHeap;         // Shared Heap Handle
    DWORD               pddglobal;
    DWORD               pfxglobal;
    DWORD               pd3global;

    //
    // If the memory is SDRAM, we can't use block writes.  If isSdram is TRUE,
    // we can't use block writes
    //
    DWORD		isSdram;

    // Memory management global info.
    DWORD               mmEvictAddr;
    DWORD               mmReclaimAddr;
    DWORD               mmFlags;
    DWORD               fpRepaintScreen;    // ptr to GDI RepaintScreen()

    // for messages from thunk32
    DWORD               charbuffer[100];
    DWORD               MsgFlag[MAX_GDI_FCN];
    DWORD               bufferptr16;
    DWORD               MsgFcn;

    DWORD           fakeVerticalBlank;

    // Added to Support Multi-Monitor
// STB Begin Changes
#ifndef _H2INC
// STB End Changes

    SstCRegs  FAR  *lpCRegs;        // This our Logical/Virtual Addresses
    SstGRegs  FAR  *lpGRegs;
    SstIORegs FAR  *lpIOregs;

// STB Begin Changes
#else
    DWORD		  	lpCRegs;        // This our Logical/Virtual Addresses
    DWORD			lpGRegs;
    DWORD			lpIOregs;
#endif
// STB End Changes

    // PRS 4457 - web browser hang - hardware WAX bug 
    DWORD lastY;

    // debug command fifo

    DWORD lastPHHwPtr;
    DWORD lastPHData;
    DWORD currPHHwPtr;
    DWORD currPHData;

    DWORD fifoDwordCount;
    DWORD InPacket;                 // =0 expecting header, =1 not expecting hdr
    DWORD WordsLeftInPacket;        // # of words left in packet, counting header
    DWORD Wrapping;
    DWORD LastPH;                   // Last packet header written
    DWORD LastWR;                   // Last DLL to write (below)
    DWORD CurrPH;                   // Current packet header in progress
    DWORD CurrWR;                   // Curr DLL to write (below)
    #define DX_SIGNATURE 1          // DirectX DLL
    #define DD16_SIGNATURE 2        // Display Driver DLL in driver.drv
    #define TH32_SIGNATURE 3        // Display Driver code in vdd
    // various display driver configuration settings
    //
    DWORD doVgaMode12h;             // allow vga mode 12h in a window?
    DWORD DevNode;  // DevNode returned by VDD_GET_DISPLAY_CONFIG call in Enable()

	DWORD dwOvlOffset;				//IRQ overlay s/w autoflipping usage
    DWORD dwRed;                 // Gamma Values
    DWORD dwGreen;
    DWORD dwBlue;

	DWORD dwVmiTv;               //VMI usage as video-in or tv-out
	DWORD dwTvoActive;           // TV out is selected as display device

#ifndef WINNT
    DWORD  dwCurrentSKSurf;     //Overlay shrink surface
    DWORD  dwShrinkSurfAddr1;   //first overlay shrink surface phys offset
    DWORD  dwShrinkSurfAddr2;   //second overlay shrink surface phys offset
    DWORD  dwTxtrSurfAddr1;    //first overlay shrink texture surface phys offset
#endif
// STB Begin Changes
#ifndef _H2INC
// STB End Changes

    //   Danger Danger Will Robinson.
    //   Do Not put any Data that will be shared with the 32 bit
    //   gdi driver after the Direct Drawstuff
    // DirectDraw stuff
#ifdef THUNK32
    // nothing
#else
    DWORD default_gamma_flag;	// 1 if gamma is "default" and open for change
				// As soon as DX6 sets gamma ramp to non-default
				// this is set to 0
    DDGAMMARAMP gamma_ramp;     // Gamma ramp for 16,24,32 bpp modes
    DDGAMMARAMP ovlgamma_ramp;  // Gamma ramp for overlay

    DWORD                       dwRBitMask;
    DWORD                       dwGBitMask;
    DWORD                       dwBBitMask;
    DWORD                       fReset;
    DDHALDDRAWFNS               HALCallbacks;
    DDHALINFO                   HALInfo;
    DDHAL_DDCALLBACKS           DDCallbacks;
    DDHAL_DDSURFACECALLBACKS    DDSurfaceCallbacks;
    DDHAL_DDPALETTECALLBACKS    DDPaletteCallbacks;
    VIDMEM                      vidMem[1];
#if defined(TnL_HAL)
    DDHAL_DDEXEBUFCALLBACKS     DDExebufCallbacks;
#endif 
#endif
    FXSURFACEDATA       ddPrimarySurfaceData;
    DWORD               ddHasShrinkBuffer;
    FXSURFACEDATA       ddOverlayShrinkSurfaceData;
    DWORD               ddShrinkBufferFree;
    DWORD               ddOverlayShrinkPitch;
    DWORD               ddVisibleOverlaySurf;
    DWORD               ddCurrentSurfaceLevel;

// STB-SR 12/30/98 Adding customer number to the structure
	WORD                customerNumber;

#endif // ifndef _H2INC
// STB End Changes

    DWORD		cpuType;	// cpu type & abilities
#ifdef KMVP
    OVLBLTPARAMS ovlBltParams;   //shrink blt for overlay
#endif
#if !defined(WINNT)
    DWORD       ovlXScale;      //pass to TVPCI
    DWORD       ovlYScale;
    DWORD       ovlCurAddr;
#endif

// STB-EC 4/26/99 Adding flag to set when using Win95 OS Version
    DWORD		win95OS;

#if !defined(WINNT)
   // BAJ 7/27/99 Adding the dwDeviceHandle to decern Win98 Primary or Secondary adapter
   //  This tells us not to call assembly functions in the secondary adapter because of 
   //  assembler's use of non contexted global data memory in the HAL

	DWORD		dwDeviceHandle; // 1 = Primary Display; any other value = secondary adapter
    DWORD      dwTvoStd;          // set to VP_TV_STANDARD_YADDAYADDA

    DWORD ulDevicesThatHaveTheirStatesSaved;  //COMMIT/CANCEL functionality
    DWORD ulSavedStatesOfSavedDevices;        //COMMIT/CANCEL functionality
    DWORD allowPALCRT;                        //allow CRT to be on at same time as PAL TV

	DWORD      DosActive;                     // Flag toggled by ToForeground and ToBackground
	DWORD      dwRelaxedOverlayOwnerMode;     // Toggled to allow re-initialization of flipping chains in DDRAW
#endif
	DWORD dwTvoCapable;          // True if board is capable of handling TvOut

#ifdef STEREO
	DWORD	StereoMode;
	FxU32  	ddStereoHeapFactor;
	DWORD	ddStereoWrapperLoaded;
#endif

	DWORD   dwAuxExclusiveMode;
	DWORD   dwVIACoreLogic;
} GLOBALDATA;

// STB Begin Changes
#ifndef _H2INC
// STB End Changes

#if defined(IS_16) && defined(GBLDATA_IN_PDEV)
// 16bit display driver pdev
typedef struct _PDEV
{
  DIBENGINE   dibEngine;
  GLOBALDATA  DriverData;
} PDEV;
#endif

#ifdef THUNK32
   #pragma pack(  )
#endif

#ifndef IS_32
#ifdef IS_16
#ifdef GBLDATA_IN_PDEV
  // use pointer to access GLOBALDATA struct
  #define _FF(field) (lpDriverData->field)
#else
  // directly access GLOBALDATA struct in data segment
  #define _FF(field) (DriverData.field)
#endif
#else // neither IS_16 or IS_32 is defined
  // 16 bit code munges ptr and must access FX
  #define _FF(field) (lpDriverData->field)
#endif
#endif

#define INVALID_ADDRESS          1

#ifdef IS_32
  #define _FF(field) (ppdev->field)
#endif

// STB Begin Changes
#endif // ifndef _H2INC
// STB End Changes

#endif // ifndef VDDONLY	/* from top of file */

#endif //__SHARED_H__
