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
** $Date: 10/11/00 7:34:13 PM$ 
**
*/

#include "atrender.h"
#include "fxatr.h"
#include <string.h>

/*
**-----------------------------------------------------------------
** File Scope Data
**-----------------------------------------------------------------
*/

static const char  _dataType[]     = "atrOpenTriSet";
static const FxU32 _binaryRevision = 0;


/*-------------------------------------------------------------------
  Function: atrOpenTriSetAllocate
  Date: 4/14
  Implementor(s): jdt
  Library: AT Render
  Description:
    Allocate and initialize a new open triSet
  Arguments:
    num - number of triSets to allocate and assign to the pointer
  Return:
    new array of AtrOpenTriSets
  -------------------------------------------------------------------*/
AtrOpenTriSet *atrOpenTriSetAllocate( FxU32 num ) {
    FxU32 *n;
    FxU32 index;
    AtrOpenTriSet *t;

#ifdef AT_DEBUGGING
    if ( num == 0 ) 
        atuError( FXTRUE, "atrOpenTriSetAllocate(): Invalid parameter.\n" );
#endif

    n = atuMemCalloc( sizeof( AtrOpenTriSet ) * num + sizeof(FxU32), 1 );
    *n = num;
    t = (AtrOpenTriSet*)(n+1);
    for ( index = 0; index < num; index++ ) 
        atrOpenTriSetDefault( t+index );
    return t;
}

/*-------------------------------------------------------------------
  Function: atrOpenTriSetDeallocate
  Date: 3/19/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Safely deallocate arrays of atrOpenTriSets.
  Arguments:
    e - pointer to enviroments
  Return:
    none
  -------------------------------------------------------------------*/
void atrOpenTriSetDeallocate( AtrOpenTriSet t[] ) {
    FxU32 *tmp;
#ifdef AT_DEBUGGING
    if ( !t )
        atuError( FXTRUE, "atrOpenTriSetDeallocate(): Invalid parameter.\n" );
#endif

    tmp = (FxU32*)t;
    tmp = tmp - 1;

    atuMemFree( tmp );
    return;
}

/*-------------------------------------------------------------------
  Function: atrOpenTriSetDefault
  Date: 4/16
  Implementor(s): jdt
  Library: AT Render
  Description:
    Initialize data structure

    For Open Tri Set:
        Clear to 0;
  Arguments:
    t - Open triset to initialize
  Return:
    none
  -------------------------------------------------------------------*/
void atrOpenTriSetDefault( AtrOpenTriSet *t ) {
#ifdef AT_DEBUGGING
    if ( !t )
        atuError( FXTRUE, "atrTriSetDefault(): Invalid parameter.\n" );
#endif
    memset( t, 0, sizeof( AtrTriSet ) );
    return;
}



/*-------------------------------------------------------------------
  Function: atrOpenTriSetStore
  Date: 3/19/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Store a triset out to a file stream.
  Arguments:
    stream - file stream
    t - triSet to store
  Return:
    FXTRUE - success
    FXFALSE - failure
  -------------------------------------------------------------------*/
