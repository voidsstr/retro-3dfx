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
**  Description: Buffers related methods for the Apple OpenGL plugin
**
** 
**
*/


#include "glr.h"
#include "glr_drawing.h"
#include "glr_glide.h"

#define INLINE inline
//#define INLINE static




static void glrVARenderNotYetImplemented(
	GLDContext			inContext,
	void *				inData,
	GLint				inFirst,
	GLsizei				inCount,
	const GLuint *		inIndices);



#define FRGB_TO_FOG(p) (float)(*(unsigned char *)(p)) * (1.0f / 255.0f)

// inline stuff

INLINE void glrLoadVertexFromArray(
	GLenum 						inFormat, 
	GLDContext					inContext,
	void *						srcVertex,
	GrVertex * 					dstVertex)
{
	switch(inFormat){
	case GLD_VA_TYPE_V4F_C4UB_T2F_T2F_OWF:	// MultiTexture
		dstVertex->x = ((GLDVertexArrayTexElement*)srcVertex)->vertex.x;
		dstVertex->y = ((GLDVertexArrayTexElement*)srcVertex)->vertex.y;
		dstVertex->ooz = ((GLDVertexArrayTexElement*)srcVertex)->vertex.z;
		
		dstVertex->oow = ((GLDVertexArrayTexElement*)srcVertex)->vertex.w;
		
		dstVertex->pargb = ((GLDVertexArrayTexElement*)srcVertex)->color;
		
		dstVertex->tmuvtx[0].sow = ((GLDVertexArrayTexElement*)srcVertex)->sw2 * inContext->hw_texture[0]->s_scale;
		dstVertex->tmuvtx[0].tow = ((GLDVertexArrayTexElement*)srcVertex)->tw2 * inContext->hw_texture[0]->t_scale;
		dstVertex->tmuvtx[0].oow = ((GLDVertexArrayTexElement*)srcVertex)->oow2;
		
		dstVertex->tmuvtx[1].sow = ((GLDVertexArrayTexElement*)srcVertex)->sw1 * inContext->hw_texture[1]->s_scale;
		dstVertex->tmuvtx[1].tow = ((GLDVertexArrayTexElement*)srcVertex)->tw1 * inContext->hw_texture[1]->t_scale;
		dstVertex->tmuvtx[1].oow = dstVertex->oow;
		break;
	case GLD_VA_TYPE_V4F_C4UB_T2F:	// Texture
		dstVertex->x = ((GLDVertexArrayTex1Element*)srcVertex)->vertex.x;
		dstVertex->y = ((GLDVertexArrayTex1Element*)srcVertex)->vertex.y;
		dstVertex->ooz = ((GLDVertexArrayTex1Element*)srcVertex)->vertex.z;
		
		dstVertex->oow = ((GLDVertexArrayTex1Element*)srcVertex)->vertex.w;
		
		dstVertex->pargb = ((GLDVertexArrayTex1Element*)srcVertex)->color;
		
		dstVertex->tmuvtx[0].sow = ((GLDVertexArrayTex1Element*)srcVertex)->sw1 * inContext->hw_texture[0]->s_scale;
		dstVertex->tmuvtx[0].tow = ((GLDVertexArrayTex1Element*)srcVertex)->tw1 * inContext->hw_texture[0]->t_scale;
		dstVertex->tmuvtx[0].oow = dstVertex->oow;
		break;
	case GLD_VA_TYPE_V4F_C4UB:	// Smooth
		dstVertex->x = ((GLDVertexArrayGrElement*)srcVertex)->vertex.x;
		dstVertex->y = ((GLDVertexArrayGrElement*)srcVertex)->vertex.y;
		dstVertex->ooz = ((GLDVertexArrayGrElement*)srcVertex)->vertex.z;
		
		dstVertex->oow = ((GLDVertexArrayGrElement*)srcVertex)->vertex.w;
		
		dstVertex->pargb = ((GLDVertexArrayGrElement*)srcVertex)->color;
		break;
	case GLD_VA_TYPE_V4F_C4UB_SF4UB_T2F_T2F_OWF: // MultiTexture with fog
		dstVertex->x = ((GLDVertexArrayFogTexElement*)srcVertex)->vertex.x;
		dstVertex->y = ((GLDVertexArrayFogTexElement*)srcVertex)->vertex.y;
		dstVertex->ooz = ((GLDVertexArrayFogTexElement*)srcVertex)->vertex.z;
		
		dstVertex->oow =  FRGB_TO_FOG(&((GLDVertexArrayFogTexElement*)srcVertex)->sfrgb);
		
		dstVertex->pargb = ((GLDVertexArrayFogTexElement*)srcVertex)->color;
		
		dstVertex->tmuvtx[0].sow = ((GLDVertexArrayFogTexElement*)srcVertex)->sw2 * inContext->hw_texture[0]->s_scale;
		dstVertex->tmuvtx[0].tow = ((GLDVertexArrayFogTexElement*)srcVertex)->tw2 * inContext->hw_texture[0]->t_scale;
		dstVertex->tmuvtx[0].oow = ((GLDVertexArrayFogTexElement*)srcVertex)->oow2;
		
		dstVertex->tmuvtx[1].sow = ((GLDVertexArrayFogTexElement*)srcVertex)->sw1 * inContext->hw_texture[1]->s_scale;
		dstVertex->tmuvtx[1].tow = ((GLDVertexArrayFogTexElement*)srcVertex)->tw1 * inContext->hw_texture[1]->t_scale;
		dstVertex->tmuvtx[1].oow = ((GLDVertexArrayFogTexElement*)srcVertex)->vertex.w;
		break;
	case GLD_VA_TYPE_V4F_C4UB_SF4UB_T2F:	// single texture with fog
		dstVertex->x = ((GLDVertexArrayFogTex1Element*)srcVertex)->vertex.x;
		dstVertex->y = ((GLDVertexArrayFogTex1Element*)srcVertex)->vertex.y;
		dstVertex->ooz = ((GLDVertexArrayFogTex1Element*)srcVertex)->vertex.z;
		
		dstVertex->oow = FRGB_TO_FOG(&((GLDVertexArrayFogTex1Element*)srcVertex)->sfrgb);
		
		dstVertex->pargb = ((GLDVertexArrayFogTex1Element*)srcVertex)->color;
		
		dstVertex->tmuvtx[0].sow = ((GLDVertexArrayFogTex1Element*)srcVertex)->sw1 * inContext->hw_texture[0]->s_scale;
		dstVertex->tmuvtx[0].tow = ((GLDVertexArrayFogTex1Element*)srcVertex)->tw1 * inContext->hw_texture[0]->t_scale;
		dstVertex->tmuvtx[0].oow = ((GLDVertexArrayFogTex1Element*)srcVertex)->vertex.w;
		break;
	case GLD_VA_TYPE_V4F_C4UB_SF4UB: // Smooth with fog
		dstVertex->x = ((GLDVertexArrayGrFogElement*)srcVertex)->vertex.x;
		dstVertex->y = ((GLDVertexArrayGrFogElement*)srcVertex)->vertex.y;
		dstVertex->ooz = ((GLDVertexArrayGrFogElement*)srcVertex)->vertex.z;
		
		dstVertex->oow = FRGB_TO_FOG(&((GLDVertexArrayGrFogElement*)srcVertex)->sfrgb);
		
		dstVertex->pargb = ((GLDVertexArrayGrFogElement*)srcVertex)->color;
		break;
	}
}


