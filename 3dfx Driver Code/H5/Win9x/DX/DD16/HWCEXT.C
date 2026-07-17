
/** $Header: hwcext.c, 10, 10/11/00 8:51:21 PM, Brent$ */
/*
** Copyright (c) 1996-1999, 3Dfx Interactive, Inc.
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
** File name:   hwcext.c
**
** Description: HW extension support functions for applets and glide.
**
** $Log: 
**  10   3dfx      1.7.1.1     10/11/00 Brent           Forced check in to enforce
**       branching.
**  9    3dfx      1.7.1.0     09/26/00 Matt McClure    Added Auxilary Exclusive
**       surface Escapes
**  8    3dfx      1.7         02/25/00 Christopher Wilcox Cleanup and
**       reorganization in minihwc functions in display driver.
** 
**  7    3dfx      1.6         12/17/99 Adam Briggs     Changed
**       FXGLIDESTATEINFO.EntryActive to be a reference count instead of a flag.
**       This keeps threads from hosing each other within a process.
**  6    3dfx      1.5         11/11/99 Steve Houston   Added new 2D memory manager
**       that improves Business WB99 score and removes the need for the hostify
**       code. Undefine PERF_NEWMM in build\stbperf.inc to switch back to the old
**       memory manager.
** 
**  5    3dfx      1.4         10/07/99 Christopher Wilcox Very minor fix for to
**       allow build without CMDFIFO.
** 
**  4    3dfx      1.3         10/04/99 Matt McClure    Ported changes for Glide
**       Context Switching from V3_OEM_100.
**  3    3dfx      1.2         09/21/99 Matt McClure    Ported Fix from V3_OEM_100
**       for PRS 6666 - Katmai detection failing in OpenGL on Windows 95. 
**  2    3dfx      1.1         09/21/99 Christopher Wilcox Cleaned up unusual
**       switch statement construct to fix compiler warnings.
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
** 
** 52    8/25/99 2:09p Lpost
** V3TV Merge into H5
** 
** 51    8/06/99 6:22p Andrew
** Added a function to get the slave registers
** 
** 50    8/06/99 4:40p Andrew
** Added new members to structures and new Glide PCI Functions
** 
** 49    7/16/99 12:45p Andrew
** changed regBase from single dword to array
** 
** 48    6/17/99 2:26p Andrew
** Removed some unused Global Variables
** 
** 47    6/10/99 8:04a Andrew
** Set the return status in hwcRlsExclusive which fixes 6340 and 6449.
** 
** 45    5/27/99 1:23p Andrew
** Removed mode reset in HwcRlsExclusive
** 
** 44    5/17/99 1:43p Stb_lpost
** V3TV Video Capture fixes for E3 Demo
** 
** 43    4/06/99 12:39a Andrew
** Changed RestoreDesktop and adding pumbing for UnGlideContext to attempt
** to fix Alt-Tab
** 
** 42    4/04/99 4:13p Sreid
** Added mechanism to coordinate loss of context between Glide and DD
** 
** 41    4/04/99 4:13p Sreid
** 
** 40    4/01/99 11:54a Andrew
** Added code to use new shared names and to return state of exclusive
** flag
** 
** 39    3/22/99 2:43p Xingc
** Add function hwcOvlAddr() to pass current overlay data out.
** 
** 38    3/19/99 6:07p Andrew
** Added code to return TRUE when we go into Low Power Mode
** 
** 37    3/04/99 12:52p Edwin
** fix PRS 4654: desktop cursor disappeared after exiting Unreal.
** hwcRestoreDesktop() needs to restore the saved cursor.
** 
** 36    2/26/99 10:21a Michael
** Proxy for KenW - These changes make alt-tab from winglide apps *much*
** more reliable.  I now see no corruption or hangs from glquake or
** heretic2.   NBA98 also worked.  PRS 3320 is possibly associated.  There
** may be others.
** 
** 35    2/05/99 4:37p Peter
** agp fifo offset
** 
** 34    1/08/99 11:05a Stuartb
** Backed out LCDCTRL changes as advised by KMW.
** 
** 33    1/07/99 1:24p Stuartb
** Another oops; Added HWCEXT_LCDCTRL for control panel flat panel ops.
** 
** 32    1/07/99 1:08p Stuartb
** oops
** 
** 31    1/07/99 12:18p Stuartb
** Added HWCEXT_LCDCTRL for control panel flat panel ops.
** 
** 30    1/04/99 11:58a Peter
** added windowed context support
** 
** 29    12/29/98 1:18p Michael
** Implement the 3Dfx/STB unified header.
** 
** 28    11/30/98 6:52p Peter
** extended execute fifo for video memory fifo's
** 
** 27    11/16/98 8:53p Andrew
** Changed DeviceID & Vendor ID since Vendor is in low part of Dword
** 
** 26    11/16/98 8:35p Andrew
** Added code to return the real Device ID, Vendor ID, and Revision ID.
** 
** 25    10/29/98 2:13p Martin
** Modification of cmdFifo macros to support future changes for
** super-sampling AA.
** 
** 24    9/11/98 10:44p Jdt
** Clean up some warnings, fix software cursor collision, minor
** optimization to data copy.
** 
** 23    9/02/98 3:29p Andrew
** Rest of fix for unreal & sw cursor
** 
** 22    8/28/98 11:54p Edwin
** Fix 2395, repaint screen as we exit Glide games.  Unreal should not
** cause desktop background to go black now.
** 
** 21    8/27/98 6:28p Andrew
** Added gamma correction
** 
** 20    8/27/98 2:25p Artg
** added ifdef cmdfifo around body of hwcexecutefifo for direct write
** compile
** 
** 19    8/03/98 6:33a Jdt
** Added code to notify D3D when I slam their hw state.
** 
** 18    7/24/98 2:01p Dow
** Added AGP Stuff
** 
** 17    7/23/98 4:17p Dow
** Return chipRev 3 for B*
** 
** 16    7/20/98 1:30p Jdt
** Increased command buffer size.
** 
** 15    7/18/98 12:20a Jdt
** Added state restoration buffer
** 
** 14    7/16/98 9:11a Andrew
** Last checkin did not restore deFlags.
** 
** 13    7/16/98 7:32a Andrew
** Added some code to restore SaveUnder buffer and reload the cursor when
** exclusive mode exits
** 
** 12    7/13/98 5:20p Jdt
** Added first primitive command buffer allocation and execution code
** for GlideWin
** 
** 
** 10    6/29/98 7:01p Dow
** Mucked around with hwcRlsExclusive
** 
** 9     6/29/98 6:50p Dow
** Added call to HWSetMode to hwcRslExclusive
** 
** 8     6/23/98 10:12a Stuartb
** Added I2C multibyte writes.
** 
** 7     6/16/98 3:54p Michael
** Make sure all device bitmaps are hostified in hwcSetExclusive().  Fixes
** #1650.
** 
** 6     5/20/98 8:13p Dow
** Device rev
** 
** 5     5/05/98 1:19p Stuartb
** Adding i2c extEscapes.
** 
** 4     4/28/98 3:57p Michael
** change Msg() to include a new parameter that will allow for selective
** debug message output.
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

#include "header.h"
#include "3dfx.h"
#include "hwcext.h"
#include "minivdd.h"
#include "cursor.h"

#ifdef INCSTBPERF
#include "..\build\stbperf.inc"
#endif

// Until we add these defines to hwcext.h

#define HWCEXT_SHARE_CONTEXT    0X15UL
#define HWCEXT_UNSHARE_CONTEXT  0X16UL

// LOCAL PROTOTYPES

static LONG hwcAGPInfo (hwcExtRequest_t *req, hwcExtResult_t *res);
static LONG hwcFifoInfo(hwcExtRequest_t* req, hwcExtResult_t *res);

// EXTERN PROTOTYPES

extern void LoadGamma(void);
extern void DoAllHost(void);
extern void DisableDeviceBitmaps(void);
#ifdef PERF_NEWMM
extern void CacheDepopulate();
#endif

/* xhwcext.asm */
extern BOOL hwcSetGDIBusy( WORD FAR *lpGDISemaphore );
extern void hwcClearGDIBusy( WORD FAR *lpGDISemaphore );

