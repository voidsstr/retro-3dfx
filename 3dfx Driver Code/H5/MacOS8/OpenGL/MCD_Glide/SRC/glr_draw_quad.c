/*________________________________________________________________________________________
** 
** Copyright (c) 1999, 3Dfx Interactive, Inc.
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
**________________________________________________________________________________________
**
**  Description: 
**
** 
**
*/


#include "glr.h"
#include "glr_drawing.h"
#include "glr_glide.h"
#include "glr_cmdfifo.h"

/*
________________________________________________________________________________________

      glrRenderFlatQuads
________________________________________________________________________________________

*/

void
glrRenderFlatQuads(
	GLDContext				inContext,
	const GLDVertex *		inVtx,
	GLint					inNumberOfVtx,
	GLbitfield				inMode)
{
  GLfloat					theZoffset;
  FxU32 vSize, pargb;
  GLint i, n;
      
  DEBUG_FASTPATH_ENTRY(glrRenderFlatQuads);
      
  theZoffset = inContext->fill_z_offset;
			
  if( inNumberOfVtx < 4 ) return;
	
  PARAM_SETUP(SST_SETUP_RGB|SST_SETUP_A|SST_SETUP_Z|SST_SETUP_Wfbi,5);
	
  n = inNumberOfVtx - 3;
  
  for(i = 0; i < n; i += 4)
  {
    SET_EXPECTED_SIZE(vSize * 4 + sizeof(FxU32));
    TRI_STRIP_BEGIN(kSetupFan, 4, vSize, SSTCP_PKT3_BDDDDD);

    pargb = packARGB(inVtx->color.a,inVtx->color.r,inVtx->color.g,inVtx->color.b);
    TRI_SETF_FAST(inVtx->window.x);
    TRI_SETF_FAST(inVtx->window.y);
    TRI_SET(pargb);
    TRI_SETF_FAST(inVtx->window.z);
    TRI_SETF_FAST(inVtx->fog);
    inVtx++;
    TRI_SETF_FAST(inVtx->window.x);
    TRI_SETF_FAST(inVtx->window.y);
    TRI_SET(pargb);
    TRI_SETF_FAST(inVtx->window.z);
    TRI_SETF_FAST(inVtx->fog);
    inVtx++;
    TRI_SETF_FAST(inVtx->window.x);
    TRI_SETF_FAST(inVtx->window.y);
    TRI_SET(pargb);
    TRI_SETF_FAST(inVtx->window.z);
    TRI_SETF_FAST(inVtx->fog);
    inVtx++;
    TRI_SETF_FAST(inVtx->window.x);
    TRI_SETF_FAST(inVtx->window.y);
    TRI_SET(pargb);
    TRI_SETF_FAST(inVtx->window.z);
    TRI_SETF_FAST(inVtx->fog);
    inVtx++;

    TRI_END;  
    CHECK_SIZE;
  }  
  
  PARAM_END;
}

/*
________________________________________________________________________________________

      glrRenderSmoothQuads
________________________________________________________________________________________

*/

void
glrRenderSmoothQuads(
	GLDContext				inContext,
	const GLDVertex *		inVtx,
	GLint					inNumberOfVtx,
	GLbitfield				inMode)
{
  GLfloat					theZoffset;
  FxU32 vSize;
  GLint i, n;
      
  DEBUG_FASTPATH_ENTRY(glrRenderSmoothQuads);
      
  theZoffset = inContext->fill_z_offset;
			
  if( inNumberOfVtx < 4 ) return;
	
  PARAM_SETUP(SST_SETUP_RGB|SST_SETUP_A|SST_SETUP_Z|SST_SETUP_Wfbi,5);
	
  n = inNumberOfVtx - 3;
  
  for(i = 0; i < n; i += 4)
  {
    SET_EXPECTED_SIZE(vSize * 4 + sizeof(FxU32));
    TRI_STRIP_BEGIN(kSetupFan, 4, vSize, SSTCP_PKT3_BDDDDD);

    TRI_SETF_FAST(inVtx->window.x);
    TRI_SETF_FAST(inVtx->window.y);
    TRI_SET(packARGB(inVtx->color.a,inVtx->color.r,inVtx->color.g,inVtx->color.b));
    TRI_SETF_FAST(inVtx->window.z);
    TRI_SETF_FAST(inVtx->fog);
    inVtx++;
    TRI_SETF_FAST(inVtx->window.x);
    TRI_SETF_FAST(inVtx->window.y);
    TRI_SET(packARGB(inVtx->color.a,inVtx->color.r,inVtx->color.g,inVtx->color.b));
    TRI_SETF_FAST(inVtx->window.z);
    TRI_SETF_FAST(inVtx->fog);
    inVtx++;
    TRI_SETF_FAST(inVtx->window.x);
    TRI_SETF_FAST(inVtx->window.y);
    TRI_SET(packARGB(inVtx->color.a,inVtx->color.r,inVtx->color.g,inVtx->color.b));
    TRI_SETF_FAST(inVtx->window.z);
    TRI_SETF_FAST(inVtx->fog);
    inVtx++;
    TRI_SETF_FAST(inVtx->window.x);
    TRI_SETF_FAST(inVtx->window.y);
    TRI_SET(packARGB(inVtx->color.a,inVtx->color.r,inVtx->color.g,inVtx->color.b));
    TRI_SETF_FAST(inVtx->window.z);
    TRI_SETF_FAST(inVtx->fog);
    inVtx++;

    TRI_END;  
    CHECK_SIZE;
  }  
  
  PARAM_END;
}



