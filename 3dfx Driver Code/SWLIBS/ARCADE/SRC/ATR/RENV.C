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
** $Date: 10/11/00 7:34:05 PM$ 
**
*/

#include "atrender.h"
#include <string.h>
#include <glide.h>

/*
**-----------------------------------------------------------------
** File Scope Data
**-----------------------------------------------------------------
*/

static const char  _dataType[]     = "atrEnv";
static const FxU32 _binaryRevision = 0;


/*-------------------------------------------------------------------
  Function: atrEnvAllocate
  Date: 3/19/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Allocate and initialize a new environment
  Arguments:
    num - number of enviroments to allocate and assign to the pointer
  Return:
    new array of AtrEnvs
  -------------------------------------------------------------------*/
AtrEnv *atrEnvAllocate( FxU32 num ) {
    AtrEnv *e;
    FxU32   index;

#ifdef AT_DEBUGGING
    if ( num == 0 ) 
        atuError( FXTRUE, "atrEnvAllocate(): Invalid parameter.\n" );
#endif

    e = ( AtrEnv * ) atuMemCalloc( sizeof( AtrEnv ), num );
    for( index = 0; index < num; index++ )
        atrEnvDefault( e+index );
    return e;
}

/*-------------------------------------------------------------------
  Function: atrEnvDeallocate
  Date: 3/19/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Safely deallocate arrays of atrEnvs.
  Arguments:
    e - pointer to enviroments
  Return:
    none
  -------------------------------------------------------------------*/
void atrEnvDeallocate( AtrEnv e[] ) {

#ifdef AT_DEBUGGING
    if ( !e )
        atuError( FXTRUE, "atrEnvDeallocate(): Invalid parameter.\n" );
#endif

    atuMemFree( e );
    return;
}

/*-------------------------------------------------------------------
  Function: AtrEnvDefault
  Date: 4/16
  Implementor(s): jdt
  Library: AT Render
  Description:
    Initialize data structure to default values

    For Env:
        ATR_FOG_DISABLE + ATR_HSR_WBUFFER
        White fog color
        Exp^2 fog table
  Arguments:
    e - Env to initialize
  Return:
    none
  -------------------------------------------------------------------*/
void atrEnvDefault( AtrEnv *e ) {
#ifdef AT_DEBUGGING
    if ( !e )
        atuError( FXTRUE, "atrEnvDefault(): Invalid parameter.\n" );
#endif
    e->flags = ATR_FOG_DISABLE | ATR_FOG_TABLE_UPDATE | ATR_HSR_WBUFFER;
    e->fogColor.r = 1.0f;
    e->fogColor.g = 1.0f;
    e->fogColor.b = 1.0f;
    e->fogFunc    = ATR_FOGFUNC_EXP2;
    e->fogDensity = 0.02f;
    e->fogNearW   = 0.0f;
    e->fogFarW    = 0.0f;
    return;
}

/*-------------------------------------------------------------------
  Function: atrEnvStore
  Date: 3/19/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Store an env out to a file stream.
  Arguments:
    stream - file stream
    e - env to store
  Return:
    FXTRUE - success
    FXFALSE - failure
  -------------------------------------------------------------------*/
