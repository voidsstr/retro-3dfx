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
** $Header: gpci.c, 33, 10/30/00 9:54:10 AM, Jacqueline Brown-Kambui$
** $Log: 
**  33   3dfx      1.28.1.3    10/30/00 Jacqueline Brown-Kambui Added support for
**       Napalm2 in grSstDetectResources routine.
**  32   3dfx      1.28.1.2    10/11/00 Brent           Forced check in to enforce
**       branching.
**  31   3dfx      1.28.1.1    09/29/00 Jonny Cochrane  Fix up jitter values and
**       option to use correct jitter values from glide3
**  30   3dfx      1.28.1.0    09/27/00 Jonny Cochrane  8x FSAA and fixes for 4
**       chip 2x FSAA.
** 
**  29   3dfx      1.28        04/13/00 Kenneth Dyke    Added support for new-style
**       two-sample AA mode.
**  28   3dfx      1.27        04/10/00 Kenneth Dyke    Added magical screenshot
**       hotkey.
**  27   3dfx      1.26        03/30/00 Kenneth Dyke    Fixed default SLI band
**       height and column width settings.
**  26   3dfx      1.25        03/28/00 Kenneth Dyke    Added fix for certain dumb
**       games.
**  25   3dfx      1.24        03/25/00 Adam Briggs     added support for
**       SSTH3_SLI_AA_CONFIGURATION (the env var the control panel uses to force AA
**       modes)
**  24   3dfx      1.23        03/24/00 Chris Dow       Added code to fence every n
**       writes (not to exceed 0x10000) where n is either 0x10000 or indicated by
**       the environment variable FX_GLIDE_FENCE_LIMIT
** 
**  23   3dfx      1.22        03/24/00 Chris Dow       Modified periodic fencing
**       to use a tunable environtment variable.
**  22   3dfx      1.21        03/16/00 Kenneth Dyke    User settable LOD bias
**       offset.
**  21   3dfx      1.20        03/14/00 Adam Briggs     enable analog sli in 2X
**       modes or when forced
**  20   3dfx      1.19        03/14/00 Adam Briggs     implemented env. var
**       'FX_GLIDE_AA_CLIP' to enable or disable the cheesey cliprect workaround
**       for AA artifacts.
**  19   3dfx      1.18        03/13/00 Adam Briggs     properly inited AA jitter
**       values for 2-chip boards
**  18   3dfx      1.17        03/08/00 Kenneth Dyke    Use new isMapped boardInfo
**       flag instead of broken gc flag.
**  17   3dfx      1.16        03/07/00 Larry  warner   Set do2ppc default value to
**       -1 so that the default for 2ppc is OFF.
**  16   3dfx      1.15        03/06/00 Kenneth Dyke    Added AA toggle key hack.
**  15   3dfx      1.14        02/23/00 Kenneth Dyke    Fixed up and enabled wax
**       and triangle buffer clear code.
**  14   3dfx      1.13        02/08/00 Kenneth Dyke    Added slave FIFO handling
**       code.
**  13   3dfx      1.12        01/31/00 Adam Briggs     changed the IS_NAPALM macro
**       to cooperate with the display driver version of the same
**  12   3dfx      1.11        01/31/00 Adam Briggs     Changed all device ID magic
**       numbers to use those defined in fxhal.h & added IS_NAPALM macro to test
**       against device ID range
**  11   3dfx      1.10        01/28/00 Adam Briggs     glide3->glide2 merge from
**       napalm bringup
**  10   3dfx      1.9         01/26/00 Jacqueline Brown-Kambui Added check for
**       Napalm in grSstDetectResources routine.
**  9    3dfx      1.8         01/26/00 Jacqueline Brown-Kambui Changed Napalm
**       device ID from 7 to 9.
**  8    3dfx      1.7         12/10/99 Kenneth Dyke    Ugh.  Undid some changes
**       for backwards compatibility.
** 
**  7    3dfx      1.6         12/08/99 Kenneth Dyke    Fixed stupid macro problem.
**  6    3dfx      1.5         12/08/99 Kenneth Dyke    Environment variable
**       checking is now actually sane and based on the value of the variable and
**       not just its presence.
**  5    3dfx      1.4         12/04/99 Larry  warner   Show the name of the
**       current Glide2x.dll, not the name of the app that's running.
**  4    3dfx      1.3         12/01/99 Adam Briggs     set gc->chipCount from
**       minihwc... this should help sli quite a bit
**  3    3dfx      1.2         11/05/99 Anthony tai     added
**       FX_GLIDE_SWAPPENDINGCOUNT. Default=1. Range 0-3
**  2    3dfx      1.1         09/13/99 Anthony tai     
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
** 
** 182   8/16/99 11:18a Adamb
** Merged in V3_OEM_100 fixes
** 
** 181   7/30/99 1:59p Adamb
** got rid of warning on non-csim build
** 
** 180   7/29/99 5:04p Adamb
** Supported FX_GLIDE_AA_SAMPLE env var to force 2 & 4 sample AA. Also
** made sli work.
** 
** 179   7/27/99 11:16a Adamb
** support for FX_GLIDE_BPP env var to force 32bpp renderMode
** 
** 178   7/18/99 11:12a Atai
** added 2 pixel per clock
** 
** 177   7/06/99 5:11p Atai
** sync with tot csim
** 
** 176   6/25/99 1:16p Atai
** fixed with 2 buffers per chip
** 
** 175   6/18/99 1:15p Kcd
** PowerPC PCI Bump & Grind stuff.
** 
** 174   6/03/99 11:05a Atai
** force deviceID to 7 for code development
** 
** 173   5/21/99 8:52a Stb_gkincade
** Put in check for : is GETENV(SSTH3_ALPHADITHERMODE) a null string?
** 
** 172   5/20/99 9:01a Atai
** is GETENV("SSTH3_ALPHADITHERMODE")  a null string?
** 
** 171   5/19/99 4:27p Kcd
** MacOS stuff.
** 
** 170   5/13/99 2:50p Stb_gkincade
** Added user support for turning on/off dither substraction
** 
** 169   4/06/99 9:57a Atai
** remove un-used variable
** 
** 168   4/05/99 10:35p Atai
** added code to check context. exit if alt-tab happens
** 
** 167   3/29/99 6:06p Atai
** initialize 2d regs
** 
** 166   3/17/99 5:08p Peter
** removed whacky stuff now that the command fifo threshold stuff appears
** to make all happy (including the k7)
** 
** 165   3/17/99 1:37p Atai
** use grHints to enable/disable uma hack
** 
** 164   3/16/99 11:51a Atai
** Back door (set FX_GLIDE_ENABLE_UMA=1) to enable unified texture memory.
** TMUn memory size will the whole texture memory space. The offset for
** each TMU points to the start address of the memory pool.
** 
** 163   3/15/99 10:51p Dow
** Vile Hack
** 
** 162   3/13/99 9:48p Dow
** optimizations for B&G
** 
** 161   3/12/99 2:31p Dow
** Removed 3DNow for K7 (temp workaround)
** 
** 160   3/08/99 6:11p Atai
** report Voodoo3 fbi/tmu rev number as Banshee for EA games
** 
** 159   3/06/99 10:59a Atai
** fixed my ·F check-in.
** 
** 158   3/05/99 2:50p Atai
** fbi/tmu rev mods
** 
** 157   3/04/99 3:15p Atai
** mods for direct write
** 
** 156   12/09/98 2:07p Peter
** More Norbert's stuff for the other 3DNow!(tm) partners
** 
** 155   12/03/98 11:27p Dow
** Code 'cleanup' heç
** 
** 154   12/03/98 10:34p Dow
** Added GLIDE_FGETENV for floats and removed registry code
** 
** 153   12/03/98 12:37p Dow
** Fixed DOS build
** 
** 152   12/02/98 2:53p Dow
** NT/9X Registry reading fix
** 
** 151   11/19/98 9:53p Jeske
** make sure we look for Voodoo3/avenger also...
** 
** 150   11/18/98 7:59p Dow
** grxclk
** 
** 149   11/18/98 7:44p Atai
** use env var FX_GLIDE_NUM_TMU
** 
** 148   11/17/98 7:04p Atai
** added env var "FX_GLIDE_DISABLE_TMU1"
** 
** 147   11/10/98 6:44p Atai
** number of tmu and texture memory allocation
** 
** 146   11/09/98 3:32p Mikec
** 
** 143   11/05/98 1:55p Atai
** initialize 2nd tmu configs
** 
** 142   10/21/98 4:20p Atai
** gamma stuff
** 
** 141   10/21/98 10:41a Atai
** mod for HAL_CSIM
** 
** 140   10/20/98 5:34p Atai
** added #ifdefs for hwc
** 
** 139   10/19/98 2:11p Peter
** ctrisetup happiness
** 
** 138   10/09/98 6:57p Peter
** dynamic 3DNow!(tm) mods
** 
** 137   9/24/98 7:40p Dow
** Quake Driver Stuff
** 
** 136   9/24/98 12:01p Dow
** Turned on extra unmentionable games
** 
** 135   9/21/98 4:00p Dow
** Added to the unmentionable
** 
** 134   9/04/98 11:36a Peter
** re-open fix for nt (thanks to taco/rob/nt bob)
** 
** 133   8/30/98 11:15a Atai
** added tigerwood 99 to the game list
** 
** 132   8/27/98 6:35p Atai
** getenv FX_GLIDE_TMU_MEMSIZE
** 
** 131   8/27/98 1:58p Peter
** fill in hwConfig union"
** 
** 130   8/26/98 10:08p Atai
** return the correct reg path
** 
** 129   8/20/98 10:08a Dow
** Fix for registry GETENV stuff
** 
** 128   8/14/98 10:25a Dow
** Fixed hwConfig union effage
** 
** 127   7/24/98 2:03p Dow
** AGP Stuff
** 
** 126   7/23/98 1:17a Dow
** Bump & Grind
** 
** 125   7/18/98 7:24p Mikec
** Made win32 calls invisible to DOS compilation.
** 
** 123   7/18/98 5:13p Mikec
** EAhack done.
** 
** 122   7/14/98 2:48p Mikec
** Added Glide environment variable FX_GLIDE_EMUL_RUSH allow Banshee glide
** to report itself as Rush to the application. Set it to 1 to enable Rush
** reporting. By default Glide still reports itself as Voodoo Graphics. 
** 
** 121   7/13/98 5:32p Dow
** GETENV from registry
** 
** 120   7/09/98 11:49a Jdt
** Fix fencing for dos build
** 
** 119   7/09/98 10:19a Dow
** Registry getenv
** 
** 118   7/06/98 7:05p Jdt
** initenvironment simplified
** 
** 117   7/06/98 11:06a Mikec
** Added fbiRev offset to distinguish Banshee from Voodoo.
** Banshee check:
** If (hwconfig.SSTs[0].sstBoard.VoodooConfig.fbiRev > 0x1000)
** 
** 116   6/24/98 11:16a Dow
** Fixed DLLMAin messages
** 
** 115   6/09/98 5:04p Dow
** %$#@!
** 
** 114   6/09/98 2:39p Mikec
** Fixed hal_csim to use the right reg addr (sstReg, cReg, etc.) in
** detectResources.
** 
** 113   6/04/98 6:53p Dow
** Resolutions to 1600x1200
** 
** 112   6/03/98 5:23p Dow
** Fixed DOS effage
** 
** 111   6/03/98 1:39p Dow
** dll main
** 
** 110   5/31/98 9:03a Dow
** 800x600 resolution
** 
** 109   5/22/98 2:37p Peter
** complete the lie that is glide2x on Banshee
** 
** 108   5/21/98 4:47p Dow
** Direct Register Writes Work
** 
** 107   5/18/98 3:20p Peter
** pts more resistant to changing rounding modes
** 
** 106   5/15/98 2:21p Dow
** Changed from Voodoo Rush to Voodoo
** 
** 105   5/12/98 2:42p Dow
** CSIM Stuff
** 
** 104   4/14/98 6:41p Peter
** Merge w/ cvg glide cleanup
** 
** 103   4/07/98 10:40p Dow
** LFB Fixes:  Round 1
** 
** 102   4/05/98 2:18p Dow
** DOS Glide Stuff
** 
** 101   4/03/98 2:11p Dow
** 
** 100   3/28/98 11:24a Dow
** itwoç
** 
** 99    3/11/98 8:28p Dow
** WinGlide
** 
** 97    2/08/98 3:08p Dow
** FIFO Works
** 
** 96    2/02/98 4:31p Dow
** IO w/o HAL now possible
** 
** 95    1/20/98 11:03a Peter
** env var to force triple buffering
 * 
 * 93    1/16/98 5:41p Peter
 * fixed sense of lod_dither
 * 
 * 92    1/14/98 10:22a Peter
 * no more hacks
 * 
 * 91    1/08/98 7:09p Peter
 * real hw stuff modulo makefile change
 * 
 * 90    1/07/98 11:18a Atai
 * remove GrMipMapInfo and GrGC.mm_table in glide3
 * 
 * 89    1/07/98 10:22a Peter
 * lod dithering env var
 * 
 * 88    12/17/97 10:08a Peter
 * fast system comdex twiddling
 * 
 * 87    12/09/97 4:20p Peter
 * 0x100 fbiRev ofset for v2
 * 
 * 86    12/09/97 12:20p Peter
 * mac glide port
 * 
 * 85    12/05/97 4:26p Peter
 * watcom warnings
 * 
 * 84    12/03/97 2:36p Peter
 * upped comdex reset defaults
 * 
 * 83    12/02/97 9:48a Dow
 * Removed some spurious code I inadvertantly added.
 * 
 * 82    11/21/97 6:24p Dow
 * Banshee Lying about being Rush stuf
 * 
 * 81    11/21/97 11:19a Dow
 * Made Banshee report Voodoo2
 * 
 * 80    11/20/97 6:39p Peter
 * fixed direct_exec w/ csim
 * 
 * 79    11/19/97 2:49p Peter
 * env vars in registry for win32
 * 
 * 78    11/17/97 4:55p Peter
 * watcom warnings/chipfield stuff
 * 
 * 77    11/15/97 7:43p Peter
 * more comdex silliness
 * 
 * 76    11/14/97 11:10p Peter
 * open vs hw init confusion
 * 
 * 75    11/14/97 5:02p Peter
 * more comdex stuff
 * 
 * 74    11/14/97 12:09a Peter
 * comdex thing and some other stuff
 * 
 * 73    11/12/97 9:54p Peter
 * fixed all the effage from new config
 * 
 * 72    11/12/97 9:37p Dow
 * Textures on Banshee half work
 * 
 * 71    11/12/97 9:22a Dow
 * h3 mods
 * 
 * 70    11/08/97 3:34p Peter
 * fixed stupid gdbg_info crasher
 * 
 * 69    11/04/97 4:00p Dow
 * Banshee Mods
 * 
 * 68    11/03/97 3:43p Peter
 * h3/cvg cataclysm
 * 
 * 67    11/01/97 12:11p Pgj
 * glide.dll ---> glide2x.dll
 * 
 * 66    10/31/97 8:53a Peter
 * last lying change, really
 * 
 * 65    10/30/97 3:42p Peter
 * protected the last bit of nonsense
 * 
 * 64    10/30/97 3:37p Peter
 * spoof sst1 shit
 * 
 * 63    10/29/97 2:45p Peter
 * C version of Taco's packing code
 * 
 * 62    10/23/97 5:28p Peter
 * sli fifo thing
 * 
 * 61    9/15/97 7:31p Peter
 * more cmdfifo cleanup, fixed normal buffer clear, banner in the right
 * place, lfb's are on, Hmmmm.. probably more
 * 
 * 60    9/10/97 10:13p Peter
 * fifo logic from GaryT, non-normalized fp first cut
 * 
 * 59    9/05/97 5:29p Peter
 * changes for direct hw
 * 
 * 58    9/01/97 3:18p Peter
 * correct integer rounding for pts
 * 
 * 57    8/30/97 5:59p Tarolli
 * init and hal fixups
 * 
 * 56    8/30/97 1:19p Peter
 * first cut at using blit to clear, more to come to do inner rects
 * 
 * 55    8/18/97 3:52p Peter
 * pre-hw arrival fixes/cleanup
 * 
 * 54    7/30/97 2:42p Peter
 * shared and sanitized
 * 
 * 53    7/28/97 2:41p Peter
 * turned sli code back on for cvg, but waiting for hal
 * 
 * 52    7/25/97 11:40a Peter
 * removed dHalf, change field name to match real use for cvg
 * 
 * 51    7/08/97 2:47p Peter
 * fixed merge stupidity from last checkin
 * 
 * 50    7/02/97 12:28p Peter
 * removed spurious NOP, tex dl
 * 
 * 49    6/24/97 4:02p Peter
 * proper cmd fifo placement
 * 
 * 48    6/23/97 4:46p Peter
 * fixed my effage
 * 47    6/23/97 4:43p Peter
 * cleaned up #defines etc for a nicer tree
