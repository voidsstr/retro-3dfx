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
** $Date: 10/11/00 7:34:03 PM$ 
**
*/

#include "atrender.h"
#include <string.h>

/*
**-----------------------------------------------------------------
** File Scope Data
**-----------------------------------------------------------------
*/

static const char  _dataType[]     = "atrXform";
static const FxU32 _binaryRevision = 0;

/*-------------------------------------------------------------------
  Function: atrCanvasAllocate
  Date: 3/20/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Allocate a new array of canvases.
  Arguments:
    num - number of canvases to allocate
  Return:
    pointer to new canvases
  -------------------------------------------------------------------*/
AtrCanvas *atrCanvasAllocate( FxU32 num ) {
    FxU32 index;
    AtrCanvas *c = atuMemCalloc( sizeof( AtrCanvas ), num );
    for( index = 0; index < num; index++ )
        atrCanvasDefault( c+index );
    return c;
}

/*-------------------------------------------------------------------
  Function: atrCanvasDeallocate
  Date: 3/20/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Deallocate an array of canvases
  Arguments:
    c - array of canvases
  Return:
    none
  -------------------------------------------------------------------*/
void atrCanvasDeallocate( AtrCanvas *c ) {
    atuMemFree( c );
    return;
}

/*-------------------------------------------------------------------
  Function: AtrCanvasDefault
  Date: 4/16
  Implementor(s): jdt
  Library: AT Render
  Description:
    Initialize data structure to default values

    For Canvas
        Data Structure Initialized size of 640x480 screen
        minX = 0
        minY = 0
        maxX = 640
        maxY = 800
  Arguments:
    t - Canvas to initialize
  Return:
    none
  -------------------------------------------------------------------*/
void atrCanvasDefault( AtrCanvas *c ) {
#ifdef AT_DEBUGGING
    if ( !c )
        atuError( FXTRUE, "atrCanvasDefault(): Invalid parameter.\n" );
#endif
    c->xMin = 0;
    c->yMin = 0;
    c->xMax = 640;
    c->yMax = 480;
    return;
}


/*-------------------------------------------------------------------
  Function: atrCanvasStore
  Date: 3/20/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Store a canvas to an output stream
  Arguments:
    stream - output stream
    c - canvas
  Return:
    FXTRUE - success
    FXFALSE - failure
  -------------------------------------------------------------------*/
FxBool atrCanvasStore( AtrCanvas *c, FILE *stream ) {
    FxU32 count;
#ifdef AT_DEBUGGING
    if ( !c || !stream ) 
        atuError( FXTRUE, "atrCanvasStore(): Invalid parameter.\n" );
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

    count = fwrite( c,
                    sizeof( AtrCanvas ),
                    1,
                    stream );
    if ( count != 1 ) return FXFALSE;

    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: atrCanvasLoad
  Date: 3/20/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Load a canvas from an input stream
  Arguments:
    stream - input stream
    c - memory into which to load canvas
  Return:
    FXTRUE - success
    FXFALSE - failure
  -------------------------------------------------------------------*/
FxBool atrCanvasLoad( AtrCanvas *c, FILE *stream ) {
    FxU32 rev;
    char  type[64];
    FxU32 count;

#ifdef AT_DEBUGGING
    if ( !c || !stream ) 
        atuError( FXTRUE, "atrCanvasLoad(): Invalid parameter.\n" );
#endif    

    count = fread( &rev, sizeof( _binaryRevision ), 1, stream );
    if ( count != 1 ) return FXFALSE;
    if ( rev != _binaryRevision ) return FXFALSE;
    count = fread( type, sizeof( _dataType ), 1, stream );
    if ( count != 1 ) return FXFALSE;
    if ( strcmp( type, _dataType ) ) return FXFALSE;
    count = fread( c, sizeof( AtrCanvas ), 1, stream );
    if ( count != 1 ) return FXFALSE;

    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: atrCanvasPrint
  Date: 3/20/96
  Implementor(s): jdt
  Library: AT Render
  Description: 
    Print a formatted description of a canvas
  Arguments:
    indent - number of spaces to indent each line
    stream - ascii output stream
    c - canvas
  Return:
    none
  -------------------------------------------------------------------*/
void atrCanvasPrint( AtrCanvas *c,
                     FILE      *stream, 
                    FxU32      indent ) {
    fprintf( stream, "%*sAtrCanvas: xMin:%d yMin:%d xMax:%d yMin:%d\n",
             indent, " ",
             c->xMin, c->yMin, 
             c->xMax, c->yMax );
    return;
}

/*-------------------------------------------------------------------
  Function: atrCanvasAssign
  Date: 3/20/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Assign the properties of one canvas to another.
  Arguments:
    dest - destination canvas
    src - src canvas
  Return:
    none
  -------------------------------------------------------------------*/
void atrCanvasAssign( AtrCanvas *dest, AtrCanvas *src ) {
    *dest = *src;
    return;
}


/*-------------------------------------------------------------------
  Function: atrCanvasClone
  Date: 3/20/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Allocate a new canvas and initialize with the supplied data
  Arguments:
    src - canvas to clone
  Return:
    cloned canvas
  -------------------------------------------------------------------*/
AtrCanvas *atrCanvasClone( AtrCanvas *src ) {
    AtrCanvas *c;
    c = atrCanvasAllocate( 1 );
    *c = *src;
    return c;
}


