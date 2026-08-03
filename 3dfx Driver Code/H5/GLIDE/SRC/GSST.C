/*
** Copyright (c) 1995, 3Dfx Interactive, Inc.
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
** $Header: gsst.c, 39, 10/11/00 8:21:51 PM, Brent$
** $Log: 
**  39   3dfx      1.31.1.1.1.410/11/00 Brent           Forced check in to enforce
**       branching.
**  38   3dfx      1.31.1.1.1.310/09/00 Jonny Cochrane  calculate chipScreenHeight
**       correctly for 4 way SLI
**  37   3dfx      1.31.1.1.1.209/29/00 Jonny Cochrane  Fix up jitter values and
**       option to use correct jitter values from glide3
**  36   3dfx      1.31.1.1.1.109/27/00 Jonny Cochrane  8x FSAA and fixes for 4
**       chip 2x FSAA.
** 
**  35   3dfx      1.31.1.1.1.008/31/00 Andy Hanson     Update to support digital
**       SLI through 1600x1200 resolution
**  34   3dfx      1.31.1.1    06/15/00 Adam Briggs     assertDefaultState set
**       windowsInit = FXFALSE.. this has the side effect of making the aaClip hack
**       continue to work when an app changes modes w/o re-initing glide.
**  33   3dfx      1.31.1.0    05/09/00 Kenneth Dyke    Added private undocumented
**       call to get chip SIP values for diags.
**  32   3dfx      1.31        04/25/00 Kenneth Dyke    Init gc->frontBuffer
**       properly for single-buffered apps.
**  31   3dfx      1.30        04/25/00 Kenneth Dyke    Fixed non-AA SLI overlay
**       mode.
**  30   3dfx      1.29        04/14/00 Kenneth Dyke    Don't clobber
**       SST_PARMADJUST when we set the column rendering width.
**  29   3dfx      1.28        04/13/00 Kenneth Dyke    Added support for new-style
**       two-sample AA mode.
**  28   3dfx      1.27        04/03/00 Kenneth Dyke    Added a safety net for DOS
**       diags.
**  27   3dfx      1.26        03/30/00 Kenneth Dyke    Fixed default SLI band
**       height and column width settings.
**  26   3dfx      1.25        03/28/00 Kenneth Dyke    Make sure we use the right
**       shutdown code path so the chips are idle before we kill the command fifo.
**  25   3dfx      1.24        03/28/00 Kenneth Dyke    Added fix for certain dumb
**       games.
**  24   3dfx      1.23        03/25/00 Adam Briggs     added support for
**       SSTH3_SLI_AA_CONFIGURATION (the env var the control panel uses to force AA
**       modes)
**  23   3dfx      1.22        03/23/00 Chris Dow       Added code to limit
**       unfenced writes to 64K for coppermine hang problems.
**  22   3dfx      1.21        03/14/00 Adam Briggs     enable analog sli in 2X
**       modes or when forced
**  21   3dfx      1.20        03/14/00 Adam Briggs     implemented cheesey
**       cliprect workaround for AA artifacts seen in Unreal.
**  20   3dfx      1.19        03/13/00 Kenneth Dyke    Improved idle code.
**  19   3dfx      1.18        03/13/00 Adam Briggs     disabled an incorrect check
**       for available memory
**  18   3dfx      1.17        03/08/00 Adam Briggs     turn on color writes in
**       renderMode when in 32bpp mode
**  17   3dfx      1.16        03/08/00 Adam Briggs     do the correct stride
**       calculation for 32bpp mode
**  16   3dfx      1.15        03/08/00 Kenneth Dyke    Don't cache hardware
**       register pointers.
**  15   3dfx      1.14        03/08/00 Kenneth Dyke    Use new isMapped boardInfo
**       flag instead of broken gc flag.
**  14   3dfx      1.13        03/07/00 Adam Briggs     implemented half-mode sli &
**       aa restrictions
**  13   3dfx      1.12        03/06/00 Kenneth Dyke    Make sure 2PPC mode is off
**       at startup.
**  12   3dfx      1.11        03/01/00 Kenneth Dyke    Fix up pixel format stuff
**       to match Glide3.  This also fixes dither rotation stuff in renderMode.
**  11   3dfx      1.10        02/24/00 Kenneth Dyke    Fix for backwards
**       lostcontext check.
** 
**  10   3dfx      1.9         02/23/00 Kenneth Dyke    Fix Y origin subtraction
**       for Glide2.x.
** 
**  9    3dfx      1.8         02/14/00 Kenneth Dyke    Constrain SLI band height
**       to make sure Y origin swapping works right.
**       Fixed memory requirement calculation to take into account SLI and AA.
** 
**  8    3dfx      1.7         01/31/00 Adam Briggs     changed the IS_NAPALM macro
**       to cooperate with the display driver version of the same
**  7    3dfx      1.6         01/31/00 Adam Briggs     Changed all device ID magic
**       numbers to use those defined in fxhal.h & added IS_NAPALM macro to test
**       against device ID range
**  6    3dfx      1.5         01/30/00 Adam Briggs     got rid of a compiler
**       warning.
**  5    3dfx      1.4         01/28/00 Adam Briggs     glide3->glide2 merge from
**       napalm bringup
**  4    3dfx      1.3         01/16/00 Kenneth Dyke    Fixes for 32-bit rendering.
**  3    3dfx      1.2         12/20/99 James Hunter    Ported Descent 3 Alt+Tab
**       PRS #9156 fix from Voodoo 3 glide
**  2    3dfx      1.1         11/30/99 Adam Briggs     Fixed yorigin swapping for
**       single chip... still needs multi-chip code tho
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
** 
** 238   7/30/99 3:33p Kcd
** Oops... the "real" fix.
** 
** 237   7/30/99 3:30p Kcd
** Minor fix for H3 build.
** 
** 236   7/29/99 5:04p Adamb
** Supported FX_GLIDE_AA_SAMPLE env var to force 2 & 4 sample AA. Also
** made sli work.
** 
** 235   7/27/99 3:52p Adamb
** duplicated rendermode defines from & removed reference to g3ext.h
** 
** 234   7/27/99 11:16a Adamb
** support for FX_GLIDE_BPP env var to force 32bpp renderMode
** 
** 233   7/18/99 2:29p Atai
** change 2ppc enabling condition
** 
** 232   7/18/99 11:12a Atai
** added 2 pixel per clock
** 
** 231   7/13/99 2:14p Atai
** fixed direct write buffer initialization
** 
** 230   7/12/99 12:34p Atai
** initialize tmu base address for napalm
** 
** 229   7/02/99 4:40p Kcd
** Handle PowerPC byte swizzling in glfb.c.
** 
** 228   6/25/99 1:16p Atai
** fixed with 2 buffers per chip
** 
** 227   6/18/99 1:15p Kcd
** PowerPC PCI Bump & Grind stuff.
** 
** 226   6/16/99 7:27p Larryw
** Fixed a misleading comment.
** 
** 225   6/14/99 4:28p Atai
** more on 2nd buffer allocation
** 
** 224   6/14/99 3:18p Atai
** added secondary buffer info
** 
** 223   6/03/99 2:25p Atai
** set default chipMask to 0xffffffff
** 
** 222   6/03/99 2:24p Atai
** set default chipMask to 0xffff
** 
** 220   5/26/99 3:18p Kcd
** Turn on byte/word swizzling by default for PowerPC systems.
** 
** 219   4/29/99 10:36p Atai
** fake context for csim
** 
** 218   4/13/99 2:45p Dow
** Fixed bogus return from lostContext in WinClose
** 
** 217   4/12/99 1:54p Atai
** fixed dos build
** 
** 216   4/12/99 1:35p Dow
** Fixed grSstWinClose bogus check of lostcontext
** 
** 215   4/08/99 5:32p Dow
** Ok.  A little more cleanup.
** 
** 214   4/08/99 5:30p Dow
** Check gc->lostcontext in winclose and do things a little different.
** 
** 213   4/08/99 5:21p Dow
** More cleanup
** 
** 212   4/08/99 5:14p Dow
** more oem crap
** 
** 211   4/08/99 5:14p Dow
** Cleaned OEMInit crap
** 
** 210   4/06/99 9:57a Atai
** remove un-used variable
** 
** 209   4/05/99 10:35p Atai
** added code to check context. exit if alt-tab happens
** 
** 208   3/26/99 12:01p Atai
** set renderMode if y origin is lower left
** 
** 207   3/22/99 12:15p Atai
** win32 only
** 
** 206   3/19/99 7:27p Atai
** avoid multiple load in grSstWinOpen
** 
** 205   3/17/99 5:08p Peter
** removed whacky stuff now that the command fifo threshold stuff appears
** to make all happy (including the k7)
** 
** 204   3/17/99 1:22p Dow
** it really should be an assignment
** 
** 203   3/17/99 10:54a Dow
** fixed euivalence test for autoBump
** 
** 202   3/16/99 11:51a Atai
** Back door (set FX_GLIDE_ENABLE_UMA=1) to enable unified texture memory.
** TMUn memory size will the whole texture memory space. The offset for
** each TMU points to the start address of the memory pool.
** 
** 201   3/15/99 10:51p Dow
** Vile Hack
** 
** 200   3/13/99 9:48p Dow
** optimizations for B&G
** 
** 199   3/10/99 6:38p Peter
** removed cruft
** 
** 198   3/10/99 10:40a Peter
** bump-n-grind workaround for katmai until the bug is better
** characterized
** 
** 197   3/06/99 10:42a Dow
** Added an idle to make sure.
** 
** 196   3/05/99 9:01a Atai
** for direct write compilation
** 
** 195   3/04/99 5:42p Atai
** cast pointer
** 
** 194   3/04/99 3:15p Atai
** mods for direct write
** 
** 193   3/03/99 4:44p Peter
** clear chroma range and texChroma for default state
** 
** 192   2/27/99 12:24p Dow
** new resolutions
** 
** 191   2/26/99 10:27a Peter
** Mmmm.... 8.3
** 
** 190   2/19/99 7:58p Peter
** doh!
** 
** 189   2/19/99 7:56p Peter
** load splash plug-in even w/ env var set
** 
** 188   2/19/99 5:54p Peter
** new splash screen
** 
** 187   2/13/99 1:59p Dow
** Added code for new resolutions
** 
** 186   12/21/98 1:47p Atai
** Update gc->swapsPending before we return the status, fixed for the
** games (such as Sentinel Returns) that rely on masking status to get
** buffernumpending.
** 
** 185   12/18/98 1:55p Atai
** Hack for Quake2 gamma setting. "SSTV2_GAMMA" is used to update the
** video/brightness setting.
** 
** 184   12/09/98 6:02p Atai
** grTexCombine did the right thing for the un-used TMU. Initialize the
** 2nd TMU value to take care of "set FX_GLIDE_NUM_TMU=1"
** 
** 183   12/09/98 5:00p Atai
** set MAXLOD = MINLOD = 8 in _grUpdateParamIndex if ST1 is not used
** 
** 182   12/09/98 2:02p Atai
** Added _grTexForceLod back. Set tLOD min = max = 8 for the 2nd TMU by
** default for Avenger to increase single texturing tri fillrate.
** 
** 181   12/03/98 11:27p Dow
** Code 'cleanup' he�
** 
** 180   12/03/98 10:37p Dow
** Removed bogus gamma setting in assertDefaultState\nRemoved protoected &
** vestigial Glide3 funcions
** 
** 179   11/18/98 8:00p Dow
** grxclk
** 
** 178   11/18/98 4:27p Atai
** direct write mods. 2 tmu mode does not work yet
** 
** 177   11/14/98 2:00a Lab
** fixed second tmu size
** 
** 176   11/10/98 6:44p Atai
** number of tmu and texture memory allocation
** 
** 175   11/10/98 4:53p Atai
** fixed gc->tramoffset
** 
** 174   11/02/98 5:34p Atai
** merge direct i/o code
** 
** 173   10/21/98 4:20p Atai
** gamma stuff
** 
** 172   10/21/98 10:41a Atai
** mod for HAL_CSIM
** 
** 171   10/20/98 5:34p Atai
** added #ifdefs for hwc
** 
** 170   10/20/98 4:39p Atai
** update tramOffset and tramSize
** 
** 169   10/13/98 8:44p Dow
** Made it work for 4MB boards
** 
** 168   10/09/98 6:57p Peter
** dynamic 3DNow!(tm) mods
** 
** 167   10/08/98 10:15a Dow
** Triple Buffering fix
** 
** 166   9/18/98 10:53a Dow
** VidMode Stuff
** 
** 165   9/04/98 11:36a Peter
** re-open fix for nt (thanks to taco/rob/nt bob)
** 
** 164   9/02/98 1:34p Peter
** watcom warnings
** 
** 163   9/01/98 10:07a Dow
** Fixed Control panel effage
** 
** 162   8/28/98 4:38p Atai
** 1. added MIN_FIFO_SIZE for memory checking
** 2. hack for resolution checking if we have 8M board and triple
** buffering on
** 
** 161   8/27/98 9:54p Peter
** use gc->frontBuffer rather than 1
** 
** 160   8/27/98 6:36p Atai
** check if environment.tmuMemory is on
** 
** 159   8/26/98 10:11p Atai
** set color buffer to 3 if control panel force triple buffering
** 
** 158   8/03/98 4:07p Mikec
** Removed glide3 splash screen.
** 
** 157   8/03/98 10:45a Atai
** rename 3dfxsplash3.dll to 3dfxspl3.dll
** 
** 156   7/29/98 3:09p Dow
** SDRAM Fixes
** 
** 155   7/24/98 2:03p Dow
** AGP Stuff
** 
** 154   7/23/98 1:17a Dow
** Bump & Grind
** 
** 153   7/15/98 2:03p Mikec
** Fixed Lfb depth buffer readback problem. It turns out grWinOpen didn't
** set up the lfb buffer pointer for the aux buffer correctly. 
** 
** 152   7/06/98 7:05p Jdt
** grSstWinOpen simplified, in-memory buffer logic added
** 
** 151   6/26/98 11:09a Jdt
** Changes to make simulator work.
** 
** 150   6/26/98 10:08a Jdt
** Protected some code to enable CSIM builds.
** 
** 149   6/25/98 6:49p Jdt
** Removed direct IO write from grSstWinOpen and moved to minihwc
** 
** 148   6/24/98 11:16a Dow
** More SST1 Statuws stuff
** 
** 147   6/23/98 2:52p Dow
** Added SST1 FIFO status.
** 
** 146   6/19/98 2:33p Mikec
** Fixed lfb buffer offset fucakge
** 
** 145   6/16/98 6:12p Dow
** Rearranged texture memory
** 
** 144   6/12/98 4:20p Dow
** Return FXFALSE from WinOpen if GR_RESOLUTION_NONE is specified.
** 
** 143   6/10/98 9:49a Peter
** lfb buffer addressing
** 
** 142   6/09/98 7:48p Mikec
** Added buffer clears to grSstWinOpen to get rid of random pixels.
** 
** 141   6/05/98 11:22a Jeske
** we never copied the hwc's memory calculation into gc->fbOffset... We
** might need to copy more data than this.. check this...
** 
** 140   6/04/98 12:12p Peter
** splash dll rename
** 
** 139   5/31/98 9:03a Dow
** 800x600 resolution
** 
** 138   5/28/98 1:46p Dow
** Swap Pending Workaround
** 
** 137   5/22/98 2:37p Peter
** wingcommander effage
** 
** 136   5/21/98 5:27p Peter
** hwcInitVideo failing is fatal for grSstWinOpen
** 
** 135   5/13/98 9:12a Dow
** More mods to make CSIM work
** 
** 134   5/12/98 2:42p Dow
** CSIM Stuff
** 
** 133   4/30/98 4:10p Peter
** Fixed comment from before
** 
** 132   4/19/98 11:31a Dow
** Fixed grSstStatus
** 
** 131   4/14/98 6:41p Peter
** Merge w/ cvg glide cleanup
** 
** 130   4/07/98 10:40p Dow
** LFB Fixes:  Round 1
** 
** 129   3/28/98 11:24a Dow
** itwo�
** 
** 128   3/20/98 1:12p Dow
** Windows happiness
** 
** 127   3/16/98 8:47p Dow
** More Windows Happiness
** 
** 125   3/03/98 11:31a Dow
** Resolutions now work
** 
** 124   2/17/98 12:52p Dow
** fifo setup code
** 
** 123   2/09/98 12:27p Dow
** Fixed some niggling hangnails
** 
** 122   2/08/98 3:08p Dow
** FIFO Works
** 
** 121   2/02/98 4:31p Dow
** IO w/o HAL now possible
** 
** 120   1/30/98 1:34p Dow
** Formed call to fxHalInitVideoOverlaySurface
** 
** 119   1/29/98 9:54p Dow
** This is Banshee
** 
** 118   1/20/98 11:03a Peter
** env var to force triple buffering
 * 
 * 116   1/16/98 4:18p Atai
 * fixed lfb and grLoadGammaTable
 * 
 * 115   1/16/98 10:47a Peter
 * fixed idle effage
 * 
 * 114   1/16/98 10:16a Atai
 * fixed grSstIldle
 * 
 * 113   1/10/98 4:01p Atai
 * inititialize vertex layout, viewport, added defines
 * 
 * 110   1/06/98 6:47p Atai
 * undo grSplash and remove gu routines
 * 
 * 109   1/06/98 3:53p Atai
 * remove grHint, modify grLfbWriteRegion and grGet
 * 
 * 107   12/18/97 2:12p Peter
 * grSstControl on v2
 * 
 * 106   12/17/97 4:48p Peter
 * groundwork for CrybabyGlide
 * 
 * 105   12/17/97 4:06p Atai
 * added grChromaRange(), grGammaCorrecionRGB(), grRest(), and grGet()
 * functions
 * 
 * 104   12/16/97 1:33p Atai
 * added grGammaCorrectionRGB()
 * 
 * 103   12/16/97 10:03a Atai
 * fixed gutexmemreset for glide2
 * 
 * 101   12/09/97 12:20p Peter
 * mac glide port
 * 
 * 100   12/05/97 4:26p Peter
 * watcom warnings
 * 
 * 99    12/03/97 11:35a Peter
 * is busy thing
 * 
 * 98    11/25/97 12:09p Peter
 * nested calls to grLfbLock vs init code locking on v2
 * 
 * 97    11/21/97 1:02p Peter
 * v^2 supported resolutions
 * 
 * 96    11/21/97 11:19a Dow
 * Added RESOLUTION_NONE hack for Banshee
 * 
 * 95    11/19/97 2:49p Peter
 * env vars in registry for win32
 * 
 * 94    11/19/97 2:22p Dow
 * gsst.c
 * 
 * 93    11/18/97 4:50p Peter
 * chipfield stuff cleanup and w/ direct writes
 * 
 * 92    11/18/97 4:00p Atai
 * fixed the GR_BEGIN and GR_END error in previous check-in
 * 
 * 91    11/18/97 3:27p Atai
 * update vData 
 * optimize state monster
 * 
 * 90    11/17/97 4:55p Peter
 * watcom warnings/chipfield stuff
 * 
 * 89    11/16/97 2:20p Peter
 * cleanup
 * 
 * 88    11/15/97 7:43p Peter
 * more comdex silliness
 * 
 * 87    11/14/97 11:10p Peter
 * open vs hw init confusion
 * 
 * 86    11/14/97 5:02p Peter
 * more comdex stuff
 * 
 * 85    11/14/97 4:47p Dow
 * New splash screen
 * 
 * 84    11/14/97 12:09a Peter
 * comdex thing and some other stuff
 * 
 * 83    11/12/97 9:37p Dow
 * Banshee crap
 * 
 * 82    11/12/97 2:27p Peter
 * simulator happiness w/o fifo
 * 
 * 81    11/12/97 1:09p Dow
 * H3 Stuf
 * 
 * 80    11/12/97 9:22a Dow
 * H3 Mods
 * 
 * 79    11/06/97 3:46p Peter
 * sli shutdown problem
 * 
 * 78    11/06/97 3:38p Dow
 * More banshee stuff
 * 
 * 77    11/04/97 5:04p Peter
 * cataclysm part deux
 * 
 * 76    11/04/97 3:58p Dow
 * Banshee stuff
 * 
 * 75    11/03/97 3:43p Peter
 * h3/cvg cataclysm
 * 
 * 74    10/29/97 4:59p Peter
 * fixed csim/non-debug stupidity
 * 
 * 73    10/29/97 2:45p Peter
 * C version of Taco's packing code
 * 
 * 72    10/23/97 5:28p Peter
 * sli fifo thing
 * 
 * 71    10/17/97 3:15p Peter
 * grSstVidMode thingee
 * 
 * 70    10/14/97 2:44p Peter
 * moved close flag set
 * 
 * 69    10/09/97 8:02p Dow
 * State Monster 1st Cut
 * 
**
*/