**
*/

#include <stdio.h>
#include <string.h>

#include <3dfx.h>
#include <glidesys.h>

#define FX_DLL_DEFINITION
#include <fxdll.h>
#include <glide.h>
#include "fxglide.h"

#if (GLIDE_PLATFORM & GLIDE_SST_SIM)
#ifdef HAL_CSIM
#include <windows.h>
#if H3
#include <h3regs.h>
#endif
#include <csim.h>
#elif HSIM
#include <gsim.h>
#endif
#endif /* (GLIDE_PLATFORM & GLIDE_SST_SIM) */

/* Some macros to prevent RSI */
#define GC      _GlideRoot.GCs[ctx]
#define SST     _GlideRoot.hwConfig.SSTs[ctx]

#define kRevisionOffset  0x1000


#if GLIDE_DISPATCH_SETUP
/* Collection of all of the known procs for a given system */
static GrTriSetupProc _triSetupProcs[][NUM_TRI_PROC_LISTS][2] = 
{
  /* Default Procs */
  {
    { _trisetup_Default_Default, _trisetup_Default_cull },
#if GLIDE_PACKED_RGB
    { _trisetup_Default_rgb,     _trisetup_Default_cull_rgb },
    { _trisetup_Default_argb,    _trisetup_Default_cull_argb },
#endif /* GLIDE_PACKED_RGB */
  },
#if GL_AMD3D
  /* 3DNow!(tm) Procs */
  {
    { _trisetup_3DNow_Default, _trisetup_3DNow_cull },
#if GLIDE_PACKED_RGB
    { _trisetup_3DNow_rgb,     _trisetup_3DNow_cull_rgb },
    { _trisetup_3DNow_argb,    _trisetup_3DNow_cull_argb },
#endif /* GLIDE_PACKED_RGB */
  },
#endif /* GL_AMD3D */
};
#endif /* GLIDE_DISPATCH_SETUP */

static GrTexDownloadProc _texDownloadProcs[][2][4] = 
{
  /* Default Procs */
  { 
    {
      _grTexDownload_Default_8_1, 
      _grTexDownload_Default_8_2,
      _grTexDownload_Default_8_4,
      _grTexDownload_Default_8_WideS
    }, 
    {
      _grTexDownload_Default_16_1,
      _grTexDownload_Default_16_2,
      _grTexDownload_Default_16_WideS,
      _grTexDownload_Default_16_WideS
    }
  },
#if GL_AMD3D
  { 
    { 
      _grTexDownload_Default_8_1, 
      _grTexDownload_Default_8_2, 
      _grTexDownload_Default_8_4, 
      _grTexDownload_3DNow_MMX, 
    },
    {
      _grTexDownload_Default_16_1,
      _grTexDownload_Default_16_2,
      _grTexDownload_3DNow_MMX,
      _grTexDownload_3DNow_MMX,
    },
  }
#endif /* GL_AMD3D */
};

#if (GLIDE_PLATFORM & GLIDE_OS_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>


BOOL WINAPI 
DllMain(HANDLE hInst, ULONG  ul_reason_for_call, LPVOID lpReserved) 
{
  switch( ul_reason_for_call ) {
  case DLL_PROCESS_DETACH:
    GDBG_INFO(80, "DllMain: DLL_PROCESS_DETACH\n");
    grGlideShutdown();
    hwcUnmapMemory() ;
    break;
  case DLL_PROCESS_ATTACH:
    GDBG_INFO(80, "DllMain: DLL_PROCESS_ATTACH\n");
    break;
  case DLL_THREAD_ATTACH:
    GDBG_INFO(80, "DllMain: DLL_THREAD_ATTACH\n");
    break;
  case DLL_THREAD_DETACH:
    GDBG_INFO(80, "DllMain: DLL_THREAD_DETACH\n");
    break;
  default:
    GDBG_INFO(80, "DllMain: Unhandled message.\n");
    break;
  }
  
  return TRUE;

} /* DllMain */

#define REGSTR_PATH_3DFXSW              "Software\\3Dfx Interactive\\Voodoo2"
#define REGSTR_PATH_GLIDE               REGSTR_PATH_3DFXSW"\\Glide"

#if !defined(GLIDE_INIT_HAL)
#ifdef GETENV 
#undef GETENV
#endif
#define GETENV(a) hwcGetenv(a)
#endif /* !defined(GLIDE_INIT_HAL) */

#endif

/* Windows */
#define SEPARATOR '\\'
/* UNIX */
#define SEPARATOR2 '/'

/*
 * parseFilename
 *
 *      Return the file name portion of a filename/path.
 */

char *
_parseFilename(char *name)
{
    int i;

    if (name == NULL) 
      return NULL;
    for(i = strlen(name); i >= 0; i--)
        if ((name[i] == SEPARATOR) ||
            (name[i] == SEPARATOR2))
            return (name + i + 1);
    return name;
}  /* End of parseFilename*/

#define CHAR_CONST 65
#define MAX_GAME_LIST 24
static int _gamelist[MAX_GAME_LIST][50] = 
{
  {19, 15, -8, -8, 30, 17}, /* tp99_r triple play*/ 
  {51, 47, -8, -8, 30, 49}, /* lowercase*/
  {13, 7, 11, -8, -9, 3, 4, 12}, /* nhl98dem */
  {45, 39, 43, -8, -9, 35, 36, 44}, /* lower */
  {13, 7, 11, -8, -9},  /* nhl98 */
  {45, 39, 43, -8, -9}, /* lower */
  {5, 8, 5, 0, 17, 19, 22, 2}, /* fifartwc fifa demo+game */
  {37, 40, 37, 32, 49, 51, 54, 34}, /* lower */
  {13, 1, 0, 22, 8, 13}, /* nbawin nba98 */
  {45, 33, 32, 54, 40, 45}, /* lower */
  {22, 2, 3, 4, 12, 14}, /* wcdemo world cup 98 */
  {54, 34, 35, 36, 44, 46}, /* lower */
  {22, 2, -8, -9}, /* wc98 (guessing name of world cup 98) */
  {54, 34, -8, -9}, /* lower */
  {6, 0, 12, 4, 12, 0, 8, 13}, /* tigerwood 99 demo */
  {38, 32, 44, 36, 44, 32, 40, 45}, /* lower */
  {'G' - CHAR_CONST, 'L' - CHAR_CONST, 'Q'- CHAR_CONST, 'U' - CHAR_CONST, 'A' - CHAR_CONST, 'K' - CHAR_CONST, 'E' - CHAR_CONST},
  {'g' - CHAR_CONST, 'l' - CHAR_CONST, 'q'- CHAR_CONST, 'u' - CHAR_CONST, 'a' - CHAR_CONST, 'k' - CHAR_CONST, 'e' - CHAR_CONST},
  {'S' - CHAR_CONST, 'I' - CHAR_CONST, 'N' - CHAR_CONST},
  {'s' - CHAR_CONST, 'i' - CHAR_CONST, 'n' - CHAR_CONST},  
  {'G' - CHAR_CONST, 'L' - CHAR_CONST, 'H' - CHAR_CONST, '2' - CHAR_CONST},
  {'g' - CHAR_CONST, 'l' - CHAR_CONST, 'h' - CHAR_CONST, '2' - CHAR_CONST},
  {'Q' - CHAR_CONST, 'U' - CHAR_CONST, 'A' - CHAR_CONST, 'K' - CHAR_CONST, 'E' - CHAR_CONST, '2' - CHAR_CONST},
  {'q' - CHAR_CONST, 'u' - CHAR_CONST, 'a' - CHAR_CONST, 'k' - CHAR_CONST, 'e' - CHAR_CONST, '2' - CHAR_CONST}
};

