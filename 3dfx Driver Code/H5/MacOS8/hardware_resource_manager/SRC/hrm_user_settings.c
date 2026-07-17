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

#include "hrm_user_settings.h"
#include "hrm_prefs.h"
#include "hrm_priv.h"
#include "hdwr_res_mgr.h"

#include <3dfx.h>
#include <h3cinit.h>
#include <h3regs.h>
#include <minihwc.h>
#include "hwcio.h"

#include <stddef.h>
#include <string.h>



#define IF_SETTING_NAME( name ) \
if ( !strcmp( name, inSettingName ) )

hrmBoard_t* hrm_FindBoardInfo(const GDHandle inDisplay);
FxU32 hrm_GetGraphicsFreq(const GDHandle inDisplay );



/*
_____________________________________________________________ hrm_GetSetting ___

    hrm_GetSetting (exported)
    
*/

short hrm_GetSettingUL(const GDHandle inDisplay, const char * inSettingName, UInt32 * outSetting)
{
  HRMPrefsTable	* thePrefs = hrm_GetPrefsTable();
  long thePrefIdx;
  Boolean theReturn = false;
  
  thePrefIdx = hrm_FindPrefIdx( inDisplay );
  if ( thePrefIdx >= 0 )
  {
    theReturn = true;
    
    IF_SETTING_NAME( HRM_2D_DisableFlags )
    	*outSetting = thePrefs->m2D[ thePrefIdx ]->disableFlags;

    else IF_SETTING_NAME( HRM_2D_AccelMode )
    {
    	if ( thePrefs->m2D[ thePrefIdx ]->disableFlags & kDisableBitBlit )
    		*outSetting = 1;	// disabled
    	else
    	{
    		if ( thePrefs->m2D[ thePrefIdx ]->disableFlags & kDisableStdBitsPatch )
    			*outSetting = 2;	// enabled, but not advanced
    		else
    			*outSetting = 3;
    	}
    }
    	
    else IF_SETTING_NAME( HRM_2D_FontCaching )
    {
    	if ( thePrefs->m2D[ thePrefIdx ]->disableFlags & kDisableTextPatches )
    		*outSetting = 1;	// disabled
    	else
    		*outSetting = 2;	
    }
    	
    else IF_SETTING_NAME( HRM_2D_PictCaching )
    {
    	if ( thePrefs->m2D[ thePrefIdx ]->disableFlags & kDisableDrawPictPatch )
    		*outSetting = 1;	// disabled
    	else
    		*outSetting = 2;	
    }
    	
    else IF_SETTING_NAME( HRM_QT_DisableFlags )
    	*outSetting = thePrefs->mQT[ thePrefIdx ]->disableFlags;
    	
    else IF_SETTING_NAME( HRM_QT_AccelMode )
    {
    	if ( thePrefs->mQT[ thePrefIdx ]->disableFlags & kDisableQTCodec_raw )
    		*outSetting = 1;	// disabled
    	else
    		*outSetting = 2;	
    }
    	
    else IF_SETTING_NAME( HRM_QT_MPEGMode )
    {
    	if ( thePrefs->mQT[ thePrefIdx ]->disableFlags & kDisableQTCodec_mpyc )
    		*outSetting = 1;	// disabled
    	else
    		*outSetting = 2;	
    }



    	
    else IF_SETTING_NAME( HRM_GlobalOpenGLAntiAliasing )
    	*outSetting = thePrefs->m3D[ thePrefIdx ]->ogl.antialiasing;
    	
    else IF_SETTING_NAME( HRM_GlobalOpenGLAntiAliasingKey )
    	*outSetting = thePrefs->m3D[ thePrefIdx ]->ogl.antialiasingKey;
    	
    else IF_SETTING_NAME( HRM_GlobalOpenGLScreenShotKey )
    	*outSetting = thePrefs->m3D[ thePrefIdx ]->ogl.screenShotKey;
    	
    else IF_SETTING_NAME( HRM_GlobalOpenGLVSyncEnable )
    	*outSetting = thePrefs->m3D[ thePrefIdx ]->ogl.VSyncEnable;
    	
    else IF_SETTING_NAME( HRM_GlobalOpenGLSwapPending )
    	*outSetting = thePrefs->m3D[ thePrefIdx ]->ogl.swappending;

   else IF_SETTING_NAME( HRM_GlobalOpenGLForce32bit )
    	*outSetting = thePrefs->m3D[ thePrefIdx ]->ogl.force32bit;



    	
    else IF_SETTING_NAME( HRM_GlobalGlideAntiAliasing )
    	*outSetting = thePrefs->m3D[ thePrefIdx ]->glide.antialiasing;
    	
    else IF_SETTING_NAME( HRM_GlobalGlideAntiAliasingKey )
    	*outSetting = thePrefs->m3D[ thePrefIdx ]->glide.antialiasingKey;
    	
    else IF_SETTING_NAME( HRM_GlobalGlideScreenShotKey )
    	*outSetting = thePrefs->m3D[ thePrefIdx ]->glide.screenShotKey;
    	
    else IF_SETTING_NAME( HRM_GlobalGlideVSyncEnable )
    	*outSetting = thePrefs->m3D[ thePrefIdx ]->glide.VSyncEnable;
    	
    else IF_SETTING_NAME( HRM_GlobalGlideSwapPending )
    	*outSetting = thePrefs->m3D[ thePrefIdx ]->glide.swappending;

   else IF_SETTING_NAME( HRM_GlobalGlideForce32bit )
    	*outSetting = thePrefs->m3D[ thePrefIdx ]->glide.force32bit;



    	
    else IF_SETTING_NAME( HRM_GlobalRaveAntiAliasing )
    	*outSetting = thePrefs->m3D[ thePrefIdx ]->rave.antialiasing;
    	
    else IF_SETTING_NAME( HRM_GlobalRaveAntiAliasingKey )
    	*outSetting = thePrefs->m3D[ thePrefIdx ]->rave.antialiasingKey;
    	
    else IF_SETTING_NAME( HRM_GlobalRaveScreenShotKey )
    	*outSetting = thePrefs->m3D[ thePrefIdx ]->rave.screenShotKey;
    	
    else IF_SETTING_NAME( HRM_GlobalRaveVSyncEnable )
    	*outSetting = thePrefs->m3D[ thePrefIdx ]->rave.VSyncEnable;
    	
    else IF_SETTING_NAME( HRM_GlobalRaveSwapPending )
    	*outSetting = thePrefs->m3D[ thePrefIdx ]->rave.swappending;
    	
   else IF_SETTING_NAME( HRM_GlobalRaveForce32bit )
    	*outSetting = thePrefs->m3D[ thePrefIdx ]->rave.force32bit;



    	
    else IF_SETTING_NAME( HRM_GraphicsClockFreq )
    	*outSetting = hrm_GetGraphicsFreq( inDisplay );
    	


    	
    else theReturn = false;

  }
  
  return theReturn;
}





