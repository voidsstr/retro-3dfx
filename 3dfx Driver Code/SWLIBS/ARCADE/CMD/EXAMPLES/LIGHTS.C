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
** $Date: 10/11/00 7:27:57 PM$ 
**
*/


#include <atrender.h>
#include <glide.h>
#include <string.h>
#include <conio.h>
#include <math.h>

float _cube[6][4][5] = {
    {   /* Front */
        { -1.0f,  1.0f, -1.0f, 0.0f, 0.0f },
        {  1.0f,  1.0f, -1.0f, 1.0f, 0.0f },
        {  1.0f, -1.0f, -1.0f, 1.0f, 1.0f },
        { -1.0f, -1.0f, -1.0f, 0.0f, 1.0f }
    },{ /* Right */
        {  1.0f,  1.0f, -1.0f, 0.0f, 0.0f },
        {  1.0f,  1.0f,  1.0f, 1.0f, 0.0f },
        {  1.0f, -1.0f,  1.0f, 1.0f, 1.0f },
        {  1.0f, -1.0f, -1.0f, 0.0f, 1.0f }
    },{ /* Back */
        {  1.0f,  1.0f,  1.0f, 0.0f, 0.0f }, 
        { -1.0f,  1.0f,  1.0f, 1.0f, 0.0f },
        { -1.0f, -1.0f,  1.0f, 1.0f, 1.0f }, 
        {  1.0f, -1.0f,  1.0f, 0.0f, 1.0f }
    },{ /* Left */
        { -1.0f,  1.0f,  1.0f, 0.0f, 0.0f },
        { -1.0f,  1.0f, -1.0f, 1.0f, 0.0f },
        { -1.0f, -1.0f, -1.0f, 1.0f, 1.0f },
        { -1.0f, -1.0f,  1.0f, 0.0f, 1.0f }
    },{ /* Bottom */
        { -1.0f, -1.0f, -1.0f, 0.0f, 0.0f },
        {  1.0f, -1.0f, -1.0f, 1.0f, 0.0f },
        {  1.0f, -1.0f,  1.0f, 1.0f, 1.0f },
        { -1.0f, -1.0f,  1.0f, 0.0f, 1.0f }
    },{ /* Top */
        { -1.0f,  1.0f,  1.0f, 0.0f, 0.0f },
        {  1.0f,  1.0f,  1.0f, 1.0f, 0.0f },
        {  1.0f,  1.0f, -1.0f, 1.0f, 1.0f },
        { -1.0f,  1.0f, -1.0f, 0.0f, 1.0f },
    }
};

float _normals[6][3] = {
    {  0.0f,  0.0f, -1.0f },
    {  1.0f,  0.0f,  0.0f },
    {  0.0f,  0.0f,  1.0f },
    { -1.0f,  0.0f,  0.0f },
    {  0.0f, -1.0f,  0.0f },
    {  0.0f,  1.0f,  0.0f }
};

/*-------------------------------------------------------------------
  Function: primSphere
  Date: 4/10/96
  Implementor(s): jdt
  Library: Primitives
  Description:
    Return a triset of a sphere of specified dimensions.  Code
    provided by Dr. Ron Levine.
  Arguments:
    r - radius of sphere
    nParallels - number of parallels
    nMeridians - number of meridians
  Return:
    AtrTriSet describing a sphere
  -------------------------------------------------------------------*/
