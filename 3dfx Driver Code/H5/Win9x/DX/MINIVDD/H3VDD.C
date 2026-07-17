/* -*-c++-*- */
/* $Header: h3vdd.c, 61, 10/23/00 2:43:58 PM, Matt McClure$ */
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
** File name:   h3vdd.c
**
** Description: VMM/VDD functions.
**
** $Revision: 61$
** $Date: 10/23/00 2:43:58 PM$
**
** $History: h3vdd.c $
** 
** *****************  Version 138  *****************
** User: Dale         Date: 9/09/99    Time: 5:55p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Removed an unnecessary call to panelOn from SetVideoMode
** 
** *****************  Version 137  *****************
** User: Rbissell     Date: 9/07/99    Time: 2:15p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Deployed the new modularized I2C code.
** 
** *****************  Version 136  *****************
** User: Andrew       Date: 9/07/99    Time: 11:44a
** Updated in $/devel/h5/Win9x/dx/minivdd
** Added Code to fix a problem where Glide switch us into SLI mode but not
** out.
** 
** *****************  Version 135  *****************
** User: Xingc        Date: 9/07/99    Time: 10:23a
** Updated in $/devel/h5/Win9x/dx/minivdd
** Add new IOControl for VMI functions
** 
** *****************  Version 134  *****************
** User: Andrew       Date: 8/31/99    Time: 9:30a
** Updated in $/devel/h5/Win9x/dx/minivdd
** Added code for WIN_CSIM when not in SLI_AA mode
** 
** *****************  Version 133  *****************
** User: Lpost        Date: 8/25/99    Time: 2:16p
** Updated in $/devel/h5/Win9x/dx/minivdd
** V3TV code merge into H5
** 
** *****************  Version 132  *****************
** User: Dalev        Date: 8/20/99    Time: 4:22p
** Updated in $/devel/h5/Win9x/dx/minivdd
** SetVideoMode() now calls PanelOn() instead of PanelSet().
** 
** *****************  Version 131  *****************
** User: Andrew       Date: 8/19/99    Time: 12:27p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Added code to enable the Fake PCI writes
** 
** *****************  Version 130  *****************
** User: Cwilcox      Date: 8/09/99    Time: 1:19p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Added #ifndef WIN_CSIM around TV out code in SetVideoMode.
** 
** *****************  Version 129  *****************
** User: Rbissell     Date: 8/07/99    Time: 3:13p
** Updated in $/devel/h5/Win9x/dx/minivdd
** tvout merge from V3_OEM_100
** 
** *****************  Version 128  *****************
** User: Andrew       Date: 8/06/99    Time: 6:12p
** Updated in $/devel/h5/Win9x/dx/minivdd
** added a return value to gethwinfo
** 
** *****************  Version 127  *****************
** User: Andrew       Date: 8/06/99    Time: 4:39p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Added PCI function for Glide
** 
** *****************  Version 126  *****************
** User: Andrew       Date: 8/05/99    Time: 4:13p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Added code to Turn on Fake PCI Routines at Enable Time
** 
** *****************  Version 125  *****************
** User: Andrew       Date: 7/30/99    Time: 1:59p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Added a fix for the Fake Environment
** 
** *****************  Version 124  *****************
** User: Dalev        Date: 7/28/99    Time: 12:51p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Updated 'QueryForVMIPld()' with 'PDEVTABLE' arguement.
** 
** *****************  Version 123  *****************
** User: Andrew       Date: 7/27/99    Time: 4:39p
** Updated in $/devel/h5/Win9x/dx/minivdd
** removed dwNumChips and added call to GetWINSIMIface
** 
** *****************  Version 122  *****************
** User: Andrew       Date: 7/26/99    Time: 5:55p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Added code to restore lfbMemConfig for Napalm 
** 
** *****************  Version 121  *****************
** User: Andrew       Date: 7/20/99    Time: 10:48a
** Updated in $/devel/h5/Win9x/dx/minivdd
** Code to support Multiple Chips from C Simulator
** 
** *****************  Version 120  *****************
** User: Andrew       Date: 7/16/99    Time: 2:06p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Changed regBase and RegBase from single dword to array to support
** sparse register mapping
** 
** *****************  Version 119  *****************
** User: Andrew       Date: 7/09/99    Time: 4:23p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Renamed the DPMS function to support SLI-AA
** 
** *****************  Version 118  *****************
** User: Andrew       Date: 7/09/99    Time: 8:28a
** Updated in $/devel/h5/Win9x/dx/minivdd
** Added code to count number of chips <Master + Slaves>
** 
** *****************  Version 117  *****************
** User: Cwilcox      Date: 7/08/99    Time: 1:09p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Added runtime checking for Napalm versus Voodoo3.
** 
** *****************  Version 116  *****************
** User: Andrew       Date: 7/07/99    Time: 2:44p
** Updated in $/devel/h5/Win9x/dx/minivdd
** SLI/AA workarounds
** 
** *****************  Version 115  *****************
** User: Dalev        Date: 7/02/99    Time: 9:59a
** Updated in $/devel/h5/Win9x/dx/minivdd
** Updated with VMI function prototypes so will compile w/ -WX.
** 
** *****************  Version 114  *****************
** User: Dalev        Date: 7/01/99    Time: 6:41p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Added detection of an external PLD and a 'W32_DeviceIOCtl()' interface
** for communicating with said PLD.
** 
** *****************  Version 113  *****************
** User: Edwin        Date: 6/29/99    Time: 3:52p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Remove obsolete Banshee ifdefs.
** 
** *****************  Version 112  *****************
** User: Andrew       Date: 6/25/99    Time: 9:57a
** Updated in $/devel/h5/Win9x/dx/minivdd
** Changes to support SLI/AA
** 
** *****************  Version 111  *****************
** User: Andrew       Date: 6/15/99    Time: 5:02p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Numerous changes for SLI/AA
** 
** *****************  Version 110  *****************
** User: Andrew       Date: 6/04/99    Time: 4:13p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Added code to support UnitNumbers
** 
** *****************  Version 109  *****************
** User: Michael      Date: 6/03/99    Time: 8:55a
** Updated in $/devel/h5/Win9x/dx/minivdd
** Add -WX compiler option and clean up offending warnings.
** 
** *****************  Version 107  *****************
** User: Stb_lpost    Date: 5/17/99    Time: 1:41p
** Updated in $/devel/h3/win95/dx/minivdd
** V3TV Video Capture fixes for E3 Demo
** 
** *****************  Version 106  *****************
** User: Stb_rbissell Date: 5/14/99    Time: 9:38p
** Updated in $/devel/h3/win95/dx/minivdd
** tvout and dfp changes
** 
** *****************  Version 105  *****************
** User: Andrew       Date: 5/14/99    Time: 1:34p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added Real,Fake Membase 0,1 to make management of these easier
** 
** *****************  Version 104  *****************
** User: Andrew       Date: 5/14/99    Time: 8:00a
** Updated in $/devel/h3/Win95/dx/minivdd
** Added a pushad and popad to VMM_Get_DDB inline function to fix cpuType
** bug
** 
** *****************  Version 103  *****************
** User: Andrew       Date: 5/13/99    Time: 4:13p
** Updated in $/devel/h3/Win95/dx/minivdd
** Changed Real to Fake on lfb to more accuratelt reflect its use
** 
** *****************  Version 102  *****************
** User: Andrew       Date: 5/06/99    Time: 4:37p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added code for New Windows "C" Simulator
** 
** *****************  Version 101  *****************
** User: Adrians      Date: 5/04/99    Time: 10:58a
** Updated in $/devel/h3/Win95/dx/minivdd
** AMD requested that we now disable AGP for IronGate revisions < 0x23
** (rev C3).
** 
** *****************  Version 100  *****************
** User: Cwilcox      Date: 4/23/99    Time: 4:15p
** Updated in $/devel/h3/Win95/dx/minivdd
** Completed Napalm 32mb/64Mb changes.
** 
** *****************  Version 99  *****************
** User: Adrians      Date: 4/21/99    Time: 7:09p
** Updated in $/devel/h3/Win95/dx/minivdd
** Add code to disable AGP when running on a K7 with an IronGate AGP
** chipset revision < 0x30.
** 
** *****************  Version 98  *****************
** User: Xingc        Date: 4/08/99    Time: 5:23p
** Updated in $/devel/h3/Win95/dx/minivdd
** Add one more IOControl function to allocate KMVTBUFF
** 
** *****************  Version 97  *****************
** User: Andrew       Date: 4/06/99    Time: 12:40a
** Updated in $/devel/h3/Win95/dx/minivdd
** Added code to MAP_FLAT UnglideContext
** 
** *****************  Version 96  *****************
** User: Xingc        Date: 3/22/99    Time: 5:46p
** Updated in $/devel/h3/Win95/dx/minivdd
** Add GetExtraAddr() IOControl function
** 
** *****************  Version 95  *****************
** User: Andrew       Date: 3/18/99    Time: 3:53p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added Code to reprogram lfbMemConfig when we go to Full-Screen DOS
** 
** *****************  Version 94  *****************
** User: Stuartb      Date: 3/18/99    Time: 11:56a
** Updated in $/devel/h3/Win95/dx/minivdd
** On DisableGDIDesktop (ie. transitioning to full screen DOS box), fixup
** CRTC registers.  Do the same for tvout if active.
** 
** *****************  Version 93  *****************
** User: Michael      Date: 3/17/99    Time: 4:08p
** Updated in $/devel/h3/Win95/dx/minivdd
** Fix PRS 5179.  In SetVideoMode, comment out clearing of screen when
** resolution is 2046.  Code moved to dd16\h3.c.
** 
** *****************  Version 92  *****************
** User: Stuartb      Date: 3/12/99    Time: 3:40p
** Updated in $/devel/h3/Win95/dx/minivdd
** In H3VDD_GET_TVINIT_STATUS revert back to reading
** BIOS_SCRATCH_REGISTER2.
** 
** *****************  Version 91  *****************
** User: Stuartb      Date: 3/12/99    Time: 8:46a
** Updated in $/devel/h3/Win95/dx/minivdd
** Be sure to pass driver new status, cvbsOut and tvBoot.
** 
** *****************  Version 90  *****************
** User: Stuartb      Date: 3/11/99    Time: 2:45p
** Updated in $/devel/h3/Win95/dx/minivdd
** Move TvOutFn, encoder specific function pointer to DEVTABLE so will
** work with secondary adapter.
** 
** *****************  Version 89  *****************
** User: Stuartb      Date: 3/03/99    Time: 5:03p
** Updated in $/devel/h3/Win95/dx/minivdd
** On MiniVDD_System_Exit reenable TVOUT if appropriate.  Fixes problem
** that only occurs on 'shutdown to MS-DOS' from windows.
** 
** *****************  Version 88  *****************
** User: Stb_srogers  Date: 2/25/99    Time: 6:59a
** Updated in $/devel/h3/win95/dx/minivdd
** Clearing the screen to black during mode switch to the 2046 mode, Fixes
** PRS#4574
** 
** *****************  Version 87  *****************
** User: Stuartb      Date: 2/23/99    Time: 2:35p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added call to tvstdToNVRAM().
** 
** *****************  Version 86  *****************
** User: Stuartb      Date: 2/16/99    Time: 3:52p
** Updated in $/devel/h3/Win95/dx/minivdd
** At boot time set tv standard to whatever BIOS thinks it should be.
** 
** *****************  Version 85  *****************
** User: Stuartb      Date: 2/16/99    Time: 11:56a
** Updated in $/devel/h3/Win95/dx/minivdd
** Expose more bits of BIOS SCRATCH_REG2  in H3VDD_GET_TVINIT_STATUS call.
** 
** *****************  Version 84  *****************
** User: Stuartb      Date: 2/10/99    Time: 2:38p
** Updated in $/devel/h3/Win95/dx/minivdd
** One byte change, pass out extra bits in BIOS scratch register that will
** be used for new PAL tvout modes.
** 
** *****************  Version 83  *****************
** User: Ken          Date: 2/08/99    Time: 2:10p
** Updated in $/devel/h3/win95/dx/minivdd
** added cpu/OS detection of pentium III (katmai) processors, and added
** katmai-optimized d3d texture download
** 
** *****************  Version 82  *****************
** User: Stuartb      Date: 2/08/99    Time: 8:59a
** Updated in $/devel/h3/Win95/dx/minivdd
** Change to get pDev from lookup, don't assume isVGA.
** 
** *****************  Version 81  *****************
** User: Stuartb      Date: 2/05/99    Time: 12:59p
** Updated in $/devel/h3/Win95/dx/minivdd
** Do not turn panel off on H3VDD_DISABLE_GDI_DESKTOP  because this
** clobbers BIOS which just turned it on!
** 
** *****************  Version 80  *****************
** User: Stuartb      Date: 2/02/99    Time: 11:02a
** Updated in $/devel/h3/Win95/dx/minivdd
** On DISABLE_GDI_DESKTOP, power down flat panel.
** 
** *****************  Version 79  *****************
** User: Stb_srogers  Date: 1/29/99    Time: 12:48p
** Updated in $/devel/h3/win95/dx/minivdd
** 
** *****************  Version 78  *****************
** User: Andrew       Date: 1/29/99    Time: 10:47a
** Updated in $/devel/h3/Win95/dx/minivdd
** Small modification to suppress multi-reads of the BIOS Version string
** 
** *****************  Version 77  *****************
** User: Stuartb      Date: 1/26/99    Time: 2:42p
** Updated in $/devel/h3/Win95/dx/minivdd
** Check for lcd enabled before turning it on at mode change.  Fixed oops
** in tvo code.
** 
** *****************  Version 76  *****************
** User: Stuartb      Date: 1/26/99    Time: 2:37p
** Updated in $/devel/h3/Win95/dx/minivdd
** I think someone clobbered me when I had this file out 1/25-26.  Am
** checking in to get back in sync.  No changes made by me.
** 
** *****************  Version 75  *****************
** User: Andrew       Date: 1/21/99    Time: 5:47p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added New Function to Handle Device IO Ctl Messages
** 
** *****************  Version 74  *****************
** User: Stuartb      Date: 1/14/99    Time: 3:49p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added H3VDD_FLATPNL_CTRL, changed H3VDD_RW_REGISTER
** to be able to write byte wide i/o regs.
** 
** *****************  Version 73  *****************
** User: Stuartb      Date: 1/12/99    Time: 2:30p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added H3VDD_RW_REGISTER, a helper to simple r/w sstio regs.
** 
** *****************  Version 72  *****************
** User: Michael      Date: 1/08/99    Time: 11:46a
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
** *****************  Version 71  *****************
** User: Stuartb      Date: 1/06/99    Time: 5:09p
** Updated in $/devel/h3/Win95/dx/minivdd
** Changed misleading define H3_VDD_GET_BIOS_SCRATCH_REG to something
** meaningful.
** 
** *****************  Version 70  *****************
** User: Stuartb      Date: 1/05/99    Time: 4:55p
** Updated in $/devel/h3/Win95/dx/minivdd
** Reversed order of reenabling flat panel & tvout on mode change.
** 
** *****************  Version 69  *****************
** User: Andrew       Date: 1/05/99    Time: 10:43a
** Updated in $/devel/h3/Win95/dx/minivdd
** Added function to get bios string at mdoe switch time and a subfunction
** to retrieve it.
** 
** *****************  Version 68  *****************
** User: Stuartb      Date: 12/18/98   Time: 5:32p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added function hook: H3VDD_FLATPNL_PHYSICAL.
** 
** *****************  Version 67  *****************
** User: Stuartb      Date: 12/15/98   Time: 3:31p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added call to xlcd enable.  Cleaned up bt868 initialization.
** 
** *****************  Version 66  *****************
** User: Andrew       Date: 12/09/98   Time: 5:29p
** Updated in $/devel/h3/Win95/dx/minivdd
** Fixed multi-monitor to work with IRQ's
** 
** *****************  Version 65  *****************
** User: Andrew       Date: 12/08/98   Time: 5:51p
** Updated in $/devel/h3/Win95/dx/minivdd
** Changed LockAPI_Init back once more so that we cna work in a
** multi-monitor situation
** 
** *****************  Version 64  *****************
** User: Peter        Date: 12/08/98   Time: 9:16a
** Updated in $/devel/h3/Win95/dx/minivdd
** c-ified video memory fifo support stuff
** 
** *****************  Version 63  *****************
** User: Andrew       Date: 12/05/98   Time: 1:55a
** Updated in $/devel/h3/Win95/dx/minivdd
** Turned off AGP when Banshee is secondary as this fixes a AGP problem
** with a Gateway machine
** 
** *****************  Version 62  *****************
** User: Martin       Date: 12/04/98   Time: 3:58p
** Updated in $/devel/h3/Win95/dx/minivdd
** Fix for multi-mon.
** 
** *****************  Version 61  *****************
** User: Stuartb      Date: 12/02/98   Time: 11:51a
** Updated in $/devel/h3/Win95/dx/minivdd
** Fix PRS3382, DVD copy protect requests leaves vidInFormat == TVOUT.
** Do not assum booted to tvout if !bIsVGA or encoder not preset.
** 
** *****************  Version 60  *****************
** User: Peter        Date: 11/30/98   Time: 6:48p
** Updated in $/devel/h3/Win95/dx/minivdd
** query for possibly re-mapped base address
** 
** *****************  Version 59  *****************
** User: Andrew       Date: 11/24/98   Time: 8:35a
** Updated in $/devel/h3/Win95/dx/minivdd
** Added code to fill in the AGPCaps when hwinfo is requested
** 
** *****************  Version 58  *****************
** User: Stuartb      Date: 11/19/98   Time: 1:17p
** Updated in $/devel/h3/Win95/dx/minivdd
** More work to support copy protection, impl. color stripe, TriggerBits.
** 
** *****************  Version 57  *****************
** User: Andrew       Date: 11/16/98   Time: 8:32p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added code to return the Device ID, Vendor ID, revision Id, and SSIDs.
** 
** *****************  Version 56  *****************
** User: Stuartb      Date: 11/12/98   Time: 12:45p
** Updated in $/devel/h3/Win95/dx/minivdd
** Fixed typo.
** 
** *****************  Version 55  *****************
** User: Stuartb      Date: 11/11/98   Time: 9:28a
** Updated in $/devel/h3/Win95/dx/minivdd
** First cut at tvout changes to support Macrovision copy protection.
** Only works under Win98.
** 
** *****************  Version 54  *****************
** User: Stuartb      Date: 11/06/98   Time: 5:41p
** Updated in $/devel/h3/Win95/dx/minivdd
** Minor change to make sure that QUERY_TVSTATUS fails properly if no part
** on board.
** 
** *****************  Version 53  *****************
** User: Stuartb      Date: 11/05/98   Time: 8:14a
** Updated in $/devel/h3/Win95/dx/minivdd
** Init tvout size & position upon first envocation.
** 
** *****************  Version 52  *****************
** User: Stuartb      Date: 11/04/98   Time: 3:43p
** Updated in $/devel/h3/Win95/dx/minivdd
** Hook to respawn TV out on VGA to hires if booted to tvout.
** 
** *****************  Version 51  *****************
** User: Stuartb      Date: 10/26/98   Time: 1:11p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added calls for H3VDD_GET_TVSIZE_CTRL.
** 
** *****************  Version 50  *****************
** User: Stuartb      Date: 10/20/98   Time: 3:37p
** Updated in $/devel/h3/Win95/dx/minivdd
** Mods to fix random I2c hang with BT868.
** 
** *****************  Version 49  *****************
** User: Stuartb      Date: 10/12/98   Time: 2:33p
** Updated in $/devel/h3/Win95/dx/minivdd
** Fixed silly oops on I2C_READ/WRITE calls.
** 
** *****************  Version 48  *****************
** User: Andrew       Date: 10/05/98   Time: 4:00p
** Updated in $/devel/h3/Win95/dx/minivdd
** Turned on Driver DDC support
** 
** *****************  Version 47  *****************
** User: Stuartb      Date: 9/17/98    Time: 5:04p
** Updated in $/devel/h3/Win95/dx/minivdd
** Reinstated minimal support for Chronte encoder, added auto detect.
** 
** *****************  Version 46  *****************
** User: Stuartb      Date: 9/16/98    Time: 11:05a
** Updated in $/devel/h3/Win95/dx/minivdd
** Added additional tvout functionality.
** 
** *****************  Version 45  *****************
** User: Stuartb      Date: 9/10/98    Time: 2:45p
** Updated in $/devel/h3/Win95/dx/minivdd
** TVOUT work in progress.
** 
** *****************  Version 44  *****************
** User: Andrew       Date: 8/24/98    Time: 6:52p
** Updated in $/devel/h3/Win95/dx/minivdd
** Need to set return value in ax in DPMS
** 
** *****************  Version 43  *****************
** User: Andrew       Date: 8/23/98    Time: 7:19p
** Updated in $/devel/h3/Win95/dx/minivdd
** Commented out the default case in VESASupport.  Put it back in.
** 
** *****************  Version 42  *****************
** User: Andrew       Date: 8/22/98    Time: 12:08p
** Updated in $/devel/h3/Win95/dx/minivdd
** Changed VESA Pre/Post Processing and Stop Hooking Int 10 since the bios
** is now doing DDC
** 
** *****************  Version 41  *****************
** User: Andrew       Date: 7/27/98    Time: 1:32p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added a parameter to SetVideoMode call.
** 
** *****************  Version 40  *****************
** User: Ken          Date: 7/23/98    Time: 4:04p
** Updated in $/devel/h3/win95/dx/minivdd
** added agp command fifo.   not added to NT build.  currently not
** functional on non-win98 systems without running a special enable apg
** script first, see Ken for that.   agp command fifo is enabled by
** setting the environment variable ACF=1 , all other settings disable it.
** Only turned on interatively in debugger in InitFifo call.   
** 
** *****************  Version 39  *****************
** User: Ken          Date: 7/18/98    Time: 6:41p
** Updated in $/devel/h3/win95/dx/minivdd
** added ability to use cmdfifo1 as the primary command fifo, #define
** PRIMARY_CMDFIFO at the top of inc\shared.h
** 
** *****************  Version 38  *****************
** User: Stuartb      Date: 6/23/98    Time: 10:16a
** Updated in $/devel/h3/Win95/dx/minivdd
** Added I2C multibyte writes.
** 
** *****************  Version 37  *****************
** User: Stuartb      Date: 6/11/98    Time: 8:34a
** Updated in $/devel/h3/Win95/dx/minivdd
** Added retrace interrupt support, check multiple banshees, share
** interrupt correctly.
** 
** *****************  Version 36  *****************
** User: Andrew       Date: 6/07/98    Time: 8:45a
** Updated in $/devel/h3/Win95/dx/minivdd
** Added DPMS processing
** 
** *****************  Version 35  *****************
** User: Stuartb      Date: 6/05/98    Time: 4:43p
** Updated in $/devel/h3/Win95/dx/minivdd
** Adding IRQ handling for Vsync.
** 
** *****************  Version 34  *****************
** User: Ken          Date: 6/01/98    Time: 11:49a
** Updated in $/devel/h3/win95/dx/minivdd
** cleaned up indentation and added comments for resetting of DRAM refresh
** in a mode set
** 
** *****************  Version 33  *****************
** User: Andrew       Date: 6/01/98    Time: 8:55a
** Updated in $/devel/h3/Win95/dx/minivdd
** Added code to set DRAM1[0] to 1.
** 
** *****************  Version 32  *****************
** User: Stuartb      Date: 5/05/98    Time: 1:20p
** Updated in $/devel/h3/Win95/dx/minivdd
** Adding i2c extEscape calls
** 
** *****************  Version 31  *****************
** User: Andrew       Date: 4/28/98    Time: 3:25p
** Updated in $/devel/h3/Win95/dx/minivdd
** Set Field Mtrr in DevTable to zero
** 
** *****************  Version 30  *****************
** User: Andrew       Date: 4/26/98    Time: 8:19a
** Updated in $/devel/h3/Win95/dx/minivdd
** Added a call to HelpVDD to get the VDD to make a bios call which causes
** it to set a flag such that when DDC is run a int 10 ax=0x3 is not
** generated when causes the FB to get corrupted.
** 
** *****************  Version 29  *****************
** User: Ken          Date: 4/21/98    Time: 6:52p
** Updated in $/devel/h3/win95/dx/minivdd
** clean up mode set, modes seem to set faster now too
** 
** *****************  Version 28  *****************
** User: Ken          Date: 4/15/98    Time: 6:41p
** Updated in $/devel/h3/win95/dx/minivdd
** added unified header to all files, with revision, etc. info in it
**
*/

/*
 * h3cvxd.c - Main Mini-vdd for 3Dfx Interactive Banshee (code named H3)
 */
#define WIN40SERVICES
#include "h3vdd.h"
#include "h3.h"

#define VDDONLY
#include "tv.h"
#include "h3g.h"
#include "h3cinitdd.h"
#include "devtable.h"
#include "h3irq.h"
#include "shared.h"
#undef  VDDONLY

#include "h3cinit.h"
#include "i2c/di_i2c.h"
#include "tvoutdef.h"
#include "bt868.h"
//////////DELETE-ME/////////#include "chrontel.h"
#include "xlcd.h"
#include "dfp.h"
#include "p6stuff.h"
#ifdef SLI_AA
#include <sliaa.h>
#endif
#include "bios.h"
#include "nvram.h"

extern const struct i2cmask I2C_PROTO;
extern const DWORD I2C_INITVAL;

#pragma VxD_LOCKED_DATA_SEG

// addition for Monitor Control Pos jmccartney
// Fix for building with Windows 98 DDK (REG_DWORD already defined)
#ifndef REG_DWORD
#define REG_DWORD ( 4 )  
#endif

// Note Bien!
// So as to not clutter up the include files with #ifdefs removing all
// references to floats, I'm defining _fltused here so that reference to floats
// can be made in macros/inline functions, etc., that aren't actually called.
// Also, you can't reference any floating point library calls -- the driver
// wouldn't link with 'em in there.  -KMW