extern DWORD dwDeviceHandle;

static char *extNames[] = {
  "HWCEXT_GETDRIVERVERSION",      // 0x00 OBSOLETE
  "HWCEXT_ALLOCCONTEXT",          // 0x01 REQUIRED
  "HWCEXT_GETDEVICECONFIG",       // 0x02 REQUIRED
  "HWCEXT_GETLINEARADDR",         // 0x03 REQUIRED
  "HWCEXT_ALLOCFIFO",             // 0x04 OBSOLETE
  "HWCEXT_EXECUTEFIFO",           // 0x05 REQUIRED
  "HWCEXT_QUERYCONTEXT",          // 0x06 OBSOLETE
  "HWCEXT_RELEASECONTEXT"         // 0x07 REQUIRED
  "HWCEXT_HWCSETEXCLUSIVE",       // 0x08 REQUIRED
  "HWCEXT_HWCRLSEXCLUSIVE",       // 0x09 REQUIRED
  "HWCEXT_I2C_WRITE_REQ",         // 0x0A UNKNOWN
  "HWCEXT_I2C_READ_REQ",          // 0x0B UNKNOWN
  "HWCEXT_I2C_READ_RES",          // 0x0C UNKNOWN
  "HWCEXT_I2C_MULTI_WRITE_REQ",   // 0x0D UNKNOWN
  "HWCEXT_GETAGPINFO",            // 0x0E REQUIRED
  "HWCEXT_VIDTIMING",             // 0x0F UNKNOWN
  "HWCEXT_FIFOINFO",              // 0x10 REQUIRED
  "HWCEXT_LINEAR_MAP_OFFSET",     // 0x11 REQUIRED
  "HWCEXT_DOWNLOAD_GAMMA",        // 0x12 UNKNOWN
  "HWCEXT_RESTORE_DESKTOP",       // 0x13 OBSOLETE
  "HWCEXT_OVERLAY_DATA",          // 0x14 UNKNOWN
  "HWCEXT_SHARE_CONTEXT",         // 0x15 REQUIRED
  "HWCEXT_UNSHARE_CONTEXT",       // 0x16 REQUIRED
  "HWCEXT_CONTEXT_DWORD_NT",      // 0x17 REQUIRED (NT only)
  "HWCEXT_PCI_OP",                // 0x18 REQUIRED (Napalm)
  "HWCEXT_GET_SLAVE_REGS",        // 0x19 REQUIRED (Napalm)
};


// SELECTOR FUNCTIONS

#define DPMI_INT  0x31
#define ALLOC_SEL 0x0
#define FREE_SEL  0x1
#define GET_BASE  0x6
#define SET_BASE  0x7
#define SET_LIMIT 0x8

/*----------------------------------------------------------------------
Function name:  allocSelector

Description:    Allocate a Selector using INT 31h.

Return:         WORD     Value of the Selector.
----------------------------------------------------------------------*/

#pragma warning( disable : 4704 ) 
static WORD allocSelector( void ) 
{
    WORD rv;
    _asm {
        mov ax, ALLOC_SEL
        mov cx, 1
        int DPMI_INT
        jb  fubar
        mov word ptr rv, ax
        jmp notfubar
fubar:
        mov word ptr rv, 0
notfubar:
    }
    return rv;
}