#include <stdio.h>
#include <string.h>
#include <3dfx.h>

#include <glidesys.h>


#if (GLIDE_PLATFORM & GLIDE_HW_SST96) 
#include <init.h>
#endif

#define FX_DLL_DEFINITION
#include <fxdll.h>
#include <glide.h>
#include "fxglide.h"

#if (GLIDE_PLATFORM & GLIDE_OS_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#if (GLIDE_PLATFORM & GLIDE_SST_SIM)
#ifdef HAL_CSIM
#include <csim.h>
static FxU32 lostcontext_csim;
#elif HSIM
#include <gsim.h>
#endif
#endif /* (GLIDE_PLATFORM & GLIDE_SST_SIM) */

#define kPageBoundarySlop 0x1000UL
#define kPageBoundaryMask (kPageBoundarySlop - 1)

/* Compatibility hack */
extern int winCheck(void);

/* Some forward declarations */
#ifdef FX_GLIDE_NAPALM
static void _grSstSetColumnsOfNWidth(FxU32 width);
static void _grDisableSliCtrl(void) ;
#endif /* FX_GLIDE_NAPALM */

/* Init hw */

typedef struct {
    GrScreenResolution_t resolution;
    FxU32                xres;
    FxU32                yres;
} ResEntry;

static ResEntry _resTable[] = {
  GR_RESOLUTION_320x200, 320, 200, /* 0x0 */
  GR_RESOLUTION_320x240, 320,  240, /*   0x1 */
  GR_RESOLUTION_400x256, 400,  256, /*   0x2 */
  GR_RESOLUTION_512x384, 512,  384, /*   0x3 */
  GR_RESOLUTION_640x200, 640,  200, /*   0x4 */
  GR_RESOLUTION_640x350, 640,  350, /*   0x5 */
  GR_RESOLUTION_640x400, 640,  400, /*   0x6 */
  GR_RESOLUTION_640x480, 640,  480, /*   0x7 */
  GR_RESOLUTION_800x600, 800,  600, /*   0x8 */
  GR_RESOLUTION_960x720, 960,  720, /*   0x9 */
  GR_RESOLUTION_856x480, 856,  480, /*   0xa */
  GR_RESOLUTION_512x256, 512,  256, /*   0xb */
  GR_RESOLUTION_1024x768, 1024,  768, /*  0xC */
  GR_RESOLUTION_1280x1024, 1280,  1024, /* 0xD */
  GR_RESOLUTION_1600x1200, 1600,  1200, /* 0xE */
  GR_RESOLUTION_400x300, 400,  300, /*   0xF */
  GR_RESOLUTION_1152x864, 1152,  864, /*  0x10 */
  GR_RESOLUTION_1280x960, 1280,  960, /*  0x11 */
  GR_RESOLUTION_1600x1024, 1600,  1024, /* 0x12 */
  GR_RESOLUTION_1792x1344, 1792,  1344, /* 0x13 */
  GR_RESOLUTION_1856x1392, 1856,  1392, /* 0x14 */
  GR_RESOLUTION_1920x1440, 1920,  1440, /* 0x15 */
  GR_RESOLUTION_2048x1536, 2048,  1536, /* 0x16 */
  GR_RESOLUTION_2048x2048, 2048,  2048 /* 0x17 */
};


/* ---------------------------------------------
   This function both sets and documents the 
   expected default state for any rendering
   context

   ..taco - separated out in preparation for
   multiple contexts
   ---------------------------------------------*/
static void 
assertDefaultState( void ) 
{
#define FN_NAME "assertDefaultState"
  GR_DCL_GC;
  GrTexInfo textureinfo = { GR_LOD_1, GR_LOD_1, 
                            GR_ASPECT_1x1, GR_TEXFMT_8BIT, 0 };

  _GlideRoot.windowsInit = FXFALSE ;

  grHints(GR_HINT_ALLOW_MIPMAP_DITHER, 0);
  grSstOrigin(gc->state.origin);
  grAlphaBlendFunction(GR_BLEND_ONE , GR_BLEND_ZERO, 
                       GR_BLEND_ONE, GR_BLEND_ZERO);
  grAlphaTestFunction(GR_CMP_ALWAYS);
  grAlphaTestReferenceValue(0);
  grChromakeyMode(GR_CHROMAKEY_DISABLE);

  grConstantColorValue((FxU32) ~0);
  grClipWindow(0, 0, gc->state.screen_width, 
               gc->state.screen_height);
  _grColorCombineDelta0Mode(FXFALSE);
  grColorCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
                 GR_COMBINE_FACTOR_ONE,
                 GR_COMBINE_LOCAL_ITERATED,
                 GR_COMBINE_OTHER_ITERATED,
                 FXFALSE);
  grAlphaCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
                 GR_COMBINE_FACTOR_ONE,
                 GR_COMBINE_LOCAL_NONE,
                 GR_COMBINE_OTHER_CONSTANT,
                 FXFALSE);
  grColorMask(FXTRUE, FXFALSE);
  grCullMode(GR_CULL_DISABLE);
  grDepthBiasLevel(0);
  grDepthMask(FXFALSE);
  grDepthBufferMode(GR_DEPTHBUFFER_DISABLE);
  grDepthBufferFunction(GR_CMP_LESS);
  grDepthBiasLevel(0);
  grDitherMode(GR_DITHER_4x4);
  grFogMode(GR_FOG_DISABLE);
  grFogColorValue(0x00000000);

  /* Get rid of crap in the buffers. */
  grSstIdle();
  if ( gc->state.num_buffers > 1 ) {
    grBufferClear( 0, 0, GR_WDEPTHVALUE_FARTHEST );
    grBufferSwap( 1 );
    grBufferClear( 0, 0, GR_WDEPTHVALUE_FARTHEST );
    grBufferSwap( 1 );
    grBufferClear( 0, 0, GR_WDEPTHVALUE_FARTHEST );
    grBufferSwap( 1 );
    grRenderBuffer( GR_BUFFER_BACKBUFFER );
  } else {
    grBufferClear( 0, 0, GR_WDEPTHVALUE_FARTHEST );
    grRenderBuffer( GR_BUFFER_FRONTBUFFER );
  }
 


  guTexMemReset();

  /* Napalm-specific initialization kruft */
  /* Do this before the TMUs are set up to make sure
   * we don't broadcast TMU0's config to TMU1. */
  if (IS_NAPALM(gc->bInfo->pciInfo.deviceID)) {
    FxU32 do2ppc = gc->do2ppc;
    /* Fake out the grTex2ppc code into doing our bidding. */
    gc->do2ppc = FXTRUE;
    gc->state.fbi_config.combineMode |= SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK;
    _grSstSetColumnsOfNWidth(8); /* 8 is default for now */    
    _grTex2ppc(GR_TMU0,FXFALSE);
    gc->do2ppc = do2ppc;
  }

  switch (gc->num_tmu) {
  case 2:
    grTexClampMode(GR_TMU1, GR_TEXTURECLAMP_CLAMP, GR_TEXTURECLAMP_CLAMP);
    grTexDetailControl(GR_TMU1, 0, 1, 1.0F);
    grTexFilterMode(GR_TMU1, GR_TEXTUREFILTER_POINT_SAMPLED, 
                    GR_TEXTUREFILTER_POINT_SAMPLED);
    grTexLodBiasValue(GR_TMU1, 0.0F);
    grTexMipMapMode(GR_TMU1, GR_MIPMAP_DISABLE, FXFALSE);
    grTexCombine(GR_TMU1, GR_COMBINE_FUNCTION_ZERO, GR_COMBINE_FACTOR_NONE,
                 GR_COMBINE_FUNCTION_ZERO, GR_COMBINE_FACTOR_NONE,
                 FXFALSE, FXFALSE);
    /* 
    ** napalm glide place its fifo at low address 0, to prevent tmu read
    ** from fifo, we initialize the tmu base address to its min address
    */
    if (IS_NAPALM(gc->bInfo->pciInfo.deviceID)) {
      grTexSource(GR_TMU1,
                  grTexMinAddress(GR_TMU1),
                  GR_MIPMAPLEVELMASK_BOTH,
                  &textureinfo);
    }
  case 1:
    grTexClampMode(GR_TMU0, GR_TEXTURECLAMP_CLAMP, GR_TEXTURECLAMP_CLAMP);
    grTexDetailControl(GR_TMU0, 0, 1, 1.0F);
    grTexFilterMode(GR_TMU0, GR_TEXTUREFILTER_POINT_SAMPLED, 
                    GR_TEXTUREFILTER_POINT_SAMPLED);
    grTexLodBiasValue(GR_TMU0, 0.0F);
    grTexMipMapMode(GR_TMU0, GR_MIPMAP_DISABLE, FXFALSE);
    grTexCombine(GR_TMU0, GR_COMBINE_FUNCTION_ZERO, GR_COMBINE_FACTOR_NONE,
                 GR_COMBINE_FUNCTION_ZERO, GR_COMBINE_FACTOR_NONE,
                 FXFALSE, FXFALSE);
    if (IS_NAPALM(gc->bInfo->pciInfo.deviceID)) {
      grTexSource(GR_TMU0,
                  grTexMinAddress(GR_TMU0),
                  GR_MIPMAPLEVELMASK_BOTH,
                  &textureinfo);
    }
  }
  grLfbConstantAlpha(0xFF);
  grLfbConstantDepth(0);

  /* Glide2x does not expose texture chroma or chroma ranges, but
   * glide3x does so we need to make sure that those bits are clear by
   * default too so that the bits don't remain set by accident.
   */
  {
    FxI32 i;

    GR_SET_EXPECTED_SIZE(((gc->num_tmu * 2) + 1) * sizeof(FxU32), (gc->num_tmu * 2) + 1);

    GR_SET(BROADCAST_ID, (SstRegs *)gc->reg_ptr, chromaRange, 0x00UL);

    for(i = 0; i < gc->num_tmu; i++) {
      GR_SET((0x02UL << i), (SstRegs *)gc->reg_ptr, chromaKey, 0x00UL);
      GR_SET((0x02UL << i), (SstRegs *)gc->reg_ptr, chromaRange, 0x00UL);
    }
    GR_CHECK_SIZE();
  }

#undef FN_NAME
} /* assertDefaultState */

static void 
doSplash(void)
{
  GR_DCL_GC;

#if (GLIDE_PLATFORM & GLIDE_OS_WIN32)
  {
    FxBool
      didLoad;

    gc->pluginInfo.moduleHandle = LoadLibrary("3dfxspl2.dll");
    didLoad = (gc->pluginInfo.moduleHandle != NULL);
    if (didLoad) {
      gc->pluginInfo.initProc = (GrSplashInitProc)GetProcAddress(gc->pluginInfo.moduleHandle, 
                                                                 "_fxSplashInit@24");
      gc->pluginInfo.shutdownProc = (GrSplashShutdownProc)GetProcAddress(gc->pluginInfo.moduleHandle, 
                                                                         "_fxSplashShutdown@0");
      gc->pluginInfo.splashProc = (GrSplashProc)GetProcAddress(gc->pluginInfo.moduleHandle, 
                                                               "_fxSplash@20");
      gc->pluginInfo.plugProc = (GrSplashPlugProc)GetProcAddress(gc->pluginInfo.moduleHandle, 
                                                                 "_fxSplashPlug@16");

      didLoad = ((gc->pluginInfo.initProc != NULL) &&
                 (gc->pluginInfo.splashProc != NULL) &&
                 (gc->pluginInfo.plugProc != NULL) &&
                 (gc->pluginInfo.shutdownProc != NULL));
      if (didLoad) {
        didLoad = (*gc->pluginInfo.initProc)(gc->grHwnd,
                                             gc->state.screen_width, gc->state.screen_height,
                                             gc->grColBuf, gc->grAuxBuf,
                                             gc->state.color_format);
        if (!didLoad) (*gc->pluginInfo.shutdownProc)();
      }
      
      if (!didLoad) FreeLibrary(gc->pluginInfo.moduleHandle);
    }

    /* Clear all the info if we could not load for some reason */
    if (!didLoad) memset(&gc->pluginInfo, 0, sizeof(gc->pluginInfo));
  }
#endif /* (GLIDE_PLATFORM & GLIDE_OS_WIN32) */

  if (_GlideRoot.environment.noSplash == 0) {
    grSplash(0.0f, 0.0f, 
             (float)gc->state.screen_width,
             (float)gc->state.screen_height,
             0);
  }
  _GlideRoot.environment.noSplash = 1;
} /* doSplash */


