/*
** Copyright (c) 1996, 3Dfx Interactive, Inc.
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
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
** rights reserved under the Copyright Laws of the United States.
**
**
** $Revision: 4$ 
** $Date: 10/11/00 7:29:42 PM$ 
**
*/

#include <3dfx.h>
#ifdef GLIDE3
#include <glide3.h>
#else
#include <glide.h>
#endif
#include <atrender.h>
#include <atinput.h>
#include <atdemo.h>

#include <string.h>
#include <time.h>

#define _WIN32_LEAN_AND_MEAN_
#include <windows.h>
#include <commctrl.h>


#include <atgui.h>
#include "resource.h"



/*--------------------------------------------------------------
  UI Data
  --------------------------------------------------------------*/
static Dialog   *mainDlg;
static Progress *progress;
static ListBox  *results;
static Button   *okButton;
static Button   *cancelButton;
static Edit     *numTestsEdit;
static Slider   *numTestsSlider;

// mt - matrix test
static Button   *mtButton;
static Edit     *mtCallsPerSec;
static Edit     *mtClocksPerOp;
static Edit     *mtTrisPerSec;
static Button   *mtDetails;

// gt - glide triangle test
static Button   *gtButton;
static Edit     *gtCallsPerSec;
static Edit     *gtClocksPerOp;
static Edit     *gtTrisPerSec;

// rts - atr triangle test 
static Button   *rtsButton;
static Edit     *rtsCallsPerSec;
static Edit     *rtsClocksPerOp;
static Edit     *rtsTrisPerSec;

// mat - atr material test 
static Button   *matButton;
static Edit     *matCallsPerSec;
static Edit     *matClocksPerOp;
static Edit     *matTrisPerSec;

// xf - atr xform test
static Button   *xfButton;
static Edit     *xfCallsPerSec;
static Edit     *xfClocksPerOp;
static Edit     *xfTrisPerSec;

/*--------------------------------------------------------------
  Benchmark Data
  --------------------------------------------------------------*/
typedef float Matrix[4][4];
typedef float Vector[4];

#define MIN_NUM_ITERATIONS 1000
#define MAX_NUM_ITERATIONS 10000000

static FxU32    sNumTests = MIN_NUM_ITERATIONS;
static char     sNumTestsString[32];

static FxU16             sTexData[256*256*2];
static GrHwConfiguration sHwConfig;

/*--------------------------------------------------------------
  Benchmark Implementation
  --------------------------------------------------------------*/
static void multVecMat( Vector dest, Matrix mat, Vector src ) {
    dest[0] = mat[0][0] * src[0] + mat[1][0] * src[1] + 
              mat[2][0] * src[2] + mat[3][0] * src[3];
    dest[1] = mat[0][1] * src[0] + mat[1][1] * src[1] + 
              mat[2][1] * src[2] + mat[3][1] * src[3];
    dest[2] = mat[0][2] * src[0] + mat[1][2] * src[1] + 
              mat[2][2] * src[2] + mat[3][2] * src[3];
    dest[3] = mat[0][3] * src[0] + mat[1][3] * src[1] + 
              mat[2][3] * src[2] + mat[3][3] * src[3];
    return;
}

/*--------------------------------------------------------------
  Initialize ATB rendering library
  --------------------------------------------------------------*/

FxBool AppInitGraphics(void) {
    return FXTRUE; /* initialize graphics later */
}

void
AppTermGraphics(void) {
    /* nothing to do */
}

static void 
InitATB(void) {
	FxU32 displayWidth, displayHeight;
	static AtrCanvas  *canvas = NULL;

	_atGlobals.driverInfo.hWnd = _atGlobals.hWndMain;
    _atGlobals.driverInfo.emulation = _atGlobals.emulation;
    _atGlobals.driverInfo.refreshRate = GR_REFRESH_60Hz,
    _atGlobals.driverInfo.numBuffers = 1; /* double buffer */
    _atGlobals.driverInfo.smoothingMode = GR_SMOOTHING_ENABLE;
    _atGlobals.driverInfo.info = NULL;

    if ( (_atGlobals.ctx = atrInit( _atGlobals.driverName,
                                    &_atGlobals.driverInfo) ) == NULL ) {
		atuError(FXTRUE, "Could not initialize rendering\n");
    }

	if ( canvas == NULL ) {
		canvas = atrCanvasAllocate( 1 );
	}

	/* configure the canvas */

	atrBeginScene(&displayWidth, &displayHeight);

	canvas->xMin = 0;
	canvas->yMin = 0;
	canvas->xMax = displayWidth-1;
	canvas->yMax = displayHeight-1;

	atrSelectCanvas( canvas );
    atrQueryDriverCaps( &_atGlobals.caps );
    _atGlobals.bFullScreen = _atGlobals.caps.fullScreen;
    _atGlobals.graphicsEnabled = FXTRUE;
}

