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
** $Date: 10/11/00 7:34:32 PM$ 
**
*/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <atscenep.h>

#define DICT_INC 10

static AtsType dict_type;
AtsType *_ats_dict_type = &dict_type;
static const FxU32 _binaryRevision = 1;

/*-------------------------------------------------------------------
  Function: atsDictNew
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Allocate a new dict object
  Arguments:
    None
  Return:
    pointer to new dict object
  -------------------------------------------------------------------*/

AtsObject* _atsDictNew(char *where, FxU32 line) {
    return _atsNew(_ats_dict_type, where, line);
}

/*-------------------------------------------------------------------
  Function: atsDictGetType
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Returns a pointer to the dict's type
  Arguments:
    None
  Return:
    pointer to dicts type 
  -------------------------------------------------------------------*/

AtsType* atsDictGetType(void) {
    return (AtsType *)_ats_dict_type;
}

/*-------------------------------------------------------------------
  Function: atsDictFind
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Look up an entry in a dictionary
  Arguments:
    dict - the dict 
    name - entry to look for
  Return:
    handle to entry
  -------------------------------------------------------------------*/

static int dictFindEntry(const AtsObject* d, char *name) {
    FxU32 i;
    AtsDict *dict ;

    VALIDATE_DICT(dict, d, "atsDictFind");

    for ( i = 0; i < dict->num_entries; i++ )
        if ( atuStringCompare(dict->entries[i].key, name) )
            return i;

    return -1;
}

AtsObject *atsDictFind(const AtsObject* d, char *name, AtsType *objectType) {
    int i;
    AtsDict *dict ;

    if (objectType == NULL)
        objectType = atsGetTypeFromName("Object");

    VALIDATE_DICT(dict, d, "atsDictFind");
   
    i = dictFindEntry(d, name);

    if (( i >= 0 ) && atsIsOfType( dict->entries[i].value, objectType))
        return dict->entries[i].value;
    else return NULL;
}

/*-------------------------------------------------------------------
  Function: atsDictExtend
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Ensure that their is at least enough space to add n 
    additional entries
  Arguments:
    dict  - the dict 
    n     - amount to extend by
  Return:
    Nothing
  -------------------------------------------------------------------*/

static void _atsDictExtend(AtsDict *dict, FxU32 n) {
    if ( dict->num_entries+n >= dict->max_entries ) {
        dict->max_entries += n+DICT_INC;
        dict->entries = (AtsDictEntry *)atuMemRealloc(dict->entries,
                                  dict->max_entries*sizeof(AtsDictEntry));
        assert(dict->entries);
    } 
}

/*-------------------------------------------------------------------
  Function: atsDictAdd
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Add an entry to the specified dict
  Arguments:
    dict - the dict 
    child - the child to be added
  Return:
    FXTRUE success
    FXFALSE failure
  -------------------------------------------------------------------*/

