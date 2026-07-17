
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

#define STORE_THRESHOLD 8



/*
________________________________________________________________________________________

      glrVARenderTextureTriangles
________________________________________________________________________________________


GLD_VA_TYPE_V4F_C4UB_T2F
GLDVertexArrayTex1Element

*/
#if 0

/* -----> glide version */
void _glrVARenderTextureTriangles(
	GLDContext			inContext,
	void *				inData,
	GLint				inFirst,
	GLsizei				inCount,
	const GLuint *		inIndices)
{
	GrVertex			v0, v1, v2;
	GLint               i, theOffset;
	GLDVertexArrayTex1Element * theVtx, *theVtx_base = (GLDVertexArrayTex1Element *) inData;

	glrVASetupIndices( inIndices );
	
	DEBUG_SLOWPATH_ENTRY( _glrVARenderTextureTriangles );
  
	inCount -= 2;
	for(i = 0; i < inCount; i += 3)
	{
		glrVAGetNextVertexPtr(theVtx);
		glrLoadVertexFromArray_SmoothTexture( inContext, theVtx, v0 );

		glrVAGetNextVertexPtr(theVtx);
		glrLoadVertexFromArray_SmoothTexture( inContext, theVtx, v1 );

		glrVAGetNextVertexPtr(theVtx);
		glrLoadVertexFromArray_SmoothTexture( inContext, theVtx, v2 );
        
		grDrawTriangle(&v0, &v1, &v2);
	}

}
#endif


/* -----> cmd fifo version */
void glrVARenderTextureTrianglesG3(
	GLDContext			inContext,
	void *				inData,
	GLint				inFirst,
	GLsizei				inCount,
	const GLuint *		inIndices)

{
	GLint               i;
	GLint noCull;
	GLDVertexArrayTex1Element *theVtx_base = (GLDVertexArrayTex1Element *) inData;
    FxU32               pktype, vSize;
    FxFloat             s_scale, t_scale; 
    FxFloat             s0, t0;  

	//glrVASetupIndices( inIndices );
	
	DEBUG_FASTPATH_ENTRY( glrVARenderTextureTrianglesG3 );

	if( inCount < 3 ) return;

	PARAM_SETUP(SST_SETUP_RGB|SST_SETUP_A|SST_SETUP_Z|SST_SETUP_Wfbi|SST_SETUP_ST0,7);

  pktype = SSTCP_PKT3_BDDDDD;
  s_scale = inContext->hw_texture[0]->s_scale;
  t_scale = inContext->hw_texture[0]->t_scale;

	ALIGN_FIFO;

  noCull = CULLING_DISABLED;
  
    {
      GLuint *vArray = (GLuint *)inIndices, v1_idx, v2_idx, v3_idx;
      GLDVertexArrayTex1Element *v1, *v2, *v3;
      GLuint history[2], replace;

      replace = 0;
	    inCount -= 2;      
	    history[0] = 0xffffffff;
	    history[1] = 0xffffffff;

      for(i = 0; i  < inCount; i += 3) 
      {
        v1_idx = *vArray++ - inFirst;
        v2_idx = *vArray++ - inFirst;
        v3_idx = *vArray++ - inFirst;
        v1 = &theVtx_base[v1_idx];
        v2 = &theVtx_base[v2_idx];
        v3 = &theVtx_base[v3_idx];

        if(noCull || cullTri(&v1->vertex.x, &v2->vertex.x, &v3->vertex.x, signMode))
        {  

          if(history[0] == v1_idx && history[1] == v2_idx && 1)
          {
	        SET_EXPECTED_SIZE((vSize * 1) + sizeof(FxU32));
	        TRI_STRIP_BEGIN_NOHEADER(kSetupStrip, 1, vSize);
	           
	        TRI_SETPAIR_IF(packetVal | SSTCP_PKT3_DDDDDD, v3->vertex.x);  
	        TRI_SETPAIR_FI(v3->vertex.y, v3->color);	
	        s0 = v3->sw1 * s_scale;
	        TRI_SETPAIR_FF(v3->vertex.z, v3->vertex.w);	
	        t0 = v3->tw1 * t_scale;
	        TRI_SETPAIR_FF(s0,t0);
	
	        TRI_END;
	        CHECK_SIZE;  
  
	        *(FxU32 *)((FxU32)history + replace) = v3_idx;            
	        replace = 4 - replace;
          }
          else
          {
	        SET_EXPECTED_SIZE((vSize * 3) + sizeof(FxU32));
	        TRI_STRIP_BEGIN_NOHEADER(kSetupStrip, 3, vSize);
	           
	        TRI_SETPAIR_IF(packetVal | SSTCP_PKT3_BDDDDD, v1->vertex.x);
	        TRI_SETPAIR_FI(v1->vertex.y, v1->color);           
	        s0 = v1->sw1 * s_scale;
	        TRI_SETPAIR_FF(v1->vertex.z, v1->vertex.w);
	        t0 = v1->tw1 * t_scale;
	        TRI_SETPAIR_FF(s0,t0);
	
	        TRI_SETPAIR_FF(v2->vertex.x, v2->vertex.y);  
	        TRI_SETPAIR_IF(v2->color, v2->vertex.z);	
	        s0 = v2->sw1 * s_scale;
	        TRI_SETPAIR_FF(v2->vertex.w, s0);	
	        t0 = v2->tw1 * t_scale;
	
	        TRI_SETPAIR_FF(t0, v3->vertex.x);  
	        TRI_SETPAIR_FI(v3->vertex.y, v3->color);	
	        s0 = v3->sw1 * s_scale;
	        TRI_SETPAIR_FF(v3->vertex.z, v3->vertex.w);	
	        t0 = v3->tw1 * t_scale;
	        TRI_SETPAIR_FF(s0,t0);
	
	        TRI_END;
	        CHECK_SIZE;  

            history[0] = v3_idx;
            history[1] = v2_idx;
            replace = 4;
          }
        }
        else /* Didn't draw, nuke history buffer */
        {
          //glr_debug_printf("c: %4d %4d %4d\n",v1_idx,v2_idx,v3_idx);
          history[0] = history[1] = 0xffffffff;
        }
      }
    }    
  
#if 0  
	inCount -= 2;
	for(i = 0; i < inCount; i += 3)
	{

	  SET_EXPECTED_SIZE((vSize * 3) + sizeof(FxU32));
	  TRI_STRIP_BEGIN_NOHEADER(kSetupStrip, 3, vSize);
	  
	  glrVAGetNextVertexPtr(theVtx);

      TRI_SETPAIR_IF(packetVal | pktype, (theVtx)->vertex.x);  
      TRI_SETPAIR_FI((theVtx)->vertex.y, (theVtx)->color);	
      s0 = (theVtx)->sw1 * s_scale;
      TRI_SETPAIR_FF((theVtx)->vertex.z, (theVtx)->vertex.w);	
      t0 = (theVtx)->tw1 * t_scale;
      TRI_SETPAIR_FF(s0,t0);

	  glrVAGetNextVertexPtr(theVtx);

      TRI_SETPAIR_FF((theVtx)->vertex.x, (theVtx)->vertex.y);  
      TRI_SETPAIR_IF((theVtx)->color, (theVtx)->vertex.z);	
      s0 = (theVtx)->sw1 * s_scale;
      TRI_SETPAIR_FF((theVtx)->vertex.w, s0);	
      t0 = (theVtx)->tw1 * t_scale;

	  glrVAGetNextVertexPtr(theVtx);

      TRI_SETPAIR_FF(t0, (theVtx)->vertex.x);  
      TRI_SETPAIR_FI((theVtx)->vertex.y, (theVtx)->color);	
      s0 = (theVtx)->sw1 * s_scale;
      TRI_SETPAIR_FF((theVtx)->vertex.z, (theVtx)->vertex.w);	
      t0 = (theVtx)->tw1 * t_scale;
      TRI_SETPAIR_FF(s0,t0);

 	  TRI_END;
	  CHECK_SIZE;  
	}
#endif
	PARAM_END;
}


