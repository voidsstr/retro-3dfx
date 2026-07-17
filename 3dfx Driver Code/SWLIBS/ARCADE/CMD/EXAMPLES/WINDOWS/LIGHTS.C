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
** $Date: 10/11/00 7:28:08 PM$ 
**
*/


#include <atrender.h>
#include <glide.h>
#include <string.h>
#include <conio.h>
#include <math.h>
#include <atinput.h>
#include <atdemo.h>

#define SAMPLE_FREQ 300

typedef struct {
    AtrCanvas *canvas;
    AtrCamera *camera;
} Window;

typedef struct {
    AtrMaterial *material;
    AtrTriSet   *geometry;
    AtrXform    *objectToWorld;
} Object;

AtiKeyboardCB saveKeyCB;
float       promptShift;
float       promptFactor;
FxU16       promptSeparator;
float      *promptFPtr;
float       promptFVal;
char        prompt[80], promptBuff[80];
FxBool      done = FXFALSE, fog = FXFALSE, 
            needFloat = FXFALSE,
            movie = FXFALSE, bilerp = FXFALSE,
            projector = FXFALSE, movieProj = FXFALSE;
Object      *cube0, 
            *cube1, 
            *sphere0, 
            *sphere1,
            *sphere2,
            *sphere3,
            *sphere4,
            *sphere5,
            *plane, 
            *arrow,
            *ball;
Window      *cameraPov, *lightPov, *textWindow, *imgWindow;
AtrMaterial *pointMat;
AtrLight    *light, *projLight;
AtrTexHandle texHandle;
AtrMaterial *texMat;
AtrMaterial *projMat;
AtrTexHandle projTex;
AtrImg       *projImg;
FxU32       sampleInterval = SAMPLE_FREQ;
float       fps = 0.0f;
AtrEnv      *env;
AtrXform    *rotation;
AtmVector3  lightPosition  = {  30.0f, 5.0f,  0.0f }, 
            lightTarget    = {  0.0f,  5.0f,   0.0f }, 
            cameraPosition = {  60.0f, 55.0f, -60.0f },
            cameraTarget   = {  0.0f,  5.0f,   0.0f }, 
            up             = {  0.0f,  1.0f,   0.0f };
AtrColor    camWin   = { 0.0f, 0.0f, 0.0f },
            lightWin = { 0.0f, 0.0f, 0.0f },
            textWin  = { 0.0f, 0.0f, 0.0f },
            imgWin   = { 0.0f, 0.0f, 0.0f };
AtrColor    c0Color = { 0.3f, 0.0f, 0.3f };
AtrColor    c1Color = { 0.3f, 0.3f, 0.0f };
AtrColor    s0Color = { 0.3f, 0.3f, 0.3f };
AtrColor    s1Color = { 0.0f, 0.5f, 0.0f };
AtrColor    s2Color = { 0.5f, 0.0f, 0.0f };
AtrColor    s3Color = { 0.0f, 0.0f, 0.5f };
AtrColor    s4Color = { 0.2f, 0.2f, 0.7f };
AtrColor    s5Color = { 0.5f, 0.1f, 0.8f };

AtrImg      *image;

FxU32  type = ATR_LIGHT_AMBIENT;
FxBool attenuation = 0;
float  quadFactor  = 0.0f;
float  linFactor   = 0.05f;
float  constFactor = 0.0f;
AtrColor lightColor   = { 1.0f, 1.0f, 1.0f };
int      specExponent = 30;
AtrColor specColor    = { 1.0f, 1.0f, 1.0f };
AtrColor black        = { 0.0f, 0.0f, 0.0f };

char     imgFilename[64];
char     spotFilename[64];

char   *typeString[] = { "AMBIENT",
                         "DIRECTED",
                         "POSITIONAL",
                         "ATTENUATED" };
char   *surfaceString[] = { "DIFFUSE",
                            "SPEC",
                            "DIFFSPEC" };
FxU32  surface = 0;
Object *typeArray[4];

