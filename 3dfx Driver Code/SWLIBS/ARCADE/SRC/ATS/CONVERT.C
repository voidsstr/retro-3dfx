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
** $Date: 10/11/00 7:34:31 PM$ 
**
*/

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <atrender.h>
#include <atscenep.h>

#define ARG_INC 10

typedef struct {
    FxI32 argID;
    FxU32 type;
    union {
        int   intVal;
        float floatVal;
        void  *ptrVal;
    } val;
} AtsConvAttr;

typedef struct {
    int numArgs;
    int maxArgs;
    AtsConvAttr *attrs;
} AtsConvAttrList;

typedef struct {
    char *ext;
    AtsConvAttrList attrList;
    void (* init) (void);
    AtsObject *(* load)( const char *filename );
    FxBool (* store)( const char *filename, AtsObject *obj );
    void (* term) (void);
} AtsConverter;
 
AtsConvAttrList globalAttrList = { 0, 0, NULL };

/* NB This list must be null terminated 
 */

static AtsConverter converter_list[] = { 
    { "bof", { 0, 0, NULL }, atsInitBOF, atsLoadFromBOF, atsStoreToBOF, atsTermBOF },
    { "tga", { 0, 0, NULL }, NULL, atsTextureCreateFromFile, NULL, NULL },
    { "ppm", { 0, 0, NULL }, NULL, atsTextureCreateFromFile, NULL, NULL },
#if !(macintosh)
    { "3ds", { 0, 0, NULL }, atsInit3DS, atsLoadFrom3DS, NULL, atsTerm3DS },
    { "3df", { 0, 0, NULL }, NULL, atsTextureCreateFromFile, NULL, NULL },
    { "rtg", { 0, 0, NULL }, atsInitRTG, atsLoadFromRTG, NULL, atsTermRTG },
    { "jby", { 0, 0, NULL }, atsInitJBY, atsLoadFromJBY, NULL, atsTermJBY },
    { "vq", { 0, 0, NULL }, atsInitVQ, atsLoadFromVQ, NULL, atsTermVQ },
    { "rad", { 0, 0, NULL }, atsInitRad, atsLoadFromRad, NULL, atsTermRad },
#endif
#ifdef AT_LOADER_MG
    { "flt", { 0, 0, NULL }, atsInitFlt, atsLoadFromFlt, NULL, atsTermFlt },
#endif
    { NULL,  { 0, 0, NULL }, NULL, NULL } };

/*-------------------------------------------------------------------
  Function: atsFindConverter
  Date: 5/30/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Find a converter given an extension
  Arguments:
    ext  - which extension
  Return:
    If found returns the pointer to the extension, if not returns NULL
  -------------------------------------------------------------------*/

static AtsConverter * atsFindConverter(const char *ext)
{
    AtsConverter *converter = converter_list;

    for ( converter = converter_list; converter->ext; converter++ ) {
        if ( atuStringCompare(ext, converter->ext ) )
            return converter;
    }

    return NULL;
}
    
/*-------------------------------------------------------------------
  Function: atsFindAttr
  Date: 5/30/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Find an attribute in a given extension
  Arguments:
    converter - which extension, if NULL use global attributes
    argID     - which argument
  Return:
    If found returns the pointer to the argument, if not returns NULL
  -------------------------------------------------------------------*/

static AtsConvAttr *atsFindAttr( AtsConvAttrList *attrList, const int argID) {
    AtsConvAttr *attr;
    int i;

    i = attrList->numArgs;
    attr = attrList->attrs;

    while ( i-- > 0 ) {
        if ( argID == attr->argID )
            return attr;
        else attr++;
    }

    return NULL;
}

/*-------------------------------------------------------------------
  Function: atsConverterAttr
  Date: 5/30/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Set a converter attribute if ext is NULL the specified global
    attribute is set otherwise the converter specific attribute 
    is set.
  Arguments:
    ext     - which extension
    argID   - which argument
    val     - argument value
  Return:
    FXTRUE if set, FXFALSE otherwise
  -------------------------------------------------------------------*/