/* -----> cmd fifo version */
#pragma profile off
// compiler bug: profiler insertion doesn't work in functions with Altivec code
void glrVARenderTextureTrianglesG4(
	GLDContext			inContext,
	void *				inData,
	GLint				inFirst,
	GLsizei				inCount,
	const GLuint *		inIndices)

{
	GLint               i;
	GLint noCull;
	GLDVertexArrayTex1Element *theVtx_base = (GLDVertexArrayTex1Element *) inData;
  FxU32               pktype, vSize;
  FxFloat             s_scale, t_scale; 

  static vector unsigned long triBuffer[32];
  unsigned long *buff;
  vector unsigned char byteSwap = { 3,  2,  1,  0, 
                                    7,  6,  5,  4, 
                                   11, 10,  9,  8,
                                   15, 14, 13, 12 };
                                     
	//glrVASetupIndices( inIndices );
	
	PROFILE_ENTRY("glrVARenderTextureTrianglesG4");

	DEBUG_FASTPATH_ENTRY( glrVARenderTextureTrianglesG4 );

	if( inCount < 3 ) goto exit;

	PARAM_SETUP(SST_SETUP_RGB|SST_SETUP_A|SST_SETUP_Z|SST_SETUP_Wfbi|SST_SETUP_ST0,7);

  pktype = SSTCP_PKT3_BDDDDD;
  s_scale = inContext->hw_texture[0]->s_scale;
  t_scale = inContext->hw_texture[0]->t_scale;

  noCull = CULLING_DISABLED;
  
	{

      GLuint *vArray = (GLuint *)inIndices, v1_idx, v2_idx, v3_idx;
      GLDVertexArrayTex1Element *v1, *v2, *v3;
      GLuint history[2], replace;
      FxU32 t0, t1, t2, t3, t4;
      FxFloat f0, f1;
      buff = (FxU32 *)triBuffer;
      
      replace = 0;
	    inCount -= 2;      
	    history[0] = 0xffffffff;
	    history[1] = 0xffffffff;
	    
      for(i = 0; i  < inCount; i += 3) 
      {
        v1_idx = *vArray++ - inFirst;
        v2_idx = *vArray++ - inFirst;
        v3_idx = *vArray++ - inFirst;
        v1 = &theVtx_base[v1_idx];
        v2 = &theVtx_base[v2_idx];
        v3 = &theVtx_base[v3_idx];

        if(noCull || cullTri(&v1->vertex.x, &v2->vertex.x, &v3->vertex.x, signMode))
        { 
          /* Okay, we know we want to draw this triangle, so see if we can continue a strip. */  
          /* I don't think I have to do any futzing around with the triangle culling bits since */
          /* In theory I should always be in sync with the hardware */        
          if(history[0] == v1_idx && history[1] == v2_idx)
          {
	          FxU32 packetVal = (((kSetupStrip) << SSTCP_PKT3_SMODE_SHIFT) |   /* [27:22] */ \
	                           ((1) << SSTCP_PKT3_NUMVERTEX_SHIFT) | /* [9:6] */ \
	                           __cullStripHdr);
	          *buff++ = packetVal | SSTCP_PKT3_DDDDDD;
	          t0 = *(FxU32 *)&v3->vertex.x;
	          t1 = *(FxU32 *)&v3->vertex.y;
	          t2 = *(FxU32 *)&v3->color;
	          t3 = *(FxU32 *)&v3->vertex.z;
	          t4 = *(FxU32 *)&v3->vertex.w;
	          *buff++ = t0;
	          *buff++ = t1;
	          *buff++ = t2;
	          *buff++ = t3;
	          *buff++ = t4;	     
	          f0 = v3->sw1;
	          f1 = v3->tw1;
	          f0 *= s_scale;
	          f1 *= t_scale;
	          *((FxFloat *)buff)++ = f0;
	          *((FxFloat *)buff)++ = f1;
            /* Update history buffer */
            *(FxU32 *)((FxU32)history + replace) = v3_idx;            
            replace = 4 - replace;
            //glr_debug_printf("s");            
            //glr_debug_printf("n: %4d %4d\n",history[0],history[1]);
          }
          else
          {           
	          FxU32 packetVal = (((kSetupStrip) << SSTCP_PKT3_SMODE_SHIFT) |   /* [27:22] */ \
	                           ((3) << SSTCP_PKT3_NUMVERTEX_SHIFT) | /* [9:6] */ \
	                           __cullStripHdr);
	          *buff++ = packetVal | SSTCP_PKT3_BDDDDD;
	          t0 = *(FxU32 *)&v1->vertex.x;
	          t1 = *(FxU32 *)&v1->vertex.y;
	          t2 = *(FxU32 *)&v1->color;
	          t3 = *(FxU32 *)&v1->vertex.z;
	          t4 = *(FxU32 *)&v1->vertex.w;
	          *buff++ = t0;
	          *buff++ = t1;
	          *buff++ = t2;
	          *buff++ = t3;
	          *buff++ = t4;	     
	          f0 = v1->sw1;
	          f1 = v1->tw1;
	          f0 *= s_scale;
	          f1 *= t_scale;
	          *((FxFloat *)buff)++ = f0;
	          *((FxFloat *)buff)++ = f1;
            
	          t0 = *(FxU32 *)&v2->vertex.x;
	          t1 = *(FxU32 *)&v2->vertex.y;
	          t2 = *(FxU32 *)&v2->color;
	          t3 = *(FxU32 *)&v2->vertex.z;
	          t4 = *(FxU32 *)&v2->vertex.w;
	          *buff++ = t0;
	          *buff++ = t1;
	          *buff++ = t2;
	          *buff++ = t3;
	          *buff++ = t4;	     
	          f0 = v2->sw1;
	          f1 = v2->tw1;
	          f0 *= s_scale;
	          f1 *= t_scale;
	          *((FxFloat *)buff)++ = f0;
	          *((FxFloat *)buff)++ = f1;

	          t0 = *(FxU32 *)&v3->vertex.x;
	          t1 = *(FxU32 *)&v3->vertex.y;
	          t2 = *(FxU32 *)&v3->color;
	          t3 = *(FxU32 *)&v3->vertex.z;
	          t4 = *(FxU32 *)&v3->vertex.w;
	          *buff++ = t0;
	          *buff++ = t1;
	          *buff++ = t2;
	          *buff++ = t3;
	          *buff++ = t4;	     
	          f0 = v3->sw1;
	          f1 = v3->tw1;
	          f0 *= s_scale;
	          f1 *= t_scale;
	          *((FxFloat *)buff)++ = f0;
	          *((FxFloat *)buff)++ = f1;

            /* Started a new triangle.  Reset history buffer. */
            history[0] = v3_idx;
            history[1] = v2_idx;
            replace = 4;
            //glr_debug_printf("t");            
          }            
          if(((FxU32)buff - (FxU32)triBuffer) > 16*STORE_THRESHOLD) {
            vector unsigned long *src = triBuffer, tmp;

            // Pad buffer to 16 bytes.
            while(((FxU32)buff & 0xf) != 0) {
              *buff++ = 0;
            }
            
            CHECK_FOR_ROOM((FxU32)buff - (FxU32)triBuffer + 16);
            TRI_STRIP_BEGIN_NOHEADER_G4(kSetupStrip, 3, vSize);
            while((FxU32)tPackPtr & 15) {
              *tPackPtr++ = 0;
            }

            // Transfer data 128 bits at a time.
            while((FxU32)src < (FxU32)buff) {
              tmp = *src++;
              // Do byte swap 
              tmp = vec_perm(tmp,tmp,byteSwap);
              *((vector unsigned long *)tPackPtr)++ = tmp;
            }
            // Reset buffer pointer
            buff = (FxU32 *)triBuffer;
 	          
 	          TRI_END;
          }
        }
        else /* Didn't draw, nuke history buffer */
        {
          //glr_debug_printf("c: %4d %4d %4d\n",v1_idx,v2_idx,v3_idx);
          history[0] = history[1] = 0xffffffff;
        }
      }  
      
      // Just a quick hack... not really doing a triangle here, I just want
      // to use the existing macros.
      if(((FxU32)buff - (FxU32)triBuffer) > 0) {
        vector unsigned long *src = triBuffer, tmp;

        // Pad buffer to 16 bytes.
        while(((FxU32)buff & 0xf) != 0) {
          *buff++ = 0;
        }
        
        CHECK_FOR_ROOM((FxU32)buff - (FxU32)triBuffer + 16);
        TRI_STRIP_BEGIN_NOHEADER_G4(kSetupStrip, 3, vSize);
        while((FxU32)tPackPtr & 15) {
          *tPackPtr++ = 0;
        }
        
        // Transfer data 128 bits at a time.
        while((FxU32)src < (FxU32)buff) {
          tmp = *src++;
          // Do byte swap 
          tmp = vec_perm(tmp,tmp,byteSwap);
          *((vector unsigned long *)tPackPtr)++ = tmp;
        }
        TRI_END;
      }
      //glr_debug_printf("\n");
    }    
	PARAM_END;
exit:
	PROFILE_EXIT();
}
#pragma profile reset


