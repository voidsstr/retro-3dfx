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
** $Date: 10/11/00 7:30:10 PM$ 
**
*/


#ifdef __WIN32__
#include <windows.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <conio.h>
#include <string.h>
#include <malloc.h>
#include <glide.h>
#include <3dfx.h>
#include <fxos.h>
#include <atutil.h>
#include <atrender.h>
#include <atinput.h>
#include <3ds.h>
#include <atscene.h>
#include <atdemo.h>
#include <windows.h>
#include "resource.h"
#include "view.h"

static AtrMaterial *reticleMaterial;

AtmCMesh *cmesh;
AtmCMesh *buildCMesh(AtsNode *n);
void editFile(char *fileName);

FxFloat  gInitRx = 0.0f;
FxFloat  gInitRy = 0.0f;
FxBool helpBanner = FXFALSE;

AtrMaterial defMat;

AtrEnv    *environment;
AtsNode *top_level, *model = NULL;
AtmSphere bsphere;
AtrXform  pivot;

FxFloat dRotation, dTranslation;
FxFloat xRot = 0.0f, yRot = 0.0f, camXRot = 0.0f, camYRot = 0.0f;

AtmVector3 camPosition = { 0.0f, 0.0f, 0.0f };

FxFloat modelRadius;
FxFloat x, y;
FxFloat joyx, joyy;
FxFloat x_from_keyboard = 0.0f;
FxFloat y_from_keyboard = 0.0f;

FxBool gSwapInterval = 1;
AtrCamera *camera;

FxU32 gCullMode = ATS_CULL_NODES;

AtrStats stats;
FxU32    pixelsOut = 0;
FxU32    totalTris = 0;

AtrXform cumulative;

char *fileName;

char *modelName;

FxBool done      = FXFALSE;

enum { FLY_MODE, FLIP_MODE } joyMode = FLIP_MODE; 
FxBool trigger = FXFALSE;
FxBool button  = FXFALSE;

static char perfStats[64]  = "INIT";
static char perfStats1[64]  = "";
char *modeString = "FLIP";
FxU32 modeCount = 200;
FxU32 frame = 0;
AtrCanvas *canvas;

EditFile *openFiles = NULL;

void SetTitle(char *mode) {
    static char buff[80];

    sprintf(buff, "ATB Viewer [%s]", mode);
    atdSetName(buff);
}

void SetModeString(char *s) {
    static char *last = NULL;

    if ( model == NULL )
        s = "No Model";

    if ( last == s )
        return;

    last = s;

    modeString = s;

    SetTitle(modeString);
}

void ResetState(void) {
    xRot = yRot = camXRot = camYRot = 0.0f;
    joyx = gInitRx;
    joyy = gInitRy;
    modeCount += 200;

    switch ( joyMode ) {
    case FLIP_MODE:
        dRotation    = 3.0f * ATM_DEGREE;
        dTranslation = modelRadius / 25.0f;
        camPosition[0] = 0.0f;
        camPosition[1] = 0.0f;
        camPosition[2] = -3.0f * modelRadius;
        break;

    case FLY_MODE:
        dRotation    = 3.0f * ATM_DEGREE;
        dTranslation = 100.0f;
        camPosition[0] = 0.0f;
        camPosition[1] = 200.0f;
        camPosition[2] = 0.0f;
        break;
    }
    atrXformSetIdentity( &camera->lcsToWCS );
    atmVector3Assign( &camera->lcsToWCS.data[12], camPosition );
}

void viewNode( AtsObject *object ) {
    model = object;

    atsComputeBSphere(model, &bsphere);

    atrXformSetTranslation( &pivot, 
             -bsphere.center[0], -bsphere.center[1], -bsphere.center[2]);
    
    atrXformAssign(&cumulative, &pivot);

    modelRadius = bsphere.radius;

    ResetState();
}

void viewTriSet( AtsObject *object ) {
    AtmSphere bsphere;

    model = object;

    atsComputeBSphere(model, &bsphere);

    atrXformSetTranslation( &pivot, 
             -bsphere.center[0], -bsphere.center[1], -bsphere.center[2]);
    
    atrXformAssign(&cumulative, &pivot);

    modelRadius = bsphere.radius;

    ResetState();
}