FxBool atrOpenTriSetStore( AtrOpenTriSet *t, FILE *stream ) {
    FxU32 count, flag;
#ifdef AT_DEBUGGING
    if ( !t || !stream ) 
        atuError( FXTRUE, "atrOpenTriSetStore(): Invalid parameter.\n" );
    if ( !t->srcVertices || 
         !t->connectivity ||
         !t->normals )
         atuError( FXTRUE, "atrOpenTriSetStore(): incomplete OpenTriSet definition.\n" );
#endif    
    count = fwrite( &_binaryRevision, 
                    sizeof( _binaryRevision ), 
                    1,
                    stream );
    if ( count != 1 ) return FXFALSE;
    count = fwrite( _dataType,
                    sizeof( _dataType ), 
                    1,
                    stream );
    if ( count != 1 ) return FXFALSE;


    /* First store number of vertices and triangles,
       then store all vertex data and triangle data,
       don't bother with the non-persistent dstVertices */
    count = fwrite( &t->numVertices,
                    sizeof( t->numVertices ),
                    1,
                    stream );
    if ( count != 1 ) return FXFALSE;
    count = fwrite( &t->numTriangles,
                    sizeof( t->numTriangles ),
                    1,
                    stream );
    if ( count != 1 ) return FXFALSE;

    /* srcVertices */
    count = fwrite( t->srcVertices,
                    t->numVertices * sizeof( AtrVertex ),
                    1,
                    stream );
    if ( count != 1 ) return FXFALSE;

    /* connectivity */
    count = fwrite( t->connectivity,
                    t->numTriangles * sizeof( FxU32[3] ),
                    1,
                    stream );
    if ( count != 1 ) return FXFALSE;

    /* normals */
    count = fwrite( t->normals,
                    t->numTriangles * sizeof( AtmVector3 ),
                    1,
                    stream );
    if ( count != 1 ) return FXFALSE;

    /* texCoords */
    if ( t->texCoords ) {
        flag = 1;
        count = fwrite( &flag,
                        sizeof( FxU32 ),
                        1,
                        stream );
        if ( count != 1 ) return FXFALSE;
        count = fwrite( t->texCoords,
                        t->numTriangles * sizeof( AtmVector2[3] ),
                        1,
                        stream );
        if ( count != 1 ) return FXFALSE;
    } else {
        flag = 0;
        count = fwrite( &flag,
                        sizeof( FxU32 ),
                        1,
                        stream );
        if ( count != 1 ) return FXFALSE;
    }
    


    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: atrOpenTriSetLoad
  Date: 4/13
  Implementor(s): jdt
  Library: AT Render
  Description:
    Load an OpenTriSet from a stream
  Arguments:
    stream - file stream
    t - memory into which to load TriSet
  Return:
    FXTRUE  - success
    FXFALSE - failure
  -------------------------------------------------------------------*/
FxBool atrOpenTriSetLoad( AtrOpenTriSet *t, FILE *stream ) {
    FxU32 rev;
    char  type[64];
    FxU32 count;
    FxU32 flag;

#ifdef AT_DEBUGGING
    if ( !t || !stream ) 
        atuError( FXTRUE, "atrTriSetLoad(): Invalid parameter.\n" );
#endif    

    count = fread( &rev, sizeof( _binaryRevision ), 1, stream );
    if ( count != 1 ) return FXFALSE;
    if ( rev != _binaryRevision ) return FXFALSE;
    count = fread( type, sizeof( _dataType ), 1, stream );
    if ( count != 1 ) return FXFALSE;
    if ( strcmp( type, _dataType ) ) return FXFALSE;

    /* Data Sizes */
    count = fread( &t->numVertices, sizeof( t->numVertices ), 1, stream );
    if ( count != 1 ) return FXFALSE;
    count = fread( &t->numTriangles, sizeof( t->numTriangles ), 1, stream );
    if ( count != 1 ) return FXFALSE;

    /* src/dst Vertices */
    t->srcVertices = atuMemCalloc( sizeof( AtrVertex ), t->numVertices );
    t->dstVertices = atuMemCalloc( sizeof( AtrDstVertex ), t->numVertices );

    count = fread( t->srcVertices, sizeof( AtrVertex ) * t->numVertices, 1, stream );
    if ( count != 1 ) return FXFALSE;

    /* connectivity */
    t->connectivity = atuMemCalloc( sizeof( FxU32[3] ), t->numTriangles );
    count = fread( t->connectivity, sizeof( FxU32[3] ) * t->numTriangles, 1, stream );
    if ( count != 1 ) return FXFALSE;

    /* normals */
    t->normals = atuMemCalloc( sizeof( AtmVector3 ), t->numTriangles );
    count = fread( t->normals, sizeof( AtmVector3 ) * t->numTriangles, 1, stream );
    if ( count != 1 ) return FXFALSE;

    /* texCoords */
    count = fread( &flag, sizeof( FxU32 ) , 1, stream );
    if ( count != 1 ) return FXFALSE;

    if ( flag ) {
        t->texCoords = atuMemCalloc( sizeof( AtmVector2[3] ), t->numTriangles );
        count = fread( t->texCoords, sizeof( AtmVector2[3] ) * t->numTriangles, 1, stream );
        if ( count != 1 ) return FXFALSE;
    }
    return FXTRUE;
}
    
/*-------------------------------------------------------------------
  Function: atrOpenTriSetCalcNormals
  Date: 6/15
  Implementor(s): jdt
  Library: AT Render
  Description:
    Calculate surface and vertex normals for an open triset. 
    Triset must have space for surface normals allocated.
  Arguments:
    t - opentriset
  Return:
    none
  -------------------------------------------------------------------*/
void atrOpenTriSetCalcNormals( AtrOpenTriSet *t ) {
    FxU32 index, face, vertex;
#ifdef AT_DEBUGGING
    if ( !t ) 
        atuError( FXTRUE, "atrOpenTriSet: Invalid parameter.\n" );
#endif
    /* Allocate Surface normals if necessary */
    if ( !t->normals ) {
        t->normals = atuMemCalloc( sizeof( AtmVector3 ), t->numTriangles );
    }

    for ( index = 0; index < t->numTriangles; index++ ) {
        AtmVector3 edge1, edge2, v0, v1, v2;
        FxU32 (*tri)[3] = t->connectivity + index;
        v0[0] = t->srcVertices[(*tri)[0]].x;
        v0[1] = t->srcVertices[(*tri)[0]].y;
        v0[2] = t->srcVertices[(*tri)[0]].z;
        v1[0] = t->srcVertices[(*tri)[1]].x;
        v1[1] = t->srcVertices[(*tri)[1]].y;
        v1[2] = t->srcVertices[(*tri)[1]].z;
        v2[0] = t->srcVertices[(*tri)[2]].x;
        v2[1] = t->srcVertices[(*tri)[2]].y;
        v2[2] = t->srcVertices[(*tri)[2]].z;
        atmVector3Sub( edge1, v1, v0 );
        atmVector3Sub( edge2, v2, v1 );
        atmVector3Cross( t->normals[index], edge1, edge2 );
        atmVector3Normalize( t->normals[index], t->normals[index] ); 
    }
 
    /* Clear All Vertex Normals */
    for( index = 0; index < t->numVertices; index++ ) {
        t->srcVertices[index].i = 0.0f;
        t->srcVertices[index].j = 0.0f;
        t->srcVertices[index].k = 0.0f;
    }

    /* Average all participating surface normals */
    for( vertex = 0; vertex < t->numVertices; vertex++ ) {
        for( face = 0; face < t->numTriangles; face++ ) {
            if ( t->connectivity[face][0] == vertex )
                atmVector3Add( (float*)&t->srcVertices[vertex].i,
                               (float*)&t->srcVertices[vertex].i,
                               t->normals[face] );
            if ( t->connectivity[face][1] == vertex )
               atmVector3Add( (float*)&t->srcVertices[vertex].i,
                              (float*)&t->srcVertices[vertex].i,
                              t->normals[face] );
           if ( t->connectivity[face][2] == vertex )
               atmVector3Add( (float*)&t->srcVertices[vertex].i,
                               (float*)&t->srcVertices[vertex].i,
                               t->normals[face] );
        }
        atmVector3Normalize( (float*)&t->srcVertices[vertex].i,
                             (float*)&t->srcVertices[vertex].i );
    }
}