/*----------------------------------------------------------------------
Function name:  freeSelector

Description:    Free a previously allocated Selector using INT 31h.

Return:         WORD    1 for success or 0 for failure.
----------------------------------------------------------------------*/
static WORD freeSelector( WORD selector ) 
{
    WORD rv;
    _asm {
        mov ax, FREE_SEL
        mov bx, word ptr selector
        int DPMI_INT
        jb  fubar
        mov word ptr rv, 1
        jmp notfubar
fubar:
        mov word ptr rv, 0
notfubar:
    }
    return rv;
}


/*----------------------------------------------------------------------
Function name:  setSelectorAddr

Description:    Set a previously allocated Selector's base
                address and limit using INT 31h.

Return:         WORD    1 for success or 0 for failure.
----------------------------------------------------------------------*/
static WORD setSelectorAddr( WORD selector, DWORD base, WORD limit ) 
{
    WORD rv;
    _asm {
        mov ax, SET_BASE
        mov bx, word ptr selector
        mov cx, word ptr [base+2]
        mov dx, word ptr base
        int DPMI_INT
        jb  fubar
        mov ax, SET_LIMIT
        mov bx, word ptr selector
        mov cx, 0
        mov dx, word ptr limit
        int DPMI_INT
        jb  fubar
        mov word ptr rv, 1
        jmp notfubar
fubar:
        mov word ptr rv, 0
notfubar:        
    }
    return rv;
}

/*----------------------------------------------------------------------
Function name:  getSelectorBase

Description:    Return the base address of a previously allocated
                Selector using INT 31h.

Return:         DWORD    Base address of the selector.
----------------------------------------------------------------------*/
static DWORD getSelectorBase( WORD selector ) 
{
    DWORD rv;
    _asm {
        mov ax, GET_BASE
        mov bx, word ptr selector
        int DPMI_INT
        jb fubar
        mov word ptr rv, dx;
        mov word ptr [rv+2], cx;
        jmp notfubar
fubar:
        mov word ptr rv, 0;
        mov word ptr [rv+2], 0;
notfubar:
    }
    return rv;
}
// LOCAL FUNCTIONS

/*----------------------------------------------------------------------
Function name:  hwcGlideNodeAllocate

Description:    Sets a Glide context to active and returns the pointer

Return:         FXGLIDESTATEINFO Pointer to new node
----------------------------------------------------------------------*/
FXGLIDESTATEINFO *
hwcGlideNodeAllocate (FxU32 procID)
{
  int index; 


  // Find Glide context for specified process ID, and increment thread active count.

  index = 0;
  while ( index < MAX_GLIDE_STATES )
  {
    if ( (_FF(GlideContext [ index ].procID) == procID) && (_FF(GlideContext [ index ].EntryActive) > 0)) 
    {
      _FF(GlideContext [ index ].EntryActive)++ ;
      return (FXGLIDESTATEINFO *)&(_FF(GlideContext [ index ] ));
    }
    index ++;
  }

  // Loop through Glide context array, find a new context for process ID.

  index = 0;
  while (index < MAX_GLIDE_STATES )
  {
    if ( _FF(GlideContext [ index ].EntryActive) == 0 )
	{
      _FF(GlideContext [ index ].EntryActive ) = 1;
      _FF(GlideContext [ index ].lostContext ) = 1;
      _FF(GlideContext [ index ].procID ) = procID;

      return (FXGLIDESTATEINFO *)&(_FF(GlideContext [ index ] ));
	}
    index ++;
  }

  // No more Glide contexts available!

  return (FXGLIDESTATEINFO *)&(_FF(GlideContext [ MAX_GLIDE_STATES -1 ]));
}

/*----------------------------------------------------------------------
Function name:  hwcGlideNodeFree

Description:    Sets a Glide context to inactive.

Return:         FXGLIDESTATEINFO Pointer to new node
----------------------------------------------------------------------*/
void
hwcGlideNodeFree (FxU32 procID)
{
  int index;
  index = 0;

  /* Loop through the list and find the matching process ID */
  while ( index < MAX_GLIDE_STATES )
  {
    /* Make sure we have not run off the list because of an invalid process ID */
    /* If we are a valid process ID, then reset the node */
    if ( _FF(GlideContext [ index ].procID ) == procID )
	{
      _FF(GlideContext [ index ].EntryActive)-- ;
      if (_FF(GlideContext [ index ].EntryActive) == 0) 
      {
        _FF(GlideContext [ index ].procID) = 0;
        _FF(GlideContext [ index ].lostContext) = 1;
        _FF(GlideContext [ index ].EntryActive) = 0;
      }
	}
    index ++;
  }
}			

// REQUIRED ENTRY POINTS

/*----------------------------------------------------------------------
Function name:  hwcGetDeviceConfig

Description:    Obtain information about the display device.

Information:    REQUIRED

Return:         returns 1 always
----------------------------------------------------------------------*/

static LONG 
hwcGetDeviceConfig(hwcExtRequest_t *req, hwcExtResult_t *res)
{
  res->optData.deviceConfigRes.devNum     = 0;
  res->optData.deviceConfigRes.vendorID   = _FF(VendorDeviceID) & 0xFFFF;
  res->optData.deviceConfigRes.deviceID   = _FF(VendorDeviceID)>>16;
  res->optData.deviceConfigRes.fbRam      = _FF(TotalVRAM);
  res->optData.deviceConfigRes.pciStride  = _FF(ddTilePitch);
  res->optData.deviceConfigRes.hwStride   = _FF(ddTileStride);
  res->optData.deviceConfigRes.tileMark   = _FF(ddTileMark);
  res->optData.deviceConfigRes.chipRev    = _FF(RevisionID);

  // New for SLI-AA
#ifdef SLI_AA
  res->optData.deviceConfigRes.isMaster   = (_FF(dwType) != SLI_AA_SLAVE_DEVICE) ? (DWORD)TRUE : (DWORD)FALSE;
  res->optData.deviceConfigRes.numChips   = _FF(dwNumUnits);
#else
  res->optData.deviceConfigRes.isMaster   = (DWORD)TRUE;
  res->optData.deviceConfigRes.numChips   = 1;
#endif

  res->resStatus = 1;
  return 1;

} /* hwcGetDeviceConfig */

