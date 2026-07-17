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

#include <math.h>

/*
________________________________________________________________________________________

      glrRenderFlatTriangleStrip
________________________________________________________________________________________

*/

void
glrRenderFlatTriangleStrip(
	GLDContext				inContext,
	const GLDVertex *		inVtx,
	GLint					inNumberOfVtx,
	GLbitfield				/*inMode*/)
{
	GLint					i;
	GrVertex				v[3];
    FxU32 replace = 0;
    
	DEBUG_SLOWPATH_ENTRY( glrRenderFlatTriangleStrip );

	if( inNumberOfVtx < 3 ) return;
      
	glrLoadVertex( inContext, inVtx[0], v[0] );
	glrLoadVertex( inContext, inVtx[1], v[1] );
	
	i = 2;
	while( i < inNumberOfVtx ) {

		glrLoadVertex_Smooth( inContext, inVtx[i], v[2]);

        v[0].pargb = v[2].pargb;
        v[1].pargb = v[2].pargb;
		
		grDrawTriangle(&v[0], &v[1], &v[2]);
		
		v[replace] = v[2];
		replace = 1 - replace;
		
		i++;
	}
}

/*
________________________________________________________________________________________

      glrRenderSmoothTriangleStrip
________________________________________________________________________________________

*/

void
glrRenderSmoothTriangleStrip(
	GLDContext				inContext,
	const GLDVertex *		inVtx,
	GLint					inNumberOfVtx,
	GLbitfield				inMode)
{
    FxU32 pktype, vSize;

	DEBUG_FASTPATH_ENTRY( glrRenderSmoothTriangleStrip );
    
	if( inNumberOfVtx < 3 ) return;
	
	PARAM_SETUP(SST_SETUP_RGB|SST_SETUP_A|SST_SETUP_Z|SST_SETUP_Wfbi,5);

    pktype = SSTCP_PKT3_BDDDDD;
    
    while(inNumberOfVtx > 0) {
      FxI32 k, vcount = inNumberOfVtx >= 15 ? 15 : inNumberOfVtx;
	  
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
	  
	  inNumberOfVtx -= 15;
	  pktype = SSTCP_PKT3_DDDDDD;
	}	
	
	PARAM_END;
}

/*
________________________________________________________________________________________

      glrRenderFlatTextureTriangleStrip
________________________________________________________________________________________

*/

void
glrRenderFlatTextureTriangleStrip(
	GLDContext				inContext,
	const GLDVertex *		inVtx,
	GLint					inNumberOfVtx,
	GLbitfield				/*inMode*/)
{
	GLint					i;
	GrVertex				v[3];
    FxU32 replace = 0;
    
	DEBUG_SLOWPATH_ENTRY( glrRenderFlatTextureTriangleStrip );

	if( inNumberOfVtx < 3 ) return;
	
	glrLoadVertex_Texture( inContext, inVtx[0], v[0]);
	glrLoadVertex_Texture( inContext, inVtx[1], v[1]);
		
	i = 2;
	while( i < inNumberOfVtx ) {
	
		glrLoadVertex_SmoothTexture( inContext, inVtx[i], v[2]);
				
        v[0].pargb = v[2].pargb;
        v[1].pargb = v[2].pargb;

		grDrawTriangle(&v[0], &v[1], &v[2]);
		  
		v[replace] = v[2];
        replace = 1 - replace;
        
		i++;
	}
}

void
glrRenderFlatMultiTextureTriangleStrip(
	GLDContext				inContext,
	const GLDVertex *		inVtx,
	GLint					inNumberOfVtx,
	GLbitfield				/*inMode*/)
{
	GLint					i;
	GrVertex				v[3];
    FxU32 replace = 0;

	DEBUG_SLOWPATH_ENTRY( glrRenderFlatMultiTextureTriangleStrip );

	if( inNumberOfVtx < 3 ) return;
	
	glrLoadVertex_Texture( inContext, inVtx[0], v[0]);
	glrLoadVertex_Texture( inContext, inVtx[1], v[1]);
		
	i = 2;
	while( i < inNumberOfVtx ) {
	
		glrLoadVertex_SmoothMultiTexture( inContext, inVtx[i], v[2]);
				
        v[0].pargb = v[2].pargb;
        v[1].pargb = v[2].pargb;

		grDrawTriangle(&v[0], &v[1], &v[2]);
		  
		v[replace] = v[2];
        replace = 1 - replace;

		i++;
	}
}

