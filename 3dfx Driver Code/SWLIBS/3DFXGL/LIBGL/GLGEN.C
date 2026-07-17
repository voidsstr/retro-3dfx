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

/*      GL_INDEX_LOGIC_OP */
/*      GL_COLOR_LOGIC_OP */

/*      GL_DITHER */

/*      GL_TEXTURE_GEN_S */
/*      GL_TEXTURE_GEN_T */
/*      GL_TEXTURE_GEN_R */
/*      GL_TEXTURE_GEN_Q */

/*      GL_MAP1_VERTEX_3 */
/*      GL_MAP1_VERTEX_4 */
/*      GL_MAP1_COLOR_4 */
/*      GL_MAP1_INDEX */
/*      GL_MAP1_NORMAL */
/*      GL_MAP1_TEXTURE_COORD_1 */
/*      GL_MAP1_TEXTURE_COORD_2 */
/*      GL_MAP1_TEXTURE_COORD_3 */
/*      GL_MAP1_TEXTURE_COORD_4 */
/*      GL_MAP2_VERTEX_3 */
/*      GL_MAP2_VERTEX_4 */
/*      GL_MAP2_COLOR_4 */
/*      GL_MAP2_INDEX */
/*      GL_MAP2_NORMAL */
/*      GL_MAP2_TEXTURE_COORD_1 */
/*      GL_MAP2_TEXTURE_COORD_2 */
/*      GL_MAP2_TEXTURE_COORD_3 */
/*      GL_MAP2_TEXTURE_COORD_4 */