#if 0
/* -----> cmd fifo + fan detect version (doesn't really work yet) */
void __glrVARenderTextureTriangles(
	GLDContext			inContext,
	void *				inData,
	GLint				inFirst,
	GLsizei				inCount,
	const GLuint *		inIndices)

{
	GLint               i, theOffset;
	GLDVertexArrayTex1Element *theVtx_base = (GLDVertexArrayTex1Element *) inData;
	GLDVertexArrayTex1Element *theVtx0, *theVtx1, *theVtx2;
	GLDVertexArrayTex1Element *theLastVtx0, *theLastVtx1, *theLastVtx2;
    FxU32               pktype, vSize;
    FxFloat             s_scale, t_scale; 
    FxFloat             s0, t0;  
/*
FxU32 theFanCount = 0;
FxU32 theStripCount = 0;
*/
	glrVASetupIndices( inIndices );
	
	DEBUG_ENTRY( __glrVARenderTextureTriangles );
	DEBUG_FASTPATH_ENTRY( __glrVARenderTextureTriangles );

	if( inCount < 3 ) return;

	PARAM_SETUP(SST_SETUP_RGB|SST_SETUP_A|SST_SETUP_Z|SST_SETUP_Wfbi|SST_SETUP_ST0,7);

    s_scale = inContext->hw_texture[0]->s_scale;
    t_scale = inContext->hw_texture[0]->t_scale;

	ALIGN_FIFO;

//	theTriType = kSetupFan;
	theLastVtx1 = 0;
	
	inCount -= 2;
	for(i = 0; i < inCount; i += 3)
	{

	  glrVAGetNextVertexPtr(theVtx0);
	  glrVAGetNextVertexPtr(theVtx1);
	  glrVAGetNextVertexPtr(theVtx2);

      /* look for strips and fans */
	  if ( theVtx0 == theLastVtx0 && theVtx1 == theLastVtx2 )
	  {
        pktype = SSTCP_PKT3_DDDDDD;
//	      theTriType == kSetupStrip;
//	      theFanCount++;
	    }
	    else
	    {
        pktype = SSTCP_PKT3_BDDDDD;
	  }
/*
	  if ( theVtx0 == theLastVtx1 && theVtx1 == theLastVtx2 )
	  {
        pktype = SSTCP_PKT3_BDDDDD;
	      theStripCount++;
  	  }
*/  	  
	  if ( pktype == SSTCP_PKT3_BDDDDD )
	  {
	    SET_EXPECTED_SIZE((vSize * 3) + sizeof(FxU32));
	    
	    TRI_STRIP_BEGIN_NOHEADER(kSetupFan, 3, vSize);

        TRI_SETPAIR_IF(packetVal | pktype, (theVtx0)->vertex.x);  
        TRI_SETPAIR_FI((theVtx0)->vertex.y, (theVtx0)->color);	
        s0 = (theVtx0)->sw1 * s_scale;
        TRI_SETPAIR_FF((theVtx0)->vertex.z, (theVtx0)->vertex.w);	
        t0 = (theVtx0)->tw1 * t_scale;
        TRI_SETPAIR_FF(s0,t0);

        TRI_SETPAIR_FF((theVtx1)->vertex.x, (theVtx1)->vertex.y);  
        TRI_SETPAIR_IF((theVtx1)->color, (theVtx1)->vertex.z);	
        s0 = (theVtx1)->sw1 * s_scale;
        TRI_SETPAIR_FF((theVtx1)->vertex.w, s0);	
        t0 = (theVtx1)->tw1 * t_scale;

        TRI_SETPAIR_FF(t0, (theVtx2)->vertex.x);  
        TRI_SETPAIR_FI((theVtx2)->vertex.y, (theVtx2)->color);	
        s0 = (theVtx2)->sw1 * s_scale;
        TRI_SETPAIR_FF((theVtx2)->vertex.z, (theVtx2)->vertex.w);	
        t0 = (theVtx2)->tw1 * t_scale;
        TRI_SETPAIR_FF(s0,t0);

 	    TRI_END;
	    CHECK_SIZE;
	  }
	  else
	  {
	    SET_EXPECTED_SIZE((vSize * 1) + sizeof(FxU32));
	    
	    TRI_STRIP_BEGIN_NOHEADER(kSetupStrip, 1, vSize);

        TRI_SETPAIR_IF(packetVal | pktype, (theVtx2)->vertex.x);  
        TRI_SETPAIR_FI((theVtx2)->vertex.y, (theVtx2)->color);	
        s0 = (theVtx2)->sw1 * s_scale;
        TRI_SETPAIR_FF((theVtx2)->vertex.z, (theVtx2)->vertex.w);	
        t0 = (theVtx2)->tw1 * t_scale;
        TRI_SETPAIR_FF(s0,t0);

 	    TRI_END;
	    CHECK_SIZE;
	  }

	  theLastVtx0 = theVtx0;
	  theLastVtx1 = theVtx1;
	  theLastVtx2 = theVtx2;
	}

/*	
	if ( theFanCount || theStripCount ) {
	DEBUG_VERBOSE( glrVARenderTextureTriangles, "@@@@@ Tri fans = %d, strips = %d\n", theFanCount , theStripCount);
	}
	else
	{
	DEBUG_VERBOSE( glrVARenderTextureTriangles, "***** no savings\n" );
	}
*/

	PARAM_END;
}
#endif

