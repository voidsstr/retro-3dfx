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
** $Date: 10/11/00 7:34:30 PM$ 
**
*/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <atscenep.h>

#define ID_INC 100

static FxI32 num_pointers;
static FxI32 max_pointers;

static void **pointers;

#define INDEX_TO_HANDLE(_h) ((void *)(_h+1))
#define HANDLE_TO_INDEX(_h) (((FxI32)(_h))-1)

typedef struct {
    FxU32 magic ;
    FxU32 versionNumber ;
} bofHeader;

/*-------------------------------------------------------------------
  Function: atsStoreString
  Date: 6/5/96
  Implementor(s): mlwp
  Library: AT Scene Manager
  Description:
    Store a string to a file stream.
  Arguments:
    s      - string
    stream - ouput stream
  Return:
    FXTRUE - success
    FXFALSE - failure
  -------------------------------------------------------------------*/

FxBool atsStoreString(char *s, FILE *stream) {
    FxU32 len;
#ifdef AT_DEBUGGING
    if ( !stream )
        atuError( FXTRUE, "atsStoreString(): Invalid parameter.\n" );
#endif

    if ( s == NULL ) {
        len = 0;
        CHECK( atuWrite32( &len, 1, stream ));
    } else {
        len = strlen( s ) + 1 ; /* include space for NULL terminator */
        CHECK( atuWrite32( &len, 1, stream ));

        CHECK( atuWrite8( s, len, stream ));
    }

    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: atsLoadString
  Date: 6/5/96
  Implementor(s): mlwp
  Library: AT Scene Manager
  Description:
    Load a string from a file stream.
  Arguments:
    s      - string
    stream - ouput stream
  Return:
    FXTRUE  - success
    FXFALSE - failure
  -------------------------------------------------------------------*/

FxBool atsLoadString(char **s, FILE *stream) {
    FxU32 len;
    char *tmp;

#ifdef AT_DEBUGGING
    if ( !s || !stream )
        atuError( FXTRUE, "atsLoadString(): Invalid parameter.\n" );
#endif

    CHECK( atuRead32( &len, 1, stream ));

    if ( len == 0 ) {
        *s = NULL;
        return FXTRUE;
    }

    tmp = atuMemCalloc( sizeof(char), len );

    CHECK( atuRead8( tmp, len, stream ));

    *s = tmp;
    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: atsStoreType
  Date: 6/5/96
  Implementor(s): mlwp
  Library: AT Scene Manager
  Description:
    Store a type to a file stream.
  Arguments:
    type    - type
    stream  - ouput stream
  Return:
    FXTRUE - success
    FXFALSE - failure
  -------------------------------------------------------------------*/

FxBool atsStoreType(AtsType *type, FILE *stream) {
#ifdef AT_DEBUGGING
    if ( !type || !stream )
        atuError( FXTRUE, "atsStoreString(): Invalid parameter.\n" );
#endif
    
    return atsStoreString(type->name, stream);
}

/*-------------------------------------------------------------------
  Function: atsLoadType
  Date: 6/5/96
  Implementor(s): mlwp
  Library: AT Scene Manager
  Description:
    Load a type from a file stream.
  Arguments:
    type    - type
    stream  - ouput stream
  Return:
    FXTRUE - success
    FXFALSE - failure
  -------------------------------------------------------------------*/

FxBool atsLoadType(AtsType **type, FILE *stream) {
    char type_name[60];
    FxU32 len;

#ifdef AT_DEBUGGING
    if ( !type || !stream )
        atuError( FXTRUE, "atsLoadType(): Invalid parameter.\n" );
#endif

    CHECK( atuRead32( &len, 1, stream ));

    CHECK( atuRead8( type_name, len, stream ));

    *type = atsGetTypeFromName(type_name);
    
    return ( *type != NULL );
}

/*-------------------------------------------------------------------
  Function: atsPointerToID
  Date: 6/5/96
  Implementor(s): mlwp
  Library: AT Scene Manager
  Description:
    Map a pointer to an object into an id. If the object has been stored
    just return the id number, if not remember we need to save this object
    and return an id number.
  Arguments:
    p      - pointer
  Return:
    Object ID, -1 on error
  -------------------------------------------------------------------*/

static void atsExtendPointerTable() {
    if ( num_pointers >= max_pointers ) {
        max_pointers += ID_INC;

        pointers = (void **)atuMemRealloc(pointers, 
                                          max_pointers*sizeof(void *));

        if ( pointers == NULL )
            atuError(FXTRUE, "atsPointerToID: Out of memory\n");
    }
}

void *atsPointerToID(void *p) {
    FxI32 i;
    void *h;

    if ( p == NULL ) {
        atuError(FXTRUE, "atsPointerToID Null pointer, id would have been %d\n",
                 num_pointers);
        return NULL;
    }

    for ( i = 0; i < num_pointers ; i++ )
        if ( p == pointers[i] )
            return INDEX_TO_HANDLE(i);

    /* pointer not found, add it to list */

    atsExtendPointerTable();

    pointers[num_pointers] = p;

    h = INDEX_TO_HANDLE(num_pointers);
 
    num_pointers++;

    return h;
}

/*-------------------------------------------------------------------
  Function: atsObjectIDLoaded
  Date: 6/5/96
  Implementor(s): mlwp
  Library: AT Scene Manager
  Description:
    Check that we have not missed an id and save object pointer
  Arguments:
    p      - pointer
    id     - object id
  Return:
    FXTRUE - success
    FXFALSE - failure
  -------------------------------------------------------------------*/

FxBool atsObjectIDProcessed(void *p, FxI32 id) {
    FxI32 i = HANDLE_TO_INDEX(id);

    if ( i != num_pointers )
        atuError(FXTRUE, "Out of sequence incoming id = %d, num_ids = %d\n",
                 id, num_pointers);

    atsExtendPointerTable();

    pointers[num_pointers++] = p;
    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: atsIDToPointer
  Date: 6/5/96
  Implementor(s): mlwp
  Library: AT Scene Manager
  Description:
    Convert an id into a pointer
  Arguments:
    id      - the object id
  Return:
    pointer to object
    NULL on error
  -------------------------------------------------------------------*/

void *atsIDToPointer(void *id) {
    FxI32 i = HANDLE_TO_INDEX(id);

    if ( id == NULL )
        return NULL;

    if (( i < 0 ) || ( i >= max_pointers ) || ( pointers[i] == NULL ))
        atuError(FXTRUE, "Invalid id 0x%lx\n", id);

    return pointers[i];
}

/*-------------------------------------------------------------------
  Function: atsStoreToBOF
  Date: 6/5/96
  Implementor(s): mlwp
  Library: AT Scene Manager
  Description:
    Store a node (and all its children) in a file in ATB internal format
  Arguments:
    fileName - which file to store node in
    node     - node to store
  Return:
    FXTRUE - success
    FXFALSE - failure
  -------------------------------------------------------------------*/

FxBool atsStoreToBOF(const char *fileName, AtsObject *node) {
    FILE *stream;
    bofHeader h;
    FxI32 num_objs_written;
    FxI32 eof = 0;

    num_objs_written = 0;
    num_pointers = 0;
    max_pointers = 0;
    pointers = NULL;

    /* traverse node and mark all objects which need to be written */

    ATS_NODE_CALL(node, EnumerateReferences)(node);

    /*
     ** open the file
     */

    if( ( stream = fopen( fileName, "wb" ) ) == NULL )
        return FXFALSE;

    /* write header */

    h.magic = BOF_MAGIC;
    h.versionNumber = BOF_VERSION;
        
    if ( atuWrite32( &h, sizeof( h )>>2, stream ) != 1 ) {
        atuError(FXFALSE, "Error writing BOF file %s\n", fileName);
        return FXFALSE;
    }

    while ( num_objs_written < num_pointers ) {
        if (!atsStore(pointers[num_objs_written], stream))
            return FXFALSE;
        num_objs_written++;
    }


    CHECK(atuWrite32( &eof, 1, stream )) ;

    fclose(stream);

    if ( pointers )
       atuMemFree(pointers);

    return FXTRUE;
}

AtsObject *atsLoadFromBOF(const char *fileName) {
    FILE *stream;
    bofHeader h;
    char full_path[256];
    int i;
    AtsNode *node;
    FxBool error_loading_file = FXFALSE;

    num_pointers = 0;
    max_pointers = 0;
    pointers = NULL;

    /*
     ** open the file
     */

    if ( ( atuFileLocate(fileName, full_path) == NULL ) || 
         ((stream = fopen( full_path, "rb" )) == NULL )) {
        return NULL; 
    }

    /* read & verify header */

    if ( atuRead32( &h, sizeof( h )>>2, stream ) != 1 ) {
        atuError(FXFALSE, "Error reading BOF file\n");
        return NULL;
    }

    if  ( h.magic != BOF_MAGIC ) {
        atuError(FXFALSE, "Invalid BOF file %s\n", fileName);
        return NULL;
    }

    if ( h.versionNumber != BOF_VERSION ) {
        atuError(FXFALSE, "Old BOF file %s\n", fileName);
        return NULL;
    }
        
    /* load file contents */

    while (!feof(stream)) {
        if (atsLoad(stream) == NULL ) {
            break;
            atuError(FXFALSE, "atsFileLoad: error loading file\n");
            error_loading_file = FXTRUE;
            goto exit;
        }
    }

    /* relink all the objects */

    for ( i = 0; i < num_pointers; i++ )
        ATS_NODE_CALL(pointers[i], Fixup)(pointers[i]);
        
exit:
    fclose(stream);

    if ( !error_loading_file && (num_pointers > 0 ))
         node = pointers[0];
    else node = NULL;

    if ( pointers )
       atuMemFree(pointers);

    return node;
}

void atsInitBOF(void) {
}

void atsTermBOF(void) {
}