int
_eadecrypt(char *name)
{
  int i, j;

  for (j = 0; j < MAX_GAME_LIST; j++){
    int success = 1;
    for (i = 0; i < (int)strlen(name)-4; i++){
      if (name[i] != _gamelist[j][i] + 65){
        success = 0;
        break;
      }
    }
    if (success) {
      return 1;
    }
  }
  return 0;
}


void
checkEmulRush()
{
#if defined( __WATCOMC__ ) || defined(macintosh)
  /* Nothing for DOS */
#else 
/* windows only */
  HMODULE module;
  module = GetModuleHandle(NULL);
  if (module) {
    char pathname[256];
    char *fname;
    GDBG_INFO(80,"Got module handle\n");
    strcpy(pathname, "deadbeef");
    GetModuleFileName(module, pathname, 256);
    fname = _parseFilename(pathname);
    if (fname != NULL){
      GDBG_INFO(80,"module name %s\n", fname);
      if (_eadecrypt(fname)){
        GDBG_INFO(80,"Rush game found\n");
        _GlideRoot.environment.emulRush = 1;
      }
    }
  }
#endif
}

/*
** For EA games, we report Voodoo3 fbi and tmu rev number as Banshee
** to work around a bug in Frank's library.
*/
#define MAX_REV_GAME_LIST 18
static int _revgamelist[MAX_REV_GAME_LIST][50] = 
{
  {19, 15, -8, -8, 30, 17}, /* tp99_r triple play*/ 
  {51, 47, -8, -8, 30, 49}, /* lowercase*/
  {13, 7, 11, -8, -9, 3, 4, 12}, /* nhl98dem */
  {45, 39, 43, -8, -9, 35, 36, 44}, /* lower */
  {13, 7, 11, -8, -9},  /* nhl98 */
  {45, 39, 43, -8, -9}, /* lower */
  {5, 8, 5, 0, 17, 19, 22, 2}, /* fifartwc fifa demo+game */
  {37, 40, 37, 32, 49, 51, 54, 34}, /* lower */
  {13, 1, 0, 22, 8, 13}, /* nbawin nba98 */
  {45, 33, 32, 54, 40, 45}, /* lower */
  {22, 2, 3, 4, 12, 14}, /* wcdemo world cup 98 */
  {54, 34, 35, 36, 44, 46}, /* lower */
  {22, 2, -8, -9}, /* wc98 (guessing name of world cup 98) */
  {54, 34, -8, -9}, /* lower */
  {6, 0, 12, 4, 12, 0, 8, 13}, /* tigerwood 99 demo */
  {38, 32, 44, 36, 44, 32, 40, 45}, /* lower */
  {'F' - CHAR_CONST, 'I' - CHAR_CONST, 'F' - CHAR_CONST, 'A' - CHAR_CONST, '9' - CHAR_CONST, '9' - CHAR_CONST},
  {'f' - CHAR_CONST, 'i' - CHAR_CONST, 'f' - CHAR_CONST, 'a' - CHAR_CONST, '9' - CHAR_CONST, '9' - CHAR_CONST}
};

int
_revdecrypt(char *name)
{
  int i, j;

  for (j = 0; j < MAX_REV_GAME_LIST; j++){
    int success = 1;
    for (i = 0; i < (int)strlen(name)-4; i++){
      if (name[i] != _revgamelist[j][i] + 65){
        success = 0;
        break;
      }
    }
    if (success) {
      return 1;
    }
  }
  return 0;
}

int 
revCheck(void)
{
#if defined( __WATCOMC__ ) || defined(macintosh)
  /* Nothing for DOS */
  return 0;
#else 
/* windows only */
  HMODULE module;
  int rv = 0;
  module = GetModuleHandle(NULL);
  if (module) {
    char pathname[256];
    char *fname;
    GDBG_INFO(80,"Got module handle\n");
    strcpy(pathname, "deadbeef");
    GetModuleFileName(module, pathname, 256);
    fname = _parseFilename(pathname);
    if (fname != NULL){
      GDBG_INFO(80,"module name %s\n", fname);
      if (_revdecrypt(fname)){
        GDBG_INFO(80,"fbi tmu rev game found\n");
        rv = 1;
      }
    }
  }
  return rv;
#endif
}

/*
** Some more EA games try to open a 1024x768 display mode
** and then fail to render to it properly.
*/
#define MAX_WIN_GAME_LIST 2
static int _wingamelist[MAX_WIN_GAME_LIST][50] = 
{
  {'A' - CHAR_CONST, 'N' - CHAR_CONST, 'D' - CHAR_CONST, 'R' - CHAR_CONST, 'E' - CHAR_CONST, 'T' - CHAR_CONST, 'T' - CHAR_CONST, 'I' - CHAR_CONST},
  {'a' - CHAR_CONST, 'n' - CHAR_CONST, 'd' - CHAR_CONST, 'r' - CHAR_CONST, 'e' - CHAR_CONST, 't' - CHAR_CONST, 't' - CHAR_CONST, 'i' - CHAR_CONST},
};

int
_windecrypt(char *name)
{
  int i, j;

  for (j = 0; j < MAX_WIN_GAME_LIST; j++){
    int success = 1;
    for (i = 0; i < (int)strlen(name)-4; i++){
      if (name[i] != _wingamelist[j][i] + 65){
        success = 0;
        break;
      }
    }
    if (success) {
      return 1;
    }
  }
  return 0;
}

int 
winCheck(void)
{
#if defined( __WATCOMC__ ) || defined(macintosh)
  /* Nothing for DOS */
  return 0;
#else 
/* windows only */
  HMODULE module;
  int rv = 0;
  module = GetModuleHandle(NULL);
  if (module) {
    char pathname[256];
    char *fname;
    GDBG_INFO(80,"Got module handle\n");
    strcpy(pathname, "deadbeef");
    GetModuleFileName(module, pathname, 256);
    fname = _parseFilename(pathname);
    if (fname != NULL){
      GDBG_INFO(80,"module name %s\n", fname);
      if (_windecrypt(fname)){
        GDBG_INFO(80,"fbi tmu rev game found\n");
        rv = 1;
      }
    }
  }
  return rv;
#endif
}

/*-------------------------------------------------------------------
  Function: _grSstDetectResources
  Date: --
  Implementor(s): Dow, Gmt, Jdt
  Library: Glide
  Description:
  Discover devices on the PCI bus.
  Discover configuration of detected devices.
  Initialize all Glide GC's 

  Recognized devices depend upon compile time flags

  This code should NOT initialize the hardware 
  any more than is necessary for discovery of 
  configuration

  Arguments: none
  Return: 
  FXTRUE  - at least one device was detected
  FXFALSE - no devices were detected.
  -------------------------------------------------------------------*/
