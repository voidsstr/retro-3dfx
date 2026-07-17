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
** $Date: 10/11/00 7:34:21 PM$ 
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

static const char  _dataType[]     = "atrTriSet";
static const FxU32 _binaryRevision = 0;


/*-------------------------------------------------------------------
  Function: atrTriSetAllocate
  Date: 3/19/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Allocate and initialize a new triSet
  Arguments:
    num - number of triSets to allocate and assign to the pointer
  Return:
    new array of AtrTriSets
  -------------------------------------------------------------------*/
AtrTriSet *atrTriSetAllocate( FxU32 num ) {
    FxU32 *n;
    FxU32 index;
    AtrTriSet *t;

#ifdef AT_DEBUGGING
    if ( num == 0 ) 
        atuError( FXTRUE, "atrTriSetAllocate(): Invalid parameter.\n" );
#endif

    n = atuMemCalloc( sizeof( AtrTriSet ) * num + sizeof(FxU32), 1 );
    *n = num;
    t = (AtrTriSet*)(n+1);
    for( index = 0; index < num;index++ ) 
        atrTriSetDefault( t+index );
    return t;
}

/*-------------------------------------------------------------------
  Function: atrTriSetDeallocate
  Date: 3/19/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Safely deallocate arrays of atrTriSets.
  Arguments:
    e - pointer to enviroments
  Return:
    none
  -------------------------------------------------------------------*/
void atrTriSetDeallocate( AtrTriSet t[] ) {
    FxU32 *tmp, num, count;
    _AtrTriSetNode *node, *nextNode;

#ifdef AT_DEBUGGING
    if ( !t )
        atuError( FXTRUE, "atrTriSetDeallocate(): Invalid parameter.\n" );
#endif

    tmp = (FxU32*)t;
    tmp = tmp - 1;
    num = *tmp;

    for( count = 0; count < num; count++ ) {
        node = t[count].nodes;
        while( node ) {
            atuMemFree( node->vertices );
            atuMemFree( node->connectivity );
            nextNode = node->next;
            atuMemFree( node );
            node = nextNode;
        }
    }

    atuMemFree( tmp );
    return;
}


/*-------------------------------------------------------------------
  Function: atrTriSetDefault
  Date: 4/16
  Implementor(s): jdt
  Library: AT Render
  Description:
    Initialize data structure

    For Tri Set:
        Clear to 0;
  Arguments:
    t - triset to initialize
  Return:
    none
  -------------------------------------------------------------------*/
void atrTriSetDefault( AtrTriSet *t ) {
#ifdef AT_DEBUGGING
    if ( !t )
        atuError( FXTRUE, "atrTriSetDefault(): Invalid parameter.\n" );
#endif
    memset( t, 0, sizeof( AtrTriSet ) );
    return;
}


/*-------------------------------------------------------------------
  Function: atrTriSetStore
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
FxBool atrTriSetStore( AtrTriSet *t, FILE *stream ) {
    FxU32 count;
    _AtrTriSetNode *n;
    FxU32 zero = 0;
    FxU32 numTris;
#ifdef AT_DEBUGGING
    if ( !t || !stream ) 
        atuError( FXTRUE, "atrTriSetStore(): Invalid parameter.\n" );
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

    n = t->nodes;
    t->nodes = 0;
    count = fwrite( t,
                    sizeof( AtrTriSet ),
                    1,
                    stream );
    if ( count != 1 ) return FXFALSE;
    t->nodes = n;

    while( n ) {
        FxU32 one = 1;
        FxU32 *connectivity;
        count = fwrite( &one, sizeof( FxU32 ), 1, stream );
        if ( count != 1 ) return FXFALSE;
        count = fwrite( &n->numVertices, sizeof( FxU32 ), 1, stream );
        if ( count != 1 ) return FXFALSE;
        count = fwrite( n->vertices, 
                        sizeof( AtrVertex ) * n->numVertices,
                        1,
                        stream );
        if ( count != 1 ) return FXFALSE;
        connectivity = n->connectivity;
        numTris = 0;
        while( *connectivity ) {
            connectivity++;
            numTris++;
        }
        count = fwrite( &numTris, sizeof( FxU32 ), 1, stream );
        if ( count != 1 ) return FXFALSE;
        count = fwrite( n->connectivity, 
                        sizeof( FxU32 ) * numTris, 
                        1, 
                        stream );
        if ( count != 1 ) return FXFALSE;
        n = n->next;
    }
    count = fwrite( &zero, sizeof( FxU32 ), 1, stream );
    if ( count != 1 ) return FXFALSE;

    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: atrTriSetLoad
  Date: 3/19/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Load an env from a file stream.
  Arguments:
    stream - file stream
    e - memory into which to load env
  Return:
    FXTRUE  - success
    FXFALSE - failure
  -------------------------------------------------------------------*/
