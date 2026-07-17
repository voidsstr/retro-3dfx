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
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished - 
** rights reserved under the Copyright Laws of the United States. 
** 
** 
** 
*/ 
#include <windows.h>
#include <glide.h>
#include <math.h>
#include <GL/gl.h>
#include "glint.h"

#include <stdio.h>

void APIENTRY glLightModelf (GLenum pname, GLfloat param)
{
  GLINT_OUTSIDE_BEGIN();

  if(pglCurContext->Listing) {

    switch(pname) {
    case GL_LIGHT_MODEL_LOCAL_VIEWER:
      if(param == 0.0f) {
	glIntIntToList(OP_LOCALVIEWER);
	glIntIntToList((int)FALSE);
      } else {
	glIntIntToList(OP_LOCALVIEWER);
	glIntIntToList((int)TRUE);
      }
      break;
    case GL_LIGHT_MODEL_TWO_SIDE:
      if(param == 0.0f) {
	glIntIntToList(OP_TWOSIDE);
	glIntIntToList((int)FALSE);
      } else {
	glIntIntToList(OP_TWOSIDE);
	glIntIntToList((int)TRUE);
      }
      break;
    default:
      GLINT_ERROR(GL_INVALID_ENUM);
      return;
      break;
    }
    
    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  switch(pname) {
  case GL_LIGHT_MODEL_LOCAL_VIEWER:
    if(param == 0.0f) {
      pglCurContext->Local_Viewer = FALSE;
    } else {
      pglCurContext->Local_Viewer = TRUE;
    }
    pglCurContext->NeedEyeDirty = TRUE;
    break;
  case GL_LIGHT_MODEL_TWO_SIDE:
    if(param == 0.0f) {
      pglCurContext->Two_Sided = FALSE;
    } else {
      pglCurContext->Two_Sided = TRUE;
    }
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }
  pglCurContext->LightDirty = TRUE;
}

void APIENTRY glLightModelfv (GLenum pname, const GLfloat *params)
{
  GLINT_OUTSIDE_BEGIN();

  if(pglCurContext->Listing) {

    switch(pname) {
    case GL_LIGHT_MODEL_LOCAL_VIEWER:
      if(params[0] == 0.0f) {
	glIntIntToList(OP_LOCALVIEWER);
	glIntIntToList((int)FALSE);
      } else {
	glIntIntToList(OP_LOCALVIEWER);
	glIntIntToList((int)TRUE);
      }
      break;
    case GL_LIGHT_MODEL_TWO_SIDE:
      if(params[0] == 0.0f) {
	glIntIntToList(OP_TWOSIDE);
	glIntIntToList((int)FALSE);
      } else {
	glIntIntToList(OP_TWOSIDE);
	glIntIntToList((int)TRUE);
      }
      break;
    case GL_LIGHT_MODEL_AMBIENT:
      glIntIntToList(OP_LIGHTMODELAMBIENT);
      glIntFloatToList(params[0]);
      glIntFloatToList(params[1]);
      glIntFloatToList(params[2]);
      break;
    default:
      GLINT_ERROR(GL_INVALID_ENUM);
      return;
      break;
    }
    
    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  switch(pname) {
  case GL_LIGHT_MODEL_LOCAL_VIEWER:
    if(params[0] == 0.0f) {
      pglCurContext->Local_Viewer = FALSE;
    } else {
      pglCurContext->Local_Viewer = TRUE;
    }
    pglCurContext->NeedEyeDirty = TRUE;
    break;
  case GL_LIGHT_MODEL_TWO_SIDE:
    if(params[0] == 0.0f) {
      pglCurContext->Two_Sided = FALSE;
    } else {
      pglCurContext->Two_Sided = TRUE;
    }
    break;
  case GL_LIGHT_MODEL_AMBIENT:
    pglCurContext->LightModelAmbient[0] = params[0];
    pglCurContext->LightModelAmbient[1] = params[1];
    pglCurContext->LightModelAmbient[2] = params[2];
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }
  pglCurContext->LightDirty = TRUE;
}

void APIENTRY glLightModeli (GLenum pname, GLint param)
{
  GLINT_OUTSIDE_BEGIN();

  if(pglCurContext->Listing) {

    switch(pname) {
    case GL_LIGHT_MODEL_LOCAL_VIEWER:
      if(param == 0) {
	glIntIntToList(OP_LOCALVIEWER);
	glIntIntToList((int)FALSE);
      } else {
	glIntIntToList(OP_LOCALVIEWER);
	glIntIntToList((int)TRUE);
      }
      break;
    case GL_LIGHT_MODEL_TWO_SIDE:
      if(param == 0) {
	glIntIntToList(OP_TWOSIDE);
	glIntIntToList((int)FALSE);
      } else {
	glIntIntToList(OP_TWOSIDE);
	glIntIntToList((int)TRUE);
      }
      break;
    default:
      GLINT_ERROR(GL_INVALID_ENUM);
      return;
      break;
    }
    
    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  switch(pname) {
  case GL_LIGHT_MODEL_LOCAL_VIEWER:
    if(param == 0) {
      pglCurContext->Local_Viewer = FALSE;
    } else {
      pglCurContext->Local_Viewer = TRUE;
    }
    pglCurContext->NeedEyeDirty = TRUE;
    break;
  case GL_LIGHT_MODEL_TWO_SIDE:
    if(param == 0) {
      pglCurContext->Two_Sided = FALSE;
    } else {
      pglCurContext->Two_Sided = TRUE;
    }
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }
  pglCurContext->LightDirty = TRUE;
}

void APIENTRY glLightModeliv (GLenum pname, const GLint *params)
{
  GLINT_OUTSIDE_BEGIN();

  if(pglCurContext->Listing) {

    switch(pname) {
    case GL_LIGHT_MODEL_LOCAL_VIEWER:
      if(params[0] == 0) {
	glIntIntToList(OP_LOCALVIEWER);
	glIntIntToList((int)FALSE);
      } else {
	glIntIntToList(OP_LOCALVIEWER);
	glIntIntToList((int)TRUE);
      }
      break;
    case GL_LIGHT_MODEL_TWO_SIDE:
      if(params[0] == 0) {
	glIntIntToList(OP_TWOSIDE);
	glIntIntToList((int)FALSE);
      } else {
	glIntIntToList(OP_TWOSIDE);
	glIntIntToList((int)TRUE);
      }
      break;
    case GL_LIGHT_MODEL_AMBIENT:
      glIntIntToList(OP_LIGHTMODELAMBIENT);
      glIntFloatToList(LighttoFloat(params[0]));
      glIntFloatToList(LighttoFloat(params[1]));
      glIntFloatToList(LighttoFloat(params[2]));
      break;
    default:
      GLINT_ERROR(GL_INVALID_ENUM);
      return;
      break;
    }
    
    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  switch(pname) {
  case GL_LIGHT_MODEL_LOCAL_VIEWER:
    if(params[0] == 0) {
      pglCurContext->Local_Viewer = FALSE;
    } else {
      pglCurContext->Local_Viewer = TRUE;
    }
    pglCurContext->NeedEyeDirty = TRUE;
    break;
  case GL_LIGHT_MODEL_TWO_SIDE:
    if(params[0] == 0) {
      pglCurContext->Two_Sided = FALSE;
    } else {
      pglCurContext->Two_Sided = TRUE;
    }
    break;
  case GL_LIGHT_MODEL_AMBIENT:
    pglCurContext->LightModelAmbient[0] = LighttoFloat(params[0]);
    pglCurContext->LightModelAmbient[1] = LighttoFloat(params[1]);
    pglCurContext->LightModelAmbient[2] = LighttoFloat(params[2]);
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }
  pglCurContext->LightDirty = TRUE;
}

void APIENTRY glLightf (GLenum light, GLenum pname, GLfloat param)
{
  int index;

  GLINT_OUTSIDE_BEGIN();

  switch(light) {
  case GL_LIGHT0:
    index = 0;
    break;
  case GL_LIGHT1:
    index = 1;
    break;
  case GL_LIGHT2:
    index = 2;
    break;
  case GL_LIGHT3:
    index = 3;
    break;
  case GL_LIGHT4:
    index = 4;
    break;
  case GL_LIGHT5:
    index = 5;
    break;
  case GL_LIGHT6:
    index = 6;
    break;
  case GL_LIGHT7:
    index = 7;
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }

  if(pglCurContext->Listing) {

    switch(pname) {
    case GL_SPOT_EXPONENT:
      glIntIntToList(OP_SPOTEXPONENT);
      glIntIntToList(index);
      glIntFloatToList(param);
      break;
    case GL_SPOT_CUTOFF:
      glIntIntToList(OP_SPOTCUTOFF);
      glIntIntToList(index);
      glIntFloatToList(param);
      break;
    case GL_CONSTANT_ATTENUATION:
      glIntIntToList(OP_CONSTANTATTENUATION);
      glIntIntToList(index);
      glIntFloatToList(param);
      break;
    case GL_LINEAR_ATTENUATION:
      glIntIntToList(OP_LINEARATTENUATION);
      glIntIntToList(index);
      glIntFloatToList(param);
      break;
    case GL_QUADRATIC_ATTENUATION:
      glIntIntToList(OP_QUADRATICATTENUATION);
      glIntIntToList(index);
      glIntFloatToList(param);
      break;
    default:
      GLINT_ERROR(GL_INVALID_ENUM);
      return;
      break;
    }
    
    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  switch(pname) {
  case GL_SPOT_EXPONENT:
    pglCurContext->Light[index].SpotExponent = param;
    break;
  case GL_SPOT_CUTOFF:
    pglCurContext->Light[index].SpotCutoff = param;
    break;
  case GL_CONSTANT_ATTENUATION:
    pglCurContext->Light[index].ConstantAtten = param;
    break;
  case GL_LINEAR_ATTENUATION:
    pglCurContext->Light[index].LinearAtten = param;
    break;
  case GL_QUADRATIC_ATTENUATION:
    pglCurContext->Light[index].QuadraticAtten = param;
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }
  pglCurContext->LightDirty = TRUE;
}

void APIENTRY glLightfv (GLenum light, GLenum pname, const GLfloat *params)
{
  int index;

  GLINT_OUTSIDE_BEGIN();

  switch(light) {
  case GL_LIGHT0:
    index = 0;
    break;
  case GL_LIGHT1:
    index = 1;
    break;
  case GL_LIGHT2:
    index = 2;
    break;
  case GL_LIGHT3:
    index = 3;
    break;
  case GL_LIGHT4:
    index = 4;
    break;
  case GL_LIGHT5:
    index = 5;
    break;
  case GL_LIGHT6:
    index = 6;
    break;
  case GL_LIGHT7:
    index = 7;
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }

  if(pglCurContext->Listing) {
    
    switch(pname) {
    case GL_SPOT_EXPONENT:
      glIntIntToList(OP_SPOTEXPONENT);
      glIntIntToList(index);
      glIntFloatToList(params[0]);
      break;
    case GL_SPOT_CUTOFF:
      glIntIntToList(OP_SPOTCUTOFF);
      glIntIntToList(index);
      glIntFloatToList(params[0]);
      break;
    case GL_CONSTANT_ATTENUATION:
      glIntIntToList(OP_CONSTANTATTENUATION);
      glIntIntToList(index);
      glIntFloatToList(params[0]);
      break;
    case GL_LINEAR_ATTENUATION:
      glIntIntToList(OP_LINEARATTENUATION);
      glIntIntToList(index);
      glIntFloatToList(params[0]);
      break;
    case GL_QUADRATIC_ATTENUATION:
      glIntIntToList(OP_QUADRATICATTENUATION);
      glIntIntToList(index);
      glIntFloatToList(params[0]);
      break;
    case GL_AMBIENT:
      glIntIntToList(OP_AMBIENT);
      glIntIntToList(index);
      glIntFloatToList(params[0]);
      glIntFloatToList(params[1]);
      glIntFloatToList(params[2]);
      break;
    case GL_DIFFUSE:
      glIntIntToList(OP_DIFFUSE);
      glIntIntToList(index);
      glIntFloatToList(params[0]);
      glIntFloatToList(params[1]);
      glIntFloatToList(params[2]);
      glIntFloatToList(params[3]);
      break;
    case GL_AMBIENT_AND_DIFFUSE:
      glIntIntToList(OP_AMBIENT);
      glIntIntToList(index);
      glIntFloatToList(params[0]);
      glIntFloatToList(params[1]);
      glIntFloatToList(params[2]);

      glIntIntToList(OP_DIFFUSE);
      glIntIntToList(index);
      glIntFloatToList(params[0]);
      glIntFloatToList(params[1]);
      glIntFloatToList(params[2]);
      glIntFloatToList(params[3]);
      break;
    case GL_SPECULAR:
      glIntIntToList(OP_SPECULAR);
      glIntIntToList(index);
      glIntFloatToList(params[0]);
      glIntFloatToList(params[1]);
      glIntFloatToList(params[2]);
      break;
    case GL_POSITION:
      glIntIntToList(OP_POSITION);
      glIntIntToList(index);
      glIntFloatToList(params[0]);
      glIntFloatToList(params[1]);
      glIntFloatToList(params[2]);
      glIntFloatToList(params[3]);
      break;
    case GL_SPOT_DIRECTION:
      glIntIntToList(OP_SPOTDIRECTION);
      glIntIntToList(index);
      glIntFloatToList(params[0]);
      glIntFloatToList(params[1]);
      glIntFloatToList(params[2]);
      glIntFloatToList(params[3]);
      break;
    default:
      GLINT_ERROR(GL_INVALID_ENUM);
      return;
      break;
    }

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  switch(pname) {
  case GL_SPOT_EXPONENT:
    pglCurContext->Light[index].SpotExponent = params[0];
    break;
  case GL_SPOT_CUTOFF:
    pglCurContext->Light[index].SpotCutoff = params[0];
    break;
  case GL_CONSTANT_ATTENUATION:
    pglCurContext->Light[index].ConstantAtten = params[0];
    break;
  case GL_LINEAR_ATTENUATION:
    pglCurContext->Light[index].LinearAtten = params[0];
    break;
  case GL_QUADRATIC_ATTENUATION:
    pglCurContext->Light[index].QuadraticAtten = params[0];
    break;
  case GL_AMBIENT:
    pglCurContext->Light[index].Ambient[0] = params[0];
    pglCurContext->Light[index].Ambient[1] = params[1];
    pglCurContext->Light[index].Ambient[2] = params[2];
    break;
  case GL_DIFFUSE:
    pglCurContext->Light[index].Diffuse[0] = params[0];
    pglCurContext->Light[index].Diffuse[1] = params[1];
    pglCurContext->Light[index].Diffuse[2] = params[2];
    pglCurContext->Light[index].Diffuse[3] = params[3];
    break;
  case GL_AMBIENT_AND_DIFFUSE:
    pglCurContext->Light[index].Ambient[0] = params[0];
    pglCurContext->Light[index].Ambient[1] = params[1];
    pglCurContext->Light[index].Ambient[2] = params[2];

    pglCurContext->Light[index].Diffuse[0] = params[0];
    pglCurContext->Light[index].Diffuse[1] = params[1];
    pglCurContext->Light[index].Diffuse[2] = params[2];
    pglCurContext->Light[index].Diffuse[3] = params[3];
    break;
  case GL_SPECULAR:
    pglCurContext->Light[index].Specular[0] = params[0];
    pglCurContext->Light[index].Specular[1] = params[1];
    pglCurContext->Light[index].Specular[2] = params[2];
    break;
  case GL_POSITION:
    {
      GLfloat tmpparams[4];
      
      tmpparams[0] = params[0];
      tmpparams[1] = params[1];
      tmpparams[2] = params[2];
      tmpparams[3] = params[3];

      if(tmpparams[3] == 0.0f) {
        glIntValidateInvTransp();
	glIntXform(tmpparams,
		   pglCurContext->CurInvTransp,
		   pglCurContext->Light[index].Position);
	pglCurContext->Light[index].Position[3] = 0.0f;
      } else {
	glIntXform(tmpparams,
		   pglCurContext->CurModelView,
		   pglCurContext->Light[index].Position);
      }
      pglCurContext->NeedEyeDirty = TRUE;
    }
    break;
  case GL_SPOT_DIRECTION:
    {
      GLfloat tmpparams[4];
      
      tmpparams[0] = params[0];
      tmpparams[1] = params[1];
      tmpparams[2] = params[2];
      tmpparams[3] = params[3];

      if(pglCurContext->Light[index].Position[3] != 0.0f) {
	tmpparams[3] = 
	  -((tmpparams[0]*pglCurContext->Light[index].Position[0])+
	    (tmpparams[1]*pglCurContext->Light[index].Position[1])+
	    (tmpparams[2]*pglCurContext->Light[index].Position[2]));
      } else {
	tmpparams[3] = 0.0f;
      }
      
      // Transform Direction Using Current Inverse Transpose
      glIntValidateInvTransp();
      glIntXform(tmpparams,
		 pglCurContext->CurInvTransp,
		 pglCurContext->Light[index].SpotDirection);
      glIntNormalize(pglCurContext->Light[index].SpotDirection);
    }
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }
  pglCurContext->LightDirty = TRUE;
}

void APIENTRY glLighti (GLenum light, GLenum pname, GLint param)
{
  int index;

  GLINT_OUTSIDE_BEGIN();

  switch(light) {
  case GL_LIGHT0:
    index = 0;
    break;
  case GL_LIGHT1:
    index = 1;
    break;
  case GL_LIGHT2:
    index = 2;
    break;
  case GL_LIGHT3:
    index = 3;
    break;
  case GL_LIGHT4:
    index = 4;
    break;
  case GL_LIGHT5:
    index = 5;
    break;
  case GL_LIGHT6:
    index = 6;
    break;
  case GL_LIGHT7:
    index = 7;
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }

  if(pglCurContext->Listing) {

    switch(pname) {
    case GL_SPOT_EXPONENT:
      glIntIntToList(OP_SPOTEXPONENT);
      glIntIntToList(index);
      glIntFloatToList((float)param);
      break;
    case GL_SPOT_CUTOFF:
      glIntIntToList(OP_SPOTCUTOFF);
      glIntIntToList(index);
      glIntFloatToList((float)param);
      break;
    case GL_CONSTANT_ATTENUATION:
      glIntIntToList(OP_CONSTANTATTENUATION);
      glIntIntToList(index);
      glIntFloatToList((float)param);
      break;
    case GL_LINEAR_ATTENUATION:
      glIntIntToList(OP_LINEARATTENUATION);
      glIntIntToList(index);
      glIntFloatToList((float)param);
      break;
    case GL_QUADRATIC_ATTENUATION:
      glIntIntToList(OP_QUADRATICATTENUATION);
      glIntIntToList(index);
      glIntFloatToList((float)param);
      break;
    default:
      GLINT_ERROR(GL_INVALID_ENUM);
      return;
      break;
    }
    
    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  switch(pname) {
  case GL_SPOT_EXPONENT:
    pglCurContext->Light[index].SpotExponent = (float)param;
    break;
  case GL_SPOT_CUTOFF:
    pglCurContext->Light[index].SpotCutoff = (float)param;
    break;
  case GL_CONSTANT_ATTENUATION:
    pglCurContext->Light[index].ConstantAtten = (float)param;
    break;
  case GL_LINEAR_ATTENUATION:
    pglCurContext->Light[index].LinearAtten = (float)param;
    break;
  case GL_QUADRATIC_ATTENUATION:
    pglCurContext->Light[index].QuadraticAtten = (float)param;
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    break;
  }
  pglCurContext->LightDirty = TRUE;
}

void APIENTRY glLightiv (GLenum light, GLenum pname, const GLint *params)
{
  int index;

  GLINT_OUTSIDE_BEGIN();

  switch(light) {
  case GL_LIGHT0:
    index = 0;
    break;
  case GL_LIGHT1:
    index = 1;
    break;
  case GL_LIGHT2:
    index = 2;
    break;
  case GL_LIGHT3:
    index = 3;
    break;
  case GL_LIGHT4:
    index = 4;
    break;
  case GL_LIGHT5:
    index = 5;
    break;
  case GL_LIGHT6:
    index = 6;
    break;
  case GL_LIGHT7:
    index = 7;
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }

  if(pglCurContext->Listing) {
    
    switch(pname) {
    case GL_SPOT_EXPONENT:
      glIntIntToList(OP_SPOTEXPONENT);
      glIntIntToList(index);
      glIntFloatToList((float)params[0]);
      break;
    case GL_SPOT_CUTOFF:
      glIntIntToList(OP_SPOTCUTOFF);
      glIntIntToList(index);
      glIntFloatToList((float)params[0]);
      break;
    case GL_CONSTANT_ATTENUATION:
      glIntIntToList(OP_CONSTANTATTENUATION);
      glIntIntToList(index);
      glIntFloatToList((float)params[0]);
      break;
    case GL_LINEAR_ATTENUATION:
      glIntIntToList(OP_LINEARATTENUATION);
      glIntIntToList(index);
      glIntFloatToList((float)params[0]);
      break;
    case GL_QUADRATIC_ATTENUATION:
      glIntIntToList(OP_QUADRATICATTENUATION);
      glIntIntToList(index);
      glIntFloatToList((float)params[0]);
      break;
    case GL_AMBIENT:
      glIntIntToList(OP_AMBIENT);
      glIntIntToList(index);
      glIntFloatToList(LighttoFloat(params[0]));
      glIntFloatToList(LighttoFloat(params[1]));
      glIntFloatToList(LighttoFloat(params[2]));
      break;
    case GL_DIFFUSE:
      glIntIntToList(OP_DIFFUSE);
      glIntIntToList(index);
      glIntFloatToList(LighttoFloat(params[0]));
      glIntFloatToList(LighttoFloat(params[1]));
      glIntFloatToList(LighttoFloat(params[2]));
      glIntFloatToList(LighttoFloat(params[3]));
      break;
    case GL_AMBIENT_AND_DIFFUSE:
      glIntIntToList(OP_AMBIENT);
      glIntIntToList(index);
      glIntFloatToList(LighttoFloat(params[0]));
      glIntFloatToList(LighttoFloat(params[1]));
      glIntFloatToList(LighttoFloat(params[2]));

      glIntIntToList(OP_DIFFUSE);
      glIntIntToList(index);
      glIntFloatToList(LighttoFloat(params[0]));
      glIntFloatToList(LighttoFloat(params[1]));
      glIntFloatToList(LighttoFloat(params[2]));
      glIntFloatToList(LighttoFloat(params[3]));
      break;
    case GL_SPECULAR:
      glIntIntToList(OP_SPECULAR);
      glIntIntToList(index);
      glIntFloatToList(LighttoFloat(params[0]));
      glIntFloatToList(LighttoFloat(params[1]));
      glIntFloatToList(LighttoFloat(params[2]));
      break;
    case GL_POSITION:
      glIntIntToList(OP_POSITION);
      glIntIntToList(index);
      glIntFloatToList((float)params[0]);
      glIntFloatToList((float)params[1]);
      glIntFloatToList((float)params[2]);
      glIntFloatToList((float)params[3]);
      break;
    case GL_SPOT_DIRECTION:
      glIntIntToList(OP_SPOTDIRECTION);
      glIntIntToList(index);
      glIntFloatToList((float)params[0]);
      glIntFloatToList((float)params[1]);
      glIntFloatToList((float)params[2]);
      glIntFloatToList((float)params[3]);
      break;
    default:
      GLINT_ERROR(GL_INVALID_ENUM);
      return;
      break;
    }

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  switch(pname) {
  case GL_SPOT_EXPONENT:
    pglCurContext->Light[index].SpotExponent = (float)params[0];
    break;
  case GL_SPOT_CUTOFF:
    pglCurContext->Light[index].SpotCutoff = (float)params[0];
    break;
  case GL_CONSTANT_ATTENUATION:
    pglCurContext->Light[index].ConstantAtten = (float)params[0];
    break;
  case GL_LINEAR_ATTENUATION:
    pglCurContext->Light[index].LinearAtten = (float)params[0];
    break;
  case GL_QUADRATIC_ATTENUATION:
    pglCurContext->Light[index].QuadraticAtten = (float)params[0];
    break;
  case GL_AMBIENT:
    pglCurContext->Light[index].Ambient[0] = LighttoFloat(params[0]);
    pglCurContext->Light[index].Ambient[1] = LighttoFloat(params[1]);
    pglCurContext->Light[index].Ambient[2] = LighttoFloat(params[2]);
    break;
  case GL_DIFFUSE:
    pglCurContext->Light[index].Diffuse[0] = LighttoFloat(params[0]);
    pglCurContext->Light[index].Diffuse[1] = LighttoFloat(params[1]);
    pglCurContext->Light[index].Diffuse[2] = LighttoFloat(params[2]);
    pglCurContext->Light[index].Diffuse[3] = LighttoFloat(params[3]);
    break;
  case GL_AMBIENT_AND_DIFFUSE:
    pglCurContext->Light[index].Ambient[0] = LighttoFloat(params[0]);
    pglCurContext->Light[index].Ambient[1] = LighttoFloat(params[1]);
    pglCurContext->Light[index].Ambient[2] = LighttoFloat(params[2]);

    pglCurContext->Light[index].Diffuse[0] = LighttoFloat(params[0]);
    pglCurContext->Light[index].Diffuse[1] = LighttoFloat(params[1]);
    pglCurContext->Light[index].Diffuse[2] = LighttoFloat(params[2]);
    pglCurContext->Light[index].Diffuse[3] = LighttoFloat(params[3]);
    break;
  case GL_SPECULAR:
    pglCurContext->Light[index].Specular[0] = LighttoFloat(params[0]);
    pglCurContext->Light[index].Specular[1] = LighttoFloat(params[1]);
    pglCurContext->Light[index].Specular[2] = LighttoFloat(params[2]);
    break;
  case GL_POSITION:
    {
      GLfloat tmpparams[4];
      
      tmpparams[0] = (float)params[0];
      tmpparams[1] = (float)params[1];
      tmpparams[2] = (float)params[2];
      tmpparams[3] = (float)params[3];

      if(tmpparams[3] == 0.0f) {
	glIntValidateInvTransp();
	glIntXform(tmpparams,
		   pglCurContext->CurInvTransp,
		   pglCurContext->Light[index].Position);
	pglCurContext->Light[index].Position[3] = 0.0f;
      } else {
	glIntXform(tmpparams,
		   pglCurContext->CurModelView,
		   pglCurContext->Light[index].Position);
	pglCurContext->NeedEyeDirty = TRUE;
      }
    }
    break;
  case GL_SPOT_DIRECTION:
    {
      GLfloat tmpparams[4];
      
      tmpparams[0] = (float)params[0];
      tmpparams[1] = (float)params[1];
      tmpparams[2] = (float)params[2];
      tmpparams[3] = (float)params[3];

      if(pglCurContext->Light[index].Position[3] != 0.0f) {
	tmpparams[3] = 
	  -((tmpparams[0]*pglCurContext->Light[index].Position[0])+
	    (tmpparams[1]*pglCurContext->Light[index].Position[1])+
	    (tmpparams[2]*pglCurContext->Light[index].Position[2]));
      } else {
	tmpparams[3] = 0.0f;
      }
      
      // Transform Direction Using Current Inverse Transpose
      glIntValidateInvTransp();
      glIntXform(tmpparams,
		 pglCurContext->CurInvTransp,
		 pglCurContext->Light[index].SpotDirection);
      glIntNormalize(pglCurContext->Light[index].SpotDirection);
    }
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }
  pglCurContext->LightDirty = TRUE;
}

void APIENTRY glGetLightfv (GLenum light, GLenum pname, GLfloat *params)
{
  glIntGetLight(light, pname, (void *)params, READ_FLOAT);
}

void APIENTRY glGetLightiv (GLenum light, GLenum pname, GLint *params)
{
  glIntGetLight(light, pname, (void *)params, READ_INT);
}

void APIENTRY glMaterialf (GLenum face, GLenum pname, GLfloat param)
{
  GLINT_OUTSIDE_BEGIN();

  if(pglCurContext->Listing) {
    
    if((face == GL_FRONT) || (face == GL_FRONT_AND_BACK)) {
      switch(pname) {
      case GL_SHININESS:
	glIntIntToList(OP_FRONTSHININESS);
	glIntFloatToList(param);
	break;
      default:
	GLINT_ERROR(GL_INVALID_ENUM);
	return;
	break;
      }
    }
      
    if((face == GL_BACK) || (face == GL_FRONT_AND_BACK)) {
      switch(pname) {
      case GL_SHININESS:
	glIntIntToList(OP_BACKSHININESS);
	glIntFloatToList(param);
	break;
      default:
	GLINT_ERROR(GL_INVALID_ENUM);
	return;
	break;
      }
    }

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }
  
  if((face == GL_FRONT) || (face == GL_FRONT_AND_BACK)) {
    switch(pname) {
    case GL_SHININESS:
      pglCurContext->FrontShininess = param;
      break;
    default:
      GLINT_ERROR(GL_INVALID_ENUM);
      return;
      break;
    }
  }
  
  if((face == GL_BACK) || (face == GL_FRONT_AND_BACK)) {
    switch(pname) {
    case GL_SHININESS:
      pglCurContext->BackShininess = param;
      break;
    default:
      GLINT_ERROR(GL_INVALID_ENUM);
      return;
      break;
    }
  }
  pglCurContext->LightDirty = TRUE;
}

void APIENTRY glMaterialfv (GLenum face, GLenum pname, const GLfloat *params)
{
  GLINT_OUTSIDE_BEGIN();

  if(pglCurContext->Listing) {
    
    if((face == GL_FRONT) || (face == GL_FRONT_AND_BACK)) {
      switch(pname) {
      case GL_SHININESS:
	glIntIntToList(OP_FRONTSHININESS);
	glIntFloatToList(params[0]);
	break;
      case GL_AMBIENT:
	glIntIntToList(OP_FRONTAMBIENT);
	glIntFloatToList(params[0]);
	glIntFloatToList(params[1]);
	glIntFloatToList(params[2]);
	break;
      case GL_DIFFUSE:
	glIntIntToList(OP_FRONTDIFFUSE);
	glIntFloatToList(params[0]);
	glIntFloatToList(params[1]);
	glIntFloatToList(params[2]);
	glIntFloatToList(params[3]);
	break;
      case GL_AMBIENT_AND_DIFFUSE:
	glIntIntToList(OP_FRONTAMBIENT);
	glIntFloatToList(params[0]);
	glIntFloatToList(params[1]);
	glIntFloatToList(params[2]);
	
	glIntIntToList(OP_FRONTDIFFUSE);
	glIntFloatToList(params[0]);
	glIntFloatToList(params[1]);
	glIntFloatToList(params[2]);
	glIntFloatToList(params[3]);
	break;
      case GL_SPECULAR:
	glIntIntToList(OP_FRONTSPECULAR);
	glIntFloatToList(params[0]);
	glIntFloatToList(params[1]);
	glIntFloatToList(params[2]);
	break;
      case GL_EMISSION:
	glIntIntToList(OP_FRONTEMISSION);
	glIntFloatToList(params[0]);
	glIntFloatToList(params[1]);
	glIntFloatToList(params[2]);
	break;
      default:
	GLINT_ERROR(GL_INVALID_ENUM);
	return;
	break;
      }
    }
  
    if((face == GL_BACK) || (face == GL_FRONT_AND_BACK)) {
      switch(pname) {
      case GL_SHININESS:
	glIntIntToList(OP_BACKSHININESS);
	glIntFloatToList(params[0]);
	break;
      case GL_AMBIENT:
	glIntIntToList(OP_BACKAMBIENT);
	glIntFloatToList(params[0]);
	glIntFloatToList(params[1]);
	glIntFloatToList(params[2]);
	break;
      case GL_DIFFUSE:
	glIntIntToList(OP_BACKDIFFUSE);
	glIntFloatToList(params[0]);
	glIntFloatToList(params[1]);
	glIntFloatToList(params[2]);
	glIntFloatToList(params[3]);
	break;
      case GL_AMBIENT_AND_DIFFUSE:
	glIntIntToList(OP_BACKAMBIENT);
	glIntFloatToList(params[0]);
	glIntFloatToList(params[1]);
	glIntFloatToList(params[2]);
	
	glIntIntToList(OP_BACKDIFFUSE);
	glIntFloatToList(params[0]);
	glIntFloatToList(params[1]);
	glIntFloatToList(params[2]);
	glIntFloatToList(params[3]);
	break;
      case GL_SPECULAR:
	glIntIntToList(OP_BACKSPECULAR);
	glIntFloatToList(params[0]);
	glIntFloatToList(params[1]);
	glIntFloatToList(params[2]);
	break;
      case GL_EMISSION:
	glIntIntToList(OP_BACKEMISSION);
	glIntFloatToList(params[0]);
	glIntFloatToList(params[1]);
	glIntFloatToList(params[2]);
	break;
      default:
	GLINT_ERROR(GL_INVALID_ENUM);
	return;
	break;
      }
    }
    
    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  if((face == GL_FRONT) || (face == GL_FRONT_AND_BACK)) {
    switch(pname) {
    case GL_SHININESS:
      pglCurContext->FrontShininess = params[0];
      break;
    case GL_AMBIENT:
      pglCurContext->FrontAmbient[0] = params[0];
      pglCurContext->FrontAmbient[1] = params[1];
      pglCurContext->FrontAmbient[2] = params[2];
      break;
    case GL_DIFFUSE:
      pglCurContext->FrontDiffuse[0] = params[0];
      pglCurContext->FrontDiffuse[1] = params[1];
      pglCurContext->FrontDiffuse[2] = params[2];
      pglCurContext->FrontDiffuse[3] = params[3];
      break;
    case GL_AMBIENT_AND_DIFFUSE:
      pglCurContext->FrontAmbient[0] = params[0];
      pglCurContext->FrontAmbient[1] = params[1];
      pglCurContext->FrontAmbient[2] = params[2];
      
      pglCurContext->FrontDiffuse[0] = params[0];
      pglCurContext->FrontDiffuse[1] = params[1];
      pglCurContext->FrontDiffuse[2] = params[2];
      pglCurContext->FrontDiffuse[3] = params[3];
      break;
    case GL_SPECULAR:
      pglCurContext->FrontSpecular[0] = params[0];
      pglCurContext->FrontSpecular[1] = params[1];
      pglCurContext->FrontSpecular[2] = params[2];
      break;
    case GL_EMISSION:
      pglCurContext->FrontEmission[0] = params[0];
      pglCurContext->FrontEmission[1] = params[1];
      pglCurContext->FrontEmission[2] = params[2];
      break;
    default:
      GLINT_ERROR(GL_INVALID_ENUM);
      return;
      break;
    }
  }
  
  if((face == GL_BACK) || (face == GL_FRONT_AND_BACK)) {
    switch(pname) {
    case GL_SHININESS:
      pglCurContext->BackShininess = params[0];
      break;
    case GL_AMBIENT:
      pglCurContext->BackAmbient[0] = params[0];
      pglCurContext->BackAmbient[1] = params[1];
      pglCurContext->BackAmbient[2] = params[2];
      break;
    case GL_DIFFUSE:
      pglCurContext->BackDiffuse[0] = params[0];
      pglCurContext->BackDiffuse[1] = params[1];
      pglCurContext->BackDiffuse[2] = params[2];
      pglCurContext->BackDiffuse[3] = params[3];
      break;
    case GL_AMBIENT_AND_DIFFUSE:
      pglCurContext->BackAmbient[0] = params[0];
      pglCurContext->BackAmbient[1] = params[1];
      pglCurContext->BackAmbient[2] = params[2];
      
      pglCurContext->BackDiffuse[0] = params[0];
      pglCurContext->BackDiffuse[1] = params[1];
      pglCurContext->BackDiffuse[2] = params[2];
      pglCurContext->BackDiffuse[3] = params[3];
      break;
    case GL_SPECULAR:
      pglCurContext->BackSpecular[0] = params[0];
      pglCurContext->BackSpecular[1] = params[1];
      pglCurContext->BackSpecular[2] = params[2];
      break;
    case GL_EMISSION:
      pglCurContext->BackEmission[0] = params[0];
      pglCurContext->BackEmission[1] = params[1];
      pglCurContext->BackEmission[2] = params[2];
      break;
    default:
      GLINT_ERROR(GL_INVALID_ENUM);
      return;
      break;
    }
  }
  pglCurContext->LightDirty = TRUE;
}

void APIENTRY glMateriali (GLenum face, GLenum pname, GLint param)
{
  GLINT_OUTSIDE_BEGIN();

  if(pglCurContext->Listing) {
    
    if((face == GL_FRONT) || (face == GL_FRONT_AND_BACK)) {
      switch(pname) {
      case GL_SHININESS:
	glIntIntToList(OP_FRONTSHININESS);
	glIntFloatToList((float)param);
	break;
      default:
	GLINT_ERROR(GL_INVALID_ENUM);
	return;
	break;
      }
    }
      
    if((face == GL_BACK) || (face == GL_FRONT_AND_BACK)) {
      switch(pname) {
      case GL_SHININESS:
	glIntIntToList(OP_BACKSHININESS);
	glIntFloatToList((float)param);
	break;
      default:
	GLINT_ERROR(GL_INVALID_ENUM);
	return;
	break;
      }
    }

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }
  
  if((face == GL_FRONT) || (face == GL_FRONT_AND_BACK)) {
    switch(pname) {
    case GL_SHININESS:
      pglCurContext->FrontShininess = (float)param;
      break;
    default:
      GLINT_ERROR(GL_INVALID_ENUM);
      return;
      break;
    }
  }
  
  if((face == GL_BACK) || (face == GL_FRONT_AND_BACK)) {
    switch(pname) {
    case GL_SHININESS:
      pglCurContext->BackShininess = (float)param;
      break;
    default:
      GLINT_ERROR(GL_INVALID_ENUM);
      return;
      break;
    }
  }
  pglCurContext->LightDirty = TRUE;
}

void APIENTRY glMaterialiv (GLenum face, GLenum pname, const GLint *params)
{
  GLINT_OUTSIDE_BEGIN();

  if(pglCurContext->Listing) {
    
    if((face == GL_FRONT) || (face == GL_FRONT_AND_BACK)) {
      switch(pname) {
      case GL_SHININESS:
	glIntIntToList(OP_FRONTSHININESS);
	glIntFloatToList((float)params[0]);
	break;
      case GL_AMBIENT:
	glIntIntToList(OP_FRONTAMBIENT);
	glIntFloatToList(LighttoFloat(params[0]));
	glIntFloatToList(LighttoFloat(params[1]));
	glIntFloatToList(LighttoFloat(params[2]));
	break;
      case GL_DIFFUSE:
	glIntIntToList(OP_FRONTDIFFUSE);
	glIntFloatToList(LighttoFloat(params[0]));
	glIntFloatToList(LighttoFloat(params[1]));
	glIntFloatToList(LighttoFloat(params[2]));
	glIntFloatToList(LighttoFloat(params[3]));
	break;
      case GL_AMBIENT_AND_DIFFUSE:
	glIntIntToList(OP_FRONTAMBIENT);
	glIntFloatToList(LighttoFloat(params[0]));
	glIntFloatToList(LighttoFloat(params[1]));
	glIntFloatToList(LighttoFloat(params[2]));
	
	glIntIntToList(OP_FRONTDIFFUSE);
	glIntFloatToList(LighttoFloat(params[0]));
	glIntFloatToList(LighttoFloat(params[1]));
	glIntFloatToList(LighttoFloat(params[2]));
	glIntFloatToList(LighttoFloat(params[3]));
	break;
      case GL_SPECULAR:
	glIntIntToList(OP_FRONTSPECULAR);
	glIntFloatToList(LighttoFloat(params[0]));
	glIntFloatToList(LighttoFloat(params[1]));
	glIntFloatToList(LighttoFloat(params[2]));
	break;
      case GL_EMISSION:
	glIntIntToList(OP_FRONTEMISSION);
	glIntFloatToList(LighttoFloat(params[0]));
	glIntFloatToList(LighttoFloat(params[1]));
	glIntFloatToList(LighttoFloat(params[2]));
	break;
      default:
	GLINT_ERROR(GL_INVALID_ENUM);
	return;
	break;
      }
    }
  
    if((face == GL_BACK) || (face == GL_FRONT_AND_BACK)) {
      switch(pname) {
      case GL_SHININESS:
	glIntIntToList(OP_BACKSHININESS);
	glIntFloatToList((float)params[0]);
	break;
      case GL_AMBIENT:
	glIntIntToList(OP_BACKAMBIENT);
	glIntFloatToList(LighttoFloat(params[0]));
	glIntFloatToList(LighttoFloat(params[1]));
	glIntFloatToList(LighttoFloat(params[2]));
	break;
      case GL_DIFFUSE:
	glIntIntToList(OP_BACKDIFFUSE);
	glIntFloatToList(LighttoFloat(params[0]));
	glIntFloatToList(LighttoFloat(params[1]));
	glIntFloatToList(LighttoFloat(params[2]));
	glIntFloatToList(LighttoFloat(params[3]));
	break;
      case GL_AMBIENT_AND_DIFFUSE:
	glIntIntToList(OP_BACKAMBIENT);
	glIntFloatToList(LighttoFloat(params[0]));
	glIntFloatToList(LighttoFloat(params[1]));
	glIntFloatToList(LighttoFloat(params[2]));
	
	glIntIntToList(OP_BACKDIFFUSE);
	glIntFloatToList(LighttoFloat(params[0]));
	glIntFloatToList(LighttoFloat(params[1]));
	glIntFloatToList(LighttoFloat(params[2]));
	glIntFloatToList(LighttoFloat(params[3]));
	break;
      case GL_SPECULAR:
	glIntIntToList(OP_BACKSPECULAR);
	glIntFloatToList(LighttoFloat(params[0]));
	glIntFloatToList(LighttoFloat(params[1]));
	glIntFloatToList(LighttoFloat(params[2]));
	break;
      case GL_EMISSION:
	glIntIntToList(OP_BACKEMISSION);
	glIntFloatToList(LighttoFloat(params[0]));
	glIntFloatToList(LighttoFloat(params[1]));
	glIntFloatToList(LighttoFloat(params[2]));
	break;
      default:
	GLINT_ERROR(GL_INVALID_ENUM);
	return;
	break;
      }
    }
    
    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  if((face == GL_FRONT) || (face == GL_FRONT_AND_BACK)) {
    switch(pname) {
    case GL_SHININESS:
      pglCurContext->FrontShininess = (float)params[0];
      break;
    case GL_AMBIENT:
      pglCurContext->FrontAmbient[0] = LighttoFloat(params[0]);
      pglCurContext->FrontAmbient[1] = LighttoFloat(params[1]);
      pglCurContext->FrontAmbient[2] = LighttoFloat(params[2]);
      break;
    case GL_DIFFUSE:
      pglCurContext->FrontDiffuse[0] = LighttoFloat(params[0]);
      pglCurContext->FrontDiffuse[1] = LighttoFloat(params[1]);
      pglCurContext->FrontDiffuse[2] = LighttoFloat(params[2]);
      pglCurContext->FrontDiffuse[3] = LighttoFloat(params[3]);
      break;
    case GL_AMBIENT_AND_DIFFUSE:
      pglCurContext->FrontAmbient[0] = LighttoFloat(params[0]);
      pglCurContext->FrontAmbient[1] = LighttoFloat(params[1]);
      pglCurContext->FrontAmbient[2] = LighttoFloat(params[2]);
      
      pglCurContext->FrontDiffuse[0] = LighttoFloat(params[0]);
      pglCurContext->FrontDiffuse[1] = LighttoFloat(params[1]);
      pglCurContext->FrontDiffuse[2] = LighttoFloat(params[2]);
      pglCurContext->FrontDiffuse[3] = LighttoFloat(params[3]);
      break;
    case GL_SPECULAR:
      pglCurContext->FrontSpecular[0] = LighttoFloat(params[0]);
      pglCurContext->FrontSpecular[1] = LighttoFloat(params[1]);
      pglCurContext->FrontSpecular[2] = LighttoFloat(params[2]);
      break;
    case GL_EMISSION:
      pglCurContext->FrontEmission[0] = LighttoFloat(params[0]);
      pglCurContext->FrontEmission[1] = LighttoFloat(params[1]);
      pglCurContext->FrontEmission[2] = LighttoFloat(params[2]);
      break;
    default:
      GLINT_ERROR(GL_INVALID_ENUM);
      return;
      break;
    }
  }
  
  if((face == GL_BACK) || (face == GL_FRONT_AND_BACK)) {
    switch(pname) {
    case GL_SHININESS:
      pglCurContext->BackShininess = (float)params[0];
      break;
    case GL_AMBIENT:
      pglCurContext->BackAmbient[0] = LighttoFloat(params[0]);
      pglCurContext->BackAmbient[1] = LighttoFloat(params[1]);
      pglCurContext->BackAmbient[2] = LighttoFloat(params[2]);
      break;
    case GL_DIFFUSE:
      pglCurContext->BackDiffuse[0] = LighttoFloat(params[0]);
      pglCurContext->BackDiffuse[1] = LighttoFloat(params[1]);
      pglCurContext->BackDiffuse[2] = LighttoFloat(params[2]);
      pglCurContext->BackDiffuse[3] = LighttoFloat(params[3]);
      break;
    case GL_AMBIENT_AND_DIFFUSE:
      pglCurContext->BackAmbient[0] = LighttoFloat(params[0]);
      pglCurContext->BackAmbient[1] = LighttoFloat(params[1]);
      pglCurContext->BackAmbient[2] = LighttoFloat(params[2]);
      
      pglCurContext->BackDiffuse[0] = LighttoFloat(params[0]);
      pglCurContext->BackDiffuse[1] = LighttoFloat(params[1]);
      pglCurContext->BackDiffuse[2] = LighttoFloat(params[2]);
      pglCurContext->BackDiffuse[3] = LighttoFloat(params[3]);
      break;
    case GL_SPECULAR:
      pglCurContext->BackSpecular[0] = LighttoFloat(params[0]);
      pglCurContext->BackSpecular[1] = LighttoFloat(params[1]);
      pglCurContext->BackSpecular[2] = LighttoFloat(params[2]);
      break;
    case GL_EMISSION:
      pglCurContext->BackEmission[0] = LighttoFloat(params[0]);
      pglCurContext->BackEmission[1] = LighttoFloat(params[1]);
      pglCurContext->BackEmission[2] = LighttoFloat(params[2]);
      break;
    default:
      GLINT_ERROR(GL_INVALID_ENUM);
      return;
      break;
    }
  }
  pglCurContext->LightDirty = TRUE;
}

void APIENTRY glColorMaterial (GLenum face, GLenum mode)
{
  GLINT_OUTSIDE_BEGIN();

  switch(face) {
  case GL_FRONT:
    break;
  case GL_BACK:
    break;
  case GL_FRONT_AND_BACK:
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }

  switch(mode) {
  case GL_EMISSION:
    break;
  case GL_AMBIENT:
    break;
  case GL_DIFFUSE:
    break;
  case GL_AMBIENT_AND_DIFFUSE:
    break;
  case GL_SPECULAR:
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }

  if(pglCurContext->Listing) {
    
    glIntIntToList(OP_COLORMATERIAL);
    glIntIntToList(face);
    glIntIntToList(mode);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->ColorMaterialFace = face;
  pglCurContext->ColorMaterialMode = mode;
  pglCurContext->LightDirty = TRUE;
}

void APIENTRY glGetMaterialfv (GLenum face, GLenum pname, GLfloat *params)
{
  glIntGetMaterial(face, pname, (void *)params, READ_FLOAT);
}

void APIENTRY glGetMaterialiv (GLenum face, GLenum pname, GLint *params)
{
  glIntGetMaterial(face, pname, (void *)params, READ_INT);
}

void glIntNormalize(GLfloat *norm)
{
    GLdouble mag;

    mag = norm[0]*norm[0] + norm[1]*norm[1] + norm[2]*norm[2];
    if (mag <= 0.0) {
	norm[0] = 0.0f;
	norm[1] = 0.0f;
	norm[2] = 0.0f;
	return;
    }
    if (mag == 1.0) {
	return;
    }
    mag = 1.0/ sqrt(mag);
    norm[0] *= (float)mag;
    norm[1] *= (float)mag;
    norm[2] *= (float)mag;
}

void glIntInitLighting(pglContext Context)
{
  int i;

  Context->ColorMaterialMode = GL_AMBIENT_AND_DIFFUSE;
  Context->ColorMaterialFace = GL_FRONT_AND_BACK;

  Context->LightModelAmbient[0] = 0.2f;
  Context->LightModelAmbient[1] = 0.2f;
  Context->LightModelAmbient[2] = 0.2f;

  Context->FrontAmbient[0] = 0.2f;
  Context->FrontAmbient[1] = 0.2f;
  Context->FrontAmbient[2] = 0.2f;

  Context->FrontDiffuse[0] = 0.8f;
  Context->FrontDiffuse[1] = 0.8f;
  Context->FrontDiffuse[2] = 0.8f;
  Context->FrontDiffuse[3] = 1.0f;

  Context->FrontSpecular[0] = 0.0f;
  Context->FrontSpecular[1] = 0.0f;
  Context->FrontSpecular[2] = 0.0f;

  Context->FrontEmission[0] = 0.0f;
  Context->FrontEmission[1] = 0.0f;
  Context->FrontEmission[2] = 0.0f;

  Context->FrontShininess = 0.0f;

  Context->BackAmbient[0] = 0.2f;
  Context->BackAmbient[1] = 0.2f;
  Context->BackAmbient[2] = 0.2f;

  Context->BackDiffuse[0] = 0.8f;
  Context->BackDiffuse[1] = 0.8f;
  Context->BackDiffuse[2] = 0.8f;
  Context->BackDiffuse[3] = 1.0f;

  Context->BackSpecular[0] = 0.0f;
  Context->BackSpecular[1] = 0.0f;
  Context->BackSpecular[2] = 0.0f;

  Context->BackEmission[0] = 0.0f;
  Context->BackEmission[1] = 0.0f;
  Context->BackEmission[2] = 0.0f;

  Context->BackShininess = 0.0f;

  for(i=0;i<8;i++) {
    Context->Light[i].Ambient[0] = 0.0f;
    Context->Light[i].Ambient[1] = 0.0f;
    Context->Light[i].Ambient[2] = 0.0f;

    if(i==0) {
      Context->Light[i].Diffuse[0] = 1.0f;
      Context->Light[i].Diffuse[1] = 1.0f;
      Context->Light[i].Diffuse[2] = 1.0f;
      Context->Light[i].Diffuse[3] = 1.0f;
      
      Context->Light[i].Specular[0] = 1.0f;
      Context->Light[i].Specular[1] = 1.0f;
      Context->Light[i].Specular[2] = 1.0f;
    } else {
      Context->Light[i].Diffuse[0] = 0.0f;
      Context->Light[i].Diffuse[1] = 0.0f;
      Context->Light[i].Diffuse[2] = 0.0f;
      Context->Light[i].Diffuse[3] = 1.0f;
      
      Context->Light[i].Specular[0] = 0.0f;
      Context->Light[i].Specular[1] = 0.0f;
      Context->Light[i].Specular[2] = 0.0f;
    }
    
    Context->Light[i].Position[0] = 0.0f;
    Context->Light[i].Position[1] = 0.0f;
    Context->Light[i].Position[2] = 1.0f;
    Context->Light[i].Position[3] = 0.0f;

    Context->Light[i].SpotDirection[0] = 0.0f;
    Context->Light[i].SpotDirection[1] = 0.0f;
    Context->Light[i].SpotDirection[2] = -1.0f;

    Context->Light[i].SpotExponent = 0.0f;
    Context->Light[i].SpotCutoff = 180.0f;
    Context->Light[i].ConstantAtten = 1.0f;
    Context->Light[i].LinearAtten = 0.0f;
    Context->Light[i].QuadraticAtten = 0.0f;
  }
}

void glIntLight(glLocalVertex *Vtx)
{
  GLfloat LightColor[4];
  GLfloat PerLightColor[4];
  GLfloat atten,spotspread,diffusedot,speculardot;
  GLfloat lightvec[3],halfvec[3];
//  int i;
  int light;
  
  if(!Vtx->Lighted) {
      // flip normal if needed
      if(pglCurContext->Two_Sided) {
	if(!pglCurContext->CurFrontFace) {
	  Vtx->TxNormal[0] = -Vtx->TxNormal[0];
	  Vtx->TxNormal[1] = -Vtx->TxNormal[1];
	  Vtx->TxNormal[2] = -Vtx->TxNormal[2];
	}
      } 
      
      // Calculate Emission
      if(pglCurContext->CurFrontFace) {
	switch(pglCurContext->EmissionTypeFront) {
	case LI_FRONT_COLOR:
	  memcpy(LightColor,pglCurContext->FrontEmission,4*4);
	  break;
	case LI_BACK_COLOR:
	  memcpy(LightColor,pglCurContext->BackEmission,4*4);
	  break;
	case LI_VERTEX_COLOR:
	  memcpy(LightColor,Vtx->Color,4*4);
	  break;
	}
      } else {
	switch(pglCurContext->EmissionTypeBack) {
	case LI_FRONT_COLOR:
	  memcpy(LightColor,pglCurContext->FrontEmission,4*4);
	  break;
	case LI_BACK_COLOR:
	  memcpy(LightColor,pglCurContext->BackEmission,4*4);
	  break;
	case LI_VERTEX_COLOR:
	  memcpy(LightColor,Vtx->Color,4*4);
	  break;
	}
      }

      // Calculate Ambient
      if(pglCurContext->Two_Sided) {
	if(pglCurContext->CurFrontFace) {
//	  for(i=0;i<3;i++)
//	    LightColor[i] += pglCurContext->FrontAmbient[i] * 
//	      pglCurContext->LightModelAmbient[i];
	    LightColor[0] += pglCurContext->FrontAmbCache[0];
	    LightColor[1] += pglCurContext->FrontAmbCache[1];
	    LightColor[2] += pglCurContext->FrontAmbCache[2];
	} else {
//	  for(i=0;i<3;i++)
//	    LightColor[i] += pglCurContext->BackAmbient[i] * 
//	      pglCurContext->LightModelAmbient[i];
	    LightColor[0] += pglCurContext->BackAmbCache[0];
	    LightColor[1] += pglCurContext->BackAmbCache[1];
	    LightColor[2] += pglCurContext->BackAmbCache[2];
	}
      } else {
//	for(i=0;i<3;i++)
//	  LightColor[i] += pglCurContext->FrontAmbient[i] * 
//	    pglCurContext->LightModelAmbient[i];
	    LightColor[0] += pglCurContext->FrontAmbCache[0];
	    LightColor[1] += pglCurContext->FrontAmbCache[1];
	    LightColor[2] += pglCurContext->FrontAmbCache[2];
      }

      // Calculate Alpha
      if(pglCurContext->CurFrontFace) {
	switch(pglCurContext->DiffuseTypeFront) {
	case LI_FRONT_COLOR:
	  LightColor[3] = pglCurContext->FrontDiffuse[3];
	  break;
	case LI_BACK_COLOR:
	  LightColor[3] = pglCurContext->BackDiffuse[3];
	  break;
	case LI_VERTEX_COLOR:
	  LightColor[3] = Vtx->Color[3];
	  break;
	}
      } else {
	switch(pglCurContext->DiffuseTypeBack) {
	case LI_FRONT_COLOR:
	  LightColor[3] = pglCurContext->FrontDiffuse[3];
	  break;
	case LI_BACK_COLOR:
	  LightColor[3] = pglCurContext->BackDiffuse[3];
	  break;
	case LI_VERTEX_COLOR:
	  LightColor[3] = Vtx->Color[3];
	  break;
	}
      }
      // do lights
      for(light=0; light<8; light++) {
	if(pglCurContext->LightEna[light]) {
	  glLight *CurLight;
	  
	  CurLight = &(pglCurContext->Light[light]);
	  
//	  // Attenuation
//	  if(CurLight->Position[3] != 0.0f) {
//	    if((CurLight->ConstantAtten == 1.0f) &&
//	       (CurLight->LinearAtten == 0.0f) &&
//	       (CurLight->QuadraticAtten == 0.0f)) {
//	      atten = 1.0f;
//	    } else {
//	      // calculate attenuation
//	      GLfloat xdist,ydist,zdist,distsq;
//	      
//	      xdist = CurLight->Position[0]-
//		Vtx->EyePos[0];
//	      ydist = CurLight->Position[1]-
//		Vtx->EyePos[1];
//	      zdist = CurLight->Position[2]-
//		Vtx->EyePos[2];
//	      
//	      distsq = (xdist*xdist)+(ydist*ydist)+(zdist*zdist);
//	      
//	      atten = 1.0f/(CurLight->ConstantAtten+
//			    (((float)sqrt(distsq))*
//			     CurLight->LinearAtten)+
//			    (distsq*
//			     CurLight->QuadraticAtten));
//	    }
//	  } else {
//	    atten = 1.0f;
//	  }

	  // Attenuation
	  if(CurLight->NoAtten) {
	    atten = 1.0f;
	  } else {
	      // calculate attenuation
	      GLfloat xdist,ydist,zdist,distsq;
	      
	      xdist = CurLight->Position[0]-
		Vtx->EyePos[0];
	      ydist = CurLight->Position[1]-
		Vtx->EyePos[1];
	      zdist = CurLight->Position[2]-
		Vtx->EyePos[2];
	      
	      distsq = (xdist*xdist)+(ydist*ydist)+(zdist*zdist);
	      
	      atten = 1.0f/(CurLight->ConstantAtten+
			    (((float)sqrt(distsq))*
			     CurLight->LinearAtten)+
			    (distsq*
			     CurLight->QuadraticAtten));
	  }

	  // Light Vector
	  // light vec is vector from vertex to light position
	  if(CurLight->Position[3] == 0.0f) {
//	    lightvec[0] = CurLight->Position[0];
//	    lightvec[1] = CurLight->Position[1];
//	    lightvec[2] = CurLight->Position[2];
	    memcpy(lightvec,CurLight->Position,3*4);
	  } else {
	    lightvec[0] = CurLight->Position[0]-
	      Vtx->EyePos[0];
	    lightvec[1] = CurLight->Position[1]-
	      Vtx->EyePos[1];
	    lightvec[2] = CurLight->Position[2]-
	      Vtx->EyePos[2];
	  }
	  glIntNormalize(lightvec);

//	  // Spotlight Angle
//	  if(CurLight->SpotCutoff == 180.0f) {
//	    spotspread = 1.0f;
//	  } else {
//	    // check spotlight cone
//	    GLfloat cosine, angle;
//	    
//	    cosine = (float)cos(CurLight->SpotCutoff*
//				glDegtoRad);
//	    
//	    angle = (CurLight->SpotDirection[0]*
//		     -lightvec[0])+
//		       (CurLight->SpotDirection[1]*
//			-lightvec[1])+
//			  (CurLight->SpotDirection[2]*
//			   -lightvec[2]);
//	    if(angle < 0.0f)
//	      angle = 0.0f;
//	    
//	    if(angle < cosine) {
//	      spotspread = 0.0f;
//	    } else {
//	      spotspread = (float)pow((double)angle,
//				      (double)pglCurContext->
//				      Light[light].SpotExponent);
//	    }
//	  }

	  // Spotlight Angle
	  if(CurLight->NoSpot) {
	    spotspread = 1.0f;
	  } else {
	    // check spotlight cone
	    GLfloat angle;
	    
	    angle = (CurLight->SpotDirection[0]*
		     -lightvec[0])+
		       (CurLight->SpotDirection[1]*
			-lightvec[1])+
			  (CurLight->SpotDirection[2]*
			   -lightvec[2]);
	    if(angle > 0.0f) {
	      if(angle < CurLight->Cosine) {
		spotspread = 0.0f;
	      } else {
		spotspread = (float)pow((double)angle,
					(double)pglCurContext->
					Light[light].SpotExponent);
	      }
	    } else {
		spotspread = 0.0f;
	    }
	  }

	  // Per Light Ambient
	  if(pglCurContext->CurFrontFace) {
	    switch(pglCurContext->AmbientTypeFront) {
	    case LI_FRONT_COLOR:
//	      for(i=0;i<3;i++)
//		PerLightColor[i] = pglCurContext->FrontAmbient[i] *
//		    CurLight->Ambient[i];
              memcpy(PerLightColor,
		     CurLight->FrontAmbient,3*4);
	      break;
	    case LI_BACK_COLOR:
//	      for(i=0;i<3;i++)
//		PerLightColor[i] = pglCurContext->BackAmbient[i] *
//		    CurLight->Ambient[i];
              memcpy(PerLightColor,
		     CurLight->BackAmbient,3*4);
	      break;
	    case LI_VERTEX_COLOR:
//	      for(i=0;i<3;i++)
		PerLightColor[0] = Vtx->Color[0] *
		    CurLight->Ambient[0];
		PerLightColor[1] = Vtx->Color[1] *
		    CurLight->Ambient[1];
		PerLightColor[2] = Vtx->Color[2] *
		    CurLight->Ambient[2];
	      break;
	    }
	  } else {
	    switch(pglCurContext->AmbientTypeBack) {
	    case LI_FRONT_COLOR:
//	      for(i=0;i<3;i++)
//		PerLightColor[i] = pglCurContext->FrontAmbient[i] *
//		    CurLight->Ambient[i];
              memcpy(PerLightColor,
		     CurLight->FrontAmbient,3*4);
	      break;
	    case LI_BACK_COLOR:
//	      for(i=0;i<3;i++)
//		PerLightColor[i] = pglCurContext->BackAmbient[i] *
//		    CurLight->Ambient[i];
              memcpy(PerLightColor,
		     CurLight->BackAmbient,3*4);
	      break;
	    case LI_VERTEX_COLOR:
//	      for(i=0;i<3;i++)
		PerLightColor[0] = Vtx->Color[0] *
		    CurLight->Ambient[0];
		PerLightColor[1] = Vtx->Color[1] *
		    CurLight->Ambient[1];
		PerLightColor[2] = Vtx->Color[2] *
		    CurLight->Ambient[2];
	      break;
	    }
	  }

	  // Per Light Diffuse
	  diffusedot = ((Vtx->TxNormal[0])*lightvec[0])+
	    ((Vtx->TxNormal[1])*lightvec[1])+
	      ((Vtx->TxNormal[2])*lightvec[2]);

	  if(diffusedot > 0.0f) {
	    if(pglCurContext->CurFrontFace) {
	      switch(pglCurContext->DiffuseTypeFront) {
	      case LI_FRONT_COLOR:
//		for(i=0;i<3;i++)
//		  PerLightColor[i] += diffusedot *
//		    pglCurContext->FrontDiffuse[i] *
//		      CurLight->Diffuse[i];
		  PerLightColor[0] += diffusedot *
		      CurLight->FrontDiffuse[0];
		  PerLightColor[1] += diffusedot *
		      CurLight->FrontDiffuse[1];
		  PerLightColor[2] += diffusedot *
		      CurLight->FrontDiffuse[2];
		break;
	      case LI_BACK_COLOR:
//		for(i=0;i<3;i++)
//		  PerLightColor[i] += diffusedot *
//		    pglCurContext->BackDiffuse[i] *
//		      CurLight->Diffuse[i];
		  PerLightColor[0] += diffusedot *
		      CurLight->BackDiffuse[0];
		  PerLightColor[1] += diffusedot *
		      CurLight->BackDiffuse[1];
		  PerLightColor[2] += diffusedot *
		      CurLight->BackDiffuse[2];
		break;
	      case LI_VERTEX_COLOR:
//		for(i=0;i<3;i++)
		  PerLightColor[0] += diffusedot *
		    Vtx->Color[0] *
		      CurLight->Diffuse[0];
		  PerLightColor[1] += diffusedot *
		    Vtx->Color[1] *
		      CurLight->Diffuse[1];
		  PerLightColor[2] += diffusedot *
		    Vtx->Color[2] *
		      CurLight->Diffuse[2];
		break;
	      }
	    } else {
	      switch(pglCurContext->DiffuseTypeBack) {
	      case LI_FRONT_COLOR:
//		for(i=0;i<3;i++)
//		  PerLightColor[i] += diffusedot *
//		    pglCurContext->FrontDiffuse[i] *
//		      CurLight->Diffuse[i];
		  PerLightColor[0] += diffusedot *
		      CurLight->FrontDiffuse[0];
		  PerLightColor[1] += diffusedot *
		      CurLight->FrontDiffuse[1];
		  PerLightColor[2] += diffusedot *
		      CurLight->FrontDiffuse[2];
		break;
	      case LI_BACK_COLOR:
//		for(i=0;i<3;i++)
//		  PerLightColor[i] += diffusedot *
//		    pglCurContext->BackDiffuse[i] *
//		      CurLight->Diffuse[i];
		  PerLightColor[0] += diffusedot *
		      CurLight->BackDiffuse[0];
		  PerLightColor[1] += diffusedot *
		      CurLight->BackDiffuse[1];
		  PerLightColor[2] += diffusedot *
		      CurLight->BackDiffuse[2];
		break;
	      case LI_VERTEX_COLOR:
//		for(i=0;i<3;i++)
		  PerLightColor[0] += diffusedot *
		    Vtx->Color[0] *
		      CurLight->Diffuse[0];
		  PerLightColor[1] += diffusedot *
		    Vtx->Color[1] *
		      CurLight->Diffuse[1];
		  PerLightColor[2] += diffusedot *
		    Vtx->Color[2] *
		      CurLight->Diffuse[2];
		break;
	      }
	    }

	    // Per Light Specular
	    if(pglCurContext->Local_Viewer) {
	      GLfloat eyevec[3];
	      eyevec[0] = -Vtx->EyePos[0];
	      eyevec[1] = -Vtx->EyePos[1];
	      eyevec[2] = -Vtx->EyePos[2];
	      glIntNormalize(eyevec);
	      halfvec[0] = lightvec[0]+eyevec[0];
	      halfvec[1] = lightvec[1]+eyevec[1];
	      halfvec[2] = lightvec[2]+eyevec[2];
	    } else {
	      halfvec[0] = lightvec[0];
	      halfvec[1] = lightvec[1];
	      halfvec[2] = lightvec[2]+1.0f;
	    }
	    
	    glIntNormalize(halfvec);
	    
	    speculardot = (Vtx->TxNormal[0]*halfvec[0])+
	      (Vtx->TxNormal[1]*halfvec[1])+
		(Vtx->TxNormal[2]*halfvec[2]);
	    
	    if(speculardot > 0.0f) {
	    
	      if((!pglCurContext->CurFrontFace)&&(pglCurContext->Two_Sided)) {
		speculardot = 
		  (float)pow((double)speculardot,
			     (double)pglCurContext->BackShininess);
	      } else {
		speculardot = 
		  (float)pow((double)speculardot,
			     (double)pglCurContext->FrontShininess);
	      }
	      
	      if(pglCurContext->CurFrontFace) {
		switch(pglCurContext->SpecularTypeFront) {
		case LI_FRONT_COLOR:
//		  for(i=0;i<3;i++)
//		    PerLightColor[i] += speculardot *
//		      pglCurContext->FrontSpecular[i] *
//			CurLight->Specular[i];
		  PerLightColor[0] += speculardot *
		      CurLight->FrontSpecular[0];
		  PerLightColor[1] += speculardot *
		      CurLight->FrontSpecular[1];
		  PerLightColor[2] += speculardot *
		      CurLight->FrontSpecular[2];
		  break;
		case LI_BACK_COLOR:
//		  for(i=0;i<3;i++)
//		    PerLightColor[i] += speculardot *
//		      pglCurContext->BackSpecular[i] *
//			CurLight->Specular[i];
		  PerLightColor[0] += speculardot *
		      CurLight->BackSpecular[0];
		  PerLightColor[1] += speculardot *
		      CurLight->BackSpecular[1];
		  PerLightColor[2] += speculardot *
		      CurLight->BackSpecular[2];
		  break;
		case LI_VERTEX_COLOR:
//		  for(i=0;i<3;i++)
//		    PerLightColor[i] += speculardot *
//		      Vtx->Color[i] *
//			CurLight->Specular[i];
		  PerLightColor[0] += speculardot *
		    Vtx->Color[0] *
		      CurLight->Specular[0];
		  PerLightColor[1] += speculardot *
		    Vtx->Color[1] *
		      CurLight->Specular[1];
		  PerLightColor[2] += speculardot *
		    Vtx->Color[2] *
		      CurLight->Specular[2];
		  break;
		}
	      } else {
		switch(pglCurContext->SpecularTypeBack) {
		case LI_FRONT_COLOR:
//		  for(i=0;i<3;i++)
//		    PerLightColor[i] += speculardot *
//		      pglCurContext->FrontSpecular[i] *
//			CurLight->Specular[i];
		  PerLightColor[0] += speculardot *
		      CurLight->FrontSpecular[0];
		  PerLightColor[1] += speculardot *
		      CurLight->FrontSpecular[1];
		  PerLightColor[2] += speculardot *
		      CurLight->FrontSpecular[2];
		  break;
		case LI_BACK_COLOR:
//		  for(i=0;i<3;i++)
//		    PerLightColor[i] += speculardot *
//		      pglCurContext->BackSpecular[i] *
//			CurLight->Specular[i];
		  PerLightColor[0] += speculardot *
		      CurLight->BackSpecular[0];
		  PerLightColor[1] += speculardot *
		      CurLight->BackSpecular[1];
		  PerLightColor[2] += speculardot *
		      CurLight->BackSpecular[2];
		  break;
		case LI_VERTEX_COLOR:
//		  for(i=0;i<3;i++)
//		    PerLightColor[i] += speculardot *
//		      Vtx->Color[i]*
//			CurLight->Specular[i];
		  PerLightColor[0] += speculardot *
		    Vtx->Color[0] *
		      CurLight->Specular[0];
		  PerLightColor[1] += speculardot *
		    Vtx->Color[1] *
		      CurLight->Specular[1];
		  PerLightColor[2] += speculardot *
		    Vtx->Color[2] *
		      CurLight->Specular[2];
		  break;
		}
	      }
	    }
	  }
	  
	  // Add in Per light Contribution
//	  for(i=0;i<3;i++)
          LightColor[0] += PerLightColor[0]*atten*spotspread;
	  LightColor[1] += PerLightColor[1]*atten*spotspread;
	  LightColor[2] += PerLightColor[2]*atten*spotspread;

	} // end of light enable
      } // end of light loop
	
      // Overwrite vertex color with clamped lighted color
      if(LightColor[0] > 1.0f) {
        Vtx->Glide.r = 255.0f;
      } else {
        Vtx->Glide.r = LightColor[0]*255.0f;
      }
      if(LightColor[1] > 1.0f) {
	Vtx->Glide.g = 255.0f;
      } else {
	Vtx->Glide.g = LightColor[1]*255.0f;
      }
      if(LightColor[2] > 1.0f) {
	Vtx->Glide.b = 255.0f;
      } else {
	Vtx->Glide.b = LightColor[2]*255.0f;
      }
      Vtx->Glide.a = LightColor[3]*255.0f;

      Vtx->Lighted = TRUE;
  }
}

void glIntGetLight(GLenum light, GLenum pname, void *params, int type)
{
  int index;

  GLINT_OUTSIDE_BEGIN();

  switch(light) {
  case GL_LIGHT0:
    index = 0;
    break;
  case GL_LIGHT1:
    index = 1;
    break;
  case GL_LIGHT2:
    index = 2;
    break;
  case GL_LIGHT3:
    index = 3;
    break;
  case GL_LIGHT4:
    index = 4;
    break;
  case GL_LIGHT5:
    index = 5;
    break;
  case GL_LIGHT6:
    index = 6;
    break;
  case GL_LIGHT7:
    index = 7;
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }

  switch(pname) {
  case GL_AMBIENT:
    GLINT_PARAM(&params, pglCurContext->Light[index].Ambient[0], READ_FLOATC);
    GLINT_PARAM(&params, pglCurContext->Light[index].Ambient[1], READ_FLOATC);
    GLINT_PARAM(&params, pglCurContext->Light[index].Ambient[2], READ_FLOATC);
    GLINT_PARAM(&params, pglCurContext->Light[index].Ambient[3], READ_FLOATC);
    break;
  case GL_DIFFUSE:
    GLINT_PARAM(&params, pglCurContext->Light[index].Diffuse[0], READ_FLOATC);
    GLINT_PARAM(&params, pglCurContext->Light[index].Diffuse[1], READ_FLOATC);
    GLINT_PARAM(&params, pglCurContext->Light[index].Diffuse[2], READ_FLOATC);
    GLINT_PARAM(&params, pglCurContext->Light[index].Diffuse[3], READ_FLOATC);
    break;
  case GL_SPECULAR:
    GLINT_PARAM(&params, pglCurContext->Light[index].Specular[0], READ_FLOATC);
    GLINT_PARAM(&params, pglCurContext->Light[index].Specular[1], READ_FLOATC);
    GLINT_PARAM(&params, pglCurContext->Light[index].Specular[2], READ_FLOATC);
    GLINT_PARAM(&params, pglCurContext->Light[index].Specular[3], READ_FLOATC);
    break;
  case GL_POSITION:
    GLINT_PARAM(&params, pglCurContext->Light[index].Position[0], READ_FLOAT);
    GLINT_PARAM(&params, pglCurContext->Light[index].Position[1], READ_FLOAT);
    GLINT_PARAM(&params, pglCurContext->Light[index].Position[2], READ_FLOAT);
    GLINT_PARAM(&params, pglCurContext->Light[index].Position[3], READ_FLOAT);
    break;
  case GL_SPOT_DIRECTION:
    GLINT_PARAM(&params,
		pglCurContext->Light[index].SpotDirection[0], READ_FLOAT);
    GLINT_PARAM(&params,
		pglCurContext->Light[index].SpotDirection[1], READ_FLOAT);
    GLINT_PARAM(&params,
		pglCurContext->Light[index].SpotDirection[2], READ_FLOAT);
    GLINT_PARAM(&params,
		pglCurContext->Light[index].SpotDirection[3], READ_FLOAT);
    break;
  case GL_SPOT_EXPONENT:
    GLINT_PARAM(&params,
		pglCurContext->Light[index].SpotExponent, READ_FLOAT);
    break;
  case GL_SPOT_CUTOFF:
    GLINT_PARAM(&params,
		pglCurContext->Light[index].SpotCutoff, READ_FLOAT);
    break;
  case GL_CONSTANT_ATTENUATION:
    GLINT_PARAM(&params,
		pglCurContext->Light[index].ConstantAtten, READ_FLOAT);
    break;
  case GL_LINEAR_ATTENUATION:
    GLINT_PARAM(&params,
		pglCurContext->Light[index].LinearAtten, READ_FLOAT);
    break;
  case GL_QUADRATIC_ATTENUATION:
    GLINT_PARAM(&params,
		pglCurContext->Light[index].QuadraticAtten, READ_FLOAT);
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }
}

void glIntGetMaterial(GLenum face, GLenum pname, void *params, int type)
{

  switch(face) {
  case GL_FRONT:
    switch(pname) {
    case GL_AMBIENT:
      GLINT_PARAM(&params, pglCurContext->FrontAmbient[0], READ_FLOATC);
      GLINT_PARAM(&params, pglCurContext->FrontAmbient[1], READ_FLOATC);
      GLINT_PARAM(&params, pglCurContext->FrontAmbient[2], READ_FLOATC);
      GLINT_PARAM(&params, pglCurContext->FrontAmbient[3], READ_FLOATC);
      break;
    case GL_DIFFUSE:
      GLINT_PARAM(&params, pglCurContext->FrontDiffuse[0], READ_FLOATC);
      GLINT_PARAM(&params, pglCurContext->FrontDiffuse[1], READ_FLOATC);
      GLINT_PARAM(&params, pglCurContext->FrontDiffuse[2], READ_FLOATC);
      GLINT_PARAM(&params, pglCurContext->FrontDiffuse[3], READ_FLOATC);
      break;
    case GL_SPECULAR:
      GLINT_PARAM(&params, pglCurContext->FrontSpecular[0], READ_FLOATC);
      GLINT_PARAM(&params, pglCurContext->FrontSpecular[1], READ_FLOATC);
      GLINT_PARAM(&params, pglCurContext->FrontSpecular[2], READ_FLOATC);
      GLINT_PARAM(&params, pglCurContext->FrontSpecular[3], READ_FLOATC);
      break;
    case GL_EMISSION:
      GLINT_PARAM(&params, pglCurContext->FrontEmission[0], READ_FLOATC);
      GLINT_PARAM(&params, pglCurContext->FrontEmission[1], READ_FLOATC);
      GLINT_PARAM(&params, pglCurContext->FrontEmission[2], READ_FLOATC);
      GLINT_PARAM(&params, pglCurContext->FrontEmission[3], READ_FLOATC);
      break;
    case GL_SHININESS:
      GLINT_PARAM(&params, pglCurContext->FrontShininess, READ_FLOAT);
      break;
    default:
      GLINT_ERROR(GL_INVALID_ENUM);
      return;
      break;
    }
    break;
  case GL_BACK:
    switch(pname) {
    case GL_AMBIENT:
      GLINT_PARAM(&params, pglCurContext->BackAmbient[0], READ_FLOAT);
      GLINT_PARAM(&params, pglCurContext->BackAmbient[1], READ_FLOAT);
      GLINT_PARAM(&params, pglCurContext->BackAmbient[2], READ_FLOAT);
      GLINT_PARAM(&params, pglCurContext->BackAmbient[3], READ_FLOAT);
      break;
    case GL_DIFFUSE:
      GLINT_PARAM(&params, pglCurContext->BackDiffuse[0], READ_FLOAT);
      GLINT_PARAM(&params, pglCurContext->BackDiffuse[1], READ_FLOAT);
      GLINT_PARAM(&params, pglCurContext->BackDiffuse[2], READ_FLOAT);
      GLINT_PARAM(&params, pglCurContext->BackDiffuse[3], READ_FLOAT);
      break;
    case GL_SPECULAR:
      GLINT_PARAM(&params, pglCurContext->BackSpecular[0], READ_FLOAT);
      GLINT_PARAM(&params, pglCurContext->BackSpecular[1], READ_FLOAT);
      GLINT_PARAM(&params, pglCurContext->BackSpecular[2], READ_FLOAT);
      GLINT_PARAM(&params, pglCurContext->BackSpecular[3], READ_FLOAT);
      break;
    case GL_EMISSION:
      GLINT_PARAM(&params, pglCurContext->BackEmission[0], READ_FLOAT);
      GLINT_PARAM(&params, pglCurContext->BackEmission[1], READ_FLOAT);
      GLINT_PARAM(&params, pglCurContext->BackEmission[2], READ_FLOAT);
      GLINT_PARAM(&params, pglCurContext->BackEmission[3], READ_FLOAT);
      break;
    case GL_SHININESS:
      GLINT_PARAM(&params, pglCurContext->BackShininess, READ_FLOAT);
      break;
    default:
      GLINT_ERROR(GL_INVALID_ENUM);
      return;
      break;
    }
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }
}

void glIntValidateNeedEye(void)
{
  int light;
  // Note: only needed for Attenuation, Positional Lighting,
  //       Local_Viewer and TexGen
  if(pglCurContext->NeedEyeDirty) {
    if(pglCurContext->Lighting) {
      if(pglCurContext->Local_Viewer) {
	pglCurContext->NeedEye = TRUE;
	return;
      } 
      for(light=0;light<8;light++) {
	if(pglCurContext->LightEna[light] &&
	   (pglCurContext->Light[light].Position[3] != 0.0f)) {
	  pglCurContext->NeedEye = TRUE;
	  return;
	}
      }
    }
    
    pglCurContext->NeedEyeDirty = FALSE;
  }
}

void glIntValidateLighting(void)
{
  int light;

  if(pglCurContext->LightDirty) {
    if(pglCurContext->Two_Sided) {
      // Two Sided
      pglCurContext->EmissionTypeFront = LI_FRONT_COLOR;
      pglCurContext->EmissionTypeBack = LI_BACK_COLOR;
      pglCurContext->AmbientTypeFront = LI_FRONT_COLOR;
      pglCurContext->AmbientTypeBack = LI_BACK_COLOR;
      pglCurContext->DiffuseTypeFront = LI_FRONT_COLOR;
      pglCurContext->DiffuseTypeBack = LI_BACK_COLOR;
      pglCurContext->SpecularTypeFront = LI_FRONT_COLOR;
      pglCurContext->SpecularTypeBack = LI_BACK_COLOR;
      if(pglCurContext->ColorMaterial) {
	switch(pglCurContext->ColorMaterialMode) {
	case GL_EMISSION:
	  switch(pglCurContext->ColorMaterialFace) {
	  case GL_FRONT:	
	    pglCurContext->EmissionTypeFront = LI_VERTEX_COLOR;
	    break;
	  case GL_BACK:
	    pglCurContext->EmissionTypeBack = LI_VERTEX_COLOR;
	    break;
	  case GL_FRONT_AND_BACK:
	    pglCurContext->EmissionTypeFront = LI_VERTEX_COLOR;
	    pglCurContext->EmissionTypeBack = LI_VERTEX_COLOR;
	    break;
	  }
	  break;
	case GL_AMBIENT:
	  switch(pglCurContext->ColorMaterialFace) {
	  case GL_FRONT:	
	    pglCurContext->AmbientTypeFront = LI_VERTEX_COLOR;
	    break;
	  case GL_BACK:
	    pglCurContext->AmbientTypeBack = LI_VERTEX_COLOR;
	    break;
	  case GL_FRONT_AND_BACK:
	    pglCurContext->AmbientTypeFront = LI_VERTEX_COLOR;
	    pglCurContext->AmbientTypeBack = LI_VERTEX_COLOR;
	    break;
	  }
	  break;
	case GL_DIFFUSE:
	  switch(pglCurContext->ColorMaterialFace) {
	  case GL_FRONT:	
	    pglCurContext->DiffuseTypeFront = LI_VERTEX_COLOR;
	    break;
	  case GL_BACK:
	    pglCurContext->DiffuseTypeBack = LI_VERTEX_COLOR;
	    break;
	  case GL_FRONT_AND_BACK:
	    pglCurContext->DiffuseTypeFront = LI_VERTEX_COLOR;
	    pglCurContext->DiffuseTypeBack = LI_VERTEX_COLOR;
	    break;
	  }
	  break;
	case GL_AMBIENT_AND_DIFFUSE:
	  switch(pglCurContext->ColorMaterialFace) {
	  case GL_FRONT:	
	    pglCurContext->AmbientTypeFront = LI_VERTEX_COLOR;
	    pglCurContext->DiffuseTypeFront = LI_VERTEX_COLOR;
	    break;
	  case GL_BACK:
	    pglCurContext->AmbientTypeBack = LI_VERTEX_COLOR;
	    pglCurContext->DiffuseTypeBack = LI_VERTEX_COLOR;
	    break;
	  case GL_FRONT_AND_BACK:
	    pglCurContext->AmbientTypeFront = LI_VERTEX_COLOR;
	    pglCurContext->DiffuseTypeFront = LI_VERTEX_COLOR;
	    pglCurContext->AmbientTypeBack = LI_VERTEX_COLOR;
	    pglCurContext->DiffuseTypeBack = LI_VERTEX_COLOR;
	    break;
	  }
	  break;
	case GL_SPECULAR:
	  switch(pglCurContext->ColorMaterialFace) {
	  case GL_FRONT:	
	    pglCurContext->SpecularTypeFront = LI_VERTEX_COLOR;
	    break;
	  case GL_BACK:
	    pglCurContext->SpecularTypeBack = LI_VERTEX_COLOR;
	    break;
	  case GL_FRONT_AND_BACK:
	    pglCurContext->SpecularTypeFront = LI_VERTEX_COLOR;
	    pglCurContext->SpecularTypeBack = LI_VERTEX_COLOR;
	    break;
	  }
	  break;
	}
      }
      // Precompute what we can
      pglCurContext->FrontAmbCache[0] = pglCurContext->FrontAmbient[0] * 
	      pglCurContext->LightModelAmbient[0];
      pglCurContext->FrontAmbCache[1] = pglCurContext->FrontAmbient[1] * 
	      pglCurContext->LightModelAmbient[1];
      pglCurContext->FrontAmbCache[2] = pglCurContext->FrontAmbient[2] * 
	      pglCurContext->LightModelAmbient[2];

      pglCurContext->BackAmbCache[0] = pglCurContext->BackAmbient[0] * 
	      pglCurContext->LightModelAmbient[0];
      pglCurContext->BackAmbCache[1] = pglCurContext->BackAmbient[1] * 
	      pglCurContext->LightModelAmbient[1];
      pglCurContext->BackAmbCache[2] = pglCurContext->BackAmbient[2] * 
	      pglCurContext->LightModelAmbient[2];

      for(light=0; light<8; light++) {
	if(pglCurContext->LightEna[light]) {
	  glLight *CurLight;
	  
	  CurLight = &(pglCurContext->Light[light]);
	  
          // Precompute per light ambient
	  CurLight->FrontAmbient[0] = 
	    pglCurContext->FrontAmbient[0] *
	      CurLight->Ambient[0];
	  CurLight->FrontAmbient[1] = 
	    pglCurContext->FrontAmbient[1] *
	      CurLight->Ambient[1];
	  CurLight->FrontAmbient[2] = 
	    pglCurContext->FrontAmbient[2] *
	      CurLight->Ambient[2];

	  CurLight->BackAmbient[0] = 
	    pglCurContext->BackAmbient[0] *
	      CurLight->Ambient[0];
	  CurLight->BackAmbient[1] = 
	    pglCurContext->BackAmbient[1] *
	      CurLight->Ambient[1];
	  CurLight->BackAmbient[2] = 
	    pglCurContext->BackAmbient[2] *
	      CurLight->Ambient[2];

          // Precompute per light diffuse
	  CurLight->FrontDiffuse[0] = 
	    pglCurContext->FrontDiffuse[0] *
	      CurLight->Diffuse[0];
	  CurLight->FrontDiffuse[1] = 
	    pglCurContext->FrontDiffuse[1] *
	      CurLight->Diffuse[1];
	  CurLight->FrontDiffuse[2] = 
	    pglCurContext->FrontDiffuse[2] *
	      CurLight->Diffuse[2];

	  CurLight->BackDiffuse[0] = 
	    pglCurContext->BackDiffuse[0] *
	      CurLight->Diffuse[0];
	  CurLight->BackDiffuse[1] = 
	    pglCurContext->BackDiffuse[1] *
	      CurLight->Diffuse[1];
	  CurLight->BackDiffuse[2] = 
	    pglCurContext->BackDiffuse[2] *
	      CurLight->Diffuse[2];

          // Precompute per light specular
	  CurLight->FrontSpecular[0] = 
	    pglCurContext->FrontSpecular[0] *
	      CurLight->Specular[0];
	  CurLight->FrontSpecular[1] = 
	    pglCurContext->FrontSpecular[1] *
	      CurLight->Specular[1];
	  CurLight->FrontSpecular[2] = 
	    pglCurContext->FrontSpecular[2] *
	      CurLight->Specular[2];

          // Precompute per light specular
	  CurLight->BackSpecular[0] = 
	    pglCurContext->BackSpecular[0] *
	      CurLight->Specular[0];
	  CurLight->BackSpecular[1] = 
	    pglCurContext->BackSpecular[1] *
	      CurLight->Specular[1];
	  CurLight->BackSpecular[2] = 
	    pglCurContext->BackSpecular[2] *
	      CurLight->Specular[2];

	  if(CurLight->Position[3] != 0.0f) {
	    if((CurLight->ConstantAtten == 1.0f) &&
	       (CurLight->LinearAtten == 0.0f) &&
	       (CurLight->QuadraticAtten == 0.0f)) {
	      CurLight->NoAtten = TRUE;
	    } else {
	      CurLight->NoAtten = FALSE;
	    }
	  } else {
	    CurLight->NoAtten = TRUE;
	  }
	}
      }
    } else {
      // One Sided
      pglCurContext->EmissionTypeFront = LI_FRONT_COLOR;
      pglCurContext->EmissionTypeBack = LI_FRONT_COLOR;
      pglCurContext->AmbientTypeFront = LI_FRONT_COLOR;
      pglCurContext->AmbientTypeBack = LI_FRONT_COLOR;
      pglCurContext->DiffuseTypeFront = LI_FRONT_COLOR;
      pglCurContext->DiffuseTypeBack = LI_FRONT_COLOR;
      pglCurContext->SpecularTypeFront = LI_FRONT_COLOR;
      pglCurContext->SpecularTypeBack = LI_FRONT_COLOR;
      if(pglCurContext->ColorMaterial) {
	switch(pglCurContext->ColorMaterialMode) {
	case GL_EMISSION:
	  if(pglCurContext->ColorMaterialFace != GL_BACK) {
	    pglCurContext->EmissionTypeFront = LI_VERTEX_COLOR;
	    pglCurContext->EmissionTypeBack = LI_VERTEX_COLOR;
	  }
	  break;
	case GL_AMBIENT:
	  if(pglCurContext->ColorMaterialFace != GL_BACK) {
	    pglCurContext->AmbientTypeFront = LI_VERTEX_COLOR;
	    pglCurContext->AmbientTypeBack = LI_VERTEX_COLOR;
	  }
	  break;
	case GL_DIFFUSE:
	  if(pglCurContext->ColorMaterialFace != GL_BACK) {
	    pglCurContext->DiffuseTypeFront = LI_VERTEX_COLOR;
	    pglCurContext->DiffuseTypeBack = LI_VERTEX_COLOR;
	  }
	  break;
	case GL_AMBIENT_AND_DIFFUSE:
	  if(pglCurContext->ColorMaterialFace != GL_BACK) {
	    pglCurContext->AmbientTypeFront = LI_VERTEX_COLOR;
	    pglCurContext->DiffuseTypeFront = LI_VERTEX_COLOR;
	    pglCurContext->AmbientTypeBack = LI_VERTEX_COLOR;
	    pglCurContext->DiffuseTypeBack = LI_VERTEX_COLOR;
	  }
	  break;
	case GL_SPECULAR:
	  if(pglCurContext->ColorMaterialFace != GL_BACK) {
	    pglCurContext->SpecularTypeFront = LI_VERTEX_COLOR;
	    pglCurContext->SpecularTypeBack = LI_VERTEX_COLOR;
	  }
	  break;
	}
      }
      // Precompute what we can
      pglCurContext->FrontAmbCache[0] = pglCurContext->FrontAmbient[0] * 
	      pglCurContext->LightModelAmbient[0];
      pglCurContext->FrontAmbCache[1] = pglCurContext->FrontAmbient[1] * 
	      pglCurContext->LightModelAmbient[1];
      pglCurContext->FrontAmbCache[2] = pglCurContext->FrontAmbient[2] * 
	      pglCurContext->LightModelAmbient[2];

      for(light=0; light<8; light++) {
	if(pglCurContext->LightEna[light]) {
	  glLight *CurLight;
	  
	  CurLight = &(pglCurContext->Light[light]);
	  
          // Precompute per light ambient
	  CurLight->FrontAmbient[0] = 
	    pglCurContext->FrontAmbient[0] *
	      CurLight->Ambient[0];
	  CurLight->FrontAmbient[1] = 
	    pglCurContext->FrontAmbient[1] *
	      CurLight->Ambient[1];
	  CurLight->FrontAmbient[2] = 
	    pglCurContext->FrontAmbient[2] *
	      CurLight->Ambient[2];

          // Precompute per light diffuse
	  CurLight->FrontDiffuse[0] = 
	    pglCurContext->FrontDiffuse[0] *
	      CurLight->Diffuse[0];
	  CurLight->FrontDiffuse[1] = 
	    pglCurContext->FrontDiffuse[1] *
	      CurLight->Diffuse[1];
	  CurLight->FrontDiffuse[2] = 
	    pglCurContext->FrontDiffuse[2] *
	      CurLight->Diffuse[2];

          // Precompute per light specular
	  CurLight->FrontSpecular[0] = 
	    pglCurContext->FrontSpecular[0] *
	      CurLight->Specular[0];
	  CurLight->FrontSpecular[1] = 
	    pglCurContext->FrontSpecular[1] *
	      CurLight->Specular[1];
	  CurLight->FrontSpecular[2] = 
	    pglCurContext->FrontSpecular[2] *
	      CurLight->Specular[2];

	  if(CurLight->Position[3] != 0.0f) {
	    if((CurLight->ConstantAtten == 1.0f) &&
	       (CurLight->LinearAtten == 0.0f) &&
	       (CurLight->QuadraticAtten == 0.0f)) {
	      CurLight->NoAtten = TRUE;
	    } else {
	      CurLight->NoAtten = FALSE;
	    }
	  } else {
	    CurLight->NoAtten = TRUE;
	  }

	  if(CurLight->SpotCutoff == 180.0f) {
	    CurLight->NoSpot = TRUE;
	  } else {
	    CurLight->NoSpot = FALSE;
	    CurLight->Cosine = (float)cos(CurLight->SpotCutoff*
					  glDegtoRad);
	  }
	}
      }
    }      

    pglCurContext->LightDirty = FALSE;
  }
}

