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
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
** rights reserved under the Copyright Laws of the United States.
**
**
** $Header: gerror.c, 3, 10/11/00 8:21:33 PM, Brent$
** $Log: 
**  3    3dfx      1.1.1.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  2    3dfx      1.1         11/23/99 Kenneth Dyke    Improved MacOS error
**       handling.
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
** 
** 37    4/06/99 10:51a Dow
** Protect CDS
** 
** 36    4/05/99 10:35p Atai
** added code to check context. exit if alt-tab happens
** 
** 35    7/06/98 7:03p Jdt
** Error checking function definition simplified
** 
** 34    6/16/98 9:36a Dow
** made error messages more like I want them
** 
** 33    3/28/98 11:24a Dow
** itwoç
** 
** 29    2/17/98 12:50p Dow
** More informative debug messages.
** 
** 28    2/08/98 3:08p Dow
** FIFO Works
 * 
 * 26    12/18/97 2:13p Peter
 * cleaned up the error code
 * 
 * 25    12/09/97 12:20p Peter
 * mac glide port
 * 
 * 24    12/05/97 4:38p Peter
 * sli vs assertions
 * 
 * 23    12/03/97 11:34a Peter
 * dos debugging
 * 
 * 22    11/17/97 4:55p Peter
 * watcom warnings/chipfield stuff
 * 
 * 21    11/15/97 8:55p Peter
 * Removed OutputDebugString
 * 
 * 20    11/15/97 7:43p Peter
 * more comdex silliness
 * 
 * 19    11/12/97 2:27p Peter
 * simulator happiness w/o fifo
 * 
 * 18    11/12/97 11:16a Peter
 * cleaned up assertions
 * 
 * 17    11/04/97 5:04p Peter
 * cataclysm part deux
 * 
 * 16    11/03/97 4:02p Peter
 * cataclysm fix
 * 
 * 15    11/03/97 3:43p Peter
 * h3/cvg cataclysm
 * 
 * 14    10/23/97 5:28p Peter
 * sli fifo thing
 * 
 * 13    9/24/97 1:29p Peter
 * more assertion spewage
 * 
 * 12    9/05/97 5:29p Peter
 * changes for direct hw
 * 
 * 11    5/30/97 5:44p Peter
 * Version that does basic triangles/registers w/ command fifo. Does not
 * currently download textures correctly.
 * 
 * 10    5/28/97 9:05a Peter
 * Merge w/ latest glide changes
 * 
 * 9     5/27/97 1:16p Peter
 * Basic cvg, w/o cmd fifo stuff. 
 * 
 * 8     5/21/97 6:05a Peter
 * 
 * 7     5/20/97 9:47a Pgj
 * Use OutputDebugString for non-fatal errors under windows
 * 
 * 6     5/19/97 7:35p Pgj
 * Print cogent error message if h/w not found
 * 
 * 5     3/09/97 10:31a Dow
 * Added GR_DIENTRY for di glide functions
 * 
 * 4     12/23/96 1:37p Dow
 * chagnes for multiplatform glide
**
*/
#include <stdio.h>
#ifdef __DOS__
#  include <malloc.h>
#endif

#ifdef __WIN32__
#  include <windows.h>
#endif

#include <3dfx.h>
#define FX_DLL_DEFINITION
#include <fxdll.h>
#include <glide.h>
#include "fxglide.h"

void (*GrErrorCallback)( const char *string, FxBool fatal );

void _doGrErrorCallback( const char *name, const char *msg, FxBool fatal )
{
  char buf[1024];

#if GDBG_INFO_ON
  /* Neuter any fifo checking from the failing
   * call. Otherwise entries into the shutdown
   * calls cause spurious crap.
   */
  if (fatal) {
    GR_DCL_GC;

    gc->checkCounter     =
    gc->expected_counter = 0;

    gc->checkCounter =
    gc->checkPtr     = 0UL;
  }
#endif /* GDBG_INFO_ON */

  gdbg_printf("%s: %s.\n", name, msg);
  sprintf(buf,"%s: %s.\n", name, msg);
  GrErrorCallback(buf, fatal);

  if (fatal) exit(1);
}

GR_DIENTRY(grErrorSetCallback, void,
           ( void (*function) ( const char *string, FxBool fatal ) ))
{
  GDBG_INFO(80,"grErrorSetCallback(0x%x)",function);
  GrErrorCallback = function;
}

