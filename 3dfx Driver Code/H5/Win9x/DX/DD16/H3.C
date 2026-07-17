/* $Header: h3.c, 62, 10/27/00 12:05:55 AM, Jonny Cochrane$ */
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
** File name:   h3.c
**
** Description: Enable/Disable and supporting functions, structures,
**              macros, etc. for the Banshee/Avenger HW.
**
** $Revision: 62$
** $Date: 10/27/00 12:05:55 AM$
**
** $History: h3.c $
** 
** *****************  Version 279  *****************
** User: Andrew       Date: 9/13/99    Time: 5:08p
** Updated in $/devel/h5/Win9x/dx/dd16
** Added Check so that Default is 60 Hz
** 
** *****************  Version 278  *****************
** User: Xingc        Date: 9/10/99    Time: 11:31a
** Updated in $/devel/h5/Win9x/dx/dd16
** setup overlay gamma in Enable1() and ToForgroung()
** 
** *****************  Version 277  *****************
** User: Dale         Date: 9/09/99    Time: 6:24p
** Updated in $/devel/h5/Win9x/dx/dd16
** Enabled the dfp detection code since it works now
** 
** *****************  Version 276  *****************
** User: Xingc        Date: 8/30/99    Time: 2:06p
** Updated in $/devel/h5/Win9x/dx/dd16
** Set DosActive and DwRelaxedOverlayOwnerMode in ToBackGround and clear
** DosActive in ToForeground
** 
** *****************  Version 275  *****************
** User: Cwilcox      Date: 8/27/99    Time: 4:29p
** Updated in $/devel/h5/Win9x/dx/dd16
** Add initialization of ddSecondaryHeapSize, from Napalm DirectDraw code
** review.
** 
** *****************  Version 274  *****************
** User: Cwilcox      Date: 8/27/99    Time: 11:32a
** Updated in $/devel/h5/Win9x/dx/dd16
** Fixed PRS #7981, caused by failing to disable SaveScreenBitmap when
** entering DirectX.
** 
** *****************  Version 273  *****************
** User: Cwilcox      Date: 8/24/99    Time: 3:35p
** Updated in $/devel/h5/Win9x/dx/dd16
** Removed LCDActive check until LCD detection problem is fixed.
** 
** *****************  Version 272  *****************
** User: Andrew       Date: 8/23/99    Time: 6:59p
** Updated in $/devel/h5/Win9x/dx/dd16
** changes for host base cursor
** 
** *****************  Version 271  *****************
** User: Cwilcox      Date: 8/23/99    Time: 3:16p
** Updated in $/devel/h5/Win9x/dx/dd16
** Added secondary heap to avoid tile mark problems.
** 
** *****************  Version 270  *****************
** User: Cwilcox      Date: 8/16/99    Time: 12:19p
** Updated in $/devel/h5/Win9x/dx/dd16
** Added code to "recycle" linear heaps.
** 
** *****************  Version 269  *****************
** User: Cwilcox      Date: 8/14/99    Time: 11:03a
** Updated in $/devel/h5/Win9x/dx/dd16
** SST_H5_SGRAM_TYPE for Napalm and SST_H4_SGRAM_TYPE for Voodoo3.
** 
** *****************  Version 268  *****************
** User: Rbissell     Date: 8/09/99    Time: 3:45p
** Updated in $/devel/h5/Win9x/dx/dd16
** 
** *****************  Version 267  *****************
** User: Tlittle      Date: 8/09/99    Time: 10:46a
** Updated in $/devel/h5/Win9x/dx/dd16
** Commented out an un-used variable to fix a compile error when warnings
** are treated as errors.
** 
** *****************  Version 266  *****************
** User: Rbissell     Date: 8/07/99    Time: 3:11p
** Updated in $/devel/h5/Win9x/dx/dd16
** tvout merge from V3_OEM_100
** 
** *****************  Version 265  *****************
** User: Msmith       Date: 7/30/99    Time: 6:27p
** Updated in $/devel/h5/Win9x/dx/dd16
** took out a useless #ifdef
** 
** *****************  Version 264  *****************
** User: Msmith       Date: 7/30/99    Time: 6:18p
** Updated in $/devel/h5/Win9x/dx/dd16
** Enabled Bob J's changes again.
** 
** *****************  Version 263  *****************
** User: Msmith       Date: 7/30/99    Time: 5:49p
** Updated in $/devel/h5/Win9x/dx/dd16
** Took Bob J's changes out temporarily
** 
** *****************  Version 262  *****************
** User: Msmith       Date: 7/30/99    Time: 4:55p
** Updated in $/devel/h5/Win9x/dx/dd16
** Checked in for Bob Johnston - added a line for multi-mon support
** 
** *****************  Version 261  *****************
** User: Andrew       Date: 7/20/99    Time: 10:44a
** Updated in $/devel/h5/Win9x/dx/dd16
** Changed a MemSize failure check to allow 32 MB and 64 MB.
** 
** *****************  Version 260  *****************
** User: Cshaw        Date: 7/19/99    Time: 12:52p
** Updated in $/devel/h5/Win9x/dx/dd16
** Changed a #if defined(h5) to runtime.
** 
** *****************  Version 259  *****************
** User: Cwilcox      Date: 7/19/99    Time: 11:15a
** Updated in $/devel/h5/Win9x/dx/dd16
** Fix previous Napalm memory allocation changes.
** 
** *****************  Version 258  *****************
** User: Cwilcox      Date: 7/16/99    Time: 5:13p
** Updated in $/devel/h5/Win9x/dx/dd16
** Implemented new memory management scheme on Napalm.
** 
** *****************  Version 257  *****************
** User: Andrew       Date: 7/16/99    Time: 12:45p
** Updated in $/devel/h5/Win9x/dx/dd16
** Changed regBase from a single dword to 4 to support sparse mapping of
** the memory mapped space
** 
** *****************  Version 256  *****************
** User: Cwilcox      Date: 7/15/99    Time: 4:10p
** Updated in $/devel/h5/Win9x/dx/dd16
** Add runtime checks for Napalm.
** 
** *****************  Version 255  *****************
** User: Andrew       Date: 7/09/99    Time: 4:19p
** Updated in $/devel/h5/Win9x/dx/dd16
** Minor change to pass the number of units around
** 
** *****************  Version 254  *****************
** User: Cwilcox      Date: 7/08/99    Time: 1:27p
** Updated in $/devel/h5/Win9x/dx/dd16
** Added runtime checking for Napalm versus Voodoo3.
** 
** *****************  Version 253  *****************
** User: Dalev        Date: 7/01/99    Time: 6:46p
** Updated in $/devel/h5/Win9x/dx/dd16
** Updated 'Enable1()' to copy 'PLDRevisionID' in 'HwInfo' struc to
** 'GLOBALDATA' struc.
** 
** *****************  Version 252  *****************
** User: Edwin        Date: 6/29/99    Time: 3:57p
** Updated in $/devel/h5/Win9x/dx/dd16
** Remove obsolete Banshee ifdefs.
** 
** *****************  Version 251  *****************
** User: Andrew       Date: 6/25/99    Time: 4:30p
** Updated in $/devel/h5/Win9x/dx/dd16
** Added code to set dwSwitchCursor
** 
** *****************  Version 250  *****************
** User: Cwilcox      Date: 6/25/99    Time: 10:33a
** Updated in $/devel/h5/Win9x/dx/dd16
** Changes tile pitch calculation, merged NO_REFRESH code.
** 
** *****************  Version 249  *****************
** User: Cwilcox      Date: 6/22/99    Time: 5:14p
** Updated in $/devel/h5/Win9x/dx/dd16
** Moved device IDs to h3g.h.
** 
** *****************  Version 248  *****************
** User: Andrew       Date: 6/18/99    Time: 3:28p
** Updated in $/devel/h5/Win9x/dx/dd16
** Changed h3.c so that you can build it with HP=H4 or HP=H5 when WCS=1
** 
** *****************  Version 247  *****************
** User: Andrew       Date: 6/18/99    Time: 10:13a
** Updated in $/devel/h5/Win9x/dx/dd16
** Added code to Wait for Idle in Disable1 and ToBackground.
** 
** *****************  Version 246  *****************
** User: Andrew       Date: 6/17/99    Time: 2:33p
** Updated in $/devel/h5/Win9x/dx/dd16
** Removed ifdef HAL_CSIM and HAL_HW
** 
** *****************  Version 245  *****************
** User: Cwilcox      Date: 6/16/99    Time: 10:39a
** Updated in $/devel/h5/Win9x/dx/dd16
** Removed kludge to limit number of buffers in 8Mb configuration.
** 
** *****************  Version 244  *****************
** User: Cwilcox      Date: 6/16/99    Time: 10:33a
** Updated in $/devel/h5/Win9x/dx/dd16
** Removed #ifdef H3_AGP_WORKAROUND code.
** 
** *****************  Version 243  *****************
** User: Andrew       Date: 6/15/99    Time: 5:21p
** Updated in $/devel/h5/Win9x/dx/dd16
** Added dwSpecialNumber for when we don't have a unique DevNode
** 
** *****************  Version 242  *****************
** User: Andrew       Date: 6/04/99    Time: 4:10p
** Updated in $/devel/h5/Win9x/dx/dd16
** Added code to start to support UnitNumbers
** 
** *****************  Version 241  *****************
** User: Michael      Date: 6/02/99    Time: 2:42p
** Updated in $/devel/h5/Win9x/dx/dd16
** Add -WX to the makefile and clean up all warnings in the DD16
** subdirectory.
** 
** *****************  Version 239  *****************
** User: Stb_srogers  Date: 5/26/99    Time: 1:33p
** Updated in $/devel/h3/win95/dx/dd16
** Removing check for ddPrimaryInTile because it is not setup at this
** point.
** 
** *****************  Version 238  *****************
** User: Cwilcox      Date: 5/26/99    Time: 1:26p
** Updated in $/devel/h3/Win95/dx/dd16
** Added AGP texture download back into TOT.
** 
** *****************  Version 237  *****************
** User: Cwilcox      Date: 5/24/99    Time: 4:39p
** Updated in $/devel/h3/Win95/dx/dd16
** Removed AGP texture download.
** 
** *****************  Version 236  *****************
** User: Srogers      Date: 5/20/99    Time: 11:55a
** Updated in $/devel/h3/Win95/dx/dd16
** Limiting 8 MB cards running at tiled fullscreen at a resolution of
** greater than 1024x768 to use 2 colorbuffers if 3 are calculated.
** 
** *****************  Version 235  *****************
** User: Stb_srogers  Date: 5/14/99    Time: 3:29p
** Updated in $/devel/h3/win95/dx/dd16
** Temporary Fix for PRS5878, Remove the disable mouse trails in Win98 so
** that Nascar Revolution will work
** 
** *****************  Version 234  *****************
** User: Andrew       Date: 5/14/99    Time: 1:33p
** Updated in $/devel/h3/Win95/dx/dd16
** Added Real/Fake Mem Base 0,1
** 
** *****************  Version 233  *****************
** User: Andrew       Date: 5/13/99    Time: 4:11p
** Updated in $/devel/h3/Win95/dx/dd16
** Changed to support name change from lfbRealBase to lfbFakeBase
** 
** *****************  Version 232  *****************
** User: Cwilcox      Date: 5/12/99    Time: 1:05p
** Updated in $/devel/h3/Win95/dx/dd16
** Implemented tiled/linear switching in 16-bit code.
** 
** *****************  Version 231  *****************
** User: Andrew       Date: 5/10/99    Time: 1:34p
** Updated in $/devel/h3/Win95/dx/dd16
** Changed PhysScreenAddr to RealregBase
** 
** *****************  Version 230  *****************
** User: Andrew       Date: 5/07/99    Time: 6:00p
** Updated in $/devel/h3/Win95/dx/dd16
** For the Win CSIM build ifdef out the code to check for LCD and TVOUT
** 
** *****************  Version 229  *****************
** User: Andrew       Date: 5/06/99    Time: 4:23p
** Updated in $/devel/h3/Win95/dx/dd16
** Modified to work with Trapping "C" Simulator
** 
** *****************  Version 228  *****************
** User: Cwilcox      Date: 5/03/99    Time: 4:31p
** Updated in $/devel/h3/Win95/dx/dd16
** Changes to allow tiled/linear switching.
** 
** *****************  Version 227  *****************
** User: Cwilcox      Date: 5/03/99    Time: 12:35p
** Updated in $/devel/h3/Win95/dx/dd16
** Reversed allocation order of tiled heap and primary surface.
** 
** *****************  Version 226  *****************
** User: Stb_echilds  Date: 4/28/99    Time: 3:17p
** Updated in $/devel/h3/win95/dx/dd16
** Moved the OS version check code from Control1 to Enable1.
** 
** *****************  Version 225  *****************
** User: Cwilcox      Date: 4/26/99    Time: 4:55p
** Updated in $/devel/h3/Win95/dx/dd16
** Added tiled memory support for 32bpp modes, Napalm only.
** 
** *****************  Version 224  *****************
** User: Cwilcox      Date: 4/22/99    Time: 10:02a
** Updated in $/devel/h3/Win95/dx/dd16
** Fixed number of buffers calculation.
** 
** *****************  Version 223  *****************
** User: Cwilcox      Date: 4/21/99    Time: 5:03p
** Updated in $/devel/h3/Win95/dx/dd16
** Removed obsolete 16-bit linear memory manager.
** 
** *****************  Version 222  *****************
** User: Cshaw        Date: 4/20/99    Time: 12:24p
** Updated in $/devel/h3/Win95/dx/dd16
** Added Napalm's enabling of 2-pixel-per-clock rendering for 16bpp modes
** of "non-high" bandwidth requirements.  We'll need to tweak which modes
** are enabled after we get Silicon back.
** 
** *****************  Version 221  *****************
** User: Cwilcox      Date: 4/19/99    Time: 11:43a
** Updated in $/devel/h3/Win95/dx/dd16
** Change tiled pitch calculation back to original!
** 
** *****************  Version 220  *****************
** User: Cwilcox      Date: 4/16/99    Time: 4:42p
** Updated in $/devel/h3/Win95/dx/dd16
** Modified to restrict offscreen heap alignment.
** 
** *****************  Version 219  *****************
** User: Stb_bseitsin Date: 4/14/99    Time: 2:38p
** Updated in $/devel/h3/win95/dx/dd16
** ifdef h4 to ifdef h4 or h5
** 
** *****************  Version 218  *****************
** User: Stb_echilds  Date: 4/13/99    Time: 12:48p
** Updated in $/devel/h3/win95/dx/dd16
** Fix PRS 5400. Check OS Version first, disable MOUSETRAILS only in
** Win98.
** 
** *****************  Version 217  *****************
** User: Stb_bseitsin Date: 4/09/99    Time: 12:37p
** Updated in $/devel/h3/win95/dx/dd16
** Added Napalm registers. Added ifdef H5.
** 
** *****************  Version 216  *****************
** User: Cwilcox      Date: 4/08/99    Time: 4:48p
** Updated in $/devel/h3/Win95/dx/dd16
** Miscellaneous cleanup of setupOffscreenMemory variables and comments.
** 
** *****************  Version 215  *****************
** User: Cwilcox      Date: 4/08/99    Time: 1:59p
** Updated in $/devel/h3/Win95/dx/dd16
** Added #ifdef LINEAR_ONLY to build without tiled mode.
** 
** *****************  Version 214  *****************
** User: Stb_sjohnston Date: 4/07/99    Time: 2:52p
** Updated in $/devel/h3/win95/dx/dd16
** Cleaner fix for Slashdot/Wax bug (PRS 4457) from Chris Edgington
** 
** *****************  Version 213  *****************
** User: Andrew       Date: 4/06/99    Time: 12:38a
** Updated in $/devel/h3/Win95/dx/dd16
** Added code to set/clear UnGlideContext
** 
** *****************  Version 212  *****************
** User: Edwin        Date: 4/05/99    Time: 2:51p
** Updated in $/devel/h3/Win95/dx/dd16
** SetupOffscreenMemory computes incorrect numBuffers above 1600x1200x16.
** Apache Havoc demo GPF as a result.
** 
** *****************  Version 211  *****************
** User: Andrew       Date: 4/01/99    Time: 11:53a
** Updated in $/devel/h3/Win95/dx/dd16
** Added code to clear Glide Exculsive flag in toforeground and
** tobackground
** 
** *****************  Version 210  *****************
** User: Pratt        Date: 2/28/99    Time: 4:50p
** Updated in $/devel/h3/Win95/dx/dd16
** added debug statements for tracing flow of LCD code.
** 
** *****************  Version 209  *****************
** User: Andrew       Date: 3/30/99    Time: 10:19a
** Updated in $/devel/h3/Win95/dx/dd16
** Added Code to ReEnable1 to fail if we are in Low Power Mode.
** 
** *****************  Version 208  *****************
** User: Cwilcox      Date: 3/26/99    Time: 4:00p
** Updated in $/devel/h3/Win95/dx/dd16
** Major modification to memory allocation and management, fix PRS #4229.
** 
** *****************  Version 207  *****************
** User: Michael      Date: 3/25/99    Time: 2:19p
** Updated in $/devel/h3/Win95/dx/dd16
** Disable the certification bit in TOT(Control1()) for now so daily
** builds aren't considered WHQL'd.
** 
** *****************  Version 206  *****************
** User: Michael      Date: 3/25/99    Time: 10:27a
** Updated in $/devel/h3/Win95/dx/dd16
** Mike Imhoff's V3 OEM fix for PRS 5262: For performance and other
** reasons, MouseTrails are not supported, we return false so that the
** Mouse Properties menu item will be grayed out.
** 
** *****************  Version 205  *****************
** User: Michael      Date: 3/19/99    Time: 11:29a
** Updated in $/devel/h3/Win95/dx/dd16
** Believed to fix PRS 4843.  Also fixes PRS 1922 which is closed by still
** reproducible.  Correctly fixes PRS 1303.  In setupPalette, change
** pointer reference to address color_table rather than biClrImport.  In
** DibEnable, reset first three entries of color_table when in 8bpp.
** Entries tell DIB engine pixel layout, not getting correctly reset after
** a change res on the fly.
** 
** *****************  Version 204  *****************
** User: Michael      Date: 3/18/99    Time: 4:45p
** Updated in $/devel/h3/Win95/dx/dd16
** In Control1(), enable the Certification Bit.
** 
** *****************  Version 203  *****************
** User: Cwilcox      Date: 3/18/99    Time: 10:02a
** Updated in $/devel/h3/Win95/dx/dd16
** Fix PRS 5065, update primary to not get stale.
** 
** *****************  Version 202  *****************
** User: Michael      Date: 3/17/99    Time: 4:11p
** Updated in $/devel/h3/Win95/dx/dd16
** Fix PRS 5179.  Clear screen in 2046 mode to remove 2-pixel garbage on
** right side of screen.
** 
** *****************  Version 201  *****************
** User: Stb_pzheng   Date: 3/12/99    Time: 10:34p
** Updated in $/devel/h3/win95/dx/dd16
** Restore command fifo size to 1 meg after fixing the bug which causes
** PRS # 4604
** 
** *****************  Version 200  *****************
** User: Xingc        Date: 3/11/99    Time: 10:24a
** Updated in $/devel/h3/Win95/dx/dd16
** Move POST_MODE_CHANGE VDDCall from ResetHiResMode() into ToForeground()
** for DOS BOX
** 
** *****************  Version 199  *****************
** User: Xingc        Date: 3/10/99    Time: 9:10p
** Updated in $/devel/h3/Win95/dx/dd16
** Add VDDCall for POST_MODE_CHANGE to fix PRS4760
** 
** *****************  Version 198  *****************
** User: Stb_srogers  Date: 3/09/99    Time: 2:37p
** Updated in $/devel/h3/win95/dx/dd16
** Fixes bug I created in last checkin
** 
** *****************  Version 197  *****************
** User: Stb_srogers  Date: 3/09/99    Time: 1:45p
** Updated in $/devel/h3/win95/dx/dd16
** Removed 1Mg Command Fifo for 1024x768@32bpp.  This is a temporary fix
** for PRS#4604, but Ping is working on a true fix.
** 
** *****************  Version 196  *****************
** User: Stb_srogers  Date: 3/06/99    Time: 7:32a
** Updated in $/devel/h3/win95/dx/dd16
** Removing 24 bit modes for 1792x1344 and 1856x1392
** 
** *****************  Version 195  *****************
** User: Stb_sjohnston Date: 3/03/99    Time: 2:07p
** Updated in $/devel/h3/win95/dx/dd16
** Fix for DELL TV PCI DLL issue
** 
** *****************  Version 194  *****************
** User: Stb_srogers  Date: 3/02/99    Time: 9:48p
** Updated in $/devel/h3/Win95/dx/dd16
** Removed #ifdef INCSTBCUST for OEM mode culling.
** This will now be apart of the nightly builds in San Jose.
** 
** *****************  Version 193  *****************
** User: Stb_srogers  Date: 3/02/99    Time: 3:56p
** Updated in $/devel/h3/Win95/dx/dd16
** Adding support for 960x720 & 1280x960 modes
** 
** *****************  Version 192  *****************
** User: Cwilcox      Date: 3/01/99    Time: 1:27p
** Updated in $/devel/h3/Win95/dx/dd16
** Fix PRS #4159, disallow locks to stale surfaces.
** 
** *****************  Version 191  *****************
** User: Michael      Date: 2/26/99    Time: 10:21a
** Updated in $/devel/h3/Win95/dx/dd16
** Proxy for KenW - These changes make alt-tab from winglide apps *much*
** more reliable.  I now see no corruption or hangs from glquake or
** heretic2.   NBA98 also worked.  PRS 3320 is possibly associated.  There
** may be others.
** 
** *****************  Version 190  *****************
** User: Stb_srogers  Date: 2/22/99    Time: 12:03p
** Updated in $/devel/h3/win95/dx/dd16
** Fixes PRS4388 & 4399, changed fifoSize to 256k instead of 64k.
** 
** *****************  Version 189  *****************
** User: Stb_srogers  Date: 2/22/99    Time: 7:31a
** Updated in $/devel/h3/win95/dx/dd16
** Changes outward appearance of 2048 to 2046
** 
** *****************  Version 188  *****************
** User: Michael      Date: 2/21/99    Time: 10:05a
** Updated in $/devel/h3/Win95/dx/dd16
** Changes memSetting[] for H4: EdwinW's fix for PRS 4473 and 4389, change
** fifosize to 1meg for 1600x1024x16.  Mirrors what was done to MT2
** branch.  Also note, ChrisW's previous checkin (version 186) fixes PRS
** 4438 and related issues.
** 
** *****************  Version 187  *****************
** User: Martin       Date: 2/20/99    Time: 5:18p
** Updated in $/devel/h3/Win95/dx/dd16
** (i) Setup buffer area in lfb space.
** (ii) Allocate agp memory for agp texture downloads
** 
** *****************  Version 186  *****************
** User: Cwilcox      Date: 2/19/99    Time: 2:04p
** Updated in $/devel/h3/Win95/dx/dd16
** Changed number of buffers for 1600x1200x16bpp.
** 
** *****************  Version 185  *****************
** User: Stb_srogers  Date: 2/18/99    Time: 4:24p
** Updated in $/devel/h3/win95/dx/dd16
** 
** *****************  Version 184  *****************
** User: Cwilcox      Date: 2/18/99    Time: 12:19p
** Updated in $/devel/h3/Win95/dx/dd16
** Final removal of tiled/linear promotion.
** 
** *****************  Version 183  *****************
** User: Stb_srogers  Date: 2/16/99    Time: 4:07p
** Updated in $/devel/h3/win95/dx/dd16
** 
** *****************  Version 182  *****************
** User: Stuartb      Date: 2/16/99    Time: 12:42p
** Updated in $/devel/h3/Win95/dx/dd16
** Move call to tvoutSetStdInternal before FirstEnable is set FALSE so we
** can tell if this is first time through.
** 
** *****************  Version 181  *****************
** User: Stb_srogers  Date: 2/11/99    Time: 6:58p
** Updated in $/devel/h3/win95/dx/dd16
** 
** *****************  Version 180  *****************
** User: Andrew       Date: 2/11/99    Time: 3:43p
** Updated in $/devel/h3/Win95/dx/dd16
** Added a check to makesure that the SW Cursor is enabled before
** restoring.
** 
** *****************  Version 179  *****************
** User: Mconrad      Date: 2/11/99    Time: 11:07a
** Updated in $/devel/h3/Win95/dx/dd16
** Support for Banshee/Voodoo3 file names based on HP= H3 or H4
** environment variable. Fixes PRSs #4087, #4088, #4151.
** 
** *****************  Version 178  *****************
** User: Cwilcox      Date: 2/10/99    Time: 3:50p
** Updated in $/devel/h3/Win95/dx/dd16
** Linear versus tiled promotion removal.
** 
** *****************  Version 177  *****************
** User: Stb_srogers  Date: 2/09/99    Time: 12:13p
** Updated in $/devel/h3/win95/dx/dd16
** 
** *****************  Version 176  *****************
** User: Ken          Date: 2/08/99    Time: 2:10p
** Updated in $/devel/h3/win95/dx/dd16
** added cpu/OS detection of pentium III (katmai) processors, and added
** katmai-optimized d3d texture download
** 
** *****************  Version 175  *****************
** User: Andrew       Date: 2/07/99    Time: 4:43p
** Updated in $/devel/h3/Win95/dx/dd16
** Added 1600x1024, 1920x1200, and 2048x1536 to MemSettings.
** 
** *****************  Version 174  *****************
** User: Andrew       Date: 1/29/99    Time: 11:29a
** Updated in $/devel/h3/Win95/dx/dd16
** Fixed a compile error
** 
** *****************  Version 173  *****************
** User: Stb_srogers  Date: 1/29/99    Time: 6:54a
** Updated in $/devel/h3/win95/dx/dd16
** 
** *****************  Version 172  *****************
** User: Andrew       Date: 1/28/99    Time: 4:23p
** Updated in $/devel/h3/Win95/dx/dd16
** Fixed minor compile warning fix to stop ship issue to a non-issue
** 
** *****************  Version 171  *****************
** User: Cwilcox      Date: 1/25/99    Time: 12:31p
** Updated in $/devel/h3/Win95/dx/dd16
** Another modification to remove compiler warnings.
** 
** *****************  Version 170  *****************
** User: Stuartb      Date: 1/25/99    Time: 11:07a
** Updated in $/devel/h3/Win95/dx/dd16
** Set _FF(dwTvoActive) TRUE on FirstEnable iff we booted to tvout.
** 
** *****************  Version 169  *****************
** User: Cwilcox      Date: 1/25/99    Time: 11:36a
** Updated in $/devel/h3/Win95/dx/dd16
** Minor modifications to remove compiler warnings.
** 
** *****************  Version 168  *****************
** User: Stuartb      Date: 1/12/99    Time: 2:45p
** Updated in $/devel/h3/Win95/dx/dd16
** Added parameter decl in Enable1 so it would compile.  My whoops.
** 
** *****************  Version 167  *****************
** User: Stuartb      Date: 1/12/99    Time: 2:26p
** Updated in $/devel/h3/Win95/dx/dd16
** Flag flatpanel as active (_FF(dwLcdActive)) if we booted to it.
** 
** *****************  Version 166  *****************
** User: Michael      Date: 1/12/99    Time: 1:41p
** Updated in $/devel/h3/Win95/dx/dd16
** Add the STB customer specific code.  All STB added code is surrounded
** by "#ifdef INCSTBCUST/#endif"
** 
** *****************  Version 165  *****************
** User: Michael      Date: 12/31/98   Time: 9:31a
** Updated in $/devel/h3/Win95/dx/dd16
** STB's customized refresh rates and customer file naming conventions.
** STB's customizations are surrounded by "#ifdef INCSTBCUST".
** 
** *****************  Version 164  *****************
** User: Michael      Date: 12/29/98   Time: 11:02a
** Updated in $/devel/h3/Win95/dx/dd16
** Implement the 3Dfx/STB unified header.
** 
** *****************  Version 163  *****************
** User: Andrew       Date: 12/24/98   Time: 2:17p
** Updated in $/devel/h3/Win95/dx/dd16
** Added registry entry force1xrate.  If set to 1 we will force 1x rate
** 
** *****************  Version 162  *****************
** User: Michael      Date: 12/22/98   Time: 5:21p
** Updated in $/devel/h3/Win95/dx/dd16
** Add code to handle a mode failure during enable.  If we fail, attempt
** to step down and force 640x480 for the current color depth.  This
** averts failing enable and coming up as vga.drv, generally caused by a
** DDC failure when a monitor is changed.  Fixes PRS 3866 for the Gateway
** GM2 release.
** 
** *****************  Version 161  *****************
** User: Andrew       Date: 12/20/98   Time: 11:10a
** Updated in $/devel/h3/Win95/dx/dd16
** Added H4 OEM Pll Table
** 
** *****************  Version 160  *****************
** User: Andrew       Date: 12/18/98   Time: 9:33p
** Updated in $/devel/h3/Win95/dx/dd16
** Added Code to Check IOBase, MemBase, and FBBase to see if they are
** mapped in correct.  If not drop back to VGA driver
** 
** *****************  Version 159  *****************
** User: Cwilcox      Date: 12/11/98   Time: 11:13a
** Updated in $/devel/h3/Win95/dx/dd16
** Redo previous checkin.
** 
** *****************  Version 157  *****************
** User: Agus         Date: 12/10/98   Time: 12:07p
** Updated in $/devel/h3/Win95/dx/dd16
** Added DVDHACK block from GLOP branch, disable AA_HEAPHACK
** 
** *****************  Version 156  *****************
** User: Martin       Date: 12/08/98   Time: 2:35p
** Updated in $/devel/h3/Win95/dx/dd16
** Default messages to off
** 
** *****************  Version 155  *****************
** User: Michael      Date: 12/02/98   Time: 9:04a
** Updated in $/devel/h3/Win95/dx/dd16
** Add ClearSwapCount=TRUE during a mode swtich.
** 
** *****************  Version 154  *****************
** User: Andrew       Date: 11/30/98   Time: 12:59a
** Updated in $/devel/h3/Win95/dx/dd16
** Added code to check to see if the Device is Busy before using the
** Device.  The Busy bit is set in low power mode so the device will
** return all f's if read from it when PCI64=D3.  This will cause a
** infinite SW loop.
** 
** *****************  Version 153  *****************
** User: Andrew       Date: 11/24/98   Time: 8:28a
** Updated in $/devel/h3/Win95/dx/dd16
** Added code to support PCI/AGP in the same driver
** 
** *****************  Version 152  *****************
** User: Jdt          Date: 11/23/98   Time: 9:07a
** Updated in $/DEVEL/h3/win95/dx/dd16
** OpenGL ICD Support: Added OPENGL_GETINFO to QUERYESCSUPPORT and
** implemention for OPENGL_GETINFO to Control1()
** 
** *****************  Version 151  *****************
** User: Andrew       Date: 11/22/98   Time: 8:21p
** Updated in $/devel/h3/Win95/dx/dd16
** Added H4 Pll Table so that Voodoo 3 can run grxClock/memclock from 30
** Mhz to 220 Mhz.  Changed GetBinaryDword to read registry as both DWORD
** and binary bytes to support Win 95 and Win 98.
** 
** *****************  Version 150  *****************
** User: Agus         Date: 11/18/98   Time: 5:11p
** Updated in $/devel/h3/Win95/dx/dd16
** Changed DDVERSION info to return HAL's DX version compiled with.
** 
** *****************  Version 149  *****************
** User: Andrew       Date: 11/16/98   Time: 8:34p
** Updated in $/devel/h3/Win95/dx/dd16
** Added code to kinda of handle the H4 PLL table
** 
** *****************  Version 148  *****************
** User: Andrew       Date: 11/15/98   Time: 5:18p
** Updated in $/devel/h3/Win95/dx/dd16
** Added ifdef so we can make build like GLOP
** 
** *****************  Version 147  *****************
** User: Stuartb      Date: 11/11/98   Time: 10:42a
** Updated in $/devel/h3/Win95/dx/dd16
** Just declared some extern fn's too decrease number of compile time
** warnings.
** 
** *****************  Version 146  *****************
** User: Stuartb      Date: 11/06/98   Time: 5:39p
** Updated in $/devel/h3/Win95/dx/dd16
** If no tvout part, report videoparameters all zero per Agus.
** 
** *****************  Version 145  *****************
** User: Stuartb      Date: 11/05/98   Time: 9:56a
** Updated in $/devel/h3/Win95/dx/dd16
** Init tvout in Enable1 if BIOS says we booted to tvout.
** 
** *****************  Version 144  *****************
** User: Martin       Date: 10/29/98   Time: 2:13p
** Updated in $/devel/h3/Win95/dx/dd16
** Modification of cmdFifo macros to support future changes for
** super-sampling AA.
** 
** *****************  Version 143  *****************
** User: Stuartb      Date: 10/22/98   Time: 10:31a
** Updated in $/devel/h3/Win95/dx/dd16
** Rolled in Macrovision stuff from GLOP and win98 tvout ControlEx stuff.
** 
** *****************  Version 142  *****************
** User: Ken          Date: 10/16/98   Time: 11:14a
** Updated in $/devel/h3/win95/dx/dd16
** mirroring GLOP changes into TOT for clock change policy (don't touch
** PLLs at boot by default, they still can be tweaked from the registry
** though), and for the new "Hank" memory timing rules (dramInit1 bit 14
** is speed dependent), and memClock and grxClock registry keys are now
** string keys
** 
** *****************  Version 141  *****************
** User: Andrew       Date: 10/08/98   Time: 4:26p
** Updated in $/devel/h3/Win95/dx/dd16
** Moved where we set ddMiscFlags to zero to before InitDeviceBitmapFilter
** 
** *****************  Version 140  *****************
** User: Andrew       Date: 10/06/98   Time: 8:56a
** Updated in $/devel/h3/Win95/dx/dd16
** Added new allocation for new cursor
** 
** *****************  Version 139  *****************
** User: Edwin        Date: 9/21/98    Time: 3:31p
** Updated in $/devel/h3/Win95/dx/dd16
** Fix 1882, 1610 (MotorHead corrupt result screen). Add dwBpp to
** GetMemConfig, return 0 numbuffers so we will not use tile heaps if not
** in 16bpp.
** 
** *****************  Version 138  *****************
** User: Artg         Date: 9/10/98    Time: 9:56a
** Updated in $/devel/h3/Win95/dx/dd16
** stuff for full screen dos.
** h4 csim supports 16 megs so the csim drv now runs with 16 megs of
** framebuffer memory.  16 megs support was necessary because the
** supersampling support for AA caused the base address to go negative.
** 
** The h4 csim does not reinitailizes conveniently.  Upon the second
** csiminitdriver call, the csim remallocs the memory needed for its
** internal state but does not reinitialize the internal state.  Rather
** than tracking it down in the csim, the csim driver now checks
** FirstEnable variable.
** 
** *****************  Version 137  *****************
** User: Ken          Date: 9/02/98    Time: 12:47p
** Updated in $/devel/h3/win95/dx/dd16
** when disabling/enabling, restore the mode active at the time of the
** disable, not the current registry setting.  fixes mtm1 title screen,
** PRS #2481
** 
** *****************  Version 136  *****************
** User: Suninn       Date: 10/30/98   Time: 5:51p
** Updated in $/devel/h3/Win95/dx/dd16
** initialize ddMiscFlags
** 
** *****************  Version 135  *****************
** User: Miriam       Date: 9/01/98    Time: 4:40p
** Updated in $/devel/h3/Win95/dx/dd16
** New flip code. 
** 
** *****************  Version 133  *****************
** User: Suninn       Date: 8/28/98    Time: 4:47p
** Updated in $/devel/h3/Win95/dx/dd16
** overlay hack for 640x480x8 bug
** 
** *****************  Version 132  *****************
** User: Andrew       Date: 8/28/98    Time: 1:00p
** Updated in $/devel/h3/Win95/dx/dd16
** Added a check to only init_gammaramp the first time
** 
** *****************  Version 131  *****************
** User: Artg         Date: 8/27/98    Time: 7:57p
** Updated in $/devel/h3/Win95/dx/dd16
** had LFBBASE = Screenaddress which was wrong.
** 
** *****************  Version 130  *****************
** User: Artg         Date: 8/27/98    Time: 11:36a
** Updated in $/devel/h3/Win95/dx/dd16
** h4 csim support.  
** h3write_hw, h3read_hw added to init banshee with csim
** 
** *****************  Version 129  *****************
** User: Edwin        Date: 8/25/98    Time: 4:36p
** Updated in $/devel/h3/Win95/dx/dd16
** Align DriverData on dword boundary.
** 
** *****************  Version 128  *****************
** User: Andrew       Date: 8/25/98    Time: 12:35a
** Updated in $/devel/h3/Win95/dx/dd16
** Addded SetBusy/ClrBusy around InitThunks
** 
** *****************  Version 127  *****************
** User: Adrians      Date: 8/23/98    Time: 12:50a
** Updated in $/devel/h3/Win95/dx/dd16
** Added support for AA SuperSampling.
** 
** *****************  Version 126  *****************
** User: Michael      Date: 8/20/98    Time: 3:41p
** Updated in $/devel/h3/Win95/dx/dd16
** Add GammaRamp support and initialization.
** 
** *****************  Version 125  *****************
** User: Andrew       Date: 8/18/98    Time: 10:21p
** Updated in $/devel/h3/Win95/dx/dd16
** Removed Hotspot from Intersect calculation and precaulcated on
** Save/Draw Cursor calls.
** 
** *****************  Version 124  *****************
** User: Andrew       Date: 8/16/98    Time: 6:48p
** Updated in $/devel/h3/Win95/dx/dd16
** Changes to support not Restoring or Saving Cursor if we are excluded.
** This is a issue with animated cursors.
** 
** *****************  Version 123  *****************
** User: Andrew       Date: 8/14/98    Time: 11:31p
** Updated in $/devel/h3/Win95/dx/dd16
** Change myBeginAccess and myEndAccess to not use BUSY flags but instead
** use Cursor BUSY flag
** 
** *****************  Version 122  *****************
** User: Ken          Date: 8/07/98    Time: 6:44p
** Updated in $/devel/h3/win95/dx/dd16
** should fix random agp hangs/crashes.   fixes jedi knight menu
** on agp board/driver
** 
** *****************  Version 121  *****************
** User: Miriam       Date: 7/31/98    Time: 10:34p
** Updated in $/devel/h3/Win95/dx/dd16
** D3D & Glide cooperation. Allow each API to set/reset state to indicate
** that  the HW state has been changed.
** 
** *****************  Version 120  *****************
** User: Ken          Date: 7/31/98    Time: 9:27p
** Updated in $/devel/h3/win95/dx/dd16
** reduced agp memory allocation from 16 to 4mb, changed init to not do
** the agp allocation if the registry entry "afifo" is set to 0 (forcing
** no command fifo)
** 
** *****************  Version 119  *****************
** User: Ken          Date: 7/30/98    Time: 2:02p
** Updated in $/devel/h3/win95/dx/dd16
** determine memory type at boot (FirstEnable in Enable1) and set a shared
** memory flag in ddMiscFlags to indicate SDRAM or SGRAM
** 
** *****************  Version 118  *****************
** User: Miriam       Date: 7/27/98    Time: 1:01p
** Updated in $/devel/h3/Win95/dx/dd16
** If there is no registry entry, default to AGP command fifo enabled.
** 
** *****************  Version 117  *****************
** User: Ken          Date: 7/24/98    Time: 10:38p
** Updated in $/devel/h3/win95/dx/dd16
** changes to allow 2d driver to run properly synchronized with an AGP
** command fifo (although video memory fifo is still used when the desktop
** has the focus, e.g., a fullscreen 3d app isn't in the foreground)
** 
** *****************  Version 116  *****************
** User: Miriam       Date: 7/24/98    Time: 7:16p
** Updated in $/devel/h3/Win95/dx/dd16
** AGP command fifo only enabled for D3D.
** 
** *****************  Version 115  *****************
** User: Ken          Date: 7/23/98    Time: 4:28p
** Updated in $/devel/h3/win95/dx/dd16
** removed int 3's
** 
** *****************  Version 113  *****************
** User: Andrew       Date: 7/20/98    Time: 4:42p
** Updated in $/devel/h3/Win95/dx/dd16
** Changed the mem_config table to support 720x400. 720x480, 720x576, and
** 400x300
** 
** *****************  Version 112  *****************
** User: Ken          Date: 7/18/98    Time: 6:41p
** Updated in $/devel/h3/win95/dx/dd16
** added ability to use cmdfifo1 as the primary command fifo, #define
** PRIMARY_CMDFIFO at the top of inc\shared.h
** 
** *****************  Version 111  *****************
** User: Michael      Date: 7/14/98    Time: 8:39a
** Updated in $/devel/h3/Win95/dx/dd16
** MarkL (IGX) - There was an error in SetupOffscreenMemory where
** it assumed that the pitch was equal to the width*bpp.  This isn't
** always
** true due to the tile alignment requirement. Fixes  PRS 1740. 
** 
** *****************  Version 110  *****************
** User: Andrew       Date: 7/13/98    Time: 5:22p
** Updated in $/devel/h3/Win95/dx/dd16
** Modified to support passing a gamma table
** 
** *****************  Version 109  *****************
** User: Andrew       Date: 7/11/98    Time: 8:31a
** Updated in $/devel/h3/Win95/dx/dd16
** Added gamma correction
** 
** *****************  Version 108  *****************
** User: Hanson       Date: 7/11/98    Time: 7:51a
** Updated in $/devel/h3/Win95/dx/dd16
** Increased cmd fifo size on 16 Meg board at 16 bits to 1Meg.
** 
** *****************  Version 107  *****************
** User: Michael      Date: 7/07/98    Time: 3:07p
** Updated in $/devel/h3/Win95/dx/dd16
** Backout Andrews changes for version 106.
** 
** *****************  Version 106  *****************
** User: Andrew       Date: 7/03/98    Time: 10:23a
** Updated in $/devel/h3/Win95/dx/dd16
** Changed validate mode to always call GetRefreshRate and commented out
** DEBUG build non-enable failure
** 
** *****************  Version 105  *****************
** User: Edwin        Date: 7/02/98    Time: 11:23p
** Updated in $/devel/h3/Win95/dx/dd16
** Only call InitDeviceBitmapFilter() once when Windows boots,  fix 1733.
** 
** *****************  Version 104  *****************
** User: Ken          Date: 7/01/98    Time: 6:36p
** Updated in $/devel/h3/win95/dx/dd16
** added 16bpp low res modes using video overlay stretching
** 
** *****************  Version 103  *****************
** User: Andrew       Date: 7/01/98    Time: 10:21a
** Updated in $/devel/h3/Win95/dx/dd16
** Moved the location of FirstEnable to after InitCursor so that
** InitCursor could use this variable to see if this was the very first
** enable
** 
** *****************  Version 102  *****************
** User: Andrew       Date: 6/30/98    Time: 4:23p
** Updated in $/devel/h3/Win95/dx/dd16
** Added a call to RestoreCursor in ToForeground
** 
** *****************  Version 101  *****************
** User: Suninn       Date: 6/30/98    Time: 11:18a
** Updated in $/devel/h3/Win95/dx/dd16
** add ddResetOverlay flag for overfly whql test
** 
** *****************  Version 100  *****************
** User: Edwin        Date: 6/29/98    Time: 2:29p
** Updated in $/devel/h3/Win95/dx/dd16
** Initialize fpRepaintScreen in GLOBALDATA, fix defect 1882.
** 
** *****************  Version 99  *****************
** User: Michael      Date: 6/26/98    Time: 9:48a
** Updated in $/devel/h3/Win95/dx/dd16
** FredW (IGX) - The Dib Enable routine sets up the first three entries of
** the color palette in 16/24/32 bpp modes to tell the DIB Engine what the
** pixel layout is.  It *also* sets the first three palette entries in
** palettized mode. We are storing the palette that needs to be restored
** there, and the first three colors are overwritten. The first is not a
** loss, because its black anyway, but the next two colors are used (the
** green is used in the Internet icon).  Fixes 1766.
** 
** *****************  Version 98  *****************
** User: Andrew       Date: 6/24/98    Time: 1:39p
** Updated in $/devel/h3/Win95/dx/dd16
** Accidently drop changes for Rev 95 and Rev 96.  Got Rev 96 merged my
** changes and checked in
** 
** *****************  Version 97  *****************
** User: Andrew       Date: 6/24/98    Time: 9:28a
** Updated in $/devel/h3/Win95/dx/dd16
** Changed the MemConfig table for new modes 1792x1344 and 1856x1392.
** Commented back in return False if ModeNumber==-1.
** 
** *****************  Version 96  *****************
** User: Suninn       Date: 6/18/98    Time: 4:17p
** Updated in $/devel/h3/Win95/dx/dd16
** one line change fix off by one page with z and color in retail build
** 
** *****************  Version 95  *****************
** User: Russ         Date: 6/17/98    Time: 7:48p
** Updated in $/devel/h3/Win95/dx/dd16
** For DEBUG builds, added function prototype and pragma intrinsic for inp
** 
** *****************  Version 94  *****************
** User: Russ         Date: 6/17/98    Time: 9:15a
** Updated in $/devel/h3/Win95/dx/dd16
** Initialize VSYNC_POLARITY bit of ddMiscFlags in Enable1 after the
** HWSetMode call
**
** *****************  Version 93  *****************
** User: Ken          Date: 6/16/98    Time: 5:49p
** Updated in $/devel/h3/win95/dx/dd16
** made registry tweak a bit more bullet-proof by disallowing changes to
** the draminit0 strap bits
**
** *****************  Version 92  *****************
** User: Miriam       Date: 6/16/98    Time: 5:37p
** Updated in $/devel/h3/Win95/dx/dd16
** tune command fifo size.
**
** *****************  Version 91  *****************
** User: Michael      Date: 6/13/98    Time: 7:05a
** Updated in $/devel/h3/Win95/dx/dd16
** FredW (IGX) changes to remove duplicated code (setupPalette() is now
** used for palette reprogramming in all cases) in ToForeground().
**
** *****************  Version 90  *****************
** User: Ken          Date: 6/11/98    Time: 6:52p
** Updated in $/devel/h3/win95/dx/dd16
** added registry clock / timing tweaking
**
** *****************  Version 89  *****************
** User: Suninn       Date: 6/11/98    Time: 5:47p
** Updated in $/devel/h3/Win95/dx/dd16
** overlay management
**
** *****************  Version 88  *****************
** User: Edwin        Date: 6/02/98    Time: 8:23p
** Updated in $/devel/h3/Win95/dx/dd16
** Fix defect 1854.  Do all MM house cleaning before setupOffscreenMemory.
**
** *****************  Version 87  *****************
** User: Ken          Date: 6/01/98    Time: 1:47p
** Updated in $/devel/h3/win95/dx/dd16
** added isSdram to globaldata, settable through control1() calls
** (probably should put this in the registry), changed d3d ddiClear to
** look at this to determine whether or not to issue a dithered clear (to
** force block writes on/off)
**
** *****************  Version 86  *****************
** User: Andrew       Date: 6/01/98    Time: 6:48a
** Updated in $/devel/h3/Win95/dx/dd16
** Changed Intersect to use LastCursorPosition
**
** *****************  Version 85  *****************
** User: Andrew       Date: 5/27/98    Time: 8:38a
** Updated in $/devel/h3/Win95/dx/dd16
** Added DIB engine cursor and removed deDriverReserved=0x42 and fixed
** bugs
**
** *****************  Version 84  *****************
** User: Suninn       Date: 5/22/98    Time: 6:00p
** Updated in $/devel/h3/Win95/dx/dd16
** primary surface data does not need to initialization anymore
**
** *****************  Version 83  *****************
** User: Russ         Date: 5/20/98    Time: 5:21p
** Updated in $/devel/h3/Win95/dx/dd16
** initialize _FF(DevNode) & _FF(ddPrimarySurfaceData) in Enable1
**
** *****************  Version 82  *****************
** User: Andrew       Date: 5/20/98    Time: 12:27p
** Updated in $/devel/h3/Win95/dx/dd16
** Move Cursor Exclude clear brefore Draw Cursor
**
** *****************  Version 81  *****************
** User: Andrew       Date: 5/19/98    Time: 5:30p
** Updated in $/devel/h3/Win95/dx/dd16
** Update to my begin/end access and new function DoIntersect
**
** *****************  Version 80  *****************
** User: Suninn       Date: 5/15/98    Time: 9:49a
** Updated in $/devel/h3/Win95/dx/dd16
** tile activation code
**
** *****************  Version 79  *****************
** User: Miriam       Date: 5/14/98    Time: 5:49p
** Updated in $/devel/h3/Win95/dx/dd16
** Increased performance with larger command fifo.
**
** *****************  Version 78  *****************
** User: Ken          Date: 5/14/98    Time: 11:09a
** Updated in $/devel/h3/win95/dx/dd16
** added perfnop to gdi16
**
** *****************  Version 77  *****************
** User: Andrew       Date: 5/13/98    Time: 11:20a
** Updated in $/devel/h3/Win95/dx/dd16
** Fixed 1600x1200x32 crash and changed #buffers for 1792,1800,1920 modes
**
** *****************  Version 76  *****************
** User: Suninn       Date: 5/11/98    Time: 5:24p
** Updated in $/devel/h3/Win95/dx/dd16
** fix aperture stride
**
** *****************  Version 75  *****************
** User: Agus         Date: 5/08/98    Time: 12:24p
** Updated in $/devel/h3/Win95/dx/dd16
** Added refresh & memclk init
**
** *****************  Version 74  *****************
** User: Andrew       Date: 5/07/98    Time: 11:26a
** Updated in $/devel/h3/Win95/dx/dd16
** Added some memory layout information
**
** *****************  Version 73  *****************
** User: Artg         Date: 5/01/98    Time: 4:25p
** Updated in $/devel/h3/Win95/dx/dd16
** sw cursor stuff
**
** *****************  Version 72  *****************
** User: Andrew       Date: 5/01/98    Time: 10:47a
** Updated in $/devel/h3/Win95/dx/dd16
** Added OverRide flag to FindClosest Refresh Rate
**
** *****************  Version 71  *****************
** User: Michael      Date: 4/28/98    Time: 3:56p
** Updated in $/devel/h3/Win95/dx/dd16
** change Msg() to include a new parameter that will allow for selective
** debug message output.
**
** *****************  Version 70  *****************
** User: Miriam       Date: 4/28/98    Time: 12:01p
** Updated in $/devel/h3/Win95/dx/dd16
** Turn tiles on by default.
**
** *****************  Version 69  *****************
** User: Suninn       Date: 4/27/98    Time: 7:40p
** Updated in $/devel/h3/Win95/dx/dd16
** change heap calculation into array
**
** *****************  Version 68  *****************
** User: Artg         Date: 4/27/98    Time: 11:28a
** Updated in $/devel/h3/Win95/dx/dd16
** forgot to cleanup debug routine
**
** *****************  Version 67  *****************
** User: Artg         Date: 4/27/98    Time: 10:51a
** Updated in $/devel/h3/Win95/dx/dd16
** added mybeginaccess and myendaccess.
** statically allocated memory (8k) for swcursor and exclusion
**     after hwcursormask space.
**
** *****************  Version 66  *****************
** User: Andrew       Date: 4/22/98    Time: 2:43p
** Updated in $/devel/h3/Win95/dx/dd16
** Updated to support new mode table. Added some functions to findmode.
** Added case to Control1 to support Query Mode table operations
**
** *****************  Version 65  *****************
** User: Ken          Date: 4/21/98    Time: 12:01a
** Updated in $/devel/h3/win95/dx/dd16
** added agp workaround (set ar=1) for readback unreliability
**
** *****************  Version 64  *****************
** User: Michael      Date: 4/20/98    Time: 8:14a
** Updated in $/devel/h3/Win95/dx/dd16
** Add runtime variable "DomsgMM16" to enable/disable trace output of
** memory manager messages separate from normal trace debug messages.
**
** *****************  Version 63  *****************
** User: Dow          Date: 4/16/98    Time: 10:15p
** Updated in $/devel/h3/Win95/dx/dd16
** Glide/Windows co-op
**
** *****************  Version 62  *****************
** User: Ken          Date: 4/15/98    Time: 1:43p
** Updated in $/devel/h3/win95/dx/dd16
** fixed compiler internal failure..gotta love ms
**
** *****************  Version 61  *****************
** User: Ken          Date: 4/15/98    Time: 12:22p
** Updated in $/devel/h3/win95/dx/dd16
** fred'd 8bpp palette fix, reorg of enable1()
**
** *****************  Version 60  *****************
** User: Ken          Date: 4/14/98    Time: 11:17a
** Updated in $/devel/h3/win95/dx/dd16
** added mode12 virtualization
**
*/

#include <string.h>
#include <stdlib.h>
#include "header.h"
#include "valmode.inc"
#include "3dfx.h"
#include "memmgr16.h"

#include "modelist.h"
#include "qmodes.h"
extern int tvoutGetBootStatus();
extern void TvoutAllowPALandCRT(FxU32 State);
extern DWORD PLL2MHz(DWORD clock );

#define Not_VxD
#include "minivdd.h"

#ifdef H5
# define DDFXS32_DLLNAME "3dfx32vs.dll"
#else
# ifdef H4
#  define DDFXS32_DLLNAME "3dfx32v3.dll"
# endif // H4
#endif // H5

#ifdef INCSTBPERF
#include "..\build\stbperf.inc"
#endif

#include "hwcext.h"
#include "cursor.h"

#include <vmm.h>
#define MIDL_PASS     // suppress 32-bit only #pragma pack(push)


#pragma warning (disable: 4047 4704)
#include <configmg.h>
#pragma warning (default: 4047 4704)

#include "gramp.h"
#include "fxtvout.h"

// edgetools stuff Paul Magee 21 Jan 99
#define STB_EDGETOOLS  //for testing
#ifdef  STB_EDGETOOLS
#include "edgeesc.h"
#endif  // STB_EDGETOOLS

#include "tv.h"

/* 
  16 bit data seg. starts on word boundary, pad up for DriverData so it
  aligns on dword boundary.
*/

WORD hwAll=1;

#ifdef GBLDATA_IN_PDEV
PDEV FAR * PASCAL lpDriverPDevice;
#else
GLOBALDATA      DriverData = { 0 };
#endif

/* ------------------ !!! ADD NEW VARIABLES BELOW. !!! ------------------*/

DWORD HWColorTable[256];

HANDLE hCsimLib;
SstRegs * sst;
SstGRegs h3g;

// assume that fifocache0 will be shared with d3d/dd
FIFOCACHE     fifocache0;
FIFOCACHE     *lpfifocache0;
SstGRegs      *lph3g;
SstCRegs      *lph3agp;
CmdFifo       *lph3cmdfifo0;
SstIORegs     *lph3IORegs;
SstRegs       *lph3_3d;
DWORD         cmdStartAddress =  PHYS_CMDFIFO_BASE_ADDR;

int IS_CSIM = 0;
int AGP        = 0;
int MANUAL_BUMP = 0;
int FIRST = 1;
int FIRSTPRIME = 0;
int INVARREG = 0;

#ifdef CRASHTEST
FxU32 doCT = 0;
#endif // #ifdef CRASHTEST

extern DWORD dwDeviceHandle;
extern DISPLAYINFO DisplayInfo;
extern int nNumModes;

/***************************************************************************
 *
 * globals
 *
 ***************************************************************************/

GLOBALDATA      *lpDriverData;
//GLOBALDATA      SST1Data;
HMODULE         hModule;

HANDLE hLib16;

DWORD fb_addr;
DWORD sel;
DWORD base_ptr;
DWORD tex_ptr;
DWORD fb_ptr;

DWORD Palettized;               // Global palettized mode flag 1=Y, 0=N.

// for enabling hw features on the fly.
/*
    hwall hwxxx    !(hwall&hwxxx)
    0        0        1
    0        1        1
    1        0        1
    1        1        0     (don't do dibeng)
*/

WORD hwBlt=1; hwBitBltSS=1, hwBitBltPS=1, hwBitBltHS=1 ;
WORD hwStretchBlt=1; hwStretchBltSS=1; hwStretchBltHS=1; hwStretchBltPS=1;

WORD hwText=1, hwTextLPDX=1; hwTextNoLPDX=1;
WORD hwOutput =1;
WORD hwAltPolygon=0, hwWindPolygon=0;
WORD hwPolyline=1;
WORD hwRect=1;
WORD hwPolyScanline=1, hwScanline =1 ;
WORD hwDibBlt =0 ; hwDibToDevice=0;
int  PacketCount=0;
DWORD  LastPtr=0;

DWORD * buffer;

#define DAC_DPMS_BITS (SST_DAC_DPMS_ON_VSYNC | SST_DAC_FORCE_VSYNC | SST_DAC_DPMS_ON_HSYNC | SST_DAC_FORCE_HSYNC)
// These are defined in minivdd/devtable.h
#define DEFAULT_RESET_DDRMODE (0x131)
#define DEFAULT_DDRMODE (0x31)
#define H3_DRAMMODE_REG (0x10d)


#define LFB_BASE _FF(lfbBase)
/***************************************************************************
 *
 * internal functions.
 *
 ***************************************************************************/

DWORD GetRegInt(LPSTR valname, DWORD def);
int   di_FindMode(DWORD dwWidth, DWORD dwHeight, DWORD dwBPP, DWORD dwFlags);
UINT  GetFlatSel(void);
void FreeFlatSel(void);

DWORD PhysToLinear(DWORD PhysAddress, DWORD Size);
BOOL  DDCreateDriverObject(BOOL bReset);
static void CmdFifo0Init(FIFOCACHE * fifo0,
                                    DWORD fifoStart,
                                    DWORD size );

extern BOOL InitThunks( DWORD dwBase, DWORD dwSize);
extern BOOL initFifo( DWORD dwSize, DWORD dwBase);
extern void FAR DbgOut(char *);

extern void FAR PASCAL hook_int2f(void);
extern void FAR PASCAL unhook_int2f(void);
extern void InitCursor(void);
extern void InitDeviceBitmapFilter(void);
extern void InitDeviceBitmap(void);
extern void EnableDeviceBitmaps(void);
extern void DoAllHost(void);
extern int tvoutSetStdInternal(void);
extern int tvoutDisableInternal(void);
extern int TVOutVideoParameters(LPVIDEOPARAMETERS);
extern void TVOutSetStandard( LPTVSETSTANDARD lpQIN );
extern void TVOutRefreshMem( LPQIN lpQIN );
extern void ClearMem(DWORD, DWORD);
extern void bumpAgp(DWORD);
extern void FAR * di_AllocModeTable( int * nummodes );
extern void di_FillModeTable( MODEINFO FAR * info, int nummodes );
extern void di_SortModeTable( MODEINFO FAR * info, int nummodes );

#ifdef PERF_NEWMM
extern BOOL FAR InitCache();
extern void CacheDepopulate();
#endif

#ifdef SSB
extern void DiscardAllSSB(void);      // in ssb.c
extern BOOL saveScreenBitmapAllowed;  // in ssb.c
extern BOOL saveScreenBitmapDisabled; // in ssb.c
#endif

void DoMouseTrails(WORD wTrails);

LONG FAR PASCAL
hwcExt(DWORD *lpInput, DWORD *lpOutput) ;

LONG FAR PASCAL
hwcShareCPUType(DWORD *req, DWORD *res);

LONG FAR PASCAL
hwcSetAuxExclusiveMode(DWORD *req, DWORD *res);

LONG FAR PASCAL
hwcReleaseAuxExclusiveMode(DWORD *req, DWORD *res);

LONG FAR PASCAL
hwcGetAuxExclusiveMode(DWORD *req, DWORD *res);


void FAR PASCAL _loadds myBeginAccess(DIBENGINE FAR *pde, int left, int top, int right, int bottom, UINT flags);
void FAR PASCAL _loadds myEndAccess(DIBENGINE FAR *pde, UINT flags);
int DoIntersect (SHORT left, SHORT top, SHORT right, SHORT bottom);
void RestoreCursor(void);

#ifdef SLI_AA
void SwitchToHostCursor(void);
#endif

#ifdef RD_ABORT_ERROR
void Modify_SLI_Read(DWORD dwRequest);
#define MODIFY_SLI_READ(Request) {\
   if (_FF(dwSLIMode) != H3VDD_SLI_READ_NOT_IN_USE) \
      {\
      _FF(dwSLIMode) = Request;\
      Modify_SLI_Read(Request);\
      }\
   }