static void
TermATB(void) {
	atrEndScene();
    atrSwapBuffer(1);
    _atGlobals.graphicsEnabled = FXFALSE;
	atrShutdown();
}

static float randFloat( float min, float max ) {
    float r = (float)rand();
    r /= (float)RAND_MAX;
    r *= max - min;
    r += min;
    return r;
}

void doMtBench( float *cps, float *cpo, float *tps ) {
    float  startTime, endTime, ticksPerSec = (float)CLOCKS_PER_SEC;
    FxU32  startClocksLo, endClocksLo;
    FxU32  startClocksHi, endClocksHi;
    _int64 startClocks, endClocks;
    FxU32  calls;
    char   message[256];
    double totalClocks;
    FxU32  row, col;
    Vector src, dest;
    Matrix mat;

    progressSetRange( progress, 0, 100 );
    progressSetPos( progress, 0 );

    listBoxReset( results );
    
    // Initialize Source Data
    listBoxAddString( results, "4x4 Matrix Vector Mult" );
    for( row = 0; row < 4; row++ ) {
        src[row] = randFloat( 0.0f, 65535.0f );
        dest[row] = randFloat( 0.0f, 65535.0f );
        for( col = 0; col < 4; col++ ) {
            mat[row][col] = randFloat( 0.0f, 65535.0f );
        }
    }

    listBoxAddString( results, "Running Test, Please Wait...." );
    listBoxPaint( results );

    progressSetPos( progress, 0 );

    calls = sNumTests;
    startTime = (float)clock();
    atuTscRead( &startClocksLo, &startClocksHi );

    while( --calls ) 
      multVecMat( dest, mat, src );

    atuTscRead( &endClocksLo, &endClocksHi );
    endTime = (float)clock();

    endClocks = endClocksHi;
    endClocks <<= 32;
    endClocks |= endClocksLo;

    startClocks = startClocksHi;
    startClocks <<= 32;
    startClocks |= startClocksLo;

    endClocks -= startClocks;
    totalClocks = (double)endClocks;
    totalClocks /= (double)sNumTests;
    *cps = (float)(((float)sNumTests) / ((endTime - startTime)/ticksPerSec));
    *cpo = (float)totalClocks;
    *tps = 0.0f;
    sprintf( message, "Completed." );
    listBoxAddString( results, message );
    return;
}

static void generateTexture( FxU16 *data ) {
    FxU32 lod;
    for( lod = 256; lod > 0; lod >>= 1 ) {
        FxU32 y;
        for( y = 0; y < lod; y++ ) {
            FxU32 x;
            for( x = 0; x < lod; x++ ) {
                *data++ = (FxU16) (( y << 8 ) + x);
            }
        }
    }
}