#define LOAD_VERTEX_FROM_ARRAY(inFormat, inContext, srcVertex, dstVertex) \
{\
	switch(inFormat){\
	case GLD_VA_TYPE_V4F_C4UB_T2F_T2F_OWF:\
		(dstVertex)->x = ((GLDVertexArrayTexElement*)(srcVertex))->vertex.x;\
		(dstVertex)->y = ((GLDVertexArrayTexElement*)(srcVertex))->vertex.y;\
		(dstVertex)->ooz = ((GLDVertexArrayTexElement*)(srcVertex))->vertex.z;\
		(dstVertex)->oow = ((GLDVertexArrayTexElement*)(srcVertex))->vertex.w;\
		(dstVertex)->pargb = ((GLDVertexArrayTexElement*)(srcVertex))->color;\
		(dstVertex)->tmuvtx[0].sow = ((GLDVertexArrayTexElement*)(srcVertex))->sw2 * inContext->hw_texture[0]->s_scale;\
		(dstVertex)->tmuvtx[0].tow = ((GLDVertexArrayTexElement*)(srcVertex))->tw2 * inContext->hw_texture[0]->t_scale;\
		(dstVertex)->tmuvtx[0].oow = ((GLDVertexArrayTexElement*)(srcVertex))->oow2;\
		(dstVertex)->tmuvtx[1].sow = ((GLDVertexArrayTexElement*)(srcVertex))->sw1 * inContext->hw_texture[1]->s_scale;\
		(dstVertex)->tmuvtx[1].tow = ((GLDVertexArrayTexElement*)(srcVertex))->tw1 * inContext->hw_texture[1]->t_scale;\
		(dstVertex)->tmuvtx[1].oow = (dstVertex)->oow;\
		break;\
	case GLD_VA_TYPE_V4F_C4UB_T2F:	\
		(dstVertex)->x = ((GLDVertexArrayTex1Element*)(srcVertex))->vertex.x;\
		(dstVertex)->y = ((GLDVertexArrayTex1Element*)(srcVertex))->vertex.y;\
		(dstVertex)->ooz = ((GLDVertexArrayTex1Element*)(srcVertex))->vertex.z;\
		(dstVertex)->oow = ((GLDVertexArrayTex1Element*)(srcVertex))->vertex.w;\
		(dstVertex)->pargb = ((GLDVertexArrayTex1Element*)(srcVertex))->color;\
		(dstVertex)->tmuvtx[0].sow = ((GLDVertexArrayTex1Element*)(srcVertex))->sw1 * inContext->hw_texture[0]->s_scale;\
		(dstVertex)->tmuvtx[0].tow = ((GLDVertexArrayTex1Element*)(srcVertex))->tw1 * inContext->hw_texture[0]->t_scale;\
		(dstVertex)->tmuvtx[0].oow = (dstVertex)->oow;\
		break;\
	case GLD_VA_TYPE_V4F_C4UB:	\
		(dstVertex)->x = ((GLDVertexArrayGrElement*)(srcVertex))->vertex.x;\
		(dstVertex)->y = ((GLDVertexArrayGrElement*)(srcVertex))->vertex.y;\
		(dstVertex)->ooz = ((GLDVertexArrayGrElement*)(srcVertex))->vertex.z;\
		(dstVertex)->oow = ((GLDVertexArrayGrElement*)(srcVertex))->vertex.w;\
		(dstVertex)->pargb = ((GLDVertexArrayGrElement*)(srcVertex))->color;\
		break;\
	case GLD_VA_TYPE_V4F_C4UB_SF4UB_T2F_T2F_OWF: \
		(dstVertex)->x = ((GLDVertexArrayFogTexElement*)(srcVertex))->vertex.x;\
		(dstVertex)->y = ((GLDVertexArrayFogTexElement*)(srcVertex))->vertex.y;\
		(dstVertex)->ooz = ((GLDVertexArrayFogTexElement*)(srcVertex))->vertex.z;\
		(dstVertex)->oow =  FRGB_TO_FOG(&((GLDVertexArrayFogTexElement*)(srcVertex))->sfrgb);\
		(dstVertex)->pargb = ((GLDVertexArrayFogTexElement*)(srcVertex))->color;\
		(dstVertex)->tmuvtx[0].sow = ((GLDVertexArrayFogTexElement*)(srcVertex))->sw2 * inContext->hw_texture[0]->s_scale;\
		(dstVertex)->tmuvtx[0].tow = ((GLDVertexArrayFogTexElement*)(srcVertex))->tw2 * inContext->hw_texture[0]->t_scale;\
		(dstVertex)->tmuvtx[0].oow = ((GLDVertexArrayFogTexElement*)(srcVertex))->oow2;\
		(dstVertex)->tmuvtx[1].sow = ((GLDVertexArrayFogTexElement*)(srcVertex))->sw1 * inContext->hw_texture[1]->s_scale;\
		(dstVertex)->tmuvtx[1].tow = ((GLDVertexArrayFogTexElement*)(srcVertex))->tw1 * inContext->hw_texture[1]->t_scale;\
		(dstVertex)->tmuvtx[1].oow = ((GLDVertexArrayFogTexElement*)(srcVertex))->vertex.w;\
		break;\
	case GLD_VA_TYPE_V4F_C4UB_SF4UB_T2F:	\
		(dstVertex)->x = ((GLDVertexArrayFogTex1Element*)(srcVertex))->vertex.x;\
		(dstVertex)->y = ((GLDVertexArrayFogTex1Element*)(srcVertex))->vertex.y;\
		(dstVertex)->ooz = ((GLDVertexArrayFogTex1Element*)(srcVertex))->vertex.z;\
		(dstVertex)->oow = FRGB_TO_FOG(&((GLDVertexArrayFogTex1Element*)(srcVertex))->sfrgb);\
		(dstVertex)->pargb = ((GLDVertexArrayFogTex1Element*)(srcVertex))->color;\
		(dstVertex)->tmuvtx[0].sow = ((GLDVertexArrayFogTex1Element*)(srcVertex))->sw1 * inContext->hw_texture[0]->s_scale;\
		(dstVertex)->tmuvtx[0].tow = ((GLDVertexArrayFogTex1Element*)(srcVertex))->tw1 * inContext->hw_texture[0]->t_scale;\
		(dstVertex)->tmuvtx[0].oow = ((GLDVertexArrayFogTex1Element*)(srcVertex))->vertex.w;\
		break;\
	case GLD_VA_TYPE_V4F_C4UB_SF4UB: \
		(dstVertex)->x = ((GLDVertexArrayGrFogElement*)(srcVertex))->vertex.x;\
		(dstVertex)->y = ((GLDVertexArrayGrFogElement*)(srcVertex))->vertex.y;\
		(dstVertex)->ooz = ((GLDVertexArrayGrFogElement*)(srcVertex))->vertex.z;\
		(dstVertex)->oow = FRGB_TO_FOG(&((GLDVertexArrayGrFogElement*)(srcVertex))->sfrgb);\
		(dstVertex)->pargb = ((GLDVertexArrayGrFogElement*)(srcVertex))->color;\
		break;\
	}\
}