void APIENTRY glEnable (GLenum cap)
{
  GLINT_OUTSIDE_BEGIN();

  if(pglCurContext->Listing) {

    switch(cap) {
    case GL_DEPTH_TEST:
    case GL_STENCIL_TEST:
    case GL_NORMALIZE:
    case GL_LIGHTING:
    case GL_LIGHT0:
    case GL_LIGHT1:
    case GL_LIGHT2:
    case GL_LIGHT3:
    case GL_LIGHT4:
    case GL_LIGHT5:
    case GL_LIGHT6:
    case GL_LIGHT7:
    case GL_CLIP_PLANE0:
    case GL_CLIP_PLANE1:
    case GL_CLIP_PLANE2:
    case GL_CLIP_PLANE3:
    case GL_CLIP_PLANE4:
    case GL_CLIP_PLANE5:
    case GL_COLOR_MATERIAL:
    case GL_CULL_FACE:
    case GL_SCISSOR_TEST:
    case GL_POINT_SMOOTH:
    case GL_LINE_SMOOTH:
    case GL_POLYGON_SMOOTH:
    case GL_ALPHA_TEST:
    case GL_BLEND:
    case GL_TEXTURE_1D:
    case GL_TEXTURE_2D:
    case GL_POLYGON_OFFSET_FILL:
    case GL_POLYGON_OFFSET_LINE:
    case GL_POLYGON_OFFSET_POINT:
    case GL_FOG:
    case GL_AUTO_NORMAL:
    case GL_LINE_STIPPLE:
    case GL_POLYGON_STIPPLE:
    case GL_DITHER:
      break;
    default:
      GLINT_ERROR(GL_INVALID_ENUM);
      break;
    }

    glIntIntToList(OP_SETBOOL);
    glIntIntToList(cap);
    glIntIntToList(TRUE);
    
    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  switch(cap) {
  case GL_DEPTH_TEST:
    pglCurContext->DepthBuffer = TRUE;
    break;
  case GL_STENCIL_TEST:
    pglCurContext->StencilTest = TRUE;
    break;
  case GL_NORMALIZE:
    pglCurContext->Normalize = TRUE;
    break;
  case GL_LIGHT0:
    pglCurContext->LightEna[0] = TRUE;
    pglCurContext->NeedEyeDirty = TRUE;
    pglCurContext->LightDirty = TRUE;
    break;
  case GL_LIGHT1:
    pglCurContext->LightEna[1] = TRUE;
    pglCurContext->NeedEyeDirty = TRUE;
    pglCurContext->LightDirty = TRUE;
    break;
  case GL_LIGHT2:
    pglCurContext->LightEna[2] = TRUE;
    pglCurContext->NeedEyeDirty = TRUE;
    pglCurContext->LightDirty = TRUE;
    break;
  case GL_LIGHT3:
    pglCurContext->LightEna[3] = TRUE;
    pglCurContext->NeedEyeDirty = TRUE;
    pglCurContext->LightDirty = TRUE;
    break;
  case GL_LIGHT4:
    pglCurContext->LightEna[4] = TRUE;
    pglCurContext->NeedEyeDirty = TRUE;
    pglCurContext->LightDirty = TRUE;
    break;
  case GL_LIGHT5:
    pglCurContext->LightEna[5] = TRUE;
    pglCurContext->NeedEyeDirty = TRUE;
    pglCurContext->LightDirty = TRUE;
    break;
  case GL_LIGHT6:
    pglCurContext->LightEna[6] = TRUE;
    pglCurContext->NeedEyeDirty = TRUE;
    pglCurContext->LightDirty = TRUE;
    break;
  case GL_LIGHT7:
    pglCurContext->LightEna[7] = TRUE;
    pglCurContext->NeedEyeDirty = TRUE;
    pglCurContext->LightDirty = TRUE;
    break;
  case GL_CLIP_PLANE0:
    pglCurContext->ClipEna[0] = TRUE;
    break;
  case GL_CLIP_PLANE1:
    pglCurContext->ClipEna[1] = TRUE;
    break;
  case GL_CLIP_PLANE2:
    pglCurContext->ClipEna[2] = TRUE;
    break;
  case GL_CLIP_PLANE3:
    pglCurContext->ClipEna[3] = TRUE;
    break;
  case GL_CLIP_PLANE4:
    pglCurContext->ClipEna[4] = TRUE;
    break;
  case GL_CLIP_PLANE5:
    pglCurContext->ClipEna[5] = TRUE;
    break;
  case GL_LIGHTING:
    pglCurContext->Lighting = TRUE;
    pglCurContext->NeedEyeDirty = TRUE;
    break;
  case GL_COLOR_MATERIAL:
    pglCurContext->ColorMaterial = TRUE;
    pglCurContext->LightDirty = TRUE;
    break;
  case GL_CULL_FACE:
    pglCurContext->CullEnable = TRUE;
    break;
  case GL_SCISSOR_TEST:
    pglCurContext->ScissorEnable = TRUE;
    break;
  case GL_POINT_SMOOTH:
    pglCurContext->PointAA = TRUE;
    break;
  case GL_LINE_SMOOTH:
    pglCurContext->LineAA = TRUE;
    break;
  case GL_POLYGON_SMOOTH:
    pglCurContext->PolyAA = TRUE;
    break;
  case GL_ALPHA_TEST:
    pglCurContext->AlphaTest = TRUE;
    break;
  case GL_BLEND:
    pglCurContext->Blend = TRUE;
    break;
  case GL_TEXTURE_1D:
    pglCurContext->Tex1D = TRUE;
    pglCurContext->TexDirty = TRUE;
    break;
  case GL_TEXTURE_2D:
    pglCurContext->Tex2D = TRUE;
    pglCurContext->TexDirty = TRUE;
    break;
  case GL_POLYGON_OFFSET_FILL:
    pglCurContext->PolyOffset = TRUE;
    break;
  case GL_POLYGON_OFFSET_LINE:
    pglCurContext->LineOffset = TRUE;
    break;
  case GL_POLYGON_OFFSET_POINT:
    pglCurContext->PointOffset = TRUE;
    break;
  case GL_FOG:
    pglCurContext->Fog = TRUE;
    break;
  case GL_AUTO_NORMAL:
    pglCurContext->AutoNormal = TRUE;
    break;
  case GL_LINE_STIPPLE:
    pglCurContext->LineStipple = TRUE;
    break;
  case GL_POLYGON_STIPPLE:
    pglCurContext->PolyStipple = TRUE;
    break;
  case GL_DITHER:
    pglCurContext->Dither = TRUE;
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    break;
  }
  pglCurContext->ContextDirty = TRUE;
}

void APIENTRY glDisable (GLenum cap)
{
  GLINT_OUTSIDE_BEGIN();

  if(pglCurContext->Listing) {

    switch(cap) {
    case GL_DEPTH_TEST:
    case GL_STENCIL_TEST:
    case GL_NORMALIZE:
    case GL_LIGHTING:
    case GL_LIGHT0:
    case GL_LIGHT1:
    case GL_LIGHT2:
    case GL_LIGHT3:
    case GL_LIGHT4:
    case GL_LIGHT5:
    case GL_LIGHT6:
    case GL_LIGHT7:
    case GL_CLIP_PLANE0:
    case GL_CLIP_PLANE1:
    case GL_CLIP_PLANE2:
    case GL_CLIP_PLANE3:
    case GL_CLIP_PLANE4:
    case GL_CLIP_PLANE5:
    case GL_COLOR_MATERIAL:
    case GL_CULL_FACE:
    case GL_SCISSOR_TEST:
    case GL_POINT_SMOOTH:
    case GL_LINE_SMOOTH:
    case GL_POLYGON_SMOOTH:
    case GL_ALPHA_TEST:
    case GL_BLEND:
    case GL_TEXTURE_1D:
    case GL_TEXTURE_2D:
    case GL_POLYGON_OFFSET_FILL:
    case GL_POLYGON_OFFSET_LINE:
    case GL_POLYGON_OFFSET_POINT:
    case GL_FOG:
    case GL_AUTO_NORMAL:
    case GL_LINE_STIPPLE:
    case GL_POLYGON_STIPPLE:
      break;
    default:
      GLINT_ERROR(GL_INVALID_ENUM);
      break;
    }

    glIntIntToList(OP_SETBOOL);
    glIntIntToList(cap);
    glIntIntToList(FALSE);
    
    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  switch(cap) {
  case GL_DEPTH_TEST:
    pglCurContext->DepthBuffer = FALSE;
    break;
  case GL_STENCIL_TEST:
    pglCurContext->StencilTest = FALSE;
    break;
  case GL_NORMALIZE:
    pglCurContext->Normalize = FALSE;
    break;
  case GL_LIGHTING:
    pglCurContext->Lighting = FALSE;
    pglCurContext->NeedEyeDirty = TRUE;
    break;
  case GL_LIGHT0:
    pglCurContext->LightEna[0] = FALSE;
    pglCurContext->NeedEyeDirty = TRUE;
    pglCurContext->LightDirty = TRUE;
    break;
  case GL_LIGHT1:
    pglCurContext->LightEna[1] = FALSE;
    pglCurContext->NeedEyeDirty = TRUE;
    pglCurContext->LightDirty = TRUE;
    break;
  case GL_LIGHT2:
    pglCurContext->LightEna[2] = FALSE;
    pglCurContext->NeedEyeDirty = TRUE;
    pglCurContext->LightDirty = TRUE;
    break;
  case GL_LIGHT3:
    pglCurContext->LightEna[3] = FALSE;
    pglCurContext->NeedEyeDirty = TRUE;
    pglCurContext->LightDirty = TRUE;
    break;
  case GL_LIGHT4:
    pglCurContext->LightEna[4] = FALSE;
    pglCurContext->NeedEyeDirty = TRUE;
    pglCurContext->LightDirty = TRUE;
    break;
  case GL_LIGHT5:
    pglCurContext->LightEna[5] = FALSE;
    pglCurContext->NeedEyeDirty = TRUE;
    pglCurContext->LightDirty = TRUE;
    break;
  case GL_LIGHT6:
    pglCurContext->LightEna[6] = FALSE;
    pglCurContext->NeedEyeDirty = TRUE;
    pglCurContext->LightDirty = TRUE;
    break;
  case GL_LIGHT7:
    pglCurContext->LightEna[7] = FALSE;
    pglCurContext->NeedEyeDirty = TRUE;
    pglCurContext->LightDirty = TRUE;
    break;
  case GL_CLIP_PLANE0:
    pglCurContext->ClipEna[0] = FALSE;
    break;
  case GL_CLIP_PLANE1:
    pglCurContext->ClipEna[1] = FALSE;
    break;
  case GL_CLIP_PLANE2:
    pglCurContext->ClipEna[2] = FALSE;
    break;
  case GL_CLIP_PLANE3:
    pglCurContext->ClipEna[3] = FALSE;
    break;
  case GL_CLIP_PLANE4:
    pglCurContext->ClipEna[4] = FALSE;
    break;
  case GL_CLIP_PLANE5:
    pglCurContext->ClipEna[5] = FALSE;
    break;
  case GL_COLOR_MATERIAL:
    pglCurContext->ColorMaterial = FALSE;
    pglCurContext->LightDirty = TRUE;
    break;
  case GL_CULL_FACE:
    pglCurContext->CullEnable = FALSE;
    break;
  case GL_SCISSOR_TEST:
    pglCurContext->ScissorEnable = FALSE;
    break;
  case GL_POINT_SMOOTH:
    pglCurContext->PointAA = FALSE;
    break;
  case GL_LINE_SMOOTH:
    pglCurContext->LineAA = FALSE;
    break;
  case GL_POLYGON_SMOOTH:
    pglCurContext->PolyAA = FALSE;
    break;
  case GL_ALPHA_TEST:
    pglCurContext->AlphaTest = FALSE;
    break;
  case GL_BLEND:
    pglCurContext->Blend = FALSE;
    break;
  case GL_TEXTURE_1D:
    pglCurContext->Tex1D = FALSE;
    pglCurContext->TexDirty = TRUE;
    break;
  case GL_TEXTURE_2D:
    pglCurContext->Tex2D = FALSE;
    pglCurContext->TexDirty = TRUE;
    break;
  case GL_POLYGON_OFFSET_FILL:
    pglCurContext->PolyOffset = FALSE;
    break;
  case GL_POLYGON_OFFSET_LINE:
    pglCurContext->LineOffset = FALSE;
    break;
  case GL_POLYGON_OFFSET_POINT:
    pglCurContext->PointOffset = FALSE;
    break;
  case GL_FOG:
    pglCurContext->Fog = FALSE;
    break;
  case GL_AUTO_NORMAL:
    pglCurContext->AutoNormal = FALSE;
    break;
  case GL_LINE_STIPPLE:
    pglCurContext->LineStipple = FALSE;
    break;
  case GL_POLYGON_STIPPLE:
    pglCurContext->PolyStipple = FALSE;
    break;
  case GL_DITHER:
    pglCurContext->Dither = FALSE;
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    break;
  }
  pglCurContext->ContextDirty = TRUE;
}

GLboolean APIENTRY glIsEnabled (GLenum cap)
{
  GLINT_OUTSIDE_BEGIN();

  switch(cap) {
  case GL_DEPTH_TEST:
    return(pglCurContext->DepthBuffer);
    break;
  case GL_STENCIL_TEST:
    return(pglCurContext->StencilTest);
    break;
  case GL_NORMALIZE:
    return(pglCurContext->Normalize);
    break;
  case GL_LIGHTING:
    return(pglCurContext->Lighting);
    break;
  case GL_LIGHT0:
    return(pglCurContext->LightEna[0]);
    break;
  case GL_LIGHT1:
    return(pglCurContext->LightEna[1]);
    break;
  case GL_LIGHT2:
    return(pglCurContext->LightEna[2]);
    break;
  case GL_LIGHT3:
    return(pglCurContext->LightEna[3]);
    break;
  case GL_LIGHT4:
    return(pglCurContext->LightEna[4]);
    break;
  case GL_LIGHT5:
    return(pglCurContext->LightEna[5]);
    break;
  case GL_LIGHT6:
    return(pglCurContext->LightEna[6]);
    break;
  case GL_LIGHT7:
    return(pglCurContext->LightEna[7]);
    break;
  case GL_CLIP_PLANE0:
    return(pglCurContext->ClipEna[0]);
    break;
  case GL_CLIP_PLANE1:
    return(pglCurContext->ClipEna[1]);
    break;
  case GL_CLIP_PLANE2:
    return(pglCurContext->ClipEna[2]);
    break;
  case GL_CLIP_PLANE3:
    return(pglCurContext->ClipEna[3]);
    break;
  case GL_CLIP_PLANE4:
    return(pglCurContext->ClipEna[4]);
    break;
  case GL_CLIP_PLANE5:
    return(pglCurContext->ClipEna[5]);
    break;
  case GL_COLOR_MATERIAL:
    return(pglCurContext->ColorMaterial);
    break;
  case GL_CULL_FACE:
    return(pglCurContext->CullEnable);
    break;
  case GL_SCISSOR_TEST:
    return(pglCurContext->ScissorEnable);
    break;
  case GL_POINT_SMOOTH:
    return(pglCurContext->PointAA);
    break;
  case GL_LINE_SMOOTH:
    return(pglCurContext->LineAA);
    break;
  case GL_POLYGON_SMOOTH:
    return(pglCurContext->PolyAA);
    break;
  case GL_ALPHA_TEST:
    return(pglCurContext->AlphaTest);
    break;
  case GL_BLEND:
    return(pglCurContext->Blend);
    break;
  case GL_TEXTURE_1D:
    return(pglCurContext->Tex1D);
    break;
  case GL_TEXTURE_2D:
    return(pglCurContext->Tex2D);
    break;
  case GL_POLYGON_OFFSET_FILL:
    return(pglCurContext->PolyOffset);
    break;
  case GL_POLYGON_OFFSET_LINE:
    return(pglCurContext->LineOffset);
    break;
  case GL_POLYGON_OFFSET_POINT:
    return(pglCurContext->PointOffset);
    break;
  case GL_FOG:
    return(pglCurContext->Fog);
    break;
  case GL_AUTO_NORMAL:
    return(pglCurContext->AutoNormal);
    break;
  case GL_LINE_STIPPLE:
    return(pglCurContext->LineStipple);
    break;
  case GL_POLYGON_STIPPLE:
    return(pglCurContext->PolyStipple);
    break;
  case GL_DITHER:
    return(pglCurContext->Dither);
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    break;
  }
  return((GLboolean)FALSE);
}

void APIENTRY glHint (GLenum target, GLenum mode)
{
  GLINT_OUTSIDE_BEGIN();
}

void APIENTRY glFlush (void)
{
  GLINT_OUTSIDE_BEGIN();

  // This flushes - Right now only way to send noop
  grSstResetPerfStats();
}

void APIENTRY glFinish (void)
{
  GLINT_OUTSIDE_BEGIN();
}

void APIENTRY glDebugEntry( DWORD arg1, DWORD arg2 )
{
} 

#ifdef RMG
void GLINT_ERROR(int val) {
  if(pglCurContext->GLerr == GL_NO_ERROR)
    pglCurContext->GLerr = val;
}

void GLINT_OUTSIDE_BEGIN(void) {
  if((pglCurContext->BeginEnd == TRUE) &&
     (pglCurContext->GLerr == GL_NO_ERROR))
    pglCurContext->GLerr =
      GL_INVALID_OPERATION;
}

void GLINT_INSIDE_BEGIN(void) {
  if((pglCurContext->BeginEnd == FALSE) &&
     (pglCurContext->GLerr == GL_NO_ERROR))
    pglCurContext->GLerr =
      GL_INVALID_OPERATION;
}
#endif