DWORD _fltused;

// card configuration
#if 0
DWORD   IoBase;                 // base of PCI relocatable IO space
DWORD   PhysMemBase[2];         // physical register address space
DWORD   RegBase;                // CPU linear address mapping to PhysMemBase[0]
DWORD   LfbBase;                // CPU linear address mapping to PhysMemBase[1]
DWORD   MemSizeInMB;            // # of megabytes of board memory

BOOL    InterruptsEnabled = 1;

BOOL    bIsVGA;

#endif
DWORD   OldHandler;
DWORD   Old_3c2_Handler;

#if 0
FxU32 GdiDesktopEnabled;
#endif

#ifdef WINNT	// OLD CPUID CODE
FxU32 isP6;
#else
CPU_FEATURES cpuFeatures;
#endif

HVM   FullScreenVM;

// Windows data

HVM hSysVM;

// Miscellaneous

WORD     wVESATotalMemory;

// pageable data

#pragma VxD_PAGEABLE_DATA_SEG

// addition for Monitor Control Pos jmccartney
#define MAX_SCREEN_LEFT		10
#define MAX_SCREEN_RIGHT	-10
#define MAX_SCREEN_UP		26

// comms with VDD and Config manager
#if 0
CMCONFIG        ConfigData;              // resources of board
DISPLAYINFO     DispInfo;                // data from main VDD
#endif
// config space



// selector for interface

WORD    wMMIfSelector;

// jmccartney variables to contain increments that have been carried out on screen pos

int		monPos[2] = {0,0};

/*
TVOUT function pointers for device specific fns.  If we want to support
multiple Banshee cards with differing encoders this will need to be moved
into DevTable.  Right now, it's just Brooktree so it's initialized
statically.
*/

extern TVOUT_DEV_CALLS NullTVDevCalls;
extern TVOUT_DEV_CALLS Bt868DevCalls;
extern TVOUT_DEV_CALLS ChrontelDevCalls;

#define TvOutFn  (((TVOUT_DEV_CALLS *)pDev->tvOutFn))

/* lock.c */
extern FxU32 lockedVM;
extern void LockAPI_ClearLock(void);
extern FxU32 hwcPageMappingFind(const FxU32 mapAddr,
                                const FxU32 remapAddr);
// Begin STB Changes
/* kmtv.c */
extern DWORD  __stdcall GetKernelInfo(DIOCPARAMETERS * pDIOCParms);
extern DWORD __stdcall VDDtoWDMScale(DIOCPARAMETERS * lpParams);
extern DWORD __stdcall VDDtoWDMQueryActive(DIOCPARAMETERS * lpParams);
// in pldvmi.c
extern DWORD __stdcall VDDVMIFunctions(DIOCPARAMETERS *pDIOCPrams);


#define H3_VMI_INT_ENABLE                   0x00200000
#define H3_VMI_ENABLE						0x00000001
#define H3_VMI_RESET_MASK					0x10000000

#ifdef REAL_NET
#define H3_VSYNC_INT_ENABLE                   0x00000004
#endif

// End STB Changes
extern DWORD GetExtraAddr( DIOCPARAMETERS * pDIOCParms);
extern DWORD AllocalSysBuff( DIOCPARAMETERS * pDIOCParms);
extern DWORD QueryForVMIPld( PDEVTABLE );

BOOL FindPCIDevice(WORD wVendorID, WORD wDeviceID, PDEVNODE pDevNode);

// Monitor Position Control additions by jmccartney 3dfx Belfast
int SetCRTC(unsigned long readWrite, unsigned long inc, PDEVTABLE pDev);
int registryMonPos(int read, int values[], PDEVTABLE pDev);
void MonChangeMode(PDEVTABLE pDev);
int GetCRTC(PDEVTABLE pDev, int valueRequired);
void setCRTC_HSyncStart(PDEVTABLE pDev, char h_inc);
void setCRTC_VSyncStart(PDEVTABLE pDev, int v_inc);
void setCRTC_HSyncEnd(PDEVTABLE pDev, char h_inc);
int setMonSize(unsigned long incs[], PDEVTABLE pDev);
void setMonPos(unsigned long incs[], PDEVTABLE pDev);
void setCRTC_overlayPos(PDEVTABLE pDev, int inc);

/****************************************************************************
 Helpers
 ***************************************************************************/

#pragma VxD_ICODE_SEG
#pragma VxD_IDATA_SEG

extern void LockAPI_Init(void);

/*----------------------------------------------------------------------
Function name:  VDD_Get_Mini_Dispatch_Table

Description:    
                
Information:    uses in-line asm.

Return:         DWORD   return from VxDCall 
----------------------------------------------------------------------*/
DWORD 
VDD_Get_Mini_Dispatch_Table(DWORD **pDT)
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
Function name:  VDD_Takeover_VGA_Port

Description:    
                
Information:    uses in-line asm.

Return:         BOOL    return from VxDCall 
----------------------------------------------------------------------*/
BOOL VXDINLINE
VDD_Takeover_VGA_Port( DWORD dwPort, DWORD dwFunc )
{
    BOOL bR;

    __asm mov edx,dwPort;
    __asm mov ecx,dwFunc;
    VxDCall(VDD_Takeover_VGA_Port);
    __asm mov OldHandler, ecx;
    __asm mov eax,0;
    __asm adc al,0;
    __asm mov bR,eax;
    return(bR);
}


/*----------------------------------------------------------------------
Function name:  _BuildDescriptorDWORDs

Description:    
                
Information:    uses in-line asm.

Return:         QWORD   return from VxDCall 
----------------------------------------------------------------------*/
QWORD VXDINLINE
_BuildDescriptorDWORDs(DWORD dwBase, 
                      DWORD dwLimit, 
                      DWORD dwType, 
                      DWORD dwSize,
                      DWORD dwFlags)
{
    QWORD qwR;

    __asm push dwFlags;
    __asm push dwSize;
    __asm push dwType;
    __asm push dwLimit;
    __asm push dwBase;
    VMMCall(_BuildDescriptorDWORDs);
    __asm add esp,5*4;
    __asm mov dword ptr qwR,eax;
    __asm mov dword ptr qwR+4,edx;
    return(qwR);
}


/*----------------------------------------------------------------------
Function name:  _Allocate_LDT_Selector

Description:    
                
Information:    uses in-line asm.

Return:         WORD    return from VxDCall 
----------------------------------------------------------------------*/
WORD VXDINLINE
_Allocate_LDT_Selector( HVM hVM, 
                        DWORD dwDescH, 
                        DWORD dwDescL, 
                        DWORD dwCount,
                        DWORD dwFlags )
{
    WORD wR;

    __asm push dwFlags;
    __asm push dwCount;
    __asm push dwDescL;
    __asm push dwDescH;
    __asm push hVM;
    VMMCall(_Allocate_LDT_Selector);
    __asm add esp,5*4;
    __asm mov wR,ax;
    return(wR);
}


/*----------------------------------------------------------------------
Function name:  _Free_LDT_Selector

Description:    
                
Information:    uses in-line asm.

Return:         VOID
----------------------------------------------------------------------*/
VOID VXDINLINE
_Free_LDT_Selector( HVM hVM, 
                    DWORD dwSel, 
                    DWORD dwFlags )
{

    __asm push dwFlags;
    __asm push dwSel;
    __asm push hVM;
    VMMCall(_Free_LDT_Selector);
    __asm add esp,3*4;
}


/*----------------------------------------------------------------------
Function name:  MakeLDTEntry

Description:    
                
Information:

Return:         WORD    return from the _Allocate_LDT_Selector.
----------------------------------------------------------------------*/
WORD
MakeLDTEntry( DWORD dwBase, DWORD dwSize )
{
    union {
        QWORD qwDesc;
        DWORD dwDesc[2];
    } Desc;

    Desc.qwDesc = _BuildDescriptorDWORDs( dwBase,
                                          ((dwSize+4095)>>12)-1,
                                          RW_DATA_TYPE,
                                          D_PAGE32,
                                          0 );
    return( _Allocate_LDT_Selector( hSysVM,
                                    Desc.dwDesc[1],
                                    Desc.dwDesc[0], 1, 0 ) );
}

/****************************************************************************
 MiniVDD_Dynamic_Init
 ***************************************************************************/
void HookInt10(void);


/*----------------------------------------------------------------------
Function name:  MiniVDD_Dynamic_Init

Description:    
                
Information:

Return:         DWORD    VXD_SUCCESS or VXD_FAILURE
----------------------------------------------------------------------*/
#ifdef SLI_AA
extern DWORD dwWin98;
#endif
DWORD _stdcall MiniVDD_Dynamic_Init( HVM hWindowsVM )
{
    PDWORD pDispTab;        // VDD dispatch table
    volatile int i;
    DWORD dwRevision = 0;
    DWORD dwNFunc;
    PDEVTABLE pDev;

    hSysVM = hWindowsVM;
    FullScreenVM = hSysVM;

    // Initialize the Table    
    for (i=0; i<MAX_BANSHEE_DEVICES; i++)
      {
      DevTable[i].dwDevNode = 0x0;
      DevTable[i].lpDriverData = 0x0;
#ifdef SLI_AA
      DevTable[i].dwUnitNum = 0x0;
      DevTable[i].pSlave = NULL;
#endif
      DevTable[i].Mtrr = 0x0;
      DevTable[i].dwIMask = 0x0;
      DevTable[i].dwPowerReg[SGRAMMODE_INDEX] = DEFAULT_SGRAMMODE;
      DevTable[i].dwMemoryBus = MEMTYPE_SDRAM;
      DevTable[i].bBIOSVersion[0] = '\0';
#ifdef SLI_AA
      DevTable[i].dwDDCCount[0] = 0;
      DevTable[i].dwDDCCount[1] = 0;
      DevTable[i].dwDDCCount[2] = 0;
      DevTable[i].dwDDCCount[3] = 0;
#endif
#ifdef NOIRQ
      DevTable[i].InterruptsEnabled = 0;
#else
      DevTable[i].InterruptsEnabled = 0x1;
#endif // #ifdef NOIRQ    
      }

    pVGADevTable = NULL;
    dwNumDevices = 0;
    VDD_Get_DISPLAYINFO( &DevTable[0].DispInfo, sizeof(DevTable[0].DispInfo) );

    Debug_Printf("Are they the same DevNode %x %x\n",
                 DevTable[0].DispInfo.diDevNodeHandle,
                 DevTable[0].DispInfo.diInfoFlags);

    dwNFunc = VDD_Get_Mini_Dispatch_Table(&pDispTab);

#ifdef SLI_AA
    if (dwNFunc >= NBR_MINI_VDD_FUNCTIONS_41)
         dwWin98 = TRUE;
    else
         dwWin98 = FALSE;

    pDev = InitDevNode(DevTable[0].DispInfo.diDevNodeHandle, 0x0);   
#else
    pDev = InitDevNode(DevTable[0].DispInfo.diDevNodeHandle);   
#endif

    if (NULL == pDev)
    {
        Debug_Printf( VNAME "Failed InitDevNode\n" );
        return(VXD_FAILURE);
    }
      

 
    if (!(pDev->DispInfo.diInfoFlags & DEVICE_IS_NOT_VGA))
    {
        if (pVGADevTable != NULL)
        {
            pVGADevTable->GdiDesktopEnabled = 0;
        }
        else
        {
            Debug_Printf(VNAME "huh? VGA, but pVGADevTable is null!\n");
        }
        
        if (dwNFunc < NBR_MINI_VDD_FUNCTIONS)
        {
            Debug_Printf( VNAME "Too Few minivdd functions. Version confict?\n" );
            return(VXD_FAILURE);
        }

        pDispTab[REGISTER_DISPLAY_DRIVER]= (DWORD)RegisterDisplayDriver;
        pDispTab[DISPLAY_DRIVER_DISABLING]= (DWORD)DisplayDriverDisabling;
        pDispTab[VESA_CALL_POST_PROCESSING]= (DWORD)VESAPostSupport;
        pDispTab[VESA_SUPPORT] = (DWORD)VESASupport;
        pDispTab[PRE_HIRES_TO_VGA] = (DWORD)PreHiResToVGA;
//      pDispTab[POST_HIRES_TO_VGA] = (DWORD)PostHiResToVGA;
        pDispTab[PRE_VGA_TO_HIRES] = (DWORD)PreVGAToHires;
        pDispTab[POST_VGA_TO_HIRES] = (DWORD)PostVGAToHiRes;
        pDispTab[GET_TOTAL_VRAM_SIZE] = (DWORD)GetTotalVRAMSize;

        pDispTab[VIRTUALIZE_SEQUENCER_OUT] = (DWORD)VirtualizeSequencerOut;
        pDispTab[VIRTUALIZE_DAC_OUT] = (DWORD)VirtualizeDacOut;
        pDispTab[VIRTUALIZE_CRTC_OUT] = (DWORD)VirtualizeCRTCout;

        pDispTab[GET_VDD_BANK] = (DWORD)GetVDDbank;

        if (VDD_Takeover_VGA_Port(0x3c2, (DWORD)Virtual_3c2h))
        {
            Debug_Printf( VNAME "Unable to hook port 0x3c2\n" );
            return(VXD_FAILURE);
        }
        Old_3c2_Handler = OldHandler;

        /* dpc - 3 nov 98 - install stuff for locks from
         * fullscreen apps. This currently hooks the
         * CheckScreenSwitch vdd entry-point.
         */
        LockAPI_Init();

        if (dwNFunc >= NBR_MINI_VDD_FUNCTIONS_41)
        {
            pDispTab[TURN_VGA_OFF] = (DWORD)TurnVGAOff;
            pDispTab[TURN_VGA_ON] = (DWORD)TurnVGAOn;
        }
    }

    // update info key - ignore errors

//    CM_Write_Registry_Value( 0, "INFO", "ChipType",
//                             REG_SZ, "test", _lstrlen("test"),
//                             CM_REGISTRY_SOFTWARE );
    HookInt10();
#if 0
    CM_Write_Registry_Value( DispInfo.diDevNodeHandle, "INFO", "ChipType",
                             REG_SZ, pszChip, _lstrlen(pszChip),
                                                         CM_REGISTRY_SOFTWARE );
    CM_Write_Registry_Value( DispInfo.diDevNodeHandle, "INFO", "DACType",
                             REG_SZ, pszDac, _lstrlen(pszDac),
                             CM_REGISTRY_SOFTWARE );
    bVal = 1;
    CM_Write_Registry_Value( DispInfo.diDevNodeHandle, "INFO", "HWCursor",
                             REG_BINARY, &bVal, 1,
                             CM_REGISTRY_SOFTWARE );
    CM_Write_Registry_Value( DispInfo.diDevNodeHandle, "INFO", "Linear",
                             REG_BINARY, &bVal, 1,
                             CM_REGISTRY_SOFTWARE );
    CM_Write_Registry_Value( DispInfo.diDevNodeHandle, "INFO", "MMIO",
                             REG_BINARY, &bVal, 1,
                             CM_REGISTRY_SOFTWARE );
    dwRevision += '0';
    CM_Write_Registry_Value( DispInfo.diDevNodeHandle, "INFO", "Revision",
                             REG_SZ, &dwRevision, 1,
                             CM_REGISTRY_SOFTWARE );
    CM_Write_Registry_Value( DispInfo.diDevNodeHandle, "INFO", "VideoMemory",
                             REG_DWORD, &pMMIf->dwFBSize, sizeof(DWORD),
                             CM_REGISTRY_SOFTWARE );
#endif /* #if 0 KMW */


    return(VXD_SUCCESS);
}

#pragma VxD_LOCKED_CODE_SEG
#pragma VxD_LOCKED_DATA_SEG


/*----------------------------------------------------------------------
Function name:  VMM_Install_IO_Handler

Description:    
                
Information:    uses in-line asm.

Return:         DWORD    return from the VxDCall.
----------------------------------------------------------------------*/
DWORD VXDINLINE
VMM_Install_IO_Handler(DWORD pCallback, DWORD port)
{
    DWORD retval;

    __asm pushad;
    __asm mov   esi, pCallback;
    __asm mov   edx, port;
    VxDCall(Install_IO_Handler);
    __asm popad;
    __asm mov   eax, 0;
    __asm adc   al, 0;
    __asm mov   retval, eax;

    return retval;
}


/*----------------------------------------------------------------------
Function name:  VMM_Remove_IO_Handler

Description:    
                
Information:    uses in-line asm.

Return:         DWORD    return from the VxDCall.
----------------------------------------------------------------------*/
DWORD VXDINLINE
VMM_Remove_IO_Handler(DWORD port)
{
    DWORD retval;

    __asm pushad;
    __asm mov   edx, port;
    VxDCall(Remove_IO_Handler);
    __asm popad;
    __asm mov   eax, 0;
    __asm adc   al, 0;
    __asm mov   retval, eax;

    return retval;
}


/*----------------------------------------------------------------------
Function name:  VMM_Enable_Global_Trapping

Description:    
                
Information:    uses in-line asm.

Return:         VOID
----------------------------------------------------------------------*/
VOID VXDINLINE
VMM_Enable_Global_Trapping(DWORD port)
{
    __asm pushad;
    __asm mov   edx, port;
    VxDCall(Enable_Global_Trapping);
    __asm popad;
}


/*----------------------------------------------------------------------
Function name:  EnableIoHandler

Description:    
                
Information:    

Return:         VOID
----------------------------------------------------------------------*/
void
EnableIoHandler()
{
    DWORD port;
    
    if (NULL != pVGADevTable)
    {
        for (port = 0; port < 0x100; port += 4)
        {
            if (VMM_Install_IO_Handler((DWORD)myIOhandler,
                                       pVGADevTable->IoBase + port))
            {
                Debug_Printf( VNAME "port trap failed for port %d\n", port);
                return;
            }

            VMM_Enable_Global_Trapping(pVGADevTable->IoBase + port);
        }
    }
}


/*----------------------------------------------------------------------
Function name:  DisableIoHandler

Description:    
                
Information:    

Return:         VOID
----------------------------------------------------------------------*/
void
DisableIoHandler()
{
    DWORD port;
    
    if (NULL != pVGADevTable)
    {
        for (port = 0; port < 0x100; port += 4)
        {
            if (VMM_Remove_IO_Handler(pVGADevTable->IoBase + port))
            {
                Debug_Printf( VNAME "port untrap failed for port %d\n", port);
                return;
            }
        }
    }
}


/*----------------------------------------------------------------------
Function name:  VMM_SelectorMapFlat

Description:    
                
Information:    

Return:         FxU32   return from the VMMCall
----------------------------------------------------------------------*/
FxU32 VXDINLINE
VMM_SelectorMapFlat(FxU32 vm, FxU32 selector, FxU32 flags)
{
    FxU32 retval;
    
    __asm pushad;
    __asm push flags;
    __asm push selector;
    __asm push vm;
    VMMCall(_SelectorMapFlat);
    __asm add esp, 3*4;
    __asm mov retval, eax;
    __asm popad;

    return retval;
}


#define _NP() \
  __asm pushfd \
  __asm pushad \
  __asm mov ebp, esp \
  __asm sub esp, __LOCAL_SIZE

#define _NE() \
  __asm mov esp, ebp \
  __asm popad \
  __asm popfd \
  __asm ret

#define _NE2(x) \
  __asm mov esp, ebp \
  __asm popad \
  __asm popfd \
  __asm x \
  __asm ret

#define _NE_NORET() \
  __asm mov esp, ebp \
  __asm popad \
  __asm popfd


DWORD _4planeVirtArea;


/*----------------------------------------------------------------------
Function name:  GetVDDbank

Description:    
                
Information:    uses in-line asm.

Return:         VOID
----------------------------------------------------------------------*/
_declspec ( naked ) VOID
GetVDDbank( VOID )
{
    _NP();

    __asm mov _4planeVirtArea, ecx;

    _NE_NORET();
    __asm 
    {
        mov     ah, 0;          /* bank offset into the virtualization area */
        mov     edx, 0x8000;    /* 32 K virtualization area */
        ret;
    }
}


/*----------------------------------------------------------------------
Function name:  myIOhandler

Description:    virtualizes the pci relocatable i/o registers
                
Information:    

Return:         VOID
----------------------------------------------------------------------*/
_declspec ( naked ) VOID
myIOhandler( VOID )
{
    DWORD hVM, ioType;
    
    _NP();
    __asm mov   hVM, ebx;
    __asm mov   ioType, ecx;

    /* only allow output to i/o ports if we're not on the windows
     * desktop (i.e., in full screen VGA/VESA).  Always allow
     * input from h/w ports.
     */
    if ((FullScreenVM != hSysVM) || !(ioType & OUTPUT))
    {
        _NE_NORET();
        VxDCall(VDD_Do_Physical_IO);
        __asm ret;
    }
    
    _NE();
}


/*----------------------------------------------------------------------
Function name:  VirtualizeCRTCout

Description:    Called whenever a i/o trapped port write to
                the VGA crtc is made.
Information:    
from msdn:
 Call With
 EAX: Contains the value to output to port (on an OUT call). 
 EBX: Contains the VM handle for which the I/O is being done. 
 ECX: Contains the I/O flags (documented in VMM.INC). 
 EDX: Contains the port that the I/O is to be done to or from. 
 EBP: Points to VM's client registers. 
 Return Values
 CY set indicates that the mini-VDD has done the I/O and that the Main
 VDD should take no further action. In this case, AL should contain the
 return value from the IN call and all registers except EBX and EBP may
 be destroyed.
 NC set indicates that the mini-VDD may have taken some action but wants
 the Main VDD to do the default action. In this case the mini-VDD must
 not have destroyed ANY registers.

 3Dfx Virtualization strategy: we use the VGA crtc to control both the
 vga and hires video timings.  When displaying on the desktop, writes
 to the vga crtc thus need to be dumped on the floor.

Return:         VOID
----------------------------------------------------------------------*/
_declspec ( naked ) VOID
VirtualizeCRTCout( VOID )
{
    DWORD dumpAccess;
    DWORD hVM;
    
    _NP();
    __asm
    {
        mov     hVM, ebx;
    }

    dumpAccess = 0;

    if (pVGADevTable->GdiDesktopEnabled && (hVM == hSysVM))
    {
        dumpAccess = 1;
    }

    if (dumpAccess)
    {
        _NE2(stc);
    }
    else
    {
        _NE2(clc);
    }
}


/*----------------------------------------------------------------------
Function name:  VirtualizeSequencerOut

Description:    Parameters and return values are the same as
                VirtualizeCRTCout.
Information:    

Return:         VOID
----------------------------------------------------------------------*/
_declspec ( naked ) VOID
VirtualizeSequencerOut( VOID )
{
    DWORD dumpAccess;
    DWORD hVM;
    
    _NP();
    __asm mov   hVM, ebx;

    dumpAccess = 0;

    if (pVGADevTable->GdiDesktopEnabled && (hVM == hSysVM))
    {
        dumpAccess = 1;
    }
    
    if (dumpAccess)
    {
        _NE2(stc);
    }
    else
    {
        _NE2(clc);
    }
}


/*----------------------------------------------------------------------
Function name:  VirtualizeDacOut

Description:    Parameters and return values are the same as
                VirtualizeCRTCout
                
Information:    

Return:         VOID
----------------------------------------------------------------------*/
_declspec ( naked ) VOID
VirtualizeDacOut( VOID )
{
    DWORD dumpAccess;
    DWORD hVM;
    
    _NP();
    __asm mov   hVM, ebx;

    dumpAccess = 0;

    if (pVGADevTable->GdiDesktopEnabled && (hVM == hSysVM))
    {
        dumpAccess = 1;
    }
    
    if (dumpAccess)
    {
        _NE2(stc);
    }
    else
    {
        _NE2(clc);
    }
}


/*----------------------------------------------------------------------
Function name:  LockVgaTimingRegisters

Description:    

Information:    

Return:         VOID
----------------------------------------------------------------------*/
void
LockVgaTimingRegisters()
{
    PDEVTABLE pDev = pVGADevTable;
    FxU32 vgainit1;
    
    // lock VGA video timing registers
    //
    if (NULL != pDev)
    {
        vgainit1 = IGET32(vgaInit1);
        vgainit1 |= (BIT(21) | BIT(22) | BIT(23) | BIT(24) |
                     BIT(25) | BIT(26) | BIT(27) | BIT(28));
        ISET32(vgaInit1, vgainit1);
    }
}


/*----------------------------------------------------------------------
Function name:  UnLockVgaTimingRegisters

Description:    

Information:    

Return:         VOID
----------------------------------------------------------------------*/
void
UnlockVgaTimingRegisters()
{
    PDEVTABLE pDev = pVGADevTable;
    FxU32 vgainit1;
    
    // unlock VGA video timing registers
    //
    if (NULL != pDev)
    {
        vgainit1 = IGET32(vgaInit1);
        vgainit1 &= ~(BIT(21) | BIT(22) | BIT(23) | BIT(24) |
                      BIT(25) | BIT(26) | BIT(27) | BIT(28));
        ISET32(vgaInit1, vgainit1);
    }
}


/*----------------------------------------------------------------------
Function name:  EnableGdiDesktop

Description:    Called from SetVideoMode to setup the protections &
                virtualizations that will prevent DOS apps & the
                BIOS  from clobbering VGA video timing registers
                and relocatable IO registers that the desktop
                requires.   Called either on a full-screen DOS to
                desktop transition, or during a desktop mode change.
                Don't re-trap i/o on a dekstop mode change.
Information:    

Return:         VOID
----------------------------------------------------------------------*/
void
EnableGdiDesktop()
{
    LockVgaTimingRegisters();

    // don't re-enable the io handler if it's already enabled
    // (on a desktop to desktop mode change)
    //
    if (NULL != pVGADevTable)
    {
        if (pVGADevTable->GdiDesktopEnabled)
            return;

        EnableIoHandler();

        pVGADevTable->GdiDesktopEnabled = 1;
    }
}


