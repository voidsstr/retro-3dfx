//: r3Draw.c
//: alt.drivers inc.
//: Glenn Nissen
//: Vadim Kochubievski
//: Rohit Gundecha


#include "r3Core.h"
#include "r3Tweaks.h"
#include "r3Context.h"
#include "r3VertexMacros.h"
#include <stdlib.h>

extern Boolean bNanosaurRunning;
extern Boolean bBugdomRunning;
extern Boolean bATVRunning;

extern TRvInfo gRvEngInfo;
extern GrVertex			vertBuf[VERTARRAYBUFSIZ];

static void DrawBitmapAsTexture(const TQADrawContext* drawContext,
					               const TQAVGouraud*    v,
					               TQABitmap*            bitmap);
					               
static void DrawBitmapWithLFBWrite(const TQADrawContext* drawContext,
						               const TQAVGouraud*    v,
						               TQABitmap*            bitmap);

static float CalcMaxZ( GrVertex* v0, GrVertex* v1, GrVertex* v2 );
static float CalcAvgZ( GrVertex* v0, GrVertex* v1, GrVertex* v2 );
static int CompareTriDepths( const void *index1, const void *index2 );
static void WrapGlideGetState( TQADrawPrivate *dp);



/////////////////////
//                 //
//  RvDrawPoint  //
//                 //
/////////////////////

void 
RvDrawPoint(const TQADrawContext* drawContext,
              const TQAVGouraud*    v)
{
	TQADrawPrivate* dp = drawContext->drawPrivate;

	GrVertex gv;
	
	AppTimerEnd();
	DrvTimerStart();
	
	DebugStr("--> RvDrawPoint - ");
	
#ifdef TNSL_TIMER
	gRvEngInfo.drvTimer.numVertex += 1;
#endif
	if (dp->tChangeMask)
	{
		SetRenderState( (TQADrawContext *)drawContext, false );
	}
	if (dp->currTriType != TNSL_GOURAUD)
	{
		SetGouraudMode( dp );
	}
	SetVertex_GouraudSingle(dp, v, &gv);

	DevTimerStart();
	grDrawPoint(&gv);
	DevTimerEnd();

bail:
	DebugStr("\n");
	DrvTimerEnd();
	AppTimerStart();
	return;
}


////////////////////
//                //
//  RvDrawLine  //
//                //
////////////////////

void 
RvDrawLine(const TQADrawContext* drawContext,
             const TQAVGouraud*    v0,
             const TQAVGouraud*    v1)
{
	TQADrawPrivate* dp = drawContext->drawPrivate;
	
	GrVertex gv0;
	GrVertex gv1;

	AppTimerEnd();
	DrvTimerStart();
	
#ifdef TNSL_TIMER
	gRvEngInfo.drvTimer.numVertex += 2;
#endif

	DebugStr("--> RvDrawLine - ");
	
	if (dp->tChangeMask)
	{
		SetRenderState( (TQADrawContext *)drawContext, false );
	}
	if (dp->currTriType != TNSL_GOURAUD)
		SetGouraudMode( dp );

	SetVertex_GouraudSingle(dp, v0, &gv0);
	SetVertex_GouraudSingle(dp, v1, &gv1);
	
	DevTimerStart();
	grDrawLine(&gv0, &gv1);
	DevTimerEnd();

bail:
	DebugStr("\n");
	DrvTimerEnd();
	AppTimerStart();
	return;
}


//////////////////////////
//                      //
//  RvDrawTriGouraud  //
//                      //
//////////////////////////

void 
RvDrawTriGouraud(const TQADrawContext* drawContext,
                   const TQAVGouraud*    v0,
                   const TQAVGouraud*    v1,
                   const TQAVGouraud*    v2,
                   unsigned long         flags)
{
	TQADrawPrivate* dp = drawContext->drawPrivate;

	GrVertex gv[3];

	AppTimerEnd();
	DrvTimerStart();
	
#ifdef TNSL_TIMER
	gRvEngInfo.drvTimer.numVertex   += 3;
	gRvEngInfo.drvTimer.numTriangle += 1;
#endif

	DebugStr("--> RvDrawTriGouraud - n=3 ");
	
	if (dp->tChangeMask)
	{
		SetRenderState( (TQADrawContext *)drawContext, false );
	}
	if (dp->currTriType != TNSL_GOURAUD)
	{
		SetGouraudMode( dp );
	}
	SetVertex_GouraudThree(dp, v0, v1, v2, gv);
	dp->usingVertexAlpha = (v0->a != 1.0f) || (v1->a != 1.0f) || (v2->a != 1.0f);
	DebugStr(" usingVertexAlpha =" ); DebugNum(dp->usingVertexAlpha);
	if (dp->bSortingAlphaTri && dp->usingVertexAlpha 						
		&& CacheTriGouraud( (TQADrawContext*)drawContext, &gv[0], &gv[1], &gv[2]))	
	{																		
		DebugStr("(cached) \n");											
		goto bail;																
	}																		
		grDrawTriangle(&gv[0], &gv[1], &gv[2]); 

bail:
	DebugStr("\n");
	DrvTimerEnd();
	AppTimerStart();
	return;
#pragma unused(flags)
}


//////////////////////////
//                      //
//  RvDrawTriTexture  //
//                      //
//////////////////////////

void 
RvDrawTriTexture(const TQADrawContext* drawContext,
                   const TQAVTexture*    v0,
                   const TQAVTexture*    v1,
                   const TQAVTexture*    v2,
                   unsigned long         flags)
{
	#pragma unused ( flags )
	
	TQADrawPrivate* dp = drawContext->drawPrivate;

    
	GrVertex gv[3];

	AppTimerEnd();
	DrvTimerStart();
	
#ifdef TNSL_TIMER
	gRvEngInfo.drvTimer.numVertex   += 3;
	gRvEngInfo.drvTimer.numTriangle += 1;
#endif

	DebugStr("--> RvDrawTriTexture - n = 3  tChangeMask="); DebugHex(dp->tChangeMask);
	
	if ((dp->tChangeMask) || (dp->currTriType != TNSL_TEXTURE))
	{
		SetRenderState( (TQADrawContext *)drawContext, true );
		SetTextureModeAndCheckTexAlpha(dp);
	}
	/*
	if ((dp->currTriType != TNSL_TEXTURE) || (dp->currBaseTexture != dp->lastBaseTextureProcessed))
	{
	}
	*/
	assert( dp->currBaseTexture );
	
	dp->usingVertexAlpha = (v0->a != 1.0f) || (v1->a != 1.0f) || (v2->a != 1.0f);
	// DebugStr(" usingVertexAlpha ="); DebugNum(dp->usingVertexAlpha);
	
	DbgRaveVertexTexture( v0, dp->currBaseTexture );
	DbgRaveVertexTexture( v1, dp->currBaseTexture );
	DbgRaveVertexTexture( v2, dp->currBaseTexture );
	
	SetVertex_TextureThree(dp, v0, v1, v2, gv);
	
	DbgGlideVertexTexture( gv[0] );
	DbgGlideVertexTexture( gv[1] );
	DbgGlideVertexTexture( gv[2] );
	
	DebugStr("\n draw TriTexture now ");
	DevTimerStart(); 
	
	if ( dp->bSortingAlphaTri && (dp->usingVertexAlpha || dp->usingTextureAlpha[GR_TMU0] )
					&& CacheTriTexture( (TQADrawContext*)drawContext, &gv[0], &gv[1], &gv[2] ))
	{
		// DebugStr("(cached) \n");
		goto bail;		// draw at RenderEnd
	}
		
	assert( dp->currBaseTexture );
	grDrawTriangle(&gv[0], &gv[1], &gv[2]);
	
	DevTimerEnd();

bail:
	DebugStr("\n");
	DrvTimerEnd();
	AppTimerStart();
	return;
}


////////////////////////
//                    //
//  RvDrawVGouraud  //
//                    //
////////////////////////

