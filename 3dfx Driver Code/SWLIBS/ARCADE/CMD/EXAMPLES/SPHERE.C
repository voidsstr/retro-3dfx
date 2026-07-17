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
** $Date: 10/11/00 7:28:02 PM$ 
**
*/

#include <atrender.h>
#include <conio.h>
#include <string.h>

typedef FxU32 tri[3];
static AtrVertex *vList;
static tri       *triList;
static FxU32     numVerts;
static FxU32     numTris;
static AtrTriSet *sphereModel;

void createSphereModel( float radius, int subdivisions ) {
    sphereModel = atrPrimSphere( radius, subdivisions * 4, subdivisions * 4 );
}

void drawSphereModel( void ) {
    atrRenderTriSet( sphereModel );
}

static char *usage = 
    "[-r radius][-s subdivisions]\n"                
    "-r radius - where radius is a positive floating point value\n"
    "            the default radius is `1.0f\n"
    "-s subdivisions - where subdivisions is the integer number of times\n"
    "                  to re-subdivide the starting shape.\n"
    "-i        - render in immediate mode.\n";

void main( int argc, char *argv[] ) {
    AtrCamera   *camera;
    AtrMaterial *material;
    AtrXform    *toObject;
    AtrTexHandle tex;
    AtrImg       *img;
    AtrLight    *light;
    AtrEnv      *env;
    AtrStats    stats;
    float       distance = 10.0f;
    float       angle;
    float       radius = 1.0f;
    FxU32       subdivisions = 1;
    FxBool      immediate = 0;

    while( --argc && (**(++argv))=='-' ) {
        switch( *(*argv+1)){
            case 'r':
                if ( !--argc ) break;
                radius = (float)atof( *(++argv) );
                break;
            case 's':
                if ( --argc )
                    subdivisions = atoi( *(++argv) );
                else 
                    argc++;
                break;
            case 'i':
                immediate = 1;
                break;
            default:
                puts( usage );
                return;
                break;
        }
    }

    atrInit( 640, 480 );

    env = atrEnvAllocate( 1 );
    env->flags = ATR_HSR_WBUFFER;
    atrPushEnv( env );

    atrImgSetPath( getenv( "AT_TEXTURE_PATH" ) );
    img = atrImgAllocate( 1 );
    atrImgCreateFrom3df( img, "marble.3df" );

    tex = atrTexNewHandle( 0 );
    atrTexAssociate( tex, img );

    material = atrMaterialAllocate( 1 );
//    atrMaterialSetup( material, ATR_MAT_DECAL_X_LIGHTING );
    atrMaterialSetup( material, ATR_MAT_GSHADE );
    material->texture[0] = tex;
    material->diffuse.r = 1.0f;
    material->diffuse.g = 1.0f;
    material->diffuse.b = 1.0f;

    atrPushMaterial( material );

    light = atrLightAllocate( 1 );
    light->flags = ATR_LIGHT_DIRECTED;
    light->color.r = 1.0f;
    light->color.g = 1.0f;
    light->color.b = 1.0f;
//    atrXformSetYRotation( &light->lcsToWCS, 30.0f * ATM_DEGREE );
    atrAddLight( light, 0 );

    toObject = atrXformAllocate( 1 );
    atrXformSetIdentity( toObject );

    camera = atrCameraAllocate( 1 );

    angle = 0.0f;
    createSphereModel( radius, subdivisions );

    atrStatsReset();
    while( !kbhit() ) {
        AtmVector3 position;
        
        atmVector3Set( position, 0.0f, 0.0f, distance );

        distance = radius * -5.0f;
        
        atmVector3Assign( &camera->lcsToWCS.data[12], position );
        
        atrXformSetYRotation( toObject, angle );

        atrClearCanvas( 0.0f, 0.0f, 1.0f, ATR_WBUFFER_CLEAR );

        atrSelectCamera( camera );
        atrPushXform( toObject );
        drawSphereModel();
        atrPopXform( 0 );

        atrSwapBuffer( 0 );

        angle += ATM_DEGREE;
        if ( angle > 2.0f * ATM_PI )
            angle = 0.0f;
    }
    atrStatsRetrieve( &stats );
    printf( "pixelsIn: %d\npixelsOut:%d\ntime:%f\nframes: %d\n"
            "tris: %d\nverts: %d\ntexMem: %d\ntexD/Ls: %d\ntcm: %d\n",
            stats.pixelsIn + stats.ffPixelsIn, stats.pixelsOut,
            stats.elapsedTime, stats.frames, stats.totalTris,
            stats.totalVerts, stats.peakTexMemoryInFrame,
            stats.textureDownloads, stats.peakTexCacheMissesInFrame );
    printf( "tris/sec: %f\n", ((float)stats.totalTris) / 
                              stats.elapsedTime );
    printf( "fps: %f\n", ((float)stats.frames)/stats.elapsedTime );

    printf( "\n\nNum Tris In Sphere: %d\n", numTris );

    return;
}
