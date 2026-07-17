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

void APIENTRY glNewList (GLuint list, GLenum mode)
{
  GLINT_OUTSIDE_BEGIN();

  if(pglCurContext->Listing == TRUE) {
    GLINT_ERROR(GL_INVALID_OPERATION);
    return;
  }

  if(list == 0) {
    GLINT_ERROR(GL_INVALID_VALUE);
    return;
  }

  switch(mode) {
  case GL_COMPILE:
    break;
  case GL_COMPILE_AND_EXECUTE:
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }

  if((pglCurContext->BuildListStart = 
      (unsigned int *)malloc(GLINT_LIST_SIZE*4)) == NULL) {
    pglCurContext->BuildProblem = TRUE;
    return;
  } else {
    pglCurContext->BuildProblem = FALSE;
  }

  pglCurContext->BuildDataStart = NULL;
  pglCurContext->BuildCurBlock = pglCurContext->BuildListStart;
  *(pglCurContext->BuildCurBlock) = 0x0;
  pglCurContext->BuildCur = pglCurContext->BuildCurBlock + 1;
  pglCurContext->BuildLeft = GLINT_LIST_SIZE - 1;
  pglCurContext->BuildName = list;

  pglCurContext->Execute = mode;
  pglCurContext->Listing = TRUE;
}

void APIENTRY glEndList (void)
{
  pglListBlock curlist;
  
  GLINT_OUTSIDE_BEGIN();

  if(pglCurContext->Listing == FALSE) {
    GLINT_ERROR(GL_INVALID_OPERATION);
    return;
  }

  if(pglCurContext->BuildProblem) {
    int *curptr, *nextptr;

    GLINT_ERROR(GL_OUT_OF_MEMORY);

    // Cleanup any used list blocks
    curptr = pglCurContext->BuildListStart;
    while(curptr != NULL) {
      nextptr = (int *)(*curptr);
      free(curptr);
      curptr = nextptr;
    }

    // Cleanup any used data blocks
    curptr = pglCurContext->BuildDataStart;
    while(curptr != NULL) {
      nextptr = (int *)(*curptr);
      free(curptr);
      curptr = nextptr;
    }
    return;
  }

  // Terminate list
  glIntIntToList(OP_END_LIST);

  // Search for name
  curlist = pglCurContext->ListHead;

  while(curlist != NULL) {
    if(curlist->ListName == pglCurContext->BuildName)
      break;
    curlist = curlist->Next;
  }

  if(curlist != NULL) {
    int *curptr, *nextptr;
    
    // delete existing lists
    curptr = curlist->ListBlocks;
    while(curptr != NULL) {
      nextptr = (int *)(*curptr);
      free(curptr);
      curptr = nextptr;
    }

    // delete existing data blocks
    curptr = curlist->DataBlocks;
    while(curptr != NULL) {
      nextptr = (int *)(*curptr);
      free(curptr);
      curptr = nextptr;
    }
  } else {
    // Create a new list block
    curlist = (pglListBlock)malloc(sizeof(glListBlock));

    // Link it into head
    curlist->Next = pglCurContext->ListHead;
    pglCurContext->ListHead = curlist;
    curlist->ListName = pglCurContext->BuildName;
  }

  // point to data blocks
  curlist->ListBlocks = pglCurContext->BuildListStart;
  curlist->DataBlocks = pglCurContext->BuildDataStart;

  pglCurContext->Listing = FALSE;
}