FxBool atrEnvStore( AtrEnv *e, FILE *stream ) {
    FxU32 count;
#ifdef AT_DEBUGGING
    if ( !e || !stream ) 
        atuError( FXTRUE, "atrEnvStore(): Invalid parameter.\n" );
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

    count = fwrite( e,
                    sizeof( AtrEnv ),
                    1,
                    stream );
    if ( count != 1 ) return FXFALSE;

    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: atrEnvLoad
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
FxBool atrEnvLoad( AtrEnv *e, FILE *stream ) {
    FxU32 rev;
    char  type[64];
    FxU32 count;

#ifdef AT_DEBUGGING
    if ( !e || !stream ) 
        atuError( FXTRUE, "atrEnvLoad(): Invalid parameter.\n" );
#endif    

    count = fread( &rev, sizeof( _binaryRevision ), 1, stream );
    if ( count != 1 ) return FXFALSE;
    if ( rev != _binaryRevision ) return FXFALSE;
    count = fread( type, sizeof( _dataType ), 1, stream );
    if ( count != 1 ) return FXFALSE;
    if ( strcmp( type, _dataType ) ) return FXFALSE;
    count = fread( e, sizeof( AtrEnv ), 1, stream );
    if ( count != 1 ) return FXFALSE;

    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: atrEnvAssign
  Date: 3/19/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Assign one env to another.
  Arguments:
    src - source env
    dest - destination env
  Return:
    none
  -------------------------------------------------------------------*/
void atrEnvAssign( AtrEnv *dest, AtrEnv *src ) {
#ifdef AT_DEBUGGING
    if ( !dest || !src )
        atuError( FXTRUE, "atrEnvAssign(): Invalid paramter.\n" );
#endif
    *dest = *src;
    return;
}

/*-------------------------------------------------------------------
  Function: atrEnvClone
  Date: 3/19/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Allocate a new env and initialize it from the provided
    env
  Arguments:
    src - env to be cloned
  Return:
    clone of src
  -------------------------------------------------------------------*/
AtrEnv *atrEnvClone( AtrEnv *src ) {
    AtrEnv *tmp;

#ifdef AT_DEBUGGING
    if ( !src )
        atuError( FXTRUE, "atrEnvClone(): Invalid paramter.\n" );
#endif

    tmp = atrEnvAllocate( 1 );
    *tmp = *src;
    return tmp;
}

/*-------------------------------------------------------------------
  Function: atrEnvPrint
  Date: 3/19/96
  Implementor(s): jdt
  Library: AT Render
  Description: 
    Print a formatted description of an environment 
  Arguments:
    indent - spaces to append to all output
    stream - output stream
    e - env to print
  Return:
    none
  -------------------------------------------------------------------*/
void atrEnvPrint( AtrEnv *e,
                  FILE *stream, 
                  FxU32 indent ) {
    
#ifdef AT_DEBUGGING
    if ( !e || !stream )
        atuError( FXTRUE, "atrEnvPrint(): Invalid paramter.\n" );
#endif

    fprintf( stream, "%*sEnv: flags 0x%x\n",
             indent, " ", e->flags );
    return;
}

/*-------------------------------------------------------------------
  Function: atrEnvFogTable
  Date: 6/10
  Implementor(s): jdt
  Library: AT Render
  Description:
    Fill in the fog table in the env structure
    ( see glide docs for more info )
  Arguments:
    e       - pointer to env structure
    func    - type of function to use to generate the table
        ATR_FOGTABLE_LINEAR  - f(w) = (w - nearW)/(farW-nearW)
        ATR_FOGTABLE_EXP     - f(w) = scale * ( 1 - exp( -(w * density) ) ) 
        ATR_FOGTABLE_EXP2    - f(w) = scale * ( 1 - exp( -(w * density)^2 )
    density - density factor
    nearW - nearestW ( least fogged )
    farW - farthestW ( most fogged )
  Return:
  -------------------------------------------------------------------*/
void atrEnvFogTable( AtrEnv            *e,
                     AtrEnvFogFunction func,
                     float             density,
                     float             nearW,
                     float             farW ) {
#ifdef AT_DEBUGGING
    if ( !e )
        atuError( FXTRUE, "atrEnvFogTable: bad func.\n" );

        switch( func ) {
        case ATR_FOGFUNC_LINEAR:
        case ATR_FOGFUNC_EXP:
        case ATR_FOGFUNC_EXP2:
            break;
        default:
            atuError( FXTRUE, "atrEnvFogTable: bad func.\n" );
            break;
        }
#endif

    e->fogFunc = func;
    e->fogDensity = density;
    e->fogNearW = nearW;
    e->fogFarW = farW;
    e->flags |= ATR_FOG_TABLE_UPDATE;
}


