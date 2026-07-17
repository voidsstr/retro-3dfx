/* -*-c++-*- */
/* $Header: power.c, 9, 10/11/00 8:54:59 PM, Brent$ */
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
** File name:   power.c
**
** Description: Power management for the monitor.
**
** $Revision: 9$ 
** $Date: 10/11/00 8:54:59 PM$
**
** $History: power.c $
** 
** *****************  Version 27  *****************
** User: Andrew       Date: 7/20/99    Time: 10:50a
** Updated in $/devel/h5/Win9x/dx/minivdd
** Removed FAKE_IT define
** 
** *****************  Version 26  *****************
** User: Andrew       Date: 7/09/99    Time: 4:25p
** Updated in $/devel/h5/Win9x/dx/minivdd
** new functions to support SLI-AA power management
** 
** *****************  Version 25  *****************
** User: Cwilcox      Date: 7/08/99    Time: 1:11p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Added runtime checking for Napalm versus Voodoo3.
** 
** *****************  Version 24  *****************
** User: Andrew       Date: 6/15/99    Time: 5:07p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Moved DevNodetoIORegs to Devtable.c
** 
** *****************  Version 22  *****************
** User: Andrew       Date: 3/30/99    Time: 10:21a
** Updated in $/devel/h3/Win95/dx/minivdd
** Added code to Enable Power State D3 and to set the mode to 640x480x60
** Hz so that DDC will work.
** 
** *****************  Version 21  *****************
** User: Andrew       Date: 1/29/99    Time: 10:49a
** Updated in $/devel/h3/Win95/dx/minivdd
** Added changes to save/restore Init Block registers
** 
** *****************  Version 20  *****************
** User: Michael      Date: 1/12/99    Time: 11:28a
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
** 19    12/08/98 5:53p Andrew
** Temporary change  -- Changed Avenger to only report Power State D0.
** 
** 18    11/24/98 8:46a Andrew
** Added code to report different CAPs for Voodoo 3
** 
** 17    10/30/98 7:28a Andrew
** Removed a call to CycleTime
** 
** 16    10/16/98 11:14a Ken
** mirroring GLOP changes into TOT for clock change policy (don't touch
** PLLs at boot by default, they still can be tweaked from the registry
** though), and for the new "Hank" memory timing rules (dramInit1 bit 14
** is speed dependent), and memClock and grxClock registry keys are now
** string keys
** 
** 15    9/27/98 5:49p Andrew
** Ken's suggestions on some improvements
** 
** 14    9/23/98 10:59p Andrew
** Added two byte fields to keep track of Monitor State and Video
** Processor State
** 
** 13    7/29/98 11:03a Andrew
** Changed GetAdapterPowerStateCaps to only report D0 state to avoid a bug
** with the SSIDs.  The SSID values are lost in D3 state and they need to
** and in the BUS state.  Also, removed save/retsore of PCI space as the
** configmg will do this.
** 
** 12    7/24/98 10:38p Ken
** changes to allow 2d driver to run properly synchronized with an AGP
** command fifo (although video memory fifo is still used when the desktop
** has the focus, e.g., a fullscreen 3d app isn't in the foreground)
** 
** 11    7/17/98 3:37p Ken
** separated default clocks into independent default values settable from
** environment variables (GCLK for graphics clock), (MCLK for memory
** clock)
** 
** 2     7/17/98 3:24p Ken
** added separate graphics and memory clock defaults, settable in build
** from environment variables GCLK (graphics clock) and MCLK (memory
** clock)
** 
** 1     7/16/98 2:49p Admin
** 
** 10    7/14/98 9:37a Andrew
** Changed h3InitSgram call to be like the one in Devtable.c 
** 
** 9     6/23/98 10:36a Andrew
** Minor Change to how we program VGAInit0 and changed NO_PMCSR_WRITE to
** PMCSR_WRITE
** 
** 8     6/23/98 7:14a Andrew
** Changed SetAdapterPowerState to not use Clock Bits to reduce power and
** for the code to be centered around the PMCSR register plus save\restore
** the PCI space.
** 
** 7     4/28/98 2:42p Andrew
** Fixed a problem where DPMS was not working right when we were the
** secondary monitor
** 
** 6     4/15/98 6:42p Ken
** added unified header to all files, with revision, etc. info in it
** 
** 5     4/09/98 10:20p Ken
** updated to phil's new init code
** 
** 4     3/20/98 9:49a Andrew
** Multi-Monitor
** 
** 3     3/12/98 3:26p Andrew
** Updates for SetAdapterPowerState
** 
** 2     3/11/98 2:15p Andrew
** Fixed call InitPower to Drop DevNode Parameter since it was not used.
** 
** 1     3/05/98 10:15a Andrew
** This is the module to implement Onnow
** $History: power.c $
** 
** *****************  Version 27  *****************
** User: Andrew       Date: 7/20/99    Time: 10:50a
** Updated in $/devel/h5/Win9x/dx/minivdd
** Removed FAKE_IT define
** 
** *****************  Version 26  *****************
** User: Andrew       Date: 7/09/99    Time: 4:25p
** Updated in $/devel/h5/Win9x/dx/minivdd
** new functions to support SLI-AA power management
** 
** *****************  Version 25  *****************
** User: Cwilcox      Date: 7/08/99    Time: 1:11p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Added runtime checking for Napalm versus Voodoo3.
** 
** *****************  Version 24  *****************
** User: Andrew       Date: 6/15/99    Time: 5:07p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Moved DevNodetoIORegs to Devtable.c
** 
** *****************  Version 22  *****************
** User: Andrew       Date: 3/30/99    Time: 10:21a
** Updated in $/devel/h3/Win95/dx/minivdd
** Added code to Enable Power State D3 and to set the mode to 640x480x60
** Hz so that DDC will work.
** 
** *****************  Version 21  *****************
** User: Andrew       Date: 1/29/99    Time: 10:49a
** Updated in $/devel/h3/Win95/dx/minivdd
** Added changes to save/restore Init Block registers
** 
** *****************  Version 20  *****************
** User: Michael      Date: 1/12/99    Time: 11:28a
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
** *****************  Version 19  *****************
** User: Andrew       Date: 12/08/98   Time: 5:53p
** Updated in $/devel/h3/Win95/dx/minivdd
** Temporary change  -- Changed Avenger to only report Power State D0.
** 
** *****************  Version 18  *****************
** User: Andrew       Date: 11/24/98   Time: 8:46a
** Updated in $/devel/h3/Win95/dx/minivdd
** Added code to report different CAPs for Voodoo 3
** 
** *****************  Version 17  *****************
** User: Andrew       Date: 10/30/98   Time: 7:28a
** Updated in $/devel/h3/Win95/dx/minivdd
** Removed a call to CycleTime
** 
** *****************  Version 16  *****************
** User: Ken          Date: 10/16/98   Time: 11:14a
** Updated in $/devel/h3/win95/dx/minivdd
** mirroring GLOP changes into TOT for clock change policy (don't touch
** PLLs at boot by default, they still can be tweaked from the registry
** though), and for the new "Hank" memory timing rules (dramInit1 bit 14
** is speed dependent), and memClock and grxClock registry keys are now
** string keys
** 
** *****************  Version 15  *****************
** User: Andrew       Date: 9/27/98    Time: 5:49p
** Updated in $/devel/h3/Win95/dx/minivdd
** Ken's suggestions on some improvements
** 
** *****************  Version 14  *****************
** User: Andrew       Date: 9/23/98    Time: 10:59p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added two byte fields to keep track of Monitor State and Video
** Processor State
** 
** *****************  Version 13  *****************
** User: Andrew       Date: 7/29/98    Time: 11:03a
** Updated in $/devel/h3/Win95/dx/minivdd
** Changed GetAdapterPowerStateCaps to only report D0 state to avoid a bug
** with the SSIDs.  The SSID values are lost in D3 state and they need to
** and in the BUS state.  Also, removed save/retsore of PCI space as the
** configmg will do this.
** 
** *****************  Version 12  *****************
** User: Ken          Date: 7/24/98    Time: 10:38p
** Updated in $/devel/h3/win95/dx/minivdd
** changes to allow 2d driver to run properly synchronized with an AGP
** command fifo (although video memory fifo is still used when the desktop
** has the focus, e.g., a fullscreen 3d app isn't in the foreground)
** 
** *****************  Version 11  *****************
** User: Ken          Date: 7/17/98    Time: 3:37p
** Updated in $/devel/h3/win95/dx/minivdd
** separated default clocks into independent default values settable from
** environment variables (GCLK for graphics clock), (MCLK for memory
** clock)
** 
** *****************  Version 2  *****************
** User: Ken          Date: 7/17/98    Time: 3:24p
** Updated in $/Releases/Banshee/A2_Merc/3dfx/devel/h3/Win95/dx/minivdd
** added separate graphics and memory clock defaults, settable in build
** from environment variables GCLK (graphics clock) and MCLK (memory
** clock)
** 
** *****************  Version 1  *****************
** User: Admin        Date: 7/16/98    Time: 2:49p
** Created in $/Releases/Banshee/A2_Merc/3dfx/devel/h3/Win95/dx/minivdd
** 
** *****************  Version 10  *****************
** User: Andrew       Date: 7/14/98    Time: 9:37a
** Updated in $/devel/h3/Win95/dx/minivdd
** Changed h3InitSgram call to be like the one in Devtable.c 
** 
** *****************  Version 9  *****************
** User: Andrew       Date: 6/23/98    Time: 10:36a
** Updated in $/devel/h3/Win95/dx/minivdd
** Minor Change to how we program VGAInit0 and changed NO_PMCSR_WRITE to
** PMCSR_WRITE
** 
** *****************  Version 8  *****************
** User: Andrew       Date: 6/23/98    Time: 7:14a
** Updated in $/devel/h3/Win95/dx/minivdd
** Changed SetAdapterPowerState to not use Clock Bits to reduce power and
** for the code to be centered around the PMCSR register plus save\restore
** the PCI space.
** 
** *****************  Version 7  *****************
** User: Andrew       Date: 4/28/98    Time: 2:42p
** Updated in $/devel/h3/Win95/dx/minivdd
** Fixed a problem where DPMS was not working right when we were the
** secondary monitor
** 
** *****************  Version 6  *****************
** User: Ken          Date: 4/15/98    Time: 6:42p
** Updated in $/devel/h3/win95/dx/minivdd
** added unified header to all files, with revision, etc. info in it
** 
** *****************  Version 5  *****************
** User: Ken          Date: 4/09/98    Time: 10:20p
** Updated in $/devel/h3/win95/dx/minivdd
** updated to phil's new init code
** 
** *****************  Version 4  *****************
** User: Andrew       Date: 3/20/98    Time: 9:49a
** Updated in $/devel/h3/Win95/dx/minivdd
** Multi-Monitor
** 
** *****************  Version 3  *****************
** User: Andrew       Date: 3/12/98    Time: 3:26p
** Updated in $/devel/h3/Win95/dx/minivdd
** Updates for SetAdapterPowerState
** 
** *****************  Version 2  *****************
** User: Andrew       Date: 3/11/98    Time: 2:15p
** Updated in $/devel/h3/Win95/dx/minivdd
** Fixed call InitPower to Drop DevNode Parameter since it was not used.
** 
** *****************  Version 1  *****************
** User: Andrew       Date: 3/05/98    Time: 10:15a
** Created in $/devel/h3/Win95/dx/minivdd
** This is the module to implement Onnow
*/

/********************************************************************************
*
* The file power.c implements the OnNow Power Management for Win '98
*
* The functions Win '98 expects are:
*
* GetAdapterPowerStateCaps(DEVNODE devnode)
* GetMonitorPowerStateCaps(DEVNODE devnode)
* SetAdapterPowerState(DEVNODE devnode, DWORD PowerState)
* SetMonitorPowerState(DEVNODE devnode, DWORD PowerState)
*
* According to the DOC a good test for these functions is "Magic School Bus" in DCT
*
*********************************************************************************/
#define POWER_TABLE
#include "h3vdd.h"
#include "h3.h"

#define VDDONLY
#include "h3g.h"
#include "h3cinitdd.h"
#include "devtable.h"
#undef  VDDONLY

#include "h3cinit.h"
#include "time.h"
#include "vmipld.h"

#pragma VxD_LOCKED_CODE_SEG
#pragma VxD_LOCKED_DATA_SEG



/*----------------------------------------------------------------------
Function name:  our_inp

Description:    Simple input byte.

Information:    

Return:         DWORD   result of the "in al,dx" operation.
----------------------------------------------------------------------*/
DWORD our_inp(DWORD Addr) 
{
   DWORD dwReturn;

   _asm {mov   edx, Addr}
   _asm {xor   eax, eax}
   _asm {in    al, dx}
   _asm {mov   dwReturn, eax}

   return dwReturn;
}


/*----------------------------------------------------------------------
Function name:  our_outp

Description:    Simple out byte.

Information:    

Return:         VOID
----------------------------------------------------------------------*/
void our_outp(DWORD addr, DWORD dwData)
{
   _asm {mov   edx, addr}
   _asm {mov   eax, dwData}
   _asm {out   dx, al}
}


/*----------------------------------------------------------------------
Function name:  VDD_Get_Mini_Dispatch_Table

Description:    Get the Dispatch Table from VDD.

Information:    

Return:         DWORD
----------------------------------------------------------------------*/
DWORD VXDINLINE VDD_Get_Mini_Dispatch_Table(DWORD **pDT)
{
    DWORD dwR;

    __asm pushad;
    VxDCall(VDD_Get_Mini_Dispatch_Table);
    __asm mov dwR,ecx;
    __asm mov eax,pDT;
    __asm mov [eax],edi;
    __asm popad;
    return(dwR);
}


/*----------------------------------------------------------------------
Function name:  GetAdapterPowerStateCaps

Description:    Get the power state capabilities of the adapter.

Information:    
    This is to fix the fact that when the chip is set in D3 the SSID
    gets whacked which confuses Microsoft's Configmgr.  To get around
    this we say we only support D0.  Another solution is to program the
    SSID's to zero.

Return:         DWORD   The power state bits.
----------------------------------------------------------------------*/
#define CM_POWERSTATE_HIBERNATE 0x00000010
#define CM_ALLSTATE (CM_POWERSTATE_D0 | CM_POWERSTATE_D1 | CM_POWERSTATE_D2 | CM_POWERSTATE_D3)

DWORD _cdecl GetAdapterPowerStateCaps(DEVNODE devnode)
{
  return (CM_POWERSTATE_D0 | CM_POWERSTATE_D3 | CM_POWERSTATE_HIBERNATE );
}


/*----------------------------------------------------------------------
Function name:  GetMonitorPowerStateCaps

Description:    Get the power state capabilities of the monitor.

Information:    

Return:         DWORD   Always returns CM_ALLSTATE.
----------------------------------------------------------------------*/
DWORD _cdecl GetMonitorPowerStateCaps(DEVNODE devnode)
{
   return CM_ALLSTATE;
}


/*----------------------------------------------------------------------
Function name:  SetAdapterPowerState

Description:    Set the power state for the adapter.

Information:    
 The register of interest is miscInit1 at Offset 0x14 in IO Space.
 The bits of interest are:

  7  -- Power Down Clut
  8  -- Power Down DAC
  9  -- Power Down Video PLL
  10 -- Power Down Graphics PLL 
  11 -- Power Down Memory PLL

 FIX_ME: Need to find out what registers I need to save and which
 windows  may invariently ask us to restore????   Also, might there
 be some disabling that I need to do?

Return:         DWORD   Nothing, D0, D1, D2, or D3.
----------------------------------------------------------------------*/
DWORD AdapterReturnState[] = {
   CR_DEFAULT,                            // <0x00> Nothing
   CR_DEFAULT,                            // <0x01> D0
   CR_DEFAULT,                            // <0x02> D1
   CR_DEFAULT,                            // <0x03> Nothing
   CR_DEFAULT,                            // <0x04> D2
   CR_DEFAULT,                            // <0x05> Nothing
   CR_DEFAULT,                            // <0x06> Nothing
   CR_DEFAULT,                            // <0x07> Nothing
   CR_DEFAULT,                            // <0x08> D3
   };

int SavePowerRegs(PDEVTABLE pDevTable, BYTE * pRegs);
int RestorePowerRegs(PDEVTABLE pDevTable, BYTE * pRegs);
#ifdef SLI_AA
DWORD _cdecl SetAdapterPowerState(DEVNODE devnode, DWORD PowerState, DWORD dwUnitNum)
#else
DWORD _cdecl SetAdapterPowerState(DEVNODE devnode, DWORD PowerState)
#endif
{
#ifdef SLI_AA
   PDEVTABLE pDev = FindPDEVFromDevNodeandUnitNum(devnode, dwUnitNum);
   SstIORegs *lpIORegs = DevNodeandUnitNumtoIORegs(devnode, dwUnitNum);
#else
   PDEVTABLE pDev = FindPDEVFromDevNode(devnode);
   SstIORegs *lpIORegs = DevNodetoIORegs(devnode);
#endif
   int i;
   DWORD addr;
   DWORD index;
   DWORD vgaInit0;
   DWORD DoInit0;
   DWORD iRet = CR_SUCCESS;


//   if (PowerState > sizeof(AdapterReturnState)/sizeof(DWORD))
//      return CR_DEFAULT;

   // wait until we are idle
   // if old state was 1 then we must be going to a lower power state
   if (CM_POWERSTATE_D0 == pDev->bPowerState)
      while (lpIORegs->status & SST_BUSY)
         ;

   if (CM_POWERSTATE_D3 == PowerState || CM_POWERSTATE_HIBERNATE == PowerState)
      {
      SavePowerRegs(pDev, (PBYTE)lpIORegs);
      DoInit0 = vgaInit0 = lpIORegs->vgaInit0;
      DoInit0 |= SST_VGA0_EXTENSIONS;
      lpIORegs->vgaInit0 = DoInit0;
      addr = (our_inp(pDev->IoBase + 0xCC) & 0x01) ? pDev->IoBase + 0xD4 : pDev->IoBase + 0xB4;
      index = our_inp(addr);
      for (i=0; i<4; i++)
         {
         our_outp(addr, 0x1C + i);
         pDev->bScratchPad[i] = (BYTE)our_inp(addr+1);
         }
      our_outp(addr, index);
      lpIORegs->vgaInit0 = vgaInit0;
      }


   if (CM_POWERSTATE_D0 == PowerState)
      {
   	
   	  // initialize h3 hardware
   	
      if(CM_POWERSTATE_D3 == pDev->bPowerState) 
      	iRet = CR_DEFAULT;

      RestorePowerRegs(pDev, (PBYTE)lpIORegs);
      DoInit0 = vgaInit0 = lpIORegs->vgaInit0;
      DoInit0 |= SST_VGA0_EXTENSIONS;
      lpIORegs->vgaInit0 = DoInit0;
      addr = pDev->IoBase;
      // Hit 3c3
      our_outp(addr + 0xc3, 0x01);
      // Hit Misc Output
      our_outp(addr + 0xc2, 0x01);
      addr = pDev->IoBase + 0xD4;
      index = our_inp(addr);
      for (i=0; i<4; i++)
         {
         our_outp(addr, 0x1C + i);
         our_outp(addr+1, (DWORD)pDev->bScratchPad[i]);
         }
      our_outp(addr, index);
      lpIORegs->vgaInit0 = vgaInit0;

      // Need to put a check in here to see if this is the VGA
      if (!(pDev->DispInfo.diInfoFlags & DEVICE_IS_NOT_VGA))
         h3InitVga(pDev->IoBase, 1);	 // enable VGA decode

      // The next two lines were added so DDC would not fail
      // when returning from Low Power Mode
      // The mode does not matter cause we will change it latter on to
      // the right mode
      // This mode should work on every monitor in the world so if
      // the monitor was changed during Low Power Mode we should be ok
      h3InitSetVideoMode(pDev, 640, 480, 60, 1, 0);
      // Wait 3/4 second for monitor to sync
      WaitTime(750000000);      
      }

   	pDev->bPowerState = (BYTE)PowerState;

	return iRet;

//   return AdapterReturnState[PowerState];
}


/*----------------------------------------------------------------------
Function name:  SetMonitorPowerState

Description:    Set the power state for the monitor.

Information:    
 The interesting register for this function are in dacMode at
 offset 0x4C.   The bits are:
    1 -- Enable DPMS on Vsync
    2 -- Force Vsync Value
    3 -- Enable DPMS on Hsync
    4 -- Force Hsync value

 FIX_ME: Need to find out if the state of VSYNC_HIGH & HSYNC_HIGH
 depend on  Miscellenous Output?  Should the bits be Held in the
 pulse or not pulse state?

Return:         DWORD   Nothing, D0, D1, D2, or D3.
----------------------------------------------------------------------*/
#define HOLD_VSYNC BIT(1)
#define VSYNC_HIGH BIT(2)
#define HOLD_HSYNC BIT(3)
#define HSYNC_HIGH BIT(4)
#define DPMS_BITS (HOLD_VSYNC | HOLD_HSYNC | VSYNC_HIGH | HSYNC_HIGH)

DWORD MonitorReturnState[] = {
   CR_DEFAULT,                            // <0x00> Nothing
   CR_SUCCESS,                            // <0x01> D0
   CR_SUCCESS,                            // <0x02> D1
   CR_DEFAULT,                            // <0x03> Nothing
   CR_SUCCESS,                            // <0x04> D2
   CR_DEFAULT,                            // <0x05> Nothing
   CR_DEFAULT,                            // <0x06> Nothing
   CR_DEFAULT,                            // <0x07> Nothing
   CR_SUCCESS,                            // <0x08> D3
   };

BYTE DacModeState[] = {
   0x00,                                  // <0x00> Nothing
   0x00,                                  // <0x01> D0
   (BYTE)HOLD_HSYNC,                      // <0x02> D1
   0x00,                                  // <0x03> Nothing
   (BYTE)HOLD_VSYNC,                      // <0x04> D2
   0x00,                                  // <0x05> Nothing
   0x00,                                  // <0x06> Nothing
   0x00,                                  // <0x07> Nothing
   (BYTE)HOLD_HSYNC | (BYTE)HOLD_VSYNC,   // <0x08> D3
   };

BYTE VidProcCfgState[] = {
   0x00,                                  // <0x00> Nothing
   SST_VIDEO_PROCESSOR_EN,                // <0x01> D0
   0x00,                                  // <0x02> D1
   0x00,                                  // <0x03> Nothing
   0x00,                                  // <0x04> D2
   0x00,                                  // <0x05> Nothing
   0x00,                                  // <0x06> Nothing
   0x00,                                  // <0x07> Nothing
   0x00,                                  // <0x08> D3
   };

#ifdef SLI_AA
DWORD _cdecl SetMonitorPowerState(DEVNODE devnode, DWORD PowerState, DWORD dwUnitNum)
#else
DWORD _cdecl SetMonitorPowerState(DEVNODE devnode, DWORD PowerState)
#endif
{
#ifdef SLI_AA
   PDEVTABLE pDev = FindPDEVFromDevNodeandUnitNum(devnode, dwUnitNum);
   SstIORegs *lpIORegs = DevNodeandUnitNumtoIORegs(devnode, dwUnitNum);
#else
   PDEVTABLE pDev = FindPDEVFromDevNode(devnode);
   SstIORegs *lpIORegs = DevNodetoIORegs(devnode);
#endif
   FxU32 dwData;
   BYTE NewState;

   if (PowerState > sizeof(MonitorReturnState)/sizeof(DWORD))
      return CR_DEFAULT;

   // This works if we are set the Video Processor up correctly
   NewState = VidProcCfgState[PowerState];

   // Save the old state of Video Proc Enable since we need it when we restore
   // or fixup the NewState if we are going into D0
   if (CM_POWERSTATE_D0 == pDev->bMonitorState)
      {
      if (CM_POWERSTATE_D0 != PowerState)
         {
         dwData = lpIORegs->vidProcCfg;
         pDev->bVideoProcEn = (BYTE)(dwData & SST_VIDEO_PROCESSOR_EN);
         }
      }
   
   if (CM_POWERSTATE_D0 == PowerState)
      NewState = NewState & pDev->bVideoProcEn;

   dwData = lpIORegs->vidProcCfg;
   dwData = (dwData & ~(SST_VIDEO_PROCESSOR_EN)) | NewState;
   lpIORegs->vidProcCfg = dwData;

   // Set DacMode
   NewState = DacModeState[PowerState];
   dwData = lpIORegs->dacMode;
   dwData = (dwData & ~(DPMS_BITS)) | NewState;
   lpIORegs->dacMode = dwData;

   // Keep track of what state we are in for restoration
   pDev->bMonitorState = (BYTE)PowerState;
   return MonitorReturnState[PowerState];
}


#ifdef SLI_AA
/*----------------------------------------------------------------------
Function name:  SavePCISpace

Description: Save PCI Space before we Power Down   

Return:         Nothing
----------------------------------------------------------------------*/
void SavePCISpace(PDEVTABLE pDev)
{
   int i;

   for (i=0; i<POWERPCISIZE; i++)
      pDev->PCISaveSpace[i] = PCI_Read_Config(pDev->dwBus, pDev->dwDevFunc, PowerPCIOffset[i]);

   // Turn off All Access
   PCI_Write_Config(pDev->dwBus, pDev->dwDevFunc, SST_PCI_COMMAND_ID, 0x0);

   // Power Down
   PCI_Write_Config(pDev->dwBus, pDev->dwDevFunc, SST_PCI_ACPI_CTL, 0x03);
}

/*----------------------------------------------------------------------
Function name:  RestorePCISpace

Description: Restore PCI Space before we Power Down   

Return:         Nothing
----------------------------------------------------------------------*/
void RestorePCISpace(PDEVTABLE pDev)
{
   FxU32 cfgInitEnable;
   int i;

   // Turn on Park me anywhere
   cfgInitEnable = PCI_Read_Config(pDev->dwBus, pDev->dwDevFunc, CFG_INIT_ENABLE);
   PCI_Write_Config(pDev->dwBus, pDev->dwDevFunc, CFG_INIT_ENABLE, cfgInitEnable | CFG_UPDATE_MEMBASE_LSBS);

   // Power Up
   PCI_Write_Config(pDev->dwBus, pDev->dwDevFunc, SST_PCI_ACPI_CTL, 0x00);

   //Restore
   for (i=0; i<POWERPCISIZE; i++)
      PCI_Write_Config(pDev->dwBus, pDev->dwDevFunc, PowerPCIOffset[i], pDev->PCISaveSpace[i]);

   // Turn off Park me anywhere
   PCI_Write_Config(pDev->dwBus, pDev->dwDevFunc, CFG_INIT_ENABLE, cfgInitEnable);
}

/*----------------------------------------------------------------------
Function name:  SetMasterAdapterPowerState

Description:    When in SLI-AA mode we need to do all the slaves ourself
since will not ask us to do them.

Return:         DWORD   Nothing, D0, D1, D2, or D3.
----------------------------------------------------------------------*/
void ClearBusyBit(DWORD lpDriverData);
void SetBusyBit(DWORD lpDriverData);
DWORD _cdecl SetMasterAdapterPowerState(DEVNODE devnode, DWORD PowerState)
{
   PDEVTABLE pDev = FindPDEVFromDevNode(devnode);
   PDEVTABLE pSlave;

   // All we need to save/restore for the Master is CFG_PCI_DECODE
   if (IS_NAPALM(pDev->dwVendorDeviceID))
      {
      if (CM_POWERSTATE_D3 == PowerState)
         pDev->PCISaveSpace[0] = PCI_Read_Config(pDev->dwBus, pDev->dwDevFunc, CFG_PCI_DECODE);
      else if (CM_POWERSTATE_D0 == PowerState)
         PCI_Write_Config(pDev->dwBus, pDev->dwDevFunc, CFG_PCI_DECODE, pDev->PCISaveSpace[0]);
      }   

   if (SLI_AA_MASTER_DEVICE == pDev->dwType)
      {
      for (pSlave = pDev->pSlave; NULL != pSlave; pSlave = pSlave->pSlave)
         {
         // Power Up the Adapter
         // Typically the OS does this
         if (CM_POWERSTATE_D0 == PowerState)
            RestorePCISpace(pSlave);
         else if (CM_POWERSTATE_D3 == PowerState)
            {
            if (0x0 != pSlave->lpDriverData)
               SetBusyBit(pSlave->lpDriverData);
            }

         // Disable Master
         PCI_Write_Config(pDev->dwBus, pDev->dwDevFunc, SST_PCI_COMMAND_ID, 0x02);      

         // Enable Slave
         PCI_Write_Config(pSlave->dwBus, pSlave->dwDevFunc, SST_PCI_COMMAND_ID, 0x03);      

         // Get Ready for Power Up or Power Down
         SetAdapterPowerState(devnode, PowerState, pSlave->dwUnitNum);
         
         // Disable Slave
         PCI_Write_Config(pSlave->dwBus, pSlave->dwDevFunc, SST_PCI_COMMAND_ID, 0x02);      

         // Enable Master
         PCI_Write_Config(pDev->dwBus, pDev->dwDevFunc, SST_PCI_COMMAND_ID, 0x03);      

         // Power Down the Adapter
         // Typically the OS does this
         if (CM_POWERSTATE_D3 == PowerState)
            SavePCISpace(pSlave);
         else if (CM_POWERSTATE_D0 == PowerState)
            {
            if (0x0 != pSlave->lpDriverData)
               ClearBusyBit(pSlave->lpDriverData);
            }
         }
      }

   // Do Master Last
   return SetAdapterPowerState(devnode, PowerState, 0x0);
}

/*----------------------------------------------------------------------
Function name:  SetMasterMonitorPowerState

Description:    When in SLI-AA mode we need to do all the slaves ourself
since will not ask us to do them.

Note: Windows calls this for every Unit Number that you report but
does not set the Unit Number in DispInfo.

Return:         DWORD   Nothing, D0, D1, D2, or D3.
----------------------------------------------------------------------*/
DWORD _cdecl SetMasterMonitorPowerState(DEVNODE devnode, DWORD PowerState)
{
   PDEVTABLE pDev = FindPDEVFromDevNode(devnode);
   PDEVTABLE pSlave;

   if (SLI_AA_MASTER_DEVICE == pDev->dwType)
      {
      for (pSlave = pDev->pSlave; NULL != pSlave; pSlave = pSlave->pSlave)
         {
         // Power Down Monitor
         SetMonitorPowerState(devnode, PowerState, pSlave->dwUnitNum);
         }
      }

   // Do Master Last
   return SetMonitorPowerState(devnode, PowerState, 0x0);
}

#endif

/*----------------------------------------------------------------------
Function name:  InitPower

Description:    This routine is used to init the mini-vdd dispatch
                table for the four power functions.
Information:    

Return:         VOID
----------------------------------------------------------------------*/
void InitPower(void)
{
   PDWORD pDispatch;
   DWORD dwNFunc;
   dwNFunc = VDD_Get_Mini_Dispatch_Table(&pDispatch);
   // If the dispatch table has these function then point to ours
   if (dwNFunc > GET_MONITOR_POWER_STATE_CAPS)
      {
#ifdef SLI_AA
	   pDispatch[SET_ADAPTER_POWER_STATE] = (DWORD)SetMasterAdapterPowerState;
#else
	   pDispatch[SET_ADAPTER_POWER_STATE] = (DWORD)SetAdapterPowerState;
#endif
	   pDispatch[GET_ADAPTER_POWER_STATE_CAPS] = (DWORD)GetAdapterPowerStateCaps;
#ifdef SLI_AA
	   pDispatch[SET_MONITOR_POWER_STATE] = (DWORD)SetMasterMonitorPowerState;
#else
	   pDispatch[SET_MONITOR_POWER_STATE] = (DWORD)SetMonitorPowerState;
#endif
	   pDispatch[GET_MONITOR_POWER_STATE_CAPS] = (DWORD)GetMonitorPowerStateCaps;
      CycleTime();
      }

}

/*----------------------------------------------------------------------
Function name:  SavePowerRegs

Description:    This function saves the registers we need to restore
                when power is restored.
Information:    

Return:         0
----------------------------------------------------------------------*/
int SavePowerRegs(PDEVTABLE pDevTable, BYTE * pRegs)
{
   SstIORegs * lpIORegs = (SstIORegs *)pRegs;
   FxU32 miscInit0;
   int i;
   
   // Setup miscinit0
   // so that we can read the real miscInit1....
   if (IS_NAPALM(pDevTable->dwVendorDeviceID))
      {
      miscInit0 = lpIORegs->miscInit0;
      miscInit0 &= ~(VIP2VMI_ACCESS);   
      lpIORegs->miscInit0 = miscInit0;
      }

   for (i=0; i<POWERREGSIZE; i++)
      {
      // This is read from the BIOS at Enable Time
      if (SGRAMMODE_INDEX == i)
         continue;
      pDevTable->dwPowerReg[i] = *(DWORD *)(pRegs + PowerRegOffset[i]);
      }
   
   return 0;
}

/*----------------------------------------------------------------------
Function name:  RestorePowerRegs

Description:    This function restores the registers we need to restore
                when power is restored.
Information:    

Return:         0
----------------------------------------------------------------------*/
int RestorePowerRegs(PDEVTABLE pDevTable, BYTE * pRegs)
{
   SstIORegs * lpIORegs = (SstIORegs *)pRegs;
   int i;

   for (i=0; i<POWERREGSIZE; i++)
      {
      if (SGRAMMODE_INDEX == i)
         {
         if (MEMTYPE_DDR == pDevTable->dwMemoryBus)
            {
            lpIORegs->dramData = DEFAULT_EXTENDED_MODE;
            lpIORegs->dramCommand = H3_DRAMMODE_REG;
            lpIORegs->dramData = DEFAULT_RESET_DDRMODE;
            lpIORegs->dramCommand = H3_DRAMMODE_REG;
            lpIORegs->dramData = DEFAULT_DDRMODE;
            lpIORegs->dramCommand = H3_DRAMMODE_REG;
            }
         else
            {
            lpIORegs->dramData = pDevTable->dwPowerReg[i];
            lpIORegs->dramCommand = H3_DRAMMODE_REG;
            // In the BIOS we set this bit if it is SGRAM
            if (!(lpIORegs->dramInit1 & SST_MCTL_TYPE_SDRAM))
               {
               lpIORegs->dramData = DEFAULT_SGRAMMASK;
               lpIORegs->dramCommand = H3_DRAMMASK_REG;
#if 0
               // If we ever figure out a good value to cram into the color register we are locked and ready
               lpIORegs->dramData = DEFAULT_SGRAMCOLOR;
               lpIORegs->dramCommand = H3_DRAMCOLOR_REG;
#endif      
               }            
            }
         }
      else
         *(DWORD *)(pRegs + PowerRegOffset[i]) = pDevTable->dwPowerReg[i];
      }
 
   return 0;  
}