/*----------------------------------------------------------------------
Function name:  hwcGetLinearAddr

Description:    Returns the linear address of regbase, lfbbase, iobase.

Information:    REQUIRED

Return:         returns 1 always
----------------------------------------------------------------------*/
static LONG
hwcGetLinearAddr(hwcExtRequest_t *req, hwcExtResult_t *res)
{
  res->optData.linearAddressRes.numBaseAddrs = 3;
  res->optData.linearAddressRes.baseAddresses[0] = _FF(regBase[0]);
  res->optData.linearAddressRes.baseAddresses[1] = _FF(lfbBase);
  res->optData.linearAddressRes.baseAddresses[2] = _FF(ioBase);

  res->resStatus = 1;
  return 1;

} /* hwcGetLinearAddr */

/*----------------------------------------------------------------------
Function name:  hwcLinearMapOffset

Description:    Obtain the Linear Map Offset.
                
Information:    REQUIRED

Return:         LONG    1 for a valid linear address or,
                        0 for failure.
----------------------------------------------------------------------*/
static LONG
hwcLinearMapOffset(hwcExtRequest_t* req, hwcExtResult_t*  res)
{
  res->resStatus = 0;

#ifdef AGP_CMDFIFO
  {
    hwcExtResult_t agpInfo;

    if (hwcFifoInfo(NULL, &agpInfo) && 
        (agpInfo.optData.fifoInfoRes.fifoType == HWCEXT_FIFO_AGP) &&
        hwcAGPInfo(NULL, &agpInfo)) {
      
      res->resStatus = ((req->optData.mapInfoReq.remapAddr >= agpInfo.optData.agpInfoRes.lAddr) &&
                        (req->optData.mapInfoReq.remapAddr < (agpInfo.optData.agpInfoRes.lAddr +
                                                              agpInfo.optData.agpInfoRes.size)));
      if (res->resStatus) {
        res->optData.mapInfoRes.linAddrOffset = ((req->optData.mapInfoReq.remapAddr - agpInfo.optData.agpInfoRes.lAddr) +
                                                 0x2000000UL); /* Size of memBase1 */
      }
    }
  }
#endif /* AGP_CMDFIFO */

  if (!res->resStatus) {
    hwcExtLinearMapInfoReq_t 
      tempData;
    FxU32
      linAddrOffset;
    
    /* Copy the data because we aren't supposed to whack the
     * request data from the client.
     */
    tempData.mapAddr   = req->optData.mapInfoReq.mapAddr;
    tempData.remapAddr = req->optData.mapInfoReq.remapAddr;
    
    VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle, 
            H3VDD_LINEAR_MAP_OFFSET, 
            (DWORD)&tempData, &linAddrOffset);
    
    res->optData.mapInfoRes.linAddrOffset = linAddrOffset;
    res->resStatus = (linAddrOffset != 0x00UL);
  }
  
  return res->resStatus;
}

/*----------------------------------------------------------------------
Function name:  hwcSetExclusive

Description:    Glide sets exclusive mode (Glide owns device!).
                
Information:    REQUIRED

Return:         returns 1 always
----------------------------------------------------------------------*/
static LONG
hwcSetExclusive(hwcExtRequest_t *req, hwcExtResult_t *res) 
{
  DPF(DBGLVL_NORMAL,"hwcSetExclusive: Setting flags");

  _FF(gdiFlags) |= SDATA_GDIFLAGS_HWC_EXCLUSIVE;
  
  // Initialize all entries in the glide context list to lost context
  // being true.
  {
    int index; 

    for ( index = 0; index < MAX_GLIDE_STATES; index ++ )
	{
      _FF(GlideContext [ index ].lostContext) = 1;
	}
  }

#ifdef PERF_NEWMM
  CacheDepopulate();        // move all device bitmaps to host
#else
  DoAllHost();              // move all device bitmaps to host
#endif 
  DisableDeviceBitmaps();   // disallow future device bitmaps

  res->resStatus = 1;
  return 1;
  
} /* hwcSetExclusive */

/*----------------------------------------------------------------------
Function name:  hwcRlsExclusive

Description:    Glide releases exclusive mode (Windows owns device!)
                
Information:

Return:         LONG    1 is always returned.
----------------------------------------------------------------------*/
static LONG
hwcRlsExclusive(hwcExtRequest_t *req, hwcExtResult_t *res) 
{
  DPF(DBGLVL_NORMAL,"hwcRlsExclusive: Clearing flags");

  _FF(gdiFlags) &= ~SDATA_GDIFLAGS_HWC_EXCLUSIVE;

  // Initialize all entries in the glide context list to lost context
  // being true.
  {
    int index; 

    for ( index = 0; index < MAX_GLIDE_STATES; index ++ )
	{
      _FF(GlideContext [ index ].lostContext) = 1;
	}
  }

  res->resStatus = 1;
  return 1;

} /* hwcRlsExclusive */

/*----------------------------------------------------------------------
Function name:  hwcAllocContext

Description:    Allocate a Glide context.

Information:    REQUIRED

Return:         returns -1 always
----------------------------------------------------------------------*/
static LONG
hwcAllocContext(hwcExtRequest_t *req, hwcExtResult_t *res)
{
  static int contextID;

  if (req->optData.allocContextReq.protocolRev != HWCEXT_PROTOCOLREV) {
    res->resStatus = -1;
    return res->resStatus;
  }
  
  res->optData.allocContextRes.contextID = ++contextID;

  res->resStatus = 1;
  return 1;

} /* hwcAllocContext */

/*----------------------------------------------------------------------
Function name:  hwcReleaseContext

Description:    Release the HW Context
                
Information:    Presently unused!

Return:         returns 1 always
----------------------------------------------------------------------*/
static LONG
hwcReleaseContext(hwcExtRequest_t *req, hwcExtResult_t *res)
{

  res->resStatus = 1; 
  return 1;

} /* hwcReleaseContext */

