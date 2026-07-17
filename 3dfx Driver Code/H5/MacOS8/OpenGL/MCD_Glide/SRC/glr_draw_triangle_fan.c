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

      glrRenderFlatTriangleFan
________________________________________________________________________________________

*/

void
glrRenderFlatTriangleFan(
	GLDContext				inContext,
	const GLDVertex *		inPivotVertex,
	const GLDVertex *		inVtx,
	GLint					inNumberOfVtx,
	GLenum					/*inMode*/)
{
	GLint					i;
	GrVertex				v0, v1, v2;
	
	DEBUG_SLOWPATH_ENTRY( glrRenderFlatTriangleFan );

	if( inNumberOfVtx < 2 ) return;
	
	glrLoadVertex( inContext, *inPivotVertex, v0);
	glrLoadVertex( inContext, inVtx[0], v1);
		
	i = 1;
	while( i < inNumberOfVtx ) {
	
		glrLoadVertex_Smooth( inContext, inVtx[i], v2);

        v0.pargb = v2.pargb;
        v1.pargb = v2.pargb;
        		
		grDrawTriangle(&v0, &v1, &v2);
		
		v1 = v2;
		i++;
	}
}


/*
________________________________________________________________________________________

      glrRenderSmoothTriangleFan
________________________________________________________________________________________

*/

void
glrRenderSmoothTriangleFan(
	GLDContext				inContext,
	const GLDVertex *		inPivotVertex,
	const GLDVertex *		inVtx,
	GLint					inNumberOfVtx,
	GLbitfield				inMode)
{
    FxU32 pktype, vSize;
    FxFloat s_scale, t_scale;   
    
    DEBUG_FASTPATH_ENTRY(glrRenderSmoothTriangleFan);
    
	if( inNumberOfVtx < 2 ) return;

	PARAM_SETUP(SST_SETUP_RGB|SST_SETUP_A|SST_SETUP_Z|SST_SETUP_Wfbi,5);
	
    pktype = SSTCP_PKT3_BDDDDD;
    s_scale = inContext->hw_texture[0]->s_scale;
    t_scale = inContext->hw_texture[0]->t_scale;
    
    if(inNumberOfVtx > 0) {
      FxI32 k, vcount = (inNumberOfVtx >= 14 ? 14 : inNumberOfVtx);
	  
	  SET_EXPECTED_SIZE(vSize * (vcount + 1) + sizeof(FxU32));
	  TRI_STRIP_BEGIN(kSetupFan, (vcount + 1), vSize, pktype);

	  TRI_SETF_FAST(inPivotVertex->window.x);
	  TRI_SETF_FAST(inPivotVertex->window.y);
	  TRI_SET(packARGB(inPivotVertex->color.a,inPivotVertex->color.r,inPivotVertex->color.g,inPivotVertex->color.b));
	  TRI_SETF_FAST(inPivotVertex->window.z);
	  TRI_SETF_FAST(inPivotVertex->fog);
	  
	  for(k = 0; k < vcount; k++) {
		  TRI_SETF_FAST(inVtx->window.x);
		  TRI_SETF_FAST(inVtx->window.y);
		  TRI_SET(packARGB(inVtx->color.a,inVtx->color.r,inVtx->color.g,inVtx->color.b));
		  TRI_SETF_FAST(inVtx->window.z);
		  TRI_SETF_FAST(inVtx->fog);
		  inVtx++;
      }	  
	  TRI_END;
	  CHECK_SIZE;
	  
	  inNumberOfVtx -= 14;
	  pktype = SSTCP_PKT3_DDDDDD;
	}	

    while(inNumberOfVtx > 0) {
      FxI32 k, vcount = inNumberOfVtx >= 15 ? 15 : inNumberOfVtx;
	  
	  SET_EXPECTED_SIZE(vSize * vcount + sizeof(FxU32));
	  TRI_STRIP_BEGIN(kSetupFan, vcount, vSize, pktype);
	  
	  for(k = 0; k < vcount; k++) {
		  TRI_SETF_FAST(inVtx->window.x);
		  TRI_SETF_FAST(inVtx->window.y);
		  TRI_SET(packARGB(inVtx->color.a,inVtx->color.r,inVtx->color.g,inVtx->color.b));
		  TRI_SETF_FAST(inVtx->window.z);
		  TRI_SETF_FAST(inVtx->fog);
		  inVtx++;
      }	  
	  TRI_END;
	  CHECK_SIZE;
	  
	  inNumberOfVtx -= 15;
	  pktype = SSTCP_PKT3_DDDDDD;
	}	
	PARAM_END;
}