void 
RvDrawVGouraud(const TQADrawContext* drawContext,
                 unsigned long         nVertices,
                 TQAVertexMode         vertexMode,
                 const TQAVGouraud     v[],
                 const unsigned long   flags[])
{
	TQADrawPrivate* dp = drawContext->drawPrivate;

	GrVertex gv[3];

	UInt32 ii;
	UInt32 jj;
	TQAVGouraud   	*v0;
    TQAVGouraud   	*v1;
    TQAVGouraud   	*v2;
    TQAVGouraud   	*vPtr;
    TQAVGouraud   	*vLimit;
	unsigned long	*flag;
	Boolean			bCaching = false;

	AppTimerEnd();
	DrvTimerStart();
	
#ifdef TNSL_TIMER
	gRvEngInfo.drvTimer.numVertex += nVertices;
#endif

	DebugStr("--> RvDrawVGouraud - ");
	DebugNum(nVertices);  DebugStr("\n");
	
	if (nVertices < 1)
		return;
	assert( v != NULL);

	if(vertexMode >= kQAVertexMode_NumModes) {
		DebugStr("*** unexpected vertexMode");
		goto bail;
	}

    vPtr 	= (TQAVGouraud *)v;
    vLimit 	= vPtr + nVertices;
    flag	= (unsigned long *)flags;
    	
	/*
	if (dp->tChangeMask || (dp->currTriType != TNSL_GOURAUD))
	{
		if (dp->tChangeMask)
			SetRenderState( (TQADrawContext *)drawContext, false );
		if (dp->currTriType != TNSL_GOURAUD)
			SetGouraudMode(dp);
	}
	*/
	if (dp->tChangeMask)
	{
		SetRenderState( (TQADrawContext *)drawContext, false );
	}
	if (dp->currTriType != TNSL_GOURAUD)
	{
		SetGouraudMode(dp);
	}

	switch(vertexMode) 
	{
	case kQAVertexMode_Point:
		DebugStr(" draw VGouraud Point now ");
		for(ii = 0; ii < nVertices; ++ii) {
			SetVertex_GouraudSingle(dp,  &v[ii], &gv[0]);
			
			DevTimerStart();
			grDrawPoint(&gv[0]);
			DevTimerEnd();
		}
		break;

	case kQAVertexMode_Line:
		DebugStr(" draw VGouraud Line now ");
		for(jj = 0, ii = 0; ii < (nVertices / 2); jj += 2, ++ii) {
			SetVertex_GouraudSingle(dp,  &v[jj + 0], &gv[0]);
			SetVertex_GouraudSingle(dp,  &v[jj + 1], &gv[1]);
			
			DevTimerStart();
			grDrawLine(&gv[0], &gv[1]);
			DevTimerEnd();
		}
		break;

	case kQAVertexMode_Polyline:
		DebugStr(" draw VGouraud Polyline now ");
		for(ii = 0; ii < (nVertices - 1); ++ii) {
			SetVertex_GouraudSingle(dp,  &v[ii + 0], &gv[0]);
			SetVertex_GouraudSingle(dp,  &v[ii + 1], &gv[1]);
			
			DevTimerStart();
			grDrawLine(&gv[0], &gv[1]);
			DevTimerEnd();
		}
		break;

	case kQAVertexMode_Tri:
		DebugStr(" draw VGouraud Tri now ");
#ifdef TNSL_TIMER
		gRvEngInfo.drvTimer.numTriangle += nVertices / 3;
#endif

		for ( ; vPtr < vLimit - 2;  )
		{
			v0 = vPtr++;
			v1 = vPtr++;
			v2 = vPtr++;

			SetVertex_GouraudThree(dp, v0, v1, v2, gv);
			dp->usingVertexAlpha = (v0->a != 1.0f) || (v1->a != 1.0f) || (v2->a != 1.0f);
			if (dp->bSortingAlphaTri && dp->usingVertexAlpha 						
				&& CacheTriGouraud( (TQADrawContext*)drawContext, &gv[0], &gv[1], &gv[2]))	
			{																		
				DebugStr("(cached) \n");											
				continue;																
			}																		
				grDrawTriangle(&gv[0], &gv[1], &gv[2]); 
		}
		break;

	case kQAVertexMode_Strip:
		DebugStr(" draw VGouraud Strip now ");
#ifdef TNSL_TIMER
		gRvEngInfo.drvTimer.numTriangle += nVertices - 2;
#endif
		if ( nVertices < 3 )
			goto bail;
			
		v0 = vPtr++;
		v1 = vPtr++;
		v2 = vPtr++;
			
			SetVertex_GouraudThree(dp, v0, v1, v2, gv);
			dp->usingVertexAlpha = (v0->a != 1.0f) || (v1->a != 1.0f) || (v2->a != 1.0f);
			bCaching = dp->bSortingAlphaTri && dp->usingVertexAlpha;
			if (bCaching && CacheTriGouraud( (TQADrawContext*)drawContext, &gv[0], &gv[1], &gv[2]))	
			{																		
				DebugStr("(cached) \n");											
				goto main_loop;																
			}																		
		grDrawTriangle(&gv[0], &gv[1], &gv[2]); 
		
main_loop:
		for ( ii = 0; vPtr < vLimit; )
		{
			if ( ii == 0 )
			{
				v0 = vPtr++;
				ii++;
			}
			else if ( ii == 1 )
			{
				v1 = vPtr++;
				ii++;
			}
			else if ( ii == 2 )
			{
				v2 = vPtr++;
				ii = 0;
			}
			
			SetVertex_GouraudThree(dp, v0, v1, v2, gv);
			if (bCaching && CacheTriGouraud( (TQADrawContext*)drawContext, &gv[0], &gv[1], &gv[2]))	
			{																		
				DebugStr("(cached) \n");											
				continue;																
			}																		
			grDrawTriangle(&gv[0], &gv[1], &gv[2]); 
		}
		break;

  case kQAVertexMode_Fan:
		DebugStr(" draw VGouraud Fan now ");
		if(nVertices < 3) break;
#ifdef TNSL_TIMER
		gRvEngInfo.drvTimer.numTriangle += nVertices - 2;
#endif
		v0 = vPtr++;
		v1 = vPtr++;
		v2 = vPtr++;
		
		SetVertex_GouraudThree(dp, v0, v1, v2, gv);
		dp->usingVertexAlpha = (v0->a != 1.0f) || (v1->a != 1.0f) || (v2->a != 1.0f);
		bCaching = dp->bSortingAlphaTri && dp->usingVertexAlpha;	
		if (bCaching && CacheTriGouraud( (TQADrawContext*)drawContext, &gv[0], &gv[1], &gv[2]))	
		{																		
			DebugStr("(cached) \n");											
			goto main_loop_fan;																
		}																		
		grDrawTriangle(&gv[0], &gv[1], &gv[2]); 
		DebugStr("\ndrew first tri of fan\n"); 
		
main_loop_fan:
		for ( ii = 0; vPtr < vLimit; )
		{
			if ( ii == 0 )
			{
				v1 = vPtr++;
				ii++;
			}
			else
			{
				v2 = vPtr++;
				ii = 0;
			}
			
			SetVertex_GouraudThree(dp, v0, v1, v2, gv);
			if (bCaching && CacheTriGouraud( (TQADrawContext*)drawContext, &gv[0], &gv[1], &gv[2]))	
			{																		
				DebugStr("(cached) \n");											
				continue;																
			}																		
			grDrawTriangle(&gv[0], &gv[1], &gv[2]); 
			DebugStr(" drew next tri of fan\n");
			
		}
		break;
	}

bail:
	DebugStr("\n");
	DrvTimerEnd();
	AppTimerStart();
	return;
#pragma unused(flags)
}

////////////////////////
//                    //
//  RvDrawVTexture  //
//                    //
////////////////////////