#define SETDW(hwRegister, data)   hwRegister = data
/*----------------------------------------------------------------------
Function name:  DisableGdiDesktop

Description:    Called from a display driver VDDcall when the GDI
                desktop is being disabled and disables VGA/IO
                virtualizations, thus allowing VGA/VESA mode
                changes to occur.
Information:    

Return:         VOID
----------------------------------------------------------------------*/
void
DisableGdiDesktop(PDEVTABLE pDev)
{
    SstIORegs *lpIOregs;
    WORD wCmd;

    if (pDev->bIsVGA)
    {
        if (!pDev->GdiDesktopEnabled)
            return;

        UnlockVgaTimingRegisters();
        DisableIoHandler();
        lpIOregs = (SstIORegs * )pDev->RegBase[HWINFO_SST_IOREGS_INDEX];
        // Enable this when C-Simulator is ready
#if 0
        if (IS_NAPALM(pDev->dwVendorDeviceID))
            {
            SETDW(lpIOregs->lfbMemoryConfig, pDev->lfbMemoryConfig[0]);
            SETDW(lpIOregs->lfbMemoryConfig, pDev->lfbMemoryConfig[1] | SST_RAW_LFB_WRITE_TILE_COMPARE);
            }
        else
#endif
            SETDW(lpIOregs->lfbMemoryConfig, pDev->lfbMemoryConfig[0]);
    }
    else
    {
        // Read Command Register
        CM_Call_Enumerator_Function( pDev->dwDevNode,
                                     PCI_ENUM_FUNC_GET_DEVICE_INFO,
                                     0x4, &wCmd, 
                                     sizeof(WORD), 0 );
            
        // Turn off Everything
        wCmd &= ~(0x03);
        CM_Call_Enumerator_Function( pDev->dwDevNode,
                                     PCI_ENUM_FUNC_SET_DEVICE_INFO,
                                     0x4, &wCmd, 
                                     sizeof(WORD), 0 );
    }  

    pDev->GdiDesktopEnabled = 0;
}

//=======================================================================================
// DYNAMIC MODE TABLE begin
//=======================================================================================
TIMING_PARAMS GTF_Params;

//              r[0]  r[1]  r[2]  r[3]  r[4]  r[5]  r[6]  r[7]  r[8]  r[9]  r[10] r[11] r[12] r[13] r[14] r[15] r[16] r[17] r[18] r[19] r[20]
// x    y  rr,    0     1     2     3     4     5     6     7     9    10     11    12    15    16    1a    1b    c2   SR1  pllctrl0  dacmode
// x    y  rr, Htotl HDEnE HBlSt HBlEn HSySt HSyEn Vtotl Ovflw MxSLn VSySt  VSyEn VDEnE VBlSt VBlEn HExtn VExtn MiscO  SR1  pllctrl0  dacmode 

WORD crtc_table[CRTC_TABLE_SIZE];

#define FPU_FUNCTION_SAVE		0
#define FPU_FUNCTION_RESTORE	1
#define FPU_STATE_SAVE_SIZE		108

/*----------------------------------------------------------------------
Function name:  FPU_State

Description:    Save/Restore the state of the Floating Processor.

Information:

Return:         VOID
----------------------------------------------------------------------*/
void FPU_State( int function )
{
static unsigned char SaveArea[FPU_STATE_SAVE_SIZE];


	switch ( function )
	{
		case FPU_FUNCTION_SAVE:
			_asm { FSAVE SaveArea };
			break;
		case FPU_FUNCTION_RESTORE:
			_asm { FRSTOR SaveArea };
			break;
	}
}

/*----------------------------------------------------------------------
Function name:  Round

Description:    round a double to the nearest integer.

Information:

Return:         double  representing nearest whole number
----------------------------------------------------------------------*/
double Round( double x )
{
double wholepart;
double fraction;
int wholeint;


	_asm { FLD x };					// Read in the number to be truncated
	_asm { FISTP wholeint };		// Write it to a memory location

	wholepart = (double)wholeint;
	fraction = ( x - wholepart );

	if ( x >= 0.0 )					// Perform rounding for positive values
	{
		if ( fraction >= 0.5 )
			return ( wholepart + 1.0 );
		else
			return ( wholepart );
	}
	else						   	// Perform rounding for negative values
	{
		if ( fraction <= -0.5 )
			return ( wholepart - 1.0 );
		else
			return ( wholepart );
	}
}

/*----------------------------------------------------------------------
Function name:  FloatToInt

Description:    truncate a double to an integer.

Information:

Return:         int representing whole number part of a double
----------------------------------------------------------------------*/
int FloatToInt( double x )
{
int wholepart;


	_asm { FLD x };					// Read in the number to be truncated
	_asm { FISTP wholepart };		// Write it to a memory location

	return ( wholepart );
}

/*----------------------------------------------------------------------
Function name:  ds_Calc_CRTC_table

Description:    Calculate and fill the crtc_table[] with values taken from 
				the VESA BIOS EXTENSION 3 parameters or use detailed 
				timings if an alternate timing is specified.

Information:

Return:         VOID
----------------------------------------------------------------------*/
VOID ds_Calc_CRTC_table( VidProcConfig *pVpc, PDEVTABLE pDev )
{
TIMING_PARAMS *pVprm = &pVpc->TimingParams;
BYTE byHTotal, byHorDispEnEnd, byHBlankStart;
BYTE byHBlankEnd, byHSyncStart, byHSyncEnd;
BYTE byVTotal, byOverflow, byMaxLineScan;
BYTE byVSyncStart, byVSyncEnd, byVertDispEnEnd;
BYTE byVBlankStart, byVBlankEnd;
BYTE byHExtensions, byVExtensions;
BYTE byMiscOutput, byDacMode;
BYTE byScanLineDoubled;
BYTE byHSyncPolarity;
BYTE byVSyncPolarity;
BYTE byCRTCflags;
BYTE bySeqDotClk;
WORD wHVisible;
WORD wHTotal;
WORD wHBlankStart;
WORD wHBlankTime;
WORD wHSyncStart;
WORD wHSyncTime;
WORD wVVisible;
WORD wVTotal;
WORD wVBlankStart;
WORD wVBlankTime;
WORD wVSyncStart;
WORD wVSyncTime;
DWORD bClockDouble;
double dbPixelClock;
// variables used for finding the best pll #s
int m, k, bestm, bestn, bestk;
double test, rndtest, newoverflow, oldoverflow;
WORD wPllCtrl0;
#ifdef DEBUG
float answer;
WORD wPossiblePllCtrl0values[256];
WORD wPossiblePllCount = 0;
int iGoodTimings = 0;
#endif

	if ( pVprm->UseAltTiming )
	{
		for	( m = 0; m < sizeof( crtc_table ); m++ )
			crtc_table[m] = (WORD)pVprm->AltTiming[m];
		return;
	}
	 
	FPU_State( FPU_FUNCTION_SAVE );

	byCRTCflags = (BYTE)pVprm->CRTCflags;
	byScanLineDoubled = ( byCRTCflags & 1 );
	byHSyncPolarity = ( ( byCRTCflags << 4 ) & 0x40 );
	byVSyncPolarity = ( ( byCRTCflags << 4 ) & 0x80 );

	wHVisible = (WORD)( pVprm->width / pVprm->CharWidth ); // HVisible same as Hor Addr Time
	wHBlankStart = wHVisible; 	// We can assume this because there are no borders
	wHTotal = (WORD)( pVprm->HTotal / pVprm->CharWidth ); 
	wHBlankTime = ( wHTotal - wHVisible );
	wHSyncStart	= (WORD)( pVprm->HSyncStart / pVprm->CharWidth );
	wHSyncTime	= (WORD)( ( pVprm->HSyncEnd - pVprm->HSyncStart ) / pVprm->CharWidth );

	wVVisible = (WORD)pVprm->height;	 // VVisible same as Ver Addr Time
	if ( byScanLineDoubled )
		wVVisible *= 2;
	wVBlankStart = wVVisible;	// We can assume this because there are no borders
	wVTotal = (WORD)pVprm->VTotal;
	wVBlankTime = ( wVTotal - wVBlankStart );
	wVSyncStart	= (WORD)pVprm->VSyncStart;
	wVSyncTime  = (WORD)( pVprm->VSyncEnd - pVprm->VSyncStart );

	dbPixelClock = (double)pVprm->PixelClock;   
	dbPixelClock = ( dbPixelClock / 1000000 );	// Convert PixelClock from Hz to double

    if(pVpc->width != pVprm->width)  //Test for a dfp centered mode
    {
        WORD H2, V2;

        H2           = (WORD)(((pVprm->width - pVpc->width) / 2) / pVprm->CharWidth);
        wHVisible    = (WORD)( pVpc->width / pVprm->CharWidth );
        wHBlankStart -= H2;
        wHSyncStart  -= H2;

        V2           = (WORD)((pVprm->height - pVpc->height) / 2);
        wVVisible    = (WORD)pVpc->height;
        wVBlankStart -= V2;
        wVSyncStart  -= V2;
    }

	// Dacmode Bit0 set to 0 for 1:1 mode, 1 for 2:1 mode
	byDacMode = 0;

	// Test for DoubleDACRate
   bClockDouble = FALSE;
   if (IS_NAPALM(pDev->dwVendorDeviceID))
      {
      // The last or clause is needed since HBlank End is only 6 bits and it would need to be 9
      // this cause problem when switch between DOS and Hi-Rez
   	if(((dbPixelClock > 262.0) && ( pVprm->width >= 1280)) || (wHTotal > 261))
         bClockDouble = TRUE;
      }
   else
      {
	   if((dbPixelClock > 160.0) && ( pVprm->width >= 1280))
         bClockDouble = TRUE;
      }

   if (bClockDouble)
	{
		byDacMode = 1;
		wHVisible >>= 1;		
		wHBlankTime	= (wHBlankTime & 0x1) + (wHBlankTime>>1);
		wHSyncStart	= (wHSyncStart & 0x1) + (wHSyncStart>>1);
		wHSyncTime	= (wHSyncTime & 0x1) + (wHSyncTime>>1);
		wHTotal		= wHVisible + wHBlankTime;
      wHBlankStart = (wHBlankStart & 0x1) + (wHBlankStart>>1);
	}
    
	// Lower 8 bits of Horizontal Total
	byHTotal = 0xff & (wHTotal - 5);

	// Lower 8 bits of the horizontal display enable end
	byHorDispEnEnd = 0xff & (wHVisible - 1);

	// Lower 8 bits of the horizontal Blanking start
	byHBlankStart = 0xff & (wHBlankStart-1);

	// Finishing the Horizontal Blank End time
	// Assuming DisplayEnableSkew is 0 and Compatibility Read is on
   byHBlankEnd = 0x1f&(wHBlankTime + ((wHBlankStart-1) &0x3f)) | 0x80;
   byHSyncEnd = (0x20&(wHBlankTime + ((wHBlankStart-1)&0x3f)))<<2;

	// Filling in the lower 8 bits of the Horizontal Sync start value
	byHSyncStart = 0xff & (wHSyncStart-1);

	// Finishing the Horzontal Sync End time
	// Assuming HorizontalSyncSkew is 0
	byHSyncEnd |= (0x1f&(wHSyncTime + wHSyncStart-1));

	// Getting the lower 8 bits of the total # of vertical lines
	byVTotal = 0xff & (wVTotal - 2);

	// Filling in the Overflow register
	// Assuming the LineComp bit 8 is 1
	byOverflow = ((0x100 & (wVTotal-2))>>8) | ((0x100 & (wVVisible-1))>>7) |
		((0x100 & (wVSyncStart-1))>>6) | ((0x100 & (wVBlankStart-1))>>5) |
		0x10 | ((0x200 & (wVTotal-2))>>4) | ((0x200 & (wVVisible-1))>>3) |
		((0x200 & (wVSyncStart-1))>>2);

	// Filling in the MaxScanLine register
	// Assuming the LineComp bit 9 is 1
	byMaxLineScan = ((0x200 & (wVBlankStart-1))>>4) | 0x40 | 
		(byScanLineDoubled ? 0x80 : 0x0);

	// Filling in the lower 8 bits of Vertical Sync Start
	byVSyncStart = 0xff & (wVSyncStart-1);

	// Filling in the Vertical Sync End time
	// Assuming that we are enabling the Vertical Interrupt, allowing access to CR0-7,
	// and not clearing the interrupt
	byVSyncEnd = (0x0f & (wVSyncTime + wVSyncStart-1)) | 0x20;

	// Filling in the in lower 8 bits of the visible vertical lines
	byVertDispEnEnd = 0xff & (wVVisible-1);

	// Filling in the lower 8 bits of Vertical Blank Start
	byVBlankStart = 0xff & (wVBlankStart - 1);

	// Filling in the Vertical Blank End time
	byVBlankEnd = 0xff & (wVBlankTime + wVBlankStart - 1);

	// Filling in the Horizontal Extensions
   byHExtensions = ((0x100 & (wHTotal-5))>>8) |
	  ((0x100 & (wHVisible-1))>>6) | ((0x100 & (wHBlankStart-1))>>4) |
	  ((0x40&(wHBlankTime + ((wHBlankStart-1)&0x3f)))>>1) |
	  ((0x100 & (wHSyncStart-1))>>2) |
	  ((0x20&(wHSyncTime + wHSyncStart-1))<<2);

	// Filling in the Vertical Extensions
	byVExtensions = ((0x400 & (wVTotal-2))>>10) |
		((0x400 & (wVVisible-1))>>8) | ((0x400 & (wVBlankStart-1))>>6) | 
		((0x400 & (wVSyncStart-1))>>4);

	// Filling in Miscellaneous Output register
	// Asuming Clock Select is dictated by the Programmable PLL,
	// RAM is enabled, and using color mode CRTC addressing
	byMiscOutput = 0xf | byHSyncPolarity | byVSyncPolarity;

	if ( pVprm->CharWidth == 9 )
		bySeqDotClk = 0x20;
	else
		bySeqDotClk = 0x21;

	oldoverflow = 1000.0;  // big number
	bestn = bestm = bestk = 0;

	if(dbPixelClock > 150.0)
		k=1;
	else if (dbPixelClock > 65.0)
		k=2;
	else
		k=3;

	// Find the correct pllTable value for the pixel clock
	for(m=1 ; m<64; m++)
	{
		// m should not start at 0 (Found empirically that m=0 will cause the
		// equation to be 14.31818*(n+2)/(1*2^k) So m adds 1 but not 2 here.)
		// As per Yancy's email on Feb. 8, 1999, use only m values >=10
		if ( (dbPixelClock > 36.0) && (m>10))
			break;
		if ( (dbPixelClock > 200.0) && (m>5))
			break;

		test = (dbPixelClock) * ((double) m + 2.0) * 
			((double) (8>>(3-k))) / 14.31818;

		if(test>257.0)
			continue;

		rndtest = Round(test);

		newoverflow = dbPixelClock - 14.31818*rndtest/(((double) m + 2.0) * ((double) (8>>(3-k))));
		if(newoverflow < 0.0)
			newoverflow *= -1.0;

#ifdef DEBUG
		if((newoverflow / dbPixelClock < 0.005) && (wPossiblePllCount < 256))
		{
			wPossiblePllCtrl0values[wPossiblePllCount++] = ( (FloatToInt(rndtest-2.0)) <<8) | (m<<2) | k;
		}

		if(newoverflow == oldoverflow)
			iGoodTimings++;
#endif
			
		// The first values found will give better results than the older values
		if(newoverflow < oldoverflow)
		{
#ifdef DEBUG
			if(newoverflow != oldoverflow)
				iGoodTimings = 0;
#endif
			bestm = m;
			bestn = FloatToInt(rndtest - 2.0);
			bestk = k;
			oldoverflow = newoverflow;
		}
	}


#ifdef DEBUG
	answer = (14.31818f * ( ((float) bestn) + 2.0f)) / 
		( (((float) bestm) + 2.0f) * ((float) (8>>(3-bestk))) );
#endif

	wPllCtrl0 = (bestn<<8) | (bestm<<2) | bestk;

//  Define this if you want to see mode/pll information
//	Deubg_Printf("\n%dx%d @ %d Hz\n", pVpc->width, pVpc->height, pVpc->refresh);
//	Deubg_Printf("0x%04x, n=%d, m=%d, k=%d, 10*VCO=%d\n", wPllCtrl0, bestn, bestm, bestk, ((1431818*(bestn+2))/(bestm+2))/10000 );

	crtc_table[0]  = byHTotal;
	crtc_table[1]  = byHorDispEnEnd;
	crtc_table[2]  = byHBlankStart;
	crtc_table[3]  = byHBlankEnd;
	crtc_table[4]  = byHSyncStart;
	crtc_table[5]  = byHSyncEnd;
	crtc_table[6]  = byVTotal;
	crtc_table[7]  = byOverflow;
	crtc_table[8]  = byMaxLineScan;
	crtc_table[9]  = byVSyncStart;
	crtc_table[10] = byVSyncEnd;
	crtc_table[11] = byVertDispEnEnd;
	crtc_table[12] = byVBlankStart;
	crtc_table[13] = byVBlankEnd;
	crtc_table[14] = byHExtensions;
	crtc_table[15] = byVExtensions;
	crtc_table[16] = byMiscOutput;
	crtc_table[17] = bySeqDotClk;	// SR1
	crtc_table[18] = wPllCtrl0 & 0xff;
	crtc_table[19] = wPllCtrl0 >> 8;
	crtc_table[20] = byDacMode;

	FPU_State( FPU_FUNCTION_RESTORE );

}

/*----------------------------------------------------------------------
Function name:  di_GetGTF_Timing

Description:    Calculate GTF timings and fill the TIMING PARAMS structure
				with appropriate values.
Information:    

Return:         VOID
----------------------------------------------------------------------*/
VOID di_GetGTF_Timing( TIMING_PARAMS *pVprm )
{
double H_pixels_rnd;
double V_lines_rnd;
double V_frame_rate;
double V_field_rate;
double V_field_rate_rqd;
double V_field_rate_est;
double cell_gran;
double H_blank;
double H_sync_time;
double H_sync_start;
double H_period;
double H_period_est;
double V_back_porch;
double total_V_lines;
double total_pixels;
double total_active_pixels;
double ideal_duty_cycle;
double pixel_clock;
double vsync_plus_bp;
double min_vsync_plus_bp = 550;
double min_porch_rnd = 1;
double V_sync_rnd = 3;
double interlace = 0;
double sync_width = 0.08;
double M =  300;
double C =  30;


	V_field_rate_rqd = (double)pVprm->refresh;

	cell_gran = (double)pVprm->CharWidth;

	H_pixels_rnd = Round((double)pVprm->width / cell_gran ) * cell_gran;

	V_lines_rnd	= (double)pVprm->height;

	// half the number of vertical lines if interlace is requested
	if ( pVprm->CRTCflags & 2 )
	{
		V_lines_rnd = V_lines_rnd / 2;
		V_field_rate_rqd *= 2;
		interlace = 0.5;
	}

	if ( pVprm->CRTCflags & 1 )	// Double scanned value
	{
		V_lines_rnd = V_lines_rnd * 2;
	}	

	H_period_est = (( 1 / V_field_rate_rqd ) - min_vsync_plus_bp / 1000000 ) / 
					( V_lines_rnd + min_porch_rnd + interlace) * 1000000;

	vsync_plus_bp = Round( min_vsync_plus_bp / H_period_est );

	V_back_porch = vsync_plus_bp - V_sync_rnd;

	total_V_lines = V_lines_rnd + vsync_plus_bp + interlace + min_porch_rnd;

	V_field_rate_est = 1 / H_period_est / total_V_lines * 1000000;

	H_period = H_period_est	/ ( V_field_rate_rqd / V_field_rate_est );

	V_field_rate = 1 / H_period / total_V_lines	* 1000000;

	V_frame_rate = V_field_rate;

	// half the number of vertical lines if interlace is requested
	if ( pVprm->CRTCflags & 2 )
		V_frame_rate /= 2;
	
	total_active_pixels = H_pixels_rnd;
	
	ideal_duty_cycle = C - ( M * H_period / 1000 );

	H_blank = Round(( total_active_pixels * ideal_duty_cycle / (100 - ideal_duty_cycle ) / 
	                ( 2 * cell_gran ))) * ( 2 * cell_gran );

	total_pixels = total_active_pixels + H_blank; 

	pixel_clock = total_pixels / H_period;


	H_sync_time = Round( sync_width * ( total_pixels / cell_gran )) * cell_gran;

	H_sync_start = total_active_pixels + (( H_blank / 2 ) - H_sync_time );

	pVprm->HTotal = FloatToInt( total_pixels );
	pVprm->HSyncStart = FloatToInt( H_sync_start );
	pVprm->HSyncEnd = FloatToInt( H_sync_start + H_sync_time );
	pVprm->VTotal = FloatToInt( total_V_lines );
	pVprm->VSyncStart = FloatToInt( V_lines_rnd + min_porch_rnd );
	pVprm->VSyncEnd = FloatToInt( V_lines_rnd + min_porch_rnd + V_sync_rnd );
	pVprm->PixelClock = FloatToInt( pixel_clock * 1000000 );
#ifdef REAL_NET
   pVprm->refresh100 = FloatToInt(V_frame_rate * 100);
#endif
}

//=======================================================================================
// DYNAMIC MODE TABLE end
//=======================================================================================

/*----------------------------------------------------------------------
Function name:  IsVIACoreLogic

Description:    Detection code for VIA Chipsets

Information:    

Return:         VOID
----------------------------------------------------------------------*/
DWORD IsVIACoreLogic ( DWORD dwDevNodeIn, DWORD *dwRetDevNode )
{
    BYTE bValue;
    DWORD dwBackDoor;
    DWORD dwNewBackDoor;
	DWORD dwIsValidVIAChipset;
	DWORD dwFoundVIASubsys;
	DWORD dwDevNode;

    // VIA KX 133 is programmed wrong for AGP performance
    // rev it up!!!! <APS>
    // Apollo Pro Also!!!!!
    // MDM - KT133 also
    // Asus K7V shows up as 1106-0691

    dwIsValidVIAChipset = 0;
   
    dwFoundVIASubsys = 0;
   
    if( FindPCIDevice( 0x1106, 0x0391, &dwDevNode ) )
    {
      dwFoundVIASubsys = 0x0391; // KX133 ( AMD EV6 Bus Slot A )
    }
    else if ( FindPCIDevice( 0x1106, 0x0305, &dwDevNode ) )
    {
      dwFoundVIASubsys = 0x0305; // KT133 ( AMD EV6 Bus Socket A ) 
    }
    else if ( FindPCIDevice( 0x1106, 0x0691, &dwDevNode ) )
    {
      dwFoundVIASubsys = 0x0691; // Apollo Pro ( P6 Bus Slot 1 / Socket 370 )
    }
  
    if ( dwFoundVIASubsys != 0 )
    {
      CM_Call_Enumerator_Function( dwDevNode, PCI_ENUM_FUNC_GET_DEVICE_INFO,	 0xFC, &dwBackDoor, sizeof(dwBackDoor), 0);
      // Chip Set is using the VIA BackDoor to pretend to be a different chip
      if (dwBackDoor & 0x01)
      {
       dwNewBackDoor = dwBackDoor & ~(0x01);               
       CM_Call_Enumerator_Function( dwDevNode, PCI_ENUM_FUNC_SET_DEVICE_INFO, 0xFC, &dwNewBackDoor, sizeof(dwNewBackDoor), 0);
       CM_Call_Enumerator_Function( dwDevNode, PCI_ENUM_FUNC_GET_DEVICE_INFO, 0x00, &dwNewBackDoor, sizeof(dwNewBackDoor), 0);
       CM_Call_Enumerator_Function( dwDevNode, PCI_ENUM_FUNC_SET_DEVICE_INFO, 0xFC, &dwBackDoor, sizeof(dwBackDoor), 0);

       // We detected to be a valid KX133, KT133, or Apollo Pro chipset
       if (0x03911106 == dwNewBackDoor || // KX133 ( AMD EV6 Bus Slot A )
           0x03051106 == dwNewBackDoor)   // KT133 ( AMD EV6 Bus Socket A ) 
       {
         dwIsValidVIAChipset = 1;
       }
	   else if (0x06911106 == dwNewBackDoor) // Apollo Pro ( P6 Bus Slot 1 / Socket 370 )
	   {
	     // Need to check the revision number
		 // MDM Why do we need to check for the revision number?
		 //     The VIA chipsets still have problems at later revisions
         CM_Call_Enumerator_Function( dwDevNode, PCI_ENUM_FUNC_GET_DEVICE_INFO,	 0x8, &bValue, sizeof(bValue), 0);
         bValue &= 0xF0;

		 if ((0x80 == bValue) || (0xC0 == bValue))
		   dwIsValidVIAChipset = 1;
		 else
		   dwIsValidVIAChipset = 0;
	   }		 
      }
      else // We detected as a Valid VIA Chipset without any backdoor hacks
      {
	   // Only special case the revision for the Apollo Pro Chipsets
	   if ( dwFoundVIASubsys == 0x0691 )
	   {
	     // Need to check the revision number
		 // MDM Why do we need to check for the revision number?
		 //     The VIA chipsets still have problems at later revisions
         CM_Call_Enumerator_Function( dwDevNode, PCI_ENUM_FUNC_GET_DEVICE_INFO,	 0x8, &bValue, sizeof(bValue), 0);
         bValue &= 0xF0;

		 if ((0x80 == bValue) || (0xC0 == bValue))
		   dwIsValidVIAChipset = 1;
		 else
		   dwIsValidVIAChipset = 0;
	   } else if ( dwFoundVIASubsys == 0x0391 ||
	               dwFoundVIASubsys == 0x0305 )
	   {
         dwIsValidVIAChipset = 1;
	   }
      }
    }

	*dwRetDevNode = dwDevNode;

	return dwIsValidVIAChipset;
}