void drawReticle( AtrMaterial *mat, 
                  float xmin, 
                  float ymin, 
                  float xmax, 
                  float ymax,
                  float r,
                  float g, 
                  float b ) {
    AtrDstVertex v[2];

    float    xdiff = xmax - xmin;
    float    ydiff = ymax - ymin;

    memset( v, 0, sizeof( GrVertex[2] ) );

    v[0].r = v[1].r = r;
    v[0].g = v[1].g = g;
    v[0].b = v[1].b = b;

    v[0].oow = v[1].oow = 1.0f;


    atrPushMaterial( mat );
    v[0].x = xmin;
    v[0].y = ymax;
    v[1].x = xmin + xdiff / 4.0f;
    v[1].y = ymax;
    atrDrawLine( v+0, v+1 );
    v[0].x = xmin;
    v[0].y = ymax;
    v[1].x = xmin;
    v[1].y = ymax - ydiff / 4.0f;
    atrDrawLine( v+0, v+1 );

    v[0].x = xmax;
    v[0].y = ymin;
    v[1].x = xmax - xdiff / 4.0f;
    v[1].y = ymin;
    atrDrawLine( v+0, v+1 );
    v[0].x = xmax;
    v[0].y = ymin;
    v[1].x = xmax;
    v[1].y = ymin + ydiff / 4.0f;
    atrDrawLine( v+0, v+1 );

    atrPopMaterial( 1 );
}

void kbFunc(AtiKeyEvent *ev, FxBool preRecorded) {
    if ( _atGlobals.keyMap[ATI_KEY_LCTRL] || _atGlobals.keyMap[ATI_KEY_RCTRL] ) {
        if ( ev->state == ATI_KEY_PRESS ) {
            switch( ev->code ) {
            case ATI_KEY_ESCAPE:
                if ( _atGlobals.done )
                    checkExitApp();
                break;
            case ATI_KEY_V:
                gSwapInterval = !gSwapInterval;
                break;
            case ATI_KEY_C:
                if (gCullMode == ATS_CULL_NODES) 
                     gCullMode = ATS_CULL_NONE;
                else gCullMode = ATS_CULL_NODES;
                break;
            }
        }
    }

    if( !( _atGlobals.keyMap[ATI_KEY_LCTRL] || _atGlobals.keyMap[ATI_KEY_RCTRL] ||
           _atGlobals.keyMap[ATI_KEY_LSHIFT] || _atGlobals.keyMap[ATI_KEY_RSHIFT ] ) ) {
        if ( ev->state == ATI_KEY_PRESS ) {
            switch( ev->code ) {
            case ATI_KEY_A:
                dRotation += 0.1f;
                break;
            case ATI_KEY_D:
                dRotation -= 0.1f;
                break;
            case ATI_KEY_Z:
                dTranslation += modelRadius / 100.0f;
                break;
            case ATI_KEY_X:
                dTranslation -= modelRadius / 100.0f;
                break;
            case ATI_KEY_M:
                if ( joyMode == FLIP_MODE ) {
                    joyMode = FLY_MODE;
                    modeCount += 200;
                } else {
                    joyMode = FLIP_MODE;
                    modeCount += 200;
                }
                break;
            case ATI_KEY_T:
                trigger = !trigger;
                modeCount += 200;
                break;
            case ATI_KEY_B:
                button = !button;
                modeCount += 200;
                break;
            case ATI_KEY_RIGHT:
                x_from_keyboard =  1.0f;
                break;
            case ATI_KEY_LEFT:
                x_from_keyboard = -1.0f;
                break;
            case ATI_KEY_DOWN:
                y_from_keyboard = -1.0f;
                break;
            case ATI_KEY_UP:
                y_from_keyboard =  1.0f;
                break;
            }
        }
    }

    if ( ev->state == ATI_KEY_RELEASE ) {
        switch( ev->code ) {
        case ATI_KEY_RIGHT:
            x_from_keyboard = 0.0f;
            break;
        case ATI_KEY_LEFT:
            x_from_keyboard = 0.0f;
            break;
        case ATI_KEY_DOWN:
            y_from_keyboard = 0.0f;
            break;
        case ATI_KEY_UP:
            y_from_keyboard = 0.0f;
            break;
        }
    }
}
    

/*--------------------------------------------------------
  Handle Input
  --------------------------------------------------------*/