AtrTriSet *primSphere( float radius, int nParallels, int nMeridians ){
    AtrTriSet *sphere = atrTriSetAllocate( 1 );
        AtrVertex *northPole = atrVertexAllocate(1);
        AtrVertex *southPole = atrVertexAllocate(1);
        AtrVertex *lattice   = atrVertexAllocate(nParallels*nMeridians);
        int latIndex, longIndex, index, index1;
        float theta, dtheta = ATM_PI/(nParallels+1); 
                //the colatitude, or polar angle
        float longitude, dlongitude   = 2.0f*ATM_PI/nMeridians;
        float cosTheta,sinTheta,z;

        northPole->x = northPole->y = southPole->x =southPole->y = 0.0f;
        northPole->z = radius;
        southPole->z = -radius;
        northPole->i = northPole->j = southPole->i = southPole->j = 0.0f;
        northPole->k =  1.f ;
        southPole->k = -1.f ;
        northPole->s0 = 0.5f;
        northPole->t0 = 1.0f;
        southPole->s0 = 0.5f;
        southPole->t0 = 1.0f*(nParallels%2);
        for(latIndex = 1; latIndex <= nParallels; latIndex++){
                theta = latIndex*dtheta;
                cosTheta = (float)cos(theta);
                sinTheta = (float)sin(theta);
                z = radius*cosTheta ;
                for(longIndex = 0; longIndex<nMeridians; longIndex++) {
                        longitude = longIndex*dlongitude ;
                        index = (latIndex-1)*nMeridians + longIndex ;
                        lattice[index].k = cosTheta ;
                        lattice[index].z = z ;
                        lattice[index].i = sinTheta*(float)cos(longitude);
                        lattice[index].j = sinTheta*(float)sin(longitude);
                        lattice[index].x = radius*lattice[index].i;
                        lattice[index].y = radius*lattice[index].j;
                        lattice[index].s0 = 1.0f*(longIndex%2);
                        lattice[index].t0 = 1.0f*(latIndex%2);
                }
        }

        atrTriSetBegin(sphere);
                //North Polar Cap
                for(longIndex=0; longIndex<nMeridians; longIndex++){
                        atrTriSetVertex(sphere,northPole);
                        atrTriSetVertex(sphere,&lattice[longIndex])     ;
                        atrTriSetVertex(sphere,&lattice[longIndex<nMeridians-1 ? longIndex+1 : 0]);
                }
                //Subpolar temperate zones
                for(latIndex=1; latIndex<nParallels ; latIndex++){
                        for(longIndex=0;longIndex<nMeridians;longIndex++){
                                index = (latIndex-1)*nMeridians + longIndex ;
                                index1 = longIndex<nMeridians-1 ? index+1: (latIndex-1)*nMeridians ;
                                atrTriSetVertex(sphere, &lattice[index]);
                                atrTriSetVertex(sphere, &lattice[index+nMeridians]);
                                atrTriSetVertex(sphere, &lattice[index1]);

                                atrTriSetVertex(sphere, &lattice[index1]);
                                atrTriSetVertex(sphere, &lattice[index+nMeridians]); 
                                atrTriSetVertex(sphere, &lattice[index1+nMeridians]);
                        }
                }
                //South Polar Cap
                for(longIndex=0; longIndex<nMeridians; longIndex++){
                        index = (nParallels-1)*nMeridians + longIndex ;
                        index1= longIndex < nMeridians -1 ? index+1 : (nParallels-1)*nMeridians ;
                        atrTriSetVertex(sphere,&lattice[index]);
                        atrTriSetVertex(sphere,southPole) ;
                        atrTriSetVertex(sphere,&lattice[index1]);
                }
        atrTriSetEnd(sphere);
        atrVertexDeallocate(northPole);
        atrVertexDeallocate(southPole);
        atrVertexDeallocate(lattice);
        return sphere;
}



/*-------------------------------------------------------------------
  Function: primCube
  Date: 4/10/96
  Implementor(s): jdt
  Library: Primitives
  Description:
    Return a triset of a cube of specified dimensions
  Arguments:
    s - length of a side
  Return:
    AtrTriSet describing a cube
  -------------------------------------------------------------------*/
AtrTriSet *primCube( float s ) {
    FxU32     side;
    AtrTriSet *set = atrTriSetAllocate( 1 );

    atrTriSetBegin( set );
    for( side = 0; side < 6; side++ ) {
        AtrVertex verts[4];
        FxU32 vertex;
        for( vertex = 0; vertex < 4; vertex++ ) {
            verts[vertex].x  = _cube[side][vertex][0];
            verts[vertex].y  = _cube[side][vertex][1];
            verts[vertex].z  = _cube[side][vertex][2];
            verts[vertex].i  = _normals[side][0];
            verts[vertex].j  = _normals[side][1];
            verts[vertex].k  = _normals[side][2];
            verts[vertex].s0 = _cube[side][vertex][3];
            verts[vertex].t0 = _cube[side][vertex][4];
            atmVector3Scale( (float*)&verts[vertex],
                             (float*)&verts[vertex],
                             0.5f * s );
        }
        atrTriSetVertex( set, &verts[0] );
        atrTriSetVertex( set, &verts[1] );
        atrTriSetVertex( set, &verts[2] );

        atrTriSetVertex( set, &verts[0] );
        atrTriSetVertex( set, &verts[2] );
        atrTriSetVertex( set, &verts[3] );
    }
    atrTriSetEnd( set );
    return set;
}