/*
________________________________________________________________________________________

      glrRenderFlatTextureQuads
________________________________________________________________________________________

*/

void
glrRenderFlatTextureQuads(
	GLDContext				inContext,
	const GLDVertex *		inVtx,
	GLint					inNumberOfVtx,
	GLbitfield				inMode)
{
  GLfloat					theZoffset;
  FxU32 vSize, pargb;
  FxFloat s_scale, t_scale;   
  GLint i, n;
   
  DEBUG_FASTPATH_ENTRY(glrRenderFlatTextureQuads);
      
  theZoffset = inContext->fill_z_offset;
			
  if( inNumberOfVtx < 4 ) return;


  PARAM_SETUP(SST_SETUP_RGB|SST_SETUP_A|SST_SETUP_Z|SST_SETUP_Wfbi|SST_SETUP_W0|SST_SETUP_ST0,8);
  
  s_scale = inContext->hw_texture[0]->s_scale;
  t_scale = inContext->hw_texture[0]->t_scale;
  n = inNumberOfVtx - 3;
  
  for(i = 0; i < n; i += 4)
  {
    SET_EXPECTED_SIZE(vSize * 4 + sizeof(FxU32));
    TRI_STRIP_BEGIN(kSetupFan, 4, vSize, SSTCP_PKT3_BDDDDD);

    TRI_SETF_FAST(inVtx->window.x);
    TRI_SETF_FAST(inVtx->window.y);
    pargb = packARGB(inVtx->color.a,inVtx->color.r,inVtx->color.g,inVtx->color.b);
    TRI_SET(pargb);
    TRI_SETF_FAST(inVtx->window.z);
    TRI_SETF_FAST(inVtx->fog);
    TRI_SETF_FAST(inVtx->texture[0].q);
    TRI_SETF(inVtx->texture[0].s * s_scale);
    TRI_SETF(inVtx->texture[0].t * t_scale);
    inVtx++;
    TRI_SETF_FAST(inVtx->window.x);
    TRI_SETF_FAST(inVtx->window.y);
    TRI_SET(pargb);
    TRI_SETF_FAST(inVtx->window.z);
    TRI_SETF_FAST(inVtx->fog);
    TRI_SETF_FAST(inVtx->texture[0].q);
    TRI_SETF(inVtx->texture[0].s * s_scale);
    TRI_SETF(inVtx->texture[0].t * t_scale);
    inVtx++;
    TRI_SETF_FAST(inVtx->window.x);
    TRI_SETF_FAST(inVtx->window.y);
    TRI_SET(pargb);
    TRI_SETF_FAST(inVtx->window.z);
    TRI_SETF_FAST(inVtx->fog);
    TRI_SETF_FAST(inVtx->texture[0].q);
    TRI_SETF(inVtx->texture[0].s * s_scale);
    TRI_SETF(inVtx->texture[0].t * t_scale);
    inVtx++;
    TRI_SETF_FAST(inVtx->window.x);
    TRI_SETF_FAST(inVtx->window.y);
    TRI_SET(pargb);
    TRI_SETF_FAST(inVtx->window.z);
    TRI_SETF_FAST(inVtx->fog);
    TRI_SETF_FAST(inVtx->texture[0].q);
    TRI_SETF(inVtx->texture[0].s * s_scale);
    TRI_SETF(inVtx->texture[0].t * t_scale);
    inVtx++;

    TRI_END;  
    CHECK_SIZE;
  }  
  
  PARAM_END;
}

