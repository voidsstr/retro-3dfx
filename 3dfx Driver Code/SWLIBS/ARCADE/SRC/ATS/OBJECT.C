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
** $Date: 10/11/00 7:34:37 PM$ 
**
*/

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <atscenep.h>

/* amount to extend array of types by */

#define TYPES_INC 10

AtsType object_type;
AtsType *_ats_object_type = &object_type;

AtsType **_ats_types = NULL;
static FxU16 num_types = 0;
static FxU16 max_types = 0;

/*-------------------------------------------------------------------
  Function: atsGetTypeFromName
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Return the type given the types name
  Arguments:
    None
  Return:
    pointer to atsType associated with name
  -------------------------------------------------------------------*/

AtsType* atsGetTypeFromName(char* type_name) {
    int i;

    for ( i = 0; i < num_types; i++ )
        if ( atuStringCompare(type_name, _ats_types[i]->name))
            return _ats_types[i];

    return NULL;
}

/*-------------------------------------------------------------------
  Function: atsNew
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Create a new instance of the the specified object type.
  Arguments:
    type  - What kind of object to create
  Return:
    pointer to new object
  -------------------------------------------------------------------*/

AtsObject* _atsNew(AtsType *type, char *where, FxU32 line) {
    AtsObject *o ;

    o = _atuMemRealloc(NULL, type->size, where, line);

    assert(o);

    atuMemType(o, type->index);

    ATS_NODE_CALL(o, Init)(o);

    return o;
}

/*-------------------------------------------------------------------
  Function: atsClone
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Create a copy of an object, the mode argument specifies what to
    copy and what simply to reference.
  Arguments:
    object  - Object to copy
    mode  - What kind of copy to create
  Return:
    pointer to copy of object
  -------------------------------------------------------------------*/

AtsObject* atsClone(AtsObject *obj, FxU32 mode) {
    return ATS_NODE_CALL(obj, Clone)(obj, mode);
}

/*-------------------------------------------------------------------
  Function: atsUnrefDelete
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Decrement the object use count and if it goes to zero free the
    object.
  Arguments:
    obj  - Object to dereference
  Return:
    Nothing
  -------------------------------------------------------------------*/

void atsUnrefDelete(AtsObject *obj) {
    if ( obj && (atuMemDeref(obj) == 0 ))
        ATS_NODE_CALL(obj, Delete)(obj);
}

/*-------------------------------------------------------------------
  Function: atsDelete
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Free the specified object independent of ref count
  Arguments:
    obj  - Object to delete
  Return:
    Nothing
  -------------------------------------------------------------------*/

void atsDelete(AtsObject *obj) {
    if ( obj )
        ATS_NODE_CALL(obj, Delete)(obj);
}

/*-------------------------------------------------------------------
  Function: atsGetTypeName
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Return the name of the type of the specified object
  Arguments:
    type  - whose name is being inquired
  Return:
    name of type
  -------------------------------------------------------------------*/

const char* atsGetTypeName(const AtsType *t) {
    return t->name;
}

/*-------------------------------------------------------------------
  Function: atsGetType
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Return the type of the specified object
  Arguments:
    obj  - Object whose type is being inquired
  Return:
    pointer to type
  -------------------------------------------------------------------*/

AtsType* atsGetType(const AtsObject *obj) {
    return _ats_types[atuMemGetType(obj)];
}

/*-------------------------------------------------------------------
  Function: atsIsSubClassOf
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Returns true if the specified type1 is derrived from the type2
  Arguments:
    type1 - object whose derivation is being queried
    type2 - type2 being comapared
  Return:
    FXTRUE if type is derrived from specified type
    FXFALSE if type is not derrived from specified type
  -------------------------------------------------------------------*/

FxBool  atsIsSubClassOf(AtsType *type1, AtsType *type2) {
    while ( type1 != NULL ) {
        if ( type1 == type2 )
            return FXTRUE;
        else type1 = type1->parent;
    }

    return FXFALSE;
}