#define NEXT_VERTEX_PTR	theVtx = (char *)inData + ((*thePtr++) - inFirst) * vertexSize
	
INLINE void glrVARenderGeneric(
	GLenum				inMode,
	GLenum 				inFormat, 

	GLDContext			inContext,
	void *				inData,
	GLint				inFirst,
	GLsizei				inCount,
	const GLuint *		inIndices)
{

	GrVertex			v0, v1, v2, v3;
	GLint               i;
	void * 				theVtx;
	const GLuint *		thePtr;
	GLint				vertexSize;
		
	switch(inFormat){
	case GLD_VA_TYPE_V4F_C4UB_T2F_T2F_OWF:	// MultiTexture
		vertexSize = sizeof(GLDVertexArrayTexElement);
		break;
	case GLD_VA_TYPE_V4F_C4UB_T2F:	// Texture
		vertexSize = sizeof(GLDVertexArrayTex1Element);
		break;
	case GLD_VA_TYPE_V4F_C4UB:	// Smooth
		vertexSize = sizeof(GLDVertexArrayGrElement);
		break;
	case GLD_VA_TYPE_V4F_C4UB_SF4UB_T2F_T2F_OWF: // MultiTextureFog
		vertexSize = sizeof(GLDVertexArrayFogTexElement);
		break;
	case GLD_VA_TYPE_V4F_C4UB_SF4UB_T2F:		// TextureFog
		vertexSize = sizeof(GLDVertexArrayFogTex1Element);
		break;
	case GLD_VA_TYPE_V4F_C4UB_SF4UB:			// SmoothFog
		vertexSize = sizeof(GLDVertexArrayGrFogElement);
		break;
	}

	thePtr = inIndices;

	switch(	inMode ){
	case GL_POINTS:
		break;
	case GL_LINES:
		break;
	case GL_LINE_LOOP:
		break;
	case GL_LINE_STRIP:
		break;
	case GL_TRIANGLES:
		inCount -= 2;
		for(i = 0; i < inCount; i += 3)
		{
			NEXT_VERTEX_PTR;
			LOAD_VERTEX_FROM_ARRAY( inFormat, inContext, theVtx, &v0 );
	
			NEXT_VERTEX_PTR;
			LOAD_VERTEX_FROM_ARRAY( inFormat, inContext, theVtx, &v1 );
	
			NEXT_VERTEX_PTR;
			LOAD_VERTEX_FROM_ARRAY( inFormat, inContext, theVtx, &v2 );
	        
			grDrawTriangle(&v0, &v1, &v2);
		}
		break;
	case GL_TRIANGLE_STRIP:
		inCount -= 2;
		NEXT_VERTEX_PTR;
		LOAD_VERTEX_FROM_ARRAY( inFormat, inContext, theVtx, &v0 );

		NEXT_VERTEX_PTR;
		LOAD_VERTEX_FROM_ARRAY( inFormat, inContext, theVtx, &v1 );
		for(i = 0; i < inCount; i += 1)
		{	
			NEXT_VERTEX_PTR;
			LOAD_VERTEX_FROM_ARRAY( inFormat, inContext, theVtx, &v2 );
	        
			grDrawTriangle(&v0, &v1, &v2);
			
			v0 = v2;
			
			i += 1;
			if(i < inCount){
				NEXT_VERTEX_PTR;
				LOAD_VERTEX_FROM_ARRAY( inFormat, inContext, theVtx, &v2 );
				grDrawTriangle(&v0, &v1, &v2);
				v1 = v2;
			}

		}
		break;
	case GL_QUADS:
		inCount -= 3;
		for(i = 0; i < inCount; i += 4)
		{
			NEXT_VERTEX_PTR;
			LOAD_VERTEX_FROM_ARRAY( inFormat, inContext, theVtx, &v0 );
	
			NEXT_VERTEX_PTR;
			LOAD_VERTEX_FROM_ARRAY( inFormat, inContext, theVtx, &v1 );
	
			NEXT_VERTEX_PTR;
			LOAD_VERTEX_FROM_ARRAY( inFormat, inContext, theVtx, &v2 );
	        
			NEXT_VERTEX_PTR;
			LOAD_VERTEX_FROM_ARRAY( inFormat, inContext, theVtx, &v3 );
	        
			grDrawTriangle(&v0, &v1, &v2);
			grDrawTriangle(&v0, &v2, &v3);
		}
		break;
	case GL_QUAD_STRIP:
		if( inCount < 4 ) return;
		
		NEXT_VERTEX_PTR;
		LOAD_VERTEX_FROM_ARRAY( inFormat, inContext, theVtx, &v0 );

		NEXT_VERTEX_PTR;
		LOAD_VERTEX_FROM_ARRAY( inFormat, inContext, theVtx, &v1 );

		inCount -= 3;
		for(i = 0; i < inCount; i += 2)
		{
			NEXT_VERTEX_PTR;
			LOAD_VERTEX_FROM_ARRAY( inFormat, inContext, theVtx, &v2 );
	        
			NEXT_VERTEX_PTR;
			LOAD_VERTEX_FROM_ARRAY( inFormat, inContext, theVtx, &v3 );
	        
			grDrawTriangle(&v0, &v1, &v3);
			grDrawTriangle(&v0, &v3, &v2);
			v0 = v2;
			v1 = v3;
		}
		break;
	case GL_TRIANGLE_FAN:
	case GL_POLYGON:
		if( inCount < 3 ) return;
		
		NEXT_VERTEX_PTR;
		LOAD_VERTEX_FROM_ARRAY( inFormat, inContext, theVtx, &v0 );
	
		NEXT_VERTEX_PTR;
		LOAD_VERTEX_FROM_ARRAY( inFormat, inContext, theVtx, &v1 );
	
		for(i = 2; i < inCount; i++)
		{
			NEXT_VERTEX_PTR;
			LOAD_VERTEX_FROM_ARRAY( inFormat, inContext, theVtx, &v2 );
			
			grDrawTriangle(&v0, &v1, &v2);
			
			v1 = v2;
	
		}
		break;
	
	}
}