/*----------------------------------------------------
  Return a GC to reset state
  
  ...taco - separated out as a first pass since this will
  be common function per context
  ----------------------------------------------------*/
static void initGC ( GrGC *gc ) {
    FxI32 t = 0;

    gc->curBuffer   = 0;
    gc->frontBuffer = (gc->nColBuffers > 1) ? 1 : 0;
    gc->backBuffer  = (gc->nColBuffers > 2) ? 2 : gc->curBuffer;

    for (t = 0; t < 7; t++) {
      gc->bufferSwaps[t] = 0xffffffff;
    }

    gc->bufferSwaps[0] =
        (FxU32) gc->cmdTransportInfo.fifoPtr -
        (FxU32) gc->cmdTransportInfo.fifoStart; 
    
    gc->swapsPending = 1;
    
    gc->lockPtrs[GR_LFB_READ_ONLY]  = (FxU32)-1;
    gc->lockPtrs[GR_LFB_WRITE_ONLY] = (FxU32)-1;
    /* Initialize the read/write registers to all 0 */
    gc->state.fbi_config.fbzColorPath  = 0;
    gc->state.fbi_config.fogMode       = 0;
    gc->state.fbi_config.alphaMode     = 0;
    gc->state.fbi_config.fbzMode       = 0;
    gc->state.fbi_config.lfbMode       = 0;
    gc->state.fbi_config.clipLeftRight = 0;
    gc->state.fbi_config.clipBottomTop = 0;
    gc->state.fbi_config.fogColor      = 0;
    gc->state.fbi_config.zaColor       = 0;
    gc->state.fbi_config.chromaKey     = 0;
    gc->state.fbi_config.stipple       = 0;
    gc->state.fbi_config.color0        = 0;
    gc->state.fbi_config.color1        = 0;
    for (t = 0; t < gc->num_tmu; t += 1) {
      FxU32 textureMode = 
        (!IS_NAPALM(gc->bInfo->pciInfo.deviceID)) ? (FxU32)SST_SEQ_8_DOWNLD : 0x00000000;
      if ((_GlideRoot.hwConfig.SSTs[_GlideRoot.current_sst].type == GR_SSTTYPE_VOODOO) && 
          (_GlideRoot.hwConfig.SSTs[_GlideRoot.current_sst].sstBoard.VoodooConfig.tmuConfig[t].tmuRev == 0)) {
        textureMode = 0;
      }
      gc->state.tmu_config[t].textureMode     = textureMode;
      gc->state.tmu_config[t].tLOD            = 0x00000000;
      gc->state.tmu_config[t].tDetail         = 0x00000000;
      gc->state.tmu_config[t].texBaseAddr     = 0x00000000;
      gc->state.tmu_config[t].texBaseAddr_1   = 0x00000000;
      gc->state.tmu_config[t].texBaseAddr_2   = 0x00000000;
      gc->state.tmu_config[t].texBaseAddr_3_8 = 0x00000000;
      gc->state.tmu_config[t].mmMode          = GR_MIPMAP_NEAREST;
      gc->state.tmu_config[t].smallLod        = G3_LOD_TRANSLATE(GR_LOD_1);
      gc->state.tmu_config[t].largeLod        = G3_LOD_TRANSLATE(GR_LOD_1);
      gc->state.tmu_config[t].evenOdd         = GR_MIPMAPLEVELMASK_BOTH;
      gc->state.tmu_config[t].nccTable        = GR_NCCTABLE_NCC0;
    } 
} /* initGC */


/*-------------------------------------------------------------------
  Function: grSstWinOpen
  Date: 3/16
  Implementor(s): dow, gmt, murali, jdt
  Mutator: dpc
  Library: Glide
  Description:

  Initialize the selected SST

  Arguments:
  hwnd - pointer to a window handle or null.  If NULL, then 
         the application window handle will be inferred though
         the GetActiveWindow() api.
  resolution - either one of the pre-defined glide resolutions,
               or GR_RESOLUTION_NONE, in which case the window
               size is inferred from the size application window
  refresh - requested fullscreen refresh rate, ignored in a window
  format  - requested ccolor format for glide packed color values
  origin  - location of coordinate origin either upper left or
                    lower left
  nColBuffers - number of color buffers to attempt to allocate
                0 - meaningless
                1 - allocate a front buffer only
                2 - allocate a front and back buffer
                3 - allocate a front, back, aux buffer for tripple buffering
  nAuxBuffers - number of aux buffers to attempt to allocate
                0 - no alpha or z buffers
                1 - allocate one aux buffer for alpha/depth buffering
                2 - allocate on depth and one alpha buffer (unsup)
  Return:
  FXTRUE - glide successfully acquired the necessary resources and a
           is ready for rendering
  FXFALSE - glide was unsuccessful in getting the necessary resources, 
            or the requested configuration is unavailble on the host
            hardware - any calls to glide rendering routines will result
            in undefined behavior.
  -------------------------------------------------------------------*/