/*
_____________________________________________________________ hrm_SetSetting ___

    hrm_SetSetting (exported)
    
*/

short hrm_SetSettingUL(const GDHandle inDisplay, const char * inSettingName, UInt32 inSetting)
{
  HRMPrefsTable	* thePrefs = hrm_GetPrefsTable();
  long thePrefIdx;
  Boolean theReturn = false;
  
  thePrefIdx = hrm_FindPrefIdx( inDisplay );
  if ( thePrefIdx >= 0 )
  {
    theReturn = true;
    
    IF_SETTING_NAME( HRM_2D_DisableFlags )
    	thePrefs->m2D[ thePrefIdx ]->disableFlags = inSetting;
    	
    else IF_SETTING_NAME( HRM_2D_AccelMode )
    {
    	#define k2DDisableBasic ( kDisableBitBlit | kDisablePatBlit | kDisableLineBlit | kDisableSlabBlit )
    	#define k2DDisableAdvanced ( kDisableStdBitsPatch | kBitAccurateOnly | kDisableVRAMGWorlds | kDisableAGPGWorlds )
    	#define k2DDisableMask ( k2DDisableBasic | k2DDisableAdvanced )
    	
    	thePrefs->m2D[ thePrefIdx ]->disableFlags &= ~k2DDisableMask;
    	
    	if ( inSetting == 1 )
    		thePrefs->m2D[ thePrefIdx ]->disableFlags |=  k2DDisableBasic | k2DDisableAdvanced;
    	else if ( inSetting == 2 )
    		thePrefs->m2D[ thePrefIdx ]->disableFlags |=  k2DDisableAdvanced;
    }
    	
    else IF_SETTING_NAME( HRM_2D_FontCaching )
    {
    	thePrefs->m2D[ thePrefIdx ]->disableFlags &= ~kDisableTextPatches;
    	
    	if ( inSetting == 1 )
    		thePrefs->m2D[ thePrefIdx ]->disableFlags |=  kDisableTextPatches;
    }
    	
    else IF_SETTING_NAME( HRM_2D_PictCaching )
    {
    	thePrefs->m2D[ thePrefIdx ]->disableFlags &= ~kDisableDrawPictPatch;
    	
    	if ( inSetting == 1 )
    		thePrefs->m2D[ thePrefIdx ]->disableFlags |=  kDisableDrawPictPatch;
    }
    	
    else IF_SETTING_NAME( HRM_QT_DisableFlags )
    	thePrefs->mQT[ thePrefIdx ]->disableFlags = inSetting;
    	
    else IF_SETTING_NAME( HRM_QT_AccelMode )
    {
    	#define kQTDisableBasic ( kDisableQTCodec_raw | kDisableQTCodec_yuvs | kDisableQTCodec_yuv2 | kDisableQTOverlay | kDisableQTFrontend )
    	#define kQTDisableMask ( kQTDisableBasic )
    	
    	thePrefs->mQT[ thePrefIdx ]->disableFlags &= ~kQTDisableMask;
    	
    	if ( inSetting == 1 )
    		thePrefs->mQT[ thePrefIdx ]->disableFlags |=  kQTDisableBasic;
    }
    	
    else IF_SETTING_NAME( HRM_QT_MPEGMode )
    {
    	thePrefs->mQT[ thePrefIdx ]->disableFlags &= ~kDisableQTCodec_mpyc;
    	
    	if ( inSetting == 1 )
    		thePrefs->mQT[ thePrefIdx ]->disableFlags |=  kDisableQTCodec_mpyc;
    }
    	
    else IF_SETTING_NAME( HRM_GlobalOpenGLAntiAliasing )
    	thePrefs->m3D[ thePrefIdx ]->ogl.antialiasing = inSetting;
    	
    else IF_SETTING_NAME( HRM_GlobalOpenGLAntiAliasingKey )
    	thePrefs->m3D[ thePrefIdx ]->ogl.antialiasingKey = inSetting;
    	
    else IF_SETTING_NAME( HRM_GlobalOpenGLScreenShotKey )
    	thePrefs->m3D[ thePrefIdx ]->ogl.screenShotKey = inSetting;
    	
    else IF_SETTING_NAME( HRM_GlobalOpenGLVSyncEnable )
    	thePrefs->m3D[ thePrefIdx ]->ogl.VSyncEnable = inSetting;
    	
    else IF_SETTING_NAME( HRM_GlobalOpenGLSwapPending )
    	thePrefs->m3D[ thePrefIdx ]->ogl.swappending = inSetting;
    	
    else IF_SETTING_NAME( HRM_GlobalOpenGLForce32bit )
    	thePrefs->m3D[ thePrefIdx ]->ogl.force32bit = inSetting;
    	



    else IF_SETTING_NAME( HRM_GlobalGlideAntiAliasing )
    	thePrefs->m3D[ thePrefIdx ]->glide.antialiasing = inSetting;
    	
    else IF_SETTING_NAME( HRM_GlobalGlideAntiAliasingKey )
    	thePrefs->m3D[ thePrefIdx ]->glide.antialiasingKey = inSetting;
    	
    else IF_SETTING_NAME( HRM_GlobalGlideScreenShotKey )
    	thePrefs->m3D[ thePrefIdx ]->glide.screenShotKey = inSetting;
    	
    else IF_SETTING_NAME( HRM_GlobalGlideVSyncEnable )
    	thePrefs->m3D[ thePrefIdx ]->glide.VSyncEnable = inSetting;
    	
    else IF_SETTING_NAME( HRM_GlobalGlideSwapPending )
    	thePrefs->m3D[ thePrefIdx ]->glide.swappending = inSetting;
    	
    else IF_SETTING_NAME( HRM_GlobalGlideForce32bit )
    	thePrefs->m3D[ thePrefIdx ]->glide.force32bit = inSetting;
    	



    else IF_SETTING_NAME( HRM_GlobalRaveAntiAliasing )
    	thePrefs->m3D[ thePrefIdx ]->rave.antialiasing = inSetting;
    	
    else IF_SETTING_NAME( HRM_GlobalRaveAntiAliasingKey )
    	thePrefs->m3D[ thePrefIdx ]->rave.antialiasingKey = inSetting;
    	
    else IF_SETTING_NAME( HRM_GlobalRaveScreenShotKey )
    	thePrefs->m3D[ thePrefIdx ]->rave.screenShotKey = inSetting;
    	
    else IF_SETTING_NAME( HRM_GlobalRaveVSyncEnable )
    	thePrefs->m3D[ thePrefIdx ]->rave.VSyncEnable = inSetting;
    	
    else IF_SETTING_NAME( HRM_GlobalRaveSwapPending )
    	thePrefs->m3D[ thePrefIdx ]->rave.swappending = inSetting;
    	
    else IF_SETTING_NAME( HRM_GlobalRaveForce32bit )
    	thePrefs->m3D[ thePrefIdx ]->rave.force32bit = inSetting;
    	



    else theReturn = false;
    
    
  }
  
  hrm_ChangePrefs( thePrefs, kPrefsFlag2D | kPrefsFlag3D | kPrefsFlagQT );
  return theReturn;
}