void joyFunc(AtiJoystickEvent *ev, FxBool preRecorded) {
    static last = 0;
    static last_trigger = 0, last_button = 0;

    joyx = (fabs(ev->x)>0.2f)?ev->x:0.0f;
    joyy = (fabs(ev->y)>0.2f)?ev->y:0.0f;

    /*--------------------------------
      gInitRx/gInitRy are an unsightly
      hack to give testing the ability
      to start up with flipping models.
      --------------------------------*/
    if ( joyy == 0.0f ) {
        joyy = gInitRy;
    } else {
        gInitRx = 0.0f;
        gInitRy = 0.0f;
    }

    if ( joyx == 0.0f ) {
        joyx = gInitRx;
    } else {
        gInitRx = 0.0f;
        gInitRy = 0.0f;
    }

    if ( last_trigger != ev->button[0] ) {
        trigger = ev->button[0];
        last_trigger = trigger;
    }

    if (( ev->button[1] == ATI_BUTTON_DOWN )  && 
        ( ev->button[1] != last )) {
        if ( joyMode == FLIP_MODE ) {
            joyMode = FLY_MODE;
            modeCount += 200;
        } else {
            joyMode = FLIP_MODE;
            modeCount += 200;
        }
    }

    last = ev->button[1];

    if ( last_button != ev->button[2] ) {
        button = ev->button[2];
        last_button = button;
    }
}

void
AppRenderScene( FxU32 displayWidth, FxU32 displayHeight) {

    x = joyx+x_from_keyboard;
    y = joyy+y_from_keyboard;

    atsFrameBegin();
    atsCullMode(gCullMode);

    if ( joyMode == FLIP_MODE ) {
        AtrXform tmp;
        AtmVector3 origin = { 0.0f, 0.0f, 0.0f };

        camXRot = 0.0f;
        camYRot = 0.0f;

        if ( trigger ) {
            camera->lcsToWCS.data[12] += y * dTranslation * 
                              camera->lcsToWCS.data[8];
            camera->lcsToWCS.data[13] += y * dTranslation * 
                              camera->lcsToWCS.data[9];
            camera->lcsToWCS.data[14] += y * dTranslation * 
                              camera->lcsToWCS.data[10];
            SetModeString("FLIP - ZOOM");
        } else {
            xRot -= y * dRotation;
            yRot -= x * dRotation;
            SetModeString("FLIP - ROTATE");
        }

        atrXformPointAt( &camera->lcsToWCS,
                     origin,
                     &camera->lcsToWCS.data[12],
                     &camera->lcsToWCS.data[4] );

        atrXformAssign(&cumulative, &pivot);
        atrXformSetXRotation( &tmp, xRot );
        atrXformCat( &cumulative, &cumulative, &tmp );
        atrXformSetYRotation( &tmp, yRot );
        atrXformCat( &cumulative, &cumulative, &tmp );
    } else { /* joyMode == FLY_MODE */
        AtrXform tmp;

        atmVector3Assign( camPosition, &camera->lcsToWCS.data[12] );
        atrXformSetIdentity( &tmp );
        atrXformSetIdentity( &camera->lcsToWCS );
        atrXformSetXRotation( &tmp, camXRot );
        atrXformCat( &camera->lcsToWCS, &camera->lcsToWCS, &tmp );
        atrXformSetYRotation( &tmp, camYRot );
        atrXformCat( &camera->lcsToWCS, &camera->lcsToWCS, &tmp );
        atmVector3Assign( &camera->lcsToWCS.data[12], camPosition );

        if ( trigger ) {        /* translate camera in z */
            camera->lcsToWCS.data[12] += y * dTranslation * camera->lcsToWCS.data[8];
            camera->lcsToWCS.data[13] += y * dTranslation * camera->lcsToWCS.data[9];
            camera->lcsToWCS.data[14] += y * dTranslation * camera->lcsToWCS.data[10];
            SetModeString("FLY - Z TRANSLATE");
        } else if ( button ) {  /* translate camera in x/y */
            camera->lcsToWCS.data[12] += y * dTranslation * camera->lcsToWCS.data[4] +
                     x * dTranslation * camera->lcsToWCS.data[0];
            camera->lcsToWCS.data[13] += y * dTranslation * camera->lcsToWCS.data[5] +
                     x * dTranslation * camera->lcsToWCS.data[1];
            camera->lcsToWCS.data[14] += y * dTranslation * camera->lcsToWCS.data[6] +
                     x * dTranslation * camera->lcsToWCS.data[2];
            SetModeString("FLY - XY TRANSLATE");
        } else {                /* Just Rotate The Camera */
            camXRot += y * dRotation * 0.1f;
            camYRot += x * dRotation * 0.1f;
            SetModeString("FLY - ROTATE");
        }
    }

    atsSelectCamera( camera );

    /*--------------------------------------------------------
      Clear The WBUFFER and the BACKBUFFER
      --------------------------------------------------------*/

    if ( _atGlobals.fog ) {
         environment->flags = ATR_HSR_WBUFFER|ATR_FOG_ON_DEPTH;
         environment->fogColor.r = 0.5f;
         environment->fogColor.g = 0.5f;
         environment->fogColor.b = 0.5f;
         atrEnvFogTable(environment, ATR_FOGFUNC_EXP2, 0.6f, 1.0f, 
                        5*modelRadius);
         atrClearCanvas( 0.5f, 0.5f, 0.5f, ATR_WBUFFER_CLEAR );
    } else {
        environment->flags = ATR_HSR_WBUFFER;
		atrClearCanvas( 0.2f, 0.2f, 0.2f, ATR_WBUFFER_CLEAR );
	}

    atrPushEnv( environment );

    /*--------------------------------------------------------
      DRAW THE OBJECT
      --------------------------------------------------------*/
    atrPushXform( &cumulative );
      
    atrPushMaterial( &defMat );

    if ( model != NULL )
        atsDraw(model, ATM_NODE_MASK_ALL);

    atrPopMaterial(FXTRUE);

    atrPopXform( 0 );

    atrPopEnv( FXTRUE );

    if ( modeCount ) {                        
        atrDrawString( modeString, 30.0f, 30.0f, 0.5f, canvas->yMax-35.0f); 
        modeCount--;
    }

    drawReticle( reticleMaterial, 299.0f, 219.0f, 340.0f, 260.0f, 
                 255.0f, 0.0f, 0.0f );
}