FxBool atrTriSetLoad( AtrTriSet *t, FILE *stream ) {
    FxU32 rev;
    char  type[64];
    FxU32 count;
    _AtrTriSetNode *n;
    _AtrTriSetNode *lastNode = 0;

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

    count = fread( t, sizeof( AtrTriSet ), 1, stream );
    if ( count != 1 ) return FXFALSE;

    count = fread( &flag, sizeof( FxU32 ), 1, stream );
    if ( count != 1 ) return FXFALSE;

    while( flag ) {
        FxU32 numTris;
        n = atuMemCalloc( sizeof( _AtrTriSetNode ), 1 );
        if ( t->nodes == 0 ) t->nodes = n;
        else lastNode->next = n;
        lastNode = n;
        count = fread( &n->numVertices, sizeof( FxU32 ), 1, stream );
        if ( count != 1 ) return FXFALSE;
        n->vertices = atuMemAlignedCalloc( n->numVertices, 
                                           sizeof( AtrVertex ),
                                           32 );
        count = fread( n->vertices, 
                       sizeof( AtrVertex ) * n->numVertices,
                       1,
                       stream );
        if ( count != 1 ) return FXFALSE;
        count = fread( &numTris, sizeof( FxU32 ), 1, stream );
        if ( count != 1 ) return FXFALSE;
        n->connectivity = atuMemAlignedCalloc( numTris, sizeof( FxU32 ), 4 );
        count = fread( n->connectivity, sizeof( FxU32 ) * numTris, 1, stream );
        if ( count != 1 ) return FXFALSE;
        count = fread( &flag, sizeof( FxU32 ), 1, stream );
        if ( count != 1 ) return FXFALSE;
    }

    return FXTRUE;
}

typedef FxU32 tri[4];
static FxBool     _inTriSet = FXFALSE;
static AtrVertex  *_vertexList;
static FxU32      *_vertexFlagList;
static FxU32      _numVertices;
static FxU32      _numFaces;
static tri        *_faceList;
static FxU32      _thisTri;
static FxU32      _maxNumFaces;
static FxU32      _maxNumVertices;
static FxBool     _calcNormals;
static AtrTexHandle *_lightMapList;  
static FxU32      _numLightMaps;
static FxU32      _maxNumLightMaps;
    
/*-------------------------------------------------------------------
  Function: atrTriSetBegin
  Date: 3/22/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Begin defining a tri-set.  All calls to atrTriSetVertex must
    be bracked in calls to begin and end.  Every three calls to 
    atrTriSetVertex will define a new face.  When all of the faces
    are defined a call to end will complete definition of the 
    set.
  Arguments:
    t - an empty AtrTriSet
  Return:
    none
  -------------------------------------------------------------------*/