void
glrRenderFlatMultiTextureQuads(
	GLDContext				inContext,
	const GLDVertex *		inVtx,
	GLint					inNumberOfVtx,
	GLbitfield				inMode)
{
  GLfloat					theZoffset;
  FxU32 vSize, pargb;
  FxFloat s_scale0, t_scale0;   
  FxFloat s_scale1, t_scale1;   
  GLint i, n;
   
  DEBUG_FASTPATH_ENTRY(glrRenderFlatMultiTextureQuads);
      
  theZoffset = inContext->fill_z_offset;
			
  if( inNumberOfVtx < 4 ) return;


  PARAM_SETUP(SST_SETUP_RGB|SST_SETUP_A|SST_SETUP_Z|SST_SETUP_Wfbi|SST_SETUP_W0|SST_SETUP_ST0|SST_SETUP_W1|SST_SETUP_ST1,11);
  
  s_scale0 = inContext->hw_texture[0]->s_scale;
  t_scale0 = inContext->hw_texture[0]->t_scale;
  s_scale1 = inContext->hw_texture[1]->s_scale;
  t_scale1 = inContext->hw_texture[1]->t_scale;
  
  n = inNumberOfVtx - 3;
  
  for(i = 0; i < n; i += 4)
  {
    SET_EXPECTED_SIZE(vSize * 4 + sizeof(FxU32));
    TRI_STRIP_BEGIN(kSetupFan, 4, vSize, SSTCP_PKT3_BDDDDD);

    TRI_SETF_FAST(inVtx->window.x);
    TRI_SETF_FAST(inVtx->window.y);
    pargb = packARGB(inVtx->color.a,inVtx->color.r,inVtx->color.g,inVtx->color.b);
    TRI_SET(pargb);
    TRI_SETF_FAST(inVtx->window.z);
    TRI_SETF_FAST(inVtx->fog);
    TRI_SETF_FAST(inVtx->texture[1].q);
    TRI_SETF(inVtx->texture[1].s * s_scale0);
    TRI_SETF(inVtx->texture[1].t * t_scale0);
    TRI_SETF_FAST(inVtx->texture[0].q);
    TRI_SETF(inVtx->texture[0].s * s_scale1);
    TRI_SETF(inVtx->texture[0].t * t_scale1);
    inVtx++;
    TRI_SETF_FAST(inVtx->window.x);
    TRI_SETF_FAST(inVtx->window.y);
    TRI_SET(pargb);
    TRI_SETF_FAST(inVtx->window.z);
    TRI_SETF_FAST(inVtx->fog);
    TRI_SETF_FAST(inVtx->texture[1].q);
    TRI_SETF(inVtx->texture[1].s * s_scale0);
    TRI_SETF(inVtx->texture[1].t * t_scale0);
    TRI_SETF_FAST(inVtx->texture[0].q);
    TRI_SETF(inVtx->texture[0].s * s_scale1);
    TRI_SETF(inVtx->texture[0].t * t_scale1);
    inVtx++;
    TRI_SETF_FAST(inVtx->window.x);
    TRI_SETF_FAST(inVtx->window.y);
    TRI_SET(pargb);
    TRI_SETF_FAST(inVtx->window.z);
    TRI_SETF_FAST(inVtx->fog);
    TRI_SETF_FAST(inVtx->texture[1].q);
    TRI_SETF(inVtx->texture[1].s * s_scale0);
    TRI_SETF(inVtx->texture[1].t * t_scale0);
    TRI_SETF_FAST(inVtx->texture[0].q);
    TRI_SETF(inVtx->texture[0].s * s_scale1);
    TRI_SETF(inVtx->texture[0].t * t_scale1);
    inVtx++;
    TRI_SETF_FAST(inVtx->window.x);
    TRI_SETF_FAST(inVtx->window.y);
    TRI_SET(pargb);
    TRI_SETF_FAST(inVtx->window.z);
    TRI_SETF_FAST(inVtx->fog);
    TRI_SETF_FAST(inVtx->texture[1].q);
    TRI_SETF(inVtx->texture[1].s * s_scale0);
    TRI_SETF(inVtx->texture[1].t * t_scale0);
    TRI_SETF_FAST(inVtx->texture[0].q);
    TRI_SETF(inVtx->texture[0].s * s_scale1);
    TRI_SETF(inVtx->texture[0].t * t_scale1);
    inVtx++;

    TRI_END;  
    CHECK_SIZE;
  }  
  
  PARAM_END;
}


