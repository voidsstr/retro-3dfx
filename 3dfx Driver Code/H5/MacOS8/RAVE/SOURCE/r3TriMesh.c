//: r3TriMesh.c
//: alt.drivers inc.
//: Glenn Nissen
//: Vadim Kochubievski
//: Rohit Gundecha


#include "r3Core.h"
#include "r3Tweaks.h"
#include "r3VertexMacros.h"

extern TRvInfo gRvEngInfo;

GrVertex			vertBuf[VERTARRAYBUFSIZ];

/////////////////////////////////
//                             //
//  RvSubmitVerticesGouraud  //
//                             //
/////////////////////////////////

void 
RvSubmitVerticesGouraud(const TQADrawContext* drawContext,
                          unsigned long         nVertices,
                          const TQAVGouraud*    v)
{
	TQADrawPrivate* dp;
	assert( drawContext->drawPrivate );
	assert( v );
	
	dp = drawContext->drawPrivate;

	AppTimerEnd();
	DrvTimerStart();
	
#ifdef TNSL_TIMER
	gRvEngInfo.drvTimer.numVertex += nVertices;
#endif

	DebugStr("--> RvSubmitVerticesGouraud - nv=");
	DebugNum(nVertices);
	dp->nVerticesGouraud = nVertices;
	dp->gouraudVertexList  = (TQAVGouraud*) v;
		
bail:
	DebugStr("\n");
	DrvTimerEnd();
	AppTimerStart();
	return;
}


/////////////////////////////////
//                             //
//  RvSubmitVerticesTexture  //
//                             //
/////////////////////////////////

void 
RvSubmitVerticesTexture(const TQADrawContext* drawContext,
                          unsigned long         nVertices,
                          const TQAVTexture*    v)
{

	TQADrawPrivate* dp = drawContext->drawPrivate;

	if ( nVertices == 0) return;
	
	assert( v != NULL );
	
	AppTimerEnd();
	DrvTimerStart();
	
#ifdef TNSL_TIMER
	gRvEngInfo.drvTimer.numVertex += nVertices;
#endif

	DebugStr("--> RvSubmitVerticesTexture - nv=");
	DebugNum(nVertices);
	
	dp->nVerticesTexture = nVertices;
	dp->textureVertexList  = (TQAVTexture*) v;

bail:
	DebugStr("\n");
	DrvTimerEnd();
	AppTimerStart();
	return;
}


//////////////////////////////
//                          //
//  RvDrawTriMeshGouraud  //
//                          //
//////////////////////////////

void RvDrawTriMeshGouraud(	const TQADrawContext*     drawContext,
                       				unsigned long             nTriangles,
                       				const TQAIndexedTriangle* triangles)
{
	TQADrawPrivate* dp = drawContext->drawPrivate;

	TQAVGouraud   	*v0;
    TQAVGouraud   	*v1;
    TQAVGouraud   	*v2;
	GrVertex gv[3];
	Boolean		bCaching = false;
	UInt32 t;

	AppTimerEnd();
	DrvTimerStart();
	
#ifdef TNSL_TIMER
	gRvEngInfo.drvTimer.numTriangle += nTriangles;
#endif

	DebugStr("\n--> RvDrawTriMeshGouraud - nT=");
	DebugNum(nTriangles);	DebugStr("\n");
	
	if (dp->gouraudVertexList == NULL) return;
	if (nTriangles < 1) return;
	
	if (dp->tChangeMask)
	{
		SetRenderState( (TQADrawContext *)drawContext, false );
	}
	if (dp->currTriType != TNSL_GOURAUD)
			SetGouraudMode(dp);
	
	v0 = &dp->gouraudVertexList[triangles[0].vertices[0]];
	v1 = &dp->gouraudVertexList[triangles[0].vertices[1]];
	v2 = &dp->gouraudVertexList[triangles[0].vertices[2]];
	dp->usingVertexAlpha = (v0->a != 1.0f) || (v1->a != 1.0f) || (v2->a != 1.0f);
	bCaching = dp->bSortingAlphaTri && dp->usingVertexAlpha;	

	DebugStr(" draw trimesh Gouraud now \n"); 
	for ( t = 0; t < nTriangles; t++ )
	{
		v0 = &dp->gouraudVertexList[triangles[t].vertices[0]];
		v1 = &dp->gouraudVertexList[triangles[t].vertices[1]];
		v2 = &dp->gouraudVertexList[triangles[t].vertices[2]];
	
		SetVertex_GouraudThree(dp, v0, v1, v2, gv);	
		if (bCaching && CacheTriGouraud( (TQADrawContext*)drawContext, &gv[0], &gv[1], &gv[2]))	
		{																		
			// DebugStr("(cached) \n");											
			continue;																
		}
			grDrawTriangle(&gv[0], &gv[1], &gv[2]); 
	}
bail:
	DebugStr("\n");
	DrvTimerEnd();
	AppTimerStart();
	return;
}