#else
#define MODIFY_SLI_READ(Request)
#endif

/***************************************************************************
 *
 * Enable   called by GDI to enable the device and set the video mode
 *
 ***************************************************************************/

WORD FirstEnable = 1;           // =1 means driver is loading for the first
                                // time -- Enable() will do its minivdd
                                // start-up protocol, needed only only per
                                // boot, FirstEnable will be =0 thereafter

FARPROC         fpRepaintScreen;


/*----------------------------------------------------------------------
Function name:  ResetHiresMode

Description:    Presently does nothing.
                
Information:    Not presently used!

Return:         VOID
----------------------------------------------------------------------*/
void _loadds ResetHiresMode()
{
    int himom = 1;
}


/*----------------------------------------------------------------------
Function name:  _InquireInfo

Description:    Register driver capabilities in pdevice.
                
Information:    

Return:         UINT    size returned from DIB_Enable or,
                        0 if DIB_Enable failed.
----------------------------------------------------------------------*/
UINT
_InquireInfo(LPVOID lpDevice,
         UINT style,
         DWORD dwResolutionX,
         DWORD dwResolutionY,
         DWORD dwBPP)
{
    GDIINFO FAR *pdp;
    UINT size;

    DPF(DBGLVL_NORMAL,"InquireInfo.");

    size = DIB_Enable(lpDevice, style, NULL, NULL, NULL);

    if (size == 0)
    return 0;

    pdp = (GDIINFO FAR *)lpDevice;

    pdp->dpCaps1 |= C1_DIBENGINE;           // we are a mini-driver
    pdp->dpCaps1 |= C1_COLORCURSOR;         // we do color cursors
    pdp->dpCaps1 |= C1_REINIT_ABLE;         // we can re-enable
    pdp->dpCaps1 |= C1_BYTE_PACKED;         // we handle BYTE packed fonts
    pdp->dpCaps1 |= C1_GLYPH_INDEX;
    pdp->dpCaps1 |= C1_GAMMA_RAMP;

    DPF(DBGLVL_NORMAL,"dpCaps1 = 0X%x",pdp->dpCaps1);

    //pdp->dpRaster|=RC_BITBLT;
    //pdp->dpRaster|=RC_PALETTE;
    //pdp->dpRaster|=RC_DIBTODEV;
    //pdp->dpRaster|=RC_BIGFONT;
    //pdp->dpRaster|=RC_STRETCHBLT;
    //pdp->dpRaster|=RC_STRETCHDIB;

    // S3's setting
    pdp->dpRaster=0xeeb9;
#ifdef SSB
        pdp->dpRaster |= RC_SAVEBITMAP;
#endif

    pdp->dpHorzRes   = (UINT)dwResolutionX; // screen width
    pdp->dpVertRes   = (UINT)dwResolutionY; // screen height
    pdp->dpBitsPixel = (UINT)dwBPP;         // screen bit depth
    pdp->dpDCManage = DC_IgnoreDFNP;        // Load a second Driver!!!!!

    //  pdp->dpLines=0x0023;
    //  pdp->dpLines|= LC_POLYSCANLINE;
    pdp->dpLines|= LC_POLYLINE;
    pdp->dpLines|= LC_STYLED;

    pdp->dpCurves=0x0089;

    pdp->dpPolygonals |= PC_ALTPOLYGON;
    pdp->dpPolygonals |= PC_RECTANGLE;
    pdp->dpPolygonals |= PC_SCANLINE;
    pdp->dpPolygonals |= PC_STYLED;
    pdp->dpPolygonals |= PC_INTERIORS;

    pdp->dpText=0x2004;
    pdp->dpClip=0x1;

    Palettized = 0;             // Initialize to Non-Palette Managed

    if (dwBPP < 8)
    {
    pdp->dpNumPens      = (1 << dwBPP); //# of pens driver realizes
    pdp->dpNumColors    = (1 << dwBPP); //# colors in color table
    }
    else if (dwBPP == 8)
    {
    pdp->dpNumPens      = 16;   //# of pens this driver realizes
    pdp->dpNumColors    = 20;   //# colors in color table
    pdp->dpNumPalReg    = 256;  //# palette registers
    pdp->dpPalReserved  = 20;   //# reserved palette entries
    pdp->dpColorRes     = 18;   //# palette res

    pdp->dpRaster |= RC_PALETTE;// mark as a palette device
    Palettized = 1;             // Indicate palette managed mode for DIBEng

    }
    else
    {
    pdp->dpNumPens   = -1; //# of pens this driver realizes
    pdp->dpNumColors = -1; //# colors in color table
    }

#ifdef GBLDATA_IN_PDEV
    // for future expansion of pdev
    // bump size for everything in PDEV after the DIBENGINE struct
    pdp->dpDEVICEsize += sizeof(PDEV) - sizeof(DIBENGINE);
#endif

    return size;
}