/*-------------------------------------------------------------------
  Function: atsIsOfType
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Returns true if the specified object's type is derrived from the
    specified type
  Arguments:
    obj  - object whose type is being inquired
    type - type being comapared
  Return:
    FXTRUE if type is derrived from specified type
    FXFALSE if type is not derrived from specified type
  -------------------------------------------------------------------*/

FxBool  atsIsOfType(const AtsObject *obj, AtsType *type) {
    AtsType *t = _ats_types[atuMemGetType(obj)];

    while ( t != NULL ) {
        if ( t == type )
            return FXTRUE;
        else t = t->parent;
    }

    return FXFALSE;
}

/*-------------------------------------------------------------------
  Function: atsIsOfExactType
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Returns true if the specified object's type is of the specified
  Arguments:
    obj  - Object whose type is being inquired
    type - type being comapared
  Return:
    FXTRUE if types match 
    FXFALSE if types don't natch
  -------------------------------------------------------------------*/

FxBool atsIsExactType(const AtsObject *obj, AtsType *type) {
    AtsType *t = _ats_types[atuMemGetType(obj)];

    return ( t == type );
}

/*-------------------------------------------------------------------
  Function: atsRef
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Increment the reference count of the specified object
  Arguments:
    obj  - object whose reference count is being incremented
  Return:
    New reference count
  -------------------------------------------------------------------*/

FxU16 atsRef(AtsObject* obj) {
    if ( obj )
        return atuMemRef(obj);
    else return 0;
}

/*-------------------------------------------------------------------
  Function: atsUnref
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Decrement the reference count of the specified object
  Arguments:
    obj  - obj whose reference count is being decremented
  Return:
    New reference count
  -------------------------------------------------------------------*/

FxU16 atsUnref(AtsObject* obj) {
    if (obj)
        return atuMemDeref(obj);
    else return 0;
}

/*-------------------------------------------------------------------
  Function: atsGetRefCount
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Determine the number of references to this object
  Arguments:
    obj  - Object whose reference count is being inquired
  Return:
    Object reference count
  -------------------------------------------------------------------*/

FxU16 atsGetRefCount(const AtsObject* obj) {
    return atuMemGetRef(obj);
}

/*-------------------------------------------------------------------
  Function: atsPrint
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Print a formatted description of the object
  Arguments:
    obj  - object to be printed
  Return:
    FXTRUE success
    FXFALSE failure
  -------------------------------------------------------------------*/

FxBool atsPrint(const AtsObject* obj, FILE *stream, 
                 FxU32 indent, FxU32 verbose){
    FxBool result;

#ifdef AT_DEBUGGING
    if ( !obj || !stream ) 
        atuError( FXTRUE, "atsPrint(): Invalid parameter.\n" );
#endif    
    
    fprintf( stream, "%*s%s {\n", indent, " ",
	atsGetTypeName(atsGetType(obj)));
    result = ATS_NODE_CALL(obj, Print)(obj, stream, indent+4, verbose);
    if ( result )
        fprintf( stream, "%*s}\n", indent, " ");
    return result;
}

/*-------------------------------------------------------------------
  Function: atsFind
  Date: 7/14/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Find an object of given type and name
  Arguments:
    obj    - object to be searched
    name   - name of object being searched for (can be NULL)
    type   - kind of object we are looking for
  Return:
    pointer to object if found, NULL if search unsuccessful
  -------------------------------------------------------------------*/

typedef struct {
    AtsObject *obj;
    char *name;
    AtsType *type;
    AtsType *nodeType;
} searchCriteria;

static FxU32 findFunc(AtsObject *object, void *data) {
    const char *name;
    searchCriteria *search = (searchCriteria *)data;

    /* compare name, only if object is a node */

    if ( search->name != NULL ) {
        if (!atsIsOfType( object, search->nodeType ))
            return ATM_TRAV_CONT;

        name = atsNodeGetName(object) ;

        if (( name == NULL ) || ( !atuStringCompare(name, search->name))) {
            return ATM_TRAV_CONT;
        }
    }

    /* compare types */

    if (!atsIsOfType( object, search->type ))
        return ATM_TRAV_CONT;

    search->obj = object;

    return ATM_TRAV_TERM;
}