static FxBool converterAttr( const char *ext, const FxU32 type, 
                            const int argID, void *val) {
    AtsConverter *converter;
    AtsConvAttrList *attrList;
    AtsConvAttr *attr;
    FxU32 size;

    if (ext != NULL) {
        if ((converter = atsFindConverter(ext) ) == NULL ) {
            atuError(FXFALSE, "Invalid Converter %s\n", ext);
            return FXFALSE;
        } else attrList = &converter->attrList;
    } else attrList = &globalAttrList;

    if ( ( attr = atsFindAttr(attrList, argID) ) == NULL ) {
        if ( attrList->numArgs >= attrList->maxArgs ) {
            attrList->maxArgs += ARG_INC ;
            size = attrList->maxArgs * sizeof(AtsConvAttr);
            attrList->attrs = (AtsConvAttr *)atuMemRealloc(attrList->attrs, 
                                                           size);
            if ( attrList->attrs == NULL )  {
                atuError(FXTRUE, "atsConverterAttr: out of memory\n");
            }
        }

        attr = attrList->attrs+attrList->numArgs;
        attr->type = type ;
        attr->argID = argID ;
        attrList->numArgs++;
    }

    if ( attr->type != type ) {
        atuError(FXFALSE, "Incorrect arg type for %d was %d should be %d\n",
                 argID, attr->type, type);
        return FXFALSE;
    }

    switch ( type ) {
    case ATS_CATTR_POINTER:
        attr->val.ptrVal =  val;
        break;
    case ATS_CATTR_INT:
        attr->val.intVal =  *(int *)val;
        break;
    case ATS_CATTR_FLOAT:
        attr->val.floatVal =  *(float *)val;
        break;
    default:
        atuError(FXFALSE, "Unknown attribute type %d\n", type);
        return FXFALSE;
    }

    return FXTRUE;
}

FxBool atsConverterAttr(const char *ext, const int argID, void *val) {
    return converterAttr( ext, ATS_CATTR_POINTER, argID, val) ;
}

/*-------------------------------------------------------------------
  Function: atsConverterGetAttr
  Date: 5/30/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Get a converter attribute. 

    If ext is NULL only the global attribute list is searched otherwise 
    the specified converters attribut list is searched and if not found 
    the global attribute list is searched.
  Arguments:
    ext     - which extension
    argID   - which argument
    val     - argument value
  Return:
    FXTRUE if found, FXFALSE otherwise
    
  -------------------------------------------------------------------*/

static FxBool converterGetAttr(const char *ext,const FxU32 type, const int argID, void *val){
    AtsConverter *converter;
    AtsConvAttr *attr;
    AtsConvAttrList *attrList;

    if (ext != NULL) {
        if ((converter = atsFindConverter(ext) ) == NULL ) {
            atuError(FXFALSE, "Invalid Converter %s\n", ext);
            return FXFALSE;
        } else attrList = &converter->attrList;
    } else attrList = &globalAttrList;

    if ( ( attr = atsFindAttr(attrList, argID) ) == NULL ) {
        if ( ext != NULL ) 
            attr = atsFindAttr(&globalAttrList, argID) ;
    }

    if ( attr == NULL )
        return FXFALSE;

    switch ( type ) {
    case ATS_CATTR_POINTER:
        *(void **)val = attr->val.ptrVal ;
        break;
    case ATS_CATTR_INT:
        *(int *)val = attr->val.intVal ;
        break;
    case ATS_CATTR_FLOAT:
        *(float *)val = attr->val.floatVal ;
        break;
    default:
        atuError(FXFALSE, "Unknown attribute type %d\n", type);
        return FXFALSE;
    }

    return FXTRUE;
}

FxBool atsConverterGetAttr(const char *ext, const int argID, void **val) {
    return converterGetAttr( ext, ATS_CATTR_POINTER, argID, val) ;
}

/*-------------------------------------------------------------------
  Function: atsConverterIntAttr
  Date: 5/30/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Set an integer converter attribute. 
    If ext is NULL the specified global attribute is set otherwise 
    the converter specific attribute 
    is set.
  Arguments:
    ext     - which extension
    argID   - which argument
    val     - argument value
  Return:
    FXTRUE if set, FXFALSE otherwise
  -------------------------------------------------------------------*/

FxBool atsConverterIntAttr(const char *ext, const int argID, int val) {
    return converterAttr( ext, ATS_CATTR_INT, argID, &val) ;
}

/*-------------------------------------------------------------------
  Function: atsConverterGetIntAttr
  Date: 5/30/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Get an integer converter attribute

    If ext is NULL only the global attribute list is searched otherwise 
    the specified converters attribut list is searched and if not found 
    the global attribute list is searched.
  Arguments:
    ext     - which extension
    argID   - which argument
    val     - argument value
  Return:
    FXTRUE if found, FXFALSE otherwise
    
  -------------------------------------------------------------------*/

FxBool atsConverterGetIntAttr(const char *ext, const int argID, int *val) {
    return converterGetAttr(ext, ATS_CATTR_INT, argID, val);
}

/*-------------------------------------------------------------------
  Function: atsConverterFloatAttr
  Date: 5/30/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Set a floating point converter attribute
    If ext is NULL the specified global attribute is set otherwise 
    the converter specific attribute 
  Arguments:
    ext     - which extension
    argID   - which argument
    val     - argument value
  Return:
    FXTRUE if set, FXFALSE otherwise
  -------------------------------------------------------------------*/

