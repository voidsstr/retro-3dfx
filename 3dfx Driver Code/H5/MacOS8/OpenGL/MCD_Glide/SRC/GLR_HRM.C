/*________________________________________________________________________________________
** 
** Copyright (c) 1999, 3Dfx Interactive, Inc.
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
**________________________________________________________________________________________
**
**  Description: 
**
** 
**
*/

#include "glr_hrm.h"
#include "glr_debug.h"
#include <CodeFragments.h>
#include <glide.h>

#if 0

long glrHRMNumOfTargets = 0;
hrmBoard_t * glrHRMTargetList[ kMaxNumberOfTargets ];
hrmDeviceConfig_t glrHRMTargetConfigList[ kMaxNumberOfTargets ];
hrmFifoInfo * glrHRMTargetFifoInfoList[ kMaxNumberOfTargets ];

hrmGetVersionInfoPtr glrHRMGetVersionInfo = 0;
hrmGetDeviceConfigPtr glrHRMGetDeviceConfig = 0;

hrmGetTargetFifoInfoPtr glrHRMGetTargetFifoInfo = 0;
hrmFifoWrapPtr glrHRMFifoWrap = 0;
hrmHwFifoPtrPtr glrHRMHwFifoPtr = 0;
hrmFifoUpdatePtr glrHRMFifoUpdate = 0;
hrmAllocWinContextPtr glrHRMAllocWinContext = 0;
hrmFreeWinContextPtr glrHRMFreeWinContext = 0;
hrmExecuteWinFifoPtr glrHRMExecuteWinFifo = 0;
hrmGetAGPInfoPtr glrHRMGetAGPInfo = 0;
hrmEnableFifo2DPtr glrHRMEnableFifo2D = 0;
hrmDisableFifo2DPtr glrHRMDisableFifo2D = 0;


static void glrHRMInitialize(void);


/*
________________________________________________________________________________________

      glrHRMInitialize
________________________________________________________________________________________


*/

void glrHRMInitialize()
{
  short
    result,
    i = 0;

  DEBUG_ENTRY( glrHRMInitialize );

  if ( glrHRMGetVersionInfo == 0 )
  {
    hrmVersionInfo_t theHRMVersion;
    
    glrHRMGetVersionInfo = hrmGetExtension( "hrmGetVersionInfo" );
    glrHRMGetDeviceConfig = hrmGetExtension( "hrmGetDeviceConfig" );
    glrHRMGetTargetFifoInfo = hrmGetExtension( "hrmGetTargetFifoInfo" );
    glrHRMFifoWrap = hrmGetExtension( "hrmFifoWrap" );
    glrHRMHwFifoPtr = hrmGetExtension( "hrmHwFifoPtr" );
    glrHRMFifoUpdate = hrmGetExtension( "hrmFifoUpdate" );
    glrHRMAllocWinContext = hrmGetExtension( "hrmAllcoWinContext" );
    glrHRMFreeWinContext = hrmGetExtension( "hrmFreeWinContext" );
    glrHRMExecuteWinFifo = hrmGetExtension( "hrmExecuteWinFifo" );
    glrHRMGetAGPInfo = hrmGetExtension( "hrmGetAGPInfo" );
    glrHRMDisableFifo2D = hrmGetExtension( "hrmDisableFifo2D" );
    glrHRMEnableFifo2D = hrmGetExtension( "hrmEnableFifo2D" );
  
    glrHRMNumOfTargets = 0;
    glrHRMGetVersionInfo( &theHRMVersion );
    
    if ( theHRMVersion.major >= 1 && theHRMVersion.minor >= 4 )
    {
      glrHRMNumOfTargets = hrmGetNumTargets();
    
      i = 0;
      while ( i < glrHRMNumOfTargets )
      {
        glrHRMTargetList[ i ] = hrmGetTargetAtIndex( i );
        result = glrHRMGetDeviceConfig( glrHRMTargetList[ i ], &glrHRMTargetConfigList[ i ] );
            
        i++;
      }
    }
    else
    {
      DEBUG_ERROR( glrHRMInitialize, "HRM version is outdated!\n");
    }
  }
}



/*
________________________________________________________________________________________

      glrValidateEnvironment
________________________________________________________________________________________


*/

long glrValidateEnvironment()
{
  FxI32 numBoards;
  DEBUG_ENTRY( glrValidateEnvironment );
  
  /* Make sure Glideis around. */
  if((long)grGet == (long)kUnresolvedCFragSymbolAddress) {
    return 0;
  }

  grGet(GR_NUM_BOARDS,sizeof(numBoards),&numBoards);
  return numBoards;
}
#endif