/*----------------------------------------------------------------------
Function name:  SetVideoMode

Description:    

Information:    

Return:         VOID
----------------------------------------------------------------------*/
DWORD GetNumChips(DWORD lpDriverData);
void
SetVideoMode(VidProcConfig *pVpc, PDEVTABLE pDev)
{
    FxU32 dramInit1;
    char path[64] = "SSTH3_VIDEO_REFRESH_OPTIMIZATION\0";
    char fullpath[40] = "DEFAULT\0";
    char value[8] = "\0";
	  FxU32 bufSize = 8;
#if 0
	int i;
	DWORD *pDesktopSurface;
#endif
    UnlockVgaTimingRegisters();

    // don't enable the video processor now, or you'll get an ugly
    // mode set (e.g., you'll see the contents of uninitialized memory)
    //
  	// Also set Bit 28 and 29.  This fixes a problem we were seeing with
	  // high-res modes in a heated environment.  srogers 5/15/00
    // Final decision - only 2 chip boards have the option in the registry
    if (CM_Read_Registry_Value(pDev->dwDevNode,
                               fullpath,
 							 								 path,
 							 								 REG_SZ,
 							 								 (BYTE *)value,
 							 								 &bufSize,
 							 								 CM_REGISTRY_SOFTWARE) == CR_SUCCESS)
    {
		    if(value[0] == '0')
            h3InitVideoProc(pDev->IoBase, SST_CURSOR_MICROSOFT | BIT(28) | BIT(29));
        else if(value[0] == '1')
            h3InitVideoProc(pDev->IoBase, SST_CURSOR_MICROSOFT);
        else // default case - Registry has something other than 0 or 1
        {
            if(GetNumChips(pDev->lpDriverData) == 2)  // Dual Chip Card
              h3InitVideoProc(pDev->IoBase, SST_CURSOR_MICROSOFT | BIT(28) | BIT(29));
            else
              h3InitVideoProc(pDev->IoBase, SST_CURSOR_MICROSOFT);
        }
    }
    else
    {
        if(GetNumChips(pDev->lpDriverData) == 2)  // Dual Chip Card
          h3InitVideoProc(pDev->IoBase, SST_CURSOR_MICROSOFT | BIT(28) | BIT(29));
        else
          h3InitVideoProc(pDev->IoBase, SST_CURSOR_MICROSOFT);
    }
    
    if (pVpc->changeDesktop)
    {
        h3InitVideoDesktopSurface(pDev->IoBase,
                                  pVpc->desktopSurface.enable,
                                  pVpc->desktopSurface.tiled,
                                  pVpc->desktopSurface.pixFmt,
                                  pVpc->desktopSurface.clutBypass,
                                  pVpc->desktopSurface.clutSelect,
                                  pVpc->desktopSurface.startAddress,
                                  pVpc->desktopSurface.stride);

    }
    
    if (pVpc->changeOverlay)
    {
        h3InitVideoOverlaySurface(pDev->IoBase,
                                  pVpc->overlaySurface.enable,
                                  pVpc->overlaySurface.stereo,
                                  pVpc->overlaySurface.horizScaling,
                                  pVpc->overlaySurface.dudx,
                                  pVpc->overlaySurface.verticalScaling,
                                  pVpc->overlaySurface.dvdy,
                                  pVpc->overlaySurface.filterMode,
                                  pVpc->overlaySurface.tiled,
                                  pVpc->overlaySurface.pixFmt,
                                  pVpc->overlaySurface.clutBypass,
                                  pVpc->overlaySurface.clutSelect,
                                  pVpc->overlaySurface.startAddress,
                                  pVpc->overlaySurface.stride);
    }
    
    if (pVpc->changeVideoMode)
    {
//=======================================================================================
// DYNAMIC MODE TABLE begins
//=======================================================================================
		ds_Calc_CRTC_table( pVpc, pDev);
//=======================================================================================
// DYNAMIC MODE TABLE ends
//=======================================================================================
        h3InitSetVideoMode(pDev,
                           pVpc->width,
                           pVpc->height,
                           pVpc->refresh,
                           1,
                           pVpc->desktopSurface.scanlinedouble); 

        // turn back on extended (non-vga) DRAM refresh -- the BIOS always
        // turns this off when setting a VGA mode.
        // 
        dramInit1 = IGET32(dramInit1);
        dramInit1 |= SST_DRAM_REFRESH_EN;
        ISET32(dramInit1, dramInit1);
    }

    EnableGdiDesktop();
	
#ifndef WIN_CSIM

    // check to see if flat panel is enabled
//???needed?    if (isPanelPresent(pDev) && isPanelActive(pDev))
//???needed?        panelOn(pDev);
   if (pDev->tvOutActive && TvOutFn)
     TvOutFn->tvout_Enable (pDev, -1);

#endif // WIN_CSIM

   	// make sure pDev knows the current resolution jmccartney
	pDev->DispInfo.diXRes = (unsigned short) pVpc->width;
	pDev->DispInfo.diYRes = (unsigned short) pVpc->height;

	MonChangeMode(pDev);   // used to set up the correct positioning during a mode switch jmccartney

}

/*----------------------------------------------------------------------
Function name:  VMM_Get_DDB

Description:    Used to get the DDB for the VMM

Information:    

Return:         PVMMDDB
----------------------------------------------------------------------*/
#define MY_GET_DDB GetVxDServiceOrdinal(Get_DDB)
VXDINLINE PVMMDDB VMM_Get_DDB(DWORD DeviceID, DWORD Name)
{
    DWORD p;
    __asm pushad
    __asm mov eax, DeviceID;
    __asm mov edi, Name;
    _asm _emit 0xcd \
    _asm _emit 0x20 \
    _asm _emit (MY_GET_DDB) & 0xff \
    _asm _emit (MY_GET_DDB >> 8) & 0xff \
    _asm _emit (MY_GET_DDB >> 16) & 0xff \
    _asm _emit (MY_GET_DDB >> 24) & 0xff; 
    __asm mov p, ecx;
    __asm popad
    return((PVMMDDB)p);
}

DWORD dwFirstTime = 0;
void HelpVDD(CLIENT_STRUCT * pCR, FxU32 vm);
void AgpService(AgpSrvc *pAs);
void GetBIOSInfo(PDEVTABLE pDevTable, FxU32 vm);

#ifdef SLI_AA
#ifdef FAKE_IT
SstIORegs * DevNodetoIORegs(DEVNODE devnode);
PDEVTABLE FindDevNode(DEVNODE oldDevNode, DWORD diUnitNumber)
{
   DEVNODE dwDevNode;
   int Found = FALSE;
   PDEVTABLE pDev = NULL;
   WORD wCmd;

   while (FindPCIDevice(0x121A, 0x0005, &dwDevNode))
      {
      if (oldDevNode != dwDevNode)
         {
         Found = TRUE;
         break;
         }
      }

   if (Found)
      {
      // Turn New On
      wCmd |= 0x03;
      CM_Call_Enumerator_Function( dwDevNode,
                                   PCI_ENUM_FUNC_SET_DEVICE_INFO,
                                   0x4, &wCmd, 
                                   sizeof(WORD), 0 );

      // Do Init
#ifdef SLI_AA
      pDev = InitDevNode(oldDevNode, diUnitNumber);
#else
      pDev = InitDevNode(dwDevNode);
#endif
      }

   
   return pDev;
}
#endif
#endif

#ifdef SLI_AA
#ifdef RD_ABORT_ERROR
void SLI_Read_Enable(PDEVTABLE pMaster);
void SLI_Read_Disable(PDEVTABLE pMaster);
#endif
#endif

//
// PLL and DAC registers
//
#define SST_DACMODE_HOLD_VSYNC			BIT(1)
#define SST_DACMODE_HOLD_HSYNC			BIT(3)
#define SST_DACMODE_DPMS_BITS			(SST_DACMODE_HOLD_HSYNC | SST_DACMODE_HOLD_VSYNC)
#define SST_DACMODE_DPMS_ON 			(0)
#define SST_DACMODE_DPMS_SUSPEND		(SST_DACMODE_HOLD_VSYNC)

#define STB_FUNCTION_VGA 1

/*----------------------------------------------------------------------
Function name:  CRT_SetActiveState

Description:    Enable or Disable the CRT device as selected by value of alternate
                display device mask passed in.

Information:

Return:         FxU32     STB_FUNCTION_VGA if success,
                          0 if the current mode does not allow completion.
----------------------------------------------------------------------*/
FxU32 CRT_SetActiveState (PDEVTABLE pContext, FxU32 activeStateMask)
{
	SstIORegs *sstIoRegs = (SstIORegs *)pContext->RegBase[HWINFO_SST_IOREGS_INDEX];
    ULONG result = STB_FUNCTION_VGA;
    ULONG ulTemp = sstIoRegs->dacMode;


    // check if CRT is supposed to be on.
    if ((activeStateMask & STB_FUNCTION_VGA) == STB_FUNCTION_VGA)
    { // CRT should be turned on
        // check if CRT if off and needs to be turned on
        if ((ulTemp & SST_DACMODE_DPMS_BITS) != SST_DACMODE_DPMS_ON)
        {
            // CRT is off and needs to be turned on.
            ulTemp &= ~SST_DACMODE_DPMS_SUSPEND;
            sstIoRegs->dacMode = ulTemp;
        }

    }
    else
    {// CRT should be turned off
        // check if CRT is on and needs to be turned off
        if ((ulTemp & SST_DACMODE_DPMS_BITS) == SST_DACMODE_DPMS_ON)
        {
            ulTemp |= SST_DACMODE_DPMS_SUSPEND;
            sstIoRegs->dacMode = ulTemp;
        }

    }
    return(result);
}