void doGtBench( float *cps, float *cpo, float *tps ) {
    float startTime, endTime, ticksPerSec = (float)CLOCKS_PER_SEC;
    FxU32 startClocksLo, endClocksLo;
    FxU32 startClocksHi, endClocksHi;
    _int64 startClocks, endClocks;
    FxU32 calls;
    char  message[256];
    double totalClocks;
    GrVertex v[3];

    GrTexInfo texInfo;

    listBoxReset( results );

    progressSetRange( progress, 0, 100 );
    progressSetPos( progress, 0 );
    
    // Initialize Source Data
    listBoxAddString( results, "Glide Triangle Test" );
    listBoxPaint( results );

    v[0].x   =  10.0f;
    v[0].y   =  10.0f;
    v[0].oow =   8.0f;
    v[0].r   = 255.0f;
    v[0].g   =   0.0f;
    v[0].b   =   0.0f;
    v[0].tmuvtx[0].sow = 0.0f;
    v[0].tmuvtx[0].tow = 0.0f;
    
    v[1].x   =  15.0f;
    v[1].y   =  15.0f;
    v[1].oow =   8.0f;
    v[1].r   =   0.0f;
    v[1].g   = 255.0f;
    v[1].b   =   0.0f;
    v[1].tmuvtx[0].sow = 16.0f;
    v[1].tmuvtx[0].tow = 16.0f;
    
    v[2].x   =  15.0f;
    v[2].y   =  10.0f;
    v[2].oow =   8.0f;
    v[2].r   =   0.0f;
    v[2].g   = 255.0f;
    v[2].b   =   0.0f;
    v[2].tmuvtx[0].sow = 16.0f;
    v[2].tmuvtx[0].tow =  0.0f;

    listBoxAddString( results, "Setting Up Hardware" );
    listBoxPaint( results );

    grGlideInit();
    if ( !grSstQueryHardware( &sHwConfig ) ) {
        MessageBox( 0, "Couldn't configure hardware.\n", 
                    "Benchmark Error", MB_OK|MB_ICONERROR );
        grGlideShutdown();
        return;
    }

    grSstSelect( 0 );

    progressSetPos( progress, 90 );

    if ( !grSstOpen( GR_RESOLUTION_640x480,
                     GR_REFRESH_60Hz,
                     GR_COLORFORMAT_ARGB,
                     GR_ORIGIN_LOWER_LEFT,
                     GR_SMOOTHING_ENABLE,
                     2 ) ) {
        MessageBox( 0, "Couldn't configure hardware.\n", 
                    "Benchmark Error", MB_OK|MB_ICONERROR );
        grGlideShutdown();
        return;
    }

    progressSetPos( progress, 95 );

    grRenderBuffer( GR_BUFFER_FRONTBUFFER );
    grDepthBufferMode( GR_DEPTHBUFFER_WBUFFER );
    grDepthBufferFunction( GR_CMP_LESS );
    grDepthMask( FXTRUE );
    grBufferClear( 0x00800000, 0, GR_WDEPTHVALUE_FARTHEST );
    grColorCombine( GR_COMBINE_FUNCTION_SCALE_OTHER, 
                    GR_COMBINE_FACTOR_LOCAL,
                    GR_COMBINE_LOCAL_ITERATED,
                    GR_COMBINE_OTHER_TEXTURE,
                    FXFALSE );
    grAlphaCombine( GR_COMBINE_FUNCTION_SCALE_OTHER,
                    GR_COMBINE_FACTOR_ONE,
                    GR_COMBINE_LOCAL_NONE,
                    GR_COMBINE_OTHER_TEXTURE,
                    FXFALSE );
    grTexCombine( GR_TMU0,
                  GR_COMBINE_FUNCTION_LOCAL,
                  GR_COMBINE_FACTOR_ZERO,
                  GR_COMBINE_FUNCTION_LOCAL,
                  GR_COMBINE_FACTOR_ZERO,
                  FXFALSE,
                  FXFALSE );
    grTexClampMode( GR_TMU0,
                    GR_TEXTURECLAMP_WRAP, 
                    GR_TEXTURECLAMP_WRAP );
    grTexMipMapMode( GR_TMU0,
                     GR_MIPMAP_NEAREST, 
                     0 );
    grTexFilterMode( GR_TMU0,
                     GR_TEXTUREFILTER_BILINEAR, 
                     GR_TEXTUREFILTER_BILINEAR );

    texInfo.smallLod    = GR_LOD_1;
    texInfo.largeLod    = GR_LOD_256;
    texInfo.aspectRatio = GR_ASPECT_1x1;
    texInfo.format      = GR_TEXFMT_RGB_565;
    texInfo.data        = &sTexData;

    generateTexture( sTexData );

    grTexDownloadMipMap( GR_TMU0, 0, GR_MIPMAPLEVELMASK_BOTH, &texInfo );
    grTexSource( GR_TMU0, 0, GR_MIPMAPLEVELMASK_BOTH, &texInfo );

    progressSetPos( progress, 100 );

    listBoxAddString( results, "Running Test, Please Wait...." );
    listBoxPaint( results );

    progressSetPos( progress, 0 );

    calls = sNumTests;
    startTime = (float)clock();
    atuTscRead( &startClocksLo, &startClocksHi );
    while( --calls ) {
        grDrawTriangle( &v[0], &v[1], &v[2] );
    }
    atuTscRead( &endClocksLo, &endClocksHi );
    endTime = (float)clock();

    endClocks = endClocksHi;
    endClocks <<= 32;
    endClocks |= endClocksLo;

    startClocks = startClocksHi;
    startClocks <<= 32;
    startClocks |= startClocksLo;

    endClocks -= startClocks;
    totalClocks = (double)endClocks;
    totalClocks /= (double)sNumTests;

    *cps = (float)(((float)sNumTests) / ((endTime - startTime)/ticksPerSec));
    *cpo = (float)totalClocks;
    *tps = *cps;

    sprintf( message, "Completed." );
    listBoxAddString( results, message );

    grGlideShutdown();
    return;
}