FxBool atsDictAdd(AtsObject* d, const char *name, AtsObject *obj) {
    AtsDict *dict ;

    VALIDATE_DICT(dict, d, "atsDictAddChild");

    _atsDictExtend(dict, 1);

    dict->entries[dict->num_entries].key = strdup(name);
    dict->entries[dict->num_entries].value = obj;
    dict->num_entries++;

    atsRef(obj);

    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: atsDictRemove
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Remove a child from the dict
  Arguments:
    dict - the dict 
    name - the entry to remove
  Return:
    FXTRUE success
    FXFALSE failure
  -------------------------------------------------------------------*/

FxBool atsDictRemove(AtsObject* d, char *name) {
    AtsDict *dict ;
    int entries_to_move, pos;
    AtsDictEntry *n;

    VALIDATE_DICT(dict, d, "atsDictRemove");

    pos = dictFindEntry(d, name);

    if ( pos < 0 )
        return FXFALSE;

    atsUnrefDelete(dict->entries[pos].value);
    atuMemFree(dict->entries[pos].key);

    entries_to_move = dict->num_entries-pos-1;
    n = dict->entries+pos;

    while ( entries_to_move-- > 0 ) {
        *n = *(n+1);
	n++;
    }

    dict->num_entries--;

    return FXTRUE; 
}

/*-------------------------------------------------------------------
  Function: atsDictGetSize
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Return the number of entries in a dict
  Arguments:
    dict - the dict 
  Return:
    Number of entries in dict
  -------------------------------------------------------------------*/

int atsDictGetSize(const AtsObject* d) {
    AtsDict *dict ;

    VALIDATE_DICT(dict, d, "atsDictGetSize");

    return dict->num_entries;
}

static void DictInit(AtsObject *obj)
{
    AtsDict *dict ;

    VALIDATE_DICT(dict, obj, "DictInit");

    dict->num_entries = 0;
    dict->max_entries = 0;
    dict->entries = NULL;
}

static AtsObject *DictClone(AtsObject* obj, FxU32 mode) {
    AtsDict *dsrc, *ddst ;
    AtsObject *ndst;
    FxU32 i;

    VALIDATE_DICT(dsrc, obj, "DictClone");

    if ((ndst = atsDictNew()) == NULL ) {
        atuError(FXTRUE, "can not allocate directory\n");
    }

    ddst = (AtsDict *)ndst;

    ddst->num_entries = 0;
    ddst->max_entries = 0;
    ddst->entries = NULL;

    _atsDictExtend(ddst, dsrc->num_entries-1);

    for ( i = 0; i < dsrc->num_entries; i++ )
        atsDictAdd(ndst, dsrc->entries[i].key, 
                   atsClone(dsrc->entries[i].value, mode));

    return ndst;
}

static void DictDelete(AtsObject *obj) {
    AtsDict *dict ;
    FxU32 i;

    VALIDATE_DICT(dict, obj, "DictDelete");

    /* decrement reference count on each of dicts entries */

    for ( i = 0; i < dict->num_entries; i++ ) {
        atsUnrefDelete(dict->entries[i].value);
        atuMemFree(dict->entries[i].key);
    }

    atuMemFree(dict->entries);
}

static FxBool DictPrint(const AtsObject* obj, FILE *stream, 
                               FxU32 indent, FxU32 verbose) {
    FxU32 i;
    AtsDict *dict ;

    VALIDATE_DICT(dict, obj, "DictPrint");

    for ( i = 0; i < dict->num_entries; i++ ) {
        fprintf( stream, "%*s %s\n", indent, " ", dict->entries[i].key);
        atsPrint(dict->entries[i].value, stream, indent+4, verbose);
    }

    return FXTRUE;
}

static FxBool DictLoad(AtsObject *obj, FILE *stream) {
    AtsDict *dict ;
    FxU32 num_entries;
    FxU32 i;
    FxU32 version;

    VALIDATE_DICT(dict, obj, "DictLoad");

    CHECK( atuRead32( &version, 1, stream ));

    CHECK( version == _binaryRevision );

    CHECK( atuRead32( &num_entries, 1, stream ));

    if ( num_entries == 0 )
        return FXTRUE;

    _atsDictExtend(dict, num_entries);
    dict->num_entries = num_entries;

    for ( i = 0; i < num_entries; i++ ) {
        CHECK(atsLoadString(&dict->entries[i].key, stream) );

        CHECK( atuRead32( &dict->entries[i].value, 1, stream ));
    }

    return FXTRUE;
}

static FxBool DictFixup(AtsObject *obj) {
    AtsDict *dict ;
    FxU32 i;

    VALIDATE_DICT(dict, obj, "DictFixup");

    if (!ATS_PARENT_CALL(_ats_dict_type, Fixup)(obj))
        return FXFALSE;

    if ( dict->num_entries == 0 ) 
        return FXTRUE;

    for ( i = 0; i < dict->num_entries; i++ ) {
        if ((dict->entries[i].value = atsIDToPointer(dict->entries[i].value)) == NULL )
            return FXFALSE;

        atsRef(dict->entries[i].value);
    }

    return FXTRUE;
}

static FxBool DictStore(AtsObject *obj, FILE *stream) {
    AtsDict *dict ;
    void *id;
    FxU32 i;

    VALIDATE_DICT(dict, obj, "DictStore");

    CHECK( atuWrite32( &_binaryRevision, 1, stream ));

    CHECK( atuWrite32( &dict->num_entries, 1, stream ));

    for ( i = 0; i < dict->num_entries; i++ ) {
        CHECK(atsStoreString(dict->entries[i].key, stream));
        id = atsPointerToID( dict->entries[i].value );
        CHECK( atuWrite32( &id, 1, stream ));
    }

    return FXTRUE;
}

static void DictEnumerateReferences(AtsObject *obj) {
    AtsDict *dict ;
    FxU32 i;

    VALIDATE_DICT(dict, obj, "DictStore");

    ATS_PARENT_CALL(_ats_dict_type, EnumerateReferences)(obj);

    for ( i = 0; i < dict->num_entries; i++ ) {
        ATS_NODE_CALL(dict->entries[i].value, EnumerateReferences)(dict->entries[i].value);
    }
}

void atsDictNewType(AtsType *dt) {
    atsNodeNewType(dt);

    /* set default methods */
           
    dt->Init = DictInit;
    dt->Clone = DictClone;
    dt->Delete = DictDelete;
    dt->Print = DictPrint;
    dt->Load = DictLoad;
    dt->Fixup = DictFixup;
    dt->Store = DictStore;
    dt->EnumerateReferences = DictEnumerateReferences;

    return ;
}

void _atsDictInitClass(void)
{
    atsDictNewType(_ats_dict_type);
    ((AtsType *)_ats_dict_type)->name = "Dict";
    ((AtsType *)_ats_dict_type)->size = sizeof(AtsDict);
    ((AtsType *)_ats_dict_type)->parent = (AtsType *)_ats_object_type;
}
