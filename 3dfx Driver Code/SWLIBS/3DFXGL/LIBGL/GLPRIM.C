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

void APIENTRY glBegin (GLenum mode)
{
  GLenum PrimAssemState;
  GLboolean line, poly;

  switch(mode) {
  case GL_POINTS:
    PrimAssemState = PA_POINT1;
    line = FALSE;
    poly = FALSE;
    break;
  case GL_LINES:
    PrimAssemState = PA_LINE1;
    line = TRUE;
    poly = FALSE;
    break;
  case GL_LINE_STRIP:
    PrimAssemState = PA_LINE_STRIP1;
    line = TRUE;
    poly = FALSE;
    break;
  case GL_LINE_LOOP:
    PrimAssemState = PA_LINE_LOOP1;
    line = TRUE;
    poly = FALSE;
    break;
  case GL_TRIANGLES:
    PrimAssemState = PA_TRIANGLES1;
    line = FALSE;
    poly = TRUE;
    break;
  case GL_TRIANGLE_STRIP:
    PrimAssemState = PA_TRIANGLE_STRIP1;
    line = FALSE;
    poly = TRUE;
    break;
  case GL_TRIANGLE_FAN:
    PrimAssemState = PA_TRIANGLE_FAN1;
    line = FALSE;
    poly = TRUE;
    break;
  case GL_QUADS:
    PrimAssemState = PA_QUADS1;
    line = FALSE;
    poly = TRUE;
    break;
  case GL_QUAD_STRIP:
    PrimAssemState = PA_QUAD_STRIP1;
    line = FALSE;
    poly = TRUE;
    break;
  case GL_POLYGON:
    PrimAssemState = PA_POLYGON1;
    line = FALSE;
    poly = TRUE;
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }

  if(pglCurContext->Listing) {

    glIntIntToList(OP_BEGIN);
    glIntIntToList(PrimAssemState);
    glIntIntToList((int)line);
    glIntIntToList((int)poly);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  GLINT_OUTSIDE_BEGIN();

  pglCurContext->PrimAssemState = PrimAssemState;
  pglCurContext->CurLine = line;
  pglCurContext->CurPoly = poly;
  pglCurContext->BeginEnd = TRUE;
  pglCurContext->CurVtx = &(pglCurContext->Vtx1);
  pglCurContext->CurEdge = TRUE;


  glIntValidateContext();
  glIntValidateTexture();
  if(pglCurContext->Lighting) {
    glIntValidateInvTransp();
    glIntValidateLighting();
  }
  glIntValidateComposite();
  glIntValidateNeedEye();
}

void APIENTRY glEnd (void)
{
  if(pglCurContext->Listing) {

    glIntIntToList(OP_END);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  GLINT_INSIDE_BEGIN();

  pglCurContext->BeginEnd = FALSE;

  glIntClose();
}

void APIENTRY glPointSize (GLfloat size)
{
  GLINT_OUTSIDE_BEGIN();

  if(pglCurContext->Listing) {

    glIntIntToList(OP_POINTSIZE);
    glIntFloatToList(size);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->PointWidth = size;
}

void APIENTRY glLineStipple (GLint factor, GLushort pattern)
{
  GLushort bitmask;
  int i;

  GLINT_OUTSIDE_BEGIN();

  if(factor < 1)
    factor = 1;
  if(factor > 255)
    factor = 255;

  if(pglCurContext->Listing) {

    glIntIntToList(OP_LINESTIPPLE);
    glIntIntToList(factor);
    glIntIntToList(pattern);

    bitmask = 0x8000;
    for(i=0;i<16;i++) {
      if(bitmask&pattern)
	glIntIntToList(0xff);
      else
	glIntIntToList(0x00);
      bitmask = bitmask>>1;
    }

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->LineStippleFactor = factor;
  pglCurContext->LinePattern = pattern;

  bitmask = 0x8000;
  for(i=0;i<16;i++) {
    if(bitmask&pattern)
      pglCurContext->LineStippleData[i] = 0xff;
    else
      pglCurContext->LineStippleData[i] = 0x00;
    bitmask = bitmask>>1;
  }

  pglCurContext->LineStippleDirty = TRUE;
  pglCurContext->ContextDirty = TRUE;
}

void APIENTRY glLineWidth (GLfloat width)
{
  GLINT_OUTSIDE_BEGIN();

  if(pglCurContext->Listing) {

    glIntIntToList(OP_LINEWIDTH);
    glIntFloatToList(width);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->LineWidth = width;
}

void APIENTRY glFrontFace (GLenum mode)
{
  GLenum FrontFace;

  GLINT_OUTSIDE_BEGIN();

  switch(mode) {
  case GL_CCW:
    FrontFace = mode;
    break;
  case GL_CW:
    FrontFace = mode;
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }

  if(pglCurContext->Listing) {

    glIntIntToList(OP_FRONTFACE);
    glIntIntToList(FrontFace);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->FrontFace = FrontFace;
}

void APIENTRY glCullFace (GLenum mode)
{
  GLenum CullMode;

  GLINT_OUTSIDE_BEGIN();

  switch(mode) {
  case GL_FRONT:
    CullMode = mode;
    break;
  case GL_BACK:
    CullMode = mode;
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }

  if(pglCurContext->Listing) {

    glIntIntToList(OP_CULLFACE);
    glIntIntToList(CullMode);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CullMode = CullMode;
}

void APIENTRY glShadeModel (GLenum mode)
{
  GLenum ShadeModel;

  GLINT_OUTSIDE_BEGIN();

  switch(mode) {
  case GL_FLAT:
    ShadeModel = mode;
    break;
  case GL_SMOOTH:
    ShadeModel = mode;
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }

  if(pglCurContext->Listing) {

    glIntIntToList(OP_SHADEMODEL);
    glIntIntToList(ShadeModel);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->ShadeModel = ShadeModel;
  pglCurContext->ContextDirty = TRUE;
}

void APIENTRY glEdgeFlag (GLboolean flag)
{
  if(pglCurContext->Listing) {

    glIntIntToList(OP_EDGE_COORD);
    glIntIntToList((int)flag);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurEdge = flag;
}

void APIENTRY glEdgeFlagv (const GLboolean *flag)
{
  if(pglCurContext->Listing) {

    glIntIntToList(OP_EDGE_COORD);
    glIntIntToList((int)flag[0]);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurEdge = flag[0];
}

void APIENTRY glPolygonMode (GLenum face, GLenum mode)
{
  GLINT_OUTSIDE_BEGIN();

  switch(mode) {
  case GL_POINT:
    break;
  case GL_LINE:
    break;
  case GL_FILL:
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }

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

  if(pglCurContext->Listing) {

    switch(face) {
    case GL_FRONT:
      glIntIntToList(OP_FRONTMODE);
      glIntIntToList(mode);
      break;
    case GL_BACK:
      glIntIntToList(OP_BACKMODE);
      glIntIntToList(mode);
      break;
    case GL_FRONT_AND_BACK:
      glIntIntToList(OP_FRONTMODE);
      glIntIntToList(mode);
      glIntIntToList(OP_BACKMODE);
      glIntIntToList(mode);
      break;
    default:
      break;
    }

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  switch(face) {
  case GL_FRONT:
    pglCurContext->PolygonFrontMode = mode;
    break;
  case GL_BACK:
    pglCurContext->PolygonBackMode = mode;
    break;
  case GL_FRONT_AND_BACK:
    pglCurContext->PolygonFrontMode = mode;
    pglCurContext->PolygonBackMode = mode;
    break;
  default:
    break;
  }
}

void APIENTRY glPolygonOffset (GLfloat factor, GLfloat units)
{
  GLINT_OUTSIDE_BEGIN();

  if(pglCurContext->Listing) {

    glIntIntToList(OP_POLYOFFSET);
    glIntIntToList((int)units);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->DepthBiasLevel = (FxI16)units;

  pglCurContext->ContextDirty = TRUE;
}

void APIENTRY glPolygonStipple (const GLubyte *mask)
{
#define STIPPLE
#ifdef STIPPLE
  int i;
  unsigned char bitmask;
  unsigned char *ptr;
  unsigned char *dataptr;

  GLINT_OUTSIDE_BEGIN();

  if(pglCurContext->Listing) {

    glIntIntToList(OP_POLYSTIPPLE);
    
    dataptr = (unsigned char *)malloc((32*32)+4);
    
    // Link to data block list and advance pointer
    *((unsigned int **)dataptr) = 
      pglCurContext->BuildDataStart;
    pglCurContext->BuildDataStart = (unsigned int *)dataptr;
    ((unsigned int **)dataptr)++;
    
    glIntIntToList((int)dataptr);
    
    ptr = (unsigned char *)mask;
    bitmask = 0x80;
    for(i=0;i<32*32;i++) {
      if(bitmask&(*ptr)) {
	*dataptr++ = 0xff;
      } else {
	*dataptr++ = 0x00;
      }
      if(bitmask == 0x01) {
	ptr++;
	bitmask = 0x80;
      } else {
	bitmask = bitmask >> 1;
      }
    }

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  ptr = (unsigned char *)mask;
  bitmask = 0x80;
  for(i=0;i<32*32;i++) {
    if(bitmask&(*ptr)) {
      pglCurContext->PolyStippleData[i] = 0xff;
    } else {
      pglCurContext->PolyStippleData[i] = 0x00;
    }
    if(bitmask == 0x01) {
      ptr++;
      bitmask = 0x80;
    } else {
      bitmask = bitmask >> 1;
    }
  }

  pglCurContext->PolyStippleDirty = TRUE;
  pglCurContext->ContextDirty = TRUE;
#else
  GLINT_OUTSIDE_BEGIN();
#endif
}

void APIENTRY glGetPolygonStipple (GLubyte *mask)
{
}

GLint APIENTRY glRenderMode (GLenum mode)
{
  GLint returnval;

  GLINT_OUTSIDE_BEGIN();

  switch(mode) {
  case GL_RENDER:
    pglCurContext->RenderMode = mode;
    returnval = 0;
    break;
  case GL_SELECT:
    if(pglCurContext->SelectBuffer == NULL) {
      GLINT_ERROR(GL_INVALID_OPERATION);
      return((GLint)0);
    }

    if(pglCurContext->RenderMode == GL_SELECT)
      returnval = pglCurContext->SelectNumHits;
    else
      returnval = 0;

    pglCurContext->SelectNumHits = 0;
    pglCurContext->CurSelectBuffer = pglCurContext->SelectBuffer;
    pglCurContext->CurSelectSize = pglCurContext->SelectSize;
    pglCurContext->RenderMode = mode;
    break;
  case GL_FEEDBACK:
    if(pglCurContext->FeedbackBuffer == NULL) {
      GLINT_ERROR(GL_INVALID_OPERATION);
      return((GLint)0);
    }

    if(pglCurContext->RenderMode == GL_FEEDBACK)
      returnval = pglCurContext->FeedbackNumHits;
    else
      returnval = 0;

    pglCurContext->FeedbackNumHits = 0;
    pglCurContext->CurFeedbackBuffer = pglCurContext->FeedbackBuffer;
    pglCurContext->CurFeedbackSize = pglCurContext->FeedbackSize;
    pglCurContext->RenderMode = mode;
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    break;
  }
  return(returnval);
}

void glIntClose(void) {
  /* primitive closure */
  switch(pglCurContext->PrimAssemState) {
  case PA_LINE_LOOP3:
    glIntProject(&(pglCurContext->Vtx1));
    glIntProject(&(pglCurContext->Vtx2));

    if(pglCurContext->ShadeModel == GL_FLAT) {
      if(pglCurContext->Lighting) {
	glIntLight(&(pglCurContext->Vtx1));
      }
      glIntFlat(&(pglCurContext->Vtx1));
    } else {
      if(pglCurContext->Lighting) {
	glIntLight(&(pglCurContext->Vtx1));
	glIntLight(&(pglCurContext->Vtx2));
      }
    }

    glIntDrawLine(&(pglCurContext->Vtx2),&(pglCurContext->Vtx1));
    break;
  case PA_LINE_LOOP4:
    glIntProject(&(pglCurContext->Vtx1));
    glIntProject(&(pglCurContext->Vtx3));

    if(pglCurContext->ShadeModel == GL_FLAT) {
      if(pglCurContext->Lighting) {
	glIntLight(&(pglCurContext->Vtx1));
      }
      glIntFlat(&(pglCurContext->Vtx1));
    } else {
      if(pglCurContext->Lighting) {
	glIntLight(&(pglCurContext->Vtx1));
	glIntLight(&(pglCurContext->Vtx3));
      }
    }

    glIntDrawLine(&(pglCurContext->Vtx3),&(pglCurContext->Vtx1));
    break;
  case PA_POLYGON4:
    if(pglCurContext->CurFrontFace) {
      switch(pglCurContext->PolygonFrontMode) {
      case GL_POINT:
	break;
      case GL_LINE:
	if(pglCurContext->Vtx3.Edge) {
	  glIntDrawLine(&(pglCurContext->Vtx3),
			&(pglCurContext->Vtx1));
	}
	break;
      case GL_FILL:
	break;
      default:
	break;
      }
    } else {
      switch(pglCurContext->PolygonBackMode) {
      case GL_POINT:
	break;
      case GL_LINE:
	if(pglCurContext->Vtx3.Edge) {
	  glIntDrawLine(&(pglCurContext->Vtx3),
			&(pglCurContext->Vtx1));
	}
	break;
      case GL_FILL:
	break;
      default:
	break;
      }
    }
    break;
  case PA_POLYGON5:
    if(pglCurContext->CurFrontFace) {
      switch(pglCurContext->PolygonFrontMode) {
      case GL_POINT:
	break;
      case GL_LINE:
	if(pglCurContext->Vtx2.Edge) {
	  glIntDrawLine(&(pglCurContext->Vtx2),
			&(pglCurContext->Vtx1));
	}
	break;
      case GL_FILL:
	break;
      default:
	break;
      }
    } else {
      switch(pglCurContext->PolygonBackMode) {
      case GL_POINT:
	break;
      case GL_LINE:
	if(pglCurContext->Vtx2.Edge) {
	  glIntDrawLine(&(pglCurContext->Vtx2),
			&(pglCurContext->Vtx1));
	}
	break;
      case GL_FILL:
	break;
      default:
	break;
      }
    }
    break;
  default:
    break;
  }
  pglCurContext->PrimAssemState = PA_FLUSH;
}

float glIntCCW(glLocalVertex *Vtx1,
	       glLocalVertex *Vtx2,
	       glLocalVertex *Vtx3)
{
  float dxAC, dxBC, dyAC, dyBC, area;
  float interp, din, dout;
  float Midx1, Midx2, Midy1, Midy2;

  // Detect verts behind the eye (w < 0)
  if((Vtx1->OutCodes|Vtx2->OutCodes|Vtx3->OutCodes)&OUT_MINUS_Z) {
     
    if((Vtx1->OutCodes&Vtx2->OutCodes&Vtx3->OutCodes)!=0)
      /* degenerate */
      return(0.0f);
      
    if(Vtx1->OutCodes&OUT_MINUS_Z) {
      // vertex 1 is out
      if(Vtx2->OutCodes&OUT_MINUS_Z) {
	// vertex 2 is out -> 3 is only vertex in
	// v1, v2 out - v3 in
	// clip 2 to 3
	din = Vtx2->Clip[3] + Vtx2->Clip[2];
	dout = Vtx3->Clip[3] + Vtx3->Clip[2];
	interp = din / ( din - dout );

	Midx1 = Vtx2->Clip[0] + 
	  (interp * (Vtx3->Clip[0] - Vtx2->Clip[0]));
	Midx1 = ((Midx1/0.01f)*pglCurContext->xScale)+
	  pglCurContext->xOffset;

	Midy1 = Vtx2->Clip[1] + 
	  (interp * (Vtx3->Clip[1] - Vtx2->Clip[1]));
	Midy1 = ((Midy1/0.01f)*pglCurContext->yScale)+
	  pglCurContext->yOffset;

	// clip 1 to 3
	din = Vtx1->Clip[3] + Vtx1->Clip[2];
	dout = Vtx3->Clip[3] + Vtx3->Clip[2];
	interp = din / ( din - dout );

	Midx2 = Vtx1->Clip[0] + 
	  (interp * (Vtx3->Clip[0] - Vtx1->Clip[0]));
	Midx2 = ((Midx2/0.01f)*pglCurContext->xScale)+
	  pglCurContext->xOffset;

	Midy2 = Vtx1->Clip[1] + 
	  (interp * (Vtx3->Clip[1] - Vtx1->Clip[1]));
	Midy2 = ((Midy2/0.01f)*pglCurContext->yScale)+
	  pglCurContext->yOffset;

	/* Compute signed area of the triangle */
	// Mid1, 3, Mid2
	dxAC = Midx1 - Midx2;
	dxBC = Vtx3->Glide.x - Midx2;
	dyAC = Midy1 - Midy2;
	dyBC = Vtx3->Glide.y - Midy2;
      } else {
	// vertex 2 in in
	if(Vtx3->OutCodes&OUT_MINUS_Z) {
	    // vertex 3 is out
	    // v1, v3 out - v2 in
	    // clip 1 to 2
	    din = Vtx1->Clip[3] + Vtx1->Clip[2];
	    dout = Vtx2->Clip[3] + Vtx2->Clip[2];
	    interp = din / ( din - dout );
	    
	    Midx1 = Vtx1->Clip[0] + 
	      (interp * (Vtx2->Clip[0] - Vtx1->Clip[0]));
	    Midx1 = ((Midx1/0.01f)*pglCurContext->xScale)+
	      pglCurContext->xOffset;
	    
	    Midy1 = Vtx1->Clip[1] + 
	      (interp * (Vtx2->Clip[1] - Vtx1->Clip[1]));
	    Midy1 = ((Midy1/0.01f)*pglCurContext->yScale)+
	      pglCurContext->yOffset;
	    
	    // clip 3 to 2
	    din = Vtx3->Clip[3] + Vtx3->Clip[2];
	    dout = Vtx2->Clip[3] + Vtx2->Clip[2];
	    interp = din / ( din - dout );
	    
	    Midx2 = Vtx3->Clip[0] + 
	      (interp * (Vtx2->Clip[0] - Vtx3->Clip[0]));
	    Midx2 = ((Midx2/0.01f)*pglCurContext->xScale)+
	      pglCurContext->xOffset;
	    
	    Midy2 = Vtx3->Clip[1] + 
	      (interp * (Vtx2->Clip[1] - Vtx3->Clip[1]));
	    Midy2 = ((Midy2/0.01f)*pglCurContext->yScale)+
	      pglCurContext->yOffset;
	    
	    /* Compute signed area of the triangle */
	    // Mid1, 2, Mid2
            dxAC = Midx1 - Midx2;
	    dxBC = Vtx2->Glide.x - Midx2;
	    dyAC = Midy1 - Midy2;
	    dyBC = Vtx2->Glide.y - Midy2;
	  } else {
	    // vertex 3 is in
	    // v1 out - v2,v3 in
  	    // clip 1 to 2
	    din = Vtx1->Clip[3] + Vtx1->Clip[2];
	    dout = Vtx2->Clip[3] + Vtx2->Clip[2];
	    interp = din / ( din - dout );
	    
	    Midx1 = Vtx1->Clip[0] + 
	      (interp * (Vtx2->Clip[0] - Vtx1->Clip[0]));
	    Midx1 = ((Midx1/0.01f)*pglCurContext->xScale)+
	      pglCurContext->xOffset;
	    
	    Midy1 = Vtx1->Clip[1] + 
	      (interp * (Vtx2->Clip[1] - Vtx1->Clip[1]));
	    Midy1 = ((Midy1/0.01f)*pglCurContext->yScale)+
	      pglCurContext->yOffset;

	    /* Compute signed area of the triangle */
	    // Mid1, 2, 3
	    dxAC = Midx1 - Vtx3->Glide.x;
	    dxBC = Vtx2->Glide.x - Vtx3->Glide.x;
	    dyAC = Midy1 - Vtx3->Glide.y;
	    dyBC = Vtx2->Glide.y - Vtx3->Glide.y;
	  }
      }
    } else {
      // vertex 1 is in
      if(Vtx2->OutCodes&OUT_MINUS_Z) {
	// vertex 2 is out
	if(Vtx3->OutCodes&OUT_MINUS_Z) {
	  // v1 in - v2, v3 out
	  // clip 2 to 1
	  din = Vtx2->Clip[3] + Vtx2->Clip[2];
	  dout = Vtx1->Clip[3] + Vtx1->Clip[2];
	  interp = din / ( din - dout );
	  
	  Midx1 = Vtx2->Clip[0] + 
	    (interp * (Vtx1->Clip[0] - Vtx2->Clip[0]));
	  Midx1 = ((Midx1/0.01f)*pglCurContext->xScale)+
	    pglCurContext->xOffset;
	  
	  Midy1 = Vtx2->Clip[1] + 
	    (interp * (Vtx1->Clip[1] - Vtx2->Clip[1]));
	  Midy1 = ((Midy1/0.01f)*pglCurContext->yScale)+
	    pglCurContext->yOffset;
	  
	  // clip 3 to 1
	  din = Vtx3->Clip[3] + Vtx3->Clip[2];
	  dout = Vtx1->Clip[3] + Vtx1->Clip[2];
	  interp = din / ( din - dout );
	  
	  Midx2 = Vtx3->Clip[0] + 
	    (interp * (Vtx1->Clip[0] - Vtx3->Clip[0]));
	  Midx2 = ((Midx2/0.01f)*pglCurContext->xScale)+
	    pglCurContext->xOffset;
	  
	  Midy2 = Vtx3->Clip[1] + 
	    (interp * (Vtx1->Clip[1] - Vtx3->Clip[1]));
	  Midy2 = ((Midy2/0.01f)*pglCurContext->yScale)+
	    pglCurContext->yOffset;
	  
	  /* Compute signed area of the triangle */
	  // 1, Mid1, Mid2
	  dxAC = Vtx1->Glide.x - Midx2;
	  dxBC = Midx1 - Midx2;
	  dyAC = Vtx1->Glide.y - Midy2;
	  dyBC = Midy1 - Midy2;
	} else {
	  // v1, v3 in - v2 out
	  // clip 2 to 1
	  din = Vtx2->Clip[3] + Vtx2->Clip[2];
	  dout = Vtx1->Clip[3] + Vtx1->Clip[2];
	  interp = din / ( din - dout );
	  
	  Midx1 = Vtx2->Clip[0] + 
	    (interp * (Vtx1->Clip[0] - Vtx2->Clip[0]));
	  Midx1 = ((Midx1/0.01f)*pglCurContext->xScale)+
	    pglCurContext->xOffset;
	  
	  Midy1 = Vtx2->Clip[1] + 
	    (interp * (Vtx1->Clip[1] - Vtx2->Clip[1]));
	  Midy1 = ((Midy1/0.01f)*pglCurContext->yScale)+
	    pglCurContext->yOffset;
	  
	  // clip 2 to 3
	  din = Vtx2->Clip[3] + Vtx2->Clip[2];
	  dout = Vtx3->Clip[3] + Vtx3->Clip[2];
	  interp = din / ( din - dout );
	  
	  Midx2 = Vtx2->Clip[0] + 
	    (interp * (Vtx3->Clip[0] - Vtx2->Clip[0]));
	  Midx2 = ((Midx2/0.01f)*pglCurContext->xScale)+
	    pglCurContext->xOffset;
	  
	  Midy2 = Vtx2->Clip[1] + 
	    (interp * (Vtx3->Clip[1] - Vtx2->Clip[1]));
	  Midy2 = ((Midy2/0.01f)*pglCurContext->yScale)+
	    pglCurContext->yOffset;
	  
	  /* Compute signed area of the triangle */
	  // 1, Mid1, Mid2
	  dxAC = Vtx1->Glide.x - Midx2;
	  dxBC = Midx1 - Midx2;
	  dyAC = Vtx1->Glide.y - Midy2;
	  dyBC = Midy1 - Midy2;
	}
      } else {
	// vertex 2 in in -> 3 must be out
	// v1, v2 in - v3 out
	// clip 3 to 2
	din = Vtx3->Clip[3] + Vtx3->Clip[2];
	dout = Vtx2->Clip[3] + Vtx2->Clip[2];
	interp = din / ( din - dout );

	Midx1 = Vtx3->Clip[0] + 
	  (interp * (Vtx2->Clip[0] - Vtx3->Clip[0]));
	Midx1 = ((Midx1/0.01f)*pglCurContext->xScale)+
	  pglCurContext->xOffset;

	Midy1 = Vtx3->Clip[1] + 
	  (interp * (Vtx2->Clip[1] - Vtx3->Clip[1]));
	Midy1 = ((Midy1/0.01f)*pglCurContext->yScale)+
	  pglCurContext->yOffset;

	/* Compute signed area of the triangle */
	// 1, 2, Mid1
	dxAC = Vtx1->Glide.x - Midx1;
	dxBC = Vtx2->Glide.x - Midx1;
	dyAC = Vtx1->Glide.y - Midy1;
	dyBC = Vtx2->Glide.y - Midy1;
      }
    }

  } else {
    /* Compute signed area of the triangle */
    dxAC = Vtx1->Glide.x - Vtx3->Glide.x;
    dxBC = Vtx2->Glide.x - Vtx3->Glide.x;
    dyAC = Vtx1->Glide.y - Vtx3->Glide.y;
    dyBC = Vtx2->Glide.y - Vtx3->Glide.y;
  }

  area = dxAC * dyBC - dxBC * dyAC;

  return(area);
}

void glIntFlat(glLocalVertex *Vtx)
{
  GrColor_t FlatColor;

  FlatColor = (((FxU32)(Vtx->Glide.a))<<24)|
    (((FxU32)(Vtx->Glide.b))<<16)|
      (((FxU32)(Vtx->Glide.g))<<8)|
	((FxU32)(Vtx->Glide.r));

  grConstantColorValue(FlatColor);
}

void glIntVertex(void)
{
  //
  // Update Context
  //

  // Model to Clip
  glIntXform(pglCurContext->CurObj,
	     pglCurContext->CurComposite,
	     pglCurContext->CurClip);

  // Note: Eye only needed for Attenuation, Positional Lighting,
  //       Local_Viewer and TexGen
  if(pglCurContext->NeedEye) {
    // Model to Eye
    glIntXform(pglCurContext->CurObj,
	       pglCurContext->CurModelView,
	       pglCurContext->CurEye);
  }

  if(pglCurContext->Lighting) {
    // Normal to World
    glIntXform3(pglCurContext->CurNormal,
		pglCurContext->CurInvTransp,
		pglCurContext->CurTxNormal);
    
    if(pglCurContext->Normalize) {
      glIntNormalize(pglCurContext->CurTxNormal);
    }
  }

  if(pglCurContext->Tex2D) {
    if(pglCurContext->TexMatrixIdentity) {
      pglCurContext->CurTxTex[0] = pglCurContext->CurTex[0]; 
      pglCurContext->CurTxTex[1] = pglCurContext->CurTex[1]; 
    } else {
      // Texture Transform
      glIntXform(pglCurContext->CurTex,
	         pglCurContext->CurTexMat,
	         pglCurContext->CurTxTex);
    
      // Normalize Texture
      if((pglCurContext->CurTxTex[3] != 1.0f)&& 
	 (pglCurContext->CurTxTex[3] != 0.0f)) {
	pglCurContext->CurTxTex[0] = pglCurContext->CurTxTex[0]/
                                     pglCurContext->CurTxTex[3];
        pglCurContext->CurTxTex[1] = pglCurContext->CurTxTex[1]/
                                     pglCurContext->CurTxTex[3];
        pglCurContext->CurTxTex[2] = pglCurContext->CurTxTex[2]/
                                     pglCurContext->CurTxTex[3];
      }
    }
  }

//
// Copy to vertex
//
  pglCurContext->CurVtx->Projected = FALSE;

  pglCurContext->CurVtx->Edge = pglCurContext->CurEdge;

//  pglCurContext->CurVtx->Clip[0] = pglCurContext->CurClip[0];
//  pglCurContext->CurVtx->Clip[1] = pglCurContext->CurClip[1];
//  pglCurContext->CurVtx->Clip[2] = pglCurContext->CurClip[2];
//  pglCurContext->CurVtx->Clip[3] = pglCurContext->CurClip[3];
    memcpy(pglCurContext->CurVtx->Clip, pglCurContext->CurClip, 4*4);

  if(pglCurContext->Lighting) {
//    pglCurContext->CurVtx->Color[0] = pglCurContext->CurColor[0];
//    pglCurContext->CurVtx->Color[1] = pglCurContext->CurColor[1];
//    pglCurContext->CurVtx->Color[2] = pglCurContext->CurColor[2];
//    pglCurContext->CurVtx->Color[3] = pglCurContext->CurColor[3];
    memcpy(pglCurContext->CurVtx->Color, pglCurContext->CurColor, 4*4);
    pglCurContext->CurVtx->Lighted = FALSE;
  } else {
    pglCurContext->CurVtx->Glide.r = pglCurContext->CurColor[0]*255.0f;
    pglCurContext->CurVtx->Glide.g = pglCurContext->CurColor[1]*255.0f;
    pglCurContext->CurVtx->Glide.b = pglCurContext->CurColor[2]*255.0f;
    pglCurContext->CurVtx->Glide.a = pglCurContext->CurColor[3]*255.0f;
    pglCurContext->CurVtx->Lighted = TRUE;
  }

  // Note: only needed for Attenuation, Positional Lighting,
  //       Local_Viewer and TexGen
  if(pglCurContext->NeedEye) {
//    pglCurContext->CurVtx->EyePos[0] = pglCurContext->CurEye[0];
//    pglCurContext->CurVtx->EyePos[1] = pglCurContext->CurEye[1];
//    pglCurContext->CurVtx->EyePos[2] = pglCurContext->CurEye[2];
//    pglCurContext->CurVtx->EyePos[3] = pglCurContext->CurEye[3];
    memcpy(pglCurContext->CurVtx->EyePos, pglCurContext->CurEye, 4*4);
  }

  if(pglCurContext->Tex2D) {
//    pglCurContext->CurVtx->Texture[0] = pglCurContext->CurTxTex[0];
//    pglCurContext->CurVtx->Texture[1] = pglCurContext->CurTxTex[1];
    memcpy(pglCurContext->CurVtx->Texture, pglCurContext->CurTxTex, 2*4);
  }

  if(pglCurContext->Lighting) {
//    pglCurContext->CurVtx->TxNormal[0] = pglCurContext->CurTxNormal[0];
//    pglCurContext->CurVtx->TxNormal[1] = pglCurContext->CurTxNormal[1];
//    pglCurContext->CurVtx->TxNormal[2] = pglCurContext->CurTxNormal[2];
    memcpy(pglCurContext->CurVtx->TxNormal, pglCurContext->CurTxNormal, 3*4);
  }

  glIntOutCodes(pglCurContext->CurVtx);

  /* primitive assembly */
  switch(pglCurContext->PrimAssemState) {
  case PA_POINT1:
    glIntProject(&(pglCurContext->Vtx1));

    if(pglCurContext->Lighting) {
      glIntLight(&(pglCurContext->Vtx1));
    }

    if(pglCurContext->ShadeModel == GL_FLAT) {
      glIntFlat(&(pglCurContext->Vtx1));
    }

    glIntDrawPoint(&(pglCurContext->Vtx1));
    
    pglCurContext->PrimAssemState = PA_POINT1;
    pglCurContext->CurVtx = &(pglCurContext->Vtx1);
    break;
  case PA_LINE1:
    pglCurContext->PrimAssemState = PA_LINE2;
    pglCurContext->CurVtx = &(pglCurContext->Vtx2);
    pglCurContext->LinePos = 0.0f;
    break;
  case PA_LINE2:
    glIntProject(&(pglCurContext->Vtx1));
    glIntProject(&(pglCurContext->Vtx2));

    if(pglCurContext->ShadeModel == GL_FLAT) {
      if(pglCurContext->Lighting) {
	glIntLight(&(pglCurContext->Vtx2));
      }
      glIntFlat(&(pglCurContext->Vtx2));
    } else {
      if(pglCurContext->Lighting) {
	glIntLight(&(pglCurContext->Vtx1));
	glIntLight(&(pglCurContext->Vtx2));
      }
    }

    glIntDrawLine(&(pglCurContext->Vtx1),&(pglCurContext->Vtx2));

    pglCurContext->PrimAssemState = PA_LINE1;
    pglCurContext->CurVtx = &(pglCurContext->Vtx1);
    break;
  case PA_LINE_STRIP1:
    pglCurContext->PrimAssemState = PA_LINE_STRIP2;
    pglCurContext->CurVtx = &(pglCurContext->Vtx2);
    pglCurContext->LinePos = 0.0f;
    break;
  case PA_LINE_STRIP2:
    glIntProject(&(pglCurContext->Vtx1));
    glIntProject(&(pglCurContext->Vtx2));

    if(pglCurContext->ShadeModel == GL_FLAT) {
      if(pglCurContext->Lighting) {
	glIntLight(&(pglCurContext->Vtx2));
      }
      glIntFlat(&(pglCurContext->Vtx2));
    } else {
      if(pglCurContext->Lighting) {
	glIntLight(&(pglCurContext->Vtx1));
	glIntLight(&(pglCurContext->Vtx2));
      }
    }

    glIntDrawLine(&(pglCurContext->Vtx1),&(pglCurContext->Vtx2));

    pglCurContext->PrimAssemState = PA_LINE_STRIP3;
    pglCurContext->CurVtx = &(pglCurContext->Vtx1);
    break;
  case PA_LINE_STRIP3:
    glIntProject(&(pglCurContext->Vtx1));
    glIntProject(&(pglCurContext->Vtx2));

    if(pglCurContext->ShadeModel == GL_FLAT) {
      if(pglCurContext->Lighting) {
	glIntLight(&(pglCurContext->Vtx1));
      }
      glIntFlat(&(pglCurContext->Vtx1));
    } else {
      if(pglCurContext->Lighting) {
	glIntLight(&(pglCurContext->Vtx1));
	glIntLight(&(pglCurContext->Vtx2));
      }
    }
    
    glIntDrawLine(&(pglCurContext->Vtx2),&(pglCurContext->Vtx1));

    pglCurContext->PrimAssemState = PA_LINE_STRIP2;
    pglCurContext->CurVtx = &(pglCurContext->Vtx2);
    break;
  case PA_LINE_STRIP4:
    glIntProject(&(pglCurContext->Vtx1));
    glIntProject(&(pglCurContext->Vtx2));

    if(pglCurContext->ShadeModel == GL_FLAT) {
      if(pglCurContext->Lighting) {
	glIntLight(&(pglCurContext->Vtx2));
      }
      glIntFlat(&(pglCurContext->Vtx2));
    } else {
      if(pglCurContext->Lighting) {
	glIntLight(&(pglCurContext->Vtx1));
	glIntLight(&(pglCurContext->Vtx2));
      }
    }

    glIntDrawLine(&(pglCurContext->Vtx1),&(pglCurContext->Vtx2));

    pglCurContext->PrimAssemState = PA_LINE_STRIP3;
    pglCurContext->CurVtx = &(pglCurContext->Vtx1);
    break;
  case PA_LINE_LOOP1:
    pglCurContext->PrimAssemState = PA_LINE_LOOP2;
    pglCurContext->CurVtx = &(pglCurContext->Vtx2);
    pglCurContext->LinePos = 0.0f;
    break;
  case PA_LINE_LOOP2:
    glIntProject(&(pglCurContext->Vtx1));
    glIntProject(&(pglCurContext->Vtx2));

    if(pglCurContext->ShadeModel == GL_FLAT) {
      if(pglCurContext->Lighting) {
	glIntLight(&(pglCurContext->Vtx2));
      }
      glIntFlat(&(pglCurContext->Vtx2));
    } else {
      if(pglCurContext->Lighting) {
	glIntLight(&(pglCurContext->Vtx1));
	glIntLight(&(pglCurContext->Vtx2));
      }
    }

    glIntDrawLine(&(pglCurContext->Vtx1),&(pglCurContext->Vtx2));

    pglCurContext->PrimAssemState = PA_LINE_LOOP3;
    pglCurContext->CurVtx = &(pglCurContext->Vtx3);
    break;
  case PA_LINE_LOOP3:
    glIntProject(&(pglCurContext->Vtx2));
    glIntProject(&(pglCurContext->Vtx3));

    if(pglCurContext->ShadeModel == GL_FLAT) {
      if(pglCurContext->Lighting) {
	glIntLight(&(pglCurContext->Vtx3));
      }
      glIntFlat(&(pglCurContext->Vtx3));
    } else {
      if(pglCurContext->Lighting) {
	glIntLight(&(pglCurContext->Vtx2));
	glIntLight(&(pglCurContext->Vtx3));
      }
    }

    glIntDrawLine(&(pglCurContext->Vtx2),&(pglCurContext->Vtx3));

    pglCurContext->PrimAssemState = PA_LINE_LOOP4;
    pglCurContext->CurVtx = &(pglCurContext->Vtx2);
    break;
  case PA_LINE_LOOP4:
    glIntProject(&(pglCurContext->Vtx2));
    glIntProject(&(pglCurContext->Vtx3));

    if(pglCurContext->ShadeModel == GL_FLAT) {
      if(pglCurContext->Lighting) {
	glIntLight(&(pglCurContext->Vtx2));
      }
      glIntFlat(&(pglCurContext->Vtx2));
    } else {
      if(pglCurContext->Lighting) {
	glIntLight(&(pglCurContext->Vtx2));
	glIntLight(&(pglCurContext->Vtx3));
      }
    }

    glIntDrawLine(&(pglCurContext->Vtx3),&(pglCurContext->Vtx2));

    pglCurContext->PrimAssemState = PA_LINE_LOOP3;
    pglCurContext->CurVtx = &(pglCurContext->Vtx3);
    break;
  case PA_TRIANGLES1:
    pglCurContext->PrimAssemState = PA_TRIANGLES2;
    pglCurContext->CurVtx = &(pglCurContext->Vtx2);
    break;
  case PA_TRIANGLES2:
    pglCurContext->PrimAssemState = PA_TRIANGLES3;
    pglCurContext->CurVtx = &(pglCurContext->Vtx3);
    break;
  case PA_TRIANGLES3:
    glIntProject(&(pglCurContext->Vtx1));
    glIntProject(&(pglCurContext->Vtx2));
    glIntProject(&(pglCurContext->Vtx3));

    if(pglCurContext->Lighting) {
      /* figure out facing */
      if(pglCurContext->FrontFace == GL_CCW) {
	if(glIntCCW(&(pglCurContext->Vtx1),
		    &(pglCurContext->Vtx2),
		    &(pglCurContext->Vtx3)) >= 0.0f)
	  pglCurContext->CurFrontFace = TRUE;
	else
	  pglCurContext->CurFrontFace = FALSE;
      } else {
	if(glIntCCW(&(pglCurContext->Vtx1),
		    &(pglCurContext->Vtx2),
		    &(pglCurContext->Vtx3)) <= 0.0f)
	  pglCurContext->CurFrontFace = TRUE;
	else
	  pglCurContext->CurFrontFace = FALSE;
      }
      
      pglCurContext->PrimAssemState = PA_TRIANGLES1;
      pglCurContext->CurVtx = &(pglCurContext->Vtx1);

      /* culling */
      if(pglCurContext->CullEnable) {
	if((pglCurContext->CullMode == GL_FRONT) && pglCurContext->CurFrontFace)
	  break;
	if((pglCurContext->CullMode == GL_BACK) && (!pglCurContext->CurFrontFace))
	  break;
      }

//      if(pglCurContext->ShadeModel == GL_FLAT) {
//	glIntLight(&(pglCurContext->Vtx3));
//      } else {
	glIntLight(&(pglCurContext->Vtx1));
	glIntLight(&(pglCurContext->Vtx2));
	glIntLight(&(pglCurContext->Vtx3));
//      }
    } else {
      pglCurContext->PrimAssemState = PA_TRIANGLES1;
      pglCurContext->CurVtx = &(pglCurContext->Vtx1);
    }

    if(pglCurContext->ShadeModel == GL_FLAT) {
      glIntFlat(&(pglCurContext->Vtx3));
    }

    if(pglCurContext->CurFrontFace) {
      switch(pglCurContext->PolygonFrontMode) {
      case GL_POINT:
	glIntDrawPoint(&(pglCurContext->Vtx1));
	glIntDrawPoint(&(pglCurContext->Vtx2));
	glIntDrawPoint(&(pglCurContext->Vtx3));
	break;
      case GL_LINE:
	if(pglCurContext->Vtx1.Edge)
	  glIntDrawLine(&(pglCurContext->Vtx1),
			&(pglCurContext->Vtx2));
	if(pglCurContext->Vtx2.Edge)
	  glIntDrawLine(&(pglCurContext->Vtx2),
			&(pglCurContext->Vtx3));
	if(pglCurContext->Vtx3.Edge)
	  glIntDrawLine(&(pglCurContext->Vtx3),
			&(pglCurContext->Vtx1));
	break;
      case GL_FILL:
	glIntDrawTriangle(&(pglCurContext->Vtx1),
			  &(pglCurContext->Vtx2),
			  &(pglCurContext->Vtx3));
	break;
      default:
	break;
      }
    } else {
      switch(pglCurContext->PolygonBackMode) {
      case GL_POINT:
	glIntDrawPoint(&(pglCurContext->Vtx1));
	glIntDrawPoint(&(pglCurContext->Vtx2));
	glIntDrawPoint(&(pglCurContext->Vtx3));
	break;
      case GL_LINE:
	if(pglCurContext->Vtx1.Edge)
	  glIntDrawLine(&(pglCurContext->Vtx1),
			&(pglCurContext->Vtx2));
	if(pglCurContext->Vtx2.Edge)
	  glIntDrawLine(&(pglCurContext->Vtx2),
			&(pglCurContext->Vtx3));
	if(pglCurContext->Vtx3.Edge)
	  glIntDrawLine(&(pglCurContext->Vtx3),
			&(pglCurContext->Vtx1));
	break;
      case GL_FILL:
	glIntDrawTriangle(&(pglCurContext->Vtx1),
			  &(pglCurContext->Vtx2),
			  &(pglCurContext->Vtx3));
	break;
      default:
	break;
      }
    }
    break;
  case PA_TRIANGLE_STRIP1:
    pglCurContext->PrimAssemState = PA_TRIANGLE_STRIP2;
    pglCurContext->CurVtx = &(pglCurContext->Vtx2);
    break;
  case PA_TRIANGLE_STRIP2:
    pglCurContext->PrimAssemState = PA_TRIANGLE_STRIP3;
    pglCurContext->CurVtx = &(pglCurContext->Vtx3);
    break;
  case PA_TRIANGLE_STRIP3:
    glIntProject(&(pglCurContext->Vtx1));
    glIntProject(&(pglCurContext->Vtx2));
    glIntProject(&(pglCurContext->Vtx3));

    if(pglCurContext->Lighting) {
      /* figure out facing */
      if(pglCurContext->FrontFace == GL_CCW) {
	if(glIntCCW(&(pglCurContext->Vtx1),
		    &(pglCurContext->Vtx2),
		    &(pglCurContext->Vtx3)) >= 0.0f)
	  pglCurContext->CurFrontFace = TRUE;
	else
	  pglCurContext->CurFrontFace = FALSE;
      } else {
	if(glIntCCW(&(pglCurContext->Vtx1),
		    &(pglCurContext->Vtx2),
		    &(pglCurContext->Vtx3)) <= 0.0f)
	  pglCurContext->CurFrontFace = TRUE;
	else
	  pglCurContext->CurFrontFace = FALSE;
      }
      
      pglCurContext->PrimAssemState = PA_TRIANGLE_STRIP4;
      pglCurContext->CurVtx = &(pglCurContext->Vtx1);
      
      /* culling */
      if(pglCurContext->CullEnable) {
	if((pglCurContext->CullMode == GL_FRONT) && pglCurContext->CurFrontFace)
	  break;
	if((pglCurContext->CullMode == GL_BACK) && (!pglCurContext->CurFrontFace))
	  break;
      }

      if(pglCurContext->ShadeModel == GL_FLAT) {
	glIntLight(&(pglCurContext->Vtx3));
      } else {
	glIntLight(&(pglCurContext->Vtx1));
	glIntLight(&(pglCurContext->Vtx2));
	glIntLight(&(pglCurContext->Vtx3));
      }
    } else {
      pglCurContext->PrimAssemState = PA_TRIANGLE_STRIP4;
      pglCurContext->CurVtx = &(pglCurContext->Vtx1);
    }

    if(pglCurContext->ShadeModel == GL_FLAT) {
      glIntFlat(&(pglCurContext->Vtx3));
    }

    if(pglCurContext->CurFrontFace) {
      switch(pglCurContext->PolygonFrontMode) {
      case GL_POINT:
	glIntDrawPoint(&(pglCurContext->Vtx1));
	glIntDrawPoint(&(pglCurContext->Vtx2));
	glIntDrawPoint(&(pglCurContext->Vtx3));
	break;
      case GL_LINE:
	glIntDrawLine(&(pglCurContext->Vtx1),
		      &(pglCurContext->Vtx2));
	glIntDrawLine(&(pglCurContext->Vtx2),
		      &(pglCurContext->Vtx3));
	glIntDrawLine(&(pglCurContext->Vtx3),
		      &(pglCurContext->Vtx1));
	break;
      case GL_FILL:
	glIntDrawTriangle(&(pglCurContext->Vtx1),
			  &(pglCurContext->Vtx2),
			  &(pglCurContext->Vtx3));
	break;
      default:
	break;
      }
    } else {
      switch(pglCurContext->PolygonBackMode) {
      case GL_POINT:
	glIntDrawPoint(&(pglCurContext->Vtx1));
	glIntDrawPoint(&(pglCurContext->Vtx2));
	glIntDrawPoint(&(pglCurContext->Vtx3));
	break;
      case GL_LINE:
	glIntDrawLine(&(pglCurContext->Vtx1),
		      &(pglCurContext->Vtx2));
	glIntDrawLine(&(pglCurContext->Vtx2),
		      &(pglCurContext->Vtx3));
	glIntDrawLine(&(pglCurContext->Vtx3),
		      &(pglCurContext->Vtx1));
	break;
      case GL_FILL:
	glIntDrawTriangle(&(pglCurContext->Vtx1),
			  &(pglCurContext->Vtx2),
			  &(pglCurContext->Vtx3));
	break;
      default:
	break;
      }
    }
    break;
  case PA_TRIANGLE_STRIP4:
    glIntProject(&(pglCurContext->Vtx1));
    glIntProject(&(pglCurContext->Vtx2));
    glIntProject(&(pglCurContext->Vtx3));

    if(pglCurContext->Lighting) {
      /* figure out facing */
      if(pglCurContext->FrontFace == GL_CCW) {
	if(glIntCCW(&(pglCurContext->Vtx2),
		    &(pglCurContext->Vtx1),
		    &(pglCurContext->Vtx3)) >= 0.0f)
	  pglCurContext->CurFrontFace = TRUE;
	else
	  pglCurContext->CurFrontFace = FALSE;
      } else {
	if(glIntCCW(&(pglCurContext->Vtx2),
		    &(pglCurContext->Vtx1),
		    &(pglCurContext->Vtx3)) <= 0.0f)
	  pglCurContext->CurFrontFace = TRUE;
	else
	  pglCurContext->CurFrontFace = FALSE;
      }
      
      pglCurContext->PrimAssemState = PA_TRIANGLE_STRIP5;
      pglCurContext->CurVtx = &(pglCurContext->Vtx2);
      
      /* culling */
      if(pglCurContext->CullEnable) {
	if((pglCurContext->CullMode == GL_FRONT) && pglCurContext->CurFrontFace)
	  break;
	if((pglCurContext->CullMode == GL_BACK) && (!pglCurContext->CurFrontFace))
	  break;
      }

      if(pglCurContext->ShadeModel == GL_FLAT) {
	glIntLight(&(pglCurContext->Vtx1));
      } else {
	glIntLight(&(pglCurContext->Vtx1));
	glIntLight(&(pglCurContext->Vtx2));
	glIntLight(&(pglCurContext->Vtx3));
      }
    } else {
      pglCurContext->PrimAssemState = PA_TRIANGLE_STRIP5;
      pglCurContext->CurVtx = &(pglCurContext->Vtx2);
    }

    if(pglCurContext->ShadeModel == GL_FLAT) {
      glIntFlat(&(pglCurContext->Vtx1));
    }

    if(pglCurContext->CurFrontFace) {
      switch(pglCurContext->PolygonFrontMode) {
      case GL_POINT:
	glIntDrawPoint(&(pglCurContext->Vtx1));
	break;
      case GL_LINE:
	glIntDrawLine(&(pglCurContext->Vtx2),
		      &(pglCurContext->Vtx1));
	glIntDrawLine(&(pglCurContext->Vtx1),
		      &(pglCurContext->Vtx3));
	break;
      case GL_FILL:
	glIntDrawTriangle(&(pglCurContext->Vtx2),
			  &(pglCurContext->Vtx1),
			  &(pglCurContext->Vtx3));
	break;
      default:
	break;
      }
    } else {
      switch(pglCurContext->PolygonBackMode) {
      case GL_POINT:
	glIntDrawPoint(&(pglCurContext->Vtx1));
	break;
      case GL_LINE:
	glIntDrawLine(&(pglCurContext->Vtx2),
		      &(pglCurContext->Vtx1));
	glIntDrawLine(&(pglCurContext->Vtx1),
		      &(pglCurContext->Vtx3));
	break;
      case GL_FILL:
	glIntDrawTriangle(&(pglCurContext->Vtx2),
			  &(pglCurContext->Vtx1),
			  &(pglCurContext->Vtx3));
	break;
      default:
	break;
      }
    }
    break;
  case PA_TRIANGLE_STRIP5:
    glIntProject(&(pglCurContext->Vtx1));
    glIntProject(&(pglCurContext->Vtx2));
    glIntProject(&(pglCurContext->Vtx3));

    if(pglCurContext->Lighting) {
      /* figure out facing */
      if(pglCurContext->FrontFace == GL_CCW) {
	if(glIntCCW(&(pglCurContext->Vtx1),
		    &(pglCurContext->Vtx2),
		    &(pglCurContext->Vtx3)) >= 0.0f)
	  pglCurContext->CurFrontFace = TRUE;
	else
	  pglCurContext->CurFrontFace = FALSE;
      } else {
	if(glIntCCW(&(pglCurContext->Vtx1),
		    &(pglCurContext->Vtx2),
		    &(pglCurContext->Vtx3)) <= 0.0f)
	  pglCurContext->CurFrontFace = TRUE;
	else
	  pglCurContext->CurFrontFace = FALSE;
      }
      
      pglCurContext->PrimAssemState = PA_TRIANGLE_STRIP6;
      pglCurContext->CurVtx = &(pglCurContext->Vtx3);
      
      /* culling */
      if(pglCurContext->CullEnable) {
	if((pglCurContext->CullMode == GL_FRONT) && pglCurContext->CurFrontFace)
	  break;
	if((pglCurContext->CullMode == GL_BACK) && (!pglCurContext->CurFrontFace))
	  break;
      }

      if(pglCurContext->ShadeModel == GL_FLAT) {
	glIntLight(&(pglCurContext->Vtx2));
      } else {
	glIntLight(&(pglCurContext->Vtx1));
	glIntLight(&(pglCurContext->Vtx2));
	glIntLight(&(pglCurContext->Vtx3));
      }
    } else {
      pglCurContext->PrimAssemState = PA_TRIANGLE_STRIP6;
      pglCurContext->CurVtx = &(pglCurContext->Vtx3);
    }

    if(pglCurContext->ShadeModel == GL_FLAT) {
      glIntFlat(&(pglCurContext->Vtx2));
    }

    if(pglCurContext->CurFrontFace) {
      switch(pglCurContext->PolygonFrontMode) {
      case GL_POINT:
	glIntDrawPoint(&(pglCurContext->Vtx2));
	break;
      case GL_LINE:
	glIntDrawLine(&(pglCurContext->Vtx1),
		      &(pglCurContext->Vtx2));
	glIntDrawLine(&(pglCurContext->Vtx2),
		      &(pglCurContext->Vtx3));
	break;
      case GL_FILL:
	glIntDrawTriangle(&(pglCurContext->Vtx1),
			  &(pglCurContext->Vtx2),
			  &(pglCurContext->Vtx3));
	break;
      default:
	break;
      }
    } else {
      switch(pglCurContext->PolygonBackMode) {
      case GL_POINT:
	glIntDrawPoint(&(pglCurContext->Vtx2));
	break;
      case GL_LINE:
	glIntDrawLine(&(pglCurContext->Vtx1),
		      &(pglCurContext->Vtx2));
	glIntDrawLine(&(pglCurContext->Vtx2),
		      &(pglCurContext->Vtx3));
	break;
      case GL_FILL:
	glIntDrawTriangle(&(pglCurContext->Vtx1),
			  &(pglCurContext->Vtx2),
			  &(pglCurContext->Vtx3));
	break;
      default:
	break;
      }
    }
    break;
  case PA_TRIANGLE_STRIP6:
    glIntProject(&(pglCurContext->Vtx1));
    glIntProject(&(pglCurContext->Vtx2));
    glIntProject(&(pglCurContext->Vtx3));

    if(pglCurContext->Lighting) {
      /* figure out facing */
      if(pglCurContext->FrontFace == GL_CCW) {
	if(glIntCCW(&(pglCurContext->Vtx2),
		    &(pglCurContext->Vtx1),
		    &(pglCurContext->Vtx3)) >= 0.0f)
	  pglCurContext->CurFrontFace = TRUE;
	else
	  pglCurContext->CurFrontFace = FALSE;
      } else {
	if(glIntCCW(&(pglCurContext->Vtx2),
		    &(pglCurContext->Vtx1),
		    &(pglCurContext->Vtx3)) <= 0.0f)
	  pglCurContext->CurFrontFace = TRUE;
	else
	  pglCurContext->CurFrontFace = FALSE;
      }
      
      pglCurContext->PrimAssemState = PA_TRIANGLE_STRIP7;
      pglCurContext->CurVtx = &(pglCurContext->Vtx1);
      
      /* culling */
      if(pglCurContext->CullEnable) {
	if((pglCurContext->CullMode == GL_FRONT) && pglCurContext->CurFrontFace)
	  break;
	if((pglCurContext->CullMode == GL_BACK) && (!pglCurContext->CurFrontFace))
	  break;
      }

      if(pglCurContext->ShadeModel == GL_FLAT) {
	glIntLight(&(pglCurContext->Vtx3));
      } else {
	glIntLight(&(pglCurContext->Vtx1));
	glIntLight(&(pglCurContext->Vtx2));
	glIntLight(&(pglCurContext->Vtx3));
      }
    } else {
      pglCurContext->PrimAssemState = PA_TRIANGLE_STRIP7;
      pglCurContext->CurVtx = &(pglCurContext->Vtx1);
    }

    if(pglCurContext->ShadeModel == GL_FLAT) {
      glIntFlat(&(pglCurContext->Vtx3));
    }

    if(pglCurContext->CurFrontFace) {
      switch(pglCurContext->PolygonFrontMode) {
      case GL_POINT:
	glIntDrawPoint(&(pglCurContext->Vtx3));
	break;
      case GL_LINE:
	glIntDrawLine(&(pglCurContext->Vtx1),
		      &(pglCurContext->Vtx3));
	glIntDrawLine(&(pglCurContext->Vtx3),
		      &(pglCurContext->Vtx2));
	break;
      case GL_FILL:
	glIntDrawTriangle(&(pglCurContext->Vtx2),
			  &(pglCurContext->Vtx1),
			  &(pglCurContext->Vtx3));
	break;
      default:
	break;
      }
    } else {
      switch(pglCurContext->PolygonBackMode) {
      case GL_POINT:
	glIntDrawPoint(&(pglCurContext->Vtx3));
	break;
      case GL_LINE:
	glIntDrawLine(&(pglCurContext->Vtx1),
		      &(pglCurContext->Vtx3));
	glIntDrawLine(&(pglCurContext->Vtx3),
		      &(pglCurContext->Vtx2));
	break;
      case GL_FILL:
	glIntDrawTriangle(&(pglCurContext->Vtx2),
			  &(pglCurContext->Vtx1),
			  &(pglCurContext->Vtx3));
	break;
      default:
	break;
      }
    }
    break;
  case PA_TRIANGLE_STRIP7:
    glIntProject(&(pglCurContext->Vtx1));
    glIntProject(&(pglCurContext->Vtx2));
    glIntProject(&(pglCurContext->Vtx3));

    if(pglCurContext->Lighting) {
      /* figure out facing */
      if(pglCurContext->FrontFace == GL_CCW) {
	if(glIntCCW(&(pglCurContext->Vtx1),
		    &(pglCurContext->Vtx2),
		    &(pglCurContext->Vtx3)) >= 0.0f)
	  pglCurContext->CurFrontFace = TRUE;
	else
	  pglCurContext->CurFrontFace = FALSE;
      } else {
	if(glIntCCW(&(pglCurContext->Vtx1),
		    &(pglCurContext->Vtx2),
		    &(pglCurContext->Vtx3)) <= 0.0f)
	  pglCurContext->CurFrontFace = TRUE;
	else
	  pglCurContext->CurFrontFace = FALSE;
      }
      
      pglCurContext->PrimAssemState = PA_TRIANGLE_STRIP8;
      pglCurContext->CurVtx = &(pglCurContext->Vtx2);
      
      /* culling */
      if(pglCurContext->CullEnable) {
	if((pglCurContext->CullMode == GL_FRONT) && pglCurContext->CurFrontFace)
	  break;
	if((pglCurContext->CullMode == GL_BACK) && (!pglCurContext->CurFrontFace))
	  break;
      }

      if(pglCurContext->ShadeModel == GL_FLAT) {
	glIntLight(&(pglCurContext->Vtx1));
      } else {
	glIntLight(&(pglCurContext->Vtx1));
	glIntLight(&(pglCurContext->Vtx2));
	glIntLight(&(pglCurContext->Vtx3));
      }
    } else {
      pglCurContext->PrimAssemState = PA_TRIANGLE_STRIP8;
      pglCurContext->CurVtx = &(pglCurContext->Vtx2);
    }

    if(pglCurContext->ShadeModel == GL_FLAT) {
      glIntFlat(&(pglCurContext->Vtx1));
    }

    if(pglCurContext->CurFrontFace) {
      switch(pglCurContext->PolygonFrontMode) {
      case GL_POINT:
	glIntDrawPoint(&(pglCurContext->Vtx1));
	break;
      case GL_LINE:
	glIntDrawLine(&(pglCurContext->Vtx3),
		      &(pglCurContext->Vtx1));
	glIntDrawLine(&(pglCurContext->Vtx1),
		      &(pglCurContext->Vtx2));
	break;
      case GL_FILL:
	glIntDrawTriangle(&(pglCurContext->Vtx1),
			  &(pglCurContext->Vtx2),
			  &(pglCurContext->Vtx3));
	break;
      default:
	break;
      }
    } else {
      switch(pglCurContext->PolygonBackMode) {
      case GL_POINT:
	glIntDrawPoint(&(pglCurContext->Vtx1));
	break;
      case GL_LINE:
	glIntDrawLine(&(pglCurContext->Vtx3),
		      &(pglCurContext->Vtx1));
	glIntDrawLine(&(pglCurContext->Vtx1),
		      &(pglCurContext->Vtx2));
	break;
      case GL_FILL:
	glIntDrawTriangle(&(pglCurContext->Vtx1),
			  &(pglCurContext->Vtx2),
			  &(pglCurContext->Vtx3));
	break;
      default:
	break;
      }
    }
    break;
  case PA_TRIANGLE_STRIP8:
    glIntProject(&(pglCurContext->Vtx1));
    glIntProject(&(pglCurContext->Vtx2));
    glIntProject(&(pglCurContext->Vtx3));

    if(pglCurContext->Lighting) {
      /* figure out facing */
      if(pglCurContext->FrontFace == GL_CCW) {
	if(glIntCCW(&(pglCurContext->Vtx2),
		    &(pglCurContext->Vtx1),
		    &(pglCurContext->Vtx3)) >= 0.0f)
	  pglCurContext->CurFrontFace = TRUE;
	else
	  pglCurContext->CurFrontFace = FALSE;
      } else {
	if(glIntCCW(&(pglCurContext->Vtx2),
		    &(pglCurContext->Vtx1),
		    &(pglCurContext->Vtx3)) <= 0.0f)
	  pglCurContext->CurFrontFace = TRUE;
	else
	  pglCurContext->CurFrontFace = FALSE;
      }
      
      pglCurContext->PrimAssemState = PA_TRIANGLE_STRIP9;
      pglCurContext->CurVtx = &(pglCurContext->Vtx3);
      
      /* culling */
      if(pglCurContext->CullEnable) {
	if((pglCurContext->CullMode == GL_FRONT) && pglCurContext->CurFrontFace)
	  break;
	if((pglCurContext->CullMode == GL_BACK) && (!pglCurContext->CurFrontFace))
	  break;
      }

      if(pglCurContext->ShadeModel == GL_FLAT) {
	glIntLight(&(pglCurContext->Vtx2));
      } else {
	glIntLight(&(pglCurContext->Vtx1));
	glIntLight(&(pglCurContext->Vtx2));
	glIntLight(&(pglCurContext->Vtx3));
      }
    } else {
      pglCurContext->PrimAssemState = PA_TRIANGLE_STRIP9;
      pglCurContext->CurVtx = &(pglCurContext->Vtx3);
    }

    if(pglCurContext->ShadeModel == GL_FLAT) {
      glIntFlat(&(pglCurContext->Vtx2));
    }

    if(pglCurContext->CurFrontFace) {
      switch(pglCurContext->PolygonFrontMode) {
      case GL_POINT:
	glIntDrawPoint(&(pglCurContext->Vtx2));
	break;
      case GL_LINE:
	glIntDrawLine(&(pglCurContext->Vtx3),
		      &(pglCurContext->Vtx2));
	glIntDrawLine(&(pglCurContext->Vtx2),
		      &(pglCurContext->Vtx1));
	break;
      case GL_FILL:
	glIntDrawTriangle(&(pglCurContext->Vtx2),
			  &(pglCurContext->Vtx1),
			  &(pglCurContext->Vtx3));
	break;
      default:
	break;
      }
    } else {
      switch(pglCurContext->PolygonBackMode) {
      case GL_POINT:
	glIntDrawPoint(&(pglCurContext->Vtx2));
	break;
      case GL_LINE:
	glIntDrawLine(&(pglCurContext->Vtx3),
		      &(pglCurContext->Vtx2));
	glIntDrawLine(&(pglCurContext->Vtx2),
		      &(pglCurContext->Vtx1));
	break;
      case GL_FILL:
	glIntDrawTriangle(&(pglCurContext->Vtx2),
			  &(pglCurContext->Vtx1),
			  &(pglCurContext->Vtx3));
	break;
      default:
	break;
      }
    }
    break;
  case PA_TRIANGLE_STRIP9:
    glIntProject(&(pglCurContext->Vtx1));
    glIntProject(&(pglCurContext->Vtx2));
    glIntProject(&(pglCurContext->Vtx3));

    if(pglCurContext->Lighting) {
      /* figure out facing */
      if(pglCurContext->FrontFace == GL_CCW) {
	if(glIntCCW(&(pglCurContext->Vtx1),
		    &(pglCurContext->Vtx2),
		    &(pglCurContext->Vtx3)) >= 0.0f)
	  pglCurContext->CurFrontFace = TRUE;
	else
	  pglCurContext->CurFrontFace = FALSE;
      } else {
	if(glIntCCW(&(pglCurContext->Vtx1),
		    &(pglCurContext->Vtx2),
		    &(pglCurContext->Vtx3)) <= 0.0f)
	  pglCurContext->CurFrontFace = TRUE;
	else
	  pglCurContext->CurFrontFace = FALSE;
      }
      
      pglCurContext->PrimAssemState = PA_TRIANGLE_STRIP4;
      pglCurContext->CurVtx = &(pglCurContext->Vtx1);
      
      /* culling */
      if(pglCurContext->CullEnable) {
	if((pglCurContext->CullMode == GL_FRONT) && pglCurContext->CurFrontFace)
	  break;
	if((pglCurContext->CullMode == GL_BACK) && (!pglCurContext->CurFrontFace))
	  break;
      }

      if(pglCurContext->ShadeModel == GL_FLAT) {
	glIntLight(&(pglCurContext->Vtx3));
      } else {
	glIntLight(&(pglCurContext->Vtx1));
	glIntLight(&(pglCurContext->Vtx2));
	glIntLight(&(pglCurContext->Vtx3));
      }
    } else {
      pglCurContext->PrimAssemState = PA_TRIANGLE_STRIP4;
      pglCurContext->CurVtx = &(pglCurContext->Vtx1);
    }
    
    if(pglCurContext->ShadeModel == GL_FLAT) {
      glIntFlat(&(pglCurContext->Vtx3));
    }

    if(pglCurContext->CurFrontFace) {
      switch(pglCurContext->PolygonFrontMode) {
      case GL_POINT:
	glIntDrawPoint(&(pglCurContext->Vtx3));
	break;
      case GL_LINE:
	glIntDrawLine(&(pglCurContext->Vtx2),
		      &(pglCurContext->Vtx3));
	glIntDrawLine(&(pglCurContext->Vtx3),
		      &(pglCurContext->Vtx1));
	break;
      case GL_FILL:
	glIntDrawTriangle(&(pglCurContext->Vtx1),
			  &(pglCurContext->Vtx2),
			  &(pglCurContext->Vtx3));
	break;
      default:
	break;
      }
    } else {
      switch(pglCurContext->PolygonBackMode) {
      case GL_POINT:
	glIntDrawPoint(&(pglCurContext->Vtx3));
	break;
      case GL_LINE:
	glIntDrawLine(&(pglCurContext->Vtx2),
		      &(pglCurContext->Vtx3));
	glIntDrawLine(&(pglCurContext->Vtx3),
		      &(pglCurContext->Vtx1));
	break;
      case GL_FILL:
	glIntDrawTriangle(&(pglCurContext->Vtx1),
			  &(pglCurContext->Vtx2),
			  &(pglCurContext->Vtx3));
	break;
      default:
	break;
      }
    }
    break;
  case PA_TRIANGLE_FAN1:
    pglCurContext->PrimAssemState = PA_TRIANGLE_FAN2;
    pglCurContext->CurVtx = &(pglCurContext->Vtx2);
    break;
  case PA_TRIANGLE_FAN2:
    pglCurContext->PrimAssemState = PA_TRIANGLE_FAN3;
    pglCurContext->CurVtx = &(pglCurContext->Vtx3);
    break;
  case PA_TRIANGLE_FAN3:
    glIntProject(&(pglCurContext->Vtx1));
    glIntProject(&(pglCurContext->Vtx2));
    glIntProject(&(pglCurContext->Vtx3));

    if(pglCurContext->Lighting) {
      /* figure out facing */
      if(pglCurContext->FrontFace == GL_CCW) {
	if(glIntCCW(&(pglCurContext->Vtx1),
		    &(pglCurContext->Vtx2),
		    &(pglCurContext->Vtx3)) >= 0.0f)
	  pglCurContext->CurFrontFace = TRUE;
	else
	  pglCurContext->CurFrontFace = FALSE;
      } else {
	if(glIntCCW(&(pglCurContext->Vtx1),
		    &(pglCurContext->Vtx2),
		    &(pglCurContext->Vtx3)) <= 0.0f)
	  pglCurContext->CurFrontFace = TRUE;
	else
	  pglCurContext->CurFrontFace = FALSE;
      }
      
      pglCurContext->PrimAssemState = PA_TRIANGLE_FAN4;
      pglCurContext->CurVtx = &(pglCurContext->Vtx2);
      
      /* culling */
      if(pglCurContext->CullEnable) {
	if((pglCurContext->CullMode == GL_FRONT) && pglCurContext->CurFrontFace)
	  break;
	if((pglCurContext->CullMode == GL_BACK) && (!pglCurContext->CurFrontFace))
	  break;
      }

      if(pglCurContext->ShadeModel == GL_FLAT) {
	glIntLight(&(pglCurContext->Vtx3));
      } else {
	glIntLight(&(pglCurContext->Vtx1));
	glIntLight(&(pglCurContext->Vtx2));
	glIntLight(&(pglCurContext->Vtx3));
      }
    } else {
      pglCurContext->PrimAssemState = PA_TRIANGLE_FAN4;
      pglCurContext->CurVtx = &(pglCurContext->Vtx2);
    }
    
    if(pglCurContext->ShadeModel == GL_FLAT) {
      glIntFlat(&(pglCurContext->Vtx3));
    }

    if(pglCurContext->CurFrontFace) {
      switch(pglCurContext->PolygonFrontMode) {
      case GL_POINT:
	glIntDrawPoint(&(pglCurContext->Vtx1));
	glIntDrawPoint(&(pglCurContext->Vtx2));
	glIntDrawPoint(&(pglCurContext->Vtx3));
	break;
      case GL_LINE:
	glIntDrawLine(&(pglCurContext->Vtx1),
		      &(pglCurContext->Vtx2));
	glIntDrawLine(&(pglCurContext->Vtx2),
		      &(pglCurContext->Vtx3));
	glIntDrawLine(&(pglCurContext->Vtx3),
		      &(pglCurContext->Vtx1));
	break;
      case GL_FILL:
	glIntDrawTriangle(&(pglCurContext->Vtx1),
			  &(pglCurContext->Vtx2),
			  &(pglCurContext->Vtx3));
	break;
      default:
	break;
      }
    } else {
      switch(pglCurContext->PolygonBackMode) {
      case GL_POINT:
	glIntDrawPoint(&(pglCurContext->Vtx1));
	glIntDrawPoint(&(pglCurContext->Vtx2));
	glIntDrawPoint(&(pglCurContext->Vtx3));
	break;
      case GL_LINE:
	glIntDrawLine(&(pglCurContext->Vtx1),
		      &(pglCurContext->Vtx2));
	glIntDrawLine(&(pglCurContext->Vtx2),
		      &(pglCurContext->Vtx3));
	glIntDrawLine(&(pglCurContext->Vtx3),
		      &(pglCurContext->Vtx1));
	break;
      case GL_FILL:
	glIntDrawTriangle(&(pglCurContext->Vtx1),
			  &(pglCurContext->Vtx2),
			  &(pglCurContext->Vtx3));
	break;
      default:
	break;
      }
    }
    break;
  case PA_TRIANGLE_FAN4:
    glIntProject(&(pglCurContext->Vtx1));
    glIntProject(&(pglCurContext->Vtx2));
    glIntProject(&(pglCurContext->Vtx3));

    if(pglCurContext->Lighting) {
      /* figure out facing */
      if(pglCurContext->FrontFace == GL_CCW) {
	if(glIntCCW(&(pglCurContext->Vtx1),
		    &(pglCurContext->Vtx3),
		    &(pglCurContext->Vtx2)) >= 0.0f)
	  pglCurContext->CurFrontFace = TRUE;
	else
	  pglCurContext->CurFrontFace = FALSE;
      } else {
	if(glIntCCW(&(pglCurContext->Vtx1),
		    &(pglCurContext->Vtx3),
		    &(pglCurContext->Vtx2)) <= 0.0f)
	  pglCurContext->CurFrontFace = TRUE;
	else
	  pglCurContext->CurFrontFace = FALSE;
      }
      
      pglCurContext->PrimAssemState = PA_TRIANGLE_FAN5;
      pglCurContext->CurVtx = &(pglCurContext->Vtx3);
      
      /* culling */
      if(pglCurContext->CullEnable) {
	if((pglCurContext->CullMode == GL_FRONT) && pglCurContext->CurFrontFace)
	  break;
	if((pglCurContext->CullMode == GL_BACK) && (!pglCurContext->CurFrontFace))
	  break;
      }

      if(pglCurContext->ShadeModel == GL_FLAT) {
	glIntLight(&(pglCurContext->Vtx2));
      } else {
	glIntLight(&(pglCurContext->Vtx1));
	glIntLight(&(pglCurContext->Vtx2));
	glIntLight(&(pglCurContext->Vtx3));
      }
    } else {
      pglCurContext->PrimAssemState = PA_TRIANGLE_FAN5;
      pglCurContext->CurVtx = &(pglCurContext->Vtx3);
    }

    if(pglCurContext->ShadeModel == GL_FLAT) {
      glIntFlat(&(pglCurContext->Vtx2));
    }

    if(pglCurContext->CurFrontFace) {
      switch(pglCurContext->PolygonFrontMode) {
      case GL_POINT:
	glIntDrawPoint(&(pglCurContext->Vtx2));
	break;
      case GL_LINE:
	glIntDrawLine(&(pglCurContext->Vtx3),
		      &(pglCurContext->Vtx2));
	glIntDrawLine(&(pglCurContext->Vtx2),
		      &(pglCurContext->Vtx1));
	break;
      case GL_FILL:
	glIntDrawTriangle(&(pglCurContext->Vtx1),
			  &(pglCurContext->Vtx3),
			  &(pglCurContext->Vtx2));
	break;
      default:
	break;
      }
    } else {
      switch(pglCurContext->PolygonBackMode) {
      case GL_POINT:
	glIntDrawPoint(&(pglCurContext->Vtx2));
	break;
      case GL_LINE:
	glIntDrawLine(&(pglCurContext->Vtx3),
		      &(pglCurContext->Vtx2));
	glIntDrawLine(&(pglCurContext->Vtx2),
		      &(pglCurContext->Vtx1));
	break;
      case GL_FILL:
	glIntDrawTriangle(&(pglCurContext->Vtx1),
			  &(pglCurContext->Vtx3),
			  &(pglCurContext->Vtx2));
	break;
      default:
	break;
      }
    }
    break;
  case PA_TRIANGLE_FAN5:
    glIntProject(&(pglCurContext->Vtx1));
    glIntProject(&(pglCurContext->Vtx2));
    glIntProject(&(pglCurContext->Vtx3));

    if(pglCurContext->Lighting) {
      /* figure out facing */
      if(pglCurContext->FrontFace == GL_CCW) {
	if(glIntCCW(&(pglCurContext->Vtx1),
		    &(pglCurContext->Vtx2),
		    &(pglCurContext->Vtx3)) >= 0.0f)
	  pglCurContext->CurFrontFace = TRUE;
	else
	  pglCurContext->CurFrontFace = FALSE;
      } else {
	if(glIntCCW(&(pglCurContext->Vtx1),
		    &(pglCurContext->Vtx2),
		    &(pglCurContext->Vtx3)) <= 0.0f)
	  pglCurContext->CurFrontFace = TRUE;
	else
	  pglCurContext->CurFrontFace = FALSE;
      }
      
      pglCurContext->PrimAssemState = PA_TRIANGLE_FAN4;
      pglCurContext->CurVtx = &(pglCurContext->Vtx2);
      
      /* culling */
      if(pglCurContext->CullEnable) {
	if((pglCurContext->CullMode == GL_FRONT) && pglCurContext->CurFrontFace)
	  break;
	if((pglCurContext->CullMode == GL_BACK) && (!pglCurContext->CurFrontFace))
	  break;
      }

      if(pglCurContext->ShadeModel == GL_FLAT) {
	glIntLight(&(pglCurContext->Vtx3));
      } else {
	glIntLight(&(pglCurContext->Vtx1));
	glIntLight(&(pglCurContext->Vtx2));
	glIntLight(&(pglCurContext->Vtx3));
      }
    } else {
      pglCurContext->PrimAssemState = PA_TRIANGLE_FAN4;
      pglCurContext->CurVtx = &(pglCurContext->Vtx2);
    }
    
    if(pglCurContext->ShadeModel == GL_FLAT) {
      glIntFlat(&(pglCurContext->Vtx3));
    }

    if(pglCurContext->CurFrontFace) {
      switch(pglCurContext->PolygonFrontMode) {
      case GL_POINT:
	glIntDrawPoint(&(pglCurContext->Vtx3));
	break;
      case GL_LINE:
	glIntDrawLine(&(pglCurContext->Vtx2),
		      &(pglCurContext->Vtx3));
	glIntDrawLine(&(pglCurContext->Vtx3),
		      &(pglCurContext->Vtx1));
	break;
      case GL_FILL:
	glIntDrawTriangle(&(pglCurContext->Vtx1),
			  &(pglCurContext->Vtx2),
			  &(pglCurContext->Vtx3));
	break;
      default:
	break;
      }
    } else {
      switch(pglCurContext->PolygonBackMode) {
      case GL_POINT:
	glIntDrawPoint(&(pglCurContext->Vtx3));
	break;
      case GL_LINE:
	glIntDrawLine(&(pglCurContext->Vtx2),
		      &(pglCurContext->Vtx3));
	glIntDrawLine(&(pglCurContext->Vtx3),
		      &(pglCurContext->Vtx1));
	break;
      case GL_FILL:
	glIntDrawTriangle(&(pglCurContext->Vtx1),
			  &(pglCurContext->Vtx2),
			  &(pglCurContext->Vtx3));
	break;
      default:
	break;
      }
    }
    break;
  case PA_QUADS1:
    pglCurContext->PrimAssemState = PA_QUADS2;
    pglCurContext->CurVtx = &(pglCurContext->Vtx2);
    break;
  case PA_QUADS2:
    pglCurContext->PrimAssemState = PA_QUADS3;
    pglCurContext->CurVtx = &(pglCurContext->Vtx3);
    break;
  case PA_QUADS3:
    pglCurContext->PrimAssemState = PA_QUADS4;
    pglCurContext->CurVtx = &(pglCurContext->Vtx4);
    break;
  case PA_QUADS4:
    glIntProject(&(pglCurContext->Vtx1));
    glIntProject(&(pglCurContext->Vtx2));
    glIntProject(&(pglCurContext->Vtx3));
    glIntProject(&(pglCurContext->Vtx4));

    if(pglCurContext->Lighting) {
      /* figure out facing */
      if(pglCurContext->FrontFace == GL_CCW) {
	if(glIntCCW(&(pglCurContext->Vtx1),
		    &(pglCurContext->Vtx2),
		    &(pglCurContext->Vtx3)) >= 0.0f)
	  pglCurContext->CurFrontFace = TRUE;
	else
	  pglCurContext->CurFrontFace = FALSE;
      } else {
	if(glIntCCW(&(pglCurContext->Vtx1),
		    &(pglCurContext->Vtx2),
		    &(pglCurContext->Vtx3)) <= 0.0f)
	  pglCurContext->CurFrontFace = TRUE;
	else
	  pglCurContext->CurFrontFace = FALSE;
      }
      
      pglCurContext->PrimAssemState = PA_QUADS1;
      pglCurContext->CurVtx = &(pglCurContext->Vtx1);
      
      /* culling */
      if(pglCurContext->CullEnable) {
	if((pglCurContext->CullMode == GL_FRONT) && pglCurContext->CurFrontFace)
	  break;
	if((pglCurContext->CullMode == GL_BACK) && (!pglCurContext->CurFrontFace))
	  break;
      }

      if(pglCurContext->ShadeModel == GL_FLAT) {
	glIntLight(&(pglCurContext->Vtx4));
      } else {
	glIntLight(&(pglCurContext->Vtx1));
	glIntLight(&(pglCurContext->Vtx2));
	glIntLight(&(pglCurContext->Vtx3));
	glIntLight(&(pglCurContext->Vtx4));
      }
    } else {
      pglCurContext->PrimAssemState = PA_QUADS1;
      pglCurContext->CurVtx = &(pglCurContext->Vtx1);
    }
    
    if(pglCurContext->ShadeModel == GL_FLAT) {
      glIntFlat(&(pglCurContext->Vtx4));
    }

    if(pglCurContext->CurFrontFace) {
      switch(pglCurContext->PolygonFrontMode) {
      case GL_POINT:
	glIntDrawPoint(&(pglCurContext->Vtx1));
	glIntDrawPoint(&(pglCurContext->Vtx2));
	glIntDrawPoint(&(pglCurContext->Vtx3));
	glIntDrawPoint(&(pglCurContext->Vtx4));
	break;
      case GL_LINE:
	if(pglCurContext->Vtx1.Edge)
	  glIntDrawLine(&(pglCurContext->Vtx1),
			&(pglCurContext->Vtx2));
	if(pglCurContext->Vtx2.Edge)
	  glIntDrawLine(&(pglCurContext->Vtx2),
			&(pglCurContext->Vtx3));
	if(pglCurContext->Vtx3.Edge)
	  glIntDrawLine(&(pglCurContext->Vtx3),
			&(pglCurContext->Vtx4));
	if(pglCurContext->Vtx4.Edge)
	  glIntDrawLine(&(pglCurContext->Vtx4),
			&(pglCurContext->Vtx1));
	break;
      case GL_FILL:
	glIntDrawTriangle(&(pglCurContext->Vtx1),
			  &(pglCurContext->Vtx2),
			  &(pglCurContext->Vtx3));
	glIntDrawTriangle(&(pglCurContext->Vtx1),
			  &(pglCurContext->Vtx3),
			  &(pglCurContext->Vtx4));
	break;
      default:
	break;
      }
    } else {
      switch(pglCurContext->PolygonBackMode) {
      case GL_POINT:
	glIntDrawPoint(&(pglCurContext->Vtx1));
	glIntDrawPoint(&(pglCurContext->Vtx2));
	glIntDrawPoint(&(pglCurContext->Vtx3));
	glIntDrawPoint(&(pglCurContext->Vtx4));
	break;
      case GL_LINE:
	if(pglCurContext->Vtx1.Edge)
	  glIntDrawLine(&(pglCurContext->Vtx1),
			&(pglCurContext->Vtx2));
	if(pglCurContext->Vtx2.Edge)
	  glIntDrawLine(&(pglCurContext->Vtx2),
			&(pglCurContext->Vtx3));
	if(pglCurContext->Vtx3.Edge)
	  glIntDrawLine(&(pglCurContext->Vtx3),
			&(pglCurContext->Vtx4));
	if(pglCurContext->Vtx4.Edge)
	  glIntDrawLine(&(pglCurContext->Vtx4),
			&(pglCurContext->Vtx1));
	break;
      case GL_FILL:
	glIntDrawTriangle(&(pglCurContext->Vtx1),
			  &(pglCurContext->Vtx2),
			  &(pglCurContext->Vtx3));
	glIntDrawTriangle(&(pglCurContext->Vtx1),
			  &(pglCurContext->Vtx3),
			  &(pglCurContext->Vtx4));
	break;
      default:
	break;
      }
    }
    break;
  case PA_QUAD_STRIP1:
    pglCurContext->PrimAssemState = PA_QUAD_STRIP2;
    pglCurContext->CurVtx = &(pglCurContext->Vtx2);
    break;
  case PA_QUAD_STRIP2:
    pglCurContext->PrimAssemState = PA_QUAD_STRIP3;
    pglCurContext->CurVtx = &(pglCurContext->Vtx3);
    break;
  case PA_QUAD_STRIP3:
    pglCurContext->PrimAssemState = PA_QUAD_STRIP4;
    pglCurContext->CurVtx = &(pglCurContext->Vtx4);
    break;
  case PA_QUAD_STRIP4:
    glIntProject(&(pglCurContext->Vtx1));
    glIntProject(&(pglCurContext->Vtx2));
    glIntProject(&(pglCurContext->Vtx3));
    glIntProject(&(pglCurContext->Vtx4));

    if(pglCurContext->Lighting) {
      /* figure out facing */
      if(pglCurContext->FrontFace == GL_CCW) {
	if(glIntCCW(&(pglCurContext->Vtx1),
		    &(pglCurContext->Vtx2),
		    &(pglCurContext->Vtx4)) >= 0.0f)
	  pglCurContext->CurFrontFace = TRUE;
	else
	  pglCurContext->CurFrontFace = FALSE;
      } else {
	if(glIntCCW(&(pglCurContext->Vtx1),
		    &(pglCurContext->Vtx2),
		    &(pglCurContext->Vtx4)) <= 0.0f)
	  pglCurContext->CurFrontFace = TRUE;
	else
	  pglCurContext->CurFrontFace = FALSE;
      }
      
      pglCurContext->PrimAssemState = PA_QUAD_STRIP5;
      pglCurContext->CurVtx = &(pglCurContext->Vtx1);
      
      /* culling */
      if(pglCurContext->CullEnable) {
	if((pglCurContext->CullMode == GL_FRONT) && pglCurContext->CurFrontFace)
	  break;
	if((pglCurContext->CullMode == GL_BACK) && (!pglCurContext->CurFrontFace))
	  break;
      }
      
      if(pglCurContext->ShadeModel == GL_FLAT) {
	glIntLight(&(pglCurContext->Vtx4));
      } else {
	glIntLight(&(pglCurContext->Vtx1));
	glIntLight(&(pglCurContext->Vtx2));
	glIntLight(&(pglCurContext->Vtx3));
	glIntLight(&(pglCurContext->Vtx4));
      }
    } else {
      pglCurContext->PrimAssemState = PA_QUAD_STRIP5;
      pglCurContext->CurVtx = &(pglCurContext->Vtx1);
    }

    if(pglCurContext->ShadeModel == GL_FLAT) {
      glIntFlat(&(pglCurContext->Vtx4));
    }

    if(pglCurContext->CurFrontFace) {
      switch(pglCurContext->PolygonFrontMode) {
      case GL_POINT:
	glIntDrawPoint(&(pglCurContext->Vtx1));
	glIntDrawPoint(&(pglCurContext->Vtx2));
	glIntDrawPoint(&(pglCurContext->Vtx3));
	glIntDrawPoint(&(pglCurContext->Vtx4));
	break;
      case GL_LINE:
	glIntDrawLine(&(pglCurContext->Vtx1),
		      &(pglCurContext->Vtx2));
	glIntDrawLine(&(pglCurContext->Vtx2),
		      &(pglCurContext->Vtx4));
	glIntDrawLine(&(pglCurContext->Vtx4),
		      &(pglCurContext->Vtx3));
	glIntDrawLine(&(pglCurContext->Vtx3),
		      &(pglCurContext->Vtx1));
	break;
      case GL_FILL:
	glIntDrawTriangle(&(pglCurContext->Vtx1),
			  &(pglCurContext->Vtx2),
			  &(pglCurContext->Vtx4));
	glIntDrawTriangle(&(pglCurContext->Vtx1),
			  &(pglCurContext->Vtx4),
			  &(pglCurContext->Vtx3));
	break;
      default:
	break;
      }
    } else {
      switch(pglCurContext->PolygonBackMode) {
      case GL_POINT:
	glIntDrawPoint(&(pglCurContext->Vtx1));
	glIntDrawPoint(&(pglCurContext->Vtx2));
	glIntDrawPoint(&(pglCurContext->Vtx3));
	glIntDrawPoint(&(pglCurContext->Vtx4));
	break;
      case GL_LINE:
	glIntDrawLine(&(pglCurContext->Vtx1),
		      &(pglCurContext->Vtx2));
	glIntDrawLine(&(pglCurContext->Vtx2),
		      &(pglCurContext->Vtx4));
	glIntDrawLine(&(pglCurContext->Vtx4),
		      &(pglCurContext->Vtx3));
	glIntDrawLine(&(pglCurContext->Vtx3),
		      &(pglCurContext->Vtx1));
	break;
      case GL_FILL:
	glIntDrawTriangle(&(pglCurContext->Vtx1),
			  &(pglCurContext->Vtx2),
			  &(pglCurContext->Vtx4));
	glIntDrawTriangle(&(pglCurContext->Vtx1),
			  &(pglCurContext->Vtx4),
			  &(pglCurContext->Vtx3));
	break;
      default:
	break;
      }
    }
    break;
  case PA_QUAD_STRIP5:
    pglCurContext->PrimAssemState = PA_QUAD_STRIP6;
    pglCurContext->CurVtx = &(pglCurContext->Vtx2);
    break;
  case PA_QUAD_STRIP6:
    glIntProject(&(pglCurContext->Vtx1));
    glIntProject(&(pglCurContext->Vtx2));
    glIntProject(&(pglCurContext->Vtx3));
    glIntProject(&(pglCurContext->Vtx4));

    if(pglCurContext->Lighting) {
      /* figure out facing */
      if(pglCurContext->FrontFace == GL_CCW) {
	if(glIntCCW(&(pglCurContext->Vtx3),
		    &(pglCurContext->Vtx4),
		    &(pglCurContext->Vtx2)) >= 0.0f)
	  pglCurContext->CurFrontFace = TRUE;
	else
	  pglCurContext->CurFrontFace = FALSE;
      } else {
	if(glIntCCW(&(pglCurContext->Vtx3),
		    &(pglCurContext->Vtx4),
		    &(pglCurContext->Vtx2)) <= 0.0f)
	  pglCurContext->CurFrontFace = TRUE;
	else
	  pglCurContext->CurFrontFace = FALSE;
      }
      
      pglCurContext->PrimAssemState = PA_QUAD_STRIP7;
      pglCurContext->CurVtx = &(pglCurContext->Vtx3);
      
      /* culling */
      if(pglCurContext->CullEnable) {
	if((pglCurContext->CullMode == GL_FRONT) && pglCurContext->CurFrontFace)
	  break;
	if((pglCurContext->CullMode == GL_BACK) && (!pglCurContext->CurFrontFace))
	  break;
      }

      if(pglCurContext->ShadeModel == GL_FLAT) {
	glIntLight(&(pglCurContext->Vtx2));
      } else {
	glIntLight(&(pglCurContext->Vtx1));
	glIntLight(&(pglCurContext->Vtx2));
	glIntLight(&(pglCurContext->Vtx3));
	glIntLight(&(pglCurContext->Vtx4));
      }
    } else {
      pglCurContext->PrimAssemState = PA_QUAD_STRIP7;
      pglCurContext->CurVtx = &(pglCurContext->Vtx3);
    }
    
    if(pglCurContext->ShadeModel == GL_FLAT) {
      glIntFlat(&(pglCurContext->Vtx2));
    }

    if(pglCurContext->CurFrontFace) {
      switch(pglCurContext->PolygonFrontMode) {
      case GL_POINT:
	glIntDrawPoint(&(pglCurContext->Vtx1));
	glIntDrawPoint(&(pglCurContext->Vtx2));
	break;
      case GL_LINE:
	glIntDrawLine(&(pglCurContext->Vtx4),
		      &(pglCurContext->Vtx2));
	glIntDrawLine(&(pglCurContext->Vtx2),
		      &(pglCurContext->Vtx1));
	glIntDrawLine(&(pglCurContext->Vtx1),
		      &(pglCurContext->Vtx3));
	break;
      case GL_FILL:
	glIntDrawTriangle(&(pglCurContext->Vtx3),
			  &(pglCurContext->Vtx4),
			  &(pglCurContext->Vtx2));
	glIntDrawTriangle(&(pglCurContext->Vtx3),
			  &(pglCurContext->Vtx2),
			  &(pglCurContext->Vtx1));
	break;
      default:
	break;
      }
    } else {
      switch(pglCurContext->PolygonBackMode) {
      case GL_POINT:
	glIntDrawPoint(&(pglCurContext->Vtx1));
	glIntDrawPoint(&(pglCurContext->Vtx2));
	break;
      case GL_LINE:
	glIntDrawLine(&(pglCurContext->Vtx4),
		      &(pglCurContext->Vtx2));
	glIntDrawLine(&(pglCurContext->Vtx2),
		      &(pglCurContext->Vtx1));
	glIntDrawLine(&(pglCurContext->Vtx1),
		      &(pglCurContext->Vtx3));
	break;
      case GL_FILL:
	glIntDrawTriangle(&(pglCurContext->Vtx3),
			  &(pglCurContext->Vtx4),
			  &(pglCurContext->Vtx2));
	glIntDrawTriangle(&(pglCurContext->Vtx3),
			  &(pglCurContext->Vtx2),
			  &(pglCurContext->Vtx1));
	break;
      default:
	break;
      }
    }
    break;
  case PA_QUAD_STRIP7:
    pglCurContext->PrimAssemState = PA_QUAD_STRIP8;
    pglCurContext->CurVtx = &(pglCurContext->Vtx4);
    break;
  case PA_QUAD_STRIP8:
    glIntProject(&(pglCurContext->Vtx1));
    glIntProject(&(pglCurContext->Vtx2));
    glIntProject(&(pglCurContext->Vtx3));
    glIntProject(&(pglCurContext->Vtx4));

    if(pglCurContext->Lighting) {
      /* figure out facing */
      if(pglCurContext->FrontFace == GL_CCW) {
	if(glIntCCW(&(pglCurContext->Vtx1),
		    &(pglCurContext->Vtx2),
		    &(pglCurContext->Vtx4)) >= 0.0f)
	  pglCurContext->CurFrontFace = TRUE;
	else
	  pglCurContext->CurFrontFace = FALSE;
      } else {
	if(glIntCCW(&(pglCurContext->Vtx1),
		    &(pglCurContext->Vtx2),
		    &(pglCurContext->Vtx4)) <= 0.0f)
	  pglCurContext->CurFrontFace = TRUE;
	else
	  pglCurContext->CurFrontFace = FALSE;
      }
      
      pglCurContext->PrimAssemState = PA_QUAD_STRIP5;
      pglCurContext->CurVtx = &(pglCurContext->Vtx1);
      
      /* culling */
      if(pglCurContext->CullEnable) {
	if((pglCurContext->CullMode == GL_FRONT) && pglCurContext->CurFrontFace)
	  break;
	if((pglCurContext->CullMode == GL_BACK) && (!pglCurContext->CurFrontFace))
	  break;
      }

      if(pglCurContext->ShadeModel == GL_FLAT) {
	glIntLight(&(pglCurContext->Vtx4));
      } else {
	glIntLight(&(pglCurContext->Vtx1));
	glIntLight(&(pglCurContext->Vtx2));
	glIntLight(&(pglCurContext->Vtx3));
	glIntLight(&(pglCurContext->Vtx4));
      }
    } else {
      pglCurContext->PrimAssemState = PA_QUAD_STRIP5;
      pglCurContext->CurVtx = &(pglCurContext->Vtx1);
    }
    
    if(pglCurContext->ShadeModel == GL_FLAT) {
      glIntFlat(&(pglCurContext->Vtx4));
    }

    if(pglCurContext->CurFrontFace) {
      switch(pglCurContext->PolygonFrontMode) {
      case GL_POINT:
	glIntDrawPoint(&(pglCurContext->Vtx3));
	glIntDrawPoint(&(pglCurContext->Vtx4));
	break;
      case GL_LINE:
	glIntDrawLine(&(pglCurContext->Vtx2),
		      &(pglCurContext->Vtx4));
	glIntDrawLine(&(pglCurContext->Vtx4),
		      &(pglCurContext->Vtx3));
	glIntDrawLine(&(pglCurContext->Vtx3),
		      &(pglCurContext->Vtx1));
	break;
      case GL_FILL:
	glIntDrawTriangle(&(pglCurContext->Vtx1),
			  &(pglCurContext->Vtx2),
			  &(pglCurContext->Vtx4));
	glIntDrawTriangle(&(pglCurContext->Vtx1),
			  &(pglCurContext->Vtx4),
			  &(pglCurContext->Vtx3));
	break;
      default:
	break;
      }
    } else {
      switch(pglCurContext->PolygonBackMode) {
      case GL_POINT:
	glIntDrawPoint(&(pglCurContext->Vtx3));
	glIntDrawPoint(&(pglCurContext->Vtx4));
	break;
      case GL_LINE:
	glIntDrawLine(&(pglCurContext->Vtx2),
		      &(pglCurContext->Vtx4));
	glIntDrawLine(&(pglCurContext->Vtx4),
		      &(pglCurContext->Vtx3));
	glIntDrawLine(&(pglCurContext->Vtx3),
		      &(pglCurContext->Vtx1));
	break;
      case GL_FILL:
	glIntDrawTriangle(&(pglCurContext->Vtx1),
			  &(pglCurContext->Vtx2),
			  &(pglCurContext->Vtx4));
	glIntDrawTriangle(&(pglCurContext->Vtx1),
			  &(pglCurContext->Vtx4),
			  &(pglCurContext->Vtx3));
	break;
      default:
	break;
      }
    }
    break;
  case PA_POLYGON1:
    pglCurContext->PrimAssemState = PA_POLYGON2;
    pglCurContext->CurVtx = &(pglCurContext->Vtx2);
    break;
  case PA_POLYGON2:
    pglCurContext->PrimAssemState = PA_POLYGON3;
    pglCurContext->CurVtx = &(pglCurContext->Vtx3);
    break;
  case PA_POLYGON3:
    glIntProject(&(pglCurContext->Vtx1));
    glIntProject(&(pglCurContext->Vtx2));
    glIntProject(&(pglCurContext->Vtx3));

    if(pglCurContext->Lighting) {
      /* figure out facing */
      if(pglCurContext->FrontFace == GL_CCW) {
	if(glIntCCW(&(pglCurContext->Vtx1),
		    &(pglCurContext->Vtx2),
		    &(pglCurContext->Vtx3)) >= 0.0f)
	  pglCurContext->CurFrontFace = TRUE;
	else
	  pglCurContext->CurFrontFace = FALSE;
      } else {
	if(glIntCCW(&(pglCurContext->Vtx1),
		    &(pglCurContext->Vtx2),
		    &(pglCurContext->Vtx3)) <= 0.0f)
	  pglCurContext->CurFrontFace = TRUE;
	else
	  pglCurContext->CurFrontFace = FALSE;
      }
      
      pglCurContext->PrimAssemState = PA_POLYGON4;
      pglCurContext->CurVtx = &(pglCurContext->Vtx2);
    
      /* culling */
      if(pglCurContext->CullEnable) {
	if((pglCurContext->CullMode == GL_FRONT) && pglCurContext->CurFrontFace) {
	  break;
	}
	if((pglCurContext->CullMode == GL_BACK) && (!pglCurContext->CurFrontFace)) {
	  break;
	}
      }

      if(pglCurContext->ShadeModel == GL_FLAT) {
	glIntLight(&(pglCurContext->Vtx1));
      } else {
	glIntLight(&(pglCurContext->Vtx1));
	glIntLight(&(pglCurContext->Vtx2));
	glIntLight(&(pglCurContext->Vtx3));
      }
    } else {
      pglCurContext->PrimAssemState = PA_POLYGON4;
      pglCurContext->CurVtx = &(pglCurContext->Vtx2);
    }
    
    if(pglCurContext->ShadeModel == GL_FLAT) {
      glIntFlat(&(pglCurContext->Vtx1));
    }

    if(pglCurContext->CurFrontFace) {
      switch(pglCurContext->PolygonFrontMode) {
      case GL_POINT:
	glIntDrawPoint(&(pglCurContext->Vtx1));
	glIntDrawPoint(&(pglCurContext->Vtx2));
	glIntDrawPoint(&(pglCurContext->Vtx3));
	break;
      case GL_LINE:
	if(pglCurContext->Vtx1.Edge)
	  glIntDrawLine(&(pglCurContext->Vtx1),
			&(pglCurContext->Vtx2));
	if(pglCurContext->Vtx2.Edge)
	  glIntDrawLine(&(pglCurContext->Vtx2),
			&(pglCurContext->Vtx3));
	break;
      case GL_FILL:
	glIntDrawTriangle(&(pglCurContext->Vtx1),
			  &(pglCurContext->Vtx2),
			  &(pglCurContext->Vtx3));
	break;
      default:
	break;
      }
    } else {
      switch(pglCurContext->PolygonBackMode) {
      case GL_POINT:
	glIntDrawPoint(&(pglCurContext->Vtx1));
	glIntDrawPoint(&(pglCurContext->Vtx2));
	glIntDrawPoint(&(pglCurContext->Vtx3));
	break;
      case GL_LINE:
	if(pglCurContext->Vtx1.Edge)
	  glIntDrawLine(&(pglCurContext->Vtx1),
			&(pglCurContext->Vtx2));
	if(pglCurContext->Vtx2.Edge)
	  glIntDrawLine(&(pglCurContext->Vtx2),
			&(pglCurContext->Vtx3));
	break;
      case GL_FILL:
	glIntDrawTriangle(&(pglCurContext->Vtx1),
			  &(pglCurContext->Vtx2),
			  &(pglCurContext->Vtx3));
	break;
      default:
	break;
      }
    }
    break;
  case PA_POLYGON4:
    glIntProject(&(pglCurContext->Vtx1));
    glIntProject(&(pglCurContext->Vtx2));
    glIntProject(&(pglCurContext->Vtx3));

    if(pglCurContext->Lighting) {
      /* figure out facing */
      if(pglCurContext->FrontFace == GL_CCW) {
	if(glIntCCW(&(pglCurContext->Vtx1),
		    &(pglCurContext->Vtx3),
		    &(pglCurContext->Vtx2)) >= 0.0f)
	  pglCurContext->CurFrontFace = TRUE;
	else
	  pglCurContext->CurFrontFace = FALSE;
      } else {
	if(glIntCCW(&(pglCurContext->Vtx1),
		    &(pglCurContext->Vtx3),
		    &(pglCurContext->Vtx2)) <= 0.0f)
	  pglCurContext->CurFrontFace = TRUE;
	else
	  pglCurContext->CurFrontFace = FALSE;
      }
      
      pglCurContext->PrimAssemState = PA_POLYGON5;
      pglCurContext->CurVtx = &(pglCurContext->Vtx3);
    
      /* culling */
      if(pglCurContext->CullEnable) {
	if((pglCurContext->CullMode == GL_FRONT) && pglCurContext->CurFrontFace) {
	  break;
	}
	if((pglCurContext->CullMode == GL_BACK) && (!pglCurContext->CurFrontFace)) {
	  break;
	}
      }

      glIntLight(&(pglCurContext->Vtx1));
      glIntLight(&(pglCurContext->Vtx2));
      glIntLight(&(pglCurContext->Vtx3));
    } else {
      pglCurContext->PrimAssemState = PA_POLYGON5;
      pglCurContext->CurVtx = &(pglCurContext->Vtx3);
    }


    if(pglCurContext->CurFrontFace) {
      switch(pglCurContext->PolygonFrontMode) {
      case GL_POINT:
	glIntDrawPoint(&(pglCurContext->Vtx2));
	break;
      case GL_LINE:
	if(pglCurContext->Vtx3.Edge)
	  glIntDrawLine(&(pglCurContext->Vtx3),
			&(pglCurContext->Vtx2));
	break;
      case GL_FILL:
	glIntDrawTriangle(&(pglCurContext->Vtx1),
			  &(pglCurContext->Vtx3),
			  &(pglCurContext->Vtx2));
	break;
      default:
	break;
      }
    } else {
      switch(pglCurContext->PolygonBackMode) {
      case GL_POINT:
	glIntDrawPoint(&(pglCurContext->Vtx2));
	break;
      case GL_LINE:
	if(pglCurContext->Vtx3.Edge)
	  glIntDrawLine(&(pglCurContext->Vtx3),
			&(pglCurContext->Vtx2));
	break;
      case GL_FILL:
	glIntDrawTriangle(&(pglCurContext->Vtx1),
			  &(pglCurContext->Vtx3),
			  &(pglCurContext->Vtx2));
	break;
      default:
	break;
      }
    }
    break;
  case PA_POLYGON5:
    glIntProject(&(pglCurContext->Vtx1));
    glIntProject(&(pglCurContext->Vtx2));
    glIntProject(&(pglCurContext->Vtx3));

    if(pglCurContext->Lighting) {
      /* figure out facing */
      if(pglCurContext->FrontFace == GL_CCW) {
	if(glIntCCW(&(pglCurContext->Vtx1),
		    &(pglCurContext->Vtx2),
		    &(pglCurContext->Vtx3)) >= 0.0f)
	  pglCurContext->CurFrontFace = TRUE;
	else
	  pglCurContext->CurFrontFace = FALSE;
      } else {
	if(glIntCCW(&(pglCurContext->Vtx1),
		    &(pglCurContext->Vtx2),
		    &(pglCurContext->Vtx3)) <= 0.0f)
	  pglCurContext->CurFrontFace = TRUE;
	else
	  pglCurContext->CurFrontFace = FALSE;
      }
      
      pglCurContext->PrimAssemState = PA_POLYGON4;
      pglCurContext->CurVtx = &(pglCurContext->Vtx2);
    
      /* culling */
      if(pglCurContext->CullEnable) {
	if((pglCurContext->CullMode == GL_FRONT) && pglCurContext->CurFrontFace) {
	  break;
	}
	if((pglCurContext->CullMode == GL_BACK) && (!pglCurContext->CurFrontFace)) {
	  break;
	}
      }

      glIntLight(&(pglCurContext->Vtx1));
      glIntLight(&(pglCurContext->Vtx2));
      glIntLight(&(pglCurContext->Vtx3));
    } else {
      pglCurContext->PrimAssemState = PA_POLYGON4;
      pglCurContext->CurVtx = &(pglCurContext->Vtx2);
    }


    if(pglCurContext->CurFrontFace) {
      switch(pglCurContext->PolygonFrontMode) {
      case GL_POINT:
	glIntDrawPoint(&(pglCurContext->Vtx3));
	break;
      case GL_LINE:
	if(pglCurContext->Vtx2.Edge)
	  glIntDrawLine(&(pglCurContext->Vtx2),
			&(pglCurContext->Vtx3));
	break;
      case GL_FILL:
	glIntDrawTriangle(&(pglCurContext->Vtx1),
			  &(pglCurContext->Vtx2),
			  &(pglCurContext->Vtx3));
	break;
      default:
	break;
      }
    } else {
      switch(pglCurContext->PolygonBackMode) {
      case GL_POINT:
	glIntDrawPoint(&(pglCurContext->Vtx3));
	break;
      case GL_LINE:
	if(pglCurContext->Vtx2.Edge)
	  glIntDrawLine(&(pglCurContext->Vtx2),
			&(pglCurContext->Vtx3));
	break;
      case GL_FILL:
	glIntDrawTriangle(&(pglCurContext->Vtx1),
			  &(pglCurContext->Vtx2),
			  &(pglCurContext->Vtx3));
	break;
      default:
	break;
      }
    }
    break;
  case PA_FLUSH:
    break;
  }
}

void glIntDrawPoint(glLocalVertex *Vtx1)
{
  // Trivial Reject
  if(Vtx1->OutCodes != 0)
    return;

//  glIntProject(Vtx1);
    
  switch(pglCurContext->RenderMode) {
  case GL_RENDER:
    // Draw Point
    if(pglCurContext->PointAA) {
      if(pglCurContext->DrawBuffer == GL_FRONT_AND_BACK) {
	grRenderBuffer(GR_BUFFER_FRONTBUFFER);
	grAADrawPoint(&(Vtx1->Glide));
	grRenderBuffer(GR_BUFFER_BACKBUFFER);
	grAADrawPoint(&(Vtx1->Glide));
      } else {
	grAADrawPoint(&(Vtx1->Glide));
      }
    } else {
      if(pglCurContext->DrawBuffer == GL_FRONT_AND_BACK) {
	grRenderBuffer(GR_BUFFER_FRONTBUFFER);
	grDrawPoint(&(Vtx1->Glide));
	grRenderBuffer(GR_BUFFER_BACKBUFFER);
	grDrawPoint(&(Vtx1->Glide));
      } else {
	grDrawPoint(&(Vtx1->Glide));
      }
    }
    break;
  case GL_SELECT:
    glIntWriteSelect(Vtx1->Glide.z, Vtx1->Glide.z);
    break;
  case GL_FEEDBACK:
    glIntWriteFeedback(Vtx1);
    break;
  }
}

void glIntDrawLine(glLocalVertex *Vtx1, glLocalVertex *Vtx2)
{
  // Trivial Reject
  if((Vtx1->OutCodes&Vtx2->OutCodes) != 0) 
    return;

  if((Vtx1->OutCodes|Vtx2->OutCodes) != 0)  {
    glIntClipDrawLine(Vtx1, Vtx2);
    return;
  }

//  glIntProject(Vtx1);
//  glIntProject(Vtx2);

#define STIPPLE
#ifdef STIPPLE
  if(pglCurContext->LineStipple) {
    // Stippling only works with no texture and no alpha compare
    if(!pglCurContext->AlphaTest && !pglCurContext->Tex2D &&
       !pglCurContext->Blend && pglCurContext->CurLine) {
      float deltax, deltay;
      
      Vtx1->Glide.tmuvtx[0].sow =
	(((float)pglCurContext->LinePos)*16.0f)
	  /((float)pglCurContext->LineStippleFactor);
      
      deltax = pglCurContext->Vtx1.Glide.x-pglCurContext->Vtx2.Glide.x;
      if(deltax < 0.0f) deltax = -deltax;
      
      deltay = pglCurContext->Vtx1.Glide.y-pglCurContext->Vtx2.Glide.y;
      if(deltay < 0.0f) deltay = -deltay;
      
      if(deltax > deltay)
	pglCurContext->LinePos += deltax;
      else
	pglCurContext->LinePos += deltay;
      
      Vtx2->Glide.tmuvtx[0].sow =
	(((float)pglCurContext->LinePos)*16.0f)
	  /((float)pglCurContext->LineStippleFactor);
    }
  }
#endif

  switch(pglCurContext->RenderMode) {
  case GL_RENDER:
    // Draw Line
    if(pglCurContext->LineAA) {
      if(pglCurContext->DrawBuffer == GL_FRONT_AND_BACK) {
	grRenderBuffer(GR_BUFFER_FRONTBUFFER);
	grAADrawLine(&(Vtx1->Glide),
		     &(Vtx2->Glide));
	grRenderBuffer(GR_BUFFER_BACKBUFFER);
	grAADrawLine(&(Vtx1->Glide),
		     &(Vtx2->Glide));
      } else {
	grAADrawLine(&(Vtx1->Glide),
		     &(Vtx2->Glide));
      }
    } else {
      if(pglCurContext->DrawBuffer == GL_FRONT_AND_BACK) {
	grRenderBuffer(GR_BUFFER_FRONTBUFFER);
	grDrawLine(&(Vtx1->Glide),
		   &(Vtx2->Glide));
	grRenderBuffer(GR_BUFFER_BACKBUFFER);
	grDrawLine(&(Vtx1->Glide),
		   &(Vtx2->Glide));
      } else {
	grDrawLine(&(Vtx1->Glide),
		   &(Vtx2->Glide));
      }
    }
    break;
  case GL_SELECT:
    {
      GLfloat zmin, zmax;

      zmin = zmax = Vtx1->Glide.z;

      if(Vtx2->Glide.z > zmax)
	zmax = Vtx2->Glide.z;
      if(Vtx2->Glide.z < zmin)
	zmin = Vtx2->Glide.z;
      
      glIntWriteSelect(zmin, zmax);
    }
    break;
  case GL_FEEDBACK:
    glIntWriteFeedback(Vtx1);
    glIntWriteFeedback(Vtx2);
    break;
  }
}

void glIntDrawTriangle(glLocalVertex *Vtx1, 
		       glLocalVertex *Vtx2, 
		       glLocalVertex *Vtx3)
{
  // Trivial Reject
  if((Vtx1->OutCodes&Vtx2->OutCodes&Vtx3->OutCodes) != 0)
    return;

  if((Vtx1->OutCodes|Vtx2->OutCodes|Vtx3->OutCodes) != 0) {
    glIntClipDrawTriangle(Vtx1, Vtx2, Vtx3); 
    return;
  }
  
//  glIntProject(Vtx1);
//  glIntProject(Vtx2);
//  glIntProject(Vtx3);

  switch(pglCurContext->RenderMode) {
  case GL_RENDER:
    // Draw Triangle
    if(pglCurContext->PolyAA) {
      if(pglCurContext->DrawBuffer == GL_FRONT_AND_BACK) {
	grRenderBuffer(GR_BUFFER_FRONTBUFFER);
	grAADrawTriangle(&(Vtx1->Glide),
			 &(Vtx2->Glide),
			 &(Vtx3->Glide),
			 FXTRUE, FXTRUE, FXTRUE);
	grRenderBuffer(GR_BUFFER_BACKBUFFER);
	grAADrawTriangle(&(Vtx1->Glide),
			 &(Vtx2->Glide),
			 &(Vtx3->Glide),
			 FXTRUE, FXTRUE, FXTRUE);
      } else {
	grAADrawTriangle(&(Vtx1->Glide),
			 &(Vtx2->Glide),
			 &(Vtx3->Glide),
			 FXTRUE, FXTRUE, FXTRUE);
      }
    } else {
      if(pglCurContext->DrawBuffer == GL_FRONT_AND_BACK) {
	grRenderBuffer(GR_BUFFER_FRONTBUFFER);
	grDrawTriangle(&(Vtx1->Glide),
		       &(Vtx2->Glide),
		       &(Vtx3->Glide));
	grRenderBuffer(GR_BUFFER_BACKBUFFER);
	grDrawTriangle(&(Vtx1->Glide),
		       &(Vtx2->Glide),
		       &(Vtx3->Glide));
      } else {
	grDrawTriangle(&(Vtx1->Glide),
		       &(Vtx2->Glide),
		       &(Vtx3->Glide));
      }

    }
    break;
  case GL_SELECT:
    {
      GLfloat zmin, zmax;

      zmin = zmax = Vtx1->Glide.z;

      if(Vtx2->Glide.z > zmax)
	zmax = Vtx2->Glide.z;
      if(Vtx2->Glide.z < zmin)
	zmin = Vtx2->Glide.z;

      if(Vtx3->Glide.z > zmax)
	zmax = Vtx3->Glide.z;
      if(Vtx3->Glide.z < zmin)
	zmin = Vtx3->Glide.z;

      glIntWriteSelect(zmin, zmax);
    }
    break;
  case GL_FEEDBACK:
    glIntWriteFeedback(Vtx1);
    glIntWriteFeedback(Vtx2);
    glIntWriteFeedback(Vtx3);
    break;
  }
}

void glIntProject(glLocalVertex *Vtx)
{
  GLfloat CurNorm[3];
  GLfloat CurWin[3];

  if(!Vtx->Projected) {
    // Clip to Normalized
    if((Vtx->Clip[3] != 1.0f)&& 
       (Vtx->Clip[3] != 0.0f)) {
      Vtx->Glide.oow = 1.0f/Vtx->Clip[3];
      CurNorm[0] = Vtx->Clip[0]*
	Vtx->Glide.oow;
      CurNorm[1] = Vtx->Clip[1]*
	Vtx->Glide.oow;
      CurNorm[2] = Vtx->Clip[2]*
	Vtx->Glide.oow;
    } else {
      Vtx->Glide.oow = 1.0f;
      CurNorm[0] = Vtx->Clip[0];
      CurNorm[1] = Vtx->Clip[1];
      CurNorm[2] = Vtx->Clip[2];
    }

    // Normalized to Window
    CurWin[0] = (CurNorm[0]*pglCurContext->xScale)+
      pglCurContext->xOffset;
    CurWin[1] = (CurNorm[1]*pglCurContext->yScale)+
      pglCurContext->yOffset;
    CurWin[2] = (CurNorm[2]*pglCurContext->zScale)+
      pglCurContext->zOffset;

    // Convert to Glide vertex format
    Vtx->Glide.x = CurWin[0];
    Vtx->Glide.y = CurWin[1];

    Vtx->Glide.z = CurWin[2]; 
    Vtx->Glide.ooz = CurWin[2]*65535.0f; 
    
    if(pglCurContext->Tex2D) {
      Vtx->Glide.tmuvtx[0].sow = 
	Vtx->Texture[0]*
	  Vtx->Glide.oow*
	    pglCurContext->Tex2DPtr->SScale;
      
      Vtx->Glide.tmuvtx[0].tow = 
	Vtx->Texture[1]*
	  Vtx->Glide.oow*
	    pglCurContext->Tex2DPtr->TScale;
    }

#define STIPPLE
#ifdef STIPPLE
  // Polygon stipple
  if(pglCurContext->PolyStipple) {
    // Stippling only works with no texture and no alpha compare
    if(!pglCurContext->AlphaTest && !pglCurContext->Tex2D &&
       !pglCurContext->Blend && pglCurContext->CurPoly) {

      Vtx->Glide.oow = 1.0f;
      Vtx->Glide.tmuvtx[0].sow = CurWin[0] * 8.0f;
      Vtx->Glide.tmuvtx[0].tow = CurWin[1] * 8.0f;
    }
  }

  // Line stipple
  if(pglCurContext->LineStipple) {
    // Stippling only works with no texture and no alpha compare
    if(!pglCurContext->AlphaTest && !pglCurContext->Tex2D &&
       !pglCurContext->Blend && pglCurContext->CurLine) {

      Vtx->Glide.oow = 1.0f;
      Vtx->Glide.tmuvtx[0].sow = 0.0f;
      Vtx->Glide.tmuvtx[0].tow = 0.0f;
    }
  }
#endif

    Vtx->Projected = TRUE;
  }
}