/*
________________________________________________________________________________________

      glrVARenderMultiTextureTriangles
________________________________________________________________________________________


GLD_VA_TYPE_V4F_C4UB_T2F_T2F_OWF
GLDVertexArrayTexElement

*/
#if 0
void _glrVARenderMultiTextureTriangles(
	GLDContext			inContext,
	void *				inData,
	GLint				inFirst,
	GLsizei				inCount,
	const GLuint *		inIndices)

{
	GrVertex			v0, v1, v2;
	GLint               i, theOffset;
	GLDVertexArrayTexElement * theVtx, *theVtx_base = (GLDVertexArrayTexElement *) inData;

	glrVASetupIndices( inIndices );
	
	DEBUG_SLOWPATH_ENTRY( _glrVARenderMultiTextureTriangles );

	inCount -= 2;
	for(i = 0; i < inCount; i += 3)
	{
		glrVAGetNextVertexPtr(theVtx);
		glrLoadVertexFromArray_SmoothMultiTexture( inContext, theVtx, v0 );

		glrVAGetNextVertexPtr(theVtx);
		glrLoadVertexFromArray_SmoothMultiTexture( inContext, theVtx, v1 );

		glrVAGetNextVertexPtr(theVtx);
		glrLoadVertexFromArray_SmoothMultiTexture( inContext, theVtx, v2 );

		grDrawTriangle(&v0, &v1, &v2);
	}

}
#endif

