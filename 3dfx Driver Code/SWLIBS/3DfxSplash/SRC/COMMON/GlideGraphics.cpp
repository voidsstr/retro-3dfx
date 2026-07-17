#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "GlideGraphics.h"
#include "RevPoint.h"
#include <math.h>
#include "clip.h"
#include <assert.h>
#define ASSERT assert

#include <stdio.h>
#include <largeint.h>
#include "convexhull.h"

#define GLIDE_ERROR(a) ASSERT( 0 )

#define USE_ORIGIN_UPPER_LEFT

#define ENVMAP_RADIUS  (400.0f)
#define ENVMAP_TEX_COORD_SCALE  (1.6f)

// if this is set to 1, duplicate edges will not be added
// when the aa edge list is being created
#define REMOVE_DUPLICATE_EDGES  1

#ifndef M_PI
#define M_PI 3.14159f
#endif

#define SNAP_BIAS ( ( float )( 3L << 18 ) )

#define DEG2RAD(a) ( a * ( M_PI / 180.0f ) )

#define NEAR_Z ( 1.0f )

int gNumTMUs;
long gDepthMinMax[2];
extern int gNormals, gWireframe;
extern float gXformedCameraDir[3];
extern int gCurrMaterialID;
extern GrColorFormat_t gColorFormat;

static inline void MakeIdentMatrix( RevGlideMatrix m )
{
#define M(row,col)  m[col*4+row]
  M(0,0) = 1.0f; M(0,1) = 0.0f; M(0,2) = 0.0f; M(0,3) = 0.0f;
  M(1,0) = 0.0f; M(1,1) = 1.0f; M(1,2) = 0.0f; M(1,3) = 0.0f;
  M(2,0) = 0.0f; M(2,1) = 0.0f; M(2,2) = 1.0f; M(2,3) = 0.0f;
  M(3,0) = 0.0f; M(3,1) = 0.0f; M(3,2) = 0.0f; M(3,3) = 1.0f;
#undef M
}

#ifdef PRINT_DEBUG
extern FILE *fp;
#endif // PRINT_DEBUG



/*
// Gary Tarolli's clever inverse square root technique
float fsqrt_inv(float f)
{
long i;
float x2, y;

	x2 = 0.5f*f;
	i = *(long *)&f;
	i = 0x5f3759df - (i>>1);
	y = *(float *)&i;
	
		y = 1.5f*y - (x2*y * y*y);
		y = 1.5f*y - (x2*y * y*y);
		
			return y;
			}
*/
const float ONE_HALF = 0.5f;
const float THREE_HALVES = 1.5f;
__declspec(naked) float fsqrt_inv(float f)
{
	__asm
	{
		fld			dword ptr [esp + 4]
		// f
		fmul		dword ptr [ONE_HALF]
		// x2 = 0.5f*f

		mov			eax, [esp + 4]
		mov			ecx, 0x5f3759df

		shr			eax, 1

		sub			ecx, eax

		mov			[esp + 4], ecx

		fmul		dword ptr [esp + 4]
		// x2*y
		fld			dword ptr [esp + 4]
		// y
		// x2*y
		fmul		dword ptr [esp + 4]
		// y*y
		// x2*y
		fld			dword ptr [THREE_HALVES]
		// 1.5f
		// y*y
		// x2*y
		fmul		dword ptr [esp + 4]
		// 1.5f*y
		// y*y
		// x2*y
		fxch		st(2)
		// x2*y
		// y*y
		// 1.5f*y
		// ******** stall 1 clock ********
		fmulp		st(1), st
		// x2*y * y*y
		// 1.5f*y
		// ******** stall 2 clocks ********
		fsubp		st(1), st
		// y = 1.5f*y - (x2*y * y*y)
		ret
	}
}


GlideGraphics::GlideGraphics()
{
  InitMatrixStacks();
  RebuildCompositeMatrix();
}

GlideGraphics::~GlideGraphics()
{
}

void GlideGraphics::SetClipWindow(int x, int y, int width, int height)
{
#ifdef USE_ORIGIN_UPPER_LEFT
	grClipWindow(x, device_height-(y+height), x+width, device_height-y);
	SetClipVolume((float)x, (float)(x+width), (float)(device_height-(y+height)), (float)(device_height-y), 1.0f, 65535.0f);
#else
  grClipWindow(x, y, x+width, y+height);
	SetClipVolume((float)x, (float)(x+width), (float)y, (float)(y+height), 1.0f, 65535.0f);
#endif // USE_ORIGIN_UPPER_LEFT
}

void GlideGraphics::InitMatrixStacks( void )
{
  int i;

  for( i = 0; i < 3; i++ )
	{
		matrix_stack[i].top_of_stack = 0;
		MakeIdentMatrix( matrix_stack[i].mat[0] );
	}
	current_matrix_stack = &matrix_stack[GLIDE_MODELVIEW];
}

void GlideGraphics::StoreGlideState()
{
#ifdef USE_GLIDE3
  grGlideGetState(oldState);
	grGlideGetVertexLayout(oldVertState);
#else
  grGlideGetState(&oldState);
#endif // USE_GLIDE3
}

void GlideGraphics::RestoreGlideState()
{
#ifdef USE_GLIDE3
  grGlideSetState(oldState);
	grGlideSetVertexLayout(oldVertState);
#else
  grGlideSetState(&oldState);
#endif // USE_GLIDE3
}

