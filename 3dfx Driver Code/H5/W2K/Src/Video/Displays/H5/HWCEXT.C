/*
** Copyright (c) 1996, 3Dfx Interactive, Inc.
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
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
**
** $Header: hwcext.c, 33, 10/11/00 8:45:40 PM, Brent$
** $Log: 
**  33   3dfx      1.19.1.0.2.9.1.110/11/00 Brent           Forced check in to
**       enforce branching.
**  32   3dfx      1.19.1.0.2.9.1.007/13/00 Daoxiang Gong   Don't call
**       hwcAllocateGlideStateStructureForProcess if we find the glide state. This
**       fixed 3D pipes problem (PRS 12843)
**  31   3dfx      1.19.1.0.2.906/20/00 Adam Briggs     debugged a debug message
**  30   3dfx      1.19.1.0.2.806/20/00 Russ Lind       added a couple lines of
**       debug output
**  29   3dfx      1.19.1.0.2.706/19/00 Adam Briggs     made tribes work in
**       win2k... basically added a ref. count to the glide context struct so that
**       the glide context would have to be freed once for each time it was alloc'd
**       before the memory gets unmapped
**  28   3dfx      1.19.1.0.2.606/19/00 Daoxiang Gong   Make global variable
**       ulCmdFifoDisabled  to be a member element of PDEV to support multimon.
**  27   3dfx      1.19.1.0.2.506/16/00 Russ Lind       bumped up debug level of
**       register dumps
**  26   3dfx      1.19.1.0.2.405/25/00 Russ Lind       add some minimal error
**       checking to hwcSLIAARequest and correct the
**       use of the EngDeviceIoControl return code
**  25   3dfx      1.19.1.0.2.305/23/00 Adam Briggs     Added HWCEXT_SLI_AA_REQUEST
**       support
**  24   3dfx      1.19.1.0.2.205/11/00 Russ Lind       restore vidPixelBufThold
**       when glide releases exclusive mode
**  23   3dfx      1.19.1.0.2.105/08/00 Russ Lind       report correct numUnits to
**       glide
**  22   3dfx      1.19.1.0.2.005/05/00 StarTeam VTS Administrator Russ Lind ==
**       5/2/00 5:57 PM
**       allocate a GLIDESTATE struct in hwcAllocContext if one doesn't already
**       exist for the current process id.  Fixes issues with OpenGL dct's and
**       Express in WinStone99 not running.
**       removed code that was using STAT_FIFO_IN_USE flag
**  21   3dfx      1.19.1.0    04/26/00 StarTeam VTS Administrator Branch and
**       revert to revision 1.18
**  20   3dfx      1.19        04/26/00 StarTeam VTS Administrator Revert to
**       revision 1.17
**  19   3dfx      1.18        04/26/00 Russ Lind       Rewrite a number of
**       functions to get the current process id and look up the GLIDESTATE struct
**       allocated for the curPID.  Modifications to hwcGetLinearAddr to look up
**       the linear address from the drivers array created by DdMapMemory should
**       fix the ogl screen saver lockup problem (PRS 12844)
**  18   3dfx      1.17        04/12/00 Russ Lind       Attempt to fix PRS 12844,
**       ogl screen savers lock up system.  In hwcExecuteFifo, set the
**       STAT_FIFO_IN_USE bit while the driver updates the driver's cmdfifo with
**       the jump instruction(s) to the glide cmdfifo, then clear the
**       STAT_FIFO_IN_USE bit.  Fix for PRS 13332, d3d trash drawn in glide window.
**        In hwcExecuteFifo, reset the glide hw state when d3d is active and also
**       tell d3d to reset it's hw state when glide is active.
**  17   3dfx      1.16        03/23/00 Russ Lind       port of glide changes from
**       V3_W2K branch, changes are #ifdef'd and currently disabled
**  16   3dfx      1.15        03/08/00 Russ Lind       tell glide/ogl there is
**       only one napalm chip on the board for NT4/W2K
**  15   3dfx      1.14        02/18/00 Russ Lind       added some debug output to
**       hwcPCIOP & hwcGetSlaveRegs
**  14   3dfx      1.13        02/15/00 Russ Lind       HACK to fix PRS 12913, in
**       hwcGetSlaveRegs, we'll attempt to see if
**       req->optData.linearAddrReq.pHandle is still a valid processID.  Glide
**       needs to give us some kind of indication what glide state struct we should
**       use for this request!  But since they don't, we have to resort to crappy
**       hacks like this.  As of the 02-14-00 daily build, it just so happens that
**       the processID is still in the linearAddrReq portion of req
**  13   3dfx      1.12        01/07/00 Don Fowler      Removed code from misc.c
**       that restored the vidprocfg register when glide released exclusive mode.
**  12   3dfx      1.11        01/07/00 Don Fowler       
**  11   3dfx      1.10        01/07/00 Don Fowler       
**  10   3dfx      1.9         01/07/00 Don Fowler      Added the code to restore
**       the desktop to the proper "end of frame buffer" location. Removed the
**       restoration of the vidproccfg register from misc.c because of a bug caused
**       by restoring to a value that did not match the current mode.
**  9    3dfx      1.8         12/22/99 Russ Lind       fix for access violation in
**       debug driver running fifa 2000 on w2k
**       somehow fifa 2000 gets hwcExt to be called with a req->which = 256 and the
**       debug driver then av's by accessing past the end of the extNames array
**  8    3dfx      1.7         11/17/99 Russ Lind       
**       changed all DISPDBG((0 's to DISPDBG((GLIDE_DBG_LEVEL 's
**       defined GLIDE_DBG_LEVEL to 4
**  7    3dfx      1.6         10/12/99 Russ Lind       in hwcGetLinearAddr, use
**       psGlideState rather than glideState[0] when reporting addresses back to
**       glide
**       modified hwcGetSlaveRegs to get slave chip addesses from GLIDESTATE struct
**       rather than reporting kernel mode mappings from DDGLOBAL
**  6    3dfx      1.5         10/11/99 Don Fowler      Changes implemented to move
**       the GLIDE Alt+Tab and Context DWORD sharing code from NT4 OEM tree to
**       current tree.
**  5    3dfx      1.4         10/07/99 Russ Lind       added sli/aa code for
**       hwcPCIOP & hwcGetSlaveRegs
**  4    3dfx      1.3         09/30/99 Russ Lind       temporarily disable sli/aa
**       code in hwcPCIOP & hwcGetSlaveRegs
**  3    3dfx      1.2         09/21/99 Russ Lind       set CMDFIFO_START_OFFSET to
**       0 for NT4
**  2    3dfx      1.1         09/15/99 Russ Lind       deleted old hwcext.c and
**       shared the one from H5TOT winnt
**       added function headers
**       for W2K, fill in tiled memory info in hwcGetDeviceConfig
**       fill in isMaster & numChips in hwcGetDeviceConfig
**       sync up cmdfifo initialization in hwcRlsExclusive with bAssertModeHardware
**       cmdfifo initialization
**       added hwcPCIOP & hwcGetSlaveRegs from win9x (the VDDCalls will need to be
**       replaced when SLI-AA is enabled)
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
**
** 28    9/01/99 11:16a Doconnell
** Move mechanism for checking for O/S support of KNI instructions from
** V3_OEM_100, but change escape number so that it is not in Microsoft
** reserved range.  Also move a few declarations in preparation for moving
** Glide and TvOut changes from V3_OEM_100.
**
** 27    8/30/99 5:47p Russ
** clean up V3/Napalm runtime check in hwcLinearMapOffset (even though
** it's w2k specific)
**
** 26    7/02/99 2:23p Jw
** Fix three warning messages.
**
** 23    5/28/99 1:36p Stb_doconnel
** Another piece of Alt-Tab Changes moved from V3_OEM_100 branch.
**
** 22    5/27/99 6:36p Stb_doconnel
** Add Alt-Tab Changes from NT4 V3_OEM_100 branch
**
** 21    5/19/99 7:17p Jw
** Add AGP command fifo support.  Disabled by default.
**
** 20    4/26/99 2:05p Russ
** temp fix for W2K hang introduced by moving cmdfifo location for glide
** alt-tab on NT4
** clean up compiler warning hwcSetContextDWORD
**
** 19    4/22/99 5:06p Stb_dfowler
**
** 18    4/22/99 3:56p Stb_dfowler
**
** 17    4/21/99 10:11a Russ
** fix compiler warning introduced in previous rev
**
** 16    4/15/99 5:26p Stb_dfowler
**
** 15    1/04/99 12:29p Bob
** Accomodation for Peter Chang's allocate/release context modifications.
**
** 14    12/22/98 3:08p Bob
** Support for Glide in a window (PRS 2596)
**
** Reduced priority of annoying ddraw debugging messages to a manageable
** level
**
** Miniport housekeeping (turned on warning level == 4, and cleaned up
** most of the resulting errors) in preparation to moving to VC 6.0
**
** 13    12/02/98 2:18p Bob
** Added support for fractional gamma tables.
**
** 12    12/02/98 2:05p Bob
** Added support for partial gamma tables.
**
** 11    12/01/98 6:07p Bob
** Fixed bug in copying gamma table for the transient case. This code has
** no callers yet, so is difficult to verify at this time.
**
** 10    12/01/98 5:10p Bob
** Added support to correctly report hardware information to glide. We
** used to report that the device was a 3dfx banshee, rev 3 without regard
** to the actual hardware type.
**
** 9     12/01/98 3:10p Bob
** PRS 2495
** Gamma support for Glide
**
** Added hwcext_gamma_download.
**
** 8     11/24/98 4:59p Jw
** Fix Rlscontext cursor restore not to restore HW pointer shape for SW
** cursor mode.
**
** 7     9/25/98 1:26p Jw
** Fix desktop cursor shape after Glide runs at desktop resolution by
** saving and restoring the cursor shape.
**
** 6     9/16/98 4:38p Russ
** fix for PRS #2395
** in HwcRlsEsclusive, if the driver has already been taken out of
** HWC_EXCLUSIVE mode by a DrvEnableSurface call then just reset the fifo
** otherwise do the normal stuff
**
** 5     8/26/98 3:42a Bob
** Gamma table support.
**
** PRS 2086, 2305
**
** 4     8/11/98 11:44a Jw
** Try to make Glide happy.
**
** 3     8/05/98 3:08p Jw
** Set chip revision for B silicon so Glide will work properly.
**
** 2     6/08/98 9:26p Jw
** Add code and structure to save some state so the desktop isn't trashed
** when returning from a Glide app.
**
** 1     4/29/98 11:04a Jw
** Glide escapes.
**
** 3     4/22/98 5:28p Dow
** Added code for HWCEXT_HWCSETEXLUSIVE and HWCEXT_RLSEXCLUSIVE
**
** 2     4/16/98 10:15p Dow
** Glide/Windows co-op
**
** 1     4/14/98 9:58a Dow
**
*/


#include "precomp.h"

#define GLIDE_DBG_LEVEL     2

#define VALIDATECONTEXT

#if 1
#define DBG_BREAK
#else
#define DBG_BREAK           _asm int 3
#endif

#if ENABLE_RECONFIG_VIDMEM
void reconfigureVideoMemory(PDEV *ppdev);
#endif

// START Alt-Tab Changes
// global pointer to shared non-cached memory.. this is only to be allocated once per driver
// run.. that's why it's global
PVOID pvMiscNonCachedMemory = NULL;
// END Alt-Tab Changes

// DWF
// structure to store the per-process context DWORD information
typedef struct _tag_context_info
{
	ULONG *pulContextDWORD;
//	ULONG ulContextDWORDLFBOffset;
	ULONG ulProcessID;
//	ULONG ulCS;
//	ULONG ulDS;
	PDEV * ppdev;

} HWCEXT_CONTEXT_INFO;

// START Alt-Tab Changes
#define _MAX_CONTEXT_INFO	64
//#ifdef preAltTabChange
//#define _MAX_CONTEXT_INFO	16
//#endif
// ENDT Alt-Tab Changes

// array of callbacks structures
HWCEXT_CONTEXT_INFO sContextInfo[ _MAX_CONTEXT_INFO ] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// total number of glide processes that can be run at a single time
#define _MAX_GLIDE_PROCESSES _MAX_CONTEXT_INFO
GLIDESTATE  glideState[ _MAX_GLIDE_PROCESSES ] = { 0 };     // Structure of register values that need to be restored
                                    // after Glide has run on NT and a counter of the number
                                    // Glide processes running.*WARNING* This structure is not currently 
									// pre-initialized. It is assumed that all memory is 0 upon driver 
									// loading

