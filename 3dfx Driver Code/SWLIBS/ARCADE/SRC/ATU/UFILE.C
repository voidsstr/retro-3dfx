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
** $Date: 10/11/00 7:34:56 PM$ 
**
*/

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <atutil.h>
#include <glidesys.h>

#if macintosh
#include <types.h>
#endif

static char *_atu_load_path = NULL;
static AtuLoadHook _atuLoadHook;

/*-------------------------------------------------------------------
  Function: atuSetLoadPath
  Date: 10/13/96
  Implementor(s): mlwp
  Library: AT Utility
  Description: 
    Set ATB's load path
  Arguments:
    load_path  -   new load path
  Return:
    nothing
  -------------------------------------------------------------------*/

void atuSetLoadPath(const char *load_path) {
    int len ;

    if ( load_path ) {

        len = strlen(load_path);

        if ( _atu_load_path == NULL )
            _atu_load_path = malloc(len+1);
        else _atu_load_path = realloc(_atu_load_path, len+1);
    
        if ( _atu_load_path == NULL ) {
            atuError( FXTRUE, "atuLoadPath: No memory\n" );
            return ;
        }

        strcpy(_atu_load_path, load_path);
    } else _atu_load_path = NULL;
}

AtuLoadHook atuSetLoadHook(AtuLoadHook hook) {
    AtuLoadHook old = _atuLoadHook;

    _atuLoadHook = hook;

    return old;
}

/*-------------------------------------------------------------------
  Function: atu:FileLocate
  Date: 10/13/96
  Implementor(s): mlwp
  Library: AT Utility
  Description: 
    Try to open the file specified by filename, if this fails and there is
    a search path try the specified directories.
  Arguments:
    filename  -   base name of file
    full_path -   buffer for expanded file path
  Return:
    expanded file path if succesful, NULL if could not find path
  -------------------------------------------------------------------*/

char *
atuFileLocate(const char *filename, char *full_path) {
    char  tmp_path[512];
    char *partial_path;
    FILE *f;

    strcpy(full_path, filename);
    if( ( f = fopen( full_path, "r" ) ) != NULL ) {
         fclose(f);
         return full_path;
    } 

    if ( _atu_load_path != NULL ) {
        strcpy( tmp_path, _atu_load_path );
        for ( partial_path = strtok( tmp_path, ";" );
             partial_path != 0 ;
             partial_path = strtok( NULL, ";" ) ) {
             #if macintosh
             /* it is hard to know what directory fopen will start in */
             /* so we need to give it a full path, starting with app_path */
             extern char app_path[];

             strcpy(full_path, app_path);

             /* if the partial_path is not the current directory, append it */
             if(strlen(partial_path) != 1 || partial_path[0] != ':') {
                 strcat(full_path, partial_path);
             }
             #else
             strcpy( full_path, partial_path );
             #endif
             strcat( full_path, ATU_FILE_SEPARATOR_STRING );
             strcat( full_path, filename );
             if( ( f = fopen( full_path, "r" ) ) != NULL ) {
                 fclose(f);
                 return full_path;
             }
        }
    } 

    if ( _atuLoadHook != NULL ) {
        char *result;

        result = _atuLoadHook(filename, full_path);
        if ( result )
            strcpy( full_path, result );
        return result;
    } 

    return NULL;
}

/*-------------------------------------------------------------------
  Function: atuGetLoadPath
  Date: 10/13/96
  Implementor(s): mlwp
  Library: AT Utility
  Description: 
    Return ATB's current load path
  Arguments:
    none
  Return:
    ATB's current load path
  -------------------------------------------------------------------*/

char *atuGetLoadPath(void) {
    return _atu_load_path ;
}

/*-------------------------------------------------------------------
  Function: atuStringCompare
  Date: 10/13/96
  Implementor(s): mlwp
  Library: AT Utility
  Description: 
    Compare two strings but ignore case
  Arguments:
    s1  - first string to compare
    s2  - second string to compare
  Return:
    FXTRUE if strings are equal, FXFALSE otherwise
  -------------------------------------------------------------------*/

FxBool
atuStringCompare(const char *s1, const char *s2) {
    while ( tolower(*s1) == tolower(*s2) ) {
        if ( *s1 == 0 ) 
            return ( *s2 == 0 );
        if ( *s2 == 0 )
            return FXFALSE;
        s1++, s2++;
    }

    return FXFALSE;
}

/*-------------------------------------------------------------------
  Function: atuRead8
  Date: 10/25/96
  Implementor(s): mlwp
  Library: AT Utility
  Description: 
    Read array of 8 bit values
  Arguments:
    d      - pointer to buffer
    count  - number of items to transfer
    stream - io stream
  Return:
    FXTRUE if successful, FXFALSE otherwise
  -------------------------------------------------------------------*/

FxBool
atuRead8 (void *d, FxU32 count, FILE *stream) {
    return (fread (d, count, 1, stream) == 1) ? FXTRUE : FXFALSE;
}