void 
RvDrawVTexture(const TQADrawContext* drawContext,
                 unsigned long         nVertices,
                 TQAVertexMode         vertexMode,
                 const TQAVTexture     v[],
                 const unsigned long   flags[])
{
	TQADrawPrivate* dp = drawContext->drawPrivate;

	GrVertex gv[3];		// , *vFresh;

	UInt32 ii;
	unsigned long	*flag;
	Boolean			bCaching = false;
	TQAVTexture   	*v0;
    TQAVTexture   	*v1;
    TQAVTexture   	*v2;
    TQAVTexture   	*vPtr;
    TQAVTexture   	*vLimit;
	const UInt32	kStride = sizeof(GrVertex);
	int				c, pos = 0, nVertFitInBuf;
	short			whichFresh = 0;

    vPtr 	= (TQAVTexture *)v;
    vLimit 	= vPtr + nVertices;
   	flag	= (unsigned long *)flags;
   	
	AppTimerEnd();
	DrvTimerStart();
#ifdef TNSL_TIMER
	gRvEngInfo.drvTimer.numVertex += nVertices;
#endif

	DebugStr("--> RvDrawVTexture - n="); 
	DebugNum(nVertices); DebugStr("\n");
	
	if (nVertices < 1)
		return;
	assert( v != NULL);
	nVertFitInBuf = min( VERTARRAYBUFSIZ, nVertices);

	if(vertexMode >= kQAVertexMode_NumModes) {
		DebugStr("*** unexpected vertexMode");
		goto bail;
	}
	
	if (dp->tChangeMask)
	{
		SetRenderState( (TQADrawContext *)drawContext, true );
	}
	if ((dp->currTriType != TNSL_TEXTURE) || (dp->currBaseTexture != dp->lastBaseTextureProcessed))
		SetTextureModeAndCheckTexAlpha(dp);

	assert( dp->currBaseTexture != NULL );

	
	switch(vertexMode) {
	
	case kQAVertexMode_Point:
		DebugStr(" draw VTexture Point now ");
		for(v0 = vPtr; v0 < vLimit; v0++) 
		{
			SetVertex_TextureSingle(dp,  v0, &gv[0]);
			
			DevTimerStart();
			grDrawPoint(&gv[0]);
			DevTimerEnd();
		}
		break;

	case kQAVertexMode_Line:
		DebugStr(" draw VTexture Line now ");
		for( ; vPtr < vLimit-1;  )
		{
			v0 = vPtr++;
			v1 = vPtr++;
			
			SetVertex_TextureSingle(dp,  v0, &gv[0]);
			SetVertex_TextureSingle(dp,  v1, &gv[1]);
			
			DevTimerStart();
			grDrawLine(&gv[0], &gv[1]);
			DevTimerEnd();
		}
	    break;

	case kQAVertexMode_Polyline:
		DebugStr(" draw VTexture Polyline now ");
		if ( vPtr < vLimit )
			v1 = vPtr++;
		
		for( ; vPtr < vLimit; )
		{
			v0 = v1;
			v1 = vPtr++;
			
			SetVertex_TextureSingle(dp,  v0, &gv[0]);
			SetVertex_TextureSingle(dp,  v1, &gv[1]);
			DevTimerStart();
			grDrawLine(&gv[0], &gv[1]);
			DevTimerEnd();
		}
		break;

	case kQAVertexMode_Tri:
		DebugStr(" draw VTexture Triangle now ");
#ifdef TNSL_TIMER
		gRvEngInfo.drvTimer.numTriangle += nVertices / 3;
#endif
		dp->usingVertexAlpha = (vPtr->a != 1.0f) || ((vPtr+1)->a != 1.0f) || ((vPtr+2)->a != 1.0f);
		bCaching = dp->bSortingAlphaTri && (dp->usingVertexAlpha || dp->usingTextureAlpha[GR_TMU0]);
		DebugStr("bCaching="); DebugNum(bCaching);
		for( pos = 0 ; vPtr < vLimit-2; )
		{
			v0 = vPtr++;
			v1 = vPtr++;
			v2 = vPtr++;
			
			DbgRaveVertexTexture( v0, dp->currBaseTexture );
			DbgRaveVertexTexture( v1, dp->currBaseTexture );
			DbgRaveVertexTexture( v2, dp->currBaseTexture );
	
			SetVertex_TextureThree(dp, v0, v1, v2, gv);
			
			DbgGlideVertexTexture( gv[0] );
			DbgGlideVertexTexture( gv[1] );
			DbgGlideVertexTexture( gv[2] );
			
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
						CacheTriTexture( (TQADrawContext*)drawContext, 
										&vertBuf[c*3+0], &vertBuf[c*3+1], &vertBuf[c*3+2]);
						DebugStr("(cached from vertex buffer) \n");
					}											
				}
				else
				{
					grDrawVertexArrayContiguous( GR_TRIANGLES, nVertFitInBuf, vertBuf, kStride );
					DebugStr("(grDrawVertexArrayContiguous( GR_TRIANGLES) ");
				}
				nVertices -= nVertFitInBuf;
				nVertFitInBuf = min( VERTARRAYBUFSIZ, nVertices);
				pos = 0;
			}
			else
			{
				pos += 3;
			}
		}
		break;

	case kQAVertexMode_Strip:
	
		DebugStr(" draw VTexture Strip now ");
#ifdef TNSL_TIMER
		gRvEngInfo.drvTimer.numTriangle += nVertices - 2;
#endif
		if ( nVertices < 3 )
			goto bail;
			
		v0 = vPtr++;
		v1 = vPtr++;
		v2 = vPtr++;

		dp->usingVertexAlpha = (v0->a != 1.0f) || (v1->a != 1.0f) || (v2->a != 1.0f);
		bCaching = dp->bSortingAlphaTri && (dp->usingVertexAlpha || dp->usingTextureAlpha[GR_TMU0]);
		DebugStr("bCaching="); DebugNum(bCaching);
			
		SetVertex_TextureThree(dp, v0, v1, v2, gv);
		if (bCaching && CacheTriTexture( (TQADrawContext*)drawContext, &gv[0], &gv[1], &gv[2]))	
		{																		
			DebugStr("(cached) \n");											
			goto main_loop_tstrip;																
		}																		
		grDrawTriangle(&gv[0], &gv[1], &gv[2]); 
		DebugStr("\ndrew first tri of strip\n");
		
main_loop_tstrip:
		for ( ii = 0; vPtr < vLimit; )
		{
			if ( ii == 0 )
			{
				v0 = vPtr++;
				ii++;
			}
			else if ( ii == 1 )
			{
				v1 = vPtr++;
				ii++;
			}
			else
			{
				v2 = vPtr++;
				ii = 0;
			}
			
			SetVertex_TextureThree(dp, v0, v1, v2, gv);
	assert( (&gv[0].sow0 != &gv[1].sow0) || (&gv[0].sow0 != &gv[2].sow0) || (&gv[1].sow0 != &gv[2].sow0));
	assert( (&gv[0].tow0 != &gv[1].tow0) || (&gv[0].tow0 != &gv[2].tow0) || (&gv[1].tow0 != &gv[2].tow0));
			if (bCaching && CacheTriTexture( (TQADrawContext*)drawContext, &gv[0], &gv[1], &gv[2]))	
			{																		
				DebugStr("(cached) \n");											
				continue;																
			}																		
			grDrawTriangle(&gv[0], &gv[1], &gv[2]);
			DebugStr("\ndrew next tri of strip\n"); 
			
		}
		break;
		
	case kQAVertexMode_Fan:
	
		DebugStr(" draw VTexture Fan now ");
		if (nVertices < 3) goto bail;
		assert( dp->currBaseTexture->alphaBits <= 8 );
		
#ifdef TNSL_TIMER
		gRvEngInfo.drvTimer.numTriangle += nVertices - 2;
#endif

		v0 = vPtr++;
		v1 = vPtr++;
		v2 = vPtr++;
		
		dp->usingVertexAlpha = (v0->a != 1.0f) || (v1->a != 1.0f) || (v2->a != 1.0f);
		bCaching = dp->bSortingAlphaTri && (dp->usingVertexAlpha || dp->usingTextureAlpha[GR_TMU0]);
		DebugStr("bCaching="); DebugNum(bCaching);
			
		SetVertex_TextureThree(dp, v0, v1, v2, gv);
			
		if (bCaching && CacheTriTexture( (TQADrawContext*)drawContext, &gv[0], &gv[1], &gv[2]))	
		{																		
			DebugStr("(cached) \n");											
			goto main_loop_tfan;																
		}																		
		grDrawTriangle(&gv[0], &gv[1], &gv[2]); 
		DebugStr("\ndrew first tri of fan\n"); 
		
main_loop_tfan:
		for ( ii = 0; vPtr < vLimit; )
		{
			if ( ii == 0 )
			{
				v1 = vPtr++;
				ii++;
			}
			else
			{
				v2 = vPtr++;
				ii = 0;
			}
			
			SetVertex_TextureThree(dp, v0, v1, v2, gv);
	// assert( (&gv[0].sow0 != &gv[1].sow0) || (&gv[0].sow0 != &gv[2].sow0) || (&gv[1].sow0 != &gv[2].sow0));
	// assert( (&gv[0].tow0 != &gv[1].tow0) || (&gv[0].tow0 != &gv[2].tow0) || (&gv[1].tow0 != &gv[2].tow0));

			if (bCaching && CacheTriTexture( (TQADrawContext*)drawContext, &gv[0], &gv[1], &gv[2]))	
			{																		
				DebugStr("(cached) \n");											
				continue;																
			}																		
			grDrawTriangle(&gv[0], &gv[1], &gv[2]); 
			DebugStr(" drew next tri of fan\n");
			
		}
		break;
		
		default:
			break;
	}

bail:
	DebugStr("\n\n");
	DrvTimerEnd();
	AppTimerStart();
	return;
#pragma unused(flags)
}


//////////////////////
//                  //
//  RvDrawBitmap  //
//                  //
//////////////////////

void 
RvDrawBitmap(const TQADrawContext* drawContext,
               const TQAVGouraud*    v,
               TQABitmap*            bitmap)
{
	DrawBitmapAsTexture( drawContext, v, bitmap );
}