static char *extNames[] = {
	"HWCEXT_GETDRIVERVERSION",      /* 0x00 */
	"HWCEXT_ALLOCCONTEXT",          /* 0x01 */
	"HWCEXT_GETDEVICECONFIG",       /* 0x02 */
	"HWCEXT_GETLINEARADDR",         /* 0x03 */
	"HWCEXT_ALLOCFIFO",             /* 0x04 */
	"HWCEXT_EXECUTEFIFO",           /* 0x05 */
	"HWCEXT_QUERYCONTEXT",          /* 0x06 */
	"HWCEXT_RELEASECONTEXT",        /* 0x07 */
	"HWCEXT_HWCSETEXCLUSIVE",       /* 0x08 */
	"HWCEXT_HWCRLSEXCLUSIVE",       /* 0x09 */
	"HWCEXT_I2C_WRITE_REQ",         /* 0x0A */
	"HWCEXT_I2C_READ_REQ",          /* 0x0B */
	"HWCEXT_I2C_READ_RES",          /* 0x0C */
	"HWCEXT_I2C_MULTI_WRITE_REQ",   /* 0x0D */
	"HWCEXT_GETAGPINFO",            /* 0x0E */
	"HWCEXT_VIDTIMING",             /* 0x0F */
	"HWCEXT_FIFOINFO",              /* 0x10 */
	"HWCEXT_LINEAR_MAP_OFFSET",     /* 0x11 */
	"HWCEXT_GAMMA_DOWNLOAD",        /* 0x12 */
	"HWCEXT_RESTORE_DESKTOP",       /* 0x13 */
	"HWCEXT_QUERYCONTEXT",          /* 0x14 */
	"HWCEXT_SHARE_CONTEXT_DWORD",   /* 0x15 */
	"HWCEXT_UNMAP_MEMORY",          /* 0x16 */
	"HWCEXT_CONTEXT_DWORD_NT",      /* 0x17 */
  "HWCEXT_PCI_OP",                /* 0x18 */
  "HWCEXT_GET_SLAVE_REGS",        /* 0x19 */
//#ifdef ENABLE_V3_W2K_GLIDE_CHANGES
  "HWCEXT_PROTECT_CMD_FIFO",      /* 0x1A */
//#endif
  "HWCEXT_SLI_AA_REQUEST"         /* 0x1B */
};

BOOL HWSetPalette ( PDEV *, int, int, PVIDEO_CLUTDATA, GAMMA_STATE );

/*----------------------------------------------------------------------
Function name:  getCurrentProcessId

Description:    calls the miniport to get the current processID

Return:         curPID or -1
----------------------------------------------------------------------*/
HANDLE
getCurrentProcessId(PDEV *ppdev)
{
  ULONG   numBytes;
  HANDLE  curPID;


  if (EngDeviceIoControl(ppdev->hDriver,
                         IOCTL_3DFX_GET_CURRENT_PROCESS_ID,
                         NULL,
                         0,
                         &curPID,
                         sizeof(curPID),
                         &numBytes))
  {
    DISPDBG((GLIDE_DBG_LEVEL, "  GET_CURRENT_PROCESS_ID failed, returning -1"));
    return (HANDLE)-1;
  }

  DISPDBG((GLIDE_DBG_LEVEL, "  GET_CURRENT_PROCESS_ID returned %8lXh", curPID));
  return curPID;
}

/*-------------------------------------------------------------------
Function Name:  SetGlideStateFlags

Description:    find any GLIDESTATE created under this ppdev and
                set the glideGDIFlags with state

Return:         nothing
-------------------------------------------------------------------*/
void
SetGlideStateFlags(PDEV *ppdev, LONG state)
{
  ULONG   i;


  DISPDBG((GLIDE_DBG_LEVEL, "SetGlideStateFlags"));
  ASSERTDD((GLDATA_GDIFLAGS_PPDEV_DESTROYED == state) ||
           (GLDATA_GDIFLAGS_PPDEV_DISABLED  == state),
           "SetGlideStateFlags passed an invalid state");

  for (i = 0; i < _MAX_GLIDE_PROCESSES; i++)
  {
    if (glideState[i].ppdev == ppdev)
    {
      glideState[i].glideGDIFlags |= state;

      DISPDBG((GLIDE_DBG_LEVEL,"  GLIDESTATE=%8lXh (for ppdev %8lXh), set gdiFlags to %8lXh",
               &glideState[i], ppdev, glideState[i].glideGDIFlags));

      // this is causing an access violation
      // set the contextDWORD
      //*((ULONG *)pvMiscNonCachedMemory + glideState[i].contextDWORDNTIndex) = TRUE;
    }
  }
}

/*-------------------------------------------------------------------
Function Name:  ClearGlideStateFlags

Description:    find any GLIDESTATE created under this ppdev and
                clear the glideGDIFlags with state

Return:         nothing
-------------------------------------------------------------------*/
void
ClearGlideStateFlags(PDEV *ppdev, LONG state)
{
  ULONG   i;


  DISPDBG((GLIDE_DBG_LEVEL, "ClearGlideStateFlags"));
  ASSERTDD((GLDATA_GDIFLAGS_PPDEV_DESTROYED == state) ||
           (GLDATA_GDIFLAGS_PPDEV_DISABLED  == state),
           "SetGlideStateFlags passed an invalid state");

  for (i = 0; i < _MAX_GLIDE_PROCESSES; i++)
  {
    if (glideState[i].ppdev == ppdev)
    {
      glideState[i].glideGDIFlags &= ~state;

      DISPDBG((GLIDE_DBG_LEVEL,"  GLIDESTATE=%8lXh (for ppdev %8lXh), set gdiFlags to %8lXh",
               &glideState[i], ppdev, glideState[i].glideGDIFlags));

      // this is causing an access violation
      // set the contextDWORD
      //*((ULONG *)pvMiscNonCachedMemory + glideState[i].contextDWORDNTIndex) = TRUE;
    }
  }
}

/*-------------------------------------------------------------------
Function Name:  hwcGetGlideStateStructureForProcess

Description:    

Return:         
-------------------------------------------------------------------*/
GLIDESTATE*
hwcGetGlideStateStructureForProcess(HANDLE hProcess)
{
  ULONG dwIndex;


  DISPDBG((GLIDE_DBG_LEVEL, "hwcGetGlideStateStructureForProcess"));

  // search through all of the glide state structures to find the one with a matching
  // process handle
  for (dwIndex = 0; dwIndex < _MAX_GLIDE_PROCESSES; dwIndex++)
  {
    // check to see if the process handle is defined for the current structure
    // and if it is the same as the current process 
    if ((glideState[dwIndex].glideProcessHandle != NULL) &&
        (glideState[dwIndex].glideProcessHandle == hProcess))
    {
      DISPDBG((GLIDE_DBG_LEVEL, "  found GLIDESTATE=%8lXh (index=%ld) for PID=%8lXh",
               &glideState[dwIndex], dwIndex, hProcess));

      // process handle found... return a pointer to the current GLIDE process structure
      return( &glideState[ dwIndex ] );
    }
  }

  DISPDBG((GLIDE_DBG_LEVEL, "  no GLIDESTATE found for PID=%8lXh", hProcess));

  // no structure found for the current GLIDE process
  return NULL;
}

#ifdef ENABLE_V3_W2K_GLIDE_CHANGES
/*-------------------------------------------------------------------

Function Name:  hwcGetGlideStateStructureForLFBMapping

Description:    

Return:         
-------------------------------------------------------------------*/

GLIDESTATE * hwcGetGlideStateStructureForLFBMapping( PDEV * ppdev,PVOID pvLFBMapping )
{
	ULONG dwIndex;
	DISPDBG((GLIDE_DBG_LEVEL, "hwcGetGlideStateStructureForLFBMapping"));

	// search through all of the glide state structures to find the one with a command fifo
	// in the current mapping 
	for( dwIndex = 0x00; dwIndex < _MAX_GLIDE_PROCESSES; dwIndex++ )
	{
		// verify that the glide state structure is valid
		if( glideState[ dwIndex ].glideProcessHandle != NULL )
		{
			// check to see if the surface pointer for the current GLIDE process is 
			// in the range between the passed pointer and the size of the LFB
			// and if it is the same as the current process... only test against the first
			// surface mapping because all should be in the same range
			if( ( glideState[ dwIndex ].dwProtectCmdFifo ) && ( glideState[ dwIndex ].pvCommandFifoLinearAddress[ 0 ] >= pvLFBMapping ) && 
				( glideState[ dwIndex ].pvCommandFifoLinearAddress[ 0 ] < ( PVOID )( ( BYTE * )pvLFBMapping + 
				( ppdev->ulScreenOffset & 0x7fffffff ) ) ) )
			{
				DISPDBG((GLIDE_DBG_LEVEL, "GLIDE State Structure Found"));

				// mapping found... return a pointer to the current GLIDE process structure
				return( &glideState[ dwIndex ] );
			}
		}
	}

	DISPDBG((GLIDE_DBG_LEVEL, "NO GLIDE State Structure Found For Mapping!"));

	// no structure found for the current GLIDE process
	return( NULL );
}
#endif

/*-------------------------------------------------------------------
Function Name:  hwcAllocateGlideStateStructureForProcess

Description:    

Return:         
-------------------------------------------------------------------*/
GLIDESTATE*
hwcAllocateGlideStateStructureForProcess(HANDLE hProcess)
{
  ULONG dwIndex;


  DISPDBG((GLIDE_DBG_LEVEL, "hwcAllocateGlideStateStructureForProcess"));

  /* search for a duplicate mapping */
  for (dwIndex = 0; dwIndex < _MAX_GLIDE_PROCESSES; dwIndex++)
  {
    /* if a mapping already exists, inc the reference count */
    if (glideState[dwIndex].glideProcessHandle == hProcess)
    {
      ++glideState[dwIndex].glideReferenceCount ;
      return (&glideState[dwIndex]) ;
    }
  }


  // search through the glide state structures until an empty slot is found
  for (dwIndex = 0; dwIndex < _MAX_GLIDE_PROCESSES; dwIndex++)
  {
    // check to see if the current glide process structure is undefined
    if (glideState[dwIndex].glideProcessHandle == NULL)
    {	
      // reset the structure to NULL
      memset(&glideState[dwIndex], 0, sizeof(GLIDESTATE));

      // store the process handle for the current process
      glideState[dwIndex].glideProcessHandle = hProcess;

      glideState[dwIndex].glideReferenceCount = 1 ;

      DISPDBG((GLIDE_DBG_LEVEL, "  allocated GLIDESTATE=%8lXh (index=%ld) for PID=%8lXh",
               &glideState[dwIndex], dwIndex, hProcess));

      // return the address of the glide context structure
      return (&glideState[dwIndex]);
    }
  }

  DISPDBG((GLIDE_DBG_LEVEL, "  unable to allocate GLIDESTATE"));

  // no open structures found.. return NULL
  return NULL;
}

/*-------------------------------------------------------------------
Function Name:  hwcReleaseGlideStateStructureForProcess

Description:    

Return:         
-------------------------------------------------------------------*/
VOID
hwcReleaseGlideStateStructureForProcess(HANDLE hProcess)
{
  ULONG dwIndex;


  DISPDBG((GLIDE_DBG_LEVEL, "hwcReleaseGlideStateStructureForProcess"));

  // search through all of the structure until the currently owned structure is found
  for (dwIndex = 0; dwIndex < _MAX_GLIDE_PROCESSES; dwIndex++)
  {
    // check to see if the current process handle matches the one to be release, if so then
    // release the structure by making the process handle 0
    if (glideState[dwIndex].glideProcessHandle == hProcess)
    {
      if (--glideState[dwIndex].glideReferenceCount == 0)
      {
        // reset the structure to NULL
        memset(&glideState[dwIndex], 0, sizeof(GLIDESTATE));

        DISPDBG((GLIDE_DBG_LEVEL, "  releasing GLIDESTATE=%8lXh (index=%ld) for PID=%8lXh",
                 &glideState[dwIndex], dwIndex, hProcess));
      }

      return;
    }
  }

  DISPDBG((GLIDE_DBG_LEVEL, "  no GLIDESTATE found for PID=%8lXh", hProcess));

  // no glide structure for current process found....ERROR
  return;
}

/*-------------------------------------------------------------------
Function Name:  CheckForGlideCmdfifoSurface

Description:    

Return:         
-------------------------------------------------------------------*/
void
CheckForGlideCmdfifoSurface(PDEV *ppdev, DWORD surfStart, DWORD surfEnd)
{
  ULONG   i;


  DISPDBG((GLIDE_DBG_LEVEL, "CheckForGlideCmdfifoSurface"));

  for (i = 0; i < _MAX_GLIDE_PROCESSES; i++)
  {
    if ((glideState[i].ppdev == ppdev) && (HWCEXT_FIFO_FB == glideState[i].fifoType))
    {
      // check if this surface address range contains the last cmdfifo used
      // for this glideState
      if (((glideState[i].lastFifoOffset >= surfStart) &&
           ((glideState[i].lastFifoOffset + glideState[i].lastFifoSize) <= surfEnd)) ||
          ((glideState[i].lastStateOffset >= surfStart) &&
           ((glideState[i].lastStateOffset + glideState[i].lastStateSize) <= surfEnd)))
      {
        glideState[i].glideGDIFlags |= GLDATA_GDIFLAGS_CMDFIFO_SURFACE_LOST;

        DISPDBG((GLIDE_DBG_LEVEL,"  GLIDESTATE=%8lXh (for ppdev %8lXh), cmdfifo surface (%8lXh) being destroyed",
                 &glideState[i], ppdev, surfStart));
  
        // set the contextDWORD
        *((ULONG *)pvMiscNonCachedMemory + glideState[i].contextDWORDNTIndex) = TRUE;

        // we better not have overlapping surfaces!
        // so if we found one glide cmdfifo surface we are done
        break;
      }
    }
  }
}