/* -----> cmd fifo version */
void glrVARenderMultiTextureTrianglesG3(
	GLDContext			inContext,
	void *				inData,
	GLint				inFirst,
	GLsizei				inCount,
	const GLuint *		inIndices)

{
	GLint               i;
	GLint noCull;
	GLDVertexArrayTexElement *theVtx_base = (GLDVertexArrayTexElement *) inData;
    FxU32               pktype, vSize;
    FxFloat             s_scale0, t_scale0; 
    FxFloat             s0, t0;  
    FxFloat             s_scale1, t_scale1; 
    FxFloat             s1, t1;  

	//glrVASetupIndices( inIndices );
	
	DEBUG_FASTPATH_ENTRY( glrVARenderTextureTrianglesG3 );

	if( inCount < 3 ) return;

	PARAM_SETUP(SST_SETUP_RGB|SST_SETUP_A|SST_SETUP_Z|SST_SETUP_Wfbi|SST_SETUP_ST0|SST_SETUP_ST1,9);

  pktype = SSTCP_PKT3_BDDDDD;
  s_scale0 = inContext->hw_texture[0]->s_scale;
  t_scale0 = inContext->hw_texture[0]->t_scale;
  s_scale1 = inContext->hw_texture[1]->s_scale;
  t_scale1 = inContext->hw_texture[1]->t_scale;

	ALIGN_FIFO;

  noCull = CULLING_DISABLED;
  

	{

      GLuint *vArray = (GLuint *)inIndices, v1_idx, v2_idx, v3_idx;
      GLDVertexArrayTexElement *v1, *v2, *v3;
      GLuint history[2], replace;

      replace = 0;
	    inCount -= 2;      
	    history[0] = 0xffffffff;
	    history[1] = 0xffffffff;

      for(i = 0; i  < inCount; i += 3) 
      {
        v1_idx = *vArray++ - inFirst;
        v2_idx = *vArray++ - inFirst;
        v3_idx = *vArray++ - inFirst;
        v1 = &theVtx_base[v1_idx];
        v2 = &theVtx_base[v2_idx];
        v3 = &theVtx_base[v3_idx];

        if(noCull || cullTri(&v1->vertex.x, &v2->vertex.x, &v3->vertex.x, signMode))
        {  
          if(history[0] == v1_idx && history[1] == v2_idx && 1)
          {
	        SET_EXPECTED_SIZE((vSize * 1) + sizeof(FxU32));
	        TRI_STRIP_BEGIN_NOHEADER(kSetupStrip, 1, vSize);
	           
	        TRI_SETPAIR_IF(packetVal | SSTCP_PKT3_DDDDDD, v3->vertex.x);  
	        TRI_SETPAIR_FI(v3->vertex.y, v3->color);	
	        s0 = v3->sw2 * s_scale0;
	        TRI_SETPAIR_FF(v3->vertex.z, v3->vertex.w);	
	        t0 = v3->tw2 * t_scale0;
	        TRI_SETPAIR_FF(s0,t0);
	        s1 = v3->sw1 * s_scale1;
	        t1 = v3->tw1 * t_scale1;
	        TRI_SETPAIR_FF(s1,t1);

	        TRI_END;
	        CHECK_SIZE;  
  
	        *(FxU32 *)((FxU32)history + replace) = v3_idx;            
	        replace = 4 - replace;
          }
          else
          {
	        SET_EXPECTED_SIZE((vSize * 3) + sizeof(FxU32));
	        TRI_STRIP_BEGIN_NOHEADER(kSetupStrip, 3, vSize);
           
	        TRI_SETPAIR_IF(packetVal | pktype, v1->vertex.x);
	        TRI_SETPAIR_FI(v1->vertex.y, v1->color);           
	        s0 = v1->sw2 * s_scale0;
	        TRI_SETPAIR_FF(v1->vertex.z, v1->vertex.w);
	        t0 = v1->tw2 * t_scale0;
	        TRI_SETPAIR_FF(s0,t0);
	        s1 = v1->sw1 * s_scale1;
	        t1 = v1->tw1 * t_scale1;
	        TRI_SETPAIR_FF(s1,t1);

	        TRI_SETPAIR_FF(v2->vertex.x, v2->vertex.y);  
	        TRI_SETPAIR_IF(v2->color, v2->vertex.z);	
	        s0 = v2->sw2 * s_scale0;
	        TRI_SETPAIR_FF(v2->vertex.w, s0);	
	        t0 = v2->tw2 * t_scale0;
	        s1 = v2->sw1 * s_scale1;
	        TRI_SETPAIR_FF(t0, s1);            
	        t1 = v2->tw1 * t_scale1;
          
	        TRI_SETPAIR_FF(t1, v3->vertex.x);  
	        TRI_SETPAIR_FI(v3->vertex.y, v3->color);	
	        s0 = v3->sw2 * s_scale0;
	        TRI_SETPAIR_FF(v3->vertex.z, v3->vertex.w);	
	        t0 = v3->tw2 * t_scale0;
	        TRI_SETPAIR_FF(s0,t0);
	        s1 = v3->sw1 * s_scale1;
	        t1 = v3->tw1 * t_scale1;
	        TRI_SETPAIR_FF(s1,t1);

	        TRI_END;
	        CHECK_SIZE;  
            history[0] = v3_idx;
            history[1] = v2_idx;
            replace = 4;
          }
        }
        else /* Didn't draw, nuke history buffer */
        {
          //glr_debug_printf("c: %4d %4d %4d\n",v1_idx,v2_idx,v3_idx);
          history[0] = history[1] = 0xffffffff;
        }
      }
    }

#if 0  
	inCount -= 2;
	for(i = 0; i < inCount; i += 3)
	{

	  SET_EXPECTED_SIZE((vSize * 3) + sizeof(FxU32));
	  TRI_STRIP_BEGIN_NOHEADER(kSetupStrip, 3, vSize);
	  
	  glrVAGetNextVertexPtr(theVtx);

      TRI_SETPAIR_IF(packetVal | pktype, (theVtx)->vertex.x);  
      TRI_SETPAIR_FI((theVtx)->vertex.y, (theVtx)->color);	
      s0 = (theVtx)->sw1 * s_scale;
      TRI_SETPAIR_FF((theVtx)->vertex.z, (theVtx)->vertex.w);	
      t0 = (theVtx)->tw1 * t_scale;
      TRI_SETPAIR_FF(s0,t0);

	  glrVAGetNextVertexPtr(theVtx);

      TRI_SETPAIR_FF((theVtx)->vertex.x, (theVtx)->vertex.y);  
      TRI_SETPAIR_IF((theVtx)->color, (theVtx)->vertex.z);	
      s0 = (theVtx)->sw1 * s_scale;
      TRI_SETPAIR_FF((theVtx)->vertex.w, s0);	
      t0 = (theVtx)->tw1 * t_scale;

	  glrVAGetNextVertexPtr(theVtx);

      TRI_SETPAIR_FF(t0, (theVtx)->vertex.x);  
      TRI_SETPAIR_FI((theVtx)->vertex.y, (theVtx)->color);	
      s0 = (theVtx)->sw1 * s_scale;
      TRI_SETPAIR_FF((theVtx)->vertex.z, (theVtx)->vertex.w);	
      t0 = (theVtx)->tw1 * t_scale;
      TRI_SETPAIR_FF(s0,t0);

 	  TRI_END;
	  CHECK_SIZE;  
	}
#endif
	PARAM_END;
}