/*
________________________________________________________________________________________

      glrRenderSmoothTriangleFan
________________________________________________________________________________________

*/

void
glrRenderFlatTextureTriangleFan(
	GLDContext				inContext,
	const GLDVertex *		inPivotVertex,
	const GLDVertex *		inVtx,
	GLint					inNumberOfVtx,
	GLbitfield				inMode)
{
    FxU32 pktype, vSize, pargb;
    FxFloat s_scale, t_scale;   
    
    DEBUG_FASTPATH_ENTRY(glrRenderFlatTextureTriangleFan);
    
	if( inNumberOfVtx < 2 ) return;
	
	PARAM_SETUP(SST_SETUP_RGB|SST_SETUP_A|SST_SETUP_Z|SST_SETUP_Wfbi|SST_SETUP_W0|SST_SETUP_ST0,8);

    pktype = SSTCP_PKT3_BDDDDD;
    s_scale = inContext->hw_texture[0]->s_scale;
    t_scale = inContext->hw_texture[0]->t_scale;
    
    pargb = packARGB(inPivotVertex->color.a,inPivotVertex->color.r,inPivotVertex->color.g,inPivotVertex->color.b);
    
    if(inNumberOfVtx > 0) {
      FxI32 k, vcount = (inNumberOfVtx >= 14 ? 14 : inNumberOfVtx);
	  
	  SET_EXPECTED_SIZE(vSize * (vcount + 1) + sizeof(FxU32));
	  TRI_STRIP_BEGIN(kSetupFan, (vcount + 1), vSize, pktype);

	  TRI_SETF_FAST(inPivotVertex->window.x);
	  TRI_SETF_FAST(inPivotVertex->window.y);
	  TRI_SET(pargb);
	  TRI_SETF(inPivotVertex->window.z);
	  TRI_SETF(inPivotVertex->fog);
	  TRI_SETF_FAST(inPivotVertex->texture[0].q);
	  TRI_SETF(inPivotVertex->texture[0].s * s_scale);
	  TRI_SETF(inPivotVertex->texture[0].t * t_scale);
	  
	  for(k = 0; k < vcount; k++) {
		  TRI_SETF_FAST(inVtx->window.x);
		  TRI_SETF_FAST(inVtx->window.y);
          TRI_SET(pargb);
		  TRI_SETF_FAST(inVtx->window.z);
		  TRI_SETF_FAST(inVtx->fog);
		  TRI_SETF_FAST(inVtx->texture[0].q);
		  TRI_SETF(inVtx->texture[0].s * s_scale);
		  TRI_SETF(inVtx->texture[0].t * t_scale);
		  inVtx++;
      }	  
	  TRI_END;
	  CHECK_SIZE;
	  
	  inNumberOfVtx -= 14;
	  pktype = SSTCP_PKT3_DDDDDD;
	}	

    while(inNumberOfVtx > 0) {
      FxI32 k, vcount = inNumberOfVtx >= 15 ? 15 : inNumberOfVtx;
	  
	  SET_EXPECTED_SIZE(vSize * vcount + sizeof(FxU32));
	  TRI_STRIP_BEGIN(kSetupFan, vcount, vSize, pktype);
	  
	  for(k = 0; k < vcount; k++) {
		  TRI_SETF_FAST(inVtx->window.x);
		  TRI_SETF_FAST(inVtx->window.y);
	      TRI_SET(pargb);
		  TRI_SETF_FAST(inVtx->window.z);
		  TRI_SETF_FAST(inVtx->fog);
		  TRI_SETF_FAST(inVtx->texture[0].q);
		  TRI_SETF(inVtx->texture[0].s * s_scale);
		  TRI_SETF(inVtx->texture[0].t * t_scale);
		  inVtx++;
      }	  
	  TRI_END;
	  CHECK_SIZE;
	  
	  inNumberOfVtx -= 15;
	  pktype = SSTCP_PKT3_DDDDDD;
	}	
	PARAM_END;
}