#define VA_RENDER_FUNC_NAME(mode, format) glrVARender_ ## mode ## _ ## format

#define VA_RENDER_FUNC(mode, format)									\
static void glrVARender_ ## mode ## _ ## format(						\
	GLDContext			inContext,										\
	void *				inData,											\
	GLint				inFirst,										\
	GLsizei				inCount,										\
	const GLuint *		inIndices)										\
{																		\
	glrVARenderGeneric(mode,format,inContext,inData,inFirst,inCount,inIndices);\
}

VA_RENDER_FUNC(GL_TRIANGLES,GLD_VA_TYPE_V4F_C4UB)

VA_RENDER_FUNC(GL_TRIANGLE_STRIP,GLD_VA_TYPE_V4F_C4UB)
VA_RENDER_FUNC(GL_TRIANGLE_STRIP,GLD_VA_TYPE_V4F_C4UB_T2F)
VA_RENDER_FUNC(GL_TRIANGLE_STRIP,GLD_VA_TYPE_V4F_C4UB_T2F_T2F_OWF)

VA_RENDER_FUNC(GL_QUADS,GLD_VA_TYPE_V4F_C4UB)
VA_RENDER_FUNC(GL_QUADS,GLD_VA_TYPE_V4F_C4UB_T2F)
VA_RENDER_FUNC(GL_QUADS,GLD_VA_TYPE_V4F_C4UB_T2F_T2F_OWF)

VA_RENDER_FUNC(GL_QUAD_STRIP,GLD_VA_TYPE_V4F_C4UB)
VA_RENDER_FUNC(GL_QUAD_STRIP,GLD_VA_TYPE_V4F_C4UB_T2F)
VA_RENDER_FUNC(GL_QUAD_STRIP,GLD_VA_TYPE_V4F_C4UB_T2F_T2F_OWF)

// fog based routines

VA_RENDER_FUNC(GL_TRIANGLES,GLD_VA_TYPE_V4F_C4UB_SF4UB)
VA_RENDER_FUNC(GL_TRIANGLES,GLD_VA_TYPE_V4F_C4UB_SF4UB_T2F)
VA_RENDER_FUNC(GL_TRIANGLES,GLD_VA_TYPE_V4F_C4UB_SF4UB_T2F_T2F_OWF)

VA_RENDER_FUNC(GL_TRIANGLE_STRIP,GLD_VA_TYPE_V4F_C4UB_SF4UB)
VA_RENDER_FUNC(GL_TRIANGLE_STRIP,GLD_VA_TYPE_V4F_C4UB_SF4UB_T2F)
VA_RENDER_FUNC(GL_TRIANGLE_STRIP,GLD_VA_TYPE_V4F_C4UB_SF4UB_T2F_T2F_OWF)

VA_RENDER_FUNC(GL_TRIANGLE_FAN,GLD_VA_TYPE_V4F_C4UB_SF4UB)
VA_RENDER_FUNC(GL_TRIANGLE_FAN,GLD_VA_TYPE_V4F_C4UB_SF4UB_T2F)
VA_RENDER_FUNC(GL_TRIANGLE_FAN,GLD_VA_TYPE_V4F_C4UB_SF4UB_T2F_T2F_OWF)

VA_RENDER_FUNC(GL_QUADS,GLD_VA_TYPE_V4F_C4UB_SF4UB)
VA_RENDER_FUNC(GL_QUADS,GLD_VA_TYPE_V4F_C4UB_SF4UB_T2F)
VA_RENDER_FUNC(GL_QUADS,GLD_VA_TYPE_V4F_C4UB_SF4UB_T2F_T2F_OWF)

VA_RENDER_FUNC(GL_QUAD_STRIP,GLD_VA_TYPE_V4F_C4UB_SF4UB)
VA_RENDER_FUNC(GL_QUAD_STRIP,GLD_VA_TYPE_V4F_C4UB_SF4UB_T2F)
VA_RENDER_FUNC(GL_QUAD_STRIP,GLD_VA_TYPE_V4F_C4UB_SF4UB_T2F_T2F_OWF)



// end of inline stuff



static glrVertexArrayRenderingFuncPtr glrVARenderColor[2][10] = {

{
	glrVARenderNotYetImplemented,		/* GL_POINTS                */
	glrVARenderNotYetImplemented,		/* GL_LINES                 */
	glrVARenderNotYetImplemented,		/* GL_LINE_LOOP             */
	glrVARenderNotYetImplemented,		/* GL_LINE_STRIP            */
	VA_RENDER_FUNC_NAME(GL_TRIANGLES,GLD_VA_TYPE_V4F_C4UB),			/* GL_TRIANGLES             */
	VA_RENDER_FUNC_NAME(GL_TRIANGLE_STRIP,GLD_VA_TYPE_V4F_C4UB),	/* GL_TRIANGLE_STRIP        */
	glrVARenderSmoothPolygonsG3,									/* GL_TRIANGLE_FAN          */
	VA_RENDER_FUNC_NAME(GL_QUADS,GLD_VA_TYPE_V4F_C4UB),				/* GL_QUADS                 */
	VA_RENDER_FUNC_NAME(GL_QUAD_STRIP,GLD_VA_TYPE_V4F_C4UB),		/* GL_QUAD_STRIP            */
	glrVARenderSmoothPolygonsG3										/* GL_POLYGON               */
},
{
	glrVARenderNotYetImplemented,		/* GL_POINTS                */
	glrVARenderNotYetImplemented,		/* GL_LINES                 */
	glrVARenderNotYetImplemented,		/* GL_LINE_LOOP             */
	glrVARenderNotYetImplemented,		/* GL_LINE_STRIP            */
	VA_RENDER_FUNC_NAME(GL_TRIANGLES,GLD_VA_TYPE_V4F_C4UB),			/* GL_TRIANGLES             */
	VA_RENDER_FUNC_NAME(GL_TRIANGLE_STRIP,GLD_VA_TYPE_V4F_C4UB),	/* GL_TRIANGLE_STRIP        */
	glrVARenderSmoothPolygonsG4,									/* GL_TRIANGLE_FAN          */
	VA_RENDER_FUNC_NAME(GL_QUADS,GLD_VA_TYPE_V4F_C4UB),				/* GL_QUADS                 */
	VA_RENDER_FUNC_NAME(GL_QUAD_STRIP,GLD_VA_TYPE_V4F_C4UB),		/* GL_QUAD_STRIP            */
	glrVARenderSmoothPolygonsG4										/* GL_POLYGON               */
}
};