/*----------------------------------------------------------------------
Function name:  hwcShareContext

Description:    Frees a Glide context in shared memory.
                
Information:    REQUIRED

Return:         LONG    1 if the operation succeeded, -1 otherwise.
----------------------------------------------------------------------*/
static LONG
hwcShareContext(hwcExtRequest_t *req, hwcExtResult_t *res) 
{
  HWGETFLATADDRESS HwGetFlat;
  FXGLIDESTATEINFO *GlideStatePtr;
  FxU32 TempProc;

  TempProc = req->optData.unmapMemoryReq.procHandle;
   
  GlideStatePtr = hwcGlideNodeAllocate ( TempProc );
   
  HwGetFlat.pData = &GlideStatePtr->lostContext;
     
  VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle,
          H3VDD_GET_FLAT_ADDRESS, 0, &HwGetFlat);

  res->optData.shareContextDWORDRes.contextDWORD = HwGetFlat.FlatAddress;

  res->resStatus=1;
  return 1;

} /* hwcShareContext */

/*----------------------------------------------------------------------
Function name:  hwcUnshareContext

Description:    Frees a Glide context in shared memory.
               
Information:    REQUIRED

Return:         returns 1 always
----------------------------------------------------------------------*/
static LONG
hwcUnshareContext(hwcExtRequest_t *req, hwcExtResult_t *res) 
{
  hwcGlideNodeFree ( req->optData.unmapMemoryReq.procHandle );

  res->resStatus=1;
  return 1;

} /* hwcUnshareContext */


/*----------------------------------------------------------------------
Function name:  hwcAGPInfo

Description:    Obtain information about AGP.
                
Information:    REQUIRED

Return:         returns 1 always
----------------------------------------------------------------------*/
static LONG
hwcAGPInfo (hwcExtRequest_t *req, hwcExtResult_t *res)
{
  res->optData.agpInfoRes.lAddr = _FF(agpMain.linAddr );
  res->optData.agpInfoRes.pAddr = _FF(agpMain.physAddr);
  res->optData.agpInfoRes.size  = _FF(agpMain.sizeInB );

  res->resStatus = 1;
  return 1;

} /* hwcAGPInfo */

/*----------------------------------------------------------------------
Function name:  hwcFifoInfo

Description:    Obtain information about command FIFO.
                
Information:    REQUIRED

Return:         returns 1 always
----------------------------------------------------------------------*/
static LONG
hwcFifoInfo(hwcExtRequest_t* req, hwcExtResult_t*  res)
{
  res->optData.fifoInfoRes.fifoType = ((_FF(enableAGPCF) && _FF(doAgpCF))
                                       ? HWCEXT_FIFO_AGP
                                       : HWCEXT_FIFO_FB);

  res->resStatus = 1;
  return 1;

}

/*----------------------------------------------------------------------
Function name:  hwcOvlAddr

Description:    Obtain information about overlay.
                
Information:    REQUIRED

Return:         returns 1 always
----------------------------------------------------------------------*/
static LONG
hwcOvlAddr (hwcExtResult_t *res)
{
  if(_FF(ddVisibleOverlaySurf))
      res->optData.ovlInfoRes.ovlAddr   = _FF(ddVisibleOverlaySurf);
  else
      res->optData.ovlInfoRes.ovlAddr   = _FF(ovlCurAddr);

  res->optData.ovlInfoRes.ovlXScale = _FF(ovlXScale);
  res->optData.ovlInfoRes.ovlYScale = _FF(ovlYScale);

  res->resStatus = 1;
  return 1;

} /* hwcOvlAddr */