static FxU32 queryTris( AtrTriSet *set ) {
    FxU32 numTris = 0;
    _AtrTriSetNode *node = set->nodes;
    while( node ) {
        FxU32 *triPtr = node->connectivity;
        while( *(triPtr++) ) numTris++;
        node = node->next;
    }
    return numTris;
}

void doRtsBench( float *cps, float *cpo, float *tps ) {
    float startTime, endTime, ticksPerSec = (float)CLOCKS_PER_SEC;
    FxU32 startClocksLo, endClocksLo;
    FxU32 startClocksHi, endClocksHi;
    _int64 startClocks, endClocks;
    FxU32 calls;
    char  message[256];
    double totalClocks;
    AtrMaterial  mat;
    AtrImg       img;
    AtrTexHandle tex;
    static AtrTriSet *triSet;
    FxU32        numTris;
    AtrLight     light;
    AtrXform     xform;
    AtrEnv       env;
    FxU32        numTests;

    listBoxReset( results );

    progressSetRange( progress, 0, 100 );
    progressSetPos( progress, 0 );
    
    // Initialize Source Data
    listBoxAddString( results, "Atr Triangle Test" );
    listBoxPaint( results );

    listBoxAddString( results, "Setting Up Hardware" );
    listBoxPaint( results );

    InitATB();

    memset( &img, 0, sizeof( img ) );
    atrImgDefault( &img );
    img.format  = ATR_IMGFMT_RGB_565;
    img.width   = 256;
    img.height  = 256;
    img.nLevels = 1;
    img.name    = "none";
    img.data    = sTexData;
    img.table   = 0;

    generateTexture( sTexData );

    tex = atrTexNewHandle( ATR_TEXELFX_0 );
    atrTexAssociate( tex, &img );

    atrMaterialDefault( &mat );
    atrMaterialSetup( &mat, ATR_MAT_DECAL_X_LIGHTING );
    mat.texture[0] = tex;
    mat.diffuse.r = 1.0f;
    mat.diffuse.g = 1.0f;
    mat.diffuse.b = 1.0f;
    atrPushMaterial( &mat );
    

    atrLightDefault( &light );
    light.flags = ATR_LIGHT_DIRECTED;
    light.color.r = 1.0f;
    light.color.g = 1.0f;
    light.color.b = 1.0f;
    atrAddLight( &light, 0 );

    atrXformSetIdentity( &xform );
    atrXformSetTranslation( &xform, 0.0f, 0.0f, 10.0f );
    atrPushXform( &xform );

    atrEnvDefault( &env );
    env.flags = ATR_HSR_WBUFFER;
    atrPushEnv( &env );

    atrRenderBuffer( ATR_BUFFER_FRONTBUFFER );

    atrClearCanvas( 0.0f, 0.0f, 1.0f, ATR_WBUFFER_CLEAR );

    // Create Tri Set if not extant 
    if ( !triSet ) {
        triSet = atrPrimSphere( 1.0f, 20, 20 );
    }

    numTris = queryTris( triSet );
    numTests = sNumTests / numTris;
    sprintf( message, "Num Tris: %d", numTris );
    listBoxAddString( results, message );
    listBoxPaint( results );

    progressSetPos( progress, 100 );

    listBoxAddString( results, "Running Test, Please Wait...." );
    listBoxPaint( results );

    progressSetPos( progress, 0 );

    calls = numTests;
    startTime = (float)clock();
    atuTscRead( &startClocksLo, &startClocksHi );
    while( --calls ) {
        atrRenderTriSet( triSet );
    }
    atuTscRead( &endClocksLo, &endClocksHi );
    endTime = (float)clock();

    endClocks = endClocksHi;
    endClocks <<= 32;
    endClocks |= endClocksLo;

    startClocks = startClocksHi;
    startClocks <<= 32;
    startClocks |= startClocksLo;

    endClocks -= startClocks;
    totalClocks = (double)endClocks;
    totalClocks /= (double)(numTests*(float)numTris);

    *cps = (float)(((float)numTests) / ((endTime - startTime)/ticksPerSec));
    *cpo = (float)totalClocks;
    *tps = ((float)(((float)numTests) / 
                    ((endTime - startTime)/ticksPerSec)))*numTris;

    sprintf( message, "Completed." );
    listBoxAddString( results, message );

    TermATB();

    return;
}

