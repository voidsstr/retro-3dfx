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

#include <stdlib.h>

#include "glr.h"
#include "glr_drawing.h"

#define glrUpdateLineSettings(inContext) glrSetPrimitive(inContext, GLR_LINE)



/*
________________________________________________________________________________________

      glrRenderFlatLines
________________________________________________________________________________________

*/

void
glrRenderFlatLines(
	GLDContext				inContext,
	const GLDVertex *		inVtx,
	GLint					inNumberOfVtx)
{
	GLint					i, j, n;
	GrVertex				v0, v1;
	
	DEBUG_SLOWPATH_ENTRY( glrRenderFlatLines );

	n = inNumberOfVtx - 1;
	
	if( inContext->primitive != GLR_LINE) glrUpdateLineSettings( inContext );
	
	for(i = 0; i < n; i += 2)
	{
		j = i + 1;

		glrLoadVertex_Smooth( inContext, inVtx[j], v1);
		glrLoadVertex_Flat( inContext, inVtx[i], v0, v1);
		
		grDrawLine(&v0, &v1);
	}
}




/*
________________________________________________________________________________________

      glrRenderSmoothLines
________________________________________________________________________________________

*/

void
glrRenderSmoothLines(
	GLDContext			inContext,
	const GLDVertex *	inVtx,
	GLint				inNumberOfVtx)
{
	GLint				i, j, n;
	GrVertex			v0, v1;
	
	DEBUG_SLOWPATH_ENTRY( glrRenderSmoothLines );

	n = inNumberOfVtx - 1;
	if( inContext->primitive != GLR_LINE)
		glrUpdateLineSettings( inContext );
	
	for(i = 0; i < n; i += 2)
	{
		j = i + 1;
		
		glrLoadVertex_Smooth( inContext, inVtx[i], v0);
		glrLoadVertex_Smooth( inContext, inVtx[j], v1);
		
		grDrawLine( &v0, &v1 );
	}
}




/*
________________________________________________________________________________________

      glrRenderFlatLinesPtr
________________________________________________________________________________________

*/

void glrRenderFlatLinesPtr(
	GLDContext			inContext,
	const GLDVertex **	inVtx,
	GLint				inNumberOfVtx,
	GLenum				/*inReset*/)
{
	GLint				i, j, n;
	GrVertex			v0, v1;
	
	DEBUG_SLOWPATH_ENTRY( glrRenderFlatLinesPtr );

	n = inNumberOfVtx - 1;
	if( inContext->primitive != GLR_LINE) glrUpdateLineSettings( inContext );
	
	for(i = 0; i < n; i += 2)
	{
		j = i + 1;
		
		glrLoadVertex_Smooth( inContext, *inVtx[j], v1);
		glrLoadVertex_Flat( inContext, *inVtx[i], v0, v1);
		
		grDrawLine(&v0, &v1);
	}
}




/*
________________________________________________________________________________________

      glrRenderSmoothLinesPtr
________________________________________________________________________________________

*/

void
glrRenderSmoothLinesPtr(
	GLDContext			inContext,
	const GLDVertex **	inVtx,
	GLint				inNumberOfVtx,
	GLenum				/*inReset*/)
{
	GLint				i, j, n;
	GrVertex			v0, v1;
	
	DEBUG_SLOWPATH_ENTRY( glrRenderSmoothLinesPtr );

	n = inNumberOfVtx - 1;
	if( inContext->primitive != GLR_LINE) glrUpdateLineSettings( inContext );
	
	for(i = 0; i < n; i += 2)
	{
		j = i + 1;
		
		glrLoadVertex_Smooth( inContext, *inVtx[i], v0);
		glrLoadVertex_Smooth( inContext, *inVtx[j], v1);
		
		grDrawLine(&v0, &v1);
	}
}



/*
________________________________________________________________________________________

      glrRenderFlatLineStrip
________________________________________________________________________________________

*/