/* -----> cmd fifo version */
#pragma profile off
// compiler bug: profiler insertion doesn't work in functions with Altivec code
void glrVARenderMultiTextureTrianglesG4(
	GLDContext			inContext,
	void *				inData,
	GLint				inFirst,
	GLsizei				inCount,
	const GLuint *		inIndices)

{
	GLint i;
	GLint noCull;
	GLDVertexArrayTexElement *theVtx_base = (GLDVertexArrayTexElement *) inData;
  FxU32               pktype, vSize;
  FxFloat             s_scale0, t_scale0; 
  FxFloat             s_scale1, t_scale1; 

  static vector unsigned long triBuffer[32];
  unsigned long *buff;
  vector unsigned char byteSwap = { 3,  2,  1,  0, 
                                    7,  6,  5,  4, 
                                   11, 10,  9,  8,
                                   15, 14, 13, 12 };
                                     
	//glrVASetupIndices( inIndices );
	
	PROFILE_ENTRY("glrVARenderMultiTextureTrianglesG4");
	
	DEBUG_FASTPATH_ENTRY( glrVARenderTextureTrianglesG4 );

	if( inCount < 3 ) goto exit;

	PARAM_SETUP(SST_SETUP_RGB|SST_SETUP_A|SST_SETUP_Z|SST_SETUP_Wfbi|SST_SETUP_ST0|SST_SETUP_ST1,9);

  pktype = SSTCP_PKT3_BDDDDD;
  s_scale0 = inContext->hw_texture[0]->s_scale;
  t_scale0 = inContext->hw_texture[0]->t_scale;
  s_scale1 = inContext->hw_texture[1]->s_scale;
  t_scale1 = inContext->hw_texture[1]->t_scale;

	//ALIGN_FIFO_16;

  noCull = CULLING_DISABLED;
  
	{

      GLuint *vArray = (GLuint *)inIndices, v1_idx, v2_idx, v3_idx;
      GLDVertexArrayTexElement *v1, *v2, *v3;
      GLuint history[2], replace;
      FxU32 t0, t1, t2, t3, t4;
      FxFloat f0, f1, f2, f3;
      buff = (FxU32 *)triBuffer;
      
      replace = 0;
	    inCount -= 2;      
	    history[0] = 0xffffffff;
	    history[1] = 0xffffffff;
	    
      for(i = 0; i  < inCount; i += 3) 
      {
        v1_idx = *vArray++ - inFirst;
        v2_idx = *vArray++ - inFirst;
        v3_idx = *vArray++ - inFirst;
        v1 = &theVtx_base[v1_idx];
        v2 = &theVtx_base[v2_idx];
        v3 = &theVtx_base[v3_idx];
        
        if(noCull || cullTri(&v1->vertex.x, &v2->vertex.x, &v3->vertex.x, signMode))
        { 
          /* Okay, we know we want to draw this triangle, so see if we can continue a strip. */  
          /* I don't think I have to do any futzing around with the triangle culling bits since */
          /* In theory I should always be in sync with the hardware */        
          if(!((history[0] ^ v1_idx) | (history[1] ^ v2_idx)))
          {
	          FxU32 packetVal = (((kSetupStrip) << SSTCP_PKT3_SMODE_SHIFT) |   /* [27:22] */ \
	                             ((1) << SSTCP_PKT3_NUMVERTEX_SHIFT) | /* [9:6] */ \
	                            __cullStripHdr);
	          *buff++ = packetVal | SSTCP_PKT3_DDDDDD;
	          #if 1
	          t0 = *(FxU32 *)&v3->vertex.x;
	          t1 = *(FxU32 *)&v3->vertex.y;
	          t2 = *(FxU32 *)&v3->color;
	          t3 = *(FxU32 *)&v3->vertex.z;
	          t4 = *(FxU32 *)&v3->vertex.w;
	          *buff++ = t0;
	          *buff++ = t1;
	          *buff++ = t2;
	          *buff++ = t3;
	          *buff++ = t4;	     
	          f0 = v3->sw2;
	          f1 = v3->tw2;
	          f2 = v3->sw1;
	          f3 = v3->tw1;
	          f0 *= s_scale0;
	          f1 *= t_scale0;
	          f2 *= s_scale1;	               
	          f3 *= t_scale1;	               
	          *((FxFloat *)buff)++ = f0;
	          *((FxFloat *)buff)++ = f1;
	          *((FxFloat *)buff)++ = f2;
	          *((FxFloat *)buff)++ = f3;
	          #else
	          *buff++ = *(FxU32 *)&v3->vertex.x;
	          *buff++ = *(FxU32 *)&v3->vertex.y;
	          *buff++ = *(FxU32 *)&v3->color;
	          *buff++ = *(FxU32 *)&v3->vertex.z;
	          *buff++ = *(FxU32 *)&v3->vertex.w;
	          *((FxFloat *)buff)++ = v3->sw2 * s_scale0;
	          *((FxFloat *)buff)++ = v3->tw2 * t_scale0;
	          *((FxFloat *)buff)++ = v3->sw1 * s_scale1;
	          *((FxFloat *)buff)++ = v3->tw1 * t_scale1;
	          #endif

            /* Update history buffer */
            *(FxU32 *)((FxU32)history + replace) = v3_idx;            
            replace = 4 - replace;
            //glr_debug_printf("s");            
          }
          else
          { 
	          FxU32 packetVal = (((kSetupStrip) << SSTCP_PKT3_SMODE_SHIFT) |   /* [27:22] */ \
	                             ((3) << SSTCP_PKT3_NUMVERTEX_SHIFT) | /* [9:6] */ \
	                            __cullStripHdr);
	          *buff++ = packetVal | SSTCP_PKT3_BDDDDD;
	          #if 1
	          t0 = *(FxU32 *)&v1->vertex.x;
	          t1 = *(FxU32 *)&v1->vertex.y;
	          t2 = *(FxU32 *)&v1->color;
	          t3 = *(FxU32 *)&v1->vertex.z;
	          t4 = *(FxU32 *)&v1->vertex.w;
	          *buff++ = t0;
	          *buff++ = t1;
	          *buff++ = t2;
	          *buff++ = t3;
	          *buff++ = t4;	     
	          f0 = v1->sw2;
	          f1 = v1->tw2;
	          f2 = v1->sw1;
	          f3 = v1->tw1;
	          f0 *= s_scale0;
	          f1 *= t_scale0;
	          f2 *= s_scale1;	               
	          f3 *= t_scale1;	               
	          *((FxFloat *)buff)++ = f0;
	          *((FxFloat *)buff)++ = f1;
	          *((FxFloat *)buff)++ = f2;
	          *((FxFloat *)buff)++ = f3;
	          #else
	          *buff++ = *(FxU32 *)&v1->vertex.x;
	          *buff++ = *(FxU32 *)&v1->vertex.y;
	          *buff++ = *(FxU32 *)&v1->color;
	          *buff++ = *(FxU32 *)&v1->vertex.z;
	          *buff++ = *(FxU32 *)&v1->vertex.w;
	          *((FxFloat *)buff)++ = v1->sw2 * s_scale0;
	          *((FxFloat *)buff)++ = v1->tw2 * t_scale0;
	          *((FxFloat *)buff)++ = v1->sw1 * s_scale1;
	          *((FxFloat *)buff)++ = v1->tw1 * t_scale1;
            #endif
            
	          #if 1
	          t0 = *(FxU32 *)&v2->vertex.x;
	          t1 = *(FxU32 *)&v2->vertex.y;
	          t2 = *(FxU32 *)&v2->color;
	          t3 = *(FxU32 *)&v2->vertex.z;
	          t4 = *(FxU32 *)&v2->vertex.w;
	          *buff++ = t0;
	          *buff++ = t1;
	          *buff++ = t2;
	          *buff++ = t3;
	          *buff++ = t4;	     
	          f0 = v2->sw2;
	          f1 = v2->tw2;
	          f2 = v2->sw1;
	          f3 = v2->tw1;
	          f0 *= s_scale0;
	          f1 *= t_scale0;
	          f2 *= s_scale1;	               
	          f3 *= t_scale1;	               
	          *((FxFloat *)buff)++ = f0;
	          *((FxFloat *)buff)++ = f1;
	          *((FxFloat *)buff)++ = f2;
	          *((FxFloat *)buff)++ = f3;
	          #else
	          *buff++ = *(FxU32 *)&v2->vertex.x;
	          *buff++ = *(FxU32 *)&v2->vertex.y;
	          *buff++ = *(FxU32 *)&v2->color;
	          *buff++ = *(FxU32 *)&v2->vertex.z;
	          *buff++ = *(FxU32 *)&v2->vertex.w;
	          *((FxFloat *)buff)++ = v2->sw2 * s_scale0;
	          *((FxFloat *)buff)++ = v2->tw2 * t_scale0;
	          *((FxFloat *)buff)++ = v2->sw1 * s_scale1;
	          *((FxFloat *)buff)++ = v2->tw1 * t_scale1;
            #endif
            
	          #if 1
	          t0 = *(FxU32 *)&v3->vertex.x;
	          t1 = *(FxU32 *)&v3->vertex.y;
	          t2 = *(FxU32 *)&v3->color;
	          t3 = *(FxU32 *)&v3->vertex.z;
	          t4 = *(FxU32 *)&v3->vertex.w;
	          *buff++ = t0;
	          *buff++ = t1;
	          *buff++ = t2;
	          *buff++ = t3;
	          *buff++ = t4;	     
	          f0 = v3->sw2;
	          f1 = v3->tw2;
	          f2 = v3->sw1;
	          f3 = v3->tw1;
	          f0 *= s_scale0;
	          f1 *= t_scale0;
	          f2 *= s_scale1;	               
	          f3 *= t_scale1;	               
	          *((FxFloat *)buff)++ = f0;
	          *((FxFloat *)buff)++ = f1;
	          *((FxFloat *)buff)++ = f2;
	          *((FxFloat *)buff)++ = f3;
	          #else
	          *buff++ = *(FxU32 *)&v3->vertex.x;
	          *buff++ = *(FxU32 *)&v3->vertex.y;
	          *buff++ = *(FxU32 *)&v3->color;
	          *buff++ = *(FxU32 *)&v3->vertex.z;
	          *buff++ = *(FxU32 *)&v3->vertex.w;
	          *((FxFloat *)buff)++ = v3->sw2 * s_scale0;
	          *((FxFloat *)buff)++ = v3->tw2 * t_scale0;
	          *((FxFloat *)buff)++ = v3->sw1 * s_scale1;
	          *((FxFloat *)buff)++ = v3->tw1 * t_scale1;
            #endif
            
            /* Started a new triangle.  Reset history buffer. */
            history[0] = v3_idx;
            history[1] = v2_idx;
            replace = 4;
            //glr_debug_printf("t");            
          }
          if(((FxU32)buff - (FxU32)triBuffer) > 16*STORE_THRESHOLD) {
            vector unsigned long *src = triBuffer, tmp;

            // Pad buffer to 16 bytes.
            while(((FxU32)buff & 0xf) != 0) {
              *buff++ = 0;
            }
            
            CHECK_FOR_ROOM((FxU32)buff - (FxU32)triBuffer + 16);
            TRI_STRIP_BEGIN_NOHEADER_G4(kSetupStrip, 3, vSize);
            while((FxU32)tPackPtr & 15) {
              *tPackPtr++ = 0;
            }
                      
            // Transfer data 128 bits at a time.
            while((FxU32)src < (FxU32)buff) {
              tmp = *src++;
              // Do byte swap 
              tmp = vec_perm(tmp,tmp,byteSwap);
              *((vector unsigned long *)tPackPtr)++ = tmp;
            }
            // Reset buffer pointer
            buff = (FxU32 *)triBuffer;
 	          
 	          TRI_END;
          }
        }
        else /* Didn't draw, nuke history buffer */
        {
          //glr_debug_printf("c: %4d %4d %4d\n",v1_idx,v2_idx,v3_idx);
          history[0] = history[1] = 0xffffffff;
        }
      }  
      
      // Just a quick hack... not really doing a triangle here, I just want
      // to use the existing macros.
      if(((FxU32)buff - (FxU32)triBuffer) > 0) {
        vector unsigned long *src = triBuffer, tmp;

        // Pad buffer to 16 bytes.
        while(((FxU32)buff & 0xf) != 0) {
          *buff++ = 0;
        }
        
        CHECK_FOR_ROOM((FxU32)buff - (FxU32)triBuffer + 16);
        TRI_STRIP_BEGIN_NOHEADER_G4(kSetupStrip, 3, vSize);
        while((FxU32)tPackPtr & 15) {
          *tPackPtr++ = 0;
        }
        
        // Transfer data 128 bits at a time.
        while((FxU32)src < (FxU32)buff) {
          tmp = *src++;
          // Do byte swap 
          tmp = vec_perm(tmp,tmp,byteSwap);
          *((vector unsigned long *)tPackPtr)++ = tmp;
        }
        TRI_END;
      }
      //glr_debug_printf("\n");            
      
    }    

	PARAM_END;
	
exit:
	PROFILE_EXIT();
}
#pragma profile reset