/*-------------------------------------------------------------------
  Function: primPlane
  Date: 4/10/96
  Implementor(s): jdt
  Library: Primitives
  Description:
    Return a triset of a plane of specified dimensions
  Arguments:
    s - length of a side
  Return:
    AtrTriSet describing a cube
  -------------------------------------------------------------------*/
AtrTriSet *primPlane( float s ) {
    AtrTriSet *set = atrTriSetAllocate( 1 );
    AtrVertex verts[4];
    FxU32 vertex;

    atrTriSetBegin( set );
    for( vertex = 0; vertex < 4; vertex++ ) {
        verts[vertex].x  = _cube[5][vertex][0];
        verts[vertex].y  = 0.0f;
        verts[vertex].z  = _cube[5][vertex][2];
        verts[vertex].i  = _normals[5][0];
        verts[vertex].j  = _normals[5][1];
        verts[vertex].k  = _normals[5][2];
        verts[vertex].s0 = _cube[5][vertex][3];
        verts[vertex].t0 = _cube[5][vertex][4];
        atmVector3Scale( (float*)&verts[vertex],
                         (float*)&verts[vertex],
                         0.5f * s );
    }

    atrTriSetVertex( set, &verts[0] );
    atrTriSetVertex( set, &verts[1] );
    atrTriSetVertex( set, &verts[2] );

    atrTriSetVertex( set, &verts[0] );
    atrTriSetVertex( set, &verts[2] );
    atrTriSetVertex( set, &verts[3] );

    atrTriSetEnd( set );
    return set;
}