void doMatBench( float *cps, float *cpo, float *tps ) {
    float startTime, endTime, ticksPerSec = (float)CLOCKS_PER_SEC;
    FxU32 startClocksLo, endClocksLo;
    FxU32 startClocksHi, endClocksHi;
    _int64 startClocks, endClocks;
    FxU32 calls;
    char  message[256];
    double totalClocks;
    AtrMaterial  mat;
    AtrImg       img;
    AtrTexHandle tex;
    AtrLight     light;
    AtrXform     xform;
    AtrEnv       env;
    FxU32        numTests = sNumTests;

    listBoxReset( results );

    progressSetRange( progress, 0, 100 );
    progressSetPos( progress, 0 );
    
    // Initialize Source Data
    listBoxAddString( results, "Atr Push Material Test" );
    listBoxPaint( results );

    listBoxAddString( results, "Setting Up Hardware" );
    listBoxPaint( results );

    InitATB();

    memset( &img, 0, sizeof( img ) );
    atrImgDefault( &img );
    img.format  = ATR_IMGFMT_RGB_565;
    img.width   = 256;
    img.height  = 256;
    img.nLevels = 1;
    img.name    = "none";
    img.data    = sTexData;
    img.table   = 0;

    generateTexture( sTexData );

    tex = atrTexNewHandle( ATR_TEXELFX_0 );
    atrTexAssociate( tex, &img );

    atrMaterialDefault( &mat );
    atrMaterialSetup( &mat, ATR_MAT_DECAL_X_LIGHTING );
    mat.texture[0] = tex;
    mat.diffuse.r = 1.0f;
    mat.diffuse.g = 1.0f;
    mat.diffuse.b = 1.0f;
    atrPushMaterial( &mat );

    atrLightDefault( &light );
    light.flags = ATR_LIGHT_DIRECTED;
    light.color.r = 1.0f;
    light.color.g = 1.0f;
    light.color.b = 1.0f;
    atrAddLight( &light, 0 );

    atrXformSetIdentity( &xform );
    atrXformSetXRotation( &xform, 30.0f * ATM_DEGREE );
    xform.data[12] =  5.3211f;
    xform.data[13] = -8.34f;
    xform.data[14] =  7.0113f;
    atrXformSetTranslation( &xform, 0.0f, 0.0f, 10.0f );
    atrPushXform( &xform );

    atrEnvDefault( &env );
    env.flags = ATR_HSR_WBUFFER;
    atrPushEnv( &env );

    atrRenderBuffer( ATR_BUFFER_FRONTBUFFER );

    atrClearCanvas( 0.0f, 0.0f, 1.0f, ATR_WBUFFER_CLEAR );

    progressSetPos( progress, 100 );

    listBoxAddString( results, "Running Test, Please Wait...." );
    listBoxPaint( results );

    progressSetPos( progress, 0 );

    calls = numTests;
    startTime = (float)clock();
    atuTscRead( &startClocksLo, &startClocksHi );
    while( --calls ) {
        atrPushMaterial( &mat );
        atrPopMaterial( 0 );
    }
    atuTscRead( &endClocksLo, &endClocksHi );
    endTime = (float)clock();

    endClocks = endClocksHi;
    endClocks <<= 32;
    endClocks |= endClocksLo;

    startClocks = startClocksHi;
    startClocks <<= 32;
    startClocks |= startClocksLo;

    endClocks -= startClocks;
    totalClocks = (double)endClocks;
    totalClocks /= (double)(numTests);

    *cps = (float)(((float)numTests) / ((endTime - startTime)/ticksPerSec));
    *cpo = (float)totalClocks;
    *tps = ((float)(((float)numTests) / 
                    ((endTime - startTime)/ticksPerSec)));

    sprintf( message, "Completed." );
    listBoxAddString( results, message );

    TermATB();

    return;
}

