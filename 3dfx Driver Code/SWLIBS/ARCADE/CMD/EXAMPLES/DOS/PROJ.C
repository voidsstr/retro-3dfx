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
** $Date: 10/11/00 7:28:04 PM$ 
**
*/


#include <atrender.h>
#include <glide.h>
#include <string.h>
#include <conio.h>
#include <math.h>

typedef struct {
    AtrCanvas *canvas;
    AtrCamera *camera;
} Window;

typedef struct {
    AtrMaterial *material;
    AtrTriSet   *geometry;
    AtrXform    *objectToWorld;
} Object;

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

float getFloat( void ) {
    float value     = 0.0f;
    float shift     = 10.0f;
    float factor    = 1.0f;
    char  separator = '.';

    while( 1 ) {
        char inchar = getch();
        if ( inchar >= '0' && inchar <= '9' )  {
            value  *= shift;
            value  += factor * (float)(inchar-'0');
            factor *= 0.1f * shift;
        } else if ( inchar == separator ) {
            shift     = 1.0f;
            factor    = 0.1f;
            separator = (char)0xFF;
        } else
            break;
    }
    return value;
}

static AtrMaterial *pointMat;

void main( int argc, char **argv  ) {
    FxBool      done = FXFALSE;
    Object      *cube0, 
                *cube1, 
                *arrow,
                *plane;
    Window       *cameraPov;
    AtrLight     *projLight;
    AtrImg       *baseImg;
    AtrTexHandle baseTex;
    AtrImg       *projImg;
    AtrTexHandle projTex;
    AtrMaterial  *projMat;
    AtrEnv       *env;
    AtrXform     *rotation;
    AtmVector3  lightPosition  = {  30.0f, 5.0f,  0.0f }, 
                lightTarget    = {  0.0f,  5.0f,   0.0f }, 
                cameraPosition = {  60.0f, 55.0f, -60.0f },
                cameraTarget   = {  0.0f,  5.0f,   0.0f }, 
                up             = {  0.0f,  1.0f,   0.0f };
    AtrColor    camWin   = { 0.0f, 0.0f, 0.0f };

    atrInit( "Glide", NULL );
    atuSetLoadPath( getenv( "AT_LOAD_PATH" ) );

    projImg = atrImgAllocate( 1 );
    baseImg   = atrImgAllocate( 1 );

    atrImgCreateFrom3df( projImg, "spot.3df" );
    atrImgCreateFrom3df( baseImg, "marble.3df" );

    projImg->nLevels = 1;

#if 0
    baseTex = atrTexNewHandle( ATR_TEXELFX_0 );
    projTex = atrTexNewHandle( ATR_TEXELFX_1 );
    
    atrTexAssociate( baseTex, baseImg );
    atrTexAssociate( projTex, projImg );

    projMat = atrMaterialAllocate( 1 );

    atrMaterialSetup( projMat, 
                      ATR_MAT_TEX_DECAL_X_PROJECTED );
    projMat->texture[0] = baseTex;
    projMat->texture[1] = projTex;
#else 
    baseTex = atrTexNewHandle( ATR_TEXELFX_0 );
    projTex = atrTexNewHandle( ATR_TEXELFX_0 );
    
    atrTexAssociate( baseTex, baseImg );
    atrTexAssociate( projTex, projImg );

    projMat = atrMaterialAllocate( 1 );

    atrMaterialSetup( projMat, 
                      ATR_MAT_TEX_PROJECTED );
    projMat->texture[0] = projTex;
#endif
    projMat->projectedTag = 0;

    cube0 = createObject( atrPrimCube( 10.0f ), 
                         projMat,
                         atrXformAllocate( 1 ) );
    atmVector3Set( &cube0->objectToWorld->data[12],
                  0.0f, 5.0f, 15.0f );
    cube1 = createObject( atrPrimCube( 10.0f ), 
                         projMat,
                         atrXformAllocate( 1 ) );
    atmVector3Set( &cube1->objectToWorld->data[12],
                  0.0f, 5.0f, -15.0f );
    plane = createObject( atrPrimPlane( 100.0f ), 
                         projMat,
                         atrXformAllocate( 1 ) );
    
    { 
        AtrMaterial *tmpMaterial;
        tmpMaterial = atrMaterialAllocate( 1 );
        atrMaterialSetup( tmpMaterial, ATR_MAT_GSHADE );
        tmpMaterial->emissive.r = 1.0f;
        tmpMaterial->emissive.g = 1.0f;
        tmpMaterial->emissive.b = 1.0f;
        arrow = createObject( atrPrimArrow( 5.0f ), 
                             tmpMaterial,
                             atrXformAllocate( 1 ) );
        atrXformPointAt( arrow->objectToWorld, lightTarget, lightPosition,
                        up );
    }
    
    {    

        AtrXform tmp;
        atrXformPointAt( &tmp, lightTarget, lightPosition, up );
        atrXformPointAt( &tmp, cameraTarget, cameraPosition, up );
        cameraPov = createWindow( 0, 0, 479, 479, &tmp );
    }

    projLight              = atrLightAllocate( 1 );
    projLight->flags       = ATR_LIGHT_PROJECTOR;
    projLight->project.tag = 0;
    projLight->project.fovInX = 90.0f * ATM_DEGREE;
    projLight->project.fovInY = 90.0f * ATM_DEGREE;
    atrXformPointAt( &projLight->lcsToWCS, lightTarget, lightPosition, up );

    /*
        Animation Loop
    */

    rotation = atrXformAllocate( 1 );
    atrXformSetYRotation( rotation, ATM_DEGREE );

    env = atrEnvAllocate( 1 );
    env->flags = ATR_HSR_WBUFFER;
    atrPushEnv( env );

    done = FXFALSE;
    while( !done ) {
        Object *marker = arrow;

        atrXformVectorMult( lightPosition, lightPosition, rotation );
        atrXformPointAt( &projLight->lcsToWCS, lightTarget, lightPosition, up );
        atrXformPointAt( marker->objectToWorld, lightTarget, lightPosition, up );

        atrAddLight( projLight, 0 );

        currentWindow( cameraPov, &camWin );
        renderObject( plane );
        renderObject( marker );

        atrRemoveLight( 0 );
        atrSwapBuffer( 0 );

        if ( kbhit() ) done = FXTRUE;
    }
    atrShutdown();
    return;
}