#pragma mark -
#pragma mark /* internal bitmap draw routines */
void 
DrawBitmapAsTexture(const TQADrawContext* drawContext,
               const TQAVGouraud*    v,
               TQABitmap*            bitmap)
{
	float xScale, yScale;
	UInt32		saveDepthFunc, saveFiltering = 0, saveGLmin, saveGLmag, saveTexWrapU, saveTexWrapV, saveFogMode;
	UInt32		saveTexOp , saveAlphaTestFunc;
	float		saveAlphaRef;
	float x,y;
	GrVertex gv[3];

	BmPiece	*bmpc;
	int 	i;
	TQATexture	*tex = NULL, *saveCurrTex;
	Rect		r;
	
	TQADrawPrivate *dp = drawContext->drawPrivate;

							    // x	 y	 z	 invW   r	 g	  b    a uOverW vOverW kd_r kd_g kd_b ks_r ks_g ks_b
	TQAVTexture bitmVertex[4] = {{0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0,   1.0, 1.0, 1.0, 0.0, 0.0, 0.0},
							     {0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 1.0, 0.0,   1.0, 1.0, 1.0, 0.0, 0.0, 0.0},
							     {0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 1.0,   1.0, 1.0, 1.0, 0.0, 0.0, 0.0},
							     {0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0,   1.0, 1.0, 1.0, 0.0, 0.0, 0.0} };
	AppTimerEnd();
	DrvTimerStart();
	
	DebugStr("\n\n--> RvDrawBitmap - "); DebugHex((UInt32)bitmap);
	
	if (bitmap == NULL)
	{
		DebugStr(" bitmap NULL, abort ?\n");
		return;
	}
	
	// set states for bitmaps
	DebugStr(" setting bitmap states...");
	// save current texture
	saveCurrTex = (TQATexture *)dp->tState[kQATag_Texture].p;
	// make z-compare always succeed.
	// If this is not done, text in Combat Mission doesn't show up, and 
	// the cursor disappears behind the Quit dialog.
	// The Rave spec doesn't say how to set this.
	// If None, there is no z-compare and the z-buffer is not written. This means that subsequent tris could overwrite.
	// If True, z test always passes and the z-buffer is written. Performance gain if subsequent tris try to overwrite.
	// saveDepthFunc = dp->tState[kQATag_ZFunction].i;
	// temp 10/19 RvSetInt( (TQADrawContext *)drawContext, kQATag_ZFunction, kQAZFunction_True);
	
	// turn off filtering
	saveGLmin = dp->tState[kQATagGL_TextureMinFilter].i;
	saveGLmag = dp->tState[kQATagGL_TextureMagFilter].i;
	saveFiltering = dp->tState[kQATag_TextureFilter].i;
	RvSetInt( (TQADrawContext *)drawContext,  kQATagGL_TextureMinFilter, 0);
	RvSetInt( (TQADrawContext *)drawContext,  kQATagGL_TextureMagFilter, 0);
	RvSetInt( (TQADrawContext *)drawContext, kQATag_TextureFilter, kQATextureFilter_Fast );
	
	// turn off alpha testing (why, what if we don't?) (Kawasaki's bitmaps get messed up if we don't.)
	saveAlphaTestFunc 	= dp->tState[kQATag_AlphaTestFunc].i;
	saveAlphaRef		= dp->tState[kQATag_AlphaTestRef].f;
	if (bATVRunning)
	{
		RvSetInt( (TQADrawContext *)drawContext, kQATag_AlphaTestFunc, kQAAlphaTest_GT);
		RvSetFloat((TQADrawContext *)drawContext, kQATag_AlphaTestRef, 0.0);
		grAlphaBlendFunction(GR_BLEND_SRC_ALPHA, GR_BLEND_ONE_MINUS_SRC_ALPHA, GR_BLEND_ONE, GR_BLEND_ZERO);

	}
	
	// turn off fog
	saveFogMode = dp->tState[kQATag_FogMode].i;
	RvSetInt( (TQADrawContext *)drawContext, kQATag_FogMode, kQAFogMode_None );
	
	// set texture clamping
	saveTexWrapU = dp->tState[kQATagGL_TextureWrapU].i;
	saveTexWrapV = dp->tState[kQATagGL_TextureWrapV].i;
	RvSetInt( (TQADrawContext *)drawContext, kQATagGL_TextureWrapU, kQAGL_Clamp);
	RvSetInt( (TQADrawContext *)drawContext, kQATagGL_TextureWrapV, kQAGL_Clamp);
	
    xScale = dp->tState[kQATag_BitmapScale_x].f;
    yScale = dp->tState[kQATag_BitmapScale_y].f;
	
	// loop thru all pieces of the bitmap
	bmpc = bitmap->pieces;
	
	for (i = 0; i < bitmap->numPieces; i++, bmpc++)
	{
		tex = bmpc->texPiece;
		r = bmpc->rectPow;
		assert( tex );
		assert( r.top >= 0 && r.top < 2000 );						// assuming no scr res >= 2000
		DebugStr( "\n draw bitmap piece "); DebugNum(i+1); 
		DebugStr( " of "); DebugNum( bitmap->numPieces);
		DebugStr( " tex format "); DebugNum( tex->glideTextureInfo.format );
	
	    x = v->x;
	    x += (float)r.left;
	    y = v->y;
	    y += (float)r.top;
	    // quick initial assignments
	    bitmVertex[0].x = bitmVertex[1].x = bitmVertex[2].x = bitmVertex[3].x = x;
	    bitmVertex[0].y = bitmVertex[1].y = bitmVertex[2].y = bitmVertex[3].y = y;
	    bitmVertex[0].z = bitmVertex[1].z = bitmVertex[2].z = bitmVertex[3].z = v->z;
		bitmVertex[0].a = bitmVertex[1].a = bitmVertex[2].a = bitmVertex[3].a = v->a;
	    bitmVertex[0].r = bitmVertex[1].r = bitmVertex[2].r = bitmVertex[3].r = v->r;
	    bitmVertex[0].g = bitmVertex[1].g = bitmVertex[2].g = bitmVertex[3].g = v->g;
	    bitmVertex[0].b = bitmVertex[1].b = bitmVertex[2].b = bitmVertex[3].b = v->b;
	    
	    /*
	     * fix up vertices. we assume that the vertices are numbered
	     * with the original vertex at the upper left, like this, and
	     * bitmaps scaling is done around upper left corner:  
	     *
	     *  0   1
	     *  2   3
		 */    
		bitmVertex[0].uOverW = bitmap->pieces[i].bitmapBiasU;
		bitmVertex[0].vOverW = bitmap->pieces[i].bitmapVoverW;
		
	    bitmVertex[1].x     += bitmap->pieces[i].bitmapWidth * xScale;
	    bitmVertex[1].uOverW = bitmap->pieces[i].bitmapUoverW;
	    bitmVertex[1].vOverW = bitmap->pieces[i].bitmapVoverW;
		
	    bitmVertex[2].y     += bitmap->pieces[i].bitmapHeight * yScale;
	    bitmVertex[2].uOverW = bitmap->pieces[i].bitmapBiasU;
	    bitmVertex[2].vOverW = bitmap->pieces[i].bitmapBiasV;  
	
	    bitmVertex[3].x     += bitmap->pieces[i].bitmapWidth * xScale;
	    bitmVertex[3].y     += bitmap->pieces[i].bitmapHeight * yScale;
	    bitmVertex[3].uOverW = bitmap->pieces[i].bitmapUoverW;
	    bitmVertex[3].vOverW = bitmap->pieces[i].bitmapBiasV;
	
		RvSetPtr( (TQADrawContext *)drawContext, kQATag_Texture, (const void*)tex);
	
		/*
		 * From RAVE ERS:
		 *  kQAPixel_Alpha1: A 1 bit/pixel bitmap. Bits that are 0 are fully transparent; bits that
		 *  are 1 are rendered in the color passed to QADrawBitmap().
		 */
		if (tex->pixelType  == kQAPixel_Alpha1 )
		{
			bitmVertex[0].kd_r = bitmVertex[1].kd_r = bitmVertex[2].kd_r = bitmVertex[3].kd_r = v->r;
		    bitmVertex[0].kd_g = bitmVertex[1].kd_g = bitmVertex[2].kd_g = bitmVertex[3].kd_g = v->g;
		    bitmVertex[0].kd_b = bitmVertex[1].kd_b = bitmVertex[2].kd_b = bitmVertex[3].kd_b = v->b;
			saveTexOp = dp->tState[kQATag_TextureOp].i;
			RvSetInt( (TQADrawContext *)drawContext, kQATag_TextureOp, kQATextureOp_Modulate );
			DebugStr(" special handling of kQAPixel_Alpha1 (modulate) ");
		}


		if (dp->tChangeMask)
		{
			SetRenderState( (TQADrawContext *)drawContext, true );
		}
		if (i == 0)
		{
			SetTextureModeAndCheckTexAlpha( dp );
		}
	
		DbgRaveVertexTexture( &bitmVertex[0], dp->currBaseTexture );
		DbgRaveVertexTexture( &bitmVertex[2], dp->currBaseTexture );
		DbgRaveVertexTexture( &bitmVertex[3], dp->currBaseTexture );
		
		SetVertex_TextureThree(dp, &bitmVertex[0], &bitmVertex[2], &bitmVertex[3], gv);
		
		DbgGlideVertexTexture( gv[0] );
		DbgGlideVertexTexture( gv[1] );
		DbgGlideVertexTexture( gv[2] );
		
		DebugStr(" "); DebugStr("top "); DebugNum( r.top ); DebugStr(" left "); DebugNum( r.left );
		DebugStr(" bot "); DebugNum( r.bottom ); DebugStr(" right "); DebugNum( r.right ); DebugStr(" now ");
		
		grDrawTriangle( &gv[0], &gv[1], &gv[2]);
		
		SetVertex_TextureThree(dp, &bitmVertex[0], &bitmVertex[3], &bitmVertex[1], gv);
		grDrawTriangle( &gv[0], &gv[1], &gv[2]);
		
	}
	RvSetInt( (TQADrawContext *)drawContext, kQATag_TextureFilter, saveFiltering );
	RvSetInt( (TQADrawContext *)drawContext,  kQATagGL_TextureMinFilter, saveGLmin);
	RvSetInt( (TQADrawContext *)drawContext,  kQATagGL_TextureMagFilter, saveGLmag);
	RvSetInt( (TQADrawContext *)drawContext, kQATag_AlphaTestFunc, saveAlphaTestFunc );
	RvSetFloat((TQADrawContext *)drawContext, kQATag_AlphaTestRef, saveAlphaRef	);

	RvSetInt( (TQADrawContext *)drawContext, kQATagGL_TextureWrapU, saveTexWrapU);
	RvSetInt( (TQADrawContext *)drawContext, kQATagGL_TextureWrapV, saveTexWrapV);
	// RvSetInt( (TQADrawContext *)drawContext, kQATag_ZFunction, saveDepthFunc);
	RvSetInt( (TQADrawContext *)drawContext, kQATag_FogMode, saveFogMode );
	RvSetInt( (TQADrawContext *)drawContext, kQATag_TextureOp, saveTexOp );
	RvSetPtr( (TQADrawContext *)drawContext, kQATag_Texture, saveCurrTex );
	ResetContextStateFlags( (TQADrawContext *)drawContext );
	dp->lastBaseTextureProcessed = NULL;
	dp->currTriType = TNSL_BITMAP;

bail:
	DebugStr("\n");
	DevTimerEnd();
	DrvTimerEnd();
	AppTimerStart();

}