Window *createWindow( FxU32 xmin, FxU32 ymin, 
                      FxU32 xmax, FxU32 ymax,
                      AtrXform *viewpoint ) {
    Window *w;
    w = atuMemCalloc( sizeof( Window ), 1 );
    w->canvas = atrCanvasAllocate( 1 );
    w->camera = atrCameraAllocate( 1 );

    w->canvas->xMin = xmin;
    w->canvas->yMin = ymin;
    w->canvas->xMax = xmax;
    w->canvas->yMax = ymax;

    w->camera->lcsToWCS = *viewpoint;
    w->camera->aspectRatio = 1.0f;

    return w;
}

void currentWindow( Window *w, AtrColor *c ) {
    atrSelectCamera( w->camera );
    atrSelectCanvas( w->canvas );
    atrClearCanvas( c->r, c->g, c->b, ATR_WBUFFER_CLEAR );
}

Object* createObject( AtrTriSet *set, AtrMaterial *mat, AtrXform *loc ) {
    Object *retval;
    retval = atuMemCalloc( sizeof( Object ), 1 );
    retval->geometry      = set;
    retval->material      = mat;
    retval->objectToWorld = loc;
    return retval;
}

void renderObject( Object *o ) {
    atrPushXform( o->objectToWorld );
    atrPushMaterial( o->material ); 
    atrRenderTriSet( o->geometry );
    atrPopMaterial( 1 );
    atrPopXform( 1 );
    return;
}


void gputs( char *s) {
    atrDrawString( s, 8.0f, 10.0f, 30.0f, 0.5f );
}

void floatKbFunc(AtiKeyEvent *ev, FxBool preRecorded) {
    if ( ev->state == ATI_KEY_RELEASE )
        return;

    if ( ev->code >= ATI_KEY_1 && ev->code <= ATI_KEY_0)  {
        promptFVal  *= promptShift;
        if ( ev->code != ATI_KEY_0 )
            promptFVal += promptFactor * (float)(ev->code-ATI_KEY_1+1);
            promptFactor *= 0.1f * promptShift;
    } else if ( ev->code == promptSeparator ) {
        promptShift     = 1.0f;
        promptFactor    = 0.1f;
        promptSeparator = 0xFF;
    } else {
        needFloat = FXFALSE;
        *promptFPtr = promptFVal;
        atiKeyboardFunc( saveKeyCB );
    }
    sprintf(promptBuff, "%s %f", prompt, promptFVal);
}

void getFloat( char *s, float *v ) {
    promptShift     = 10.0f;
    promptFactor    = 1.0f;
    promptSeparator = ATI_KEY_PERIOD;
    needFloat = FXTRUE;
    strcpy(prompt, s);
    strcpy( promptBuff, s);
    promptFPtr = v;
    promptFVal = 0.0f;
    saveKeyCB =  atiKeyboardFunc(floatKbFunc);
}