static glrVertexArrayRenderingFuncPtr glrVARenderTexture[2][10] = {
{
	glrVARenderNotYetImplemented,		/* GL_POINTS                */
	glrVARenderNotYetImplemented,		/* GL_LINES                 */
	glrVARenderNotYetImplemented,		/* GL_LINE_LOOP             */
	glrVARenderNotYetImplemented,		/* GL_LINE_STRIP            */
	glrVARenderTextureTrianglesG3,		/* GL_TRIANGLES             */
	VA_RENDER_FUNC_NAME(GL_TRIANGLE_STRIP,GLD_VA_TYPE_V4F_C4UB_T2F),	/* GL_TRIANGLE_STRIP        */
	glrVARenderTexturePolygonsG3,										/* GL_TRIANGLE_FAN          */
	VA_RENDER_FUNC_NAME(GL_QUADS,GLD_VA_TYPE_V4F_C4UB_T2F),				/* GL_QUADS                 */
	VA_RENDER_FUNC_NAME(GL_QUAD_STRIP,GLD_VA_TYPE_V4F_C4UB_T2F),		/* GL_QUAD_STRIP            */
	glrVARenderTexturePolygonsG3										/* GL_POLYGON               */
},
{
	glrVARenderNotYetImplemented,		/* GL_POINTS                */
	glrVARenderNotYetImplemented,		/* GL_LINES                 */
	glrVARenderNotYetImplemented,		/* GL_LINE_LOOP             */
	glrVARenderNotYetImplemented,		/* GL_LINE_STRIP            */
	glrVARenderTextureTrianglesG4,		/* GL_TRIANGLES             */
	VA_RENDER_FUNC_NAME(GL_TRIANGLE_STRIP,GLD_VA_TYPE_V4F_C4UB_T2F),	/* GL_TRIANGLE_STRIP        */
	glrVARenderTexturePolygonsG4,										/* GL_TRIANGLE_FAN          */
	VA_RENDER_FUNC_NAME(GL_QUADS,GLD_VA_TYPE_V4F_C4UB_T2F),				/* GL_QUADS                 */
	VA_RENDER_FUNC_NAME(GL_QUAD_STRIP,GLD_VA_TYPE_V4F_C4UB_T2F),		/* GL_QUAD_STRIP            */
	glrVARenderTexturePolygonsG4										/* GL_POLYGON               */
}
};

static glrVertexArrayRenderingFuncPtr glrVARenderMultiTexture[2][10] = {
{
	glrVARenderNotYetImplemented,		/* GL_POINTS                */
	glrVARenderNotYetImplemented,		/* GL_LINES                 */
	glrVARenderNotYetImplemented,		/* GL_LINE_LOOP             */
	glrVARenderNotYetImplemented,		/* GL_LINE_STRIP            */
	glrVARenderMultiTextureTrianglesG3,											/* GL_TRIANGLES             */
	VA_RENDER_FUNC_NAME(GL_TRIANGLE_STRIP,GLD_VA_TYPE_V4F_C4UB_T2F_T2F_OWF),	/* GL_TRIANGLE_STRIP        */
	glrVARenderMultiTexturePolygonsG3,											/* GL_TRIANGLE_FAN          */
	VA_RENDER_FUNC_NAME(GL_QUADS,GLD_VA_TYPE_V4F_C4UB_T2F_T2F_OWF),				/* GL_QUADS                 */
	VA_RENDER_FUNC_NAME(GL_QUAD_STRIP,GLD_VA_TYPE_V4F_C4UB_T2F_T2F_OWF),		/* GL_QUAD_STRIP            */
	glrVARenderMultiTexturePolygonsG3											/* GL_POLYGON               */
},
{
	glrVARenderNotYetImplemented,		/* GL_POINTS                */
	glrVARenderNotYetImplemented,		/* GL_LINES                 */
	glrVARenderNotYetImplemented,		/* GL_LINE_LOOP             */
	glrVARenderNotYetImplemented,		/* GL_LINE_STRIP            */
	glrVARenderMultiTextureTrianglesG4,	/* GL_TRIANGLES             */
	VA_RENDER_FUNC_NAME(GL_TRIANGLE_STRIP,GLD_VA_TYPE_V4F_C4UB_T2F_T2F_OWF),	/* GL_TRIANGLE_STRIP        */
	glrVARenderMultiTexturePolygonsG4,											/* GL_TRIANGLE_FAN          */
	VA_RENDER_FUNC_NAME(GL_QUADS,GLD_VA_TYPE_V4F_C4UB_T2F_T2F_OWF),				/* GL_QUADS                 */
	VA_RENDER_FUNC_NAME(GL_QUAD_STRIP,GLD_VA_TYPE_V4F_C4UB_T2F_T2F_OWF),		/* GL_QUAD_STRIP            */
	glrVARenderMultiTexturePolygonsG4											/* GL_POLYGON               */
}
};