/*----------------------------------------------------------------------
Function name:  hwcGetDriverVersion

Description:    Obtain the version number of the driver.

Information:

Return:         LONG    1L always returned
----------------------------------------------------------------------*/
static LONG
hwcGetDriverVersion(hwcExtRequest_t *req, hwcExtResult_t *res)
{
  res->optData.driverVersionRes.major = 0xdead;
  res->optData.driverVersionRes.minor = 0xcafe;

  res->resStatus = 1;

  return res->resStatus;

} /* hwcGetDriverVersion */

/*----------------------------------------------------------------------
Function name:  hwcAllocContext

Description:    Obtain the Allocation Context.

Information:

Return:         LONG     1L for success or,
                        -1L for failure.
----------------------------------------------------------------------*/
static LONG
hwcAllocContext(hwcExtRequest_t *req, hwcExtResult_t *res, PDEV *ppdev)
{
  //static contextID;
  HANDLE      curPID;
  GLIDESTATE  *pGS;


  curPID = getCurrentProcessId(ppdev);
  if ((HANDLE)-1 == curPID)
  {
    DISPDBG((0, "  hwcAllocContext - getCurrentProcessId failed"));
    DBG_BREAK;
    res->resStatus = -1;
    return res->resStatus;
  }

  // allocate a GLIDE state structure for the current process
  pGS = hwcAllocateGlideStateStructureForProcess(curPID);
  if (NULL == pGS)
  {
    DISPDBG((0, "  hwcAllocContext - unable to allocate a GLIDESTATE for curPID=%8lXh", curPID));
    DBG_BREAK;
    res->resStatus = -1;
    return res->resStatus;
  }

  // save ppdev in GLIDESTATE
  pGS->ppdev = ppdev;

  if (req->optData.allocContextReq.protocolRev != HWCEXT_PROTOCOLREV) {
    res->resStatus = -1;
    return res->resStatus;
  }

  //res->optData.allocContextRes.contextID = ++contextID;

  // if glide bothered to fill in the correct req->contextID for ALL
  // requests, then this would be helpful, but glide doesn't fill in
  // the correct req->contextID for ALL (yes that is ALL) requests so
  // returning the context as the address of the GLIDESTATE structure
  // is somewhat of useless exercise
  res->optData.allocContextRes.contextID = (FxU32)pGS;
  DISPDBG((GLIDE_DBG_LEVEL, "  contextID = %8lXh", res->optData.allocContextRes.contextID));
  res->resStatus = 1;
  return 1;

} /* hwcAllocContext */

/*----------------------------------------------------------------------
Function name:  hwcGetDeviceConfig

Description:    Obtain information about the display device.

Information:

Return:         LONG     1L is always returned.
----------------------------------------------------------------------*/
static LONG
hwcGetDeviceConfig(hwcExtRequest_t *req, hwcExtResult_t *res, PDEV *ppdev)
{

  VALIDATECONTEXT;

  res->optData.deviceConfigRes.devNum		= 0;
  res->optData.deviceConfigRes.vendorID		= (ULONG) ppdev->usVendorID;
  res->optData.deviceConfigRes.deviceID		= (ULONG) ppdev->usDeviceID;
  res->optData.deviceConfigRes.fbRam		= ppdev->cjBank;   // VRam Size
  res->optData.deviceConfigRes.chipRev		= ppdev->usChipRev;
#if (_WIN32_WINNT >= 0x0500) && ENABLE_RECONFIG_VIDMEM
  res->optData.deviceConfigRes.pciStride = _FF(ddTilePitch);
  res->optData.deviceConfigRes.hwStride  = _FF(ddTileStride);
  res->optData.deviceConfigRes.tileMark  = _FF(ddTileMark);
#else
  // NT4 doesn't have tiled memory support
  res->optData.deviceConfigRes.pciStride	= 1024;
  res->optData.deviceConfigRes.hwStride		= 1024;
  res->optData.deviceConfigRes.tileMark		= 16 * 1024 * 1024;	// All linear
#endif

  // New for SLI-AA
#ifdef SLI_AA
  //res->optData.deviceConfigRes.isMaster = (_FF(dwType) != SLI_AA_SLAVE_DEVICE) ? TRUE : FALSE;
  res->optData.deviceConfigRes.isMaster = TRUE;
#if 0
  res->optData.deviceConfigRes.numChips = 1;
#else
  res->optData.deviceConfigRes.numChips = _FF(dwNumUnits);
#endif
#else
  res->optData.deviceConfigRes.isMaster = TRUE;
  res->optData.deviceConfigRes.numChips = 1;
#endif

  res->resStatus = 1;

  return res->resStatus;

} /* hwcGetDeviceConfig */

/*----------------------------------------------------------------------
Function name:  hwcGetLinearAddr

Description:    Obtain the Linear Address.

Information:

Return:         LONG     1L is always returned.
----------------------------------------------------------------------*/
LONG
hwcGetLinearAddr(hwcExtRequest_t *req, hwcExtResult_t *res, PDEV *ppdev)
{
  HANDLE      curPID;
  GLIDESTATE  *pGS;


  VALIDATECONTEXT;

  // For NT we need to map the frame-buffer and register space in.
  DISPDBG((GLIDE_DBG_LEVEL, "hwcGetLinearAddr:  devNum=%8lXh, handle=%8lXh",
           req->optData.linearAddrReq.devNum,
           req->optData.linearAddrReq.pHandle));

  curPID = getCurrentProcessId(ppdev);
  if ((HANDLE)-1 == curPID)
  {
    DISPDBG((0, "  hwcGetLinearAddr - getCurrentProcessId failed"));
    DBG_BREAK;
    res->resStatus = 0;
    return 0;
  }
#if DBG
  if (curPID == (HANDLE)req->optData.linearAddrReq.pHandle)
    DISPDBG((GLIDE_DBG_LEVEL, "  PID passed by glide matches PID found by miniport.  (PID=%8lXh)", curPID));
  else
    DISPDBG((GLIDE_DBG_LEVEL, "  PID passed by glide (%8lXh) doesn't match PID found by miniport (%8lXh)!!!",
             req->optData.linearAddrReq.pHandle, curPID));
#endif

  pGS = hwcGetGlideStateStructureForProcess(curPID);
  if(pGS != NULL)
  {
    /* if a glide state structure already exists, return the old mapping */
    //pGS = hwcAllocateGlideStateStructureForProcess(curPID);

    res->optData.linearAddressRes.numBaseAddrs = 3;
    res->optData.linearAddressRes.baseAddresses[0] = (FxU32) pGS->glideRegBase;
    res->optData.linearAddressRes.baseAddresses[1] = (FxU32) pGS->glideScreenBase;
    res->optData.linearAddressRes.baseAddresses[2] = (FxU32) pGS->glideIOBase;

    res->resStatus = 1;
  }
   else
  {
    // allocate a GLIDE state structure for the current process
    pGS = hwcAllocateGlideStateStructureForProcess(curPID);
    if (NULL == pGS)
    {
      DISPDBG((0, "  hwcGetLinearAddr - unable to allocate a GLIDESTATE for curPID=%8lXh", curPID));
      DBG_BREAK;
      res->resStatus = 0;
      return res->resStatus;
    }

    // save ppdev in GLIDESTATE
    pGS->ppdev = ppdev;

    if (miscGlideMapMemoryBases(ppdev, curPID))
    {
      res->optData.linearAddressRes.numBaseAddrs = 3;
      res->optData.linearAddressRes.baseAddresses[0] = (FxU32) pGS->glideRegBase;
      res->optData.linearAddressRes.baseAddresses[1] = (FxU32) pGS->glideScreenBase;
      res->optData.linearAddressRes.baseAddresses[2] = (FxU32) pGS->glideIOBase;

      res->resStatus = 1;
    }
    else
    {
      res->resStatus = 0;
    }
  }
  //miscSaveExtState( ppdev );

  return res->resStatus;

} /* hwcGetLinearAddr */

/*----------------------------------------------------------------------
Function name:  hwcAllocFifo

Description:    Allocate the command FIFO

Information:

Return:         LONG    1 for success or,
                        0 for failure.
----------------------------------------------------------------------*/
static LONG
hwcAllocFifo(hwcExtRequest_t *req, hwcExtResult_t *res)
{
  return 0;
} /* hwcAllocFifo */