void
glrRenderFlatMultiTextureTriangleFan(
	GLDContext				inContext,
	const GLDVertex *		inPivotVertex,
	const GLDVertex *		inVtx,
	GLint					inNumberOfVtx,
	GLbitfield				inMode)
{
    FxU32 pktype, vSize, pargb;
    FxFloat s_scale0, t_scale0;   
    FxFloat s_scale1, t_scale1;   
    
    DEBUG_FASTPATH_ENTRY(glrRenderFlatMultiTextureTriangleFan);
    
	if( inNumberOfVtx < 2 ) return;
	
	PARAM_SETUP(SST_SETUP_RGB|SST_SETUP_A|SST_SETUP_Z|SST_SETUP_Wfbi|SST_SETUP_W0|SST_SETUP_ST0|SST_SETUP_W1|SST_SETUP_ST1,11);

    pktype = SSTCP_PKT3_BDDDDD;
    s_scale0 = inContext->hw_texture[0]->s_scale;
    t_scale0 = inContext->hw_texture[0]->t_scale;
    s_scale1 = inContext->hw_texture[1]->s_scale;
    t_scale1 = inContext->hw_texture[1]->t_scale;
    
    pargb = packARGB(inPivotVertex->color.a,inPivotVertex->color.r,inPivotVertex->color.g,inPivotVertex->color.b);
    
    if(inNumberOfVtx > 0) {
      FxI32 k, vcount = (inNumberOfVtx >= 14 ? 14 : inNumberOfVtx);
	  
	  SET_EXPECTED_SIZE(vSize * (vcount + 1) + sizeof(FxU32));
	  TRI_STRIP_BEGIN(kSetupFan, (vcount + 1), vSize, pktype);

	  TRI_SETF_FAST(inPivotVertex->window.x);
	  TRI_SETF_FAST(inPivotVertex->window.y);
	  TRI_SET(pargb);
	  TRI_SETF(inPivotVertex->window.z);
	  TRI_SETF(inPivotVertex->fog);
	  TRI_SETF_FAST(inPivotVertex->texture[1].q);
	  TRI_SETF(inPivotVertex->texture[1].s * s_scale0);
	  TRI_SETF(inPivotVertex->texture[1].t * t_scale0);
	  TRI_SETF_FAST(inPivotVertex->texture[0].q);
	  TRI_SETF(inPivotVertex->texture[0].s * s_scale1);
	  TRI_SETF(inPivotVertex->texture[0].t * t_scale1);
	  
	  for(k = 0; k < vcount; k++) {
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
		  TRI_SETF(inVtx->texture[0].t * t_scale0);
		  inVtx++;
      }	  
	  TRI_END;
	  CHECK_SIZE;
	  
	  inNumberOfVtx -= 14;
	  pktype = SSTCP_PKT3_DDDDDD;
	}	

    while(inNumberOfVtx > 0) {
      FxI32 k, vcount = inNumberOfVtx >= 15 ? 15 : inNumberOfVtx;
	  
	  SET_EXPECTED_SIZE(vSize * vcount + sizeof(FxU32));
	  TRI_STRIP_BEGIN(kSetupFan, vcount, vSize, pktype);
	  
	  for(k = 0; k < vcount; k++) {
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
		  TRI_SETF(inVtx->texture[0].t * t_scale0);
		  inVtx++;
      }	  
	  TRI_END;
	  CHECK_SIZE;
	  
	  inNumberOfVtx -= 15;
	  pktype = SSTCP_PKT3_DDDDDD;
	}	
	PARAM_END;
}


