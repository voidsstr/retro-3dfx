#pragma once

float CalcFogNone(float, float, TQADrawPrivate*);

extern TRvInfo	gRvEngInfo;
extern float	gSTWfactor;

#define CONVERT_A(a) (255.9f * (a))
#define CONVERT_R(r) (255.9f * (r))
#define CONVERT_G(g) (255.9f * (g))
#define CONVERT_B(b) (255.9f * (b))

#define CONVERT_X(x) ((float) (x))
#define CONVERT_Y(y) ((float) (y))
#define CONVERT_Z(z) (65536.f * (z))

#define CONVERT_S(s,m) ((m) * (s))
#define CONVERT_T(t,m) ((m) * (t))

#define CONVERT_L(r,g,b) (((float) (r) * 0.30f) + ((float) (g) * 0.59) + ((float) (b) * 0.11))

#define CONVERT_COLOR(r,g,b) \
  (((FxU32) CONVERT_R(r) << 16) | ((FxU32) CONVERT_G(g) << 8) | (FxU32) CONVERT_B(b))

#define CONVERT_PARGB(o,i) \
  (o)->argb = (((FxU32) CONVERT_A((i)->a) << 24) | ((FxU32) CONVERT_R((i)->r) << 16) | ((FxU32) CONVERT_G((i)->g) << 8) | (FxU32) CONVERT_B((i)->b))

#define CONVERT_ZBUFFER_XYZ(o,i,dp) \
  (o)->x   = (i)->x;  			\
  (o)->y   = (i)->y;  			\
  (o)->ooz = CONVERT_Z((i)->z);	\
  (o)->oow = (dp)->fogFunc((i)->invW, (i)->a, (dp));

#define CONVERT_ZBUFFER_XYZ_NOFOG(o,i) \
  (o)->x   = (i)->x;  			\
  (o)->y   = (i)->y;  			\
  (o)->ooz = CONVERT_Z((i)->z);	\
  (o)->oow = (1.0);

#define CONVERT_WBUFFER_XYZ(o,i) \
  (o)->x   = CONVERT_X((i)->x);  \
  (o)->y   = CONVERT_Y((i)->y);  \
  (o)->ooz = CONVERT_Z((i)->z);
	
#define CONVERT_NORMAL_RGB(o,i)	\
  (o)->r = CONVERT_R((i)->r); \
  (o)->g = CONVERT_G((i)->g); \
  (o)->b = CONVERT_B((i)->b);
  
#define CONVERT_MODULATE_RGB(o,i) \
  (o)->argb = (((FxU32) CONVERT_R((i)->kd_r) << 16) | ((FxU32) CONVERT_G((i)->kd_g) << 8) | (FxU32) CONVERT_B((i)->kd_b))

#define CONVERT_HIGHLIGHT_RGB(o,i) \
  (o)->argb = (((FxU32) CONVERT_R((i)->ks_r) << 16) | ((FxU32) CONVERT_G((i)->ks_g) << 8) | (FxU32) CONVERT_B((i)->ks_b))
	
#define CONVERT_BASE_STW(o,i,r) \
  (o)->sow0 = gSTWfactor * CONVERT_S((i)->uOverW, (r)->ratioX); \
  (o)->tow0 = gSTWfactor * CONVERT_T((i)->vOverW, (r)->ratioY); \
  (o)->q0 	= gSTWfactor * (i)->invW; 
  
#define CONVERT_MULTI_0_STW(o,i,r) \
  (o)->sow1 = CONVERT_S((i)->uOverW, (r)->ratioX); \
  (o)->tow1 = CONVERT_T((i)->vOverW, (r)->ratioY); \
  (o)->q1 = (i)->invW;
  
#define CONVERT_NORMAL_A(o,i)		\
		(o)->argb |= (FxU32) CONVERT_R((i)->a) << 24;
  
#define CONVERT_HIGHLIGHT_A(o,i) \
  (o)->a = min(CONVERT_L((i)->ks_r, (i)->ks_g, (i)->ks_b), 1.0f);
  
#define CONVERT_MODSPEC_RGB(o,i) \
	(o)->argb = ( ((FxU32)  CONVERT_R( (i)->kd_r * (i)->ks_r) << 16) \
				| ((FxU32)  CONVERT_G( (i)->kd_g * (i)->ks_g) << 8)  \
				| ((FxU32)  CONVERT_B( (i)->kd_b * (i)->ks_b)))

#define CONVERT_NORMAL_W(o,i)	\
  (o)->q0 = CONVERT_B((i)->invW);
	