/*
____________________________________________________________ hrm_GetSettingF ___
    
*/

short hrm_GetSettingF(const GDHandle inDisplay, const char * inSettingName, float * outSetting)
{
  HRMPrefsTable	* thePrefs = hrm_GetPrefsTable();
  long thePrefIdx;
  Boolean theReturn = false;
  
  thePrefIdx = hrm_FindPrefIdx( inDisplay );
  if ( thePrefIdx >= 0 )
  {
    theReturn = true;
    
    IF_SETTING_NAME( HRM_GlobalOpenGLGamma )
    	*outSetting = thePrefs->m3D[ thePrefIdx ]->ogl.gammaValue;
    	
    else IF_SETTING_NAME( HRM_GlobalGlideGamma )
    	*outSetting = thePrefs->m3D[ thePrefIdx ]->glide.gammaValue;
    	
    else IF_SETTING_NAME( HRM_GlobalRaveGamma )
    	*outSetting = thePrefs->m3D[ thePrefIdx ]->rave.gammaValue;
    	
    else theReturn = false;

  }
    	
  
  return theReturn;
}





/*
____________________________________________________________ hrm_SetSettingF ___
    
*/

short hrm_SetSettingF(const GDHandle inDisplay, const char * inSettingName, float inSetting)
{
  HRMPrefsTable	* thePrefs = hrm_GetPrefsTable();
  long thePrefIdx;
  Boolean theReturn = false;
  
  thePrefIdx = hrm_FindPrefIdx( inDisplay );
  if ( thePrefIdx >= 0 )
  {
    theReturn = true;
    
    IF_SETTING_NAME( HRM_GlobalOpenGLGamma )
    	thePrefs->m3D[ thePrefIdx ]->ogl.gammaValue = inSetting;
    	
    else IF_SETTING_NAME( HRM_GlobalGlideGamma )
    	thePrefs->m3D[ thePrefIdx ]->glide.gammaValue = inSetting;
    	
    else IF_SETTING_NAME( HRM_GlobalRaveGamma )
    	thePrefs->m3D[ thePrefIdx ]->rave.gammaValue = inSetting;
    	

    else theReturn = false;
    
    
  }
  
  hrm_ChangePrefs( thePrefs, kPrefsFlag3D );
  return theReturn;
}