void doXfBench( float *cps, float *cpo, float *tps ) {
    float startTime, endTime, ticksPerSec = (float)CLOCKS_PER_SEC;
    FxU32 startClocksLo, endClocksLo;
    FxU32 startClocksHi, endClocksHi;
    _int64 startClocks, endClocks;
    FxU32 calls;
    char  message[256];
    double totalClocks;
    AtrMaterial  mat;
    AtrImg       img;
    AtrTexHandle tex;
    AtrLight     light;
    AtrXform     xform;
    AtrEnv       env;
    FxU32        numTests = sNumTests;

    listBoxReset( results );

    progressSetRange( progress, 0, 100 );
    progressSetPos( progress, 0 );
    
    // Initialize Source Data
    listBoxAddString( results, "Atr Push Material Test" );
    listBoxPaint( results );

    listBoxAddString( results, "Setting Up Hardware" );
    listBoxPaint( results );

    InitATB();

    memset( &img, 0, sizeof( img ) );
    atrImgDefault( &img );
    img.format  = ATR_IMGFMT_RGB_565;
    img.width   = 256;
    img.height  = 256;
    img.nLevels = 1;
    img.name    = "none";
    img.data    = sTexData;
    img.table   = 0;

    generateTexture( sTexData );

    tex = atrTexNewHandle( ATR_TEXELFX_0 );
    atrTexAssociate( tex, &img );

    atrMaterialDefault( &mat );
    atrMaterialSetup( &mat, ATR_MAT_DECAL_X_LIGHTING );
    mat.texture[0] = tex;
    mat.diffuse.r = 1.0f;
    mat.diffuse.g = 1.0f;
    mat.diffuse.b = 1.0f;
    atrPushMaterial( &mat );
    

    atrLightDefault( &light );
    light.flags = ATR_LIGHT_DIRECTED;
    light.color.r = 1.0f;
    light.color.g = 1.0f;
    light.color.b = 1.0f;
    atrAddLight( &light, 0 );

    atrXformSetIdentity( &xform );
    atrXformSetXRotation( &xform, 30.0f * ATM_DEGREE );
    xform.data[12] =  5.3211f;
    xform.data[13] = -8.34f;
    xform.data[14] =  7.0113f;
    atrXformSetTranslation( &xform, 0.0f, 0.0f, 10.0f );
    atrPushXform( &xform );

    atrEnvDefault( &env );
    env.flags = ATR_HSR_WBUFFER;
    atrPushEnv( &env );

    atrRenderBuffer( ATR_BUFFER_FRONTBUFFER );

    atrClearCanvas( 0.0f, 0.0f, 1.0f, ATR_WBUFFER_CLEAR );

    progressSetPos( progress, 100 );

    listBoxAddString( results, "Running Test, Please Wait...." );
    listBoxPaint( results );

    progressSetPos( progress, 0 );

    calls = numTests;
    startTime = (float)clock();
    atuTscRead( &startClocksLo, &startClocksHi );
    while( --calls ) {
        atrPushXform( &xform );
        atrPopXform( 0 );
    }
    atuTscRead( &endClocksLo, &endClocksHi );
    endTime = (float)clock();

    endClocks = endClocksHi;
    endClocks <<= 32;
    endClocks |= endClocksLo;

    startClocks = startClocksHi;
    startClocks <<= 32;
    startClocks |= startClocksLo;

    endClocks -= startClocks;
    totalClocks = (double)endClocks;
    totalClocks /= (double)(numTests);

    *cps = (float)(((float)numTests) / ((endTime - startTime)/ticksPerSec));
    *cpo = (float)totalClocks;
    *tps = ((float)(((float)numTests) / 
                    ((endTime - startTime)/ticksPerSec)));

    sprintf( message, "Completed." );
    listBoxAddString( results, message );

    TermATB();

    return;
}