/*
________________________________________________________________________________________

      glrRenderSmoothTriangleFan
________________________________________________________________________________________

*/

void
glrRenderSmoothTextureTriangleFan(
	GLDContext				inContext,
	const GLDVertex *		inPivotVertex,
	const GLDVertex *		inVtx,
	GLint					inNumberOfVtx,
	GLbitfield				inMode)
{
    FxU32 pktype, vSize;
    FxFloat s_scale, t_scale, s0, t0;   
    
    DEBUG_FASTPATH_ENTRY(glrRenderSmoothTextureTriangleFan);
    
	if( inNumberOfVtx < 2 ) return;
	
	PARAM_SETUP(SST_SETUP_RGB|SST_SETUP_A|SST_SETUP_Z|SST_SETUP_Wfbi|SST_SETUP_W0|SST_SETUP_ST0,8);

    pktype = SSTCP_PKT3_BDDDDD;
    s_scale = inContext->hw_texture[0]->s_scale;
    t_scale = inContext->hw_texture[0]->t_scale;
    
    ALIGN_FIFO;
    
    if(inNumberOfVtx > 0) {
      FxI32 k, vcount = (inNumberOfVtx >= 14 ? 14 : inNumberOfVtx);
	  
	  SET_EXPECTED_SIZE(vSize * (vcount + 1) + sizeof(FxU32) * 2);
	  TRI_STRIP_BEGIN_NOHEADER(kSetupFan, (vcount + 1), vSize);

      TRI_SETPAIR_II(0, packetVal | pktype);
      TRI_SETPAIR_FF(inPivotVertex->window.x, inPivotVertex->window.y);
	  TRI_SETPAIR_IF(packARGB(inPivotVertex->color.a,inPivotVertex->color.r,inPivotVertex->color.g,inPivotVertex->color.b),
	                 inPivotVertex->window.z);  
	  TRI_SETPAIR_FF(inPivotVertex->fog, inPivotVertex->texture[0].q);
	  s0 = inPivotVertex->texture[0].s * s_scale;
	  t0 = inPivotVertex->texture[0].t * t_scale;
	  TRI_SETPAIR_FF(s0, t0);
	  
	  for(k = 0; k < vcount; k++) {
	      TRI_SETPAIR_FF(inVtx->window.x, inVtx->window.y);
	      TRI_SETPAIR_IF(packARGB(inVtx->color.a,inVtx->color.r,inVtx->color.g,inVtx->color.b),
	                     inVtx->window.z);
		  TRI_SETPAIR_FF(inVtx->fog,inVtx->texture[0].q);
	      s0 = inVtx->texture[0].s * s_scale;
	      t0 = inVtx->texture[0].t * t_scale;
	      TRI_SETPAIR_FF(s0, t0);
		  inVtx++;
      }	  
	  TRI_END;
	  CHECK_SIZE;
	  
	  inNumberOfVtx -= 14;
	  pktype = SSTCP_PKT3_DDDDDD;
	}	

    while(inNumberOfVtx > 0) {
      FxI32 k, vcount = inNumberOfVtx >= 15 ? 15 : inNumberOfVtx;
	  
	  SET_EXPECTED_SIZE(vSize * vcount + sizeof(FxU32) * 2);
	  TRI_STRIP_BEGIN_NOHEADER(kSetupFan, vcount, vSize);
	  
	  TRI_SETPAIR_II(0, packetVal | pktype);
	  
	  for(k = 0; k < vcount; k++) {
	      TRI_SETPAIR_FF(inVtx->window.x, inVtx->window.y);
	      TRI_SETPAIR_IF(packARGB(inVtx->color.a,inVtx->color.r,inVtx->color.g,inVtx->color.b),
	                     inVtx->window.z);
		  TRI_SETPAIR_FF(inVtx->fog,inVtx->texture[0].q);
	      s0 = inVtx->texture[0].s * s_scale;
	      t0 = inVtx->texture[0].t * t_scale;
	      TRI_SETPAIR_FF(s0, t0);
		  inVtx++;
      }	  
	  TRI_END;
	  CHECK_SIZE;
	  
	  inNumberOfVtx -= 15;
	  pktype = SSTCP_PKT3_DDDDDD;
	}	
	PARAM_END;
}