void kbFunc(AtiKeyEvent *ev, FxBool preRecorded) {
    if ( ev->state == ATI_KEY_RELEASE )
        return;

	if(( _atGlobals.keyMap[ATI_KEY_LCTRL] || _atGlobals.keyMap[ATI_KEY_RCTRL]) )
		return;

    switch(  ev->code ) {
      case ATI_KEY_SPACE:
        done = FXTRUE;
        break;
      case ATI_KEY_M:
        type++;
        type = type % 4;
        break;
      case ATI_KEY_Q:
        getFloat( "Enter new quadratic attenuation factor:" , &quadFactor );
        break;
      case ATI_KEY_L:
        getFloat( "Enter new linear attenuation factor:" , &linFactor );
        break;
      case ATI_KEY_C:
        getFloat( "Enter new constant attenuation factor:" , &constFactor );
        break;
      case ATI_KEY_R:
        getFloat( "Enter new red light value:" , &lightColor.r );
        break;
      case ATI_KEY_G:
        getFloat( "Enter new green light value:" , &lightColor.g );
        break;
      case ATI_KEY_B:
        getFloat( "Enter new blue light value:" , &lightColor.b );
      case ATI_KEY_E:
        // specExponent = ( FxU32 ) getFloat( "Enter new specular exponent:" );
        break;
      case ATI_KEY_S:
        surface++;
        surface = surface % 3;
        break;
      case ATI_KEY_1:
        getFloat( "Enter new red spec value:" , &specColor.r );
        break;
      case ATI_KEY_2:
        getFloat( "Enter new green spec value:" , &specColor.g );
        break;
      case ATI_KEY_3:
        getFloat( "Enter new blue spec value:" , &specColor.b );
        break;
      case ATI_KEY_F:
        fog ^= 1;
        break;
      case ATI_KEY_8:
        movie ^= 1;
        break;
      case ATI_KEY_PERIOD:
        bilerp ^= 1;
        break;
      case ATI_KEY_P:
        projector ^= 1;
        break;
      case ATI_KEY_LBRACKET:
        movieProj ^= 1;
        break;
      case ATI_KEY_MINUS:
        break;
    }
}

