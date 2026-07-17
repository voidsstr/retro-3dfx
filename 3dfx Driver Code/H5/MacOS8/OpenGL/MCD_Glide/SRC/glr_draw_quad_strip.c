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

      glrRenderFlatQuadStrip
________________________________________________________________________________________

*/

void
glrRenderFlatQuadStrip(
	GLDContext				inContext,
	const GLDVertex *		inVtx,
	GLint					inNumberOfVtx,
	GLenum					/*inMode*/)
{
	GLint					i, j, n;
	GrVertex				v0, v1, v2, v3;

	DEBUG_SLOWPATH_ENTRY( glrRenderFlatQuadStrip );

	n = inNumberOfVtx - 1;
			
	if(inNumberOfVtx < 4) return;
	
	glrLoadVertex( inContext, inVtx[0], v0);
	glrLoadVertex( inContext, inVtx[1], v1);
	
	for(i = 2; i < n; i += 2)
	{
		j = i + 1;
		
		glrLoadVertex_Smooth( inContext, inVtx[j], v3);
		glrLoadVertex_Flat( inContext, inVtx[i], v2, v3);
		
		v0.pargb = v3.pargb;
		v1.pargb = v3.pargb;
				
		grDrawTriangle(&v0, &v1, &v2);
		grDrawTriangle(&v2, &v1, &v3);
		
		v0 = v2;
		v1 = v3;
	}
}


/*
________________________________________________________________________________________

      glrRenderSmoothQuadStrip
________________________________________________________________________________________

*/