FxBool atsConverterFloatAttr(const char *ext, const int argID, float val) {
    return converterAttr( ext, ATS_CATTR_FLOAT, argID, &val) ;
}

/*-------------------------------------------------------------------
  Function: atsConverterGetFloatAttr
  Date: 5/30/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Get a float converter attribute

    If ext is NULL only the global attribute list is searched otherwise 
    the specified converters attribut list is searched and if not found 
    the global attribute list is searched.
  Arguments:
    ext     - which extension
    argID   - which argument
    val     - argument value
  Return:
    FXTRUE if found, FXFALSE otherwise
    
  -------------------------------------------------------------------*/

FxBool atsConverterGetFloatAttr(const char *ext, const int argID, float *val) {
    return converterGetAttr(ext, ATS_CATTR_FLOAT, argID, val);
}

FxBool _atsInitConverters(void) {
    AtsConverter *converter ;

    /* initialize our default settings */

    atsConverterIntAttr(NULL, ATS_CATTR_LOAD_TEXTURES, FXTRUE);
    
    atsConverterAttr(NULL, ATS_CATTR_TARGET_TYPE, atsGetTypeFromName("Node"));

    for ( converter = converter_list; ( converter->ext ); converter++ ) {
        if ( converter->init )
            ( (* converter->init )() );
    }

    return FXTRUE;
}

FxBool _atsTermConverters(void) {
    AtsConverter *converter ;

    for ( converter = converter_list; ( converter->ext ); converter++ ) {
        if ( converter->term )
            ( (* converter->term )() );
    }

    return FXTRUE;
}

FxBool atsTextureHasAlpha(AtsTexture *h) {
    AtrTexInfo info;

    atrTexInfo( h->handle, &info );

    if ( !info.img ) 
      atuError( FXTRUE, "atsTextureHasAlpha: texture handle not associated with an image.\n" );

    switch ( info.img->format ) {
    case ATR_IMGFMT_ALPHA_8:
    case ATR_IMGFMT_ALPHA_INTENSITY_44:
    case ATR_IMGFMT_ARGB_8332: 
    case ATR_IMGFMT_AYIQ_8422:             
    case ATR_IMGFMT_ARGB_1555:            
    case ATR_IMGFMT_ARGB_4444:           
    case ATR_IMGFMT_ALPHA_INTENSITY_88: 
        return FXTRUE;
    default:
        return FXFALSE;
    }
}
    
/*-------------------------------------------------------------------
  Function: atsFileLoad
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Load an object from a file and return a pointer to the object
  Arguments:
    filename   - name of file to load object from
    objectType - type of object needed
  Return:
    pointer to the loaded object
  -------------------------------------------------------------------*/

AtsObject *atsFileLoad(const char *filename, AtsType *objectType ) {
    int i;
    AtsConverter *converter;
    const char *ext;
    AtsObject *obj;

    if (objectType == NULL)
        objectType = atsGetTypeFromName("Object");

    i = strlen(filename);

    atsConverterAttr(NULL, ATS_CATTR_TARGET_TYPE, objectType);

    for ( i = strlen(filename)-1; i != -1; i-- ) {
        if ( filename[i] == '.' ) {
             ext = filename+i+1;
             if (( converter = atsFindConverter(ext) ) != NULL ) {
                 obj = (converter->load)(filename);
                 if ( obj == NULL ) {
                      return NULL;
                 } else if (!atsIsOfType(obj, objectType)) {
                     atuError(FXTRUE, "atsFileLoad type wanted %s, got %s\n",
                              atsGetTypeName(objectType),
                              atsGetTypeName(atsGetType(obj)));
                 } else return obj;
             }
             return NULL;         
        }
    }
 
    return NULL;
}

/*-------------------------------------------------------------------
  Function: atsFileStore
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Store an object to a model file
  Arguments:
    fileName _ where to store the object
    obj      _ object to store
  Return:
    FXTRUE  on success
    FXFALSE on failure
  -------------------------------------------------------------------*/

FxBool atsFileStore(const char *filename, AtsObject *obj) {
    int i;
    AtsConverter *converter;
    const char *ext;

    i = strlen(filename);

    for ( i = strlen(filename)-1; i != -1; i-- ) {
        if ( filename[i] == '.' ) {
             ext = filename+i+1;
             if (( converter = atsFindConverter(ext) ) != NULL ) {
                 if ( converter->store )
                     return (converter->store)(filename, obj);
                 else {
                    atuError(FXFALSE, "No store converter for type %s\n", ext);
                    return FXFALSE;
                 }
             }
             return FXFALSE;         
        }
    }
 
    return FXFALSE;
}