/*----------------------------------------------------------------------
Function name:  DibEnable

Description:    Call dib engine to enable, fill in BITMAPINFO. create
                and initialize dib pdevice.
                
Information:    

Return:         INT     FALSE if DIB_Enable fails,
                        TRUE  otherwise.
----------------------------------------------------------------------*/
int
DibEnable(LPVOID lpDevice,
     UINT style,
     int ModeNumber)
{
    DIBENGINE FAR *pde;
	static DWORD dwOldBpp = 0;
    int i;

    //
    // call the DIBENG and let it enable
    //
    if (!DIB_Enable(lpDevice, style, NULL, NULL, NULL))
    {
    DPF(DBGLVL_NORMAL,"DIBENG failed to enable");
    return FALSE;
    }

    //
    // now fill in a BITMAPINFO that describes our mode
    // and call the DIBENG function CreateDIBPDevice to
    // fill in our PDevice
    //
    _FF(bi).biSize        = sizeof(BITMAPINFOHEADER);
    _FF(bi).biPlanes      = 1;
    _FF(bi).biWidth       = ModeList[ModeNumber].dwWidth;
    _FF(bi).biHeight      = ModeList[ModeNumber].dwHeight;
    _FF(bi).biBitCount    = (BYTE)ModeList[ModeNumber].dwBPP;
    _FF(bi).biCompression =  0;

    if (8 == ModeList[ModeNumber].dwBPP)
         {
         // If 8-bpp, the color table must be left alone.
         // This fixes defect Track 1766 (PRS 4843).
       // Hmm...  I think this is incorrect.  Actually,
       // I believe these should get set back to all zeros.
       // This fixes the GUIMan flashing (PRS 4843).
       // I think this was also responsible for PRS 1922.
       // srogers - 10/5/99 The flashing that occurs on GUIMan is
       // because the mode is switching from 16 bpp mode to 
       // 8 bpp mode and the color table is updated some seconds
       // after the test starts.  So the dac is incorrect for awhile.
       // But if we always set the first 3 entries to 0, then a 
       // refresh rate change in an 8bpp mode would cause the first
       // 3 palette entries to be 0.  We would see the green part of 
       // the recycle bin show up as black (PRS 7993). So to solve
       // this we are adding a static varialbe, dwOldBpp, to save the
       // old bpp.  Then only if we are doing an X to 8 transition,
	   // where X !=8, would we zero out the first 3 entries.
           if(dwOldBpp != 8)
		   {
             _FF(color_table[0]) = 0x00000000L;
             _FF(color_table[1]) = 0x00800000L;
             _FF(color_table[2]) = 0x00008000L;
             _FF(color_table[3]) = 0x00808000L;
             _FF(color_table[4]) = 0x00000080L;
             _FF(color_table[5]) = 0x00800080L;
             _FF(color_table[6]) = 0x00008080L;
             _FF(color_table[7]) = 0x01C0C0C0L;
             _FF(color_table[8]) = 0x81C0DCC0L;
             _FF(color_table[9]) = 0x81A6CAF0L;
             for (i=10; i< 246; i++)
                _FF(color_table[i]) = 0x80000000L;
             _FF(color_table[246]) = 0x81FFFBF0L;
             _FF(color_table[247]) = 0x81A0A0A4L;
             _FF(color_table[248]) = 0x01808080L;
             _FF(color_table[249]) = 0x00FF0000L;
             _FF(color_table[250]) = 0x0100FF00L;
             _FF(color_table[251]) = 0x01FFFF00L;
             _FF(color_table[252]) = 0x000000FFL;
             _FF(color_table[253]) = 0x00FF00FFL;
             _FF(color_table[254]) = 0x0100FFFFL;
             _FF(color_table[255]) = 0x01FFFFFFL;
   		   }
         }
      else if (16 == ModeList[ModeNumber].dwBPP)
         {
         _FF(color_table[0]) = 0x0000F800L;
         _FF(color_table[1]) = 0x000007E0L;
         _FF(color_table[2]) = 0x0000001FL;
         }
      else if (24 == ModeList[ModeNumber].dwBPP)
         {
         _FF(color_table[0]) = 0x00FF0000L;
         _FF(color_table[1]) = 0x0000FF00L;
         _FF(color_table[2]) = 0x000000FFL;
         }
      else
         {
         _FF(color_table[0]) = 0x00FF0000L;
         _FF(color_table[1]) = 0x0000FF00L;
         _FF(color_table[2]) = 0x000000FFL;
         }

	dwOldBpp =  ModeList[ModeNumber].dwBPP;

    CreateDIBPDevice(&_FF(bi), lpDevice, NULL,
           MINIDRIVER|VRAM);
/*
  {
  __asm int 3
  DPF(DBGLVL_NORMAL,"CreateDIBPDevice failed");
  return FALSE;
  }
  */
    //
    //  set a few things in the DIBENGINE structure that CreateDIBPDevice
    //  did not do.
    //
    pde = (DIBENGINE FAR *)lpDevice;

    pde->deBitsSelector = GetFlatSel();
    pde->deBitsOffset = _FF(ScreenAddress);

    if (_FF(ddPrimaryInTile))
    {
      pde->deDeltaScan = _FF(ddTilePitch);
    }
    else
    {
      pde->deDeltaScan = _FF(pitch);
    }

    (DWORD)pde->deBeginAccess = (DWORD)BeginAccess;
    (DWORD)pde->deEndAccess = (DWORD)EndAccess;

    if (ModeList[ModeNumber].dwBPP == 8)
    pde->deFlags |= PALETTIZED;
    else
    pde->deFlags &= ~PALETTIZED;

    if (ModeList[ModeNumber].dwBPP == 16)
    pde->deFlags |= FIVE6FIVE;
    else
    pde->deFlags &= ~FIVE6FIVE;

    if (_FF(ScreenAddress) == 0)
    pde->deFlags |= BANKEDVRAM;
    else
    pde->deFlags &= ~BANKEDVRAM;

    if ((0x10000 % pde->deDeltaScan) != 0 && (pde->deFlags & BANKEDVRAM))
    pde->deFlags |= BANKEDSCAN;
    else
    pde->deFlags &= ~BANKEDSCAN;

    return TRUE;
}

