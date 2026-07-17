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

#define glrUpdatePointSettings(inContext) glrSetPrimitive(inContext, GLR_POINT)

static float circle[16] = {
	0.0, 0.353553, 0.5, 0.353553,
	0.0, -0.353553, -0.5, -0.353553,
	0.0, 0.353553, 0.5, 0.353553,
	0.0, -0.353553, -0.5, -0.353553,
};


static GLuint glrCalcLoadOptions(GLDContext inContext)
{
	GLuint options = 0;
	
	if(inContext->state->fog_mode.enable){
		options |= LOAD_FOG;
	}
	
	if(inContext->hw_texture[0]){
		if(inContext->hw_texture[1]){
			options |= LOAD_TEX0 | LOAD_TEX1;
		} else {
			if(inContext->arb_unit_map[0] == GR_TMU0){
				options |= LOAD_TEX0;
			} else {
				options |= LOAD_TEX1;		
			}
		}
	}
	
	if(inContext->cpuType){
		options |= LOAD_G4;
	}
	
	return options;
}

static void glrRenderBigPoints(
	GLDContext			inContext,
	const GLDVertex *	inVtx,
	GLint				inNumberOfVtx)
{
	GLint 				i;
	GrVertex			v[4];
	GrVertex			*vp[4];
	GLfloat				size;
	GLfloat				minX, minY, maxX, maxY;
	GLuint				options;
	
	/*****/

	DEBUG_SLOWPATH_ENTRY( glrRenderBigPoints );

	options	= glrCalcLoadOptions(inContext);
	
	size = inContext->state->point_mode.size;
	
	if(inContext->state->point_mode.smooth_enable){
		for(i = 0; i < inNumberOfVtx; i += 1){
			GLint j;
			
			glrLoadVertexGeneric( inContext, inVtx + i, v, options);
			for(j = 0; j < 8; j++){
				v[1] = v[2] = v[0];			

				v[1].x += size * circle[j];
				v[1].y += size * circle[j + 2];
		
				v[2].x += size * circle[j + 1];
				v[2].y += size * circle[j + 3];
				
				grAADrawTriangle(&v[0], &v[1], &v[2], 0, 1, 0);
			}
	
		}	
	} else {
		vp[0] = &v[0];
		vp[1] = &v[1];
		vp[2] = &v[2];
		vp[3] = &v[3];
	
		for(i = 0; i < inNumberOfVtx; i += 1){
		
			glrLoadVertexGeneric( inContext, inVtx + i, v, options);
			v[1] = v[2] = v[3] = v[0];
	
			minX = v[0].x - size * 0.5;
			minY = v[0].y - size * 0.5;
			maxX = minX + size;
			maxY = minY + size;
	
			v[0].x = minX;
			v[0].y = minY;
	
			v[1].x = maxX;
			v[1].y = minY;
	
			v[2].x = maxX;
			v[2].y = maxY;
	
			v[3].x = minX;
			v[3].y = maxY;
	
			grDrawVertexArray(GR_POLYGON, 4, vp);
		}	
	}
}


static void glrRenderBigPointsPtr(
	GLDContext			inContext,
	const GLDVertex **	inVtx,
	GLint				inNumberOfVtx)
{
	GLint 				i;
	GrVertex			v[4];
	GrVertex			*vp[4];
	GLfloat				size;
	GLfloat				minX, minY, maxX, maxY;
	GLuint				options;
	
	/*****/

	DEBUG_SLOWPATH_ENTRY( glrRenderBigPoints );

	options	= glrCalcLoadOptions(inContext);
	
	size = inContext->state->point_mode.size;
	
	if(inContext->state->point_mode.smooth_enable){
		for(i = 0; i < inNumberOfVtx; i += 1){
			GLint j;
			
			glrLoadVertexGeneric( inContext, inVtx[i], v, options);
			for(j = 0; j < 8; j++){
				v[1] = v[2] = v[0];			

				v[1].x += size * circle[j];
				v[1].y += size * circle[j + 2];
		
				v[2].x += size * circle[j + 1];
				v[2].y += size * circle[j + 3];
				
				grAADrawTriangle(&v[0], &v[1], &v[2], 0, 1, 0);
			}
	
		}	
	} else {
		vp[0] = &v[0];
		vp[1] = &v[1];
		vp[2] = &v[2];
		vp[3] = &v[3];
	
		for(i = 0; i < inNumberOfVtx; i += 1){
		
			glrLoadVertexGeneric( inContext, inVtx[i], v, options);
			v[1] = v[2] = v[3] = v[0];
	
			minX = v[0].x - size * 0.5;
			minY = v[0].y - size * 0.5;
			maxX = minX + size;
			maxY = minY + size;
	
			v[0].x = minX;
			v[0].y = minY;
	
			v[1].x = maxX;
			v[1].y = minY;
	
			v[2].x = maxX;
			v[2].y = maxY;
	
			v[3].x = minX;
			v[3].y = maxY;
	
			grDrawVertexArray(GR_POLYGON, 4, vp);
		}	
	}
}



/*
________________________________________________________________________________________

      glrRenderPoints
________________________________________________________________________________________

*/

void glrRenderPoints(
	GLDContext			inContext,
	const GLDVertex *	inVtx,
	GLint				inNumberOfVtx)
{
	GLint				i;
	GrVertex			v0;
	GLuint				options;
	
	DEBUG_SLOWPATH_ENTRY( glrRenderPoints );
	if(inContext->state->point_mode.size != 1.0){
		glrRenderBigPoints(inContext, inVtx, inNumberOfVtx);
		return;
	}
	
	options	= glrCalcLoadOptions(inContext);
	
	for(i = 0; i < inNumberOfVtx; i += 1)
	{		
		glrLoadVertexGeneric( inContext, inVtx + i, &v0, options);
		
		grDrawPoint( &v0 );
	}
}





/*
________________________________________________________________________________________

      glrRenderPointsPtr
________________________________________________________________________________________

*/

void glrRenderPointsPtr(
	GLDContext			inContext,
	const GLDVertex **	inVtx,
	GLint				inNumberOfVtx)
{
	GLint				i;
	GrVertex			v0;
	GLuint				options;
	
	DEBUG_SLOWPATH_ENTRY( glrRenderPointsPtr );

	if(inContext->state->point_mode.size != 1.0){
		glrRenderBigPointsPtr(inContext, inVtx, inNumberOfVtx);
		return;
	}

	options	= glrCalcLoadOptions(inContext);
	
	for(i = 0; i < inNumberOfVtx; i += 1)
	{		
		glrLoadVertexGeneric( inContext, inVtx[i], &v0, options);
		
		grDrawPoint( &v0 );
	}
}