GR_ENTRY(grSstWinOpen, FxBool, ( FxU32                   hWnd,
                                 GrScreenResolution_t    resolution, 
                                 GrScreenRefresh_t       refresh, 
                                 GrColorFormat_t         format, 
                                 GrOriginLocation_t      origin, 
                                 int                     nColBuffers,
                                 int                     nAuxBuffers))
{
#define FN_NAME "grSstWinOpen"
#define TILE_WIDTH_PXLS   64
#define TILE_HEIGHT_PXLS  32
#define BYTES_PER_PIXEL   2
#define MIN_TEXTURE_STORE 0x200000
#define MIN_FIFO_SIZE     0x10000
#if defined( GLIDE_INIT_HWC )
  hwcBoardInfo  *bInfo   = 0;
  hwcVidInfo    *vInfo   = 0;
  hwcBufferInfo *bufInfo = 0;
  hwcFifoInfo   *fInfo   = 0;
  FxU32         hwPixelFormat = SST_OVERLAY_PIXEL_RGB565D;
  FxU32         pixelformat = GR_PIXFMT_RGB_565;

#elif defined( GLIDE_INIT_HAL )
  FxDeviceInfo   devInfo;
  hwcBoardInfo  *bInfo   = 0;
  hwcVidInfo    *vInfo   = 0;
  hwcBufferInfo *bufInfo = 0;
  hwcFifoInfo   *fInfo   = 0;
#endif /* defined ( GLIDE_INIT_HAL ) */
  /* #if USE_PACKET_FIFO */
    int            buffer;
    /* #endif */
    struct cmdTransportInfo *gcFifo = 0;

    GR_BEGIN_NOFIFOCHECK_NORET("grSstWinOpen",80);
    GDBG_INFO_MORE(gc->myLevel,
                   "(rez=%d,ref=%d,cformat=%d,origin=%s,#bufs=%d, #abufs=%d)\n",
                   resolution,refresh,format,
                   origin ? "LL" : "UL",
                   nColBuffers, nAuxBuffers);
    GR_CHECK_F(FN_NAME, !gc, "no SST selected as current (gc==NULL)");
    
#if (GLIDE_PLATFORM & GLIDE_OS_DOS32)
    /* The manufacturing diags are evil and don't always close down
     * the hardware the right way.  In theory grSstWinOpen() should
     * be able to deal with this without having to resort to grSstWinClose().
     * On the other hand, I don't have time to figure out what the 
     * problem is, and I can't make Glide foolproof from poorly written
     * software. %%KCD
     */
    if(gc->open) {
      grSstWinClose();
    } 
#endif
       
#if 1
    /* ??? the eff is this about */
    if ((nColBuffers == 2) &&
        (nAuxBuffers == 0)) nAuxBuffers = 1;
#endif

    /* Certain games don't work right in anything but 640x480
     * even if they think they do. */
    if(winCheck() && resolution != GR_RESOLUTION_640x480) {
      return FXFALSE;
    }    
    
#ifdef GLIDE_INIT_HWC
    /*
    ** check if the environment variable for triple buffering is on
    */
    if (_GlideRoot.environment.nColorBuffer != -1) {
      if (_GlideRoot.environment.nColorBuffer > 0 &&
          _GlideRoot.environment.nColorBuffer <= 3) {
        nColBuffers = _GlideRoot.environment.nColorBuffer;
      }
    }
#else
    /* H4 CSIM Note: double buffering for now */
#endif

    resolution =
      (((FxU32)resolution) > (sizeof(_resTable) / sizeof(ResEntry)))
        ? GR_RESOLUTION_640x480 
          : resolution;

#ifdef GLIDE_INIT_HWC
    /*
    ** hack!!!
    ** if we have 8M board and triple buffering, reject resolutions greater than 800x600
    */
    if ((nColBuffers == 3) && (gc->bInfo->h3Mem == 8)) {
      switch (resolution) {
      case GR_RESOLUTION_960x720:
      case GR_RESOLUTION_1024x768:
      case GR_RESOLUTION_1280x1024:
      case GR_RESOLUTION_1600x1200:
        GDBG_INFO( gc->myLevel, "Failed to open for insufficient memory\n" );
        GrErrorCallback( "grSstWinOpen: not enough memory for requested buffers", FXFALSE );
        GR_RETURN( FXFALSE );
        break;
      default:
        break;
      }
    }
#endif

    gc->state.screen_width  = _resTable[resolution].xres;
    gc->state.screen_height = _resTable[resolution].yres;
    gc->nColBuffers = nColBuffers;
    GR_CHECK_F( FN_NAME, resolution != _resTable[resolution].resolution, 
                "resolution table compilation incorrect" );
    if ( gc->vidTimings ) {
        gc->state.screen_width  = gc->vidTimings->xDimension;
        gc->state.screen_height = gc->vidTimings->yDimension;
    }

    gc->state.color_format       = format;
    gc->state.origin             = origin;
    gc->grSstRez                 = resolution;
    gc->grSstRefresh             = refresh;
    gc->grColBuf                 = gc->state.num_buffers = nColBuffers;
    gc->grAuxBuf                 = nAuxBuffers;
    gc->fbStride                 = gc->state.screen_width * BYTES_PER_PIXEL;
    gc->grHwnd                   = (int) hWnd;
#ifdef FX_GLIDE_NAPALM
    gc->chipmask                 = SST_CHIP_MASK_ALL_CHIPS;
#endif

#ifndef FX_GLIDE_NAPALM
    gc->grPixelSize              = 2;
    gc->grPixelSample            = 1;
    hwPixelFormat                = SST_OVERLAY_PIXEL_RGB565D;
#else    

    if (IS_NAPALM(gc->bInfo->pciInfo.deviceID)) 
    {
      /* I apologize for this hack:
       * if glide was previously in a half-mode
       * gc->chipCount was forced to one. 
       * It should be restored before we try to use it.
       */
      gc->chipCount = gc->bInfo->pciInfo.numChips ;

      /*
      /* All this stuff lets Joe bag-o-donuts force old apps */
      /* to render with 32bpp and AA modes. */
      /* Bear in mind: it is silly to try to force 16bpp */
      /* rendering since we don't want to scale down the */
      /* apps Z or W values to fit in a 16 bit depth buffer. */

      if (_GlideRoot.environment.outputBpp == 32 || pixelformat == GR_PIXFMT_ARGB_8888)  {
        if ((_GlideRoot.environment.aaSample == 8) &&	/* 8xaa */
            (gc->chipCount > 2))
          pixelformat = GR_PIXFMT_AA_8_ARGB_8888 ;
        else if ((_GlideRoot.environment.aaSample == 4) &&
            (gc->chipCount > 1))
          pixelformat = GR_PIXFMT_AA_4_ARGB_8888 ;
        else if (_GlideRoot.environment.aaSample == 2)
          pixelformat = GR_PIXFMT_AA_2_ARGB_8888 ;
        else
          pixelformat = GR_PIXFMT_ARGB_8888 ;
      } 
      else if (_GlideRoot.environment.outputBpp == 15 || pixelformat == GR_PIXFMT_ARGB_1555) {
        if ((_GlideRoot.environment.aaSample == 8) &&	/* 8xaa */
            (gc->chipCount > 2))
          pixelformat = GR_PIXFMT_AA_8_ARGB_1555 ;
        else if ((_GlideRoot.environment.aaSample == 4) &&
            (gc->chipCount > 1))
          pixelformat = GR_PIXFMT_AA_4_ARGB_1555 ;
        else if (_GlideRoot.environment.aaSample == 2)
          pixelformat = GR_PIXFMT_AA_2_ARGB_1555 ;
        else
          pixelformat = GR_PIXFMT_ARGB_1555 ;
      } 
      else if (pixelformat == GR_PIXFMT_RGB_565)  {
        if ((_GlideRoot.environment.aaSample == 8) &&	/* 8xaa */
            (gc->chipCount > 2))
          pixelformat = GR_PIXFMT_AA_8_RGB_565;
        else if ((_GlideRoot.environment.aaSample == 4) &&
            (gc->chipCount > 1))
          pixelformat = GR_PIXFMT_AA_4_RGB_565 ;
        else
          if (_GlideRoot.environment.aaSample == 2)
            pixelformat = GR_PIXFMT_AA_2_RGB_565 ;
      }
    }

    /* Automagic SLI band height settings */
    if(gc->state.screen_height >= 768) {
      gc->sliBandHeight = 5;
    } else {
      gc->sliBandHeight = 4;
    }    

    GDBG_INFO(80,"Default band height: %d\n",gc->sliBandHeight);

    /* Allow user override (within reason). */
    if(_GlideRoot.environment.sliBandHeight != 0) {
      GDBG_INFO(80,"User set sli band height to: %d\n",_GlideRoot.environment.sliBandHeight);
      gc->sliBandHeight = _GlideRoot.environment.sliBandHeight;
      if(gc->sliBandHeight < 1) {
        GDBG_INFO(80,"Clamping band height to 1.\n");
        gc->sliBandHeight = 1;
      } else if(gc->sliBandHeight > 5) {
        GDBG_INFO(80,"Clamping band height to 5.\n");
        gc->sliBandHeight = 5;
      }  
    }    

    switch (pixelformat) {
    case GR_PIXFMT_RGB_565:
      gc->grPixelSample          = 1;
      gc->grPixelSize            = 2;
      hwPixelFormat              = SST_OVERLAY_PIXEL_RGB565D;
      break;
    case GR_PIXFMT_ARGB_1555:
      gc->grPixelSample          = 1;
      gc->grPixelSize            = 2;
      hwPixelFormat              = SST_OVERLAY_PIXEL_RGB1555D;
      break;
    case GR_PIXFMT_ARGB_8888:
      gc->grPixelSample          = 1;
      gc->grPixelSize            = 4;
      hwPixelFormat              = SST_OVERLAY_PIXEL_RGB32U;
      break;
    case GR_PIXFMT_AA_2_RGB_565:
      gc->grPixelSample          = 2;
      gc->grPixelSize            = 2;
      hwPixelFormat              = SST_OVERLAY_PIXEL_RGB565U;
      break;
    case GR_PIXFMT_AA_2_ARGB_1555:
      gc->grPixelSample          = 2;
      gc->grPixelSize            = 2;
      hwPixelFormat              = SST_OVERLAY_PIXEL_RGB1555U;
      break;
    case GR_PIXFMT_AA_2_ARGB_8888:
      gc->grPixelSample          = 2;
      gc->grPixelSize            = 4;
      hwPixelFormat              = SST_OVERLAY_PIXEL_RGB32U;
      break;
    case GR_PIXFMT_AA_4_RGB_565:
      gc->grPixelSample          = 4;
      gc->grPixelSize            = 2;
      hwPixelFormat              = SST_OVERLAY_PIXEL_RGB565U;
      break;
    case GR_PIXFMT_AA_4_ARGB_1555:
      gc->grPixelSample          = 4;
      gc->grPixelSize            = 2;
      hwPixelFormat              = SST_OVERLAY_PIXEL_RGB1555U;
      break;
    case GR_PIXFMT_AA_4_ARGB_8888:
      gc->grPixelSample          = 4;
      gc->grPixelSize            = 4;
      hwPixelFormat              = SST_OVERLAY_PIXEL_RGB32U;
      break;
    case GR_PIXFMT_AA_8_RGB_565:	/* 8xaa */
      gc->grPixelSample          = 8;
      gc->grPixelSize            = 2;
      hwPixelFormat              = SST_OVERLAY_PIXEL_RGB565U;
      break;
    case GR_PIXFMT_AA_8_ARGB_1555:
      gc->grPixelSample          = 8;
      gc->grPixelSize            = 2;
      hwPixelFormat              = SST_OVERLAY_PIXEL_RGB1555U;
      break;
    case GR_PIXFMT_AA_8_ARGB_8888:
      gc->grPixelSample          = 8;
      gc->grPixelSize            = 4;
      hwPixelFormat              = SST_OVERLAY_PIXEL_RGB32U;
      break;
    default:
      gc->grPixelSample          = 0;
      GDBG_INFO( gc->myLevel, "Unsupported Pixel Format = %d\n", pixelformat);
      GrErrorCallback( "grSstWinOpen: unsupported pixel format", FXFALSE );
      return 0;
      break;
    }

#if 0 /* Old Way */
    gc->sliCount = gc->chipCount / ((gc->grPixelSample == 4) ? 2 : 1);

    /* Default for the SLI case... */
    gc->grSamplesPerChip = gc->grPixelSample > 1 ? 2 : 1;
            
    /* We have two ways to do 2-sample AA when we have more than two chips.  We
     * can either do 2 samples per chip with 2-way (or 4-way) SLI, or we can do
     * one sample per chip with 1-way (or 2-way) SLI.  By default we'll opt for
     * non-SLI since in theory it has better performance.  We have an environment 
     * variable to override this, though. */
    if(gc->chipCount == 2 && gc->grPixelSample == 2 && gc->sliCount == 2) {
      if(!_GlideRoot.environment.forceOldAA) {
        gc->grSamplesPerChip = 1;
        gc->sliCount = 1;
        /* In this mode I think the video filter still works... */
        if(hwPixelFormat == SST_OVERLAY_PIXEL_RGB1555U) {
          hwPixelFormat = SST_OVERLAY_PIXEL_RGB1555D;
        } else if(hwPixelFormat == SST_OVERLAY_PIXEL_RGB565U) {
          hwPixelFormat = SST_OVERLAY_PIXEL_RGB565D;
        }    
      }     
    } else if(gc->chipCount == 4 && gc->grPixelSample == 2 && gc->sliCount == 4) {
      /* This doesn't work yet */
      if(0 && !_GlideRoot.environment.forceOldAA) {
        gc->grSamplesPerChip = 1;
        gc->sliCount = 2;
        /* In this mode I think the video filter still works... */
        if(hwPixelFormat == SST_OVERLAY_PIXEL_RGB1555U) {
          hwPixelFormat = SST_OVERLAY_PIXEL_RGB1555D;
        } else if(hwPixelFormat == SST_OVERLAY_PIXEL_RGB565U) {
          hwPixelFormat = SST_OVERLAY_PIXEL_RGB565D;
        }    
      }  
    }    

#else

	switch( gc->chipCount )
	{
		case 4:
			switch( gc->grPixelSample )
			{
				case 8:
						gc->sliCount 			= 1;
						gc->grSamplesPerChip 	= 2;
				break;
			
				case 4:
						gc->sliCount 			= 1;   /*no sli, 1 sample per chip */
						gc->grSamplesPerChip 	= 1;
				break;

				case 2:
			     	if(!_GlideRoot.environment.forceOldAA) {
        				gc->grSamplesPerChip	= 1;  //2 way SLI, 1 sample per SLI unit
        				gc->sliCount 			= 2;
				        /* In this mode I think the video filter still works... */
        				if(hwPixelFormat == SST_OVERLAY_PIXEL_RGB1555U) {
				       		hwPixelFormat = SST_OVERLAY_PIXEL_RGB1555D;
        				} 
        				else if(hwPixelFormat == SST_OVERLAY_PIXEL_RGB565U) {
          					hwPixelFormat = SST_OVERLAY_PIXEL_RGB565D;

	       				}    
      				}
      				else {
        				gc->grSamplesPerChip	= 2; /* 4 way SLI, 2 samples per SLI unit */
        				gc->sliCount 			= 4; /* doesn't work yet */				  
      				}  

				break;

				case 1:
						gc->sliCount 			= 4;
						gc->grSamplesPerChip 	= 1;
				break;		
			}
		break;

		case 2: 
			switch( gc->grPixelSample )
			{
				case 4:
						gc->sliCount 			= 1;
						gc->grSamplesPerChip 	= 2;
				break;

				case 2:
      				if(!_GlideRoot.environment.forceOldAA) {
						gc->sliCount 			= 1;			/* no sli, 1 sample per chip */
						gc->grSamplesPerChip 	= 1;
		        		if(hwPixelFormat == SST_OVERLAY_PIXEL_RGB1555U) {
          					hwPixelFormat = SST_OVERLAY_PIXEL_RGB1555D;
        				} 
        				else if(hwPixelFormat == SST_OVERLAY_PIXEL_RGB565U) {
          					hwPixelFormat = SST_OVERLAY_PIXEL_RGB565D;
        				}    
					} 
					else
					{
						gc->sliCount 			= 2;			/* 2 samples per SLI pair */
						gc->grSamplesPerChip 	= 2;
					}
	
				break;

				case 1:
						gc->sliCount 			= 2;
						gc->grSamplesPerChip 	= 1;
				break;
			}
		break;


		case 1:
			switch( gc->grPixelSample )
			{
				case 2:
      					gc->sliCount 			= 1; 
						gc->grSamplesPerChip 	= 2;
				break;

				case 1:
						gc->sliCount 			= 1;
						gc->grSamplesPerChip 	= 1;
				break;
			}
		break;


		default:
						gc->sliCount 			= 1;
						gc->grSamplesPerChip 	= 1;
		break;

	}

#endif

    /* Yeesh. */
    gc->enableSecondaryBuffer = gc->grSamplesPerChip > 1 ? FXTRUE : FXFALSE;

    /* Precompute which table entries to use for per-chip primary and secondary AA offsets. */
    /* Index 0 - No AA
     * Index 1 - 2-sample AA, 2 samples per chip
     * Index 2 - 2-sample AA, 1 sample per chip
     * Index 3 - 4-sample AA, 2 samples per chip
     */
//adjust offset index for 4 chip cards
	if(  gc->chipCount == 4 )
	{
		switch ( gc->grPixelSample )
		{
			case 8:
			gc->sampleOffsetIndex = 9;
			break;

			case 4:
			gc->sampleOffsetIndex =	8;
			break;

			case 2:
			gc->sampleOffsetIndex = 7;
			break;
		}
	}
	else
	{
    	gc->sampleOffsetIndex = gc->grPixelSample-1 + ((gc->grSamplesPerChip == 1) ? 1 : 0);
//    	if (!GETENV("FX_GLIDE_AA_SAMPLE") && gc->sampleOffsetIndex)
//      		gc->sampleOffsetIndex+=3;

	}


#endif /* FX_GLIDE_NAPALM */

    gc->grPixelFormat = pixelformat;

    /* compute tile dimensions */
    gc->strideInTiles  = ( gc->state.screen_width * (gc->grPixelSize >> 1) + ( TILE_WIDTH_PXLS - 1 ) ) / TILE_WIDTH_PXLS;
    GDBG_INFO(80, "%s: strideInTiles = 0X%x\n", FN_NAME, gc->strideInTiles);
    gc->heightInTiles  = ( gc->state.screen_height + ( TILE_HEIGHT_PXLS - 1 ) ) / TILE_HEIGHT_PXLS;
    GDBG_INFO(80, "%s: heightInTiles = 0x%x\n", FN_NAME, gc->heightInTiles);
    gc->bufferStride   = gc->strideInTiles * TILE_WIDTH_PXLS * BYTES_PER_PIXEL;
    GDBG_INFO(80, "%s: bufferStride = 0x%x\n", FN_NAME, gc->bufferStride);
    gc->bufSizeInTiles = gc->strideInTiles * gc->heightInTiles;
    GDBG_INFO(80, "%s: bufSizeInTiles = 0x%x\n", FN_NAME, gc->bufSizeInTiles);
    gc->bufSize = gc->bufSizeInTiles * TILE_WIDTH_PXLS * TILE_HEIGHT_PXLS * BYTES_PER_PIXEL;

#ifdef FX_GLIDE_NAPALM
    /* We must constrain the sli band height such that Y origin swapping
     * values work correctly. */
    if(gc->sliCount > 1)
    {
        FxU32 chipScreenHeight;
        FxU32 maxBandHeightLog2 = 0;
        FxU32 sliBandHeightInPixels;
        FxU32 numBands;
                 
//        chipScreenHeight = gc->state.screen_height >> (gc->sliCount - 1);

        chipScreenHeight = gc->state.screen_height / gc->sliCount;

        /* Find the biggest value that's still 
         * divisible by a power of two.  The check
         * for a non-zero chipScreenHeight is just 
         * in case something bad happens and it starts
         * out as zero. */
        while(!(chipScreenHeight & 1) && chipScreenHeight) {
          maxBandHeightLog2++;
          chipScreenHeight >>= 1;
        }


        if(gc->sliBandHeight > maxBandHeightLog2) {
          gc->sliBandHeight = maxBandHeightLog2;
          GDBG_INFO(80, "%s: Clamping SLI band height (Log2) to %d\n",FN_NAME, maxBandHeightLog2);
        }

        /* Recompute buffer memory requirements */
        sliBandHeightInPixels = 1L << gc->sliBandHeight;
        //chipScreenHeight = gc->state.screen_height >> (gc->sliCount - 1);
        chipScreenHeight = gc->state.screen_height / gc->sliCount;


        GDBG_INFO(80, "%s: SLI band height in pixels: %d\n",FN_NAME, sliBandHeightInPixels);
        numBands = (chipScreenHeight + (sliBandHeightInPixels - 1)) / sliBandHeightInPixels;
        GDBG_INFO(80, "%s: SLI bands required: %d\n", FN_NAME, numBands);
        chipScreenHeight = numBands * sliBandHeightInPixels;
        GDBG_INFO(80, "%s: SLI chip screen height: %d\n",FN_NAME, chipScreenHeight);
        gc->heightInTiles  = ( chipScreenHeight + ( TILE_HEIGHT_PXLS - 1 ) ) / TILE_HEIGHT_PXLS;
        GDBG_INFO(80, "%s: SLI heightInTiles = 0x%x\n", FN_NAME, gc->heightInTiles);
        gc->bufSizeInTiles = gc->strideInTiles * gc->heightInTiles;
        GDBG_INFO(80, "%s: SLI bufSizeInTiles = 0x%x\n", FN_NAME, gc->bufSizeInTiles);
        gc->bufSize = gc->bufSizeInTiles * TILE_WIDTH_PXLS * TILE_HEIGHT_PXLS * BYTES_PER_PIXEL;
        GDBG_INFO(80, "%s: SLI bufSize = 0x%x\n", FN_NAME, gc->bufSize);
           
    }
    
    if (gc->grPixelSize == 4)
      gc->bufSize *= 2 ;
#endif /* FX_GLIDE_NAPALM */

    GDBG_INFO(80, "%s: bufSize = 0x%x\n", FN_NAME, gc->bufSize);


    /*
    ** 2 pixel per clock rendering will only be enabled if 
    ** 1) we are in 15/16 bpp rendering mode (may not help in high res)
    **    high res = GR_RESOLUTION_1600x1200 for now
    ** 2) low resolution in 32 bpp
    **    low res = 0 for now ( we may never turn it on )
    ** In other cases, we still rely on the lodmin = lodmax = 1 to 
    ** minimize the texture access
    **
    ** sli & aa rendering must be disabled in all half modes
    */
    if (IS_NAPALM(gc->bInfo->pciInfo.deviceID)) 
    {
      gc->do2ppc = FXFALSE;
      gc->bInfo->h3analogSli = 0 ;

      if (gc->grPixelSize <= 2) 
      {
        switch (gc->grSstRez) 
        {
          case GR_RESOLUTION_1600x1024:
            //gc->bInfo->h3analogSli = 1 ;
            gc->do2ppc = FXTRUE;
            break ;
          case GR_RESOLUTION_1600x1200:
             break;
          case GR_RESOLUTION_1792x1344:
          case GR_RESOLUTION_1856x1392:
          case GR_RESOLUTION_1920x1440:
          case GR_RESOLUTION_2048x1536:
          case GR_RESOLUTION_2048x2048:
            gc->bInfo->h3analogSli = 1 ;
            break;          
          case GR_RESOLUTION_400x300:
          case GR_RESOLUTION_320x200:
          case GR_RESOLUTION_320x240:
          case GR_RESOLUTION_400x256:
          case GR_RESOLUTION_512x256:
          case GR_RESOLUTION_512x384:
          case GR_RESOLUTION_640x200:
            gc->sliCount = 1 ;
            gc->chipCount = 1 ;
            gc->grPixelSample = 1 ;
          default:
            gc->do2ppc = FXTRUE;
            break;
        }
      }
      else if (gc->grPixelSize == 4) 
      {
        switch (gc->grSstRez) 
        {
          case GR_RESOLUTION_1600x1024:
            gc->bInfo->h3analogSli = 1 ;
            gc->do2ppc = FXTRUE;
            break ;
          case GR_RESOLUTION_1600x1200:
          case GR_RESOLUTION_1792x1344:
          case GR_RESOLUTION_1856x1392:
          case GR_RESOLUTION_1920x1440:
          case GR_RESOLUTION_2048x1536:
          case GR_RESOLUTION_2048x2048:
            gc->bInfo->h3analogSli = 1 ;
            break;          
          case GR_RESOLUTION_400x300:
          case GR_RESOLUTION_320x200:
          case GR_RESOLUTION_320x240:
          case GR_RESOLUTION_400x256:
          case GR_RESOLUTION_512x256:
          case GR_RESOLUTION_512x384:
          case GR_RESOLUTION_640x200:
            gc->sliCount = 1 ;
            gc->chipCount = 1 ;
            gc->grPixelSample = 1 ;
          default:
            break;
        }
      }

      /*
      ** If we force the env var to 1, always turn 2ppc on.
      ** Otherwise, we only enable 2ppc in certain condition.
      */
      if (_GlideRoot.environment.do2ppc < 0) 
      {
        gc->do2ppc = FXFALSE;
      }
      else
        if (_GlideRoot.environment.do2ppc)
        {
          gc->do2ppc = FXTRUE;
        }


      /*
       * Ditto for analog sli
       */
      if (_GlideRoot.environment.analogSli < 0)
        gc->bInfo->h3analogSli = 0 ;
       else
         if (_GlideRoot.environment.analogSli)
           gc->bInfo->h3analogSli = 1 ;

       /*
        * This seems like bad news to me, but the control
        * panel applet is supposed to be able to turn off
        * SLI.
        */
       if (_GlideRoot.environment.forceSingleChip)
       {
         gc->sliCount = 1 ;
         gc->chipCount = 1 ;
       }
    }
          
//enable analog for 8xaa 4 chip cards
	if( gc->chipCount == 4 ) 
		gc->bInfo->h3analogSli = 1 ;


    /* Check for enough memory */
#ifdef GLIDE_INIT_HWC
    if ( ( 
#if FX_GLIDE_NAPALM            
    /* If we are doing 2 or 4 sample AA, then each chip needs twice as many buffers */
            gc->grSamplesPerChip * 
#endif                    
            gc->bufSize * ( gc->grColBuf + gc->grAuxBuf ) + MIN_TEXTURE_STORE + MIN_FIFO_SIZE) >
         ( gc->bInfo->h3Mem << 20 ) ) {
        GDBG_INFO( gc->myLevel, "Failed to open for insufficient memory\n" );
        GrErrorCallback( "grSstWinOpen: not enough memory for requested buffers", FXFALSE );
        GR_RETURN( FXFALSE );
    }
#endif

    /* Allocate Color/Aux Buffers, Set Memory Layout */
    gcFifo = &gc->cmdTransportInfo;

//#if defined( USE_PACKET_FIFO ) AJB CRAP
#if defined( GLIDE_INIT_HWC )
    bInfo   = gc->bInfo;
    vInfo   = &bInfo->vidInfo;
    bufInfo = &bInfo->buffInfo;

    /* If we closed down then the hw may have been un-mapped (on
     * systems that actually support this) so we need to re-map the
     * board and re-cache our hw pointers.  
     */
    if (!gc->bInfo->isMapped) {
      if (!hwcMapBoard(bInfo, HWC_BASE_ADDR_MASK)) {
        GDBG_INFO( gc->myLevel, "Failed to re-map the hw.\n" );
        GrErrorCallback( FN_NAME": Failed to re-map the hw.", FXFALSE );
        GR_RETURN( FXFALSE );
      }

      if (!hwcInitRegisters(bInfo)) {
        GDBG_INFO( gc->myLevel, "Failed to re-initialize the hw.\n" );
        GrErrorCallback( FN_NAME": Failed to re-initialize the hw.", FXFALSE );
        GR_RETURN( FXFALSE );
      }

    }

    /* Don't assume that because the board was mapped that these didn't change (NT) */
    gc->sstRegs = (SstRegs*)bInfo->regInfo.sstBase;
    gc->ioRegs  = (SstIORegs*)bInfo->regInfo.ioMemBase;
    gc->cRegs   = (SstCRegs*)bInfo->regInfo.cmdAGPBase;
    gc->lfb_ptr = (FxU32*)bInfo->regInfo.lfbBase;
    gc->rawLfb  = (FxU32*)bInfo->regInfo.rawLfbBase;
    gc->tex_ptr = (FxU32*)SST_TEX_ADDRESS(bInfo->regInfo.sstBase);

    vInfo->xRes              = gc->state.screen_width;
    vInfo->yRes              = gc->state.screen_height;
    vInfo->refresh           = gc->grSstRefresh;
    vInfo->tiled             = FXTRUE;
    vInfo->initialized       = FXTRUE;
    bufInfo->enable2ndbuffer = gc->enableSecondaryBuffer;
    bInfo->h3pixelSize       = gc->grPixelSize;
    bInfo->h3pixelSample     = gc->grPixelSample;
    bInfo->h3nwaySli         = gc->sliCount ;
    bInfo->h3sliBandHeight   = 1L << gc->sliBandHeight; /* KCD: hwc buffer allocation needs this */

    if ( hwcAllocBuffers( bInfo, nColBuffers, nAuxBuffers ) == FXFALSE ) {
        GDBG_INFO( gc->myLevel, "hwcAllocBuffers failed\n" );
        GrErrorCallback(hwcGetErrorString(), FXFALSE);
        GR_RETURN( FXFALSE );
    }

    for (buffer = 0; buffer < nColBuffers; buffer++) {
      gc->buffers0[buffer] = bufInfo->colBuffStart0[buffer];
      GDBG_INFO(80, "Buffer %d:  Start: 0x%x\n", buffer, gc->buffers0[buffer]);
      gc->lfbBuffers[buffer] = (FxU32)gc->rawLfb + bufInfo->lfbBuffAddr0[buffer];
      if (bInfo->buffInfo.enable2ndbuffer) {
        gc->buffers1[buffer] = bufInfo->colBuffStart1[buffer];
        GDBG_INFO(80, "Buffer %d:  Start: 0x%x\n", buffer, gc->buffers1[buffer]);
      }
    }
    if (nAuxBuffers != 0) {
      gc->buffers0[buffer] = bufInfo->auxBuffStart0;
      gc->lfbBuffers[buffer] = (FxU32)gc->rawLfb + bufInfo->lfbBuffAddr0[buffer];
      GDBG_INFO(80, "Aux Buffer:  Start: 0x%x\n", gc->buffers0[buffer]);
      if (bInfo->buffInfo.enable2ndbuffer) {
        gc->buffers1[buffer] = bufInfo->auxBuffStart1;
        GDBG_INFO(80, "Aux Buffer:  Start: 0x%x\n", gc->buffers1[buffer]);
      }
    }

    gc->fbOffset               = bInfo->fbOffset;

    switch (gc->num_tmu) {
    case 2:
      if (_GlideRoot.environment.enUma) {
        /*
        ** hack for Glide2x to enable uma
        */
        gc->tramOffset[0]          = gc->tramOffset[1]          = bInfo->tramOffset;
        gc->tramSize[0]            = gc->tramSize[1]            = bInfo->tramSize;
        gc->tmu_state[0].total_mem = gc->tmu_state[1].total_mem = gc->tramSize[0];
        /* disable splash screen in case it uses 2nd tmu the old style */
        _GlideRoot.environment.noSplash = 1;
      }
      else {
        gc->tramOffset[0]          = bInfo->tramOffset;
        /*
        ** if the environment variable is on, use the its texture memory size * 2
        */
        if ((_GlideRoot.environment.tmuMemory != -1) && (gc->fbOffset >= 0x200000))
          bInfo->tramSize = _GlideRoot.environment.tmuMemory << 21;
        /*
        ** when tmu size is 2, split the texture memory for each tmu
        */
        gc->tramSize[0]            = bInfo->tramSize >> 1;
        gc->tmu_state[0].total_mem = gc->tramSize[0];
        gc->tramOffset[1]          = gc->tramOffset[0] + gc->tramSize[0];
        gc->tramSize[1]            = bInfo->tramSize >> 1;
        gc->tmu_state[1].total_mem = gc->tramSize[1];
      }
      break;
    case 1:
    default:
      gc->tramOffset[0]          = bInfo->tramOffset;
      /*
      ** if the environment variable is on, use the its texture memory size
      */
      if ((_GlideRoot.environment.tmuMemory != -1) && (gc->fbOffset >= 0x200000))
        bInfo->tramSize = _GlideRoot.environment.tmuMemory << 20;
      gc->tramSize[0]            = bInfo->tramSize;
      gc->tmu_state[0].total_mem = bInfo->tramSize;
      break;
    }

    vInfo->hWnd     = gc->grHwnd;
    vInfo->sRes     = gc->grSstRez;
    vInfo->vRefresh = gc->grSstRefresh;

    if ( hwcInitVideo( bInfo, FXTRUE, gc->vidTimings, hwPixelFormat, FXTRUE ) == FXFALSE ) {
        GrErrorCallback(hwcGetErrorString(), FXFALSE);
        GDBG_INFO( gc->myLevel, "hwcInitVideo failed\n" );
        GR_RETURN( FXFALSE );
    }

    GDBG_INFO(80, FN_NAME ":  GammaRGB = %1.2f, %1.2f, %1.2f\n", 
              _GlideRoot.environment.gammaR,
              _GlideRoot.environment.gammaG,
              _GlideRoot.environment.gammaB);

    /*
    ** initialize context checking
    */
    hwcShareContextData(gc->bInfo, &(gc->lostcontext));
    *gc->lostcontext = FXFALSE;

    {
      /*
      ** This is a hack for Quake2 gamma setting. When video/brightness is
      ** changed in Quake2, grSstWinOpen is called to update the new
      ** parameters. Therefore, we have to stick the hack in here.
      ** Quake2 also uses "SSTV2_GAMMA" to update the gamma value.
      */
      char *gammastr;
      if (gammastr = GETENV("SSTV2_GAMMA")) {
        _GlideRoot.environment.gammaR 
          = _GlideRoot.environment.gammaG 
          = _GlideRoot.environment.gammaB 
          = (float)atof(gammastr);
      }
    }

    if (
        _GlideRoot.environment.gammaR != -1.f &&
        _GlideRoot.environment.gammaG != -1.f &&
        _GlideRoot.environment.gammaB != -1.f
        ) {
      hwcGammaRGB(gc->bInfo, _GlideRoot.environment.gammaR, 
                      _GlideRoot.environment.gammaG,
                      _GlideRoot.environment.gammaB);
    } else {
      hwcGammaRGB(gc->bInfo, 1.3f, 1.3f, 1.3f);
    }

    if (_GlideRoot.environment.grxClk != -1L)
      hwcSetGrxClock(gc->bInfo, _GlideRoot.environment.grxClk);

#if !DIRECT_IO /* AJB CRAP */
    if (gc->cmdTransportInfo.autoBump = _GlideRoot.environment.autoBump) {
      if (!hwcInitFifo( bInfo, gc->cmdTransportInfo.autoBump)) {
        hwcRestoreVideo(bInfo);
        GrErrorCallback(hwcGetErrorString(), FXFALSE);
        GDBG_INFO(gc->myLevel, "hwcInitFifo failed\n");
        GR_RETURN(FXFALSE);
      }
    } else {
#if __POWERPC__
      if (!hwcInitAGPFifo(bInfo, FXFALSE)) {
#else
      if (!hwcInitAGPFifo(bInfo, FXTRUE)) {
#endif      
        hwcRestoreVideo(bInfo);
        GrErrorCallback(hwcGetErrorString(), FXFALSE);
        GDBG_INFO(gc->myLevel, "hwcInitFifo failed\n");
        GR_RETURN(FXFALSE);
      }
      
      /* Check to see where the command fifo was placed since the agp
       * allocation might have failed for some reason, and fallen back
       * to using the normal video command fifo.
       */
      gc->cmdTransportInfo.autoBump = ((GR_CAGP_GET(baseSize) & SST_CMDFIFO_DISABLE_HOLES) == 0);
    }

    /* COMMAND FIFO SETUP */
    fInfo  = &gc->bInfo->fifoInfo;

    /* Establish physical bounds of cmd fifo from HWC calculation */
    gcFifo->fifoOffset = fInfo->fifoStart;
    gcFifo->fifoSize   = fInfo->fifoLength;
#endif /* DIRECT_IO */
#elif defined( GLIDE_INIT_HAL ) 
#if 0
    gc->fbOffset               = 0x200000;
    gc->tramOffset             = 0x0;
    gc->tramSize             = gc->fbOffset;
    gc->tmu_state[0].total_mem = gc->tramSize;
#else
    /* gc->fbOffset               = (FxU32)fxHalFbiGetMemory((SstRegs*)gc->reg_ptr); */
    gc->fbOffset               = (FxU32)gc->rawLfb;
    gc->fbOffset               = 0;

#ifdef FX_GLIDE_NAPALM
    gc->tramOffset[0]          =
      (gc->grPixelSize == 4) ? 0x400000 : 0x200000;
#else
    gc->tramOffset[0] = 0x200000 ;
#endif /* FX_GLIDE_NAPALM */

    gc->tramSize[0]            = 0x200000;
    gc->tramOffset[1]          = gc->tramSize[0] + gc->tramOffset[0];
    gc->tramSize[1]            = 0x200000;
    gc->tmu_state[0].total_mem = gc->tramSize[0];
    gc->tmu_state[1].total_mem = gc->tramSize[1];
#endif

    {
      bInfo   = gc->bInfo;
      vInfo   = &bInfo->vidInfo;
      bufInfo = &bInfo->buffInfo;
      
      vInfo->xRes        = gc->state.screen_width;
      vInfo->yRes        = gc->state.screen_height;
      vInfo->refresh     = gc->grSstRefresh;
      vInfo->tiled       = FXFALSE;
      vInfo->initialized = FXTRUE;

      gc->colTiled = gc->auxTiled = FXFALSE ; /* AJB- grBufferClear needs to know this */

      if (GETENV("SST_FBI_MEM")) {
        bInfo->h3Mem = atoi(GETENV("SST_FBI_MEM"));
      }
      else {
        bInfo->h3Mem = 32;
      }
     bInfo->h3pixelSize = gc->grPixelSize ;

      if (gc->grPixelSample > 1)
        bInfo->buffInfo.enable2ndbuffer = FXTRUE;
      else
        bInfo->buffInfo.enable2ndbuffer = FXFALSE;

      if ( hwcAllocBuffers( gc->bInfo, nColBuffers, nAuxBuffers ) == FXFALSE ) {
      }
      for ( buffer = 0; buffer < nColBuffers; buffer++ ) {
        gc->buffers0[buffer] = bufInfo->colBuffStart0[buffer];
        GDBG_INFO(80, "Buffer %d:  Start: 0x%x\n", buffer, gc->buffers0[buffer]);
        gc->lfbBuffers[buffer] = (FxU32)gc->rawLfb + bufInfo->lfbBuffAddr0[buffer];
        if (bInfo->buffInfo.enable2ndbuffer) {
          gc->buffers1[buffer] = bufInfo->colBuffStart1[buffer];
          GDBG_INFO(80, "Buffer %d:  Start: 0x%x\n", buffer, gc->buffers1[buffer]);
        }
      }
      if (nAuxBuffers != 0) {
        gc->buffers0[buffer] = bufInfo->auxBuffStart0;
        GDBG_INFO(80, "Aux Buffer:  Start: 0x%x\n", gc->buffers0[buffer]);
        gc->lfbBuffers[buffer] = (FxU32)gc->rawLfb + bufInfo->lfbBuffAddr0[buffer];
        if (bInfo->buffInfo.enable2ndbuffer) {
          gc->buffers1[buffer] = bufInfo->auxBuffStart1;
          GDBG_INFO(80, "Aux Buffer:  Start: 0x%x\n", gc->buffers1[buffer]);
        }
      }
    }

    if( !fxHalGetDeviceInfo((SstRegs*)gc->reg_ptr, &devInfo) ) {
        GrErrorCallback("  XXXGetDeviceInfo failed.\n", FXFALSE);
        GDBG_INFO( gc->myLevel, 
                   "   XXXGetDeviceInfo failed. (0x%x)\n", 
                   gc->reg_ptr );
        GR_RETURN( FXFALSE );
    }

    /* COMMAND FIFO SETUP */
#if 0
    gcFifo->fifoOffset = gc->fbOffset + 
        ( gc->bufSize * ( gc->grColBuf + gc->grAuxBuf ) );
    gcFifo->fifoSize = 0x400000 - gcFifo->fifoOffset + gc->fbOffset;
#else
    {
      gc->fbOffset = bInfo->fbOffset;
      gc->tramOffset[0] = bInfo->tramOffset;
      gc->tramSize[0] = bInfo->tramSize >> 1;
      gc->tmu_state[0].total_mem = gc->tramSize[0];
      
      gc->tramOffset[1] = gc->tramOffset[0] + gc->tramSize[0];
      gc->tramSize[1]   = (bInfo->tramSize >> 1);
      gc->tmu_state[1].total_mem   = gc->tramSize[1];
      gcFifo->fifoOffset = 0x0;
      gcFifo->fifoSize = 0x20000;
    }
#endif

    if ( !fxHalInitCmdFifo((SstRegs *) gc->reg_ptr,
                           0, /* which fifo - 0 for 3d cmd fifo */
                           /* v fifoStart - offset from hw base v */
                           gcFifo->fifoOffset, 
                           gcFifo->fifoSize, /* size - in bytes */ 
                           FXTRUE, /* directExec */
                           FXFALSE, /* disableHoles */
                           FXFALSE) /* agpEnable */ ) {
#ifdef GLIDE_INIT_HWC
        GrErrorCallBack( "fxHalInitCmdFifo failed.\n", FxFALSE );
#endif
        GDBG_INFO( 0, "Error: fxHalInitCmdFifo failed\n" );
        GR_RETURN( FXFALSE );
    }
    if ( !fxHalInitVideo( (SstRegs*) gc->reg_ptr,
                          (resolution == GR_RESOLUTION_NONE) ? GR_RESOLUTION_640x480
                          : (resolution), 
                          refresh, 
                          NULL ) ) {
#ifdef GLIDE_INIT_HWC
        GrErrorCallBack( "fxHalInitVideo failed.\n", FxFALSE );
#endif
        GDBG_INFO( 0, "Error: fxHalInitVideo failed\n" );
        GR_RETURN( FXFALSE );
    }
    fxHalInitVideoOverlaySurface( (SstRegs*) gc->reg_ptr,     /* SstRegs */
                                  FXTRUE, /* 1=enable Overlay surface*/
                                  FXFALSE, /* 1=enable OS stereo, 0=disable*/
                                  FXFALSE, /* 1=enable horizontal*/
                                  0, /* horizontal scale factor (ignored if*/
                                  FXFALSE, /* 1=enable vertical scaling,*/
                                  0, /* vertical scale factor (ignored if not*/
                                  0, /* duh*/
                                  1, /* 0=OS linear, 1=tiled*/
                                  SST_OVERLAY_PIXEL_RGB565U, /* pixel format of OS*/
                                  FXFALSE, /* bypass clut for OS?*/
                                  FXFALSE, /* 0=lower 256 CLUT entries,*/
                                  gc->buffers0[gc->curBuffer], /* board address of beginning of OS  */
                                  gc->strideInTiles ); /* distance between scanlines of the OS, in*/

    /*
    ** initialize context checking
    */
    {
      gc->lostcontext = &lostcontext_csim;
      *gc->lostcontext = FXFALSE;
    }

#endif /*  defined( GLIDE_INIT_HAL )  */
/* AJB- Hacked this out. Ask me why maybe I'll tell you */
#if 0
//#else  /* !defined( USE_PACKET_FIFO ) */
    gc->fbOffset               = 0x0;
    gc->tramOffset[0]          = 0x200000;
    gc->tramSize[0]            = 0x200000;
    gc->tramOffset[1]          = gc->tramSize[0] + gc->tramOffset[0];
    gc->tramSize[1]            = 0x200000;
#if 0
    gc->tmu_state[0].total_mem = gc->tramSize;
    gc->fbOffset               = 0x200000;
    gc->tramOffset             = 0x0;
    gc->tramSize               = gc->fbOffset;
    gc->tmu_state[0].total_mem = gc->tramSize;

    for ( buffer = 0; buffer < nColBuffers; buffer++ ) {
      gc->buffers[buffer]    = gc->fbOffset + buffer * gc->bufSize;
      /* XXXjdt: this is never initialized in the old code */
      gc->lfbBuffers[buffer] = 0; 
      GDBG_INFO(80, "%s: Buffer %d: 0x%x\n",
                FN_NAME, buffer, gc->buffers[buffer]);
    }
#endif
    {
      bInfo   = gc->bInfo;
      vInfo   = &bInfo->vidInfo;
      bufInfo = &bInfo->buffInfo;
      
      vInfo->xRes        = gc->state.screen_width;
      vInfo->yRes        = gc->state.screen_height;
      vInfo->refresh     = gc->grSstRefresh;
      vInfo->tiled       = FXFALSE;
      vInfo->initialized = FXTRUE;

      gc->colTiled = gc->auxTiled = FXFALSE ; /* AJB- grBufferClear needs to know this */

      if (GETENV("SST_FBI_MEM")) {
        bInfo->h3Mem = atoi(GETENV("SST_FBI_MEM"));
      }
      else {
        bInfo->h3Mem = 32;
      }
      bInfo->h3pixelSize = gc->grPixelSize ;

      if (gc->grPixelSample > 1)
        bInfo->buffInfo.enable2ndbuffer = FXTRUE;
      else
        bInfo->buffInfo.enable2ndbuffer = FXFALSE;

      if ( hwcAllocBuffers( gc->bInfo, nColBuffers, nAuxBuffers ) == FXFALSE ) {
      }
      for ( buffer = 0; buffer < nColBuffers; buffer++ ) {
        gc->buffers0[buffer] = bufInfo->colBuffStart0[buffer];
        GDBG_INFO(80, "Buffer %d:  Start: 0x%x\n", buffer, gc->buffers0[buffer]);
        gc->lfbBuffers[buffer] = (FxU32)gc->rawLfb + bufInfo->lfbBuffAddr0[buffer];
        if (bInfo->buffInfo.enable2ndbuffer) {
          gc->buffers1[buffer] = bufInfo->colBuffStart1[buffer];
          GDBG_INFO(80, "Buffer %d:  Start: 0x%x\n", buffer, gc->buffers1[buffer]);
        }
      }
      if (nAuxBuffers != 0) {
        gc->buffers0[buffer] = bufInfo->auxBuffStart0;
        GDBG_INFO(80, "Aux Buffer:  Start: 0x%x\n", gc->buffers0[buffer]);
        gc->lfbBuffers[buffer] = (FxU32)gc->rawLfb + bufInfo->lfbBuffAddr0[buffer];
        if (bInfo->buffInfo.enable2ndbuffer) {
          gc->buffers1[buffer] = bufInfo->auxBuffStart1;
          GDBG_INFO(80, "Aux Buffer:  Start: 0x%x\n", gc->buffers1[buffer]);
        }
      }
    }
    if( !fxHalGetDeviceInfo((SstRegs*)gc->reg_ptr, &devInfo) ) {
        GrErrorCallback("  XXXGetDeviceInfo failed.\n", FXFALSE);
        GDBG_INFO( gc->myLevel, 
                   "   XXXGetDeviceInfo failed. (0x%x)\n", 
                   gc->reg_ptr );
        GR_RETURN( 0 );
    }
#if 0
    /* COMMAND FIFO SETUP */
    gcFifo->fifoOffset = gc->fbOffset + 
        ( gc->bufSize * ( gc->grColBuf + gc->grAuxBuf ) );
    gcFifo->fifoSize = 0x400000 - gcFifo->fifoOffset + gc->fbOffset;

    if ( !fxHalInitCmdFifo((SstRegs *) gc->reg_ptr,
                           0, /* which fifo - 0 for 3d cmd fifo */
                           /* v fifoStart - offset from hw base v */
                           gcFifo->fifoOffset, 
                           gcFifo->fifoSize, /* size - in bytes */ 
                           FXTRUE, /* directExec */
                           FXFALSE, /* disableHoles */
                           FXFALSE) /* agpEnable */ ) {
#ifdef FX_FAIL_HWC
        GrErrorCallBack( "fxHalInitCmdFifo failed.\n", FxFALSE );
#endif
        GDBG_INFO( 0, "Error: fxHalInitCmdFifo failed\n" );
        GR_RETURN( 0 );
    }
#endif
    if ( !fxHalInitVideo( (SstRegs*) gc->reg_ptr,
                          (resolution == GR_RESOLUTION_NONE) ? GR_RESOLUTION_640x480
                          : (resolution), 
                          refresh, 
                          NULL ) ) {
        GR_RETURN( 0 );
    }
    fxHalInitVideoOverlaySurface( (SstRegs*) gc->reg_ptr, /* SstRegs */
                                  FXTRUE, /* 1=enable Overlay surface*/
                                  FXFALSE, /* 1=enable OS stereo, 0=disable*/
                                  FXFALSE, /* 1=enable horizontal*/
                                  0, /* horizontal scale factor (ignored if*/
                                  FXFALSE, /* 1=enable vertical scaling,*/
                                  0, /* vertical scale factor (ignored if not*/
                                  0, /* duh*/
                                  1, /* 0=OS linear, 1=tiled*/
                                  SST_OVERLAY_PIXEL_RGB565U, /* pixel format of OS*/
                                  FXFALSE, /* bypass clut for OS?*/
                                  FXFALSE, /* 0=lower 256 CLUT entries,*/
                                  gc->buffers0[gc->curBuffer], /* board address of beginning of OS  */
                                  gc->strideInTiles ); /* distance between scanlines of the OS, in*/
    _grReCacheFifo(0);
    /*
    ** initialize context checking
    */
    {
      gc->lostcontext = &lostcontext_csim;
      *gc->lostcontext = FXFALSE;
    }
#endif /* !defined( USE_PACKET_FIFO ) */

    /* Compute Virtual FIFO address extents */
#ifdef GLIDE_INIT_HWC
    if (bInfo->fifoInfo.agpFifo) {
      gcFifo->fifoStart = (FxU32 *) bInfo->fifoInfo.agpVirtAddr;
      gcFifo->fifoOffset = bInfo->fifoInfo.agpPhysAddr;
    } else {
#else
    {
#endif
      gcFifo->fifoStart = gc->rawLfb + ( gcFifo->fifoOffset >> 2 );
    }
    gcFifo->fifoEnd   = gcFifo->fifoStart + ( gcFifo->fifoSize >> 2 );

    /* Adjust room values. 
    ** RoomToEnd needs enough room for the jmp packet since we never
    ** allow the hw to auto-wrap. RoomToRead needs to be adjusted so that
    ** we never acutally write onto the read ptr.
    **
    ** fifoRoom is generally the min of roomToEnd and roomToRead, but we
    ** 'know' here that roomToRead < roomToEnd.   
    */
#if USE_PACKET_FIFO
    gcFifo->roomToEnd = gcFifo->fifoSize - FIFO_END_ADJUST;
    gcFifo->fifoRoom  = gcFifo->roomToReadPtr = gcFifo->roomToEnd - sizeof( FxU32 );

          
    /* Set initial fifo state. hw read and sw write pointers at
    ** start of the fifo.
    */
    gcFifo->fifoPtr  = gcFifo->fifoStart;
    gcFifo->fifoRead = HW_FIFO_PTR( FXTRUE );
    /*  We havn't really fenced here, but we set the lastFence for */
    /*  later calculations  */  
    gcFifo->lastFence = gcFifo->fifoStart;

#endif /* !USE_PACKET_FIFO */

    if ( (void*)gcFifo->fifoPtr != (void*)gcFifo->fifoRead ) {
#ifdef GLIDE_INIT_HWC
        hwcRestoreVideo( bInfo );
#endif
        GDBG_INFO( gc->myLevel, "Initial fifo state is incorrect\n" );
        GR_RETURN( FXFALSE );
    }

#if __POWERPC__ && PCI_BUMP_N_GRIND
    enableCopyBackCache((FxU32)gcFifo->fifoStart,gcFifo->fifoSize);
#endif

    if (!gc->cmdTransportInfo.autoBump) {
      gcFifo->bumpSize = _GlideRoot.environment.bumpSize;

      gcFifo->lastBump = gcFifo->fifoPtr;
      gcFifo->bumpPos = gcFifo->fifoPtr + gcFifo->bumpSize;

#if __POWERPC__ && PCI_BUMP_N_GRIND
      gcFifo->fifoJmpHdr[0] = 
        ( SSTCP_PKT0_JMP_LOCAL |
          ( gcFifo->fifoOffset << ( SSTCP_PKT0_ADDR_SHIFT - 2 )));
#else
      gcFifo->fifoJmpHdr[0] = 
        ( SSTCP_PKT0_JMP_AGP |
          ( gcFifo->fifoOffset << ( SSTCP_PKT0_ADDR_SHIFT - 2 )));
      gcFifo->fifoJmpHdr[1] = (gcFifo->fifoOffset >> 25);
#endif

    } else {
      gcFifo->fifoJmpHdr[0] = 
        ( SSTCP_PKT0_JMP_LOCAL |
          ( gcFifo->fifoOffset << ( SSTCP_PKT0_ADDR_SHIFT - 2 )));


    }


    GDBG_INFO(80, 
              "Command Fifo:\n"
              "\tfifoStart:       0x%x\n"
              "\tfifoEnd:         0x%x\n"
              "\tfifoOffset:      0x%x\n"
              "\tfifoSize:        0x%x\n"
              "\tfifoPtr:         0x%x\n",
              gcFifo->fifoStart, 
              gcFifo->fifoEnd,
              gcFifo->fifoOffset, 
              gcFifo->fifoSize,
              gcFifo->fifoPtr ); 

#if defined( TACO_MEMORY_FIFO_HACK ) 
    /* XXXTACOHACK!!! - Insert in-memory fifo */
    gcFifo->vFifoStart = calloc( gcFifo->fifoRoom, 1 );
    gcFifo->vFifoEnd   = gcFifo->vFifoStart + ( gcFifo->fifoRoom >> 2 );
    gcFifo->fifoPtr    = gcFifo->vFifoStart;
    if ( gcFifo->fifoPtr == NULL ) {
        GDBG_INFO( gc->myLevel, "Couldn't allocate in-memory fifo\n" );
        GR_RETURN(FXFALSE);
    }
    GDBG_INFO( 0, "Running with whack in-memory fifo.\n" );
#endif /* defined ( TACO_MEMORY_FIFO_HACK ) */

    /* The hw is now in a usable state from the fifo macros */
    gc->open = FXTRUE;

    /* Setup the arch dependent texture procs */
    gc->archDispatchProcs.texDownloadProcs = _GlideRoot.curTexProcs;

    /*------------------------------------------------------
      GC Init
      ------------------------------------------------------*/
    GDBG_INFO(gc->myLevel, "  GC Init\n");
    initGC( gc );
    

    /*------------------------------------------------------
      3D State Init 
      ------------------------------------------------------*/
    GDBG_INFO( gc->myLevel, "  3D State Init\n");
    
    GDBG_INFO( gc->myLevel, "   Setting default register states\n" );
    gc->state.fbi_config.fbzMode = ( SST_ENRECTCLIP | SST_ENZBIAS );
    
    GDBG_INFO( gc->myLevel, "   Setting up initial draw buffer state\n" );
    REG_GROUP_BEGIN(BROADCAST_ID, leftOverlayBuf, 1, 0x1);
    REG_GROUP_SET(hw, leftOverlayBuf, gc->buffers0[gc->frontBuffer]);
    REG_GROUP_END();
    
    REG_GROUP_BEGIN(BROADCAST_ID, swapbufferCMD, 1, 0x1); 
    REG_GROUP_SET(hw, swapbufferCMD, 0x0);
    REG_GROUP_END();

#ifdef GLIDE_INIT_HWC    
    if (IS_NAPALM(gc->bInfo->pciInfo.deviceID)) {
      _grChipMask( gc->chipmask );
    }

    REG_GROUP_BEGIN(BROADCAST_ID, colBufferAddr, 4, 0xf); 
    REG_GROUP_SET(hw, colBufferAddr, gc->buffers0[gc->curBuffer]);
    REG_GROUP_SET(hw, colBufferStride, gc->strideInTiles | SST_BUFFER_MEMORY_TILED );
    REG_GROUP_SET(hw, auxBufferAddr, gc->buffers0[nColBuffers]);
    REG_GROUP_SET(hw, auxBufferStride, gc->strideInTiles | SST_BUFFER_MEMORY_TILED); 
    REG_GROUP_END();


    if (gc->grPixelSample > 1) {
      
      if(gc->enableSecondaryBuffer) {
        REG_GROUP_BEGIN(BROADCAST_ID, colBufferAddr, 4, 0xf);
        {
            REG_GROUP_SET(hw, colBufferAddr, gc->buffers1[gc->curBuffer] | SST_BUFFER_BASE_SELECT);
            REG_GROUP_SET(hw, colBufferStride, gc->strideInTiles | SST_BUFFER_MEMORY_TILED );
            REG_GROUP_SET(hw, auxBufferAddr, gc->buffers1[nColBuffers] | SST_BUFFER_BASE_SELECT);
            REG_GROUP_SET(hw, auxBufferStride, gc->strideInTiles | SST_BUFFER_MEMORY_TILED); 
        }
        REG_GROUP_END();
      }
      /*
      ** setup aaCtrl register
      */
      {
        _grAAOffsetValue(_GlideRoot.environment.aaXOffset[gc->sampleOffsetIndex],
                         _GlideRoot.environment.aaYOffset[gc->sampleOffsetIndex],
                         0, gc->chipCount - 1, gc->enableSecondaryBuffer) ;


      }
    }
#else
    /* enable all chip mask */

    if (IS_NAPALM(gc->bInfo->pciInfo.deviceID)) {
      REG_GROUP_BEGIN(BROADCAST_ID, chipMask, 1, 0x1); 
      REG_GROUP_SET(hw, chipMask, 0xffffffff);
      REG_GROUP_END();
    }
       {
        /*
        ** initialize pci registers for sli/aa
        */
        FxI32 chipIndex;
        for(chipIndex=0; (FxU32)chipIndex < gc->chipCount; chipIndex++) {
          halCfgStore32(0x3FC, 1, gc->halInfo->boardInfo[chipIndex].pciBusNumber,
                        gc->halInfo->boardInfo[chipIndex].pciDeviceNumber,
                        gc->halInfo->boardInfo[chipIndex].pciFunctionNumber,
                        offsetof(SstPCIConfigRegs, cfgSliLfbCtrl),
                        SST_SLI_LFB_CPU_WRITE_ENABLE |
                        SST_SLI_LFB_DISPATCH_WRITE_ENABLE |
                        SST_SLI_LFB_READ_ENABLE);                   
          halCfgStore32(0x3FC, 1, gc->halInfo->boardInfo[chipIndex].pciBusNumber,
                        gc->halInfo->boardInfo[chipIndex].pciDeviceNumber,
                        gc->halInfo->boardInfo[chipIndex].pciFunctionNumber,
                        offsetof(SstPCIConfigRegs, cfgAADepthBufferAperture),
                        0);
          halCfgStore32(0x3FC, 1, gc->halInfo->boardInfo[chipIndex].pciBusNumber,
                        gc->halInfo->boardInfo[chipIndex].pciDeviceNumber,
                        gc->halInfo->boardInfo[chipIndex].pciFunctionNumber,
                        offsetof(SstPCIConfigRegs, cfgAALfbCtrl),
                        SST_AA_LFB_CPU_WRITE_ENABLE |
                        SST_AA_LFB_DISPATCH_WRITE_ENABLE |
                        SST_AA_LFB_READ_ENABLE);
        }
      }

    gc->bufferStride = gc->state.screen_width * bInfo->h3pixelSize;
    REG_GROUP_BEGIN(BROADCAST_ID, colBufferAddr, 4, 0xf);
    {
      REG_GROUP_SET(hw, colBufferAddr, gc->buffers0[gc->curBuffer]);
      REG_GROUP_SET(hw, colBufferStride, gc->bufferStride);
      REG_GROUP_SET(hw, auxBufferAddr, gc->buffers0[nColBuffers]);
      REG_GROUP_SET(hw, auxBufferStride, gc->bufferStride); 
    }
    REG_GROUP_END();

    if (gc->enableSecondaryBuffer) {
      REG_GROUP_BEGIN(BROADCAST_ID, colBufferAddr, 4, 0xf);
      {
        REG_GROUP_SET(hw, colBufferAddr, gc->buffers1[gc->curBuffer] | SST_BUFFER_BASE_SELECT);
        REG_GROUP_SET(hw, colBufferStride, gc->bufferStride);
        REG_GROUP_SET(hw, auxBufferAddr, gc->buffers1[nColBuffers] | SST_BUFFER_BASE_SELECT);
        REG_GROUP_SET(hw, auxBufferStride, gc->bufferStride); 
      }
      REG_GROUP_END();
      /*
      ** setup aaCtrl register
      */
      {
        /* Anti-aliasing default perturbation values */
//8xaa
        static FxU32 defaultXOffset[8];// = {0x7a, 0x2, 0x7c, 0x4};
        static FxU32 defaultYOffset[8];// = {0x7b, 0x4, 0x3, 0x7d};
        _grAAOffsetValue(defaultXOffset, defaultYOffset, 
                           0, gc->chipCount - 1, gc->enableSecondaryBuffer);
      }
    }
#endif
  
#ifdef FX_GLIDE_NAPALM 

    if (IS_NAPALM(gc->bInfo->pciInfo.deviceID))
    {
      if (gc->sliCount > 1) {
        _grSliCtrl();
      }

      _grRenderMode(gc->grPixelFormat) ;
    }

#endif /* FX_GLIDE_NAPALM */
 
    GDBG_INFO( gc->myLevel, "  Setting all Glide state\n" );
    assertDefaultState();

#if defined( TACO_MEMORY_FIFO_HACK ) 
    _FifoFlush();
#endif /* defined( TACO_MEMORY_FIFO_HACK ) */

    /* --------------------------------------------------------
       Splash Screen
       --------------------------------------------------------*/
    doSplash();

    _GlideRoot.windowsInit = FXTRUE; /* to avoid race with grSstControl() */
    

    /* AJB- workaround AA garbage */
    if ((gc->grPixelSample > 1) &&
       (_GlideRoot.environment.aaClip))
      grClipWindow(0, 0, gc->state.screen_width, 
                   gc->state.screen_height);

    GR_RETURN(FXTRUE);
#undef FN_NAME
} /* grSstWinOpen */

/*-------------------------------------------------------------------
  Function: grSstWinClose
  Date: 3/16
  Implementor(s): jdt
  Library: Glide
  Description:
  Shut down the selected SST

  Shutdown has 4 steps

  3D Idle 
    the 3D engine must be idled to make sure that there are no
    commands executing in the transport when the registers are
    reset

  GC Reset
    the GC is flagged as unitialized - (nosup)

  Command Transport Disable
    the command transport to the 3D device is put in a state
    of reset.  No further commands may be issued throught the
    command transport

  Video Restore
    video is restored to its pre-open state.

  Arguments:
  none
  Return:
  none
  -------------------------------------------------------------------*/
GR_ENTRY(grSstWinClose, void, (void))
{
#define FN_NAME "grSstWinClose"
  GR_BEGIN_NOFIFOCHECK_NORET("grSstWinClose", 80);
  GDBG_INFO_MORE(gc->myLevel,"()\n");

  if (gc) {
    if(gc->open) {
      if (gc->lostcontext && *gc->lostcontext) {
        /* If gc->lostcontext is true, then we lost display context,
         * so we should NOT try to restore video.  The lostcontext
         * check was backwards in Glide2.x. %%KCD */
        gc->open = FXFALSE;
        /* gc->hwInitP = FXFALSE; */
#if (GLIDE_PLATFORM & GLIDE_OS_WIN32)
        if (gc->pluginInfo.moduleHandle) {
          FreeLibrary(gc->pluginInfo.moduleHandle);
          gc->pluginInfo.moduleHandle = 0L;
        }
#endif
        return;
      }
    }
  }

  if ((gc != NULL) && (gc->open)) {
#if GLIDE_INIT_HAL
    /* dpc - 22 may 1997 - FixMe!
     * We need the equivilant stuff in the hal layer too.
     */
#else /* !GLIDE_INIT_HAL */
    /* avoid multiple load in grSstWinOpen */
#if (GLIDE_PLATFORM & GLIDE_OS_WIN32)
    if (gc->pluginInfo.moduleHandle) FreeLibrary(gc->pluginInfo.moduleHandle);
#endif

    /*--------------------------
      3D Idle
      --------------------------*/
    GDBG_INFO(gc->myLevel, "  3D Idle");
    grSstIdle();

    /*--------------------------
      Command Transport Disable
      --------------------------*/
    GDBG_INFO(gc->myLevel, "  Command Transport Disable");
    
#if __POWERPC__ && PCI_BUMP_N_GRIND
      restoreCacheSettings();
#endif
    
      /* disable SLI and AA */
#ifdef FX_GLIDE_NAPALM
      if (IS_NAPALM(gc->bInfo->pciInfo.deviceID)) {
        _grChipMask( SST_CHIP_MASK_ALL_CHIPS );
        _grTex2ppc(GR_TMU0, FXFALSE);
        if (gc->grPixelSample > 1) {
          _grAAOffsetValue(_GlideRoot.environment.aaXOffset[0],
                           _GlideRoot.environment.aaYOffset[0],
                           0, gc->chipCount - 1, FXFALSE) ;

        }
        if (gc->sliCount > 1)
          _grDisableSliCtrl();
      }
#endif

    /* Video Restore 
     *
     * NB: The hwcRestoreVideo in addition to restoring the video also
     * turns off the command fifo and then releases the hw context
     * which can unmap the board at the driver level.  The next time
     * we use grSstWinOpen we need to re-map the board etc just to be
     * safe everywhere.
     */
    GDBG_INFO(gc->myLevel, "  Restore Video");
    hwcRestoreVideo(gc->bInfo);

#endif /* !GLIDE_INIT_HAL */

    /*--------------------------
      GC Reset
      --------------------------*/
    GDBG_INFO(gc->myLevel, "  GC Reset");

    /* These are really two different things.  
     *
     * hwInitP indicates whether the init code mapping/init sequence
     * is active for this hw. 
     *
     * open includes setting up video, command transport, and the
     * initial glide state.
     */
    /* gc->hwInitP = FXFALSE; */
  }
  gc->open = FXFALSE;
    
  GR_END();
#undef FN_NAME
} /* grSstWinClose */

GR_ENTRY(grSstControl, FxBool, (GrControl_t code)) 
{
#if 0
  GR_DCL_GC;
#define FN_NAME "grSstControl"  
  GDBG_INFO(41, "%s:  code = 0x%x, windowsInit = %d\n", FN_NAME,
    code, _GlideRoot.windowsInit);
  
  if (_GlideRoot.windowsInit && gc->open) {
#if (GLIDE_PLATFORM & GLIDE_HW_SST96)
    /* For VG96, windows haven't been created, so there's no point
     * trying to control it. 
     */
    
    /* Believe it or not, the following code really should be bracketed
       like this.  The reason is that GR_BEGIN_NOFIFOCHECK seg faults
       when grSstControl is called before the Glide window is truly
       initialized.  This is a real concern, as grSstControl is called
       from event loops, which are asynchronous.
       */
    FxU32
      status,
      xRes, yRes,
      clipLeftRight, clipBottomTop;
    
    GR_BEGIN_NOFIFOCHECK(FN_NAME,80);
    
    xRes = initControl(code);
    
    GDBG_INFO((80, "%s:  initControl returned 0x%x\n", FN_NAME, xRes));

    if (!xRes) return FXFALSE;
  
    yRes = xRes >> 16;
    xRes = xRes & 0xffff;
  
    /* !! FIXME +++ this call should properly update the gc->fbStride,
       this information is known only to the init layer and needs to be
       exposed, it is only really necessary to lfblock right now,
       and therefore is returned by initGetBufferPtr */
  
    gc->state.screen_width = xRes;
    gc->state.screen_height = yRes;

    /* Wait for Idle. */
  
    nTries = 0;
    do {
      if (nTries++ >> 999) {
        GDBG_INFO((80, "%s:  returning FALSE after %d checks for idle\n", 
          FN_NAME, nTries));
        return FXTRUE;
      }

      status = GET(hw->status);
        
    } while (status & 0x1);

    /* Set ClipRect Via direct writes */
    _grClipNormalizeAndGenerateRegValues(0, 0, xRes, yRes,
      &clipLeftRight,
      &clipBottomTop);
    SET_DIRECT(clipLeftRight1, clipLeftRight);
    SET_DIRECT(clipBottomTop1, clipBottomTop);

#elif (GLIDE_PLATFORM&GLIDE_HW_SST1)
    return initControl(code);
#elif (GLIDE_PLATFORM & GLIDE_HW_CVG) && !GLIDE_INIT_HAL
    {
      FxBool isValidP = FXTRUE;
      FxBool passFlag;

      switch ( code ) {
      case GR_CONTROL_DEACTIVATE:
        passFlag = FXTRUE;
        break;

      case GR_CONTROL_ACTIVATE:
        passFlag = FXFALSE;
        break;

      default:
        isValidP = FXFALSE;
      }

      if (isValidP) sst1InitVgaPassCtrl(gc->base_ptr, passFlag);
    }
#endif    
  }

  GDBG_INFO(41, "%s:  Returning TRUE\n", FN_NAME);
  return FXTRUE;  
#undef FN_NAME
#else
  return FXTRUE;
#endif

} /* grSstControl */

/*---------------------------------------------------------------------------
**  grSstPerfStats
*/ 
#ifndef GLIDE3_ALPHA
GR_ENTRY(grSstPerfStats, void, (GrSstPerfStats_t *pStats))
{
#define FN_NAME "grSstPerfStats"
  GR_BEGIN_NOFIFOCHECK("grSstPerfStats",83);
  GDBG_INFO_MORE(gc->myLevel,"(0x%x)\n",pStats);
  GR_CHECK_F(FN_NAME, !pStats, "NULL pointer");

  pStats->pixelsIn   = GR_GET(hw->stats.fbiPixelsIn);
  pStats->chromaFail = GR_GET(hw->stats.fbiChromaFail);
  pStats->zFuncFail  = GR_GET(hw->stats.fbiZfuncFail);
  pStats->aFuncFail  = GR_GET(hw->stats.fbiAfuncFail);
  pStats->pixelsOut  = GR_GET(hw->stats.fbiPixelsOut);

  GR_END();
#undef FN_NAME
} /* grSstPerfStats */
#endif

/*---------------------------------------------------------------------------
**  grStatsResetPerfStats
*/

#if defined(GLIDE3) && (GLIDE3_ALPHA)
void FX_CSTYLE
_grSstResetPerfStats(void)
#else
GR_ENTRY(grSstResetPerfStats, void, (void))
#endif
{
#define FN_NAME "grSstResetPerfStats"
  GR_BEGIN("grSstResetPerfStats",83,4, 1);
  GDBG_INFO_MORE(gc->myLevel,"()\n");
  GR_SET(BROADCAST_ID, hw, nopCMD, 1);
  GR_END();
#undef FN_NAME
} /* grSstResetPerfStats */


/*---------------------------------------------------------------------------
**  grSstStatus - return contents of status register
*/

#if defined(GLIDE3) && (GLIDE3_ALPHA)
FxU32 FX_CSTYLE
_grSstStatus(void)
#else
GR_ENTRY(grSstStatus, FxU32, (void))
#endif
{
#define FN_NAME "grSstStatus"
  FxU32 status;

  GR_BEGIN_NOFIFOCHECK(FN_NAME, 83);

  if (!gc->cmdTransportInfo.autoBump)
    GR_BUMP_N_GRIND;


  status = GR_GET(hw->status);

#ifdef FX_GLIDE_NAPALM
  /*
  ** AJB check slave status
  */
  {
    FxU32 chip ; 

    if (gc->chipCount)
      for (chip = 0 ;
           chip < gc->chipCount - 1 ;
           chip++)
        status |= GR_GET(gc->slaveSstRegs[chip]->status) ;
  }
#endif

  /* For effed-up stuff */
  status |= (0xffff << 0xc);

  status &= 0xfffffff;          /* Clear high nibble */

  /* OR in buffers pending */
  if (gc->swapsPending)
    status |= ((grBufferNumPending() & 0x7) << 28);

  GR_RETURN(status);
#undef FN_NAME
}/* grSstStatus */

/*---------------------------------------------------------------------------
**  grSstVideoLine - return current video line number
*/

#if defined(GLIDE3) && defined(GLIDE3_ALPHA)
FxU32 FX_CSTYLE
_grSstVideoLine(void)
#else
GR_ENTRY(grSstVideoLine, FxU32, (void))
#endif
{
  FxU32 vline = 1;

  return vline;
}/* grSstVideoLine */

/*---------------------------------------------------------------------------
**  grSstVRetrace - return contents of SST_VRETRACE bit of status register;
*/

#if defined(GLIDE3) && defined(GLIDE3_ALPHA)
FxBool FX_CSTYLE
_grSstVRetraceOn(void)
#else
GR_ENTRY(grSstVRetraceOn, FxBool, (void))
#endif
{
  FxU32 status;
  GR_BEGIN_NOFIFOCHECK("grSstVRetraceOn",83);

  status = grSstStatus();

  return ((status & SST_VRETRACE) == 0);

}/* grSstVRetrace */

/*---------------------------------------------------------------------------
** grSstIdle/grFinish
*/
#if defined(GLIDE3) && defined(GLIDE3_ALPHA)
GR_ENTRY(grFinish, void, (void))
#define FN_NAME "grFinish"
#else
GR_ENTRY(grSstIdle, void, (void))
#define FN_NAME "grSstIdle"
#endif
{
//  FxU32 status;
  FxU32 i = 0 ;
  GR_BEGIN_NOFIFOCHECK(FN_NAME, 83);
  GDBG_INFO_MORE(gc->myLevel,"()\n");

  gc->counter = gc->expected_counter = 0x0;

#if GLIDE_DEBUG
  {
    const FxU32 savedLockCount = gc->cmdTransportInfo.lfbLockCount;
    gc->cmdTransportInfo.lfbLockCount = 0;
#endif /* GLIDE_DEBUG */

    REG_GROUP_BEGIN(BROADCAST_ID, nopCMD, 0x1, 0x1);
    REG_GROUP_SET(hw, nopCMD, 0x0);
    REG_GROUP_END();

#if GLIDE_DEBUG
    gc->cmdTransportInfo.lfbLockCount = savedLockCount;
  }
#endif /* GLIDE_DEBUG */

  /*do {
    status = grSstStatus();
  } while (status & SST_BUSY);*/

    /* retro3dfx: bounded idle wait. If the accelerator wedges with
     * SST_BUSY stuck (hw never goes idle), the original loop spins
     * forever; on the grSstWinClose path that hangs the app at exit
     * and the desktop is never restored. ~4M status polls is seconds
     * of continuous BUSY on real hw -- treat that as a wedge and
     * proceed with shutdown anyway. */
    {
      FxU32 busyPolls = 0;
      do {
        if(grSstStatus() & SST_BUSY) {
          i = 0; /* Reset counter */
          if (++busyPolls > 4000000UL)
            break; /* WEDGE-BREAK: hw stuck busy, give up idling */
        } else
          i++;
      } while(i < 3);
    }

//    while (grSstStatus() & SST_BUSY) ;
//    while (((grSstStatus() & SST_BUSY) == 0) &&
//            (++i < 3)) ;

  GR_END();
#undef FN_NAME
} /* grSstIdle */

/*-------------------------------------------------------------------
  Function: grFlush
  Date: 09-Jan-98
  Implementor(s): atai
  Description:

  Arguments:
  
  Return:
  -------------------------------------------------------------------*/
#if defined(GLIDE3) && defined(GLIDE3_ALPHA)
GR_ENTRY(grFlush, void, (void))
{
#define FN_NAME "grFlush"
//  FxU32 status;
  FxU32 i ;
  GR_BEGIN("grFlush", 83, 4, 1);
  GDBG_INFO_MORE(gc->myLevel,"()\n");

  GR_SET(BROADCAST_ID, hw, nopCMD, 0x0);

/*  do {
    status = grSstStatus();
  } while (status & SST_BUSY);*/

    while (grSstStatus() & SST_BUSY) ;
    while (((grSstStatus() & SST_BUSY) == 0) &&
            (++i < 3)) ;
  GR_END();
#undef FN_NAME
} /* grSstIdle */
#endif

/*---------------------------------------------------------------------------
**  grSstIsBusy - find out if the SST is busy or not
*/

#if defined(GLIDE3) && defined(GLIDE_ALPHA)
FxBool FX_CSTYLE
grSstIsBusy(void)
#else
GR_ENTRY(grSstIsBusy, FxBool, (void))
#endif
{
#define FN_NAME "grSstIsBusy"
  static FxBool nopP = FXTRUE;
  FxBool idle;
  FxU32  i = 0 ;
  GR_BEGIN_NOFIFOCHECK("grSstIsBusy", 80);

  /* dpc - 22 may 1997 - FixMe!
   * Seems like the simplest way to do it, but is this really the way
   * to do it?  
   */
  if (nopP) {
    GR_SET_EXPECTED_SIZE(sizeof(FxU32), 1);
    GR_SET(BROADCAST_ID, hw, nopCMD, 0);
    GR_CHECK_SIZE();
  }

#if defined(GLIDE3) && (GLIDE3_ALPHA)
  busy = ((_grSstStatus() & SST_BUSY) != 0);
#else
  /*
   * Napalm must be read idle three times for us
   * to believe it.
   */
  while ((idle = ((grSstStatus() & SST_BUSY) == 0)) &&
         (++i < 3));
#endif
  nopP = idle ;

  GDBG_INFO(84,"grSstIsBusy() => 0x%x\n", !idle);

  return !idle ;
#undef FN_NAME
}/* grSstIsBusy */

/*---------------------------------------------------------------------------
**  grGammaCorrectionValue - set the gamma correction value
*/

GR_ENTRY(grGammaCorrectionValue, void, (float gamma))
{
  GR_BEGIN_NOFIFOCHECK("grGammaCorrectionValue",80);
  GDBG_INFO_MORE(gc->myLevel,"(%g)\n",gamma);

#if GLIDE_INIT_HAL
  fxHalInitGamma(hw, gamma);
#else /* !GLIDE_INIT_HAL */

  hwcGammaRGB(gc->bInfo, gamma, gamma, gamma);

#endif /* !GLIDE_INIT_HAL */

  GR_END();
} /* grGammaCorrectionValue */

/*---------------------------------------------------------------------------
**  grSstOrigin - Set the orgin orientation of the screen.
**
**  Returns:
**
**  Notes:
**
*/

GR_STATE_ENTRY(grSstOrigin, void, (GrOriginLocation_t origin))
{
#define FN_NAME "grSstOrigin"
  FxU32 fbzMode;
  GR_BEGIN_NOFIFOCHECK("grSstOrigin", 83);
  GDBG_INFO_MORE(gc->myLevel, "(%d)\n", origin);

  /* Initialize FBZMODE register */
  fbzMode = gc->state.fbi_config.fbzMode;
  if (origin == GR_ORIGIN_LOWER_LEFT)
    fbzMode |= SST_YORIGIN;
  else
    fbzMode &= ~(SST_YORIGIN);

  /* dpc - 22 may 1997 - FixMe! 
   * Do we need to do anything here for the HAL?
   */
#if !GLIDE_INIT_HAL
  /* dpc - 5 sep 1997 - FixMe!
   * This is the old way. Is there anything else we
   * need to do here?
   *
   * initOrigin(origin); 
   */
#endif

  gc->state.fbi_config.fbzMode = fbzMode;

  REG_GROUP_BEGIN(BROADCAST_ID, fbzMode, 0x1, 0x1);
  REG_GROUP_SET(hw, fbzMode, fbzMode);
  REG_GROUP_END();

#ifdef FX_GLIDE_NAPALM
  /*
  ** Don't know if we are going to ship the same binary for 
  ** Napalm/V3/Banshee,
  ** use FX_GLIDE_NAPALM for napalm specific code temporarily
  */
  if (IS_NAPALM(gc->bInfo->pciInfo.deviceID))
  {
    FxU32 renderMode = gc->state.fbi_config.renderMode;
    renderMode &= ~(SST_RM_YORIGIN_TOP);
    renderMode |= SST_RM_YORIGIN_SELECT;
    if (origin == GR_ORIGIN_LOWER_LEFT)
    {
      /* AJB - According to the Napalm spec this is correct, but the winsim seems to perform the yorigin
      ** subtraction BEFORE the SLI munging */
      renderMode |= (((gc->state.screen_height / gc->sliCount) - 1) << SST_RM_YORIGIN_TOP_SHIFT);
      /* renderMode |= ((gc->state.screen_height - 1) << SST_RM_YORIGIN_TOP_SHIFT);  */
    }
    REG_GROUP_BEGIN(BROADCAST_ID, renderMode, 0x1, 0x1);
    REG_GROUP_SET(hw, renderMode, renderMode);
    REG_GROUP_END();
    gc->state.fbi_config.renderMode = renderMode;

    if (gc->sliCount > 1) 
      _grSliCtrl() ;
    
  }
#endif
#undef FN_NAME
} /* grSstOrigin */

/* GMT: do we really have users for this???
 * CHD: No.
 * JDT: Huh?  If you're talking about grSstOrigin, you're smoking crack.
 *      if you are talking about SstConfigPipeline, it is evil and must
 *      be destroyed. :)
 * dpc: There is one user that I know of. This 'Nature' demo that Scott just
 *      gave me.
 * chd: It's a stub now.
 * (much time elapses)
 * chd: But WTF is that forward decl down there?
 * dpc: Its to get rid of the damn compiler warning for a function that
 *      we only sort of export.
 */


extern FX_ENTRY void FX_CALL
grSstConfigPipeline(GrChipID_t chip, FxU32 reg, FxU32 value);

/*---------------------------------------------------------------------------
** grSstConfigPipeline
*/

GR_ENTRY(grSstConfigPipeline, void, (GrChipID_t chip, FxU32 reg, FxU32 value))
{
} /* grSstConfigPipeline */


#ifdef FX_GLIDE_NAPALM


/*
**  _grSstSetColumnsOfNWidth
**
**  Sets the 'columns of N' width (fka columns of 8).
**  
*/
static void
_grSstSetColumnsOfNWidth(FxU32 width)
{
#define FN_NAME "_grSstSetColumnsOfNWidth"
  FxU32
    bits,                       /* Bits for the column band control field */
    fbzColorPath;               /* local shadow of same */
  GR_BEGIN_NOFIFOCHECK(FN_NAME, 80);

  if (_GlideRoot.environment.columnWidth)
    width = _GlideRoot.environment.columnWidth;

  GDBG_INFO_MORE(gc->myLevel, "(%d)\n", width);

  /* Validate Argument
  **  
  **  There might be some effed-up algorithm for this, but it escapes
  **  me at this time.  It's not like this is a performance-critical
  **  routine, anyway.
  */
  switch (width) {
  case 32:
    bits = 0x2;
  case 16:
    bits = 0x1;
    break;
  case 8:
    bits = 0x0;
    break;
  case 4:
    bits = 0x3;
    break;

  default:
    GDBG_INFO(80, FN_NAME ":  Invalid argument %d.  Falling back to default\n",
              width);
    width = 8;
    bits = 0x0;
    break;
  }

  fbzColorPath = gc->state.fbi_config.fbzColorPath;

  fbzColorPath &= ~SST_TRIANGLE_ITERATOR_COLUMN_BAND_CONTROL;

  fbzColorPath |= (bits << SST_TRIANGLE_ITERATOR_COLUMN_BAND_CONTROL_SHIFT);

  REG_GROUP_BEGIN(BROADCAST_ID, fbzColorPath, 0x1, 0x1);
  REG_GROUP_SET(hw, fbzColorPath, fbzColorPath);
  REG_GROUP_END();

  gc->state.fbi_config.fbzColorPath = fbzColorPath;

#undef FN_NAME
} /* _grSstSetColumnsOfNWidth */

/*---------------------------------------------------------------------------
** _grChipMask
*/

void
_grChipMask(FxU32 mask)
{
#define FN_NAME "_grChipMask"  
  GR_BEGIN_NOFIFOCHECK("_grChipMask", 85);
  GDBG_INFO_MORE(gc->myLevel, "(0x%x)\n", mask);
  
  REG_GROUP_BEGIN(BROADCAST_ID, chipMask, 1, 0x1); 
  REG_GROUP_SET(hw, chipMask, mask);
  REG_GROUP_END();

#undef FN_NAME
} /* _grChipMask */

/*---------------------------------------------------------------------------
** _grAAOffsetValue
** chipid is 0 based
*/

void
_grAAOffsetValue(FxU32 *xOffset, 
                 FxU32 *yOffset, 
                 FxU32 minchipid,
                 FxU32 maxchipid,
                 FxBool enable)
{
#define FN_NAME "_grAAOffsetValue"  
  FxU32 chipIndex, aaCtrl;
  GR_BEGIN_NOFIFOCHECK("_grAAOffsetValue", 85);
  GDBG_INFO_MORE(gc->myLevel, "(0x%x,0x%x,%d,%d,%d)\n", xOffset, yOffset,
                 minchipid, maxchipid, enable);
  
  for (chipIndex = minchipid; chipIndex <= maxchipid; chipIndex++) {
  
    _grChipMask( 1 << chipIndex);

//8xaa
    aaCtrl = (xOffset[(chipIndex * 2)%8] << SST_AA_CONTROL_PRIMARY_X_OFFSET_SHIFT) |
             (yOffset[(chipIndex * 2)%8] << SST_AA_CONTROL_PRIMARY_Y_OFFSET_SHIFT) |
             (xOffset[(chipIndex * 2 + 1)%8] << SST_AA_CONTROL_SECONDARY_X_OFFSET_SHIFT) |
             (yOffset[(chipIndex * 2 + 1)%8] << SST_AA_CONTROL_SECONDARY_Y_OFFSET_SHIFT) |
			 ((enable) ? SST_AA_CONTROL_AA_ENABLE : 0);

    REG_GROUP_BEGIN(BROADCAST_ID, aaCtrl, 1, 0x1);
    REG_GROUP_SET(hw, aaCtrl, aaCtrl);
    REG_GROUP_END();

  }
  
  _grChipMask( gc->chipmask );

  /*              XXX FIX THIS CRAP XXX
   * fogMode, for some crazy reason I may never understand
   * selects the dither matrix for AA mode and setting
   * the forMode to something will make certain that happens.
   * When I start to care this will be done in a nice way
   * but until then: HAHAHAHA
   */
  grFogMode(GR_FOG_DISABLE);

#undef FN_NAME
} /* _grAAOffsetValue */

/*---------------------------------------------------------------------------
** _grSliCtrl
*/

void
_grSliCtrl(void)
{
#define FN_NAME "_grSliCtrl"  
  FxU32 chipIndex;
  FxI32 sliChipCountDivisor;
  FxU32 renderMask;
  FxU32 scanMask;
  FxU32 log2chipCount;
  GR_BEGIN_NOFIFOCHECK("_grSliCtrl", 85);
  /*
  ** enable sli mode
  */
//8xaa
  if( gc-> chipCount == 2 )
  	sliChipCountDivisor = (gc->grPixelSample == 4) ? 2 : 1;

  if( gc-> chipCount == 4 )
	sliChipCountDivisor = (gc->grPixelSample == 2) ? 2 : 1;

  renderMask = (gc->chipCount / sliChipCountDivisor - 1) << gc->sliBandHeight;
  scanMask = (1 << gc->sliBandHeight) - 1;
  log2chipCount = 0;
  
  while (( 0x1UL << log2chipCount ) != (gc->chipCount / sliChipCountDivisor))
    log2chipCount++;
  
  for (chipIndex = 0; chipIndex < gc->chipCount; chipIndex++) 
  {
    FxU32 compareMask ;
    FxU32 sliCtrl ;

    /* AJB- When YORIGIN swapping we have to swap the compareMask too--
       Otherwise line ownership gets hosed. 'course winsim behaves 
      completely differently, so this might need some fixing on 
      real hardware.
    */
    if (gc->state.fbi_config.fbzMode & SST_YORIGIN)
     compareMask = ((gc->chipCount - chipIndex - 1) / sliChipCountDivisor) << gc->sliBandHeight;
    else
     compareMask = (chipIndex / sliChipCountDivisor) << gc->sliBandHeight;
    
    sliCtrl = 
      ( (renderMask << SST_SLI_CONTROL_RENDER_MASK_SHIFT) |
        (compareMask << SST_SLI_CONTROL_COMPARE_MASK_SHIFT) |
        (scanMask << SST_SLI_CONTROL_SCAN_MASK_SHIFT) |
        (log2chipCount << SST_SLI_CONTROL_LOG2_CHIP_COUNT_SHIFT) |
        SST_SLI_CONTROL_SLI_ENABLE);
    
    _grChipMask( 1 << chipIndex );
    
    REG_GROUP_BEGIN(BROADCAST_ID, sliCtrl, 1, 0x1);
    REG_GROUP_SET(hw, sliCtrl, sliCtrl);
    REG_GROUP_END();
    
  }
  
  _grChipMask( gc->chipmask );
  
#undef FN_NAME
} /* _grSliCtrl */

/*---------------------------------------------------------------------------
** _grDisableSliCtrl
*/

void
_grDisableSliCtrl(void)
{
#define FN_NAME "_grDisableSliCtrl"  
  FxU32 chipIndex;
  GR_BEGIN_NOFIFOCHECK("_grDisableSliCtrl", 85);
  /*
  ** disable sli mode
  */
  for (chipIndex = 0; chipIndex < gc->chipCount; chipIndex++) {
    FxU32 sliCtrl = 0;    
    _grChipMask( 1 << chipIndex );
    REG_GROUP_BEGIN(BROADCAST_ID, sliCtrl, 1, 0x1);
    REG_GROUP_SET(hw, sliCtrl, sliCtrl);
    REG_GROUP_END();
  }
  _grChipMask( gc->chipmask );
  
#undef FN_NAME
} /* _grDisableSliCtrl */

/*---------------------------------------------------------------------------
** _grRenderMode
** stolen from glide3 code to support 32bpp rendering
*/

void
_grRenderMode(FxU32 pixelformat)
{
#define FN_NAME "_grRenderMode"  
  FxU32 renderMode = 0 ;
  GR_BEGIN_NOFIFOCHECK("_grRenderMode", 85);

  /*
  ** setup default rendermode
  */
  renderMode = gc->state.fbi_config.renderMode ;
  renderMode &= ~(SST_RM_3D_MODE);
  if (_GlideRoot.environment.guardbandclipping) {
    renderMode |= SST_RM_ENGUARDBAND;
  }

  /*
   *          XXX Memo to Myself XXX
   * For 4 chip SLI, do different stuff with the
   * dither rotation bit. Touch the puppet head.
   */
  switch (pixelformat) {

  case GR_PIXFMT_AA_2_ARGB_1555:
  case GR_PIXFMT_AA_4_ARGB_1555:
  case GR_PIXFMT_AA_8_ARGB_1555:	/* 8xaa */
	renderMode |= SST_RM_DITHER_ROTATION ;
  case GR_PIXFMT_ARGB_1555:
    renderMode |= SST_RM_15BPP;
    break;
  case GR_PIXFMT_ARGB_8888:
  case GR_PIXFMT_AA_2_ARGB_8888:
  case GR_PIXFMT_AA_4_ARGB_8888:
  case GR_PIXFMT_AA_8_ARGB_8888:	/* 8xaa */
    renderMode |= SST_RM_32BPP;
    renderMode |= SST_RM_RED_WMASK | SST_RM_GREEN_WMASK | SST_RM_BLUE_WMASK | SST_RM_ALPHA_WMASK ;
    break;
  case GR_PIXFMT_AA_2_RGB_565:
  case GR_PIXFMT_AA_4_RGB_565:
  case GR_PIXFMT_AA_8_RGB_565:		/* 8xaa */
    renderMode |= SST_RM_DITHER_ROTATION ;
  default:
    renderMode |= SST_RM_16BPP;
    break;
  }

  REG_GROUP_BEGIN(BROADCAST_ID, renderMode, 0x1, 0x1);
  REG_GROUP_SET(hw, renderMode, renderMode);
  REG_GROUP_END();

  gc->state.fbi_config.renderMode = renderMode ;

#undef FN_NAME
} /* _grRenderMode */

#endif /* FX_GLIDE_NAPALM */

/*---------------------------------------------------------------------------
**  grGammaCorrectionValue - set the gamma correction value
*/

GR_ENTRY(_grGetSipValues, FxU32, (FxU32 chipNum, FxU32 *nandChain, FxU32 *norChain))
{
  GR_BEGIN_NOFIFOCHECK("_grGetSipValues",80);

  if(chipNum < gc->bInfo->pciInfo.numChips) {
    hwcCalcSipValue(gc->bInfo, chipNum, nandChain, norChain);
    return 1;
  } else {
    return 0;
  }  
  GR_END();
} /* grGammaCorrectionValue */