/*----------------------------------------------------------------------
Function name:  RegisterDisplayDriver

Description:    Called as a general communication method between
                the display driver and the mini VDD.

Information:    
  Entry..
  ebp   -   Client Register pointer
  ebx   -   VM 

Return:         PVMMDDB
----------------------------------------------------------------------*/
void InitRegswoDF(DWORD lpMasterDriverData, DWORD lpSlaveDriverData);
void AGPForceRate(PDEVTABLE pDev, HWAGPFORCERATE * pAGPForceRate);
#ifdef WIN_CSIM
int GetWINSIMIface(PDEVTABLE pDev, DWORD dwUnitNum);
#endif
_declspec ( naked ) VOID RegisterDisplayDriver( VOID )
{
    BYTE PCIConfSpace[256];
    PVMMDDB pDDB;
    CLIENT_STRUCT *pCR;           // client registers
    FxU32 vm;
    extern DWORD func_table[];
    VidProcConfig *pVpc;
    AgpSrvc *pAgpSrvc;
    HwInfo *pHwInfo;
    HWGETBIOSVERSION * pHWGetBIOSVersion;
	 HWGETOEMBOARDNAME * pHWGetOemBoardName;
	 HWGETCHIPNAME	* pHWGetChipName;
    HWAGPFORCERATE * pAGPForceRate;
    DFPDEVICEPACKET * pDfpDevicePacket;
    PDEVTABLE pDev;
    HWGETFLATADDRESS * pHwGetFlat;
    DWORD dwDevNode;
    //********struct i2cmask I2CMask;
    DWORD *lpdw;
    void *tvout;
    int i;
#ifdef SLI_AA
    PDEVTABLE pSlave;
    HWPCIOP * pHwPCIOp;
    CHIPINFO ChipInfo;
#endif
    I2CKEY key;
    int result;
    BYTE bValue;
    DWORD dwValue;
	DWORD dwIsValidVIAChipset;
    FxU8 bRegData;
   
	// vars added by jmccartney for Edge Tools MonitorControl page
	DWORD			*scrUpdate;
	int				regOp, registry1[2];

    __asm mov   eax, ebp;               // save ebp, _NP() will wack it
    _NP();
    __asm mov   pCR, eax;
    __asm mov   vm, ebx;
    
    switch( pCR->CRS.Client_ECX )
    {
    case H3VDD_GET_HW_INFO:
	// translate dd16's es:di into a flat pointer to return the
	// memory bases & memory size to the display driver
	//
	pHwInfo = (HwInfo *) VMM_SelectorMapFlat(vm,
						 pCR->CRS.Client_ES,
						 0);
	pHwInfo = (HwInfo *)((FxU32)pHwInfo + pCR->CWRS.Client_DI);

#ifdef SLI_AA
   if (0x0 != pHwInfo->diUnitNumber)
      {
      // Find DevNode
#ifdef FAKE_IT
      pDev = FindDevNode(pCR->CRS.Client_ESI, pHwInfo->diUnitNumber);      
#else
      pDev = FindPDEVFromDevNodeandUnitNum(pCR->CRS.Client_ESI, pHwInfo->diUnitNumber);
#endif
      pHwInfo->dwDevNode = pDev->dwDevNode;
      pHwInfo->dwSpecialNumber = ((DWORD)pDev - (DWORD)&DevTable[0]) / sizeof(DEVTABLE);
      }
   else
      {
   	pDev = FindPDEVFromDevNode(pCR->CRS.Client_ESI);
      pHwInfo->dwSpecialNumber = 0x0;

#ifdef WIN_CSIM
      {
      int k;
      int iNumChips;
      PDEVTABLE pNewDev;
      extern DWORD dwUseFakePCI;

      iNumChips = GetWINSIMIface(pDev, 0x0);
      for (k=1; k<iNumChips; k++)
         {
         pNewDev = InitDevNode(pCR->CRS.Client_ESI, k);
         }
      // Turn on Fake PCI Support
      // Leave it off until new release
      dwUseFakePCI = 1;
      }
#endif

      }
   pHwInfo->dwType = pDev->dwType;
   pHwInfo->dwNum = 1;
   if (SLI_AA_MASTER_DEVICE == pDev->dwType)
      {
      for (pSlave = pDev->pSlave; NULL != pSlave; pSlave = pSlave->pSlave)
         pHwInfo->dwNum++;      
#ifdef FAKE_IT
      pHwInfo->dwNum = 2;
#endif
      }
#else
   pDev = FindPDEVFromDevNode(pCR->CRS.Client_ESI);
#ifdef WIN_CSIM
   {
   int iNumChips;
   extern DWORD dwUseFakePCI;

   iNumChips = GetWINSIMIface(pDev, 0x0);
   // Turn on Fake PCI Support
   // Leave it off until new release
   dwUseFakePCI = 1;
   }
#endif
#endif
	pCR->CRS.Client_EAX = 0x0;
	if (NULL != pDev)
	   {
   	pCR->CRS.Client_EAX = 0x1;
      for (i=0; i<HWINFO_SST_MAX_BASE_INDEX; i++)
   	    pHwInfo->regBase[i] = pDev->RegBase[i];
#ifndef NOIRQ
	    // enable PCI interrupts, no selective interrupts enabled at this
	    // point so nothing should happen.
	    ((SstIORegs *)pDev->RegBase[HWINFO_SST_IOREGS_INDEX])->pciInit0 |= 0x40000;
#endif // #ifdef NOIRQ    
	    pHwInfo->lfbBase = pDev->LfbBase;
	    pHwInfo->ioBase = pDev->IoBase;
	    pHwInfo->memSizeInMB = pDev->MemSizeInMB;

#ifdef WIN_CSIM
	    pHwInfo->RealregBase = pDev->RealRegBase;
	    pHwInfo->ReallfbBase = pDev->RealLfbBase;
	    pHwInfo->FakeregBase = pDev->FakeRegBase;
	    pHwInfo->FakelfbBase = pDev->FakeLfbBase;
#endif         

	    CM_Call_Enumerator_Function( pDev->dwDevNode,
					 PCI_ENUM_FUNC_GET_DEVICE_INFO,
					 SST_PCI_DEVICE_ID, &PCIConfSpace,
					 sizeof(PCIConfSpace), 0);
 
	    pHwInfo->VendorDeviceID = *(FxU32 *)(&PCIConfSpace[SST_PCI_DEVICE_ID]);
	    pHwInfo->RevisionID = 0x0;
	    pHwInfo->RevisionID |= PCIConfSpace[SST_PCI_REVISION_ID];
	    pHwInfo->SSID = *(FxU32 *)(&PCIConfSpace[SST_PCI_SSID]);

	    // Determine if PLD is external to ASIC & set PLDRevisionID in PDevice.
	    pHwInfo->PLDRevisionID = QueryForVMIPld( pDev );

	    // Determine AGP capabilities

	    // Put this in to solve a problem with a Gateway box
	    // where AGP was not happening?????
	    pHwInfo->AGPCaps = 0x0;
       if (FALSE == pDev->dwPCI)
          {
          pHwInfo->AGPCaps |= IS_AGP_CARD;
	       if (pDev->bIsVGA)
            pHwInfo->AGPCaps |= IS_PRIMARY_DISPLAY;
          }              

       // Check to see if this is a K7
#ifdef WINNT	// OLD CPUID CODE
       if( (pHwInfo->AGPCaps & IS_AGP_CARD) && (isP6 == P6_NONINTEL_WITH_INTEL_MTRRS) )
#else
       if( (pHwInfo->AGPCaps & IS_AGP_CARD) && (cpuFeatures.dwCpuType & CPU_TYPE_AMD_K7) )
#endif
          {

          if( FindPCIDevice( 0x1022, 0x7006, &dwDevNode ) )
            {
            BYTE rev;

    	      CM_Call_Enumerator_Function( dwDevNode,
               PCI_ENUM_FUNC_GET_DEVICE_INFO,
               0x08, &rev,
               sizeof(BYTE), 0);

             if( rev < 0x23 )
                {
                pHwInfo->AGPCaps &= ~IS_PRIMARY_DISPLAY;
                }
            }
          }

      pDDB = VMM_Get_DDB(VMM_DEVICE_ID, NULL);
      if ((NULL != pDDB) && ((GetVxDServiceOrdinal(_GARTMemAttributes) & 0xFFFF) <= pDDB->DDB_Service_Table_Size))
         {
         pHwInfo->AGPCaps |= IS_GART_AVAILABLE; 
         }

#ifdef WINNT	// OLD CPUID CODE
      pHwInfo->cpuType = 0;
      if (isP6 == (P6_INTELCPU_WITH_MTRRS | P6_INTELCPU_WITH_KNI))
  		   pHwInfo->cpuType |= P6_INTELCPU_WITH_KNI;
#else
      pHwInfo->cpuType = cpuFeatures.dwCpuFlags;	// copy to global structure
#endif
	   }

    // Load up the compatibility setting for VIA Chipsets
	{
	  char path[64] = "SSTH3_VIA_COMPATIBILITY\0";
      char fullpath[40] = "DEFAULT\0";
      char value[8] = "\0";
      DWORD bufSize = 8;

      if (CM_Read_Registry_Value(pDev->dwDevNode,
                                 fullpath,
 		 						 path,
 		 						 REG_SZ,
 		 						 (BYTE *)value,
 		 						 &bufSize,
 		    					 CM_REGISTRY_SOFTWARE) != CR_SUCCESS) {
		dwValue = 0;
	  } else {
	    if ( value [0] == '0' ) {
		  dwValue = 0;
		} else if ( value [0] == '1' ) {
		  dwValue = 1;
		} else if ( value [0] == '2' ) {
		  dwValue = 2;
		} else {
		  dwValue = 0;
		}
	  }
	}
    
	dwIsValidVIAChipset = IsVIACoreLogic ( pDev->dwDevNode, &dwDevNode );

    // Check for specific VIA Core Logic which Likes PCI Init programmed differently
    pDev->dwVIACoreLogic = dwIsValidVIAChipset;
	pHwInfo->dwVIACoreLogic = dwIsValidVIAChipset;

    // We are either a KX133, KT133, or Via Apollo Pro
    if ( dwIsValidVIAChipset )
    {
      if (dwValue == 0)
      {
         CM_Call_Enumerator_Function( dwDevNode, PCI_ENUM_FUNC_GET_DEVICE_INFO,	 0x70, &bValue, sizeof(bValue), 0);

		 // Enable CPU-PCI Post Write Buffers
		 // And Enhanced CPU-PCI Write
         bValue |= 0x88;

         CM_Call_Enumerator_Function( dwDevNode, PCI_ENUM_FUNC_SET_DEVICE_INFO,	 0x70, &bValue, sizeof(bValue), 0);

		 CM_Call_Enumerator_Function( dwDevNode, PCI_ENUM_FUNC_GET_DEVICE_INFO,	 0x71, &bValue, sizeof(bValue), 0);

		 // Enable Dynamic Burst and PCI Burst
		 bValue |= 0x88;

         CM_Call_Enumerator_Function( dwDevNode, PCI_ENUM_FUNC_SET_DEVICE_INFO,	 0x71, &bValue, sizeof(bValue), 0);

         if (FindPCIDevice( 0x1106, 0x8391, &dwDevNode ) || 
             FindPCIDevice( 0x1106, 0x8305, &dwDevNode ) ||
             FindPCIDevice( 0x1106, 0x8598, &dwDevNode ) ||
             FindPCIDevice( 0x1106, 0x8391, &dwDevNode ))
         {
            CM_Call_Enumerator_Function( dwDevNode, PCI_ENUM_FUNC_GET_DEVICE_INFO,	 0x40, &bValue, sizeof(bValue), 0);

			// Enable CPU-AGP Post Write Buffers
            bValue |= 0x80;

            CM_Call_Enumerator_Function( dwDevNode, PCI_ENUM_FUNC_SET_DEVICE_INFO,	 0x40, &bValue, sizeof(bValue), 0);

            bValue = 0x72;
            CM_Call_Enumerator_Function( dwDevNode, PCI_ENUM_FUNC_SET_DEVICE_INFO,	 0x45, &bValue, sizeof(bValue), 0);
         }
      } else if (dwValue == 1) { // The user wants to disable us for compatibility reasons
         CM_Call_Enumerator_Function( dwDevNode, PCI_ENUM_FUNC_GET_DEVICE_INFO,	 0x70, &bValue, sizeof(bValue), 0);

		 // Disable CPU-PCI Post Write Buffers
		 bValue &= ~0x80;

         CM_Call_Enumerator_Function( dwDevNode, PCI_ENUM_FUNC_SET_DEVICE_INFO,	 0x70, &bValue, sizeof(bValue), 0);

//		 CM_Call_Enumerator_Function( dwDevNode, PCI_ENUM_FUNC_GET_DEVICE_INFO,	 0x71, &bValue, sizeof(bValue), 0);

		 // Disable Dynamic Burst and PCI Burst
//		 bValue &= ~0x88;

//         CM_Call_Enumerator_Function( dwDevNode, PCI_ENUM_FUNC_SET_DEVICE_INFO,	 0x71, &bValue, sizeof(bValue), 0);

         if (FindPCIDevice( 0x1106, 0x8391, &dwDevNode ) || 
             FindPCIDevice( 0x1106, 0x8305, &dwDevNode ) ||
             FindPCIDevice( 0x1106, 0x8598, &dwDevNode ) ||
             FindPCIDevice( 0x1106, 0x8391, &dwDevNode ))
         {
            CM_Call_Enumerator_Function( dwDevNode, PCI_ENUM_FUNC_GET_DEVICE_INFO,	 0x40, &bValue, sizeof(bValue), 0);

			// Disable CPU-AGP Post Write Buffers
            bValue &= ~0x80;

            CM_Call_Enumerator_Function( dwDevNode, PCI_ENUM_FUNC_SET_DEVICE_INFO,	 0x40, &bValue, sizeof(bValue), 0);

//            bValue = 0x72;
//            CM_Call_Enumerator_Function( dwDevNode, PCI_ENUM_FUNC_SET_DEVICE_INFO,	 0x45, &bValue, sizeof(bValue), 0);
         }
	  } // End if dwValue == 0 or User wants to disable the optimizations for compatibility reaons
    } // End if dwIsValidVIAChipset
	break;

    case H3VDD_SET_VIDEO_MODE:
	// The VDD needs a int 10 to warm up.  The first time the VDD
	// hooks a int 10 it issues a int 10 <ax=0x0003>.  This will corrupt
	// our frame buffer. This is late enough in the init process 
	// and is a good time to give it one.
	if (0 == dwFirstTime) 
	{
	    HelpVDD(pCR, vm);
	    dwFirstTime = 1;
	}

	// translate dd16's es:di into a flat pointer to get the vpc
	// settings for this mode
	pVpc = (VidProcConfig *) VMM_SelectorMapFlat(vm,
						     pCR->CRS.Client_ES,
						     0);
	pVpc = (VidProcConfig *)((FxU32)pVpc + pCR->CWRS.Client_DI);
#ifdef SLI_AA
	if (0x0 != pVpc->diUnitNumber)
      pDev = FindPDEVFromUnitNumber(pVpc->diUnitNumber);
   else
   	pDev = FindPDEVFromDevNode(pCR->CRS.Client_ESI);
#else
   pDev = FindPDEVFromDevNode(pCR->CRS.Client_ESI);
#endif
	if (NULL != pDev) {
      if (lockedVM != 0x00UL) 
         LockAPI_ClearLock();
#ifdef SLI_AA
      // Save Video Proc Config
      pDev->Vpc = *pVpc;
#endif
#ifdef SLI_AA
      // This was added to make sure that SLI and AA are disabled when
      // we exit Glide
      if (IS_NAPALM(pDev->dwVendorDeviceID))
         {
         ChipInfo.dwChips = 1;
         ChipInfo.dwsliEn = 0;
         ChipInfo.dwaaEn = 0;
         ChipInfo.dwaaSampleHigh = 0;
         ChipInfo.dwsliAaAnalog = 0;
         ChipInfo.dwsli_nlines = 0;
         ChipInfo.dwCfgSwapAlgorithm = 0;
         ChipInfo.dwMasterID = 0;
         ChipInfo.pDevTable = (DWORD)pDev;

         for (pSlave = pDev->pSlave; NULL != pSlave; pSlave = pSlave->pSlave)
            ChipInfo.dwChips++;
      
         H3_SETUP_SLI_AA(SLI_AA_DISABLE, &ChipInfo, NULL);
         }
#endif
      SetVideoMode(pVpc, pDev);
	}
	break;

    case H3VDD_GET_FN_TABLE32:
	pCR->CRS.Client_EAX = (FxU32) func_table;
	break;

    case H3VDD_DISABLE_GDI_DESKTOP:
	pDev = FindPDEVFromDevNode(pCR->CRS.Client_ESI);
	if (NULL != pDev)
	{
	    DisableGdiDesktop(pDev);
		if (isPanelActive(pDev))                      // if panel is active
			panelFixupVGA (pDev, 0);
		if (pDev->tvOutActive && TvOutFn)             // if tvout is active
			TvOutFn->tvout_FixupVGA (pDev, 0);
	}
	break;

    case H3VDD_I2C_WRITE:
	    if (!(pDev = FindPDEVFromDevNode(pCR->CRS.Client_ESI)))
	    {
	        pCR->CRS.Client_EAX = 0xDEADBEEF;
	        break;
	    }

      pCR->CRS.Client_EAX = 0;    /* failed */
      key = i2c_getaccess(pDev, I2C_TVENCODER, I2C_NORMALSPEED);
      if (key == I2C_NOTAKEY)
        break;

      result = 1;
      result &= i2c_stop(pDev, key);
      result &= i2c_start(pDev, key);
      result &= i2c_sendbyte(pDev, key, (FxU8)((pCR->CRS.Client_EDX >> 16) | I2C_WRITESLAVE)  );
      result &= i2c_sendbyte(pDev, key, (FxU8)((pCR->CRS.Client_EDX >> 8)) );
      result &= i2c_sendbyte(pDev, key, (FxU8)((pCR->CRS.Client_EDX >> 0)) );
      result &= i2c_stop(pDev, key);

      i2c_endaccess(pDev, key);
      pCR->CRS.Client_EAX = result;
      break;


/********
	    I2CMask = I2C_PROTO;
	    I2CMask.pReg = (DWORD *)(pDev->RegBase[HWINFO_SST_IOREGS_INDEX] + I2COUT_PORT);
       *I2CMask.pReg = ((*I2CMask.pReg & ~I2C_INITMASK) | I2C_INITVAL);
	    I2CMask.bAddr = (FxU8)(pCR->CRS.Client_EDX >> 16);
	    I2CMask.nReg = (FxU8)(pCR->CRS.Client_EDX >> 8);
	    I2CMask.dwData = (FxU8)(pCR->CRS.Client_EDX >> 0);
	    /* Debug_Printf( VNAME "i2c parm = %x\n", pCR->CRS.Client_EDX); * /
	    pCR->CRS.Client_EAX = WriteI2CRegister (&I2CMask, 0);
	    break;
*********/

    case H3VDD_I2C_MULTI_WRITE:
      {
        int i, size;
        FxU8* pdata;

	      if (!(pDev = FindPDEVFromDevNode(pCR->CRS.Client_ESI)))
	      {
	        pCR->CRS.Client_EAX = 0xDEADBEEF;
	        break;
	      }
	      // convert 16bit _far ptr to flat
	      lpdw = (DWORD*)VMM_SelectorMapFlat(vm, pCR->CRS.Client_EDX >> 16, 0);
	      lpdw = (DWORD *)((FxU32)lpdw + (pCR->CRS.Client_EDX & 0xffff));

        result = 1;
        result &= i2c_stop(pDev, key);
        result &= i2c_start(pDev, key);
        result &= i2c_sendbyte(pDev, key, (FxU8)((pCR->CRS.Client_EDX >> 16) | I2C_WRITESLAVE));
        result &= i2c_sendbyte(pDev, key, (FxU8)((pCR->CRS.Client_EDX >> 8)));

        size = (FxU8)((pCR->CRS.Client_EDX >> 0) & 0x000000FF);
        pdata = (FxU8*)lpdw[1];

        for (i=0; i<size; i++)
          result &= i2c_sendbyte(pDev, key, pdata[i]);

        result &= i2c_stop(pDev, key);

        i2c_endaccess(pDev, key);
        pCR->CRS.Client_EAX = result;
      }
      break;

/********
	I2CMask = I2C_PROTO;
	I2CMask.pReg = (DWORD *)(pDev->RegBase[HWINFO_SST_IOREGS_INDEX] + I2COUT_PORT);
   *I2CMask.pReg = ((*I2CMask.pReg & ~I2C_INITMASK) | I2C_INITVAL);
	I2CMask.bAddr = (FxU8)(lpdw[0] >> 16);
	I2CMask.nReg = (FxU8)(lpdw[0] >> 8);
	I2CMask.nSize = (FxU8)(lpdw[0] >> 0);
	I2CMask.multiWriteData = (FxU8 *)lpdw[1];
	/* Debug_Printf( VNAME "i2c parm = %x\n", pCR->CRS.Client_EDX); * /
	pCR->CRS.Client_EAX = WriteI2CRegisterMulti (&I2CMask, 0);
	break;
********/

    case H3VDD_I2C_READ:
	    if (!(pDev = FindPDEVFromDevNode(pCR->CRS.Client_ESI)))
	    {
	        pCR->CRS.Client_EAX = 0xDEADBEEF;
	        break;
	    }

      pCR->CRS.Client_EAX = 0;    /* failed */
      key = i2c_getaccess(pDev, I2C_TVENCODER, I2C_NORMALSPEED);
      if (key == I2C_NOTAKEY)
        break;

      result = 1;
      result &= i2c_stop(pDev, key);
      result &= i2c_start(pDev, key);
      result &= i2c_sendbyte(pDev, key, (FxU8)((pCR->CRS.Client_EDX >> 8) | I2C_WRITESLAVE)  );
      result &= i2c_sendbyte(pDev, key, (FxU8)(pCR->CRS.Client_EDX >> 0)  );

      result &= i2c_start(pDev, key);
      result &= i2c_sendbyte(pDev, key, (FxU8)((pCR->CRS.Client_EDX >> 8) | I2C_READSLAVE)  );
      result &= i2c_readbyte(pDev, key, &bRegData, 0);
      result &= i2c_stop(pDev, key);

      i2c_endaccess(pDev, key);
      pCR->CRS.Client_EAX = (result ? (((FxU32)bRegData) | 0x100) : 0UL);
      break;


/********
	I2CMask = I2C_PROTO;
	I2CMask.pReg = (DWORD *)(pDev->RegBase[HWINFO_SST_IOREGS_INDEX] + I2COUT_PORT);
  *I2CMask.pReg = ((*I2CMask.pReg & ~I2C_INITMASK) | I2C_INITVAL);
	I2CMask.bAddr = (FxU8)(pCR->CRS.Client_EDX >> 8);
	I2CMask.nReg = (FxU8)(pCR->CRS.Client_EDX >> 0);
	/* Debug_Printf( VNAME "i2c parm = %x\n", pCR->CRS.Client_EDX); * /
	if (FXTRUE == ReadI2CRegister (&I2CMask, 0))
	    pCR->CRS.Client_EAX = I2CMask.dwData | 0x100;
	else
	    pCR->CRS.Client_EAX = 0;    /* failed * /

      break;
********/

    case H3VDD_AGP_SERVICE:
	pAgpSrvc = (AgpSrvc *) VMM_SelectorMapFlat(vm,
						   pCR->CRS.Client_ES,
						   0);
	pAgpSrvc = (AgpSrvc *)((FxU32)pAgpSrvc + pCR->CWRS.Client_DI);
	AgpService(pAgpSrvc);
	//
	// AgpService places the return value into the AgpSrvc structure
	//
	break;

    case H3VDD_GET_BIOS_VERSION:
	pCR->CRS.Client_EAX = 0xFFFFFFFF;
	pDev = FindPDEVFromDevNode(pCR->CRS.Client_ESI);
	if (NULL != pDev)
	{
	    pHWGetBIOSVersion = (HWGETBIOSVERSION *)VMM_SelectorMapFlat(vm, pCR->CRS.Client_ES, 0);
	    if (0xFFFFFFFF == (DWORD)pHWGetBIOSVersion)
		break; 
	    pHWGetBIOSVersion = (HWGETBIOSVERSION *)((DWORD)pHWGetBIOSVersion + pCR->CWRS.Client_DI);
	    for (i=0; i<MAX_BIOS_VERSION_STRING; i++)
		pHWGetBIOSVersion->bBIOSVersion[i] = pDev->bBIOSVersion[i];                   
	    pCR->CRS.Client_EAX = 0;
	}
	break;

    case H3VDD_GET_OEM_BOARD_NAME:
		pCR->CRS.Client_EAX = 0xFFFFFFFF;
		pDev = FindPDEVFromDevNode(pCR->CRS.Client_ESI);
		if (NULL != pDev)
		{
	   	pHWGetOemBoardName = (HWGETOEMBOARDNAME *)VMM_SelectorMapFlat(vm, pCR->CRS.Client_ES, 0);
	    	if (0xFFFFFFFF == (DWORD)pHWGetOemBoardName)
				break; 
	    	pHWGetOemBoardName = (HWGETOEMBOARDNAME *)((DWORD)pHWGetOemBoardName + pCR->CWRS.Client_DI);
	    	for (i=0; i<MAX_BIOS_VERSION_STRING; i++)
				pHWGetOemBoardName->bOEMBoardName[i] = pDev->bOEMBoardName[i];
	    	pCR->CRS.Client_EAX = 0;
		}
	break;

    case H3VDD_GET_CHIP_NAME:
		pCR->CRS.Client_EAX = 0xFFFFFFFF;
		pDev = FindPDEVFromDevNode(pCR->CRS.Client_ESI);
		if (NULL != pDev)
		{
	   	pHWGetChipName = (HWGETCHIPNAME *)VMM_SelectorMapFlat(vm, pCR->CRS.Client_ES, 0);
	    	if (0xFFFFFFFF == (DWORD)pHWGetChipName)
				break; 
	    	pHWGetChipName = (HWGETCHIPNAME *)((DWORD)pHWGetChipName + pCR->CWRS.Client_DI);
	    	for (i=0; i<MAX_BIOS_VERSION_STRING; i++)
				pHWGetChipName->bChipName[i] = pDev->bChipName[i];                   
	    	pCR->CRS.Client_EAX = 0;
		}
	break;

   case H3VDD_GET_FLAT_ADDRESS:
      pHwGetFlat = (HWGETFLATADDRESS *)VMM_SelectorMapFlat(vm,
            pCR->CRS.Client_ES, 0x0);
      pHwGetFlat = (HWGETFLATADDRESS *)((FxU32)pHwGetFlat + pCR->CWRS.Client_DI);
      pHwGetFlat->FlatAddress = (DWORD)VMM_SelectorMapFlat(vm,
            ((DWORD)pHwGetFlat->pData >> 16) & 0xFFFF, 0x0);
      pHwGetFlat->FlatAddress += ((FxU32)pHwGetFlat->pData & 0xFFFF);
      pCR->CRS.Client_EAX = 0;
      break;

    case H3VDD_INITFIFO:
   	pDev = FindPDEVFromDevNode(pCR->CRS.Client_ESI);
	   if (NULL != pDev)
         InitRegswoDF(pDev->lpDriverData, pDev->lpDriverData);
      break;

#ifdef SLI_AA
    case H3VDD_CLEAR_SLAVE_BIT:
   	pDev = FindPDEVFromDevNode(pCR->CRS.Client_ESI);
      // FIX_ME should I only look at master's here?
	   if (NULL != pDev)
         ClearAllSlaveBits(pDev);
      break;

   case H3VDD_PCI_OP:
      pHwPCIOp = (HWPCIOP *)VMM_SelectorMapFlat(vm, pCR->CRS.Client_ES, 0x0);
      pHwPCIOp = (HWPCIOP *)((FxU32)pHwPCIOp + pCR->CWRS.Client_DI);
   	pDev = FindPDEVFromDevNode(pCR->CRS.Client_ESI);
	   pCR->CRS.Client_EAX = 0x1;
      if (NULL != pDev)
         {
         if (H3G_PCI_READ == pHwPCIOp->dwOp)
            pHwPCIOp->dwValue = PCI_Read_Config(pDev->dwBus, pDev->dwDevFunc | (pHwPCIOp->dwFunc & 0x03), pHwPCIOp->dwOffset);
         else
            PCI_Write_Config(pDev->dwBus, pDev->dwDevFunc | (pHwPCIOp->dwFunc & 0x03), pHwPCIOp->dwOffset, pHwPCIOp->dwValue);
         }
      else
	      pCR->CRS.Client_EAX = 0xDEADBEEF;
      break;

#ifdef RD_ABORT_ERROR
    case H3VDD_ENABLE_SLI_READ:
   	pDev = FindPDEVFromDevNode(pCR->CRS.Client_ESI);
	   if (NULL != pDev)
         SLI_Read_Enable(pDev);
      break;

    case H3VDD_DISABLE_SLI_READ:
   	pDev = FindPDEVFromDevNode(pCR->CRS.Client_ESI);
	   if (NULL != pDev)
         SLI_Read_Disable(pDev);
      break;

#endif
#endif

    case H3VDD_AGP_FORCE_RATE:
   	pDev = FindPDEVFromDevNode(pCR->CRS.Client_ESI);
	   if (NULL != pDev)
         {
	   	pAGPForceRate = (HWAGPFORCERATE *)VMM_SelectorMapFlat(vm, pCR->CRS.Client_ES, 0);
	    	if (0xFFFFFFFF == (DWORD)pAGPForceRate)
				break;
         pAGPForceRate = (HWAGPFORCERATE *)((DWORD)pAGPForceRate + pCR->CWRS.Client_DI);
         AGPForceRate(pDev, pAGPForceRate);
         }
      break;

    case H3VDD_GET_TVINIT_STATUS:
	if (!(pDev = FindPDEVFromDevNode(pCR->CRS.Client_ESI)))
	{
		pCR->CRS.Client_EAX = 0xDEADBEEF;
		break;
	}
	// check if initialized
	if (((TVOUT_CURSETUP *)pDev->tvOutData)->encoderInitAttempt)
	{
		if (TvOutFn == &Bt868DevCalls)
		{
		    if (IS_VOODOO3(pDev->dwVendorDeviceID))
            {
		   	    READSCRATCHREGISTER2 (pDev, pCR->CRS.Client_EAX);
            }
            else
            {
            // manufacture a status byte that looks like the Voodoo3 Bios Scratch Register 2
                pCR->CRS.Client_EAX = (FxU8)nvramRead(pDev, NV_TVSTANDARD);
		   	    READSCRATCHREGISTER2 (pDev, bRegData);
                if (bRegData & BIOS_CRx1E_TVACTIVE)
                    pCR->CRS.Client_EAX |= BIOS_TVOUT_ACTIVE;
            }
		}
        else
        {
            pCR->CRS.Client_EAX = 0;    // no tv hardware case
        }
	}
	else
	    // if unitialized return -1
		pCR->CRS.Client_EAX = 0xffffffff;

	break;

   case H3VDD_GET_TVNVRAM_STATUS:
	   if (!(pDev = FindPDEVFromDevNode(pCR->CRS.Client_ESI)))
	   {
		   pCR->CRS.Client_EAX = 0;  //default to no tv attached
	   }
      else
      {
         pCR->CRS.Client_EAX = GetTvoutInfoNVRAM(pDev);
      }
   	break;


	//TV-Out support functions     
    case H3VDD_GET_TVSTATUS:
    case H3VDD_SET_TVSTANDARD:        
    case H3VDD_SET_TVCONTROL:
    case H3VDD_SET_TVPOSITION:
    case H3VDD_SET_TVSIZE:
    case H3VDD_SET_SPECIAL:
    case H3VDD_GET_TVPOSITION_CTRL:
    case H3VDD_GET_TVSIZE_CTRL:
    case H3VDD_GET_TVPIC_CTRL:
    case H3VDD_GET_FILTER_CTRL:
    case H3VDD_DISABLE_TV:
    case H3VDD_SET_REGISTRY_INFO:
    case H3VDD_GET_REGISTRY_INFO:
    case H3VDD_GET_PIC_CAPS:
    case H3VDD_GET_FILTER_CAPS:
    case H3VDD_GET_POS_CAPS:
    case H3VDD_GET_SIZE_CAPS:
    case H3VDD_GET_STANDARD:
    case H3VDD_MACROVISION_ON:
    case H3VDD_MACROVISION_OFF:
    case H3VDD_ALLOW_CRT_WITH_TV:
    case H3VDD_ALLOW_PAL_AND_CRT:
	if (!(pDev = FindPDEVFromDevNode(pCR->CRS.Client_ESI)))
	{
	    pCR->CRS.Client_EAX = 0xDEADBEEF;
	    break;
	}
	if (TvOutFn == NULL)
	{
	  GetBIOSInfo(pDev, vm);

      // Initialize Digital Flat Panel if any.
      DFP_Initialize(pDev);

      switch (pCR->CRS.Client_ECX)
      {
         case H3VDD_SET_REGISTRY_INFO:
         case H3VDD_GET_REGISTRY_INFO:
            break;

         default:
            ((TVOUT_CURSETUP *)pDev->tvOutData)->encoderInitAttempt = 1;
            if (BT868_GetStatus (pDev, 0) >= 0)
            {
               TvOutFn = &Bt868DevCalls;
               BT868_SetupTvOut(pDev);
            }
            else
            {
               TvOutFn = &NullTVDevCalls;
               pCR->CRS.Client_EAX = 0xDEADBEEF;
            }
            break;
      }
	}
	tvout = (PTVSETSTANDARD)VMM_SelectorMapFlat(vm, pCR->CRS.Client_ES, 0);
	tvout = (PTVSETSTANDARD)((FxU32)tvout + pCR->CWRS.Client_DI);
	switch (pCR->CRS.Client_ECX)
	{
      case H3VDD_ALLOW_CRT_WITH_TV:
         pDev->dwAllowCRTwithTV = *(DWORD*)tvout;
         break;

      case H3VDD_ALLOW_PAL_AND_CRT:
         pDev->dwAllowPALandCRT = *(DWORD*)tvout;
         break;

	case H3VDD_GET_TVSTATUS:
	    pCR->CRS.Client_EAX = (FxU32)TvOutFn->tvout_GetStatus (pDev, 0);
	    if (pCR->CRS.Client_EAX == 0xffffffff)    // error
		break;
	    pCR->CRS.Client_EAX = (pCR->CRS.Client_EAX << 8) | 
		(FxU32)TvOutFn->tvout_GetStatus (pDev, 1);
	    pCR->CRS.Client_EAX = (pCR->CRS.Client_EAX << 8) | 
		(FxU32)TvOutFn->tvout_GetStatus (pDev, 2);

	    pCR->CRS.Client_EAX &= ~0x80;

	    if (pDev->tvOutActive)
		    pCR->CRS.Client_EAX |= 0x80;
		else
			(FxU32)TvOutFn->tvout_Disable (pDev);
	    break;
	case H3VDD_SET_TVSTANDARD:
	    pCR->CRS.Client_EAX = \
			(FxU32)TvOutFn->tvout_SetStandard ((PTVSETSTANDARD)tvout, pDev);
	    break;
	case H3VDD_SET_TVCONTROL:
	    pCR->CRS.Client_EAX =  \
		(FxU32)TvOutFn->tvout_SetPicControl ((PTVSETCAP)tvout, pDev);
	    break;
	case H3VDD_SET_TVPOSITION:
	    pCR->CRS.Client_EAX =  \
		(FxU32)TvOutFn->tvout_SetPosition ((PTVSETPOS)tvout, pDev);
	    break;
	case H3VDD_SET_TVSIZE:
	    pCR->CRS.Client_EAX = \
		(FxU32)TvOutFn->tvout_SetSize ((PTVSETSIZE)tvout, pDev);
	    break;
	case H3VDD_SET_SPECIAL:
	    pCR->CRS.Client_EAX =  \
		(FxU32)TvOutFn->tvout_SetSpecial((PTVSETSPECIAL)tvout,
						 pDev->RegBase[HWINFO_SST_IOREGS_INDEX]);
	    break;
	case H3VDD_DISABLE_TV:
	    pCR->CRS.Client_EAX =  \
		(FxU32)TvOutFn->tvout_Disable (pDev);
	    break;
	case H3VDD_GET_TVPOSITION_CTRL:
	    pCR->CRS.Client_EAX =  \
		TvOutFn->tvout_GetPosition ((PTVCURPOS)tvout, &pDev->tvOutData);
	    break;
	case H3VDD_GET_TVPIC_CTRL:
	    pCR->CRS.Client_EAX =  \
		TvOutFn->tvout_GetPicControl ((PTVCURCAP)tvout,
					      &pDev->tvOutData);
	    break;
	case H3VDD_GET_FILTER_CTRL:
	    break;
	case H3VDD_GET_REGISTRY_INFO:
	    pCR->CRS.Client_EAX = tvOutGetNextValue ((FxU8 *)tvout,
						     (TVOUT_CURSETUP *)&pDev->tvOutData);
	    break;
	case H3VDD_SET_REGISTRY_INFO:
	    pCR->CRS.Client_EAX = tvOutSetNextValue ((FxU8 *)tvout,
						     pCR->CRS.Client_EDX, (TVOUT_CURSETUP *)&pDev->tvOutData);
	    break;
	case H3VDD_GET_STANDARD:
	    pCR->CRS.Client_EAX = TvOutFn->tvout_GetStandard (pDev, tvout);
	    break;
	case H3VDD_GET_SIZE_CAPS:
	    pCR->CRS.Client_EAX = TvOutFn->tvout_GetSizeCap (pDev, tvout);
	    break;
	case H3VDD_GET_POS_CAPS:
	    pCR->CRS.Client_EAX = TvOutFn->tvout_GetPosCap (pDev, tvout);
	    break;
	case H3VDD_GET_FILTER_CAPS:
	    pCR->CRS.Client_EAX = TvOutFn->tvout_GetFilterCap (pDev, tvout);
	    break;
	case H3VDD_GET_PIC_CAPS:
	    pCR->CRS.Client_EAX = TvOutFn->tvout_GetPicCap (pDev, tvout);
	    break;
	case H3VDD_GET_TVSIZE_CTRL:
	    pCR->CRS.Client_EAX = \
		TvOutFn->tvout_GetSizeControl (tvout, &pDev->tvOutData);
	    break;
	case H3VDD_MACROVISION_ON:
	    pCR->CRS.Client_EAX = \
		TvOutFn->tvout_CopyProtect (pDev, *(int *)tvout);
	    break;
	case H3VDD_MACROVISION_OFF:
	    pCR->CRS.Client_EAX = TvOutFn->tvout_CopyProtect (pDev, 0);
	    break;
	}
	break;

    case H3VDD_LINEAR_MAP_OFFSET:
    {
	MapInfo* clientData;
	FxU32* linAddr;
	FxU32 selBase = VMM_SelectorMapFlat(vm,
					    pCR->CRS.Client_ES,
					    0);
	clientData = (MapInfo*)(selBase + pCR->CWRS.Client_DX);
	linAddr = (FxU32*)(selBase + pCR->CWRS.Client_DI);

	*linAddr = hwcPageMappingFind(clientData->mapAddr,
				      clientData->remapAddr);
    }
    break;

    case H3VDD_DFP_REQUEST:
	    pCR->CRS.Client_EAX = 0xffffffff;
	    pDfpDevicePacket = (DFPDEVICEPACKET *)VMM_SelectorMapFlat(vm, pCR->CRS.Client_ES, 0);
	    if (0xFFFFFFFF == (DWORD)pDfpDevicePacket)
		    break; 
        pDfpDevicePacket = (DFPDEVICEPACKET *)((FxU32)pDfpDevicePacket + pCR->CWRS.Client_DI);
	    if (!(pDev = FindPDEVFromDevNode(pCR->CRS.Client_ESI)))
	    {
	        pCR->CRS.Client_EAX = 0xDEADBEEF;
	        break;
	    }
	    pCR->CRS.Client_EAX = DFP_ProcessRequest (pDev, pDfpDevicePacket);
	    break;

    case H3VDD_RW_REGISTER:
	if (!(pDev = FindPDEVFromDevNode(pCR->CRS.Client_ESI)))
	{
	    pCR->CRS.Client_EAX = 0xDEADBEEF;
	    break;
	}
	tvout = (void *)VMM_SelectorMapFlat(vm, pCR->CRS.Client_ES, 0);
	tvout = (void *)((FxU32)tvout + pCR->CWRS.Client_DI);
	lpdw = (DWORD *)(pDev->RegBase[HWINFO_SST_IOREGS_INDEX] + ((SstIoRegRW *)tvout)->regAddr);

	if (((SstIoRegRW *)tvout)->regRWflags & RWFLAG_WRITE_BYTE)
	{
	    outp ((FxU16)(pDev->IoBase + \
			  ((SstIoRegRW *)tvout)->regAddr),
		  (FxU8)((SstIoRegRW *)tvout)->regValue);
	}
	else if (((SstIoRegRW *)tvout)->regRWflags & RWFLAG_WRITE_REG)
	    *lpdw = ((SstIoRegRW *)tvout)->regValue;
	else
	    ((SstIoRegRW *)tvout)->regValue = *lpdw;
	break;

   case H3VDD_MOVE_SCREEN:  // jmccartney code addition by edge tools request

		/* This case changes the position of the image on the monitor.  A record
		   of what changes has been made to the position is stored in the variable,
		   monPos.  This is kept incase the user wants to revert to the previous
		   settings.

		   monPos[0] is amount moved horizontally
		   monPos[1] is amount moved vertically
		*/

		pDev = FindPDEVFromDevNode(pCR->CRS.Client_ESI);  //jmccartney 3dfx Belfast

		scrUpdate = (DWORD *)VMM_SelectorMapFlat(vm, pCR->CRS.Client_EDX >> 16, 0);		
		scrUpdate = (DWORD *)((FxU32)scrUpdate + (pCR->CRS.Client_EDX & 0xffff));
		
		// scrUpdate[0] contains the horizontal increment value
		// scrUpdate[1] contains the vertical increment value

		// increment monPos because this global stores 
		// how much has been moved for Resetting to Previous position
		monPos[0] += scrUpdate[0];
		monPos[1] += scrUpdate[1];

		regOp = registryMonPos(1, registry1, pDev);
		if (regOp != 1)
			registry1[0] = registry1[1] = 0;// encase there is a problem with reading the values

		/*	following three lines included to stop moving the screen down due to the
			problems found when moving the screen down.  
			Overlay for video is out of sync when moving down
			As is hardware cursor and the hotspot for the mouse.*/

		regOp = registry1[1] + monPos[1];
		if (regOp < 0)
		{
				monPos[1] = monPos[1] - scrUpdate[1];
				scrUpdate[1] = 0;
		}

		registry1[0] += monPos[0];
		registry1[1] += monPos[1];

		/*	Error checking to stop moving the image too far and causing the 
			monitor to display no picture */
		
		if (registry1[0] > MAX_SCREEN_LEFT)
		{
			monPos[0] -= scrUpdate[0];
			scrUpdate[0] = 0;
		}
		else if (registry1[0] < MAX_SCREEN_RIGHT)
		{
			monPos[0] -= scrUpdate[0];
			scrUpdate[0] = 0;
		}
		
		if (registry1[1] > MAX_SCREEN_UP)
		{
			monPos[1] -= scrUpdate[1];;
			scrUpdate[1] = 0;
		} 

		// call the function that changes the position of the image
		// by modifying the CRTC register relating to the position
		setMonPos(scrUpdate, pDev);
		
		pCR->CRS.Client_EAX = 0xDEADBEEF;  // return value	
		break;

	case H3VDD_SET_MON_POS_REGISTRY:  //jmccartney 3dfx Belfast

		/* This case saves the changes made by the user to the position of the image on the monitor
		   into the registry so that they can be recalled.  This gets called when the user clicks
		   apply or OK on the monitor control page, ie the user is happy with the settings and wants
		   to store them
		*/
		pDev = FindPDEVFromDevNode(pCR->CRS.Client_ESI);

		scrUpdate = (DWORD *)VMM_SelectorMapFlat(vm, pCR->CRS.Client_EDX >> 16, 0);
		scrUpdate = (DWORD *)((FxU32)scrUpdate + (pCR->CRS.Client_EDX & 0xffff));

		// init. to 0
		registry1[0] = registry1[1] = 0;
		/* call a function to read from the registry where the image was positioned previously */
		regOp = registryMonPos(1, registry1, pDev);
		if (regOp != 1)
			registry1[0] = registry1[1] = 0;					// encase there is a problem with reading the values
		
		// add on the new amount moved so that it can be written into the registry
		registry1[0] += monPos[0];
		registry1[1] += monPos[1];	
		// write the new value into the registry.
		regOp = registryMonPos(2, registry1, pDev);
		// reset the global var that keeps track of the movements
		monPos[0] = monPos[1] = 0;

		pCR->CRS.Client_EAX = 0x00000009;  // return nothing really

		break;

	case H3VDD_RESTORE_POS:  //jmccartney 3dfx Belfast

		/* 	This case is used when the user has moved the image but
			wants to recall his previous settings.  This case restores
			the image to its previous position.  It is also used to restore
			the factory default settings for the card.
		*/
	
		pDev = FindPDEVFromDevNode(pCR->CRS.Client_ESI);
		
		scrUpdate = (DWORD *)VMM_SelectorMapFlat(vm, pCR->CRS.Client_EDX >> 16, 0);			// do some crazy magic
		scrUpdate = (DWORD *)((FxU32)scrUpdate + (pCR->CRS.Client_EDX & 0xffff));
		
		// init. registry1 variable
		registry1[0] = registry1[1] = 0;

		// if scrUpdate[0] != 0 then we are resetting to factory defaults
		if (scrUpdate[0])
		{
			// read in the position stored in the registry
			regOp = registryMonPos(1, registry1, pDev);
			/* 	We want to store the amount moved from the previous position
				so that if the user doesn't like factory defaults he can revert	to the previous position.
				We use regOp as a temp var to copy the values into monPos
			*/
			regOp = registry1[0];
			registry1[0] += monPos[0];
			monPos[0] = -regOp;
			regOp = registry1[1];
			registry1[1] += monPos[1];
			monPos[1] = -regOp;
			// set the crtc registers to their previous values
		    registry1[0] = -registry1[0];
			registry1[1] = -registry1[1];
			setMonPos(registry1, pDev);
		}
		else
		{
			/* if scrUpdate[0] == 0 then we are resetting to previous Position
			   reset the crtc registers to what they where before the user moved the image
			   position around */
			monPos[0] = -monPos[0];
			monPos[1] = -monPos[1];
			setMonPos(monPos, pDev);
			monPos[0] = monPos[1] = 0;
		}
		pCR->CRS.Client_EAX = 0x00000009;
		break;
	case H3VDD_RESIZE_SCREEN:

		/* 	This case will in the future resize the image on the monitor.  However at the minute
			resizing causes some of the image at the top or left to be "blacked out". */
		pDev = FindPDEVFromDevNode(pCR->CRS.Client_ESI);
		
		scrUpdate = (DWORD *)VMM_SelectorMapFlat(vm, pCR->CRS.Client_EDX >> 16, 0);			// do some crazy magic
		scrUpdate = (DWORD *)((FxU32)scrUpdate + (pCR->CRS.Client_EDX & 0xffff));

		//regOp = setMonSize(scrUpdate, pDev); //not used at this time because it doesn't work as expected
		break;

    case H3VDD_FLATPNL_EDID:
	if (!(pDev = FindPDEVFromDevNode(pCR->CRS.Client_ESI)))
	{
	    pCR->CRS.Client_EAX = 0xDEADBEEF;
	    break;
	}
// the following definitions duplicate definitions in monitor.c
#define EDID_20_SIZE 128
#define MAX_EDID_SIZE (EDID_20_SIZE + 2)
// currently monitor.c does not provide anymore space than this so don't return any more
// data or you will trash memory.  DanO 6/2/2000
	pCR->CRS.Client_EAX = 
	       panelEDID(pDev, (FxU8 *)(pCR->CWRS.Client_DI + VMM_SelectorMapFlat(vm, pCR->CRS.Client_ES, 0)), MAX_EDID_SIZE);
	break;

	case H3VDD_GET_GTF_TIMINGS:	  // DYNAMIC MODE TABLE	begins
		{
		extern VOID di_GetGTF_Timing( TIMING_PARAMS *pVprm );
		TIMING_PARAMS * pParams;

		// translate dd16's es:di into a flat pointer to return the
		// memory bases & memory size to the display driver
		//
		pParams = (TIMING_PARAMS *)VMM_SelectorMapFlat(vm,
						 pCR->CRS.Client_ES,
						 0);
		pParams = (TIMING_PARAMS *)((FxU32)pParams + pCR->CWRS.Client_DI);

		di_GetGTF_Timing( pParams );
		}
	break;						 // DYNAMIC MODE TABLE ends

	case H3VDD_SET_ALT_DISPLAYS:
		{
		DWORD *pio;
		DWORD result = 0;
		pDev = FindPDEVFromDevNode(pCR->CRS.Client_ESI);

		// translate dd16's es:di into a flat pointer to return the
		// memory bases & memory size to the display driver
		//
		pio = (DWORD *)VMM_SelectorMapFlat(vm, pCR->CRS.Client_ES, 0);
		pio = (DWORD *)((FxU32)pio + pCR->CWRS.Client_DI);

		// set TvOut display as requested.
		result |= BT868_SetActiveState(pDev, *pio);

		// set DFP display as requested.
		result |= DFP_SetActiveState(pDev, *pio);

		// set VGA display as requested.
		result |= CRT_SetActiveState(pDev, *pio);

		// fill in return result		
		*pio = result;
		}
	break;

    default:           
	pCR->CRS.Client_EAX = 0xDEADBEEF;
	break;
    }

    _NE();
}