#ifndef NOLOWRESFIX
FxU32 lowreshack = 0;
FxU32 lowresheight;
#endif // #ifndef NOLOWRESFIX


/*----------------------------------------------------------------------
Function name:  GetRegStrToInt

Description:    Reads a string key from the registry and returns
                the value converted to an integer.  Ignores any
                characters that aren't decimal numbers ('0' - '9').
Information:    

Return:         FxBOOL  FXTRUE  if successfully read from registry,
                        FXFALSE otherwise.
----------------------------------------------------------------------*/
char str[256];

FxBool
GetRegStrToInt(DWORD dwDevNodeHandle,
               char *lpStr,
               FxI32 *lpValue)
{
    DWORD length = sizeof(str);

    *lpValue = 0;

    if (CM_Read_Registry_Value(DisplayInfo.diDevNodeHandle,
                               "DEFAULT",
                               lpStr,
                               REG_SZ,
                               (LPBYTE)&str[0],
                               &length,
                               CM_REGISTRY_SOFTWARE) != CR_SUCCESS)
    {
        return FXFALSE;
    }

	*lpValue = atoi ( (const char *)&str );
        
    return FXTRUE;
}


/*----------------------------------------------------------------------
Function name:  GetBinaryDword

Description:    Reads a binary or DWORD value from the registry.

Information:    

Return:         FxBOOL  FXTRUE  if successfully read from registry,
                        FXFALSE otherwise.
----------------------------------------------------------------------*/
FxBool
GetBinaryDword(DWORD dwDevNodeHandle,
               char *lpStr,
               DWORD *lpValue)
{
    FxU32 bufSize = sizeof(DWORD);

    // Try first for Binary Value
    if (CM_Read_Registry_Value(DisplayInfo.diDevNodeHandle,
                               "DEFAULT",
                               lpStr,
                               REG_BINARY,
                               (LPBYTE)lpValue,
                               &bufSize,
                               CM_REGISTRY_SOFTWARE) == CR_SUCCESS)
    {
        return FXTRUE;
    }

    // Try next for Dword Value
    if (CM_Read_Registry_Value(DisplayInfo.diDevNodeHandle,
                               "DEFAULT",
                               lpStr,
                               REG_DWORD,
                               (LPBYTE)lpValue,
                               &bufSize,
                               CM_REGISTRY_SOFTWARE) == CR_SUCCESS)
    {
        return FXTRUE;
    }

    return FXFALSE;
}

/*----------------------------------------------------------------------
Function name:  confirmGrxClock

Description:    Determines if grxClock is valid from the value of
				confirmClk which is created when the 3dfx Tools Over Clock
				Page is used.
Information:    

Return:         FxBool	FXTRUE      Allow OverClocking
						FXFALSE		Don't OverClock
----------------------------------------------------------------------*/
FxBool
confirmGrxClock(DWORD dwDevNodeHandle)
{
	FxU32 confirmClk;
	char  DevNodeKey[MAX_VMM_REG_KEY_LEN];
	HKEY  hKey;

	if(GetBinaryDword(dwDevNodeHandle, "confirmClk", &confirmClk))
	{
		if(0 == confirmClk)
		{
			// This is the first reboot since the clock speed was changed
			// Set the value to 1 to indicate that it must now be confirmed by the user
			confirmClk = 1;
			if(CR_SUCCESS == CM_Write_Registry_Value(DisplayInfo.diDevNodeHandle, 
													 "DEFAULT", 
													 "confirmClk",
													 REG_DWORD, 
													 &confirmClk, 
													 sizeof(DWORD), 
													 CM_REGISTRY_SOFTWARE ))
			{
				//
				// Must flush the registry so that the new confirmClk value is saved even if 
				// the card is overclocked way above it's max value and the system hangs before
				// the registrys lazy flusher saves the changes
				//
				if (CR_SUCCESS == CM_Get_DevNode_Key(DisplayInfo.diDevNodeHandle,
											NULL,
											(PFARVOID)DevNodeKey,
											sizeof(DevNodeKey),
											CM_REGISTRY_SOFTWARE))
				{
					if(ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE, DevNodeKey, &hKey))
					{
						RegFlushKey(hKey);
						RegCloseKey(hKey);

						return FXTRUE;
					}

				}
			}
		}

		else if(1 == confirmClk)
		{
			// Set value to 2 to indicate that new clock setting hasn't been confirmed
			// and overclocking should not happen - 3dfx tools will delete the grxClock
			// and confirmClock values from the registry
			confirmClk = 2;
			if(CR_SUCCESS == CM_Write_Registry_Value(DisplayInfo.diDevNodeHandle, 
													 "DEFAULT", 
													 "confirmClk",
													 REG_DWORD, 
													 &confirmClk, 
													 sizeof(DWORD), 
													 CM_REGISTRY_SOFTWARE ))
			{
				return FXFALSE;
			}
		
		}
	}

	//
	// If the confirmClk value does not exist - allow overclocking
	//
	return FXTRUE;
}

#include "../minivdd/plltable.h"
#include "../minivdd/h4pll.h"
#include "../minivdd/h4oempll.h"

#ifdef AGP_CMDFIFO
FxU32 doAgp = 1;
#endif // #ifdef AGP_CMDFIFO

/*----------------------------------------------------------------------
Function name:  TweakOnModeSwitch

Description:    Modifies certain HW registers based on overridding
                values found (if any) in the registry on every mode switch
Information:
  Currently may "tweak" any of the following:
   vidPixelBufThold

Return:         VOID
----------------------------------------------------------------------*/
void TweakOnModeSwitch()
{
  FxU32 vidPixelBufThold;
#ifdef SLI_AA
  SstIORegs  *lpIORegs;
  int i;
#endif

  // SETDW and GET need the flat selector
  //
  GetFlatSel();

  vidPixelBufThold = 0x00010410; 
  if (GetBinaryDword(_FF(DevNode), "vidPixelBufThold", &vidPixelBufThold))
  {
    DPF(DBGLVL_ALL,"tweaking vidPixelBufTHold: new value = 0x%08lx\n", vidPixelBufThold);
#ifdef SLI_AA
    for (i=0; i<(int)_FF(dwNumUnits); i++)
        {
        lpIORegs = (SstIORegs *)_FF(regBase[i * HWINFO_SST_MAX_NUM_CHIPS + HWINFO_SST_IOREGS_INDEX]);        
        SETDW(lpIORegs->vidPixelBufThold, vidPixelBufThold);
        }
#else
    SETDW(lph3IORegs->vidPixelBufThold, vidPixelBufThold);
#endif
  }

   _FF(vidPixelBufThold) = vidPixelBufThold;

}

/*----------------------------------------------------------------------
Function name:  tweakFromRegistry

Description:    Modifies certain HW registers based on overridding
                values found (if any) in the registry.
Information:
  Currently may "tweak" any of the following:
  sgramMode, dramInit0, dramInit1, memSize, memClock, grxClock;

Return:         VOID
----------------------------------------------------------------------*/
void
tweakFromRegistry()
{
  FxU32 sgramMode, dramInit0, dramInit1, memSize, memClock, grxClock, tmuGbeInit;
  FxU32 cbValue = sizeof(DWORD);
  FxU32 physicalMemSizeInMB;
  FxU32 old_dramInit0;
#ifdef SLI_AA
  SstIORegs  *lpIORegs;
  int i;
#endif

  // SETDW and GET need the flat selector
  //
  GetFlatSel();

  if (GetBinaryDword(_FF(DevNode), "sgramMode", &sgramMode))
  {
    DPF(DBGLVL_ALL,"tweamMode: new value = 0x%08lx\n", sgramMode);
#ifdef SLI_AA
    for (i=0; i<(int)_FF(dwNumUnits); i++)
        {
        lpIORegs = (SstIORegs *)_FF(regBase[i * HWINFO_SST_MAX_NUM_CHIPS + HWINFO_SST_IOREGS_INDEX]);        
        SETDW(lpIORegs->dramData, sgramMode);
        SETDW(lpIORegs->dramCommand, 0x10dL);
        }
#else
    SETDW(lph3IORegs->dramData, sgramMode);
    SETDW(lph3IORegs->dramCommand, 0x10dL);
#endif
  }

  if (GetBinaryDword(_FF(DevNode), "dramInit0", &dramInit0))
  {
    old_dramInit0 = GET(lph3IORegs->dramInit0);

    DPF(DBGLVL_ALL,"tweaking dramInit0: old=0x%08lx, ", old_dramInit0);

    // preserve strap bits from the current value of dramInit0 into
    // the new dramInit0

    if (IS_NAPALM)
    {
      dramInit0 &= ~(SST_SGRAM_NUM_CHIPSETS | SST_H5_SGRAM_TYPE);
      old_dramInit0 &= SST_SGRAM_NUM_CHIPSETS | SST_H5_SGRAM_TYPE;
    }
    else
    {
      dramInit0 &= ~(SST_SGRAM_NUM_CHIPSETS | SST_H4_SGRAM_TYPE);
      old_dramInit0 &= SST_SGRAM_NUM_CHIPSETS | SST_H4_SGRAM_TYPE;
    }

    dramInit0 = dramInit0 | old_dramInit0;
    DPF(DBGLVL_ALL, "new=0x%08lx\n", dramInit0);

#ifdef SLI_AA
    for (i=0; i<(int)_FF(dwNumUnits); i++)
        {
        lpIORegs = (SstIORegs *)_FF(regBase[i * HWINFO_SST_MAX_NUM_CHIPS + HWINFO_SST_IOREGS_INDEX]);        
        SETDW(lpIORegs->dramInit0, dramInit0);
        }
#else
    SETDW(lph3IORegs->dramInit0, dramInit0);
#endif
  }

  if (GetBinaryDword(_FF(DevNode), "dramInit1", &dramInit1))
  {
    DPF(DBGLVL_ALL,"tweaking dramInit1: old=0x%08lx, new=0x%08lx\n",
          GET(lph3IORegs->dramInit1), dramInit1);

#ifdef SLI_AA
    for (i=0; i<(int)_FF(dwNumUnits); i++)
        {
        lpIORegs = (SstIORegs *)_FF(regBase[i * HWINFO_SST_MAX_NUM_CHIPS + HWINFO_SST_IOREGS_INDEX]);        
        SETDW(lpIORegs->dramInit1, dramInit1);
        }
#else
    SETDW(lph3IORegs->dramInit1, dramInit1);
#endif
  }

  if (GetBinaryDword(_FF(DevNode), "memSize", &memSize))
  {
    physicalMemSizeInMB = _FF(TotalVRAM) / (1024L * 1024L);
    if (memSize <= physicalMemSizeInMB)
    {
      DPF(DBGLVL_ALL,"tweaking memSize: old=%ldMB, new=%ldMB\n",
            physicalMemSizeInMB, memSize);

            _FF(TotalVRAM) = memSize * 1024L * 1024L;
    }
    else
    {
              DPF(DBGLVL_ALL,
            "requested memSize (%dMB) greater than physical mem (%dMB): "
            " no change\n", memSize, physicalMemSizeInMB);
    }
  }

  if (GetRegStrToInt(_FF(DevNode), "memClock", (FxI32 *)&memClock))
  {
    if ((memClock >= MIN_PLL_FREQ) && (memClock <= MAX_PLL_FREQ))
    {
      DPF(DBGLVL_ALL,
      "tweaking memClock: old (PLL)=0x%08lx, new (MHz)=0x%08lx\n",
      GET(lph3IORegs->pllCtrl2), memClock);

            SETDW(lph3IORegs->pllCtrl2, pllTable[memClock]);

            dramInit1 = GET(lph3IORegs->dramInit1);
            dramInit1 &= ~SST_SGRAM_USE_INV_SAMPLE;

            //
            // Look for a "new" style (Hank) dramInit1 timing.
            // If it doesn't match, assume it's an older style timing
            // that doesn't need bit 14 adjustment, and don't touch dramInit1.
            // If it does match, then assume that it needs bit 14 adjustment,
            // and change dramInit1 so that all slow/fast to fast/slow
            // transitions are correctly handled
            //
      if ((dramInit1 & 0xFFFFFFL) == 0x238031L)
      {
              // "new" style, adjust bit 14 and write dramInit1
              //
              if (memClock <= 105)
                        dramInit1 |= SST_SGRAM_USE_INV_SAMPLE;

            SETDW(lph3IORegs->dramInit1, dramInit1);
      }
    }
    else
    {
              DPF(DBGLVL_ALL,"memClock Mhz value out of range: %d, ignoring\n",
            memClock);
              DPF(DBGLVL_ALL,"(valid values are from %d to %d, inclusive)\n",
            MIN_PLL_FREQ, MAX_PLL_FREQ);
    }
  }
  
  /* Save of the default clock rate */
  
  _FF(dwDefaultClock) = GET(lph3IORegs->pllCtrl1);
  
  if (GetRegStrToInt(_FF(DevNode), "grxClock", (FxI32 *)&grxClock))
  {
	  // 
	  // SJACKSON 13/8/99
	  // Don't over clock if confirmClk registry value is 1
	  // i.e. changes to clock frequency haven't been confirmed by the user
	  //
	  if(confirmGrxClock(_FF(DevNode)))
	  {
	    if ((grxClock >= MIN_PLL_FREQ) && (grxClock <= MAX_H4_PLL_FREQ))
      {
        DPF(DBGLVL_ALL,
          "tweaking grxClock: old (PLL)=0x%08lx, new (MHz)=0x%08lx\n",
        GET(lph3IORegs->pllCtrl1), grxClock);

#ifdef SLI_AA
        for (i=0; i<(int)_FF(dwNumUnits); i++)
        {
          lpIORegs = (SstIORegs *)_FF(regBase[i * HWINFO_SST_MAX_NUM_CHIPS + HWINFO_SST_IOREGS_INDEX]);        
          SETDW(lpIORegs->pllCtrl1, h4pllTable[grxClock]);

          if(IS_DAYTONA)
          {
            // Check for DDR memory
            tmuGbeInit = GET(lpIORegs->tmuGbeInit);
            tmuGbeInit &= SST_MCTL_MEMORY_CONFIG;
            if ((tmuGbeInit >= SST_MCTL_DDR_64x256) && (tmuGbeInit <= SST_MCTL_DDR_32x1024))
            {
              // Reset the memory.
              SETDW(lpIORegs->dramData, DEFAULT_RESET_DDRMODE);
              SETDW(lpIORegs->dramCommand, H3_DRAMMODE_REG); 
              SETDW(lpIORegs->dramData, DEFAULT_DDRMODE);
              SETDW(lpIORegs->dramCommand, H3_DRAMMODE_REG); 
            }            
          }
        }
#else
		  SETDW(lph3IORegs->pllCtrl1, h4pllTable[grxClock]);

      if(IS_DAYTONA)
      {
        // Check for DDR memory
        tmuGbeInit = GET(lph3IORegs->tmuGbeInit);
        tmuGbeInit &= SST_MCTL_MEMORY_CONFIG;
        if ((tmuGbeInit >= SST_MCTL_DDR_64x256) && (tmuGbeInit <= SST_MCTL_DDR_32x1024))
        {
          // Reset the memory.
          SETDW(lph3IORegs->dramData, DEFAULT_RESET_DDRMODE);
          SETDW(lph3IORegs->dramCommand, H3_DRAMMODE_REG); 
          SETDW(lph3IORegs->dramData, DEFAULT_DDRMODE);
          SETDW(lph3IORegs->dramCommand, H3_DRAMMODE_REG); 
        }            
      }
#endif
		}
		else
		{
		  DPF(DBGLVL_ALL,"grxClock Mhz value out of range: %d, ignoring\n",
		    grxClock);
		  DPF(DBGLVL_ALL,"(valid values are from %d to %d, inclusive)\n",
		    MIN_PLL_FREQ, MAX_H4_PLL_FREQ);
		}
	  }
  }
  
#ifdef STEREO
{
  	#define REG_D3D_STEREO 	"D3D" 
  	char tempStr[256];  
   	DWORD length = sizeof(tempStr); 

	extern int atoi( const char *string );
		
	_FF(ddStereoWrapperLoaded)  = 0;
	_FF(ddStereoHeapFactor) 	= 0;

 	if(CM_Read_Registry_Value(DisplayInfo.diDevNodeHandle, REG_D3D_STEREO, "SSTH3_STEREO", REG_SZ,  
 	   					(LPBYTE)&tempStr[0], &length, CM_REGISTRY_SOFTWARE) == CR_SUCCESS)
   	{
       	_FF(ddStereoWrapperLoaded) =  atoi(tempStr);
    }        
	else
	{
		_FF(ddStereoWrapperLoaded)  = 0;
		_FF(ddStereoHeapFactor) 	= 0;
	}
}	
#endif



#ifdef AGP_CMDFIFO
  if ((DWORD)IS_AGP_READY == (_FF(AGPCaps) & IS_AGP_READY)) {
    FxI32 iEnableAGPCF;    
    if (GetRegStrToInt(_FF(DevNode), "SSTH3_AGP_XRATE", &iEnableAGPCF) == FXFALSE) {
	  _FF(enableAGPCF) = 1;
	} else { /* We did succeed in getting the registry key */
	  if ( iEnableAGPCF == -1 ) { /* If 3dfx Tools sets this to -1, then disable */
	    _FF(enableAGPCF) = 0;
	  } else { /* Else we enable it */
	    _FF(enableAGPCF) = 1;
	  }
	}
  } else {
    _FF(enableAGPCF) = 0;
  }

  // Only do AGP transfers on cards that support it
  // IS_AGP_READY_NAPALM Subsys ID table is stored in h3g.h
  if (!(IS_NAPALM) || !(IS_AGP_READY_NAPALM))
  {
    _FF(enableAGPCF) = 0;
  }

  // if the agp command fifo is turned off, don't try to allocate
  // AGP memory from the system at all (agp support may be busted or not
  // properly installed on this system)
  if (_FF(enableAGPCF) == 0)
    doAgp = 0;
#endif

}


/*----------------------------------------------------------------------
Function name:  setupVgaMode12h

Description:    Sets up for VGA mode 12h.  Decides whether to support
                mode 12h in a window.  (If we don't do it, we get
                more memory for 3d apps.)
Information:

Return:         INT     value read out of registry to determine if
                        we want to support mode 12h in a window.
----------------------------------------------------------------------*/
void GetBinary(DWORD dwDevNodeHandle, WORD FAR * lpValue,
               char FAR * lpStr, WORD nDefault);

int
setupVgaMode12h(DWORD dwDeviceHandle)
{
    WORD doVgaMode12h;
    long mode12Argument;

    // decide whether or not to do VGA mode 12h in a window
    // (if we don't do it, we get more memory for 3d apps...)
    // if the registry key isny key isn't present, use the default value
    //
    GetBinary(DisplayInfo.diDevNodeHandle, &doVgaMode12h,
              "vgamode12", (WORD) _FF(doVgaMode12h));

    if (doVgaMode12h)
        mode12Argument = 0;             // allow mode 12h in a window
    else
        mode12Argument = -1;            // don't allow mode 12h in a window
        
    VDDCall(VDD_DRIVER_REGISTER,
            dwDeviceHandle,
            0x200000L,    /* XXX FIXME XXX nbytes used by the screen */
            mode12Argument,             // do mode12 in a window?
            (LPVOID)ResetHiresMode);

    return doVgaMode12h;
}