void GlideGraphics::SetSplashGlideState()
{
#ifdef USE_GLIDE3
	grCoordinateSpace(GR_WINDOW_COORDS);

	// enable all necessary vertex parameters
	grVertexLayout(GR_PARAM_XY, (long)&((GrVertex *)0)->x, GR_PARAM_ENABLE);
	grVertexLayout(GR_PARAM_RGB, (long)&((GrVertex *)0)->r, GR_PARAM_ENABLE);
	grVertexLayout(GR_PARAM_A, (long)&((GrVertex *)0)->a, GR_PARAM_ENABLE);
	grVertexLayout(GR_PARAM_Q, (long)&((GrVertex *)0)->oow, GR_PARAM_ENABLE);
	grVertexLayout(GR_PARAM_ST0, (long)&((GrVertex *)0)->tmuvtx[0].sow, GR_PARAM_ENABLE);

	// disable all unused vertex parameters
	grVertexLayout(GR_PARAM_ST1, 0, GR_PARAM_DISABLE);
	grVertexLayout(GR_PARAM_Z, 0, GR_PARAM_DISABLE);
	grVertexLayout(GR_PARAM_W, 0, GR_PARAM_DISABLE);
	grVertexLayout(GR_PARAM_FOG_EXT, 0, GR_PARAM_DISABLE);
	grVertexLayout(GR_PARAM_PARGB, 0, GR_PARAM_DISABLE);
	grVertexLayout(GR_PARAM_Q0, 0, GR_PARAM_DISABLE);
	grVertexLayout(GR_PARAM_Q1, 0, GR_PARAM_DISABLE);
#else
	// disable some hints
//	grHints(GR_HINT_STWHINT, 0);
//	grHints(GR_HINT_ALLOW_MIPMAP_DITHER, 0);
#endif // USE_GLIDE3

	grSstOrigin(GR_ORIGIN_LOWER_LEFT);

	SetClipWindow(viewport_x, viewport_y, viewport_width, viewport_height);

	grAlphaBlendFunction(GR_BLEND_ONE, GR_BLEND_ZERO, GR_BLEND_ONE, GR_BLEND_ZERO);
	grAlphaCombine(GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE, GR_COMBINE_LOCAL_ITERATED, GR_COMBINE_OTHER_NONE, FXFALSE);
	grAlphaTestFunction(GR_CMP_ALWAYS);
	grAlphaTestReferenceValue(0xff);
	grColorCombine(GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE, GR_COMBINE_LOCAL_ITERATED, GR_COMBINE_OTHER_NONE, FXFALSE);
	grColorMask(FXTRUE, FXFALSE);
	grConstantColorValue(0xffffffff);
	grCullMode(GR_CULL_NEGATIVE);
	grDepthBiasLevel(0);
	grDepthBufferFunction(GR_CMP_LEQUAL);
	grDepthBufferMode(GR_DEPTHBUFFER_WBUFFER);
	grDepthMask(FXTRUE);
	grDitherMode(GR_DITHER_4x4);
	grFogColorValue(0);
	grFogMode(GR_FOG_DISABLE);

	grTexClampMode(GR_TMU0, GR_TEXTURECLAMP_WRAP, GR_TEXTURECLAMP_WRAP);
	grTexCombine(GR_TMU0, GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE, GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE, FXFALSE, FXFALSE);
	grTexDetailControl(GR_TMU0, 0, 0, 0.0f);
	grTexFilterMode(GR_TMU0, GR_TEXTUREFILTER_BILINEAR, GR_TEXTUREFILTER_BILINEAR);
	grTexLodBiasValue(GR_TMU0, 0.5f);
	grTexMipMapMode(GR_TMU0, GR_MIPMAP_NEAREST, FXFALSE);

	// this never changes, so set it once here
	grTexCombine(GR_TMU0,
							 GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE,
							 GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE,
							 FXFALSE, FXFALSE);

#ifdef USE_GLIDE3
	grDisable(GR_AA_ORDERED);
	grDisable(GR_ALLOW_MIPMAP_DITHER);

	// no need to disable PASSTHRU, and it might hang cards that don't support passthru (like Rush)
//	grEnable(GR_PASSTHRU);

	// no need for this either
//	grDisable(GR_VIDEO_SMOOTHING);

	// don't change the shameless plug enable/disable
	// this'll be done from the app, otherwise fxSplashPlug will not be called
	// if we disable it here, or will always be on if we enable it here
//	grDisable(GR_SHAMELESS_PLUG);
#endif // USE_GLIDE3
}

