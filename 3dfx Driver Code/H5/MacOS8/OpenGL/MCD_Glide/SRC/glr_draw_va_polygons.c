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




#pragma mark -
/*
________________________________________________________________________________________

      glrVARenderSmoothPolygons
________________________________________________________________________________________


GLD_VA_TYPE_V4F_C4UB
GLDVertexArrayGrElement

*/


#if 0
void _glrVARenderSmoothPolygonsG3(
	GLDContext			inContext,
	void *				inData,
	GLint				inFirst,
	GLsizei				inCount,
	const GLuint *		inIndices)
{
	GrVertex			v0, v1, v2;
	GLint               i, theOffset;
	GLDVertexArrayGrElement *theVtx, *theVtx_base = (GLDVertexArrayGrElement *) inData;
        
	glrVASetupIndices( inIndices );
	
	DEBUG_SLOWPATH_ENTRY( _glrVARenderSmoothPolygonsG3 );


	if( inCount < 3 ) return;
	
	glrVAGetNextVertexPtr(theVtx);
	glrLoadVertexFromArray_Smooth( inContext, theVtx, v0 );

	glrVAGetNextVertexPtr(theVtx);
	glrLoadVertexFromArray_Smooth( inContext, theVtx, v1 );

	for(i = 2; i < inCount; i++)
	{
		glrVAGetNextVertexPtr(theVtx);
		glrLoadVertexFromArray_Smooth( inContext, theVtx, v2 );
		
		grDrawTriangle(&v0, &v1, &v2);
		
		v1 = v2;
	}

}
#endif

/* -----> cmd fifo version */	
void glrVARenderSmoothPolygonsG3(
	GLDContext			inContext,
	void *				inData,
	GLint				inFirst,
	GLsizei				inCount,
	const GLuint *		inIndices)
{
	GLint               theOffset;
	GLDVertexArrayGrElement *theVtx, *theVtx_base = (GLDVertexArrayGrElement *) inData;
    FxU32               pktype, vSize;

	glrVASetupIndices( inIndices );

	DEBUG_FASTPATH_ENTRY( glrVARenderSmoothPolygonsG3 );

	if( inCount < 3 ) return;
	
	PARAM_SETUP(SST_SETUP_RGB|SST_SETUP_A|SST_SETUP_Z|SST_SETUP_Wfbi,5);

    pktype = SSTCP_PKT3_BDDDDD;

	ALIGN_FIFO;

    
    while(inCount-- > 0) {
	  
	  SET_EXPECTED_SIZE(vSize + sizeof(FxU32));
	  TRI_STRIP_BEGIN_NOHEADER(kSetupFan, 1, vSize);
	  
	  glrVAGetNextVertexPtr(theVtx);

#if STORE_64

      TRI_SETPAIR_IF(packetVal | pktype, (theVtx)->vertex.x);  
      TRI_SETPAIR_FI((theVtx)->vertex.y, (theVtx)->color);	
      TRI_SETPAIR_FF((theVtx)->vertex.z, (theVtx)->vertex.w);	

 	  TRI_END;
 	  
#else

      TRI_SET(packetVal | pktype);
      TRI_SETF((theVtx)->vertex.x);
      TRI_SETF((theVtx)->vertex.y);
      TRI_SET((theVtx)->color);
      TRI_SETF((theVtx)->vertex.z);
      TRI_SETF((theVtx)->vertex.w);

      TRI_END;
 
#endif
      CHECK_SIZE;
      
	  pktype = SSTCP_PKT3_DDDDDD;
	}	
	
	PARAM_END;
}