/*----------------------------------------------------------------------
Function name:  setupPalette

Description:    Program the HW palette.

Information:

Return:         VOID
----------------------------------------------------------------------*/
void
setupPalette()
{
    DWORD *p = &_FF(color_table[0]);   /* Point to screen dib palette */
    int i;

      if (8 == _FF(bpp))
      {

      /* Convert to something we can send to hardware */
      for ( i = 0; i < 256; ++i )
        {
        char __far *src = (char __far *)(p + i);
             char __far *dst = (char __far *)(HWColorTable + i);

             /* Flip bytes around */
             dst[0] = src[2];
             dst[1] = src[1];
             dst[2] = src[0];
             dst[3] = src[3];
        }
      HWSetPalette(0, 256, HWColorTable);
      }

    /* If we're in high color modes, set up the gamma ramp
     * Doesn't hurt in low color modes either
     */
    set_gammaramp(0);
}

/*----------------------------------------------------------------------
Function name:  DisplayMemoryLayout

Description:    Dump memory layout to the debug terminal.

Information:    Only used for debugging.

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
#ifdef DEBUG
int DisplayMemoryLayout(void)
{
   DPF(DBGLVL_ALL,"VGA Start %lx Size %lx", _FF(vgaStart), _FF(vgaSize));
   DPF(DBGLVL_ALL,"FIFO Start %lx Size %lx", _FF(fifoStart), _FF(fifoSize));
   DPF(DBGLVL_ALL,"Font Start %lx Size %lx", _FF(FontCacheAddr), _FF(FontCacheSize));
   DPF(DBGLVL_ALL,"Cursor Start %lx Size %lx", _FF(cursorStart), _FF(cursorSize));
   DPF(DBGLVL_ALL,"SW Cursor Start %lx Size %lx", _FF(SWcursorAndStart), _FF(SWcursorSize));
   DPF(DBGLVL_ALL,"SW Cursor Start %lx Size %lx", _FF(SWcursorXorStart), _FF(SWcursorSize));
   DPF(DBGLVL_ALL,"SW Cursor Start %lx Size %lx", _FF(SWcursorSrcStart), _FF(SWcursorSize));
   DPF(DBGLVL_ALL,"SW Cursor Exclusion %lx Size %lx", _FF(SWcursorExclusionStart), _FF(SWcursorSize));
   DPF(DBGLVL_ALL,"stretchBlt Start %lx Size %lx", _FF(stretchBltStart), _FF(stretchBltSize));
   DPF(DBGLVL_ALL,"ddLinearHeapSize %lx Size %lx", _FF(ddLinearHeapStart), _FF(ddLinearHeapSize));
   DPF(DBGLVL_ALL,"gdiDesktopStart %lx Size %lx", _FF(gdiDesktopStart), _FF(gdiDesktopSize));
   DPF(DBGLVL_ALL,"ddTiledHeap Start %lx Size %lx", _FF(ddTiledHeapStart), _FF(ddTiledHeapSize));
   return 0;
}
#endif


/*----------------------------------------------------------------------
Function name:  setupOffscreenMemory

Description:    Allocates offscreen memory as shown:

  --------------------------  
          VGA memory
  --------------------------  
          Command FIFO
  --------------------------  
          Hardware cursor
  --------------------------  
          Software cursor
  --------------------------  
          Stretch BLT
  --------------------------  
          Linear heap
  --------------------------  
          Tiled heap
  --------------------------- 
          Desktop/Primary
  ---------------------------

  Banshee and Avenger use tiled memory only in 16bpp
  modes.  Napalm uses tiled memory in 16/32bpp modes.
  For other modes, memory is setup as entirely linear.
  When using tiled memory, the tile mark is initially
  set to point at the primary.  This allows GDI to use
  the tiled heap as linear memory for bitmap caching.  
  When DirectX starts, the tile mark moves to the start
  of the tiled heap, and bitmap caching is disabled.
                                                            
----------------------------------------------------------------------*/

void
setupOffscreenMemory(int modeNumber, int doVgaMode12h)
{
  DWORD dwResolutionX, dwResolutionY, dwBPP, screenSize;

  // Get resolution from ModeList table.

  dwResolutionX = (DWORD)ModeList[modeNumber].dwWidth;
  dwResolutionY = (DWORD)ModeList[modeNumber].dwHeight;
  dwBPP = (DWORD) ModeList[modeNumber].dwBPP;

  // Compute tiled width and height, based on resolution.

  _FF(ddTileStride) = ((dwResolutionX * (dwBPP >> 3)) + SST_TILE_WIDTH  - 1) >> SST_TILE_WIDTH_BITS;
  _FF(ddTileHeight) =  (dwResolutionY                 + SST_TILE_HEIGHT - 1) >> SST_TILE_HEIGHT_BITS;

  // Ensure correct alignment of buffers, even when SLI is enabled.

  _FF(ddTileHeight) = (_FF(ddTileHeight) + 1) & 0xFFFFFFFE; // must be multiple of 2
  _FF(ddTileStride) = (_FF(ddTileStride) + 1) & 0xFFFFFFFE; // must be multiple of 2

  // Make tile height an even multiple of the number of chips.

#ifdef SLI_AA
  if (_FF(dwNumUnits) == 4)
  {
    _FF(ddTileHeight) = (_FF(ddTileHeight) + 3) & 0xFFFFFFFC; // must be multiple of 4
  }
#endif

  // Compute screen size.

  screenSize = _FF(ddTileStride) * _FF(ddTileHeight) * SST_TILE_SIZE;

  // Even though we're only using 32K of mode 12h virtualization
  // memory, Banshee's VGA actually has a footprint of about 96K
  // If the value returned from GetVDDBank is 64K instead of 32K,
  // this size must be adjusted too!

  // Allocate VGA memory  

  _FF(vgaStart) = 0;
  if (doVgaMode12h)
      _FF(vgaSize) = 96L * 1024L;
  else
      _FF(vgaSize) = 0;

  // Allocate command fifo and compute size (4K aligned)

  _FF(fifoStart) = ((_FF(vgaStart) + _FF(vgaSize)) + 0xfffL) & ~0xfffL;
  switch(_FF(TotalVRAM) >> 20)
  {
    case 64: _FF(fifoSize) = 0x100000L; break;  // 1Mb
    case 32: _FF(fifoSize) = 0x100000L; break;  // 1Mb
    case 16: _FF(fifoSize) = 0x100000L; break;  // 1Mb
    case  8: _FF(fifoSize) = 0x040000L; break;  // 256Kb
    case  4: _FF(fifoSize) = 0x010000L; break;  // 64Kb
    default: _FF(fifoSize) = 0x010000L; break;  // 64Kb
  }

  // Allocate hardware cursor (1K aligned)

  _FF(cursorStart) = ((_FF(fifoStart) + _FF(fifoSize)) + 0x3ffL) & ~0x3ffL;
  _FF(cursorSize)  = 0x400L;

  // Allocate software cursor (1K aligned)

  _FF(SWcursorAndStart) = ((_FF(cursorStart) + _FF(cursorSize)) + 0x3ffL) & ~0x3ffL;;
  _FF(SWcursorSize)  = SWCURSOR_SIZE;
  _FF(SWcursorXorStart) = (_FF(SWcursorAndStart) + _FF(SWcursorSize));
  _FF(SWcursorSrcStart) = (_FF(SWcursorXorStart) + _FF(SWcursorSize));
  _FF(SWcursorExclusionStart) = (_FF(SWcursorSrcStart) + _FF(SWcursorSize));

#ifdef SLI_AA
   _FF(HostcursorAndStart) = _FF(SWcursorAndStart) + _FF(lfbBase);
   _FF(HostcursorXorStart) = _FF(SWcursorXorStart) + _FF(lfbBase);
   _FF(HostcursorSrcStart) = _FF(SWcursorSrcStart) + _FF(lfbBase);
   _FF(HostcursorExclusionStart) = _FF(SWcursorExclusionStart) + _FF(lfbBase);
#endif

  // Allocate StretchBLT scratch (1K aligned)

  _FF(stretchBltStart) = ((_FF(SWcursorExclusionStart) + _FF(SWcursorSize)) + 0x3ffL) & ~0x3ffL;
  _FF(stretchBltSize)  = 0x2000L;

  // Linear heap starts after reserved area (4K aligned).

  _FF(ddLinearHeapStart) = _FF(stretchBltStart) + _FF(stretchBltSize);
  _FF(ddLinearHeapStart) = (_FF(ddLinearHeapStart) + 0xfffL) & ~0xfffL;

#ifdef LINEAR_ONLY
  _FF(ddPrimaryInTile) = FALSE;
#else
  if (IS_NAPALM)
  {
    _FF(ddPrimaryInTile) = ((dwBPP == 16) || (dwBPP == 32));
  }
  else
  {
    _FF(ddPrimaryInTile) = (dwBPP == 16);
  }
#endif

  // Compute number of buffers.

  _FF(ddNumColorBuff) = 0;
  _FF(ddSecondaryHeapSize) = 0;

  if _FF(ddPrimaryInTile)
  {
    DWORD minimumLinear;
    DWORD memoryAvailable;

    switch(_FF(TotalVRAM) >> 20)
    {
      // Minimum linear heap allocation.

      case 64: minimumLinear = 0x600000L; break;   // 6.0Mb
      case 32: minimumLinear = 0x300000L; break;   // 3.0Mb
      case 16: minimumLinear = 0x180000L; break;   // 1.5Mb
      default: minimumLinear = 0x180000L; break;   // 1.5Mb
    }

    // Compute number of primary buffers.

    memoryAvailable = _FF(TotalVRAM) - _FF(ddLinearHeapStart);
    if      (memoryAvailable >= (minimumLinear + (screenSize * 4))) _FF(ddNumColorBuff) = 3;
    else if (memoryAvailable >= (minimumLinear + (screenSize * 3))) _FF(ddNumColorBuff) = 2;
    else if (memoryAvailable >= (minimumLinear + (screenSize * 2))) _FF(ddNumColorBuff) = 1;

    // Compute size of secondary heap (all or nothing!).

    if ((IS_NAPALM) && (memoryAvailable >= (minimumLinear + (screenSize * 8))))
    {
      _FF(ddSecondaryHeapSize) = (4 * screenSize);
      _FF(ddSecondaryHeapSize) += _FF(ddTileStride) * SST_TILE_SIZE * _FF(dwNumUnits); // Align zbuffer (AA)
    }
  }

  // Allocate desktop, at end of memory.

  _FF(gdiDesktopSize)  = screenSize;
  _FF(gdiDesktopStart) = _FF(TotalVRAM) - _FF(gdiDesktopSize);

  // Allocate primary tiled heap, with room for alignment.

#ifdef STEREO
  if(_FF(ddStereoWrapperLoaded)) _FF(ddStereoHeapFactor) = 4;
  _FF(ddTiledHeapSize)  = ( _FF(ddNumColorBuff) + _FF(ddStereoHeapFactor) ) * screenSize;
#else
  _FF(ddTiledHeapSize)  = (_FF(ddNumColorBuff) ) * screenSize;
#endif

  if (_FF(ddNumColorBuff) > 1)
  {
    _FF(ddTiledHeapSize) += _FF(ddTileStride) * SST_TILE_SIZE * _FF(dwNumUnits); // Align zbuffer
  }

  _FF(ddTiledHeapStart) = _FF(gdiDesktopStart) - _FF(ddTiledHeapSize);

  // Allocate secondary tiled heap, with room for alignment.

  _FF(ddSecondaryHeapStart) = _FF(ddTiledHeapStart) - _FF(ddSecondaryHeapSize);

  // Allocate linear heap, with remaining memory.

  _FF(ddLinearHeapSize)  = _FF(ddSecondaryHeapStart) - _FF(ddLinearHeapStart); 

  // Set hardware flag if primary is tiled.

  if _FF(ddPrimaryInTile)
  {
    _FF(gdiDesktopStart) |= SSTG_IS_TILED;
  }

  DPF(DBGLVL_NORMAL,"%dx%dx%d, numBuffer = %d", (int)dwResolutionX, (int)dwResolutionY, (int)dwBPP, (int)_FF(ddNumColorBuff));

#ifdef DEBUG
    DisplayMemoryLayout();
#endif

}


/*----------------------------------------------------------------------
Function name:  reserve_commit

Description:    Initializes global variables and structures for
                AGP memory.
Information:

Return:         DWORD
----------------------------------------------------------------------*/
static DWORD
reserve_commit(AgpSrvc *srvc, DWORD nPages)
{
  srvc->service                = RESERVE;
  srvc->params.reserve.devNode = DisplayInfo.diDevNodeHandle;
  srvc->params.reserve.nPages  = nPages;

  VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle, H3VDD_AGP_SERVICE, 0, srvc);

  if (srvc->params.reserve.gartLinAddr == 0)
  {
    DPF(DBGLVL_NORMAL, "reserve failed\n");
    return FALSE;
  }

  srvc->service                   = COMMIT;
  srvc->params.commit.gartLinAddr = srvc->params.reserve.gartLinAddr;
  srvc->params.commit.pageOffset  = 0;
  srvc->params.commit.nPages      = nPages;
  
  VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle, H3VDD_AGP_SERVICE, 0, srvc);
  
  if (srvc->params.commit.retval == 0)
  {
    DPF(DBGLVL_NORMAL, "commit failed\n");
    return FALSE; 
  }

  DPF(DBGLVL_NORMAL, "committed: NP:%d LA:0x%08lx, GA:0x%08lx",
                     srvc->params.commit.nPages,
                     srvc->params.commit.gartLinAddr,
                     srvc->params.commit.gartPhysAddr);

  return TRUE;
}


/*----------------------------------------------------------------------
Function name:  setupAgpMemory

Description:    Initializes global variables and structures for
                AGP memory.
Information:

Return:         VOID
----------------------------------------------------------------------*/
#define nAgpPages     1024L     //   1K    pages == 4MB
#define nTexPages       32L     //   256*256*2 / 4096
#define nSyncPages       1L   //   really only need 1 word.

void setupAgpMemory()
{
  HWAGPFORCERATE HwAgpForceRate;
  AgpSrvc agpSrvc;
  DWORD dwForce;
  
  DPF(DBGLVL_NORMAL, "setupAgpMemory:");
  if (FALSE == reserve_commit( &agpSrvc, nAgpPages ))
  {
    _FF(agpMain.linAddr ) = 0;
    _FF(agpMain.physAddr) = 0;
    _FF(agpMain.sizeInB ) = 0;

    _FF(enableAGPCF) = 0;
  }
  else
  {
    _FF(agpMain.linAddr ) = agpSrvc.params.commit.gartLinAddr;
    _FF(agpMain.physAddr) = agpSrvc.params.commit.gartPhysAddr;
    _FF(agpMain.sizeInB ) = (nAgpPages * 4 * 1024L);
  }
 
  if (IS_NAPALM)
      {
      // if it does not exist default to 1x
      if (FXFALSE == GetRegStrToInt(_FF(DevNode),"SSTH3_AGP_XRATE", (FxI32 *)&dwForce))
         dwForce = 1;
   
      if (dwForce > 2)
         dwForce = 1;

      if (0 != dwForce)
         {
         DPF(DBGLVL_NORMAL,"Forcing %dx AGP Data Rate", dwForce);
         HwAgpForceRate.dwRate = dwForce;
         VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle, H3VDD_AGP_FORCE_RATE, 0, &HwAgpForceRate);
         }
      }
}


/*----------------------------------------------------------------------
Function name:  FXWAITFORIDLE

Description:    Spin until the HW is available.

Information:

Return:         VOID			  SstIORegs FAR *lph3IORegs;

----------------------------------------------------------------------*/
void
FXWAITFORIDLE()
{
  SstIORegs FAR *lph3IORegs;
  CmdFifo FAR *lpCmdFifo;
#ifdef SLI_AA
  DWORD dwStatus;
  int i;								 
  int nNumChips;
#endif

  P6FENCE; // Flush write combine buffers


  lph3IORegs = (SstIORegs *)_FF(regBase[HWINFO_SST_IOREGS_INDEX]);
  if (_FF(doAgpCF) && CMDFIFOUNBUMPEDWORDS)
  {
    bumpAgp( CMDFIFOUNBUMPEDWORDS );
    
    while (GET(lph3IORegs->status) & SST_PCIFIFO_BUSY)
      ;
   
   lpCmdFifo = (CmdFifo *)&(((SstCRegs *)(_FF(regBase[HWINFO_SST_CMDFIFOREGS_INDEX])))->PRIMARY_CMDFIFO);
    while (GET(lpCmdFifo->depth) > 0)
      ;
  }


#ifdef SLI_AA
  // If we are in SLI AA Mode
  if (_FF(gdiFlags) & SDATA_GDIFLAGS_SLI_AA_MASTER)
      nNumChips = (int)_FF(dwNumUnits);
  else 
      nNumChips = 1;

  do
      {
      dwStatus = 0x0L;
      for (i=0; i<nNumChips; i++)
         {
         lph3IORegs = (SstIORegs *)_FF(regBase[i * HWINFO_SST_MAX_NUM_CHIPS + HWINFO_SST_IOREGS_INDEX]);        
         dwStatus |= GET(lph3IORegs->status);         
         }
      } 
   while (dwStatus & SST_BUSY);

#else
  while (GET(lph3IORegs->status) & SST_BUSY)
    ;
#endif
}

/*----------------------------------------------------------------------
Function name:  LoadTVPCIDLL

Description:    Load TV PCI 16bit DLL (STBTV16.DLL) into video driver 
                name space so that no other versions of the STBTV16.DLL 
                will be loaded
            
Information:    DELL specific routine to support DELL WDM driver

Return:         void
----------------------------------------------------------------------*/

void LoadTVPCIDLL (char * DLLName)
{
/*  Original STBWTV16.DLL was hardcoded specifically for nVidia TNT in 
   accordance to DELL specifictions. DELL now wants this same DLL to 
   work with Voodoo3 but does not want to update the DLL in an effort to
   not test a new PCI TV driver with all of their products. So, we load 
   the updated TV PCI DLL into the system's windows directory and load
   it during driver load so that the new DLL will be used instead of 
   the old DLL while keeping the old DLL in the Windows\System directory.
*/
    DWORD retVal;
   char FileName[MAX_PATH];

    // get string with system's Windows directory
    retVal = GetWindowsDirectory (FileName, MAX_PATH);
   if (retVal == 0) return;

    // append the DLL that we need to load
    strcat (FileName, "\\");
   strcat (FileName, DLLName);

    // load the TV PCI DLL with Voodoo3 support
    LoadLibrary (FileName);
}

/*----------------------------------------------------------------------
Function name:  Enable1

Description:    Enables the HW.
                Initializes lots of variables, hooks INT 2Fh,
                initialized memory manager, device bitmaps, cursors,
                palette, etc., and set the required mode.

Information:    Refer to the driver DDK documentation for generic
                description.

Return:         UINT    Size of the structure filled in or,
                        TRUE if successful or,
                        FALSE if failure.
----------------------------------------------------------------------*/
DWORD  apfnTable32;     // table of 32-bit entrypoints

long tvoutCallMinivdd (int parameter, void *io);

#ifdef DEBUG
int __cdecl inp(unsigned);
#pragma intrinsic(inp)
#endif

int __cdecl outp(unsigned, unsigned);
#pragma intrinsic(outp)

#ifdef SLI_AA
DWORD dwSpecialNumber;
#endif

