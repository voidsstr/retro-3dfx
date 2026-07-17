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

      glrRenderFlatPolygon
________________________________________________________________________________________

*/

void
glrRenderFlatPolygon(
	GLDContext				inContext,
	const GLDVertex *		inVtx,
	GLint					inNumberOfVtx,
	GLenum					/*inMode*/)
{
	GLint					i;
	GrVertex				v0, v1, v2;
	
	DEBUG_SLOWPATH_ENTRY( glrRenderFlatPolygon );

	if( inNumberOfVtx < 3 ) return;
	
	glrLoadVertex_Smooth( inContext, inVtx[0], v0);
	glrLoadVertex_Flat( inContext, inVtx[1], v1, v0);
	
	v2.pargb = v0.pargb;
	
	for(i = 2; i < inNumberOfVtx; i++)
	{
		glrLoadVertex( inContext, inVtx[i], v2);
		
		grDrawTriangle(&v0, &v1, &v2);
		
		v1 = v2;
	}
}




/*
________________________________________________________________________________________

      glrRenderSmoothPolygon
________________________________________________________________________________________

*/

void
glrRenderSmoothPolygon(
	GLDContext				inContext,
	const GLDVertex *		inVtx,
	GLint					inNumberOfVtx,
	GLenum					/*inMode*/)
{
	GLint					i;
	GrVertex				v0, v1, v2;

	DEBUG_SLOWPATH_ENTRY( glrRenderSmoothPolygon );

	if( inNumberOfVtx < 3 ) return;
	
	glrLoadVertex_Smooth( inContext, inVtx[0], v0);
	glrLoadVertex_Smooth( inContext, inVtx[1], v1);
	
	for(i = 2; i < inNumberOfVtx; i++)
	{
		glrLoadVertex_Smooth( inContext, inVtx[i], v2);
			
		grDrawTriangle(&v0, &v1, &v2);
		
		v1 = v2;
	}
}

/*
________________________________________________________________________________________

      glrRenderFlatTexturePolygon
________________________________________________________________________________________

*/

void
glrRenderFlatTexturePolygon(
	GLDContext				inContext,
	const GLDVertex *		inVtx,
	GLint					inNumberOfVtx,
	GLbitfield				inMode)
{
    FxU32 pktype, vSize, pargb;
    FxFloat s_scale0, t_scale0;   
    FxFloat s0, t0;
	
	DEBUG_FASTPATH_ENTRY( glrRenderFlatTexturePolygon );

	if( inNumberOfVtx < 3 ) return;
	
	PARAM_SETUP(SST_SETUP_RGB|SST_SETUP_A|SST_SETUP_Z|SST_SETUP_Wfbi|SST_SETUP_W0|SST_SETUP_ST0,8);

    pktype = SSTCP_PKT3_BDDDDD;
    s_scale0 = inContext->hw_texture[0]->s_scale;
    t_scale0 = inContext->hw_texture[0]->t_scale;

	pargb = packARGB(inVtx->color.a,
	                 inVtx->color.r,
	                 inVtx->color.g,
	                 inVtx->color.b);
    
	ALIGN_FIFO;
	  
    if(CULLING_DISABLED || cullTri(&inVtx->window.x,&(inVtx+1)->window.x,&(inVtx+2)->window.x,signMode)) {
       
      pktype = SSTCP_PKT3_BDDDDD;
		  
	  while(inNumberOfVtx > 0) {
        SET_EXPECTED_SIZE((vSize + sizeof(FxU32) * 2));           

        TRI_STRIP_BEGIN_NOHEADER(kSetupFan, 1, vSize);
  
        TRI_SETPAIR_IF(packetVal | pktype, inVtx->window.x);  
		TRI_SETPAIR_FI(inVtx->window.y, pargb);
	    TRI_SETPAIR_FF(inVtx->window.z, inVtx->fog);	
        s0 = inVtx->texture[0].s * s_scale0;
		TRI_SETPAIR_FF(inVtx->texture[0].q,s0);
		t0 = inVtx->texture[0].t * t_scale0;
		TRI_SETPAIR_FI(t0, 0);
        TRI_END;
          
        CHECK_SIZE;
        
        inVtx++;  
        inNumberOfVtx--;
		pktype = SSTCP_PKT3_DDDDDD;		       
      }
	}      
	PARAM_END;
}