/*----------------------------------------------------------------------
Function name:  AGPForceRate

Description:    Use to Force the AGP rate

Information:    
  Entry..
Return:         VOID
----------------------------------------------------------------------*/
void AGPForceRate(PDEVTABLE pDev, HWAGPFORCERATE * pAGPForceRate)
{
   DWORD dwData;
   DWORD dwBus;
   DWORD dwDevFunc;
   DWORD dwLoc;
   DWORD dwHostBus;
   DWORD dwHostDevFunc;
   DWORD dwRate;
   DWORD dwReg;

   // First we need to see if we are enabled
   CM_Call_Enumerator_Function(pDev->dwDevNode, PCI_ENUM_FUNC_GET_DEVICE_INFO, SST_AGP_CMD_ID, &dwData, sizeof(dwData), 0);
   if (0x0 == (dwData & 0x07)) 
      return;

   // Next we need to find the Host Chip Set
   dwLoc = 0x100;
   for (dwBus = 0x0; (dwBus < 256) && (0x100 == dwLoc); dwBus+=1)
      {
      for (dwDevFunc = 0x0; (dwDevFunc < 0x100) && (0x100 == dwLoc); dwDevFunc +=0x08)
         {
         dwData = PCI_Read_Config(dwBus, dwDevFunc, 0x0);
         if (0xFFFFFFFF == dwData)
            continue;

         dwData = PCI_Read_Config(dwBus, dwDevFunc, 0x8);

         // Host Bridge Chip???
         if (0x06000000 != (dwData & 0xFFFF0000))
            continue;                     

         // Status Indicate Linked list of Attributes
         dwData = PCI_Read_Config(dwBus, dwDevFunc, 0x4);
         if (0x100000 != (dwData & 0x100000))
            continue;

         dwData = PCI_Read_Config(dwBus, dwDevFunc, SST_1ST_CAP);
         dwReg = dwData & 0xFF;
         while (0x0 != dwReg)                     
            {
            dwData = PCI_Read_Config(dwBus, dwDevFunc, dwReg);
            if (SST_AGP_CAP == (dwData & 0xFF))
               {
               dwLoc = dwReg;
               dwHostBus = dwBus;
               dwHostDevFunc = dwDevFunc;
               dwData = 0;
               }
            dwReg = (dwData >> 8) & 0xFF;
            }
         }   
      }

   // If we found the Host Chip Set
   if (0x100 != dwLoc)
      {
      // Disable Us
      dwData = PCI_Read_Config(pDev->dwBus, pDev->dwDevFunc, SST_AGP_CMD_ID);
      dwData &= 0xFFFFFEFF;
      PCI_Write_Config(pDev->dwBus, pDev->dwDevFunc, SST_AGP_CMD_ID, dwData);

      // Disable Them
      dwData = PCI_Read_Config(dwHostBus, dwHostDevFunc, dwLoc+8);
      dwData &= 0xFFFFFEFF;
      PCI_Write_Config(dwHostBus, dwHostDevFunc, dwLoc+8, dwData);

      // Check that rate is not beyond host
      dwData = PCI_Read_Config(dwHostBus, dwHostDevFunc, dwLoc+4);
      dwRate = dwData & 0x07;
      if ((pAGPForceRate->dwRate > dwRate) || (0x0 == (dwRate & pAGPForceRate->dwRate)))
         pAGPForceRate->dwRate = 0x01;

      // Check that rate is not beyond us
      dwData = PCI_Read_Config(pDev->dwBus, pDev->dwDevFunc, SST_AGP_STATUS_ID);
      dwRate = dwData & 0x07;
      if ((pAGPForceRate->dwRate > dwRate) || (0x0 == (dwRate & pAGPForceRate->dwRate)))
         pAGPForceRate->dwRate = 0x01;

      // Force Them
      dwData = PCI_Read_Config(dwHostBus, dwHostDevFunc, dwLoc+8);
      dwData &= 0xFFFFFFF8;
      dwData |= pAGPForceRate->dwRate;
      PCI_Write_Config(dwHostBus, dwHostDevFunc, dwLoc+8, dwData);

      // Force Us
      dwData = PCI_Read_Config(pDev->dwBus, pDev->dwDevFunc, SST_AGP_CMD_ID);
      dwData &= 0xFFFFFFF8;
      dwData |= pAGPForceRate->dwRate;
      PCI_Write_Config(pDev->dwBus, pDev->dwDevFunc, SST_AGP_CMD_ID, dwData);

      // Enable Them
      dwData = PCI_Read_Config(dwHostBus, dwHostDevFunc, dwLoc+8);
      dwData |= 0x100;
      PCI_Write_Config(dwHostBus, dwHostDevFunc, dwLoc+8, dwData);

      // Enable Us
      dwData = PCI_Read_Config(pDev->dwBus, pDev->dwDevFunc, SST_AGP_CMD_ID);
      dwData |= 0x100;
      PCI_Write_Config(pDev->dwBus, pDev->dwDevFunc, SST_AGP_CMD_ID, dwData);
      }
}


int DPMSSupport(PCRS pCRS, PDEVTABLE pDevTable);
int DDCSupport(PDEVTABLE pDevTable, PCRS pCRS);
int nInDDC = FALSE;


/*----------------------------------------------------------------------
Function name:  VESASupport

Description:    Called before passing VESA command to BIOS.

Information:    
  Entry..
  ebp   -   Client Register pointer
  ebx   -   VM which we use to ignore
  Exit..
  CY set -  Handled.. no need to call BIOS or TSR
  CY clear - Normal VESA processing

Return:         VOID
----------------------------------------------------------------------*/
_declspec ( naked ) VOID VESASupport( VOID )
{
    PCRS pCR;           // client registers
    PDEVTABLE pDevTable;
    int passToBIOS;
    int failCall;
    int nSkip;

    // naked prolog code
    __asm
    {
        push    ebp;
        mov     ebp,esp;
        sub     esp,__LOCAL_SIZE;
        push    esi;
        push    edi;
        mov     eax,[ebp];      // get client ptr
        mov     pCR,eax;
    }

    passToBIOS = 0;
    failCall = 0;
    nSkip = 1;    

    // route VESA call to correct handler

    switch( pCR->Client_EAX & 0xffff )
    {
      case 0x4f00:
          passToBIOS = 1;
          break;

      case 0x4F10:
         pDevTable = FindActiveBanshee();
         if (NULL == pDevTable)
            passToBIOS = 1;
         else
            {
            nSkip = 0;
            passToBIOS = 0;
            DPMSSupport(pCR, pDevTable);
            }
         break;

      case 0x4F15:
#if 1
#ifdef SLI_AA
         pDevTable = FindActiveBansheewithCount(pCR->Client_EBX & 0xFF);
#else
         pDevTable = FindActiveBanshee();
#endif
         if (NULL == pDevTable)
            passToBIOS = 1;
         else
            {
            passToBIOS = !DDCSupport(pDevTable, pCR);
            nSkip = 0;
            }
#else
         nInDDC = TRUE;
         if (1 == pVGADevTable->GdiDesktopEnabled)
            { 
              if (VMM_Remove_IO_Handler(pVGADevTable->IoBase + 0x78))
                    {
                         Debug_Printf( VNAME "port untrap failed for port %d\n", 0x78);
                    }
            } 
              passToBIOS = 1;
#endif
         break;

      default:
          if (pVGADevTable->GdiDesktopEnabled)
              failCall = 1;
          else
              passToBIOS = 1;
          break;
    }

    if (passToBIOS)
    {
        __asm
        {
            pop     edi;
            pop     esi;
            clc;
            mov     esp,ebp;
            pop     ebp;
            ret;
        }
    }
    else
    {
      if (nSkip)
         {
           // handle the call here, returning either sucess or failure
              pCR->Client_EAX &= ~0xffff;
        
         if (failCall)
                  pCR->Client_EAX |= 0x034f;    // func. call invalid in curr. mode
              else
                 pCR->Client_EAX |= 0x004f;     // call succeeded
         }
      }

        __asm
        {
            pop     edi;
            pop     esi;
            stc;
            mov     esp,ebp;
            pop     ebp;
            ret;
        }
}


/*----------------------------------------------------------------------
Function name:  VESAPostSupport

Description:    Called after passing VESA command to BIOS.

Information:    
  Entry..
  ebp   -   Client Register pointer
  ebx   -   VM which we use to ignore

Return:         VOID
----------------------------------------------------------------------*/
_declspec ( naked ) VOID VESAPostSupport( VOID )
{
    PCRS pCR;           // client registers

    // naked prolog code
    __asm
    {
        push    ebp;
        mov     ebp,esp;
        sub     esp,__LOCAL_SIZE;
        push    esi;
        push    edi;
        mov     eax,[ebp];      // get client ptr
        mov     pCR,eax;
    }

   if (TRUE == nInDDC)
      {
      nInDDC = FALSE;
      if (1 == pVGADevTable->GdiDesktopEnabled)
         { 
              if (VMM_Install_IO_Handler((DWORD)myIOhandler, pVGADevTable->IoBase + 0x78))
            {
                      Debug_Printf( VNAME "port untrap failed for port %d\n", 0x78);
                 }
         else
                 VMM_Enable_Global_Trapping(pVGADevTable->IoBase + 0x78);
         }
      }
        __asm
        {
            pop     edi;
            pop     esi;
            mov     esp,ebp;
            pop     ebp;
            ret;
        }
}


/*----------------------------------------------------------------------
Function name:  PostVGAToHiRes

Description:    Called after changing to hires mode

Information:    
  Entry..
  ebp   -   Client Register pointer
  ebx   -   VM which we ignore

Return:         VOID
----------------------------------------------------------------------*/
VOID 
PostVGAToHiRes( VOID )
{
}


/*----------------------------------------------------------------------
Function name:  TurnVGAOff

Description:    Disable the VGA registers and memory aperture.

Information:    

Return:         BOOL    TRUE is always returned.
----------------------------------------------------------------------*/
BOOL TurnVGAOff( VOID )
{
   WORD wCmd;

   if (NULL != pVGADevTable)
      {
      // Disable Interrupts
      if ((pVGADevTable->InterruptsEnabled) && (pVGADevTable->hIRQ))
         {
         DisableInterrupts(pVGADevTable);
         VPICD_Force_Default_Behavior(pVGADevTable->hIRQ);
         pVGADevTable->hIRQ=0x0;
         }

      wCmd = 0x0;
   	CM_Call_Enumerator_Function( pVGADevTable->dwDevNode,
	   			     PCI_ENUM_FUNC_SET_DEVICE_INFO,
		   		     SST_PCI_COMMAND_ID, &wCmd, sizeof(WORD), 0 );
      }
    return TRUE;
}


/*----------------------------------------------------------------------
Function name:  TurnVGAOn

Description:    Enable the VGA registers and memory aperture.

Information:    

Return:         BOOL    TRUE is always returned.
----------------------------------------------------------------------*/
BOOL TurnVGAOn( VOID )
{
   WORD wCmd;

   if (NULL != pVGADevTable)
      {
   	CM_Call_Enumerator_Function( pVGADevTable->dwDevNode,
	   			     PCI_ENUM_FUNC_GET_DEVICE_INFO,
		   		     SST_PCI_ACPI_CTL, &wCmd, sizeof(WORD), 0x0);

      // Sometimes this function is called when
      // we are in a low power mode
      // in this case don't enable us
      // cause that will hang the machine
      if (0x03 != wCmd)
         {
         wCmd = 0x03;
      	CM_Call_Enumerator_Function( pVGADevTable->dwDevNode,
	      			     PCI_ENUM_FUNC_SET_DEVICE_INFO,
		      		     SST_PCI_COMMAND_ID, &wCmd, sizeof(WORD), 0x0);

         //Reenable Interrupts
	      if (pVGADevTable->InterruptsEnabled &&
	         (pVGADevTable->ConfigData.bIRQRegisters[0] != 0))
            pVGADevTable->InterruptsEnabled = InitializeInterrupts(pVGADevTable->ConfigData.bIRQRegisters[0], pVGADevTable);
         }

      }
    return TRUE;
}

#ifdef SLI_AA
/*----------------------------------------------------------------------
Function name:  GetNumUnits

Description: Returns the number of units that we want to report

Information: This function will never return more then 2.  Therefore
units 3 and 4 need to be inited via here

Return:         VOID
----------------------------------------------------------------------*/
_declspec ( naked ) DWORD GetNumUnits(DWORD dwDevNode)
{
   DWORD dwReturn;
   DWORD i;
#ifndef WIN_CSIM
   DWORD dwBus;
   DWORD dwDevFunc;
   DWORD dwVendorDeviceID;
   BYTE bHeader;
#endif
   
    // naked prolog code
    __asm
    {
        push    ebp;
        mov     ebp,esp;
        sub     esp,__LOCAL_SIZE;
        push    esi;
        push    edi;
        pushad
    }

   Debug_Printf("DevNode %x\n", dwDevNode);

#ifdef WIN_CSIM
   dwReturn = 1;
#else

   // Get PCI Header Type
   CM_Call_Enumerator_Function(dwDevNode,
                               PCI_ENUM_FUNC_GET_DEVICE_INFO,
                               SST_PCI_HEADER_TYPE_ID, &bHeader, 
                               sizeof(BYTE), 0 );

   // If this is a Multifunction Device then we will allow PCI
   // space to tell all about the number of devices
   dwReturn = 1;
   if ((bHeader & 0x80) != 0x80)
      {
      PCIGetBusDevFunc(dwDevNode, &dwBus, &dwDevFunc);

      // How we need to walk the PCI Functions to see if there are any others
      for (i=1; i<8; i++)
         {
         // It appears that the ulDevFunc is stored right shifted by 8 as to how it is written on the bus
         dwDevFunc &= ~0x07;
         dwDevFunc |= i;
         dwVendorDeviceID = PCI_Read_Config(dwBus, dwDevFunc, 0x0);
         if (IS_VOODOO3(dwVendorDeviceID) || IS_NAPALM(dwVendorDeviceID))
            dwReturn++;
         }
      }

#endif      // WIN_CSIM

    __asm
    {
        popad
        mov     eax,dwReturn;
        pop     edi;
        pop     esi;
        stc;
        mov     esp,ebp;
        pop     ebp;
        ret;
    }
}
#endif

/*----------------------------------------------------------------------
Function name:  GetTotalVRAMSize

Description:    

Information:    

Return:         VOID
----------------------------------------------------------------------*/
_declspec ( naked ) VOID
GetTotalVRAMSize( VOID )
{
    DWORD dwSize;

    // naked prolog code
    __asm
    {
        push    ebp;
        mov     ebp,esp;
        sub     esp,__LOCAL_SIZE;
        push    esi;
        push    edi;
    }

    if (NULL != pVGADevTable)
      dwSize = pVGADevTable->MemSizeInMB * 1024L * 1024L;
    else
      dwSize = 0x0;

    __asm
    {
        mov     ecx,dwSize;
        pop     edi;
        pop     esi;
        stc;
        mov     esp,ebp;
        pop     ebp;
        ret;
    }
}

#pragma VxD_LOCKED_CODE_SEG
#pragma VxD_LOCKED_DATA_SEG

/****************************************************************************
 Message code etc.
 ***************************************************************************/


/*----------------------------------------------------------------------
Function name:  PreHiResToVGA

Description:    Called before changing to VGA mode.

Information:    
  Entry..
  ebp   -   Client Register pointer
  ebx   -   VM going to full screen VGA mode

Return:         VOID
----------------------------------------------------------------------*/

VOID 
PreHiResToVGA( VOID )
{
    PDEVTABLE pDev = pVGADevTable;
    SstCRegs *lpCRegs;
    FxU8 bRegValue;

    if (NULL == pDev)
        return;
    
    __asm mov FullScreenVM, ebx;


    // @RVB-TV, we're about to go full-screen.  The problem is,
    // the user could've changed the TV connector type or signal
    // type on us. We need to make sure the BIOS has the latest
    // information regarding how it needs to initialize the full
    // screen DOS box.  This addresses PRS 6651.
    if (IS_VOODOO3(pDev->dwVendorDeviceID) && (pDev->tvOutActive))
    {
	  READSCRATCHREGISTER2 (pDev, bRegValue);
      bRegValue &= ~(0x40 | 0x20 | BIOS_TVSTD_MASK);

      // tvboot
      bRegValue |= 0x20;   // tvboot

      // tvstd
      bRegValue |= ((TVOUT_CURSETUP *)pDev->tvOutData)->tvStd;

      // tvcvbs
      bRegValue |= ((((TVOUT_CURSETUP *)pDev->tvOutData)->cvbsOut) ? 0x40 : 0);

      outp ((FxU16)(pDev->IoBase + 0xd5), bRegValue);
    }

	//V3TV If video port is active then disable it
	if (pDev->dwIMask & H3_VMI_INT_ENABLE) {
   	   DWORD dwReg;
 	   dwReg = (((SstIORegs *)(pDev->RegBase[HWINFO_SST_IOREGS_INDEX]))->vidSerialParallelPort) & ~(H3_VMI_ENABLE);
  	   (((SstIORegs *)(pDev->RegBase[HWINFO_SST_IOREGS_INDEX]))->vidSerialParallelPort) = dwReg;

	   //disable VMI interrupts if enabled previously
		dwReg = (((SstRegs *)(pDev->RegBase[HWINFO_SST_3DREGS_INDEX]))->intrCtrl & 0x7FFFFFFF);
		dwReg &= ~H3_VMI_INT_ENABLE;
		((SstRegs *)(pDev->RegBase [HWINFO_SST_3DREGS_INDEX]))->intrCtrl = dwReg;
	}

    DisableGdiDesktop(pDev);

    // wait for chip idle and turn off the command fifo
    while (IGET32(status) & SST_BUSY)
        ;

    lpCRegs = (SstCRegs *) ((FxU32)pDev->RegBase[HWINFO_SST_CMDFIFOREGS_INDEX]);
    
    SETDW(lpCRegs->PRIMARY_CMDFIFO.baseSize, 0);
}


/*----------------------------------------------------------------------
Function name:  PostHiResToVGA

Description:    

Information:    Function is empty.

Return:         VOID
----------------------------------------------------------------------*/
VOID 
PostHiResToVGA( VOID )
{
//      if (pVGADevTable->tvOutActive)
//          TvOutFn->tvout_Enable (pVGADevTable, 3);
}