#pragma optimize("", off)
UINT FAR PASCAL _loadds
Enable1(LPVOID      lpDevice,
  UINT        style,
  LPSTR       lpDeviceType,
  LPSTR       lpOutput,
  LPVOID      lpStuff)
{
    DWORD dwResolutionX;
    DWORD dwResolutionY;
    DWORD dwBPP;
    DWORD dwMemClk;
    DWORD dwFlags;
    DWORD dwData;
    int   modeNumber;
    UINT size;
    HwInfo myHwInfo;
    HwInfo *pHwInfo = &myHwInfo;
    DWORD fIsDisplay;
    int doVgaMode12h;
    FxU32 dramInit1;
    BYTE bIndex;
    BYTE bAddr;
    int i;
    char valueName[24];
    DWORD vsize, value;
#ifndef WIN_CSIM
    TVSETSTANDARD TVSetStd;
#endif
    DWORD OverrideTV =0;
    DWORD dwData1;
    int j;

// STB-SR 12/30/98 Adding variable for temporary storage of the customer number
  FxU32 customerNumber;

// STB-EC 4/26/99  Need OS version info
    OSVERSIONINFO   ovi;

    // STB-SJ - 3/3/99
    // if this is on a DELL system, load the fixed TV PCI DLL
   // to get Voodoo3 support - see comment for LoadTVPCIDLL() above
    if (_FF(customerNumber) == 9) LoadTVPCIDLL ("stbwtv16.dll");

#ifdef SLI_AA
     // if SLAVE then don't pay attention to modes sets until time
    if (SDATA_GDIFLAGS_SLI_AA_SLAVE & _FF(gdiFlags))
      return TRUE;
#endif

    fIsDisplay = TRUE;

    if (!FirstEnable)
	{
   		if  (!(GET(lph3IORegs->dacMode) & DAC_DPMS_BITS)) // Do not call wait for idle
	      	FXWAITFORIDLE();							  // if we are in DPMS power save.
	}


    // Initialize gamma ramp, TvOut and Flat panel status
    if (FirstEnable)
    {
            init_gammaramp();
            _FF(ddRunTimeVersion) = 0;
            _FF(ddMiscFlags) = 0;

#ifndef WIN_CSIM
         // Note: dfp initialization is done as a side effect of tvout Initialization because it
         // was easier to call GetBIOSInfo in that code path and we need the values from the bios
         // filled into "biosBoardConfigInfo" before we initialize the DFP code.
         _FF(dwTvoActive) = !!tvoutGetBootStatus();
#else
         _FF(dwTvoActive) = 0x0;
#endif

	   	// Dynamically create the Mode Table here based upon registry entries from the INF.
	   	// See "INFNOTES.TXT" in the DOCS directory for information on how to edit modes.
	   	// DYNAMIC MODE TABLE
	   	if ( ModeList == NULL )		// If ModeList has not been created, alloc space for it
	   	{
	   		ModeList = ( MODEINFO FAR *)di_AllocModeTable( &nNumModes );
	   		if ( ModeList == NULL )
	   			return FALSE;
	   		di_FillModeTable( ModeList, nNumModes );		// Parse registry & fill ModeList
	   		di_SortModeTable( ModeList, nNumModes );		// Sort the ModeList
	   	}

/* RYAN@990625, not sure if I want to do this.
         // if the BIOS says we booted to the TV, but no TV is present now,
         // punt back to using the CRT
         if (_FF(dwTvoActive))
            _FF(dwTvoActive) = !!tvoutCallMinivdd (H3VDD_GET_TVNVRAM_STATUS, 0);
*/

         // if we didn't boot to the TV, but the user shut down while
         // on the TV last time, we want to go there now, UNLESS one of
         // the following is true:
         //    --There is no TV connected (@TODO: Implement this!)
         //    --The only reason we were on the TV last time was because
         //      no monitor was present, but had there been, we would have been
         //      on the monitor
         if (!_FF(dwTvoActive))
         {
            vsize = sizeof(value);
            value = 0; // initialize to "off" in case key isn't in registry
            strcpy(valueName, "enabled");
            CM_Read_Registry_Value (DisplayInfo.diDevNodeHandle, "TV",
                                    valueName, REG_DWORD, (LPBYTE)&value, 
                                    &vsize, CM_REGISTRY_SOFTWARE);

            if (value)
            {
               // hmm... registry says use the TV.  If the reason for this was
               // because no monitor was present last time, then we should ignore
               // this, because there IS a monitor present THIS time.
               vsize = sizeof(value);
               value = 0; // initialize to "no" in case key isn't in registry
               strcpy(valueName, "priorbootTV");
               CM_Read_Registry_Value (DisplayInfo.diDevNodeHandle, "TV",
                                       valueName, REG_DWORD, (LPBYTE)&value, 
                                       &vsize, CM_REGISTRY_SOFTWARE);

               _FF(dwTvoActive) = value ? 0 : 1;
            }
            else
               _FF(dwTvoActive) = 0;

            // if the last time we booted up, there was a tv, but this time:
            //    --there is no tv
            //    --the "connector" setting is set to "auto"
            // then we should ignore the registry and boot up to the CRT.
            if (_FF(dwTvoActive))
            {
               _FF(dwTvoActive) = !!tvoutCallMinivdd (H3VDD_GET_TVNVRAM_STATUS, 0);
               if (!(_FF(dwTvoActive)))
               {
                  OverrideTV = 1; // don't commit to registry
               }
            }
         }

         // commit our decisions back to the registry
         value = OverrideTV ? 1 : _FF(dwTvoActive);  // are we presently active?
         strcpy(valueName, "enabled");
         CM_Write_Registry_Value(DisplayInfo.diDevNodeHandle, "TV",
                                 valueName, REG_DWORD, &value,
                                 sizeof(DWORD), CM_REGISTRY_SOFTWARE);

         value = OverrideTV ? 0 : !!tvoutGetBootStatus();  // did we boot to TV this time?
         strcpy(valueName, "priorbootTV");
         CM_Write_Registry_Value(DisplayInfo.diDevNodeHandle, "TV",
                                 valueName, REG_DWORD, &value,
                                 sizeof(DWORD), CM_REGISTRY_SOFTWARE);

         if (GetRegStrToInt(_FF(DevNode), "allowPALCRT", (FxI32 *)&value))
         {
             DPF(DBGLVL_ALL, "The 'allowPALCRT' value was found in the registry.\n");
             TvoutAllowPALandCRT(1UL);
             _FF(allowPALCRT) = 1;
         }
         else
         {
             DPF(DBGLVL_ALL, "The 'allowPALCRT' value was NOT found in the registry.\n");
             TvoutAllowPALandCRT(0UL);
             _FF(allowPALCRT) = 0;
         }
      
    }
    
    //
    //  if we are the display driver read the mode
    //  info from the registry.
    //
    //  if we are not the display driver go into
    //  a default mode.
    //
    if (fIsDisplay)
    {
            DPF(DBGLVL_NORMAL,"Enable as the DISPLAY driver.");

            VDDCall(VDD_GET_DISPLAY_CONFIG, dwDeviceHandle, sizeof(DisplayInfo), 0,
                    &DisplayInfo);
            dwBPP         = DisplayInfo.diBpp;
            dwFlags       = DisplayInfo.diInfoFlags;
            dwResolutionX = DisplayInfo.diXRes;
            dwResolutionY = DisplayInfo.diYRes;
    }
    else
    {
            DPF(DBGLVL_NORMAL,"Enable as external driver.");
            dwBPP         = ModeList[0].dwBPP;
            dwResolutionX = ModeList[0].dwWidth;
            dwResolutionY = ModeList[0].dwHeight;
            dwFlags       = 0;
    }

    //
    // see if the mode is valid.
    // if it is not a mode we can support we fail
    //
    
    // if this is an enable that matches a driver disable, then restore
    // the mode that was last set before the disable, not the current settings
    // from the registry (monster truck madness 1 depends on this behavior)
    // careful!  don't clear the disabled flag until we get the second
    // enable call
    //
    if (_FF(ddMiscFlags) & DDMF_DRIVER_DISABLED)
    {
            modeNumber = (int) _FF(ModeNumber);

            // we need to adjust dwResolutionX and Y so that the _InquireInfo
            // has the right values
            dwResolutionX = ModeList[modeNumber].dwWidth;
            dwResolutionY = ModeList[modeNumber].dwHeight;
    }
    else
    {

// STB-SR 12/30/98 Adding support for specific customer number read from registry
// We need to do this before we go through the available list of modes.
        if (GetRegStrToInt(DisplayInfo.diDevNodeHandle, "custNum", (FxI32 *)&customerNumber))
        {
         DPF(DBGLVL_ALL, "Reading Customer Number: %d\n", customerNumber);
         _FF(customerNumber) = (WORD)customerNumber;
        }
        else
        {
         DPF(DBGLVL_ALL, "Customer Number Not Found! Make sure INF is installing key.\n");
         _FF(customerNumber) = 0;
        }

            modeNumber = di_FindMode(dwResolutionX, dwResolutionY, dwBPP, dwFlags);

            // If we fail to find the mode, then step down and attempt to
            // force 640x480xdwBpp.  This will avert booting into the MS
            // vga.drv driver.  Fixes PRS 3866.

            // @RBISSELL, note that this functionality is also needed
            // for TVOUT to work correctly.
            if ((modeNumber == -1) && (dwResolutionX > 640))
            {
                dwResolutionX = 640;
                dwResolutionY = 480;
                modeNumber = di_FindMode(dwResolutionX, dwResolutionY, dwBPP, dwFlags);
            }
    }

    //
    // InquireInfo means fill in a GDIINFO structure
    // that describes the mode and the capabilities of the device
    //
    // we call DIB_Enable() and modify the fields specific to our
    // driver.
    //
    // NOTE you should never set (ie assign to) the dpRasterCaps
    // or dpCaps1 fields.  you should set specific bits (|=val), or in
    // rare cases clear a bit (&=~val).
    //
    // return the size of the structure we filled in.
    //
    if (style == InquireInfo)
    {
            size = _InquireInfo(lpDevice, style,
                                dwResolutionX, dwResolutionY, dwBPP);
            return size;
    }

    // clear the diasbled flag
    // 
    if (_FF(ddMiscFlags ) & DDMF_DRIVER_DISABLED)
      _FF(ddMiscFlags) &= ~DDMF_DRIVER_DISABLED;

#ifdef GBLDATA_IN_PDEV
    lpDriverPDevice = lpDevice;
    lpDriverData = &(((PDEV FAR *)lpDevice)->DriverData);
    if (FirstEnable)
      memset(lpDriverData,0,sizeof(GLOBALDATA));
#else
    lpDriverData = &DriverData;

    // @RBISSELL, TVOUT and DFP multimon fix
    if (FirstEnable)
    {
      lpDriverData->ulSavedStatesOfSavedDevices = 0;
      lpDriverData->ulDevicesThatHaveTheirStatesSaved = 0;
    }
#endif


#ifndef NOLOWRESFIX
#if 0
    if (ModeList[modeNumber].dwWidth == 320)
    {
            lowresheight = ModeList[modeNumber].dwHeight;
            dwResolutionX = 640;
            dwResolutionY = 480;
            modeNumber = FindMode(dwResolutionX, dwResolutionY, dwBPP, dwFlags);
            lowreshack = 1;
    }
    else
#endif // #if 0
    {
            lowreshack = 0;
    }

#endif // #ifndef NOLOWRESFIX

#if 0
#ifdef DEBUG
    //
    // DISPLAY drivers need to fail if the mode is not in the registry
    // but while debugging it is nice not to do this.
    //
    if (modeNumber == -1)
    {
            DPF(DBGLVL_NORMAL,"cant find mode, using default.");
            modeNumber = 0;
            dwBPP         = ModeList[0].dwBPP;
            dwResolutionX = ModeList[0].dwWidth;
            dwResolutionY = ModeList[0].dwHeight;
            dwFlags       = 0;
    }
#endif
#endif

    if (modeNumber == -1)
    {
            DPF(DBGLVL_NORMAL,"Enable failed, cant find mode.");
            return FALSE;
    }

    _FF(fIsDisplay) = fIsDisplay;

    //
    //  EnableDevice means actualy set the mode.
    //  return zero for fail, non zero for success.
    //

    if (style != EnableDevice)
      return 0;

    //make sure the bit that identifies the code segment is 32 bit is set.
    //Changeto32();

    DPF(DBGLVL_NORMAL,"PhysicalEnable");
    // hook int 2f
#ifdef SLI_AA
    if (_FF(fIsDisplay) && (1 == dwDeviceHandle) && (0x0 == DisplayInfo.diUnitNumber))
#else
    if (_FF(fIsDisplay) && (1 == dwDeviceHandle))
#endif
    {
      hook_int2f();
    }

    if (FirstEnable)
    {
#ifdef SLI_AA
   pHwInfo->diUnitNumber = DisplayInfo.diUnitNumber;
#endif
   VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle,
      H3VDD_GET_HW_INFO, 0, pHwInfo);

#ifdef SLI_AA
   dwSpecialNumber = pHwInfo->dwSpecialNumber;
   _FF(dwType) = pHwInfo->dwType;
   _FF(dwNumUnits) = pHwInfo->dwNum;
#else
   _FF(dwNumUnits) = 1;
#endif

        if (pHwInfo->ioBase == 0)
            return FALSE;
        
        // Let's ramp these tests up a bit
        // The problem is that the Win '98 ConfigMgr will fail
        // to program the bridge chip correct if
        // it has to do a double rebalance.  This condition will occur
        // if you insert a ISA device that the system bios does not
        // know about.  This will causes two rebalances to have to be done
        // 1) For AGP Video ROM BAR -- This is typically not allocated space by
        // system bioes
        // 2) For the new ISA device IRQ.
        // After a reboot the Configmgr will write the IRQ to the ESCD
        // and the next boot will work right.  In the meantime we should
   // not hang the system.
   //
        bIndex = (BYTE)inp((unsigned)pHwInfo->ioBase + 0xd4);
        outp((unsigned)pHwInfo->ioBase + 0xd4, 0x1c);
        bAddr = (BYTE)inp((unsigned)pHwInfo->ioBase + 0xd5);
        outp((unsigned)pHwInfo->ioBase + 0xd4, (unsigned)bIndex);
        if (bAddr != pHwInfo->ioBase >> 8)
            return FALSE;
   
        if (pHwInfo->regBase[HWINFO_SST_2DREGS_INDEX] == 0)
            return FALSE;

        // More Tests
        // Yo... Registers are you mapped and ready?
        GetFlatSel();
        lph3g = (SstGRegs *) (pHwInfo->regBase[HWINFO_SST_2DREGS_INDEX]);
        dwData = GET(lph3g->colorBack);
        SETDW(lph3g->colorBack, 0xA55A5AA5);

        // Read/writes are not strongly ordered so read a few
        // times
        for (j=0; j<1000; j++)
            {
            dwData1 = GET(lph3g->colorBack);
            if (0xA55A5AA5 == dwData1)
               {
               break; 
               }
            }

        SETDW(lph3g->colorBack, dwData);
        if (0xA55A5AA5 != dwData1)
            {
            return FALSE;
            }      

        if (pHwInfo->lfbBase == 0)
            return FALSE;

        // Yet More Tests
        // Yo... FrameBuffer are you mapped and ready?
        dwData = h3READ(NULL, (unsigned long far *)pHwInfo->lfbBase);
        h3WRITE(NULL, (unsigned long far *)pHwInfo->lfbBase, 0x5AA5A55A);
        if (0x5AA5A55A != h3READ(NULL, (unsigned long far *)pHwInfo->lfbBase))
   {
            h3WRITE(NULL, (unsigned long far *)pHwInfo->lfbBase, dwData);
            return FALSE;
   }
        h3WRITE(NULL, (unsigned long far *)pHwInfo->lfbBase, dwData);
          
        if ((pHwInfo->memSizeInMB !=  4) && (pHwInfo->memSizeInMB !=  8) &&
            (pHwInfo->memSizeInMB != 16) && (pHwInfo->memSizeInMB != 32) &&
            (pHwInfo->memSizeInMB != 64))
        {
            return FALSE;
        }

        _FF(ioBase) = pHwInfo->ioBase;
        for (i=0; i<HWINFO_SST_MAX_BASE_INDEX; i++) 
           _FF(regBase[i]) = pHwInfo->regBase[i];
        _FF(lfbBase) = pHwInfo->lfbBase;
#ifdef WIN_CSIM
         _FF(regRealBase) = pHwInfo->RealregBase;
         _FF(lfbRealBase) = pHwInfo->ReallfbBase;
         _FF(regFakeBase) = pHwInfo->FakeregBase;
         _FF(lfbFakeBase) = pHwInfo->FakelfbBase;
#endif
        _FF(TotalVRAM) = (FxU32) pHwInfo->memSizeInMB * 1024L * 1024L;
       _FF(VendorDeviceID) = pHwInfo->VendorDeviceID;
       _FF(RevisionID)     = pHwInfo->RevisionID;
        _FF(PLDRevisionID)  = pHwInfo->PLDRevisionID;
       _FF(SSID)           = pHwInfo->SSID;
       _FF(AGPCaps)        = pHwInfo->AGPCaps;
       _FF(cpuType)        = pHwInfo->cpuType;
	   _FF(dwVIACoreLogic) = pHwInfo->dwVIACoreLogic;

       // Initialize all Glide Context flags to not active and default state
       // For new per process glide modifications.
	   {
         int index; 

         for ( index = 0; index < MAX_GLIDE_STATES; index ++ )
		 {
           _FF(GlideContext [ index ].procID) = 0;
           _FF(GlideContext [ index ].lostContext) = 1;
           _FF(GlideContext [ index ].EntryActive) = 0;
		 }
	   }

        // need to check pHwInfo to see whether this is an SDRAM board
        // or not.  For now, just assume that its SGRAM, and we can turn
        // this on/off for performance evaluation via the control call.

        _FF(isSdram) = FALSE;

        // choose the default value for doing vga mode 12h in a window
        // based on the memory size.   8MB and 16MB boards will have it
        // on by default, 4MB boards will have it off by default

        if (pHwInfo->memSizeInMB == 4)
            _FF(doVgaMode12h) = 0;
        else
            _FF(doVgaMode12h) = 1;

        _FF(DevNode) = DisplayInfo.diDevNodeHandle;

        if (fpRepaintScreen == NULL)
        {
            HINSTANCE h;

            if (h = GetModuleHandle("USER"))
                fpRepaintScreen = GetProcAddress(h, MAKEINTATOM(275));
            _FF(fpRepaintScreen) = (DWORD) fpRepaintScreen;
        }

        lph3agp = (SstCRegs *)(_FF(regBase[HWINFO_SST_CMDFIFOREGS_INDEX]));
        lph3IORegs = (SstIORegs *)(_FF(regBase[HWINFO_SST_IOREGS_INDEX]));
        lph3_3d = (SstRegs *)(_FF(regBase[HWINFO_SST_3DREGS_INDEX]));
        lph3cmdfifo0 = (CmdFifo *)&(lph3agp->PRIMARY_CMDFIFO);
        lpfifocache0=&fifocache0;

        // read the registry to get tweak values, if any
        //
        tweakFromRegistry();

       // Move by APS to before we set the DDMF_ENABLE_DEVICEBITMAPS bit
        _FF(ddMiscFlags) = 0;

        InitDeviceBitmapFilter();
        EnableDeviceBitmaps();    // set a flag to enable device bitmaps

        // initialized by D3D when it loads so default to unknown state
        _FF(pD3context) = 0xffffffff; 
        _FF(pD3changed) = 0xffffffff;
        _FF(pD3colbuff) = 0xffffffff;
        _FF(pD3auxbuff) = 0xffffffff;

        dramInit1 = GET(lph3IORegs->dramInit1);
        if (dramInit1 & SST_MCTL_TYPE_SDRAM)
            _FF(ddMiscFlags) |= DDMF_MEMTYPE_SDRAM;

        _FF(ddCurrentSurfaceLevel) = 0;

        // PRS 4457 - web browser hang - hardware WAX bug 
        _FF(lastY) = 0;

        _FF(dwAuxExclusiveMode) = 0;

    } // FirstEnable

    doVgaMode12h = setupVgaMode12h(dwDeviceHandle);

    //
    // Do all linear memory manager house cleaning before we change the
    // offscreen memory configuration in setupOffscreenMemory().  Fix
    // defect 1854.
    //
#ifdef SSB    
    DiscardAllSSB();    // invalidate save screen bitmaps
#endif    
        
#ifdef PERF_NEWMM
    CacheDepopulate();
#else        
    DoAllHost();        // Hostify
#endif    

#ifdef SLI_AA
   _FF(dwSLIMode) = H3VDD_SLI_READ_NOT_IN_USE;
#endif

    setupOffscreenMemory(modeNumber, doVgaMode12h);

    if((DWORD) ModeList[modeNumber].dwBPP == 16)
        _FF(ddMiscFlags) |= DDMF_16BPP_PRIMARY;
    else
        _FF(ddMiscFlags) &= ~DDMF_16BPP_PRIMARY;
      
#ifdef AGP_CMDFIFO
    if (doAgp)
    {
            setupAgpMemory();
            doAgp = 0;
    }
#endif // #ifdef AGP_CMDFIFO

    //
    // set the video mode
    // enables the desktop surface to start at _FF(gdiDesktopOffset)
    // disables the overlay surface
    //

    _FF(ddVisibleOverlaySurf) = 0;
    if (!HWSetMode(modeNumber))
    {
        DPF(DBGLVL_NORMAL,"HWSetMode failed");
        return FALSE;
    }

    // read Misc Output reg and save bit 7 (vsync polarity) in ddMiscFlags
    _FF(ddMiscFlags) &= ~DDMF_VSYNC_POLARITY_MASK;
   if ((FxU16)inp((WORD)_FF(ioBase) + 0xCC) & 0x80)
    {
      _FF(ddMiscFlags) |= DDMF_VSYNC_POLARITY_MASK;
    }

    //
    // every thing worked remember the mode number and the driver
    // PDevice and return success
    //
    _FF(dwVersion) = DDMINI_VERSION;
    _FF(ModeNumber) = modeNumber;
    _FF(lpPDevice) = lpDevice;
    _FF(fHardwareCursor) = FALSE;
    _FF(pitch) = ModeList[modeNumber].lPitch ;
    if (IS_NAPALM) {
        _FF(ModeUses2PixPerClkRender) = (ModeList[modeNumber].dwFlags & REND2PIX_PER_CLK) ? 
                                    SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK:0x0;
    }
    _FF(hres) = (WORD)ModeList[modeNumber].dwWidth;
    _FF(vres) = (WORD)ModeList[modeNumber].dwHeight;
    _FF(bpp) = (BYTE)ModeList[modeNumber].dwBPP;
    _FF(refresh) = ModeList[modeNumber].wVert;
    dwMemClk = GET(lph3IORegs->pllCtrl1);
    _FF(memclk) = (WORD)(PLL2MHz((DWORD)dwMemClk) / 10000L);	


    if (8 == ModeList[modeNumber].dwBPP)
    {
        _FF(dwRBitMask) = 0x00000000L;
        _FF(dwGBitMask) = 0x00000000L;
        _FF(dwBBitMask) = 0x00000000L;
    }
    else if (16 == ModeList[modeNumber].dwBPP)
    {
        _FF(dwRBitMask) = 0x0000F800L;
        _FF(dwGBitMask) = 0x000007E0L;
        _FF(dwBBitMask) = 0x0000001FL;
    }
    else if (24 == ModeList[modeNumber].dwBPP)
    {
        _FF(dwRBitMask) = 0x00FF0000L;
        _FF(dwGBitMask) = 0x0000FF00L;
        _FF(dwBBitMask) = 0x000000FFL;
    }
    else
    {
        _FF(dwRBitMask) = 0x00FF0000L;
        _FF(dwGBitMask) = 0x0000FF00L;
        _FF(dwBBitMask) = 0x000000FFL;
    }
    _FF(fReset) = TRUE;
    _FF(lastOverlayAddress) = INVALID_ADDRESS;

    _FF(dd3DSurfaceCount) = 0;
    _FF(ddHasShrinkBuffer) = FALSE;
    _FF(ddShrinkBufferFree) = FALSE;

    // Compute tile mark and pitch.

    if _FF(ddPrimaryInTile)
    {
      _FF(ddTileMark) = (_FF(gdiDesktopStart) & (~SSTG_IS_TILED));
    }
    else
    {
      _FF(ddTileMark) = _FF(TotalVRAM);
    }

    if      ((_FF(ddTileStride) * SST_TILE_WIDTH) <= 1024) _FF(ddTilePitch) = 1024L;
    else if ((_FF(ddTileStride) * SST_TILE_WIDTH) <= 2048) _FF(ddTilePitch) = 2048L;
    else if ((_FF(ddTileStride) * SST_TILE_WIDTH) <= 4096) _FF(ddTilePitch) = 4096L;
    else if ((_FF(ddTileStride) * SST_TILE_WIDTH) <= 8192) _FF(ddTilePitch) = 8192L;
    else                                                   _FF(ddTilePitch) = 16384L;

    _FF(ddTiledHeapActive) = FALSE;

    _FF(ScreenAddress) =  (_FF(gdiDesktopStart) & (~SSTG_IS_TILED)) + _FF(LFBBASE);

    _FF(MsgFcn) = (DWORD) DbgOut;
    (char *)(_FF(bufferptr16)) = (char*) &(_FF(charbuffer[0]));
    (char *)(buffer) = (char *)(_FF(charbuffer));

    if (!DibEnable(lpDevice, style, modeNumber))
        return FALSE;

    //
    // now re-register with DirectDraw so it knows all about the
    // new display mode.
    //
    if (_FF(HALCallbacks).lpSetInfo)
    {
        DDCreateDriverObject(TRUE);
    }

    apfnTable32 = VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle,
                          H3VDD_GET_FN_TABLE32, 0, 0);

    _FF(lpPDevice)->deFlags |= BUSY;
    // This needs to come first so when we call InitRegs we will use the right value
    TweakOnModeSwitch();
    // arguements are only inportant for csim builds
    InitThunks(_FF(LFBBASE), 0x1000000L);

#ifdef SLI_AA
    VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle,
      H3VDD_CLEAR_SLAVE_BIT, 0, 0x0);
#endif
    _FF(lpPDevice)->deFlags &= ~BUSY;

    // forcibly remove HWC_exclusive.
    // XXX TODO XXX
    // To be complete, we also need to
    // set state noting that we've done this, which isn't done here
    //
    _FF(gdiFlags) &= ~(SDATA_GDIFLAGS_HWC_EXCLUSIVE);


    // Initialize all entries in the glide context list to lost context
    // being true.
    // For new per process glide modifications.
	{
      int index; 

      for ( index = 0; index < MAX_GLIDE_STATES; index ++ )
	  {
        _FF(GlideContext [ index ].lostContext) = 1;
	  }
	}
    

