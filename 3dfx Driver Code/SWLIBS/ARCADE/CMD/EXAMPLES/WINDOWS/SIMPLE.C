/*
** Copyright (c) 1997, 3Dfx Interactive, Inc.
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
** $Date: 10/11/00 7:29:26 PM$ 
**
*/

#include <atrender.h>
#include <atscene.h>
#include <atinput.h>
#include <atdemo.h>

/*
 * GLOBAL VARIABLES
 */


AtrTriSet   *cubeModel;
AtrCamera   *camera;
AtrMaterial *cubeMaterial;
AtrXform    *toObject;
AtrLight    *light;

float       distance = 10.0f;
float       angle;
float       radius = 1.0f;
AtmVector3 position;
AtrEnv      *env;
AtmVector3  lightPosition  = {  0.0f, 0.0f,  3.0f },
            lightTarget    = {  0.0f,  0.0f,   0.0f },
            up             = {  0.0f,  1.0f,   0.0f };

#define STEP 0.1f

void kbFunc(AtiKeyEvent *ev, FxBool preRecorded);

void kbFunc(AtiKeyEvent *ev, FxBool preRecorded) {
    FXUNUSED(preRecorded);

    if ( ev->state == ATI_KEY_PRESS ) {
        
        switch( ev->code ) {
		case ATI_KEY_UP:
			distance += STEP;
			break;
		case ATI_KEY_DOWN:
			distance -= STEP;
			break;
        }
    }
}

void
AppRenderScene( FxU32 displayWidth, FxU32 displayHeight) {
    FXUNUSED (displayWidth);

   /*--------------------------------------------------------
     Clear The WBUFFER and the BACKBUFFER
     --------------------------------------------------------*/

    atrClearCanvas( 0.2f, 0.2f, 0.2f, ATR_WBUFFER_CLEAR );
                
   /*--------------------------------------------------------
     Orient camera
     --------------------------------------------------------*/

    atmVector3Set( position, 0.0f, 0.0f, distance );

    atmVector3Assign( &camera->lcsToWCS.data[12], position );
        
    atrXformSetYRotation( toObject, angle );

    atrSelectCamera( camera );

   /*--------------------------------------------------------
     Set state: enable W buffering, set objects transform,
     set objects material
     --------------------------------------------------------*/

    atrPushEnv( env );

    atrPushXform( toObject );

    atdPushMaterial( cubeMaterial );

   /*--------------------------------------------------------
     Draw object, pop state
     --------------------------------------------------------*/

    atrRenderTriSet( cubeModel );

    atdPopMaterial( FXFALSE );

    atrPopXform( FXTRUE );

    atrPopEnv( FXTRUE );

   /*--------------------------------------------------------
     Draw string
     --------------------------------------------------------*/

	atrDrawString( "Hello World", 30.0f, 30.0f, 0.5f, displayHeight-30.0f);

    angle += ATM_DEGREE;
    if ( angle > 2.0f * ATM_PI )
        angle = 0.0f;
}

static void
resetFunc(void) {
	distance = radius * -5.0f;

    angle = 0.0f;
}

static void
AppInit(void) {

   /*--------------------------------------------------------
     Set window title
     --------------------------------------------------------*/

    atdSetName("Simple");

   /*--------------------------------------------------------
     Initialize input, set event callbacks
     --------------------------------------------------------*/

    atdEventInit();
    atdKeyboardFunc(kbFunc);
    atdResetFunc(resetFunc);

   /*--------------------------------------------------------
     Initialize scene manager
     --------------------------------------------------------*/

    if ( !atsInit( ) ) {
      atuError( FXTRUE, "Couldn't initialize scene library.\n" );
    }

   /*--------------------------------------------------------
     Allocate environment prepare for w buffering
     --------------------------------------------------------*/

    env = atrEnvAllocate( 1 );
    env->flags = ATR_HSR_WBUFFER;
    atrPushEnv( env );

   /*--------------------------------------------------------
     Create material for cube
     --------------------------------------------------------*/

    cubeMaterial = atrMaterialAllocate( 1 );
    cubeMaterial->diffuse.r = 1.0f;
    cubeMaterial->diffuse.g = 0.0f;
    cubeMaterial->diffuse.b = 0.0f;

   /*--------------------------------------------------------
     Create light
     --------------------------------------------------------*/

    light = atrLightAllocate( 1 );
    light->flags = ATR_LIGHT_DIRECTED;
    light->color.r = 1.0f;
    light->color.g = 1.0f;
    light->color.b = 1.0f;
    atrAddLight( light, 0 );

   /*--------------------------------------------------------
     Create object's transform
     --------------------------------------------------------*/

    toObject = atrXformAllocate( 1 );

   /*--------------------------------------------------------
     Create camera
     --------------------------------------------------------*/

    camera = atrCameraAllocate( 1 );
	camera->fovRadians = 60.0f*ATM_DEGREE;

   /*--------------------------------------------------------
     Create object
     --------------------------------------------------------*/

	cubeModel = atrPrimCube(radius*2.0f);
  
   /*--------------------------------------------------------
     Set application initial state
     --------------------------------------------------------*/

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