void
glrRenderFlatMultiTexturePolygon(
	GLDContext				inContext,
	const GLDVertex *		inVtx,
	GLint					inNumberOfVtx,
	GLbitfield				inMode)
{
    FxU32 pktype, vSize, pargb;
    FxFloat s_scale0, t_scale0;   
    FxFloat s_scale1, t_scale1;   
    FxFloat s0, t0, s1, t1;
	
	DEBUG_FASTPATH_ENTRY( glrRenderFlatMultiTexturePolygon );

	if( inNumberOfVtx < 3 ) return;
	
	PARAM_SETUP(SST_SETUP_RGB|SST_SETUP_A|SST_SETUP_Z|SST_SETUP_Wfbi|SST_SETUP_W0|SST_SETUP_ST0|SST_SETUP_W1|SST_SETUP_ST1,11);

    pktype = SSTCP_PKT3_BDDDDD;
    s_scale0 = inContext->hw_texture[0]->s_scale;
    t_scale0 = inContext->hw_texture[0]->t_scale;
    s_scale1 = inContext->hw_texture[1]->s_scale;
    t_scale1 = inContext->hw_texture[1]->t_scale;
    pargb = packARGB(inVtx->color.a,inVtx->color.r,inVtx->color.g,inVtx->color.b);
	ALIGN_FIFO;
	  
    if(CULLING_DISABLED || cullTri(&inVtx->window.x,&(inVtx+1)->window.x,&(inVtx+2)->window.x,signMode)) {
       
      pktype = SSTCP_PKT3_BDDDDD;
		  
	  while(inNumberOfVtx > 0) {
        SET_EXPECTED_SIZE((vSize + sizeof(FxU32)) * 1);           

        TRI_STRIP_BEGIN_NOHEADER(kSetupFan, 1, vSize);
  
        TRI_SETPAIR_IF(packetVal | pktype, inVtx->window.x);  
		TRI_SETPAIR_FI(inVtx->window.y, pargb);	
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
        
        inVtx++;  
        inNumberOfVtx--;
		pktype = SSTCP_PKT3_DDDDDD;		       
      }
	}      
	PARAM_END;
}


/*
________________________________________________________________________________________

      glrRenderSmoothTexturePolygon
________________________________________________________________________________________

*/