RevBool GlideGraphics::Open( void *hwnd_ /*HWND hwnd_*/ )
{
  hwnd = ( HWND )hwnd_;

#ifndef SPLASH_DLL

  grGlideInit();

  grSstSelect( 0 );

#else // !SPLASH_DLL

#ifdef USE_GLIDE3
	long size;
	if (!grGet(GR_GLIDE_STATE_SIZE, 4, &size))
	{
		return REVFALSE;
	}
	oldState = new FxU8[size];
	if (!oldState)
	{
		return REVFALSE;
	}

	if (!grGet(GR_GLIDE_VERTEXLAYOUT_SIZE, 4, &size))
	{
		return REVFALSE;
	}
	oldVertState = new FxU8[size];
	if (!oldVertState)
	{
		return REVFALSE;
	}
#endif // USE_GLIDE3

#endif // !SPLASH_DLL

#ifdef USE_GLIDE3
	// set the number of TMUs available
	if (grGet(GR_NUM_TMU, 4, (long *)&gNumTMUs) != 4)
	{
		return REVFALSE;
	}

	if (grGet(GR_WDEPTH_MIN_MAX, 8, gDepthMinMax) != 8)
	{
		return REVFALSE;
	}
#else
  if (!grSstQueryHardware( &hwconfig ))
	{
		return REVFALSE;
	}

	// set the number of TMUs available
	switch (hwconfig.SSTs[0].type)
	{
		case GR_SSTTYPE_VOODOO:
			gNumTMUs = hwconfig.SSTs[0].sstBoard.VoodooConfig.nTexelfx;
			break;

		case GR_SSTTYPE_SST96:
			gNumTMUs = hwconfig.SSTs[0].sstBoard.SST96Config.nTexelfx;
			break;

		case GR_SSTTYPE_AT3D:
			gNumTMUs = 1;
			break;

		case GR_SSTTYPE_Voodoo2:
			gNumTMUs = hwconfig.SSTs[0].sstBoard.Voodoo2Config.nTexelfx;
			break;

		default:
			gNumTMUs = 1;
			break;
	}

	gDepthMinMax[0] = 0;
	gDepthMinMax[1] = 0xffff;
#endif // USE_GLIDE3

  if (hwnd == NULL)
  {
		fullscreen = REVTRUE;
  }
  else
  {
		fullscreen = REVFALSE;
  }

  /*
	* Run in a window if you can, otherwise run fullscreen.
	*/
#ifndef SPLASH_DLL
#ifdef USE_GLIDE3
	if (!(context =
#else
	if (!(
#endif
		grSstWinOpen((FxU32)hwnd, GR_RESOLUTION_NONE, GR_REFRESH_60Hz, GR_COLORFORMAT_ARGB, GR_ORIGIN_LOWER_LEFT, 2, 1)))
	{
#ifdef USE_GLIDE3
			context = 
#endif
				grSstWinOpen((FxU32)hwnd, GR_RESOLUTION_640x480, GR_REFRESH_60Hz, GR_COLORFORMAT_ARGB, GR_ORIGIN_LOWER_LEFT, 2, 1);
		fullscreen = REVTRUE;
	}
#endif // !SPLASH_DLL

	grBufferClear( 0, 0, gDepthMinMax[1]);
	grBufferSwap( 1 );
	grBufferClear( 0, 0, gDepthMinMax[1]);

	return REVTRUE;
}

RevBool GlideGraphics::Close( void )
{
#ifndef SPLASH_DLL

#ifdef USE_GLIDE3
  grSstWinClose(context);
#else
  grSstWinClose();
#endif // USE_GLIDE3
  grGlideShutdown();

#else // !SPLASH_DLL

#ifdef USE_GLIDE3
	if (oldState)
	{
		delete [] oldState;
		oldState = NULL;
	}
	if (oldVertState)
	{
		delete [] oldVertState;
		oldVertState = NULL;
	}
#endif // USE_GLIDE3

#endif // SPLASH_DLL

  return REVTRUE;
}

void GlideGraphics::FadeIn(float fade, float r, float g, float b)
{
	GrVertex verts[4];

	grColorCombine(GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE, GR_COMBINE_LOCAL_ITERATED, GR_COMBINE_OTHER_TEXTURE, FXFALSE);
	grAlphaCombine(GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE, GR_COMBINE_LOCAL_ITERATED, GR_COMBINE_OTHER_TEXTURE, FXFALSE);
	grAlphaBlendFunction(GR_BLEND_SRC_ALPHA, GR_BLEND_ONE_MINUS_SRC_ALPHA, GR_BLEND_ZERO, GR_BLEND_ZERO);

	fade *= 255.0f;

	verts[0].x = (float)viewport_x;
	verts[1].x = (float)(viewport_x + viewport_width);
	verts[2].x = (float)viewport_x;
	verts[3].x = (float)(viewport_x + viewport_width);

#ifdef USE_ORIGIN_UPPER_LEFT
	verts[0].y = (float)(device_height - (viewport_y+viewport_height));
	verts[1].y = (float)(device_height - (viewport_y+viewport_height));
	verts[2].y = (float)(device_height - viewport_y);
	verts[3].y = (float)(device_height - viewport_y);
#else
	verts[0].y = (float)viewport_y;
	verts[1].y = (float)viewport_y;
	verts[2].y = (float)(viewport_y + viewport_height);
	verts[3].y = (float)(viewport_y + viewport_height);
#endif // USE_ORIGIN_UPPER_LEFT

	verts[0].oow = 1.0f;
	verts[0].r = r;
	verts[0].g = g;
	verts[0].b = b;
	verts[0].a = fade;

	verts[1].oow = 1.0f;
	verts[1].r = r;
	verts[1].g = g;
	verts[1].b = b;
	verts[1].a = fade;

	verts[2].oow = 1.0f;
	verts[2].r = r;
	verts[2].g = g;
	verts[2].b = b;
	verts[2].a = fade;

	verts[3].oow = 1.0f;
	verts[3].r = r;
	verts[3].g = g;
	verts[3].b = b;
	verts[3].a = fade;

#ifdef USE_GLIDE3
	grDrawVertexArrayContiguous(GR_TRIANGLE_STRIP, 4, verts, sizeof(GrVertex));
#else
	grDrawTriangle(&verts[0], &verts[1], &verts[2]);
	grDrawTriangle(&verts[1], &verts[3], &verts[2]);
#endif // USE_GLIDE3
}

void GlideGraphics::Swap( int swapinterval )
{
  grBufferSwap( swapinterval );
}

void GlideGraphics::Clear( RevU32 flags, float r, float g, float b )
{
  FxU32 color;

  if( flags & GLIDE_GRAPHICS_CLEAR_COLOR )
	{
		color = 
			( ( int )( r * 255.0f ) << 16 ) |
			( ( int )( g * 255.0f ) << 8 ) |
			( ( int )( b * 255.0f ) << 0 );
	}
  else
    grColorMask( FXFALSE, FXFALSE );

  if (!( flags & GLIDE_GRAPHICS_CLEAR_DEPTH ))
	{
    grDepthMask( FXFALSE );
	}

  grBufferClear( color, 0x0, gDepthMinMax[1]);
  grDepthMask( FXTRUE );
  grColorMask( FXTRUE, FXFALSE );
}

void GlideGraphics::Resize( int width, int height )
{
  device_width = viewport_width = width;
  device_height = viewport_height = height;
#ifndef USE_GLIDE3
  grSstControl( GR_CONTROL_RESIZE );
#endif
  SetClipWindow(0, 0, device_width, device_height);
}

void GlideGraphics::Move( void )
{
#ifndef USE_GLIDE3
  if( !fullscreen )
    grSstControl( GR_CONTROL_MOVE );
#endif
}

//void GlideGraphics::DrawLine( RevPoint *p1, RevPoint *p2, RevU32 color )
void GlideGraphics::DrawLine( RevVert *p1, RevVert *p2, RevU32 color )
{
  GrVertex a, b;
  RevFloat *modelview_mat = matrix_stack[GLIDE_MODELVIEW].mat[matrix_stack[GLIDE_MODELVIEW].top_of_stack];
  
  /*
	* Transform from world space to view space.
	*/
#define M(row,col)  modelview_mat[col*4+row]
#define P(offset)   ( p1->pos.Get( offset ) )
  a.x = M(0,0) * P(0) + M(0,1) * P(1) + M(0,2) * P(2) + M(0,3);
  a.y = M(1,0) * P(0) + M(1,1) * P(1) + M(1,2) * P(2) + M(1,3);
  a.z = M(2,0) * P(0) + M(2,1) * P(1) + M(2,2) * P(2) + M(2,3);
  a.z = -a.z;
#undef P
	
#define P(offset)   ( p2->pos.Get( offset ) )
  b.x = M(0,0) * P(0) + M(0,1) * P(1) + M(0,2) * P(2) + M(0,3);
  b.y = M(1,0) * P(0) + M(1,1) * P(1) + M(1,2) * P(2) + M(1,3);
  b.z = M(2,0) * P(0) + M(2,1) * P(1) + M(2,2) * P(2) + M(2,3);
  b.z = -b.z;
#undef M
#undef P
	
  if( a.z < NEAR_Z || b.z < NEAR_Z )
    return;
	
		/*
		* project
	*/
  a.oow = 1.0f / a.z;
  a.tmuvtx[0].sow = p1->texvert.Get( 0 ) * a.oow;
  a.tmuvtx[0].tow = p1->texvert.Get( 1 ) * a.oow;
  a.x = a.x * a.oow;
  a.y = a.y * a.oow;
  b.oow = 1.0f / b.z;
  b.tmuvtx[0].sow = p2->texvert.Get( 0 ) * b.oow;
  b.tmuvtx[0].tow = p2->texvert.Get( 1 ) * b.oow;
  b.x = b.x * b.oow;
  b.y = b.y * b.oow;
  
  /* 
	* viewport transform.
	*/
  a.x = a.x * x_scale + x_offset;
  a.y = a.y * y_scale + y_offset;
	
  b.x = b.x * x_scale + x_offset;
  b.y = b.y * y_scale + y_offset;

#ifndef USE_GLIDE3
  a.x -= SNAP_BIAS;
  a.y -= SNAP_BIAS;
  b.x -= SNAP_BIAS;
  b.y -= SNAP_BIAS;
#endif // !USE_GLIDE3
	
  RevFloat t;
	
  if( a.x < 0.0f && b.x < 0.0f )
    return;
  if( a.x > viewport_width && b.x > viewport_width )
    return;
  if( a.y < 0.0f && b.y < 0.0f )
    return;
  if( a.y > viewport_height && b.y > viewport_height )
    return;
	
  /* clip left. */
  if( a.x < 0.0f )
	{
		t = ( 0.0f - a.x ) / ( b.x - a.x );
		a.x = 0.0f;
		a.y = a.y + ( b.y - a.y ) * t;
		a.oow = a.oow + ( b.oow - a.oow ) * t;
		a.tmuvtx[0].sow = a.tmuvtx[0].sow + ( b.tmuvtx[0].sow - a.tmuvtx[0].sow ) * t;
		a.tmuvtx[0].tow = a.tmuvtx[0].tow + ( b.tmuvtx[0].tow - a.tmuvtx[0].tow ) * t;
	}
	
  if( b.x < 0.0f )
	{
		t = ( 0.0f - b.x ) / ( a.x - b.x );
		b.x = 0.0f;
		b.y = b.y + ( a.y - b.y ) * t;
		b.oow = b.oow + ( a.oow - b.oow ) * t;
		b.tmuvtx[0].sow = b.tmuvtx[0].sow + ( a.tmuvtx[0].sow - b.tmuvtx[0].sow ) * t;
		b.tmuvtx[0].tow = b.tmuvtx[0].tow + ( a.tmuvtx[0].tow - b.tmuvtx[0].tow ) * t;
	}
	
  /* clip bottom. */
  if( a.y < 0.0f )
	{
		t = ( 0.0f - a.y ) / ( b.y - a.y );
		a.y = 0.0f;
		a.x = a.x + ( b.x - a.x ) * t;
		a.oow = a.oow + ( b.oow - a.oow ) * t;
		a.tmuvtx[0].sow = a.tmuvtx[0].sow + ( b.tmuvtx[0].sow - a.tmuvtx[0].sow ) * t;
		a.tmuvtx[0].tow = a.tmuvtx[0].tow + ( b.tmuvtx[0].tow - a.tmuvtx[0].tow ) * t;
	}
	
  if( b.y < 0.0f )
	{
		t = ( 0.0f - b.y ) / ( a.y - b.y );
		b.y = 0.0f;
		b.x = b.x + ( a.x - b.x ) * t;
		b.oow = b.oow + ( a.oow - b.oow ) * t;
		b.tmuvtx[0].sow = b.tmuvtx[0].sow + ( a.tmuvtx[0].sow - b.tmuvtx[0].sow ) * t;
		b.tmuvtx[0].tow = b.tmuvtx[0].tow + ( a.tmuvtx[0].tow - b.tmuvtx[0].tow ) * t;
	}
	
  /*
	* clip right.
	*/
  if( b.x > viewport_width )
	{
		t = ( viewport_width - a.x ) / ( b.x - a.x );
		b.x = ( float )viewport_width;
		b.y = a.y + ( b.y - a.y ) * t;
		b.oow = a.oow + ( b.oow - a.oow ) * t;
		b.tmuvtx[0].sow = a.tmuvtx[0].sow + ( b.tmuvtx[0].sow - a.tmuvtx[0].sow ) * t;
		b.tmuvtx[0].tow = a.tmuvtx[0].tow + ( b.tmuvtx[0].tow - a.tmuvtx[0].tow ) * t;
	}
	
  if( a.x > viewport_width )
	{
		t = ( viewport_width - b.x ) / ( a.x - b.x );
		a.x = ( float )viewport_width;
		a.y = b.y + ( a.y - b.y ) * t;
		a.oow = b.oow + ( a.oow - b.oow ) * t;
		a.tmuvtx[0].sow = b.tmuvtx[0].sow + ( a.tmuvtx[0].sow - b.tmuvtx[0].sow ) * t;
		a.tmuvtx[0].tow = b.tmuvtx[0].tow + ( a.tmuvtx[0].tow - b.tmuvtx[0].tow ) * t;
	}
	
  /*
	* clip top.
	*/
  if( b.y > viewport_height )
	{
		t = ( viewport_height - a.y ) / ( b.y - a.y );
		b.y = ( float )viewport_height;
		b.x = a.x + ( b.x - a.x ) * t;
		b.oow = a.oow + ( b.oow - a.oow ) * t;
		b.tmuvtx[0].sow = a.tmuvtx[0].sow + ( b.tmuvtx[0].sow - a.tmuvtx[0].sow ) * t;
		b.tmuvtx[0].tow = a.tmuvtx[0].tow + ( b.tmuvtx[0].tow - a.tmuvtx[0].tow ) * t;
	}
	
  if( a.y > viewport_height )
	{
		t = ( viewport_height - b.y ) / ( a.y - b.y );
		a.y = ( float )viewport_height;
		a.x = b.x + ( a.x - b.x ) * t;
		a.oow = b.oow + ( a.oow - b.oow ) * t;
		a.tmuvtx[0].sow = b.tmuvtx[0].sow + ( a.tmuvtx[0].sow - b.tmuvtx[0].sow ) * t;
		a.tmuvtx[0].tow = b.tmuvtx[0].tow + ( a.tmuvtx[0].tow - b.tmuvtx[0].tow ) * t;
	}
	
  // hack
  grConstantColorValue( color );
  //  guColorCombineFunction( GR_COLORCOMBINE_CCRGB );
	
#ifndef USE_GLIDE3
  a.x += SNAP_BIAS;
  a.x -= SNAP_BIAS;
	
  a.y += SNAP_BIAS;
  a.y -= SNAP_BIAS;
	
  b.x += SNAP_BIAS;
  b.x -= SNAP_BIAS;
	
  b.y += SNAP_BIAS;
  b.y -= SNAP_BIAS;
#endif // !USE_GLIDE3
	
#if 1
  if( a.x < -1.0f || a.x > viewport_width + 1 || a.y < -1.0f || a.y > viewport_height + 1 ||
		b.x < -1.0f || b.x > viewport_width + 1 || b.y < -1.0f || b.y > viewport_height + 1 )
	{
		//      TRACE( "viewport: %f %f\n", ( float )viewport_width, ( float )viewport_height );
		//      ASSERT( 0 );
		return;
	}
#endif
	
#if 0
  // hack
  a.tmuvtx[0].sow = 0.0f;
  a.tmuvtx[0].tow = 0.0f;
  a.oow = 1.0f;
	
  b.tmuvtx[0].sow = 255.0f;
  b.tmuvtx[0].tow = 255.0f;
  b.oow = 1.0f;
#endif
	
  grDrawLine( &a, &b );
}

void GlideGraphics::MatrixMode( RevU32 mode )
{
  current_matrix_stack = &matrix_stack[mode];
}

void GlideGraphics::LoadIdentity( void )
{
  MakeIdentMatrix( current_matrix_stack->mat[current_matrix_stack->top_of_stack] );
  RebuildCompositeMatrix();
}

void GlideGraphics::Perspective( RevFloat fovy, RevFloat aspect, RevFloat zNear,
																RevFloat zFar )
{
  RevFloat xmin, xmax, ymin, ymax;
  
  ymax = zNear * ( float )tan( fovy * M_PI / 360.0 );
  ymin = -ymax;
  
  xmin = ymin * aspect;
  xmax = ymax * aspect;
  
  Frustum( xmin, xmax, ymin, ymax, zNear, zFar );
}

void GlideGraphics::Frustum( RevFloat left, RevFloat right, 
														RevFloat bottom, RevFloat top, 
														RevFloat nearval, RevFloat farval )
{
  RevFloat x, y, a, b, c, d;
  RevGlideMatrix m;

#ifdef PRINT_DEBUG
	if (right-left == 0.0f || top-bottom == 0.0f || farval-nearval == 0.0f)
	{
		fprintf(fp, "divide by zero: Frustum\n");
		fflush(fp);
	}

  if( nearval <= 0.0f || farval <= 0.0f ) 
	{
		fprintf(fp, "GlideGraphics::Frustum nearval/farval out of range\n");
		fflush(fp);
	}
#endif // PRINT_DEBUG
	
	x = ( 2.0f * nearval ) / ( right - left );
	y = ( 2.0f * nearval ) / ( top - bottom );
	a = ( right + left ) / ( right - left );
	b = ( top + bottom ) / ( top - bottom );
	c = -( farval + nearval ) / ( farval - nearval );
	d = -( 2.0f * farval * nearval ) / ( farval-nearval );
	
#define M(row,col)  m[col*4+row]
	M(0,0) = x;     M(0,1) = 0.0F;  M(0,2) = a;      M(0,3) = 0.0F;
	M(1,0) = 0.0F;  M(1,1) = y;     M(1,2) = b;      M(1,3) = 0.0F;
	M(2,0) = 0.0F;  M(2,1) = 0.0F;  M(2,2) = c;      M(2,3) = d;
	M(3,0) = 0.0F;  M(3,1) = 0.0F;  M(3,2) = -1.0F;  M(3,3) = 0.0F;
#undef M
	
	MultMatrix( m );
}

/*
* Hack: This is hacked to force a 90 fov in x since the Frustum/ortho stuff
* isn't used yet in the pipeline.
*/
void GlideGraphics::Viewport( RevI32 x, RevI32 y, RevI32 width, RevI32 height )
{
  viewport_x = x;
	viewport_y = y;
  viewport_width = width;
  viewport_height = height;

	SetClipWindow(viewport_x, viewport_y, viewport_width, viewport_height);
}

void GlideGraphics::Ortho( RevFloat left, RevFloat right, RevFloat bottom, RevFloat top, 
													RevFloat zNear, RevFloat zFar )
{
  RevGlideMatrix m;
  RevFloat tx, ty, tz;

#ifdef PRINT_DEBUG
	if (right-left == 0.0f || top-bottom == 0.0f || zFar-zNear == 0.0f)
	{
		fprintf(fp, "divide by zero: Frustum\n");
		fflush(fp);
	}
#endif // PRINT_DEBUG

  tx = -( right + left ) / ( right - left );
  ty = -( top + bottom ) / ( top - bottom );
  tz = -( zFar + zNear ) / ( zFar - zNear );
	
#define M(row,col)  m[col*4+row]
	M(0,0) = 2.0f / ( right - left ); M(0,1) = 0.0F;                    M(0,2) = 0.0f;                    M(0,3) = tx;
	M(1,0) = 0.0F;                    M(1,1) = 2.0f / ( top - bottom ); M(1,2) = 0.0f;                    M(1,3) = ty;
	M(2,0) = 0.0F;                    M(2,1) = 0.0F;                    M(2,2) = 2.0f / ( zFar - zNear ); M(2,3) = tz;
	M(3,0) = 0.0F;                    M(3,1) = 0.0F;                    M(3,2) = 0.0F;                    M(3,3) = 1.0F;
#undef M
	
	MultMatrix( m );
}

void GlideGraphics::Translate( RevFloat x, RevFloat y, RevFloat z )
{
  RevGlideMatrix m;
	
  /*
	* Gee, this could be a little faster.
	*/
#define M(row,col)  m[col*4+row]
  M(0,0) = 1.0f; M(0,1) = 0.0f; M(0,2) = 0.0f; M(0,3) = x;
  M(1,0) = 0.0f; M(1,1) = 1.0f; M(1,2) = 0.0f; M(1,3) = y;
  M(2,0) = 0.0f; M(2,1) = 0.0f; M(2,2) = 1.0f; M(2,3) = z;
  M(3,0) = 0.0f; M(3,1) = 0.0f; M(3,2) = 0.0f; M(3,3) = 1.0f;
#undef M
  MultMatrix( m );
}

void GlideGraphics::Rotate( RevFloat angle, RevFloat x, RevFloat y, RevFloat z )
{
  RevGlideMatrix m;
  RevFloat rad;
  RevFloat s, c, omc;
	
  if( angle == 0.0f )
    return;
	
  rad = DEG2RAD( angle );
  s = ( float )sin( rad );
  c = ( float )cos( rad );
  omc = 1.0f - c;
	
#define M(row,col)  m[col*4+row]
  M(0,0) = x*x*omc+c;   M(0,1) = x*y*omc-z*s; M(0,2) = x*z*omc+y*s; M(0,3) = 0.0f;
  M(1,0) = y*x*omc+z*s; M(1,1) = y*y*omc+c;   M(1,2) = y*z*omc-x*s; M(1,3) = 0.0f;
  M(2,0) = x*z*omc-y*s; M(2,1) = y*z*omc+x*s; M(2,2) = z*z*omc+c;   M(2,3) = 0.0f;
  M(3,0) = 0.0f;        M(3,1) = 0.0f;        M(3,2) = 0.0f;        M(3,3) = 1.0f;
#undef M
  MultMatrix( m );
}

void GlideGraphics::PushMatrix( void )
{
  if( ( current_matrix_stack->top_of_stack++ ) > GLIDE_GLIDE_MATRIX_STACK_DEPTH )
	{
		GLIDE_ERROR(( "GlideGraphics::PushMatrix",
			"Stack overflow" ));
	}
	
  memcpy( current_matrix_stack->mat[current_matrix_stack->top_of_stack],
		current_matrix_stack->mat[current_matrix_stack->top_of_stack-1],          
		sizeof( RevFloat[16] ) );
}

void GlideGraphics::PopMatrix( void )
{
  if( ( current_matrix_stack->top_of_stack-- ) < 0 )
	{
		GLIDE_ERROR(( "GlideGraphics::PopMatrix",
			"Stack underflow" ));
	}
  RebuildCompositeMatrix();
}

void GlideGraphics::MultMatrix( const RevFloat *m )
{
  RevGlideMatrix tmp;
  RevFloat *left;
  int r, c;
	
  left = current_matrix_stack->mat[current_matrix_stack->top_of_stack];
	
#define L(row,col)  m[col*4+row]
#define R(row,col)  left[col*4+row]
#define T(row,col)  tmp[col*4+row]
	
  for( r = 0; r < 4; r++ ) 
	{
		for( c = 0; c < 4; c++ ) 
		{
			T(r,c) = 
				L(0,c) * R(r,0) +
				L(1,c) * R(r,1) +
				L(2,c) * R(r,2) +
				L(3,c) * R(r,3);
		}
	}
#undef R
#undef L
#undef T
	
  memcpy( left, tmp, sizeof( RevGlideMatrix ) );
  RebuildCompositeMatrix();
}


void GlideGraphics::GetFloatv( RevU32 pname, RevFloat *params )
{
  switch( pname )
	{
	case GLIDE_MODELVIEW_MATRIX:
		memcpy( params, 
			matrix_stack[GLIDE_MODELVIEW].mat[matrix_stack[GLIDE_MODELVIEW].top_of_stack], 
			sizeof( RevFloat[16] ) );
		break;
	case GLIDE_DEVICE_WIDTH:
		*params = ( float )device_width;
		break;
	case GLIDE_DEVICE_HEIGHT:
		*params = ( float )device_height;
		break;
	default:
		GLIDE_ERROR(( "GlideGraphics::GetFloatv",
			"unknown pname" ));
		break;
	}
}

void GlideGraphics::MatrixMultiply( RevFloat *dest, const RevFloat *left, RevFloat *right )
{
  RevGlideMatrix tmp;
  int r, c;
	
  // Hack: this seems backwards.
	
#define L(row,col)  right[col*4+row]
#define R(row,col)  left[col*4+row]
#define T(row,col)  tmp[col*4+row]
	
  for( r = 0; r < 4; r++ ) 
	{
		for( c = 0; c < 4; c++ ) 
		{
			T(r,c) = 
				L(0,c) * R(r,0) +
				L(1,c) * R(r,1) +
				L(2,c) * R(r,2) +
				L(3,c) * R(r,3);
		}
	}
#undef R
#undef L
#undef T
	
  memcpy( dest, tmp, sizeof( RevGlideMatrix ) );
}

void GlideGraphics::RebuildCompositeMatrix( void )
{
/*
* composite_matrix = projection * modelview
* (composite_matrix = projection * worldview * modelview)
	*/
  MatrixMultiply( composite_matrix, 
		matrix_stack[GLIDE_PROJECTION].mat[matrix_stack[GLIDE_PROJECTION].top_of_stack],
		matrix_stack[GLIDE_MODELVIEW].mat[matrix_stack[GLIDE_MODELVIEW].top_of_stack] );
}

void GlideGraphics::RenderBuffer( RevU32 flags )
{
  if( flags & GLIDE_BUFFER_FRONT )
    grRenderBuffer( GR_BUFFER_FRONTBUFFER );
  if( flags & GLIDE_BUFFER_BACK )
    grRenderBuffer( GR_BUFFER_BACKBUFFER );
}


#define GLIDE_MAX_NUM_FACE_VERTS 100
#define SIDE_OUT 0
#define SIDE_IN  1

void GlideGraphics::ComputeLighting(float *diffuse, float *specular, RevVert *vert, float light_dir[3])
{
	float reflection[3], oomag;

	// light the vert.
	*diffuse = vert->normal.GetX()*light_dir[0] + vert->normal.GetY()*light_dir[1] + vert->normal.GetZ()*light_dir[2];

	// find the reflection of the light about the normal
	reflection[0] = 2.0f*(*diffuse)*vert->normal.GetX() - light_dir[0];
	reflection[1] = 2.0f*(*diffuse)*vert->normal.GetY() - light_dir[1];
	reflection[2] = 2.0f*(*diffuse)*vert->normal.GetZ() - light_dir[2];
	oomag = fsqrt_inv(reflection[0]*reflection[0] + reflection[1]*reflection[1] + reflection[2]*reflection[2]);
	reflection[0] *= oomag;
	reflection[1] *= oomag;
	reflection[2] *= oomag;

	*specular = gXformedCameraDir[0]*reflection[0] + gXformedCameraDir[1]*reflection[1] + gXformedCameraDir[2]*reflection[2];

	if (*(int *)diffuse < 0)
	{
		*diffuse = 0.0f;
	}

	if (*(int *)specular <= 0)
	{
		*specular = 0.0f;
	}
	else
	{
		*specular *= *specular;
		*specular *= *specular;
		*specular *= *specular; // exponential
	}
}

void GlideGraphics::DrawPolygon( int num_verts, RevVert *vert_ptrs[], unsigned int flags )
{
	/*
	* Get info about the current transform.
	*/
	static RevVert unclipped_verts[GLIDE_MAX_NUM_FACE_VERTS];
	static GrVertex unclipped_xformed_verts[GLIDE_MAX_NUM_FACE_VERTS];
	static GrVertex clipped_xformed_verts[GLIDE_MAX_NUM_FACE_VERTS];
	RevFloat *modelview_mat = matrix_stack[GLIDE_MODELVIEW].mat[matrix_stack[GLIDE_MODELVIEW].top_of_stack];
	int vertnum, clipped_vertnum, unclipped_vertnum;
	float oomag, omni_delta[3], diffuse0, specular0, diffuse1, specular1;
	const float sqrt3_inv = 1.0f/(float)sqrt(3.0);

	dirlight_direction[0] = sqrt3_inv;
	dirlight_direction[1] = sqrt3_inv;
	dirlight_direction[2] = -sqrt3_inv;

	unclipped_vertnum = num_verts;

	for( vertnum = 0; vertnum < num_verts; vertnum++ )
	{
		RevVert *model_vert = vert_ptrs[vertnum];
		RevVert *unclipped_vert = &unclipped_verts[vertnum];
		GrVertex *vert = &unclipped_xformed_verts[vertnum];

		/*
		* Transform from object space to view space.
		*/
#define M(row,col)  modelview_mat[col*4+row]
#define P(offset)   ( model_vert->pos.Get( offset ) )
		//      *unclipped_vert = verts[vertnum];
		unclipped_vert->texvert.Set( vert_ptrs[vertnum]->texvert.GetX() * smult, vert_ptrs[vertnum]->texvert.GetY() * tmult, 0.0f );
		unclipped_vert->pos.SetX( M(0,0) * P(0) + M(0,1) * P(1) + M(0,2) * P(2) + M(0,3) );
		unclipped_vert->pos.SetY( M(1,0) * P(0) + M(1,1) * P(1) + M(1,2) * P(2) + M(1,3) );
		unclipped_vert->pos.SetZ( M(2,0) * P(0) + M(2,1) * P(1) + M(2,2) * P(2) + M(2,3) );
		unclipped_vert->pos.SetZ( -unclipped_vert->pos.GetZ() );
#undef M
#undef P

		/*
		* Transform normal from object space to view space.
		*/
#define M(row,col)  modelview_mat[col*4+row]
#define P(offset)   ( model_vert->normal.Get( offset ) )
		unclipped_vert->normal.SetX( M(0,0) * P(0) + M(0,1) * P(1) + M(0,2) * P(2) );
		unclipped_vert->normal.SetY( M(1,0) * P(0) + M(1,1) * P(1) + M(1,2) * P(2) );
		unclipped_vert->normal.SetZ( M(2,0) * P(0) + M(2,1) * P(1) + M(2,2) * P(2) );
		unclipped_vert->normal.SetZ( -unclipped_vert->normal.GetZ() );
#undef M
#undef P

		// normalize the normal
		oomag = fsqrt_inv( unclipped_vert->normal.GetX() * unclipped_vert->normal.GetX() +
			unclipped_vert->normal.GetY() * unclipped_vert->normal.GetY() +
			unclipped_vert->normal.GetZ() * unclipped_vert->normal.GetZ() );

		unclipped_vert->normal.SetX( unclipped_vert->normal.GetX() * oomag );
		unclipped_vert->normal.SetY( unclipped_vert->normal.GetY() * oomag );
		unclipped_vert->normal.SetZ( unclipped_vert->normal.GetZ() * oomag );

		vert->z = unclipped_vert->GetZ();

		vert->oow = 1.0f / vert->z;
		vert->x = unclipped_vert->GetX() * vert->oow;
		vert->y = unclipped_vert->GetY() * vert->oow;
		if (flags & ENVMAP)
		{
//			vert->tmuvtx[1].sow = 256.0f*(0.5f + 0.5f*unclipped_vert->normal.GetX()) * vert->oow;
//			vert->tmuvtx[1].tow = 256.0f*(0.5f + 0.5f*unclipped_vert->normal.GetY()) * vert->oow;
			float x0, y0, z0, nx, ny, nz;
			float a, b, c, t;

			// find the intersection of the line with endpoint at the vertex
			// and direction in the direction of the vertex normal with a sphere
			// centered at the origin, with radius ENVMAP_RADIUS
			// use the intersection of the line and the sphere as the (s, t) coords
			// into the envmap texture
			x0 = unclipped_vert->pos.GetX();
			y0 = unclipped_vert->pos.GetY();
			z0 = unclipped_vert->pos.GetZ();
			nx = unclipped_vert->normal.GetX();
			ny = unclipped_vert->normal.GetY();
			nz = unclipped_vert->normal.GetZ();

			// solve the quadratic equation:
			// t^2 * (nx^2 + ny^2 + nz^2) + t * (2*x0*nx + 2*y0*ny + 2*z0*nz) + (x0^2 + y0^2 + z0^2 - r^2) = 0
			// t = (-b +/- sqrt(b^2 - 4*a*c)) / 2*a
			a = nx*nx + ny*ny + nz*nz;
			b = 2.0f*(x0*nx + y0*ny + z0*nz);
			c = x0*x0 + y0*y0 + z0*z0 - ENVMAP_RADIUS*ENVMAP_RADIUS;
			t = b*b - 4*a*c;
			if (t > 0.0f) // make sure the line intersects the sphere
			{
				t = (-b + (float)sqrt(t))/(2.0f*a);

				// point on the sphere
				x0 = x0 + t*nx;
				y0 = y0 + t*ny;

				// normalize
				x0 *= 1.0f/ENVMAP_RADIUS;
				y0 *= 1.0f/ENVMAP_RADIUS;

				vert->tmuvtx[0].sow = ENVMAP_TEX_COORD_SCALE*256.0f*(0.5f + 0.5f*x0) * vert->oow;
				vert->tmuvtx[0].tow = ENVMAP_TEX_COORD_SCALE*256.0f*(0.5f + 0.5f*y0) * vert->oow;
			}
			else // this line doesn't intersect the sphere!!!
			{
				vert->tmuvtx[0].sow = 0.0f;
				vert->tmuvtx[0].tow = 0.0f;
			}
		}
		else
		{
			vert->tmuvtx[0].sow = unclipped_vert->GetU() * vert->oow;
			vert->tmuvtx[0].tow = unclipped_vert->GetV() * vert->oow;
		}

		// scale to viewport
		vert->x = vert->x * x_scale + x_offset;
		vert->y = vert->y * y_scale + y_offset;
#ifndef USE_GLIDE3
		vert->x -= SNAP_BIAS;
		vert->y -= SNAP_BIAS;
#endif // !USE_GLIDE3

#ifdef PLAYBACK

		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

		// dir light
		ComputeLighting(&diffuse0, &specular0, unclipped_vert, dirlight_direction);

		// omni light
		omni_delta[0] = omnilight_position[0] - unclipped_vert->pos.GetX();
		omni_delta[1] = omnilight_position[1] - unclipped_vert->pos.GetY();
		omni_delta[2] = omnilight_position[2] - unclipped_vert->pos.GetZ();
		oomag = fsqrt_inv( omni_delta[0] * omni_delta[0] + omni_delta[1] * omni_delta[1] + omni_delta[2] * omni_delta[2] );
		omni_delta[0] *= oomag;
		omni_delta[1] *= oomag;
		omni_delta[2] *= oomag;
		ComputeLighting(&diffuse1, &specular1, unclipped_vert, omni_delta);

		// falloff for the omni light
		const float FF = 2000.0f*2000.0f;
		float dist_sqr = (omnilight_position[0] - unclipped_vert->pos.GetX())*(omnilight_position[0] - unclipped_vert->pos.GetX()) +
										 (omnilight_position[1] - unclipped_vert->pos.GetY())*(omnilight_position[1] - unclipped_vert->pos.GetY()) +
										 (omnilight_position[2] - unclipped_vert->pos.GetZ())*(omnilight_position[2] - unclipped_vert->pos.GetZ());

		if (FF < dist_sqr)
		{
			float f = FF/dist_sqr;
			f *= f;
			f *= f;
			specular1 *= f;
			diffuse1 *= f;
		}

		const float AMBIENT_COLOR = 16.0f;
		vert->r = AMBIENT_COLOR + 255.0f*(diffuse0 + specular0) + omnilight_color[0]*(diffuse1 + specular1);
		vert->g = AMBIENT_COLOR + 255.0f*(diffuse0 + specular0) + omnilight_color[1]*(diffuse1 + specular1);
		vert->b = AMBIENT_COLOR + 255.0f*(diffuse0 + specular0) + omnilight_color[2]*(diffuse1 + specular1);
		vert->a = 255.0f;

		// clamp the colors to [0, 255]
		// they shouldn't be < 0, since diffuse and specular are both clamped to > 0
		if( vert->r > 255.0f )
			vert->r = 255.0f;

		if( vert->g > 255.0f )
			vert->g = 255.0f;

		if( vert->b > 255.0f )
			vert->b = 255.0f;

		if( vert->a > 255.0f )
			vert->a = 255.0f;

		/*
		{
			GrVertex v;
			float vv[3];

			if (gNormals)
			{
				// draw the normal
				vv[0] = unclipped_vert->pos.GetX() + 5.0f*unclipped_vert->normal.GetX();
				vv[1] = unclipped_vert->pos.GetY() + 5.0f*unclipped_vert->normal.GetY();
				vv[2] = unclipped_vert->pos.GetZ() + 5.0f*unclipped_vert->normal.GetZ();
				v.oow = 1.0f/vv[2];
				v.x = vv[0]*v.oow;
				v.y = vv[1]*v.oow;
				v.x = v.x * x_scale + x_offset;
				v.y = v.y * y_scale + y_offset;
#ifndef USE_GLIDE3
				v.x -= SNAP_BIAS;
				v.y -= SNAP_BIAS;
#endif // !USE_GLIDE3
				v.tmuvtx[0].sow = v.tmuvtx[0].tow = 0.0f;
				v.r = v.g = v.b = v.a = 255.0f;
				grDrawLine(vert, &v);
			}
#if 0
			// draw the reflection
			vv[0] = unclipped_vert->pos.GetX() + 20.0f*reflection[0];
			vv[1] = unclipped_vert->pos.GetY() + 20.0f*reflection[1];
			vv[2] = unclipped_vert->pos.GetZ() + 20.0f*reflection[2];
			v.oow = 1.0f/vv[2];
			v.x = vv[0]*v.oow;
			v.y = vv[1]*v.oow;
			v.x = v.x * x_scale + x_offset;
			v.y = v.y * y_scale + y_offset;
#ifndef USE_GLIDE3
			v.x -= SNAP_BIAS;
			v.y -= SNAP_BIAS;
#endif // !USE_GLIDE3
			v.tmuvtx[0].sow = v.tmuvtx[0].tow = 0.0f;
			v.g = v.a = 255.0f;
			v.r = v.b = 0.0f;
			grDrawLine(vert, &v);
#endif
#if 0
			// draw the light dir
			vv[0] = unclipped_vert->pos.GetX() + 20.0f*omni_delta[0];
			vv[1] = unclipped_vert->pos.GetY() + 20.0f*omni_delta[1];
			vv[2] = unclipped_vert->pos.GetZ() + 20.0f*omni_delta[2];
			v.oow = 1.0f/vv[2];
			v.x = vv[0]*v.oow;
			v.y = vv[1]*v.oow;
			v.x = v.x * x_scale + x_offset;
			v.y = v.y * y_scale + y_offset;
#ifndef USE_GLIDE3
			v.x -= SNAP_BIAS;
			v.y -= SNAP_BIAS;
#endif // !USE_GLIDE3
			v.tmuvtx[0].sow = v.tmuvtx[0].tow = 0.0f;
			v.r = v.a = 255.0f;
			v.g = v.b = 0.0f;
			grDrawLine(vert, &v);
#endif
#if 0
			// draw the camera dir
			vv[0] = unclipped_vert->pos.GetX() + 50.0f*camera[0];
			vv[1] = unclipped_vert->pos.GetY() + 50.0f*camera[1];
			vv[2] = unclipped_vert->pos.GetZ() + 50.0f*camera[2];
			v.oow = 1.0f/vv[2];
			v.x = vv[0]*v.oow;
			v.y = vv[1]*v.oow;
			v.x = v.x * x_scale + x_offset;
			v.y = v.y * y_scale + y_offset;
#ifndef USE_GLIDE3
			v.x -= SNAP_BIAS;
			v.y -= SNAP_BIAS;
#endif // !USE_GLIDE3
			v.tmuvtx[0].sow = v.tmuvtx[0].tow = 0.0f;
			v.r = v.g = v.b = v.a = 255.0f;
			grDrawLine(vert, &v);
#endif
		}
		*/
#endif // PLAYBACK
	}

	// backface cull
	float v0x, v0y, v1x, v1y, normz;
	v0x = unclipped_xformed_verts[0].x - unclipped_xformed_verts[1].x;
	v0y = unclipped_xformed_verts[0].y - unclipped_xformed_verts[1].y;
	v1x = unclipped_xformed_verts[2].x - unclipped_xformed_verts[1].x;
	v1y = unclipped_xformed_verts[2].y - unclipped_xformed_verts[1].y;
	normz = v0x*v1y - v0y*v1x;
	if (normz >= 0) return;

	// clip the vertices and create a clipped polygon
#if 1
	clipped_vertnum = ClipPolyVerts(clipped_xformed_verts, unclipped_vertnum, unclipped_xformed_verts);
#else
	clipped_vertnum = unclipped_vertnum;
	for (vertnum=0; vertnum<clipped_vertnum; vertnum++)
	{
		clipped_xformed_verts[vertnum] = unclipped_xformed_verts[vertnum];
	}
#endif

	if (flags & DO_DRAW)
	{
		/*
		if (gWireframe)
		{
			for (vertnum=0; vertnum<clipped_vertnum-1; vertnum++)
				ClipAndDrawLine(&clipped_xformed_verts[vertnum], &clipped_xformed_verts[vertnum+1]);
			// last one wraps
			ClipAndDrawLine(&clipped_xformed_verts[clipped_vertnum-1], &clipped_xformed_verts[0]);
		}
		else
		*/
		{
#ifdef USE_GLIDE3
			grDrawVertexArrayContiguous(GR_POLYGON, clipped_vertnum, clipped_xformed_verts, sizeof(GrVertex));
#else
			grDrawPolygonVertexList( clipped_vertnum, clipped_xformed_verts );
#endif // USE_GLIDE3
		}
	}

#ifdef PLAYBACK
	// add vertices to the antialias edge list
	if (flags & ANTI_ALIAS)
		for (vertnum = 0; vertnum < clipped_vertnum-1; vertnum++) {
			AddEdge(clipped_xformed_verts[vertnum],clipped_xformed_verts[vertnum+1], gCurrMaterialID, REMOVE_DUPLICATE_EDGES);
		}
		// last one wraps
		AddEdge(clipped_xformed_verts[clipped_vertnum-1],clipped_xformed_verts[0], gCurrMaterialID, REMOVE_DUPLICATE_EDGES);
#endif
}

// hack: have to set this stuff after setting the viewport

void GlideGraphics::RecalcFOV( void )
{
  //  printf( "%f %f\n", fovx, fovy );
  //90 -> 1.0f
  //0  -> inf
  //180 -> 0
	float tan_angle;

	tan_angle = ( float )tan( fovx * .5f * M_PI / 180.0f );

#ifdef PRINT_DEBUG
	if (tan_angle == 0.0f)
	{
		fprintf(fp, "divide by zero: tan: fovx = %f\n", fovx);
		fflush(fp);
	}
#endif // PRINT_DEBUG

  x_scale = .5f * viewport_width * ( 1.0f / tan_angle );
  x_offset = ( viewport_x + viewport_width * 0.5f );
  //  y_scale = viewport_width * 0.5f * tan( .5f * fovx * M_PI / 180.0f );
  y_scale = x_scale;
  y_offset = ( viewport_y + viewport_height * 0.5f );

#ifdef USE_ORIGIN_UPPER_LEFT
	y_offset = device_height - y_offset;
#endif // USE_ORIGIN_UPPER_LEFT

#ifndef USE_GLIDE3
	x_offset += SNAP_BIAS;
	y_offset += SNAP_BIAS;
#endif // !USE_GLIDE3
}

void GlideGraphics::SetFovX( RevFloat fovx_ )
{
  fovx = fovx_;
  fovy = ( float )viewport_height / ( float )viewport_width * fovx;
  RecalcFOV();
}

void GlideGraphics::SetFovY( RevFloat fovy_ )
{
  fovy = fovy_;
  fovx = ( float )viewport_width / ( float )viewport_height * fovy;
  RecalcFOV();
}

void GlideGraphics::SetFov( RevFloat fovx_, RevFloat fovy_ )
{
  fovx = fovx_;
  fovy = fovy_;
  RecalcFOV();
}

void GlideGraphics::ObjectToView( RevPoint& object_point, RevPoint& view_point )
{
  RevFloat *modelview_mat = matrix_stack[GLIDE_MODELVIEW].mat[matrix_stack[GLIDE_MODELVIEW].top_of_stack];
	/*
	* Transform from object space to view space.
	*/
#define M(row,col)  modelview_mat[col*4+row]
#define P(offset)   ( object_point.Get( offset ) )
  view_point.SetX( M(0,0) * P(0) + M(0,1) * P(1) + M(0,2) * P(2) + M(0,3) );
  view_point.SetY( M(1,0) * P(0) + M(1,1) * P(1) + M(1,2) * P(2) + M(1,3) );
  view_point.SetZ( M(2,0) * P(0) + M(2,1) * P(1) + M(2,2) * P(2) + M(2,3) );
  view_point.SetZ( -view_point.GetZ() );
#undef M
#undef P
}

#ifdef PLAYBACK

/*
Fades to black over two seconds according to the time base of the frame rate

*/
extern int frame_rate;

void GlideGraphics::FadeToBlack()
{
	int i, swap_rate;
	float alpha = 0.0f;

	GrVertex v0,v1,v2,v3;

	grColorCombine(GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE, GR_COMBINE_LOCAL_ITERATED, GR_COMBINE_OTHER_NONE, FXFALSE);
	grAlphaCombine(GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE, GR_COMBINE_LOCAL_ITERATED, GR_COMBINE_OTHER_NONE, FXFALSE);
	grAlphaBlendFunction(GR_BLEND_ONE,GR_BLEND_ONE_MINUS_SRC_ALPHA,GR_BLEND_ONE,GR_BLEND_ZERO);

	v0.oow = v1.oow = v2.oow =v3.oow = 1.0f;

	v0.r = v1.r = v2.r = v3.r = 0.0f;
	v0.g = v1.g = v2.g = v3.g = 0.0f;
	v0.b = v1.b = v2.b = v3.b = 0.0f;

	//	__asm int 3

	v0.x = (float)(viewport_x);
	v1.x = (float)(viewport_x + viewport_width);
	v2.x = (float)(viewport_x + viewport_width);
	v3.x = (float)(viewport_x);

#ifdef USE_ORIGIN_UPPER_LEFT
	v0.y = (float)(device_height - (viewport_y+viewport_height));
	v1.y = (float)(device_height - (viewport_y+viewport_height));
	v2.y = (float)(device_height - viewport_y);
	v3.y = (float)(device_height - viewport_y);
#else
	v0.y = (float)(viewport_y);
	v1.y = (float)(viewport_y);
	v2.y = (float)(viewport_y + viewport_height);
	v3.y = (float)(viewport_y + viewport_height);
#endif // USE_ORIGIN_UPPER_LEFT

	grDitherMode(GR_DITHER_DISABLE);

	swap_rate = 60 / frame_rate;
	for (i = 0; i < 2 * frame_rate; i++)
	{

		// Check the current time and wait if we are ahead of the 
		// desired animation rate.

		v0.a = v1.a = v2.a = v3.a = alpha;

		grDrawTriangle(&v0,&v1,&v2);
		grDrawTriangle(&v0,&v2,&v3);

		grBufferSwap(2);

		alpha = (102.0f / (2 * frame_rate)) * i;
		if (alpha > 255.0f) alpha = 255.0f;
	}

}

#endif