void atrTriSetBegin( AtrTriSet *t ) {

    FXUNUSED(t);

    if ( _inTriSet )
        atuError( FXTRUE, "atrTriSetBegin(): Nested calls to atrTriSetBegin"
                  " are not allowed.\n" );
    _inTriSet        = FXTRUE;
    _numVertices     = 0;
    _numFaces        = 0;
    _numLightMaps    = 0;
    _faceList        = atuMemCalloc( sizeof( tri ), 1000 );
    _vertexList      = atuMemCalloc( sizeof( AtrVertex ), 1000 );
    _lightMapList    = atuMemCalloc( sizeof( AtrTexHandle ), 1000 );
    _maxNumVertices  = 1000;
    _maxNumFaces     = 1000;
    _maxNumLightMaps = 1000;
    _thisTri = 0;
    _calcNormals = FXFALSE;
}

/*-------------------------------------------------------------------
  Function: atrTriSetVertex
  Date: 3/22/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Add a vertex to an AtrTriSet
  Arguments:
    v - vertex
    t - triset
  Return:   
    none
  -------------------------------------------------------------------*/
void atrTriSetVertex( AtrTriSet *t, AtrVertex *v ) {
    FxU32 index;

    FXUNUSED(t);

    if ( !_inTriSet ) 
        atuError( FXTRUE, "atrTriSetVertex(): Called outside of "
                  "atrTriSetBegin()/End() pair.\n" );
    
    /* Search Vertex List */
    for( index = 0; index < _numVertices; index++ ) {
        if ( !memcmp( v, _vertexList + index, sizeof( AtrVertex ) ) )
            break;
    }

    /* Append */
    if ( index == _numVertices ) {
        _numVertices++;
        _vertexList[index] = *v;
    }

    /* Grow */
    if ( _numVertices == _maxNumVertices ) {
        AtrVertex *oldList = _vertexList;
        _maxNumVertices += 1000;
        _vertexList = atuMemCalloc( sizeof( AtrVertex ), _maxNumVertices );
        memcpy( _vertexList, oldList, sizeof( AtrVertex ) *_numVertices );
        atuMemFree( oldList );
    }

    _faceList[_numFaces][_thisTri++] = index;

    if ( _thisTri == 3 ) {
        _thisTri = 0;
        _numFaces++;
        /* Grow */
        if ( _numFaces == _maxNumFaces ) {
            tri *oldList = _faceList;
            _maxNumFaces += 1000;
            _faceList = atuMemCalloc( sizeof( tri ), _maxNumFaces );
            memcpy( _faceList, oldList, sizeof( tri ) * _numFaces );
            atuMemFree( oldList );
        }
    }                                
    return;
}

/*-------------------------------------------------------------------
  Function: atrTriSetLightMap
  Date: 4/25/96
  Implementor(s): jdt
  Library: AT Render
  Description: 
    Assign a light map to the current face.
  Arguments:
    t - tri set under construction
    m - handle to light map texture
  Return:
    none
  -------------------------------------------------------------------*/
void atrTriSetLightMap( AtrTriSet *t, AtrTexHandle m ) {

    FXUNUSED(t);

    if ( !_inTriSet ) 
        atuError( FXTRUE, "atrTriSetLightMap(): Called outside of"
                  " begin/end pairs.\n" );

#ifdef AT_DEBUGGING
        if ( !t )
          atuError( FXTRUE, "atrTriSetLightMap(): invalid parameter t\n" );
        if ( !m )
          atuError( FXTRUE, "atrTriSetLightMap(): invalid parameter m\n" );
#endif

    if ( _numLightMaps == _maxNumLightMaps ) {
        _maxNumLightMaps += 1000;
        _lightMapList = atuMemRealloc( _lightMapList, 
                                    sizeof( AtrTexHandle )*_maxNumLightMaps );
    }
    _lightMapList[_numLightMaps] = m;
    _numLightMaps++;
}


/*-------------------------------------------------------------------
  Function: _findFirstFit
  Date: 4/2/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Find the first triangle to meet the specified cache-matching re
    quirements.
  Arguments:
    minMatch - minimum matching reqs.
    faceList - connectivity list to search
    numFaces - extents of connectivity list
    vertexFlagList - list storing match/chunk position info.
    nextVertex - index of next vertex in destination chunk
  Return:
    FXTRUE - found match and update flag list
    FXFALSE - couldn't make match
  -------------------------------------------------------------------*/
