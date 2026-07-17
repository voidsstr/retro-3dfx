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
** $Date: 10/11/00 7:34:49 PM$ 
**
**/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <atscenep.h>

static const FxU32 _binaryRevision = 1;
static const char  _dataType[]     = "atrMaterial";

static AtsType material_type;
AtsType *_ats_material_type = &material_type;

AtsObject *_atsMaterialNew( char *where, FxU32 line ) {
    return _atsNew( _ats_material_type, where, line );
}

void atsMaterialTexture(AtsObject *m, int which, AtsObject *t) {
    char *func = "atsMaterialTexture";
    AtsMaterial *mat;
    AtsTexture *tex;

    VALIDATE_MATERIAL(mat, m, func);
    VALIDATE_TEXTURE(tex, t, func);

#ifdef AT_DEBUGGING
    if (( which < 0 ) || ( which > 1 )) {
        atuError(FXFALSE, " atsMaterialTexture: wrong texture index %d\n", 
                 which);
        return;
    }
#endif

    mat->textures[which] = tex;
    mat->material.texture[which] = tex->handle;
}

AtsObject *atsMaterialGetTexture(AtsObject *m, int which) {
    char *func = "atsMaterialGetTexture";
    AtsMaterial *mat;

    VALIDATE_MATERIAL(mat, m, func);

#ifdef AT_DEBUGGING
    if (( which < 0 ) || ( which > 1 )) {
        atuError(FXFALSE, " atsMaterialTexture: wrong texture index %d\n", 
                 which);
        return NULL;
    }
#endif

    return mat->textures[which] ;
}

void atsMaterialSuccessor(AtsObject *m, AtsObject *s) {
    char *func = "atsMaterialSuccessor";
    AtsMaterial *mat, *successor;

    VALIDATE_MATERIAL(mat, m, func);
    VALIDATE_MATERIAL(successor, s, func);

    mat->successor = successor;
    mat->material.successor = &successor->material;
}

AtsObject *atsMaterialGetSuccessor(AtsObject *m) {
    char *func = "atsMaterialGetSuccessor";
    AtsMaterial *mat;

    VALIDATE_MATERIAL(mat, m, func);

    return mat->successor ;
}

static void MaterialInit(AtsObject *obj) {
    AtsMaterial *m;

    VALIDATE_MATERIAL(m, obj, "MaterialInit"); 
    ATS_PARENT_CALL(_ats_material_type, Init)(obj);

    atrMaterialDefault( &m->material );
    m->successor = NULL;
    m->material.successor = NULL;
    m->textures[0] = 0;
    m->textures[1] = 0;
    return;
}

static void MaterialDelete(AtsObject *obj) {
    AtsMaterial *m;

    VALIDATE_MATERIAL(m, obj, "MaterialDelete"); 
}

static AtsObject *MaterialClone(AtsObject* obj, FxU32 mode) {
    AtsMaterial *msrc, *mdst;

    VALIDATE_MATERIAL(msrc, obj, "MaterialClone"); 

    switch ( mode ) {
    case ATS_CLONE_HIERARCHY: /* clone only the hierarchy */
        atsRef(obj);
        return obj;
    case ATS_CLONE_ALL:       /* clone the hierarchy and geometry */
        mdst = atsMaterialNew();    
        *mdst = *msrc;
        atuMemType(mdst, _ats_material_type->index);
        return (AtsObject *)mdst;
    default:
        atuError( FXTRUE, "MaterialClone(): unknown clone method %d\n", mode );
        return NULL;
    }
}

static FxBool MaterialPrint(const AtsObject* obj, FILE *stream, 
                               FxU32 indent, FxU32 verbose) {
    AtsMaterial *m;

    VALIDATE_MATERIAL(m, obj, "MaterialPrint"); 

    atrMaterialPrint(&m->material, stream, indent);

    if ( m->textures[0] )
        atsPrint(m->textures[0], stream, indent, verbose);

    return FXTRUE;
}

static FxBool
atsReadColor( AtrColor *c, FxU32 count, FILE *stream) {
    CHECK ( atuRead32( c, count*3, stream ) );

    return FXTRUE;
}