void
glrRenderFlatLineStrip(
	GLDContext			inContext,
	const GLDVertex *	inVtx,
	GLint				inNumberOfVtx,
	GLenum				/*inReset*/)
{
	GLint				i;
	GrVertex			v0, v1;
	
	DEBUG_SLOWPATH_ENTRY( glrRenderFlatLineStrip );

	if( inContext->primitive != GLR_LINE) glrUpdateLineSettings( inContext );
	
	if( inNumberOfVtx < 2) return;
	
	glrLoadVertex( inContext, inVtx[0], v0);
		
	for(i = 1; i < inNumberOfVtx; i++)
	{
		glrLoadVertex_Smooth( inContext, inVtx[i], v1);
		
		v0.pargb = v1.pargb;
		
		grDrawLine(&v0, &v1);
		
		v0 = v1;
	}
}




/*
________________________________________________________________________________________

      glrRenderSmoothLineStrip
________________________________________________________________________________________

*/

void
glrRenderSmoothLineStrip(
	GLDContext			inContext,
	const GLDVertex *	inVtx,
	GLint				inNumberOfVtx,
	GLenum				/*inReset*/)
{
	GLint				i;
	GrVertex			v0, v1;
	
	DEBUG_SLOWPATH_ENTRY( glrRenderSmoothLineStrip );

	if( inContext->primitive != GLR_LINE) glrUpdateLineSettings( inContext );
	
	if(inNumberOfVtx < 2) return;
	
	glrLoadVertex_Smooth( inContext, inVtx[0], v0);
	
	for(i = 1; i < inNumberOfVtx; i++)
	{
		glrLoadVertex_Smooth( inContext, inVtx[i], v1);
		
		grDrawLine(&v0, &v1);
		
		v0 = v1;
	}
}




/*
________________________________________________________________________________________

      glrRenderFlatLineLoop
________________________________________________________________________________________

*/

void
glrRenderFlatLineLoop(
	GLDContext			inContext,
	const GLDVertex *	inVtx,
	GLint				inNumberOfVtx,
	GLenum				/*inReset*/)
{
	GLint				i;
	GrVertex			v0, v1;
	
	DEBUG_SLOWPATH_ENTRY( glrRenderFlatLineLoop );

	if( inContext->primitive != GLR_LINE) glrUpdateLineSettings( inContext );
	
	if(inNumberOfVtx < 2) return;
	
	glrLoadVertex( inContext, inVtx[0], v0);
	
	for(i = 1; i < inNumberOfVtx; i++)
	{
		glrLoadVertex_Smooth( inContext, inVtx[i], v1);
		
		v0.pargb = v1.pargb;

		grDrawLine(&v0, &v1);
		
		v0 = v1;
	}
	
	glrLoadVertex_Smooth( inContext, inVtx[0], v1);
	
	v0.pargb = v1.pargb;
	
	grDrawLine(&v0, &v1);
}




/*
________________________________________________________________________________________

      glrRenderSmoothLineLoop
________________________________________________________________________________________

*/

void
glrRenderSmoothLineLoop(
	GLDContext			inContext,
	const GLDVertex *	inVtx,
	GLint				inNumberOfVtx,
	GLenum				/*inReset*/)
{
	GLint				i;
	GrVertex			v0, v1;
	
	DEBUG_SLOWPATH_ENTRY( glrRenderSmoothLineLoop );

	if( inContext->primitive != GLR_LINE) glrUpdateLineSettings( inContext );
	
	if(inNumberOfVtx < 2) return;
	
	glrLoadVertex_Smooth( inContext, inVtx[0], v0);
	
	for(i = 1; i < inNumberOfVtx; i++)
	{
		glrLoadVertex_Smooth( inContext, inVtx[i], v1);
		
		grDrawLine(&v0, &v1);
		
		v0 = v1;
	}
	
	glrLoadVertex_Smooth( inContext, inVtx[0], v1);
	
	grDrawLine(&v0, &v1);
}