/*
________________________________________________________________________________________

      glrRenderSmoothTextureQuads
________________________________________________________________________________________

*/

void
glrRenderSmoothTextureQuads(
	GLDContext				inContext,
	const GLDVertex *		inVtx,
	GLint					inNumberOfVtx,
	GLbitfield				inMode)
{
  GLfloat					theZoffset;
  FxU32 vSize;
  FxFloat s_scale, t_scale, s0, t0;   
  GLint i, n;
  
  DEBUG_FASTPATH_ENTRY(glrRenderSmoothTextureQuads);
      
  theZoffset = inContext->fill_z_offset;
			
  if( inNumberOfVtx < 4 ) return;

  PARAM_SETUP(SST_SETUP_RGB|SST_SETUP_A|SST_SETUP_Z|SST_SETUP_Wfbi|SST_SETUP_W0|SST_SETUP_ST0,8);
	
  s_scale = inContext->hw_texture[0]->s_scale;
  t_scale = inContext->hw_texture[0]->t_scale;
  n = inNumberOfVtx - 3;
  
  ALIGN_FIFO;
  
  for(i = 0; i < n; i += 4)
  {
    if(CULLING_DISABLED || cullTri(&inVtx->window.x,&(inVtx+1)->window.x,&(inVtx+2)->window.x,signMode)) {
      SET_EXPECTED_SIZE(vSize * 4 + 2 * sizeof(FxU32));
      
	  TRI_STRIP_BEGIN_NOHEADER(kSetupFan, 4, vSize);      
      TRI_SETPAIR_II(0,packetVal | SSTCP_PKT3_BDDDDD);
      
      TRI_SETPAIR_FF(inVtx->window.x, inVtx->window.y);      
      TRI_SETPAIR_IF(packARGB(inVtx->color.a,inVtx->color.r,inVtx->color.g,inVtx->color.b),inVtx->window.z);      
      TRI_SETPAIR_FF(inVtx->fog,inVtx->texture[0].q);      
      s0 = inVtx->texture[0].s * s_scale;
      t0 = inVtx->texture[0].t * t_scale;      
      TRI_SETPAIR_FF(s0,t0);
      inVtx++;
      
      TRI_SETPAIR_FF(inVtx->window.x, inVtx->window.y);      
      TRI_SETPAIR_IF(packARGB(inVtx->color.a,inVtx->color.r,inVtx->color.g,inVtx->color.b),inVtx->window.z);      
      TRI_SETPAIR_FF(inVtx->fog,inVtx->texture[0].q);      
      s0 = inVtx->texture[0].s * s_scale;
      t0 = inVtx->texture[0].t * t_scale;      
      TRI_SETPAIR_FF(s0,t0);
      inVtx++;
      
      TRI_SETPAIR_FF(inVtx->window.x, inVtx->window.y);      
      TRI_SETPAIR_IF(packARGB(inVtx->color.a,inVtx->color.r,inVtx->color.g,inVtx->color.b),inVtx->window.z);      
      TRI_SETPAIR_FF(inVtx->fog,inVtx->texture[0].q);      
      s0 = inVtx->texture[0].s * s_scale;
      t0 = inVtx->texture[0].t * t_scale;      
      TRI_SETPAIR_FF(s0,t0);
      inVtx++;
      
      TRI_SETPAIR_FF(inVtx->window.x, inVtx->window.y);      
      TRI_SETPAIR_IF(packARGB(inVtx->color.a,inVtx->color.r,inVtx->color.g,inVtx->color.b),inVtx->window.z);      
      TRI_SETPAIR_FF(inVtx->fog,inVtx->texture[0].q);      
      s0 = inVtx->texture[0].s * s_scale;
      t0 = inVtx->texture[0].t * t_scale;      
      TRI_SETPAIR_FF(s0,t0);
      inVtx++;

      TRI_END;  
      CHECK_SIZE;
    }
  }  
  
  PARAM_END;
}