FxBool 
_grSstDetectResources(void)
{
  static FxBool calledP = FXFALSE;
  FxBool rv = FXFALSE;
  FxU32 ctx;
  FxU32 chipCount = 1;


  GDBG_INFO(280, "_grSstDetectResources()\n");

  if (!calledP) {
#if GLIDE_INIT_HAL
    FxU32 count = HAL_MAX_BOARDS;
    SstRegs* devRegs;

    /* The first time through the init code we need to map 
     * all of the boards. Future calls can just grab this
     * info out of the halInfo that we have here.
     */
    FxU32     device;
    HalInfo* halInfo = fxHalInit(0);
    if (halInfo == NULL) goto __errExit;
    
    {
      /*
      ** hack alert!!
      ** by default, there is no device around so we hack the device 
      ** number in here (copy code from diag).
      */
      char* envChipNum = GETENV("FX_GLIDE_NUM_CHIPS");
      FxBool useMultiFunctionDevices=FXFALSE;
      FxU32 busNumber, deviceNumber, functionNumber;
      FxU32 counter;

      if (envChipNum)
        chipCount = atoi(envChipNum);

      switch(chipCount) {
      case 4:
        busNumber      = 3;
        deviceNumber   = 2;
        functionNumber = 0;
        halInfo->boardInfo[2].pciBusNumber = busNumber;
        halInfo->boardInfo[2].pciDeviceNumber = deviceNumber;
        halInfo->boardInfo[2].pciFunctionNumber = functionNumber; 
        halInfo->boardInfo[2].deviceNumber = busNumber * 32 + deviceNumber;

        busNumber      = 3;
        deviceNumber   = 2;
        functionNumber = 1;
        halInfo->boardInfo[3].pciBusNumber = busNumber;
        halInfo->boardInfo[3].pciDeviceNumber = deviceNumber;
        halInfo->boardInfo[3].pciFunctionNumber = functionNumber; 
        halInfo->boardInfo[3].deviceNumber = busNumber * 32 + deviceNumber;

      case 2:
        busNumber      = 2;
        deviceNumber   = 1;
        functionNumber = 1;
        halInfo->boardInfo[1].pciBusNumber = busNumber;
        halInfo->boardInfo[1].pciDeviceNumber = deviceNumber;
        halInfo->boardInfo[1].pciFunctionNumber = functionNumber; 
        halInfo->boardInfo[1].deviceNumber = busNumber * 32 + deviceNumber;

        useMultiFunctionDevices=FXTRUE;
      case 1:
      default:
        busNumber      = 2;
        deviceNumber   = 1;
        functionNumber = 0;
        halInfo->boardInfo[0].pciBusNumber = busNumber;
        halInfo->boardInfo[0].pciDeviceNumber = deviceNumber;
        halInfo->boardInfo[0].pciFunctionNumber = functionNumber; 
        halInfo->boardInfo[0].deviceNumber = busNumber * 32 + deviceNumber;
        break;
      }
      /* Initialize the csim boards/chips */
      for(counter=0; counter<chipCount; counter++) {
        halInfo->boardInfo[counter].sstCSIM = csimInit(counter);      
        if(useMultiFunctionDevices)
          csimMakeMultiFunctionDevice(halInfo->boardInfo[counter].sstCSIM);
        CSIM_PRIVATE(halInfo->boardInfo[counter].sstCSIM)->environment.chipCount = chipCount;
        CSIM_PRIVATE(halInfo->boardInfo[counter].sstCSIM)->environment.sliBandHeight 
          = _GlideRoot.environment.sliBandHeight;

        /*
        if(counter == 0)
          diago.sstCSIM = diago.halInfo->boardInfo[counter].sstCSIM;
        else
          diago.sstChildrenCSIM[counter-1] = diago.halInfo->boardInfo[counter].sstCSIM;
        */
      }
    }


#if (GLIDE_PLATFORM & GLIDE_SST_SIM)
    /* The simulator can support any number of boards through
     * successive calls to fxHalMapBoard.
     */
    {
      const char* envBoardNum = GETENV("FX_SIM_BOARDS");
      FxU32 temp;
        
      count = (((envBoardNum != NULL) && (sscanf(envBoardNum, "%ld", &temp) == 1)) 
               ? temp
               : 1);
      if (count > HAL_MAX_BOARDS) {
        GDBG_INFO(0, "Error: FX_SIM_BOARDS(%ld) > %d. Using %d.\n",
                  count, HAL_MAX_BOARDS, HAL_MAX_BOARDS);
        count = HAL_MAX_BOARDS;
      }
      count *= chipCount ;
    }
#endif /* (GLIDE_PLATFORM & GLIDE_SST_SIM) */

     for(ctx = device = 0; device < count; device++) {
      devRegs = fxHalMapBoard(device);
    }     

    for(ctx = device = 0; device < count; device++) {
      /* See RSI-prevention macros for usage of [ctx] */
      const FxDeviceInfo* curDev = NULL;
      FxBool regInitP = FXFALSE;

      devRegs = fxHalMapBoard(device);
      if ((device % chipCount) == 0)
        GC.is_master = FXTRUE;
      else
        GC.is_master = FXFALSE;

      GC.chipCount  = chipCount;
      GC.sliBandHeight = _GlideRoot.environment.sliBandHeight;

      curDev = halInfo->boardInfo + device;

      GC.halInfo = halInfo; 

      if (devRegs != NULL) {
        FxU32 tmuMem = 0x00;

        SST.type = GR_SSTTYPE_VOODOO;

        if (!fxHalInitRegisters(curDev->virtAddr[0])) goto __errRegFailure;
          
#if USE_PACKET_FIFO && GLIDE_DEBUG && HAL_CSIM
        if ((halInfo->csim == 1) && (GETENV("FX_FIFO_DIRECT_EXEC") != NULL)) {
          //            halInfo->csim = -1;
          GDBG_INFO(80, "Turning on direct fifo execution\n");
        }
#endif /* USE_PACKET_FIFO && GLIDE_DEBUG && HAL_CSIM */

#ifdef DIRECT_IO
        GC.sstRegs =
          (SstRegs *) (curDev->physAddr[0] + 0x200000);
        GC.ioRegs = (SstIORegs *) (curDev->physAddr[0]);
        GC.cRegs = (SstCRegs *) (curDev->physAddr[0] + 0x80000);
        GC.rawLfb = (FxU32 *) curDev->physAddr[1];
#endif

        /* This device is ready to go. */
        regInitP = FXTRUE;

        GC.reg_ptr   = (FxU32*)devRegs;

#ifdef HAL_CSIM

        /* Set up pointers to the various address spaces within the hw */
/*        GC.base_ptr  = (FxU32*)SST_BASE_ADDRESS(devRegs);
        GC.lfb_ptr   = (FxU32*)SST_LFB_ADDRESS(devRegs);
        GC.tex_ptr   = (FxU32*)SST_TEX_ADDRESS(devRegs); */
        GC.base_ptr  = (FxU32*)SST_BASE_ADDRESS(curDev->physAddr[0]);
        GC.reg_ptr   = (FxU32*)devRegs;
        GC.lfb_ptr   = (FxU32*)SST_LFB_ADDRESS(curDev->physAddr[0]);
        GC.tex_ptr   = (FxU32*)SST_TEX_ADDRESS(curDev->physAddr[0]);

        /* Forces sstRegs, ioRegs to be offset off devRegs */
        GC.sstRegs =
          (SstRegs *) devRegs;
        GC.ioRegs = (SstIORegs *) SST_IO_ADDRESS(devRegs);
        GC.cRegs = (SstCRegs *) SST_CMDAGP_ADDRESS(devRegs);
        GC.gRegs = (SstGRegs *) SST_GUI_ADDRESS(devRegs);
#define SST_RAW_LFB_ADDRESS(sst)    (SST_RAW_LFB_OFFSET+SST_BASE_ADDRESS(sst))
        GC.rawLfb = (FxU32 *) SST_RAW_LFB_ADDRESS(devRegs);
        /* GC.rawLfb = (FxU32 *) SST_LFB_ADDRESS(devRegs); */

#ifdef FX_GLIDE_NAPALM
        /* force the deviceID to 7 for code development */
        {
          static hwcBoardInfo tmpbInfo;
          GC.bInfo = &tmpbInfo;
          GC.bInfo->pciInfo.deviceID = SST_DEVICE_ID_AP_OEM;
        }
#endif

#endif  


#ifdef HAL_HW
        /* Set up pointers to the various address spaces within the hw */
        GC.base_ptr  = (FxU32*)SST_BASE_ADDRESS(curDev->physAddr[0]);
        GC.lfb_ptr   = (FxU32*)SST_LFB_ADDRESS(curDev->physAddr[0]);
        GC.tex_ptr   = (FxU32*)SST_TEX_ADDRESS(curDev->physAddr[0]);
#endif        

        /* Video parameters */
        GC.grSstRez     = GR_RESOLUTION_NONE;
        GC.grSstRefresh = curDev->fbiVideoRefresh;

        GC.scanline_interleaved = (GC.chipCount > 1) ;
        
        /* Chip configuration */
        GC.num_tmu              = curDev->numberTmus;
        GC.fbuf_size            = curDev->fbiMemSize;
        
        _GlideRoot.hwConfig.num_sst++;

        /* We claim that we are an sst1 for the sake of Avenger */
        {
          const FxU32 curTmuMemSize = 0x2;
              
#if 0
          SST.sstBoard.VoodooConfig.fbRam = 2;
#else
          SST.sstBoard.VoodooConfig.fbRam = 16;
#endif

          /* Banshee's ID is 0x1000. Always check that it's at least 0x1000
           * for Banshee card. 
           */
#if 0
          /* H4 Note: need to setup 2nd tmu configuration */
          SST.sstBoard.VoodooConfig.fbiRev = 2 + 0x1000;
          SST.sstBoard.VoodooConfig.nTexelfx = 1;

          SST.sstBoard.VoodooConfig.tmuConfig[0].tmuRev = (2 + kRevisionOffset);
              
          SST.sstBoard.VoodooConfig.tmuConfig[0].tmuRam = curTmuMemSize;
#else
          SST.sstBoard.VoodooConfig.fbiRev = curDev->fbiRevision;
          SST.sstBoard.VoodooConfig.nTexelfx = GC.num_tmu;
          SST.sstBoard.VoodooConfig.tmuConfig[0].tmuRev = curDev->tmuRevision;
          SST.sstBoard.VoodooConfig.tmuConfig[0].tmuRam = curTmuMemSize;
          SST.sstBoard.VoodooConfig.tmuConfig[1].tmuRev = curDev->tmuRevision;
              
          SST.sstBoard.VoodooConfig.tmuConfig[1].tmuRam = curTmuMemSize;
#endif
          tmuMem += curTmuMemSize;

          /* Clear the tmu state */
          memset(&GC.tmu_state[0], 0, sizeof(GC.tmu_state[0]));       
          GC.tmu_state[0].total_mem    = (curTmuMemSize << 20);              
          GC.tmu_state[0].ncc_mmids[0] = 
          GC.tmu_state[0].ncc_mmids[1] = GR_NULL_MIPMAP_HANDLE;
#ifdef HAL_CSIM
          /* H4 Note: need to add info for 2nd tmu */
          memset(&GC.tmu_state[1], 0, sizeof(GC.tmu_state[1]));       
          GC.tmu_state[1].total_mem    = (curTmuMemSize << 20);              
          GC.tmu_state[1].ncc_mmids[0] = 
          GC.tmu_state[1].ncc_mmids[1] = GR_NULL_MIPMAP_HANDLE;
#endif
        }
          
        rv = FXTRUE;
        ctx++;
      }

  __errRegFailure:
      /* Either this is not the hw we're expecting, or we could not
       * init/map the board for some reason. Either way try to cleanup.  
       */
      if (!regInitP && (devRegs != NULL)) {
        fxHalShutdown(devRegs);
      }
    }

    /* Done setting up. Don't do the silly mapping thing again. */
#else  /* GLIDE_INIT_HWC */
    /* There's a left brace before the #if */
    hwcBoardInfo
      *bInfo;
    hwcInfo
      *hInfo;                   /* Info about all the relavent boards */


    if ((hInfo = hwcInit(0x121a, 0xB)) == NULL) {    /* Voodoo4/5 (Napalm 2) */
       if ((hInfo = hwcInit(0x121a, 0x9)) == NULL) {    /* Voodoo4/5 (Napalm) */
          if ((hInfo = hwcInit(0x121a, 0x5)) == NULL) {    /* Voodoo3 */
             if ((hInfo = hwcInit(0x121a, 0x3)) == NULL) {  /* Banshee */
                goto __errExit; 
             }
          }
       }
    }

    checkEmulRush();
    if (_GlideRoot.environment.emulRush){
      GDBG_INFO(80,"Emulating rush\n");
    }
    
    /* Iterate through boards found */
    for (ctx = 0; ctx < hInfo->nBoards; ctx++) {
      bInfo = &hInfo->boardInfo[ctx];

      GC.bInfo = bInfo;


      GDBG_INFO(80, "Board %d, devRev: %d\n", ctx,bInfo->devRev);

      if (!hwcMapBoard(bInfo, HWC_BASE_ADDR_MASK)) {
        GrErrorCallback(hwcGetErrorString(), FXTRUE);
      }

      if (!hwcInitRegisters(bInfo)) {
        GrErrorCallback(hwcGetErrorString(), FXTRUE);
      }
      
      /* NB: We cannot fail to map this board after this point */
      /* GC.hwInitP = FXTRUE; */

      GC.sstRegs = (SstRegs *) bInfo->regInfo.sstBase;
      GC.ioRegs = (SstIORegs *) bInfo->regInfo.ioMemBase;
      GC.cRegs = (SstCRegs *) bInfo->regInfo.cmdAGPBase;
      GC.lfb_ptr = (FxU32 *) bInfo->regInfo.lfbBase;
      GC.rawLfb = (FxU32 *) bInfo->regInfo.rawLfbBase;
      GC.tex_ptr = (FxU32*)SST_TEX_ADDRESS(bInfo->regInfo.sstBase);

      GC.chipCount  = bInfo->pciInfo.numChips;
      GC.sliBandHeight = _GlideRoot.environment.sliBandHeight;

#ifdef FX_GLIDE_NAPALM
      // AJB- Point at slave chip regs
      {
        FxU32 chip ;

        if (GC.chipCount)
          for (chip = 0 ;
               chip < GC.chipCount - 1 ;
               chip++) {
            GC.slaveSstRegs[chip] = (SstRegs *) bInfo->regInfo.slaveSstBase[chip] ;
            GC.slaveCRegs[chip] = (SstCRegs *) bInfo->regInfo.slaveCmdBase[chip] ;
          }
      }
#endif

      /* Video Parameters */
      GC.grSstRez = GR_RESOLUTION_NONE;
      GC.grSstRefresh = 0L;

      GC.scanline_interleaved = FXFALSE;

      switch (hInfo->boardInfo[ctx].pciInfo.deviceID) {
      case SST_DEVICE_ID_H3: /* Banshee */
        GC.num_tmu = 1;
        GC.fbuf_size = (bInfo->h3Mem - 2);
        break;
      case SST_DEVICE_ID_H4: /* Avenger low speed */
        /*
        ** For 8M board, we may only use one tmu for higher resolution.
        ** Need to re-visit the issue. 11/5/98
        */ 
        GC.num_tmu = 2;
        GC.fbuf_size = (bInfo->h3Mem - 4);
        break;
      case SST_DEVICE_ID_H4_OEM: /* Avenger high speed */
      case SST_DEVICE_ID_AP: /* Napalm */
        GC.num_tmu = 2;
        GC.fbuf_size = (bInfo->h3Mem - 4);
        break;
      default:
        if (IS_NAPALM(hInfo->boardInfo[ctx].pciInfo.deviceID))
        {
          /* Napalm->Napalm2 */
          GC.num_tmu = 2;
          GC.fbuf_size = (bInfo->h3Mem - 4);
        }
         else
        {
          GC.num_tmu = 1;
          GC.fbuf_size = (bInfo->h3Mem - 2);
        }
        break;
      }
      if (bInfo->h3Mem == 4) {
        GC.num_tmu = 1;
        GC.fbuf_size = (bInfo->h3Mem - 2);
      }
      if (GETENV("FX_GLIDE_NUM_TMU")) {
        int num_tmu = atoi(GETENV("FX_GLIDE_NUM_TMU"));
        switch (num_tmu) {
        case 1:
          GC.num_tmu = 1;
          GC.fbuf_size = (bInfo->h3Mem - 2);
          break;
        case 2:
          GC.num_tmu = 2;
          GC.fbuf_size = (bInfo->h3Mem - 4);
          break;
        }         
      }

      _GlideRoot.hwConfig.num_sst++;

      {
        /* Default to the minimum texture memory taht we will
         * advertise for any screen resolution.  
         */
        const FxU32 curTmuMemSize = 0x2;
        int tmu;

        if (_GlideRoot.environment.emulRush) {
          SST.type = GR_SSTTYPE_SST96;
          SST.sstBoard.SST96Config.fbRam =  GC.fbuf_size;

          SST.sstBoard.SST96Config.nTexelfx = GC.num_tmu;
          SST.sstBoard.SST96Config.tmuConfig.tmuRev = (2 + kRevisionOffset);
          SST.sstBoard.SST96Config.tmuConfig.tmuRam = curTmuMemSize;
        } else {
          /* Banshee's ID is 0x1000. Always check that it's at least
           * 0x1000 for Banshee card.  
           */
          SST.type = GR_SSTTYPE_VOODOO;
          SST.sstBoard.VoodooConfig.fbRam = GC.fbuf_size;
          SST.sstBoard.VoodooConfig.fbiRev = 2 + 0x1000;
          SST.sstBoard.VoodooConfig.sliDetect = FXFALSE;

          SST.sstBoard.VoodooConfig.nTexelfx = GC.num_tmu;
          for (tmu = 0; tmu < GC.num_tmu; tmu++) {
            SST.sstBoard.VoodooConfig.tmuConfig[tmu].tmuRev = (2 + kRevisionOffset);
            SST.sstBoard.VoodooConfig.tmuConfig[tmu].tmuRam = curTmuMemSize;
          }
        }

        /* Clear the tmu state */
        for (tmu = 0; tmu < GC.num_tmu; tmu++) {
          memset(&GC.tmu_state[tmu], 0, sizeof(GC.tmu_state[tmu]));       
          GC.tmu_state[tmu].total_mem    = (curTmuMemSize << 20);              
          GC.tmu_state[tmu].ncc_mmids[0] =
            GC.tmu_state[tmu].ncc_mmids[1] = GR_NULL_MIPMAP_HANDLE;
        }
      }
      if (hInfo->boardInfo[ctx].pciInfo.deviceID > SST_DEVICE_ID_H3) {
        /* Report as Banshee for EA games */
        if (!revCheck()) {
          if (_GlideRoot.environment.emulRush) {
            SST.sstBoard.SST96Config.tmuConfig.tmuRev |= 0x10000;
          }
          else {
            SST.sstBoard.VoodooConfig.fbiRev |= 0x10000;
            SST.sstBoard.VoodooConfig.tmuConfig[0].tmuRev |= 0x10000;
            SST.sstBoard.VoodooConfig.tmuConfig[1].tmuRev |= 0x10000;
          }
        }
      }

    } /* iterate through boards found */
#endif
  }

  /* Did we previously find boards? */
  rv = (_GlideRoot.hwConfig.num_sst != 0);

  calledP = FXTRUE;

  goto __errExit; /* Keep warnings happy */
__errExit:
  ;
  return rv;
} /* _grSstDetectResources */


