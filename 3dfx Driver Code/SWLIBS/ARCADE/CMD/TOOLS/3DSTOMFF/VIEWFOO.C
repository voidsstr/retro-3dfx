/*
** Copyright (c) 1995, 3Dfx Interactive, Inc.
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
** $Date: 10/11/00 7:29:40 PM$ 
**
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <dos.h>
#include <conio.h>

#include <3dfx.h>
#include <fxos.h>
#ifdef GLIDE3
#include <glide3.h>
#else
#include <glide.h>
#endif
#include <atutil.h>
#include <atmath.h>
#include <atrender.h>

#include "scene.c"

int main( int argc, char *argv[] ) {
    Scene *scene;

    AtrCanvas      *canvas;
    AtrCamera      *camera;
    AtrLight       *light1;
    AtrEnvironment *environment;
    AtrTransform   *localTransform;
    FxBool          done = FXFALSE;
    FxBool          freeze;
    FILE           *infile;
    FxU32           index;
    FxBool          animate = FXFALSE;
    int frame, maxFrames;

    float angle = 90.0f;
    float angle2 = 0.0f;

    if ( argc != 2 ) {
        puts( "viewfoo file.foo" );
        exit( 0 );
    }

    infile = fopen( argv[1], "rb" );
    if ( !infile ) {
        perror( argv[1] );
        exit( -1 );
    }

    scene = loadScene( infile );
    fclose( infile );

    if ( scene->numAnimations > 0 ) {
        printf( "Has animation, using keyframe data.\n" );
        animate = FXTRUE;
    } else {
        animate = FXFALSE;
    }

    if ( !atrInit( GR_RESOLUTION_640x480,
                    10000 ) ) {
        fprintf( stderr, "Couldn't initialize rendering library.\n" );
        return( -1 );
    }

    /*
    ** CREATE ALL GRAPHICS OBJECTS
    */
    canvas      = atrCanvasAllocate( 1 );
    camera      = atrCameraAllocate( 1 );
    light1      = atrLightAllocate( 1 );
    environment = atrEnvironmentAllocate( 1 );

    /*
    ** INITIALIZE ALL GRAPHICS OBJECTS
    */

    /* configure the canvas */
    canvas->xMin = 0;
    canvas->yMin = 0;
    canvas->xMax = 639;
    canvas->yMax = 479;

    /* configure the lights */
    
    light1->r    = 1.0f;
    light1->g    = 1.0f;
    light1->b    = 1.0f;
    atrTransformSetYRotation( light1->lcsToWCS, -ATU_PI / 4.0f );
    
    /* configure the environment */
    environment->flags = ATR_ENVIRONMENT_HSR_WDEPTH_BUFFER;

    /* configure the camera */
    camera->fieldOfViewAngle = 60.0f;
    camera->aspectRatio      = 1.333333f;
    camera->nearClip         = 1.0f;
    camera->farClip          = 65000.0f;
    atrTransformSetTranslation( camera->lcsToWCS, -1050.0f, 300.0f, -3700.0f );

    localTransform = atrTransformAllocate( 1 );

    /*
    ** SET UP RENDERER STATE
    */
    atrLightAdd( light1 );
    atrEnvironmentPush( environment );
    atrCanvasSelect( canvas );

    /*
    ** FLIP
    */
    
    fprintf( stderr, "Press <space> to exit.\n" );

    freeze = 1;

    angle =  34.0f;
    angle2 = 0.0f;


    if ( animate ) {
        frame = 0;
        maxFrames = scene->animationArray[0].totalCells;
    }

    while( !done ) {
        atrTransformSetYRotation( localTransform, 2.0f * ATU_PI * angle / 360.0f );
        atrTransformAddXRotation( localTransform, 2.0f * ATU_PI * angle2 / 360.0f );

        atrCanvasClear( 0.0f, 0.0f, 0.7f, GR_WDEPTHVALUE_FARTHEST );
        atrCameraSelect( camera );

        for( index = 0; index < scene->numRenderTargets; index++ ) {
            atrMaterialPush( scene->materialArray + scene->renderTargetArray[index].materialIndex );
            atrTransformPushCat( localTransform );
            if ( animate ) {
                AtrTransform *thisXform;
                AtrAnimation *thisAnim;
                thisAnim = scene->animationArray+scene->renderTargetArray[index].animationIndex;
                thisXform = atrAnimationGetCellByIndex( thisAnim, frame );
                atrTransformPushCat( thisXform ); 
            } else {
                atrTransformPushCat( scene->transformArray + scene->renderTargetArray[index].transformIndex );
            }
            atrRenderMeshWithClip( scene->meshArray + scene->renderTargetArray[index].meshIndex );
            atrTransformPop();
            atrTransformPop();
            atrMaterialPop();
        }

        atrBufferSwap();
    
        if ( animate ) {
            frame++;
            if ( frame > maxFrames ) frame = 0;
        }


        if ( !freeze ) {
            angle +=  2.0f;
//            angle2 += 2.0f;
        }
        if ( angle  > 360.0f ) angle  = 0.0f;
        if ( angle2 > 360.0f ) angle2 = 0.0f;

        if ( kbhit() ) {
            switch( getch() ) {
                case ' ':
                    done = FXTRUE;
                    break;
                case 'u':
                    camera->lcsToWCS->data[13] += 50.0f;
                    break;
                case 'd':
                    camera->lcsToWCS->data[13] -= 50.0f;
                    break;
                case 'l':
                    camera->lcsToWCS->data[12] += 50.0f;
                    break;
                case 'r':
                    camera->lcsToWCS->data[12] -= 50.0f;
                    break;
                case 'f':
                    camera->lcsToWCS->data[14] += 50.0f;
                    break;
                case 'b':
                    camera->lcsToWCS->data[14] -= 50.0f;
                    break;
                case 'z':
                    freeze ^= 1;
                    break;
            }
        }
    }

    /*
    ** SHUT DOWN LIBRARY
    */

    atrShutdown();
    
    return 0;
}