void APIENTRY glCallList (GLuint list)
{
  pglListBlock curlist;
  
  if(pglCurContext->Listing) {

    glIntIntToList(OP_CALLLIST);
    glIntIntToList(list);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  // find list
  curlist = pglCurContext->ListHead;
  while(curlist != NULL) {
    if(curlist->ListName == list)
      break;
    curlist = curlist->Next;
  }

  if(curlist != NULL) {
    int curop;
    
    // Setup *FromList
    pglCurContext->ExecCurBlock[pglCurContext->ExecLevel] =
      curlist->ListBlocks;
    pglCurContext->ExecCur[pglCurContext->ExecLevel] = 
      pglCurContext->ExecCurBlock[pglCurContext->ExecLevel] + 1;
    pglCurContext->ExecLeft[pglCurContext->ExecLevel] = 
      GLINT_LIST_SIZE - 1;

    while(((curop = glIntIntFromList()) != OP_END_LIST) ||
	  (pglCurContext->ExecLevel != 0))
      glIntExecBlock(curop);

    // Restore in context values
    pglCurContext->CurColor = pglCurContext->LocCurColor;
    pglCurContext->CurTex = pglCurContext->LocCurTex;
    pglCurContext->CurNormal = pglCurContext->LocCurNormal;
    pglCurContext->CurObj = pglCurContext->LocCurObj;
  }
}

void APIENTRY glCallLists (GLsizei n, GLenum type, const GLvoid *lists)
{
  int i;
  GLuint list;

  switch(type) {
  case GL_BYTE:
  case GL_UNSIGNED_BYTE:
  case GL_SHORT:
  case GL_UNSIGNED_SHORT:
  case GL_INT:
  case GL_UNSIGNED_INT:
  case GL_FLOAT:
  case GL_2_BYTES:
  case GL_3_BYTES:
  case GL_4_BYTES:
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }
  
  for(i=0;i<n;i++) {
    switch(type) {
    case GL_BYTE:
      list = (GLuint)*((char *)lists)++;
      break;
    case GL_UNSIGNED_BYTE:
      list = (GLuint)*((unsigned char *)lists)++;
      break;
    case GL_SHORT:
      list = (GLuint)*((short *)lists)++;
      break;
    case GL_UNSIGNED_SHORT:
      list = (GLuint)*((unsigned short *)lists)++;
      break;
    case GL_INT:
      list = (GLuint)*((int *)lists)++;
      break;
    case GL_UNSIGNED_INT:
      list = (GLuint)*((unsigned int *)lists)++;
      break;
    case GL_FLOAT:
      list = (GLuint)*((float *)lists)++;
      break;
    case GL_2_BYTES:
      list = ((GLuint)*((unsigned char *)lists)++)<<8;
      list |= (GLuint)*((unsigned char *)lists)++;
      break;
    case GL_3_BYTES:
      list = ((GLuint)*((unsigned char *)lists)++)<<16;
      list |= ((GLuint)*((unsigned char *)lists)++)<<8;
      list |= (GLuint)*((unsigned char *)lists)++;
      break;
    case GL_4_BYTES:
      list = ((GLuint)*((unsigned char *)lists)++)<<24;
      list |= ((GLuint)*((unsigned char *)lists)++)<<16;
      list |= ((GLuint)*((unsigned char *)lists)++)<<8;
      list |= (GLuint)*((unsigned char *)lists)++;
      break;
    }

    if(pglCurContext->Listing) {
      
      glIntIntToList(OP_CALLLIST_OFFSET);
      glIntIntToList(list);
      
      if(pglCurContext->Execute == GL_COMPILE)
	return;
    }
    
    glCallList(list + pglCurContext->ListBase);
  }
}

void APIENTRY glDeleteLists (GLuint list, GLsizei range)
{
  pglListBlock curlist, lastlist;
  GLuint name;

  GLINT_OUTSIDE_BEGIN();

  if(range == 0)
    return;

  // repeat for lists in range
  for(name=list;name<list+range;name++) {

    // Search for name
    curlist = pglCurContext->ListHead;
    lastlist = NULL;

    while(curlist != NULL) {
      if(curlist->ListName == name)
	break;
      lastlist = curlist;
      curlist = curlist->Next;
    }
    
    if(curlist != NULL) {
      int *curptr, *nextptr;
      
      // delete existing list blocks
      curptr = curlist->ListBlocks;
      while(curptr != NULL) {
	nextptr = (int *)(*curptr);
	free(curptr);
	curptr = nextptr;
      }

      // delete existing data blocks
      curptr = curlist->DataBlocks;
      while(curptr != NULL) {
	nextptr = (int *)(*curptr);
	free(curptr);
	curptr = nextptr;
      }

      if(lastlist == NULL) {
	// remove from head of list
	pglCurContext->ListHead = curlist->Next;
	free(curlist);
      } else {
	// remove from somewhere in list
	lastlist->Next = curlist->Next;
	free(curlist);
      }
    }
  }
}

GLuint APIENTRY glGenLists (GLsizei range)
{
  GLuint startname, curname, currange;
  pglListBlock curlist;
  int i;
  
  GLINT_OUTSIDE_BEGIN();
  
  if(range == 0)
    return((GLuint)0);

  startname = 1;
  curname = 1;
  currange = range;
  
  while(1) {
    // Search for name
    curlist = pglCurContext->ListHead;
    
    while(curlist != NULL) {
      if(curlist->ListName == curname)
	break;
      curlist = curlist->Next;
    }
    
    if(curlist != NULL) {
      currange = range;
      curname++;
      startname = curname;
    } else {
      if(--currange == 0) {
	break;
      }
      if(curname == 0xffffffff) {
	return((GLuint)0);
      }
    curname++;
    }
  }

  // mark lists as null but used
  curname = startname;
  for(i=0;i<range;i++) {
    // Create a new list block
    curlist = (pglListBlock)malloc(sizeof(glListBlock));

    // Link it into head
    curlist->Next = pglCurContext->ListHead;
    pglCurContext->ListHead = curlist;
    curlist->ListName = curname++;

    // null data blocks
    curlist->ListBlocks = NULL;
    curlist->DataBlocks = NULL;
  }
  
  return(startname);
}

GLboolean APIENTRY glIsList (GLuint list)
{
  pglListBlock curlist;

  GLINT_OUTSIDE_BEGIN();

  // Search for name
  curlist = pglCurContext->ListHead;

  while(curlist != NULL) {
    if(curlist->ListName == list)
      break;
    curlist = curlist->Next;
  }
    
  if(curlist != NULL)
    return((GLboolean)TRUE);
  else
    return((GLboolean)FALSE);
}

void APIENTRY glListBase (GLuint base)
{
  GLINT_OUTSIDE_BEGIN();

  if(pglCurContext->Listing) {

    glIntIntToList(OP_LISTBASE);
    glIntIntToList(base);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->ListBase = base;
}

void glIntIntToList(int op)
{
  if(pglCurContext->BuildLeft == 0) {
    // Generate and link a new block
    unsigned int *newblock;
    
    if((newblock = (unsigned int *)malloc(GLINT_LIST_SIZE*4)) == NULL) {
      pglCurContext->BuildProblem = TRUE;
      return;
    } else {
      pglCurContext->BuildProblem = FALSE;
    }
    
    *(pglCurContext->BuildCurBlock) = (unsigned int)newblock;
    pglCurContext->BuildCurBlock = newblock;
    *(pglCurContext->BuildCurBlock) = 0x0;
    pglCurContext->BuildCur = newblock + 1;
    pglCurContext->BuildLeft = GLINT_LIST_SIZE - 1;
  }

  // Add value to list
  if(!pglCurContext->BuildProblem) {
    pglCurContext->BuildLeft--;
    *(pglCurContext->BuildCur)++ = op;
  }
}

void glIntMakeRoom(unsigned int size)
{
  if(!pglCurContext->BuildProblem) {
// Appears not to work with Left < Size ???
    if(pglCurContext->BuildLeft <= size) {
      // Generate and link a new block
      unsigned int *newblock;

      // Store go to next block in current block
      *(pglCurContext->BuildCur) = OP_NEXT_BLOCK;

      if((newblock = (unsigned int *)malloc(GLINT_LIST_SIZE*4)) == NULL) {
	pglCurContext->BuildProblem = TRUE;
	return;
      } else {
	pglCurContext->BuildProblem = FALSE;
      }
    
      *(pglCurContext->BuildCurBlock) = (unsigned int)newblock;
      pglCurContext->BuildCurBlock = newblock;
      *(pglCurContext->BuildCurBlock) = 0x0;
      pglCurContext->BuildCur = newblock + 1;
      pglCurContext->BuildLeft = GLINT_LIST_SIZE - 1;
    }
  }
}

int glIntIntFromList(void)
{
  if(pglCurContext->ExecLeft[pglCurContext->ExecLevel] == 0) {
    // Get a new block
    pglCurContext->ExecCurBlock[pglCurContext->ExecLevel] = 
      (unsigned int *)*(pglCurContext->ExecCurBlock[pglCurContext->ExecLevel]);
    pglCurContext->ExecCur[pglCurContext->ExecLevel] =
      pglCurContext->ExecCurBlock[pglCurContext->ExecLevel] + 1;
    pglCurContext->ExecLeft[pglCurContext->ExecLevel] = GLINT_LIST_SIZE - 1;
  }

  // Get value from list
  pglCurContext->ExecLeft[pglCurContext->ExecLevel]--;
  return(*(pglCurContext->ExecCur[pglCurContext->ExecLevel])++);
}

void glIntFloatToList(float val)
{
  if(pglCurContext->BuildLeft == 0) {
    // Generate and link a new block
    unsigned int *newblock;
    
    if((newblock = (unsigned int *)malloc(GLINT_LIST_SIZE*4)) == NULL) {
      pglCurContext->BuildProblem = TRUE;
      return;
    } else {
      pglCurContext->BuildProblem = FALSE;
    }
    
    *(pglCurContext->BuildCurBlock) = (unsigned int)newblock;
    pglCurContext->BuildCurBlock = newblock;
    *(pglCurContext->BuildCurBlock) = 0x0;
    pglCurContext->BuildCur = newblock + 1;
    pglCurContext->BuildLeft = GLINT_LIST_SIZE - 1;
  }

  // Add value to list
  if(!pglCurContext->BuildProblem) {
    pglCurContext->BuildLeft--;
    *((float *)(pglCurContext->BuildCur))++ = val;
  }
}

float glIntFloatFromList(void)
{
  if(pglCurContext->ExecLeft[pglCurContext->ExecLevel] == 0) {
    // Get a new block
    pglCurContext->ExecCurBlock[pglCurContext->ExecLevel] = 
      (unsigned int *)*(pglCurContext->ExecCurBlock[pglCurContext->ExecLevel]);
    pglCurContext->ExecCur[pglCurContext->ExecLevel] = 
      pglCurContext->ExecCurBlock[pglCurContext->ExecLevel] + 1;
    pglCurContext->ExecLeft[pglCurContext->ExecLevel] = GLINT_LIST_SIZE - 1;
  }

  // Get value from list
  pglCurContext->ExecLeft[pglCurContext->ExecLevel]--;
  return(*((float *)(pglCurContext->ExecCur[pglCurContext->ExecLevel]))++);
}

void glIntExecBlock(int op)
{
  switch(op) {
  case OP_VERTEX_COORD:
//    pglCurContext->CurObj[0] = glIntFloatFromList(); 
//    pglCurContext->CurObj[1] = glIntFloatFromList(); 
//    pglCurContext->CurObj[2] = glIntFloatFromList(); 
//    pglCurContext->CurObj[3] = glIntFloatFromList(); 

    pglCurContext->CurObj = 
      (float *)(pglCurContext->ExecCur[pglCurContext->ExecLevel]);
    pglCurContext->ExecLeft[pglCurContext->ExecLevel] -= 4;
    pglCurContext->ExecCur[pglCurContext->ExecLevel] += 4;
    
    glIntVertex();
    break;
  case OP_NORMAL_COORD:
//    pglCurContext->CurNormal[0] = glIntFloatFromList();
//    pglCurContext->CurNormal[1] = glIntFloatFromList();
//    pglCurContext->CurNormal[2] = glIntFloatFromList();

    pglCurContext->CurNormal = 
      (float *)(pglCurContext->ExecCur[pglCurContext->ExecLevel]);
    pglCurContext->ExecLeft[pglCurContext->ExecLevel] -= 3;
    pglCurContext->ExecCur[pglCurContext->ExecLevel] += 3;
    break;
  case OP_TEXTURE_COORD:
//    pglCurContext->CurTex[0] = glIntFloatFromList();
//    pglCurContext->CurTex[1] = glIntFloatFromList();
//    pglCurContext->CurTex[2] = glIntFloatFromList();
//    pglCurContext->CurTex[3] = glIntFloatFromList();
    pglCurContext->CurTex = 
      (float *)(pglCurContext->ExecCur[pglCurContext->ExecLevel]);
    pglCurContext->ExecLeft[pglCurContext->ExecLevel] -= 4;
    pglCurContext->ExecCur[pglCurContext->ExecLevel] += 4;
    break;
  case OP_COLOR_COORD:
//    pglCurContext->CurColor[0] = glIntFloatFromList();
//    pglCurContext->CurColor[1] = glIntFloatFromList();
//    pglCurContext->CurColor[2] = glIntFloatFromList();
//    pglCurContext->CurColor[3] = glIntFloatFromList();
    pglCurContext->CurColor = 
      (float *)(pglCurContext->ExecCur[pglCurContext->ExecLevel]);
    pglCurContext->ExecLeft[pglCurContext->ExecLevel] -= 4;
    pglCurContext->ExecCur[pglCurContext->ExecLevel] += 4;
    break;
  case OP_EDGE_COORD:
    pglCurContext->CurEdge = (boolean)glIntIntFromList();
    break;
  case OP_BEGIN:
    pglCurContext->PrimAssemState = glIntIntFromList();
    pglCurContext->CurLine = (GLboolean)glIntIntFromList();
    pglCurContext->CurPoly = (GLboolean)glIntIntFromList();
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
    break;
  case OP_END:
    pglCurContext->BeginEnd = FALSE;
    glIntClose();
    break;
  case OP_POINTSIZE:
    pglCurContext->PointWidth = glIntFloatFromList();
    break;
  case OP_LINEWIDTH:
    pglCurContext->LineWidth = glIntFloatFromList();
    break;
  case OP_LINESTIPPLE:
    {
      int i;

      pglCurContext->LineStippleFactor = glIntIntFromList();
      pglCurContext->LinePattern = (short)glIntIntFromList();
      
      for(i=0;i<16;i++)
	pglCurContext->LineStippleData[i] =
	  (unsigned char)glIntIntFromList();

      pglCurContext->LineStippleDirty = TRUE;
      pglCurContext->ContextDirty = TRUE;
    }
    break;
  case OP_FRONTFACE:
    pglCurContext->FrontFace = glIntIntFromList();
    break;
  case OP_CULLFACE:
    pglCurContext->CullMode = glIntIntFromList();
    break;
  case OP_SHADEMODEL:
    pglCurContext->ShadeModel = glIntIntFromList();
    pglCurContext->ContextDirty = TRUE;
    break;
  case OP_FRONTMODE:
    pglCurContext->PolygonFrontMode = glIntIntFromList();
    break;
  case OP_BACKMODE:
    pglCurContext->PolygonBackMode = glIntIntFromList();
    break;
  case OP_POLYOFFSET:
    pglCurContext->DepthBiasLevel = (FxI16)glIntIntFromList();
    pglCurContext->ContextDirty = TRUE;
    break;
  case OP_POLYSTIPPLE:
    {
      unsigned char *stipple;

      stipple = (unsigned char *)glIntIntFromList();

      memcpy(pglCurContext->PolyStippleData,stipple,32*32);

      pglCurContext->PolyStippleDirty = TRUE;
      pglCurContext->ContextDirty = TRUE;
    }
    break;
  case OP_ALPHAFUNC:
    pglCurContext->gAlphaFunc = (GrCmpFnc_t)glIntIntFromList();
    pglCurContext->AlphaFunc = glIntIntFromList();
    pglCurContext->AlphaRef = (GrAlpha_t)glIntIntFromList();
    pglCurContext->ContextDirty = TRUE;
    break;
  case OP_BLENDFUNC:
    pglCurContext->gSrcBlend = (GrAlphaBlendFnc_t)glIntIntFromList();
    pglCurContext->SrcBlend = glIntIntFromList();
    pglCurContext->gDstBlend = (GrAlphaBlendFnc_t)glIntIntFromList();
    pglCurContext->DstBlend = glIntIntFromList();
    pglCurContext->ContextDirty = TRUE;
    break;
  case OP_DEPTHFUNC:
    pglCurContext->gDepthFunc = (GrCmpFnc_t)glIntIntFromList();
    pglCurContext->DepthFunc = glIntIntFromList();
    pglCurContext->ContextDirty = TRUE;
    break;
  case OP_CLIPPLANE:
    {
      int pl;

      pl = glIntIntFromList();
      pglCurContext->ClipDef[pl][0] = glIntFloatFromList();
      pglCurContext->ClipDef[pl][1] = glIntFloatFromList();
      pglCurContext->ClipDef[pl][2] = glIntFloatFromList();
      pglCurContext->ClipDef[pl][3] = glIntFloatFromList();
      
      glIntValidateInvTransp();
      glIntXform(pglCurContext->ClipDef[pl],
		 pglCurContext->CurInvTransp,
		 pglCurContext->ClipXform[pl]);
    }
    break;
  case GL_FOG_MODE:
    pglCurContext->FogMode = glIntIntFromList();

    glIntInitFog(pglCurContext->FogTable,
		 pglCurContext->FogMode,
		 pglCurContext->FogDensity,
		 pglCurContext->FogStart,
		 pglCurContext->FogEnd);
    
    pglCurContext->ContextDirty = TRUE;
    break;
  case GL_FOG_DENSITY:
    pglCurContext->FogDensity = glIntFloatFromList();

    glIntInitFog(pglCurContext->FogTable,
		 pglCurContext->FogMode,
		 pglCurContext->FogDensity,
		 pglCurContext->FogStart,
		 pglCurContext->FogEnd);
    
    pglCurContext->ContextDirty = TRUE;
    break;
  case GL_FOG_START:
    pglCurContext->FogStart = glIntFloatFromList();

    glIntInitFog(pglCurContext->FogTable,
		 pglCurContext->FogMode,
		 pglCurContext->FogDensity,
		 pglCurContext->FogStart,
		 pglCurContext->FogEnd);
    
    pglCurContext->ContextDirty = TRUE;
    break;
  case GL_FOG_END:
    pglCurContext->FogEnd = glIntFloatFromList();

    glIntInitFog(pglCurContext->FogTable,
		 pglCurContext->FogMode,
		 pglCurContext->FogDensity,
		 pglCurContext->FogStart,
		 pglCurContext->FogEnd);
    
    pglCurContext->ContextDirty = TRUE;
    break;
  case GL_FOG_COLOR:
    pglCurContext->FogColor = glIntIntFromList();

    glIntInitFog(pglCurContext->FogTable,
		 pglCurContext->FogMode,
		 pglCurContext->FogDensity,
		 pglCurContext->FogStart,
		 pglCurContext->FogEnd);
    
    pglCurContext->ContextDirty = TRUE;
    break;
  case OP_DEPTHRANGE:
    pglCurContext->zNear = glIntFloatFromList();
    pglCurContext->zFar = glIntFloatFromList();
    pglCurContext->zScale = (float)((pglCurContext->zFar -
				     pglCurContext->zNear)/2.0);
    pglCurContext->zOffset = (float)((pglCurContext->zNear +
				      pglCurContext->zFar)/2.0);
    break;
  case OP_VIEWPORT:
    pglCurContext->ViewX = glIntIntFromList();
    pglCurContext->ViewY = glIntIntFromList();
    pglCurContext->ViewWidth = glIntIntFromList();
    pglCurContext->ViewHeight = glIntIntFromList();
    pglCurContext->xScale = (float)(pglCurContext->ViewWidth/2);
    pglCurContext->xOffset = (float)(pglCurContext->ViewX+
				     (pglCurContext->ViewWidth/2))+SNAP_BIAS;
    pglCurContext->yScale = (float)(pglCurContext->ViewHeight/2);
    pglCurContext->yOffset = (float)(pglCurContext->ViewY+
				     (pglCurContext->ViewHeight/2))+SNAP_BIAS;
    break;
  case OP_SCISSOR:
    pglCurContext->minx = glIntIntFromList();
    pglCurContext->miny = glIntIntFromList();
    pglCurContext->maxx = glIntIntFromList();
    pglCurContext->maxy = glIntIntFromList();

    pglCurContext->ContextDirty = TRUE;
    break;
  case OP_MATRIXMODE:
    pglCurContext->MatMode = glIntIntFromList();
    break;
  case OP_LOADIDENTITY:
    switch(pglCurContext->MatMode) {
    case GL_MODELVIEW:
      glIntIdentity(pglCurContext->CurModelView);
      glIntIdentity(pglCurContext->CurInvTransp);
      pglCurContext->InvTranspDirty = FALSE;
      pglCurContext->CompositeDirty = TRUE;
      break;
    case GL_PROJECTION:
      glIntIdentity(pglCurContext->CurProjMat);
      pglCurContext->CompositeDirty = TRUE;
      break;
    case GL_TEXTURE:
      glIntIdentity(pglCurContext->CurTexMat);
      pglCurContext->TexMatrixIdentity = TRUE;
      break;
    }
    break;
  case OP_LOADMATRIX:
    {
      int i;
      GLfloat m[16];

      for(i=0;i<16;i++)
	m[i] = glIntFloatFromList();

      switch(pglCurContext->MatMode) {
      case GL_MODELVIEW:
	glIntMatCopy(m,pglCurContext->CurModelView);
	// Calculate inverse transpose for normals
	pglCurContext->InvTranspDirty = TRUE;
	pglCurContext->CompositeDirty = TRUE;
	break;
      case GL_PROJECTION:
	glIntMatCopy(m,pglCurContext->CurProjMat);
	pglCurContext->CompositeDirty = TRUE;
	break;
      case GL_TEXTURE:
	glIntMatCopy(m,pglCurContext->CurTexMat);
	// Eventually Characterize?
        pglCurContext->TexMatrixIdentity = FALSE;
	break;
      }
    }
    break;
  case OP_MULTMATRIX:
    
  {    int i;
      GLfloat m[16];

      for(i=0;i<16;i++)
	m[i] = glIntFloatFromList();

      switch(pglCurContext->MatMode) {
      case GL_MODELVIEW:
	glIntMatMult(m,pglCurContext->CurModelView);
	// Calculate inverse transpose for normals
	pglCurContext->InvTranspDirty = TRUE;
	pglCurContext->CompositeDirty = TRUE;
	break;
      case GL_PROJECTION:
	glIntMatMult(m,pglCurContext->CurProjMat);
	pglCurContext->CompositeDirty = TRUE;
	break;
      case GL_TEXTURE:
	glIntMatMult(m,pglCurContext->CurTexMat);
	// Eventually Characterize?
        pglCurContext->TexMatrixIdentity = FALSE;
	break;
      }
    }
    break;
  case OP_POPMATRIX:
    switch(pglCurContext->MatMode) {
    case GL_MODELVIEW:
      if(pglCurContext->ModelViewPos > 0) {
	pglCurContext->ModelViewPos--;
	glIntMatCopy(pglCurContext->ModelView[pglCurContext->ModelViewPos],
		     pglCurContext->CurModelView);
	
	// Calculate inverse transpose for normals
	pglCurContext->InvTranspDirty = TRUE;
	pglCurContext->CompositeDirty = TRUE;
      } else {
	GLINT_ERROR(GL_STACK_UNDERFLOW);
      }
      break;
    case GL_PROJECTION:
      if(pglCurContext->ProjMatPos > 0) {
	pglCurContext->ProjMatPos--;
	glIntMatCopy(pglCurContext->ProjMat[pglCurContext->ProjMatPos],
		     pglCurContext->CurProjMat);
      } else {
	GLINT_ERROR(GL_STACK_UNDERFLOW);
      }
      break;
    case GL_TEXTURE:
      if(pglCurContext->TexMatPos > 0) {
	pglCurContext->TexMatPos--;
	glIntMatCopy(pglCurContext->TexMat[pglCurContext->TexMatPos],
		     pglCurContext->CurTexMat);
	// Eventually Characterize?
        pglCurContext->TexMatrixIdentity = FALSE;
      } else {
	GLINT_ERROR(GL_STACK_UNDERFLOW);
      }
      break;
    }
    break;
  case OP_PUSHMATRIX:
    switch(pglCurContext->MatMode) {
    case GL_MODELVIEW:
      if(pglCurContext->ModelViewPos < 32) {
	glIntMatCopy(pglCurContext->CurModelView,
		     pglCurContext->ModelView[pglCurContext->ModelViewPos]);
	pglCurContext->ModelViewPos++;
      } else {
	GLINT_ERROR(GL_STACK_OVERFLOW);
      }
      break;
    case GL_PROJECTION:
      if(pglCurContext->ProjMatPos < 2) {
	glIntMatCopy(pglCurContext->CurProjMat,
		     pglCurContext->ProjMat[pglCurContext->ProjMatPos]);
	pglCurContext->ProjMatPos++;
      } else {
	GLINT_ERROR(GL_STACK_OVERFLOW);
      }
      break;
    case GL_TEXTURE:
      if(pglCurContext->TexMatPos < 2) {
	glIntMatCopy(pglCurContext->CurTexMat,
		     pglCurContext->TexMat[pglCurContext->TexMatPos]);
	pglCurContext->TexMatPos++;
      } else {
	GLINT_ERROR(GL_STACK_OVERFLOW);
      }
      break;
    }
    break;
  case OP_SETBOOL:
    switch(glIntIntFromList()) {
    case GL_DEPTH_TEST:
      pglCurContext->DepthBuffer = (boolean)glIntIntFromList();
      break;
    case GL_STENCIL_TEST:
      pglCurContext->StencilTest = (boolean)glIntIntFromList();
      break;
    case GL_NORMALIZE:
      pglCurContext->Normalize = (boolean)glIntIntFromList();
      break;
    case GL_LIGHT0:
      pglCurContext->LightEna[0] = (boolean)glIntIntFromList();
      pglCurContext->NeedEyeDirty = TRUE;
      pglCurContext->LightDirty = TRUE;
      break;
    case GL_LIGHT1:
      pglCurContext->LightEna[1] = (boolean)glIntIntFromList();
      pglCurContext->NeedEyeDirty = TRUE;
      pglCurContext->LightDirty = TRUE;
      break;
    case GL_LIGHT2:
      pglCurContext->LightEna[2] = (boolean)glIntIntFromList();
      pglCurContext->NeedEyeDirty = TRUE;
      pglCurContext->LightDirty = TRUE;
      break;
    case GL_LIGHT3:
      pglCurContext->LightEna[3] = (boolean)glIntIntFromList();
      pglCurContext->NeedEyeDirty = TRUE;
      pglCurContext->LightDirty = TRUE;
      break;
    case GL_LIGHT4:
      pglCurContext->LightEna[4] = (boolean)glIntIntFromList();
      pglCurContext->NeedEyeDirty = TRUE;
      pglCurContext->LightDirty = TRUE;
      break;
    case GL_LIGHT5:
      pglCurContext->LightEna[5] = (boolean)glIntIntFromList();
      pglCurContext->NeedEyeDirty = TRUE;
      pglCurContext->LightDirty = TRUE;
      break;
    case GL_LIGHT6:
      pglCurContext->LightEna[6] = (boolean)glIntIntFromList();
      pglCurContext->NeedEyeDirty = TRUE;
      pglCurContext->LightDirty = TRUE;
      break;
    case GL_LIGHT7:
      pglCurContext->LightEna[7] = (boolean)glIntIntFromList();
      pglCurContext->NeedEyeDirty = TRUE;
      pglCurContext->LightDirty = TRUE;
      break;
    case GL_CLIP_PLANE0:
      pglCurContext->ClipEna[0] = (boolean)glIntIntFromList();
      break;
    case GL_CLIP_PLANE1:
      pglCurContext->ClipEna[1] = (boolean)glIntIntFromList();
      break;
    case GL_CLIP_PLANE2:
      pglCurContext->ClipEna[2] = (boolean)glIntIntFromList();
      break;
    case GL_CLIP_PLANE3:
      pglCurContext->ClipEna[3] = (boolean)glIntIntFromList();
      break;
    case GL_CLIP_PLANE4:
      pglCurContext->ClipEna[4] = (boolean)glIntIntFromList();
      break;
    case GL_CLIP_PLANE5:
      pglCurContext->ClipEna[5] = (boolean)glIntIntFromList();
      break;
    case GL_LIGHTING:
      pglCurContext->Lighting = (boolean)glIntIntFromList();
      pglCurContext->NeedEyeDirty = TRUE;
      break;
    case GL_COLOR_MATERIAL:
      pglCurContext->ColorMaterial = (boolean)glIntIntFromList();
      pglCurContext->LightDirty = TRUE;
      break;
    case GL_CULL_FACE:
      pglCurContext->CullEnable = (boolean)glIntIntFromList();
      break;
    case GL_SCISSOR_TEST:
      pglCurContext->ScissorEnable = (boolean)glIntIntFromList();
      break;
    case GL_POINT_SMOOTH:
      pglCurContext->PointAA = (boolean)glIntIntFromList();
      break;
    case GL_LINE_SMOOTH:
      pglCurContext->LineAA = (boolean)glIntIntFromList();
      break;
    case GL_POLYGON_SMOOTH:
      pglCurContext->PolyAA = (boolean)glIntIntFromList();
      break;
    case GL_ALPHA_TEST:
      pglCurContext->AlphaTest = (boolean)glIntIntFromList();
      break;
    case GL_BLEND:
      pglCurContext->Blend = (boolean)glIntIntFromList();
      break;
    case GL_TEXTURE_1D:
      pglCurContext->Tex1D = (boolean)glIntIntFromList();
      pglCurContext->TexDirty = TRUE;
      break;
    case GL_TEXTURE_2D:
      pglCurContext->Tex2D = (boolean)glIntIntFromList();
      pglCurContext->TexDirty = TRUE;
      break;
    case GL_POLYGON_OFFSET_FILL:
      pglCurContext->PolyOffset = (boolean)glIntIntFromList();
      break;
    case GL_POLYGON_OFFSET_LINE:
      pglCurContext->LineOffset = (boolean)glIntIntFromList();
      break;
    case GL_POLYGON_OFFSET_POINT:
      pglCurContext->PointOffset = (boolean)glIntIntFromList();
      break;
    case GL_FOG:
      pglCurContext->Fog = (boolean)glIntIntFromList();
      break;
    case GL_AUTO_NORMAL:
      pglCurContext->AutoNormal = (boolean)glIntIntFromList();
      break;
    case GL_LINE_STIPPLE:
      pglCurContext->LineStipple = (boolean)glIntIntFromList();
      break;
    case GL_POLYGON_STIPPLE:
      pglCurContext->PolyStipple = (boolean)glIntIntFromList();
      break;
    case GL_DITHER:
      pglCurContext->Dither = (boolean)glIntIntFromList();
      break;
    }

    pglCurContext->ContextDirty = TRUE;

    break;
  case OP_CLEARDEPTH:
    pglCurContext->GLClrDepth = glIntFloatFromList();
    pglCurContext->ClrDepth = (FxU16)(pglCurContext->GLClrDepth*65535.0);
    break;
  case OP_CLEARCOLOR:
    pglCurContext->ColorClrRed = glIntFloatFromList();
    pglCurContext->ColorClrGreen = glIntFloatFromList();
    pglCurContext->ColorClrBlue = glIntFloatFromList();
    pglCurContext->ColorClrAlpha = glIntFloatFromList();
    pglCurContext->ClrColor = 
      (((FxU8)(pglCurContext->ColorClrBlue*255.0))<<16)|
      (((FxU8)(pglCurContext->ColorClrGreen*255.0))<<8)|
	((FxU8)(pglCurContext->ColorClrRed*255.0));
    pglCurContext->ClrAlpha = (FxU8)(pglCurContext->ColorClrAlpha*255.0);
    break;
  case OP_CLEARACCUM:
    pglCurContext->AccumClrRed = glIntFloatFromList();
    pglCurContext->AccumClrGreen = glIntFloatFromList();
    pglCurContext->AccumClrBlue = glIntFloatFromList();
    pglCurContext->AccumClrAlpha = glIntFloatFromList();
    break;
  case OP_DEPTHMASK:
    pglCurContext->DepthMask = (boolean)glIntIntFromList();

    pglCurContext->ContextDirty = TRUE;
    break;
  case OP_COLORMASK:
    pglCurContext->RedMask = (boolean)glIntIntFromList();
    pglCurContext->GreenMask = (boolean)glIntIntFromList();
    pglCurContext->BlueMask = (boolean)glIntIntFromList();
    pglCurContext->AlphaMask = (boolean)glIntIntFromList();
    
    pglCurContext->ContextDirty = TRUE;
    break;
  case OP_CLEAR:
    {
      int mask;
      
      mask = glIntIntFromList();
      
      if(mask&GL_COLOR_BUFFER_BIT) {
	// no alpha
	grColorMask(FXTRUE,FXFALSE);
      } else {
	grColorMask(FXFALSE,FXFALSE);
      }
      
      if(mask&GL_DEPTH_BUFFER_BIT) {
	grDepthMask(FXTRUE);
      } else {
	grDepthMask(FXFALSE);
      }
      
      if(mask&GL_ACCUM_BUFFER_BIT) {
      } else {
      }
      
      if(mask&GL_STENCIL_BUFFER_BIT) {
      } else {
      }
      
      grBufferClear(pglCurContext->ClrColor,
		    pglCurContext->ClrAlpha,
		    pglCurContext->ClrDepth);
      
      /* restore buffer masks*/
      pglCurContext->ContextDirty = TRUE;
    }
    break;
  case OP_STENCILOP:
    pglCurContext->StencilZFail = glIntIntFromList();
    pglCurContext->StencilFail = glIntIntFromList();
    pglCurContext->StencilZPass = glIntIntFromList();
    break;
  case OP_STENCILMASK:
    pglCurContext->StencilMask = glIntIntFromList();
    break;
  case OP_STENCILFUNC:
    pglCurContext->StencilFunc = glIntIntFromList();
    pglCurContext->StencilRef = glIntIntFromList();
    pglCurContext->StencilFuncMask = glIntIntFromList();
    break;
  case OP_RASTERPOS:
    pglCurContext->RasObj[0] = glIntFloatFromList();
    pglCurContext->RasObj[1] = glIntFloatFromList();
    pglCurContext->RasObj[2] = glIntFloatFromList();
    pglCurContext->RasObj[3] = glIntFloatFromList();
    
    glIntRasterPos();
    break;
  case OP_LOCALVIEWER:
    pglCurContext->Local_Viewer = (boolean)glIntIntFromList();
    pglCurContext->LightDirty = TRUE;
    break;
  case OP_TWOSIDE:
    pglCurContext->Two_Sided = (boolean)glIntIntFromList();
    pglCurContext->NeedEyeDirty = TRUE;
    pglCurContext->LightDirty = TRUE;
    break;
  case OP_LIGHTMODELAMBIENT:
    pglCurContext->LightModelAmbient[0] = glIntFloatFromList();
    pglCurContext->LightModelAmbient[1] = glIntFloatFromList();
    pglCurContext->LightModelAmbient[2] = glIntFloatFromList();
    pglCurContext->LightDirty = TRUE;
    break;
  case OP_SPOTEXPONENT:
    {
      int index;

      index = glIntIntFromList();

      pglCurContext->Light[index].SpotExponent = glIntFloatFromList();
      pglCurContext->LightDirty = TRUE;
    }
    break;
  case OP_SPOTCUTOFF:
    {
      int index;

      index = glIntIntFromList();

      pglCurContext->Light[index].SpotCutoff = glIntFloatFromList();
      pglCurContext->LightDirty = TRUE;
    }
    break;
  case OP_CONSTANTATTENUATION:
    {
      int index;

      index = glIntIntFromList();

      pglCurContext->Light[index].ConstantAtten = glIntFloatFromList();
      pglCurContext->LightDirty = TRUE;
    }
    break;
  case OP_LINEARATTENUATION:
    {
      int index;

      index = glIntIntFromList();

      pglCurContext->Light[index].LinearAtten = glIntFloatFromList();
      pglCurContext->LightDirty = TRUE;
    }
    break;
  case OP_QUADRATICATTENUATION:
    {
      int index;
      
      index = glIntIntFromList();
      
      pglCurContext->Light[index].QuadraticAtten = glIntFloatFromList();
      pglCurContext->LightDirty = TRUE;
    }
    break;
  case OP_AMBIENT:
    {
      int index;
      
      index = glIntIntFromList();

      pglCurContext->Light[index].Ambient[0] = glIntFloatFromList();
      pglCurContext->Light[index].Ambient[1] = glIntFloatFromList();
      pglCurContext->Light[index].Ambient[2] = glIntFloatFromList();
      pglCurContext->LightDirty = TRUE;
    }
    break;
  case OP_DIFFUSE:
    {
      int index;
      
      index = glIntIntFromList();
      
      pglCurContext->Light[index].Diffuse[0] = glIntFloatFromList();
      pglCurContext->Light[index].Diffuse[1] = glIntFloatFromList();
      pglCurContext->Light[index].Diffuse[2] = glIntFloatFromList();
      pglCurContext->Light[index].Diffuse[3] = glIntFloatFromList();
      pglCurContext->LightDirty = TRUE;
    }
    break;
  case OP_SPECULAR:
    {
      int index;
      
      index = glIntIntFromList();
      
      pglCurContext->Light[index].Specular[0] = glIntFloatFromList();
      pglCurContext->Light[index].Specular[1] = glIntFloatFromList();
      pglCurContext->Light[index].Specular[2] = glIntFloatFromList();
      pglCurContext->LightDirty = TRUE;
    }
    break;
  case OP_POSITION:
    {
      int index;
      GLfloat tmpparams[4];
      
      index = glIntIntFromList();
      
      tmpparams[0] = glIntFloatFromList();
      tmpparams[1] = glIntFloatFromList();
      tmpparams[2] = glIntFloatFromList();
      tmpparams[3] = glIntFloatFromList();
      
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
      pglCurContext->LightDirty = TRUE;
    }
    break;
  case OP_SPOTDIRECTION:
    {
      int index;
      GLfloat tmpparams[4];
      
      index = glIntIntFromList();
      
      tmpparams[0] = glIntFloatFromList();
      tmpparams[1] = glIntFloatFromList();
      tmpparams[2] = glIntFloatFromList();
      tmpparams[3] = glIntFloatFromList();

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
      pglCurContext->LightDirty = TRUE;
    }
    break;
  case OP_FRONTSHININESS:
    pglCurContext->FrontShininess = glIntFloatFromList();
    pglCurContext->LightDirty = TRUE;
    break;
  case OP_FRONTAMBIENT:
    pglCurContext->FrontAmbient[0] = glIntFloatFromList();
    pglCurContext->FrontAmbient[1] = glIntFloatFromList();
    pglCurContext->FrontAmbient[2] = glIntFloatFromList();
    pglCurContext->LightDirty = TRUE;
    break;
  case OP_FRONTDIFFUSE:
    pglCurContext->FrontDiffuse[0] = glIntFloatFromList();
    pglCurContext->FrontDiffuse[1] = glIntFloatFromList();
    pglCurContext->FrontDiffuse[2] = glIntFloatFromList();
    pglCurContext->FrontDiffuse[3] = glIntFloatFromList();
    pglCurContext->LightDirty = TRUE;
    break;
  case OP_FRONTSPECULAR:
    pglCurContext->FrontSpecular[0] = glIntFloatFromList();
    pglCurContext->FrontSpecular[1] = glIntFloatFromList();
    pglCurContext->FrontSpecular[2] = glIntFloatFromList();
    pglCurContext->LightDirty = TRUE;
    break;
  case OP_FRONTEMISSION:
    pglCurContext->FrontEmission[0] = glIntFloatFromList();
    pglCurContext->FrontEmission[1] = glIntFloatFromList();
    pglCurContext->FrontEmission[2] = glIntFloatFromList();
    pglCurContext->LightDirty = TRUE;
    break;
  case OP_BACKSHININESS:
    pglCurContext->BackShininess = glIntFloatFromList();
    pglCurContext->LightDirty = TRUE;
    break;
  case OP_BACKAMBIENT:
    pglCurContext->BackAmbient[0] = glIntFloatFromList();
    pglCurContext->BackAmbient[1] = glIntFloatFromList();
    pglCurContext->BackAmbient[2] = glIntFloatFromList();
    pglCurContext->LightDirty = TRUE;
    break;
  case OP_BACKDIFFUSE:
    pglCurContext->BackDiffuse[0] = glIntFloatFromList();
    pglCurContext->BackDiffuse[1] = glIntFloatFromList();
    pglCurContext->BackDiffuse[2] = glIntFloatFromList();
    pglCurContext->BackDiffuse[3] = glIntFloatFromList();
    pglCurContext->LightDirty = TRUE;
    break;
  case OP_BACKSPECULAR:
    pglCurContext->BackSpecular[0] = glIntFloatFromList();
    pglCurContext->BackSpecular[1] = glIntFloatFromList();
    pglCurContext->BackSpecular[2] = glIntFloatFromList();
    pglCurContext->LightDirty = TRUE;
    break;
  case OP_BACKEMISSION:
    pglCurContext->BackEmission[0] = glIntFloatFromList();
    pglCurContext->BackEmission[1] = glIntFloatFromList();
    pglCurContext->BackEmission[2] = glIntFloatFromList();
    pglCurContext->LightDirty = TRUE;
    break;
  case OP_COLORMATERIAL:
    pglCurContext->ColorMaterialFace = glIntIntFromList();
    pglCurContext->ColorMaterialMode = glIntIntFromList();
    pglCurContext->LightDirty = TRUE;
    break;
  case OP_INITNAMES:
    if(pglCurContext->RenderMode != GL_SELECT)
      return;
    pglCurContext->SelectDepth = -1;
    break;
  case OP_LOADNAME:
    {
      GLuint name;

      name = glIntIntFromList();
      if(pglCurContext->RenderMode != GL_SELECT)
	return;
      if(pglCurContext->SelectDepth != -1)
	pglCurContext->NameStack[pglCurContext->SelectDepth] = name;
      else
	GLINT_ERROR(GL_INVALID_OPERATION);
    }
    break;
  case OP_POPNAME:
    if(pglCurContext->RenderMode != GL_SELECT)
      return;
    if(pglCurContext->SelectDepth != -1)
      pglCurContext->SelectDepth--;
    else
      GLINT_ERROR(GL_STACK_UNDERFLOW);
    break;
  case OP_PUSHNAME:
    {
      GLuint name;

      name = glIntIntFromList();
      if(pglCurContext->RenderMode != GL_SELECT)
	return;
      if(pglCurContext->SelectDepth < 63)
	pglCurContext->NameStack[pglCurContext->SelectDepth++] = name;
      else
	GLINT_ERROR(GL_STACK_OVERFLOW);
    }
    break;
  case OP_PASSTHRU:
    glIntWriteFeedbackFloat(glIntFloatFromList());
    break;
  case OP_BIND_1D:
    glIntBindTexture(GL_TEXTURE_1D, glIntIntFromList());
    pglCurContext->TexDirty = TRUE;
    break;
  case OP_BIND_2D:
    glIntBindTexture(GL_TEXTURE_2D, glIntIntFromList());
    pglCurContext->TexDirty = TRUE;
    break;
  case OP_TEXTURE:
    {
      pglTexture TexPtr;
      int size;

      switch(glIntIntFromList()) {
      case GL_TEXTURE_1D:
	TexPtr = pglCurContext->Tex1DPtr;
	break;
      case GL_TEXTURE_2D:
	TexPtr = pglCurContext->Tex2DPtr;
	break;
      }

      TexPtr->BorderWidth = glIntIntFromList();
      TexPtr->Width = glIntIntFromList();
      TexPtr->Height = glIntIntFromList();
      TexPtr->GlideTex.smallLod = glIntIntFromList();
      TexPtr->GlideTex.largeLod = glIntIntFromList();
      TexPtr->GlideTex.aspectRatio = glIntIntFromList();
      TexPtr->SScale = glIntFloatFromList();
      TexPtr->TScale = glIntFloatFromList();
      TexPtr->GlideTex.format = glIntIntFromList();
      TexPtr->GlideSize = 0;
      size = glIntIntFromList();

      if(TexPtr->GlideTex.data != NULL)
	if(!TexPtr->TexFromList)
	  free((void *)TexPtr->GlideTex.data);
      TexPtr->TexFromList = TRUE;
      
      TexPtr->GlideTex.data = (void *)glIntIntFromList();
      
      if(pglCurContext->Tex2DPtr->Cached)
	pglCurContext->Tex2DPtr->Invalid = TRUE;
      
      pglCurContext->TexDirty = TRUE;
    }
    break;
  case OP_TEXTUREMIP:
    {
      pglTexture TexPtr;
      
      switch(glIntIntFromList()) {
      case GL_TEXTURE_1D:
	TexPtr = pglCurContext->Tex1DPtr;
	break;
      case GL_TEXTURE_2D:
	TexPtr = pglCurContext->Tex2DPtr;
	break;
      }

      TexPtr->GlideTex.smallLod = GR_LOD_1;
      
      if(pglCurContext->Tex2DPtr->Cached)
	pglCurContext->Tex2DPtr->Invalid = TRUE;
      
      pglCurContext->TexDirty = TRUE;
    }
    break;
  case OP_TEXTUREENVCOLOR:
    pglCurContext->TexEnvColor[0] = glIntFloatFromList();
    pglCurContext->TexEnvColor[1] = glIntFloatFromList();
    pglCurContext->TexEnvColor[2] = glIntFloatFromList();
    pglCurContext->TexEnvColor[3] = glIntFloatFromList();
    
    pglCurContext->TexDirty = TRUE;
    break;
  case OP_TEXTUREENVMODE:
    pglCurContext->TexMode = glIntIntFromList();
    pglCurContext->TexDirty = TRUE;
    break;
  case OP_TEXTUREPRIORITY:
    {
      pglTexture tex;
      
      switch(glIntIntFromList()) {
      case GL_TEXTURE_1D:
	tex = pglCurContext->Tex1DPtr;
	break;
      case GL_TEXTURE_2D:
	tex = pglCurContext->Tex2DPtr;
	break;
      }
      
      tex->Priority = glIntFloatFromList();
      pglCurContext->TexDirty = TRUE;
    }
    break;
  case OP_TEXTUREBORDER:
    {
      pglTexture tex;
      
      switch(glIntIntFromList()) {
      case GL_TEXTURE_1D:
	tex = pglCurContext->Tex1DPtr;
	break;
      case GL_TEXTURE_2D:
	tex = pglCurContext->Tex2DPtr;
	break;
      }
      
      tex->BorderColor[0] = glIntFloatFromList();
      tex->BorderColor[1] = glIntFloatFromList();
      tex->BorderColor[2] = glIntFloatFromList();
      tex->BorderColor[3] = glIntFloatFromList();
      
      pglCurContext->TexDirty = TRUE;
    }
    break;
  case OP_TEXTURESWRAP:
    {
      pglTexture tex;
      
      switch(glIntIntFromList()) {
      case GL_TEXTURE_1D:
	tex = pglCurContext->Tex1DPtr;
	break;
      case GL_TEXTURE_2D:
	tex = pglCurContext->Tex2DPtr;
	break;
      }
      
      tex->SWrap = glIntIntFromList();
      tex->GlideSWrap = glIntIntFromList();
      pglCurContext->TexDirty = TRUE;
    }
    break;
  case OP_TEXTURETWRAP:
    {
      pglTexture tex;
      
      switch(glIntIntFromList()) {
      case GL_TEXTURE_1D:
	tex = pglCurContext->Tex1DPtr;
	break;
      case GL_TEXTURE_2D:
	tex = pglCurContext->Tex2DPtr;
	break;
      }
      
      tex->TWrap = glIntIntFromList();
      tex->GlideTWrap = glIntIntFromList();
      pglCurContext->TexDirty = TRUE;
    }
    break;
  case OP_TEXTUREMIN:
    {
      pglTexture tex;
      
      switch(glIntIntFromList()) {
      case GL_TEXTURE_1D:
	tex = pglCurContext->Tex1DPtr;
	break;
      case GL_TEXTURE_2D:
	tex = pglCurContext->Tex2DPtr;
	break;
      }
      
      tex->Min = glIntIntFromList();
      tex->GlideMin = glIntIntFromList();
      tex->GlideMIPMode = glIntIntFromList();
      tex->GlideLodBlend = glIntIntFromList();
      pglCurContext->TexDirty = TRUE;
    }
    break;
  case OP_TEXTUREMAX:
    {
      pglTexture tex;
      
      switch(glIntIntFromList()) {
      case GL_TEXTURE_1D:
	tex = pglCurContext->Tex1DPtr;
	break;
      case GL_TEXTURE_2D:
	tex = pglCurContext->Tex2DPtr;
	break;
      }
      
      tex->Mag = glIntIntFromList();
      tex->GlideMag = glIntIntFromList();
      pglCurContext->TexDirty = TRUE;
    }
    break;
  case OP_BITMAP:
    {
      unsigned short *startptr, *sptr;
      unsigned char *bitptr, *curbitptr;
      unsigned char mask;
      unsigned short color;
      int x, y;
      int width, height;
      GLfloat xorig, yorig, xmove, ymove;
      GrLfbInfo_t info;
      GrBuffer_t buffer;

      width = glIntIntFromList();
      height = glIntIntFromList();
      xorig = glIntFloatFromList();
      yorig = glIntFloatFromList();
      xmove = glIntFloatFromList();
      ymove = glIntFloatFromList();
      bitptr = (unsigned char *)glIntIntFromList();
      
      if(!pglCurContext->RasValid)
	return;
      
      switch(pglCurContext->DrawBuffer) {
      case GL_BACK:
      case GL_BACK_LEFT:
      case GL_BACK_RIGHT:
	buffer = GR_BUFFER_BACKBUFFER;
	break;
      case GL_FRONT_AND_BACK:
	buffer = GR_BUFFER_BACKBUFFER;
	break;
      default:
	buffer = GR_BUFFER_FRONTBUFFER;
	break;
      }
      
      // Get raster color
      color = ((unsigned short)
		 (pglCurContext->RasVtx.Color[0]*31.0f*2048.0f))&0xf800;
      color |= ((unsigned short)
		(pglCurContext->RasVtx.Color[1]*63.0f*32.0f))&0x07e0;
      color |= ((unsigned short)
		(pglCurContext->RasVtx.Color[2]*31.0f))&0x001f;
      
      if(pglCurContext->DrawBuffer == GL_FRONT_AND_BACK) {


	grRenderBuffer(GR_BUFFER_FRONTBUFFER);
	if ( !grLfbLock( GR_LFB_WRITE_ONLY, GR_BUFFER_FRONTBUFFER,
			GR_LFBWRITEMODE_565, GR_ORIGIN_LOWER_LEFT, FXTRUE,
			&info ) ) return;
	startptr = (unsigned short *)info.lfbPtr;
	startptr += ((int)(-xorig+(pglCurContext->RasVtx.Glide.x-SNAP_BIAS))) +
	  (((int)(-yorig+(pglCurContext->RasVtx.Glide.y-SNAP_BIAS)))
	   *(info.strideInBytes/2));
	
	curbitptr = bitptr;
	mask = 0x80;
	for(y=0;y<height;y++) {
	  sptr = startptr;
	  for(x=0;x<width;x++) {
	    if(*curbitptr&mask)
	      *sptr++ = color;
	    else
	      sptr++;
	    
	    if(mask == 0x01) {
	      mask = 0x80;
	      curbitptr++;
	    } else {
	      mask = mask>>1;
	    }
	  }
	  startptr += info.strideInBytes/2;
	}
	
	grLfbUnlock( GR_LFB_WRITE_ONLY, GR_BUFFER_FRONTBUFFER );
	grRenderBuffer(GR_BUFFER_BACKBUFFER);
      }

      if ( !grLfbLock( GR_LFB_WRITE_ONLY, buffer,
		      GR_LFBWRITEMODE_565, GR_ORIGIN_LOWER_LEFT, FXTRUE,
		      &info ) ) return;
      startptr = (unsigned short *)info.lfbPtr;
      startptr += ((int)(-xorig+(pglCurContext->RasVtx.Glide.x-SNAP_BIAS))) +
	(((int)(-yorig+(pglCurContext->RasVtx.Glide.y-SNAP_BIAS)))
	 *(info.strideInBytes/2));

      curbitptr = bitptr;
      mask = 0x80;
      for(y=0;y<height;y++) {
	sptr = startptr;
	for(x=0;x<width;x++) {
	  if(*curbitptr&mask)
	    *sptr++ = color;
	  else
	    sptr++;

	  if(mask == 0x01) {
	    mask = 0x80;
	    curbitptr++;
	  } else {
	    mask = mask>>1;
	  }
	}
	startptr += info.strideInBytes/2;
      }

      grLfbUnlock( GR_LFB_WRITE_ONLY, buffer );

      pglCurContext->RasVtx.Glide.x += xmove;
      pglCurContext->RasVtx.Glide.y += ymove;

    }
    break;
  case OP_DRAWPIXEL:
    {
      unsigned short *data_ptr, *fb_ptr, *start_ptr;
      GLenum format;
      int width, height;
      int x,y;
      GrLfbInfo_t info;
      GrBuffer_t buffer;

      format = glIntIntFromList();
      width = glIntIntFromList();
      height = glIntIntFromList();
      data_ptr = (unsigned short *)glIntIntFromList();
      
      if(format == GL_DEPTH_COMPONENT) {
	if ( !grLfbLock( GR_LFB_WRITE_ONLY, GR_BUFFER_DEPTHBUFFER,
			GR_TEXFMT_DEPTH, GR_ORIGIN_LOWER_LEFT, FXTRUE,
			&info ) ) return;
	start_ptr = (unsigned short *)info.lfbPtr;
      } else {
	
	switch(pglCurContext->DrawBuffer) {
	case GL_BACK:
	case GL_BACK_LEFT:
	case GL_BACK_RIGHT:
	  buffer = GR_BUFFER_BACKBUFFER;
	  break;
	default:
	  buffer = GR_BUFFER_FRONTBUFFER;
	  break;
	}
	if ( !grLfbLock( GR_LFB_WRITE_ONLY, buffer,
			GR_LFBWRITEMODE_565, GR_ORIGIN_LOWER_LEFT, FXTRUE,
			&info ) ) return;
	start_ptr = (unsigned short *)info.lfbPtr;
      }
      
      start_ptr += (int)(pglCurContext->RasVtx.Glide.x) +
	(int)(pglCurContext->RasVtx.Glide.y*(info.strideInBytes/2));
	  
      for(y=0;y<height;y++) {
	fb_ptr = start_ptr;
	for(x=0;x<width;x++) {
	  *fb_ptr++ = *data_ptr++;
	}
	start_ptr += (info.strideInBytes/2);
      }
  
      if(format == GL_DEPTH_COMPONENT) {
	grLfbUnlock( GR_LFB_WRITE_ONLY, GR_BUFFER_DEPTHBUFFER );
      } else {
	grLfbUnlock( GR_LFB_WRITE_ONLY, buffer);
      }
    }
    break;
  case OP_PIXELMAP:
    {
      int mapindex, mapsize;
      int i;

      mapindex = glIntIntFromList();
      mapsize = glIntIntFromList();
    
      for(i=0;i<mapsize;i++) {
	pglCurContext->PixelMaps[mapindex][i] = 
	  glIntFloatFromList();
      }
    }
    break;
  case OP_PACKSWAPBYTES:
    pglCurContext->PackSwapBytes = glIntIntFromList();
    break;
  case OP_PACKLSBFIRST:
    pglCurContext->PackLsbFirst = glIntIntFromList();
    break;
  case OP_PACKROWLENGTH:
    pglCurContext->PackRowLength = glIntIntFromList();
    break;
  case OP_PACKSKIPPIXELS:
    pglCurContext->PackSkipPixels = glIntIntFromList();
    break;
  case OP_PACKSKIPROWS:
    pglCurContext->PackSkipRows = glIntIntFromList();
    break;
  case OP_PACKALIGNMENT:
    pglCurContext->PackAlignment = glIntIntFromList();
    break;
  case OP_UNPACKSWAPBYTES:
    pglCurContext->UnPackSwapBytes = glIntIntFromList();
    break;
  case OP_UNPACKLSBFIRST:
    pglCurContext->UnPackLsbFirst = glIntIntFromList();
    break;
  case OP_UNPACKROWLENGTH:
    pglCurContext->UnPackRowLength = glIntIntFromList();
    break;
  case OP_UNPACKSKIPPIXELS:
    pglCurContext->UnPackSkipPixels = glIntIntFromList();
    break;
  case OP_UNPACKSKIPROWS:
    pglCurContext->UnPackSkipRows = glIntIntFromList();
    break;
  case OP_UNPACKALIGNMENT:
    pglCurContext->UnPackAlignment = glIntIntFromList();
    break;
  case OP_MAPCOLOR:
    pglCurContext->PixelMapColor = glIntIntFromList();
    break;
  case OP_MAPSTENCIL:
    pglCurContext->PixelMapStencil = glIntIntFromList();
    break;
  case OP_INDEXSHIFT:
    pglCurContext->PixelIndexShift = glIntIntFromList();
    break;
  case OP_INDEXOFFSET:
    pglCurContext->PixelIndexOffset = glIntIntFromList();
    break;
  case OP_REDSCALE:
    pglCurContext->PixelRedScale = glIntFloatFromList();
    break;
  case OP_REDBIAS:
    pglCurContext->PixelRedBias = glIntFloatFromList();
    break;
  case OP_GREENSCALE:
    pglCurContext->PixelGreenScale = glIntFloatFromList();
    break;
  case OP_GREENBIAS:
    pglCurContext->PixelGreenBias = glIntFloatFromList();
    break;
  case OP_BLUESCALE:
    pglCurContext->PixelBlueScale = glIntFloatFromList();
    break;
  case OP_BLUEBIAS:
    pglCurContext->PixelBlueBias = glIntFloatFromList();
    break;
  case OP_ALPHASCALE:
    pglCurContext->PixelAlphaScale = glIntFloatFromList();
    break;
  case OP_ALPHABIAS:
    pglCurContext->PixelAlphaBias = glIntFloatFromList();
    break;
  case OP_DEPTHSCALE:
    pglCurContext->PixelDepthScale = glIntFloatFromList();
    break;
  case OP_DEPTHBIAS:
    pglCurContext->PixelDepthBias = glIntFloatFromList();
    break;
  case OP_PUSHATTRIB:
    {
      GLbitfield mask;
      
      mask = (GLbitfield)glIntIntFromList();
      
      if(pglCurContext->AttribDepth < 16) {
	glIntCopyContextToAttrib(
          &pglCurContext->AttribStack[pglCurContext->AttribDepth],
          mask);
	pglCurContext->AttribDepth++;
      } else {
	GLINT_ERROR(GL_STACK_OVERFLOW);
      }
    }
    break;
  case OP_POPATTRIB:
    if(pglCurContext->AttribDepth > 0) {
      pglCurContext->AttribDepth--;
      glIntCopyAttribToContext(
	&pglCurContext->AttribStack[pglCurContext->AttribDepth]);
    } else {
      GLINT_ERROR(GL_STACK_UNDERFLOW);
    }
    pglCurContext->TexDirty = TRUE;
    pglCurContext->ContextDirty = TRUE;
    break;
  case OP_ACCUM:
    break;
  case OP_ACCUMLOAD:
    break;
  case OP_ACCUMMULT:
    break;
  case OP_ACCUMADD:
    break;
  case OP_ACCUMRETURN:
    break;
  case OP_LISTBASE:
    pglCurContext->ListBase = glIntIntFromList();
    break;
  case OP_CALLLIST:
    {
      pglListBlock curlist;
      GLuint list;

      // find list
      list = glIntIntFromList();
      curlist = pglCurContext->ListHead;
      while(curlist != NULL) {
	if(curlist->ListName == list)
	  break;
	curlist = curlist->Next;
      }
      
      if(curlist != NULL) {
	
	if(pglCurContext->ExecLevel == 63) {
	  GLINT_ERROR(GL_STACK_OVERFLOW);
	} else {
	  pglCurContext->ExecLevel++;

	  // Setup *FromList for sub list
	  pglCurContext->ExecCurBlock[pglCurContext->ExecLevel] =
	    curlist->ListBlocks;
	  pglCurContext->ExecCur[pglCurContext->ExecLevel] = 
	    pglCurContext->ExecCurBlock[pglCurContext->ExecLevel] + 1;
	  pglCurContext->ExecLeft[pglCurContext->ExecLevel] = 
	    GLINT_LIST_SIZE - 1;
	}
      }
    }
    break;
  case OP_CALLLIST_OFFSET:
    {
      pglListBlock curlist;
      GLuint list;

      // find list
      list = glIntIntFromList()+pglCurContext->ListBase;
      curlist = pglCurContext->ListHead;
      while(curlist != NULL) {
	if(curlist->ListName == list)
	  break;
	curlist = curlist->Next;
      }
      
      if(curlist != NULL) {
	
	if(pglCurContext->ExecLevel == 63) {
	  GLINT_ERROR(GL_STACK_OVERFLOW);
	} else {
	  pglCurContext->ExecLevel++;

	  // Setup *FromList for sub list
	  pglCurContext->ExecCurBlock[pglCurContext->ExecLevel] =
	    curlist->ListBlocks;
	  pglCurContext->ExecCur[pglCurContext->ExecLevel] = 
	    pglCurContext->ExecCurBlock[pglCurContext->ExecLevel] + 1;
	  pglCurContext->ExecLeft[pglCurContext->ExecLevel] = 
	    GLINT_LIST_SIZE - 1;
	}
      }
    }
    break;
  case OP_NEXT_BLOCK:
    // Exec a new block
    pglCurContext->ExecCurBlock[pglCurContext->ExecLevel] = 
      (unsigned int *)*(pglCurContext->ExecCurBlock[pglCurContext->ExecLevel]);
    pglCurContext->ExecCur[pglCurContext->ExecLevel] =
      pglCurContext->ExecCurBlock[pglCurContext->ExecLevel] + 1;
    pglCurContext->ExecLeft[pglCurContext->ExecLevel] = GLINT_LIST_SIZE - 1;
    break;
  case OP_END_LIST:
    // Setup *FromList to continue previous list 
    pglCurContext->ExecLevel--;
    break;
  }
}