/*
________________________________________________________________________________________

      glrRenderSmoothTextureTriangleStrip
________________________________________________________________________________________

*/

/* Quake3 fast path */
void
glrRenderSmoothTextureTriangleStrip(
	GLDContext				inContext,
	const GLDVertex *		inVtx,
	GLint					inNumberOfVtx,
	GLbitfield				inMode)
{
    FxU32 pktype, vSize;
    FxFloat s_scale0, t_scale0;   
    FxFloat s0, t0;
    const GLDVertex *lastVtx;
	
	DEBUG_FASTPATH_ENTRY( glrRenderSmoothTextureTriangleStrip );

	if( inNumberOfVtx < 3 ) return;
	
	lastVtx = inVtx + inNumberOfVtx - 2;
	
	/* The vertex size is really only 8 words, but there is an extra pad word to keep alignment. */
	PARAM_SETUP(SST_SETUP_RGB|SST_SETUP_A|SST_SETUP_Z|SST_SETUP_Wfbi|SST_SETUP_W0|SST_SETUP_ST0,9);

    pktype = SSTCP_PKT3_BDDDDD;
    s_scale0 = inContext->hw_texture[0]->s_scale;
    t_scale0 = inContext->hw_texture[0]->t_scale;
    
    /* For now, just make enough room for the worst case scenario. */
	ALIGN_FIFO;
	  
	if(CULLING_ENABLED)
	{	
	  while(1)
	  {
cullTriangles:	
        if(inVtx >= lastVtx)
          break;
            
        /* If we get a valid triangle in this state, then
           send down three vertices and increment vertex pointer
           by one and switch to strip state. */  
        if(cullTri(&inVtx->window.x,&(inVtx+1)->window.x,&(inVtx+2)->window.x,signMode)) {
          SET_EXPECTED_SIZE((vSize + sizeof(FxU32)) * 3);           
	
		  pktype = SSTCP_PKT3_BDDDDD;
		  __cullStripHdr &= ~(SST_SETUP_CULL_NEGATIVE << SSTCP_PKT3_PMASK_SHIFT);
		  if(signMode) __cullStripHdr |= (SST_SETUP_CULL_NEGATIVE << SSTCP_PKT3_PMASK_SHIFT);
		  
		  TRI_STRIP_BEGIN_NOHEADER(kSetupStrip, 1, vSize);
		  
		  TRI_SETPAIR_IF(packetVal | pktype, inVtx->window.x);  
		  TRI_SETPAIR_FI(inVtx->window.y, packARGB(inVtx->color.a,inVtx->color.r,inVtx->color.g,inVtx->color.b));	
	      TRI_SETPAIR_FF(inVtx->window.z, inVtx->fog);	
          s0 = inVtx->texture[0].s * s_scale0;
		  TRI_SETPAIR_FF(inVtx->texture[0].q,s0);
		  t0 = inVtx->texture[0].t * t_scale0;
		  TRI_SETPAIR_FI(t0, 0);

          inVtx++;
		  pktype = SSTCP_PKT3_DDDDDD;		       
		  TRI_SETPAIR_IF(packetVal | pktype, inVtx->window.x);  
		  TRI_SETPAIR_FI(inVtx->window.y, packARGB(inVtx->color.a,inVtx->color.r,inVtx->color.g,inVtx->color.b));	
	      TRI_SETPAIR_FF(inVtx->window.z, inVtx->fog);	
          s0 = inVtx->texture[0].s * s_scale0;
		  TRI_SETPAIR_FF(inVtx->texture[0].q,s0);
		  t0 = inVtx->texture[0].t * t_scale0;
		  TRI_SETPAIR_FI(t0, 0);

          inVtx++;
		  TRI_SETPAIR_IF(packetVal | pktype, inVtx->window.x);  
		  TRI_SETPAIR_FI(inVtx->window.y, packARGB(inVtx->color.a,inVtx->color.r,inVtx->color.g,inVtx->color.b));	
	      TRI_SETPAIR_FF(inVtx->window.z, inVtx->fog);	
          s0 = inVtx->texture[0].s * s_scale0;
		  TRI_SETPAIR_FF(inVtx->texture[0].q,s0);
		  t0 = inVtx->texture[0].t * t_scale0;
		  TRI_SETPAIR_FI(t0, 0);
            
          TRI_END;
          
          CHECK_SIZE;
            
          inVtx--;
          signMode ^= 0x80000000;
		  goto cullStrips;
		}
		inVtx++;
		signMode ^= 0x80000000;
		goto cullTriangles;

cullStrips:		    
        if(inVtx >= lastVtx)
          break;

        /* If we get a valid triangle in this state, then just
           send down the next vertex.  Otherwise we have to go
           back to culling full triangles so we can restart. */
        if(cullTri(&inVtx->window.x,&(inVtx+1)->window.x,&(inVtx+2)->window.x,signMode)) {
          inVtx += 2;
          SET_EXPECTED_SIZE((vSize + sizeof(FxU32)));           
		  TRI_STRIP_BEGIN_NOHEADER(kSetupStrip, 1, vSize);

		  TRI_SETPAIR_IF(packetVal | pktype, inVtx->window.x);  
		  TRI_SETPAIR_FI(inVtx->window.y, packARGB(inVtx->color.a,inVtx->color.r,inVtx->color.g,inVtx->color.b));	
	      TRI_SETPAIR_FF(inVtx->window.z, inVtx->fog);	
          s0 = inVtx->texture[0].s * s_scale0;
		  TRI_SETPAIR_FF(inVtx->texture[0].q,s0);
		  t0 = inVtx->texture[0].t * t_scale0;
		  TRI_SETPAIR_FI(t0, 0);
		  
		  TRI_END;
		  
		  CHECK_SIZE;
          inVtx--;
	      signMode ^= 0x80000000;
          goto cullStrips;          
            
        }
        inVtx++;
        signMode ^= 0x80000000;
        goto cullTriangles;
	  }
	} else {
	  SET_EXPECTED_SIZE((vSize + sizeof(FxU32)) * inNumberOfVtx);
	  while(inNumberOfVtx > 0) {
	    /* No culling checks... */	        
        TRI_STRIP_BEGIN_NOHEADER(kSetupStrip, 1, vSize);
		
		TRI_SETPAIR_IF(packetVal | pktype, inVtx->window.x);  
		TRI_SETPAIR_FI(inVtx->window.y, packARGB(inVtx->color.a,inVtx->color.r,inVtx->color.g,inVtx->color.b));	
	    TRI_SETPAIR_FF(inVtx->window.z, inVtx->fog);	
        s0 = inVtx->texture[0].s * s_scale0;
		TRI_SETPAIR_FF(inVtx->texture[0].q,s0);
		t0 = inVtx->texture[0].t * t_scale0;
		TRI_SETPAIR_FI(t0, 0);
        
        TRI_END;
	    
	    inVtx++;
	    inNumberOfVtx--;
	    pktype = SSTCP_PKT3_DDDDDD;
	  }
	  CHECK_SIZE;
	}      
	PARAM_END;
}