/*----------------------------------------------------------------------
Function name:  hwcExecuteFifo

Description:    Service the Execute FIFO requests.
                
Information:

Return:         LONG    1 for success or,
                        0 for failure.
----------------------------------------------------------------------*/
static LONG
hwcExecuteFifo(hwcExtRequest_t *req, hwcExtResult_t *res)
{
  DWORD retVal = 1;
  DWORD jmpWrite = 0;

#ifdef CMDFIFO

  CMDFIFO_PROLOG(cmdFifo);
  
  /* Punt if software cursor or FSEM active. */

  if (!hwcSetGDIBusy( &_FF(lpPDevice)->deFlags)) 
  {
    if  ((GET(lph3IORegs->dacMode) & (SST_DAC_DPMS_ON_VSYNC|SST_DAC_FORCE_VSYNC|SST_DAC_DPMS_ON_HSYNC|SST_DAC_FORCE_HSYNC)))
    {
      res->resStatus = 1;
      return 1;
    }
    else
    {
      res->resStatus = 0;
      return 0;
    }
  }

  res->resStatus = 0;

  CMDFIFO_SETUP(cmdFifo);
  {
    /* Storage for jmp packets */
    FxU32 stateJmp[2];
    FxU32 stateRet[2];
    FxU32 cmdJmp[2];
    FxU32 cmdRet[2];
    FxU32	jmpWords;
    static FxU32 lastContextID = 0x00UL;
    FxBool doStateP = ((_FF(pD3context) != 0xFFFFFFFFUL) || /* d3d loaded? */
                       (_FF(pD3context) != 0x00UL) ||       /* d3d context? */
                       (req->contextID != lastContextID));  /* last execution? */
      
    switch(req->optData.executeFifoReq.fifoType) 
    {
      case HWCEXT_FIFO_FB:
      {
        jmpWords = 1;

        if (doStateP) 
        {
          stateJmp[0] = ((req->optData.executeFifoReq.stateOffset << (SSTCP_PKT0_ADDR_SHIFT - 2UL)) |
                         SSTCP_PKT0_JSR |
                         SSTCP_PKT0);
          stateRet[0] = (SSTCP_PKT0_RET |
                         SSTCP_PKT0);
        }
            
        cmdJmp[0] = ((req->optData.executeFifoReq.fifoOffset << (SSTCP_PKT0_ADDR_SHIFT - 2UL)) |
                     SSTCP_PKT0_JSR |
                     SSTCP_PKT0);
        cmdRet[0] = (SSTCP_PKT0_RET |
                     SSTCP_PKT0);
                                      
        jmpWrite = 1;
      }
      break;

#ifdef AGP_CMDFIFO
      case HWCEXT_FIFO_AGP:
      {
        const FxU32 agpCmdFifoOffset = (cmdFifo - _FF(agpMain.linAddr));
        jmpWords = 2;
          
        if (doStateP) 
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

        jmpWrite = 1;
      }
      break;
#endif /* AGP_CMDFIFO */

      case HWCEXT_FIFO_HOST:
      {
        DWORD far* cmdBuf;
        DWORD far* cmdBufEnd;
        DWORD numCmds;
        WORD sel = allocSelector();

        if (doStateP) 
        {
          CMDFIFO_CHECKROOM( cmdFifo, req->optData.executeFifoReq.stateSize);
            
          numCmds = req->optData.executeFifoReq.stateSize;
          setSelectorAddr( sel, req->optData.executeFifoReq.statePtr, (WORD)numCmds * 4 );
            
          cmdBuf    = MAKELP( sel, 0 );
          cmdBufEnd = cmdBuf + numCmds;
            
          while( cmdBuf < cmdBufEnd ) 
          {
            /* This has NO direct write equivalent */
            SET( cmdFifo, 0, *cmdBuf );
            cmdBuf++;
          }
          BUMP( numCmds );
        }
        
        /* Current command stream */
        CMDFIFO_CHECKROOM( cmdFifo, req->optData.executeFifoReq.fifoSize );
        numCmds = req->optData.executeFifoReq.fifoSize;
        setSelectorAddr( sel, req->optData.executeFifoReq.fifoPtr, (WORD)numCmds * 4 );
        
        cmdBuf    = MAKELP( sel, 0 );
        cmdBufEnd = cmdBuf + numCmds;
        
        while( cmdBuf < cmdBufEnd ) 
        {
          /* This has NO direct write equivalent */
          SET( cmdFifo, 0, *cmdBuf );
          cmdBuf++;
        }
        BUMP( numCmds );

        freeSelector( sel );
      }
      break;

      default:
        retVal = 0;
      break;

    }

    if (jmpWrite)
    {
      FxU32 i,*clientPtr;

      P6FENCE;

      if (doStateP) 
      {
        /* State return to fifo */
        clientPtr = (FxU32*)(req->optData.executeFifoReq.statePtr + 
                             (req->optData.executeFifoReq.stateSize << 0x02UL));
            
        for(i = 0; i < jmpWords; i++) 
          h3WRITE(NULL, clientPtr + i, stateRet[i]);
            
        P6FENCE;
            
        /* Jump to state routine */
        CMDFIFO_CHECKROOM(cmdFifo, jmpWords);
        for(i = 0; i < jmpWords; i++) 
          SET(cmdFifo, 0, stateJmp[i]);
        BUMP(req->optData.executeFifoReq.psSizeInDWORDS + (jmpWords << 0x01UL));
      }

      /* command return to fifo */
      clientPtr = (FxU32*)(req->optData.executeFifoReq.fifoPtr +
                           (req->optData.executeFifoReq.fifoSize << 2UL));
          
      for(i = 0; i < jmpWords; i++) 
        h3WRITE(NULL, clientPtr + i, cmdRet[i]);

      P6FENCE;

      /* Jump to state routine */
      CMDFIFO_CHECKROOM(cmdFifo, jmpWords);
      for(i = 0; i < jmpWords; i++) 
        SET(cmdFifo, 0, cmdJmp[i]);
      BUMP(req->optData.executeFifoReq.fifoSizeInDWORDS + (jmpWords << 0x01UL));

      /* Write the serial # for this fifo execution. */
      CMDFIFO_CHECKROOM(cmdFifo, 3);
      SET(cmdFifo, 0, (SSTCP_PKT5_LFB |
                       (0x00UL << SSTCP_PKT5_BYTEN_W2_SHIFT) |
                       (0x00UL << SSTCP_PKT5_BYTEN_WN_SHIFT) |
                       (0x01UL << SSTCP_PKT5_NWORDS_SHIFT) |
                       SSTCP_PKT5));
      SET(cmdFifo, 0, req->optData.executeFifoReq.sentinalOffset);
      SET(cmdFifo, 0, req->optData.executeFifoReq.serialNumber);
    }

    lastContextID = req->contextID;
  }

  CMDFIFO_EPILOG(cmdFifo);
  D3notify();
  hwcClearGDIBusy( &_FF(lpPDevice)->deFlags );

#endif /* CMDFIFO */

  res->resStatus = retVal;
  return retVal;

} /* hwcExecuteFifo */

// OBSOLETE ENTRY POINTS

/*----------------------------------------------------------------------
Function name:  hwcGetDriverVersion
Description:    Obtain the version number of the driver.
Information:    OBSOLETE
Return:         returns -1 always
----------------------------------------------------------------------*/
static LONG
hwcGetDriverVersion(hwcExtRequest_t *req, hwcExtResult_t *res)
{
  res->resStatus = -1;
  return -1;
} /* hwcGetDriverVersion */

/*----------------------------------------------------------------------
Function name:  hwcAllocFifo
Description:    Allocate a CMDFIFO for Glide.
Information:    OBSOLETE
Return:         returns -1 always
----------------------------------------------------------------------*/
static LONG
hwcAllocFifo(hwcExtRequest_t *req, hwcExtResult_t *res)
{
  res->resStatus = -1;
  return -1;
} /* hwcAllocFifo */