void
AppRenderScene( FxU32 displayWidth, FxU32 displayHeight) {
    static char charCache[64];

    Object *marker = typeArray[type];

    atrXformVectorMult( lightPosition, lightPosition, rotation );
    atrXformPointAt( &light->lcsToWCS, lightTarget, lightPosition, up );
    atrXformPointAt( &projLight->lcsToWCS, lightTarget, lightPosition, up );
    atrXformPointAt( &lightPov->camera->lcsToWCS, 
                     lightTarget, lightPosition, up );
    atrXformPointAt( marker->objectToWorld, lightTarget, 
                     lightPosition, up );

    light->flags =  type;
    light->color = lightColor;
    light->attenuation[ATR_LIGHT_QUADRATIC] = quadFactor;
    light->attenuation[ATR_LIGHT_LINEAR]    = linFactor;
    light->attenuation[ATR_LIGHT_CONSTANT]  = constFactor;

    sphere0->material->specExponent = 
    sphere1->material->specExponent = 
    sphere2->material->specExponent = 
    sphere3->material->specExponent = 
    sphere4->material->specExponent = 
    sphere5->material->specExponent = 
    cube0->material->specExponent =
    cube1->material->specExponent =
        specExponent;

    if ( movieProj ) 
      projMat->texture[0] = texHandle;
    else
      projMat->texture[0] = projTex;

    if ( fog ) {
        env->flags = ATR_HSR_WBUFFER | ATR_FOG_ON_DEPTH;
        env->fogColor.r = 0.0f;
        env->fogColor.g = 0.0f;
        env->fogColor.b = 0.0f;
        atrEnvFogTable( env, ATR_FOGFUNC_EXP2, 0.020f, 0.0f, 0.0f );
        atrPushEnv( env );
    } else {
        env->flags = ATR_HSR_WBUFFER;
        atrPushEnv( env );
    }

    if ( !projector ) {
        sphere0->material->successor = 0;
        sphere1->material->successor = 0;
        sphere2->material->successor = 0;
        sphere3->material->successor = 0;
        sphere4->material->successor = 0;
        sphere5->material->successor = 0;
        cube0->material->successor   = 0;
        cube1->material->successor   = 0;
        plane->material->successor   = 0;
    } else {
        sphere0->material->successor = projMat;
        sphere1->material->successor = projMat;
        sphere2->material->successor = projMat;
        sphere3->material->successor = projMat;
        sphere4->material->successor = projMat;
        sphere5->material->successor = projMat;
        cube0->material->successor   = projMat;
        cube1->material->successor   = projMat;
        plane->material->successor   = projMat;
    }

    switch ( surface ) {
        case 0:
            sphere0->material->diffuse = s0Color;
            sphere1->material->diffuse = s1Color;
            sphere2->material->diffuse = s2Color;
            sphere3->material->diffuse = s3Color;
            sphere4->material->diffuse = s4Color;
            sphere5->material->diffuse = s5Color;
            cube0->material->diffuse = c0Color;
            cube1->material->diffuse = c1Color;
            sphere0->material->specular = 
            sphere1->material->specular = 
            sphere2->material->specular = 
            sphere3->material->specular = 
            sphere4->material->specular = 
            sphere5->material->specular = 
            cube0->material->specular =
            cube1->material->specular =
                black;
            break;
        case 1:
            sphere0->material->diffuse = 
            sphere1->material->diffuse = 
            sphere2->material->diffuse = 
            sphere3->material->diffuse = 
            sphere4->material->diffuse = 
            sphere5->material->diffuse = 
            cube0->material->diffuse =
            cube1->material->diffuse =
                black;
            sphere0->material->specular = 
            sphere1->material->specular = 
            sphere2->material->specular = 
            sphere3->material->specular = 
            sphere4->material->specular = 
            sphere5->material->specular = 
            cube0->material->specular =
            cube1->material->specular =
                specColor;
            break;
        case 2:
            sphere0->material->diffuse = s0Color;
            sphere1->material->diffuse = s1Color;
            sphere2->material->diffuse = s2Color;
            sphere3->material->diffuse = s3Color;
            sphere4->material->diffuse = s4Color;
            sphere5->material->diffuse = s5Color;
            cube0->material->diffuse = c0Color;
            cube1->material->diffuse = c1Color;
            sphere0->material->specular = 
            sphere1->material->specular = 
            sphere2->material->specular = 
            sphere3->material->specular = 
            sphere4->material->specular = 
            sphere5->material->specular = 
            cube0->material->specular =
            cube1->material->specular =
                specColor;
            break;
    }

    atrAddLight( light, 0 );
    if ( projector ) atrAddLight( projLight, 0 );
    /* Update Light Position */

    /* Render Windows */    
    if ( !bilerp ) {
        currentWindow( cameraPov, &camWin );
        renderObject( plane );
        renderObject( marker );
        renderObject( cube0 );
        renderObject( cube1 );
        renderObject( sphere0 );
        renderObject( sphere1 );
        renderObject( sphere2 );
        renderObject( sphere3 );
        renderObject( sphere4 );
        renderObject( sphere5 );
    }
    

    currentWindow( lightPov, &lightWin );
    renderObject( marker );
    renderObject( plane );
    renderObject( cube0 );
    renderObject( cube1 );
    renderObject( sphere0 );
    renderObject( sphere1 );
    renderObject( sphere2 );
    renderObject( sphere3 );
    renderObject( sphere4 );
    renderObject( sphere5 );

    if (bilerp) {
        AtrDstVertex a,b,c,d;
        a.x =   0.0f, a.y = 479.0f, a.oow = 1.0f;
        b.x = 479.0f, b.y = 479.0f, b.oow = 1.0f;
        c.x =   0.0f, c.y =   0.0f, c.oow = 1.0f;
        d.x = 479.0f, d.y =   0.0f, d.oow = 1.0f;
        a.s0 =   0.0f, a.t0 = 255.0f, a.oow0 = 1.0f;
        b.s0 = 255.0f, b.t0 = 255.0f, b.oow0 = 1.0f;
        c.s0 =   0.0f, c.t0 =   0.0f, c.oow0 = 1.0f;
        d.s0 = 255.0f, d.t0 =   0.0f, d.oow0 = 1.0f;

        atrPushMaterial( texMat );
        currentWindow( cameraPov, &camWin );
        atrDrawTriangle( &a, &b, &c );
        atrDrawTriangle( &c, &b, &d );
        atrPopMaterial( 0 );
    }

    if ( movie ) {
        atrSelectCanvas( lightPov->canvas );
        atrGrabImg( image, 
                   0, 
                   0, 
                   ATR_BUFFER_BACKBUFFER );
        atrTexAssociate( texHandle, image );
    }
        
    currentWindow( imgWindow, &imgWin );
    atrRenderImg( image, 0, 0 );

    /* Draw Text */
    currentWindow( textWindow, &textWin );
    sprintf( charCache, "TYPE:  %s", typeString[type] );
    atrDrawString( charCache, 8.0f, 10.0f, 480.0f, 330.0f );
    sprintf( charCache, "QUAD:  %2.3f", quadFactor );
    atrDrawString( charCache, 8.0f, 10.0f, 480.0f, 320.0f );
    sprintf( charCache, "LIN:   %2.3f", linFactor );
    atrDrawString( charCache, 8.0f, 10.0f, 480.0f, 310.0f );
    sprintf( charCache, "CONST: %2.3f", constFactor );
    atrDrawString( charCache, 8.0f, 10.0f, 480.0f, 300.0f );
    sprintf( charCache, "RED:   %2.3f", lightColor.r );
    atrDrawString( charCache, 8.0f, 10.0f, 480.0f, 290.0f );
    sprintf( charCache, "GREEN: %2.3f", lightColor.g );
    atrDrawString( charCache, 8.0f, 10.0f, 480.0f, 280.0f );
    sprintf( charCache, "BLUE:  %2.3f", lightColor.b );
    atrDrawString( charCache, 8.0f, 10.0f, 480.0f, 270.0f );

    sprintf( charCache, "SURF:  %s", surfaceString[surface] );
    atrDrawString( charCache, 8.0f, 10.0f, 480.0f, 250.0f );
    sprintf( charCache, "SPECE: %d", specExponent );
    atrDrawString( charCache, 8.0f, 10.0f, 480.0f, 240.0f );
    sprintf( charCache, "SPECR: %2.3f", specColor.r );
    atrDrawString( charCache, 8.0f, 10.0f, 480.0f, 230.0f );
    sprintf( charCache, "SPECG: %2.3f", specColor.g );
    atrDrawString( charCache, 8.0f, 10.0f, 480.0f, 220.0f );
    sprintf( charCache, "SPECB: %2.3f", specColor.b );
    atrDrawString( charCache, 8.0f, 10.0f, 480.0f, 210.0f );

    sprintf( charCache, "SI:     %3d", sampleInterval );
    atrDrawString( charCache, 8.0f, 10.0f, 480.0f, 180.0f );
    sprintf( charCache, "FPS:    %2.3f", _atGlobals.fps );
    atrDrawString( charCache, 8.0f, 10.0f, 480.0f, 170.0f );

    atrRemoveLight( 0 );
    atrPopEnv( 0 );

    if ( needFloat )
        gputs( promptBuff );
}

