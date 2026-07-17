/*
** Copyright (c) 1997, 3Dfx Interactive, Inc.
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
** File name:   d3scene.c
**
** Description: Implementation of scene capture API 
**
** $Revision: 2$
** $Date: 10/11/00 8:48:31 PM$
**
** $Log: 
**  2    3dfx      1.0.2.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
** 
** 2     6/17/99 2:45p Tlittle
** removed un-needed $if check
** 
** 1     6/02/99 6:43a Michael
** Branch from H3
** 
** 27    5/24/99 5:05p Bseitsin
** Removal of Antialiasing code.
** 
** 26    4/21/99 3:24p Edwin
** Added MapOffscreenToFront() and MapIterator(), they were in ddblt32.c.
** 
** 24    4/09/99 12:35p Stb_bseitsin
** Added Napalm registers. Added ifdef H5.
** 
** 23    2/19/99 8:58a Russ
** fix for VC6 compiler warning, line 177 change == to =
** 
** 22    1/26/99 5:27p Peterm
** Added unified header information
** 
** 21    12/09/98 7:29a Russ
** NT5 D3D changes for Banshee
**
** 20    12/02/98 1:44p Martin
** Fix for earlier checkin
**
** 19    12/02/98 8:24a Martin
** Turn off anti-aliasing by default and rename some variables for
** super-sampling AA.
**
** 18    11/22/98 9:03p Andrew
** Changes to support multi-monitor
**
** 17    10/16/98 4:33p Artg
** change ifdef h3 to if defned(h3) || defined (H4)
**
** 16    10/01/98 10:38p Adrians
** Re-coded Fog which fixes a bug when Wbuffering with Vertex Fog.
**
** 13    8/28/98 2:33p Martin
** Conditionalize banshee compilation.
**
** 12    8/28/98 12:37p Martin
** Do super-sampling AA when the registry key is set to 2.  Do edge AA if
** registry key == 1 OR we are not running at the 2 magical resolutions of
** 640x480 or 800x600.
**
** No more conditional compilation of AA.
**
** 11    8/23/98 12:49a Adrians
** Added support for AA SuperSampling.
**
** 10    7/24/98 1:37p Hohn
**
** 9     7/24/98 11:25a Adrians
** Fix broken build. Needed an #ifdef H3.
**
** 8     7/22/98 11:53a Adrians
** Added Lock Idle code.
**
** 7     7/02/98 12:52p Adrians
** Fix for AA hang.
**
** 6     5/26/98 7:24p Adrians
** Added antialiasing.
**
** 5     5/06/98 6:09p Adrians
** Changes for DX6 into DX5 driver.
**
** 2     5/01/98 4:11p Adrians
** Compile options for dx5 and dx6.
** Removed redundent returns.
**
**
**
** 1     4/29/98 6:31p Adrians
** Created
**
** 4     11/23/97 3:56p Suninn
** replay _d3Global with _D3 & D3G macros
**
** 3     9/03/97 5:52p Adrians
** Updated File Header Comment.
** Now includes LOG of SourceSafe changes.
*/

#include "precomp.h"

#ifndef WINNT
#include "d3dhal.h"
#include "d3global.h"
#include "ddglobal.h"
#include "fxglobal.h"
#include "d3contxt.h"
#endif

#include "dxins.h"

/*-------------------------------------------------------------------
Function Name:  ddiSceneCapture

Description:    Implementation of scene capture API

Return:         void
-------------------------------------------------------------------*/
DWORD __stdcall ddiSceneCapture(LPD3DHAL_SCENECAPTUREDATA psc)
{
extern void insPrint( void );

  SETUP_PPDEV(psc->dwhContext)
  RC *pRc = CONTEXT_PTR( psc->dwhContext );

  D3D_ENTRY( "ddiSceneCapture" );
  INS_ENTRY( INSC_D3DSCENECAPTURE );

  D3DPRINT( 255, "ddiSceneCapture %s",(psc->dwFlag ? "End" : "Begin") );

  if (psc->dwFlag)
  {
    // Processing an EndScene ...
    _D3(flags) &= ~IN_RENDER_SCENE;

    psc->ddrval = DD_OK;
    INS_EXITEVENT( INS_EVENT_ENDSCENE );
    D3D_EXIT( DDHAL_DRIVER_HANDLED );
  }

  // Processing a BeginScene ...
  _D3(flags) |= IN_RENDER_SCENE;

  _D3( sceneCount )++;

  psc->ddrval = DD_OK;
  INS_EXIT( );
  D3D_EXIT( DDHAL_DRIVER_HANDLED );
}