/*----------------------------------------------------------------------
Function name:  hwcExecuteFifo

Description:    Service the Execute FIFO requests.

Information:

Return:         LONG    1 for success or,
                        0 for failure.
----------------------------------------------------------------------*/
static LONG
hwcExecuteFifo(hwcExtRequest_t *req, hwcExtResult_t *res, PDEV *ppdev)
{
  BYTE        *pjH3Base = ppdev->pjH3Base;
  LONG        retVal = 1;
#ifdef H3_FIFO
  HANDLE      curPID;
  GLIDESTATE  *pGS;

  CMDFIFO_PROLOG(hwPtr);


// START Alt-Tab Changes
  if( ppdev->ulCmdFifoDisabled )
  {
    hwcSetContextDWORD();
    res->resStatus = FALSE;
    CMDFIFO_EPILOG(hwPtr);
    return res->resStatus ;
  }

  curPID = getCurrentProcessId(ppdev);
  if ((HANDLE)-1 == curPID)
  {
    DISPDBG((0, "  hwcExecuteFifo - getCurrentProcessId failed"));
    DBG_BREAK;
    res->resStatus = 0;
    CMDFIFO_EPILOG(hwPtr);
    return 0;
  }
#if DBG && ENABLE_V3_W2K_GLIDE_CHANGES
  if (curPID == (HANDLE)req->optData.executeFifoReq.procId)
    DISPDBG((GLIDE_DBG_LEVEL, "  PID passed by glide matches PID found by miniport.  (PID=%8lXh)", curPID));
  else
    DISPDBG((GLIDE_DBG_LEVEL, "  PID passed by glide (%8lXh) doesn't match PID found by miniport (%8lXh)!!!",
             req->optData.executeFifoReq.procId, curPID));
#endif
  pGS = hwcGetGlideStateStructureForProcess(curPID);
  if (NULL == pGS)
  {
    DISPDBG((0, "  hwcExecuteFifo - getGlideContext failed"));
    DBG_BREAK;
    res->resStatus = 0;
    CMDFIFO_EPILOG(hwPtr);
    return 0;
  }
#if DBG
  if (req->contextID != (FxU32)pGS)
    DISPDBG((GLIDE_DBG_LEVEL, "  context passed in by glide (%8lXh) not the same as pGS (%8lXh)",
             req->contextID, pGS));
#endif
  if (! (pGS->glideGDIFlags & GLDATA_GDIFLAGS_HWC_EXCLUSIVE))
  {
    if (ppdev != pGS->ppdev)
    {
      DISPDBG((0, "  hwcExecuteFifo - wrong ppdev"));
      DBG_BREAK;
      res->resStatus = 0;
      CMDFIFO_EPILOG(hwPtr);
      return 0;
    }
#if 0
    if (pGS->glideGDIFlags & (GLDATA_GDIFLAGS_PPDEV_DESTROYED | GLDATA_GDIFLAGS_PPDEV_DISABLED))
    {
      DISPDBG((0, "  hwcExecuteFifo - ppdev destroyed or disabled (GDIFlags=%8lXh)", pGS->glideGDIFlags));
      DBG_BREAK;
      res->resStatus = 0;
      CMDFIFO_EPILOG(hwPtr);
      return 0;
    }
#endif
  }
  if (pGS->glideGDIFlags & GLDATA_GDIFLAGS_CMDFIFO_SURFACE_LOST)
  {
    DISPDBG((0, "  hwcExecuteFifo - cmdfifo surface was destroyed (GDIFlags=%8lXh)", pGS->glideGDIFlags));
#if 1
    DISPDBG((0, "                   we need some method of detecting when glide recreates their cmdfifo surface"));
    DBG_BREAK;
    // clear this flag and continue for now
    pGS->glideGDIFlags &= ~GLDATA_GDIFLAGS_CMDFIFO_SURFACE_LOST;
#else
    DBG_BREAK;
    res->resStatus = 0;
    CMDFIFO_EPILOG(hwPtr);
    return 0;
#endif
  }

#ifdef ENABLE_V3_W2K_GLIDE_CHANGES
#if 0 
  // get the current process structure to see if we need to lock out this FIFO execution
  if (NULL != pGS)
  {
    // check to see if the current glide process structure is undefined
    if (pGS->dwUnMapMemoryLFB)
    {	
      switch (req->optData.executeFifoReq.fifoType)
      {
#if (_WIN32_WINNT >= 0x0500) && defined (AGP_CMDFIFO)
        case HWCEXT_FIFO_AGP:
#endif
        case HWCEXT_FIFO_FB:
        {
          P6_FENCE;

          CMDFIFO_CHECKROOM(hwPtr, 2);
          /* Write the serial # for this fifo execution. */
          SETPH(hwPtr, (SSTCP_PKT5_LFB |
                          (0x00UL << SSTCP_PKT5_BYTEN_W2_SHIFT) |
                          (0x00UL << SSTCP_PKT5_BYTEN_WN_SHIFT) |
                          (0x01UL << SSTCP_PKT5_NWORDS_SHIFT) |
                          SSTCP_PKT5));
          SETPD(hwPtr, 0, req->optData.executeFifoReq.sentinalOffset);
          SETPD(hwPtr, 0, req->optData.executeFifoReq.serialNumber);
        }
        break;
      }

      res->resStatus = FALSE;
      CMDFIFO_EPILOG(hwPtr);
      return retVal;
    }
  }
#endif
#endif
// END Alt-Tab Changes

  {
    // Storage for jmp packets
    FxU32	stateJmp[2], stateRet[2], cmdJmp[2], cmdRet[2];
    FxU32	jmpWords;
    static FxU32 lastContextID = 0x00UL;
    FxBool  doStateP;


#if ENABLE_3D
    doStateP = (_D3(lastContext) != 0) || (req->contextID != lastContextID);
#else
    doStateP = (req->contextID != lastContextID);
#endif

    switch(req->optData.executeFifoReq.fifoType) {
    case HWCEXT_FIFO_FB:
    {
      jmpWords = 1;

      if (doStateP &&
          ((0 == req->optData.executeFifoReq.stateOffset) ||
           (0 == req->optData.executeFifoReq.stateSize)))
        doStateP = FALSE;

      if (doStateP)	// new register state
      {
        stateJmp[0] = ((req->optData.executeFifoReq.stateOffset << (SSTCP_PKT0_ADDR_SHIFT - 2UL)) |
                      SSTCP_PKT0_JSR | SSTCP_PKT0);
        stateRet[0] = (SSTCP_PKT0_RET | SSTCP_PKT0);
      }
      
      cmdJmp[0] = ((req->optData.executeFifoReq.fifoOffset << (SSTCP_PKT0_ADDR_SHIFT - 2UL)) |
                  SSTCP_PKT0_JSR | SSTCP_PKT0);
      cmdRet[0] = (SSTCP_PKT0_RET | SSTCP_PKT0);
      
      goto __jmpWrite;
    }
    break;
#if (_WIN32_WINNT >= 0x0500) && defined (AGP_CMDFIFO)
    case HWCEXT_FIFO_AGP:
    {
      const FxU32 agpCmdFifoOffset = (FxU32) (((BYTE *) ppdev->fifoData.fifoPtr) - ppdev->fifoData.fifoStart);

      jmpWords = 2;
      
      if (doStateP &&
          ((0 == req->optData.executeFifoReq.stateOffset) ||
           (0 == req->optData.executeFifoReq.stateSize)))
        doStateP = FALSE;

      if (doStateP) 	// new register state
      {
        stateJmp[0] = ((req->optData.executeFifoReq.stateOffset << (SSTCP_PKT0_ADDR_SHIFT - 2UL)) |
                      SSTCP_PKT0_JMP_AGP |
                      SSTCP_PKT0);
        stateJmp[1] = 0x00UL;
      
        stateRet[0] = (((agpCmdFifoOffset + (sizeof(FxU32) << 0x01UL)) << (SSTCP_PKT0_ADDR_SHIFT - 2UL)) |
                      SSTCP_PKT0_JMP_AGP |
                      SSTCP_PKT0);
        stateRet[1] = 0x00UL;
      }
      
      cmdJmp[0] = ((req->optData.executeFifoReq.fifoOffset << (SSTCP_PKT0_ADDR_SHIFT - 2UL)) |
                  SSTCP_PKT0_JMP_AGP |
                  SSTCP_PKT0);
      cmdJmp[1] = 0x00UL;
      
      cmdRet[0] = (((agpCmdFifoOffset + (sizeof(FxU32) << (0x01UL + doStateP))) << (SSTCP_PKT0_ADDR_SHIFT - 2UL)) |
                  SSTCP_PKT0_JMP_AGP |
                  SSTCP_PKT0);
      cmdRet[1] = 0x00UL;
      
      goto __jmpWrite;
    }
    break;
#endif /* AGP_CMDFIFO */

  __jmpWrite:
    {
      FxU32 i, *pClient;


      P6_FENCE;

      // if the glide cmdfifo is in tiled memory
      // forget it
      if ((req->optData.executeFifoReq.fifoOffset >= _FF(ddTileMark)) ||
          (doStateP && (req->optData.executeFifoReq.stateOffset >= _FF(ddTileMark))))
      {
        DISPDBG((0, "  hwcExecuteFifo - cmdfifo in tiled memory"));
        DBG_BREAK;
        res->resStatus = 0;
        CMDFIFO_EPILOG(hwPtr);
        return 0;
      }

      // if the glide cmdfifo is beyond 16MB, copy the data from their cmdfifo into
      // the display driver's cmdfifo
      if ((req->optData.executeFifoReq.fifoOffset >= 16*1024*1024) ||
          ((req->optData.executeFifoReq.fifoOffset + req->optData.executeFifoReq.fifoSize) >= 16*1024*1024) ||
          (doStateP && ((req->optData.executeFifoReq.stateOffset >= 16*1024*1024) ||
                        ((req->optData.executeFifoReq.stateOffset + req->optData.executeFifoReq.stateSize) >= 16*1024*1024))))
      {
        DWORD *cmdBuf, *cmdBufEnd;
        DWORD numCmds;


        if (doStateP)	// new register state
        {
          /* State return to fifo */
          pClient = (FxU32*)(req->optData.executeFifoReq.statePtr +
                    (req->optData.executeFifoReq.stateSize << 0x02UL));
          
          for (i = 0; i < jmpWords; i++)
            pClient[i] = stateRet[i];
          
          P6_FENCE;
  
          CMDFIFO_CHECKROOM(hwPtr, req->optData.executeFifoReq.stateSize + req->optData.executeFifoReq.fifoSize + 2);
        
          numCmds = req->optData.executeFifoReq.stateSize;
        
          cmdBuf    = (DWORD *) (req->optData.executeFifoReq.stateOffset + ppdev->pjLfbBase);
          cmdBufEnd = cmdBuf + numCmds;
        
          while (cmdBuf < cmdBufEnd)
          {
            /* This has NO direct write equivalent */
            SETPD(hwPtr, 0, *cmdBuf);
            cmdBuf++;
          }
        }
        else
        {
          CMDFIFO_CHECKROOM(hwPtr, req->optData.executeFifoReq.fifoSize + 2);
        }
  
        /* command return to fifo */
        pClient = (FxU32*)(req->optData.executeFifoReq.fifoPtr +
                  (req->optData.executeFifoReq.fifoSize << 2UL));
        
        for (i = 0; i < jmpWords; i++)
          pClient[i] = cmdRet[i];
        
        P6_FENCE;
  
        /* Current command stream */
        //CHECK_FIFO_ROOM(ppdev, req->optData.executeFifoReq.fifoSize);
        numCmds = req->optData.executeFifoReq.fifoSize;
        
        cmdBuf    = (DWORD *) (req->optData.executeFifoReq.fifoOffset + ppdev->pjLfbBase);
        cmdBufEnd = cmdBuf + numCmds;
        
        while (cmdBuf < cmdBufEnd)
        {
          /* This has NO direct write equivalent */
          SETPD(hwPtr, 0, *cmdBuf);
          cmdBuf++;
        }

        /* Write the serial # for this fifo execution. */
        //CHECK_FIFO_ROOM(ppdev, 2);
        SETPH(hwPtr, (SSTCP_PKT5_LFB |
                      (0x00UL << SSTCP_PKT5_BYTEN_W2_SHIFT) |
                      (0x00UL << SSTCP_PKT5_BYTEN_WN_SHIFT) |
                      (0x01UL << SSTCP_PKT5_NWORDS_SHIFT) |
                      SSTCP_PKT5));
        SETPD(hwPtr, 0, req->optData.executeFifoReq.sentinalOffset);
        SETPD(hwPtr, 0, req->optData.executeFifoReq.serialNumber);
      }
      // if the cmdfifo isn't beyond 16MB, insert return instruction at the end of
      // the glide cmdfifo, then insert a jump instruction in the display driver's
      // cmdfifo
      else
      {
        if (doStateP)	// new register state
        {
          /* State return to fifo */
          pClient = (FxU32*)(req->optData.executeFifoReq.statePtr +
                    (req->optData.executeFifoReq.stateSize << 0x02UL));
          
          for (i = 0; i < jmpWords; i++)
            pClient[i] = stateRet[i];
          
          P6_FENCE;
          
          /* Jump to state routine */
          CMDFIFO_CHECKROOM(hwPtr, 2 * jmpWords + 2);

          for (i = 0; i < jmpWords; i++)
            SETPD(hwPtr, 0, stateJmp[i]);
        }
        else
        {
          CMDFIFO_CHECKROOM(hwPtr, jmpWords + 2);
        }
  
        /* command return to fifo */
        pClient = (FxU32*)(req->optData.executeFifoReq.fifoPtr +
                  (req->optData.executeFifoReq.fifoSize << 2UL));
        
        for (i = 0; i < jmpWords; i++)
          pClient[i] = cmdRet[i];
        
        P6_FENCE;
        
        /* Jump to state routine */
        //CHECK_FIFO_ROOM(ppdev, jmpWords);
        for (i = 0; i < jmpWords; i++)
          SETPD(hwPtr, 0, cmdJmp[i]);
        
        /* Write the serial # for this fifo execution. */
        //CHECK_FIFO_ROOM(ppdev, 2);
        SETPH(hwPtr, (SSTCP_PKT5_LFB |
                      (0x00UL << SSTCP_PKT5_BYTEN_W2_SHIFT) |
                      (0x00UL << SSTCP_PKT5_BYTEN_WN_SHIFT) |
                      (0x01UL << SSTCP_PKT5_NWORDS_SHIFT) |
                      SSTCP_PKT5));
        SETPD(hwPtr, 0, req->optData.executeFifoReq.sentinalOffset);
        SETPD(hwPtr, 0, req->optData.executeFifoReq.serialNumber);
      }
    }
    pGS->fifoType = req->optData.executeFifoReq.fifoType;
    pGS->lastFifoOffset = req->optData.executeFifoReq.fifoOffset;
    pGS->lastFifoSize = req->optData.executeFifoReq.fifoSize;
    if (doStateP)
    {
      pGS->lastStateOffset = req->optData.executeFifoReq.stateOffset;
      pGS->lastStateSize = req->optData.executeFifoReq.stateSize;
    }

    break;

    case HWCEXT_FIFO_HOST:
    {
      DWORD *cmdBuf, *cmdBufEnd;
      DWORD numCmds;
      
      if (doStateP)	// new register state
      {
        CMDFIFO_CHECKROOM(hwPtr, req->optData.executeFifoReq.stateSize + req->optData.executeFifoReq.fifoSize);
      
        numCmds = req->optData.executeFifoReq.stateSize;
      
        cmdBuf    = (DWORD *) req->optData.executeFifoReq.statePtr;
        cmdBufEnd = cmdBuf + numCmds;
      
        while (cmdBuf < cmdBufEnd)
        {
          /* This has NO direct write equivalent */
          SETPD(hwPtr, 0, *cmdBuf);
          cmdBuf++;
        }
      }
      else
      {
        CMDFIFO_CHECKROOM(hwPtr, req->optData.executeFifoReq.fifoSize);
      }

      /* Current command stream */
      //CHECK_FIFO_ROOM(ppdev, req->optData.executeFifoReq.fifoSize);
      numCmds = req->optData.executeFifoReq.fifoSize;
      
      cmdBuf    = (DWORD *) req->optData.executeFifoReq.fifoPtr;
      cmdBufEnd = cmdBuf + numCmds;
      
      while (cmdBuf < cmdBufEnd)
      {
        /* This has NO direct write equivalent */
        SETPD(hwPtr, 0, *cmdBuf);
        cmdBuf++;
      }
    }
    break;

    default:
      retVal = 0;
    break;

    }

    lastContextID = req->contextID;
  }

  CMDFIFO_EPILOG(hwPtr);

#if ENABLE_3D
  // notify D3D that glide is executing so the hw state may have changed
  if (_D3(lastContext))
  {
    RC *pRc = (RC *)_D3(lastContext);
    pRc->hwStateChanged = SC_STATE | SC_FOGALL | SC_BUFFERS;
  
    _D3(lastContext) = 0;
  }
#endif
#endif
  return retVal;
} /* hwcExecuteFifo */

/*----------------------------------------------------------------------
Function name:  hwcQueryContext

Description:    Query the HW Context

Information:    Presently unused!

Return:         LONG    0 is always returned.
----------------------------------------------------------------------*/
static LONG
hwcQueryContext(hwcExtRequest_t *req, hwcExtResult_t *res, PDEV *ppdev)
{
	VALIDATECONTEXT;

 	// return no error
	res->resStatus = 0;
	
	return 1;

} /* hwcQueryContext */


