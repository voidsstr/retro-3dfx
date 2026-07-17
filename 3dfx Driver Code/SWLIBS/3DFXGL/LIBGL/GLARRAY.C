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

void APIENTRY glVertexPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
  switch(size) {
  case 2:
  case 3:
  case 4:
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }

  switch(type) {
  case GL_SHORT:
    if(stride==0)
      pglCurContext->VertexStride = size*2;
    else
      pglCurContext->VertexStride = stride;
    break;
  case GL_INT:
  case GL_FLOAT:
    if(stride==0)
      pglCurContext->VertexStride = size*4;
    else
      pglCurContext->VertexStride = stride;
    break;
  case GL_DOUBLE:
    if(stride==0)
      pglCurContext->VertexStride = size*8;
    else
      pglCurContext->VertexStride = stride;
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }

  pglCurContext->VertexSize = size;
  pglCurContext->VertexType = type;
  pglCurContext->VertexPtr = (void *)pointer;
}

void APIENTRY glNormalPointer (GLenum type, GLsizei stride, const GLvoid *pointer)
{
  switch(type) {
  case GL_BYTE:
    if(stride==0)
      pglCurContext->NormalStride = 3;
    else
      pglCurContext->NormalStride = stride;
    break;
  case GL_SHORT:
    if(stride==0)
      pglCurContext->NormalStride = 6;
    else
      pglCurContext->NormalStride = stride;
    break;
  case GL_INT:
  case GL_FLOAT:
    if(stride==0)
      pglCurContext->NormalStride = 12;
    else
      pglCurContext->NormalStride = stride;
    break;
  case GL_DOUBLE:
    if(stride==0)
      pglCurContext->NormalStride = 24;
    else
      pglCurContext->NormalStride = stride;
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }

  pglCurContext->NormalType = type;
  pglCurContext->NormalPtr = (void *)pointer;
}

void APIENTRY glColorPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
  switch(size) {
  case 3:
  case 4:
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }

  switch(type) {
  case GL_BYTE:
  case GL_UNSIGNED_BYTE:
    if(stride==0)
      pglCurContext->ColorStride = size;
    else
      pglCurContext->ColorStride = stride;
    break;
  case GL_SHORT:
  case GL_UNSIGNED_SHORT:
    if(stride==0)
      pglCurContext->ColorStride = size*2;
    else
      pglCurContext->ColorStride = stride;
    break;
  case GL_INT:
  case GL_UNSIGNED_INT:
  case GL_FLOAT:
    if(stride==0)
      pglCurContext->ColorStride = size*4;
    else
      pglCurContext->ColorStride = stride;
    break;
  case GL_DOUBLE:
    if(stride==0)
      pglCurContext->ColorStride = size*8;
    else
      pglCurContext->ColorStride = stride;
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }

  pglCurContext->ColorSize = size;
  pglCurContext->ColorType = type;
  pglCurContext->ColorPtr = (void *)pointer;
}

void APIENTRY glTexCoordPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
  switch(size) {
  case 1:
  case 2:
  case 3:
  case 4:
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }

  switch(type) {
  case GL_SHORT:
    if(stride==0)
      pglCurContext->TexStride = size*2;
    else
      pglCurContext->TexStride = stride;
    break;
  case GL_INT:
  case GL_FLOAT:
    if(stride==0)
      pglCurContext->TexStride = size*4;
    else
      pglCurContext->TexStride = stride;
    break;
  case GL_DOUBLE:
    if(stride==0)
      pglCurContext->TexStride = size*8;
    else
      pglCurContext->TexStride = stride;
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }

  pglCurContext->TexSize = size;
  pglCurContext->TexType = type;
  pglCurContext->TexPtr = (void *)pointer;
}

void APIENTRY glEdgeFlagPointer (GLsizei stride, const GLvoid *pointer)
{
  if(stride==0)
    pglCurContext->EdgeFlagStride = 4;
  else
    pglCurContext->EdgeFlagStride = stride;

  pglCurContext->EdgeFlagPtr = (void *)pointer;
}

void APIENTRY glEnableClientState (GLenum array)
{
  GLINT_OUTSIDE_BEGIN();

  switch(array) {
  case GL_VERTEX_ARRAY:
    pglCurContext->VertexEnable = TRUE;
    break;
  case GL_NORMAL_ARRAY:
    pglCurContext->NormalEnable = TRUE;
    break;
  case GL_COLOR_ARRAY:
    pglCurContext->ColorEnable = TRUE;
    break;
  case GL_TEXTURE_COORD_ARRAY:
    pglCurContext->TexEnable = TRUE;
    break;
  case GL_EDGE_FLAG_ARRAY:
    pglCurContext->EdgeFlagEnable = TRUE;
    break;
  case GL_INDEX_ARRAY:
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    break;
  }
}