static FxBool
atsWriteColor( AtrColor *c, FxU32 count, FILE *stream) {
    CHECK ( atuWrite32( c, count*3, stream ) );

    return FXTRUE;
}

static FxBool
atsReadTexDetail( AtrTexDetail *t, FxU32 count, FILE *stream) {
    FxU32 i;

    for ( i = 0; i < count; i++ ) {
        CHECK ( atuRead32( &t[i].bias, 1, stream ) );
        CHECK ( atuRead32( &t[i].scale, 1, stream ) );
        CHECK ( atuRead32( &t[i].max, 1, stream ) );
    }

    return FXTRUE;
}

static FxBool
atsWriteTexDetail( AtrTexDetail *t, FxU32 count, FILE *stream) {
    FxU32 i;

    for ( i = 0; i < count; i++ ) {
        CHECK ( atuWrite32( &t[i].bias, 1, stream ) );
        CHECK ( atuWrite32( &t[i].scale, 1, stream ) );
        CHECK ( atuWrite32( &t[i].max, 1, stream ) );
    }

    return FXTRUE;
}

static FxBool 
MaterialLoad(AtsObject *obj, FILE *stream) {
    AtsMaterial *m;
    AtrMaterial *mat;
    void *id;
    FxU32 version;
    char  type[64];

    VALIDATE_MATERIAL(m, obj, "MaterialLoad"); 

    mat = &(m->material);

    CHECK ( atuRead32( &version, 1, stream ) );

    CHECK ( version == _binaryRevision );

    CHECK( atuRead8( type, sizeof( _dataType ), stream ));

    CHECK(!strcmp( type, _dataType ));
    
    CHECK( atuRead32( &mat->typeFlag, 1, stream));

    CHECK( atsReadColor( &mat->emissive, 1, stream));
    CHECK( atsReadColor( &mat->diffuse, 1, stream));
    CHECK( atsReadColor( &mat->specular, 1, stream));

    CHECK( atuRead32( &mat->specExponent, 1, stream));
    CHECK( atuRead32( &mat->constant, 1, stream));
    CHECK( atuRead32( &mat->chromaKeyEnable, 1, stream));
    CHECK( atuRead32( &mat->chromaKeyValue, 1, stream));

    CHECK( atuRead32( &id, 1, stream ) );

    m->textures[0] = id;

    CHECK( atuRead32( &id, 1, stream ) );

    m->textures[1] = id;
    
    CHECK( atuRead32( &mat->projectedTag, 1, stream));
    CHECK( atuRead32( &mat->planarScale, 1, stream));
    CHECK( atuRead32( &mat->isTwoSided, 1, stream));

    CHECK( atuRead32( mat->lmCoords, 6, stream));

    CHECK( atuRead32( &id, 1, stream ) );

    m->successor = id ;

    CHECK( atuRead32( &mat->irgbSrc, 1, stream));
    CHECK( atuRead32( &mat->iaSrc, 1, stream));
    CHECK( atuRead32( &mat->crgbSrc, 1, stream));

    CHECK( atuRead32( mat->texSrc, 2, stream));
    CHECK( atuRead32( mat->tcSrc, 2, stream));

    CHECK( atuRead32( mat->tcuCFunction, 2, stream));
    CHECK( atuRead32( mat->tcuCFactor, 2, stream));
    CHECK( atuRead32( mat->tcuAFunction, 2, stream));
    CHECK( atuRead32( mat->tcuAFactor, 2, stream));
    CHECK( atuRead32( mat->tcuCInvert, 2, stream));
    CHECK( atuRead32( mat->tcuAInvert, 2, stream));

    CHECK( atuRead32( &mat->atestFunc, 1, stream));

    CHECK( atuRead8( &mat->aReference, 1, stream));

    CHECK( atuRead32( mat->texSClamp, 2, stream));
    CHECK( atuRead32( mat->texTClamp, 2, stream));
    CHECK( atuRead32( mat->texMMMode, 2, stream));
    CHECK( atuRead32( mat->texMinFilter, 2, stream));
    CHECK( atuRead32( mat->texMagFilter, 2, stream));
    CHECK( atuRead32( mat->texLODBlend, 2, stream));

    CHECK( atsReadTexDetail( mat->texDetail, 2, stream));

    CHECK( atuRead32( &mat->ccuFunction, 1, stream));
    CHECK( atuRead32( &mat->ccuFactor, 1, stream));
    CHECK( atuRead32( &mat->ccuLocal, 1, stream));
    CHECK( atuRead32( &mat->ccuOther, 1, stream));
    CHECK( atuRead32( &mat->ccuInvert, 1, stream));
    CHECK( atuRead32( &mat->acuFunction, 1, stream));
    CHECK( atuRead32( &mat->acuFactor, 1, stream));
    CHECK( atuRead32( &mat->acuLocal, 1, stream));
    CHECK( atuRead32( &mat->acuOther, 1, stream));
    CHECK( atuRead32( &mat->acuInvert, 1, stream));
    CHECK( atuRead32( &mat->abuSrcFactor, 1, stream));
    CHECK( atuRead32( &mat->abuDstFactor, 1, stream));
    CHECK( atuRead32( &mat->depthMask, 1, stream));
    CHECK( atuRead32( &mat->sysFlags, 1, stream));

    return FXTRUE;
}