static void
displayBoardInfo(int i, GrHwConfiguration *hwc)
{
  if ((hwc->SSTs[i].type == GR_SSTTYPE_VOODOO) ||
      (hwc->SSTs[i].type == GR_SSTTYPE_Voodoo2)) {
    int tmuNum;

    GDBG_INFO(80,"SST board %d: 3Dfx Voodoo%s\n", 
              i, ((hwc->SSTs[i].type == GR_SSTTYPE_VOODOO) ? " Graphics" : "^2"));
    if (hwc->SSTs[i].sstBoard.VoodooConfig.sliDetect) {
      GDBG_INFO(80,"\tScanline Interleaved\n");
    }

    GDBG_INFO(80,"\tPixelfx rev 0x%lX with %d MB Frame Buffer\n",
              hwc->SSTs[i].sstBoard.VoodooConfig.fbiRev,
              hwc->SSTs[i].sstBoard.VoodooConfig.fbRam);
    GDBG_INFO(80,"\t%d Texelfx chips:\n",
              hwc->SSTs[i].sstBoard.VoodooConfig.nTexelfx);
    for (tmuNum = 0;
         tmuNum < hwc->SSTs[i].sstBoard.VoodooConfig.nTexelfx;
         tmuNum++) {
      GDBG_INFO(80,"\t\tTexelfx %d: Rev 0x%lX, %d MB Texture\n", tmuNum,
                hwc->SSTs[i].sstBoard.VoodooConfig.tmuConfig[tmuNum].tmuRev,
                hwc->SSTs[i].sstBoard.VoodooConfig.tmuConfig[tmuNum].tmuRam);
    }
  } else if (hwc->SSTs[i].type == GR_SSTTYPE_SST96) {
    GDBG_INFO(80,"SST board %d: 3Dfx Voodoo Rush\n", i);
    GDBG_INFO(80,"\tFBI Jr. with %d MB Frame Buffer\n",
              hwc->SSTs[i].sstBoard.SST96Config.fbRam);
    GDBG_INFO(80,"\tTexelfx chips:  1\n");
  } else {
    GDBG_INFO(80,"error: SSTs %d: unknown type\n",i);
  }
} /* displayBoardInfo */


#if defined( __WATCOMC__ )
FxU32 p6FenceVar;
#endif