#ifdef SSB
    #ifdef PERF_NEWMM
    saveScreenBitmapAllowed = InitCache();
    #else
    // enable SaveScreenBitmap support if mmInit() succeeds
    saveScreenBitmapAllowed = mmInit();
    #endif
#else
    #ifdef PERF_NEWMM
    InitCache();
#else
    mmInit();
#endif
#endif

    InitCursor();
#ifdef SLI_AA
    _FF(dwSwitchCursor) = (DWORD)SwitchToHostCursor;
#endif

#ifdef STB_EDGETOOLS
   EdgeInit();
#endif

    setupPalette();
    set_gammaramp(1);

    // STB-EC 4/26/99 check for Win95 OS and set the win95OS variable in GLOBALDATA
    memset(&ovi, 0, sizeof(ovi));
    ovi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&ovi);
    if (ovi.dwMinorVersion < 10) 
       (_FF(win95OS) = 1);
    else 
       (_FF(win95OS) = 0);    

    // Initialize Device Bit Maps
    InitDeviceBitmap();
#ifndef WIN_CSIM
    tvoutSetStdInternal();

    // @RBISSELL, part of the fix for PRS 6294
    // If the TV is supposed to be on at boot, make sure it is on.
    if (FirstEnable && _FF(dwTvoActive))
    {
     // Imhoff - Make sure size and centering adjustments are loaded from
     // saved registry settings. No action is taken if reg entries are missing.
       TVOutRefreshMem( NULL );

      // this is how you turn on the TV, oddly enough...
         TVSetStd.dwSubFunc = QUERYSETSTANDARD;
         TVSetStd.dwStandard = 0; 
         TVOutSetStandard(&TVSetStd);
    }
#endif

    // Move Down here so Cursor Enable Bit would not be lost
    FirstEnable = 0;

    VDDCall(VDD_POST_MODE_CHANGE, dwDeviceHandle, 0, 0, 0);

#ifdef WIN_CSIM
    SETDW(((SstIORegs *)_FF(regRealBase))->vidDesktopStartAddr, (_FF(gdiDesktopStart) & SST_VIDEO_START_ADDR) << SST_VIDEO_START_ADDR_SHIFT);
    {
    SstRegs FAR * l3dRegs = (SstRegs *)(_FF(regFakeBase) + SST_3D_OFFSET);
    SETDW(l3dRegs->nopCMD, 0xF)
    SETDW(l3dRegs->chipMask, 0xFFFFFFFFL);
    SETDW(l3dRegs->fbzMode, SST_RGBWRMASK);
    SETDW(l3dRegs->aaCtrl, 0);
    SETDW(l3dRegs->combineMode, 0);
    }
#endif


    // BAJ 7/27/99 Adding the dwDeviceHandle to decern Win98 Primary or Secondary adapter
    //  This tells us not to call assembly functions in the secondary adapter because of 
    //  assembler's use of non contexted global data memory in the HAL
    lpDriverData->dwDeviceHandle = dwDeviceHandle;


    DPF(DBGLVL_NORMAL,"PhysicalEnable success");
    return TRUE;
}


/*----------------------------------------------------------------------
Function name:  Disable1

Description:    Disables the HW.
                Hostifies device bitmaps and  objects under the
                control of the memory manager, unhooks INT 2Fh,
                disables TV-out (if applicable).

Information:    Refer to the driver DDK documentation for generic
                description.

Return:         UINT    1 is always returned.
----------------------------------------------------------------------*/
#pragma optimize("", on)
UINT FAR PASCAL _loadds Disable1(DIBENGINE FAR *pde)
{
    DPF(DBGLVL_NORMAL,"Disable");

    // Hostify Bitmaps
#ifdef PERF_NEWMM
    CacheDepopulate();
#else        
    DoAllHost();
#endif    

    DIB_Disable(pde);           // let the DIBENG clean up.

#ifndef PERF_NEWMM    
    FreeAllNodeTablePages();    // free system memory used by mem mgr
#endif 
    
    if (_FF(fIsDisplay) && (1 == dwDeviceHandle))
    {
        unhook_int2f();         // remove us from the int 2f chain
    }

    pde->deFlags |= BUSY;       // device is BUSY

    // Wait for all commands to Flush 
    FXWAITFORIDLE();

    VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle,
            H3VDD_DISABLE_GDI_DESKTOP, 0, 0);

    HWSetMode(-1);              // call code in setmode.c to "unset" the mode

    //FreeFlatSel();            // dont leak a selector
    _FF(ddMiscFlags) |= DDMF_DRIVER_DISABLED;

#ifndef WIN_CSIM
// Imhoff - calls to tvoutDisableInternal() were causing the "Restart in MS-DOS mode"
// to display no TV signal... It is also likely that the "It's now safe to turn off your computer"
// screen would not be visible during a shutdown. Since having the TV on when unneeded is likely less 
// of a crime than TV off when TV on is absolutely needed, we will leave it on here.
//        tvoutDisableInternal();
#endif
    return 1;
}


/*----------------------------------------------------------------------
Function name:  ToBackground

Description:    Disables GDI desktop.
                Hostifies device bitmaps and  objects under the
                control of the memory manager.

Information:    Refer to the driver DDK documentation for generic
                description.

Return:         VOID
----------------------------------------------------------------------*/
void FAR PASCAL ToBackground()
{
    DPF(DBGLVL_NORMAL,"ToBackground");

    // Clear Flag to Tell Glide that context has been lost
    _FF(gdiFlags) &= ~(SDATA_GDIFLAGS_HWC_EXCLUSIVE);

    // Initialize all entries in the glide context list to lost context
    // being true.
    // For new per process glide modifications.
	{
      int index; 

      for ( index = 0; index < MAX_GLIDE_STATES; index ++ )
	  {
        _FF(GlideContext [ index ].lostContext) = 1;
	  }
	}

    _FF(DosActive) = 1;
    _FF(dwRelaxedOverlayOwnerMode) = 1;

    // Invalidate/remove all GDI objects in the Memory Manager
#ifdef PERF_NEWMM
    CacheDepopulate();
#else    
    DoAllHost();                                   // Device Bitmaps
#endif
    
#ifdef SSB
    DiscardAllSSB();                               // Save Screen Bitmaps
#endif
    _FF(mmFlags) |= MM_FONT_CACHE_INVALID;   // Fonts

    _FF(lpPDevice)->deFlags |= BUSY;

    // Wait for all commands to Flush 
    FXWAITFORIDLE();
    VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle, H3VDD_DISABLE_GDI_DESKTOP, 0, 0);
}


/*----------------------------------------------------------------------
Function name:  ToForeground

Description:    Enables GDI desktop.
                Resets the desktop mode, reinitializes thunks,
                resets palette, repaints screen, restores the
                cursor.

Information:    Refer to the driver DDK documentation for generic
                description.

Return:         VOID
----------------------------------------------------------------------*/
void FAR PASCAL ToForeground()
{
    DPF(DBGLVL_NORMAL,"ToForeground");

    // Clear Flag to Tell Glide that context has been lost
    _FF(gdiFlags) &= ~(SDATA_GDIFLAGS_HWC_EXCLUSIVE);

    // Initialize all entries in the glide context list to lost context
    // being true.
    // For new per process glide modifications.
	{
      int index; 

      for ( index = 0; index < MAX_GLIDE_STATES; index ++ )
	  {
        _FF(GlideContext [ index ].lostContext) = 1;
	  }
	}

    _FF(DosActive) = 0;
    if ((int)_FF(ModeNumber) < 0)
        return;

    HWSetMode((int)_FF(ModeNumber));
    
    _FF(lpPDevice)->deFlags |= BUSY;

    TweakOnModeSwitch();
    // We use to call InitThunks here but this turn out to be a bad deal
    // what happens is occasionally, we would be in the 32 bit thunk code
    // and we would get a interrupt.  When this happen, sometimes
    // the upper half of eip would get hacked... <some sorta of Inside
    // windows joke> and we would get a gpf.
    // To fix this I just call and ask the mini-vdd to politely ask to do this 
    // for me....
    VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle, H3VDD_INITFIFO, 0, 0);

    setupPalette(); // reset palette or gamma ramp
    set_gammaramp(1);

    _FF(lpPDevice)->deFlags &= ~BUSY;

    if (fpRepaintScreen != NULL)
    {
        fpRepaintScreen();
    }

    // Sometimes when exiting a DOS Application like
    // vmtest the order of SetCursor and ToForeground is not
    // ToForeground --> SetCursor.  ToForeground has the sideeffect of
    // clearing the vidProcCfg register.  Unfortunately, we cannot be
    // smarter in SetVideoMode since sometimes the calls are such that
    // the SetCursor calls come before ToBackground which is quite strange.
    // I sure don't understand why the order of the calls vary???
    // But I do know that if I reenable the cursor here if it is enabled
    // then all is well in BansheeLand.
    RestoreCursor();

    VDDCall(VDD_POST_MODE_CHANGE, dwDeviceHandle, 0, 0, 0);
}



/*----------------------------------------------------------------------
Function name:  ValidateMode1

Description:    Called by GDI to verify a mode is valid 
                prior to setting it.

Information:    Refer to the driver DDK documentation for generic
                description.

Return:         UINT    VALMODE_YES        if successful or,
                        VALMODE_NO_UNKNOWN if unsuccessful
----------------------------------------------------------------------*/
UINT FAR PASCAL _loadds
ValidateMode1(DISPVALMODE FAR *pdvm)
{
    DPF(DBGLVL_NORMAL,"ValidateMode1");

#if 0
    //
    // if the system is just testing possible modes by calling this
    // function, return YES to everything
    //
    if (FirstEnable)
    return VALMODE_YES;
#endif
    //
    // otherwise, go ahead and look up the mode in the mode table,
    // and run HwTestMode to make sure it can fit
    //
    if (di_FindMode(pdvm->dvmXRes, pdvm->dvmYRes, pdvm->dvmBpp, 0) != -1)
        return VALMODE_YES;
    else
        return VALMODE_NO_UNKNOWN;
}

/*----------------------------------------------------------------------
Function name:  ReEnable1

Description:    Re-enables the HW.
                Needs to Hostify all device bitmaps prior to
                re-enabling.

Information:    Refer to the driver DDK documentation for generic
                description.

Return:         UINT    TRUE    if successful or,
                        FALSE   if unsuccessful
----------------------------------------------------------------------*/
UINT FAR PASCAL _loadds ReEnable1(
    DIBENGINE FAR *     lpPDevice,
    GDIINFO FAR *       lpGDIInfo)
{
    DPF(DBGLVL_NORMAL,"ReEnable");

    // Low Power Mode?
    if ((_FF(lpPDevice)->deFlags & BUSY) && (0xFFFFFFFF == GET(lph3IORegs->pllCtrl1)))
       return FALSE; 

    // Here is one spot that we need to do this
    // Before we set the mode make sure DeviceBitMaps are on the host
    // Hostify
#ifdef PERF_NEWMM
    CacheDepopulate();
#else    
    DoAllHost();
#endif    

    if (Enable1(lpPDevice, EnableDevice, NULL, NULL, NULL))
    {
        Enable1(lpGDIInfo, InquireInfo, NULL, NULL, NULL);
        _FF(ddCurrentSurfaceLevel)++;
        _FF(ddPrimarySurfaceData).surfaceLevel = _FF(ddCurrentSurfaceLevel);
        return TRUE;
    }

    return FALSE;
}


/*----------------------------------------------------------------------
Function name:  BeginAccess

Description:    Support exclusion/restoration of the SW cursor.  Also,
                supports controlled access to the frame buffer.

Information:    Refer to the driver DDK documentation for generic
                description.

Return:         VOID
----------------------------------------------------------------------*/
void FAR PASCAL _loadds
BeginAccess(DIBENGINE FAR *pde,
       int left,
       int top,
       int right,
       int bottom,
       UINT flags)
{
   // Hey... If we are BUSY we may be in Low Power Mode
   // if so we will wait in here for the device to be ready forever
   if (_FF(lpPDevice)->deFlags & BUSY)
      return;

   if (_FF(gdiFlags) & SDATA_GDIFLAGS_SW_CURSOR)
      {  //
         // software cursor is in use, exclude it
         // EndAccess will wait for blter to idle
         //
         myBeginAccess(pde, left, top, right, bottom, flags);
      }
    //
    // idle the chip before allowing windows/the dib engine lfb access to the
    // framebuffer
    //
   FXWAITFORIDLE();
   MODIFY_SLI_READ(H3VDD_ENABLE_SLI_READ);

    // if we are using a DIB cursor turn it off.
    if ((_FF(gdiFlags) & SDATA_GDIFLAGS_DIB_CURSOR) &&
        (_FF(lpPDevice) == pde) &&
        (flags & CURSOREXCLUDE))
    {
        DIB_BeginAccess(pde, left, top, right, bottom, flags);
    }
}


/*----------------------------------------------------------------------
Function name:  EndAccess

Description:    Support exclusion/restoration of the SW cursor.  Also,
                supports controlled access to the frame buffer.

Information:    Refer to the driver DDK documentation for generic
                description.

Return:         VOID
----------------------------------------------------------------------*/
void FAR PASCAL _loadds EndAccess(DIBENGINE FAR *pde, UINT flags)
{
   if (_FF(gdiFlags) & SDATA_GDIFLAGS_SW_CURSOR)
      {  //
         // software cursor is in use, exclude it
         // EndAccess will wait for blter to idle
         //
         myEndAccess(pde, flags);
      }

    // if we are using a software cursor turn it back on.
    if ((_FF(gdiFlags) & SDATA_GDIFLAGS_DIB_CURSOR) &&
        (_FF(lpPDevice) == pde) &&
        (flags & CURSOREXCLUDE))
    {
        DIB_EndAccess(pde, flags);
#if 0
        // Hey I want to see the cursor here
        // but this really makes Winbench suck!!!!
        DIBMoveCursor(_FF(CursorPosX), _FF(CursorPosY));
#endif
    }

    //
    // XXX FIXME? XXX
    //
    // we may need an idle here even w/out CRASHTEST, check to see whether any
    // write occuring behind an LFB write could possibly cause anything to get
    // to memory ahead of the LFB write
    // -KMW
    //
   MODIFY_SLI_READ(H3VDD_DISABLE_SLI_READ);
#ifdef CRASHTEST
    FXWAITFORIDLE();
#endif // #ifdef CRASHTEST
}


FNSAVECURSOREXCLUDE * pSaveCursorExclude;
FNRESTORECURSOREXCLUDE * pRestoreCursorExclude;
FNDRAWCURSOR * pDrawCursor;
/*----------------------------------------------------------------------
Function name:  myBeginAccess

Description:    HW specific functionality of generic BeginAccess.

Information:

Return:         VOID
----------------------------------------------------------------------*/
void FAR PASCAL _loadds
myBeginAccess(DIBENGINE FAR *pde,
       int left,
       int top,
       int right,
       int bottom,
       UINT flags)
{
   if (_FF(lpPDevice)->deFlags & BUSY)
      return;

   if ((flags & CURSOREXCLUDE) &&
       (_FF(gdiFlags) & SDATA_GDIFLAGS_SW_CURSOR) )
   {
       // check for cursor exclusion
       if (DoIntersect(left, top, right, bottom))
       {

          if ((SetCursorBusy( (WORD FAR *) & (_FF(cursorBusy)) ) ) )
             {
             if (_FF(gdiFlags) & SDATA_GDIFLAGS_CURSOR_ENABLED)
                pRestoreCursorExclude(_FF(LastCursorPosX), _FF(LastCursorPosY));
             _FF(gdiFlags) |= SDATA_GDIFLAGS_CURSOR_EXCLUDE;
             ClearCursorBusy((WORD FAR *) & (_FF(cursorBusy))); 
             }
         _FF(gdiFlags) |= SDATA_GDIFLAGS_CURSOR_IS_EXCLUDED;

       }
   }

   _FF(gdiFlags) |= SDATA_GDIFLAGS_CURSOR_EXCLUDE;
#ifdef DEBUG_CURSOR
   GottaCursor();
#endif
}


/*----------------------------------------------------------------------
Function name:  myEndAccess

Description:    HW specific functionality of generic EndAccess.

Information:

Return:         VOID
----------------------------------------------------------------------*/
void FAR PASCAL _loadds myEndAccess(DIBENGINE FAR *pde, UINT flags)
{
   int x;
   int y;

   // Wait for FB writes to Flush <Doesnot really work but should pause us a little>
   if (!(_FF(lpPDevice)->deFlags & BUSY))
      FXWAITFORIDLE();

    // if we are using a software cursor turn it back on.
   if ( (flags & CURSOREXCLUDE) &&
       (_FF(gdiFlags) & SDATA_GDIFLAGS_SW_CURSOR) )
   {
// 1   2   4   5   6   7   8   9   a   b   c   d   e   f

           if (SDATA_GDIFLAGS_CURSOR_IS_EXCLUDED & _FF(gdiFlags))
              {
              x = _FF(CursorPosX) - _FF(HotspotX);
              y = _FF(CursorPosY) - _FF(HotspotY);
              if ((SetCursorBusy( (WORD FAR *) & (_FF(cursorBusy)) ) ) )
                 {
                 _FF(gdiFlags) &= ~(SDATA_GDIFLAGS_CURSOR_EXCLUDE);
                 if (!(_FF(lpPDevice)->deFlags & BUSY))
                     {
                     pSaveCursorExclude(x,y);
                     pDrawCursor(x,y);
#ifdef DEBUG_CURSOR
                     GottaCursor();
#endif
                     }
                 ClearCursorBusy((WORD FAR *) & (_FF(cursorBusy))); 
                 }
              else
                 _FF(gdiFlags) &= ~(SDATA_GDIFLAGS_CURSOR_EXCLUDE);
              }
    }
    //
    // XXX FIXME? XXX
    //
    // we may need an idle here even w/out CRASHTEST, check to see whether any
    // write occuring behind an LFB write could possibly cause anything to get
    // to memory ahead of the LFB write
    // -KMW
    //
#ifdef CRASHTEST
   if (!(_FF(lpPDevice)->deFlags & BUSY))
      FXWAITFORIDLE();
#endif // #ifdef CRASHTEST

   _FF(gdiFlags) &= ~(SDATA_GDIFLAGS_CURSOR_EXCLUDE | SDATA_GDIFLAGS_CURSOR_IS_EXCLUDED);
}


/*----------------------------------------------------------------------
Function name:  SetPalette1

Description:    Set the HW palette.

Information:

Return:         UINT    value returned from call to DIB_SetPaletteExt.
----------------------------------------------------------------------*/
UINT FAR PASCAL _loadds SetPalette1(
    UINT        start,
    UINT        count,
    DWORD FAR  *lpPalette)
{
    UINT rc;

    DPF(DBGLVL_NORMAL,"SetPalette1");

    rc = DIB_SetPaletteExt(start, count, lpPalette, _FF(lpPDevice));

    if (!(_FF(lpPDevice)->deFlags & BUSY))
    {
        HWSetPalette(start, count, lpPalette);
    }

    return rc;
}


/*----------------------------------------------------------------------
Function name:  Control1

Description:    Called by GDI when an application calls Escape
                or ExtEscape.  If the escape is not handled, it
                needs to be passed on to the dib engine.

Information:    Refer to the driver DDK documentation for generic
                description.

Return:         LONG    value returned from call to DIB_Control or
                        other numerous possibilities.
----------------------------------------------------------------------*/
#define OPENGL_GETINFO  4353        /* for OpenGL ExtEscape */