void
glrRenderSmoothMultiTextureQuads(
	GLDContext				inContext,
	const GLDVertex *		inVtx,
	GLint					inNumberOfVtx,
	GLbitfield				inMode)
{
  GLfloat					theZoffset;
  FxU32 vSize;
  FxFloat s_scale0, t_scale0;   
  FxFloat s_scale1, t_scale1;   
  GLint i, n;
  
  DEBUG_FASTPATH_ENTRY(glrRenderSmoothMultiTextureQuads);
      
  theZoffset = inContext->fill_z_offset;
			
  if( inNumberOfVtx < 4 ) return;
	
  PARAM_SETUP(SST_SETUP_RGB|SST_SETUP_A|SST_SETUP_Z|SST_SETUP_Wfbi|SST_SETUP_W0|SST_SETUP_ST0|SST_SETUP_W1|SST_SETUP_ST1,11);
	
  s_scale0 = inContext->hw_texture[0]->s_scale;
  t_scale0 = inContext->hw_texture[0]->t_scale;
  s_scale1 = inContext->hw_texture[1]->s_scale;
  t_scale1 = inContext->hw_texture[1]->t_scale;
  n = inNumberOfVtx - 3;
  
  for(i = 0; i < n; i += 4)
  {
    SET_EXPECTED_SIZE(vSize * 4 + sizeof(FxU32));
    TRI_STRIP_BEGIN(kSetupFan, 4, vSize, SSTCP_PKT3_BDDDDD);

    TRI_SETF_FAST(inVtx->window.x);
    TRI_SETF_FAST(inVtx->window.y);
    TRI_SET(packARGB(inVtx->color.a,inVtx->color.r,inVtx->color.g,inVtx->color.b));
    TRI_SETF_FAST(inVtx->window.z);
    TRI_SETF_FAST(inVtx->fog);
    TRI_SETF_FAST(inVtx->texture[1].q);
    TRI_SETF(inVtx->texture[1].s * s_scale0);
    TRI_SETF(inVtx->texture[1].t * t_scale0);
    TRI_SETF_FAST(inVtx->texture[0].q);
    TRI_SETF(inVtx->texture[0].s * s_scale1);
    TRI_SETF(inVtx->texture[0].t * t_scale1);
    inVtx++;
    TRI_SETF_FAST(inVtx->window.x);
    TRI_SETF_FAST(inVtx->window.y);
    TRI_SET(packARGB(inVtx->color.a,inVtx->color.r,inVtx->color.g,inVtx->color.b));
    TRI_SETF_FAST(inVtx->window.z);
    TRI_SETF_FAST(inVtx->texture[1].q);
    TRI_SETF(inVtx->texture[1].s * s_scale0);
    TRI_SETF(inVtx->texture[1].t * t_scale0);
    TRI_SETF_FAST(inVtx->texture[0].q);
    TRI_SETF(inVtx->texture[0].s * s_scale1);
    TRI_SETF(inVtx->texture[0].t * t_scale1);
    inVtx++;
    TRI_SETF_FAST(inVtx->window.x);
    TRI_SETF_FAST(inVtx->window.y);
    TRI_SET(packARGB(inVtx->color.a,inVtx->color.r,inVtx->color.g,inVtx->color.b));
    TRI_SETF_FAST(inVtx->window.z);
    TRI_SETF_FAST(inVtx->texture[1].q);
    TRI_SETF(inVtx->texture[1].s * s_scale0);
    TRI_SETF(inVtx->texture[1].t * t_scale0);
    TRI_SETF_FAST(inVtx->texture[0].q);
    TRI_SETF(inVtx->texture[0].s * s_scale1);
    TRI_SETF(inVtx->texture[0].t * t_scale1);
    inVtx++;
    TRI_SETF_FAST(inVtx->window.x);
    TRI_SETF_FAST(inVtx->window.y);
    TRI_SET(packARGB(inVtx->color.a,inVtx->color.r,inVtx->color.g,inVtx->color.b));
    TRI_SETF_FAST(inVtx->window.z);
    TRI_SETF_FAST(inVtx->texture[1].q);
    TRI_SETF(inVtx->texture[1].s * s_scale0);
    TRI_SETF(inVtx->texture[1].t * t_scale0);
    TRI_SETF_FAST(inVtx->texture[0].q);
    TRI_SETF(inVtx->texture[0].s * s_scale1);
    TRI_SETF(inVtx->texture[0].t * t_scale1);
    inVtx++;

    TRI_END;  
    CHECK_SIZE;
  }  
  
  PARAM_END;
}