//////////////////////////////////////////
//                          			//
//  RvDrawTriMeshTextureVertexArray   //
//                          			//
//////////////////////////////////////////

void 
RvDrawTriMeshTextureVertexArray(const TQADrawContext*     drawContext,
	                       unsigned long             nTriangles,
	                       const TQAIndexedTriangle* triangles)
{
	Boolean			bCaching = false;
	int				t, c, pos = 0, nVertFitInBuf;
	TQADrawPrivate* dp = drawContext->drawPrivate;

	TQAVTexture   	*v0;
	TQAVTexture   	*v1;
	TQAVTexture   	*v2;
	GrVertex 		gv[3];
	
	const UInt32	kStride = sizeof(GrVertex);
	int				nV = nTriangles * 3;

	if (dp->textureVertexList == NULL) return;
	if (nTriangles < 1) return;
	
	assert( triangles != NULL );
	
	AppTimerEnd();
	DrvTimerStart();
	
	DebugStr("\n--> RvDrawTriMeshTextureVertexArray - nT=");
	DebugNum(nTriangles); DebugStr("\n");
	
#ifdef TNSL_TIMER
	gRvEngInfo.drvTimer.numTriangle += nTriangles;
#endif

	if (dp->tChangeMask)
	{
		SetRenderState( (TQADrawContext *)drawContext, true );
	}
	assert( dp->currBaseTexture != NULL );
	SetTextureModeAndCheckTexAlpha(dp);
	
	// grab vertices early to determine usingVertexAlpha
	v0 = &dp->textureVertexList[triangles[0].vertices[0]];
	v1 = &dp->textureVertexList[triangles[0].vertices[1]];
	v2 = &dp->textureVertexList[triangles[0].vertices[2]];
	dp->usingVertexAlpha  = ((v0->a != 1.0f) || (v1->a != 1.0f) || (v2->a != 1.0f));
	
	bCaching = dp->bSortingAlphaTri && (dp->usingVertexAlpha || dp->usingTextureAlpha[GR_TMU0]);
	DebugStr("bCaching="); DebugNum(bCaching);
	assert( dp->currBaseTexture != NULL );
	
	DebugStr(" draw trimesh texture vertex array now \n"); 
	nVertFitInBuf = min( VERTARRAYBUFSIZ, nV);
	DebugStr(" nVertFitInBuf="); DebugNum(nVertFitInBuf);
	
	for (t = 0, pos = 0; t < nTriangles; t++)
	{
		
		v0 = &dp->textureVertexList[triangles[t].vertices[0]];
		v1 = &dp->textureVertexList[triangles[t].vertices[1]];
		v2 = &dp->textureVertexList[triangles[t].vertices[2]];
	
		SetVertex_TextureThree(dp, v0, v1, v2, gv);
		
		vertBuf[pos + 0]	= gv[0];
		vertBuf[pos + 1] 	= gv[1];
		vertBuf[pos + 2]	= gv[2];
		
		if (pos + 3 == nVertFitInBuf )
		{
			if (bCaching)
			{
				// TBFL doesn't consider possibility that caching can fail.
				for (c = 0; c < nVertFitInBuf / 3; c++ )
				{
					CacheTriTexture( (TQADrawContext*)drawContext, &vertBuf[c*3], &vertBuf[c*3+1], &vertBuf[c*3+2]);
				}
				DebugStr("(cached)");
			}
			else
			{	
				grDrawVertexArrayContiguous( GR_TRIANGLES, nVertFitInBuf, vertBuf, kStride );
			}
			nV -= nVertFitInBuf;
			nVertFitInBuf = min( VERTARRAYBUFSIZ, nV);
			DebugStr(" nVertFitInBuf="); DebugNum(nVertFitInBuf);
			pos = 0;
		}
		else
		{
			pos += 3;
		}
	}
		
bail:
	DebugStr("\n");
	DrvTimerEnd();
	AppTimerStart();

	return;
}
//////////////////////////////
//                          //
//  RvDrawTriMeshTexture  //
//                          //
//////////////////////////////