/* Quake3 fast path */
void
glrRenderSmoothMultiTextureTriangleStrip(
	GLDContext				inContext,
	const GLDVertex *		inVtx,
	GLint					inNumberOfVtx,
	GLbitfield				inMode)
{
    FxU32 pktype, vSize;
    FxFloat s_scale0, t_scale0;   
    FxFloat s_scale1, t_scale1;   
    FxFloat s0, t0, s1, t1;
    const GLDVertex *lastVtx;
	
	DEBUG_FASTPATH_ENTRY( glrRenderSmoothMultiTextureTriangleStrip );

	if( inNumberOfVtx < 3 ) return;
	
	lastVtx = inVtx + inNumberOfVtx - 2;
	
	PARAM_SETUP(SST_SETUP_RGB|SST_SETUP_A|SST_SETUP_Z|SST_SETUP_Wfbi|SST_SETUP_W0|SST_SETUP_ST0|SST_SETUP_W1|SST_SETUP_ST1,11);

    pktype = SSTCP_PKT3_BDDDDD;
    s_scale0 = inContext->hw_texture[0]->s_scale;
    t_scale0 = inContext->hw_texture[0]->t_scale;
    s_scale1 = inContext->hw_texture[1]->s_scale;
    t_scale1 = inContext->hw_texture[1]->t_scale;
    
    /* For now, just make enough room for the worst case scenario. */
	ALIGN_FIFO;
	  
    if(CULLING_ENABLED)
    {	
	  while(1)
	  {
cullTriangles:	
        if(inVtx >= lastVtx)
          break;
            
        /* If we get a valid triangle in this state, then
           send down three vertices and increment vertex pointer
           by one and switch to strip state. */  
        if(cullTri(&inVtx->window.x,&(inVtx+1)->window.x,&(inVtx+2)->window.x,signMode)) {
          SET_EXPECTED_SIZE((vSize + sizeof(FxU32)) * 3);           
	
		  pktype = SSTCP_PKT3_BDDDDD;
		  __cullStripHdr &= ~(SST_SETUP_CULL_NEGATIVE << SSTCP_PKT3_PMASK_SHIFT);
		  if(signMode) __cullStripHdr |= (SST_SETUP_CULL_NEGATIVE << SSTCP_PKT3_PMASK_SHIFT);
		  
		  TRI_STRIP_BEGIN_NOHEADER(kSetupStrip, 1, vSize);
		  
		  TRI_SETPAIR_IF(packetVal | pktype, inVtx->window.x);  
		  TRI_SETPAIR_FI(inVtx->window.y, packARGB(inVtx->color.a,inVtx->color.r,inVtx->color.g,inVtx->color.b));	
	      TRI_SETPAIR_FF(inVtx->window.z, inVtx->fog);	
          s0 = inVtx->texture[1].s * s_scale0;
		  TRI_SETPAIR_FF(inVtx->texture[1].q,s0);
		  t0 = inVtx->texture[1].t * t_scale0;
		  TRI_SETPAIR_FF(t0, inVtx->texture[0].q);	
		  s1 = inVtx->texture[0].s * s_scale1;
		  t1 = inVtx->texture[0].t * t_scale1;
		  TRI_SETPAIR_FF(s1,t1);

          inVtx++;
		  pktype = SSTCP_PKT3_DDDDDD;		       
		  TRI_SETPAIR_IF(packetVal | pktype, inVtx->window.x);  
		  TRI_SETPAIR_FI(inVtx->window.y, packARGB(inVtx->color.a,inVtx->color.r,inVtx->color.g,inVtx->color.b));	
	      TRI_SETPAIR_FF(inVtx->window.z, inVtx->fog);	
          s0 = inVtx->texture[1].s * s_scale0;
		  TRI_SETPAIR_FF(inVtx->texture[1].q,s0);
		  t0 = inVtx->texture[1].t * t_scale0;
		  TRI_SETPAIR_FF(t0, inVtx->texture[0].q);	
		  s1 = inVtx->texture[0].s * s_scale1;
		  t1 = inVtx->texture[0].t * t_scale1;
		  TRI_SETPAIR_FF(s1,t1);

          inVtx++;
		  TRI_SETPAIR_IF(packetVal | pktype, inVtx->window.x);  
		  TRI_SETPAIR_FI(inVtx->window.y, packARGB(inVtx->color.a,inVtx->color.r,inVtx->color.g,inVtx->color.b));	
	      TRI_SETPAIR_FF(inVtx->window.z, inVtx->fog);	
          s0 = inVtx->texture[1].s * s_scale0;
		  TRI_SETPAIR_FF(inVtx->texture[1].q,s0);
		  t0 = inVtx->texture[1].t * t_scale0;
		  TRI_SETPAIR_FF(t0, inVtx->texture[0].q);	
		  s1 = inVtx->texture[0].s * s_scale1;
		  t1 = inVtx->texture[0].t * t_scale1;
		  TRI_SETPAIR_FF(s1,t1);
            
          TRI_END;
          
          CHECK_SIZE;
            
          inVtx--;
          signMode ^= 0x80000000;
		  goto cullStrips;
		}
		inVtx++;
		signMode ^= 0x80000000;
		goto cullTriangles;

cullStrips:		    
        if(inVtx >= lastVtx)
          break;

        /* If we get a valid triangle in this state, then just
           send down the next vertex.  Otherwise we have to go
           back to culling full triangles so we can restart. */
        if(cullTri(&inVtx->window.x,&(inVtx+1)->window.x,&(inVtx+2)->window.x,signMode)) {
          inVtx += 2;
          SET_EXPECTED_SIZE((vSize + sizeof(FxU32)));           
		  TRI_STRIP_BEGIN_NOHEADER(kSetupStrip, 1, vSize);

		  TRI_SETPAIR_IF(packetVal | pktype, inVtx->window.x);  
		  TRI_SETPAIR_FI(inVtx->window.y, packARGB(inVtx->color.a,inVtx->color.r,inVtx->color.g,inVtx->color.b));	
	      TRI_SETPAIR_FF(inVtx->window.z, inVtx->fog);	
          s0 = inVtx->texture[1].s * s_scale0;
		  TRI_SETPAIR_FF(inVtx->texture[1].q,s0);
		  t0 = inVtx->texture[1].t * t_scale0;
		  TRI_SETPAIR_FF(t0, inVtx->texture[0].q);	
		  s1 = inVtx->texture[0].s * s_scale1;
		  t1 = inVtx->texture[0].t * t_scale1;
		  TRI_SETPAIR_FF(s1,t1);
		  
		  TRI_END;
		  
		  CHECK_SIZE;
          inVtx--;
	      signMode ^= 0x80000000;
          goto cullStrips;          
            
        }
        inVtx++;
        signMode ^= 0x80000000;
        goto cullTriangles;
	  }
	} else {
	  SET_EXPECTED_SIZE((vSize + sizeof(FxU32)) * inNumberOfVtx);
	  while(inNumberOfVtx > 0) {
	    /* No culling checks... */	        
        TRI_STRIP_BEGIN_NOHEADER(kSetupStrip, 1, vSize);
	    TRI_SETPAIR_IF(packetVal | pktype, inVtx->window.x);  
	    TRI_SETPAIR_FI(inVtx->window.y, packARGB(inVtx->color.a,inVtx->color.r,inVtx->color.g,inVtx->color.b));
        TRI_SETPAIR_FF(inVtx->window.z, inVtx->fog);	
        s0 = inVtx->texture[1].s * s_scale0;
		TRI_SETPAIR_FF(inVtx->texture[1].q,s0);
		t0 = inVtx->texture[1].t * t_scale0;
		TRI_SETPAIR_FF(t0, inVtx->texture[0].q);	
		s1 = inVtx->texture[0].s * s_scale1;
		t1 = inVtx->texture[0].t * t_scale1;
		TRI_SETPAIR_FF(s1,t1);
        TRI_END;
	    inVtx++;
	    inNumberOfVtx--;
	    pktype = SSTCP_PKT3_DDDDDD;
	  }
	  CHECK_SIZE;
	}      
	PARAM_END;
}