/* -----> cmd fifo version */
#pragma profile off
// compiler bug: profiler insertion doesn't work in functions with Altivec code
void glrVARenderSmoothPolygonsG4(
	GLDContext			inContext,
	void *				inData,
	GLint				inFirst,
	GLsizei				inCount,
	const GLuint *		inIndices)
{
	GLint               theOffset;
	GLDVertexArrayGrElement *theVtx, *theVtx_base = (GLDVertexArrayGrElement *) inData;
    FxU32               pktype, vSize;

  static vector unsigned long triBuffer[128];
  unsigned long *buff;
  vector unsigned char byteSwap = { 3,  2,  1,  0, 
                                    7,  6,  5,  4, 
                                   11, 10,  9,  8,
                                   15, 14, 13, 12 };


	glrVASetupIndices( inIndices );

	PROFILE_ENTRY("glrVARenderSmoothPolygonsG4");

	DEBUG_FASTPATH_ENTRY( glrVARenderSmoothPolygonsG4 );

	if( inCount < 3 ) goto exit;
	
	PARAM_SETUP(SST_SETUP_RGB|SST_SETUP_A|SST_SETUP_Z|SST_SETUP_Wfbi,5);

    pktype = SSTCP_PKT3_BDDDDD;

	ALIGN_FIFO_16;

    buff = (FxU32 *)triBuffer;

    while(inCount-- > 0) {
	  
	  FxU32 packetVal = (((kSetupFan) << SSTCP_PKT3_SMODE_SHIFT) |   /* [27:22] */ \
                           ((1) << SSTCP_PKT3_NUMVERTEX_SHIFT) | /* [9:6] */ \
                           __cullStripHdr);

	  glrVAGetNextVertexPtr(theVtx);

      *buff++ = packetVal | pktype;
      *((FxFloat *)buff)++ = (theVtx)->vertex.x;
      *((FxFloat *)buff)++ = (theVtx)->vertex.y;
      *buff++ = (theVtx)->color;
      *((FxFloat *)buff)++ = (theVtx)->vertex.z;
      *((FxFloat *)buff)++ = (theVtx)->vertex.w;
      
	  pktype = SSTCP_PKT3_DDDDDD;
	  
          if(((FxU32)buff - (FxU32)triBuffer) > 16*16) {
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
	
	PARAM_END;
exit:
	PROFILE_EXIT();

}
#pragma profile reset


#pragma mark -


/*
________________________________________________________________________________________

      glrVARenderTexturePolygons
________________________________________________________________________________________


GLD_VA_TYPE_V4F_C4UB_T2F
GLDVertexArrayTex1Element

*/



#if 0
/* -----> glide version */
void _glrVARenderTexturePolygonsG3(
	GLDContext			inContext,
	void *				inData,
	GLint				inFirst,
	GLsizei				inCount,
	const GLuint *		inIndices)
{
	GrVertex			v0, v1, v2;
	GLint               i, theOffset;
	GLDVertexArrayTex1Element *theVtx, *theVtx_base = (GLDVertexArrayTex1Element *) inData;
        
	glrVASetupIndices( inIndices );
	
	DEBUG_SLOWPATH_ENTRY( _glrVARenderTexturePolygonsG3 );


	if( inCount < 3 ) return;
	
	glrVAGetNextVertexPtr(theVtx);
	glrLoadVertexFromArray_SmoothTexture( inContext, theVtx, v0 );

	glrVAGetNextVertexPtr(theVtx);
	glrLoadVertexFromArray_SmoothTexture( inContext, theVtx, v1 );

	for(i = 2; i < inCount; i++)
	{
		glrVAGetNextVertexPtr(theVtx);
		glrLoadVertexFromArray_SmoothTexture( inContext, theVtx, v2 );
		
		grDrawTriangle(&v0, &v1, &v2);
		
		v1 = v2;

	}

}
#endif

/* -----> cmd fifo version */
void glrVARenderTexturePolygonsG3(
	GLDContext			inContext,
	void *				inData,
	GLint				inFirst,
	GLsizei				inCount,
	const GLuint *		inIndices)
{
	GLint               theOffset;
	GLDVertexArrayTex1Element *theVtx, *theVtx_base = (GLDVertexArrayTex1Element *) inData;
    FxU32               pktype, vSize;
    FxFloat             s_scale, t_scale; 
    FxFloat             s0, t0;  

	glrVASetupIndices( inIndices );

	DEBUG_FASTPATH_ENTRY( glrVARenderTexturePolygonsG3 );

	if( inCount < 3 ) return;
	
	PARAM_SETUP(SST_SETUP_RGB|SST_SETUP_A|SST_SETUP_Z|SST_SETUP_Wfbi|SST_SETUP_ST0,7);

    pktype = SSTCP_PKT3_BDDDDD;
    s_scale = inContext->hw_texture[0]->s_scale;
    t_scale = inContext->hw_texture[0]->t_scale;

	ALIGN_FIFO;

    
    while(inCount-- > 0) {
	  
	  SET_EXPECTED_SIZE(vSize + sizeof(FxU32));
	  TRI_STRIP_BEGIN_NOHEADER(kSetupFan, 1, vSize);
	  
	  glrVAGetNextVertexPtr(theVtx);

#if STORE_64

      TRI_SETPAIR_IF(packetVal | pktype, (theVtx)->vertex.x);  
      TRI_SETPAIR_FI((theVtx)->vertex.y, (theVtx)->color);	
      s0 = (theVtx)->sw1 * s_scale;
      TRI_SETPAIR_FF((theVtx)->vertex.z, (theVtx)->vertex.w);	
      t0 = (theVtx)->tw1 * t_scale;
      TRI_SETPAIR_FF(s0,t0);

 	  TRI_END;
 	  
#else

      TRI_SET(packetVal | pktype);
      TRI_SETF((theVtx)->vertex.x);
      TRI_SETF((theVtx)->vertex.y);
      TRI_SET((theVtx)->color);
      TRI_SETF((theVtx)->vertex.z);
      TRI_SETF((theVtx)->vertex.w);
      TRI_SETF((theVtx)->sw1 * s_scale);
      TRI_SETF((theVtx)->tw1 * t_scale);

      TRI_END;
 
#endif
      CHECK_SIZE;
      
	  pktype = SSTCP_PKT3_DDDDDD;
	}	
	
	PARAM_END;
}


/* -----> cmd fifo version */
#pragma profile off
// compiler bug: profiler insertion doesn't work in functions with Altivec code
void glrVARenderTexturePolygonsG4(
	GLDContext			inContext,
	void *				inData,
	GLint				inFirst,
	GLsizei				inCount,
	const GLuint *		inIndices)
{
	GLint               theOffset;
	GLDVertexArrayTex1Element *theVtx, *theVtx_base = (GLDVertexArrayTex1Element *) inData;
    FxU32               pktype, vSize;
    FxFloat             s_scale, t_scale; 

  static vector unsigned long triBuffer[128];
  unsigned long *buff;
  vector unsigned char byteSwap = { 3,  2,  1,  0, 
                                    7,  6,  5,  4, 
                                   11, 10,  9,  8,
                                   15, 14, 13, 12 };

	
	glrVASetupIndices( inIndices );

	PROFILE_ENTRY("glrVARenderTexturePolygonsG4");

	DEBUG_FASTPATH_ENTRY( glrVARenderTexturePolygonsG4 );

	if( inCount < 3 ) goto exit;
	
	PARAM_SETUP(SST_SETUP_RGB|SST_SETUP_A|SST_SETUP_Z|SST_SETUP_Wfbi|SST_SETUP_ST0,7);

    pktype = SSTCP_PKT3_BDDDDD;
    s_scale = inContext->hw_texture[0]->s_scale;
    t_scale = inContext->hw_texture[0]->t_scale;

	ALIGN_FIFO_16;

    buff = (FxU32 *)triBuffer;

    while(inCount-- > 0) {
	  
	  FxU32 packetVal = (((kSetupFan) << SSTCP_PKT3_SMODE_SHIFT) |   /* [27:22] */ \
                           ((1) << SSTCP_PKT3_NUMVERTEX_SHIFT) | /* [9:6] */ \
                           __cullStripHdr);

	  glrVAGetNextVertexPtr(theVtx);

      *buff++ = packetVal | pktype;
      *((FxFloat *)buff)++ = (theVtx)->vertex.x;
      *((FxFloat *)buff)++ = (theVtx)->vertex.y;
      *buff++ = (theVtx)->color;
      *((FxFloat *)buff)++ = (theVtx)->vertex.z;
      *((FxFloat *)buff)++ = (theVtx)->vertex.w;
      *((FxFloat *)buff)++ = (theVtx)->sw1 * s_scale;
      *((FxFloat *)buff)++ = (theVtx)->tw1 * t_scale;
      
	  pktype = SSTCP_PKT3_DDDDDD;
	  
          if(((FxU32)buff - (FxU32)triBuffer) > 16*16) {
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
	
	PARAM_END;
exit:
	PROFILE_EXIT();
}
#pragma profile reset



/*
________________________________________________________________________________________

      glrVARenderMultiTexturePolygons
________________________________________________________________________________________


GLD_VA_TYPE_V4F_C4UB_T2F_T2F_OWF
GLDVertexArrayTexElement

*/
#if 0
void _glrVARenderMultiTexturePolygonsG3(
	GLDContext			inContext,
	void *				inData,
	GLint				inFirst,
	GLsizei				inCount,
	const GLuint *		inIndices)
{
	GLint               theOffset;
	GrVertex			v0, v1, v2;
	GLint               i;
	GLDVertexArrayTexElement *theVtx, *theVtx_base = (GLDVertexArrayTexElement *) inData;

	glrVASetupIndices( inIndices );
	
	DEBUG_SLOWPATH_ENTRY( glrVARenderMultiTexturePolygonsG3 );


	if( inCount < 3 ) return;
	
	glrVAGetNextVertexPtr(theVtx);
	glrLoadVertexFromArray_SmoothMultiTexture( inContext, theVtx, v0 );

	glrVAGetNextVertexPtr(theVtx);
	glrLoadVertexFromArray_SmoothMultiTexture( inContext, theVtx, v1 );

	for(i = 2; i < inCount; i++)
	{
		glrVAGetNextVertexPtr(theVtx);
		glrLoadVertexFromArray_SmoothMultiTexture( inContext, theVtx, v2 );
		
		grDrawTriangle(&v0, &v1, &v2);
		
		v1 = v2;

	}
}
#endif


void glrVARenderMultiTexturePolygonsG3(
	GLDContext			inContext,
	void *				inData,
	GLint				inFirst,
	GLsizei				inCount,
	const GLuint *		inIndices)
{
	GLint               theOffset;
	GLDVertexArrayTexElement *theVtx, *theVtx_base = (GLDVertexArrayTexElement *) inData;
    FxU32               pktype, vSize;
    FxFloat             s_scale0, t_scale0, s_scale1, t_scale1; 
    FxFloat             s0, t0, s1, t1;  

	glrVASetupIndices( inIndices );
	
	DEBUG_FASTPATH_ENTRY( glrVARenderTexturePolygonsG3 );

	if( inCount < 3 ) return;
	
	PARAM_SETUP(SST_SETUP_RGB|SST_SETUP_A|SST_SETUP_Z|SST_SETUP_Wfbi|SST_SETUP_ST0|SST_SETUP_ST1,9);

    pktype = SSTCP_PKT3_BDDDDD;
    s_scale0 = inContext->hw_texture[0]->s_scale;
    t_scale0 = inContext->hw_texture[0]->t_scale;
    s_scale1 = inContext->hw_texture[1]->s_scale;
    t_scale1 = inContext->hw_texture[1]->t_scale;

	ALIGN_FIFO;


    while(inCount-- > 0) {
	  
	  SET_EXPECTED_SIZE(vSize + sizeof(FxU32));
	  TRI_STRIP_BEGIN_NOHEADER(kSetupFan, 1, vSize);
	  
	  glrVAGetNextVertexPtr(theVtx);

      TRI_SETPAIR_IF(packetVal | pktype, (theVtx)->vertex.x);  
      TRI_SETPAIR_FI((theVtx)->vertex.y, (theVtx)->color);	
      TRI_SETPAIR_FF((theVtx)->vertex.z, (theVtx)->vertex.w);	
      s0 = (theVtx)->sw2 * s_scale0;
      t0 = (theVtx)->tw2 * t_scale0;
      TRI_SETPAIR_FF(s0,t0);
      s1 = (theVtx)->sw1 * s_scale1;
      t1 = (theVtx)->tw1 * t_scale1;
      TRI_SETPAIR_FF(s1,t1);

 	  TRI_END;
      CHECK_SIZE;
      
	  pktype = SSTCP_PKT3_DDDDDD;
	}	

	PARAM_END;

}


/* -----> cmd fifo version */
#pragma profile off
// compiler bug: profiler insertion doesn't work in functions with Altivec code
void glrVARenderMultiTexturePolygonsG4(
	GLDContext			inContext,
	void *				inData,
	GLint				inFirst,
	GLsizei				inCount,
	const GLuint *		inIndices)
{
	GLint               theOffset;
	GLDVertexArrayTexElement *theVtx, *theVtx_base = (GLDVertexArrayTexElement *) inData;
    FxU32               pktype, vSize;
    FxFloat             s_scale0, t_scale0; 
    FxFloat             s_scale1, t_scale1; 

  static vector unsigned long triBuffer[128];
  unsigned long *buff;
  vector unsigned char byteSwap = { 3,  2,  1,  0, 
                                    7,  6,  5,  4, 
                                   11, 10,  9,  8,
                                   15, 14, 13, 12 };


	glrVASetupIndices( inIndices );

	PROFILE_ENTRY("glrVARenderMultiTexturePolygonsG4");

	DEBUG_FASTPATH_ENTRY( glrVARenderTexturePolygonsG4 );

	if( inCount < 3 ) goto exit;
	
	PARAM_SETUP(SST_SETUP_RGB|SST_SETUP_A|SST_SETUP_Z|SST_SETUP_Wfbi|SST_SETUP_ST0|SST_SETUP_ST1,9);

    pktype = SSTCP_PKT3_BDDDDD;
    s_scale0 = inContext->hw_texture[0]->s_scale;
    t_scale0 = inContext->hw_texture[0]->t_scale;
    s_scale1 = inContext->hw_texture[1]->s_scale;
    t_scale1 = inContext->hw_texture[1]->t_scale;

	ALIGN_FIFO_16;

    buff = (FxU32 *)triBuffer;

    while(inCount-- > 0) {
	  
	  FxU32 packetVal = (((kSetupFan) << SSTCP_PKT3_SMODE_SHIFT) |   /* [27:22] */ \
                           ((1) << SSTCP_PKT3_NUMVERTEX_SHIFT) | /* [9:6] */ \
                           __cullStripHdr);

	  glrVAGetNextVertexPtr(theVtx);

      *buff++ = packetVal | pktype;
      *((FxFloat *)buff)++ = (theVtx)->vertex.x;
      *((FxFloat *)buff)++ = (theVtx)->vertex.y;
      *buff++ = (theVtx)->color;
      *((FxFloat *)buff)++ = (theVtx)->vertex.z;
      *((FxFloat *)buff)++ = (theVtx)->vertex.w;
      *((FxFloat *)buff)++ = (theVtx)->sw2 * s_scale0;
      *((FxFloat *)buff)++ = (theVtx)->tw2 * t_scale0;
      *((FxFloat *)buff)++ = (theVtx)->sw1 * s_scale1;
      *((FxFloat *)buff)++ = (theVtx)->tw1 * t_scale1;
      
	  pktype = SSTCP_PKT3_DDDDDD;
	  
          if(((FxU32)buff - (FxU32)triBuffer) > 16*16) {
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
	
	PARAM_END;
exit:
	PROFILE_EXIT();
}
#pragma profile reset
