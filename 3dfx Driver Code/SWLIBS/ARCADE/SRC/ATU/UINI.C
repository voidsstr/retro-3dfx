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
** $Date: 10/11/00 7:34:54 PM$ 
**
*/


#include "atutil.h"
#include <fxos.h>
#include <stdio.h>
#include <string.h>

#define PATH_LEN 512
#define MAX_TAG_LEN    80
#define MAX_STRING_LEN 1024
#define TMP_FILE       "tmp.tmp"

#if !macintosh
static char path[PATH_LEN] = ".";
#else
static char path[PATH_LEN] = ":";
#endif

static void _printTyped( FILE *stream, const char *tag, const void *data, AtuIniType type ) {
    switch( type ) {
      case ATU_INI_STRING:
        fprintf( stream, "%s\n%s\n", tag, data );
        break;
      case ATU_INI_FLOAT:
        fprintf( stream, "%s\n%f\n", tag, *(float*)data );
        break;
      case ATU_INI_INT:
        fprintf( stream, "%s\n%d\n", tag, *(FxU32*)data );
        break;
    }
}

/*-------------------------------------------------------------------
  Function: atuIniPath
  Date: 7/10
  Implementor(s): jdt
  Library: AT Util
  Description:
  Set the search path for the atuIni api
  Arguments:
  path - path to set
  Return:
  non
  -------------------------------------------------------------------*/
void atuIniPath( const char *path_ ) {
    if ( strlen( path ) >= PATH_LEN )
      atuError( FXFALSE, "atuIniPath: maximum path length (%d) exceeded.\n", PATH_LEN );

    strcpy( path, path_ );
    return;
}


/*-------------------------------------------------------------------
  Function: atuIniGet
  Date: 7/10
  Implementor(s): jdt
  Library: AT Utility
  Description: 
  Extract data from an ini file

  Ini files are of the format:

  TAG  <\n>
  DATA <\n>
  <space> 
  TAG  <\n>
  .....

  Data must come on the line immediately following a tag, and the data
  is considered to continue until the next newline

  Arguments:
  filename - name of ini file
  tag      - string identifying data
  type     - type of data, one of 
             ATU_INI_STRING - null terminated string
             ATU_INI_INT    - one integer
             ATU_INI_FLOAT  - one float
  data     - void pointer pointer to storage for data, buffer will be filled up
             to 'size' or end of data
  size     - size of buffer passed as 'data'
  Return:
  FXTRUE  - successs
  FXFALSE - failure - file or tag does not exist
  -------------------------------------------------------------------*/
FxBool atuIniGet(const char *filename, const char *tag, 
                 AtuIniType type, void *data, FxU32 size ) {
    FILE *infile;
    char tmpTag[MAX_TAG_LEN];
    char tmpValue[MAX_STRING_LEN];
    FxBool found = FXFALSE;

    infile = fxFopenPath( filename, "r", path, 0 );
    if ( !infile ) return FXFALSE;

    while( fscanf( infile, "%[^\n]\n%[^\n]\n", tmpTag, tmpValue ) == 2 ) {
        if( strcmp( tmpTag, tag ) == 0 ) {
            switch( type ) {
              case ATU_INI_STRING:
                strncpy( data, tmpValue, size );
                break;
              case ATU_INI_INT:
                *(FxU32*)data = atoi( tmpValue );
                break;
              case ATU_INI_FLOAT:
                *(float*)data = (float)atof( tmpValue );
                break;
            }
            found = FXTRUE;
        }
    }
    fclose( infile );
    return found;
}

/*-------------------------------------------------------------------
  Function: atuIniSet
  Date: 7/10
  Implementor(s): jdt
  Library: AT Utility
  Description: 
  Set data in an ini file

  Ini files are of the format:

  TAG  <\n>
  DATA <\n>
  <space> 
  TAG  <\n>
  .....

  Data comes on the line immediately following a tag, and the data
  is considered to continue until the next newline

  Arguments:
  filename - name of ini file
  tag      - string identifying data
  type     - type of data, one of 
             ATU_INI_STRING - null terminated string
             ATU_INI_INT    - single integer
             ATU_INI_FLOAT  - single float
  data     - void pointer pointer to data
  Return:
  FXTRUE  - successs
  FXFALSE - failure - file could not be created
  -------------------------------------------------------------------*/
FxBool atuIniSet(const char *filename, const char *tag, 
                 AtuIniType type, void *data  ) {
    FILE *infile;
    FILE *outfile;
    char tmpTag[MAX_TAG_LEN];
    char tmpValue[MAX_STRING_LEN];
    char *pp;
    char fullFilePath[PATH_LEN];

    infile = fxFopenPath( filename, "r", path, &pp );
    outfile = fopen( TMP_FILE, "w" );

    if ( type > 2 ) 
        atuError( FXTRUE, "atuIniSet, Bad type.\n" );

    if ( !outfile ) {
        return FXFALSE;
    }

    if ( strlen( tag ) >= MAX_TAG_LEN ) 
      atuError( FXTRUE, "atuIniSet, tag length exceeds max, %d\n", MAX_TAG_LEN );

    if ( type == ATU_INI_STRING && strlen( data ) > MAX_STRING_LEN )
      atuError( FXTRUE, "atuIniSet, string length exceeds max, %d\n", MAX_STRING_LEN );

    if ( !infile ) {
        strcpy( fullFilePath, ATU_CURRENT_DIRECTORY_STRING );
        strcat( fullFilePath, filename );

        _printTyped( outfile, tag, data, type );
        fclose( outfile );
    } else {
        FxBool found = FXFALSE;

        if ( pp ) {
            strcpy( fullFilePath, pp );
            strcat( fullFilePath, ATU_FILE_SEPARATOR_STRING );
            strcat( fullFilePath, filename );
        } else {
            strcpy( fullFilePath, ATU_CURRENT_DIRECTORY_STRING );
            strcat( fullFilePath, filename );
        }

        while( fscanf( infile, "%[^\n]\n%[^\n]\n", tmpTag, tmpValue ) == 2 ) {
            if( strcmp( tmpTag, tag ) ) {
                fprintf( outfile, "%s\n%s\n", tmpTag, tmpValue );
            } else {
                _printTyped( outfile, tag, data, type );
                found = FXTRUE;
            }
        }
        if ( !found ) {
            _printTyped( outfile, tag, data, type );
        }
        fclose( infile );
        fclose( outfile );
        #if !(macintosh)
        if ( unlink( fullFilePath ) )
            atuError( FXTRUE, "atuIniSet: Error unlinking %s.\n", fullFilePath );
        #endif
    }

    if ( rename( TMP_FILE, fullFilePath ) ) 
        atuError( FXTRUE,  "atuIniSet: Error renaming %s to %s.\n", TMP_FILE, fullFilePath );

    return FXTRUE;
}