static glrVertexArrayRenderingFuncPtr glrVARenderColorFog[2][10] = {
{
	glrVARenderNotYetImplemented,		/* GL_POINTS                */
	glrVARenderNotYetImplemented,		/* GL_LINES                 */
	glrVARenderNotYetImplemented,		/* GL_LINE_LOOP             */
	glrVARenderNotYetImplemented,		/* GL_LINE_STRIP            */
	VA_RENDER_FUNC_NAME(GL_TRIANGLES,GLD_VA_TYPE_V4F_C4UB_SF4UB),		/* GL_TRIANGLES             */
	VA_RENDER_FUNC_NAME(GL_TRIANGLE_STRIP,GLD_VA_TYPE_V4F_C4UB_SF4UB),	/* GL_TRIANGLE_STRIP        */
	VA_RENDER_FUNC_NAME(GL_TRIANGLE_FAN,GLD_VA_TYPE_V4F_C4UB_SF4UB),	/* GL_TRIANGLE_FAN          */
	VA_RENDER_FUNC_NAME(GL_QUADS,GLD_VA_TYPE_V4F_C4UB_SF4UB),			/* GL_QUADS                 */
	VA_RENDER_FUNC_NAME(GL_QUAD_STRIP,GLD_VA_TYPE_V4F_C4UB_SF4UB),		/* GL_QUAD_STRIP            */
	VA_RENDER_FUNC_NAME(GL_TRIANGLE_FAN,GLD_VA_TYPE_V4F_C4UB_SF4UB)		/* GL_POLYGON               */
},
{
	glrVARenderNotYetImplemented,		/* GL_POINTS                */
	glrVARenderNotYetImplemented,		/* GL_LINES                 */
	glrVARenderNotYetImplemented,		/* GL_LINE_LOOP             */
	glrVARenderNotYetImplemented,		/* GL_LINE_STRIP            */
	VA_RENDER_FUNC_NAME(GL_TRIANGLES,GLD_VA_TYPE_V4F_C4UB_SF4UB),		/* GL_TRIANGLES             */
	VA_RENDER_FUNC_NAME(GL_TRIANGLE_STRIP,GLD_VA_TYPE_V4F_C4UB_SF4UB),	/* GL_TRIANGLE_STRIP        */
	VA_RENDER_FUNC_NAME(GL_TRIANGLE_FAN,GLD_VA_TYPE_V4F_C4UB_SF4UB),	/* GL_TRIANGLE_FAN          */
	VA_RENDER_FUNC_NAME(GL_QUADS,GLD_VA_TYPE_V4F_C4UB_SF4UB),			/* GL_QUADS                 */
	VA_RENDER_FUNC_NAME(GL_QUAD_STRIP,GLD_VA_TYPE_V4F_C4UB_SF4UB),		/* GL_QUAD_STRIP            */
	VA_RENDER_FUNC_NAME(GL_TRIANGLE_FAN,GLD_VA_TYPE_V4F_C4UB_SF4UB)		/* GL_POLYGON               */
}
};

static glrVertexArrayRenderingFuncPtr glrVARenderTextureFog[2][10] = {
{
	glrVARenderNotYetImplemented,		/* GL_POINTS                */
	glrVARenderNotYetImplemented,		/* GL_LINES                 */
	glrVARenderNotYetImplemented,		/* GL_LINE_LOOP             */
	glrVARenderNotYetImplemented,		/* GL_LINE_STRIP            */
	VA_RENDER_FUNC_NAME(GL_TRIANGLES,GLD_VA_TYPE_V4F_C4UB_SF4UB_T2F),		/* GL_TRIANGLES             */
	VA_RENDER_FUNC_NAME(GL_TRIANGLE_STRIP,GLD_VA_TYPE_V4F_C4UB_SF4UB_T2F),	/* GL_TRIANGLE_STRIP        */
	VA_RENDER_FUNC_NAME(GL_TRIANGLE_FAN,GLD_VA_TYPE_V4F_C4UB_SF4UB_T2F),	/* GL_TRIANGLE_FAN          */
	VA_RENDER_FUNC_NAME(GL_QUADS,GLD_VA_TYPE_V4F_C4UB_SF4UB_T2F),			/* GL_QUADS                 */
	VA_RENDER_FUNC_NAME(GL_QUAD_STRIP,GLD_VA_TYPE_V4F_C4UB_SF4UB_T2F),		/* GL_QUAD_STRIP            */
	VA_RENDER_FUNC_NAME(GL_TRIANGLE_FAN,GLD_VA_TYPE_V4F_C4UB_SF4UB_T2F)		/* GL_POLYGON               */
},
{
	glrVARenderNotYetImplemented,		/* GL_POINTS                */
	glrVARenderNotYetImplemented,		/* GL_LINES                 */
	glrVARenderNotYetImplemented,		/* GL_LINE_LOOP             */
	glrVARenderNotYetImplemented,		/* GL_LINE_STRIP            */
	VA_RENDER_FUNC_NAME(GL_TRIANGLES,GLD_VA_TYPE_V4F_C4UB_SF4UB_T2F),		/* GL_TRIANGLES             */
	VA_RENDER_FUNC_NAME(GL_TRIANGLE_STRIP,GLD_VA_TYPE_V4F_C4UB_SF4UB_T2F),	/* GL_TRIANGLE_STRIP        */
	VA_RENDER_FUNC_NAME(GL_TRIANGLE_FAN,GLD_VA_TYPE_V4F_C4UB_SF4UB_T2F),	/* GL_TRIANGLE_FAN          */
	VA_RENDER_FUNC_NAME(GL_QUADS,GLD_VA_TYPE_V4F_C4UB_SF4UB_T2F),			/* GL_QUADS                 */
	VA_RENDER_FUNC_NAME(GL_QUAD_STRIP,GLD_VA_TYPE_V4F_C4UB_SF4UB_T2F),		/* GL_QUAD_STRIP            */
	VA_RENDER_FUNC_NAME(GL_TRIANGLE_FAN,GLD_VA_TYPE_V4F_C4UB_SF4UB_T2F)		/* GL_POLYGON               */
}
};