/*----------------------------------------------------------------------
Function name:  hwcReleaseContext

Description:    Release the HW Context

Information:    Presently unused!

Return:         LONG    1 is always returned.
----------------------------------------------------------------------*/
static LONG
hwcReleaseContext(hwcExtRequest_t *req, hwcExtResult_t *res, PDEV *ppdev)
{
  HANDLE      curPID;
  GLIDESTATE  *pGS;


  VALIDATECONTEXT;

  curPID = getCurrentProcessId(ppdev);
  if ((HANDLE)-1 == curPID)
  {
    DISPDBG((0, "  hwcReleaseContext - getCurrentProcessId failed"));
    DBG_BREAK;
    res->resStatus = -1;
    return -1;
  }
#if ENABLE_V3_W2K_GLIDE_CHANGES
  if (curPID == (HANDLE)req->optData.releaseContextReq.procID)
    DISPDBG((0, "  PID passed by glide matches PID found by miniport.  (PID=%8lXh)", curPID));
  else
    DISPDBG((0, "  PID passed by glide (%8lXh) doesn't match PID found by miniport (%8lXh)!!!",
             req->optData.releaseContextReq.procID, curPID));
#endif

  // find the GLIDESTATE associated with this curPID
  pGS = hwcGetGlideStateStructureForProcess(curPID);
  if (NULL == pGS)
  {
    DISPDBG((0, "  hwcReleaseContext - getGlideContext failed"));
    DBG_BREAK;
    res->resStatus = -1;
    return -1;
  }
#if DBG
  if (req->contextID != (FxU32)pGS)
    DISPDBG((GLIDE_DBG_LEVEL, "  context passed in by glide (%8lXh) not the same as pGS (%8lXh)",
             req->contextID, pGS));
#endif

  /* only unmap the memory if we are really done with it */
  if (pGS->glideReferenceCount == 1)
  {
    // free sContextInfo 
    memset(&sContextInfo[pGS->contextDWORDNTIndex], 0, sizeof(sContextInfo[0]));

    miscGlideUnmapMemoryBases(ppdev, curPID);
  }

  // release the GLIDE state structure for the current process
  hwcReleaseGlideStateStructureForProcess(curPID);

  //miscRestoreExtState( ppdev );

  res->resStatus = 1;
  return 1;
} /* hwcReleaseContext */

/*----------------------------------------------------------------------
Function name:  hwcSetExclusive

Description:    Sets environment for exclusive mode.

Information:

Return:         LONG    1 is always returned.
----------------------------------------------------------------------*/
static LONG
hwcSetEsclusive(hwcExtRequest_t *req, hwcExtResult_t *res, PDEV *ppdev)
{
  HANDLE      curPID;
  GLIDESTATE  *pGS;


  curPID = getCurrentProcessId(ppdev);
  if ((HANDLE)-1 == curPID)
  {
    DISPDBG((0, "  hwcSetEsclusive - getCurrentProcessId failed"));
    DBG_BREAK;
    res->resStatus = -1;
    return -1;
  }

  pGS = hwcGetGlideStateStructureForProcess(curPID);
  if (NULL == pGS)
  {
    DISPDBG((0, "  hwcSetEsclusive - getGlideContext failed"));
    DBG_BREAK;
    res->resStatus = -1;
    return -1;
  }
#if DBG
  if (req->contextID != (FxU32)pGS)
    DISPDBG((GLIDE_DBG_LEVEL, "  context passed in by glide (%8lXh) not the same as pGS (%8lXh)",
             req->contextID, pGS));
#endif
  pGS->glideGDIFlags |= GLDATA_GDIFLAGS_HWC_EXCLUSIVE;

  glideState[0].glideGDIFlags |= GLDATA_GDIFLAGS_HWC_EXCLUSIVE;

  DUMP_H3_REGS(ppdev, GLIDE_DBG_LEVEL+10);

//  DiscardAllSSB();        // invalidate all save screen bitmaps
  bMoveAllDfbsFromOffscreenToDibs(ppdev);   // move all device bitmaps to host
//  DisableDeviceBitmaps();   // disallow future device bitmaps

  HWSetPalette(ppdev, 0, 256, (PVIDEO_CLUTDATA)ppdev->pPal, GAMMA_GLIDE);

  res->resStatus = 1;

  return 1;
} /* hwcSetEsclusive */

#ifdef ENABLE_V3_W2K_GLIDE_CHANGES
/*----------------------------------------------------------------------
Function name:  hwcProtectCmdFifo

Description:    Sets environment for exclusive mode.

Information:

Return:         LONG    1 is always returned.
----------------------------------------------------------------------*/
static LONG
hwcProtectCmdFifo(hwcExtRequest_t *req, hwcExtResult_t *res, PDEV *ppdev)
{
	GLIDESTATE * psGlideState;

	if( ( psGlideState = hwcGetGlideStateStructureForProcess( ( HANDLE )req->optData.protectCmdFifo.procId ) ) )
	{
		// check to see if the memory is to be protected, or released
		if( req->optData.protectCmdFifo.protect )
		{
			// the memory address is to be logged to be protected
			// only store in address 0 for the time being, no need to store 
			// more than one pointer
			psGlideState->pvCommandFifoLinearAddress[ 0 ] = ( PVOID )req->optData.protectCmdFifo.linearAddress;

			// set the flag to indicate that the command fifo memory space is to be 
			// protected
			psGlideState->dwProtectCmdFifo++;
			
		}
		else
		{
			// check to see if the memory is to be unmapped, if so then unmap the memory
			if( !--psGlideState->dwUnMapMemoryLFB )
			{
				DWORD ReturnedDataLength;

				// this functionality requires -1 to indicate the current process..this
				// should be fine because the GLIDE process is calling us from the current
				// process( duh ) and the unmapping came from this process
				psGlideState->sUnMapShareMemory.ProcessHandle = ( HANDLE )-1;

				// unmap the memory because GLIDE does not own it
				if (EngDeviceIoControl(ppdev->hDriver,
								IOCTL_VIDEO_UNSHARE_VIDEO_MEMORY,
			                    &psGlideState->sUnMapShareMemory,
				                sizeof(VIDEO_SHARE_MEMORY),
					            NULL,
						        0,
							    &ReturnedDataLength))
				{
					RIP("Failed IOCTL_VIDEO_UNSHARE_MEMORY");
				}
			}

			// reset the flag to indicate that the command fifo memory space is no longer
			// to be protected
			psGlideState->dwProtectCmdFifo--;
		}
	}

	res->resStatus = 1;

	return 1;

} /* hwcProtectCmdFifo */
#endif

/*----------------------------------------------------------------------
Function name:  hwcRlsExclusive

Description:    Releases exclusive mode.

Information:

Return:         LONG    1 is always returned.
----------------------------------------------------------------------*/
static BYTE ajPointerBuf[HW_POINTER_TOTAL_SIZE];

#define CMDFIFO_START_OFFSET  0

static LONG
hwcRlsEsclusive(hwcExtRequest_t *req, hwcExtResult_t *res, PDEV *ppdev)
{
  DWORD       ReturnedDataLength;
  DWORD       *pulSrc;
  DWORD       *pulDst;
  LONG        i;
  HANDLE      curPID;
  GLIDESTATE  *pGS;


  VALIDATECONTEXT;

  if (ppdev->bHwPointerActive)
  {
    // Save hardware pointer shape for restore after mode set.

    pulSrc = (ULONG*) ppdev->pvPointerShape;
    pulDst = (ULONG*) ajPointerBuf;
    for (i = HW_POINTER_TOTAL_SIZE / sizeof(ULONG); i != 0; i--)
    {
      *pulDst++ = *pulSrc++;
    }
  }

  curPID = getCurrentProcessId(ppdev);
  if ((HANDLE)-1 == curPID)
  {
    DISPDBG((0, "  hwcRlsEsclusive - getCurrentProcessId failed"));
    DBG_BREAK;
  }
  else
  {
    pGS = hwcGetGlideStateStructureForProcess(curPID);
    if (NULL == pGS)
    {
      DISPDBG((0, "  hwcRlsEsclusive - getGlideContext failed"));
      DBG_BREAK;
    }
    else
    {
#if DBG
      if (req->contextID != (FxU32)pGS)
        DISPDBG((GLIDE_DBG_LEVEL, "  context passed in by glide (%8lXh) not the same as pGS (%8lXh)",
                 req->contextID, pGS));
#endif
      pGS->glideGDIFlags &= ~GLDATA_GDIFLAGS_HWC_EXCLUSIVE;
    }
  }

  // fix for PRS #2395
  // if the driver has already been taken out of HWC_EXCLUSIVE mode
  // by a DrvEnableSurface call then just reset the fifo
  // otherwise do the normal stuff
  if (GLDATA_GDIFLAGS_HWC_EXCLUSIVE & glideState[0].glideGDIFlags)
  {
    // Call the miniport via an IOCTL to set the graphics mode.

    DISPDBG((GLIDE_DBG_LEVEL, "hwcRlsExclusive: setting Mode Number 0x%d", ppdev->ulMode));

    if (EngDeviceIoControl(ppdev->hDriver,
                           IOCTL_VIDEO_SET_CURRENT_MODE,
                           &ppdev->ulMode,  // input buffer
                           sizeof(DWORD),
                           NULL,
                           0,
                           &ReturnedDataLength))
    {
      DISPDBG((GLIDE_DBG_LEVEL, "hwcRlsEsclusive - VIDEO_SET_CURRENT_MODE failed from HWC release exclusive"));
    }

    bAssertModeHardware(ppdev, TRUE);

#if ENABLE_RECONFIG_VIDMEM
    reconfigureVideoMemory(ppdev);
#endif

    vAssertModeText( ppdev, FALSE);

    if (ppdev->bHwPointerActive)
    {
      // Modeset is finished.  Restore hardware pointer shape.

      pulSrc = (ULONG*) ajPointerBuf;
      pulDst = (ULONG*) ppdev->pvPointerShape;
      for (i = HW_POINTER_TOTAL_SIZE / sizeof(ULONG); i != 0; i--)
      {
        *pulDst++ = *pulSrc++;
      }
    }

    glideState[0].glideGDIFlags &= ~GLDATA_GDIFLAGS_HWC_EXCLUSIVE;
  }
#ifdef H3_FIFO
  else
#if (_WIN32_WINNT >= 0x0500) && defined (AGP_CMDFIFO)
  {
    BOOL                    doAGPFifo;
    H3_AGP_INFO             h3AgpInfo;

    doAGPFifo = (ppdev->flCaps & CAPS_AGP_FIFO) ? TRUE : FALSE;
    ppdev->doAGPFifo = doAGPFifo;

    if( doAGPFifo == TRUE )
    {
        ppdev->ulScreenOffset = CMDFIFO_START_OFFSET;

        if (EngDeviceIoControl(ppdev->hDriver,
                               IOCTL_3DFX_GET_AGP_FIFO_INFO,
                               NULL,
                               0,
                               &h3AgpInfo,
                               sizeof(h3AgpInfo),
                               &ReturnedDataLength))
        {
            RIP( "hwcRlsEsclusive - failed GET_AGP_FIFO_INFO" );
        }

        ppdev->fifoData.physBaseL = h3AgpInfo.physAddr.LowPart;
        ppdev->fifoData.physBaseH = (ULONG) h3AgpInfo.physAddr.HighPart;

        ppdev->fifoData.fifoOffset = 0;
        ppdev->fifoData.fifoStart = h3AgpInfo.virtualAddr;
        ppdev->fifoData.fifoPtr = (ULONG *) ppdev->fifoData.fifoStart;
        ppdev->fifoData.fifoLastRead = (ULONG) ppdev->fifoData.physBaseL;
        ppdev->fifoData.fifoSize = h3AgpInfo.agpFifoSizeInB;
        ppdev->fifoData.fifoEnd = ppdev->fifoData.fifoStart + ppdev->fifoData.fifoSize;

        /* Adjust room values.
         * RoomToEnd needs enough room for the jmp packet since we
         * never allow the hw to auto-wrap. RoomToRead needs to be
         * adjusted so that we never acutally write onto the read ptr.
         *
         * fifoRoom is generally the min of roomToEnd and roomToRead,
         * but we 'know' here that roomToRead < roomToEnd.
         */
        ppdev->fifoData.roomToEnd     = ppdev->fifoData.fifoSize - H3_FIFO_END_ADJUST;
        ppdev->fifoData.fifoRoom      =
        ppdev->fifoData.roomToReadPtr = ppdev->fifoData.roomToEnd;

        // Pre-compute the packet to return us back to the start.

        ppdev->fifoData.fifoJmpHdr = (SSTCP_PKT0_JMP_AGP  | ((ppdev->fifoData.physBaseL << (SSTCP_PKT0_ADDR_SHIFT - 2)) & SSTCP_PKT0_ADDR));
        ppdev->fifoData.fifoJmpHdr2 = (ppdev->fifoData.physBaseL >> 25);

        CmdFifo0Init( ppdev,
                      ppdev->fifoData.fifoOffset,
                      ppdev->fifoData.fifoSize,
                      0,
                      doAGPFifo );

        ppdev->fifoData.fifoSize -= H3_FIFO_END_ADJUST;
    }
    else
#endif // AGP_CMDFIFO
    {
#if USE_D3D_CODE && (_WIN32_WINNT >= 0x0500)
        ULONG   fifoSize;
#endif
        LONG  cjFifoOffset;

        // Put the fifo at the beginning of memory.
        cjFifoOffset = CMDFIFO_START_OFFSET;
        ppdev->fifoData.fifoOffset = cjFifoOffset;
#if CSIM
        ppdev->fifoData.fifoStart = (BYTE *) CP_BEGIN(cjFifoOffset);
#else
        ppdev->fifoData.fifoStart = ppdev->pjScreenBase + cjFifoOffset;
#endif
        /* Set initial fifo state. hw read and sw write pointers at
         * start of the fifo.
         */
        ppdev->fifoData.fifoPtr = (ULONG *) ppdev->fifoData.fifoStart;
        ppdev->fifoData.fifoLastRead = (ULONG) ppdev->fifoData.fifoOffset;

#if USE_D3D_CODE && (_WIN32_WINNT >= 0x0500)
        if ((2 == ppdev->cjPelSize) || ((IS_NAPALM) && (4 == ppdev->cjPelSize)))
        {
#if 1
          if (GetRegDWORD(ppdev, "CmdfifoSize", &fifoSize))
          {
            // round up to nearest 4kB size
            fifoSize = ((fifoSize + (4*1024 - 1)) / (4*1024)) * (4*1024);

            // limit minimum fifoSize to 64kB and maximum fifoSize to 1MB
            if (HW_CMDFIFO_TOTAL_SIZE > fifoSize)
            {
              fifoSize = HW_CMDFIFO_TOTAL_SIZE;
            }
            else if (1*1024*1024 < fifoSize)
            {
              fifoSize = 1*1024*1024;
            }
          }
          else
#endif
          {
            // default to 512kB cmdfifo
            fifoSize = 512*1024;
          }

          ppdev->fifoData.fifoSize = fifoSize;
          DISPDBG((GLIDE_DBG_LEVEL, "  cmdfifoSize = %8lXh", ppdev->fifoData.fifoSize));
        }
        else
#endif
          ppdev->fifoData.fifoSize = HW_CMDFIFO_TOTAL_SIZE;
        ppdev->fifoData.fifoEnd = ppdev->fifoData.fifoStart + ppdev->fifoData.fifoSize;

        /* Adjust room values.
         * RoomToEnd needs enough room for the jmp packet since we
         * never allow the hw to auto-wrap. RoomToRead needs to be
         * adjusted so that we never acutally write onto the read ptr.
         *
         * fifoRoom is generally the min of roomToEnd and roomToRead,
         * but we 'know' here that roomToRead < roomToEnd.
         */
        ppdev->fifoData.roomToEnd     = ppdev->fifoData.fifoSize - H3_FIFO_END_ADJUST;
        ppdev->fifoData.fifoRoom      =
        ppdev->fifoData.roomToReadPtr = ppdev->fifoData.roomToEnd;

        // Pre-compute the packet to return us back to the start.

        ppdev->fifoData.fifoJmpHdr = (SSTCP_PKT0_JMP_LOCAL  | (CMDFIFO_START_OFFSET << (SSTCP_PKT0_ADDR_SHIFT - 2)));

        // Adjust display start in PDEV and video unit to after fifo.
#if USE_D3D_CODE && (_WIN32_WINNT >= 0x0500)
        if ((2 == ppdev->cjPelSize) || ((IS_NAPALM) && (4 == ppdev->cjPelSize)))
          ppdev->ulScreenOffset = fifoSize + CMDFIFO_START_OFFSET;
        else
#endif
          ppdev->ulScreenOffset = HW_CMDFIFO_TOTAL_SIZE + CMDFIFO_START_OFFSET;
        DISPDBG((GLIDE_DBG_LEVEL, "adjust ulScreenOffset for cmdfifo: %ld", ppdev->ulScreenOffset));

        // Enable command fifo here.

        CmdFifo0Init(ppdev,
                     ppdev->fifoData.fifoOffset,
                     ppdev->fifoData.fifoSize,
                     0,
                     0);
// START Alt-Tab Changes
        ppdev->ulCmdFifoDisabled = FALSE;
// END Alt-Tab Changes

        ppdev->fifoData.fifoSize -= H3_FIFO_END_ADJUST;
    }
#if (_WIN32_WINNT >= 0x0500) && defined (AGP_CMDFIFO)
  }
#endif
#endif	// H3_FIFO

  DUMP_H3_REGS(ppdev, GLIDE_DBG_LEVEL+10);

  // restore registers that glide modified
  for (i = 0; i < (LONG)_FF(dwNumUnits); i++)
  {
    SstIORegs *pIORegs = (SstIORegs *)(_FF(regBase[i * HWINFO_SST_MAX_CHIP_INDEX + HWINFO_SST_IOREGS_INDEX]));

    pIORegs->vidPixelBufThold = 0x10410;
  }

  res->resStatus = 1;

  return 1;

} /* hwcRlsEsclusive */