LONG FAR PASCAL _loadds Control1(
    DIBENGINE FAR * lpDevice,
    UINT            function,
    LPVOID          lpInput,
    LPVOID          lpOutput)
{
    DCICMD FAR *pdci;
    int retval;

    LPVIDEOPARAMETERS lpVidParams;

    DPF(DBGLVL_NORMAL,"Control1");

#ifdef STB_EDGETOOLS //Paul Magee 21 Jan 99
   if (EdgeEscape((int) function, (void FAR *)lpInput, (void FAR *) lpOutput))
   {
      return TRUE;
   }
      
#endif /* STB_EDGETOOLS */

    switch (function)
    {

#ifdef STEREOx
	  #define STEREOMODE 0x3344
	  case STEREOMODE:
	       	
			_FF(ddStereoHeapFactor) 	= 0;		   	
			_FF(ddStereoWrapperLoaded) 	= 0;

//		   	if( *(FxU32 *)lpInput ) _FF(ddStereoHeapFactor) = 4; //use 2 for double buffer 4 for triple
		   	if( *(FxU32 *)lpInput ) _FF(ddStereoWrapperLoaded) = 1; //use 2 for double buffer 4 for triple

	  break;
#endif

      case QUERYESCSUPPORT:
     //
     // QUERYESCSUPPORT is sent to ask
     // "do you support this" lpInput points
     // the function code to query
     //
     switch (*(UINT FAR *)lpInput)
     {
     case DCICOMMAND:
       // This is the "MS magic" that allows for
       // the DirectX certification bit to be
       // enabled.  Normally, this returns
       // DD_HAL_VERSION (0x100).  By changing
       // to return 0xFF, the certification bit
       // is "magically" set!
//         return 0xFF;       //Set the magic certification bit
       return DD_HAL_VERSION;
         
     case VIDEO_PARAMETERS:
         return TRUE;
         
         /* OpenGL Driver Info Escape */
     case OPENGL_GETINFO:
         return TRUE;

   // PRS 5262: For performance and other reasons,
   // MouseTrails are not supported, we return false
   // so that the Mouse Properties menu will be grayed out.
   // PRS 5400: Disabling MOUSETRAILS in Win95 causes problems.
   // Check OS first, disable only for Win98.
   // srogers 5/14/99 - Temporary Fix for PRS 5878.  If we disable mouse trails,
   // then Nascar Revolution does not load.  We will work with EA Sports to fix this
//  case MOUSETRAILS:
//         if (_FF(win95OS) == 0)
//            return FALSE;
     }
     break;

    case OPENGL_GETINFO: {
#       define MAX_PATH        260
        typedef struct {
            unsigned long ulVersion;
            unsigned long ulDriverVersion;
            char  awch[MAX_PATH+1];
        } OglOutInfo;
        OglOutInfo FAR *info = lpOutput;
        info->ulVersion       = 2;
        info->ulDriverVersion = 1;
        info->awch[0]         = '3';
        info->awch[1]         = 'D';
        info->awch[2]         = 'f';
        info->awch[3]         = 'x';
        info->awch[4]         = 0;
        return TRUE;
        break;
    }

      case MOUSETRAILS:
          //
          // keep track of state of mouse trails
          // we need to know if mouse trails are on
          // so we can turn off our hardware cursor.
          //
          if (lpInput)
             DoMouseTrails(*(UINT FAR *)lpInput);

          break;

      case QUERYESCMODE:
         if (TDFXACK == QueryMode((LPQIN)lpInput, lpOutput))
            return TDFXACK;
         break;

      case DCICOMMAND:
     pdci = (DCICMD FAR *)lpInput;

     if (pdci == NULL || pdci->dwVersion != DD_VERSION)
     {
         break;
     }

     /*
      * this request gives us our direct draw routines to call
      */
     if (pdci->dwCommand == DDNEWCALLBACKFNS)
     {
         DPF(DBGLVL_NORMAL,"DDNEWCALLBACKFNS");
         _FF(HALCallbacks) = *((LPDDHALDDRAWFNS)pdci->dwParam1);
         return TRUE;
     }
     /*
      * return information about our 32-bit DLL
      *
      * we pass a point to our shared global Data
      * to the 32-bit driver so we can talk to each
      * other by reading each others mind.
      */
     else if (pdci->dwCommand == DDGET32BITDRIVERNAME)
     {
         LPDD32BITDRIVERDATA p32dd = (LPDD32BITDRIVERDATA)lpOutput;

         DPF(DBGLVL_NORMAL,"DDGET32BITDRIVERNAME");

         lstrcpy(p32dd->szName, DDFXS32_DLLNAME);
         lstrcpy(p32dd->szEntryPoint, "DriverInit");
         p32dd->dwContext = GetSelectorBase(SELECTOROF(lpDriverData)) +
                            OFFSETOF(lpDriverData);

         return TRUE;
     }
     /*
      * handle the request to create a driver
      * NOTE we must return our HINSTANCE in *lpOutput
      */
     else if (pdci->dwCommand == DDCREATEDRIVEROBJECT)
     {
         DPF(DBGLVL_NORMAL,"DDCREATEDRIVEROBJECT");
         DDCreateDriverObject(FALSE);
         *(DWORD FAR *)lpOutput = _FF(HALInfo).hInstance;
         return TRUE;
     }
     else if (pdci->dwCommand == DDVERSIONINFO)
     {
         LPDDVERSIONDATA pddvd;

        _FF(ddRunTimeVersion) = pdci->dwParam1;     // Save runtime version
         pddvd = (LPDDVERSIONDATA) lpOutput;
         pddvd->dwHALVersion = DD_RUNTIME_VERSION; //Declare our HAL version
         DPF(DBGLVL_NORMAL, "Setting HAL Version=%08lx", pddvd->dwHALVersion );
         return TRUE;
     }
     break;

      case 0x8050:
          // Super, duper, ugly hack, for turning on/off SDRAM support
          // on the fly (e.g., turning on/off use of block writes)
          //
          switch (*(FxU32 *)lpInput)
          {
              FxU32 miscInit1;
        
            case 0:
                // block write off (== SDRAM on)
                miscInit1 = GET(lph3IORegs->miscInit1);
                miscInit1 |= 1L << 15;
                SETDW(lph3IORegs->miscInit1, miscInit1);
                _FF(isSdram) = TRUE;
                break;

            case 1:
                // block write on (== SDRAM off)
                miscInit1 = GET(lph3IORegs->miscInit1);
                miscInit1 &= ~(1L << 15);
                SETDW(lph3IORegs->miscInit1, miscInit1);
                _FF(isSdram) = FALSE;
                break;
          }
          break;

     case EXT_HWC:
       return hwcExt((DWORD *) lpInput, (DWORD *) lpOutput);
       break;

     case EXT_HWC_SHARE_CPUTYPE:
       return hwcShareCPUType ((DWORD *)lpInput, (DWORD *)lpOutput);
       break;

     case EXT_HWC_SET_AUX_EXCLUSIVE_MODE:
       return hwcSetAuxExclusiveMode((DWORD *)lpInput, (DWORD *)lpOutput);
	   break;

     case EXT_HWC_RELEASE_AUX_EXCLUSIVE_MODE:
       return hwcReleaseAuxExclusiveMode((DWORD *)lpInput, (DWORD *)lpOutput);
       break;

     case EXT_HWC_GET_AUX_EXCLUSIVE_MODE:
       return hwcGetAuxExclusiveMode((DWORD *)lpInput, (DWORD *)lpOutput);
	   break;

     case VIDEO_PARAMETERS:
                 retval = TVOutVideoParameters ((LPVIDEOPARAMETERS)lpInput);
                 if (retval >= 0)
                         return (retval);
            lpVidParams = (LPVIDEOPARAMETERS)lpInput;
         if ( lpVidParams->dwCommand == VP_COMMAND_GET )
         {
                 lpVidParams->dwFlags = 0;
                 lpVidParams->dwMode = 0;
                 lpVidParams->dwTVStandard = 0;
                 lpVidParams->dwAvailableModes = 0;
                 lpVidParams->dwAvailableTVStandard = 0;
                 lpVidParams->dwFlickerFilter = 0;
                 lpVidParams->dwOverScanX = 0;
                 lpVidParams->dwOverScanY = 0;
                 lpVidParams->dwMaxUnscaledX = 0;
                 lpVidParams->dwMaxUnscaledY = 0;
                 lpVidParams->dwPositionX = 0;
                 lpVidParams->dwPositionY = 0;
                 lpVidParams->dwBrightness = 0;
                 lpVidParams->dwContrast = 0;
                 lpVidParams->dwCPType = 0;
                 lpVidParams->dwCPCommand = 0;
                 lpVidParams->dwCPStandard = 0;
                 lpVidParams->dwCPKey = 0;
                 lpVidParams->bCP_APSTriggerBits = 0;
                 memset (lpVidParams->bOEMCopyProtection, 0,
                  sizeof(lpVidParams->bOEMCopyProtection));
       }
            return TRUE;
    }

    return DIB_Control(lpDevice,function,lpInput,lpOutput);
}


/*----------------------------------------------------------------------
Function name:  GetRegInt

Description:    Read a integer from the HKEY_CURRENT_CONFIG\
                Display\Settings key in the registry.  Will read
                a string value and return a integer, if the string
                is of the form X,Y will return X<<16+Y
Information:

Return:         DWORD   int from registry, or default if not there
----------------------------------------------------------------------*/
DWORD GetRegInt(LPSTR valname, DWORD def)
{
    HKEY    hkey;
    char    ach[20];
    LONG    cb;
    int     i;

    if (RegOpenKey(HKEY_CURRENT_CONFIG, "Display\\Settings", &hkey) == 0)
    {
        ach[0] = 0;
        cb = sizeof(ach);

        if (RegQueryValueEx(hkey, valname, NULL, NULL, ach, &cb) == 0)
        {
            for (def=i=0; ach[i]; i++)
            {
                if (ach[i] >= '0' && ach[i] <= '9')
                    *((WORD*)&def) = LOWORD(def) * 10 + ach[i]-'0';

                if (ach[i] == ',')
                    def = def << 16;
            }
        }

        RegCloseKey(hkey);
    }

    return def;
}


/*----------------------------------------------------------------------
Function name:  GetFlatSel

Description:    Allocates a selector and calls INT 31h to
                set the limit.
Information:    

Return:         UINT    The selector if successful or,
                        0 if failure.
----------------------------------------------------------------------*/

// STB Begin Changes
// STB-SR 1/13/98 Added code for bj
#ifdef STB_FIFO16_ENABLE
UINT FlatSel;           // Compiling for FIFO16 code, remove contraints on FlatSel
#else
// STB End Changes
static UINT FlatSel;    // Else, keep it static.
// STB Begin Changes
#endif
// STB End Changes


#pragma optimize("", off)
UINT GetFlatSel()
{
    if (FlatSel != 0)
        return FlatSel;

    FlatSel = AllocSelector(SELECTOROF((LPVOID)&FlatSel));

    if (FlatSel == 0)
        return 0;

    SetSelectorBase(FlatSel, 0);

    // SetSelectorLimit(FlatSel, -1);
    _asm    mov     ax,0008h            ; DPMI set limit
    _asm    mov     bx,FlatSel
    _asm    mov     dx,-1
    _asm    mov     cx,-1
    _asm    int     31h

    return FlatSel;
}
#pragma optimize("", on)


/*----------------------------------------------------------------------
Function name:  FreeFlatSel

Description:    Releases a previously allocated flat selector.

Information:    

Return:         UINT    The selector if successful or,
                        0 if failure.
----------------------------------------------------------------------*/
void FreeFlatSel()
{
    if (FlatSel)
    {
        SetSelectorLimit(FlatSel, 0);
        FreeSelector(FlatSel);
        FlatSel = 0;
    }
}


/*----------------------------------------------------------------------
Function name:  PhysToLinear

Description:    Calls INT 31h to do a physical to linear tranlation.

Information:    

Return:         DWORD   The linear address
----------------------------------------------------------------------*/
#pragma optimize("", off)
DWORD PhysToLinear(DWORD PhysAddress, DWORD PhysSize)
{
    DWORD LinearAddress;

    PhysSize = PhysSize-1;      // we want limit, not size for DPMI

    _asm
    {
        mov     cx, word ptr PhysAddress[0]
        mov     bx, word ptr PhysAddress[2]
        mov     di, word ptr PhysSize[0]
        mov     si, word ptr PhysSize[2]
        mov     ax, 0800h               ; DPMI phys to linear
        int     31h
        mov     word ptr LinearAddress[0], cx
        mov     word ptr LinearAddress[2], bx
    }

    return LinearAddress;
}


#ifdef TRACE

/*----------------------------------------------------------------------
Function name:  Msg

Description:    Format a debug string and send it to the debug
                monitor.

Information:    Only used when "#ifdef TRACE"

Return:         VOID
----------------------------------------------------------------------*/
/* h3g.h defines DPF(DBGLVL_NORMAL,) into Msg() */
DWORD DbgMsgLvl = 0; //DBGLVL_NORMAL;
//DWORD DbgMsgLvl = DBGLVL_LCD;

#define START_STR "H3 2D: "

void __cdecl _loadds Msg(DWORD DbgLvl, LPSTR szFormat, ...)
{
    char        str[1024];
    static int (WINAPI *fpwvsprintf)(LPSTR lpszOut, LPCSTR lpszFmt,
                  const void FAR* lpParams);

    if (!(DbgLvl & DbgMsgLvl)) return;

    if (fpwvsprintf == NULL)
    {
        fpwvsprintf = (LPVOID) GetProcAddress(GetModuleHandle("USER"),"wvsprintf");
        if (fpwvsprintf == NULL)
            return;
    }

    lstrcpy(str, START_STR);
    fpwvsprintf(str+lstrlen(str), szFormat, (LPVOID)(&szFormat+1));
    lstrcat(str, "\r\n");
    OutputDebugString(str);
}

#endif /* #ifdef TRACE */


/*
addr        : address to the base memmaped registers.
            if sim the addr must == 0x100000000.
register    : offset from register base address
data        :  dword data

Typically GET is a macro but I am implementing it as a function until
debugged.
Routine decides hardware or csim.
The level above decide hw or dibengine.

*/

#define VXDLDR_INIT_DEVICE 1
//#define VXDLDR_DEVICE_ID 0x27
#define VXDLDR_LoadDevice 1
#define VXDLDR_UnloadDevice 2


/*----------------------------------------------------------------------
Function name:  get_vxdldr_apiproc

Description:    Get the dynamic vxd loader's protected mode api
                entry point.
Information:    

Return:         INT     1 for Success, or
                        0 for Failure.
----------------------------------------------------------------------*/
DWORD vxdldr_apiproc;
int
get_vxdldr_apiproc()
{
    if (vxdldr_apiproc != 0L)
    return 1;

    _asm xor di,di;
    _asm mov es,di;
    _asm mov ax, 1684h;
    _asm mov bx,VXDLDR_DEVICE_ID;
    _asm int 2fh;
    _asm mov ax,es;
    _asm or ax,di;
    _asm jnz GoAhead;
    DPF(DBGLVL_NORMAL,"Could not find vxdldr entry point");
    return 0;

  GoAhead:
    _asm mov word ptr [vxdldr_apiproc],di;
    _asm mov word ptr [vxdldr_apiproc+2],es;

    return 1;
}


char VxDName[] = "H3HAL.VXD";
char VxDModName[]="H3HAL";


/*----------------------------------------------------------------------
Function name:  loadCsimVxd

Description:    Calls the VXDLDR to dynamically load the csim vxd

Information:    

Return:         INT     1 for Success, or
                        0 for Failure.
----------------------------------------------------------------------*/
int
loadCsimVxd()
{
    _asm mov ax,VXDLDR_LoadDevice;
    _asm mov dx, offset VxDName;
    _asm call dword ptr [vxdldr_apiproc];
    _asm jnc GoAhead2;
    DPF(DBGLVL_NORMAL,"Error loading csim VxD");
    return 0;

  GoAhead2:
    return 1;
}


/*----------------------------------------------------------------------
Function name:  get_csim_apiproc

Description:    gets the csim vxd's PM API entrypoint

Information:    

Return:         INT     1 for Success, or
                        0 for Failure.
----------------------------------------------------------------------*/
DWORD csim_api;
int
get_csim_apiproc()
{
    _asm xor di,di;
    _asm mov es,di;
    _asm mov ax, 1684h;
    _asm mov bx,0beefh;    csim vxdid (hack!)
    _asm int 2fh;
    _asm mov ax,es;
    _asm or ax,di;
    _asm jnz GoAhead;
    DPF(DBGLVL_NORMAL,"Could not find csim vxd entry point");
    return 0;

  GoAhead:
    _asm mov word ptr [csim_api],di;
    _asm mov word ptr [csim_api+2],es;
    return 1;

}


/*----------------------------------------------------------------------
Function name:  unloadCsimVxD

Description:    Calls the VXDLDR to dynamically unload the csim vxd.

Information:    

Return:         INT     1 for Success, or
                        0 for Failure.
----------------------------------------------------------------------*/
int
unloadCsimVxD()
{
    csim_api = 0L;    /* clear the PM api entrypoint */

    _asm mov ax,VXDLDR_UnloadDevice;
    _asm mov dx, offset VxDModName;
    _asm mov bx,-1;
    _asm call dword ptr [vxdldr_apiproc];
    _asm jnc GoAhead3;
    DPF(DBGLVL_NORMAL,"Error unloading csim VxD");
    return 0;

  GoAhead3:
    return 1;
}


/*----------------------------------------------------------------------
Function name:  start_csim

Description:    Loads the csim vxd and initializes the
                invariant registers.
Information:    

Return:         INT     1 for Success, or
                        0 for Failure.
----------------------------------------------------------------------*/
int
start_csim()
{
    DWORD screenAddress = _FF(ScreenAddress);
    DWORD nbytes = 0x400000L;

    if (!get_vxdldr_apiproc())
    return 0;

    if (!loadCsimVxd())
    return 0;

    if (!get_csim_apiproc())
    return 0;

    __asm _emit 66h __asm push di;
    __asm _emit 66h __asm mov di, word ptr screenAddress;
    __asm _emit 66h __asm mov ax, word ptr nbytes;
    __asm _emit 66h __asm xor cx, cx;
    __asm mov cx, 1;             //ecx is fn #1 (init simulator)
    __asm call dword ptr [csim_api];
    __asm _emit 66h __asm pop di;

    return 1;
}


WORD is_csim_started = 0;
WORD reload_csim = 0;
WORD csimLoadError = 0;

DWORD funresult;
/*----------------------------------------------------------------------
Function name:  h3WRITE

Description:    Write a Banshee/Avenger HW register.

Information:    Used for actual HW.

Return:         VOID
----------------------------------------------------------------------*/
void FAR _loadds
h3WRITE(unsigned long far * hwptr, DWORD * addr, DWORD data)
{
    __asm
    {
       push    es;
 _emit 0x66 _asm push    di;
 _emit 0x66 _asm push    si;
       mov    ax, word ptr FlatSel;
       mov    es, ax;
 _emit 0x66 _asm mov    di, word ptr addr
 _emit 0x66 _asm mov    cx, word ptr data
// this is what I want
//    mov   es:[edi], ecx
//  but the assembler can't figure that out
//  so I will do it for it......
      _emit 0x67
      _emit 0x66
      _emit 0x26
      _emit 0x89
      _emit 0x0F
 _emit 0x66 _asm pop    si;
 _emit 0x66 _asm pop    di;
       pop    es;
    }

// this makes the cursor damn slow
#if 0
    DPF(DBGLVL_NORMAL,"h3Write: address = 0x%08lx, data = 0x%08lx\n",
    (DWORD)addr,
    (DWORD) data);
#endif

}


/*----------------------------------------------------------------------
Function name:  h3READ

Description:    Read a Banshee/Avenger HW register.

Information:    Used for actual HW.

Return:         ULONG   register value
----------------------------------------------------------------------*/
unsigned long FAR _loadds
h3READ(unsigned long far * hwptr,
       unsigned long far * addr)
{
    FxU32 retval;

    __asm
    {
       push    es;
 _emit 0x66 _asm push    di;
 _emit 0x66 _asm push    si;
       mov    ax, word ptr FlatSel;
       mov    es, ax;
 _emit 0x66 _asm mov    di, word ptr addr
// this is what I want
//    mov   eax, es:[edi]
//  but the assembler can figure that out
//  so I will do it for it......
      _emit 0x67
      _emit 0x66
      _emit 0x26
      _emit 0x8B
      _emit 0x07
 _emit 0x66 _asm mov    word ptr retval, ax
 _emit 0x66 _asm pop    si;
 _emit 0x66 _asm pop    di;
       pop    es;

    }

#if 0
    DPF(DBGLVL_NORMAL,"h3Read: address = 0x%08lx\n", (DWORD)addr);
#endif

    return retval;
}

/*----------------------------------------------------------------------
Function name:  VDDCall

Description:    Calls down into the display driver's VDD.

Information:    

Return:         DWORD   Result returned by the call into the VDD or,
                        -1L if failure.
----------------------------------------------------------------------*/
#pragma optimize("", off)
DWORD VDDCall(DWORD myEAX, DWORD VDDmagicNumber, DWORD myECX, DWORD myEDX, LPVOID esdi)
{
    static DWORD   VDDEntryPoint = 0;
#if 0
    static DWORD   VDDmagicNumber = 0;
#endif
    DWORD   result=0xFFFFFFFF;
    DWORD dwDevNode;

    if (VDDEntryPoint == 0)
    {
    _asm
    {
       xor       di,di           ;//set these to zero before calling
       mov     es,di           ;
       mov     ax,1684h        ;//INT 2FH: Get VxD API Entry Point
       mov     bx,0ah          ;//this is device code forVDD
       int     2fh             ;//call the multiplex interrupt
       mov     word ptr VDDEntryPoint[0],di    ;
       mov     word ptr VDDEntryPoint[2],es    ;//save the returned data
#if 0
       mov     ax, 1683h;
       int       2fh;
       mov     word ptr VDDmagicNumber[0], bx
#endif
    }


    if (VDDEntryPoint == 0)
       return result;
    }

    dwDevNode = DisplayInfo.diDevNodeHandle;

    _asm
    {
    _emit 66h _asm push si                       ;// push esi
    _emit 66h _asm push di                       ;// push edi
    _emit 66h _asm mov ax,word ptr myEAX      ;//eax = function
    _emit 66h _asm mov bx,word ptr VDDmagicNumber   ;//ebx = device
    _emit 66h _asm mov cx,word ptr myECX      ;//ecx = buffer_size
    _emit 66h _asm mov dx,word ptr myEDX         ;//edx = flags
    _emit 66h _asm mov si,word ptr dwDevNode    ;//esi=dwDevNode
    _emit 66h _asm xor di,di                     ;// HIWORD(edi)=0
    les     di, esdi                ;
    call    dword ptr VDDEntryPoint             ;//call the VDDs PM API
    _emit 66h _asm mov word ptr result, ax

      _emit 66h _asm pop di               ;// pop edi
      _emit 66h _asm pop si                        ;// pop esi
    }

    return result;
}
#pragma optimize("", on)


/*----------------------------------------------------------------------
Function name:  DoIntersect

Description:    Evaluate whether or not two rectangles intersect.
                One rectange is always the cursor rect.  Routine
                used bitmask to evalulate all possibilities at once.
Information:    

Return:         DWORD   TRUE  if is intersect or
                        FALSE if not intersect.
----------------------------------------------------------------------*/
int IsIntersect[] = {  // y2 x2 y1 x1
   FALSE,            // 0  0  0  0
   FALSE,            // 0  0  0  1
   FALSE,            // 0  0  1  0
   TRUE,             // 0  0  1  1
   FALSE,            // 0  1  0  0
   FALSE,            // 0  1  0  1
   TRUE,             // 0  1  1  0
   TRUE,             // 0  1  1  1
   FALSE,            // 1  0  0  0
   TRUE,             // 1  0  0  1
   FALSE,            // 1  0  1  0
   TRUE,             // 1  0  1  1
   TRUE,             // 1  1  0  0
   TRUE,             // 1  1  0  1
   TRUE,             // 1  1  1  0
   TRUE,             // 1  1  1  1
   };
#define RANGE(val,lo,hi) ((lo) <= (val) && (val) <= (hi))
int DoIntersect (SHORT left, SHORT top, SHORT right, SHORT bottom)
{
   int bIntersect;
   int cl, cr, ct, cb;

   DEBUG_FIX;


#if 0
   cl = _FF(LastCursorPosX) - _FF(HotspotX);
   cr = cl + (int) SWCURSOR_WIDTH;
   ct = _FF(LastCursorPosY) - _FF(HotspotY);
   cb = ct + (int) SWCURSOR_HEIGHT;
#else
   cl = _FF(LastCursorPosX);
   cr = cl + (int) SWCURSOR_WIDTH;
   ct = _FF(LastCursorPosY);
   cb = ct + (int) SWCURSOR_HEIGHT;
#endif

   if (right - left <= SWCURSOR_WIDTH)
      {
      bIntersect = RANGE(left, cl, cr);
      bIntersect |= (RANGE(right, cl, cr) << 2);
      }
   else
      {
      bIntersect = RANGE(cl, left, right);
      bIntersect |= (RANGE(cr, left, right) << 2);
      }

   if (bottom - top <= SWCURSOR_WIDTH)
      {
      bIntersect |= (RANGE(top, ct, cb) << 1);
      bIntersect |= (RANGE(bottom, ct, cb) << 3);
      }
   else
      {
      bIntersect |= (RANGE(ct, top, bottom) << 1);
      bIntersect |= (RANGE(cb, top, bottom) << 3);
      }
   return IsIntersect[bIntersect];
}

/*----------------------------------------------------------------------
Function name:  Modify_SLI_Read

Description:    Function is used to Enable/Disable SLI Reads
Information:    

Return:         None
                    
----------------------------------------------------------------------*/
#ifdef RD_ABORT_ERROR
void Modify_SLI_Read(DWORD dwRequest)
{
   VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle,
      dwRequest, 0, NULL);
}
#endif