#ifdef __WIN32__
void
_grErrorDefaultCallback( const char *s, FxBool fatal )
{
  if ( fatal ) {
    GDBG_ERROR("glide", s);
    grGlideShutdown();
    MessageBox(NULL, s, NULL, MB_OK);
  }
}
#else
void
_grErrorDefaultCallback( const char *s, FxBool fatal )
{
  if ( fatal ) {
    GDBG_ERROR("glide",s);
    grGlideShutdown();

#if (GLIDE_PLATFORM & GLIDE_OS_MACOS)
                {
                        //Str255 errBuf;
                        
                        //errBuf[0] = sprintf((char*)(errBuf + 1), "%s", s);
                        //DebugStr(errBuf);
                        ErrorMacCallback(s);
                        ExitToShell();
                }
#endif /* (GLIDE_PLATFORM * GLIDE_OS_MACOS) */
  }
}
#endif

#ifdef __DOS__
int _guHeapCheck( void )
{
  int i = _heapchk();

  if ( i != _HEAPOK )
    printf( "heapchk: %d\n", i );

  return ( i == 0 );
}
#endif

void
_grAssert(char *exp, char *fileName, int lineNo)
{
  static int depth;

  if (depth)
    return;

  depth++;

#if GLIDE_INIT_HAL

#else /* !GLIDE_INIT_HAL */
  /* initRestoreVideo(); */
#endif /* !GLIDE_INIT_HAL */

  gdbg_printf("ASSERTION FAILED:\n");
  gdbg_printf("\tExpression:   %s\n", exp);
  gdbg_printf("\tFile:         %s\n", fileName);
  gdbg_printf("\tLine:         %d\n", lineNo);

#if USE_PACKET_FIFO
  /* Spew about the state of the fifo since that's what most of the
   * assertions are about anyway.  
   */
  {
    GR_DCL_GC;
    GR_DCL_HW;
    
    gdbg_printf("Command Fifo:\n");
    gdbg_printf("\tSoftware:\n");
    gdbg_printf("\t\tfifoPtr:     0x%x\n", (FxU32)
      gc->cmdTransportInfo.fifoPtr - (FxU32) gc->rawLfb);
    gdbg_printf("\t\tfifoOffset:        0x%x\n",
      gc->cmdTransportInfo.fifoOffset); 
    gdbg_printf("\t\tfifoEnd:           0x%x\n",
      gc->cmdTransportInfo.fifoEnd - (FxU32) gc->rawLfb);
    gdbg_printf("\tfifoSize:            0x%x\n",
      gc->cmdTransportInfo.fifoSize); 
    gdbg_printf("\t\troomToReadPtr:     0x%x\n",
      gc->cmdTransportInfo.roomToReadPtr);
    gdbg_printf("\t\troomToEnd:         0x%x\n",
      gc->cmdTransportInfo.roomToEnd);
    gdbg_printf("\tHardware:\n");
    gdbg_printf("\t\treadPtrL:          0x%x\n", GR_CAGP_GET(readPtrL));
    gdbg_printf("\t\tdepth:             0x%x\n", GR_CAGP_GET(depth));
    gdbg_printf("\t\tholeCount:         0x%x\n", GR_CAGP_GET(holeCount));
    gdbg_printf("\t\tbaseAddrL:         0x%x\n", GR_CAGP_GET(baseAddrL));
    gdbg_printf("\t\taMin:              0x%x\n", GR_CAGP_GET(aMin));
    gdbg_printf("\t\taMax:              0x%x\n", GR_CAGP_GET(aMax));
    gdbg_printf("\t\tStatus:            0x%x\n", GR_GET(hw->status));

#if GLIDE_USE_DEBUG_FIFO && 0
    if (gc->cmdTransportInfo.fifoShadowBase != NULL) {
      const FxU32* fifoPtr = gc->cmdTransportInfo.fifoShadowBase;

      GDBG_PRINTF("Shadow Fifo: \n");
      while(fifoPtr != gc->cmdTransportInfo.fifoShadowPtr) GDBG_PRINTF("0x%X\n", *fifoPtr++);
      GDBG_PRINTF("\n");

      GDBG_PRINTF("Up to fifo wrap: \n");
      while(fifoPtr < gc->cmdTransportInfo.fifoShadowBase + (kDebugFifoSize >> 2)) 
        GDBG_PRINTF("0x%X\n", *fifoPtr++);

      free(gc->cmdTransportInfo.fifoShadowBase);
    }
#endif /* GLIDE_USE_DEBUG_FIFO */
  }
#endif /* (GLIDE_PLATFORM & GLIDE_HW_CVG) && USE_PACKET_FIFO */

  gdbg_printf("ABNORMAL TERMINATION\n");

  grGlideShutdown();

  depth--;

  exit(-1);

} /* _grAssert */

void
_grAltTabExit(void)
{
#define FN_NAME "_grAltTabExit"
  static int called = 0;
  GDBG_INFO(80, FN_NAME": Lost hwc context.\n");
  if (called)
    return;
  called++;
  grGlideShutdown();
#if defined(__MSC__)
  ChangeDisplaySettings(0, 0);
#endif
  exit(-1);
#undef FN_NAME  
}