#pragma mark -
#pragma mark /* Transparent tri caching */

#if CUSTOM_GLIDE_STATE_DEF

static Boolean CustomGlideStatesEqual( TRaveRenderGlideState *s1, TRaveRenderGlideState *s2 )
{
	if (	
			s1->AlphaBlendFnc_rgb_sf	== s2->AlphaBlendFnc_rgb_sf			&&
			s1->AlphaBlendFnc_rgb_df	== s2->AlphaBlendFnc_rgb_df			&&
			s1->alphaCombine_function	== s2->alphaCombine_function		&&
			s1->alphaCombine_factor		== s2->alphaCombine_factor			&&
			s1->alphaCombine_local		== s2->alphaCombine_local			&&
			s1->alphaCombine_other		== s2->alphaCombine_other			&&
			// s1->alphaTest_function		== s2->alphaTest_function			&&
			// s1->alphaRef_value			== s2->alphaRef_value				&&
			s1->chromakeyMode_mode		== s2->chromakeyMode_mode			&&
			s1->chromakeyValue			== s2->chromakeyValue				&&
			s1->colorCombineFunction	== s2->colorCombineFunction			&&
			s1->colorCombineFactor		== s2->colorCombineFactor			&&
			s1->colorCombineLocal		== s2->colorCombineLocal			&&
			s1->colorCombineOther		== s2->colorCombineOther			&&
			s1->DepthBufferFunction		== s2->DepthBufferFunction			&&
			s1->DepthBufferMode			== s2->DepthBufferMode				&&
			s1->fogMode					== s2->fogMode						&&
			s1->s_clampmode				== s2->s_clampmode					&&
			s1->t_clampmode				== s2->t_clampmode					&&
			// s1->texCombineRgb_function	== s2->texCombineRgb_function		&&
			// s1->texCombineRgb_factor	== s2->texCombineRgb_factor			&&
			// s1->texCombineAlpha_function	== s2->texCombineAlpha_function	&&
			// s1->texCombineAlpha_factor	== s2->texCombineAlpha_factor		&&
			s1->minfilter_mode			== s2->minfilter_mode				&&
			s1->magfilter_mode			== s2->magfilter_mode			)
			
			return true;
		else
			return false;
}

#if TNSL_PROFILING

static void WrapGlideGetState( TQADrawPrivate *dp)
{
		grGlideGetState( dp->pCurrAlphaGlideState);
}

#endif
	
static UInt32 CaptureGlideRenderState( TQADrawPrivate *dp, TRaveRenderGlideState *outState)
{
	TRaveRenderGlideState *glst;
	UInt32	thisStateIndex = 0;
	Boolean	bWritingToList = (outState == NULL);
	
	// outState will be NULL if the intent is to write it to the list of stored states.
	if (!bWritingToList)
		glst = outState;
	else if (dp->pAlphaGlideStates)
	{
		thisStateIndex = dp->idxAlphaTriStatesList;	// this points beyond the last captured state
		glst = &dp->pAlphaGlideStates[ dp->idxAlphaTriStatesList ];
	}
	else
		return 0;
		
	// if we think the state has changed or this is the first state for this frame.
	if ( dp->bStateInCacheChanged || !bWritingToList || dp->idxAlphaTriStatesList == 0 )
	{
		// capture Glide state for later replay
		// if we have room
		if (dp->idxAlphaTriStatesList < dp->maxCachedStates)
		{	
			DebugStr(" CaptureGlideRenderState ");
			glst->AlphaBlendFnc_rgb_sf	= dp->rgb_sf;
			glst->AlphaBlendFnc_rgb_df	= dp->rgb_df;
			
			// grAlphaCombine
			glst->alphaCombine_function		= dp->currAlphaCombine_function;
			glst->alphaCombine_factor		= dp->currAlphaCombine_factor;
			glst->alphaCombine_local		= dp->currAlphaCombine_local;
			glst->alphaCombine_other		= dp->currAlphaCombine_other;
			
			// not used in FlushCache()
			if (!bWritingToList)
			{
				// grAlphaTestFunction
				glst->alphaTest_function		= ConvertRaveAlphaTestFunc(dp);
			
				// grAlphaTestReferenceValue
				glst->alphaRef_value 			= (FxU8)(dp->tState[kQATag_AlphaTestRef].f * 255.99f);
			}
			// grChromakeyMode
			glst->chromakeyMode_mode		= dp->tState[kQATag_ChromakeyEnable].i > 0 ? GR_CHROMAKEY_ENABLE : GR_CHROMAKEY_DISABLE;
			// grChromakeyValue
			glst->chromakeyValue			= dp->currChromakeyValue;
			// grColorCombine
			glst->colorCombineFunction		= dp->currColorCombineFunction;
			glst->colorCombineFactor		= dp->currColorCombineFactor;
			glst->colorCombineLocal			= dp->currColorCombineLocal;
			glst->colorCombineOther			= dp->currColorCombineOther;
			
			// grDepthBufferFunction
			glst->DepthBufferFunction		= GR_CMP_LESS;
			
			// grDepthBufferMode
			glst->DepthBufferMode = GR_DEPTHBUFFER_ZBUFFER;
			// grFogMode
			glst->fogMode = dp->tState[kQATag_FogMode].i == kQAZFunction_None ? GR_FOG_DISABLE : GR_FOG_WITH_TABLE_ON_Q;		

			// grTexClampMode
			glst->s_clampmode = (dp->tState[kQATagGL_TextureWrapU].i == kQAGL_Clamp) ? GR_TEXTURECLAMP_CLAMP : GR_TEXTURECLAMP_WRAP;
			glst->t_clampmode = (dp->tState[kQATagGL_TextureWrapV].i == kQAGL_Clamp) ? GR_TEXTURECLAMP_CLAMP : GR_TEXTURECLAMP_WRAP;

			// grTexCombine; enable when supporting multitexture
			// glst->texCombineRgb_function	= dp->currTexCombineRgb_function;
			// glst->texCombineRgb_factor		= dp->currTexCombineRgb_factor;
			// glst->texCombineAlpha_function	= dp->currTexCombineAlpha_function;
			// glst->texCombineAlpha_factor	= dp->currTexCombineAlpha_factor;
			
			// grTexFilterMode
		    switch( dp->tState[kQATag_TextureFilter].i )
		    {
		        case kQATextureFilter_Mid:
					glst->minfilter_mode = glst->magfilter_mode = GR_TEXTUREFILTER_BILINEAR;
		            break;
		
		        case kQATextureFilter_Best:
					glst->minfilter_mode = glst->magfilter_mode = GR_TEXTUREFILTER_BILINEAR;
		        	break;
		            	
		        case kQATextureFilter_Fast:
		       	default:
					glst->minfilter_mode = glst->magfilter_mode = GR_TEXTUREFILTER_BILINEAR;
		            break;
		    }

			DebugStr(" cacheStateNum="); DebugNum(dp->idxAlphaTriStatesList);
			// only increment states counter if this state is different from last
			if (bWritingToList)
			{
				if (dp->idxAlphaTriStatesList > 0)
				{
					if (!CustomGlideStatesEqual( &dp->pAlphaGlideStates[ dp->idxAlphaTriStatesList - 1], glst ))
					{
						dp->idxAlphaTriStatesList++;
					}
					else
					{
						thisStateIndex = max( 0, thisStateIndex - 1);
					}						
				}
				else
				{
					dp->idxAlphaTriStatesList = 1;
				}
				assert (thisStateIndex >= 0);
			}
		}
		else
		{
			DebugStr("\n Ran out of memory for States! ? \n");
			return 0;
		}
	}
	else if (bWritingToList )	// state in cache has not changed.
	{
		thisStateIndex = max( 0, thisStateIndex - 1);
	}
	dp->bStateInCacheChanged = false;
	return thisStateIndex;
}