#define CALC_UVW(tc,vt)	\
  if(!((tc)->flags & TNSL_VALID)) { \
    (tc)->flags |= TNSL_VALID; \
    (tc)->w = 1.0f / (vt)->invW; \
    (tc)->u = (vt)->uOverW * (tc)->w; \
    (tc)->v = (vt)->vOverW * (tc)->w; \
  }
	
		
	FX_ENTRY void FX_CALL
grAADrawTriangle(
                 const void *a, const void *b, const void *c,
                 FxBool ab_antialias, FxBool bc_antialias, FxBool ca_antialias
                 );





#define AVERAGE_VERTICES(o,i,c) \
  (o)->x = ((i)->x + (c)->x) / 2; \
  (o)->y = ((i)->y + (c)->y) / 2; \
  (o)->z = ((i)->z + (c)->z) / 2; \
  (o)->r = ((i)->r + (c)->r) / 2; \
  (o)->g = ((i)->g + (c)->g) / 2; \
  (o)->b = ((i)->b + (c)->b) / 2; \
  (o)->a = ((i)->a + (c)->a) / 2; \
  (o)->invW   = ((i)->invW + (c)->invW) / 2; \
  (o)->uOverW = ((i)->uOverW + (c)->uOverW) / 2; \
  (o)->vOverW = ((i)->vOverW + (c)->vOverW) / 2;



///////////////////////////////
//                           //
//  SetVertex_GouraudSingle  //
//                           //
///////////////////////////////

inline void 
SetVertex_GouraudSingle(TQADrawPrivate*    dp,
                        const TQAVGouraud* v,
                        GrVertex*          gv)
{
	assert( v && gv );
	
	CONVERT_ZBUFFER_XYZ(gv, v, dp);
	// CONVERT_NORMAL_W(gv, v);
	//+ CONVERT_NORMAL_RGB(gv, v);
	CONVERT_PARGB(gv, v);
	
	dp->usingVertexAlpha = (v->a != 1.0f) ? 1 : 0;
}


//////////////////////////////
//                          //
//  SetVertex_GouraudThree  //
//                          //
//////////////////////////////

inline void 
SetVertex_GouraudThree(TQADrawPrivate*    dp,
                       const TQAVGouraud* v0,
                       const TQAVGouraud* v1,
                       const TQAVGouraud* v2,
                       GrVertex*          gv)
{
	// do this outside of the loop; dp->usingVertexAlpha  = ((v0->a != 1.0f) || (v1->a != 1.0f) || (v2->a != 1.0f));
	
	assert( v0 && v1 && v2 && gv );

	if (dp->fogFunc == CalcFogNone)
	{
		CONVERT_ZBUFFER_XYZ_NOFOG(&gv[0], v0);
		CONVERT_ZBUFFER_XYZ_NOFOG(&gv[1], v1);
		CONVERT_ZBUFFER_XYZ_NOFOG(&gv[2], v2);
	}
	else
	{
		CONVERT_ZBUFFER_XYZ(&gv[0], v0, dp);
		CONVERT_ZBUFFER_XYZ(&gv[1], v1, dp);
		CONVERT_ZBUFFER_XYZ(&gv[2], v2, dp);
	}
	
	CONVERT_PARGB(&gv[0], v0);
	CONVERT_PARGB(&gv[1], v1);
	CONVERT_PARGB(&gv[2], v2);
	
}


///////////////////////////////
//                           //
//  SetVertex_TextureSingle  //
//                           //
///////////////////////////////

inline void 
SetVertex_TextureSingle(TQADrawPrivate*    dp,
                        const TQAVTexture* v,
                        GrVertex*          gv)
{
	TQATexture* 		theCurrMultiTex;
	TQATexture* 		theCurrBaseTexture = (TQATexture*)dp->tState[kQATag_Texture].p;

	assert( theCurrBaseTexture );
	CONVERT_ZBUFFER_XYZ(gv, v, dp);
	CONVERT_BASE_STW(gv, v, theCurrBaseTexture);

	if (dp->tState[kQATag_TextureOp].i & kQATextureOp_Decal) 
	{
		//+CONVERT_NORMAL_RGB(gv, v);
		CONVERT_PARGB( gv, v );
	} 
	/*
	else if (dp->tState[kQATag_TextureOp].i & (kQATextureOp_Modulate | kQATextureOp_Highlight))
	{
		CONVERT_MODSPEC_RGB(gv, v);
	}
	*/
	else if (dp->tState[kQATag_TextureOp].i & kQATextureOp_Modulate) 
	{
		CONVERT_MODULATE_RGB(gv, v);
		CONVERT_NORMAL_A(gv, v);
	} 
	else if (dp->tState[kQATag_TextureOp].i & kQATextureOp_Highlight) 
	{
		CONVERT_HIGHLIGHT_RGB(gv, v);
		CONVERT_NORMAL_A(gv, v);
	} 
	else 
	{
		CONVERT_PARGB(gv, v);
	}
	
	dp->usingVertexAlpha = (v->a != 1.0f) ? 1 : 0;
	
	// Consider multitexturing
	if (dp->tState[kQATag_MultiTextureEnable].i)
	{
		theCurrMultiTex = (TQATexture*)dp->tState[kQATag_MultiTexture].p;
		assert( theCurrMultiTex );
		assert( dp->multiTexParams );
		assert( dp->nMultiTexParams );
		CONVERT_MULTI_0_STW( gv, dp->multiTexParams, theCurrMultiTex );
	}
}