/*--------------------------------------------------------------
  UI Implementation
  --------------------------------------------------------------*/
void doOkButton( WPARAM wParam, LPARAM lParam, void *data ) {
    FXUNUSED( wParam );
    FXUNUSED( lParam );
    dialogDone( mainDlg, FXTRUE );
}

void doCancelButton( WPARAM wParam, LPARAM lParam, void *data ) {
    FXUNUSED( wParam );
    FXUNUSED( lParam );
    dialogDone( mainDlg, FXTRUE );
}


void doMtButton( WPARAM wParam, LPARAM lParam, void *data ) {

    float cps, cpo, tps;
    char  message[256];
    FXUNUSED( wParam );
    FXUNUSED( lParam );

    doMtBench( &cps, &cpo, &tps );

    sprintf( message, "%.1f", cps );
    editSetText( mtCallsPerSec, message );
    sprintf( message, "%.1f", cpo );
    editSetText( mtClocksPerOp, message );
    editSetText( mtTrisPerSec, "N/A" );

    return;
}

void doGtButton( WPARAM wParam, LPARAM lParam, void *data ) {
    float cps, cpo, tps;
    char  message[256];
    FXUNUSED( wParam );
    FXUNUSED( lParam );

    doGtBench( &cps, &cpo, &tps );

    sprintf( message, "%.1f", cps );
    editSetText( gtCallsPerSec, message );
    sprintf( message, "%.1f", cpo );
    editSetText( gtClocksPerOp, message );
    sprintf( message, "%.1f", tps );
    editSetText( gtTrisPerSec, message );

    return;
}


void doRtsButton( WPARAM wParam, LPARAM lParam, void *data ) {
    float cps, cpo, tps;
    char  message[256];
    FXUNUSED( wParam );
    FXUNUSED( lParam );

    doRtsBench( &cps, &cpo, &tps );

    sprintf( message, "%.1f", cps );
    editSetText( rtsCallsPerSec, message );
    sprintf( message, "%.1f", cpo );
    editSetText( rtsClocksPerOp, message );
    sprintf( message, "%.1f", tps );
    editSetText( rtsTrisPerSec, message );

    return;
}

void doMatButton( WPARAM wParam, LPARAM lParam, void *data ) {
    float cps, cpo, tps;
    char  message[256];
    FXUNUSED( wParam );
    FXUNUSED( lParam );

    doMatBench( &cps, &cpo, &tps );

    sprintf( message, "%.1f", cps );
    editSetText( matCallsPerSec, message );
    sprintf( message, "%.1f", cpo );
    editSetText( matClocksPerOp, message );
    sprintf( message, "%.1f", tps );
    editSetText( matTrisPerSec, message );

    return;
}

void doXfButton( WPARAM wParam, LPARAM lParam, void *data ) {
    float cps, cpo, tps;
    char  message[256];
    FXUNUSED( wParam );
    FXUNUSED( lParam );

    doXfBench( &cps, &cpo, &tps );

    sprintf( message, "%.1f", cps );
    editSetText( xfCallsPerSec, message );
    sprintf( message, "%.1f", cpo );
    editSetText( xfClocksPerOp, message );
    sprintf( message, "%.1f", tps );
    editSetText( xfTrisPerSec, message );

    return;
}