#else

static UInt32 CacheGlideState(TQADrawPrivate *dp)
{
	UInt32	thisStateIndex = 0;
	assert (dp->pAlphaGlideStates != NULL);
		
	if (dp->pAlphaGlideStates == NULL) return 0;
	thisStateIndex = dp->idxAlphaTriStatesList;
	
	// if we think the state has changed or this is the first state for this frame.
	if ( dp->bStateInCacheChanged || dp->idxAlphaTriStatesList == 0 )
	{
		// capture Glide state for later replay
		// if we have room
		if (dp->idxAlphaTriStatesList < dp->maxCachedStates)
		{	
			assert( dp->pCurrAlphaGlideState );
			// TBFL this should not be an assert; should handle as normal error.
			// assert( dp->pCurrAlphaGlideState < (char *)dp->pAlphaGlideStates + gRvEngInfo.sizeOfGlideState * dp->maxCachedStates);
			grGlideGetState( dp->pCurrAlphaGlideState);
			
			if(dp->idxAlphaTriStatesList > 0){
				// if this state is the same as the last... then don't increment state counters.
				if(memcmp(dp->pCurrAlphaGlideState, (Ptr)dp->pCurrAlphaGlideState - gRvEngInfo.sizeOfGlideState, gRvEngInfo.sizeOfGlideState) != 0){
					dp->idxAlphaTriStatesList++;
					(Ptr)dp->pCurrAlphaGlideState += gRvEngInfo.sizeOfGlideState;
				}
			} else {
				
				(Ptr)dp->pCurrAlphaGlideState += gRvEngInfo.sizeOfGlideState;
				dp->idxAlphaTriStatesList = 1;
			}
			DebugStr(" cacheStateNum="); DebugNum(dp->idxAlphaTriStatesList);
			dp->bStateInCacheChanged = false;
		}
		else
		{
			DebugStr("\n Ran out of memory for States! ? \n");
			return 0;
		}
	}

	return dp->idxAlphaTriStatesList - 1;
}

#endif		// else  !CUSTOM_GLIDE_STATE_DEF


Boolean CacheTriGouraud( 	TQADrawContext *dc,
							GrVertex *v0,
							GrVertex *v1,
							GrVertex *v2 )
{
	CacheTriangle	*pCacheTri;
	CacheIndex		*pCacheIndex;
	TQADrawPrivate *dp = dc->drawPrivate;
	dp->idxAlphaTri = dp->countAlphaTri;
	
	pCacheTri 		= &dp->cachedTri[dp->idxAlphaTri];
	pCacheIndex 	= &dp->cacheIndex[dp->idxAlphaTri];
	
	if (dp->idxAlphaTri >= dp->maxCachedTri)
	{
		DebugStr("\n ? Whoops! alpha tri cache FULL; count="); DebugNum(dp->idxAlphaTri); DebugStr("\n");
		return false;
	}
	pCacheTri->triType = TNSL_GOURAUD;
	pCacheTri->vert[0] 	= *v0;
	pCacheTri->vert[1] 	= *v1;
	pCacheTri->vert[2] 	= *v2;
	
	// pCacheTri->flags = inFlags;
	
	pCacheIndex->fMaxZ		= CalcMaxZ( v0, v1, v2 );
	pCacheIndex->fAvgZ		= CalcAvgZ( v0, v1, v2 );
	
	// State 0 is always the initial state.
	
#if CUSTOM_GLIDE_STATE_DEF	
	pCacheTri->stateIdx = CaptureGlideRenderState(dp, NULL);
#else
	pCacheTri->stateIdx = CacheGlideState(dp);
	pCacheTri->rgb_sf	= dp->rgb_sf;
	pCacheTri->rgb_df	= dp->rgb_df;
#endif
	
	dp->countAlphaTri++;
	DebugStr("cT="); DebugNum(dp->countAlphaTri);
	#if TNSL_DEBUG
	if ( dp->countAlphaTri % 80 == 0)
		DebugStr("\n");
	#endif
	return true;
}

Boolean CacheTriTexture( 	TQADrawContext *dc,
							GrVertex *v0,
							GrVertex *v1,
							GrVertex *v2 )
{
	CacheTriangle	*pCacheTri;
	CacheIndex		*pCacheIndex;
	TQADrawPrivate *dp = dc->drawPrivate;
	
	dp->idxAlphaTri = dp->countAlphaTri;
	
	pCacheTri 		= &dp->cachedTri[dp->idxAlphaTri];
	pCacheIndex 	= &dp->cacheIndex[dp->idxAlphaTri];

	if (dp->idxAlphaTri >= dp->maxCachedTri)
	{
		DebugStr("\n ? Whoops! alpha tri cache FULL; count="); DebugNum(dp->idxAlphaTri); DebugStr("\n");
		return false;
	}
	pCacheTri->triType = TNSL_TEXTURE;
	pCacheTri->vert[0] 	= *v0;
	pCacheTri->vert[1] 	= *v1;
	pCacheTri->vert[2] 	= *v2;
	pCacheTri->baseTexture		= dp->currBaseTexture;
	
	dp->currBaseTexture->cached = FXTRUE;
	
//	pCacheTri->multiTexture		= dp->currMultiTexture;
	
	pCacheIndex->fMaxZ		= CalcMaxZ( v0, v1, v2 );
	pCacheIndex->fAvgZ		= CalcAvgZ( v0, v1, v2 );
	
#if CUSTOM_GLIDE_STATE_DEF	
	pCacheTri->stateIdx = CaptureGlideRenderState(dp, NULL);
#else
	pCacheTri->stateIdx = CacheGlideState(dp);
	pCacheTri->rgb_sf	= dp->rgb_sf;
	pCacheTri->rgb_df	= dp->rgb_df;
#endif
	dp->countAlphaTri++;
	DebugStr("cT="); DebugNum(dp->countAlphaTri);
	#if TNSL_DEBUG
	if ( dp->countAlphaTri % 80 == 0)
		DebugStr("\n");
	#endif
	
	return true;
}


#if TNSL_DEBUG
#define showCacheMaxValues 0
#else
#define showCacheMaxValues 0
#endif

#if showCacheMaxValues
static int maxTris = 0;
static int maxStates = 0;
#endif

#define stateBufferSize 3100

#if CUSTOM_GLIDE_STATE_DEF

