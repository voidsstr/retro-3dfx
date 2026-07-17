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
** $Date: 10/11/00 7:34:22 PM$ 
**
*/

#include "atrender.h"
#include <string.h>

/*
**-----------------------------------------------------------------
** File Scope Data
**-----------------------------------------------------------------
*/

static const char  _dataType[]     = "atrVertex";
static const FxU32 _binaryRevision = 0;

/*-------------------------------------------------------------------
  Function: atrVertexAllocate
  Date: 3/18/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Allocate a given number of AtrVertices.  AtrVertices are always
    allocated with 32-byte alignment.
  Arguments:
    num - number of vertices to allocate
  Return:
    none
  -------------------------------------------------------------------*/
AtrVertex *atrVertexAllocate( FxU32 num ) {
#if AT_DEBUGGING    
    if ( !num ) 
        atuError( FXTRUE, "atrVertexAllocate(): Bad parameter.\n" );
#endif
    return atuMemAlignedCalloc( sizeof( AtrVertex ) * num, 1, 32 );
}

/*-------------------------------------------------------------------
  Function: atrVertexDeallocate
  Date: 3/18/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Safely deallocate one or more atrVertices.
  Arguments:
    v - pointer to vertex or vertices
  Return:
    none
  -------------------------------------------------------------------*/
void atrVertexDeallocate( AtrVertex v[] ) {
#ifdef AT_DEBUGGING
    if ( !v )
        atuError( FXTRUE, "atrVertexDeallocate(): Invalid parameter.\n" );
#endif
    atuMemFree( v );
    return;
}


/*-------------------------------------------------------------------
  Function: AtrVertexDefault
  Date: 4/16
  Implementor(s): jdt
  Library: AT Render
  Description:
    Initialize data structure to default values

    For Vertex:
        Data structure cleared to Zero
  Arguments:
    v - vertex to initialize
  Return:
    none
  -------------------------------------------------------------------*/
void atrVertexDefault( AtrVertex *v ) {
#ifdef AT_DEBUGGING
    if ( !v )
        atuError( FXTRUE, "atrVertexDefault(): Invalid parameter.\n" );
#endif
    v->x  = 0.0f;
    v->y  = 0.0f;
    v->z  = 0.0f;
    v->i  = 0.0f;
    v->j  = 0.0f;
    v->k  = 0.0f;
    v->s0 = 0.0f;
    v->t0 = 0.0f;
    v->r  = 0.0f;
    v->g  = 0.0f;
    v->b  = 0.0f;
    v->a  = 0.0f;
    v->s1 = 0.0f;
    v->t1 = 0.0f;
    v->s2 = 0.0f;
    v->t2 = 0.0f;
    return;
}



/*-------------------------------------------------------------------
  Function: atrVertexStore
  Date: 3/18/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Store a vertex out to a file stream.
  Arguments:
    stream - output stream
    v - vertex to be stored
  Return:
    FXTRUE  - store successful 
    FXFALSE - store unsuccesful
  -------------------------------------------------------------------*/
FxBool atrVertexStore( AtrVertex *v, FILE *stream ) {
    FxU32 count;
#ifdef AT_DEBUGGING
    if ( !stream || !v )
        atuError( FXTRUE, "atrVertexStore(): Invalid paramter.\n" );
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
    count = fwrite( v,
                    sizeof( AtrVertex ), 
                    1,
                    stream );
    if ( count != 1 ) return FXFALSE;
    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: atrVertexLoad
  Date: 3/18/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Load a vertex from the given input stream, validate the data,
    and store it to a provided memory area.
  Arguments:
    stream - input stream
    v - pointer to an empty atr vertex.
  Return:
    FXTRUE  - load successful
    FXFALSE - load unsuccessful ( either stream empty or wrong 
              type/revision of data )
  -------------------------------------------------------------------*/
FxBool atrVertexLoad( AtrVertex *v, FILE *stream ) {
    FxU32 rev;
    char  type[64];
    FxU32 count;

#ifdef AT_DEBUGGING
    if ( !v || !stream ) 
        atuError( FXTRUE, "atrVertexLoad(): Invalid parameter.\n" );
#endif    
    count = fread( &rev, sizeof( _binaryRevision ), 1, stream );
    if ( count != 1 ) return FXFALSE;
    if ( rev != _binaryRevision ) return FXFALSE;
    count = fread( type, sizeof( _dataType ), 1, stream );
    if ( count != 1 ) return FXFALSE;
    if ( strcmp( type, _dataType ) ) return FXFALSE;
    count = fread( v, sizeof( AtrVertex ), 1, stream );
    if ( count != 1 ) return FXFALSE;
    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: atrVertexAssign
  Date: 3/18/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Assign one vertex to another.
  Arguments:
    src - source vertex
    dest - destination vertex
  Return:
    none
  -------------------------------------------------------------------*/
void atrVertexAssign( AtrVertex *dest, AtrVertex *src ) {
#ifdef AT_DEBUGGING
    if ( !dest || !src ) 
        atuError( FXTRUE, "atrVertexAssign(): Invalid parameter.\n" );
#endif
    *dest = *src;
    return;
}

/*-------------------------------------------------------------------
  Function: atrVertexClone
  Date: 3/18/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Allocate a new vertex and initialize with data from the provided
    vertex.
  Arguments:
    src - vertex to be cloned
  Return:
    pointer to clone
  -------------------------------------------------------------------*/
AtrVertex *atrVertexClone( AtrVertex *src ) {
    AtrVertex *tmp;
#ifdef AT_DEBUGGING
    if ( !src ) 
        atuError( FXTRUE, "atrVertexClone(): Invalid parameter.\n" );
#endif
    tmp = atuMemAlignedCalloc( sizeof( AtrVertex ), 1, 32 );
    *tmp = *src;
    return tmp;
}

/*-------------------------------------------------------------------
  Function: atrVertexPrint
  Date: 3/18/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Formatted output of a vertex
  Arguments:
    indent - indent level to aid legibility
    stream - output stream
    v - vertex to display
  Return:
    none
  -------------------------------------------------------------------*/
void atrVertexPrint( AtrVertex *v, FILE *stream, FxU32 indent ) {
    fprintf( stream, 
             "%*sVertex: pos: %.3f %.3f %.3f norm: %.3f %.3f %.3f\n"
             "%*s        color: %.3f %.3f %.3f tex0: %.3f %.3f\n"
             "%*s        tex1: %.3f %.3f tex2: %.3f %.3f\n",
             indent, " ", v->x, v->y, v->z, v->i, v->j, v->k,
             indent, " ", v->r, v->g, v->b, v->s0, v->t0,
             indent, " ", v->s1, v->t1, v->s2, v->t2 );
    return;
}