/*
________________________________________________ hrm_GetNumberOfGameSettings ___

    hrm_GetNumberOfGameSettings (exported)
    
*/

short hrm_GetNumberOfGameSettings()
{
  return 0;
}





/*
________________________________________________________ hrm_GetGameSettings ___

    hrm_GetGameSettings (exported)
    
*/

short hrm_GetGameSettings(char * /*inIdx*/, hrmGameInfo * /*outGameSetting*/)
{
  return -1;
}





/*
________________________________________________________ hrm_SetGameSettings ___

    hrm_SetGameSettings (exported)
    
*/

short hrm_SetGameSettings(char * /*inIdx*/, hrmGameInfo * /*inGameSetting*/)
{
  return -1;
}

#pragma mark -







/*
____________________________________________________________ hrm_FindPrefIdx ___
    
*/

long hrm_FindPrefIdx(const GDHandle inDisplay)
{
  hrmBoard_t * theBoard = hrm_FindBoardInfo( inDisplay );
  return theBoard ? theBoard->prefsIdx : -1;
}







/*
__________________________________________________________ hrm_FindBoardInfo ___
    
*/

hrmBoard_t* hrm_FindBoardInfo(const GDHandle inDisplay)
{
  return ( hrmIdentifyTarget( (*inDisplay)->gdRefNum ) );
}







/*
________________________________________________________ hrm_GetGraphicsFreq ___
    
*/

FxU32 hrm_GetGraphicsFreq(const GDHandle inDisplay )
{
  hrmBoard_t * theBoard = hrm_FindBoardInfo( inDisplay );
  long n,m,k;
  float freq;
  FxU32 theValue;

  HWC_IO_LOAD( theBoard->boardInfo.regInfo, pllCtrl1, theValue );

  n = (theValue >> 8) & 0xFF;
  m = (theValue >> 2) & 0x3F;
  k = 1 << (theValue & 0x3);

  freq = (14.31818 * ((float) n + 2)) / ( ((float) m + 2 ) * ( (float) k ) );
  theValue = (unsigned long) (freq * 100.0);
  
  return theValue;
}