/*----------------------------------------------------------------------
Function name:  PreVGAToHires

Description:    

Information:    
  Entry..
  ebp   -   Client Register pointer
  ebx   -   VM leaving full screen VGA mode

Return:         VOID
----------------------------------------------------------------------*/
VOID 
PreVGAToHires( VOID )
{
    FullScreenVM = hSysVM;

	//V3TV If video port was active then re-enable it
	if (pVGADevTable->dwIMask & H3_VMI_INT_ENABLE) {
       DWORD dwReg;
	   //enable video port
 	   dwReg = (((SstIORegs *)(pVGADevTable->RegBase[HWINFO_SST_IOREGS_INDEX]))->vidSerialParallelPort) | H3_VMI_ENABLE;
  	   (((SstIORegs *)(pVGADevTable->RegBase[HWINFO_SST_IOREGS_INDEX]))->vidSerialParallelPort) = dwReg;

	   //enable VMI interrupts if enabled previously
		dwReg = (((SstRegs *)(pVGADevTable->RegBase[HWINFO_SST_3DREGS_INDEX]))->intrCtrl & 0x7FFFFFFF);
		dwReg |= H3_VMI_INT_ENABLE;
		((SstRegs *)(pVGADevTable->RegBase[HWINFO_SST_3DREGS_INDEX]))->intrCtrl = dwReg;
	}
}


/*----------------------------------------------------------------------
Function name:  MiniVDD_Sys_VM_Terminate

Description:    

Information:    

Return:         DWORD   VXD_SUCCESS is always returned.
----------------------------------------------------------------------*/
DWORD __stdcall 
MiniVDD_Sys_VM_Terminate( VOID )
{
    if (NULL != pVGADevTable)
    {
        if( pVGADevTable->RegBase[HWINFO_SST_IOREGS_INDEX] != NULL )      // just in case
        {
            PreHiResToVGA();
            // int 10 should do the rest
        }
    }
    return(VXD_SUCCESS);
}


/*----------------------------------------------------------------------
Function name:  MiniVDD_System_Exit

Description:    

Information:    Function is empty.

Return:         DWORD   VXD_SUCCESS is always returned.
----------------------------------------------------------------------*/
DWORD __stdcall
MiniVDD_System_Exit( VOID )
{
	PDEVTABLE pDev = pVGADevTable;
    FxU8 ucTemp;

	/*
	Unfortunately, when we get here the driver has already done an INT10 BUT
	later disabled the desktop (turning off TVOUT).  This is the reverse of
	the order when opening a full screen DOS session.  We need to reenable
	TVOUT if it was active.
	*/

	if (pDev && TvOutFn && pDev->RegBase[HWINFO_SST_IOREGS_INDEX] && pDev->IoBase)
	{
        READSCRATCHREGISTER2 (pDev, ucTemp);
		if ((IS_VOODOO3(pDev->dwVendorDeviceID) && (ucTemp & BIOS_TVOUT_ACTIVE)) ||
		    (IS_NAPALM(pDev->dwVendorDeviceID) && (ucTemp & BIOS_CRx1E_TVACTIVE)))

		{
			((SstIORegs * )pDev->RegBase[HWINFO_SST_IOREGS_INDEX])->vidInFormat = \
											SST_VIDEOIN_TVOUT_ENABLE       |
											SST_VIDEOIN_G4_FOR_POSEDGE     |
											SST_VIDEOIN_VSYNC_POLARITY_LOW |
											SST_VIDEOIN_HSYNC_POLARITY_LOW;
		}
	}
    return(VXD_SUCCESS);
}


/*----------------------------------------------------------------------
Function name:  DisplayDriverDisabling

Description:    

Information:    

Return:         VOID
----------------------------------------------------------------------*/
VOID
DisplayDriverDisabling( VOID )
{
    MiniVDD_Sys_VM_Terminate();
}

/****************************************************************************
 Port Trapping.
 ***************************************************************************/


/*----------------------------------------------------------------------
Function name:  VDD_Get_VM_Info

Description:    

Information:    

Return:         HVM
----------------------------------------------------------------------*/
HVM VXDINLINE
VDD_Get_VM_Info()
{
    HVM hVM;

    VxDCall(VDD_Get_VM_Info);
    __asm mov hVM,edi;
    return(hVM);
}


/*----------------------------------------------------------------------
Function name:  MiniVDD_Virtual3C2H

Description:    Called for port access to 3C2.

Information:    
  Entry..
  eax   -   Value to write
  ebx   -   VM
  ecx   -   Type of I/O
  edx   -   Port

Return:         HVM
----------------------------------------------------------------------*/
_declspec ( naked ) VOID
Virtual_3c2h()
{
    WORD wPort;
    WORD wValue;
    HVM  hVM;
    DWORD dwFlags;
    static BOOL bTrig;
    static BYTE dbExt7;
    DWORD dumpAccess;

    _NP();
    __asm 
    {
        mov     wPort, dx;
        mov     wValue, ax;
        mov     dwFlags, ecx;
        mov     hVM, ebx;
    }

    dumpAccess = 0;

    if (pVGADevTable->GdiDesktopEnabled && (hVM == hSysVM))
    {
        dumpAccess = 1;
    }

    if (dumpAccess)
    {
        _NE();
    }
    else
    {
        _NE_NORET();
        __asm jmp       Old_3c2_Handler;
    }
}

#ifdef IDONTTHINKWENEEDTHIS_KMW


/*----------------------------------------------------------------------
Function name:  MiniVDD_Virtual3DEH

Description:    Called for port access to 3DE/3DF.

Information:    Presently #ifdef'd out.
  Entry..
  eax   -   Value to write
  ebx   -   VM
  ecx   -   Type of I/O
  edx   -   Port

Return:         DWORD
----------------------------------------------------------------------*/
_declspec ( naked ) DWORD MiniVDD_Virtual3DEH( VOID )
{
    WORD wPort;
    WORD wValue;
    HVM  hVM;
    DWORD dwFlags;
    DWORD dwReply;
    static BOOL bTrig;
    static BYTE dbExt7;

    // naked prolog code
    __asm
    {
        push    ebp;
        mov     ebp,esp;
        sub     esp,__LOCAL_SIZE;
        mov     wPort,dx;
        mov     wValue,ax;
        mov     dwFlags,ecx;
        mov     hVM,ebx;
    }

    // physical if controlling VM??
    if( hVM == hSysVM ||
        hVM == VDD_Get_VM_Info() )
    {
        switch( dwFlags )
        {
        case BYTE_INPUT:
            dwReply = inp(wPort);            
            break;

        case BYTE_OUTPUT:
            outp( wPort, (BYTE)wValue );
            break;

        case WORD_INPUT:
            dwReply = inpw( wPort );
            break;

        case WORD_OUTPUT:
            outpw( wPort, wValue );
            break;

        default:
            break;
        }
    }
    else switch( dwFlags )
    {
    case BYTE_INPUT:
        if( wPort == 0x3DF && bTrig )
            dwReply = dbExt7;
        break;

    case BYTE_OUTPUT:
        if( wPort == 0x3DE)
        {
            if( (wValue & 0xFF) == 7 )
                bTrig = TRUE;
            else
                bTrig = FALSE;
        }
        else
        if( wPort == 0x3DF && bTrig )
            dbExt7 = wValue;
        break;

    case WORD_INPUT:
        if( wPort == 0x3DE && bTrig )
            dwReply = (dbExt7 << 8) | 7;
        break;

    case WORD_OUTPUT:
        if( wPort == 0x3DE && (wValue & 0xFF) == 7 )
        {
            bTrig = TRUE;
            dbExt7 = wValue >> 8;
        }
        else
            bTrig = FALSE;
        break;

    default:
        break;
    }
    // naked epilog code
    __asm
    {
        mov     eax,dwReply;
        mov     esp,ebp;
        pop     ebp;
        ret;
    }
}

#endif // #ifdef IDONTTHINKWENEEDTHIS_KMW

#ifdef SLI_AA
DWORD _cdecl SetMasterMonitorPowerState(DEVNODE devnode, DWORD PowerState);
#define DPMS_ROUTINE SetMasterMonitorPowerState
#else
DWORD _cdecl SetMonitorPowerState(DEVNODE devnode, DWORD PowerState);
#define DPMS_ROUTINE SetMonitorPowerState
#endif
/****************************************************************************
 DPMS support.
 ***************************************************************************/


/*----------------------------------------------------------------------
Function name:  DPMSSupport

Description:    

Information:    

Return:         INT     1 if success, 0 if failure.
----------------------------------------------------------------------*/
int DPMSSupport(PCRS pCRS, PDEVTABLE pDevTable)
{
   int nReturn = 0;
   CLIENT_STRUCT *pCR = (CLIENT_STRUCT *)pCRS;
 
   switch( pCR->CBRS.Client_BL & 0xFF )
      {
      case 0:     // report
         pCR->CWRS.Client_AX = 0x4F;
         pCR->CWRS.Client_BX = 0x0710;   // full support
         nReturn = 1;
         break;

      case 1:     // set state
         pCR->CWRS.Client_AX = 0x4F;
         switch(pCR->CBRS.Client_BH)
            {
            case 0:         // normal
               DPMS_ROUTINE(pDevTable->dwDevNode, 0x01);
               pDevTable->bDPMSState = 0;
               nReturn = 1;
               break;

            case 1:         // standby
               DPMS_ROUTINE(pDevTable->dwDevNode, 0x02);
               pDevTable->bDPMSState = 1;
               nReturn = 1;
               break;

            case 2:         // suspend
               DPMS_ROUTINE(pDevTable->dwDevNode, 0x04);
               pDevTable->bDPMSState = 2;
               nReturn = 1;
               break;


            case 4:         // off
               DPMS_ROUTINE(pDevTable->dwDevNode, 0x08);
               pDevTable->bDPMSState = 4;
               nReturn = 1;
               break;

            default:
               pCR->CWRS.Client_AX = 0x14F;    // failure
               break;
            }
        break;

      case 2:     // get state
         pCR->CWRS.Client_AX = 0x4F;
         pCR->CBRS.Client_BH = pDevTable->bDPMSState;
         nReturn = 1;
         break;

      default:   // bogus state ... give them a error
         pCR->CWRS.Client_AX = 0x14F;
         break;
   }

   return nReturn;
}

/*----------------------------------------------------------------------
Function name:  MiniVDD_W32_DevIoCtl

Description:    
This function handles the DEVIOCTL calls.

Information:    

Return:         INT     1 if success, 0 if failure.
----------------------------------------------------------------------*/

void HotKey(void);
DWORD VKD_Define_Hot_Key(DWORD pFunc, DWORD pData, DWORD keycode, DWORD shiftstate, DWORD flags);
DWORD VKD_Remove_Hot_Key(DWORD Handle);

HOTKEY_DATA_NODE   HotKey_Table[Number_HotKeys] ; 

#ifdef SLI_AA
void EnableSLIAA(PDEVTABLE pMaster, PSLI_AA_REQUEST pRequest);
void DisableSLIAA(PDEVTABLE pMaster, PSLI_AA_REQUEST pRequest);
#endif
DWORD __stdcall MiniVDD_W32_DevIoCtl(DIOCPARAMETERS * pDIOCParms)
{
   PDEVTABLE pDev;
   DWORD dwReturn = 0;
   DWORD dwMMIO;
   DWORD dwOrigMMIO;
   DWORD dwFB;
   DWORD dwOrigFB;
   PDIOC_DATA pDIOC;
#ifdef REAL_NET
   DWORD * pCount;
   SstRegs * p3DRegs;
   DWORD dwReg;
#endif
   WORD wCmd;
   WORD wOrigCmd;

   switch (pDIOCParms->dwIoControlCode)
      {

	  //V3TV communication to WDM driver if available
	  //Send scale coordinates to WDM driver to scale video only/notVBI
      case VDD_V3TV_WDMSCALE:
		dwReturn = VDDtoWDMScale(pDIOCParms);
		break;

	  //V3TV - Query if WDM driver is active before communication with it
      case VDD_V3TV_WDMQUERYACTIVE:
		dwReturn = VDDtoWDMQueryActive(pDIOCParms);
		break;

      case VDD_VMI_FUNCTION:
        dwReturn = VDDVMIFunctions(pDIOCParms);
        break;

      case DIOC_OPEN:
      case DIOC_CLOSEHANDLE:
         dwReturn = 0;
         break;

      case VDD_IOCTL_GET_DDHAL:         
         dwReturn = GetKernelInfo(pDIOCParms);
         break;

      // Update the Interrupt Mask <Used in EnableInterrupt>
      case UPDATE_IMASK:
         if (NULL != (pDIOC = (PDIOC_DATA)pDIOCParms->lpvInBuffer))
            {
            pDev = FindPDEVFromDevNode(pDIOC->dwDevNode);
            if (NULL != pDev)
               pDev->dwIMask = pDIOC->dwSpare & H3_IMASK;
            else
               {
               dwReturn = ERROR_INVALID_PARAMETER;
               }
            }
         else
            {
            dwReturn = ERROR_INVALID_PARAMETER;
            }
         break;         


      // This is the magic script
      case AGP_WARMUP:
         if (NULL != (pDIOC = (PDIOC_DATA)pDIOCParms->lpvInBuffer))
            {
            pDev = FindPDEVFromDevNode(pDIOC->dwDevNode);
            if (NULL != pDev)
               {
               CM_Call_Enumerator_Function( pDev->dwDevNode, PCI_ENUM_FUNC_GET_DEVICE_INFO, SST_PCI_COMMAND_ID, &wCmd, sizeof(WORD), 0 );
               wOrigCmd = wCmd;
               wCmd &= ~(0x03);
               // Disable the Device
               CM_Call_Enumerator_Function( pDev->dwDevNode, PCI_ENUM_FUNC_SET_DEVICE_INFO, SST_PCI_COMMAND_ID, &wCmd, sizeof(WORD), 0 );

               CM_Call_Enumerator_Function( pDev->dwDevNode, PCI_ENUM_FUNC_GET_DEVICE_INFO, SST_PCI_MMIO_ID, &dwMMIO, sizeof(DWORD), 0 );
               CM_Call_Enumerator_Function( pDev->dwDevNode, PCI_ENUM_FUNC_GET_DEVICE_INFO, SST_PCI_FB_ID, &dwFB, sizeof(DWORD), 0 );
        
               // Save Original
               dwOrigMMIO = dwMMIO & 0xFF000000;
               dwOrigFB = dwFB & 0xFF000000;

               // Max Size
               dwMMIO = 0xFFFFFFFF;
               dwFB = 0xFFFFFFFF;
               // Find Size
               CM_Call_Enumerator_Function( pDev->dwDevNode, PCI_ENUM_FUNC_SET_DEVICE_INFO, SST_PCI_MMIO_ID, &dwMMIO, sizeof(DWORD), 0 );
               CM_Call_Enumerator_Function( pDev->dwDevNode, PCI_ENUM_FUNC_GET_DEVICE_INFO, SST_PCI_MMIO_ID, &dwMMIO, sizeof(DWORD), 0 );
               CM_Call_Enumerator_Function( pDev->dwDevNode, PCI_ENUM_FUNC_SET_DEVICE_INFO, SST_PCI_FB_ID, &dwFB, sizeof(DWORD), 0 );
               CM_Call_Enumerator_Function( pDev->dwDevNode, PCI_ENUM_FUNC_GET_DEVICE_INFO, SST_PCI_FB_ID, &dwFB, sizeof(DWORD), 0 );

               // Back to Original        
               CM_Call_Enumerator_Function( pDev->dwDevNode, PCI_ENUM_FUNC_SET_DEVICE_INFO, SST_PCI_MMIO_ID, &dwOrigMMIO, sizeof(DWORD), 0 );
               CM_Call_Enumerator_Function( pDev->dwDevNode, PCI_ENUM_FUNC_SET_DEVICE_INFO, SST_PCI_FB_ID, &dwOrigFB, sizeof(DWORD), 0 );
      
               // Reenable IO             
               CM_Call_Enumerator_Function( pDev->dwDevNode, PCI_ENUM_FUNC_SET_DEVICE_INFO, SST_PCI_COMMAND_ID, &wOrigCmd, sizeof(WORD), 0 );
               }
            else
               {
               dwReturn = ERROR_INVALID_PARAMETER;
               }
            }
         else
            {
            dwReturn = ERROR_INVALID_PARAMETER;
            }
         break;
      case GET_DIB_ADDR:
         dwReturn = GetExtraAddr( pDIOCParms);
         break;

	  //Allocate for KMVTBUFF memory used for kernel mode video transport
	  //functions in video port code both in ddvpe32, ddovl32 and kmtv and h3irq
      case  VIDEO_SYS_BUF:
         dwReturn = AllocalSysBuff( pDIOCParms);
         break;
#ifdef SLI_AA
      case SLI_AA_ENABLE:
         if (NULL != (pDIOC = (PDIOC_DATA)pDIOCParms->lpvInBuffer))
            {
            pDev = FindPDEVFromDevNode(pDIOC->dwDevNode);
            if (NULL != pDev)
               {
               SetAllSlaveBits(pDev);   
               EnableSLIAA(pDev, (PSLI_AA_REQUEST)pDIOC->dwSpare);
               }
            else
               {
               dwReturn = ERROR_INVALID_PARAMETER;
               }
            }
         else
            {
            dwReturn = ERROR_INVALID_PARAMETER;
            }
         break;         

      case SLI_AA_DISABLE:
         if (NULL != (pDIOC = (PDIOC_DATA)pDIOCParms->lpvInBuffer))
            {
            pDev = FindPDEVFromDevNode(pDIOC->dwDevNode);
            if (NULL != pDev)
               {
               ClearAllSlaveBits(pDev);   
               DisableSLIAA(pDev, (PSLI_AA_REQUEST)pDIOC->dwSpare);
               }
            else
               {
               dwReturn = ERROR_INVALID_PARAMETER;
               }
            }
         else
            {
            dwReturn = ERROR_INVALID_PARAMETER;
            }
         break;         
#ifdef RD_ABORT_ERROR
      case ENABLE_SLI_READ:
         if (NULL != (pDIOC = (PDIOC_DATA)pDIOCParms->lpvInBuffer))
            {
            pDev = FindPDEVFromDevNode(pDIOC->dwDevNode);
            if (NULL != pDev)
               {
               SLI_Read_Enable(pDev);
               }
            else
               {
               dwReturn = ERROR_INVALID_PARAMETER;
               }
            }
         else
            {
            dwReturn = ERROR_INVALID_PARAMETER;
            }
         break;         

      case DISABLE_SLI_READ:
         if (NULL != (pDIOC = (PDIOC_DATA)pDIOCParms->lpvInBuffer))
            {
            pDev = FindPDEVFromDevNode(pDIOC->dwDevNode);
            if (NULL != pDev)
               {
               SLI_Read_Disable(pDev);
               }
            else
               {
               dwReturn = ERROR_INVALID_PARAMETER;
               }
            }
         else
            {
            dwReturn = ERROR_INVALID_PARAMETER;
            }
         break;         

#endif
#endif
      case HOT_KEY_FLAG:
         if (NULL != (pDIOC = (PDIOC_DATA)pDIOCParms->lpvInBuffer))
            {
            // New Scan Code ?
            if ((HotKey_Table[pDIOC->dwOffset].dwHotKeyScan != pDIOC->dwSpare) || (HotKey_Table[pDIOC->dwOffset].dwHotKeyModifier != pDIOC->dwModifier)) 
               {
               // Save Old Code
               HotKey_Table[pDIOC->dwOffset].dwHotKeyScan = pDIOC->dwSpare;
               HotKey_Table[pDIOC->dwOffset].dwHotKeyModifier = pDIOC->dwModifier;
               // Remove Old Handle
               if (HotKey_Table[pDIOC->dwOffset].dwHotKeyHandle)
                  {
                  VKD_Remove_Hot_Key(HotKey_Table[pDIOC->dwOffset].dwHotKeyHandle);
                  HotKey_Table[pDIOC->dwOffset].dwHotKeyHandle = 0x0;
                  }
               // Get New Handle
               HotKey_Table[pDIOC->dwOffset].dwHotKeyHandle = VKD_Define_Hot_Key((DWORD)HotKey, (DWORD)(&HotKey_Table[pDIOC->dwOffset].HotKeyData), 0xFF00 | (pDIOC->dwSpare & 0xFF), pDIOC->dwModifier, 0x08); 
               }
            pDIOC->dwSpare = (DWORD)&HotKey_Table[pDIOC->dwOffset].HotKeyData;
            }
         else
            {
            dwReturn = ERROR_INVALID_PARAMETER;
            }
         break;         

      case HOT_KEY_OFF_FLAG:
         if (NULL != (pDIOC = (PDIOC_DATA)pDIOCParms->lpvInBuffer))
            {
            HotKey_Table[pDIOC->dwOffset].dwHotKeyScan = 0xFFFFFFFF;
            HotKey_Table[pDIOC->dwOffset].dwHotKeyModifier = 0xFFFFFFFF;
            if (HotKey_Table[pDIOC->dwOffset].dwHotKeyHandle)
			{
               VKD_Remove_Hot_Key(HotKey_Table[pDIOC->dwOffset].dwHotKeyHandle);
               HotKey_Table[pDIOC->dwOffset].dwHotKeyHandle = 0x0;
			}
		 }
         break;         

#ifdef REAL_NET
      case CLEAR_VCOUNT:
         if (NULL != (pDIOC = (PDIOC_DATA)pDIOCParms->lpvInBuffer))
            {
            pDev = FindPDEVFromDevNode(pDIOC->dwDevNode);
            if (NULL != pDev)
               {
               pDev->dwVCount = 0;
               }
            else
               {
               dwReturn = ERROR_INVALID_PARAMETER;
               }
            }
         else
            {
            dwReturn = ERROR_INVALID_PARAMETER;
            }
         break;         

      case DISABLE_VCOUNT:
         if (NULL != (pDIOC = (PDIOC_DATA)pDIOCParms->lpvInBuffer))
            {
            pDev = FindPDEVFromDevNode(pDIOC->dwDevNode);
            if (NULL != pDev)
               {
               pDev->dwIMask &= ~H3_VSYNC_INT_ENABLE;
               p3DRegs = (SstRegs *)pDev->RegBase[HWINFO_SST_3DREGS_INDEX];
               dwReg = p3DRegs->intrCtrl;
               dwReg &= ~H3_VSYNC_INT_ENABLE;
               p3DRegs->intrCtrl = dwReg;
               }
            else
               {
               dwReturn = ERROR_INVALID_PARAMETER;
               }
            }
         else
            {
            dwReturn = ERROR_INVALID_PARAMETER;
            }
         break;         

      case ENABLE_VCOUNT:
         if (NULL != (pDIOC = (PDIOC_DATA)pDIOCParms->lpvInBuffer))
            {
            pDev = FindPDEVFromDevNode(pDIOC->dwDevNode);
            if (NULL != pDev)
               {
               pDev->dwIMask |= H3_VSYNC_INT_ENABLE;
               p3DRegs = (SstRegs *)pDev->RegBase[HWINFO_SST_3DREGS_INDEX];
               dwReg = p3DRegs->intrCtrl;
               dwReg |= H3_VSYNC_INT_ENABLE;
               p3DRegs->intrCtrl = dwReg;
               }
            else
               {
               dwReturn = ERROR_INVALID_PARAMETER;
               }
            }
         else
            {
            dwReturn = ERROR_INVALID_PARAMETER;
            }
         break;         

      case RETURN_VCOUNT:          
         if (NULL != (pDIOC = (PDIOC_DATA)pDIOCParms->lpvInBuffer))
            {
            pDev = FindPDEVFromDevNode(pDIOC->dwDevNode);
            if ((NULL != pDev) && (NULL != (pCount = (DWORD *)pDIOCParms->lpvOutBuffer)) && (pDIOCParms->cbOutBuffer >= 4))
               {
               *pCount = pDev->dwVCount;
               }
            else
               {
               dwReturn = ERROR_INVALID_PARAMETER;
               }
            }
         else
            {
            dwReturn = ERROR_INVALID_PARAMETER;
            }
         break;         
#endif

      default:
         // use WIN32 GetLastError to find out about this
         dwReturn = ERROR_NOT_SUPPORTED;
         break;
      }

   return dwReturn;
}

/*----------------------------------------------------------------------
Function name:  FindPCIDevice

Description:    Search for a PCI device based on the Vendor and Device ID's.

Information:    

Return:         BOOL FALSE - Device NOT Found.
                     TRUE  - Device Found.

                pDevNode   - devNode of located device.

----------------------------------------------------------------------*/

/******************************************************************************
*   Type Definitions and Structures
******************************************************************************/
typedef struct tagPCICommand            /* PCI command register structure    */
{
    WORD    bIOSpace : 1;               /* I/O space access control bit      */
    WORD    bMemSpace : 1;              /* Memory space access control bit   */
    WORD    bBusMaster : 1;             /* Bus master control bit            */
    WORD    bSpecialCycle : 1;          /* Special cycle control bit         */
    WORD    bInvalidate : 1;            /* Memory write invalidate ctrl. bit */
    WORD    bPaletteSnoop : 1;          /* VGA palette snoop control bit     */
    WORD    bParity : 1;                /* Parity error control bit          */
    WORD    bWait : 1;                  /* Wait cycle control bit            */
    WORD    bSerr : 1;                  /* SERR control bit                  */
    WORD    bFast : 1;                  /* Fast back-to-back control bit     */
    WORD    : 6;                        /* Reserved                          */
} PCICommand;

typedef struct tagPCIStatus             /* PCI status register structure     */
{
    WORD    : 5;                        /* Reserved                          */
    WORD    b66MHz : 1;                 /* 66 MHz capable flag               */
    WORD    bUDF  : 1;                  /* UDF supported flag                */
    WORD    bFast : 1;                  /* Fast back-to-back capable flag    */
    WORD    bParity : 1;                /* Data parity error flag            */
    WORD    bDevSel : 2;                /* Device selection timing value     */
    WORD    bSigTarget : 1;             /* Signaled target abort flag        */
    WORD    bRecTarget : 1;             /* Received target abort flag        */
    WORD    bRecMaster : 1;             /* Received master abort flag        */
    WORD    bSigSystem : 1;             /* Signaled system error flag        */
    WORD    bDetParity : 1;             /* Detected parity error flag        */
} PCIStatus;