/*----------------------------------------------------------------------
Function name:  hwcFifoInfo

Description:

Information:

Return:
----------------------------------------------------------------------*/
static LONG
hwcFifoInfo(hwcExtRequest_t* req, hwcExtResult_t* res, PDEV *ppdev)
{
#if (_WIN32_WINNT >= 0x0500) && defined (AGP_CMDFIFO)
//  res->optData.fifoInfoRes.fifoType = ((_FF(enableAGPCF) && _FF(doAgpCF))
  res->optData.fifoInfoRes.fifoType = ppdev->doAGPFifo
                                       ? HWCEXT_FIFO_AGP
                                       : HWCEXT_FIFO_FB;
#else
  res->optData.fifoInfoRes.fifoType = HWCEXT_FIFO_FB;
#endif
  res->resStatus = 1;

  return 1;
}

#ifdef ENABLE_V3_W2K_GLIDE_CHANGES
/*----------------------------------------------------------------------
Function name:  hwcSetContextDWORDBit


Information:

Return:
----------------------------------------------------------------------*/
VOID hwcSetContextDWORDBit( HANDLE hProcess, DWORD dwBitMask )
{
	GLIDESTATE * psGlideState;
	ULONG ulIndex;

	// verify that the process is valid
	if( ( psGlideState = hwcGetGlideStateStructureForProcess( hProcess ) ) )
	{
		// find the context info structure for the current process
		for( ulIndex = 0x00; ulIndex < _MAX_CONTEXT_INFO; ulIndex++ )	
		{
			// find a matchine process ID in the context info list
			if( sContextInfo[ ulIndex ].ulProcessID == ( ULONG )hProcess )
			{
				// set the bit
				*(( ULONG * )pvMiscNonCachedMemory + ulIndex ) |= 0x80000000;

				break;
			}
		}

	}

	return;

} /* hwcSetContextDWORDBit */
#endif

#if (_WIN32_WINNT >= 0x0500) && defined (AGP_CMDFIFO)
/*----------------------------------------------------------------------
Function name:  hwcAGPInfo

Description:    Obtain information about AGP.

Information:

Return:         LONG    1 is always returned.
----------------------------------------------------------------------*/
static LONG
hwcAGPInfo (hwcExtRequest_t *req, hwcExtResult_t *res, PDEV *ppdev)
{
  res->optData.agpInfoRes.lAddr = (ULONG) ppdev->fifoData.fifoStart;
  res->optData.agpInfoRes.pAddr = ppdev->fifoData.physBaseL;
  res->optData.agpInfoRes.size  = ppdev->fifoData.fifoSize;

  res->resStatus = 1;

  return 1;
} /* hwcAGPInfo */
#endif


/*----------------------------------------------------------------------
Function name:  hwcLinearMapOffset

Description:    Obtain the Linear Map Offset.

Information:

Return:         LONG    1 for a valid linear address or,
                        0 for failure.
----------------------------------------------------------------------*/
static LONG
hwcLinearMapOffset(hwcExtRequest_t* req, hwcExtResult_t*  res, PDEV *ppdev)
{
  res->resStatus = 0;

#if (_WIN32_WINNT >= 0x0500) && defined (AGP_CMDFIFO)
  {
    hwcExtResult_t agpInfo;

    if ( ppdev->doAGPFifo == TRUE )
    {

      res->resStatus = ((req->optData.mapInfoReq.remapAddr >= ppdev->fifoData.physBaseL) &&
                        (req->optData.mapInfoReq.remapAddr < (ppdev->fifoData.physBaseL +
                                                              ppdev->fifoData.fifoSize)));
      if (res->resStatus)
      {
#pragma message(__FILELINE__ "Are these sizes right?")
        if (IS_NAPALM)
          res->optData.mapInfoRes.linAddrOffset = ((req->optData.mapInfoReq.remapAddr - ppdev->fifoData.physBaseL) +
                                                   0x8000000UL); /* Size of memBase1 */
        else
          res->optData.mapInfoRes.linAddrOffset = ((req->optData.mapInfoReq.remapAddr - ppdev->fifoData.physBaseL) +
                                                   0x2000000UL); /* Size of memBase1 */
      }
    }
  }
#endif /* AGP_CMDFIFO */
  if (!res->resStatus)
  {
    DWORD       i;
    HANDLE      curPID;
    GLIDESTATE  *pGS;


    curPID = getCurrentProcessId(ppdev);
    if ((HANDLE)-1 == curPID)
    {
      DISPDBG((0, "hwcLinearMapOffset - getCurrentProcessId failed!!!"));
      DBG_BREAK;
      res->optData.mapInfoRes.linAddrOffset = 0;
      res->resStatus = 0;
      return 0;
    }
    pGS = hwcGetGlideStateStructureForProcess(curPID);
    if (NULL == pGS)
    {
      DISPDBG((0, "hwcLinearMapOffset - No GLIDESTATE struct found for process id %lXh!!!", curPID));
      DBG_BREAK;
      res->optData.mapInfoRes.linAddrOffset = 0;
      res->resStatus = 0;
      return 0;
    }
#if DBG
    if (req->contextID != (FxU32)pGS)
      DISPDBG((GLIDE_DBG_LEVEL, "  context passed in by glide (%8lXh) not the same as pGS (%8lXh)",
               req->contextID, pGS));
#endif
#if 0
    if (pGS->glideGDIFlags & (GLDATA_GDIFLAGS_PPDEV_DESTROYED | GLDATA_GDIFLAGS_PPDEV_DISABLED))
    {
      DISPDBG((0, "hwcLinearMapOffset - ppdev is destroyed or disabled!!! (curPID=%lXh, pGS=%8lXh, flags=%8lXh)",
               curPID, pGS, pGS->glideGDIFlags));
      DBG_BREAK;
      res->optData.mapInfoRes.linAddrOffset = 0;
      res->resStatus = 0;
      return 0;
    }
#endif

    for (i = 0; i < MAX_ADDRESS_TABLE_SIZE; i++)
    {
      if (curPID == LfbMappings[i].ProcessID)
      {
        DISPDBG((GLIDE_DBG_LEVEL, "  found mapping in display driver array, index=%ld", i));

        DISPDBG((GLIDE_DBG_LEVEL, "ulAddress     = %08lXh", req->optData.mapInfoReq.remapAddr));

        res->optData.mapInfoRes.linAddrOffset = (FxU32) req->optData.mapInfoReq.remapAddr -
                                                (FxU32) LfbMappings[i].LFBAddr;
        DISPDBG((GLIDE_DBG_LEVEL, "ulLinearBase  = %08lXh", LfbMappings[i].LFBAddr));
        DISPDBG((GLIDE_DBG_LEVEL, "linAddrOffset = %08lXh", res->optData.mapInfoRes.linAddrOffset));
    
        // check to see that the offset is within the 2*ppdev->cjBank range, if not then the address
        // passed in must be invalid.. probably a system memory surface gone undetected.
        if (res->optData.mapInfoRes.linAddrOffset > (ULONG)2*ppdev->cjBank)
        {
          // zero out the offset to return an error
          res->optData.mapInfoRes.linAddrOffset = 0;
    
          // return an error
          res->resStatus = 0;
          return res->resStatus;
        }

        break;
      }
    }

    if (i == MAX_ADDRESS_TABLE_SIZE)
    {
      // zero out the offset to return an error
      res->optData.mapInfoRes.linAddrOffset = 0;

      // return an error
      res->resStatus = 0;
      return res->resStatus;
    }

    res->resStatus = 1;
  }

  return res->resStatus;
}