static FxBool MaterialFixup(AtsObject *obj) {
    AtsMaterial *m ;

    VALIDATE_MATERIAL(m, obj, "MaterialFixup");

    if ( m->textures[0] ) {
        m->textures[0] = atsIDToPointer( m->textures[0] );
        if ( !m->textures[0] ) return FXFALSE;
        m->material.texture[0] = ((AtsTexture *)m->textures[0])->handle;
    }

    if ( m->textures[1] ) {
        m->textures[1] = atsIDToPointer( m->textures[1] );
        if ( !m->textures[1] ) return FXFALSE;
        m->material.texture[1] = ((AtsTexture *)m->textures[1])->handle;
    }

    if ( m->successor ) {
        m->successor = atsIDToPointer( m->successor );
        CHECK ( m->successor != NULL ) ;
        m->material.successor = &m->successor->material;
    }

    atrMaterialModify(m);
    return FXTRUE;
}

static FxBool MaterialStore(AtsObject *obj, FILE *stream) {
    AtsMaterial *m;
    AtrMaterial *mat;
    void *id;

    VALIDATE_MATERIAL(m, obj, "MaterialStore"); 

    mat = &(m->material);

    CHECK ( atuWrite32( &_binaryRevision, 1, stream ) );

    CHECK( atuWrite8( _dataType, sizeof( _dataType ), stream ));

    CHECK( atuWrite32( &mat->typeFlag, 1, stream));

    CHECK( atsWriteColor( &mat->emissive, 1, stream));
    CHECK( atsWriteColor( &mat->diffuse, 1, stream));
    CHECK( atsWriteColor( &mat->specular, 1, stream));

    CHECK( atuWrite32( &mat->specExponent, 1, stream));
    CHECK( atuWrite32( &mat->constant, 1, stream));
    CHECK( atuWrite32( &mat->chromaKeyEnable, 1, stream));
    CHECK( atuWrite32( &mat->chromaKeyValue, 1, stream));

    /* texture handles */

    if ( m->textures[0] != NULL ) {
        id = atsPointerToID( m->textures[0] );
    } else {
        id = NULL;
    }

    CHECK ( atuWrite32( &id, 1, stream ) );

    if ( m->textures[1] != NULL ) {
        id = atsPointerToID( m->textures[1] );
    } else {
        id = NULL;
    }

    CHECK ( atuWrite32( &id, 1, stream ) );

    CHECK( atuWrite32( &mat->projectedTag, 1, stream));
    CHECK( atuWrite32( &mat->planarScale, 1, stream));
    CHECK( atuWrite32( &mat->isTwoSided, 1, stream));

    CHECK( atuWrite32( mat->lmCoords, 6, stream));

    /* material successor */

    if ( m->successor != NULL ) {
        id = atsPointerToID( m->successor );
    } else {
        id = NULL;
    }

    CHECK ( atuWrite32( &id, 1, stream ) );

    CHECK( atuWrite32( &mat->irgbSrc, 1, stream));
    CHECK( atuWrite32( &mat->iaSrc, 1, stream));
    CHECK( atuWrite32( &mat->crgbSrc, 1, stream));

    CHECK( atuWrite32( mat->texSrc, 2, stream));
    CHECK( atuWrite32( mat->tcSrc, 2, stream));

    CHECK( atuWrite32( mat->tcuCFunction, 2, stream));
    CHECK( atuWrite32( mat->tcuCFactor, 2, stream));
    CHECK( atuWrite32( mat->tcuAFunction, 2, stream));
    CHECK( atuWrite32( mat->tcuAFactor, 2, stream));
    CHECK( atuWrite32( mat->tcuCInvert, 2, stream));
    CHECK( atuWrite32( mat->tcuAInvert, 2, stream));

    CHECK( atuWrite32( &mat->atestFunc, 1, stream));

    CHECK( atuWrite8( &mat->aReference, 1, stream));

    CHECK( atuWrite32( mat->texSClamp, 2, stream));
    CHECK( atuWrite32( mat->texTClamp, 2, stream));
    CHECK( atuWrite32( mat->texMMMode, 2, stream));
    CHECK( atuWrite32( mat->texMinFilter, 2, stream));
    CHECK( atuWrite32( mat->texMagFilter, 2, stream));
    CHECK( atuWrite32( mat->texLODBlend, 2, stream));

    CHECK( atsWriteTexDetail( mat->texDetail, 2, stream));

    CHECK( atuWrite32( &mat->ccuFunction, 1, stream));
    CHECK( atuWrite32( &mat->ccuFactor, 1, stream));
    CHECK( atuWrite32( &mat->ccuLocal, 1, stream));
    CHECK( atuWrite32( &mat->ccuOther, 1, stream));
    CHECK( atuWrite32( &mat->ccuInvert, 1, stream));
    CHECK( atuWrite32( &mat->acuFunction, 1, stream));
    CHECK( atuWrite32( &mat->acuFactor, 1, stream));
    CHECK( atuWrite32( &mat->acuLocal, 1, stream));
    CHECK( atuWrite32( &mat->acuOther, 1, stream));
    CHECK( atuWrite32( &mat->acuInvert, 1, stream));
    CHECK( atuWrite32( &mat->abuSrcFactor, 1, stream));
    CHECK( atuWrite32( &mat->abuDstFactor, 1, stream));
    CHECK( atuWrite32( &mat->depthMask, 1, stream));
    CHECK( atuWrite32( &mat->sysFlags, 1, stream));

    return FXTRUE;
}

static void MaterialEnumerateReferences(AtsObject *obj) {
    AtsMaterial *m ;

    VALIDATE_MATERIAL(m, obj, "MaterialEnumerateReferences");

    if ( m->textures[0] != NULL )
        ATS_NODE_CALL(m->textures[0], EnumerateReferences)(m->textures[0]);

    if ( m->successor != NULL ) {
        ATS_NODE_CALL(m->successor, EnumerateReferences)(m->successor);
    }
}

void _atsMaterialInitClass(void) {
    atsObjectNewType(_ats_material_type);
    _ats_material_type->name      = "Material";
    _ats_material_type->size      = sizeof(AtsMaterial);
    _ats_material_type->parent    = _ats_object_type;

    /* initialize methods */

    _ats_material_type->Init                = MaterialInit;
    _ats_material_type->Delete              = MaterialDelete;
    _ats_material_type->Clone               = MaterialClone;
    _ats_material_type->Print               = MaterialPrint;
    _ats_material_type->Load                = MaterialLoad;
    _ats_material_type->Fixup               = MaterialFixup;
    _ats_material_type->Store               = MaterialStore;
    _ats_material_type->EnumerateReferences = MaterialEnumerateReferences;
    return ;
}