AtsObject* atsFind(AtsObject* obj, const char *name, const AtsType *type) {
    searchCriteria search;

    if (type == NULL)
        type = atsGetTypeFromName("Object");

    search.obj = NULL;
    search.name = (char *)name;
    search.type = (AtsType *)type;
    search.nodeType = atsGetTypeFromName("Node");

    atsTraverse(obj, findFunc, &search);

    return search.obj;
}

/*-------------------------------------------------------------------
  Function: atsStore
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Store an object to a file stream
  Arguments:
    obj    - object to be stored
    stream - output stream
  Return:
    FXTRUE success
    FXFALSE failure
  -------------------------------------------------------------------*/

FxBool atsStore(AtsObject *obj, FILE *stream) {
    AtsType *t = _ats_types[atuMemGetType(obj)];
    void *id ;

    if ((id = atsPointerToID(obj)) == NULL )
        return FXFALSE;

    CHECK( atuWrite32( &id, 1, stream ));

    CHECK(atsStoreType(t, stream));

    return ATS_NODE_CALL(obj, Store)(obj, stream);
}

/*-------------------------------------------------------------------
  Function: atsLoad
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Load an object from a file stream
  Arguments:
    None
  Return:
    Object loaded from file if successful
    NULL if error
  -------------------------------------------------------------------*/

AtsObject* atsLoad(FILE *stream) {
    FxI32 id ;
    AtsType *t;
    AtsObject *obj;

    if ( !atuRead32( &id, 1, stream )) {
        atuError(FXFALSE, "atsLoad: Error reading object id\n");
        return NULL;
    }

    if ( id == 0 ) {
        printf("loaded all objects");
        return NULL;
    }

    if ( atsLoadType(&t, stream) ) {

        /* short cut the case of the base object type since objects of 
         * this type can have any size so we can't create an object of
         * this type using the value in the type definition.
         * N.B. Any basic objects being written this way must consist of
         * an array of 32 bit values in order to be transferable between
         * systems with different byte orderings.
         */

        if ( t == _ats_object_type ) {
            FxU32 size ;

            if ( !atuRead32( &size, 1, stream ))
                return NULL;

            obj = atuMemCalloc(1, size);

            if ( obj == NULL )
                return NULL;

            atuMemType(obj, t->index);

            if ( !atuRead32( obj, size>>2, stream ))
                return NULL;

            if ( !atsObjectIDProcessed(obj, id) ) {
                atuError(FXFALSE, "atsLoad: Error getting id\n");
                return NULL;
            }

            return obj;

        }

        obj = atsNew(t);
        if ( !atsObjectIDProcessed(obj, id) ) {
            atuError(FXFALSE, "atsLoad: Error getting id\n");
            return NULL;
        }
    
        if ( ATS_NODE_CALL(obj, Load)(obj, stream))
            return obj;
        else {
            atuError(FXFALSE, "atsLoad: Error processing object\n");
            return NULL;
        }
    } else {
        atuError(FXFALSE, "atsLoad: Unknown object type\n");
        return NULL;
    }
    
    return NULL; 
}

/*-------------------------------------------------------------------
  Function: atsDraw
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Render a specified object (and its children if any)
  Arguments:
    obj  - Object being drawn
    mask - mask to specify which objects to be drawn
  Return:
    Nothing
  -------------------------------------------------------------------*/

void atsDraw(AtsObject* obj, FxU32 mask) {
    ATS_NODE_CALL(obj, Draw)(obj, mask);
}

/*-------------------------------------------------------------------
  Function: atsUpdate
  Date: 6/4/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Update a specified object (and its children if any)
  Arguments:
    obj  - Object being updated
  Return:
    Nothing
  -------------------------------------------------------------------*/

void atsUpdate(AtsObject* obj) {
    ATS_NODE_CALL(obj, Update)(obj);
}

/*-------------------------------------------------------------------
  Function: atsTraverse
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Traverse a specified object (and its children if any), application gets
    called back for each object descended from and including top object
  Arguments:
    obj     - Object being traversed
    func    - Callback function
    data    - data to be passed to callback along with the object
  Return:
    Nothing
  -------------------------------------------------------------------*/