static glrVertexArrayRenderingFuncPtr glrVARenderMultiTextureFog[2][10] = {
{
	glrVARenderNotYetImplemented,		/* GL_POINTS                */
	glrVARenderNotYetImplemented,		/* GL_LINES                 */
	glrVARenderNotYetImplemented,		/* GL_LINE_LOOP             */
	glrVARenderNotYetImplemented,		/* GL_LINE_STRIP            */
	VA_RENDER_FUNC_NAME(GL_TRIANGLES,GLD_VA_TYPE_V4F_C4UB_SF4UB_T2F_T2F_OWF),		/* GL_TRIANGLES             */
	VA_RENDER_FUNC_NAME(GL_TRIANGLE_STRIP,GLD_VA_TYPE_V4F_C4UB_SF4UB_T2F_T2F_OWF),	/* GL_TRIANGLE_STRIP        */
	VA_RENDER_FUNC_NAME(GL_TRIANGLE_FAN,GLD_VA_TYPE_V4F_C4UB_SF4UB_T2F_T2F_OWF),	/* GL_TRIANGLE_FAN          */
	VA_RENDER_FUNC_NAME(GL_QUADS,GLD_VA_TYPE_V4F_C4UB_SF4UB_T2F_T2F_OWF),			/* GL_QUADS                 */
	VA_RENDER_FUNC_NAME(GL_QUAD_STRIP,GLD_VA_TYPE_V4F_C4UB_SF4UB_T2F_T2F_OWF),		/* GL_QUAD_STRIP            */
	VA_RENDER_FUNC_NAME(GL_TRIANGLE_FAN,GLD_VA_TYPE_V4F_C4UB_SF4UB_T2F_T2F_OWF)		/* GL_POLYGON               */
},
{
	glrVARenderNotYetImplemented,		/* GL_POINTS                */
	glrVARenderNotYetImplemented,		/* GL_LINES                 */
	glrVARenderNotYetImplemented,		/* GL_LINE_LOOP             */
	glrVARenderNotYetImplemented,		/* GL_LINE_STRIP            */
	VA_RENDER_FUNC_NAME(GL_TRIANGLES,GLD_VA_TYPE_V4F_C4UB_SF4UB_T2F_T2F_OWF),		/* GL_TRIANGLES             */
	VA_RENDER_FUNC_NAME(GL_TRIANGLE_STRIP,GLD_VA_TYPE_V4F_C4UB_SF4UB_T2F_T2F_OWF),	/* GL_TRIANGLE_STRIP        */
	VA_RENDER_FUNC_NAME(GL_TRIANGLE_FAN,GLD_VA_TYPE_V4F_C4UB_SF4UB_T2F_T2F_OWF),	/* GL_TRIANGLE_FAN          */
	VA_RENDER_FUNC_NAME(GL_QUADS,GLD_VA_TYPE_V4F_C4UB_SF4UB_T2F_T2F_OWF),			/* GL_QUADS                 */
	VA_RENDER_FUNC_NAME(GL_QUAD_STRIP,GLD_VA_TYPE_V4F_C4UB_SF4UB_T2F_T2F_OWF),		/* GL_QUAD_STRIP            */
	VA_RENDER_FUNC_NAME(GL_TRIANGLE_FAN,GLD_VA_TYPE_V4F_C4UB_SF4UB_T2F_T2F_OWF)		/* GL_POLYGON               */
}
};





/*
________________________________________________________________________________________

      gldAllocVertexBuffer
________________________________________________________________________________________

*/

static void *vBuff[10];
static FxU32 vCount[10];

void *
gldAllocVertexBuffer(
	GLDContext				inContext,
	GLenum					inFormat,
	GLsizei *				ioCount)
{
	void *                  theData = 0;
	unsigned long           theDataSize = 0;
	
	DEBUG_ENTRY( gldAllocVertexBuffer );

	DEBUG_VERBOSE( gldAllocVertexBuffer, "format = %d (0x%08x), count = %d \n", inFormat, inFormat, *ioCount );

	*ioCount *= 2;
	if ( *ioCount > GLR_VERTEX_MAX_COUNT )
	{
		DEBUG_VERBOSE( gldAllocVertexBuffer, "vertex count requested is too large (%d), will be cut down to : %d \n", *ioCount, GLR_VERTEX_MAX_COUNT );
		*ioCount = GLR_VERTEX_MAX_COUNT;
	}

	switch ( inFormat )
	{
		case GLD_VA_TYPE_V4F_C4UB_T2F_T2F_OWF:
			inContext->va[inFormat].renderFuncTbl = glrVARenderMultiTexture[inContext->cpuType];
			theDataSize = *ioCount * sizeof(GLDVertexArrayTexElement);
			break;

		case GLD_VA_TYPE_V4F_C4UB_T2F:
			inContext->va[inFormat].renderFuncTbl = glrVARenderTexture[inContext->cpuType];
			theDataSize = *ioCount * sizeof(GLDVertexArrayTex1Element);
			break;

		case GLD_VA_TYPE_V4F_C4UB:
			inContext->va[inFormat].renderFuncTbl = glrVARenderColor[inContext->cpuType];
			theDataSize = *ioCount * sizeof(GLDVertexArrayGrElement);
			break;

		case GLD_VA_TYPE_V4F_C4UB_SF4UB_T2F_T2F_OWF:
			inContext->va[inFormat].renderFuncTbl = glrVARenderMultiTextureFog[inContext->cpuType];
			theDataSize = *ioCount * sizeof(GLDVertexArrayFogTexElement);
			break;

		case GLD_VA_TYPE_V4F_C4UB_SF4UB_T2F:
			inContext->va[inFormat].renderFuncTbl = glrVARenderTextureFog[inContext->cpuType];
			theDataSize = *ioCount * sizeof(GLDVertexArrayFogTex1Element);
			break;

		case GLD_VA_TYPE_V4F_C4UB_SF4UB:
			inContext->va[inFormat].renderFuncTbl = glrVARenderColorFog[inContext->cpuType];
			theDataSize = *ioCount * sizeof(GLDVertexArrayGrFogElement);
			break;

		default:
			/* if values are added here, adjust GLR_VA_TYPE_LAST accordingly */
			DEBUG_ERROR( gldAllocVertexBuffer, "Unknow vertex array format: %d (0x%08x)\n", inFormat, inFormat );
			break;
	}


	if ( theDataSize )
	{
	  GLuint current_va_index;
	  current_va_index = inContext->va[inFormat].current_va_index = (inContext->va[inFormat].current_va_index + 1) & (GLR_NUM_VA_BUFFERS - 1);
		inContext->currentVertexDataType = inFormat;
		inContext->currentVertexDataTypeRenderFuncTbl = inContext->va[inFormat].renderFuncTbl;
		
		if ( inContext->va[inFormat].dataSize[current_va_index] < theDataSize )
		{
	        if ( inContext->va[inFormat].dataSize[current_va_index] && inContext->va[inFormat].data[current_va_index] )
	        {
			  glmFree( inContext->va[inFormat].data[current_va_index] );
			}
			inContext->va[inFormat].data[current_va_index] = glmMalloc( theDataSize );
			if(!inContext->va[inFormat].data[current_va_index])
			{
			  *ioCount = 0;
			  theDataSize = 0;
			}
			inContext->va[inFormat].dataSize[current_va_index] = theDataSize;			
		}
		theData = inContext->va[inFormat].data[current_va_index];
	}
	else
	{
		*ioCount = 0;
	}
	
	
	return theData;
}




