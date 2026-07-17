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

static const char  _dataType[]     = "atrCamera";
static const FxU32 _binaryRevision = 0;

/*-------------------------------------------------------------------
  Function: atrCameraAllocate
  Date: 3/20/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Allocate cameras
  Arguments:
    num - number of cameras to allocate
  Return:
    pointer to array of cameras
  -------------------------------------------------------------------*/
AtrCamera *atrCameraAllocate( FxU32 num ) {
    AtrCamera *c;
    FxU32 index;
    c = atuMemCalloc( num, sizeof( AtrCamera ) );
    for ( index = 0; index < num; index++ ) {
        atrCameraDefault( c+index );
    }
    return c;
}

/*-------------------------------------------------------------------
  Function: atrCameraDeallocate
  Date: 3/20/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Deallocate an array of cameras.
  Arguments:
    c - camera array
  Return:
    none
  -------------------------------------------------------------------*/
void atrCameraDeallocate( AtrCamera *c ) {
    atuMemFree( c );
    return;
}

/*-------------------------------------------------------------------
  Function: AtrCameraDefault
  Date: 4/16
  Implementor(s): jdt
  Library: AT Render
  Description:
    Initialize data structure to default values

    For Camera:
      AR       - 4/3
      fov      - 60.0 Degrees
      nearClip - 1.0f
      farClip  - 65K
      xOffset  - 0
      yOffset  - 0
  Arguments:
    c - camera to initialize
  Return:
    none
  -------------------------------------------------------------------*/
void atrCameraDefault( AtrCamera *c ) {
#ifdef AT_DEBUGGING
    if ( !c )
        atuError( FXTRUE, "atrCameraDefault(): Invalid parameter.\n" );
#endif
    c->aspectRatio      = 1.3333333f;
    c->fovRadians       = ( 1.0f / 3.0f ) * ATM_PI;
    c->nearClip         = 1.0f;
    c->farClip          = 65535.0f;
    c->xOffset          = 0.0f;
    c->yOffset          = 0.0f;
    atrXformSetIdentity( &c->lcsToWCS );
    atrXformSetIdentity( &c->wcsToCCS );
    return;
}



/*-------------------------------------------------------------------
  Function: atrCameraStore
  Date: 3/20/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Store a camera to a file stream.
  Arguments:
    c - camera
    stream - ouput stream
  Return:
    FXTRUE - success
    FXFALSE - failure
  -------------------------------------------------------------------*/
FxBool atrCameraStore( AtrCamera *c, FILE *stream ) {
    FxU32 count;
#ifdef AT_DEBUGGING
    if ( !c || !stream ) 
        atuError( FXTRUE, "atrCameraSTore(): Invalid parameter.\n" );
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

    count = fwrite( c, sizeof( AtrCamera ), 1, stream );
    if ( count != 1 ) return FXFALSE;

    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: atrCameraLoad
  Date: 3/20/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Load a camera from a file stream
  Arguments:
    c - camera
    stream - input stream
  Return:
    FXTRUE - success
    FXFALSE - failure
  -------------------------------------------------------------------*/
FxBool atrCameraLoad( AtrCamera *c, FILE *stream ) {
    FxU32 rev;
    char  type[64];
    FxU32 count;

#ifdef AT_DEBUGGING
    if ( !c || !stream ) 
        atuError( FXTRUE, "atrCameraLoad(): Invalid parameter.\n" );
#endif    

    count = fread( &rev, sizeof( _binaryRevision ), 1, stream );
    if ( count != 1 ) return FXFALSE;
    if ( rev != _binaryRevision ) return FXFALSE;
    count = fread( type, sizeof( _dataType ), 1, stream );
    if ( count != 1 ) return FXFALSE;
    if ( strcmp( type, _dataType ) ) return FXFALSE;
    count = fread( c, sizeof( AtrCamera ), 1, stream );
    if ( count != 1 ) return FXFALSE;
    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: atrCameraPrint
  Date: 3/20/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Print a formatted description of a camera
  Arguments:
    indent - number of spaces to indent each line
    stream - ascii output stream
    c - camera
  Return:
    none
  -------------------------------------------------------------------*/
void atrCameraPrint( AtrCamera *c,
                     FILE      *stream, 
                     FxU32     indent ) {
#ifdef AT_DEBUGGING
    if ( !c || !stream ) 
        atuError( FXTRUE, "atrCameraPrint(): Invalid parameter.\n" );
#endif    

	fprintf( stream, "%*sCamera: FOV: %f AR: %f NC: %f FC: %f\n",
             indent, " ",
	         c->fovRadians,
             c->aspectRatio,
             c->nearClip,
             c->farClip );
    return;
}

/*-------------------------------------------------------------------
  Function: atrCameraAssign
  Date: 3/20/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Assign one camera to another.
  Arguments:
    src - source camera
    dest - destination camera
  Return:
    none
  -------------------------------------------------------------------*/
void atrCameraAssign( AtrCamera *dest, AtrCamera *src ) {
#ifdef AT_DEBUGGING
    if ( !dest || !src ) 
        atuError( FXTRUE, "atrCameraAssign(): Invalid parameter.\n" );
#endif    

    *dest = *src;
    return;
}

/*-------------------------------------------------------------------
  Function: atrCameraClone
  Date: 3/20/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Allocate a new camera and initialize from an existing camera.
  Arguments:
    src - camera to be cloned
  Return:
    cloned camera
  -------------------------------------------------------------------*/
AtrCamera *atrCameraClone( AtrCamera *src ) {
    AtrCamera *c;

#ifdef AT_DEBUGGING
    if ( !src )
        atuError( FXTRUE, "atrCameraClone(): Invalid parameter.\n" );
#endif    

    c = atuMemCalloc( sizeof( AtrCamera ), 1 );
    *c = *src;
    return c;
}