static FxBool _findFirstFit( FxU32 minMatch, tri faceList[], 
                      FxU32 numFaces, FxU32 *vertexFlagList,
                      FxU32 *nextVertex ) {
    FxU32 index;
    FxU32 matches;
    for( index = 0; index < numFaces; index++ ) {
        if ( faceList[index][3] ) continue;
        else {
            matches = 0;
            if ( vertexFlagList[faceList[index][0]] )
                matches++;
            if ( vertexFlagList[faceList[index][1]] )
                matches++;
            if ( vertexFlagList[faceList[index][2]] )
                matches++;
            if ( matches == minMatch ) {
                if ( !vertexFlagList[faceList[index][0]] ) {
                    vertexFlagList[faceList[index][0]] = *nextVertex + 1;
                    (*nextVertex)++;
                }
                if ( !vertexFlagList[faceList[index][1]] ) {
                    vertexFlagList[faceList[index][1]] = *nextVertex + 1;
                    (*nextVertex)++;
                }
                if ( !vertexFlagList[faceList[index][2]] ) {
                    vertexFlagList[faceList[index][2]] = *nextVertex + 1;
                    (*nextVertex)++;
                }
                faceList[index][3] = 1;
                return FXTRUE;
            }
        }
    }
    return FXFALSE;
}


#define BYTE_PACK( A, B, C, D ) ((((A) & 0xFF) << 24) | \
                                 (((B) & 0xFF) << 16) | \
                                 (((C) & 0xFF) <<  8) | \
                                 (((D) & 0xFF) <<  0) )

/*-------------------------------------------------------------------
  Function: _extractChunk
  Date: 4/2/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Extract a cache-sorted chunk from the file-scope vertex and face
    build lists.  Returns a completed node for inclusion in a triSet.
  Arguments:
    node - node to extract chunk into
    vertexList - list of vertices from which to extract data
    vertexFlagList - storeage for sorting data
    numFaces - number of faces in face list ( after compression,
               the new number of faces is stored here ).
    facelist - list of conectivity for all triangles in file-scopre
               vertex and face build list.
  Return:
    none
  -------------------------------------------------------------------*/
