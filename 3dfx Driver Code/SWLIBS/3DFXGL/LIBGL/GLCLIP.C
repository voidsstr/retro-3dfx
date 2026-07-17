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

void APIENTRY glClipPlane (GLenum plane, const GLdouble *equation)
{
  int pl;
  GLINT_OUTSIDE_BEGIN();

  switch(plane) {
  case GL_CLIP_PLANE0:
    pl = 0;
    break;
  case GL_CLIP_PLANE1:
    pl = 1;
    break;
  case GL_CLIP_PLANE2:
    pl = 2;
    break;
  case GL_CLIP_PLANE3:
    pl = 3;
    break;
  case GL_CLIP_PLANE4:
    pl = 4;
    break;
  case GL_CLIP_PLANE5:
    pl = 5;
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }

  if(pglCurContext->Listing) {

    glIntIntToList(OP_CLIPPLANE);
    glIntIntToList(pl);
    glIntFloatToList((float)equation[0]);
    glIntFloatToList((float)equation[1]);
    glIntFloatToList((float)equation[2]);
    glIntFloatToList((float)equation[3]);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->ClipDef[pl][0] = (float)equation[0];
  pglCurContext->ClipDef[pl][1] = (float)equation[1];
  pglCurContext->ClipDef[pl][2] = (float)equation[2];
  pglCurContext->ClipDef[pl][3] = (float)equation[3];

  glIntValidateInvTransp();
  glIntXform(pglCurContext->ClipDef[pl],
	     pglCurContext->CurInvTransp,
	     pglCurContext->ClipXform[pl]);
}

void APIENTRY glGetClipPlane (GLenum plane, GLdouble *equation)
{
  int pl;
  GLINT_OUTSIDE_BEGIN();

  switch(plane) {
  case GL_CLIP_PLANE0:
    pl = 0;
    break;
  case GL_CLIP_PLANE1:
    pl = 1;
    break;
  case GL_CLIP_PLANE2:
    pl = 2;
    break;
  case GL_CLIP_PLANE3:
    pl = 3;
    break;
  case GL_CLIP_PLANE4:
    pl = 4;
    break;
  case GL_CLIP_PLANE5:
    pl = 5;
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }

  equation[0] = (double)pglCurContext->ClipDef[pl][0];
  equation[1] = (double)pglCurContext->ClipDef[pl][1];
  equation[2] = (double)pglCurContext->ClipDef[pl][2];
  equation[3] = (double)pglCurContext->ClipDef[pl][3];
}

void glIntOutCodes(glLocalVertex *Vtx)
{
  // Calculate Outcodes for this vertex
  if(Vtx->Clip[0] > Vtx->Clip[3])
    Vtx->OutCodes = OUT_PLUS_X;
  else 
    Vtx->OutCodes = 0;
  if(Vtx->Clip[0] < -Vtx->Clip[3])
    Vtx->OutCodes |= OUT_MINUS_X;
  if(Vtx->Clip[1] > Vtx->Clip[3])
    Vtx->OutCodes |= OUT_PLUS_Y;
  if(Vtx->Clip[1] < -Vtx->Clip[3])
    Vtx->OutCodes |= OUT_MINUS_Y;
  if(Vtx->Clip[2] > Vtx->Clip[3])
    Vtx->OutCodes |= OUT_PLUS_Z;
  if(Vtx->Clip[2] < -Vtx->Clip[3])
    Vtx->OutCodes |= OUT_MINUS_Z;

  if(pglCurContext->ClipEna[0]) {
    if(((pglCurContext->ClipXform[0][0]*Vtx->Clip[0])+
	(pglCurContext->ClipXform[0][1]*Vtx->Clip[1])+
	(pglCurContext->ClipXform[0][2]*Vtx->Clip[2])+
	(pglCurContext->ClipXform[0][3]*Vtx->Clip[3])) < 0.0f)
      Vtx->OutCodes |= OUT_CLIP0;
  }
  if(pglCurContext->ClipEna[1]) {
    if(((pglCurContext->ClipXform[1][0]*Vtx->Clip[0])+
	(pglCurContext->ClipXform[1][1]*Vtx->Clip[1])+
	(pglCurContext->ClipXform[1][2]*Vtx->Clip[2])+
	(pglCurContext->ClipXform[1][3]*Vtx->Clip[3])) < 0.0f)
      Vtx->OutCodes |= OUT_CLIP1;
  }
  if(pglCurContext->ClipEna[2]) {
    if(((pglCurContext->ClipXform[2][0]*Vtx->Clip[0])+
	(pglCurContext->ClipXform[2][1]*Vtx->Clip[1])+
	(pglCurContext->ClipXform[2][2]*Vtx->Clip[2])+
	(pglCurContext->ClipXform[2][3]*Vtx->Clip[3])) < 0.0f)
      Vtx->OutCodes |= OUT_CLIP2;
  }
  if(pglCurContext->ClipEna[3]) {
    if(((pglCurContext->ClipXform[3][0]*Vtx->Clip[0])+
	(pglCurContext->ClipXform[3][1]*Vtx->Clip[1])+
	(pglCurContext->ClipXform[3][2]*Vtx->Clip[2])+
	(pglCurContext->ClipXform[3][3]*Vtx->Clip[3])) < 0.0f)
      Vtx->OutCodes |= OUT_CLIP3;
  }
  if(pglCurContext->ClipEna[4]) {
    if(((pglCurContext->ClipXform[4][0]*Vtx->Clip[0])+
	(pglCurContext->ClipXform[4][1]*Vtx->Clip[1])+
	(pglCurContext->ClipXform[4][2]*Vtx->Clip[2])+
	(pglCurContext->ClipXform[4][3]*Vtx->Clip[3])) < 0.0f)
      Vtx->OutCodes |= OUT_CLIP4;
  }
  if(pglCurContext->ClipEna[5]) {
    if(((pglCurContext->ClipXform[5][0]*Vtx->Clip[0])+
	(pglCurContext->ClipXform[5][1]*Vtx->Clip[1])+
	(pglCurContext->ClipXform[5][2]*Vtx->Clip[2])+
	(pglCurContext->ClipXform[5][3]*Vtx->Clip[3])) < 0.0f)
      Vtx->OutCodes |= OUT_CLIP5;
  }
}

void glIntInterp(glLocalVertex *Vtx1,
		 glLocalVertex *Vtx2,
		 glLocalVertex *Mid,
		 int plane)
{
  float din, dout, interp;

  // plane has been crossed
  switch(plane) {
  case OUT_MINUS_X:
    din = Vtx1->Clip[3] + Vtx1->Clip[0];
    dout = Vtx2->Clip[3] + Vtx2->Clip[0];
    break;
  case OUT_PLUS_X:
    din = Vtx1->Clip[3] - Vtx1->Clip[0];
    dout = Vtx2->Clip[3] - Vtx2->Clip[0];
    break;
  case OUT_MINUS_Y:
    din = Vtx1->Clip[3] + Vtx1->Clip[1];
    dout = Vtx2->Clip[3] + Vtx2->Clip[1];
    break;
  case OUT_PLUS_Y:
    din = Vtx1->Clip[3] - Vtx1->Clip[1];
    dout = Vtx2->Clip[3] - Vtx2->Clip[1];
    break;
  case OUT_MINUS_Z:
    din = Vtx1->Clip[3] + Vtx1->Clip[2];
    dout = Vtx2->Clip[3] + Vtx2->Clip[2];
    break;
  case OUT_PLUS_Z:
    din = Vtx1->Clip[3] - Vtx1->Clip[2];
    dout = Vtx2->Clip[3] - Vtx2->Clip[2];
    break;
  case OUT_CLIP0:
    din = (pglCurContext->ClipXform[0][0]*Vtx2->Clip[0])+
          (pglCurContext->ClipXform[0][1]*Vtx2->Clip[1])+
	  (pglCurContext->ClipXform[0][2]*Vtx2->Clip[2])+
	  (pglCurContext->ClipXform[0][3]*Vtx2->Clip[3]);
    dout = (pglCurContext->ClipXform[0][0]*Vtx1->Clip[0])+
           (pglCurContext->ClipXform[0][1]*Vtx1->Clip[1])+
	   (pglCurContext->ClipXform[0][2]*Vtx1->Clip[2])+
	   (pglCurContext->ClipXform[0][3]*Vtx1->Clip[3]);
    break;
  case OUT_CLIP1:
    din = (pglCurContext->ClipXform[1][0]*Vtx2->Clip[0])+
          (pglCurContext->ClipXform[1][1]*Vtx2->Clip[1])+
	  (pglCurContext->ClipXform[1][2]*Vtx2->Clip[2])+
	  (pglCurContext->ClipXform[1][3]*Vtx2->Clip[3]);
    dout = (pglCurContext->ClipXform[1][0]*Vtx1->Clip[0])+
           (pglCurContext->ClipXform[1][1]*Vtx1->Clip[1])+
	   (pglCurContext->ClipXform[1][2]*Vtx1->Clip[2])+
	   (pglCurContext->ClipXform[1][3]*Vtx1->Clip[3]);
    break;
  case OUT_CLIP2:
    din = (pglCurContext->ClipXform[2][0]*Vtx2->Clip[0])+
          (pglCurContext->ClipXform[2][1]*Vtx2->Clip[1])+
	  (pglCurContext->ClipXform[2][2]*Vtx2->Clip[2])+
	  (pglCurContext->ClipXform[2][3]*Vtx2->Clip[3]);
    dout = (pglCurContext->ClipXform[2][0]*Vtx1->Clip[0])+
           (pglCurContext->ClipXform[2][1]*Vtx1->Clip[1])+
	   (pglCurContext->ClipXform[2][2]*Vtx1->Clip[2])+
	   (pglCurContext->ClipXform[2][3]*Vtx1->Clip[3]);
    break;
  case OUT_CLIP3:
    din = (pglCurContext->ClipXform[3][0]*Vtx2->Clip[0])+
          (pglCurContext->ClipXform[3][1]*Vtx2->Clip[1])+
	  (pglCurContext->ClipXform[3][2]*Vtx2->Clip[2])+
	  (pglCurContext->ClipXform[3][3]*Vtx2->Clip[3]);
    dout = (pglCurContext->ClipXform[3][0]*Vtx1->Clip[0])+
           (pglCurContext->ClipXform[3][1]*Vtx1->Clip[1])+
	   (pglCurContext->ClipXform[3][2]*Vtx1->Clip[2])+
	   (pglCurContext->ClipXform[3][3]*Vtx1->Clip[3]);
    break;
  case OUT_CLIP4:
    din = (pglCurContext->ClipXform[4][0]*Vtx2->Clip[0])+
          (pglCurContext->ClipXform[4][1]*Vtx2->Clip[1])+
	  (pglCurContext->ClipXform[4][2]*Vtx2->Clip[2])+
	  (pglCurContext->ClipXform[4][3]*Vtx2->Clip[3]);
    dout = (pglCurContext->ClipXform[4][0]*Vtx1->Clip[0])+
           (pglCurContext->ClipXform[4][1]*Vtx1->Clip[1])+
	   (pglCurContext->ClipXform[4][2]*Vtx1->Clip[2])+
	   (pglCurContext->ClipXform[4][3]*Vtx1->Clip[3]);
    break;
  case OUT_CLIP5:
    din = (pglCurContext->ClipXform[5][0]*Vtx2->Clip[0])+
          (pglCurContext->ClipXform[5][1]*Vtx2->Clip[1])+
	  (pglCurContext->ClipXform[5][2]*Vtx2->Clip[2])+
	  (pglCurContext->ClipXform[5][3]*Vtx2->Clip[3]);
    dout = (pglCurContext->ClipXform[5][0]*Vtx1->Clip[0])+
           (pglCurContext->ClipXform[5][1]*Vtx1->Clip[1])+
	   (pglCurContext->ClipXform[5][2]*Vtx1->Clip[2])+
	   (pglCurContext->ClipXform[5][3]*Vtx1->Clip[3]);
    break;
  }
    
  interp = din / ( din - dout );
  
  //
  // Create and project interpolated vertex
  //
  Mid->Clip[0] = Vtx1->Clip[0] + 
    (interp * (Vtx2->Clip[0] - Vtx1->Clip[0]));
  Mid->Clip[1] = Vtx1->Clip[1] + 
    (interp * (Vtx2->Clip[1] - Vtx1->Clip[1]));
  Mid->Clip[2] = Vtx1->Clip[2] + 
    (interp * (Vtx2->Clip[2] - Vtx1->Clip[2]));
  Mid->Clip[3] = Vtx1->Clip[3] + 
    (interp * (Vtx2->Clip[3] - Vtx1->Clip[3]));
  

  Mid->Glide.r = Vtx1->Glide.r + 
    (interp * (Vtx2->Glide.r - Vtx1->Glide.r));
  Mid->Glide.g = Vtx1->Glide.g + 
    (interp * (Vtx2->Glide.g - Vtx1->Glide.g));
  Mid->Glide.b = Vtx1->Glide.b + 
    (interp * (Vtx2->Glide.b - Vtx1->Glide.b));
  Mid->Glide.a = Vtx1->Glide.a + 
    (interp * (Vtx2->Glide.a - Vtx1->Glide.a));
  
  Mid->Texture[0] = Vtx1->Texture[0] + 
    (interp * (Vtx2->Texture[0] - Vtx1->Texture[0]));
  Mid->Texture[1] = Vtx1->Texture[1] + 
    (interp * (Vtx2->Texture[1] - Vtx1->Texture[1]));
  
  Mid->Projected = FALSE;
  
  glIntOutCodes(Mid);
}

void glIntClipDrawLine(glLocalVertex *Vtx1, glLocalVertex *Vtx2)
{
  // line clip
  int i;
  int plane;
  glLocalVertex *V1, *V2;
  glLocalVertex vert[12];
  
  V1 = Vtx1;
  V2 = Vtx2;
  plane = 1;
  
  // repeat for all 6 planes + 6 model clip planes
  for(i=0;i<12;i++) {
    // Clip this plane ?
    if((V1->OutCodes|V2->OutCodes)&plane) {
      glIntInterp(V1,V2,&vert[i],plane);
      
      if(V1->OutCodes&plane) {
	// first vertex is outside
	V1 = &vert[i];
      } else {
	// second vertex is outside
	V2 = &vert[i];
      }
    }

    // Trivial Reject
    if((V1->OutCodes&V2->OutCodes) != 0) 
      return;

    plane = plane<<1;
  }
    
  glIntProject(V1);
  glIntProject(V2);

  switch(pglCurContext->RenderMode) {
  case GL_RENDER:
    // Draw Line
    if(pglCurContext->LineAA) {
      if(pglCurContext->DrawBuffer == GL_FRONT_AND_BACK) {
	grRenderBuffer(GR_BUFFER_FRONTBUFFER);
	grAADrawLine(&(V1->Glide),
		     &(V2->Glide));
	grRenderBuffer(GR_BUFFER_BACKBUFFER);
	grAADrawLine(&(V1->Glide),
		     &(V2->Glide));
      } else {
	grAADrawLine(&(V1->Glide),
		     &(V2->Glide));
      }
    } else {
      if(pglCurContext->DrawBuffer == GL_FRONT_AND_BACK) {
	grRenderBuffer(GR_BUFFER_FRONTBUFFER);
	grDrawLine(&(V1->Glide),
		     &(V2->Glide));
	grRenderBuffer(GR_BUFFER_BACKBUFFER);
	grDrawLine(&(V1->Glide),
		     &(V2->Glide));
      } else {
	grDrawLine(&(V1->Glide),
		     &(V2->Glide));
      }
    }
    break;
  case GL_SELECT:
    {
      GLfloat zmin, zmax;

      zmin = zmax = V1->Glide.z;

      if(V2->Glide.z > zmax)
	zmax = V2->Glide.z;
      if(V2->Glide.z < zmin)
	zmin = V2->Glide.z;
      
      glIntWriteSelect(zmin, zmax);
    }
    break;
  case GL_FEEDBACK:
    glIntWriteFeedback(V1);
    glIntWriteFeedback(V2);
    break;
  }
}

void glIntClipDrawTriangle(glLocalVertex *Vtx1, 
			   glLocalVertex *Vtx2, 
			   glLocalVertex *Vtx3)
{
  // Triangle Clip
  int i,j;
  int plane;
  int num_vert;
  glLocalVertex *V[30];
  int curscratch;
  glLocalVertex vert[24];
  glLocalVertex *last, *start, *end;
  int current;
  int reject;
  int do_plane;

  num_vert = 3;
  V[0] = Vtx1;
  V[1] = Vtx2;
  V[2] = Vtx3;
  
  do_plane = Vtx1->OutCodes|Vtx2->OutCodes|Vtx3->OutCodes;

  curscratch = 0;
  plane = 1;

  // repeat for all 6 planes + 6 model clip planes
  for(i=0;i<12;i++) {
    if((do_plane&plane) != 0) {
      last = V[0];
      current = 1;
      start = V[0];
      end = V[num_vert-1];
      
      while(current<num_vert) {
	if((last->OutCodes)&plane) {
	  if((V[current]->OutCodes)&plane) {
	    // OUT/OUT
	    // remove last vertex
            last = V[current];
	    for(j=current+1;j<num_vert;j++)
	      V[j-1] = V[j];
	    num_vert--;
	  } else {
	    // OUT/IN
	    // Add vertex
	    if(current == 1) {
	      // Started Outside
	      glIntInterp(last,V[current],&vert[curscratch],plane);
	      V[0] = &vert[curscratch++];
	      last = V[current];
	      current++;
	    } else {
	      // Started inside
	      glIntInterp(last,V[current],&vert[curscratch],plane);
	      for(j=num_vert;j>current;j--)
		V[j] = V[j-1];
	      V[current] = &vert[curscratch++];
	      num_vert++;
	      last = V[current+1];
	      current+=2;
	    }
	  }
	} else {
	  if((V[current]->OutCodes)&plane) {
	    // IN/OUT
	    glIntInterp(last,V[current],&vert[curscratch],plane);
	    last = V[current];
	    V[current] = &vert[curscratch++];
	    current++;
	  } else {
	    // IN/IN
	    last = V[current];
	    current++;
	  }
	}
      }
      
      if(((start->OutCodes)&plane)!= ((end->OutCodes)&plane)) {
	// last segment crosses plane - insert last vertex
	glIntInterp(start,end,&vert[curscratch],plane);
	V[current] = &vert[curscratch++];
	num_vert++;
      }

      // verify new outcodes for trivial reject
      do_plane = V[0]->OutCodes;
      reject = V[0]->OutCodes;
      for(j=1;j<num_vert;j++) {
	do_plane |= V[j]->OutCodes;
	reject &= V[j]->OutCodes;
      }
      
      if(reject != 0)
	return;
    }
    
    plane = plane<<1;
  }
  
  glIntProject(V[0]);
  glIntProject(V[1]);
  
  switch(pglCurContext->RenderMode) {
  case GL_RENDER:
    for(i=2;i<num_vert;i++) {
      // Project last vertex
      glIntProject(V[i]);

      // Draw Triangle
      if(pglCurContext->PolyAA) {
	if(pglCurContext->DrawBuffer == GL_FRONT_AND_BACK) {
	  grRenderBuffer(GR_BUFFER_FRONTBUFFER);
	  grAADrawTriangle(&(V[i-1]->Glide),
			   &(V[i]->Glide),
			   &(V[0]->Glide),
			   FXTRUE, FXTRUE, FXTRUE);
	  grRenderBuffer(GR_BUFFER_BACKBUFFER);
	  grAADrawTriangle(&(V[i-1]->Glide),
			   &(V[i]->Glide),
			   &(V[0]->Glide),
			   FXTRUE, FXTRUE, FXTRUE);
	} else {
	  grAADrawTriangle(&(V[i-1]->Glide),
			   &(V[i]->Glide),
			   &(V[0]->Glide),
			   FXTRUE, FXTRUE, FXTRUE);
	}
      } else {
	if(pglCurContext->DrawBuffer == GL_FRONT_AND_BACK) {
	  grRenderBuffer(GR_BUFFER_FRONTBUFFER);
	  grDrawTriangle(&(V[i-1]->Glide),
			 &(V[i]->Glide),
			 &(V[0]->Glide));
	  grRenderBuffer(GR_BUFFER_BACKBUFFER);
	  grDrawTriangle(&(V[i-1]->Glide),
			 &(V[i]->Glide),
			 &(V[0]->Glide));
	} else {
	  grDrawTriangle(&(V[i-1]->Glide),
			 &(V[i]->Glide),
			 &(V[0]->Glide));
	}
      }
    }
    break;
  case GL_SELECT:
    {
      GLfloat zmin, zmax;

      zmin = zmax = V[0]->Glide.z;

      for(i=1;i<num_vert;i++) {
	if(V[i]->Glide.z > zmax)
	  zmax = V[i]->Glide.z;
	if(V[i]->Glide.z < zmin)
	  zmin = V[i]->Glide.z;
      }

      glIntWriteSelect(zmin, zmax);
    }
    break;
  case GL_FEEDBACK:
    for(i=0;i<num_vert;i++)
      glIntWriteFeedback(V[i]);
    break;
  }
}