/*----------------------------------------------------------------------
Function name:  hwcRestoreDesktop
Description:    Restore desktop to pre-exclusive mode state
Information:    OBSOLETE
Return:         returns -1 always
----------------------------------------------------------------------*/
static LONG
hwcRestoreDesktop()
{
  return -1;
} /* hwcRestoreDesktop */

/*----------------------------------------------------------------------
Function name:  hwcQueryContext
Description:    Query the Glide context.
Information:    OBSOLETE
Return:         returns -1 always
----------------------------------------------------------------------*/
static LONG
hwcQueryContext(hwcExtRequest_t *req, hwcExtResult_t *res)
{
  res->resStatus = -1;
  return -1;
} /* hwcQueryContext */

// I2C FUNCTIONS

/*----------------------------------------------------------------------
Function name:  hwcI2C_write

Description:    Perform an I2C write via the display driver's VxD.

Return:         LONG    Result of the VDDCall or, FXFALSE if i2C was busy.
----------------------------------------------------------------------*/
#ifndef ERROR_BUSY
//
// MessageId: ERROR_BUSY
//
// MessageText:
//
//  The requested resource is in use.
//
#define ERROR_BUSY                       170L
#endif

static LONG
hwcI2C_write (hwcExtRequest_t *req, hwcExtResult_t *res) 
{
  static FxU16 busy = 1;

  _asm    /*had to put in assbly to get atomic reference to busy*/
    {
      dec word ptr [busy];
      jnz i2c_busy;
    }
  {
    res->resStatus = VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, 
                             dwDeviceHandle, H3VDD_I2C_WRITE,
                             (req->optData.i2c_WriteReq.deviceAddress << 16) |
                             (req->optData.i2c_WriteReq.i2c_regNum << 8) |
                             req->optData.i2c_WriteReq.i2c_regValue, 0);
    busy = 1;
    return (res->resStatus);
  }
i2c_busy:
    {
      res->resStatus = ERROR_BUSY;
      return (FXFALSE);
    }
} /* hwcI2C_write */


/*----------------------------------------------------------------------
Function name:  hwcI2C_MultiWrite

Description:    Perform multiple I2C writes via the display driver's VxD.
                
Return:         LONG    Result of the VDDCall or, FXFALSE if i2C was busy.
----------------------------------------------------------------------*/
static LONG
hwcI2C_MultiWrite (hwcExtRequest_t *req, hwcExtResult_t *res) 
{
  static FxU16 busy = 1;
  long mwParms[2];   // too much stuff to pass, pass by reference

  _asm    /*had to put in assbly to get atomic reference to busy*/
    {
      dec word ptr [busy];
      jnz i2c_busy;
    }
  {
    mwParms[0] = (req->optData.i2c_MultiWriteReq.deviceAddress << 16) |
      (req->optData.i2c_MultiWriteReq.i2c_regNum << 8) |
      (req->optData.i2c_MultiWriteReq.i2c_numBytes);
    // note: app's flat pointer will make more sense on vxd side
    mwParms[1] = (long)req->optData.i2c_MultiWriteReq.i2c_regValues;

    res->resStatus = VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, 
                             dwDeviceHandle, H3VDD_I2C_MULTI_WRITE,
                             (DWORD)((long _far*)mwParms), 0);

    busy = 1;
    return (res->resStatus);
  }
i2c_busy:
    {
      res->resStatus = ERROR_BUSY;
      return (FXFALSE);
    }
} /* hwcI2C_MultiWrite */

/*----------------------------------------------------------------------
Function name:  hwcI2C_Read

Description:    Perform an I2C read via the display driver's VxD.
                
Return:         LONG    Result of the VDDCall or, FXFALSE if i2C was busy.
----------------------------------------------------------------------*/
static LONG
hwcI2C_read (hwcExtRequest_t *req, hwcExtResult_t *res) 
{
  static FxU16 busy = 1;

  _asm    /*had to put in assbly to get atomic reference to busy*/
    {
      dec word ptr [busy];
      jnz i2c_busy;
    }
  {
    res->resStatus = VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, 
                             dwDeviceHandle, H3VDD_I2C_READ,
                             (req->optData.i2c_ReadReq.deviceAddress << 8) |
                             req->optData.i2c_ReadReq.i2c_regNum, 0);
    busy = 1;
    res->optData.i2c_ReadRes.i2c_regValue = res->resStatus & 255;
    return (res->resStatus >> 8);
  }
i2c_busy:
    {
      res->resStatus = ERROR_BUSY;
      return (FXFALSE);
    }
} /* hwcI2C_read */

/*----------------------------------------------------------------------
Function name:  hwcPCIOP

Description:    PCI Operation
                
Information:    REQUIRED (Napalm)

Return:         returns 1 always
----------------------------------------------------------------------*/
static LONG hwcPCIOP(hwcExtRequest_t *req, hwcExtResult_t *res) 
{
#ifdef SLI_AA
   HWPCIOP HwPCIOp;
   
   HwPCIOp.dwOp = (HWCEXT_PCI_READ == req->optData.pciOpReq.Operation) ? H3G_PCI_READ: H3G_PCI_WRITE;
   HwPCIOp.dwFunc = req->optData.pciOpReq.DeviceId;
   HwPCIOp.dwOffset = req->optData.pciOpReq.Offset;
   HwPCIOp.dwValue = req->optData.pciOpReq.Value;

   res->resStatus = VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle,
	    H3VDD_PCI_OP, 0, &HwPCIOp);

   res->optData.pciOpRes.Value = HwPCIOp.dwValue;
   return 1;
#else
   res->resStatus = 0x0;
   return 0x0;
#endif
} /* hwcPCIOP */