void
glrRenderSmoothTexturePolygon(
	GLDContext				inContext,
	const GLDVertex *		inVtx,
	GLint					inNumberOfVtx,
	GLenum					inMode)
{
    FxU32 pktype, vSize;
    FxFloat s_scale0, t_scale0;   
	DEBUG_FASTPATH_ENTRY( glrRenderSmoothTexturePolygon);
    
	if( inNumberOfVtx < 3 ) return;
	
	PARAM_SETUP(SST_SETUP_RGB|SST_SETUP_A|SST_SETUP_Z|SST_SETUP_Wfbi|SST_SETUP_W0|SST_SETUP_ST0,8);

    pktype = SSTCP_PKT3_BDDDDD;
    s_scale0 = inContext->hw_texture[0]->s_scale;
    t_scale0 = inContext->hw_texture[0]->t_scale;
    
    while(inNumberOfVtx > 0) {
      FxI32 k, vcount = inNumberOfVtx >= 15 ? 15 : inNumberOfVtx;
	  
	  SET_EXPECTED_SIZE(vSize * vcount + sizeof(FxU32));
	  TRI_STRIP_BEGIN(kSetupFan, vcount, vSize, pktype);
	  		                   
	  for(k = 0; k < vcount; k++) {
		  TRI_SETF(inVtx->window.x);
		  TRI_SETF(inVtx->window.y);
		  TRI_SET(packARGB(inVtx->color.a,
		                   inVtx->color.r,
		                   inVtx->color.g,
		                   inVtx->color.b));
		  TRI_SETF_FAST(inVtx->window.z);
		  TRI_SETF_FAST(inVtx->fog);
		  TRI_SETF_FAST(inVtx->texture[0].q);
		  TRI_SETF(inVtx->texture[0].s * s_scale0);
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

void
glrRenderSmoothMultiTexturePolygon(
	GLDContext				inContext,
	const GLDVertex *		inVtx,
	GLint					inNumberOfVtx,
	GLenum					inMode)
{
    FxU32 pktype, vSize;
    FxFloat s_scale0, t_scale0;   
    FxFloat s_scale1, t_scale1;   
	DEBUG_FASTPATH_ENTRY( glrRenderSmoothMultiTexturePolygon);
    			
	if( inNumberOfVtx < 3 ) return;
	
	PARAM_SETUP(SST_SETUP_RGB|SST_SETUP_A|SST_SETUP_Z|SST_SETUP_Wfbi|SST_SETUP_W0|SST_SETUP_ST0|SST_SETUP_W1|SST_SETUP_ST1,11);

    pktype = SSTCP_PKT3_BDDDDD;
    s_scale0 = inContext->hw_texture[0]->s_scale;
    t_scale0 = inContext->hw_texture[0]->t_scale;
    s_scale1 = inContext->hw_texture[1]->s_scale;
    t_scale1 = inContext->hw_texture[1]->t_scale;
    
    while(inNumberOfVtx > 0) {
      FxI32 k, vcount = inNumberOfVtx >= 15 ? 15 : inNumberOfVtx;
	  
	  SET_EXPECTED_SIZE(vSize * vcount + sizeof(FxU32));
	  TRI_STRIP_BEGIN(kSetupFan, vcount, vSize, pktype);
	  		                   
	  for(k = 0; k < vcount; k++) {
		  TRI_SETF(inVtx->window.x);
		  TRI_SETF(inVtx->window.y);
		  TRI_SET(packARGB(inVtx->color.a,
		                   inVtx->color.r,
		                   inVtx->color.g,
		                   inVtx->color.b));
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




/*
________________________________________________________________________________________

      glrRenderFlatPolygonPtr
________________________________________________________________________________________

*/

void
glrRenderFlatPolygonPtr(
	GLDContext				inContext,
	const GLDVertex **		inVtx,
	GLint					inNumberOfVtx,
	GLenum					inMode)
{
    FxU32 pktype, vSize, pargb;
	
	DEBUG_FASTPATH_ENTRY( glrRenderFlatPolygonPtr);
    
	if( inNumberOfVtx < 3 ) return;
	
	PARAM_SETUP(SST_SETUP_RGB|SST_SETUP_A|SST_SETUP_Z|SST_SETUP_Wfbi,5);

    pktype = SSTCP_PKT3_BDDDDD;
    
	pargb = packARGB((*inVtx)->color.a,
	                 (*inVtx)->color.r,
	                 (*inVtx)->color.g,
	                 (*inVtx)->color.b);
    
    ALIGN_FIFO;
    
    while(inNumberOfVtx > 0) {
      FxI32 k, vcount = inNumberOfVtx >= 15 ? 15 : inNumberOfVtx;
	  
	  SET_EXPECTED_SIZE(vSize * vcount + sizeof(FxU32) * 2);
	  TRI_STRIP_BEGIN_NOHEADER(kSetupFan, vcount, vSize);
	  TRI_SETPAIR_II(0, packetVal | pktype);
	  	                   
	  for(k = 0; k < vcount; k++) {
		  TRI_SETPAIR_FF((*inVtx)->window.x,(*inVtx)->window.y);
		  TRI_SETPAIR_IF(pargb,(*inVtx)->window.z);
		  TRI_SETF_FAST((*inVtx)->fog);
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

      glrRenderFlatPolygonPtr
________________________________________________________________________________________

*/

void
glrRenderSmoothPolygonPtr(
	GLDContext				inContext,
	const GLDVertex **		inVtx,
	GLint					inNumberOfVtx,
	GLenum					inMode)
{
    FxU32 pktype, vSize;
	DEBUG_FASTPATH_ENTRY( glrRenderSmoothPolygonPtr);
    
	if( inNumberOfVtx < 3 ) return;
	
	PARAM_SETUP(SST_SETUP_RGB|SST_SETUP_A|SST_SETUP_Z|SST_SETUP_Wfbi,5);

    pktype = SSTCP_PKT3_BDDDDD;
    
    while(inNumberOfVtx > 0) {
      FxI32 k, vcount = inNumberOfVtx >= 15 ? 15 : inNumberOfVtx;
	  
	  SET_EXPECTED_SIZE(vSize * vcount + sizeof(FxU32));
	  TRI_STRIP_BEGIN(kSetupFan, vcount, vSize, pktype);
	  		                   
	  for(k = 0; k < vcount; k++) {
		  TRI_SETF((*inVtx)->window.x);
		  TRI_SETF((*inVtx)->window.y);
		  TRI_SET(packARGB((*inVtx)->color.a,
	                 (*inVtx)->color.r,
	                 (*inVtx)->color.g,
	                 (*inVtx)->color.b));
		  TRI_SETF_FAST((*inVtx)->window.z);
		  TRI_SETF_FAST((*inVtx)->fog);
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

      glrRenderFlatTexturePolygonPtr
________________________________________________________________________________________

*/

void
glrRenderFlatTexturePolygonPtr(
	GLDContext				inContext,
	const GLDVertex **		inVtx,
	GLint					inNumberOfVtx,
	GLbitfield				inMode)
{
    FxU32 pktype, vSize, pargb;
    FxFloat s_scale0, t_scale0;   
    FxFloat s0, t0;
	
	DEBUG_FASTPATH_ENTRY( glrRenderFlatTexturePolygonPtr );

	if( inNumberOfVtx < 3 ) return;
	
	PARAM_SETUP(SST_SETUP_RGB|SST_SETUP_A|SST_SETUP_Z|SST_SETUP_Wfbi|SST_SETUP_W0|SST_SETUP_ST0,8);

    pktype = SSTCP_PKT3_BDDDDD;
    s_scale0 = inContext->hw_texture[0]->s_scale;
    t_scale0 = inContext->hw_texture[0]->t_scale;

	pargb = packARGB((*inVtx)->color.a,
	                 (*inVtx)->color.r,
	                 (*inVtx)->color.g,
	                 (*inVtx)->color.b);
    
	ALIGN_FIFO;
	  
    if(CULLING_DISABLED || cullTri(&(*inVtx)->window.x,&(*(inVtx+1))->window.x,&(*(inVtx+2))->window.x,signMode)) {
       
      pktype = SSTCP_PKT3_BDDDDD;
		  
	  while(inNumberOfVtx > 0) {
        SET_EXPECTED_SIZE((vSize + sizeof(FxU32) * 2));           

        TRI_STRIP_BEGIN_NOHEADER(kSetupFan, 1, vSize);
  
        TRI_SETPAIR_IF(packetVal | pktype, (*inVtx)->window.x);  
		TRI_SETPAIR_FI((*inVtx)->window.y, pargb);
	    TRI_SETPAIR_FF((*inVtx)->window.z, (*inVtx)->fog);	
        s0 = (*inVtx)->texture[0].s * s_scale0;
		TRI_SETPAIR_FF((*inVtx)->texture[0].q,s0);
		t0 = (*inVtx)->texture[0].t * t_scale0;
		TRI_SETPAIR_FI(t0, 0);
        TRI_END;
          
        CHECK_SIZE;
        
        inVtx++;  
        inNumberOfVtx--;
		pktype = SSTCP_PKT3_DDDDDD;		       
      }
	}      
	PARAM_END;
}

void
glrRenderFlatMultiTexturePolygonPtr(
	GLDContext				inContext,
	const GLDVertex **		inVtx,
	GLint					inNumberOfVtx,
	GLbitfield				inMode)
{
    FxU32 pktype, vSize, pargb;
    FxFloat s_scale0, t_scale0;   
    FxFloat s_scale1, t_scale1;   
    FxFloat s0, t0, s1, t1;
	
	DEBUG_FASTPATH_ENTRY( glrRenderFlatMultiTexturePolygonPtr );

	if( inNumberOfVtx < 3 ) return;
	
	PARAM_SETUP(SST_SETUP_RGB|SST_SETUP_A|SST_SETUP_Z|SST_SETUP_Wfbi|SST_SETUP_W0|SST_SETUP_ST0|SST_SETUP_W1|SST_SETUP_ST1,11);

    pktype = SSTCP_PKT3_BDDDDD;
    s_scale0 = inContext->hw_texture[0]->s_scale;
    t_scale0 = inContext->hw_texture[0]->t_scale;
    s_scale1 = inContext->hw_texture[1]->s_scale;
    t_scale1 = inContext->hw_texture[1]->t_scale;
    pargb = packARGB((*inVtx)->color.a,(*inVtx)->color.r,(*inVtx)->color.g,(*inVtx)->color.b);
	ALIGN_FIFO;
	  
    if(CULLING_DISABLED || cullTri(&(*inVtx)->window.x,&(*(inVtx+1))->window.x,&(*(inVtx+2))->window.x,signMode)) {
       
      pktype = SSTCP_PKT3_BDDDDD;
		  
	  while(inNumberOfVtx > 0) {
        SET_EXPECTED_SIZE((vSize + sizeof(FxU32)) * 1);           

        TRI_STRIP_BEGIN_NOHEADER(kSetupFan, 1, vSize);
  
        TRI_SETPAIR_IF(packetVal | pktype, (*inVtx)->window.x);  
		TRI_SETPAIR_FI((*inVtx)->window.y, pargb);	
	    TRI_SETPAIR_FF((*inVtx)->window.z, (*inVtx)->fog);	
        s0 = (*inVtx)->texture[1].s * s_scale0;
		TRI_SETPAIR_FF((*inVtx)->texture[1].q,s0);
		t0 = (*inVtx)->texture[1].t * t_scale0;
		TRI_SETPAIR_FF(t0, (*inVtx)->texture[0].q);	
		s1 = (*inVtx)->texture[0].s * s_scale1;
		t1 = (*inVtx)->texture[0].t * t_scale1;
		TRI_SETPAIR_FF(s1,t1);
        TRI_END;
          
        CHECK_SIZE;
        
        inVtx++;  
        inNumberOfVtx--;
		pktype = SSTCP_PKT3_DDDDDD;		       
      }
	}      
	PARAM_END;
}

/*
________________________________________________________________________________________

      glrRenderSmoothTexturePolygonPtr
________________________________________________________________________________________

*/

/* Quake3 fast path */
void
glrRenderSmoothTexturePolygonPtr(
	GLDContext				inContext,
	const GLDVertex **		inVtx,
	GLint					inNumberOfVtx,
	GLbitfield				inMode)
{
    FxU32 pktype, vSize;
    FxFloat s_scale0, t_scale0;   
    FxFloat s0, t0;
	
	DEBUG_FASTPATH_ENTRY( glrRenderSmoothTexturePolygonPtr );

	if( inNumberOfVtx < 3 ) return;
	
	PARAM_SETUP(SST_SETUP_RGB|SST_SETUP_A|SST_SETUP_Z|SST_SETUP_Wfbi|SST_SETUP_W0|SST_SETUP_ST0,8);

    pktype = SSTCP_PKT3_BDDDDD;
    s_scale0 = inContext->hw_texture[0]->s_scale;
    t_scale0 = inContext->hw_texture[0]->t_scale;
    
	ALIGN_FIFO;
	  
    if(CULLING_DISABLED || cullTri(&(*inVtx)->window.x,&(*(inVtx+1))->window.x,&(*(inVtx+2))->window.x,signMode)) {
       
      pktype = SSTCP_PKT3_BDDDDD;
		  
	  while(inNumberOfVtx > 0) {
        SET_EXPECTED_SIZE((vSize + sizeof(FxU32) * 2));           

        TRI_STRIP_BEGIN_NOHEADER(kSetupFan, 1, vSize);
  
        TRI_SETPAIR_IF(packetVal | pktype, (*inVtx)->window.x);  
		TRI_SETPAIR_FI((*inVtx)->window.y, packARGB((*inVtx)->color.a,(*inVtx)->color.r,(*inVtx)->color.g,(*inVtx)->color.b));	
	    TRI_SETPAIR_FF((*inVtx)->window.z, (*inVtx)->fog);	
        s0 = (*inVtx)->texture[0].s * s_scale0;
		TRI_SETPAIR_FF((*inVtx)->texture[0].q,s0);
		t0 = (*inVtx)->texture[0].t * t_scale0;
		TRI_SETPAIR_FI(t0, 0);
        TRI_END;
          
        CHECK_SIZE;
        
        inVtx++;  
        inNumberOfVtx--;
		pktype = SSTCP_PKT3_DDDDDD;		       
      }
	}      
	PARAM_END;
}

/* Quake3 fast path */
void
glrRenderSmoothMultiTexturePolygonPtr(
	GLDContext				inContext,
	const GLDVertex **		inVtx,
	GLint					inNumberOfVtx,
	GLbitfield				inMode)
{
    FxU32 pktype, vSize;
    FxFloat s_scale0, t_scale0;   
    FxFloat s_scale1, t_scale1;   
    FxFloat s0, t0, s1, t1;
	
	DEBUG_FASTPATH_ENTRY( glrRenderSmoothMultiTexturePolygonPtr );

	if( inNumberOfVtx < 3 ) return;
	
	PARAM_SETUP(SST_SETUP_RGB|SST_SETUP_A|SST_SETUP_Z|SST_SETUP_Wfbi|SST_SETUP_W0|SST_SETUP_ST0|SST_SETUP_W1|SST_SETUP_ST1,11);

    pktype = SSTCP_PKT3_BDDDDD;
    s_scale0 = inContext->hw_texture[0]->s_scale;
    t_scale0 = inContext->hw_texture[0]->t_scale;
    s_scale1 = inContext->hw_texture[1]->s_scale;
    t_scale1 = inContext->hw_texture[1]->t_scale;
    
	ALIGN_FIFO;
		
    if(CULLING_DISABLED || cullTri(&(*inVtx)->window.x,&(*(inVtx+1))->window.x,&(*(inVtx+2))->window.x,signMode)) {
       
      pktype = SSTCP_PKT3_BDDDDD;
		  
	  while(inNumberOfVtx > 0) {
        SET_EXPECTED_SIZE((vSize + sizeof(FxU32)) * 1);           

        TRI_STRIP_BEGIN_NOHEADER(kSetupFan, 1, vSize);
  
        TRI_SETPAIR_IF(packetVal | pktype, (*inVtx)->window.x);  
		TRI_SETPAIR_FI((*inVtx)->window.y, packARGB((*inVtx)->color.a,(*inVtx)->color.r,(*inVtx)->color.g,(*inVtx)->color.b));	
	    TRI_SETPAIR_FF((*inVtx)->window.z, (*inVtx)->fog);	
        s0 = (*inVtx)->texture[1].s * s_scale0;
		TRI_SETPAIR_FF((*inVtx)->texture[1].q,s0);
		t0 = (*inVtx)->texture[1].t * t_scale0;
		TRI_SETPAIR_FF(t0, (*inVtx)->texture[0].q);	
		s1 = (*inVtx)->texture[0].s * s_scale1;
		t1 = (*inVtx)->texture[0].t * t_scale1;
		TRI_SETPAIR_FF(s1,t1);
        TRI_END;
          
        CHECK_SIZE;
        
        inVtx++;  
        inNumberOfVtx--;
		pktype = SSTCP_PKT3_DDDDDD;		       
      }
	}      
	PARAM_END;
}