void doNumTestsSlider(WPARAM wParam, LPARAM lParam, void *data ) {
    float newPos;
    switch( LOWORD( wParam ) ) {
      case TB_ENDTRACK:
        newPos = (float)sliderGetPos( numTestsSlider );
        newPos /= 100.0f;
        newPos *= MAX_NUM_ITERATIONS - MIN_NUM_ITERATIONS;
        newPos += MIN_NUM_ITERATIONS;
        sNumTests = (FxU32)newPos;
        sprintf( sNumTestsString, "%d", sNumTests );
        editSetText( numTestsEdit, sNumTestsString );
        break;
    }
    return;
}


void doMtDetails(WPARAM wParam, LPARAM lParam, void *data ) {
    FXUNUSED( lParam );
    FXUNUSED( wParam );
    if ( !editObject( 0 , "matdlg\\matdlg.dll", mainDlg->handle ) ) {
        MessageBox( NULL, "editObject failed.\n", "Error", MB_OK|MB_ICONERROR );
    }
    return;
}



/*--------------------------------------------------------------
  Public Interface
  --------------------------------------------------------------*/
void benchCreateControls( Dialog *dlg ) {
    /*-----------------------------------------------------------------
      Attach Dialog Controls To Control Objects
      -----------------------------------------------------------------*/
    okButton        = newButton( dlg, IDOK, doOkButton, NULL );
    cancelButton    = newButton( dlg, IDCANCEL, doCancelButton, NULL );
    progress        = newProgress( dlg, IDC_PROGRESS, NULL );
    results         = newListBox( dlg, IDC_RESULTS, NULL );
    numTestsEdit    = newEdit( dlg, IDC_NUMTESTSEDIT, doNothing, NULL );
    numTestsSlider  = newSlider( dlg, IDC_NUMTESTSSLIDER, doNumTestsSlider, NULL );

    mtButton        = newButton( dlg, IDC_MTBUTTON, doMtButton, NULL );
    mtCallsPerSec   = newEdit( dlg, IDC_MT_CPS, doNothing, NULL );
    mtClocksPerOp   = newEdit( dlg, IDC_MT_CPO, doNothing, NULL );
    mtTrisPerSec    = newEdit( dlg, IDC_MT_TPS, doNothing, NULL );
    mtDetails       = newButton( dlg, IDC_MTDETAILS, doMtDetails, NULL );

    gtButton        = newButton( dlg, IDC_GDTBUTTON, doGtButton, NULL );
    gtCallsPerSec   = newEdit( dlg, IDC_GDT_CPS, doNothing, NULL );
    gtClocksPerOp   = newEdit( dlg, IDC_GDT_CPO, doNothing, NULL );
    gtTrisPerSec    = newEdit( dlg, IDC_GDT_TPS, doNothing, NULL );

    rtsButton        = newButton( dlg, IDC_RTSBUTTON, doRtsButton, NULL );
    rtsCallsPerSec   = newEdit( dlg, IDC_RTS_CPS, doNothing, NULL );
    rtsClocksPerOp   = newEdit( dlg, IDC_RTS_CPO, doNothing, NULL );
    rtsTrisPerSec    = newEdit( dlg, IDC_RTS_TPS, doNothing, NULL );

    matButton        = newButton( dlg, IDC_MATBUTTON, doMatButton, NULL );
    matCallsPerSec   = newEdit( dlg, IDC_MAT_CPS, doNothing, NULL );
    matClocksPerOp   = newEdit( dlg, IDC_MAT_CPO, doNothing, NULL );
    matTrisPerSec    = newEdit( dlg, IDC_MAT_TPS, doNothing, NULL );

    xfButton        = newButton( dlg, IDC_XFBUTTON, doXfButton, NULL );
    xfCallsPerSec   = newEdit( dlg, IDC_XF_CPS, doNothing, NULL );
    xfClocksPerOp   = newEdit( dlg, IDC_XF_CPO, doNothing, NULL );
    xfTrisPerSec    = newEdit( dlg, IDC_XF_TPS, doNothing, NULL );

    sprintf( sNumTestsString, "%d", sNumTests );
    editSetText( numTestsEdit, sNumTestsString );

    sliderSetRange( numTestsSlider, 0, 100 );
    sliderSetPos( numTestsSlider, 0 );

    progressSetRange( progress, 0, 100 );
    progressSetPos( progress, 0 );

    mainDlg = dlg;
    return;
}