void APIENTRY glDisableClientState (GLenum array)
{
  GLINT_OUTSIDE_BEGIN();

  switch(array) {
  case GL_VERTEX_ARRAY:
    pglCurContext->VertexEnable = FALSE;
    break;
  case GL_NORMAL_ARRAY:
    pglCurContext->NormalEnable = FALSE;
    break;
  case GL_COLOR_ARRAY:
    pglCurContext->ColorEnable = FALSE;
    break;
  case GL_TEXTURE_COORD_ARRAY:
    pglCurContext->TexEnable = FALSE;
    break;
  case GL_EDGE_FLAG_ARRAY:
    pglCurContext->EdgeFlagEnable = FALSE;
    break;
  case GL_INDEX_ARRAY:
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    break;
  }
}

void APIENTRY glInterleavedArrays (GLenum format, GLsizei stride, const GLvoid *pointer)
{
  GLINT_OUTSIDE_BEGIN();

  pglCurContext->EdgeFlagEnable = FALSE;
  pglCurContext->VertexEnable = TRUE;

  switch(format) {
  case GL_V2F:
    pglCurContext->NormalEnable = FALSE;
    pglCurContext->ColorEnable = FALSE;
    pglCurContext->TexEnable = FALSE;

    if(stride == 0) {
      pglCurContext->VertexStride = 8;
    } else {
      pglCurContext->VertexStride = stride;
    }
    pglCurContext->VertexSize = 2;
    pglCurContext->VertexType = GL_FLOAT;
    pglCurContext->VertexPtr = (void *)pointer;
    break;
  case GL_V3F:
    pglCurContext->NormalEnable = FALSE;
    pglCurContext->ColorEnable = FALSE;
    pglCurContext->TexEnable = FALSE;

    if(stride == 0) {
      pglCurContext->VertexStride = 12;
    } else {
      pglCurContext->VertexStride = stride;
    }
    pglCurContext->VertexSize = 3;
    pglCurContext->VertexType = GL_FLOAT;
    pglCurContext->VertexPtr = (void *)pointer;
    break;
  case GL_C4UB_V2F:
    pglCurContext->NormalEnable = FALSE;
    pglCurContext->ColorEnable = TRUE;
    pglCurContext->TexEnable = FALSE;

    if(stride == 0) {
      pglCurContext->VertexStride = 12;
      pglCurContext->ColorStride = 12;
    } else {
      pglCurContext->VertexStride = stride;
      pglCurContext->ColorStride = stride;
    }
    pglCurContext->VertexSize = 2;
    pglCurContext->VertexType = GL_FLOAT;
    pglCurContext->VertexPtr = (void *)((char *)pointer+4);

    pglCurContext->ColorSize = 4;
    pglCurContext->ColorType = GL_UNSIGNED_BYTE;
    pglCurContext->ColorPtr = (void *)pointer;
    break;
  case GL_C4UB_V3F:
    pglCurContext->NormalEnable = FALSE;
    pglCurContext->ColorEnable = TRUE;
    pglCurContext->TexEnable = FALSE;

    if(stride == 0) {
      pglCurContext->VertexStride = 16;
      pglCurContext->ColorStride = 16;
    } else {
      pglCurContext->VertexStride = stride;
      pglCurContext->ColorStride = stride;
    }
    pglCurContext->VertexSize = 3;
    pglCurContext->VertexType = GL_FLOAT;
    pglCurContext->VertexPtr = (void *)((char *)pointer+4);

    pglCurContext->ColorSize = 4;
    pglCurContext->ColorType = GL_UNSIGNED_BYTE;
    pglCurContext->ColorPtr = (void *)pointer;
    break;
  case GL_C3F_V3F:
    pglCurContext->NormalEnable = FALSE;
    pglCurContext->ColorEnable = TRUE;
    pglCurContext->TexEnable = FALSE;

    if(stride == 0) {
      pglCurContext->VertexStride = 24;
      pglCurContext->ColorStride = 24;
    } else {
      pglCurContext->VertexStride = stride;
      pglCurContext->ColorStride = stride;
    }
    pglCurContext->VertexSize = 3;
    pglCurContext->VertexType = GL_FLOAT;
    pglCurContext->VertexPtr = (void *)((char *)pointer+12);

    pglCurContext->ColorSize = 3;
    pglCurContext->ColorType = GL_FLOAT;
    pglCurContext->ColorPtr = (void *)pointer;
    break;
  case GL_N3F_V3F:
    pglCurContext->NormalEnable = TRUE;
    pglCurContext->ColorEnable = FALSE;
    pglCurContext->TexEnable = FALSE;

    if(stride == 0) {
      pglCurContext->VertexStride = 24;
      pglCurContext->NormalStride = 24;
    } else {
      pglCurContext->VertexStride = stride;
      pglCurContext->NormalStride = stride;
    }
    pglCurContext->VertexSize = 3;
    pglCurContext->VertexType = GL_FLOAT;
    pglCurContext->VertexPtr = (void *)((char *)pointer+12);

    pglCurContext->NormalType = GL_FLOAT;
    pglCurContext->NormalPtr = (void *)pointer;
    break;
  case GL_C4F_N3F_V3F:
    pglCurContext->NormalEnable = TRUE;
    pglCurContext->ColorEnable = TRUE;
    pglCurContext->TexEnable = FALSE;

    if(stride == 0) {
      pglCurContext->VertexStride = 40;
      pglCurContext->NormalStride = 40;
      pglCurContext->ColorStride = 40;
    } else {
      pglCurContext->VertexStride = stride;
      pglCurContext->NormalStride = stride;
      pglCurContext->ColorStride = stride;
    }
    pglCurContext->VertexSize = 3;
    pglCurContext->VertexType = GL_FLOAT;
    pglCurContext->VertexPtr = (void *)((char *)pointer+28);

    pglCurContext->NormalType = GL_FLOAT;
    pglCurContext->NormalPtr = (void *)((char *)pointer+16);

    pglCurContext->ColorSize = 4;
    pglCurContext->ColorType = GL_FLOAT;
    pglCurContext->ColorPtr = (void *)pointer;
    break;
  case GL_T2F_V3F:
    pglCurContext->NormalEnable = FALSE;
    pglCurContext->ColorEnable = FALSE;
    pglCurContext->TexEnable = TRUE;

    if(stride == 0) {
      pglCurContext->VertexStride = 20;
      pglCurContext->TexStride = 20;
    } else {
      pglCurContext->VertexStride = stride;
      pglCurContext->TexStride = stride;
    }
    pglCurContext->VertexSize = 3;
    pglCurContext->VertexType = GL_FLOAT;
    pglCurContext->VertexPtr = (void *)((char *)pointer+8);

    pglCurContext->TexSize = 2;
    pglCurContext->TexType = GL_FLOAT;
    pglCurContext->TexPtr = (void *)pointer;
    break;
  case GL_T4F_V4F:
    pglCurContext->NormalEnable = FALSE;
    pglCurContext->ColorEnable = FALSE;
    pglCurContext->TexEnable = TRUE;

    if(stride == 0) {
      pglCurContext->VertexStride = 32;
      pglCurContext->TexStride = 32;
    } else {
      pglCurContext->VertexStride = stride;
      pglCurContext->TexStride = stride;
    }
    pglCurContext->VertexSize = 4;
    pglCurContext->VertexType = GL_FLOAT;
    pglCurContext->VertexPtr = (void *)((char *)pointer+16);

    pglCurContext->TexSize = 4;
    pglCurContext->TexType = GL_FLOAT;
    pglCurContext->TexPtr = (void *)pointer;
    break;
  case GL_T2F_C4UB_V3F:
    pglCurContext->NormalEnable = FALSE;
    pglCurContext->ColorEnable = TRUE;
    pglCurContext->TexEnable = TRUE;

    if(stride == 0) {
      pglCurContext->VertexStride = 24;
      pglCurContext->TexStride = 24;
      pglCurContext->ColorStride = 24;
    } else {
      pglCurContext->VertexStride = stride;
      pglCurContext->TexStride = stride;
      pglCurContext->ColorStride = stride;
    }
    pglCurContext->VertexSize = 3;
    pglCurContext->VertexType = GL_FLOAT;
    pglCurContext->VertexPtr = (void *)((char *)pointer+12);

    pglCurContext->ColorSize = 4;
    pglCurContext->ColorType = GL_UNSIGNED_BYTE;
    pglCurContext->ColorPtr = (void *)((char *)pointer+8);

    pglCurContext->TexSize = 2;
    pglCurContext->TexType = GL_FLOAT;
    pglCurContext->TexPtr = (void *)pointer;
    break;
  case GL_T2F_C3F_V3F:
    pglCurContext->NormalEnable = FALSE;
    pglCurContext->ColorEnable = TRUE;
    pglCurContext->TexEnable = TRUE;

    if(stride == 0) {
      pglCurContext->VertexStride = 32;
      pglCurContext->TexStride = 32;
      pglCurContext->ColorStride = 32;
    } else {
      pglCurContext->VertexStride = stride;
      pglCurContext->TexStride = stride;
      pglCurContext->ColorStride = stride;
    }
    pglCurContext->VertexSize = 3;
    pglCurContext->VertexType = GL_FLOAT;
    pglCurContext->VertexPtr = (void *)((char *)pointer+20);

    pglCurContext->ColorSize = 3;
    pglCurContext->ColorType = GL_FLOAT;
    pglCurContext->ColorPtr = (void *)((char *)pointer+8);

    pglCurContext->TexSize = 2;
    pglCurContext->TexType = GL_FLOAT;
    pglCurContext->TexPtr = (void *)pointer;
    break;
  case GL_T2F_N3F_V3F:
    pglCurContext->NormalEnable = TRUE;
    pglCurContext->ColorEnable = FALSE;
    pglCurContext->TexEnable = TRUE;

    if(stride == 0) {
      pglCurContext->VertexStride = 32;
      pglCurContext->TexStride = 32;
      pglCurContext->NormalStride = 32;
    } else {
      pglCurContext->VertexStride = stride;
      pglCurContext->TexStride = stride;
      pglCurContext->NormalStride = stride;
    }
    pglCurContext->VertexSize = 3;
    pglCurContext->VertexType = GL_FLOAT;
    pglCurContext->VertexPtr = (void *)((char *)pointer+20);

    pglCurContext->NormalType = GL_FLOAT;
    pglCurContext->NormalPtr = (void *)((char *)pointer+8);

    pglCurContext->TexSize = 2;
    pglCurContext->TexType = GL_FLOAT;
    pglCurContext->TexPtr = (void *)pointer;
    break;
  case GL_T2F_C4F_N3F_V3F:
    pglCurContext->NormalEnable = TRUE;
    pglCurContext->ColorEnable = TRUE;
    pglCurContext->TexEnable = TRUE;

    if(stride == 0) {
      pglCurContext->VertexStride = 48;
      pglCurContext->TexStride = 48;
      pglCurContext->NormalStride = 48;
      pglCurContext->ColorStride = 48;
    } else {
      pglCurContext->VertexStride = stride;
      pglCurContext->TexStride = stride;
      pglCurContext->NormalStride = stride;
      pglCurContext->ColorStride = stride;
    }
    pglCurContext->VertexSize = 3;
    pglCurContext->VertexType = GL_FLOAT;
    pglCurContext->VertexPtr = (void *)((char *)pointer+36);

    pglCurContext->NormalType = GL_FLOAT;
    pglCurContext->NormalPtr = (void *)((char *)pointer+24);

    pglCurContext->ColorSize = 4;
    pglCurContext->ColorType = GL_FLOAT;
    pglCurContext->ColorPtr = (void *)((char *)pointer+8);

    pglCurContext->TexSize = 2;
    pglCurContext->TexType = GL_FLOAT;
    pglCurContext->TexPtr = (void *)pointer;
    break;
  case GL_T4F_C4F_N3F_V4F:
    pglCurContext->NormalEnable = TRUE;
    pglCurContext->ColorEnable = TRUE;
    pglCurContext->TexEnable = TRUE;

    if(stride == 0) {
      pglCurContext->VertexStride = 60;
      pglCurContext->TexStride = 60;
      pglCurContext->NormalStride = 60;
      pglCurContext->ColorStride = 60;
    } else {
      pglCurContext->VertexStride = stride;
      pglCurContext->TexStride = stride;
      pglCurContext->NormalStride = stride;
      pglCurContext->ColorStride = stride;
    }
    pglCurContext->VertexSize = 4;
    pglCurContext->VertexType = GL_FLOAT;
    pglCurContext->VertexPtr = (void *)((char *)pointer+44);

    pglCurContext->NormalType = GL_FLOAT;
    pglCurContext->NormalPtr = (void *)((char *)pointer+32);

    pglCurContext->ColorSize = 4;
    pglCurContext->ColorType = GL_FLOAT;
    pglCurContext->ColorPtr = (void *)((char *)pointer+16);

    pglCurContext->TexSize = 4;
    pglCurContext->TexType = GL_FLOAT;
    pglCurContext->TexPtr = (void *)pointer;
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    break;
  };
}