static void ResetGlideRenderState( TQADrawPrivate *dp, UInt32 idx )
{
	DebugStr(" ResetGlideRenderState ");
	grChromakeyMode( dp->pAlphaGlideStates[idx].chromakeyMode_mode );
	grChromakeyValue( dp->pAlphaGlideStates[idx].chromakeyValue );
	grDepthBufferFunction( dp->pAlphaGlideStates[idx].DepthBufferFunction );
	grDepthBufferMode( dp->pAlphaGlideStates[idx].DepthBufferMode );
	grFogMode( dp->pAlphaGlideStates[idx].fogMode );
	grTexFilterMode( GR_TMU0,
	        dp->pAlphaGlideStates[idx].minfilter_mode,
	        dp->pAlphaGlideStates[idx].magfilter_mode );
	grTexClampMode( GR_TMU0,
	               dp->pAlphaGlideStates[idx].s_clampmode,
	               dp->pAlphaGlideStates[idx].t_clampmode	);
	grAlphaCombine( dp->pAlphaGlideStates[idx].alphaCombine_function,
					dp->pAlphaGlideStates[idx].alphaCombine_factor,
					dp->pAlphaGlideStates[idx].alphaCombine_local,
					dp->pAlphaGlideStates[idx].alphaCombine_other,
					FXFALSE	);
					
	grColorCombine( dp->pAlphaGlideStates[idx].colorCombineFunction,
					dp->pAlphaGlideStates[idx].colorCombineFactor,
					dp->pAlphaGlideStates[idx].colorCombineLocal,
					dp->pAlphaGlideStates[idx].colorCombineOther,
					FXFALSE );

	/*
	grTexCombine(
             GR_TMU0,
             dp->pAlphaGlideStates[idx].texCombineRgb_function,
             dp->pAlphaGlideStates[idx].texCombineRgb_factor,
             dp->pAlphaGlideStates[idx].texCombineAlpha_function,
             dp->pAlphaGlideStates[idx].texCombineAlpha_factor,
             FXFALSE, FXFALSE );
    */
	grAlphaBlendFunction(
				dp->pAlphaGlideStates[idx].AlphaBlendFnc_rgb_sf,
				dp->pAlphaGlideStates[idx].AlphaBlendFnc_rgb_df,
				GR_BLEND_ONE, GR_BLEND_ZERO );
	grAlphaTestFunction( 
	 			dp->pAlphaGlideStates[idx].alphaTest_function );
	grAlphaTestReferenceValue( 
				dp->pAlphaGlideStates[idx].alphaRef_value );
}

static void ResetGlideSavedState( TRaveRenderGlideState *savedState )
{
	DebugStr(" ResetGlideSavedState ");
	grChromakeyMode( savedState->chromakeyMode_mode );
	grChromakeyValue( savedState->chromakeyValue );
	grFogMode( savedState->fogMode );
	
	grAlphaBlendFunction(
				savedState->AlphaBlendFnc_rgb_sf,
				savedState->AlphaBlendFnc_rgb_df,
				GR_BLEND_ONE, GR_BLEND_ZERO );
	grAlphaTestFunction( 
				savedState->alphaTest_function );
	grAlphaTestReferenceValue( 
				savedState->alphaRef_value );
	
	grAlphaCombine(
				savedState->alphaCombine_function,
				savedState->alphaCombine_factor,
				savedState->alphaCombine_local,
				savedState->alphaCombine_other,
				FXFALSE	);
					
	grColorCombine(
				savedState->colorCombineFunction,
				savedState->colorCombineFactor,
				savedState->colorCombineLocal,
				savedState->colorCombineOther,
				FXFALSE );
	grDepthBufferFunction( savedState->DepthBufferFunction );
	grDepthBufferMode( savedState->DepthBufferMode );
	grFogMode( savedState->fogMode );
	grTexClampMode(
	               GR_TMU0,
	               savedState->s_clampmode,
	               savedState->t_clampmode	);

	/*
	grTexCombine(
             GR_TMU0,
             savedState->texCombineRgb_function,
             savedState->texCombineRgb_factor,
             savedState->texCombineAlpha_function,
             savedState->texCombineAlpha_factor,
             FXFALSE, FXFALSE );
    */
	grTexFilterMode(
	        GR_TMU0,
	        savedState->minfilter_mode,
	        savedState->magfilter_mode );
}
#endif

#if TNSL_PROFILING
static void WrapGlideGetIncomingState( char* inGlideState )
{
		grGlideGetState( inGlideState );
}
#endif
	
#if TNSL_PROFILING
static void WrapGlideSetState( char* savedGlideState )
{
		grGlideSetState( savedGlideState );
}
#endif