void atsTraverse(AtsObject* obj, AtsTravFunc func, void *data) {
    ATS_NODE_CALL(obj, Traverse)(obj, func, data);
}

/*-------------------------------------------------------------------
  Function: atsTraverseVertices
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Traverse a specified object (and its children if any), application gets
    called back for each vertex descended from and including top object
  Arguments:
    obj  - Object being traversed
    func - Callback function for each object
    data - data to be passed to callback along with the object
  Return:
    Nothing
  -------------------------------------------------------------------*/

void atsTraverseVertices(AtsObject* obj, AtsTravVertexFunc func, 
                             void *data) {
    ATS_NODE_CALL(obj, TraverseVertices)(obj, func, data);
}

static void _computeBBox(int count, AtrVertex *v, void *d)
{
   AtmBox *box = (AtmBox *)d;
   int stride = sizeof(AtrVertex)/sizeof(float);
   AtrXform t;

   atsQueryXform( &t );

   atmBoxAddXformedPoints(box, count, stride, (float *)v, t.data);
}  

/*-------------------------------------------------------------------
  Function: atsComputeBBox
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Get a objects bounding box
  Arguments:
    obj  - the object whose bounding volume is being enquired
    box   - returns the object bounding box
  Return:
    Nothing
  -------------------------------------------------------------------*/

void atsComputeBBox(AtsObject* obj, AtmBox *box) {
   AtrXform t;

   atrXformSetIdentity( &t );
   atmBoxEmpty(box);

   atsPushXform( &t );
   ATS_NODE_CALL(obj, TraverseVertices)(obj, _computeBBox, box);
   atsPopXform(); /* TBD: do we really need to update state? */
}

typedef struct {
   FxU32 total_verts;
   AtmVector3 sum;
} COGData;

static void _sumVertices(int count, AtrVertex *v, void *d)
{
   COGData *data = (COGData *)d;
   int i;
   AtrXform t;
   AtmVector3 tmp;

   atsQueryXform( &t );

   for ( i = 0; i < count; i++ ) {
     atmVector3Matrix4x4Mult( tmp, (float *)(v+i), t.data );
     atmVector3Add(data->sum, data->sum, tmp);
   }

   data->total_verts += count;
}  

typedef struct {
   float max;
   AtmVector3 center;
} RadiusData;

static void _computeRadius(int count, AtrVertex *v, void *d)
{
   RadiusData *data = (RadiusData *)d;
   int i;
   float dist, dx, dy, dz;
   AtrXform t;
   AtmVector3 tmp;

   atsQueryXform( &t );

   for ( i = 0; i < count; i++ ) {
       atmVector3Matrix4x4Mult( tmp, (float *)(v+i), t.data );
       dx = tmp[0]-data->center[0];
       dy = tmp[1]-data->center[1];
       dz = tmp[2]-data->center[2];
       dist = dx*dx+dy*dy+dz*dz;
       if ( dist > data->max ) 
          data->max = dist;
   }
}  

/*-------------------------------------------------------------------
  Function: atsComputeBSphere
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Compute a objects bounding sphere
  Arguments:
    obj      - the object whose bounding volume is being enquired
    bsphere   - returns the object bounding sphere
  Return:
    Nothing
  -------------------------------------------------------------------*/

void atsComputeBSphere(AtsObject* obj, AtmSphere *bsphere) {
   COGData cog;
   RadiusData rad;
   AtrXform t;

   atrXformSetIdentity( &t );

   atsPushXform( &t );

   cog.total_verts = 0;
   cog.sum[0] = cog.sum[1] = cog.sum[2] = 0.0f;

   ATS_NODE_CALL(obj, TraverseVertices)(obj, _sumVertices, &cog);

   bsphere->center[0] = rad.center[0] = cog.sum[0]/(float)cog.total_verts;
   bsphere->center[1] = rad.center[1] = cog.sum[1]/(float)cog.total_verts;
   bsphere->center[2] = rad.center[2] = cog.sum[2]/(float)cog.total_verts;
   rad.max = 0.0f;

   ATS_NODE_CALL(obj, TraverseVertices)(obj, _computeRadius, &rad);
   atsPopXform(); /* TBD: do we really need to update state? */

   bsphere->radius = (float)sqrt(rad.max);
}