/*----------------------------------------------------------------------
Function name:  hwcDownloadGamma

Description:

Information:

Return:
----------------------------------------------------------------------*/
LONG
hwcDownloadGamma(hwcExtRequest_t *req, hwcExtResult_t *res, PDEV *ppdev)
{
  HANDLE      curPID;
  GLIDESTATE  *pGS;


  if ((PULONG)req->optData.gamma.gammaPtr == NULL ||
      (req->optData.gamma.numGammaEntries > 256))
  {
    DISPDBG((0, "hwcDownloadGamma - gammaPtr NULL or too many gamma entries!!!"));
    DBG_BREAK;
    res->resStatus = -1;
    return res->resStatus;
  }

  curPID = getCurrentProcessId(ppdev);
  if ((HANDLE)-1 == curPID)
  {
    DISPDBG((0, "hwcDownloadGamma - getCurrentProcessId failed!!!"));
    DBG_BREAK;
    res->resStatus = 0;
    return 0;
  }
  pGS = hwcGetGlideStateStructureForProcess(curPID);
  if (NULL == pGS)
  {
    DISPDBG((0, "hwcDownloadGamma - No GLIDESTATE struct found for process id %lXh!!!", curPID));
    DBG_BREAK;
    res->resStatus = 0;
    return 0;
  }
#if DBG
  if (req->contextID != (FxU32)pGS)
    DISPDBG((GLIDE_DBG_LEVEL, "  context passed in by glide (%8lXh) not the same as pGS (%8lXh)",
             req->contextID, pGS));
#endif
  if (! (pGS->glideGDIFlags & GLDATA_GDIFLAGS_HWC_EXCLUSIVE))
  {
    DISPDBG((0, "hwcDownloadGamma - not in hwc exclusive mode!!! (curPID=%lXh, pGS=%8lXh)", curPID, pGS));
    DBG_BREAK;
    res->resStatus = 0;
    return 0;
  }

  /* copy the data to the pdev */
  memcpy(ppdev->TransientGammaTable,
         (PVOID)req->optData.gamma.gammaPtr,
         (req->optData.gamma.numGammaEntries * sizeof(ULONG)));

  HWSetPalette(ppdev, 0, 256, (PVIDEO_CLUTDATA)ppdev->pPal, GAMMA_TRANSIENT);

  res->resStatus = 1;
  return 1;
} /* hwcDownloadGamma */

/*----------------------------------------------------------------------
Function name:  hwcShareContextDWORD

Description:    Coordinates loss of context safely with Glide.

Information:

Return:         LONG    1 is always returned.
----------------------------------------------------------------------*/
LONG
hwcShareContextDWORD(hwcExtRequest_t *req, hwcExtResult_t *res, PDEV *ppdev)
{
	// store a pointer to the variable shared with GLIDE to store the context state
	res->optData.shareContextDWORDRes.contextDWORD = 
		req->optData.shareContextDWORDReq.contextDWORD;
		
	return 1;

} /* hwcShareContextDWORD */

/*----------------------------------------------------------------------
Function name:  hwcUnMapMemory

Description:

Information:

Return:
----------------------------------------------------------------------*/
LONG
hwcUnMapMemory(hwcExtRequest_t *req, hwcExtResult_t *res, PDEV *ppdev)
{
  ULONG       ulIndex;
  HANDLE      curPID;
  GLIDESTATE  *pGS;


  curPID = getCurrentProcessId(ppdev);
  if ((HANDLE)-1 == curPID)
  {
    DISPDBG((0, "hwcUnMapMemory - getCurrentProcessId failed!!!"));
    DBG_BREAK;
    curPID = (HANDLE)(req->optData.unmapMemoryReq.procHandle);
  }
#if DBG
  if (curPID == (HANDLE)req->optData.unmapMemoryReq.procHandle)
    DISPDBG((GLIDE_DBG_LEVEL, "  PID passed by glide matches PID found by miniport.  (PID=%8lXh)", curPID));
  else
    DISPDBG((GLIDE_DBG_LEVEL, "  PID passed by glide (%8lXh) doesn't match PID found by miniport (%8lXh)!!!",
             req->optData.unmapMemoryReq.procHandle, curPID));
#endif

  
  // the glide context may have already been destroyed in hwcReleaseContext
  pGS = hwcGetGlideStateStructureForProcess(curPID);
  if (NULL == pGS)
  {
    DISPDBG((0, "hwcUnMapMemory - No GLIDESTATE struct found for process id %lXh!!!", curPID));
    //DBG_BREAK;

    // unmap the device from this process
    miscGlideUnmapMemoryBases(ppdev, curPID);

    // find and free the sContextInfo struct
    for (ulIndex = 0; ulIndex < _MAX_CONTEXT_INFO; ulIndex++)
    {
      if (sContextInfo[ulIndex].ulProcessID == (ULONG)req->optData.unmapMemoryReq.procHandle)
      {
        memset(&sContextInfo[ulIndex], 0, sizeof(sContextInfo[0]));
        break;
      }
    }
  }
  else
  {
#if DBG
    if (req->contextID != (FxU32)pGS)
      DISPDBG((GLIDE_DBG_LEVEL, "  context passed in by glide (%8lXh) not the same as pGS (%8lXh)",
               req->contextID, pGS));
#endif

    if (pGS->glideReferenceCount == 1)
    {
      // unmap the device from this process
      miscGlideUnmapMemoryBases(ppdev, curPID);

      // free the sContextInfo struct
      memset(&sContextInfo[pGS->contextDWORDNTIndex], 0, sizeof(sContextInfo[0]));
    }

    // release the GLIDE state structure for the current process
    hwcReleaseGlideStateStructureForProcess(curPID);
  }

  res->resStatus = 1;
	return 1;
} /* hwcUnMapMemory */

/*----------------------------------------------------------------------
Function name:  hwcContextDWORDNT

Description:

Information:

Return:
----------------------------------------------------------------------*/
LONG
hwcContextDWORDNT(hwcExtRequest_t *req, hwcExtResult_t *res, PDEV *ppdev)
{
  ULONG       ulIndex;
  ULONG       ulSize = _MAX_CONTEXT_INFO * sizeof(ULONG);
  ULONG       ulInputBuffer[2];
  ULONG       *pulProcessFlag;
  ULONG       ReturnedDataLength;
  HANDLE      curPID;
  GLIDESTATE  *pGS;


	// check to see if the non-cached pool memory is allocated, if not then allocate it.
	// this is only to be done once.
  if (pvMiscNonCachedMemory == NULL)
  {
    // Call the miniport via an IOCTL to allocated the NON-cached memory
    if (EngDeviceIoControl(ppdev->hDriver,
                           IOCTL_ALLOCATE_NONCACHED_MEMORY,
                           &ulSize,  // input buffer
                           sizeof(ULONG),
                           &pvMiscNonCachedMemory,
                           sizeof(void *),
                           &ReturnedDataLength))
    {
      DISPDBG((0, "hwcContextDWORDNT - Failed ALLOCATE_NONCACHED_MEMORY"));
      res->resStatus = 0;
      return 0;
    }
    DISPDBG((GLIDE_DBG_LEVEL, "  pvmMiscNonCachedMemory = %8lXh", pvMiscNonCachedMemory));
    memset(&sContextInfo[0], 0, sizeof(sContextInfo));
  }

  curPID = getCurrentProcessId(ppdev);
  if ((HANDLE)-1 == curPID)
  {
    DISPDBG((0, "hwcContextDWORDNT - getCurrentProcessId failed!!!"));
    DBG_BREAK;
    res->resStatus = 0;
    return 0;
  }
#if DBG
  if (curPID == (HANDLE)req->optData.contextDwordNTReq.procId)
    DISPDBG((GLIDE_DBG_LEVEL, "  PID passed by glide matches PID found by miniport.  (PID=%8lXh)", curPID));
  else
    DISPDBG((GLIDE_DBG_LEVEL, "  PID passed by glide (%8lXh) doesn't match PID found by miniport (%8lXh)!!!",
             req->optData.contextDwordNTReq.procId, curPID));
#endif
  pGS = hwcGetGlideStateStructureForProcess(curPID);
  if (NULL == pGS)
  {
    DISPDBG((0, "hwcContextDWORDNT - No GLIDESTATE struct found for process id %lXh!!!", curPID));
    DBG_BREAK;
    res->resStatus = 0;
    return 0;
  }
#if DBG
  if (req->contextID != (FxU32)pGS)
    DISPDBG((GLIDE_DBG_LEVEL, "  context passed in by glide (%8lXh) not the same as pGS (%8lXh)",
             req->contextID, pGS));
#endif

  // verify that the process does not already exist
  for (ulIndex = 0; ulIndex < _MAX_CONTEXT_INFO; ulIndex++)
  {
    // check to make sure that the process handle doesn't already exist
    if (sContextInfo[ulIndex].ulProcessID == (ULONG)curPID)
    {
      // return the index into the LFB that the DWORD exists at
      res->optData.contextDwordNTRes.dwordOffset = (ULONG)sContextInfo[ulIndex].pulContextDWORD;
      res->resStatus = 1;
      return 1;
    }
  }

  // find an open slot in the array
  for (ulIndex = 0; ulIndex < _MAX_CONTEXT_INFO; ulIndex++)
  {
    // check to make sure that the process handle doesn't already exist
    if (sContextInfo[ulIndex].pulContextDWORD == NULL)
    {
      ulInputBuffer[0] = (ULONG)((ULONG *)pvMiscNonCachedMemory + ulIndex);
      ulInputBuffer[1] = sizeof(ULONG);
			
      // Call the miniport via an IOCTL to map contextDWORD to current process
      if (EngDeviceIoControl(ppdev->hDriver,
                             IOCTL_MAP_MEMORY_TO_CURRENT_PROCESS,
                             &ulInputBuffer,  // input buffer
                             sizeof(ulInputBuffer),
                             &pulProcessFlag,
                             sizeof(void *),
                             &ReturnedDataLength))
      {
        DISPDBG((0, "hwcContextDWORDNT - Failed IOCTL_MAP_MEMORY_TO_CURRENT_PROCESS"));
        res->resStatus = 0;
        return 0;
      }

      sContextInfo[ulIndex].pulContextDWORD = (ULONG *)pulProcessFlag;
      sContextInfo[ulIndex].ulProcessID = (ULONG)curPID;
      sContextInfo[ulIndex].ppdev = ppdev;

      pGS->contextDWORDNTIndex = ulIndex;

      DISPDBG((GLIDE_DBG_LEVEL, "  ulIndex = %ld, pulContextDWORDNT = %8lXh", ulIndex, pulProcessFlag));

      // return the index into the LFB that the DWORD exists at
      res->optData.contextDwordNTRes.dwordOffset = (ULONG)pulProcessFlag;

      // return
      res->resStatus = 1;
      return 1;
    }
  }

  // no open slots, so fail
  res->resStatus = 0;
  return 0;
} /* hwcContextDWORDNT */

/*----------------------------------------------------------------------
Function name:  hwcSetContextDWORD

Description:

Information:

Return:
----------------------------------------------------------------------*/
VOID hwcSetContextDWORD( VOID )
{
	ULONG ulIndex;
	PDEV * ppdev;

	// 
	if( pvMiscNonCachedMemory )
	{
		for( ulIndex = 0x00; ulIndex < _MAX_CONTEXT_INFO; ulIndex++ )	
		{
			*( ( ULONG * )pvMiscNonCachedMemory + ulIndex ) = TRUE;
		}
	}

	return;

} /* hwcSetContextDWORD */

#ifdef ENABLE_V3_W2K_GLIDE_CHANGES
/*----------------------------------------------------------------------
Function name:  hwcSetContextDWORDHighBit

Description:Sets the high bit in the context dword so that GLIDE will know to re-lock
			its ddraw surfaces

Information:

Return:
----------------------------------------------------------------------*/
VOID hwcSetContextDWORDHighBit( VOID )
{
	ULONG ulIndex;
	PDEV * ppdev;

	// 
	if( pvMiscNonCachedMemory )
	{
		for( ulIndex = 0x00; ulIndex < _MAX_CONTEXT_INFO; ulIndex++ )	
		{
			*( ( ULONG * )pvMiscNonCachedMemory + ulIndex ) |= 0x80000000;
		}
	}

	return;

} /* hwcSetContextDWORDHighBit */
#endif

/*----------------------------------------------------------------------
Function name:  hwcPCIOP

Description:    PCI Operation

Information:

Return:         LONG    1 is always returned.
----------------------------------------------------------------------*/
static LONG
hwcPCIOP(hwcExtRequest_t *req, hwcExtResult_t *res, PDEV *ppdev)
{
#ifdef SLI_AA
  HWPCIOP     HwPCIOp;
  DWORD       numBytes;
  HANDLE      curPID;
  GLIDESTATE  *pGS;


  curPID = getCurrentProcessId(ppdev);
  if ((HANDLE)-1 == curPID)
  {
    DISPDBG((0, "hwcPCIOP - getCurrentProcessId failed!!!"));
    DBG_BREAK;
    res->resStatus = 0;
    return 0;
  }
  pGS = hwcGetGlideStateStructureForProcess(curPID);
  if (NULL == pGS)
  {
    DISPDBG((0, "hwcPCIOP - No GLIDESTATE struct found for process id %lXh!!!", curPID));
    DBG_BREAK;
    res->resStatus = 0;
    return 0;
  }
#if DBG
  if (req->contextID != (FxU32)pGS)
    DISPDBG((GLIDE_DBG_LEVEL, "  context passed in by glide (%8lXh) not the same as pGS (%8lXh)",
             req->contextID, pGS));
#endif
  if (! (pGS->glideGDIFlags & GLDATA_GDIFLAGS_HWC_EXCLUSIVE))
  {
    DISPDBG((0, "hwcPCIOP - not in hwc exclusive mode!!! (curPID=%lXh, pGS=%8lXh)", curPID, pGS));
    DBG_BREAK;
    res->resStatus = 0;
    return 0;
  }

#if DBG
  DISPDBG((GLIDE_DBG_LEVEL, "  Operation=HWCEXT_PCI_%s", (req->optData.pciOpReq.Operation == HWCEXT_PCI_READ) ? "READ" : "WRITE"));
  DISPDBG((GLIDE_DBG_LEVEL, "  DeviceId =%lXh", req->optData.pciOpReq.DeviceId));
  DISPDBG((GLIDE_DBG_LEVEL, "  Offset   =%lXh", req->optData.pciOpReq.Offset));
  if (HWCEXT_PCI_WRITE == req->optData.pciOpReq.Operation)
    DISPDBG((GLIDE_DBG_LEVEL, "  Value    =%lXh", req->optData.pciOpReq.Value));
#endif

  HwPCIOp.dwOp = (HWCEXT_PCI_READ == req->optData.pciOpReq.Operation) ? H3G_PCI_READ: H3G_PCI_WRITE;
  HwPCIOp.dwFunc = req->optData.pciOpReq.DeviceId;
  HwPCIOp.dwOffset = req->optData.pciOpReq.Offset;
  HwPCIOp.dwValue = req->optData.pciOpReq.Value;

  // call the miniport
  if (EngDeviceIoControl(ppdev->hDriver,
                         IOCTL_3DFX_PCI_OP,
                         &HwPCIOp,
                         sizeof(HwPCIOp),
                         &HwPCIOp,
                         sizeof(HwPCIOp),
                         &numBytes))
  {
    DISPDBG((GLIDE_DBG_LEVEL, "Failed IOCTL_3DFX_PCI_OP"));
    res->resStatus = 0;
    return 0;
  }
  else
  {
#if DBG
    if (HWCEXT_PCI_READ == req->optData.pciOpReq.Operation)
      DISPDBG((GLIDE_DBG_LEVEL, "  Value    =%lXh", req->optData.pciOpReq.Value));
#endif
    res->optData.pciOpRes.Value = HwPCIOp.dwValue;
    res->resStatus = 1;
    return 1;
  }

  return 1;
#else
  res->resStatus = 0x0;
  return 0x0;
#endif
} /* hwcPCIOP */