//////////////////////////////
//                          //
//  SetVertex_TextureThree  //
//                          //
//////////////////////////////


inline void 
SetVertex_TextureThree(TQADrawPrivate*    dp,
                       register const TQAVTexture* v0,
                       register const TQAVTexture* v1,
                       register const TQAVTexture* v2,
                       register GrVertex*          gv)
{
	// TQATexture* 		theCurrMultiTex;
	TQATexture* 		theCurrBaseTexture = (TQATexture*)dp->tState[kQATag_Texture].p;
	
	assert( theCurrBaseTexture );

	if (dp->fogFunc == CalcFogNone)
	{
		CONVERT_ZBUFFER_XYZ_NOFOG(&gv[0], v0);
		CONVERT_ZBUFFER_XYZ_NOFOG(&gv[1], v1);
		CONVERT_ZBUFFER_XYZ_NOFOG(&gv[2], v2);
	}
	else
	{
		CONVERT_ZBUFFER_XYZ(&gv[0], v0, dp);
		CONVERT_ZBUFFER_XYZ(&gv[1], v1, dp);
		CONVERT_ZBUFFER_XYZ(&gv[2], v2, dp);
	}
	
	CONVERT_BASE_STW(&gv[0], v0, theCurrBaseTexture);
	CONVERT_BASE_STW(&gv[1], v1, theCurrBaseTexture);
	CONVERT_BASE_STW(&gv[2], v2, theCurrBaseTexture);
	
	if(dp->tState[kQATag_TextureOp].i & kQATextureOp_Modulate) 
	{
		DebugStr("M");
		CONVERT_MODULATE_RGB(&gv[0], v0);
		CONVERT_MODULATE_RGB(&gv[1], v1);
		CONVERT_MODULATE_RGB(&gv[2], v2);
		CONVERT_NORMAL_A(&gv[0], v0);
		CONVERT_NORMAL_A(&gv[1], v1);
		CONVERT_NORMAL_A(&gv[2], v2);
	}
	else if (dp->tState[kQATag_TextureOp].i & kQATextureOp_Decal) 
	{
		DebugStr("D");
		CONVERT_PARGB(&gv[0], v0);
		CONVERT_PARGB(&gv[1], v1);
		CONVERT_PARGB(&gv[2], v2);
	} 
	/* not supporting specular highlights yet
	else if(dp->tState[kQATag_TextureOp].i & kQATextureOp_Highlight) 
	{
		DebugStr("H");
		CONVERT_HIGHLIGHT_RGB(&gv[0], v0);
		CONVERT_HIGHLIGHT_RGB(&gv[1], v1);
		CONVERT_HIGHLIGHT_RGB(&gv[2], v2);
		CONVERT_NORMAL_A(&gv[0], v0);
		CONVERT_NORMAL_A(&gv[1], v1);
		CONVERT_NORMAL_A(&gv[2], v2);
	} 
	*/
	else 
	{
		DebugStr("N");
		CONVERT_PARGB(&gv[0], v0);
		CONVERT_PARGB(&gv[1], v1);
		CONVERT_PARGB(&gv[2], v2);
	}
	
	/* do this outside the loop
	{
		dp->usingVertexAlpha = ((v0->a != 1.0f) || (v1->a != 1.0f) || (v2->a != 1.0f));
#if TNSL_DEBUG
		if(dp->usingVertexAlpha){
			DebugStr("A");
		}
#endif
	}	*/
	// not supporting multitexturing 
#if 0	
	if (dp->tState[kQATag_MultiTextureEnable].i)
	{
		TQAVMultiTexture *mtp = dp->multiTexParams;
		theCurrMultiTex = (TQATexture*)dp->tState[kQATag_MultiTexture].p;
		if (theCurrMultiTex)
		{
			assert( dp->multiTexParams );
			assert( dp->nMultiTexParams >= 3);
			CONVERT_MULTI_0_STW( &gv[0], mtp++, theCurrMultiTex );
			CONVERT_MULTI_0_STW( &gv[1], mtp++, theCurrMultiTex );
			CONVERT_MULTI_0_STW( &gv[2], mtp++, theCurrMultiTex );
		}
	}
#endif	
}
