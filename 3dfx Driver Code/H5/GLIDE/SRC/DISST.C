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
** $Header: disst.c, 2, 10/11/00 8:21:08 PM, Brent$
** $Log: 
**  2    3dfx      1.0.1.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
** 
** 27    4/05/99 10:35p Atai
** added code to check context. exit if alt-tab happens
** 
** 26    11/30/98 6:39p Mikec
** Make sure grSstQueryBoards returns the right num_sst.
** 
** 25    5/22/98 2:37p Peter
** complete the lie that is glide2x on Banshee
** 
** 24    5/08/98 10:58a Dow
** Fixed accvio in debug version
** 
** 23    3/28/98 11:24a Dow
** itwoç
** 
** 20    2/08/98 3:08p Dow
** FIFO Works
 * 
 * 18    12/17/97 4:05p Atai
 * added grChromaRange(), grGammaCorrecionRGB(), grRest(), and grGet()
 * functions
 * 
 * 17    12/09/97 12:20p Peter
 * mac glide port
 * 
 * 16    11/19/97 2:49p Peter
 * env vars in registry for win32
 * 
 * 15    11/18/97 4:36p Peter
 * chipfield stuff cleanup and w/ direct writes
 * 
 * 14    10/31/97 9:15a Peter
 * only lie about v2 boards
 * 
 * 13    10/31/97 8:53a Peter
 * last lying change, really
 * 
 * 12    9/05/97 5:29p Peter
 * changes for direct hw
 * 
 * 11    6/20/97 9:56a Peter
 * better lines/pts, hopefully
 * 
 * 10    6/02/97 4:09p Peter
 * Compile w/ gcc for Dural
 * 
 * 9     5/27/97 1:16p Peter
 * Basic cvg, w/o cmd fifo stuff. 
 * 
 * 8     5/21/97 6:04a Peter
 * 
 * 7     5/02/97 2:07p Pgj
 * grSstScreenWidth/Height now FxU32
 * 
 * 6     3/17/97 6:25a Jdt
 * Added initDeviceSelect to grSstSelect()
 * 
 * 5     3/09/97 10:31a Dow
 * Added GR_DIENTRY for di glide functions
 * 
 * 4     3/04/97 9:08p Dow
 * Neutered multiplatform multiheaded monster
 * 
 * 3     1/18/97 11:39p Dow
 * Changed location of _curGCFuncs
 * 
 * 2     1/16/97 3:39p Dow
 * Added ref to _curGCFuncs during grSstSelect()
 * 
 * 1     12/23/96 1:39p Dow
 * Changes for multiplatform
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


/*---------------------------------------------------------------------------
** grSstQueryBoards
**
** NOTE:  it is OK to call this routine before grGlideInit
*/
#ifndef GLIDE3_ALPHA
GR_DIENTRY(grSstQueryBoards, FxBool, ( GrHwConfiguration *hwc ))
{
  GDBG_INIT();
  GDBG_INFO(80,"grSstQueryBoards(0x%x)\n",hwc);
  hwc->num_sst = (_grSstDetectResources()
                  ? _GlideRoot.hwConfig.num_sst
                  : 0);
  return FXTRUE;
} /* grSstQueryBoards */
#endif

/*---------------------------------------------------------------------------
** grSstQueryHardware
**
*/
GR_DIENTRY(grSstQueryHardware, FxBool, ( GrHwConfiguration *hwc ))
{
  FxBool retVal;

  GDBG_INFO(80, "grSstQueryHardware(0x%x)\n", hwc);

  /* init and copy the data back to the user's structure */
  retVal = _GlideRoot.hwConfig.num_sst > 0;
  *hwc = _GlideRoot.hwConfig;

  return retVal;

} /* grSstQueryHardware */

/*---------------------------------------------------------------------------
** grSstSelect
*/
GR_DIENTRY(grSstSelect, void, ( int which ))
{
  if ( which >= _GlideRoot.hwConfig.num_sst )
    GrErrorCallback( "grSstSelect:  non-existent SST", FXTRUE );

  _GlideRoot.current_sst = which;
  _GlideRoot.curGC = &_GlideRoot.GCs[which];
#ifdef GLIDE_MULTIPLATFORM
  _GlideRoot.curGCFuncs = _GlideRoot.curGC->gcFuncs;
#endif

  /* now begin a normal Glide routine's flow */
  {
    GR_BEGIN_NOFIFOCHECK_NORET("grSstSelect",80);
    GDBG_INFO_MORE(gc->myLevel,"(%d)\n",which);

    /* GMT: C-simulator recognizes this special address!!! (don't change it)*/
    _GlideRoot.packerFixAddress  = ( FxU32 ) gc->tex_ptr;
    _GlideRoot.packerFixAddress += ( ( ( FxU32 ) 3 ) << 21 );
    _GlideRoot.packerFixAddress += ( ( ( FxU32 ) 1 ) << 17 );

    /* Now that we have selected a board, we can build the offests and register
       lists for the  optimized triangle setup code */
    _grRebuildDataList();

    /* dpc - 22 may 1997 - FixMe!
     * We don't need to move around the driver init
     * functions since we're using the hal crap.
     * Is there anything else that we need to do?
     */
#if GLIDE_INIT_HAL

#else /* !GLIDE_INIT_HAL */
    /* dpc - 5 sep 1997 - FixMe!
     * initDeviceSelect( which );
     */
#endif /* !GLIDE_INIT_HAL */

    GR_END();
  }
} /* grSstSelect */

/*---------------------------------------------------------------------------
** grSstScreenWidth
*/
GR_DIENTRY(grSstScreenWidth, FxU32, (void))
{
  GR_DCL_GC;
  GR_ASSERT(gc != NULL);
  return gc->state.screen_width;
} /* grSstScreenWidth */

/*---------------------------------------------------------------------------
** grSstScreenHeight
*/
GR_DIENTRY(grSstScreenHeight, FxU32,  (void))
{
  GR_DCL_GC;
  GR_ASSERT(gc != NULL);
  return gc->state.screen_height;
}

/* this is an un-documented external for arcade developers */
extern FX_ENTRY void FX_CALL
grSstVidMode(FxU32 whichSst, FxVideoTimingInfo* vidTimings);

/*---------------------------------------------------------------------------
**  grSstVidMode - override args to grSstOpen()
*/
GR_DIENTRY(grSstVidMode, void, (FxU32 whichSst, FxVideoTimingInfo* vidTimings))
{
  GDBG_INFO(80,"grSstVidMode(%d,0x%x)\n",whichSst,vidTimings);
#ifdef GLIDE_DEBUG
  if (whichSst >= MAX_NUM_SST) {
    char errStr[1028];
    sprintf(errStr, "grSstVidMode:  %ld greater than MAX_NUM_SST (%d)\n",
            whichSst, MAX_NUM_SST);

    GrErrorCallback(errStr, FXTRUE);
  }
#endif

  _GlideRoot.GCs[whichSst].vidTimings = vidTimings;    
} /* grSstVidMode */