static void _extractChunk( _AtrTriSetNode *node, 
                    AtrVertex      vertexList[],
                    FxU32          vertexFlagList[],
                    FxU32          *numFaces,
                    tri            faceList[] ) {
    FxU32 nextVertex = 0;
    FxU32 facesLeft = *numFaces;
    tri   *tmpFaceList;
    AtrTexHandle *tmpLightMapList;
    FxU32 index;

#ifdef AT_DEBUGGING
    /* Validate Parameters */
    if ( !node || 
         !vertexList ||
         !numFaces   ||
         !*numFaces  ||
         !faceList   ||
         !*faceList )
        atuError( FXTRUE, "_extractChunk(): Invalid parameter.\n" );            
#endif

    /* Clear Vertex Flag List */
    memset( vertexFlagList, 0, sizeof( FxU32 ) * _numVertices );
    
    /* Mark First Triangle */
    faceList[0][3] = 1;
    facesLeft--;
    vertexFlagList[faceList[0][0]] = (nextVertex+1);
    nextVertex++;
    vertexFlagList[faceList[0][1]] = (nextVertex+1);
    nextVertex++;                        
    vertexFlagList[faceList[0][2]] = (nextVertex+1);
    nextVertex++;                        

    /* For Each Pass Over Vertices Find the First/Best Fit */
    while( ( nextVertex < ATR_CHUNK_SIZE ) && facesLeft ) {
        FxU32 space = ATR_CHUNK_SIZE - nextVertex;
        if ( _findFirstFit( 3, faceList, *numFaces, 
                            vertexFlagList, &nextVertex ) ) {
            facesLeft--;
            continue;
        }
        if ( _findFirstFit( 2, faceList, *numFaces, 
                            vertexFlagList, &nextVertex ) ) {
            facesLeft--;
            continue;
        }
        if ( space > 1 ) {
            if ( _findFirstFit( 1, faceList, *numFaces, 
                                vertexFlagList, &nextVertex ) ) {
                facesLeft--;
                continue;
            } else if ( space == 2 ) {
                break;
            }
        } else {
            break;
        }
        if ( space > 2 ) {
            _findFirstFit( 0, faceList, *numFaces, 
                           vertexFlagList, &nextVertex );
            facesLeft--;
            continue;
        }
    }

    /* Catch Any Last Full Matches */
    while( facesLeft && 
           _findFirstFit(3,faceList,*numFaces,vertexFlagList,&nextVertex))
        facesLeft--;

    /* Fill Node */
    node->numVertices = nextVertex;
    node->vertices = atuMemAlignedCalloc( sizeof( AtrVertex ), 
                                          node->numVertices, 
                                          32 );
    node->connectivity = atuMemAlignedCalloc( sizeof( FxU32 ), 
                                              (*numFaces - facesLeft)+1,
                                              32 );

    if ( _numLightMaps )
        node->lightMaps = 
            atuMemCalloc( sizeof( AtrTexHandle ),(*numFaces - facesLeft ) );

    tmpLightMapList = _lightMapList;
    tmpFaceList = faceList;
    for( index = 0; index < (*numFaces - facesLeft); index++ ) {
        FxU32 newVertexIndex0;
        FxU32 newVertexIndex1;
        FxU32 newVertexIndex2;

        while( !((*tmpFaceList)[3]) ) {
            tmpFaceList++;
            tmpLightMapList++;
        }

        newVertexIndex0 = vertexFlagList[(*tmpFaceList)[0]] - 1;
        newVertexIndex1 = vertexFlagList[(*tmpFaceList)[1]] - 1;
        newVertexIndex2 = vertexFlagList[(*tmpFaceList)[2]] - 1;

        node->connectivity[index] = BYTE_PACK( 0,
                                               newVertexIndex0,
                                               newVertexIndex1,
                                               newVertexIndex2 );

        if ( _numLightMaps ) node->lightMaps[index] = *tmpLightMapList;

        node->vertices[newVertexIndex0] = vertexList[(*tmpFaceList)[0]];
        node->vertices[newVertexIndex1] = vertexList[(*tmpFaceList)[1]];
        node->vertices[newVertexIndex2] = vertexList[(*tmpFaceList)[2]];

        tmpFaceList++;
        tmpLightMapList++;
    }

    /* Compress Triangle List (and light map list)*/
    {
        FxU32 nextSrcFace = 0;
        FxU32 nextDestFace = 0;
        while( nextSrcFace < *numFaces ) {
            if ( !faceList[nextSrcFace][3] ) {
                faceList[nextDestFace][0] = faceList[nextSrcFace][0];
                faceList[nextDestFace][1] = faceList[nextSrcFace][1];
                faceList[nextDestFace][2] = faceList[nextSrcFace][2];
                faceList[nextDestFace][3] = faceList[nextSrcFace][3];
                if ( _numLightMaps ) 
                    _lightMapList[nextDestFace] = _lightMapList[nextSrcFace];
                nextDestFace++;
            }
            nextSrcFace++;
        }
    }
    *numFaces = facesLeft;
    return;    
}

#undef BYTE_PACK

/*-------------------------------------------------------------------
  Function: atrTriSetCalcNormals
  Date: 4/8/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Instruct build code to calculate vertex/surface normals at end
    of build process.  THIS MUST BE CALLED BETWEEN Begin/End PAIRS
    TO BE EFFECTIVE.
  Arguments:
    none
  Return:
    none
  -------------------------------------------------------------------*/
void atrTriSetCalcNormals( void ) {
    _calcNormals = FXTRUE;
    return;
}

/*-------------------------------------------------------------------
  Function: atrTriSetEnd
  Date: 3/22/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Complete definition of a triset    
  Arguments:
    t - tri set
  Return:
    none
  -------------------------------------------------------------------*/