/*----------------------------------------------------------------------
Function name:  hwcGetSlaveRegs

Description:    Get Slave Registers                

Information:    REQUIRED (Napalm)

Return:         returns 1 always
----------------------------------------------------------------------*/
static LONG hwcGetSlaveRegs(hwcExtRequest_t *req, hwcExtResult_t *res) 
{
#ifdef SLI_AA
   HwInfo ourHwInfo;
   int i;
   
   ourHwInfo.diUnitNumber = req->optData.slaveRegReq.DeviceId;

   res->resStatus = VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle,
	    H3VDD_GET_HW_INFO, 0, &ourHwInfo);

   for (i=0; i<HWCEXT_MAX_SLAVE_REGS; i++)
      res->optData.slaveRegRes.Regs[i] = ourHwInfo.regBase[i];
   return 1;
#else
   res->resStatus = 0x0;
   return 0x0;
#endif
} /* hwcGetSlaveRegs */

/*----------------------------------------------------------------------
Function name:  hwcShareCPUType

Description:    Shares the CPU Type with the calling process
                
Information:    UNKNOWN

Return:         LONG    1 is always returned.
----------------------------------------------------------------------*/
LONG FAR PASCAL
hwcShareCPUType(DWORD *req, DWORD *res) 
{
   DWORD TempDWORD;
   
   TempDWORD = _FF(cpuType);

   *res = TempDWORD;
   return 1;

} /* hwcShareCPUType */

/*----------------------------------------------------------------------
Function name:  hwcSetAuxExclusiveMode

Description:    Sets the Aux Exclusive Mode Setting.
                
Information:    UNKNOWN

Return:         LONG    1 is always returned.
----------------------------------------------------------------------*/
LONG FAR PASCAL
hwcSetAuxExclusiveMode(DWORD *req, DWORD *res) 
{
   _FF(dwAuxExclusiveMode) = 1;

   return 1;

} /* hwcSetAuxExclusiveMode */

/*----------------------------------------------------------------------
Function name:  hwcReleaseAuxExclusiveMode

Description:    Releases the Aux Exclusive Mode Setting.
                
Information:    UNKNOWN

Return:         LONG    1 is always returned.
----------------------------------------------------------------------*/
LONG FAR PASCAL
hwcReleaseAuxExclusiveMode(DWORD *req, DWORD *res) 
{
   _FF(dwAuxExclusiveMode) = 0;

   return 1;

} /* hwcReleaseAuxExclusiveMode */

/*----------------------------------------------------------------------
Function name:  hwcGetAuxExclusiveMode

Description:    Gets the Aux Exclusive Mode Setting.
                
Information:    UNKNOWN

Return:         LONG    1 is always returned.
----------------------------------------------------------------------*/
LONG FAR PASCAL
hwcGetAuxExclusiveMode(DWORD *req, DWORD *res) 
{
   DWORD TempDWORD;
   
   TempDWORD = _FF(dwAuxExclusiveMode);

   *res = TempDWORD;
   
   return 1;

} /* hwcGetAuxExclusiveMode */

/*----------------------------------------------------------------------
Function name:  hwcExt

Description:    Massive switch statement that parses/processes
                the Extended request.
Information:

Return:         LONG    Value of the subsequently called function.
----------------------------------------------------------------------*/
LONG FAR PASCAL
hwcExt(DWORD *lpInput, DWORD *lpOutput) {
  hwcExtRequest_t *req = (hwcExtRequest_t *) lpInput;
  hwcExtResult_t  *res = (hwcExtResult_t *) lpOutput;

  DPF(DBGLVL_NORMAL, "hwcExt(%s)\n", extNames[req->which]);

  switch (req->which) {

  case HWCEXT_GETDRIVERVERSION:
    return hwcGetDriverVersion(req, res);
    break;

  case HWCEXT_ALLOCCONTEXT:
    return hwcAllocContext(req, res);
    break;

  case HWCEXT_GETDEVICECONFIG:
    return hwcGetDeviceConfig(req, res);
    break;

  case HWCEXT_GETLINEARADDR:
    return hwcGetLinearAddr(req, res);
    break;
    
  case HWCEXT_ALLOCFIFO:
    return hwcAllocFifo(req, res);
    break;
    
  case HWCEXT_EXECUTEFIFO:
    return hwcExecuteFifo(req, res);
    break;

  case HWCEXT_QUERYCONTEXT:
    return hwcQueryContext(req, res);
    break;

  case HWCEXT_RELEASECONTEXT:
    return hwcReleaseContext(req, res);
    break;

  case HWCEXT_HWCSETEXCLUSIVE:
    return hwcSetExclusive(req, res);
    break;

  case HWCEXT_HWCRLSEXCLUSIVE:
    return hwcRlsExclusive(req, res);
    break;

  case HWCEXT_I2C_WRITE_REQ:
    return hwcI2C_write (req, res);
    break;

  case HWCEXT_I2C_MULTI_WRITE_REQ:
    return hwcI2C_MultiWrite (req, res);
    break;

  case HWCEXT_I2C_READ_REQ:
    return hwcI2C_read (req, res);
    break;

  case HWCEXT_GETAGPINFO:
    return hwcAGPInfo(req, res);
    break;

  case HWCEXT_FIFOINFO:
    return hwcFifoInfo(req, res);
    break;

  case HWCEXT_LINEAR_MAP_OFFSET:
    return hwcLinearMapOffset(req, res);
    break;

  case HWCEXT_RESTORE_DESKTOP:
    return hwcRestoreDesktop();
    break;

  case HWCEXT_OVERLAY_DATA:
    return hwcOvlAddr ( res);
    break;
    
  case HWCEXT_SHARE_CONTEXT:
    return hwcShareContext( req, res );
    break;

  case HWCEXT_UNSHARE_CONTEXT:
    return hwcUnshareContext( req, res );
    break;

  case HWCEXT_PCI_OP:
    return hwcPCIOP(req, res);
    break; 

  case HWCEXT_GET_SLAVE_REGS:
    return hwcGetSlaveRegs(req, res);
    break; 

  default:
    return -1;
    break;
  }

  return 1;  
} /* hwcExt */