void
_GlideInitEnvironment(void)
{
#define FN_NAME "_GlideInitEnvironment"
  int i;
  FxU32 ditherMode;
    const char* envStr;
#if GLIDE_PLATFORM & GLIDE_OS_WIN32
  OSVERSIONINFO ovi;
#endif
  

  if (_GlideRoot.initialized)          /* only execute once */
    return;
  GDBG_INIT();                          /* init the GDEBUG libraray */
  GDBG_INFO(80,"%s()\n", FN_NAME);
  GDBG_INFO(0,"GLIDE DEBUG LIBRARY\n"); /* unconditional display */

  if (_GlideRoot.initialized)           /* only execute once */
    return;
  
#if GLIDE_PLATFORM & GLIDE_OS_WIN32
  ovi.dwOSVersionInfoSize = sizeof ( ovi );
  GetVersionEx ( &ovi );
  if (ovi.dwPlatformId == VER_PLATFORM_WIN32_NT)
    _GlideRoot.OSWin95 = 0;
  else
    _GlideRoot.OSWin95 = 1;
#endif

  GDBG_INFO(80, "%s:  OS = %s\n", FN_NAME, _GlideRoot.OSWin95 ? "W9X" : "WNT");

#if defined(FX_DLL_ENABLE) && (GLIDE_PLATFORM & GLIDE_OS_WIN32)
    { /* GMT: display the DLL pathname for sanity checking */
        char buf[132] = "failed";
        GetModuleFileName(GetModuleHandle("glide2x.dll"), buf, sizeof(buf));
        GDBG_INFO(0,"DLL path: %s\n",buf);
    }
#endif

    /* Check for user environment tweaks */
#define GLIDE_GETENV(__envVar, __defVal) \
    (((envStr = GETENV(__envVar)) == NULL) ? (__defVal) : atol(envStr))
#define GLIDE_FGETENV(__envVar, __defVal) \
    (((envStr = GETENV(__envVar)) == NULL) ? (__defVal) : (float) atof(envStr))
#define GLIDE_34GETENV(__envVar, __defVal) \
  (((signed char)(atof(((envStr = GETENV(__envVar)) == NULL) ? (__defVal) : (envStr))*16.0f)+8)&0x7f)

    
    _GlideRoot.environment.triBoundsCheck =
      (GETENV("FX_GLIDE_BOUNDS_CHECK") != NULL);
  GDBG_INFO(80,"\ttriBoundsCheck: %d\n",
            _GlideRoot.environment.triBoundsCheck); 
  
  _GlideRoot.environment.noSplash =
    (GETENV("FX_GLIDE_NO_SPLASH") != NULL);
  GDBG_INFO(80,"\tnoSplash: %d\n",_GlideRoot.environment.noSplash);
  
  _GlideRoot.environment.shamelessPlug =
    (GETENV("FX_GLIDE_SHAMELESS_PLUG") != NULL);
  GDBG_INFO(80,"\tshamelessPlug: %d\n",
            _GlideRoot.environment.shamelessPlug); 
  
  _GlideRoot.environment.ignoreReopen = 
    (GETENV("FX_GLIDE_IGNORE_REOPEN") != NULL);
  GDBG_INFO(80,"\tignoreReopen: %d\n", _GlideRoot.environment.ignoreReopen);
  
  _GlideRoot.environment.disableDitherSub =
    (GETENV("FX_GLIDE_NO_DITHER_SUB") != NULL);
  GDBG_INFO(80,"\tdisableDitherSub: %d\n",
            _GlideRoot.environment.disableDitherSub); 

  /*
   * AJB- 7/23/99 support forcing 16/32bpp rendering
   */
  _GlideRoot.environment.outputBpp = GLIDE_GETENV("FX_GLIDE_BPP", 16L) ;

  /*
   * AJB-  Support the slightly silly way the DirectX gang controls
   *       SLI & AA from the 3dfx tools control panel, just to make
   *       life a little easier for Reid.
   *
   *       Here is a breakdown of the bizarre table of magic numbers:
   *
   *       0 - SLI & AA disable
   *       1 - SLI disabled, 2 sample AA enabled
   *       2 - 2-way SLI enabled, AA disabled
   *       3 - 2-way SLI enabled, 2 sample AA enabled
   *       4 - SLI disabled, 4 sample AA enabled
   *       5 - 4-way SLI enabled, AA disabled
   *       6 - 4-way SLI enabled, 2 sample AA enabled
   *       7 - 2-way SLI enabled, 4 sample AA enabled
   *
   *       to add to the silliness:
   *
   *       settings 0 & 1 are valid for all configurations
   *       settings 2, 3 & 4 are valid for 2 chip boards only
   *       settings 5, 6 & 7 are valid for 4 chip boards only
   *
   *       to make life easier on everyone, we won't enforce the board
   *       type restriction and we will default to use whatever SLI
   *       is available with no AA if the variable cannot be found.
   */
  _GlideRoot.environment.forceSingleChip = 0 ;
  _GlideRoot.environment.aaSample = 0 ;

  switch(GLIDE_GETENV("SSTH3_SLI_AA_CONFIGURATION", 2))
  { 
    case 1:
      _GlideRoot.environment.aaSample = 2 ;
    case 0:
      _GlideRoot.environment.forceSingleChip = 1 ;
      break ;
    case 3:
    case 6:
      _GlideRoot.environment.aaSample = 2 ;
      break ;
    case 4:
    case 7:
      _GlideRoot.environment.aaSample = 4 ;
      break ;
//8xaa
	case 8:
	  _GlideRoot.environment.aaSample = 8 ;
      break ;
    default:
      break ;
  }

  /*
   * AJB- 7/27/99 support forcing anti-aliasing
   */
  /* Note- If the old school Glide env. vars for AA sample & Num chips
   * are active, they should ALWAYS override the control panel variable
   */
  if (GETENV("FX_GLIDE_AA_SAMPLE"))
    _GlideRoot.environment.aaSample = atol(GETENV("FX_GLIDE_AA_SAMPLE")) ;

  if (GLIDE_GETENV("FX_GLIDE_NUM_CHIPS", 0) > 1)
    _GlideRoot.environment.forceSingleChip = 0 ;

  _GlideRoot.environment.aaXOffset[1][0]   = GLIDE_GETENV("FX_GLIDE_AA2_OFFSET_X0",PRIBUFVTXOFFX_2SMPL_DEF_VAL);
  _GlideRoot.environment.aaXOffset[1][1]   = GLIDE_GETENV("FX_GLIDE_AA2_OFFSET_X1",SECBUFVTXOFFX_2SMPL_DEF_VAL);
  _GlideRoot.environment.aaXOffset[1][2]   = GLIDE_GETENV("FX_GLIDE_AA2_OFFSET_X0",PRIBUFVTXOFFX_2SMPL_DEF_VAL);
  _GlideRoot.environment.aaXOffset[1][3]   = GLIDE_GETENV("FX_GLIDE_AA2_OFFSET_X1",SECBUFVTXOFFX_2SMPL_DEF_VAL);
  _GlideRoot.environment.aaXOffset[1][4]   = GLIDE_GETENV("FX_GLIDE_AA2_OFFSET_X0",PRIBUFVTXOFFX_2SMPL_DEF_VAL);
  _GlideRoot.environment.aaXOffset[1][5]   = GLIDE_GETENV("FX_GLIDE_AA2_OFFSET_X1",SECBUFVTXOFFX_2SMPL_DEF_VAL);
  _GlideRoot.environment.aaXOffset[1][6]   = GLIDE_GETENV("FX_GLIDE_AA2_OFFSET_X0",PRIBUFVTXOFFX_2SMPL_DEF_VAL);
  _GlideRoot.environment.aaXOffset[1][7]   = GLIDE_GETENV("FX_GLIDE_AA2_OFFSET_X1",SECBUFVTXOFFX_2SMPL_DEF_VAL);

  _GlideRoot.environment.aaYOffset[1][0]   = GLIDE_GETENV("FX_GLIDE_AA2_OFFSET_Y0",PRIBUFVTXOFFY_2SMPL_DEF_VAL);
  _GlideRoot.environment.aaYOffset[1][1]   = GLIDE_GETENV("FX_GLIDE_AA2_OFFSET_Y1",SECBUFVTXOFFY_2SMPL_DEF_VAL);
  _GlideRoot.environment.aaYOffset[1][2]   = GLIDE_GETENV("FX_GLIDE_AA2_OFFSET_Y0",PRIBUFVTXOFFY_2SMPL_DEF_VAL);
  _GlideRoot.environment.aaYOffset[1][3]   = GLIDE_GETENV("FX_GLIDE_AA2_OFFSET_Y1",SECBUFVTXOFFY_2SMPL_DEF_VAL);
  _GlideRoot.environment.aaYOffset[1][4]   = GLIDE_GETENV("FX_GLIDE_AA2_OFFSET_Y0",PRIBUFVTXOFFY_2SMPL_DEF_VAL);
  _GlideRoot.environment.aaYOffset[1][5]   = GLIDE_GETENV("FX_GLIDE_AA2_OFFSET_Y1",SECBUFVTXOFFY_2SMPL_DEF_VAL);
  _GlideRoot.environment.aaYOffset[1][6]   = GLIDE_GETENV("FX_GLIDE_AA2_OFFSET_Y0",PRIBUFVTXOFFY_2SMPL_DEF_VAL);
  _GlideRoot.environment.aaYOffset[1][7]   = GLIDE_GETENV("FX_GLIDE_AA2_OFFSET_Y1",SECBUFVTXOFFY_2SMPL_DEF_VAL);

  _GlideRoot.environment.aaXOffset[2][0]   = GLIDE_GETENV("FX_GLIDE_AA2_OFFSET_X0",PRIBUFVTXOFFX_2SMPL_DEF_VAL);
  _GlideRoot.environment.aaXOffset[2][1]   = GLIDE_GETENV("FX_GLIDE_AA2_OFFSET_X0",PRIBUFVTXOFFX_2SMPL_DEF_VAL);
  _GlideRoot.environment.aaXOffset[2][2]   = GLIDE_GETENV("FX_GLIDE_AA2_OFFSET_X1",SECBUFVTXOFFX_2SMPL_DEF_VAL);
  _GlideRoot.environment.aaXOffset[2][3]   = GLIDE_GETENV("FX_GLIDE_AA2_OFFSET_X1",SECBUFVTXOFFX_2SMPL_DEF_VAL);
  _GlideRoot.environment.aaXOffset[2][4]   = GLIDE_GETENV("FX_GLIDE_AA2_OFFSET_X0",PRIBUFVTXOFFX_2SMPL_DEF_VAL);
  _GlideRoot.environment.aaXOffset[2][5]   = GLIDE_GETENV("FX_GLIDE_AA2_OFFSET_X0",PRIBUFVTXOFFX_2SMPL_DEF_VAL);
  _GlideRoot.environment.aaXOffset[2][6]   = GLIDE_GETENV("FX_GLIDE_AA2_OFFSET_X1",SECBUFVTXOFFX_2SMPL_DEF_VAL);
  _GlideRoot.environment.aaXOffset[2][7]   = GLIDE_GETENV("FX_GLIDE_AA2_OFFSET_X1",SECBUFVTXOFFX_2SMPL_DEF_VAL);

  _GlideRoot.environment.aaYOffset[2][0]   = GLIDE_GETENV("FX_GLIDE_AA2_OFFSET_Y0",PRIBUFVTXOFFY_2SMPL_DEF_VAL);
  _GlideRoot.environment.aaYOffset[2][1]   = GLIDE_GETENV("FX_GLIDE_AA2_OFFSET_Y0",PRIBUFVTXOFFY_2SMPL_DEF_VAL);
  _GlideRoot.environment.aaYOffset[2][2]   = GLIDE_GETENV("FX_GLIDE_AA2_OFFSET_Y1",SECBUFVTXOFFY_2SMPL_DEF_VAL);
  _GlideRoot.environment.aaYOffset[2][3]   = GLIDE_GETENV("FX_GLIDE_AA2_OFFSET_Y1",SECBUFVTXOFFY_2SMPL_DEF_VAL);
  _GlideRoot.environment.aaYOffset[2][4]   = GLIDE_GETENV("FX_GLIDE_AA2_OFFSET_Y0",PRIBUFVTXOFFY_2SMPL_DEF_VAL);
  _GlideRoot.environment.aaYOffset[2][5]   = GLIDE_GETENV("FX_GLIDE_AA2_OFFSET_Y0",PRIBUFVTXOFFY_2SMPL_DEF_VAL);
  _GlideRoot.environment.aaYOffset[2][6]   = GLIDE_GETENV("FX_GLIDE_AA2_OFFSET_Y1",SECBUFVTXOFFY_2SMPL_DEF_VAL);
  _GlideRoot.environment.aaYOffset[2][7]   = GLIDE_GETENV("FX_GLIDE_AA2_OFFSET_Y1",SECBUFVTXOFFY_2SMPL_DEF_VAL);

  _GlideRoot.environment.aaXOffset[3][0]   = GLIDE_GETENV("FX_GLIDE_AA4_OFFSET_X0",PRIBUFVTXOFFX_4SMPL_CHP0_DEF_VAL);
  _GlideRoot.environment.aaXOffset[3][1]   = GLIDE_GETENV("FX_GLIDE_AA4_OFFSET_X1",SECBUFVTXOFFX_4SMPL_CHP0_DEF_VAL);
  _GlideRoot.environment.aaXOffset[3][2]   = GLIDE_GETENV("FX_GLIDE_AA4_OFFSET_X2",PRIBUFVTXOFFX_4SMPL_CHP1_DEF_VAL);
  _GlideRoot.environment.aaXOffset[3][3]   = GLIDE_GETENV("FX_GLIDE_AA4_OFFSET_X3",SECBUFVTXOFFX_4SMPL_CHP1_DEF_VAL);
  _GlideRoot.environment.aaXOffset[3][4]   = GLIDE_GETENV("FX_GLIDE_AA4_OFFSET_X0",PRIBUFVTXOFFX_4SMPL_CHP0_DEF_VAL);
  _GlideRoot.environment.aaXOffset[3][5]   = GLIDE_GETENV("FX_GLIDE_AA4_OFFSET_X1",SECBUFVTXOFFX_4SMPL_CHP0_DEF_VAL);
  _GlideRoot.environment.aaXOffset[3][6]   = GLIDE_GETENV("FX_GLIDE_AA4_OFFSET_X2",PRIBUFVTXOFFX_4SMPL_CHP1_DEF_VAL);
  _GlideRoot.environment.aaXOffset[3][7]   = GLIDE_GETENV("FX_GLIDE_AA4_OFFSET_X3",SECBUFVTXOFFX_4SMPL_CHP1_DEF_VAL);
  
  _GlideRoot.environment.aaYOffset[3][0]   = GLIDE_GETENV("FX_GLIDE_AA4_OFFSET_Y0",PRIBUFVTXOFFY_4SMPL_CHP0_DEF_VAL);
  _GlideRoot.environment.aaYOffset[3][1]   = GLIDE_GETENV("FX_GLIDE_AA4_OFFSET_Y1",SECBUFVTXOFFY_4SMPL_CHP0_DEF_VAL);
  _GlideRoot.environment.aaYOffset[3][2]   = GLIDE_GETENV("FX_GLIDE_AA4_OFFSET_Y2",PRIBUFVTXOFFY_4SMPL_CHP1_DEF_VAL);
  _GlideRoot.environment.aaYOffset[3][3]   = GLIDE_GETENV("FX_GLIDE_AA4_OFFSET_Y3",SECBUFVTXOFFY_4SMPL_CHP1_DEF_VAL);
  _GlideRoot.environment.aaYOffset[3][4]   = GLIDE_GETENV("FX_GLIDE_AA4_OFFSET_Y0",PRIBUFVTXOFFY_4SMPL_CHP0_DEF_VAL);
  _GlideRoot.environment.aaYOffset[3][5]   = GLIDE_GETENV("FX_GLIDE_AA4_OFFSET_Y1",SECBUFVTXOFFY_4SMPL_CHP0_DEF_VAL);
  _GlideRoot.environment.aaYOffset[3][6]   = GLIDE_GETENV("FX_GLIDE_AA4_OFFSET_Y2",PRIBUFVTXOFFY_4SMPL_CHP1_DEF_VAL);
  _GlideRoot.environment.aaYOffset[3][7]   = GLIDE_GETENV("FX_GLIDE_AA4_OFFSET_Y3",SECBUFVTXOFFY_4SMPL_CHP1_DEF_VAL);


  /* these are the correct jitter vaules */
#if 1
  _GlideRoot.environment.aaXOffset[4][0]   = GLIDE_34GETENV("FX_GLIDE_AA2_OFFSET_X0",PRIBUFVTXOFFX_2SMPL_CORRECT_DEF);
  _GlideRoot.environment.aaXOffset[4][1]   = GLIDE_34GETENV("FX_GLIDE_AA2_OFFSET_X1",SECBUFVTXOFFX_2SMPL_CORRECT_DEF);
  _GlideRoot.environment.aaXOffset[4][2]   = GLIDE_34GETENV("FX_GLIDE_AA2_OFFSET_X0",PRIBUFVTXOFFX_2SMPL_CORRECT_DEF);
  _GlideRoot.environment.aaXOffset[4][3]   = GLIDE_34GETENV("FX_GLIDE_AA2_OFFSET_X1",SECBUFVTXOFFX_2SMPL_CORRECT_DEF);
  _GlideRoot.environment.aaXOffset[4][4]   = GLIDE_34GETENV("FX_GLIDE_AA2_OFFSET_X0",PRIBUFVTXOFFX_2SMPL_CORRECT_DEF);
  _GlideRoot.environment.aaXOffset[4][5]   = GLIDE_34GETENV("FX_GLIDE_AA2_OFFSET_X1",SECBUFVTXOFFX_2SMPL_CORRECT_DEF);
  _GlideRoot.environment.aaXOffset[4][6]   = GLIDE_34GETENV("FX_GLIDE_AA2_OFFSET_X0",PRIBUFVTXOFFX_2SMPL_CORRECT_DEF);
  _GlideRoot.environment.aaXOffset[4][7]   = GLIDE_34GETENV("FX_GLIDE_AA2_OFFSET_X1",SECBUFVTXOFFX_2SMPL_CORRECT_DEF);

  _GlideRoot.environment.aaYOffset[4][0]   = GLIDE_34GETENV("FX_GLIDE_AA2_OFFSET_Y0",PRIBUFVTXOFFY_2SMPL_CORRECT_DEF);
  _GlideRoot.environment.aaYOffset[4][1]   = GLIDE_34GETENV("FX_GLIDE_AA2_OFFSET_Y1",SECBUFVTXOFFY_2SMPL_CORRECT_DEF);
  _GlideRoot.environment.aaYOffset[4][2]   = GLIDE_34GETENV("FX_GLIDE_AA2_OFFSET_Y0",PRIBUFVTXOFFY_2SMPL_CORRECT_DEF);
  _GlideRoot.environment.aaYOffset[4][3]   = GLIDE_34GETENV("FX_GLIDE_AA2_OFFSET_Y1",SECBUFVTXOFFY_2SMPL_CORRECT_DEF);
  _GlideRoot.environment.aaYOffset[4][4]   = GLIDE_34GETENV("FX_GLIDE_AA2_OFFSET_Y0",PRIBUFVTXOFFY_2SMPL_CORRECT_DEF);
  _GlideRoot.environment.aaYOffset[4][5]   = GLIDE_34GETENV("FX_GLIDE_AA2_OFFSET_Y1",SECBUFVTXOFFY_2SMPL_CORRECT_DEF);
  _GlideRoot.environment.aaYOffset[4][6]   = GLIDE_34GETENV("FX_GLIDE_AA2_OFFSET_Y0",PRIBUFVTXOFFY_2SMPL_CORRECT_DEF);
  _GlideRoot.environment.aaYOffset[4][7]   = GLIDE_34GETENV("FX_GLIDE_AA2_OFFSET_Y1",SECBUFVTXOFFY_2SMPL_CORRECT_DEF);

  _GlideRoot.environment.aaXOffset[5][0]   = GLIDE_34GETENV("FX_GLIDE_AA2_OFFSET_X0",PRIBUFVTXOFFX_2SMPL_CORRECT_DEF);
  _GlideRoot.environment.aaXOffset[5][1]   = GLIDE_34GETENV("FX_GLIDE_AA2_OFFSET_X0",PRIBUFVTXOFFX_2SMPL_CORRECT_DEF);
  _GlideRoot.environment.aaXOffset[5][2]   = GLIDE_34GETENV("FX_GLIDE_AA2_OFFSET_X1",SECBUFVTXOFFX_2SMPL_CORRECT_DEF);
  _GlideRoot.environment.aaXOffset[5][3]   = GLIDE_34GETENV("FX_GLIDE_AA2_OFFSET_X1",SECBUFVTXOFFX_2SMPL_CORRECT_DEF);
  _GlideRoot.environment.aaXOffset[5][4]   = GLIDE_34GETENV("FX_GLIDE_AA2_OFFSET_X0",PRIBUFVTXOFFX_2SMPL_CORRECT_DEF);
  _GlideRoot.environment.aaXOffset[5][5]   = GLIDE_34GETENV("FX_GLIDE_AA2_OFFSET_X0",PRIBUFVTXOFFX_2SMPL_CORRECT_DEF);
  _GlideRoot.environment.aaXOffset[5][6]   = GLIDE_34GETENV("FX_GLIDE_AA2_OFFSET_X1",SECBUFVTXOFFX_2SMPL_CORRECT_DEF);
  _GlideRoot.environment.aaXOffset[5][7]   = GLIDE_34GETENV("FX_GLIDE_AA2_OFFSET_X1",SECBUFVTXOFFX_2SMPL_CORRECT_DEF);

  _GlideRoot.environment.aaYOffset[5][0]   = GLIDE_34GETENV("FX_GLIDE_AA2_OFFSET_Y0",PRIBUFVTXOFFY_2SMPL_CORRECT_DEF);
  _GlideRoot.environment.aaYOffset[5][1]   = GLIDE_34GETENV("FX_GLIDE_AA2_OFFSET_Y0",PRIBUFVTXOFFY_2SMPL_CORRECT_DEF);
  _GlideRoot.environment.aaYOffset[5][2]   = GLIDE_34GETENV("FX_GLIDE_AA2_OFFSET_Y1",SECBUFVTXOFFY_2SMPL_CORRECT_DEF);
  _GlideRoot.environment.aaYOffset[5][3]   = GLIDE_34GETENV("FX_GLIDE_AA2_OFFSET_Y1",SECBUFVTXOFFY_2SMPL_CORRECT_DEF);  
  _GlideRoot.environment.aaYOffset[5][4]   = GLIDE_34GETENV("FX_GLIDE_AA2_OFFSET_Y0",PRIBUFVTXOFFY_2SMPL_CORRECT_DEF);
  _GlideRoot.environment.aaYOffset[5][5]   = GLIDE_34GETENV("FX_GLIDE_AA2_OFFSET_Y0",PRIBUFVTXOFFY_2SMPL_CORRECT_DEF);
  _GlideRoot.environment.aaYOffset[5][6]   = GLIDE_34GETENV("FX_GLIDE_AA2_OFFSET_Y1",SECBUFVTXOFFY_2SMPL_CORRECT_DEF);
  _GlideRoot.environment.aaYOffset[5][7]   = GLIDE_34GETENV("FX_GLIDE_AA2_OFFSET_Y1",SECBUFVTXOFFY_2SMPL_CORRECT_DEF);  
                                                                                     
  _GlideRoot.environment.aaXOffset[6][0]   = GLIDE_34GETENV("FX_GLIDE_AA4_OFFSET_X0",PRIBUFVTXOFFX_4SMPL_CHP0_CORRECT_DEF);
  _GlideRoot.environment.aaXOffset[6][1]   = GLIDE_34GETENV("FX_GLIDE_AA4_OFFSET_X1",SECBUFVTXOFFX_4SMPL_CHP0_CORRECT_DEF);
  _GlideRoot.environment.aaXOffset[6][2]   = GLIDE_34GETENV("FX_GLIDE_AA4_OFFSET_X2",PRIBUFVTXOFFX_4SMPL_CHP1_CORRECT_DEF);
  _GlideRoot.environment.aaXOffset[6][3]   = GLIDE_34GETENV("FX_GLIDE_AA4_OFFSET_X3",SECBUFVTXOFFX_4SMPL_CHP1_CORRECT_DEF);
  _GlideRoot.environment.aaXOffset[6][4]   = GLIDE_34GETENV("FX_GLIDE_AA4_OFFSET_X0",PRIBUFVTXOFFX_4SMPL_CHP0_CORRECT_DEF);
  _GlideRoot.environment.aaXOffset[6][5]   = GLIDE_34GETENV("FX_GLIDE_AA4_OFFSET_X1",SECBUFVTXOFFX_4SMPL_CHP0_CORRECT_DEF);
  _GlideRoot.environment.aaXOffset[6][6]   = GLIDE_34GETENV("FX_GLIDE_AA4_OFFSET_X2",PRIBUFVTXOFFX_4SMPL_CHP1_CORRECT_DEF);
  _GlideRoot.environment.aaXOffset[6][7]   = GLIDE_34GETENV("FX_GLIDE_AA4_OFFSET_X3",SECBUFVTXOFFX_4SMPL_CHP1_CORRECT_DEF);

  _GlideRoot.environment.aaYOffset[6][0]   = GLIDE_34GETENV("FX_GLIDE_AA4_OFFSET_Y0",PRIBUFVTXOFFY_4SMPL_CHP0_CORRECT_DEF);
  _GlideRoot.environment.aaYOffset[6][1]   = GLIDE_34GETENV("FX_GLIDE_AA4_OFFSET_Y1",SECBUFVTXOFFY_4SMPL_CHP0_CORRECT_DEF);
  _GlideRoot.environment.aaYOffset[6][2]   = GLIDE_34GETENV("FX_GLIDE_AA4_OFFSET_Y2",PRIBUFVTXOFFY_4SMPL_CHP1_CORRECT_DEF);
  _GlideRoot.environment.aaYOffset[6][3]   = GLIDE_34GETENV("FX_GLIDE_AA4_OFFSET_Y3",SECBUFVTXOFFY_4SMPL_CHP1_CORRECT_DEF);
  _GlideRoot.environment.aaYOffset[6][4]   = GLIDE_34GETENV("FX_GLIDE_AA4_OFFSET_Y0",PRIBUFVTXOFFY_4SMPL_CHP0_CORRECT_DEF);
  _GlideRoot.environment.aaYOffset[6][5]   = GLIDE_34GETENV("FX_GLIDE_AA4_OFFSET_Y1",SECBUFVTXOFFY_4SMPL_CHP0_CORRECT_DEF);
  _GlideRoot.environment.aaYOffset[6][6]   = GLIDE_34GETENV("FX_GLIDE_AA4_OFFSET_Y2",PRIBUFVTXOFFY_4SMPL_CHP1_CORRECT_DEF);
  _GlideRoot.environment.aaYOffset[6][7]   = GLIDE_34GETENV("FX_GLIDE_AA4_OFFSET_Y3",SECBUFVTXOFFY_4SMPL_CHP1_CORRECT_DEF);

#endif
/* jcochrane 4 chip offsets */

/* 4chip 2xaa */
  _GlideRoot.environment.aaXOffset[7][0]   = 0x04;
  _GlideRoot.environment.aaXOffset[7][1]   = 0x00;
  _GlideRoot.environment.aaXOffset[7][2]   = 0x0c;  
  _GlideRoot.environment.aaXOffset[7][3]   = 0x00;
  _GlideRoot.environment.aaXOffset[7][4]   = 0x04;
  _GlideRoot.environment.aaXOffset[7][5]   = 0x00;
  _GlideRoot.environment.aaXOffset[7][6]   = 0x0c;
  _GlideRoot.environment.aaXOffset[7][7]   = 0x00;

  _GlideRoot.environment.aaYOffset[7][0]   = 0x04;
  _GlideRoot.environment.aaYOffset[7][1]   = 0x00;
  _GlideRoot.environment.aaYOffset[7][2]   = 0x0c;  
  _GlideRoot.environment.aaYOffset[7][3]   = 0x00;
  _GlideRoot.environment.aaYOffset[7][4]   = 0x04;
  _GlideRoot.environment.aaYOffset[7][5]   = 0x00;
  _GlideRoot.environment.aaYOffset[7][6]   = 0x0c;  
  _GlideRoot.environment.aaYOffset[7][7]   = 0x00;

/* 4chip 4xaa */
  _GlideRoot.environment.aaXOffset[8][0]   = 0x06; 
  _GlideRoot.environment.aaXOffset[8][1]   = 0x00; 
  _GlideRoot.environment.aaXOffset[8][2]   = 0x0e; 
  _GlideRoot.environment.aaXOffset[8][3]   = 0x00; 
  _GlideRoot.environment.aaXOffset[8][4]   = 0x02; 
  _GlideRoot.environment.aaXOffset[8][5]   = 0x00; 
  _GlideRoot.environment.aaXOffset[8][6]   = 0x0a; 
  _GlideRoot.environment.aaXOffset[8][7]   = 0x00; 

  _GlideRoot.environment.aaYOffset[8][0]   = 0x02; 
  _GlideRoot.environment.aaYOffset[8][1]   = 0x00;    
  _GlideRoot.environment.aaYOffset[8][2]   = 0x06; 
  _GlideRoot.environment.aaYOffset[8][3]   = 0x00;    
  _GlideRoot.environment.aaYOffset[8][4]   = 0x0a; 
  _GlideRoot.environment.aaYOffset[8][5]   = 0x00;    
  _GlideRoot.environment.aaYOffset[8][6]   = 0x0e; 
  _GlideRoot.environment.aaYOffset[8][7]   = 0x00;    

/* 4chip 8xaa */
  _GlideRoot.environment.aaXOffset[9][0]   = 0x06; 
  _GlideRoot.environment.aaXOffset[9][1]   = 0x04; 
  _GlideRoot.environment.aaXOffset[9][2]   = 0x0e; 
  _GlideRoot.environment.aaXOffset[9][3]   = 0x0a; 
  _GlideRoot.environment.aaXOffset[9][4]   = 0x02; 
  _GlideRoot.environment.aaXOffset[9][5]   = 0x0c; 
  _GlideRoot.environment.aaXOffset[9][6]   = 0x0a; 
  _GlideRoot.environment.aaXOffset[9][7]   = 0x06; 

  _GlideRoot.environment.aaYOffset[9][0]   = 0x02; 
  _GlideRoot.environment.aaYOffset[9][1]   = 0x06; 
  _GlideRoot.environment.aaYOffset[9][2]   = 0x06; 
  _GlideRoot.environment.aaYOffset[9][3]   = 0x04; 
  _GlideRoot.environment.aaYOffset[9][4]   = 0x0a; 
  _GlideRoot.environment.aaYOffset[9][5]   = 0x0a; 
  _GlideRoot.environment.aaYOffset[9][6]   = 0x0e; 
  _GlideRoot.environment.aaYOffset[9][7]   = 0x0c; 


  /*
  **  CHD - This let's Joe bag-o-croissant force rendering-column width.
  */
  _GlideRoot.environment.columnWidth = GLIDE_GETENV("FX_GLIDE_COLUMN_WIDTH", 
                                                    32L) ;

  /*
   * AJB- This lets those lucky people with non-flaky SLI slave chips
   *      enable the WAX functions that cause us unlucky folks to hang.
   */
  _GlideRoot.environment.waxon = GLIDE_GETENV("FX_GLIDE_WAX_ON", 1L) ;

  /*
   * KCD- Let user toggle AA on and off on the fly to impress their friends.
   */
  _GlideRoot.environment.aaToggleKey = GLIDE_GETENV("FX_GLIDE_AA_TOGGLE_KEY", 0L) ;

  /* KCD- Let the user save off a full-precision 32-bit screenshot */
  _GlideRoot.environment.aaScreenshotKey = GLIDE_GETENV("FX_GLIDE_SCREENSHOT_KEY", 0L) ;

  /*
   * AJB- Further impress users friends by clipping away the AA artifacts
   *      (visible in Unreal Tournament).
   */
  _GlideRoot.environment.aaClip = GLIDE_GETENV("FX_GLIDE_AA_CLIP", 1L) ;
  
  /* Which way to do 2-sample AA? */
  _GlideRoot.environment.forceOldAA = GLIDE_GETENV("FX_GLIDE_FORCE_OLD_AA", 0L);

  /*
   * AJB- 1= Always analog sli, 0= Glide decides, -1= always digital
   */
  _GlideRoot.environment.analogSli = GLIDE_GETENV("FX_GLIDE_ANALOG_SLI", 0L) ;

  _GlideRoot.environment.lodBias = GLIDE_GETENV("FX_GLIDE_LOD_BIAS", 0L) ;
  
  /****************************************************************** 
   * 5/4/99 gregk
   * Adding user support for turning on/off dither substraction
   * According to Alpha Blending Quality Tab of control panel choices   
   * Optimal/Sharper -> disable dither subtraction    
   * Smoother        -> enable  dither subtraction                     
   ******************************************************************/ 
   
    
   ditherMode = GLIDE_GETENV("SSTH3_ALPHADITHERMODE", 1L);
   switch(ditherMode)   
      {
      default:
      case OPTIMAL: /* Or Automatic? */
      case SHARPER:
        _GlideRoot.environment.disableDitherSub = FXTRUE;
        break;
      case SMOOTHER:
        _GlideRoot.environment.disableDitherSub = FXFALSE;
        break;
      }  
    GDBG_INFO(80,"  disableDitherSub: %d\n",
                _GlideRoot.environment.disableDitherSub);
   
    
  _GlideRoot.environment.texLodDither =
    GLIDE_GETENV("FX_GLIDE_LOD_DITHER", 0) ? SST_TLODDITHER : 0x00UL;
  GDBG_INFO(80,"\ttexLodDither: %d\n",_GlideRoot.environment.texLodDither);
  
  _GlideRoot.environment.nColorBuffer =
    GLIDE_GETENV("FX_GLIDE_ALLOC_COLOR", -1L);
  GDBG_INFO(80,"\tnColorBuffer: %d\n",_GlideRoot.environment.nColorBuffer);
  
  _GlideRoot.environment.tmuMemory =
    GLIDE_GETENV("FX_GLIDE_TMU_MEMSIZE", -1L);
  GDBG_INFO(80,"\ttmuMemory: %d\n",_GlideRoot.environment.tmuMemory);
  
  _GlideRoot.environment.nAuxBuffer =
    GLIDE_GETENV("FX_GLIDE_ALLOC_AUX", -1L);    
  GDBG_INFO(80,"\tnAuxBuffer: %d\n",_GlideRoot.environment.nAuxBuffer);
  
  _GlideRoot.environment.swFifoLWM =
    GLIDE_GETENV("FX_GLIDE_LWM", -1L);
  GDBG_INFO(80,"\tswFifoLWM: %d\n",_GlideRoot.environment.swFifoLWM);
  
  _GlideRoot.environment.swapInterval =
    GLIDE_GETENV("FX_GLIDE_SWAPINTERVAL", -1L);
  GDBG_INFO(80,"\tswapInterval: %d\n",_GlideRoot.environment.swapInterval);
  
  _GlideRoot.environment.snapshot = GLIDE_GETENV("FX_SNAPSHOT", -1L);
  GDBG_INFO(80,"\tsnapshot: %d\n",_GlideRoot.environment.snapshot);

  _GlideRoot.environment.guardbandclipping = GLIDE_GETENV("FX_GLIDE_GBC", 1L);
  GDBG_INFO(80," guardbandclipping: %d\n",_GlideRoot.environment.guardbandclipping);
  _GlideRoot.environment.do2ppc            = GLIDE_GETENV("FX_GLIDE_2PPC", -1L);
  GDBG_INFO(80," do2ppc           : %d\n",_GlideRoot.environment.do2ppc);
  _GlideRoot.environment.band2ppc          = GLIDE_GETENV("FX_GLIDE_2PPC_BAND", 1L);
  GDBG_INFO(80," band2ppc         : %d\n",_GlideRoot.environment.band2ppc);
  _GlideRoot.environment.sliBandHeight     = GLIDE_GETENV("FX_GLIDE_SLI_BAND_HEIGHT", 0L);
  GDBG_INFO(80," sliBandHeight    : %d\n",_GlideRoot.environment.sliBandHeight);
  _GlideRoot.environment.swapPendingCount  = GLIDE_GETENV("FX_GLIDE_SWAPPENDINGCOUNT", 1L);
  if (_GlideRoot.environment.swapPendingCount > 3)
    _GlideRoot.environment.swapPendingCount = 3;
  if (_GlideRoot.environment.swapPendingCount < 0)
    _GlideRoot.environment.swapPendingCount = 0;
  GDBG_INFO(80," swapPendingCount : %d\n",_GlideRoot.environment.swapPendingCount);


  _GlideRoot.environment.gammaR = GLIDE_FGETENV("SSTH3_RGAMMA", -1.f);
  _GlideRoot.environment.gammaG = GLIDE_FGETENV("SSTH3_GGAMMA", -1.f);
  _GlideRoot.environment.gammaB = GLIDE_FGETENV("SSTH3_BGAMMA", -1.f);

  _GlideRoot.environment.enUma = 0;

  /* Setup the basic proc tables based on the cpu type. */
  {
    _GlideRoot.CPUType = GLIDE_GETENV("FX_CPU", _cpu_detect_asm() );
    GDBG_INFO(80,"\tcpu: %d\n",_GlideRoot.CPUType);
      
    /* Default case */
#if GLIDE_DISPATCH_SETUP
    _GlideRoot.curTriProcs = _triSetupProcs + 0;
#endif /* GLIDE_DISPATCH_SETUP */

    _GlideRoot.curTexProcs = _texDownloadProcs + 0;
      
    /* Check for vendor specific optimization cases */
    switch((_GlideRoot.CPUType & 0xFFFF0000UL) >> 16UL) {
    case kCPUVendorIntel:
      break;
        
    case kCPUVendorAMD:
    case kCPUVendorCyrix:
    case kCPUVendorIDT:
      if ((_GlideRoot.CPUType & 0x02UL) == 0x02UL) { /* MMX & 3DNow!(tm) feature bits set */
#if GLIDE_DISPATCH_SETUP
        _GlideRoot.curTriProcs = _triSetupProcs + 1;
#endif /* GLIDE_DISPATCH_SETUP */

        _GlideRoot.curTexProcs = _texDownloadProcs + 1;
      }
      break;
        
    case kCPUVendorUnknown:
    default:
      break;
    }
  }
  
  _GlideRoot.environment.emulRush = (GETENV("FX_GLIDE_EMUL_RUSH") != NULL);
  GDBG_INFO(80,"\temulate Rush: %d\n", _GlideRoot.environment.emulRush);
  
#if __POWERPC__ && PCI_BUMP_N_GRIND
  _GlideRoot.environment.autoBump = FXFALSE;
#else  
  _GlideRoot.environment.autoBump = (GETENV("FX_GLIDE_BUMP") == NULL);
#endif  
  GDBG_INFO(80, "\tautoBump:          %s\n",
            _GlideRoot.environment.autoBump ? "FXTRUE" : "FXFALSE");
  
  if (GETENV("FX_GLIDE_BUMPSIZE"))
    sscanf(GETENV("FX_GLIDE_BUMPSIZE"), "%x",
           &_GlideRoot.environment.bumpSize);
  else
#if __POWERPC__
    _GlideRoot.environment.bumpSize = 0x1000;
#else
    _GlideRoot.environment.bumpSize = 0x10000;
#endif    
  GDBG_INFO(80, "\tbumpSize:          0x%x\n", _GlideRoot.environment.bumpSize);
  
  _GlideRoot.environment.bumpSize >>= 2; /* So we don't have to later */

  _GlideRoot.environment.grxClk = GLIDE_GETENV("FX_GLIDE_GRXCLK", -1);

  _GlideRoot.environment.fenceLimit = GLIDE_GETENV("FX_GLIDE_FENCE_LIMIT", 0x10000);

  if (_GlideRoot.environment.fenceLimit > 0x10000)
    _GlideRoot.environment.fenceLimit = 0x10000;

  GDBG_INFO(80, "\tGamma R:          %1.1f\n",
            _GlideRoot.environment.gammaR);
  GDBG_INFO(80, "\tGamma G:          %1.1f\n",
            _GlideRoot.environment.gammaG);
  GDBG_INFO(80, "\tGamma B:          %1.1f\n",
            _GlideRoot.environment.gammaB);


  /* constant pool */
  _GlideRoot.pool.f0   =   0.0F;
  _GlideRoot.pool.fHalf=   0.5F;
  _GlideRoot.pool.f1   =   1.0F;
  _GlideRoot.pool.f255 = 255.0F;
  
#if GLIDE_PACKED_RGB
  _GlideRoot.pool.fBiasHi = (float)(0x01 << 15);
  _GlideRoot.pool.fBiasLo = (float)(0x01 << 23);
#endif /* GLIDE_PACKED_RGB */
  
  _GlideRoot.current_sst = 0;                    /* make sure there's a valid GC */
  _GlideRoot.curGC       = &_GlideRoot.GCs[0];   /* just for 'booting' the library */
  
  grErrorSetCallback(_grErrorDefaultCallback);
  
  if ( !_grSstDetectResources() ) {
#ifdef GLIDE_INIT_HWC
    GrErrorCallback( hwcGetErrorString(), FXTRUE );
#endif
  }
  
  for (i = 0; i < _GlideRoot.hwConfig.num_sst; i++) {
    _GlideRoot.GCs[i].mm_table.free_mmid = 0;
    displayBoardInfo(i, &_GlideRoot.hwConfig);
  }
  
  _grMipMapInit();
  _GlideRoot.initialized = FXTRUE;               /* save this for the end */
} /* _GlideInitEnvironment */