AtrTriSet *primArrow( float length ) {
    AtrTriSet *arrow = atrTriSetAllocate( 1 );
    AtrVertex arrowPoints[7];
    /*----------------------------------------------------------------
      Create an Arrow
         0

      1 2 3 4
        

            
        5 6
      ----------------------------------------------------------------*/
    arrowPoints[0].x =  0.0f * length;
    arrowPoints[0].y =  0.0f * length;
    arrowPoints[0].z =  1.0f * length;
    arrowPoints[0].i =  0.0f, arrowPoints[0].j = 1.0f, arrowPoints[0].k = 0.0f;

    arrowPoints[1].x = -0.3f * length;
    arrowPoints[1].y =  0.0f * length;
    arrowPoints[1].z =  0.8f * length;
    arrowPoints[1].i =  0.0f, arrowPoints[1].j = 1.0f, arrowPoints[1].k = 0.0f;

    arrowPoints[2].x = -0.1f * length;
    arrowPoints[2].y =  0.0f * length;
    arrowPoints[2].z =  0.8f * length;
    arrowPoints[2].i =  0.0f, arrowPoints[2].j = 1.0f, arrowPoints[2].k = 0.0f;

    arrowPoints[3].x =  0.1f * length;
    arrowPoints[3].y =  0.0f * length;
    arrowPoints[3].z =  0.8f * length;
    arrowPoints[3].i =  0.0f, arrowPoints[3].j = 1.0f, arrowPoints[3].k = 0.0f;

    arrowPoints[4].x =  0.3f * length;
    arrowPoints[4].y =  0.0f * length;
    arrowPoints[4].z =  0.8f * length;
    arrowPoints[4].i =  0.0f, arrowPoints[4].j = 1.0f, arrowPoints[4].k = 0.0f;

    arrowPoints[5].x = -0.1f * length;
    arrowPoints[5].y =  0.0f * length;
    arrowPoints[5].z =  0.0f * length;
    arrowPoints[5].i =  0.0f, arrowPoints[5].j = 1.0f, arrowPoints[5].k = 0.0f;

    arrowPoints[6].x =  0.1f * length;
    arrowPoints[6].y =  0.0f * length;
    arrowPoints[6].z =  0.0f * length;
    arrowPoints[6].i =  0.0f, arrowPoints[6].j = 1.0f, arrowPoints[6].k = 0.0f;

    atrTriSetBegin( arrow );

    atrTriSetVertex( arrow, &arrowPoints[1] );
    atrTriSetVertex( arrow, &arrowPoints[0] );
    atrTriSetVertex( arrow, &arrowPoints[4] );

    atrTriSetVertex( arrow, &arrowPoints[2] );
    atrTriSetVertex( arrow, &arrowPoints[3] );
    atrTriSetVertex( arrow, &arrowPoints[5] );

    atrTriSetVertex( arrow, &arrowPoints[5] );
    atrTriSetVertex( arrow, &arrowPoints[3] );
    atrTriSetVertex( arrow, &arrowPoints[6] );

    arrowPoints[0].i =  0.0f, arrowPoints[0].j = -1.0f, arrowPoints[0].k = 0.0f;
    arrowPoints[1].i =  0.0f, arrowPoints[1].j = -1.0f, arrowPoints[1].k = 0.0f;
    arrowPoints[2].i =  0.0f, arrowPoints[2].j = -1.0f, arrowPoints[2].k = 0.0f;
    arrowPoints[3].i =  0.0f, arrowPoints[3].j = -1.0f, arrowPoints[3].k = 0.0f;
    arrowPoints[4].i =  0.0f, arrowPoints[4].j = -1.0f, arrowPoints[4].k = 0.0f;
    arrowPoints[5].i =  0.0f, arrowPoints[5].j = -1.0f, arrowPoints[5].k = 0.0f;
    arrowPoints[6].i =  0.0f, arrowPoints[6].j = -1.0f, arrowPoints[6].k = 0.0f;

    atrTriSetVertex( arrow, &arrowPoints[1] );
    atrTriSetVertex( arrow, &arrowPoints[4] );
    atrTriSetVertex( arrow, &arrowPoints[0] );

    atrTriSetVertex( arrow, &arrowPoints[2] );
    atrTriSetVertex( arrow, &arrowPoints[5] );
    atrTriSetVertex( arrow, &arrowPoints[3] );

    atrTriSetVertex( arrow, &arrowPoints[5] );
    atrTriSetVertex( arrow, &arrowPoints[6] );
    atrTriSetVertex( arrow, &arrowPoints[3] );

    arrowPoints[0].x =  0.0f * length;
    arrowPoints[0].y =  0.0f * length;
    arrowPoints[0].z =  1.0f * length;
    arrowPoints[0].i =  1.0f;
    arrowPoints[0].j =  0.0f;
    arrowPoints[0].k =  0.0f;

    arrowPoints[1].x =  0.0f * length;
    arrowPoints[1].y = -0.3f * length;
    arrowPoints[1].z =  0.8f * length;
    arrowPoints[1].i =  1.0f;
    arrowPoints[1].j =  0.0f;
    arrowPoints[1].k =  0.0f;

    arrowPoints[2].x =  0.0f * length;
    arrowPoints[2].y = -0.1f * length;
    arrowPoints[2].z =  0.8f * length;
    arrowPoints[2].i =  1.0f;
    arrowPoints[2].j =  0.0f;
    arrowPoints[2].k =  0.0f;

    arrowPoints[3].x =  0.0f * length;
    arrowPoints[3].y =  0.1f * length;
    arrowPoints[3].z =  0.8f * length;
    arrowPoints[3].i =  1.0f;
    arrowPoints[3].j =  0.0f;
    arrowPoints[3].k =  0.0f;

    arrowPoints[4].x =  0.0f * length;
    arrowPoints[4].y =  0.3f * length;
    arrowPoints[4].z =  0.8f * length;
    arrowPoints[4].i =  1.0f;
    arrowPoints[4].j =  0.0f;
    arrowPoints[4].k =  0.0f;

    arrowPoints[5].x =  0.0f * length;
    arrowPoints[5].y = -0.1f * length;
    arrowPoints[5].z =  0.0f * length;
    arrowPoints[5].i =  1.0f;
    arrowPoints[5].j =  0.0f;
    arrowPoints[5].k =  0.0f;

    arrowPoints[6].x =  0.0f * length;
    arrowPoints[6].y =  0.1f * length;
    arrowPoints[6].z =  0.0f * length;
    arrowPoints[6].i =  1.0f;
    arrowPoints[6].j =  0.0f;
    arrowPoints[6].k =  0.0f;

    atrTriSetVertex( arrow, &arrowPoints[1] );
    atrTriSetVertex( arrow, &arrowPoints[4] );
    atrTriSetVertex( arrow, &arrowPoints[0] );

    atrTriSetVertex( arrow, &arrowPoints[2] );
    atrTriSetVertex( arrow, &arrowPoints[5] );
    atrTriSetVertex( arrow, &arrowPoints[3] );

    atrTriSetVertex( arrow, &arrowPoints[5] );
    atrTriSetVertex( arrow, &arrowPoints[6] );
    atrTriSetVertex( arrow, &arrowPoints[3] );

    arrowPoints[0].i = -1.0f;
    arrowPoints[0].j =  0.0f;
    arrowPoints[0].k =  0.0f;
    arrowPoints[1].i = -1.0f;
    arrowPoints[1].j =  0.0f;
    arrowPoints[1].k =  0.0f;
    arrowPoints[2].i = -1.0f;
    arrowPoints[2].j =  0.0f;
    arrowPoints[2].k =  0.0f;
    arrowPoints[3].i = -1.0f;
    arrowPoints[3].j =  0.0f;
    arrowPoints[3].k =  0.0f;
    arrowPoints[4].i = -1.0f;
    arrowPoints[4].j =  0.0f;
    arrowPoints[4].k =  0.0f;
    arrowPoints[5].i = -1.0f;
    arrowPoints[5].j =  0.0f;
    arrowPoints[5].k =  0.0f;
    arrowPoints[6].i = -1.0f;
    arrowPoints[6].j =  0.0f;
    arrowPoints[6].k =  0.0f;

    atrTriSetVertex( arrow, &arrowPoints[1] );
    atrTriSetVertex( arrow, &arrowPoints[0] );
    atrTriSetVertex( arrow, &arrowPoints[4] );

    atrTriSetVertex( arrow, &arrowPoints[2] );
    atrTriSetVertex( arrow, &arrowPoints[3] );
    atrTriSetVertex( arrow, &arrowPoints[5] );

    atrTriSetVertex( arrow, &arrowPoints[5] );
    atrTriSetVertex( arrow, &arrowPoints[3] );
    atrTriSetVertex( arrow, &arrowPoints[6] );

    atrTriSetEnd( arrow );
    return arrow;
}

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