#if TNSL_PROFILING
static void WrapQSort(void* idx, size_t cnt, size_t siz, _compare_function f)
{
	qsort( idx, cnt, siz, f );
}
#endif
	
	
void FlushTriCache( TQADrawContext *dc )
{
	UInt32	i;
	int t;
	CacheTriangle	*pCacheTri;
	TQATexture		*saveBase, *saveMulti, *tex, *next;
#if CUSTOM_GLIDE_STATE_DEF
	TRaveRenderGlideState	inGlideState;
#else
	char			inGlideState[stateBufferSize];
#endif
	TQADrawPrivate *dp = dc->drawPrivate;
	UInt32			lastStateIndex;
	
	
	if (dp->countAlphaTri == 0)
		return;
	
	DebugStr("FlushTriCache num tris = "); DebugNum(dp->countAlphaTri);
	DebugStr("Num saved Glide states = "); DebugNum(dp->idxAlphaTriStatesList);
	
#if !CUSTOM_GLIDE_STATE_DEF
	{
		int stateSize;
		grGet(GR_GLIDE_STATE_SIZE, 4, (FxI32 *)&stateSize);
		assert(stateSize == gRvEngInfo.sizeOfGlideState);
		assert(stateSize <= stateBufferSize);
	}
#endif
	
#if showCacheMaxValues
	if(dp->countAlphaTri > maxTris || dp->idxAlphaTriStatesList > maxStates){
		if(dp->countAlphaTri > maxTris) maxTris = dp->countAlphaTri;
		if(dp->idxAlphaTriStatesList > maxStates) maxStates = dp->idxAlphaTriStatesList;
		{
			FILE * f;
			f = fopen("MaxLog", "a");
			fprintf(f, "maxTris = %d, maxStates = %d\n", maxTris, maxStates);
			fclose(f);
		}
	}
#endif

	
	// save state
	saveBase = (TQATexture *)dp->tState[kQATag_Texture].p;
	saveMulti = (TQATexture *)dp->tState[kQATag_MultiTexture].p;
#if CUSTOM_GLIDE_STATE_DEF	
	CaptureGlideRenderState(dp, &inGlideState);
#else
	#if TNSL_PROFILING
		WrapGlideGetIncomingState( inGlideState );
	#else
		grGlideGetState( inGlideState );
	#endif
#endif

	// initialize ordering
	for ( i = 0; i < dp->countAlphaTri; i++ )
		dp->cacheIndex[i].index = i;
		
	// sort triangles by max depth; in case of tie, use avg vertex depth
	if (dp->countAlphaTri > 1)
	{
		#if TNSL_PROFILING
			WrapQSort((void *)dp->cacheIndex, dp->countAlphaTri, sizeof(CacheIndex), CompareTriDepths );
		#else
			qsort( (void *)dp->cacheIndex, dp->countAlphaTri, sizeof(CacheIndex), CompareTriDepths );
		#endif
	}
	
	// draw alpha triangles back to front
	lastStateIndex = (UInt32)-1;
	
	// grDepthMask must be done after grGlideSetState as it is part of the glide state
	grDepthMask( FXFALSE );					// turn off z-writes; this is necessary for this algorithm.
	grAlphaTestFunction(GR_CMP_ALWAYS);		// we are not caching 1555 textures, unless also vertex alpha
	if (bNanosaurRunning)
		grAlphaBlendFunction(GR_BLEND_SRC_ALPHA, GR_BLEND_ONE_MINUS_SRC_ALPHA, GR_BLEND_ONE, GR_BLEND_ZERO);
	DebugStr("\n");
	/*
	 * Here's the big replay loop...
	 */
	 
	for ( t = dp->countAlphaTri - 1; t >= 0; t-- )
	{
		pCacheTri = &dp->cachedTri[ dp->cacheIndex[t].index ];
		
		if( pCacheTri->stateIdx != lastStateIndex )
		{
		#if !CUSTOM_GLIDE_STATE_DEF
			char *savedGlideState = ((char *)dp->pAlphaGlideStates) + pCacheTri->stateIdx * gRvEngInfo.sizeOfGlideState;
		#endif

			lastStateIndex = pCacheTri->stateIdx;

		#if CUSTOM_GLIDE_STATE_DEF
			ResetGlideRenderState( dp, pCacheTri->stateIdx );
		#else
			#if TNSL_PROFILING
				WrapGlideSetState( savedGlideState);
			#else
				// Theoretically, this should restore all Glide states, but...
				grGlideSetState( savedGlideState );
			#endif
		#endif
		

			if (0 && !bNanosaurRunning)	// temp 10/23
			{
		#if CUSTOM_GLIDE_STATE_DEF
				grAlphaBlendFunction(dp->pAlphaGlideStates[pCacheTri->stateIdx].AlphaBlendFnc_rgb_sf, 
										dp->pAlphaGlideStates[pCacheTri->stateIdx].AlphaBlendFnc_rgb_df, 
										GR_BLEND_ONE, GR_BLEND_ZERO);
		#else
				grAlphaBlendFunction(pCacheTri->rgb_sf, pCacheTri->rgb_df, 
										GR_BLEND_ONE, GR_BLEND_ZERO);
		#endif
			}
		}
		
		if( pCacheTri->triType == TNSL_TEXTURE){
	  /* temp 10/12
			grAlphaCombine(
				GR_COMBINE_FUNCTION_BLEND_OTHER,
		        GR_COMBINE_FACTOR_LOCAL,
		        GR_COMBINE_LOCAL_ITERATED,
		        GR_COMBINE_OTHER_TEXTURE,
		        FXFALSE
			);
			grColorCombine(	GR_COMBINE_FUNCTION_BLEND_OTHER,
			             	GR_COMBINE_FACTOR_LOCAL,
			             	GR_COMBINE_LOCAL_ITERATED,
			             	GR_COMBINE_OTHER_TEXTURE,
			             	FXFALSE);
			assert( dp->pAlphaGlideStates[ pCacheTri->stateIdx ].alphaCombine_function == GR_COMBINE_FUNCTION_BLEND_OTHER);
			assert( dp->pAlphaGlideStates[ pCacheTri->stateIdx ].alphaCombine_factor == GR_COMBINE_FACTOR_LOCAL);
			assert( dp->pAlphaGlideStates[ pCacheTri->stateIdx ].alphaCombine_local == GR_COMBINE_LOCAL_ITERATED);
			assert( dp->pAlphaGlideStates[ pCacheTri->stateIdx ].alphaCombine_other == GR_COMBINE_OTHER_TEXTURE);

			assert( dp->pAlphaGlideStates[ pCacheTri->stateIdx ].colorCombineFunction	== GR_COMBINE_FUNCTION_BLEND_OTHER);
			assert( dp->pAlphaGlideStates[ pCacheTri->stateIdx ].colorCombineFactor	== GR_COMBINE_FACTOR_LOCAL);
			assert( dp->pAlphaGlideStates[ pCacheTri->stateIdx ].colorCombineLocal	== GR_COMBINE_LOCAL_ITERATED);
			assert( dp->pAlphaGlideStates[ pCacheTri->stateIdx ].colorCombineOther	== GR_COMBINE_OTHER_TEXTURE);
			             	*/
			
		
			LoadTextureAndTableToVRAM(dp, pCacheTri->baseTexture);
			// This allows a 1555 blended with vertex alpha.
			grAlphaCombine(
				GR_COMBINE_FUNCTION_BLEND_OTHER,
		        GR_COMBINE_FACTOR_LOCAL,
		        GR_COMBINE_LOCAL_ITERATED,
		        GR_COMBINE_OTHER_TEXTURE,
		        FXFALSE
			);
		// dp->lastBaseTextureProcessed = pCacheTri->baseTexture;	// prevents unnecessary grTexSource calls
		} 
		else {
			/*
			assert( dp->pAlphaGlideStates[ pCacheTri->stateIdx ].alphaCombine_function == GR_COMBINE_FUNCTION_LOCAL_ALPHA);
			assert( dp->pAlphaGlideStates[ pCacheTri->stateIdx ].alphaCombine_factor == GR_COMBINE_FACTOR_NONE);
			assert( dp->pAlphaGlideStates[ pCacheTri->stateIdx ].alphaCombine_local == GR_COMBINE_LOCAL_ITERATED);
			assert( dp->pAlphaGlideStates[ pCacheTri->stateIdx ].alphaCombine_other == GR_COMBINE_OTHER_NONE);
			
			assert( dp->pAlphaGlideStates[ pCacheTri->stateIdx ].colorCombineFunction	== GR_COMBINE_FUNCTION_LOCAL);
			assert( dp->pAlphaGlideStates[ pCacheTri->stateIdx ].colorCombineFactor	== GR_COMBINE_FACTOR_NONE);
			assert( dp->pAlphaGlideStates[ pCacheTri->stateIdx ].colorCombineLocal	== GR_COMBINE_LOCAL_ITERATED);
			assert( dp->pAlphaGlideStates[ pCacheTri->stateIdx ].colorCombineOther	== GR_COMBINE_OTHER_NONE);
			
			grColorCombine(
				GR_COMBINE_FUNCTION_LOCAL, 
				GR_COMBINE_FACTOR_NONE,
				GR_COMBINE_LOCAL_ITERATED, 
				GR_COMBINE_OTHER_NONE, 
				FXFALSE
				);
			*/
			grAlphaCombine(
				GR_COMBINE_FUNCTION_LOCAL_ALPHA,        // combine function
				GR_COMBINE_FACTOR_NONE,					// combine factor
				GR_COMBINE_LOCAL_ITERATED, 				// local color
				GR_COMBINE_OTHER_NONE, 				    // other color
				FXFALSE									// invert
				);

		}

		grDrawTriangle( &pCacheTri->vert[0], &pCacheTri->vert[1], &pCacheTri->vert[2]);
		#if TNSL_DEBUG
		DebugStr("t");
		if ( t % 80 == 0)
			DebugStr("\n");
		#endif

			
	}
	// set z-writes back to normal
	grDepthMask(dp->tState[kQATag_ZFunction].i != kQAZFunction_None);
	// restore alpha test function
	grAlphaTestFunction( ConvertRaveAlphaTestFunc(dp) );
	// delete textures that app deleted, but were needed until now
	tex = gRvEngInfo.textureList;
	
	while ( tex )
	{
		tex->cached = FXFALSE;
		next = tex->next;
		if (tex->toBeDeleted)
		{
			DebugStr("/n Deferred texture delete "); DebugHex(tex);
			RvTextureDelete( tex );
		}
		tex = next;
	}
	
	// reset tri cache
	dp->countAlphaTri = 0;
	
	dp->idxAlphaTriStatesList = 0;			// reset the state index counter
	dp->pCurrAlphaGlideState = dp->pAlphaGlideStates;	// reset the state pointer counter
	dp->bStateInCacheChanged = false;
	// restore state
#if CUSTOM_GLIDE_STATE_DEF
	ResetGlideSavedState( &inGlideState );
#else
	grGlideSetState( inGlideState );
#endif
	dp->tState[kQATag_Texture].p = saveBase;
	dp->tState[kQATag_MultiTexture].p = saveMulti;
	dp->lastBaseTextureProcessed = NULL;
	dp->currTriType = TNSL_NONE;
	SetAndLoadTextureMap(dp);
	DebugStr("\n");

}

#pragma mark -

#define PRACTICALLY_ZERO(f) 	(fabs((f)) < 0.000001 ? 1 : 0)

int CompareTriDepths( const void *index1, const void *index2 )
{
	if ( PRACTICALLY_ZERO( ((CacheIndex *)index1)->fMaxZ - ((CacheIndex *)index2)->fMaxZ))
		return ( (((CacheIndex *)index1)->fAvgZ > ((CacheIndex *)index2)->fAvgZ) ? 1 : -1);
	else
		return( (((CacheIndex *)index1)->fMaxZ > ((CacheIndex *)index2)->fMaxZ) ? 1 : -1);
}

float CalcMaxZ( GrVertex* v0, GrVertex* v1, GrVertex* v2 )
{
	float f;
	f = max( v0->ooz, v1->ooz );
	f = max( f, v2->ooz );
	return f;
}

float CalcAvgZ( GrVertex* v0, GrVertex* v1, GrVertex* v2 )
{
	float f;
	f = ( v0->ooz + v1->ooz + v2->ooz ) / 3.0;
	return f;
}

#pragma mark -
#pragma mark /* unused; for later optimization */
void	resetPipeline( void )
{
	// stub
}

void ChooseNewTriFcts( TQADrawContext* dc )
{
	#pragma unused( dc )
}