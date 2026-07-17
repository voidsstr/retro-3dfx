/******************************************************************
 * atb_rave.h : header file for the atb rave engine               *
 ******************************************************************/

#ifndef __ATB_RAVE_H__

#define __ATB_RAGE_H__

#include "rave_tc.h"

#ifdef _WINDOWS
typedef RECT RECTANGLE;
#endif

#define MAKE_RGB16(r,g,b)	((r<<10)|(g<<5)|(b))
#define MAKE_RGBA32(r,g,b,a)	((a<<24)|(r<<16)|(g<<8)|(b))

typedef struct 
{
    AtrDriver       any; /* this MUST be first element in structure, CAPS are here */
//AtrDriverCaps  caps;
	TQADrawContext  *ravecontext;
	TQARect	rect;
	TQAEngine		*engine;
}AtrRaveDriver;	

/*** I need this intermediate image struct since the native Rave
 *** image struct doesn't contain the image format info which I needed
 *** when I create the texture. ***/

typedef struct
{
	TQAImage *rave_image;
	TQAImagePixelType rave_format;
	TQABitmap *bitmap;			//rave bitmap
} _AtrXImage;

typedef struct _ATRRAVTE {
    _AtrTexEntry any;		//must be first entry
	_AtrXImage *ximage;			//image that this texture uses
	TQATexture *tex;		
} _AtrRaveTexEntry;


/* Rendering Callbacks  */
AtrVertexFunc _atrRAV_TCSRC0_TC0;
AtrVertexFunc _atrRAV_TCSRC0_TC1;
AtrVertexFunc _atrRAV_TCSRC0_TC2;
AtrVertexFunc _atrRAV_TCSRC0_PROJECTED;
AtrVertexFunc _atrRAV_TCSRC0_PLANAR;
AtrVertexFunc _atrRAV_TCSRC0_EMAP;
AtrVertexFunc _atrRAV_TCSRC1_TC0;
AtrVertexFunc _atrRAV_TCSRC1_TC1;
AtrVertexFunc _atrRAV_TCSRC1_TC2;
AtrVertexFunc _atrRAV_TCSRC1_EMAP;
AtrVertexFunc _atrRAV_TCSRC1_PROJECTED;
AtrVertexFunc _atrRAV_TCSRC1_PLANAR;

void _atrRAVSetRaveTVertex(TQAVTexture_TC *v,const GrVertex *a,long h);

void _atrRAVTransformVertices( AtrDstVertex *dest, AtrVertex *src, 
                                 AtrVertex *end );
void _atrRAVSpecialTransformVertices( AtrDstVertex *dest, AtrVertex *src, 
                                        AtrVertex *end );
void _atrRAVTransformVertices2D( AtrDstVertex *dest, AtrVertex *src, 
                                   AtrVertex *end );
void atrRAVRenderTri( AtrVertex *a, AtrVertex *b, AtrVertex *c );
void atrRAVRenderTriSet( AtrTriSet *t );
void atrRAVSpecialRenderTriSet( AtrTriSet *t );
void atrRAVRenderTriWF( AtrVertex *a, AtrVertex *b, AtrVertex *c );
void atrRAVRenderTriSetWF( AtrTriSet *t );
void atrRAVTransformOpenTriSet( AtrOpenTriSet *t );
void atrRAVSpecialTransformOpenTriSet( AtrOpenTriSet *t );
void atrRAVRenderOpenTriSet( AtrOpenTriSet *t );
void atrRAVRenderOpenTriSetWF( AtrOpenTriSet *t );
void atrRAVRenderSegment( AtrVertex *a, AtrVertex *b );
void atrRAVRender2DTri( AtrVertex *a, AtrVertex *b, AtrVertex *c );
void _atrRAVClipAndRenderSegment(AtrDstVertex *a,AtrDstVertex *b,FxU32 clipCode );
void _atrRAVClipAndRenderTri( AtrDstVertex *a,AtrDstVertex *b,AtrDstVertex *c,FxU32 clipCode );
static FxBool _atrRAVRealizeImg( AtrImg *img ); 
void test_draw(void); 
BOOL flip_image(TQAImage *image);
void _atrRAVSetRaveGVertex(TQAVGouraud *v,const GrVertex *a,long h);
void _atrRAVClipAndRenderTriWF( AtrDstVertex *a,AtrDstVertex *b, AtrDstVertex *c,FxU32 clipCode );  
BOOL _atrRAVIsBackface(const GrVertex *a,const  GrVertex *b,const  GrVertex *c);
void draw_marker_triangle(TQAVTexture_TC v[]);
static void _atrRAVTexSource2( _AtrTexEntry *entry, FxU32 mask );

#endif