void
glrRenderSmoothQuadStrip(
	GLDContext				inContext,
	const GLDVertex *		inVtx,
	GLint					inNumberOfVtx,
	GLbitfield				inMode)
{
	GLfloat					theZoffset;
    FxU32 pktype, vSize;
    
    DEBUG_SLOWPATH_ENTRY(glrRenderSmoothQuadStrip);
    
	theZoffset = inContext->fill_z_offset;
			
	if( inNumberOfVtx < 4 ) return;

    PARAM_SETUP(SST_SETUP_RGB|SST_SETUP_A|SST_SETUP_Z|SST_SETUP_Wfbi,5);
	
    pktype = SSTCP_PKT3_BDDDDD;
     
    while(inNumberOfVtx > 0) {
      FxI32 k, vcount = inNumberOfVtx >= 14 ? 14 : inNumberOfVtx;
      
      /* Round down to even number of vertices... */
	  vcount &= ~1;
	  SET_EXPECTED_SIZE(vSize * vcount + sizeof(FxU32));
	  TRI_STRIP_BEGIN(kSetupStrip, vcount, vSize, pktype);
	  
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
	
	PARAM_END;
}



/*
________________________________________________________________________________________

      glrRenderFlatTextureQuadStrip
________________________________________________________________________________________

*/

void
glrRenderFlatTextureQuadStrip(
	GLDContext				inContext,
	const GLDVertex *		inVtx,
	GLint					inNumberOfVtx,
	GLenum					/*inMode*/)
{
	GLint					i, j, n;
	GrVertex				v0, v1, v2, v3;

	DEBUG_SLOWPATH_ENTRY( glrRenderFlatTextureQuadStrip );

	n = inNumberOfVtx - 1;
			
	if(inNumberOfVtx < 4) return;
	
	glrLoadVertex_Texture( inContext, inVtx[0], v0);
	glrLoadVertex_Texture( inContext, inVtx[1], v1);
		
	for(i = 2; i < n; i += 2)
	{
		j = i + 1;
		
		glrLoadVertex_SmoothTexture( inContext, inVtx[j], v3);
		glrLoadVertex_FlatTexture( inContext, inVtx[i], v2, v3);
		
		v0.pargb = v3.pargb;
		v1.pargb = v3.pargb;
		
		grDrawTriangle(&v0, &v1, &v2);
		grDrawTriangle(&v2, &v1, &v3);
		
		v0 = v2;
		v1 = v3;
	}

}

void
glrRenderFlatMultiTextureQuadStrip(
	GLDContext				inContext,
	const GLDVertex *		inVtx,
	GLint					inNumberOfVtx,
	GLenum					/*inMode*/)
{
	GLint					i, j, n;
	GrVertex				v0, v1, v2, v3;

	DEBUG_SLOWPATH_ENTRY( glrRenderFlatTextureQuadStrip );

	n = inNumberOfVtx - 1;
			
	if(inNumberOfVtx < 4) return;
	
	glrLoadVertex_MultiTexture( inContext, inVtx[0], v0);
	glrLoadVertex_MultiTexture( inContext, inVtx[1], v1);
		
	for(i = 2; i < n; i += 2)
	{
		j = i + 1;
		
		glrLoadVertex_SmoothMultiTexture( inContext, inVtx[j], v3);
		glrLoadVertex_FlatMultiTexture( inContext, inVtx[i], v2, v3);
		
		v0.pargb = v3.pargb;
		v1.pargb = v3.pargb;
		
		grDrawTriangle(&v0, &v1, &v2);
		grDrawTriangle(&v2, &v3, &v1);
		
		v0 = v2;
		v1 = v3;
	}

}



/*
________________________________________________________________________________________

      glrRenderSmoothTextureQuadStrip
________________________________________________________________________________________

*/

void
glrRenderSmoothTextureQuadStrip(
	GLDContext				inContext,
	const GLDVertex *		inVtx,
	GLint					inNumberOfVtx,
	GLbitfield				inMode)
{
    FxU32 pktype, vSize;
    FxFloat s_scale, t_scale;   
    
    DEBUG_FASTPATH_ENTRY(glrRenderSmoothTextureQuadStrip);
    
	if( inNumberOfVtx < 4 ) return;
	
    PARAM_SETUP(SST_SETUP_RGB|SST_SETUP_A|SST_SETUP_Z|SST_SETUP_Wfbi|SST_SETUP_W0|SST_SETUP_ST0,8);
	
    pktype = SSTCP_PKT3_BDDDDD;
    s_scale = inContext->hw_texture[0]->s_scale;
    t_scale = inContext->hw_texture[0]->t_scale;
    
    while(inNumberOfVtx > 0) {
      FxI32 k, vcount = inNumberOfVtx >= 14 ? 14 : inNumberOfVtx;
      
      /* Round down to even number of vertices... */
	  vcount &= ~1;
	  SET_EXPECTED_SIZE(vSize * vcount + sizeof(FxU32));
	  TRI_STRIP_BEGIN(kSetupStrip, vcount, vSize, pktype);
	  
	  for(k = 0; k < vcount; k++) {
		  TRI_SETF_FAST(inVtx->window.x);
		  TRI_SETF_FAST(inVtx->window.y);
		  TRI_SET(packARGB(inVtx->color.a,inVtx->color.r,inVtx->color.g,inVtx->color.b));
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
	
	PARAM_END;
}

void
glrRenderSmoothMultiTextureQuadStrip(
	GLDContext				inContext,
	const GLDVertex *		inVtx,
	GLint					inNumberOfVtx,
	GLbitfield				inMode)
{
    FxU32 pktype, vSize;
    FxFloat s_scale0, t_scale0;   
    FxFloat s_scale1, t_scale1;   
    
    DEBUG_FASTPATH_ENTRY(glrRenderSmoothMultiTextureQuadStrip);
    
	if( inNumberOfVtx < 4 ) return;
	
    PARAM_SETUP(SST_SETUP_RGB|SST_SETUP_A|SST_SETUP_Z|SST_SETUP_Wfbi|SST_SETUP_W0|SST_SETUP_ST0|SST_SETUP_W1|SST_SETUP_ST1,11);
	
    pktype = SSTCP_PKT3_BDDDDD;
    s_scale0 = inContext->hw_texture[0]->s_scale;
    t_scale0 = inContext->hw_texture[0]->t_scale;
    s_scale1 = inContext->hw_texture[1]->s_scale;
    t_scale1 = inContext->hw_texture[1]->t_scale;
    
    while(inNumberOfVtx > 0) {
      FxI32 k, vcount = inNumberOfVtx >= 14 ? 14 : inNumberOfVtx;
      
      /* Round down to even number of vertices... */
	  vcount &= ~1;
	  SET_EXPECTED_SIZE(vSize * vcount + sizeof(FxU32));
	  TRI_STRIP_BEGIN(kSetupStrip, vcount, vSize, pktype);
	  
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
	
	PARAM_END;	
}