/*-------------------------------------------------------------------
  Function: atuRead16
  Date: 10/25/96
  Implementor(s): mlwp
  Library: AT Utility
  Description: 
    Read array of 16 bit values, msb first
  Arguments:
    d      - pointer to buffer
    count  - number of items to transfer
    stream - io stream
  Return:
    FXTRUE if successful, FXFALSE otherwise
  -------------------------------------------------------------------*/

FxBool
atuRead16 (void *d, FxU32 count, FILE *stream) {
    #if GLIDE_ENDIAN == GLIDE_ENDIAN_BIG
    #if macintosh
    extern void soundCleanUp(void);

    extern Boolean gSoundAttnFlag;
    if(gSoundAttnFlag) {
        soundCleanUp();
    }
    #endif
    if(fread(d, sizeof(unsigned short), count, stream) != count) {
        return FXFALSE;
    }
    #else
    FxU16 *data;
    FxU8 byte[2];
    FxU32 i;

    for ( i = 0, data = d; i < count; i++, data++ ) {
        if (fread (byte, 2, 1, stream) != 1) return FXFALSE;
        *data = (((FxU16) byte[0]) << 8) | ((FxU16) byte[1]);
    }
    #endif

    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: atuRead32
  Date: 10/25/96
  Implementor(s): mlwp
  Library: AT Utility
  Description: 
    Read array of 32 bit values, msb first
  Arguments:
    d      - pointer to buffer
    count  - number of items to transfer
    stream - io stream
  Return:
    FXTRUE if successful, FXFALSE otherwise
  -------------------------------------------------------------------*/

FxBool
atuRead32 (void *d, FxU32 count, FILE *stream) {

   #if GLIDE_ENDIAN == GLIDE_ENDIAN_BIG
   #if macintosh
   extern void soundCleanUp(void);

   extern Boolean gSoundAttnFlag;
   if(gSoundAttnFlag) {
       soundCleanUp();
   }
   #endif
   if(fread(d, sizeof(unsigned long), count, stream) != count) {
       return FXFALSE;
   }
   #else
    FxU8 byte[4];
    FxU32 *data;
    FxU32 i;

    for ( i = 0, data = d; i < count; i++, data++ ) {
        if (fread (byte, 4, 1, stream) != 1) return FXFALSE;

        *data = (((FxU32) byte[0]) << 24) |
                (((FxU32) byte[1]) << 16) |
                (((FxU32) byte[2]) <<  8) |
                 ((FxU32) byte[3]);
    }
    #endif

    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: atuWrite8
  Date: 10/25/96
  Implementor(s): mlwp
  Library: AT Utility
  Description: 
    Write array of 8 bit values
  Arguments:
    d      - pointer to buffer
    count  - number of items to transfer
    stream - io stream
  Return:
    FXTRUE if successful, FXFALSE otherwise
  -------------------------------------------------------------------*/

FxBool
atuWrite8 (const void *d, FxU32 count, FILE *stream) {
    return (fwrite (d, count, 1, stream) == 1) ? FXTRUE :  FXFALSE;
}

/*-------------------------------------------------------------------
  Function: atuWrite16
  Date: 10/25/96
  Implementor(s): mlwp
  Library: AT Utility
  Description: 
    Write array of 16 bit values, msb first
  Arguments:
    d      - pointer to buffer
    count  - number of items to transfer
    stream - io stream
  Return:
    FXTRUE if successful, FXFALSE otherwise
  -------------------------------------------------------------------*/

FxBool
atuWrite16 (const void *d, FxU32 count, FILE *stream) {
    FxU8 byte[2];
    const FxU16 *data;
    FxU32 i;

    for ( i = 0, data = d; i < count; i++, data++ ) {
        byte[0] = (FxU8) ((*data >> 8) & 0xFF);
        byte[1] = (FxU8) ((*data     ) & 0xFF);

        if (fwrite (byte, 2, 1, stream) != 1) return FXFALSE;
    }

    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: atuWrite32
  Date: 10/25/96
  Implementor(s): mlwp
  Library: AT Utility
  Description: 
    Write array of 32 bit values, msb first
  Arguments:
    d      - pointer to buffer
    count  - number of items to transfer
    stream - io stream
  Return:
    FXTRUE if successful, FXFALSE otherwise
  -------------------------------------------------------------------*/

FxBool
atuWrite32 (const void *d, FxU32 count, FILE *stream) {
    const FxU32 *data;
    FxU8 byte[4];
    FxU32 i;

    for ( i = 0, data = d; i < count; i++, data++ ) {
        byte[0] = (FxU8) ((*data >> 24) & 0xFF);
        byte[1] = (FxU8) ((*data >> 16) & 0xFF);
        byte[2] = (FxU8) ((*data >>  8) & 0xFF);
        byte[3] = (FxU8) ((*data      ) & 0xFF);
    
        if (fwrite (byte, 4, 1, stream) != 1) return FXFALSE;
    }

    return FXTRUE;
}