void 
RvDrawTriMeshTexture(const TQADrawContext*     drawContext,
                       unsigned long             nTriangles,
                       const TQAIndexedTriangle* triangles)
{
	UInt32 t;
	Boolean			bCaching = false;
	TQADrawPrivate* dp = drawContext->drawPrivate;

	TQAVTexture   	*v0;
	TQAVTexture   	*v1;
	TQAVTexture   	*v2;
	GrVertex 		gv[3];

	if (dp->textureVertexList == NULL) return;
	if (nTriangles < 1) return;
	
	assert( triangles != NULL );
	
	AppTimerEnd();
	DrvTimerStart();
	
	DebugStr("\n--> RvDrawTriMeshTexture - nT=");
	DebugNum(nTriangles); DebugStr("\n");
	
#if !ALLOW_GLIDE_FULLSCREEN
	assert( dp->renderPort );
	assert( dp->renderSurface );
	assert( ((dp->z_bits == 0) && (dp->auxSurface == NULL)) || ((dp->z_bits > 0 ) && (dp->auxSurface != NULL)));
#endif
	assert( dp->glideContext );
	
#ifdef TNSL_TIMER
	gRvEngInfo.drvTimer.numTriangle += nTriangles;
#endif

	if (dp->tChangeMask)
	{
		SetRenderState( (TQADrawContext *)drawContext, true );
	}
	assert( dp->currBaseTexture != NULL );
	SetTextureModeAndCheckTexAlpha(dp);
	
	// grab vertices early to determine usingVertexAlpha
	v0 = &dp->textureVertexList[triangles[0].vertices[0]];
	v1 = &dp->textureVertexList[triangles[0].vertices[1]];
	v2 = &dp->textureVertexList[triangles[0].vertices[2]];
	dp->usingVertexAlpha  = ((v0->a != 1.0f) || (v1->a != 1.0f) || (v2->a != 1.0f));
	bCaching = dp->bSortingAlphaTri && (dp->usingVertexAlpha || dp->usingTextureAlpha[GR_TMU0]);

	assert( dp->currBaseTexture != NULL );
	
	DebugStr(" draw trimesh texture now \n"); 
	for ( t = 0; t < nTriangles; t++ )
	{
		v0 = &dp->textureVertexList[triangles[t].vertices[0]];
		v1 = &dp->textureVertexList[triangles[t].vertices[1]];
		v2 = &dp->textureVertexList[triangles[t].vertices[2]];
	
		DbgRaveVertexTexture( v0, dp->currBaseTexture );
		DbgRaveVertexTexture( v1, dp->currBaseTexture );
		DbgRaveVertexTexture( v2, dp->currBaseTexture );
	
		SetVertex_TextureThree(dp, v0, v1, v2, gv);
		
		DbgGlideVertexTexture( gv[0] );
		DbgGlideVertexTexture( gv[1] );
		DbgGlideVertexTexture( gv[2] );
		
#if	TNSL_DEBUG_QZ
		// samples showing relation between z & q
		if ( t % 200 == 0 )
		{
			DebugStr( "\n z="); DebugFloat( v0->z );
			DebugStr( " q=oow=" );  DebugFloat( gv[0].oow );
			DebugStr("\n");
		}
#endif
		if (bCaching && CacheTriTexture( (TQADrawContext*)drawContext, &gv[0], &gv[1], &gv[2]))	
		{																		
			// DebugStr("(cached) \n");											
			continue;																
		}																		
			grDrawTriangle(&gv[0], &gv[1], &gv[2]); 
		
	}
		

bail:
	DebugStr("\n");
	DrvTimerEnd();
	AppTimerStart();

	return;
}