int 
AppMain( int argc, char **argv ) {
    AtsType *dictType = atsGetTypeFromName("Dict");
    int frameNumber = 0;
    int frameInc = 0;
    char      *loadPath;
    AtrLight  *light;

    SetModeString("Flip");
    atdSetHelpFile("fliphelp.tga");
    atdKeyboardFunc(kbFunc);
    atdJoystickFunc(joyFunc);
    atdResetFunc(ResetState);

    atrMaterialDefault( &defMat );
    atrMaterialSetup( &defMat, ATR_MAT_GSHADE );
    defMat.emissive.r = 1.0f;
    defMat.emissive.g = 1.0f;
    defMat.emissive.b = 1.0f;

    modelName = NULL;

    /*--------------------------------------------------------
      Init Supplemental Libraries ( atr, ati, ats )
      --------------------------------------------------------*/

    /* do this first to ensure system does not hang */

    /* initialize scene manager */

    if ( loadPath = getenv("AT_LOAD_PATH"))
         atuSetLoadPath(loadPath);
    else atuSetLoadPath(".;..\\models");

    /* Allocate Graphics Objects */
    canvas      = atrCanvasAllocate( 1 );
    light       = atrLightAllocate( 1 );
    environment = atrEnvAllocate( 1 );

    light->color.r = 1.0f;
    light->color.g = 1.0f;
    light->color.b = 1.0f;
    // atrXformSetYRotation( &light->lcsToWCS, 90.0f * ATM_DEGREE );
    atrAddLight( light, 0 );

    /* Allocate camera */

    camera = atrCameraAllocate( 1 );
    camera->fovRadians  = 60.0f * ATM_DEGREE;
    camera->aspectRatio = 1.333333f;
    camera->nearClip    = 1.0f;
    camera->farClip     = 65535.0f;

    /* configure the environment */

    environment->flags = ATR_HSR_WBUFFER;
    atrPushEnv( environment );

    reticleMaterial = atrMaterialAllocate( 1 );
    atrMaterialSetup( reticleMaterial, ATR_MAT_GSHADE );

    atiEventHistoryFile("flip.evt");

    /*--------------------------------------------------------
      Init Startup Data
      --------------------------------------------------------*/

    /* atsConverterIntAttr(NULL, ATS_LOAD_TEXTURES, FXFALSE); */
 
    atsConverterIntAttr(NULL, ATS_CATTR_TEXTURE_WRAP_FIX, FXTRUE);

    if ( argc > 1 )
        editFile(argv[1]);

    while (!quit) {
        _atGlobals.done = FXFALSE;
        atdEventLoop();
        if ( _atGlobals.done && !quit ) {
            checkExitApp();
        }
    }

    return 0;
}