void main( void ) {
    FxBool      done = FXFALSE, fog = FXFALSE, 
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
    AtrLight    *light, *projLight;
    AtrTexHandle texHandle;
    AtrMaterial *texMat;
    AtrMaterial *projMat;
    AtrTexHandle projTex;
    AtrImg       *projImg;
    FxU32       sampleInterval = 120;
    float       fps = 0.0f;
    AtrStats    stats;
    AtrEnv      *env;
    AtrXform    *rotation;
    AtmVector3  lightPosition  = {  30.0f, 5.0f,  0.0f }, 
                lightTarget    = {  0.0f,  5.0f,   0.0f }, 
                cameraPosition = {  60.0f, 55.0f, -60.0f },
                cameraTarget   = {  0.0f,  5.0f,   0.0f }, 
                up             = {  0.0f,  1.0f,   0.0f };
    AtrColor    camWin   = { 0.0f, 0.0f, 0.0f },
                lightWin = { 0.0f, 0.0f, 0.0f },
                textWin  = { 0.0f, 0.00f, 0.0f },
                imgWin  = { 0.0f, 0.00f, 0.0f };
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

    atrInit( 640, 480 );
    atrImgSetPath( getenv( "AT_TEXTURE_PATH" ) );

    image = atrImgAllocate( 1 );

    if ( !atuIniGet( "test.ini", "IMAGE FILE", ATU_INI_STRING, imgFilename, 64 ) ) {
        atuIniSet( "test.ini", "IMAGE FILE", ATU_INI_STRING, "marble.3df" );
        strcpy( imgFilename, "marble.3df" );
    }

    if ( !atrImgCreateFrom3df( image, imgFilename ) )
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

    if ( !atrImgCreateFrom3df( projImg, spotFilename ) )
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
        cube0 = createObject( primCube( 10.0f ), 
                             tmpMaterial,
                             atrXformAllocate( 1 ) );
        atmVector3Set( &cube0->objectToWorld->data[12],
                       0.0f, 5.0f, 15.0f );


        tmpMaterial = texMat;
        cube1 = createObject( primCube( 10.0f ), 
                              tmpMaterial,
                              atrXformAllocate( 1 ) );
        atmVector3Set( &cube1->objectToWorld->data[12],
                       0.0f, 5.0f, -15.0f );


        tmpMaterial = atrMaterialAllocate( 1 );
        atrMaterialSetup( tmpMaterial, ATR_MAT_GSHADE );
        sphere0 = createObject( primSphere( 5.0f, 10, 10 ), 
                               tmpMaterial,
                               atrXformAllocate( 1 ) );
        atmVector3Set( &sphere0->objectToWorld->data[12],
                        -15.0f, 5.0f, 0.0f );


        tmpMaterial = atrMaterialAllocate( 1 );
        atrMaterialSetup( tmpMaterial, ATR_MAT_GSHADE );
        sphere1 = createObject( primSphere( 5.0f, 10, 10 ), 
                               tmpMaterial,
                               atrXformAllocate( 1 ) );
        atmVector3Set( &sphere1->objectToWorld->data[12],
                        15.0f, 5.0f, 0.0f );

        tmpMaterial = atrMaterialAllocate( 1 );
        atrMaterialSetup( tmpMaterial, ATR_MAT_GSHADE );
        sphere2 = createObject( primSphere( 5.0f, 10, 10 ), 
                               tmpMaterial,
                               atrXformAllocate( 1 ) );
        atmVector3Set( &sphere2->objectToWorld->data[12],
                        30.0f, 5.0f, 30.0f );

        tmpMaterial = atrMaterialAllocate( 1 );
        atrMaterialSetup( tmpMaterial, ATR_MAT_GSHADE );
        sphere3 = createObject( primSphere( 5.0f, 10, 10 ), 
                               tmpMaterial,
                               atrXformAllocate( 1 ) );
        atmVector3Set( &sphere3->objectToWorld->data[12],
                        30.0f, 5.0f, -30.0f );

        tmpMaterial = atrMaterialAllocate( 1 );
        atrMaterialSetup( tmpMaterial, ATR_MAT_GSHADE );
        sphere4 = createObject( primSphere( 5.0f, 10, 10 ), 
                               tmpMaterial,
                               atrXformAllocate( 1 ) );
        atmVector3Set( &sphere4->objectToWorld->data[12],
                        -30.0f, 5.0f, 30.0f );

        tmpMaterial = atrMaterialAllocate( 1 );
        atrMaterialSetup( tmpMaterial, ATR_MAT_GSHADE );
        sphere5 = createObject( primSphere( 5.0f, 10, 10 ), 
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
        plane = createObject( primPlane( 100.0f ), 
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
        arrow = createObject( primArrow( 5.0f ), 
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
        ball = createObject(  primSphere( 1.0f, 10, 10 ), 
                              tmpMaterial,
                              atrXformAllocate( 1 ) );
        atrXformPointAt( ball->objectToWorld, lightTarget, lightPosition,
                         up );
    }
    

    {
        AtrXform tmp;
        atrXformPointAt( &tmp, lightTarget, lightPosition, up );
        lightPov = createWindow( 480, 351, 608, 479, &tmp );
        lightPov->camera->fovRadians = 90.0f * ATM_DEGREE;

        atrXformPointAt( &tmp, cameraTarget, cameraPosition, up );
        cameraPov = createWindow( 0, 0, 479, 479, &tmp );

        textWindow = createWindow( 480, 128, 639, 350, &tmp );
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

    done = FXFALSE;
    while( !done ) {
        char charCache[64];

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
            GrVertex a,b,c,d;
            a.x =   0.0f, a.y = 479.0f, a.oow = 1.0f;
            b.x = 479.0f, b.y = 479.0f, b.oow = 1.0f;
            c.x =   0.0f, c.y =   0.0f, c.oow = 1.0f;
            d.x = 479.0f, d.y =   0.0f, d.oow = 1.0f;
            a.tmuvtx[0].sow =   0.0f, a.tmuvtx[0].tow = 255.0f, a.tmuvtx[0].oow = 1.0f;
            b.tmuvtx[0].sow = 255.0f, b.tmuvtx[0].tow = 255.0f, b.tmuvtx[0].oow = 1.0f;
            c.tmuvtx[0].sow =   0.0f, c.tmuvtx[0].tow =   0.0f, c.tmuvtx[0].oow = 1.0f;
            d.tmuvtx[0].sow = 255.0f, d.tmuvtx[0].tow =   0.0f, d.tmuvtx[0].oow = 1.0f;

            atrPushMaterial( texMat );
            currentWindow( cameraPov, &camWin );
            grDrawTriangle( &a, &b, &c );
            grDrawTriangle( &c, &b, &d );
            atrPopMaterial( 0 );
        }

        if ( movie ) {
            atrSelectCanvas( lightPov->canvas );
            grSstIdle();
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

        if ( !sampleInterval ) {
            atrStatsRetrieve( &stats );
            fps = ((float)stats.frames) / stats.elapsedTime;
            atrStatsReset();
            sampleInterval = 300;
        }
        sampleInterval--;

        sprintf( charCache, "SI:     %3d", sampleInterval );
        atrDrawString( charCache, 8.0f, 10.0f, 480.0f, 180.0f );
        sprintf( charCache, "FPS:    %2.3f", fps );
        atrDrawString( charCache, 8.0f, 10.0f, 480.0f, 170.0f );

        grClipWindow( 0, 0, 640, 480 );
        atrSwapBuffer( 0 );

        atrRemoveLight( 0 );
        atrPopEnv( 0 );
        if (kbhit()) {
            switch( getch() ) {
              case ' ':
                done = FXTRUE;
                break;
              case 'm':
                type++;
                type = type % 4;
                break;
              case 'q':
                puts( "Enter new quadratic attenuation factor:" );
                quadFactor = getFloat();
                break;
              case 'l':
                puts( "Enter new linear attenuation factor:" );
                linFactor = getFloat();
                break;
              case 'c':
                puts( "Enter new constant attenuation factor:" );
                constFactor = getFloat();
                break;
              case 'r':
                puts( "Enter new red light value:" );
                lightColor.r = getFloat();
                break;
              case 'g':
                puts( "Enter new green light value:" );
                lightColor.g = getFloat();
                break;
              case 'b':
                puts( "Enter new blue light value:" );
                lightColor.b = getFloat();
              case 'e':
                puts( "Enter new specular exponent:" );
                specExponent = ( FxU32 ) getFloat();
                break;
              case 's':
                surface++;
                surface = surface % 3;
                break;
              case '1':
                puts( "Enter new red spec value:" );
                specColor.r = getFloat();
                break;
              case '2':
                puts( "Enter new green spec value:" );
                specColor.g = getFloat();
                break;
              case '3':
                puts( "Enter new blue spec value:" );
                specColor.b = getFloat();
                break;
              case 'f':
                fog ^= 1;
                break;
              case '*':
                movie ^= 1;
                break;
              case '.':
                bilerp ^= 1;
                break;
              case 'p':
                projector ^= 1;
                break;
              case '[':
                movieProj ^= 1;
                break;
              case '-':
                break;
            }
        }
    }
    atrShutdown();
    return;
}