void ResetState(void) {
}

int AppMain( int argc, char **argv ) {
    atdSetName("Lights");
    atdEventInit();
    atdKeyboardFunc(kbFunc);
    atdResetFunc(ResetState);

    atuSetLoadPath( getenv( "AT_LOAD_PATH" ) );

    image = atrImgAllocate( 1 );

    if ( !atuIniGet( "test.ini", "IMAGE FILE", ATU_INI_STRING, imgFilename, 64 ) ) {
        atuIniSet( "test.ini", "IMAGE FILE", ATU_INI_STRING, "marble.3df" );
        strcpy( imgFilename, "marble.3df" );
    }

    if ( !atrImgCreateFromFile( image, imgFilename ) )
      atuError( FXTRUE, "Couldn't load %s.\n", imgFilename );

    image->nLevels = 1;

    texHandle = atrTexNewHandle( ATR_TEXELFX_0 );
    atrTexAssociate( texHandle, image );

    texMat = atrMaterialAllocate( 1 );
    atrMaterialSetup( texMat, ATR_MAT_DECAL );
    texMat->texture[0] = texHandle;

    projImg = atrImgAllocate( 1 );

    if ( !atuIniGet( "test.ini", "SPOT FILE", ATU_INI_STRING, spotFilename, 64 ) ) {
        atuIniSet( "test.ini", "SPOT FILE", ATU_INI_STRING, "spot.3df" );
        strcpy( spotFilename, "spot.3df" );
    }

    if ( !atrImgCreateFromFile( projImg, spotFilename ) )
      atuError( FXTRUE, "Couldn't load %s\n", spotFilename );

    projImg->nLevels = 1;

    projTex = atrTexNewHandle( 0 );
    atrTexAssociate( projTex, projImg );

    projMat = atrMaterialAllocate( 1 );
    atrMaterialSetup(projMat, 
                     ATR_MAT_TEX_PROJECTED | ATR_MAT_FB_MULTIPLY );
    projMat->texture[0] = projTex;
    projMat->projectedTag = 0;

    {
        AtrMaterial *tmpMaterial;


        tmpMaterial = atrMaterialAllocate( 1 );
        atrMaterialSetup( tmpMaterial, ATR_MAT_GSHADE );
        cube0 = createObject( atrPrimCube( 10.0f ), 
                             tmpMaterial,
                             atrXformAllocate( 1 ) );
        atmVector3Set( &cube0->objectToWorld->data[12],
                       0.0f, 5.0f, 15.0f );


        tmpMaterial = texMat;
        cube1 = createObject( atrPrimCube( 10.0f ), 
                              tmpMaterial,
                              atrXformAllocate( 1 ) );
        atmVector3Set( &cube1->objectToWorld->data[12],
                       0.0f, 5.0f, -15.0f );


        tmpMaterial = atrMaterialAllocate( 1 );
        atrMaterialSetup( tmpMaterial, ATR_MAT_GSHADE );
        sphere0 = createObject( atrPrimSphere( 5.0f, 10, 10 ), 
                               tmpMaterial,
                               atrXformAllocate( 1 ) );
        atmVector3Set( &sphere0->objectToWorld->data[12],
                        -15.0f, 5.0f, 0.0f );


        tmpMaterial = atrMaterialAllocate( 1 );
        atrMaterialSetup( tmpMaterial, ATR_MAT_GSHADE );
        sphere1 = createObject( atrPrimSphere( 5.0f, 10, 10 ), 
                               tmpMaterial,
                               atrXformAllocate( 1 ) );
        atmVector3Set( &sphere1->objectToWorld->data[12],
                        15.0f, 5.0f, 0.0f );

        tmpMaterial = atrMaterialAllocate( 1 );
        atrMaterialSetup( tmpMaterial, ATR_MAT_GSHADE );
        sphere2 = createObject( atrPrimSphere( 5.0f, 10, 10 ), 
                               tmpMaterial,
                               atrXformAllocate( 1 ) );
        atmVector3Set( &sphere2->objectToWorld->data[12],
                        30.0f, 5.0f, 30.0f );

        tmpMaterial = atrMaterialAllocate( 1 );
        atrMaterialSetup( tmpMaterial, ATR_MAT_GSHADE );
        sphere3 = createObject( atrPrimSphere( 5.0f, 10, 10 ), 
                               tmpMaterial,
                               atrXformAllocate( 1 ) );
        atmVector3Set( &sphere3->objectToWorld->data[12],
                        30.0f, 5.0f, -30.0f );

        tmpMaterial = atrMaterialAllocate( 1 );
        atrMaterialSetup( tmpMaterial, ATR_MAT_GSHADE );
        sphere4 = createObject( atrPrimSphere( 5.0f, 10, 10 ), 
                               tmpMaterial,
                               atrXformAllocate( 1 ) );
        atmVector3Set( &sphere4->objectToWorld->data[12],
                        -30.0f, 5.0f, 30.0f );

        tmpMaterial = atrMaterialAllocate( 1 );
        atrMaterialSetup( tmpMaterial, ATR_MAT_GSHADE );
        sphere5 = createObject( atrPrimSphere( 5.0f, 10, 10 ), 
                               tmpMaterial,
                               atrXformAllocate( 1 ) );
        atmVector3Set( &sphere5->objectToWorld->data[12],
                        -30.0f, 5.0f, -30.0f );


        tmpMaterial = atrMaterialAllocate( 1 );
        atrMaterialSetup( tmpMaterial, ATR_MAT_GSHADE );
        tmpMaterial->diffuse.r = 0.0f;
        tmpMaterial->diffuse.g = 0.0f;
        tmpMaterial->diffuse.b = 0.0f;
        tmpMaterial->emissive.r = 0.2f;
        tmpMaterial->emissive.g = 0.2f;
        tmpMaterial->emissive.b = 0.2f;
        plane = createObject( atrPrimPlane( 100.0f ), 
                              tmpMaterial,
                              atrXformAllocate( 1 ) );

        tmpMaterial = atrMaterialAllocate( 1 );
        atrMaterialSetup( tmpMaterial, ATR_MAT_GSHADE );
        tmpMaterial->diffuse.r = 0.0f;
        tmpMaterial->diffuse.g = 0.5f;
        tmpMaterial->diffuse.b = 0.0f;
        tmpMaterial->emissive.r = 1.0f;
        tmpMaterial->emissive.g = 1.0f;
        tmpMaterial->emissive.b = 1.0f;
        arrow = createObject( atrPrimArrow( 5.0f ), 
                              tmpMaterial,
                              atrXformAllocate( 1 ) );
        atrXformPointAt( arrow->objectToWorld, lightTarget, lightPosition,
                         up );


        tmpMaterial = atrMaterialAllocate( 1 );
        atrMaterialSetup( tmpMaterial, ATR_MAT_GSHADE );
        tmpMaterial->diffuse.r = 0.0f;
        tmpMaterial->diffuse.g = 0.5f;
        tmpMaterial->diffuse.b = 0.0f;
        tmpMaterial->emissive.r = 1.0f;
        tmpMaterial->emissive.g = 1.0f;
        tmpMaterial->emissive.b = 1.0f;
        ball = createObject(  atrPrimSphere( 1.0f, 10, 10 ), 
                              tmpMaterial,
                              atrXformAllocate( 1 ) );
        atrXformPointAt( ball->objectToWorld, lightTarget, lightPosition,
                         up );
    }
    

    {
        AtrXform tmp;
        atrXformPointAt( &tmp, lightTarget, lightPosition, up );
        lightPov = createWindow( 480, 350, 608, 480, &tmp );
        lightPov->camera->fovRadians = 90.0f * ATM_DEGREE;

        atrXformPointAt( &tmp, cameraTarget, cameraPosition, up );
        cameraPov = createWindow( 0, 0, 480, 480, &tmp );

        textWindow = createWindow( 480, 128, 640, 350, &tmp );
        imgWindow = createWindow( 480, 0, 608, 128, &tmp );
    }

    light          = atrLightAllocate( 1 );
    light->flags   = ATR_LIGHT_DIRECTED;
    light->color.r = 1.0f;
    light->color.g = 1.0f;
    light->color.b = 1.0f;
    atrXformPointAt( &light->lcsToWCS, lightTarget, lightPosition, up );

    projLight              = atrLightAllocate( 1 );
    projLight->flags       = ATR_LIGHT_PROJECTOR;
    projLight->project.tag = 0;
    projLight->project.fovInX = 90.0f * ATM_DEGREE;
    projLight->project.fovInY = 90.0f * ATM_DEGREE;
    atrXformPointAt( &projLight->lcsToWCS, lightTarget, lightPosition, up );

    env        = atrEnvAllocate( 1 );

    /*
        Animation Loop
    */

    rotation = atrXformAllocate( 1 );
    atrXformSetYRotation( rotation, ATM_DEGREE );

    typeArray[ATR_LIGHT_AMBIENT]    = ball;
    typeArray[ATR_LIGHT_DIRECTED]   = arrow;
    typeArray[ATR_LIGHT_POSITIONAL] = ball;
    typeArray[ATR_LIGHT_ATTENUATED] = ball;

    atdEventLoop();

    return 0;
}
