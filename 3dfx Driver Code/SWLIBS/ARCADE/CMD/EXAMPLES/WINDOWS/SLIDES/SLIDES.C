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
** $Date: 10/11/00 7:29:31 PM$ 
**
*/

#include <stdio.h>
#include <string.h>
#include <time.h>
#ifdef __WIN32__
#include <conio.h>
#endif
#ifdef GLIDE3
#include <glide3.h>
#else
#include <glide.h>
#endif
#include <atrender.h>
#include <atscene.h>
#include <atinput.h>
#include <atdemo.h>
#include <ataudio.h>

/*
 * GLOBAL VARIABLES
 */


#define MAX_SHAPES 3

AtrCamera   *camera;
AtrLight    *light;
AtrEnv      *env;
AtrStats    stats;
AtrCanvas  *canvas;
FxBool gPaused = FALSE;
AtrImg     *img1, *img2;
AtrMaterial  *imgMaterial;
FxFloat imgAlpha = 1.0f;

#define STEP 0.1f

static char *usage = 
    "";                

void kbFunc(AtiKeyEvent *ev, FxBool preRecorded);

void kbFunc(AtiKeyEvent *ev, FxBool preRecorded) {
    FXUNUSED(preRecorded);

    if ( ev->state == ATI_KEY_PRESS ) {
        
        switch( ev->code ) {
		case ATI_KEY_UP:
			imgAlpha += STEP;
			if ( imgAlpha > 1.0f )
				imgAlpha = 1.0f;
			break;
		case ATI_KEY_DOWN:
			imgAlpha -= STEP;
			if ( imgAlpha < 0.0f )
				imgAlpha = 0.0f;
			break;
        }
    }
}

void
drawImg(AtrImg *i, FxFloat alpha) {
    FxU32 x = 0, y = 0;
    AtrEnv env;
	
    if ( i->width < _atGlobals.caps.width )
        x = (_atGlobals.caps.width - i->width )>>1;

    if ( i->height < _atGlobals.caps.height )
        y = (_atGlobals.caps.height - i->height )>>1;

	imgMaterial->constant = ((FxU32)(alpha*255))<<24;
    atrMaterialModify(imgMaterial);

    env.flags = 0;
    atrPushEnv(&env);
    atrPushMaterial(imgMaterial);
    atrRenderImg(i, x, y);
    atrPopMaterial(FXTRUE);
    atrPopEnv(FXTRUE);
}

void
AppRenderScene( FxU32 displayWidth, FxU32 displayHeight) {
    FXUNUSED (displayWidth);

   /*--------------------------------------------------------
     Clear The WBUFFER and the BACKBUFFER
     --------------------------------------------------------*/

    if ( _atGlobals.fog ) {
        atrClearCanvas( 0.5f, 0.5f, 0.5f, ATR_WBUFFER_CLEAR );
    } else {
        atrClearCanvas( 0.0f, 1.0f, 0.0f, ATR_WBUFFER_CLEAR );
    }
                
    camera->fovRadians = 60.0f*ATM_DEGREE;
    camera->xOffset = 0.f;

    atrSelectCanvas( canvas );
    atrSelectCamera( camera );

    atrPushEnv( env );

    drawImg(img1, imgAlpha);
	drawImg(img2, 1.0f-imgAlpha);

    atrPopEnv( FXTRUE );

	atrDrawString( "Hello World", 30.0f, 30.0f, 0.5f, displayHeight-30.0f);
}

static void
resetFunc(void) {
    _atGlobals.fog = FXFALSE;
}

AtrImg *
loadImage(char *fileName) {
    AtrImg *i;

    i = atrImgAllocate( 1 );
    if ( !atrImgCreateFromFile( i, fileName)) {
        atuError(FXTRUE, "Could not load image");
    }

    return i;
}

void
initImages(void) {
    /* load image data */

    img1 = loadImage( "loading.tga" );
    img2 = loadImage( "fake1.tga" );

    /* Initialize image material */

	imgMaterial = atrMaterialAllocate(1);

    imgMaterial->acuFunction = GR_COMBINE_FUNCTION_SCALE_OTHER;
    imgMaterial->acuFactor = GR_COMBINE_FACTOR_ONE;
    imgMaterial->acuLocal = GR_COMBINE_LOCAL_CONSTANT;
    imgMaterial->acuOther = GR_COMBINE_OTHER_TEXTURE;
    imgMaterial->acuInvert = FXFALSE;

    imgMaterial->ccuFunction =  GR_COMBINE_FUNCTION_SCALE_OTHER;
    imgMaterial->ccuFactor = GR_COMBINE_FACTOR_ONE;
    imgMaterial->ccuLocal = GR_COMBINE_LOCAL_NONE;
    imgMaterial->ccuOther = GR_COMBINE_OTHER_TEXTURE;
    imgMaterial->ccuInvert = FXFALSE;

    imgMaterial->abuSrcFactor = GR_BLEND_SRC_ALPHA;
    imgMaterial->abuDstFactor = GR_BLEND_ONE_MINUS_SRC_ALPHA;

    imgMaterial->depthMask  = FXFALSE;

    imgMaterial->chromaKeyEnable = FXFALSE;
}

static void
AppInit(void) {
    atdSetName("Slides");

    atdEventInit();
    atdKeyboardFunc(kbFunc);
    atdResetFunc(resetFunc);

    if ( !atsInit( ) ) {
      atuError( FXTRUE, "Couldn't initialize scene library.\n" );
    }

    env = atrEnvAllocate( 1 );
    env->flags = ATR_HSR_WBUFFER;

    atuSetLoadPath( getenv( "AT_LOAD_PATH" ) );

    initImages();

    light = atrLightAllocate( 1 );
    light->flags = ATR_LIGHT_DIRECTED;
    light->color.r = 1.0f;
    light->color.g = 1.0f;
    light->color.b = 1.0f;
//    atrXformSetYRotation( &light->lcsToWCS, 30.0f * ATM_DEGREE );

    camera = atrCameraAllocate( 1 );

    canvas      = atrCanvasAllocate( 1 );

    atrStatsReset();
    resetFunc();
}

int
AppMain(int argc, char **argv) {

    FXUNUSED(argc);
    FXUNUSED(argv);

    AppInit();

	atdEventLoop();

    return 1;
}