void APIENTRY glArrayElement (GLint i)
{
  void *normptr;
  void *colorptr;
  void *texptr;
  void *edgeptr;
  void *vertexptr;
  
  GLINT_INSIDE_BEGIN();

  if(pglCurContext->VertexEnable) {
    // Calculate Vertex Pointer
    vertexptr = ((char *)pglCurContext->VertexPtr)+
      (i*pglCurContext->VertexStride);
  }

  if(pglCurContext->EdgeFlagEnable) {
    // Calculate Edge Flag Pointer
    edgeptr = ((char *)pglCurContext->EdgeFlagPtr)+
      (i*pglCurContext->EdgeFlagStride);
  }

  if(pglCurContext->TexEnable) {
    // Calculate Texture Pointer
    texptr = ((char *)pglCurContext->TexPtr)+
      (i*pglCurContext->TexStride);
  }

  if(pglCurContext->ColorEnable) {
    // Calculate Color Pointer
    colorptr = ((char *)pglCurContext->ColorPtr)+
      (i*pglCurContext->ColorStride);
  }

  if(pglCurContext->NormalEnable) {
    // Calculate Normal pointer
    normptr = ((char *)pglCurContext->NormalPtr)+
      (i*pglCurContext->NormalStride);
  }

  glIntLoadArray(normptr, colorptr, texptr, edgeptr, vertexptr);
}