void atrTriSetEnd( AtrTriSet *t ) {
    _AtrTriSetNode *last = 0;
    _AtrTriSetNode *n = 0;

    if ( !_inTriSet ) 

        atuError( FXTRUE, "atrTriSetEnd(): Called without matching "
                  "atrTriSetBegin().\n" );

    if ( _thisTri != 0 ) 
        atuError( FXTRUE, "atrTriSetEnd(): Called with an incomplete "
                  "triangle definition.\n" );

    if ( _numLightMaps && ( _numLightMaps != _numFaces ) ) 
        atuError( FXTRUE, "atrTriSetEnd(): A tri set with light maps "
                          "may not be completed until the number\nof light "
                          "maps matches the number of faces defined.\n" );

    if ( !_numFaces ) {
        goto cleanup;
        return;
    }

    _vertexFlagList = atuMemCalloc( sizeof( FxU32 ), _numVertices );

    /* Calculate Normals If Necessary */
    if ( _calcNormals ){
        AtmVector3 *surfaceNormals;
        FxU32 index, face, vertex;
        surfaceNormals = atuMemCalloc( sizeof( AtmVector3 ), _numFaces );

        for ( index = 0; index < _numFaces; index++ ) {
            AtmVector3 edge1, edge2, v0, v1, v2;
            tri *t = _faceList + index;
            v0[0] = _vertexList[(*t)[0]].x;
            v0[1] = _vertexList[(*t)[0]].y;
            v0[2] = _vertexList[(*t)[0]].z;
            v1[0] = _vertexList[(*t)[1]].x;
            v1[1] = _vertexList[(*t)[1]].y;
            v1[2] = _vertexList[(*t)[1]].z;
            v2[0] = _vertexList[(*t)[2]].x;
            v2[1] = _vertexList[(*t)[2]].y;
            v2[2] = _vertexList[(*t)[2]].z;
            atmVector3Sub( edge1, v1, v0 );
            atmVector3Sub( edge2, v2, v1 );
            atmVector3Cross( surfaceNormals[index], edge1, edge2 );
            atmVector3Normalize( surfaceNormals[index], surfaceNormals[index] ); 
        }
        
        /* Clear All Vertex Normals */
        for( index = 0; index < _numVertices; index++ ) {
            _vertexList[index].i = 0.0f;
            _vertexList[index].j = 0.0f;
            _vertexList[index].k = 0.0f;
        }

        /* Average all participating surface normals */
        for( vertex = 0; vertex < _numVertices; vertex++ ) {
            for( face = 0; face < _numFaces; face++ ) {
                if ( _faceList[face][0] == vertex )
                    atmVector3Add( (float*)&_vertexList[vertex].i,
                                   (float*)&_vertexList[vertex].i,
                                   surfaceNormals[face] );
                if ( _faceList[face][1] == vertex )
                    atmVector3Add( (float*)&_vertexList[vertex].i,
                                   (float*)&_vertexList[vertex].i,
                                   surfaceNormals[face] );
                if ( _faceList[face][2] == vertex )
                    atmVector3Add( (float*)&_vertexList[vertex].i,
                                   (float*)&_vertexList[vertex].i,
                                   surfaceNormals[face] );
            }
            atmVector3Normalize( (float*)&_vertexList[vertex].i,
                                 (float*)&_vertexList[vertex].i );
        }
                
        atuMemFree( surfaceNormals );
    }

    while( _numFaces ) {
        n = atuMemCalloc( sizeof( _AtrTriSetNode ), 1 );
        if ( last == 0 ) t->nodes = last = n; 
        else last->next = n, last = last->next;
        _extractChunk( n, 
                       _vertexList, 
                       _vertexFlagList,
                       &_numFaces, 
                       _faceList );
    }

cleanup:
    atuMemFree( _faceList );
    atuMemFree( _vertexList );
    atuMemFree( _vertexFlagList );
    atuMemFree( _lightMapList );
    last->next = 0;
    _inTriSet = FXFALSE;
    return;
}


