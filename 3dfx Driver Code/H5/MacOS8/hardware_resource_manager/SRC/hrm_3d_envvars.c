/*
** Copyright (c) 2000, 3Dfx Interactive, Inc.
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
**
*/

#include "hrm_3d_envvars.h"
#include "hrm_prefs.h"
#include "hrm_priv.h"

#include <Processes.h>

#include "DCON.h"
#include <stdio.h>



HRM3DClientType GetCurrentClient();

void sb_init();
void sb_printf(	const char * inFormat, ...);
char * sb_lock();

static ProcessSerialNumber gCurApp = {};
static HRM3DClientType gCurClient;










/*
_______________________________________________________ hrm_Register3DClient ___
     
*/

void hrm_Register3DClient( HRM3DClientType inClient )
{
  ProcessSerialNumber theCurApp;
  
  dprintf( "hrm_Register3DClient(): inClient = %d\n", inClient );

  GetCurrentProcess(&theCurApp);
  
  if ( theCurApp.highLongOfPSN != gCurApp.highLongOfPSN
               || theCurApp.lowLongOfPSN != gCurApp.lowLongOfPSN  )
  {
    gCurApp = theCurApp;
    gCurClient = inClient;
    dprintf( "hrm_Register3DClient(): ... new client is now registered!\n" );
  }
}





/*
________________________________________________________ hrm_GetGlideEnvVars ___
     
*/

void hrm_GetGlideEnvVars( GDHandle inBoard, char ** inEnvData)
{
  HRMPrefsTable * thePrefs = hrm_GetPrefsTable();
  HRM3DClientType theClientType;
  short theIndex, theAASample, theAAKey;
  float theGamma;
  UInt32 theVSyncEnable;
  UInt32 the32bit;
  
  theIndex = inBoard ? hrm_FindPrefIdx( inBoard ) : 0;
  
  dprintf( "hrm_GetGlideEnvVars(): inBoard = 0x%08x, theIndex = %d\n", inBoard, theIndex );

  
  sb_init();
  sb_printf( "[Voodoo]\n" );
  
  theClientType = GetCurrentClient();
  
  // this is hard coded to Glide for now, since it makes the user interface cleaner
  // in the control panel.
  
  theClientType = kHRM_Glide_Client;
  
  switch( theClientType )
  {
    case kHRM_OpenGL_Client:
      theAASample = thePrefs->m3D[theIndex]->ogl.antialiasing;
      theAAKey = thePrefs->m3D[theIndex]->ogl.antialiasingKey;
      theGamma = thePrefs->m3D[theIndex]->ogl.gammaValue;
      theVSyncEnable = thePrefs->m3D[theIndex]->ogl.VSyncEnable;
      the32bit = thePrefs->m3D[theIndex]->ogl.force32bit;
      break;
    
    case kHRM_Rave_Client:
      theAASample = thePrefs->m3D[theIndex]->rave.antialiasing;
      theAAKey = thePrefs->m3D[theIndex]->rave.antialiasingKey;
      theGamma = thePrefs->m3D[theIndex]->rave.gammaValue;
      theVSyncEnable = thePrefs->m3D[theIndex]->rave.VSyncEnable;
      the32bit = thePrefs->m3D[theIndex]->rave.force32bit;
      break;
    
    default:
    case kHRM_Glide_Client:
      theAASample = thePrefs->m3D[theIndex]->glide.antialiasing;
      theAAKey = thePrefs->m3D[theIndex]->glide.antialiasingKey;
      theGamma = thePrefs->m3D[theIndex]->glide.gammaValue;
      theVSyncEnable = thePrefs->m3D[theIndex]->glide.VSyncEnable;
      the32bit = thePrefs->m3D[theIndex]->glide.force32bit;
      break;    
  }

  dprintf( "hrm_GetGlideEnvVars(): theClientType = %d, theAASample = %d\n", theClientType, theAASample );
  
  if ( theAASample == 1 || theAASample == 0 )
    sb_printf( "FX_GLIDE_NUM_CHIPS = 1\n" );
  else if ( theAASample == 3 )
    sb_printf( "FX_GLIDE_AA_SAMPLE = 2\n" );
  else if ( theAASample == 4 )
    sb_printf( "FX_GLIDE_AA_SAMPLE = 4\n" );

  sb_printf( "FX_GLIDE_AA_TOGGLE_KEY = %d\n", theAAKey );
  
  if ( theGamma < 1.0 ) theGamma = 1.0;
  if ( theGamma > 3.0 ) theGamma = 3.0;
  
  sb_printf( "SSTH3_RGAMMA = %f\n", theGamma );
  sb_printf( "SSTH3_GGAMMA = %f\n", theGamma );
  sb_printf( "SSTH3_BGAMMA = %f\n", theGamma );
  
  if ( theVSyncEnable == 2 )
  {
    sb_printf( "FX_GLIDE_SWAPPENDINGCOUNT = 1\n" );
  }
  else
  {
    sb_printf( "FX_GLIDE_SWAPPENDINGCOUNT = 3\n" );
  }
 
  if ( the32bit )
    sb_printf( "FX_GLIDE_BPP = 32\n" );

  *inEnvData = sb_lock();
  dprintf( "hrm_GetGlideEnvVars(): *inEnvData = 0x%08x, stream = \n%s\n", *inEnvData, *inEnvData );
}





/*
_______________________________________________________ hrm_GetOpenGLEnvVars ___
    
*/

void hrm_GetOpenGLEnvVars( GDHandle inBoard, char ** inEnvData)
{
  HRMPrefsTable * thePrefs = hrm_GetPrefsTable();
  short theIndex;
  
  theIndex = inBoard ? hrm_FindPrefIdx( inBoard ) : 0;
  
  dprintf( "hrm_GetOpenGLEnvVars(): inBoard = 0x%08x, theIndex = %d\n", inBoard, theIndex );
  
  sb_init();
  *inEnvData = sb_lock();

  dprintf( "hrm_GetOpenGLEnvVars(): *inEnvData = 0x%08x, stream = \n%s\n", *inEnvData, *inEnvData );
}





/*
_________________________________________________________ hrm_GetRaveEnvVars ___
    
*/

void hrm_GetRaveEnvVars( GDHandle inBoard, char ** inEnvData)
{
  HRMPrefsTable * thePrefs = hrm_GetPrefsTable();
  short theIndex;
  
  theIndex = inBoard ? hrm_FindPrefIdx( inBoard ) : 0;
  
  dprintf( "hrm_GetRaveEnvVars(): inBoard = 0x%08x, theIndex = %d\n", inBoard, theIndex );
  
  sb_init();
  *inEnvData = sb_lock();

  dprintf( "hrm_GetRaveEnvVars(): *inEnvData = 0x%08x, stream = \n%s\n", *inEnvData, *inEnvData );
}





/*
___________________________________________________________ GetCurrentClient ___
     
*/

HRM3DClientType GetCurrentClient()

{
  return gCurClient;
}



/*
________________________________________________________________________________

    sb_init (private)
    
*/

static char streamBuffer[2048];
static char * sbPtr;

void sb_init()
{

  sbPtr = streamBuffer;
  *sbPtr = 0;
}





/*
________________________________________________________________________________

    sb_printf (private)
    
*/

void sb_printf(	const char * inFormat, ...)
{
  va_list theArgs;

  va_start( theArgs, inFormat );
  vsprintf( sbPtr, inFormat, theArgs );
  va_end( theArgs );
  
  while ( *sbPtr ) sbPtr++;
}





/*
________________________________________________________________________________

    sb_lock (private)
    
*/

char * sb_lock()
{
  return streamBuffer;
}