#define DEVSEL_FAST     0               /* Fast DEVSEL timing value          */
#define DEVSEL_MEDIUM   1               /* Medium DEVSEL timing value        */
#define DEVSEL_SLOW     2               /* Slow DEVSEL timing value          */

typedef struct tagPCIBIST               /* PCI BIST structure                */
{
    BYTE    bCompletion : 4;            /* PCI BIST completion code          */
    BYTE    : 2;                        /* Reserved                          */
    BYTE    bStartBIST : 1;             /* Start BIST flag                   */
    BYTE    bBISTCapable : 1;           /* BIST capable flag                 */
} PCIBIST;

#define PCI_NO_INTERRUPT    0xFF        /* No device interrupt value         */
#define INT_NOT_ASSIGNED    0x00        /* PCI interrupt not assigned        */

typedef struct tagPCIMemBase            /* PCI memory base address structure */
{
    DWORD   : 1;                        /* Memory space indicator (0)        */
    DWORD   bMemType: 2;                /* Memory address type value         */
    DWORD   bPrefetchable : 1;          /* Prefetchable flag                 */
    DWORD   bMemAddress : 28;           /* Memory address value              */
} PCIMemBase;
#define PCI_MEM_ANY_32      0           /* Memory anywhere in 32-bit type    */
#define PCI_MEM_BELOW_1M    1           /* Memory below 1Mb type             */
#define PCI_MEM_ANY_64      2           /* Memory anywhere in 64-bit type    */

typedef struct tagPCIIOBase             /* PCI I/O base address structure    */
{
    DWORD   : 1;                        /* I/O space indicator (1)           */
    DWORD   : 1;                        /* Reserved                          */
    DWORD   bIOAddress : 30;            /* I/O address value                 */
} PCIIOBase;

typedef struct tagPCIBaseAddress        /* Generic PCI base address structure*/
{
    DWORD bType : 1;                    /* Base address type value (Mem/IO)  */
    DWORD bBaseAddress: 31;             /* Base address value                */
} PCIBaseAddress;
#define PCI_MEM_ADDRESS     0           /* Memory address type value         */
#define PCI_IO_ADDRESS      1           /* I/O address type value            */
#define PCI_NO_ADDRESS      0x00000000  /* NO PCI base address value         */

typedef struct tagPCIExpansionROM       /* PCI expansion ROM structure       */
{
    DWORD   bROMDecode : 1;             /* ROM decode enable flag            */
    DWORD   : 10;                       /* Reserved                          */
    DWORD   bROMAddress : 21;           /* ROM address (Upper 21 bits)       */
} PCIExpansionROM;
#define PCI_NO_ROM          0x00000000  /* No PCI expansion ROM value        */

typedef struct tagPCIHeader             /* PCI header structure              */
{
    WORD            wVendorID;          /* PCI vendor ID value               */
    WORD            wDeviceID;          /* PCI device ID value               */
    PCICommand      wCommand;           /* PCI command value                 */
    PCIStatus       wStatus;            /* PCI status value                  */
    BYTE            bRevision;          /* PCI revision ID                   */
    BYTE            bInterface;         /* PCI device register interface code*/
    BYTE            bSubClass;          /* PCI device sub-class code         */
    BYTE            bClass;             /* PCI device class code             */
    BYTE            bCacheSize;         /* PCI cache line size (DWords)      */
    BYTE            bLatency;           /* PCI latency value (Bus clocks)    */
    BYTE            bType;              /* PCI header type value             */
    PCIBIST         bBIST;              /* PCI BIST value                    */
    PCIBaseAddress  dwBaseAddress[6];   /* PCI base address values           */
    DWORD           dwCISPointer;       /* PCI cardbus CIS pointer           */
    WORD            wVendorSubID;       /* PCI vendor sub-system ID value    */
    WORD            wSubsystemID;       /* PCI sub-system ID value           */
    PCIExpansionROM dwROMAddress;       /* PCI expansion ROM base address    */
    DWORD           dwReserved[2];      /* PCI reserved header space         */
    BYTE            bIntLine;           /* PCI interrupt line value          */
    BYTE            bIntPin;            /* PCI interrupt pin value           */
    BYTE            bMinGnt;            /* PCI minimum grant value           */
    BYTE            bMaxLat;            /* PCI maximum latency value         */
} PCIHeader;

typedef struct tagPCIConfig             /* PCI configuration space structure */
{
        PCIHeader       Header;         /* PCI header structure              */
        DWORD           dwConfig[48];   /* PCI device configuration data     */
} PCIConfig;

#define PCI_GET_DEVICE_INFO     0x00
#define PCI_SET_DEVICE_INFO     0x01

BOOL FindPCIDevice(WORD wVendorID, WORD wDeviceID, PDEVNODE pDevNode)
{
    BOOL l_bResult = FALSE;             /* Find device result code            */
    DEVNODE l_RootNode, l_DevNode;      /* Root and current DevNode values    */
    DEVNODE l_ChildNode, l_ParentNode;  /* Child and parent DevNode values    */
    DEVNODE l_SiblingNode;              /* Sibling DevNode value              */
    CMBUSTYPE l_cmBusType;              /* Configuration manager bus type     */
    DWORD l_InfoSize;                   /* Information buffer size value      */
    PCIConfig l_pciConfig;              /* PCI configuration header structure */

    /* Try to locate the root DevNode */

    if (CM_Locate_DevNode(&l_RootNode, NULL, 0) == CR_SUCCESS)
    {
        /* Setup to search the hardware DevNode tree */

        l_DevNode = l_RootNode;

        /* Search the hardware DevNode tree for PCI devices */

        while (l_DevNode != NULL)
        {
            /* Try to get the bus information for current DevNode */

            l_InfoSize = 0;
            if (CM_Get_Bus_Info(l_DevNode, &l_cmBusType, &l_InfoSize, NULL, 0) == CR_SUCCESS)
            {
                /* Check for a PCI bus type value */

                if (l_cmBusType == BusType_PCI)
                {
                    /* Call enumerator to read PCI configuration header */

                    if (CM_Call_Enumerator_Function(l_DevNode, PCI_GET_DEVICE_INFO, 0, &l_pciConfig, sizeof(PCIConfig), 0) == CR_SUCCESS)
                    {
                        /* Check for the requested device */

                        if ((l_pciConfig.Header.wVendorID == wVendorID) &&
                            (l_pciConfig.Header.wDeviceID == wDeviceID))
                        {
                            /* Save the device DevNode handle */

                            *pDevNode = l_DevNode;
                            l_bResult = TRUE;

                            /* Force the search loop to terminate */

                            l_DevNode = NULL;
                            break;
                        }
                    }
                }
            }
            /* Check for child DevNodes present */

            if (CM_Get_Child(&l_ChildNode, l_DevNode, 0) == CR_SUCCESS)
            {
                /* Setup current DevNode to next child DevNode */

                l_DevNode = l_ChildNode;
            }
            else    /* No child DevNodes */
            {
                /* Check for sibling DevNodes present */

                if (CM_Get_Sibling(&l_SiblingNode, l_DevNode, 0) == CR_SUCCESS)
                {
                    /* Setup current DevNode to next sibling DevNode */

                    l_DevNode = l_SiblingNode;
                }
                else    /* No sibling DevNodes */
                {
                    /* Search for more parent DevNodes */

                    while (l_DevNode != NULL)
                    {
                        /* Get the parent DevNode */

                        if (CM_Get_Parent(&l_ParentNode, l_DevNode, 0) == CR_SUCCESS)
                        {
                            /* Check for more siblings of the parent DevNode */

                            if (CM_Get_Sibling(&l_SiblingNode, l_ParentNode, 0) == CR_SUCCESS)
                            {
                                /* Setup current DevNode to parent sibling DevNode */

                                l_DevNode = l_SiblingNode;

                                /* Exit the parent search loop */

                                break;
                            }
                            else    /* No parent sibling DevNodes */
                            {
                                /* Set current DevNode to parent and keep searching */

                                l_DevNode = l_ParentNode;
                            }
                        }
                        else    /* No more parent DevNodes */
                        {
                            /* Force the DevNode search to terminate */

                            l_DevNode = NULL;

                            /* Break out of the parent search loop */

                            break;
                        }
                    }
                }
            }
        }
    }
    /* Return result of find to the caller */

    return(l_bResult);
}


/*	Sets the HsyncStart register enabling the position of the
	image on the monitor to be moved vertically. 
	jmccartney 3dfx Belfast added all functions below
	
	The increment the image is to be moved by is supplied by
	h_inc */

void setCRTC_HSyncStart(PDEVTABLE pDev, char h_inc)
{
	unsigned int	hsync_start, cr4, cr1A;

	hsync_start = cr1A = cr4 = 0;

	// Allow the code to write to the CRTC registers
	UnlockVgaTimingRegisters();

	/* 	read in the value in CRTC Register 1A to the var cr1A
		register 0x1A is the Horizontal Extension register
		it contains the 8th bit of the Start Horizontal Sync register (0x04)  
		which the code modifies to change the horizontal position of the image
	*/
	outp((FxU16)(pDev->IoBase + 0xd4), 0x1A);
	cr1A = (inp((FxU16)(pDev->IoBase + 0xd5)));

	// Write out 4 to the CRTC Index Register
	outp((FxU16)(pDev->IoBase + 0xd4), 0x4);
	// read in the value of the Horizontal Sync Start register CRTC 0x04
	cr4 = (inp((FxU16)(pDev->IoBase + 0xd5)));

	// and the bits of the Horizontal Extension register into hsync_start
	hsync_start = cr4 & 0x00FF;
	/* 	OR the 8th bit of the Horizontal Sync Start register onto
		hsync_start so that we can add the increment.  The 8th bit is the 6th bit of 
		the CRTC register 0x1B
	*/
	hsync_start |= (cr1A & 0x0040) ? 0x100 : 0x0;

	// mask out the bit in CRTC register 0x1A that corresponds to Horizontal sync Start
	cr1A &= ~0x0040;

	// add on the increment value passed in h_inc
	hsync_start += h_inc;

	// put the first 8 bits of the Horizontal Sync Start register into cr4
	cr4 = hsync_start & 0x00FF;

	// put back into cr1A the bit that corresponds to Horizontal sync Start	 bit 6 of CRTC 1A
	cr1A |= (hsync_start & 0x100) ? 0x0040 : 0x0; 

	// write out the new value to CRTC register 0x04
	outp((FxU16)(pDev->IoBase + 0xd5), (BYTE)cr4);

	// write out the new value to CRTC register 0x1A
	outp((FxU16)(pDev->IoBase + 0xd4), 0x1A);
    outp((FxU16)(pDev->IoBase + 0xd5), (BYTE)cr1A);

	// Lock the registers as we are finished with them.
	LockVgaTimingRegisters();
}

/*	This function sets the Horizontal Sync End CRTC
	register.  This register is CRTC register 0x05
	The function reads in the current value of the register
	increments it and then writes the new value back.

	The increment is supplied by the var h_inc which
	is passed to the function
*/
void setCRTC_HSyncEnd(PDEVTABLE pDev, char h_inc)
{
	unsigned int	hsync_end, cr1A;

	hsync_end = cr1A = 0;

	// Allow the code to write to the CRTC registers
	UnlockVgaTimingRegisters();

	// Write out 0x05 to the CRTC Index register so we can access
	// that register through 0xd5
	outp((FxU16)(pDev->IoBase + 0xd4), 0x5);
 	
	// Read in the value of 0x05 Horizontal End Sync into hsync_end
 	hsync_end = (inp((FxU16)(pDev->IoBase + 0xd5)));

	/* 	Write out 0x1A to the CRTC Index register so we can access
	 	that register through 0xd5.  Bit 7 of CRTC register 1A is bit
		5 of the Horizontal End Sync register
	*/
	outp((FxU16)(pDev->IoBase + 0xd4), 0x1A);

	// Read in the value of the CRTC register 0x1A into cr1A
	cr1A = (inp((FxU16)(pDev->IoBase + 0xd5)));

	// Bit 7 of CRTC 0x1A is bit 5 of the Horizontal End Sync Register
	// Or this into hsync_end
	hsync_end |= (cr1A & 0x80) ? 0x20 : 0;

	// Increment Horizontal End Sync by the increment value
	hsync_end += h_inc;

	// Mask out the	bit in cr1A that corresponds to the Horizontal End Sync Register
	cr1A &= ~0x80;

	// Or bit 5 of Horizontal Sync End back into cr1A
	cr1A |= (hsync_end & 0x020) ? 0x80 : 0;

	// output the values to the appropriate registers

	outp((FxU16)(pDev->IoBase + 0xd5), (BYTE)cr1A);

	outp((FxU16)(pDev->IoBase + 0xd4), 0x5);

	outp((FxU16)(pDev->IoBase + 0xd5), (BYTE)hsync_end);

	LockVgaTimingRegisters();
}

/*	This function modifies the Vertical Retrace start register
	to enable the vertical position of the image on the monitor
	to be moved.  The values are read from the required CRTC
	register, then incremented and written back into the registers.
	
	The increment value is supplied by the value passed to the function
	in v_inc.  */

void setCRTC_VSyncStart(PDEVTABLE pDev, int v_inc)
{
	unsigned int	vsync_start = 0, cr7, cr1B, cr10;

	UnlockVgaTimingRegisters();

    // Read vsync_start registers.

	outp((FxU16)(pDev->IoBase + 0xd4), 0x7);
	cr7 = (inp((FxU16)(pDev->IoBase + 0xd5)));
	
	outp((FxU16)(pDev->IoBase + 0xd4), 0x1B);
	cr1B = (inp((FxU16)(pDev->IoBase + 0xd5)));

	outp((FxU16)(pDev->IoBase + 0xd4), 0x10);
	cr10 = (inp((FxU16)(pDev->IoBase + 0xd5)));

    // Compute vsync_start value.

	vsync_start = cr10 & 0x00ff;               // bits[0..7] of vsync_start
	vsync_start |= (cr7  & 0x04) ? 0x0100 : 0; // bit[8] of vsync_start    
	vsync_start |= (cr7  & 0x80) ? 0x0200 : 0; // bit[9] of vsync_start    
	vsync_start |= (cr1B & 0x40) ? 0x0400 : 0; // bit[10] of vsync_start    

    // Adjust vsync_start value.

	vsync_start += v_inc;

    // Clear vsync_start extended bits.

	cr7  &= ~0x0084;
	cr1B &= ~0x0040;

    // Recompute vsync_start registers.

	cr10 = vsync_start & 0xff;                  // bits[0..7] of vsync_start
    cr7  |= (vsync_start & 0x0100) ? 0x04 : 0;  // bit[8] of vsync_start    
    cr7  |= (vsync_start & 0x0200) ? 0x80 : 0;  // bit[9] of vsync_start    
    cr1B |= (vsync_start & 0x0400) ? 0x40 : 0;  // bit[10] of vsync_start    

    // Write vsync_start registers.

	outp((FxU16)(pDev->IoBase + 0xd4), 0x7);
	outp((FxU16)(pDev->IoBase + 0xd5), (BYTE) cr7);

	outp((FxU16)(pDev->IoBase + 0xd4), 0x1B);
	outp((FxU16)(pDev->IoBase + 0xd5), (BYTE) cr1B);

	outp((FxU16)(pDev->IoBase + 0xd4), 0x10);
	outp((FxU16)(pDev->IoBase + 0xd5), (BYTE) cr10);

	LockVgaTimingRegisters();
}

/*	used when changing modes sets the position of the image on
	the monitor.  Also used when coming back from a Dos box to the
	desktop */

/*	This function sets the position of the image on the monitor after
	a mode change.  It reads the value the user has selected from the
	registry and uses this to modify the CRTC registers referring to
	the positon of the image on the monitor.  This is also called when
	coming back froma Dos box to the desktop. */

void MonChangeMode(PDEVTABLE pDev)
{
	int ret, xyValues[2];

	// read in the values from the registry
	ret = registryMonPos(1, xyValues, pDev);
	/* 	if ret = -1 something happened and we didn't get
		values from the registry correctly.  So just set
		the increments to 0. */
	if (ret == -1)
		xyValues[0] = xyValues[1] = 0;
	/* 	Update the CRTC registers with the position
		values read from the registry */
	setMonPos(xyValues, pDev);
	/* 	set monPos[0..1] to 0.  This is the var that stores the increments the user
		has selected but has not saved to the registry by clicking Apply or OK.
		We do this incase the user does something like change mode while on the
		MonitorControl page after having moved the position but not having saved it */
	monPos[0] = monPos[1] = 0;
}

/*	This function reads in values from the CRTC and returns them.
	valueRequired is the value to be read in:
	If valueRequired==1
		it returns Horizontal Sync Start
	2:
		returns Horizontal Sync End
	3:
		returns Vertical Sync Start
	4:
		returns Vertical Sync End
*/

int GetCRTC(PDEVTABLE pDev, int valueRequired)
{
	int syncValue = 0;

	switch (valueRequired)
	{
		case 1:
			// Get HSyncStart
		outp((FxU16)(pDev->IoBase + 0xd4), 0x4);
		syncValue = (inp((FxU16)(pDev->IoBase + 0xd5)));
			break;
		case 2:
			// Get HSyncEnd
		outp((FxU16)(pDev->IoBase + 0xd4), 0x5);
		syncValue = (inp((FxU16)(pDev->IoBase + 0xd5)));
			break;
		case 3:
			// Get VSyncStart
			outp((FxU16)(pDev->IoBase + 0xd4), 0x10);
			syncValue = (inp((FxU16)(pDev->IoBase + 0xd5)));
			break;
		case 4:
			// Get VSyncEnd
			outp((FxU16)(pDev->IoBase + 0xd4), 0x11);
			syncValue = (inp((FxU16)(pDev->IoBase + 0xd5)));
			break;
		default:
		// If there is a bug and valueRequired != 1..4 then just return 0
			break;
	}
	return syncValue;
}

// not used at this time
void setCRTC_overlayPos(PDEVTABLE pDev, int inc)
{
	/*DWORD	bit_11_0;
	DWORD	bit_23_12;
	DWORD	full;

	UnlockVgaTimingRegisters();

	full = (inp((FxU16)(pDev->IoBase + 0x9C)));

	bit_11_0 = (full & 0x00000FFF);
	bit_23_12 = (full & 0x00FFF000);

	full = (full & 0xFFFFF000);
	full = (full & 0xFF000FFF);

	bit_11_0 += inc;
	bit_23_12 += inc;

	full = (full | bit_11_0);
	full = (full | bit_23_12);

	outp((FxU16)(pDev->IoBase + 0x9C), (unsigned char)full);

	LockVgaTimingRegisters();*/
}

 /*	This function sets the position of the image on the monitor after the
 	user moves the postion of the image on the monitor.  It calls the functions
 	that set the CRTC register relating to the position of the image.
 	incs[] contains the 2 increment values.
 	incs[0] is the Horizontal increment value
 	incs[1] is the Vertical increment value */

void setMonPos(unsigned long incs[], PDEVTABLE pDev)
{
	int temp = 0;
	if (incs[0])
	{
		setCRTC_HSyncStart(pDev, (char) incs[0]);
		setCRTC_HSyncEnd(pDev, (char) incs[0]);
	}
	if (incs[1])
	{
		setCRTC_VSyncStart(pDev, (char) incs[1]);
	}
}

/* 	This two functions are to be used for resizing the
	image but aren't use as it doesn't work correctly
	at the minute */
void setCRTC_HTotal(PDEVTABLE pDev, int hSizeInc)
{
	BYTE	hTotal;
	int		temp03;


	UnlockVgaTimingRegisters();

	outp((FxU16)(pDev->IoBase + 0xd4), 0x0);
	hTotal = (inp((FxU16)(pDev->IoBase + 0xd5)));
	hTotal -= hSizeInc;

	outp((FxU16)(pDev->IoBase + 0xd5), hTotal);

	outp((FxU16)(pDev->IoBase + 0xd4), 0x03);
	temp03 = (inp((FxU16)(pDev->IoBase + 0xd5)));
	temp03 -= hSizeInc;

	outp((FxU16)(pDev->IoBase + 0xd5), (BYTE)temp03);

	LockVgaTimingRegisters();
}

void setCRTC_VTotal(PDEVTABLE pDev, int vSizeInc)
{
	BYTE vTotal;

	UnlockVgaTimingRegisters();

	outp((FxU16)(pDev->IoBase + 0xd4), 0x6);
	vTotal = (inp((FxU16)(pDev->IoBase + 0xd5)));
	vTotal += vSizeInc;
	outp((FxU16)(pDev->IoBase + 0xd5), vTotal);

	LockVgaTimingRegisters();
}

//used when changing modes set the size of the image on the monitor
// not currently used as resizing does not function correctly at the
// minute

int setMonSize(unsigned long incs[], PDEVTABLE pDev)
{
	int temp = 0;

	if (incs[0])
		setCRTC_HTotal(pDev, incs[0]);
	else if(incs[1])
		setCRTC_VTotal(pDev, incs[1]);

	return temp;
}

/* does same thing as strcat.  
 No checking to see if destination string 
 is long enough so make sure it is
 */

void fxstrcat(char *dest, char *source)
{
	char *temp, *temp2;

	temp = dest;
	temp2 = source;

	while (*temp != '\0')
	{
		temp++;
	}

	while (*temp2 != '\0')
	{
		*temp++ = *temp2++;
	}
}

// just a version of strcpy

void fxstrcpy(char *dest, char *source)
{
	while (*source != '\0')
		*dest++ = *source++;
	*dest = *source;
}

/*----------------------------------------------------------------------
Function name: dditoa

Description:   This version has been modified by jmccartney
				so that it does not add a new line character at the end.

Return:        int
----------------------------------------------------------------------*/
int dditoa( int val, char *result)
{
   DWORD     temp_val, rem; // sign;
   int     i=0;
   int     digits=0;


   temp_val = (DWORD) val;
   while(  (temp_val /= 10) != 0 )
      digits++;


   temp_val = (DWORD) val;
   for( i=digits; i>=0; --i)
   {
      rem = temp_val % 10;
      temp_val /= 10;
      result[i] = (char) rem;
      result[i] += '0';
   }

   return 1;
}// dditoa

/*  This function reads and writes the registry entires that
	store the increments that have been made to the position
	of the image on the monitor.  All are stored in
	pDev->dwDevNode\MODES\16\"resolution"\. 

	The var read refers to whether a read or a write is required.
	1 for a read and 2 for a write.

	The array values is the values to be read or written to the registry
	
	It returns 1 if it successful and -1 if it can't
	read or read the entries
	-2 should never be returned but is there incase	
	It is important we return whether we succed or not because on a machine
	that has just had Edge Tools installed there will be no reg. entries and the
	machine will just has been booted up.  This involves a mode switch which will
	try and read the values from the registry*/

int registryMonPos(int read, int values[], PDEVTABLE pDev)
{
	int		result, xValue = 0, yValue = 0;
	char	xRes[10] = "", yRes[10]="";
	char	path[30] = "";
	char	fullpath[40] = "";
	DWORD	value;
	FxU32	bufSize = sizeof(DWORD);
	
	/* 	Set up the string containing the path to the
		registry entries we require */
	dditoa((int)pDev->DispInfo.diXRes, xRes);
	dditoa((int)pDev->DispInfo.diYRes, yRes);
	fxstrcat(xRes, ",");
	fxstrcat(path, xRes);
	fxstrcat(path, yRes);
	fxstrcat(fullpath, "MODES\\16\\");
	fxstrcat(fullpath, path);
	fxstrcpy(path, "m_xcur");

	// if we want to read the entries
	if (read == 1)
	{
		if (CM_Read_Registry_Value(pDev->dwDevNode,
					   		       fullpath,
								   path,
								   REG_DWORD,
								   (BYTE *)&value,
								   &bufSize,
								   CM_REGISTRY_SOFTWARE) == CR_SUCCESS)
		{
			values[0] = (int) value;
			result = 1; // success
		}
		else
		{
			result = -1;  // -1 = failed to read registry
			return result;
		}	
	}
	else if(read == 2) // ie we want to write to the registry
	{
		value = values[0];
		if(CM_Write_Registry_Value(pDev->dwDevNode,
								   fullpath,
								   path,
								   REG_DWORD,
								   &value,
								   sizeof(DWORD),
								   CM_REGISTRY_SOFTWARE) == CR_SUCCESS)
		{
			result = 1; // success
		}
		else			// writing should never fail anyway
			return -1;
	}

	fxstrcpy(path, "m_ycur");	// now we want to read/write the Vertical entries (y)
	if (read == 1)
	{
		if (CM_Read_Registry_Value(pDev->dwDevNode,
					   		       fullpath,
								   path,
								   REG_DWORD,
								   (BYTE *)&value,
								   &bufSize,
								   CM_REGISTRY_SOFTWARE) == CR_SUCCESS)
		{
			values[1] = (int) value;
			result = 1; // success
		}
		else
		{
			return -1;
		}

	}
	else if(read == 2)
	{
		value = values[1];
		if(CM_Write_Registry_Value(pDev->dwDevNode,
								   fullpath,
								   path,
								   REG_DWORD,
								   &value,
								   sizeof(DWORD),
								   CM_REGISTRY_SOFTWARE) == CR_SUCCESS)
		{
			result = 1; // success
		}
		else			
			return -1;	// writing should never fail anyway
	}
	return result;
}

/*
 * Debug callbacks
 */
#pragma VxD_DEBUG_ONLY_CODE_SEG
#pragma VxD_DEBUG_ONLY_DATA_SEG

/*******************************************************************************
*
*       OnDebugQuery
*               This message cannot be failed
*
*       Input:
*
*       Output:
*               Return          -       TRUE to indicate success
*
*       Call the debug menu function
*       Return TRUE
*
*******************************************************************************/
void Napalm_Debug();
BOOL __stdcall OnDebugQuery()
{
        Napalm_Debug();

        return TRUE;
}