static void ObjectInit(AtsObject *obj) {
    FXUNUSED(obj);
}

static AtsObject *ObjectClone(AtsObject* src, FxU32 mode) {
    AtsObject *dst;
    AtsType *t = _ats_types[atuMemGetType(src)];
    
    dst = atsNew(t);
  
    memcpy(dst, src, t->size);

    switch ( mode ) {
    case ATS_CLONE_OBJECT:    /* clone just this object */
    case ATS_CLONE_HIERARCHY: /* clone only the hierarchy */
    case ATS_CLONE_ALL:       /* clone the hierarchy and geometry */
        return dst;
    default:
        atuError( FXTRUE, "ObjectClone(): unknown clone method %d\n", mode );
        return NULL;
    }
}

static void ObjectDelete(AtsObject *obj) {
    atuMemFree(obj);
}

static FxBool ObjectPrint(const AtsObject* obj, FILE *stream, FxU32 indent, 
			 FxU32 verbose) {
    FXUNUSED(stream);
    FXUNUSED(verbose);
    printf("%*s object 0x%lx\n", indent, obj);

    return FXTRUE;
}

static FxBool ObjectLoad(AtsObject *obj, FILE *stream){

    FXUNUSED(obj);
    FXUNUSED(stream);
    atuError(FXTRUE, "ObjectLoad: Should not be called\n");
    return FXFALSE;
}

static FxBool ObjectFixup(AtsObject *obj) {
    FXUNUSED(obj);
    return FXTRUE;
}

static FxBool ObjectStore(AtsObject *obj, FILE *stream)
{
    FxU32 size = atuMemGetSize( obj );

    CHECK( atuWrite32( &size, 1, stream ));

    CHECK( atuWrite32( obj, size>>2, stream ));

    return FXTRUE;
}

static void ObjectEnumerateReferences(AtsObject *obj) {
    atsPointerToID(obj);
}

static void 
ObjectDraw(AtsObject *obj, FxU32 mask) {
    FXUNUSED(obj);
    FXUNUSED(mask);
}

static void 
ObjectUpdate(AtsObject *obj) {
    FXUNUSED(obj);
}

static FxU32 ObjectTraverse(AtsObject* obj, AtsTravFunc func, void *data) {
    FxU32 status;

    status = (* func)(obj, data);

    if ( status == ATM_TRAV_PRUNE )
        status = ATM_TRAV_CONT;

    return status;
}

static void ObjectTraverseVertices(AtsObject* obj, AtsTravVertexFunc func, 
                                    void *data) {
    /* do nothing */
    FXUNUSED(obj);
    FXUNUSED(func);
    FXUNUSED(data);
}

FxBool atsObjectNewType(AtsType *new_type) {

    if ( num_types >= max_types ) {
       max_types += TYPES_INC;
       _ats_types = atuMemRealloc(_ats_types, max_types*sizeof(AtsType *));
       assert(_ats_types);
    }

    _ats_types[num_types] = new_type;
    new_type->index = num_types;
    num_types++;

    /* set default methods */
           
    new_type->name = "unnamed";
    new_type->Init = ObjectInit;
    new_type->Clone = ObjectClone;
    new_type->Delete = ObjectDelete;
    new_type->Print = ObjectPrint;
    new_type->Load = ObjectLoad;
    new_type->Fixup = ObjectFixup;
    new_type->Store = ObjectStore;
    new_type->EnumerateReferences = ObjectEnumerateReferences;
    new_type->Draw = ObjectDraw;
    new_type->Update = ObjectUpdate;
    new_type->Traverse = ObjectTraverse;
    new_type->TraverseVertices = ObjectTraverseVertices;
    return FXTRUE;
}

void _atsObjectInitClass(void)
{
    atsObjectNewType(_ats_object_type);
    ((AtsType *)_ats_object_type)->name = "Object";
    ((AtsType *)_ats_object_type)->size = 0;
    ((AtsType *)_ats_object_type)->parent = NULL;
}