/*----------------------------------------------------------------------
Function name:  hwcGetSlaveRegs

Description:    Get Slave Registers
Information:

Return:         LONG    1 is always returned.
----------------------------------------------------------------------*/
static LONG
hwcGetSlaveRegs(hwcExtRequest_t *req, hwcExtResult_t *res, PDEV *ppdev)
{
#ifdef SLI_AA
  int         i;
  int         unitNumber;
  HANDLE      curPID;
  GLIDESTATE  *pGS;


  curPID = getCurrentProcessId(ppdev);
  if ((HANDLE)-1 == curPID)
  {
    DISPDBG((0, "hwcGetSlaveRegs - getCurrentProcessId failed!!!"));
    DBG_BREAK;
    res->resStatus = 0;
    return 0;
  }
  pGS = hwcGetGlideStateStructureForProcess(curPID);
  if (NULL == pGS)
  {
    DISPDBG((0, "hwcGetSlaveRegs - No GLIDESTATE struct found for process id %lXh!!!", curPID));
    DBG_BREAK;
    res->resStatus = 0;
    return 0;
  }
#if DBG
  if (req->contextID != (FxU32)pGS)
    DISPDBG((GLIDE_DBG_LEVEL, "  context passed in by glide (%8lXh) not the same as pGS (%8lXh)",
             req->contextID, pGS));
#endif

  // what's in DeviceId, a slave chip function number?  I hope!
  unitNumber = req->optData.slaveRegReq.DeviceId;
  DISPDBG((GLIDE_DBG_LEVEL, "  deviceId = %lXh", req->optData.slaveRegReq.DeviceId));

  for (i = 0; i < HWCEXT_MAX_SLAVE_REGS; i++)
    res->optData.slaveRegRes.Regs[i] =
              (ULONG)pGS->glideSlaveRegBase[(unitNumber * HWINFO_SST_MAX_CHIP_INDEX) + i];

  res->resStatus = 1;
  return 1;
#else
  res->resStatus = 0;
  return 0;
#endif
} /* hwcGetSlaveRegs */

/*----------------------------------------------------------------------
Function name:  hwcSliAARequest

Description:    Call miniport to enable/disable SLI/AA
Information:

Return:         LONG    1 is always returned.
----------------------------------------------------------------------*/
static LONG
hwcSliAARequest(hwcExtRequest_t *req, hwcExtResult_t *res, PDEV *ppdev)
{
#ifdef SLI_AA
  ULONG           ReturnedDataLength ;
  SLI_AA_REQUEST  theRequest ;
  HANDLE          curPID;
  GLIDESTATE      *pGS;


  curPID = getCurrentProcessId(ppdev);
  if ((HANDLE)-1 == curPID)
  {
    DISPDBG((0, "hwcSliAARequest - getCurrentProcessId failed!!!"));
    DBG_BREAK;
    res->resStatus = 0;
    return 0;
  }
  pGS = hwcGetGlideStateStructureForProcess(curPID);
  if (NULL == pGS)
  {
    DISPDBG((0, "hwcSliAARequest - No GLIDESTATE struct found for process id %lXh!!!", curPID));
    DBG_BREAK;
    res->resStatus = 0;
    return 0;
  }
#if DBG
  if (req->contextID != (FxU32)pGS)
    DISPDBG((GLIDE_DBG_LEVEL, "  context passed in by glide (%8lXh) not the same as pGS (%8lXh)",
             req->contextID, pGS));
#endif
  if (! (pGS->glideGDIFlags & GLDATA_GDIFLAGS_HWC_EXCLUSIVE))
  {
    DISPDBG((0, "hwcSliAARequest - not in hwc exclusive mode!!! (curPID=%lXh, pGS=%8lXh)", curPID, pGS));
    DBG_BREAK;
    res->resStatus = 0;
    return 0;
  }

  /*
   * This mess is here in case the SLI_AA_REQUEST structure
   * someday gets out of sync with the hwcExtRequest_t structure
   */
  theRequest.ChipInfo.dwChips = req->optData.sliAAReq.ChipInfo.dwChips ;
  theRequest.ChipInfo.dwsliEn = req->optData.sliAAReq.ChipInfo.dwsliEn ;
  theRequest.ChipInfo.dwaaEn = req->optData.sliAAReq.ChipInfo.dwaaEn ;
  theRequest.ChipInfo.dwaaSampleHigh = req->optData.sliAAReq.ChipInfo.dwaaSampleHigh ;
  theRequest.ChipInfo.dwsliAaAnalog = req->optData.sliAAReq.ChipInfo.dwsliAaAnalog ;
  theRequest.ChipInfo.dwsli_nlines = req->optData.sliAAReq.ChipInfo.dwsli_nlines ;
  theRequest.ChipInfo.dwCfgSwapAlgorithm = req->optData.sliAAReq.ChipInfo.dwCfgSwapAlgorithm ;

  theRequest.MemInfo.dwTotalMemory = req->optData.sliAAReq.MemInfo.dwTotalMemory ;
  theRequest.MemInfo.dwTileMark = req->optData.sliAAReq.MemInfo.dwTileMark ;
  theRequest.MemInfo.dwTileCmpMark = req->optData.sliAAReq.MemInfo.dwTileCmpMark ;
  theRequest.MemInfo.dwaaSecondaryColorBufBegin =
    req->optData.sliAAReq.MemInfo.dwaaSecondaryColorBufBegin ;
  theRequest.MemInfo.dwaaSecondaryDepthBufBegin = 
    req->optData.sliAAReq.MemInfo.dwaaSecondaryDepthBufBegin ;
  theRequest.MemInfo.dwaaSecondaryDepthBufEnd = req->optData.sliAAReq.MemInfo.dwaaSecondaryDepthBufEnd ;
  theRequest.MemInfo.dwBpp = req->optData.sliAAReq.MemInfo.dwBpp ;

  if (req->optData.sliAAReq.ChipInfo.dwaaEn ||
      req->optData.sliAAReq.ChipInfo.dwsliEn)
  {
    DISPDBG((0, "hwcSliAARequest - Glide is entering SLI or AA mode"));

    if (EngDeviceIoControl(ppdev->hDriver,
                           IOCTL_3DFX_SLI_AA_ENABLE,
                           &theRequest,
                           sizeof(SLI_AA_REQUEST),
                           NULL,
                           0,
                           &ReturnedDataLength))
    {
      DISPDBG((0, "hwcSliAARequest - IOCTL_3DFX_SLI_AA_ENABLE failed"));
      res->resStatus = 0;
    }
    else
    {
      res->resStatus = 1;
    }
  }
  else
  {
    DISPDBG((0, "hwcSliAARequest - Glide is leaving SLI or AA mode"));

    if (EngDeviceIoControl(ppdev->hDriver,
                           IOCTL_3DFX_SLI_AA_DISABLE,
                           &theRequest,
                           sizeof(SLI_AA_REQUEST),
                           NULL,
                           0,
                           &ReturnedDataLength))
    {
      DISPDBG((0, "hwcSliAARequest - IOCTL_3DFX_SLI_AA_DISABLE failed"));
      res->resStatus = 0;
    }
    else
    {
      res->resStatus = 1;
    }
  }

  return res->resStatus;
#else
  res->resStatus = 0;
  return 0;
#endif
}

/*----------------------------------------------------------------------
Function name:  hwcExt

Description:    Massive switch statement that parses/processes
                the Extended request.
Information:

Return:         LONG    Value of the subsequently called function.
----------------------------------------------------------------------*/
LONG
hwcExt(DWORD *lpInput, DWORD *lpOutput, PDEV *ppdev)
{
  hwcExtRequest_t *req = (hwcExtRequest_t *) lpInput;
  hwcExtResult_t  *res = (hwcExtResult_t *) lpOutput;

#if DBG
  if (req->which < (sizeof(extNames)/sizeof(extNames[0])))
    DISPDBG((GLIDE_DBG_LEVEL, "hwcExt(%s)", extNames[req->which]));
  else
    DISPDBG((GLIDE_DBG_LEVEL, "hwcExt(which=%lXh)", req->which));
#endif

  switch (req->which) {
  case HWCEXT_GETDRIVERVERSION:
    return hwcGetDriverVersion(req, res);
    break;

  case HWCEXT_ALLOCCONTEXT:
    return hwcAllocContext(req, res, ppdev);
    break;

  case HWCEXT_GETDEVICECONFIG:
    return hwcGetDeviceConfig(req, res, ppdev);
    break;

  case HWCEXT_GETLINEARADDR:
    return hwcGetLinearAddr(req, res, ppdev);
    break;

  case HWCEXT_ALLOCFIFO:
    return hwcAllocFifo(req, res);
    break;

  case HWCEXT_EXECUTEFIFO:
    return hwcExecuteFifo(req, res, ppdev);
    break;

  case HWCEXT_QUERYCONTEXT:
    return hwcQueryContext(req, res, ppdev);
    break;

  case HWCEXT_RELEASECONTEXT:
    return hwcReleaseContext(req, res, ppdev);
    break;

  case HWCEXT_HWCSETEXCLUSIVE:
    return hwcSetEsclusive(req, res, ppdev);
    break;

  case HWCEXT_HWCRLSEXCLUSIVE:
    return hwcRlsEsclusive(req, res, ppdev);
    break;

#if (_WIN32_WINNT >= 0x0500) && defined (AGP_CMDFIFO)
  case HWCEXT_GETAGPINFO:
    return hwcAGPInfo(req, res, ppdev);
    break;
#endif

  case HWCEXT_FIFOINFO:
    return hwcFifoInfo(req, res, ppdev);
    break;

  case HWCEXT_LINEAR_MAP_OFFSET:
    return hwcLinearMapOffset(req, res, ppdev);
    break;

  case HWCEXT_DOWNLOAD_GAMMA:
    return hwcDownloadGamma(req, res, ppdev);
    break;

  case HWCEXT_SHARE_CONTEXT_DWORD:
	return hwcShareContextDWORD(req, res, ppdev);
	break;

  case HWCEXT_UNMAP_MEMORY:
	return hwcUnMapMemory(req, res, ppdev);
	break;

  case HWCEXT_CONTEXT_DWORD_NT:
    return hwcContextDWORDNT(req, res, ppdev);
	break;

  case HWCEXT_PCI_OP:
    return hwcPCIOP(req, res, ppdev);
  break;

  case HWCEXT_GET_SLAVE_REGS:
    return hwcGetSlaveRegs(req, res, ppdev);
  break;

#ifdef ENABLE_V3_W2K_GLIDE_CHANGES
  case HWCEXT_PROTECT_CMD_FIFO:
    return hwcProtectCmdFifo( req, res, ppdev );
  break;
#endif
 
  case HWCEXT_SLI_AA_REQUEST:
    return hwcSliAARequest(req, res, ppdev) ;
  break ;

  default:
    return -1;
    break;
  }

  return 1;

} /* hwcExt */


/*----------------------------------------------------------------------
Function name:  hwcShareCPUType

Description:    Shares the CPU Type with the calling process

Information:

Return:         LONG    1 is always returned.
----------------------------------------------------------------------*/
LONG
hwcShareCPUType(DWORD *req, DWORD *res, PDEV *ppdev)
{
   DWORD TempDWORD;

   TempDWORD = ppdev->cpuType;

   *res = TempDWORD;

   return 1;
}
