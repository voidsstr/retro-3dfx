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

#include <QuickDraw.h>

#ifdef __cplusplus
extern "C"
{
#endif


typedef enum
{
  kHRM_OpenGL_Client = 1,
  kHRM_Glide_Client,
  kHRM_Rave_Client
} HRM3DClientType;



void hrm_Register3DClient( HRM3DClientType inClient );
void hrm_GetOpenGLEnvVars( GDHandle inBoard, char ** inEnvData);
void hrm_GetGlideEnvVars( GDHandle inBoard, char ** inEnvData);
void hrm_GetRaveEnvVars( GDHandle inBoard, char ** inEnvData);


#ifdef __cplusplus
}
#endif