void
glrRenderSmoothMultiTextureTriangleFan(
	GLDContext				inContext,
	const GLDVertex *		inPivotVertex,
	const GLDVertex *		inVtx,
	GLint					inNumberOfVtx,
	GLbitfield				inMode)
{
    FxU32 pktype, vSize;
    FxFloat s_scale0, t_scale0;   
    FxFloat s_scale1, t_scale1;   
    
    DEBUG_FASTPATH_ENTRY(glrRenderSmoothTextureTriangleFan);
    
	if( inNumberOfVtx < 2 ) return;
	
	PARAM_SETUP(SST_SETUP_RGB|SST_SETUP_A|SST_SETUP_Z|SST_SETUP_Wfbi|SST_SETUP_W0|SST_SETUP_ST0|SST_SETUP_W1|SST_SETUP_ST1,11);

    pktype = SSTCP_PKT3_BDDDDD;
    s_scale0 = inContext->hw_texture[0]->s_scale;
    t_scale0 = inContext->hw_texture[0]->t_scale;
    s_scale1 = inContext->hw_texture[1]->s_scale;
    t_scale1 = inContext->hw_texture[1]->t_scale;
    
    if(inNumberOfVtx > 0) {
      FxI32 k, vcount = (inNumberOfVtx >= 14 ? 14 : inNumberOfVtx);
	  
	  SET_EXPECTED_SIZE(vSize * (vcount + 1) + sizeof(FxU32));
	  TRI_STRIP_BEGIN(kSetupFan, (vcount + 1), vSize, pktype);

	  TRI_SETF_FAST(inPivotVertex->window.x);
	  TRI_SETF_FAST(inPivotVertex->window.y);
	  TRI_SET(packARGB(inPivotVertex->color.a,inPivotVertex->color.r,inPivotVertex->color.g,inPivotVertex->color.b));
	  TRI_SETF(inPivotVertex->window.z);
	  TRI_SETF(inPivotVertex->fog);
	  TRI_SETF_FAST(inPivotVertex->texture[1].q);
	  TRI_SETF(inPivotVertex->texture[1].s * s_scale0);
	  TRI_SETF(inPivotVertex->texture[1].t * t_scale0);
	  TRI_SETF_FAST(inPivotVertex->texture[0].q);
	  TRI_SETF(inPivotVertex->texture[0].s * s_scale1);
	  TRI_SETF(inPivotVertex->texture[0].t * t_scale1);
	  
	  for(k = 0; k < vcount; k++) {
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
      }	  
	  TRI_END;
	  CHECK_SIZE;
	  
	  inNumberOfVtx -= 14;
	  pktype = SSTCP_PKT3_DDDDDD;
	}	

    while(inNumberOfVtx > 0) {
      FxI32 k, vcount = inNumberOfVtx >= 15 ? 15 : inNumberOfVtx;
	  
	  SET_EXPECTED_SIZE(vSize * vcount + sizeof(FxU32));
	  TRI_STRIP_BEGIN(kSetupFan, vcount, vSize, pktype);
	  
	  for(k = 0; k < vcount; k++) {
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
      }	  
	  TRI_END;
	  CHECK_SIZE;
	  
	  inNumberOfVtx -= 15;
	  pktype = SSTCP_PKT3_DDDDDD;
	}	
	PARAM_END;
}
