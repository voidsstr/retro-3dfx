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
** $Date: 10/11/00 7:34:14 PM$ 
**
*/

#include "atrender.h"
#include "math.h"

static float _cube[6][4][5] = {
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

static float _normals[6][3] = {
    {  0.0f,  0.0f, -1.0f },
    {  1.0f,  0.0f,  0.0f },
    {  0.0f,  0.0f,  1.0f },
    { -1.0f,  0.0f,  0.0f },
    {  0.0f, -1.0f,  0.0f },
    {  0.0f,  1.0f,  0.0f }
};

/*-------------------------------------------------------------------
  Function: atrPrimSphere
  Date: 7/12
  Implementor(s): Ron Levine
  Library: AT Render
  Description:
  Generate a sphere triset
  Arguments:
  radius - radius of sphere
  nParallels - subdivisions in parallels
  nMeridians - subdivisions in meridians
  Return:
  new triset
  -------------------------------------------------------------------*/
AtrTriSet *atrPrimSphere( float radius, int nParallels, int nMeridians ){
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
  Function: atrPrimCube
  Date: 7/12
  Implementor(s): jdt
  Library: AT Render
  Description:
  Generate a cube triset
  Arguments:
  s - length along a side
  Return:
  new triset
  -------------------------------------------------------------------*/
AtrTriSet *atrPrimCube( float s ) {
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
  Function: atrPrimPlane
  Date: 7/12
  Implementor(s): jdt
  Library: AT Render
  Description:
  Generate a plane triset
  Arguments:
  s - length along a side
  Return:
  new triset
  -------------------------------------------------------------------*/
AtrTriSet *atrPrimPlane( float s ) {
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

/*-------------------------------------------------------------------
  Function: atrPrimArrow
  Date: 7/12
  Implementor(s): jdt
  Library: AT Render
  Description:
  Generate a plane triset
  Arguments:
  length - length of arrow
  Return:
  new triset
  -------------------------------------------------------------------*/
AtrTriSet *atrPrimArrow( float length ) {
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