/*
________________________________________________________________________________________

      gldCompleteVertexBuffer
________________________________________________________________________________________

*/
void gldCompleteVertexBuffer(
	GLDContext 		ctx, 
	void *			data, 
	GLsizei 		count)
{
	DEBUG_NOT_YET_IMPLEMENTED( gldCompleteVertexBuffer );
}




/*
________________________________________________________________________________________

      gldFreeVertexBuffer
________________________________________________________________________________________

We don't actually release, that way we already have one ready the next time we need one.

*/

void
gldFreeVertexBuffer(
	GLDContext				inContext,
	void *					inData)
{
	DEBUG_ENTRY( gldFreeVertexBuffer );
}




/*
________________________________________________________________________________________

      gldBeginPrimitiveBuffer
________________________________________________________________________________________

*/

void *
gldBeginPrimitiveBuffer(
	GLDContext				inContext,
	GLsizei					inCount,
	GLenum					inFormat)
{
	DEBUG_NOT_YET_IMPLEMENTED( gldBeginPrimitiveBuffer );
	return 0;
}




/*
________________________________________________________________________________________

      glrEndPrimitiveBuffer
________________________________________________________________________________________

*/

void
glrEndPrimitiveBuffer(
	GLDContext				inContext,
	void *					inData,
	GLbitfield				inOptions,
	GLenum					inMode,
	GLsizei					inCount)
{
	DEBUG_NOT_YET_IMPLEMENTED( glrEndPrimitiveBuffer );
}







/*
________________________________________________________________________________________

      glrRenderVertexBuffer
________________________________________________________________________________________

*/

void
glrRenderVertexBuffer(
	GLDContext				inContext,
	void *					inData,
	GLbitfield				inOptions,
	GLenum					inMode,
	GLint					inFirst,
	GLsizei					inCount,
	GLenum					inType,
	const void *			inIndices)
{
	DEBUG_ENTRY( glrRenderVertexBuffer );

#if GLR_DEBUG
	if ( inContext->currentVertexDataTypeRenderFuncTbl[ inMode ] == glrVARenderNotYetImplemented )
	{
		char * theFormat;
		char * theMode;
		
		switch ( inContext->currentVertexDataType )
		{
			case GLD_VA_TYPE_V4F_C4UB_T2F_T2F_OWF:
				theFormat = "GLD_VA_TYPE_V4F_C4UB_T2F_T2F_OWF";
				break;
			
			case GLD_VA_TYPE_V4F_C4UB_T2F:
				theFormat = "GLD_VA_TYPE_V4F_C4UB_T2F";
				break;
			
			case GLD_VA_TYPE_V4F_C4UB:
				theFormat = "GLD_VA_TYPE_V4F_C4UB";
				break;
			
			case GLD_VA_TYPE_V4F_C4UB_SF4UB_T2F_T2F_OWF:
				theFormat = "GLD_VA_TYPE_V4F_C4UB_SF4UB_T2F_T2F_OWF";
				break;
			
			case GLD_VA_TYPE_V4F_C4UB_SF4UB_T2F:
				theFormat = "GLD_VA_TYPE_V4F_C4UB_SF4UB_T2F";
				break;
			
			case GLD_VA_TYPE_V4F_C4UB_SF4UB:
				theFormat = "GLD_VA_TYPE_V4F_C4UB_SF4UB";
				break;

			default:
				theFormat = "unknown";
				break;
		}

		
		switch ( inMode )
		{
			case GL_POINTS:
				theMode = "GL_POINTS";
				break;
			
			case GL_LINES:
				theMode = "GL_LINES";
				break;
			
			case GL_LINE_LOOP:
				theMode = "GL_LINE_LOOP";
				break;
			
			case GL_LINE_STRIP:
				theMode = "GL_LINE_STRIP";
				break;
			
			case GL_TRIANGLES:
				theMode = "GL_TRIANGLES";
				break;
			
			case GL_TRIANGLE_STRIP:
				theMode = "GL_TRIANGLE_STRIP";
				break;

			case GL_TRIANGLE_FAN:
				theMode = "GL_TRIANGLE_FAN";
				break;

			case GL_QUADS:
				theMode = "GL_QUADS";
				break;

			case GL_QUAD_STRIP:
				theMode = "GL_QUAD_STRIP";
				break;

			case GL_POLYGON:
				theMode = "GL_POLYGON";
				break;

			default:
				theFormat = "unknown";
				break;
		}
			
		DEBUG_ERROR( glrRenderVertexBuffer, "Not Yet Implemented: format = %s, inMode = %s (%d)\n", theFormat, theMode, inMode );
	}
#endif 


	if(inType == GL_UNSIGNED_INT){
		// fast track
		(*inContext->currentVertexDataTypeRenderFuncTbl[ inMode ])( inContext, inData, inFirst, inCount, inIndices );
	} else {
		// slow track
		GLuint i;
		const GLuint * indices;
			
		switch(inType){
		case GL_UNSIGNED_BYTE:
			if(inCount > inContext->indexBufferSize){
				if(inContext->indexBuffer){
					glmFree(inContext->indexBuffer);
					inContext->indexBuffer = glmMalloc(inCount * sizeof(GLuint));
					DEBUG_ASSERT(inContext->indexBuffer != NULL);
					inContext->indexBufferSize = inCount;
				}
			}
			for(i = 0; i < inCount; i++){
				inContext->indexBuffer[i] = ((GLubyte *)inIndices)[i];
			}
			indices = inContext->indexBuffer;
			break;
		case GL_UNSIGNED_SHORT:
			if(inCount > inContext->indexBufferSize){
				if(inContext->indexBuffer){
					glmFree(inContext->indexBuffer);
					inContext->indexBuffer = glmMalloc(inCount * sizeof(GLuint));
					DEBUG_ASSERT(inContext->indexBuffer != NULL);
					inContext->indexBufferSize = inCount;
				}
			}
			for(i = 0; i < inCount; i++){
				inContext->indexBuffer[i] = ((GLushort *)inIndices)[i];
			}
			indices = inContext->indexBuffer;
			break;
		}
	
		(*inContext->currentVertexDataTypeRenderFuncTbl[ inMode ])( inContext, inData, inFirst, inCount, indices );
	}
}




/*
________________________________________________________________________________________

      glrVARenderNotYetImplemented
________________________________________________________________________________________

*/

static void glrVARenderNotYetImplemented(
	GLDContext			inContext,
	void *				inData,
	GLint				inFirst,
	GLsizei				inCount,
	const GLuint *		inIndices)
{
	DEBUG_NOT_YET_IMPLEMENTED( glrVARenderNotYetImplemented );
}