void APIENTRY glDrawElements (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
{
  void *normptr;
  void *colorptr;
  void *texptr;
  void *edgeptr;
  void *vertexptr;
  int i,pos;

  switch(mode) {
  case GL_POINTS:
    break;
  case GL_LINES:
    break;
  case GL_LINE_STRIP:
    break;
  case GL_LINE_LOOP:
    break;
  case GL_TRIANGLES:
    break;
  case GL_TRIANGLE_STRIP:
    break;
  case GL_TRIANGLE_FAN:
    break;
  case GL_QUADS:
    break;
  case GL_QUAD_STRIP:
    break;
  case GL_POLYGON:
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }

  switch(type) {
  case GL_UNSIGNED_BYTE:
    break;
  case GL_UNSIGNED_SHORT:
    break;
  case GL_UNSIGNED_INT:
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }

  glBegin(mode);

  for(i=0;i<count;i++) {

    switch(type) {
    case GL_UNSIGNED_BYTE:
      pos = ((unsigned char *)indices)[i];
      break;
    case GL_UNSIGNED_SHORT:
      pos = ((unsigned short *)indices)[i];
      break;
    case GL_UNSIGNED_INT:
      pos = ((unsigned int *)indices)[i];
      break;
    }

    if(pglCurContext->VertexEnable) {
      // Calculate Vertex Pointer
      vertexptr = ((char *)pglCurContext->VertexPtr)+
	  (pos*pglCurContext->VertexStride);
    }
    
    if(pglCurContext->EdgeFlagEnable) {
      // Calculate Edge Flag Pointer
      edgeptr = ((char *)pglCurContext->EdgeFlagPtr)+
	(pos*pglCurContext->EdgeFlagStride);
    }

    if(pglCurContext->TexEnable) {
      // Calculate Texture Pointer
      texptr = ((char *)pglCurContext->TexPtr)+
	(pos*pglCurContext->TexStride);
    }

    if(pglCurContext->ColorEnable) {
      // Calculate Color Pointer
      colorptr = ((char *)pglCurContext->ColorPtr)+
	(pos*pglCurContext->ColorStride);
    }

    if(pglCurContext->NormalEnable) {
      // Calculate Normal pointer
      normptr = ((char *)pglCurContext->NormalPtr)+
	(pos*pglCurContext->NormalStride);
    }
    
    glIntLoadArray(normptr, colorptr, texptr, edgeptr, vertexptr);
  }

  glEnd();
}

void APIENTRY glDrawArrays (GLenum mode, GLint first, GLsizei count)
{
  void *normptr;
  void *colorptr;
  void *texptr;
  void *edgeptr;
  void *vertexptr;
  int i;

  switch(mode) {
  case GL_POINTS:
    break;
  case GL_LINES:
    break;
  case GL_LINE_STRIP:
    break;
  case GL_LINE_LOOP:
    break;
  case GL_TRIANGLES:
    break;
  case GL_TRIANGLE_STRIP:
    break;
  case GL_TRIANGLE_FAN:
    break;
  case GL_QUADS:
    break;
  case GL_QUAD_STRIP:
    break;
  case GL_POLYGON:
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }

  glBegin(mode);

  if(pglCurContext->VertexEnable) {
    // Calculate Vertex Pointer
    vertexptr = ((char *)pglCurContext->VertexPtr)+
      (first*pglCurContext->VertexStride);
  }

  if(pglCurContext->EdgeFlagEnable) {
    // Calculate Edge Flag Pointer
    edgeptr = ((char *)pglCurContext->EdgeFlagPtr)+
      (first*pglCurContext->EdgeFlagStride);
  }

  if(pglCurContext->TexEnable) {
    // Calculate Texture Pointer
    texptr = ((char *)pglCurContext->TexPtr)+
      (first*pglCurContext->TexStride);
  }

  if(pglCurContext->ColorEnable) {
    // Calculate Color Pointer
    colorptr = ((char *)pglCurContext->ColorPtr)+
      (first*pglCurContext->ColorStride);
  }

  if(pglCurContext->NormalEnable) {
    // Calculate Normal pointer
    normptr = ((char *)pglCurContext->NormalPtr)+
      (first*pglCurContext->NormalStride);
  }

  for(i=0;i<count;i++) {
    glIntLoadArray(normptr, colorptr, texptr, edgeptr, vertexptr);

    if(pglCurContext->NormalEnable)
      (char *)normptr += pglCurContext->NormalStride;
    if(pglCurContext->ColorEnable)
      (char *)colorptr += pglCurContext->ColorStride;
    if(pglCurContext->TexEnable)
      (char *)texptr += pglCurContext->TexStride;
    if(pglCurContext->EdgeFlagEnable)
      (char *)edgeptr += pglCurContext->EdgeFlagStride;
    if(pglCurContext->VertexEnable)
      (char *)vertexptr += pglCurContext->VertexStride;
  }

  glEnd();
}

void APIENTRY glIndexPointer (GLenum type, GLsizei stride, const GLvoid *pointer)
{
}


void glIntLoadArray(void *normptr, void *colorptr,  void *texptr,
		    void *edgeptr, void *vertexptr)
{
  GLfloat CurColor[4];
  GLfloat CurTex[4];
  GLfloat CurNormal[3];
  GLboolean CurEdge;
  GLfloat CurObj[4];

  if(pglCurContext->NormalEnable) {

    switch(pglCurContext->NormalType) {
    case GL_BYTE:
      CurNormal[0] = (float)*((byte *)normptr)++;
      CurNormal[1] = (float)*((byte *)normptr)++;
      CurNormal[2] = (float)*((byte *)normptr)++;
      break;
    case GL_SHORT:
      CurNormal[0] = (float)*((short *)normptr)++;
      CurNormal[1] = (float)*((short *)normptr)++;
      CurNormal[2] = (float)*((short *)normptr)++;
      break;
    case GL_INT:
      CurNormal[0] = (float)*((int *)normptr)++;
      CurNormal[1] = (float)*((int *)normptr)++;
      CurNormal[2] = (float)*((int *)normptr)++;
      break;
    case GL_FLOAT:
      CurNormal[0] = *((float *)normptr)++;
      CurNormal[1] = *((float *)normptr)++;
      CurNormal[2] = *((float *)normptr)++;
      break;
    case GL_DOUBLE:
      CurNormal[0] = (float)*((double *)normptr)++;
      CurNormal[1] = (float)*((double *)normptr)++;
      CurNormal[2] = (float)*((double *)normptr)++;
      break;
    }
  }

  if(pglCurContext->ColorEnable) {

    switch(pglCurContext->ColorType) {
    case GL_BYTE:
      CurColor[0] = 
	((float)*((char *)colorptr)++)/127.0f;
      CurColor[1] = 
	((float)*((char *)colorptr)++)/127.0f;
      CurColor[2] = 
	((float)*((char *)colorptr)++)/127.0f;
      if(pglCurContext->ColorSize < 4) {
	CurColor[3] = 1.0f;
      } else {
	CurColor[3] = 
	  ((float)*((char *)colorptr)++)/127.0f;
      }
      break;
    case GL_UNSIGNED_BYTE:
      CurColor[0] = 
	((float)*((unsigned char *)colorptr)++)/255.0f;
      CurColor[1] = 
	((float)*((unsigned char *)colorptr)++)/255.0f;
      CurColor[2] = 
	((float)*((unsigned char *)colorptr)++)/255.0f;
      if(pglCurContext->ColorSize < 4) {
	CurColor[3] = 1.0f;
      } else {
	CurColor[3] = 
	  ((float)*((unsigned char *)colorptr)++)/255.0f;
      }
      break;
    case GL_SHORT:
      CurColor[0] = 
	((float)*((short *)colorptr)++)/32767.0f;
      CurColor[1] = 
	((float)*((short *)colorptr)++)/32767.0f;
      CurColor[2] = 
	((float)*((short *)colorptr)++)/32767.0f;
      if(pglCurContext->ColorSize < 4) {
	CurColor[3] = 1.0f;
      } else {
	CurColor[3] = 
	  ((float)*((short *)colorptr)++)/32767.0f;
      }
      break;
    case GL_UNSIGNED_SHORT:
      CurColor[0] = 
	((float)*((unsigned short *)colorptr)++)/65535.0f;
      CurColor[1] = 
	((float)*((unsigned short *)colorptr)++)/65535.0f;
      CurColor[2] = 
	((float)*((unsigned short *)colorptr)++)/65535.0f;
      if(pglCurContext->ColorSize < 4) {
	CurColor[3] = 1.0f;
      } else {
	CurColor[3] = 
	  ((float)*((unsigned short *)colorptr)++)/65535.0f;
      }
      break;
    case GL_INT:
      CurColor[0] = 
	((float)*((int *)colorptr)++)/(32767.0f*65536.0f);
      CurColor[1] = 
	((float)*((int *)colorptr)++)/(32767.0f*65536.0f);
      CurColor[2] = 
	((float)*((int *)colorptr)++)/(32767.0f*65536.0f);
      if(pglCurContext->ColorSize < 4) {
	CurColor[3] = 1.0f;
      } else {
	CurColor[3] = 
	  ((float)*((int *)colorptr)++)/(32767.0f*65536.0f);
      }
      break;
    case GL_UNSIGNED_INT:
      CurColor[0] = 
	((float)*((unsigned int *)colorptr)++)/(65535.0f*65536.0f);
      CurColor[1] = 
	((float)*((unsigned int *)colorptr)++)/(65535.0f*65536.0f);
      CurColor[2] = 
	((float)*((unsigned int *)colorptr)++)/(65535.0f*65536.0f);
      if(pglCurContext->ColorSize < 4) {
	CurColor[3] = 1.0f;
      } else {
	CurColor[3] = 
	  ((float)*((unsigned int *)colorptr)++)/(65535.0f*65536.0f);
      }
      break;
    case GL_FLOAT:
      CurColor[0] = 
	*((float *)colorptr)++;
      CurColor[1] = 
	*((float *)colorptr)++;
      CurColor[2] = 
	*((float *)colorptr)++;
      if(pglCurContext->ColorSize < 4) {
	CurColor[3] = 1.0f;
      } else {
	CurColor[3] = 
	  *((float *)colorptr)++;
      }
      break;
    case GL_DOUBLE:
      CurColor[0] = 
	((float)*((double *)colorptr)++);
      CurColor[1] = 
	((float)*((double *)colorptr)++);
      CurColor[2] = 
	((float)*((double *)colorptr)++);
      if(pglCurContext->ColorSize < 4) {
	CurColor[3] = 1.0f;
      } else {
	CurColor[3] = 
	  ((float)*((double *)colorptr)++);
      }
      break;
    }
  }

  if(pglCurContext->TexEnable) {

    switch(pglCurContext->TexType) {
    case GL_SHORT:
      CurTex[0] = 
	((float)*((short *)colorptr)++);
      
      if(pglCurContext->TexSize < 2) {
	CurTex[1] = 0.0f;
      } else {
	CurTex[1] = 
	  ((float)*((short *)colorptr)++);
      }
	
      if(pglCurContext->TexSize < 3) {
	CurTex[2] = 0.0f;
      } else {
	CurTex[2] = 
	  ((float)*((short *)colorptr)++);
      }

      if(pglCurContext->TexSize < 4) {
	CurTex[3] = 1.0f;
      } else {
	CurTex[3] = 
	  ((float)*((short *)colorptr)++);
      }
      break;
    case GL_INT:
      CurTex[0] = 
	((float)*((int *)colorptr)++);
      
      if(pglCurContext->TexSize < 2) {
	CurTex[1] = 0.0f;
      } else {
	CurTex[1] = 
	  ((float)*((int *)colorptr)++);
      }
	
      if(pglCurContext->TexSize < 3) {
	CurTex[2] = 0.0f;
      } else {
	CurTex[2] = 
	  ((float)*((int *)colorptr)++);
      }

      if(pglCurContext->TexSize < 4) {
	CurTex[3] = 1.0f;
      } else {
	CurTex[3] = 
	  ((float)*((int *)colorptr)++);
      }
      break;
    case GL_FLOAT:
      CurTex[0] = 
	*((float *)colorptr)++;
      
      if(pglCurContext->TexSize < 2) {
	CurTex[1] = 0.0f;
      } else {
	CurTex[1] = 
	  *((float *)colorptr)++;
      }
	
      if(pglCurContext->TexSize < 3) {
	CurTex[2] = 0.0f;
      } else {
	CurTex[2] = 
	  *((float *)colorptr)++;
      }

      if(pglCurContext->TexSize < 4) {
	CurTex[3] = 1.0f;
      } else {
	CurTex[3] = 
	  *((float *)colorptr)++;
      }
      break;
    case GL_DOUBLE:
      CurTex[0] = 
	((float)*((double *)colorptr)++);
      
      if(pglCurContext->TexSize < 2) {
	CurTex[1] = 0.0f;
      } else {
	CurTex[1] = 
	  ((float)*((double *)colorptr)++);
      }
	
      if(pglCurContext->TexSize < 3) {
	CurTex[2] = 0.0f;
      } else {
	CurTex[2] = 
	  ((float)*((double *)colorptr)++);
      }

      if(pglCurContext->TexSize < 4) {
	CurTex[3] = 1.0f;
      } else {
	CurTex[3] = 
	  ((float)*((double *)colorptr)++);
      }
      break;
    }
  }

  if(pglCurContext->EdgeFlagEnable) {
    CurEdge = *((boolean *)edgeptr)++;
  }

  if(pglCurContext->VertexEnable) {

    switch(pglCurContext->VertexType) {
    case GL_SHORT:
      CurObj[0] = 
	((float)*((short *)vertexptr)++);
      
      CurObj[1] = 
	((float)*((short *)vertexptr)++);
	
      if(pglCurContext->TexSize < 3) {
	CurObj[2] = 0.0f;
      } else {
	CurObj[2] = 
	  ((float)*((short *)vertexptr)++);
      }

      if(pglCurContext->TexSize < 4) {
	CurObj[3] = 1.0f;
      } else {
	CurObj[3] = 
	  ((float)*((short *)vertexptr)++);
      }
      break;
    case GL_INT:
      CurObj[0] = 
	((float)*((int *)vertexptr)++);
      
      CurObj[1] = 
	((float)*((int *)vertexptr)++);
	
      if(pglCurContext->TexSize < 3) {
	CurObj[2] = 0.0f;
      } else {
	CurObj[2] = 
	  ((float)*((int *)vertexptr)++);
      }

      if(pglCurContext->TexSize < 4) {
	CurObj[3] = 1.0f;
      } else {
	CurObj[3] = 
	  ((float)*((int *)vertexptr)++);
      }
      break;
    case GL_FLOAT:
      CurObj[0] = 
	*((float *)vertexptr)++;
      
      CurObj[1] = 
	*((float *)vertexptr)++;
	
      if(pglCurContext->TexSize < 3) {
	CurObj[2] = 0.0f;
      } else {
	CurObj[2] = 
	  *((float *)vertexptr)++;
      }

      if(pglCurContext->TexSize < 4) {
	CurObj[3] = 1.0f;
      } else {
	CurObj[3] = 
	  *((float *)vertexptr)++;
      }
      break;
    case GL_DOUBLE:
      CurObj[0] = 
	((float)*((double *)vertexptr)++);
      
      CurObj[1] = 
	((float)*((double *)vertexptr)++);
	
      if(pglCurContext->TexSize < 3) {
	CurObj[2] = 0.0f;
      } else {
	CurObj[2] = 
	  ((float)*((double *)vertexptr)++);
      }

      if(pglCurContext->TexSize < 4) {
	CurObj[3] = 1.0f;
      } else {
	CurObj[3] = 
	  ((float)*((double *)vertexptr)++);
      }
      break;
    }
  }

  if(pglCurContext->Listing) {

    // Copy to display list
    if(pglCurContext->NormalEnable) {
      glIntIntToList(OP_NORMAL_COORD);
      glIntFloatToList(CurNormal[0]);
      glIntFloatToList(CurNormal[1]);
      glIntFloatToList(CurNormal[2]);
    }
    if(pglCurContext->ColorEnable) {
      glIntIntToList(OP_COLOR_COORD);
      glIntFloatToList(CurColor[0]);
      glIntFloatToList(CurColor[1]);
      glIntFloatToList(CurColor[2]);
      glIntFloatToList(CurColor[3]);
    }
    if(pglCurContext->TexEnable) {
      glIntIntToList(OP_TEXTURE_COORD);
      glIntFloatToList(CurTex[0]);
      glIntFloatToList(CurTex[1]);
      glIntFloatToList(CurTex[2]);
      glIntFloatToList(CurTex[3]);
    }
    if(pglCurContext->EdgeFlagEnable) {
      glIntIntToList(OP_EDGE_COORD);
      glIntIntToList((int)CurEdge);
    }
    if(pglCurContext->VertexEnable) {
      glIntIntToList(OP_VERTEX_COORD);
      glIntFloatToList(CurObj[0]);
      glIntFloatToList(CurObj[1]);
      glIntFloatToList(CurObj[2]);
      glIntFloatToList(CurObj[3]);
    }

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  // Copy to context
  if(pglCurContext->NormalEnable) {
    pglCurContext->CurNormal[0] = CurNormal[0];
    pglCurContext->CurNormal[1] = CurNormal[1];
    pglCurContext->CurNormal[2] = CurNormal[2];
  }
  if(pglCurContext->ColorEnable) {
    pglCurContext->CurColor[0] = CurColor[0];
    pglCurContext->CurColor[1] = CurColor[1];
    pglCurContext->CurColor[2] = CurColor[2];
    pglCurContext->CurColor[3] = CurColor[3];
  }
  if(pglCurContext->TexEnable) {
    pglCurContext->CurTex[0] = CurTex[0];
    pglCurContext->CurTex[1] = CurTex[1];
    pglCurContext->CurTex[2] = CurTex[2];
    pglCurContext->CurTex[3] = CurTex[3];
  }
  if(pglCurContext->EdgeFlagEnable) {
    pglCurContext->CurEdge = CurEdge;
  }
  if(pglCurContext->VertexEnable) {
    pglCurContext->CurObj[0] = CurObj[0];
    pglCurContext->CurObj[1] = CurObj[1];
    pglCurContext->CurObj[2] = CurObj[2];
    pglCurContext->CurObj[3] = CurObj[3];
  }

  glIntVertex();
}







