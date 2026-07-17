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

#pragma once

#include <MacTypes.h>
#include <QuickDraw.h>

#define HRM_2D_DisableFlags              "2DdisableFlags"
#define HRM_2D_AccelMode                 "2DAccelMode"
#define HRM_2D_FontCaching               "2DFontCaching"
#define HRM_2D_PictCaching               "2DPictCaching"

#define HRM_QT_DisableFlags              "QTdisableFlags"
#define HRM_QT_AccelMode                 "QTAccelMode"
#define HRM_QT_MPEGMode                  "QTMPEGMode"

#define HRM_GlobalOpenGLAntiAliasing     "gOpenGLAA"
#define HRM_GlobalOpenGLAntiAliasingKey  "gOpenGLAAKey"
#define HRM_GlobalOpenGLScreenShotKey    "gOpenGLScreenShotKey"
#define HRM_GlobalOpenGLGamma            "gOpenGLGamma"
#define HRM_GlobalOpenGLVSyncEnable      "gOpenGLVSyncEnable"
#define HRM_GlobalOpenGLSwapPending      "gOpenGLSwapPending"
#define HRM_GlobalOpenGLForce32bit       "gOpenGLForce32bit"

#define HRM_GlobalGlideAntiAliasing      "gGlideAA"
#define HRM_GlobalGlideAntiAliasingKey   "gGlideAAKey"
#define HRM_GlobalGlideScreenShotKey     "gGlideScreenShotKey"
#define HRM_GlobalGlideGamma             "gGlideGamma"
#define HRM_GlobalGlideVSyncEnable       "gGlideVSyncEnable"
#define HRM_GlobalGlideSwapPending       "gGlideSwapPending"
#define HRM_GlobalGlideForce32bit        "gGlideForce32bit"

#define HRM_GlobalRaveAntiAliasing       "gRaveAA"
#define HRM_GlobalRaveAntiAliasingKey    "gRaveAAKey"
#define HRM_GlobalRaveScreenShotKey      "gRaveScreenShotKey"
#define HRM_GlobalRaveGamma              "gRaveGamma"
#define HRM_GlobalRaveVSyncEnable        "gRaveVSyncEnable"
#define HRM_GlobalRaveSwapPending        "gRaveSwapPending"
#define HRM_GlobalRaveForce32bit         "gRaveForce32bit"

#define HRM_GraphicsClockFreq            "GraphicsClockFreq"



#ifdef __cplusplus
extern "C"
{
#endif


typedef struct
{
	UInt32			prefsFlags;
} hrmGameInfo;


short hrm_GetSettingUL(const GDHandle inDisplay, const char * inSettingName, UInt32 * outSetting);
short hrm_SetSettingUL(const GDHandle inDisplay, const char * inSettingName, UInt32 inSetting);
short hrm_GetSettingF(const GDHandle inDisplay, const char * inSettingName, float * outSetting);
short hrm_SetSettingF(const GDHandle inDisplay, const char * inSettingName, float inSetting);

short hrm_GetNumberOfGameSettings();
short hrm_GetGameSettings(char * inIdx, hrmGameInfo * outGameSetting);
short hrm_SetGameSettings(char * inIdx, hrmGameInfo * inGameSetting);


#ifdef __cplusplus
}
#endif

