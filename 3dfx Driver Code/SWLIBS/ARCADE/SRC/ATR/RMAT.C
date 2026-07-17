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
** $Date: 10/11/00 7:34:12 PM$ 
**
*/

#include "atrender.h"
#include "fxatr.h"
#include <string.h>
#define  GLIDE_HARDWARE
#include <glide.h>

/*
**-----------------------------------------------------------------
** File Scope Data
**-----------------------------------------------------------------
*/

static const char  _dataType[]     = "atrMaterial";
static const FxU32 _binaryRevision = 0;

/*-------------------------------------------------------------------
  Function: atrMaterialAllocate
  Date: 3/20/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Allocate materials.
  Arguments:
    num - number of materials to allocate
  Return:
    pointer to array of materials
  -------------------------------------------------------------------*/
AtrMaterial *atrMaterialAllocate( FxU32 num ) {
    AtrMaterial *m;
    FxU32 index;
#ifdef AT_DEBUGGING
    if ( num == 0 ) 
        atuError( FXTRUE, "atrMaterialAllocate: Won't allocate 0 structures.\n" );
#endif

    m = atuMemCalloc( num, sizeof( AtrMaterial ) );
    for( index = 0; index < num; index++ )
        atrMaterialDefault( m+index );
    return m;
}

/*-------------------------------------------------------------------
  Function: atrMaterialDeallocate
  Date: 3/20/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Deallocate an array of cameras.
  Arguments:
    m - material array
  Return:
    none
  -------------------------------------------------------------------*/
void atrMaterialDeallocate( AtrMaterial *m ) {
#ifdef AT_DEBUGGING
    if ( !m ) 
        atuError( FXTRUE, "atrMaterialDeallocate: invalid parameter.\n" );
#endif

    atuMemFree( m );
    return;
}

/*-------------------------------------------------------------------
  Function: AtrMaterialDefault
  Date: 4/16
  Implementor(s): jdt
  Library: AT Render
  Description:
    Initialize data structure to default values

    For Material
        Cleared to white diffuse colored material
  Arguments:
    m - material to initialize
  Return:
    none
  -------------------------------------------------------------------*/
void atrMaterialDefault( AtrMaterial *m ) {
#ifdef AT_DEBUGGING
    if ( !m ) 
        atuError( FXTRUE, "atrMaterialDefault: invalid parameter.\n" );
#endif
    atrMaterialSetup( m, ATR_MAT_GSHADE );
    m->diffuse.r = 0.0f;
    m->diffuse.g = 0.0f;
    m->diffuse.b = 0.0f;
    m->specular.r = 0.0f;
    m->specular.g = 0.0f;
    m->specular.b = 0.0f;
    m->specExponent = 20;
    m->atestFunc = ATR_CMP_ALWAYS;
    m->planarScale = 0.04f;
    m->aReference = 0;
    m->texture[0] = 0;
    m->texture[1] = 0;
    m->successor = 0;
    return;
}


/*-------------------------------------------------------------------
  Function: atrMaterialStore
  Date: 3/20/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Store a material to a file stream.
  Arguments:
    m - material
    stream - ouput stream
  Return:
    FXTRUE - success
    FXFALSE - failure
  -------------------------------------------------------------------*/
FxBool atrMaterialStore( AtrMaterial *m, FILE *stream ) {
    FxU32 count;
    AtrTexHandle oldtexture;
#ifdef AT_DEBUGGING
    if ( !m || !stream ) 
        atuError( FXTRUE, "atrMaterialStore(): Invalid parameter.\n" );
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

    oldtexture = m->texture[0];
    m->texture[0] = 0;
    count = fwrite( m, sizeof( AtrMaterial ), 1, stream );
    m->texture[0] = oldtexture;

    if ( count != 1 ) return FXFALSE;

    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: atrMaterialLoad
  Date: 3/20/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Load a material from a file stream
  Arguments:
    m - material
    stream - input stream
  Return:
    FXTRUE - success
    FXFALSE - failure
  -------------------------------------------------------------------*/
FxBool atrMaterialLoad( AtrMaterial *m, FILE *stream ) {
    FxU32 rev;
    char  type[64];
    FxU32 count;

#ifdef AT_DEBUGGING
    if ( !m || !stream ) 
        atuError( FXTRUE, "atrMaterialLoad(): Invalid parameter.\n" );
#endif    

    count = fread( &rev, sizeof( _binaryRevision ), 1, stream );
    if ( count != 1 ) return FXFALSE;
    if ( rev != _binaryRevision ) return FXFALSE;
    count = fread( type, sizeof( _dataType ), 1, stream );
    if ( count != 1 ) return FXFALSE;
    if ( strcmp( type, _dataType ) ) return FXFALSE;
    count = fread( m, sizeof( AtrMaterial ), 1, stream );
    if ( count != 1 ) return FXFALSE;
    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: atrMaterialPrint
  Date: 3/20/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Print a formatted description of a material
  Arguments:
    indent - number of spaces to indent each line
    stream - ascii output stream
    m - material
  Return:
    none
  -------------------------------------------------------------------*/
void atrMaterialPrint( AtrMaterial *m,
                       FILE        *stream, 
                       FxU32       indent ) {
#ifdef AT_DEBUGGING
    if ( !m || !stream ) 
        atuError( FXTRUE, "atrMaterialPrint(): Invalid parameter.\n" );
#endif    

        fprintf( stream, "%*sMaterial: er:%f eg:%f eb:%f\ndr:%f dg:%f db:%f\n"
                     "%*s          sr:%f sg:%f sb:%f\n",
             indent, " ",
             m->emissive.r,
             m->emissive.g,
             m->emissive.b,
             m->diffuse.r,
             m->diffuse.g,
             m->diffuse.b,
             indent, " ",
             m->specular.r,
             m->specular.g,
             m->specular.b );
    return;
}

/*-------------------------------------------------------------------
  Function: atrMaterialAssign
  Date: 3/20/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Assign one material to another.
  Arguments:
    src - source material
    dest - destination material
  Return:
    none
  -------------------------------------------------------------------*/
void atrMaterialAssign( AtrMaterial *dest, AtrMaterial *src ) {
#ifdef AT_DEBUGGING
    if ( !dest || !src ) 
        atuError( FXTRUE, "atrMaterialAssign(): Invalid parameter.\n" );
#endif    

    *dest = *src;
    return;
}

/*-------------------------------------------------------------------
  Function: atrMaterialClone
  Date: 3/20/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Allocate a new material and initialize from an material camera.
  Arguments:
    src - material to be cloned
  Return:
    cloned material
  -------------------------------------------------------------------*/
AtrMaterial *atrMaterialClone( AtrMaterial *src ) {
    AtrMaterial *m;

#ifdef AT_DEBUGGING
    if ( !src )
        atuError( FXTRUE, "atrMaterialClone(): Invalid parameter.\n" );
#endif    

    m = atuMemCalloc( sizeof( AtrMaterial ), 1 );
    *m = *src;
    return m;
}

/*-------------------------------------------------------------------
  Function: atrMaterialSetup
  Date: 3/26/96
  Implementor(s): jdt
  Library: AT Render
  Description: 
    Setup a material with a simple type parameter.  
  Arguments:
    type - desired material effect
        ATR_MAT_GSHADE - set up simple gouraud shading
        ATR_MAT_DECAL  - set up decal texture
        ATR_MAT_EMAP   - set up environment mapping
        ATR_MAT_DECAL_X_LIGHTING - set up lit textures
        ATR_MAT_EMAP_X_LIGHTING - set up lit environment mapping
  Return:
  -------------------------------------------------------------------*/
void atrMaterialSetup( AtrMaterial *m, FxU32 type ) {
#ifdef AT_DEBUGGING
    if ( !m )
        atuError( FXTRUE, "atrMaterialSetup(): Invalid parameter\n" );
#endif

    /*-----------------------------------------------------------
      All materials that are set up here expect 
        OTHER -> TEXTURE INPUT ( 1, 0, TEXTURE )
        LOCAL -> LIGHT INPUT ( itrgb, crgb )
      -----------------------------------------------------------*/
    m->typeFlag  = type;
    m->ccuOther  = GR_COMBINE_OTHER_TEXTURE;
    m->depthMask = FXTRUE;
    m->iaSrc     = ATR_IASRC_NONE;

    m->tcuCInvert[0] = FXFALSE;
    m->tcuAInvert[0] = FXFALSE;
    m->tcuCInvert[1] = FXFALSE;
    m->tcuAInvert[1] = FXFALSE;
    m->texSClamp[0] = GR_TEXTURECLAMP_WRAP;
    m->texSClamp[1] = GR_TEXTURECLAMP_WRAP;
    m->texTClamp[0] = GR_TEXTURECLAMP_WRAP;
    m->texTClamp[1] = GR_TEXTURECLAMP_WRAP;
    m->texMMMode[0] = GR_MIPMAP_NEAREST;
    m->texMMMode[1] = GR_MIPMAP_NEAREST;
    m->texMinFilter[0] = GR_TEXTUREFILTER_BILINEAR;
    m->texMagFilter[0] = GR_TEXTUREFILTER_BILINEAR;
    m->texMinFilter[1] = GR_TEXTUREFILTER_BILINEAR;
    m->texMagFilter[1] = GR_TEXTUREFILTER_BILINEAR;
    m->texSrc[0]       = ATR_TEXSRC_NONE;
    m->texSrc[1]       = ATR_TEXSRC_NONE;

    switch( type & ATR_MAT_TEX_MASK ) {
    case ATR_MAT_TEX_NONE:
        m->texSrc[0]       = ATR_TEXSRC_NONE;
        m->tcSrc[0]        = ATR_TCSRC_NONE;
        m->tcuCFunction[0] = GR_COMBINE_FUNCTION_ZERO; 
        m->tcuCFactor[0]   = GR_COMBINE_FACTOR_ZERO;
        m->tcuAFunction[0] = GR_COMBINE_FUNCTION_ZERO;
        m->tcuAFactor[0]   = GR_COMBINE_FACTOR_ZERO;
        m->texLODBlend[0]  = FXFALSE;
        if ( _atrDriver->caps.numTex == 2 ) {
            m->texSrc[1]       = ATR_TEXSRC_NONE;
            m->tcSrc[1]        = ATR_TCSRC_NONE;
            m->tcuCFunction[1] = GR_COMBINE_FUNCTION_ZERO; 
            m->tcuCFactor[1]   = GR_COMBINE_FACTOR_ZERO;
            m->tcuAFunction[1] = GR_COMBINE_FUNCTION_ZERO;
            m->tcuAFactor[1]   = GR_COMBINE_FACTOR_ZERO;
            m->texLODBlend[1]  = FXFALSE;
        }
        break;
    case ATR_MAT_TEX_DECAL:
        m->texSrc[0]       = ATR_TEXSRC_DECAL;
        m->tcSrc[0]        = ATR_TCSRC_TC0;
        m->tcuCFunction[0] = GR_COMBINE_FUNCTION_LOCAL;
        m->tcuCFactor[0]   = GR_COMBINE_FACTOR_ZERO;
        m->tcuAFunction[0] = GR_COMBINE_FUNCTION_LOCAL;
        m->tcuAFactor[0]   = GR_COMBINE_FACTOR_ZERO;
        m->texLODBlend[0]  = FXFALSE;
        if ( _atrDriver->caps.numTex == 2 ) {
            m->texSrc[1]       = ATR_TEXSRC_NONE;
            m->tcSrc[1]        = ATR_TCSRC_NONE;
            m->tcuCFunction[1] = GR_COMBINE_FUNCTION_ZERO; 
            m->tcuCFactor[1]   = GR_COMBINE_FACTOR_ZERO;
            m->tcuAFunction[1] = GR_COMBINE_FUNCTION_ZERO;
            m->tcuAFactor[1]   = GR_COMBINE_FACTOR_ZERO;
            m->texLODBlend[1]  = FXFALSE;
        }
        break;
    case ATR_MAT_TEX_DECAL1:
        m->texSrc[0]       = ATR_TEXSRC_NONE;
        m->tcSrc[0]        = ATR_TCSRC_NONE;
        m->tcuCFunction[0] = GR_COMBINE_FUNCTION_SCALE_OTHER;
        m->tcuCFactor[0]   = GR_COMBINE_FACTOR_ONE;
        m->tcuAFunction[0] = GR_COMBINE_FUNCTION_SCALE_OTHER;
        m->tcuAFactor[0]   = GR_COMBINE_FACTOR_ONE;
        m->texLODBlend[0]  = FXFALSE;
        if ( _atrDriver->caps.numTex == 2 ) {
            m->texSrc[1]       = ATR_TEXSRC_DECAL;
            m->tcSrc[1]        = ATR_TCSRC_TC0;
            m->tcuCFunction[1] = GR_COMBINE_FUNCTION_LOCAL;
            m->tcuCFactor[1]   = GR_COMBINE_FACTOR_ZERO;
            m->tcuAFunction[1] = GR_COMBINE_FUNCTION_LOCAL;
            m->tcuAFactor[1]   = GR_COMBINE_FACTOR_ZERO;
            m->texLODBlend[1]  = FXFALSE;
        }
        break;
    case ATR_MAT_TEX_LODBLEND_PASS0:
        m->texSrc[0]       = ATR_TEXSRC_DECAL;
        m->tcSrc[0]        = ATR_TCSRC_TC0;
        m->tcuCFunction[0] = GR_COMBINE_FUNCTION_BLEND_LOCAL;
        m->tcuCFactor[0]   = GR_COMBINE_FACTOR_ONE_MINUS_LOD_FRACTION;
        m->tcuAFunction[0] = GR_COMBINE_FUNCTION_BLEND_LOCAL;
        m->tcuAFactor[0]   = GR_COMBINE_FACTOR_ONE_MINUS_LOD_FRACTION;
        m->texLODBlend[0]  = FXTRUE;
        if ( _atrDriver->caps.numTex == 2 ) {
            m->texSrc[1]       = ATR_TEXSRC_NONE;
            m->tcSrc[1]        = ATR_TCSRC_NONE;
            m->tcuCFunction[1] = GR_COMBINE_FUNCTION_ZERO; 
            m->tcuCFactor[1]   = GR_COMBINE_FACTOR_ZERO;
            m->tcuAFunction[1] = GR_COMBINE_FUNCTION_ZERO;
            m->tcuAFactor[1]   = GR_COMBINE_FACTOR_ZERO;
            m->texLODBlend[1]  = FXFALSE;
        }

        break;
    case ATR_MAT_TEX_LODBLEND_PASS1:
        m->texSrc[0]       = ATR_TEXSRC_DECAL;
        m->tcSrc[0]        = ATR_TCSRC_TC0;
        m->tcuCFunction[0] = GR_COMBINE_FUNCTION_BLEND_LOCAL;
        m->tcuCFactor[0]   = GR_COMBINE_FACTOR_LOD_FRACTION;
        m->tcuAFunction[0] = GR_COMBINE_FUNCTION_BLEND_LOCAL;
        m->tcuAFactor[0]   = GR_COMBINE_FACTOR_LOD_FRACTION;
        m->texLODBlend[0]  = FXTRUE;
        if ( _atrDriver->caps.numTex == 2 ) {
            m->texSrc[1]       = ATR_TEXSRC_NONE;
            m->tcSrc[1]        = ATR_TCSRC_NONE;
            m->tcuCFunction[1] = GR_COMBINE_FUNCTION_ZERO; 
            m->tcuCFactor[1]   = GR_COMBINE_FACTOR_ZERO;
            m->tcuAFunction[1] = GR_COMBINE_FUNCTION_ZERO;
            m->tcuAFactor[1]   = GR_COMBINE_FACTOR_ZERO;
            m->texLODBlend[1]  = FXFALSE;
        }
        break;
    case ATR_MAT_TEX_LODBLEND_SINGLEPASS:
        if ( _atrDriver->caps.numTex != 2 )
            atuError( FXTRUE, "atrMaterialSetup: Tried to up"
                      " multiple texelFx operation on single "
                      "texelFx system.\n" );
        m->texSrc[0]       = ATR_TEXSRC_DECAL;
        m->tcSrc[0]        = ATR_TCSRC_TC0;
        m->tcuCFunction[0] = GR_COMBINE_FUNCTION_BLEND;
        m->tcuCFactor[0]   = GR_COMBINE_FACTOR_LOD_FRACTION;
        m->tcuAFunction[0] = GR_COMBINE_FUNCTION_BLEND;
        m->tcuAFactor[0]   = GR_COMBINE_FACTOR_LOD_FRACTION;
        m->texLODBlend[0]  = FXFALSE;

        m->texSrc[1]       = ATR_TEXSRC_DECAL;
        m->tcSrc[1]        = ATR_TCSRC_TC0;
        m->tcuCFunction[1] = GR_COMBINE_FUNCTION_BLEND_LOCAL;
        m->tcuCFactor[1]   = GR_COMBINE_FACTOR_ONE_MINUS_LOD_FRACTION;
        m->tcuAFunction[1] = GR_COMBINE_FUNCTION_BLEND_LOCAL;
        m->tcuAFactor[1]   = GR_COMBINE_FACTOR_ONE_MINUS_LOD_FRACTION;
        m->texLODBlend[1]  = FXFALSE;
        break;
    case ATR_MAT_TEX_DETAIL_PASS0:
		// Denis. fixing detail texture 2 passes
        //m->texSrc[0]       = ATR_TEXSRC_DECAL;
        m->texSrc[0]       = ATR_TEXSRC_DETAIL;
        m->tcSrc[0]        = ATR_TCSRC_TC0;
        m->tcuCFunction[0] = GR_COMBINE_FUNCTION_BLEND_LOCAL,
        m->tcuCFactor[0]   = GR_COMBINE_FACTOR_DETAIL_FACTOR,
        m->tcuAFunction[0] = GR_COMBINE_FUNCTION_BLEND_LOCAL,
        m->tcuAFactor[0]   = GR_COMBINE_FACTOR_DETAIL_FACTOR,
        m->texLODBlend[0]  = FXFALSE;
        if ( _atrDriver->caps.numTex == 2 ) {
            m->texSrc[1]       = ATR_TEXSRC_NONE;
            m->tcSrc[1]        = ATR_TCSRC_NONE;
            m->tcuCFunction[1] = GR_COMBINE_FUNCTION_ZERO; 
            m->tcuCFactor[1]   = GR_COMBINE_FACTOR_ZERO;
            m->tcuAFunction[1] = GR_COMBINE_FUNCTION_ZERO;
            m->tcuAFactor[1]   = GR_COMBINE_FACTOR_ZERO;
            m->texLODBlend[1]  = FXFALSE;
        }
        break;
    case ATR_MAT_TEX_DETAIL_PASS1:
		// Denis. fixing detail texture 2 passes
        //m->texSrc[0]       = ATR_TEXSRC_DECAL;
        m->texSrc[0]       = ATR_TEXSRC_DETAIL;
        m->tcSrc[0]        = ATR_TCSRC_TC0_SCALE;
        m->tcuCFunction[0] = GR_COMBINE_FUNCTION_BLEND_LOCAL,
        m->tcuCFactor[0]   = GR_COMBINE_FACTOR_ONE_MINUS_DETAIL_FACTOR,
        m->tcuAFunction[0] = GR_COMBINE_FUNCTION_BLEND_LOCAL,
        m->tcuAFactor[0]   = GR_COMBINE_FACTOR_ONE_MINUS_DETAIL_FACTOR,
        m->texLODBlend[0]  = FXFALSE;
        if ( _atrDriver->caps.numTex == 2 ) {
            m->texSrc[1]       = ATR_TEXSRC_NONE;
            m->tcSrc[1]        = ATR_TCSRC_NONE;
            m->tcuCFunction[1] = GR_COMBINE_FUNCTION_ZERO; 
            m->tcuCFactor[1]   = GR_COMBINE_FACTOR_ZERO;
            m->tcuAFunction[1] = GR_COMBINE_FUNCTION_ZERO;
            m->tcuAFactor[1]   = GR_COMBINE_FACTOR_ZERO;
            m->texLODBlend[1]  = FXFALSE;
        }
        break;
    case ATR_MAT_TEX_DETAIL_SINGLEPASS:
        if ( _atrDriver->caps.numTex != 2 ) {
            atuError( FXTRUE, "atrMaterialSetup: Tried to up"
                      " multiple texelFx operation on single "
                      "texelFx system.\n" );
        }
        m->texSrc[0]          = ATR_TEXSRC_DETAIL;
		m->tcSrc[0]           = ATR_TCSRC_TC0;
        m->tcuCFunction[0]    = GR_COMBINE_FUNCTION_BLEND;
        m->tcuCFactor[0]      = GR_COMBINE_FACTOR_DETAIL_FACTOR,
        m->tcuAFunction[0]    = GR_COMBINE_FUNCTION_BLEND;
        m->tcuAFactor[0]      = GR_COMBINE_FACTOR_DETAIL_FACTOR;
        m->texLODBlend[0]     = FXFALSE;
        m->texDetail[0].bias  = 10;
        m->texDetail[0].scale = 2;
        m->texDetail[0].max   = .3f;
        m->texSrc[1]          = ATR_TEXSRC_DETAIL;
        // Denis. Fixing Detail Textures 
		//m->tcSrc[1]           = ATR_TCSRC_NONE;
		m->tcSrc[1]           = ATR_TCSRC_TC0_SRC1_SCALE_TC0;
        
        m->tcuCFunction[1]    = GR_COMBINE_FUNCTION_BLEND;
        m->tcuCFactor[1]      = GR_COMBINE_FACTOR_ONE_MINUS_DETAIL_FACTOR;
        m->tcuAFunction[1]    = GR_COMBINE_FUNCTION_BLEND;
        m->tcuAFactor[1]      = GR_COMBINE_FACTOR_ONE_MINUS_DETAIL_FACTOR;
        m->texLODBlend[1]     = FXFALSE;
        m->texDetail[1].bias  = 10;
        m->texDetail[1].scale = 2;
        m->texDetail[1].max   = .3f;

        
		//m->texSrc[0]       = ATR_TEXSRC_DECAL;
        //m->tcSrc[0]        = ATR_TCSRC_TC0;
		/*
		m->texSrc[0]       = ATR_TEXSRC_DECAL;
        m->tcSrc[0]        = ATR_TCSRC_TC0;
        m->tcuCFunction[0] = GR_COMBINE_FUNCTION_SCALE_OTHER;
        m->tcuCFactor[0]   = GR_COMBINE_FACTOR_LOCAL;
        m->tcuAFunction[0] = GR_COMBINE_FUNCTION_LOCAL;
        m->tcuAFactor[0]   = GR_COMBINE_FACTOR_ZERO;
        m->texLODBlend[0]  = FXFALSE;
		
        if ( _atrDriver->caps.numTex == 2 ) {
            m->texSrc[1]       = ATR_TEXSRC_PROJECTED;
            m->tcSrc[1]        = ATR_TCSRC_PLANAR;
            m->tcuCFunction[1] = GR_COMBINE_FUNCTION_LOCAL;
            m->tcuCFactor[1]   = GR_COMBINE_FACTOR_ZERO;
            m->tcuAFunction[1] = GR_COMBINE_FUNCTION_LOCAL;
            m->tcuAFactor[1]   = GR_COMBINE_FACTOR_ZERO;
            m->texLODBlend[1]  = FXFALSE;
            m->texSClamp[1]    = GR_TEXTURECLAMP_WRAP;
            m->texTClamp[1]    = GR_TEXTURECLAMP_WRAP;
        }
		*/

        break;
    case ATR_MAT_TEX_EMAP:
        m->texSrc[0]       = ATR_TEXSRC_EMAP;
        m->tcSrc[0]        = ATR_TCSRC_EMAP;
        m->tcuCFunction[0] = GR_COMBINE_FUNCTION_LOCAL;
        m->tcuCFactor[0]   = GR_COMBINE_FACTOR_ZERO;
        m->tcuAFunction[0] = GR_COMBINE_FUNCTION_LOCAL;
        m->tcuAFactor[0]   = GR_COMBINE_FACTOR_ZERO;
        m->texLODBlend[0]  = FXFALSE;
        if ( _atrDriver->caps.numTex == 2 ) {
            m->texSrc[1]       = ATR_TEXSRC_NONE;
            m->tcSrc[1]        = ATR_TCSRC_NONE;
            m->tcuCFunction[1] = GR_COMBINE_FUNCTION_ZERO; 
            m->tcuCFactor[1]   = GR_COMBINE_FACTOR_ZERO;
            m->tcuAFunction[1] = GR_COMBINE_FUNCTION_ZERO;
            m->tcuAFactor[1]   = GR_COMBINE_FACTOR_ZERO;
            m->texLODBlend[1]  = FXFALSE;
        }
        break;
    case ATR_MAT_TEX_LMAP:
        m->texSrc[0]       = ATR_TEXSRC_LMAP;
        m->tcSrc[0]        = ATR_TCSRC_LMAP;
        m->tcuCFunction[0] = GR_COMBINE_FUNCTION_LOCAL;
        m->tcuCFactor[0]   = GR_COMBINE_FACTOR_ZERO;
        m->tcuAFunction[0] = GR_COMBINE_FUNCTION_LOCAL;
        m->tcuAFactor[0]   = GR_COMBINE_FACTOR_ZERO;
        m->texLODBlend[0]  = FXFALSE;
        if ( _atrDriver->caps.numTex == 2 ) {
            m->texSrc[1]       = ATR_TEXSRC_NONE;
            m->tcSrc[1]        = ATR_TCSRC_NONE;
            m->tcuCFunction[1] = GR_COMBINE_FUNCTION_ZERO; 
            m->tcuCFactor[1]   = GR_COMBINE_FACTOR_ZERO;
            m->tcuAFunction[1] = GR_COMBINE_FUNCTION_ZERO;
            m->tcuAFactor[1]   = GR_COMBINE_FACTOR_ZERO;
            m->texLODBlend[1]  = FXFALSE;
        }

        break;
    case ATR_MAT_TEX_PROJECTED:
        m->texSrc[0]       = ATR_TEXSRC_PROJECTED;
        m->tcSrc[0]        = ATR_TCSRC_PROJECTED;
        m->tcuCFunction[0] = GR_COMBINE_FUNCTION_LOCAL;
        m->tcuCFactor[0]   = GR_COMBINE_FACTOR_ZERO;
        m->tcuAFunction[0] = GR_COMBINE_FUNCTION_LOCAL;
        m->tcuAFactor[0]   = GR_COMBINE_FACTOR_ZERO;
        m->texLODBlend[0]  = FXFALSE;
        m->texSClamp[0]    = GR_TEXTURECLAMP_CLAMP;
        m->texTClamp[0]    = GR_TEXTURECLAMP_CLAMP;
        if ( _atrDriver->caps.numTex == 2 ) {
            m->texSrc[1]       = ATR_TEXSRC_NONE;
            m->tcSrc[1]        = ATR_TCSRC_NONE;
            m->tcuCFunction[1] = GR_COMBINE_FUNCTION_ZERO; 
            m->tcuCFactor[1]   = GR_COMBINE_FACTOR_ZERO;
            m->tcuAFunction[1] = GR_COMBINE_FUNCTION_ZERO;
            m->tcuAFactor[1]   = GR_COMBINE_FACTOR_ZERO;
            m->texLODBlend[1]  = FXFALSE;
        }
        break;
    case ATR_MAT_TEX_DECAL_X_PROJECTED:
        m->texSrc[0]       = ATR_TEXSRC_DECAL;
        m->tcSrc[0]        = ATR_TCSRC_TC0;
        m->tcuCFunction[0] = GR_COMBINE_FUNCTION_SCALE_OTHER;
        m->tcuCFactor[0]   = GR_COMBINE_FACTOR_LOCAL;
        m->tcuAFunction[0] = GR_COMBINE_FUNCTION_LOCAL;
        m->tcuAFactor[0]   = GR_COMBINE_FACTOR_ZERO;
        m->texLODBlend[0]  = FXFALSE;
        if ( _atrDriver->caps.numTex == 2 ) {
            m->texSrc[1]       = ATR_TEXSRC_PROJECTED;
            m->tcSrc[1]        = ATR_TCSRC_PROJECTED;
            m->tcuCFunction[1] = GR_COMBINE_FUNCTION_LOCAL;
            m->tcuCFactor[1]   = GR_COMBINE_FACTOR_ZERO;
            m->tcuAFunction[1] = GR_COMBINE_FUNCTION_LOCAL;
            m->tcuAFactor[1]   = GR_COMBINE_FACTOR_ZERO;
            m->texLODBlend[1]  = FXFALSE;
            m->texSClamp[1]    = GR_TEXTURECLAMP_CLAMP;
            m->texTClamp[1]    = GR_TEXTURECLAMP_CLAMP;
        }
        break;
      case ATR_MAT_TEX_PLANAR:
        m->texSrc[0]       = ATR_TEXSRC_PROJECTED;
        m->tcSrc[0]        = ATR_TCSRC_PLANAR;
        m->tcuCFunction[0] = GR_COMBINE_FUNCTION_LOCAL;
        m->tcuCFactor[0]   = GR_COMBINE_FACTOR_ZERO;
        m->tcuAFunction[0] = GR_COMBINE_FUNCTION_LOCAL;
        m->tcuAFactor[0]   = GR_COMBINE_FACTOR_ZERO;
        m->texLODBlend[0]  = FXFALSE;
        m->texSClamp[0]    = GR_TEXTURECLAMP_WRAP;
        m->texTClamp[0]    = GR_TEXTURECLAMP_WRAP;
        if ( _atrDriver->caps.numTex == 2 ) {
            m->texSrc[1]       = ATR_TEXSRC_NONE;
            m->tcSrc[1]        = ATR_TCSRC_NONE;
            m->tcuCFunction[1] = GR_COMBINE_FUNCTION_ZERO; 
            m->tcuCFactor[1]   = GR_COMBINE_FACTOR_ZERO;
            m->tcuAFunction[1] = GR_COMBINE_FUNCTION_ZERO;
            m->tcuAFactor[1]   = GR_COMBINE_FACTOR_ZERO;
            m->texLODBlend[1]  = FXFALSE;
        }
        break;
      case ATR_MAT_TEX_PLANAR_1:
        if ( _atrDriver->caps.numTex == 2 ) {
			m->texSrc[1]       = ATR_TEXSRC_PROJECTED;
			m->tcSrc[1]        = ATR_TCSRC_PLANAR;
			m->tcuCFunction[1] = GR_COMBINE_FUNCTION_LOCAL;
			m->tcuCFactor[1]   = GR_COMBINE_FACTOR_ZERO;
			m->tcuAFunction[1] = GR_COMBINE_FUNCTION_LOCAL;
			m->tcuAFactor[1]   = GR_COMBINE_FACTOR_ZERO;
			m->texLODBlend[1]  = FXFALSE;
			m->texSClamp[1]    = GR_TEXTURECLAMP_WRAP;
			m->texTClamp[1]    = GR_TEXTURECLAMP_WRAP;
		}
        m->texSrc[0]       = ATR_TEXSRC_NONE;
        m->tcSrc[0]        = ATR_TCSRC_NONE;
        m->tcuCFunction[0] = GR_COMBINE_FUNCTION_SCALE_OTHER; 
        m->tcuCFactor[0]   = GR_COMBINE_FACTOR_ONE;
        m->tcuAFunction[0] = GR_COMBINE_FUNCTION_SCALE_OTHER;
        m->tcuAFactor[0]   = GR_COMBINE_FACTOR_ONE;
        m->texLODBlend[0]  = FXFALSE;
        break;
    case ATR_MAT_TEX_DECAL_X_PLANAR:
        m->texSrc[0]       = ATR_TEXSRC_DECAL;
        m->tcSrc[0]        = ATR_TCSRC_TC0;
        m->tcuCFunction[0] = GR_COMBINE_FUNCTION_SCALE_OTHER;
        m->tcuCFactor[0]   = GR_COMBINE_FACTOR_LOCAL;
        m->tcuAFunction[0] = GR_COMBINE_FUNCTION_LOCAL;
        m->tcuAFactor[0]   = GR_COMBINE_FACTOR_ZERO;
        m->texLODBlend[0]  = FXFALSE;
        if ( _atrDriver->caps.numTex == 2 ) {
            m->texSrc[1]       = ATR_TEXSRC_PROJECTED;
            m->tcSrc[1]        = ATR_TCSRC_PLANAR;
            m->tcuCFunction[1] = GR_COMBINE_FUNCTION_LOCAL;
            m->tcuCFactor[1]   = GR_COMBINE_FACTOR_ZERO;
            m->tcuAFunction[1] = GR_COMBINE_FUNCTION_LOCAL;
            m->tcuAFactor[1]   = GR_COMBINE_FACTOR_ZERO;
            m->texLODBlend[1]  = FXFALSE;
            m->texSClamp[1]    = GR_TEXTURECLAMP_WRAP;
            m->texTClamp[1]    = GR_TEXTURECLAMP_WRAP;
        }
        break;
    case ATR_MAT_TEX_DECAL_ADD_PLANAR:
        m->texSrc[0]       = ATR_TEXSRC_DECAL;
        m->tcSrc[0]        = ATR_TCSRC_TC0;
        m->tcuCFunction[0] = GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL;
        m->tcuCFactor[0]   = GR_COMBINE_FACTOR_ONE;
        m->tcuAFunction[0] = GR_COMBINE_FUNCTION_LOCAL;
        m->tcuAFactor[0]   = GR_COMBINE_FACTOR_ZERO;
        m->texLODBlend[0]  = FXFALSE;
        if ( _atrDriver->caps.numTex == 2 ) {
            m->texSrc[1]       = ATR_TEXSRC_PROJECTED;
            m->tcSrc[1]        = ATR_TCSRC_PLANAR;
            m->tcuCFunction[1] = GR_COMBINE_FUNCTION_LOCAL;
            m->tcuCFactor[1]   = GR_COMBINE_FACTOR_ZERO;
            m->tcuAFunction[1] = GR_COMBINE_FUNCTION_LOCAL;
            m->tcuAFactor[1]   = GR_COMBINE_FACTOR_ZERO;
            m->texLODBlend[1]  = FXFALSE;
            m->texSClamp[1]    = GR_TEXTURECLAMP_WRAP;
            m->texTClamp[1]    = GR_TEXTURECLAMP_WRAP;
        }
        break;
    default:
        atuError( FXTRUE, 
                  "atrMaterialSetup(): Unrecognized flag 0x%x\n",
                  type );
        break;
    }

    switch( type & ATR_MAT_LIGHTSRC_MASK ) {
      case ATR_MAT_LIGHTSRC_NONE:
        m->irgbSrc      = ATR_IRGBSRC_NONE;
        m->crgbSrc      = ATR_CRGBSRC_NONE;
        m->ccuLocal     = GR_COMBINE_LOCAL_NONE;
        m->color_combine= CC_LOCAL_NONE;
        break;
      case ATR_MAT_LIGHTSRC_LIGHT:
        m->crgbSrc      = ATR_CRGBSRC_NONE;
        m->irgbSrc      = ATR_IRGBSRC_LIGHTING;
        m->ccuLocal     = GR_COMBINE_LOCAL_ITERATED;
        m->color_combine= CC_LOCAL_ITERATED;
        break;
      case ATR_MAT_LIGHTSRC_STATIC:
        m->crgbSrc      = ATR_CRGBSRC_NONE;
        m->irgbSrc      = ATR_IRGBSRC_STATIC;
        m->ccuLocal     = GR_COMBINE_LOCAL_ITERATED;
        m->color_combine= CC_LOCAL_ITERATED;
        break;
      case ATR_MAT_LIGHTSRC_CONSTANT:
        m->crgbSrc      = ATR_CRGBSRC_STATIC;
        m->irgbSrc      = ATR_IRGBSRC_NONE;
        m->ccuLocal     = GR_COMBINE_LOCAL_CONSTANT;
        m->color_combine= CC_LOCAL_CONSTANT;
        break;
    }

    switch( type & ATR_MAT_LIGHTING_MASK ) {
    case ATR_MAT_LIGHTING_NONE:
        m->ccuFunction  = GR_COMBINE_FUNCTION_SCALE_OTHER;
        m->ccuFactor    = GR_COMBINE_FACTOR_ONE;
        m->ccuInvert    = FXFALSE;
        m->acuFunction  = GR_COMBINE_FUNCTION_SCALE_OTHER;
        m->acuFactor    = GR_COMBINE_FACTOR_ONE;
        m->acuLocal     = GR_COMBINE_LOCAL_NONE;
        m->acuOther     = GR_COMBINE_OTHER_TEXTURE;
        m->acuInvert    = FXFALSE;
        break;
    case ATR_MAT_LIGHTING_MULTIPLY:
        m->ccuFunction  = GR_COMBINE_FUNCTION_SCALE_OTHER;
        m->ccuFactor    = GR_COMBINE_FACTOR_LOCAL;
        m->ccuInvert    = FXFALSE;
        m->acuFunction  = GR_COMBINE_FUNCTION_SCALE_OTHER;
        m->acuFactor    = GR_COMBINE_FACTOR_ONE;
        m->acuLocal     = GR_COMBINE_LOCAL_NONE;
        m->acuOther     = GR_COMBINE_OTHER_TEXTURE;
        m->acuInvert    = FXFALSE;
        break;
    case ATR_MAT_LIGHTING_ADD:
        m->ccuFunction  = GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL;
        m->ccuFactor    = GR_COMBINE_FACTOR_ONE;
        m->ccuInvert    = FXFALSE;
        m->acuFunction  = GR_COMBINE_FUNCTION_SCALE_OTHER;
        m->acuFactor    = GR_COMBINE_FACTOR_ONE;
        m->acuLocal     = GR_COMBINE_LOCAL_NONE;
        m->acuOther     = GR_COMBINE_OTHER_TEXTURE;
        m->acuInvert    = FXFALSE;
        break;
    case ATR_MAT_LIGHTING_BLEND_ON_TEXALPHA:
        m->ccuFunction  = GR_COMBINE_FUNCTION_BLEND;
        m->ccuFactor    = GR_COMBINE_FACTOR_TEXTURE_ALPHA;
        m->ccuInvert    = FXFALSE;
        m->acuFunction  = GR_COMBINE_FUNCTION_SCALE_OTHER;
        m->acuFactor    = GR_COMBINE_FACTOR_ONE;
        m->acuLocal     = GR_COMBINE_LOCAL_NONE;
        m->acuOther     = GR_COMBINE_OTHER_TEXTURE;
        m->acuInvert    = FXFALSE;
        break;
    default:
        atuError( FXTRUE, 
                  "atrMaterialSetup(): Unrecognized flag 0x%x\n",
                  type );
        break;
    }

    switch( type & ATR_MAT_FB_MASK ) {
    case ATR_MAT_FB_ASSIGN:
        m->abuSrcFactor = GR_BLEND_ONE;
        m->abuDstFactor = GR_BLEND_ZERO;
        break;
    case ATR_MAT_FB_MULTIPLY:
        m->abuSrcFactor = GR_BLEND_ZERO;
        m->abuDstFactor = GR_BLEND_SRC_COLOR;
        break;
    case ATR_MAT_FB_ADD:
        m->abuSrcFactor = GR_BLEND_ONE;
        m->abuDstFactor = GR_BLEND_ONE;
        break;
    case ATR_MAT_FB_BLEND:
        m->abuSrcFactor = GR_BLEND_SRC_ALPHA;
        m->abuDstFactor = GR_BLEND_ONE_MINUS_SRC_ALPHA;
        break;
    case ATR_MAT_FB_ZERO:
        m->abuSrcFactor = GR_BLEND_ZERO;
        m->abuDstFactor = GR_BLEND_ZERO;
        break;
    case ATR_MAT_FB_MULT_MP_FOG:
        m->abuSrcFactor = GR_BLEND_ONE;
        m->abuDstFactor = GR_BLEND_PREFOG_COLOR;
        break;
    default:
        atuError( FXTRUE, 
                  "atrMaterialSetup(): Unrecognized flag 0x%x\n",
                  type );
        break;
    }

    atrMaterialModify(m);
    return;
}

static void atrGetTextureMode(AtrMaterial *m,FxU32 tmu);
static void atrGetColorCombineMode(AtrMaterial *m);
static void atrGetAlphaCombineMode(AtrMaterial *m);
static void atrGetAlphaBlendMode(AtrMaterial *m);

void atrMaterialModify(AtrMaterial *m)
{
   atrGetTextureMode(m,0);
   atrGetTextureMode(m,1);
   atrGetColorCombineMode(m);
   atrGetAlphaCombineMode(m);
   atrGetAlphaBlendMode(m);
}

static void atrGetTextureMode(AtrMaterial *m,FxU32 tmu)
{

#ifdef AT_DEBUGGING
   if (tmu>1)
      atuError(FXTRUE,"atrMaterialModify: tmu > 1\n");
#endif    

   if ((m->tcuCFunction[tmu] == GR_COMBINE_FUNCTION_ZERO)&&
       (m->tcuCFactor[tmu]   == GR_COMBINE_FACTOR_ZERO)&&
       (m->tcuAFunction[tmu] == GR_COMBINE_FUNCTION_ZERO)&&
       (m->tcuAFactor[tmu]   == GR_COMBINE_FACTOR_ZERO))
   {
      m->texture_combine[tmu]=TC_MODE_1;
   }
   else if ((m->tcuCFunction[tmu] == GR_COMBINE_FUNCTION_LOCAL)&&
            (m->tcuCFactor[tmu]   == GR_COMBINE_FACTOR_ZERO)&&
            (m->tcuAFunction[tmu] == GR_COMBINE_FUNCTION_LOCAL)&&
            (m->tcuAFactor[tmu]   == GR_COMBINE_FACTOR_ZERO))
   {
      m->texture_combine[tmu]=TC_MODE_2;
   }
   else if ((m->tcuCFunction[tmu] == GR_COMBINE_FUNCTION_SCALE_OTHER)&&
            (m->tcuCFactor[tmu]   == GR_COMBINE_FACTOR_ONE)&&
            (m->tcuAFunction[tmu] == GR_COMBINE_FUNCTION_SCALE_OTHER)&&
            (m->tcuAFactor[tmu]   == GR_COMBINE_FACTOR_ONE))
   {
      m->texture_combine[tmu]=TC_MODE_3;
   }
   else if ((m->tcuCFunction[tmu] == GR_COMBINE_FUNCTION_BLEND_LOCAL)&&
            (m->tcuCFactor[tmu]   == GR_COMBINE_FACTOR_ONE_MINUS_LOD_FRACTION)&&
            (m->tcuAFunction[tmu] == GR_COMBINE_FUNCTION_BLEND_LOCAL)&&
            (m->tcuAFactor[tmu]   == GR_COMBINE_FACTOR_ONE_MINUS_LOD_FRACTION))
   {
      m->texture_combine[tmu]=TC_MODE_4;
   }
   else if ((m->tcuCFunction[tmu] == GR_COMBINE_FUNCTION_BLEND_LOCAL)&&
            (m->tcuCFactor[tmu]   == GR_COMBINE_FACTOR_LOD_FRACTION)&&
            (m->tcuAFunction[tmu] == GR_COMBINE_FUNCTION_BLEND_LOCAL)&&
            (m->tcuAFactor[tmu]   == GR_COMBINE_FACTOR_LOD_FRACTION))
   {
      m->texture_combine[tmu]=TC_MODE_5;
   }
   else if ((m->tcuCFunction[tmu] == GR_COMBINE_FUNCTION_BLEND)&&
            (m->tcuCFactor[tmu]   == GR_COMBINE_FACTOR_LOD_FRACTION)&&
            (m->tcuAFunction[tmu] == GR_COMBINE_FUNCTION_BLEND)&&
            (m->tcuAFactor[tmu]   == GR_COMBINE_FACTOR_LOD_FRACTION))
   {
      m->texture_combine[tmu]=TC_MODE_6;
   }
   else if ((m->tcuCFunction[tmu] == GR_COMBINE_FUNCTION_BLEND_LOCAL)&&
            (m->tcuCFactor[tmu]   == GR_COMBINE_FACTOR_DETAIL_FACTOR)&&
            (m->tcuAFunction[tmu] == GR_COMBINE_FUNCTION_BLEND_LOCAL)&&
            (m->tcuAFactor[tmu]   == GR_COMBINE_FACTOR_DETAIL_FACTOR))
   {
      m->texture_combine[tmu]=TC_MODE_7;
   }
   else if ((m->tcuCFunction[tmu] == GR_COMBINE_FUNCTION_BLEND_LOCAL)&&
            (m->tcuCFactor[tmu]   == GR_COMBINE_FACTOR_ONE_MINUS_DETAIL_FACTOR)&&
            (m->tcuAFunction[tmu] == GR_COMBINE_FUNCTION_BLEND_LOCAL)&&
            (m->tcuAFactor[tmu]   == GR_COMBINE_FACTOR_ONE_MINUS_DETAIL_FACTOR))
   {
      m->texture_combine[tmu]=TC_MODE_8;
   }
   else if ((m->tcuCFunction[tmu] == GR_COMBINE_FUNCTION_BLEND)&&
            (m->tcuCFactor[tmu]   == GR_COMBINE_FACTOR_DETAIL_FACTOR)&&
            (m->tcuAFunction[tmu] == GR_COMBINE_FUNCTION_BLEND)&&
            (m->tcuAFactor[tmu]   == GR_COMBINE_FACTOR_DETAIL_FACTOR))
   {
      m->texture_combine[tmu]=TC_MODE_9;
   }
   else if ((m->tcuCFunction[tmu] == GR_COMBINE_FUNCTION_SCALE_OTHER)&&
            (m->tcuCFactor[tmu]   == GR_COMBINE_FACTOR_LOCAL)&&
            (m->tcuAFunction[tmu] == GR_COMBINE_FUNCTION_LOCAL)&&
            (m->tcuAFactor[tmu]   == GR_COMBINE_FACTOR_ZERO))
   {
      m->texture_combine[tmu]=TC_MODE_10;
   }
   else if ((m->tcuCFunction[tmu] == GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL)&&
            (m->tcuCFactor[tmu]   == GR_COMBINE_FACTOR_ONE)&&
            (m->tcuAFunction[tmu] == GR_COMBINE_FUNCTION_LOCAL)&&
            (m->tcuAFactor[tmu]   == GR_COMBINE_FACTOR_ZERO))
   {
      m->texture_combine[tmu]=TC_MODE_11;
   }
   else if ((m->tcuCFunction[tmu] == GR_COMBINE_FUNCTION_BLEND)&&
            (m->tcuCFactor[tmu]   == GR_COMBINE_FACTOR_ONE_MINUS_DETAIL_FACTOR)&&
            (m->tcuAFunction[tmu] == GR_COMBINE_FUNCTION_BLEND)&&
            (m->tcuAFactor[tmu]   == GR_COMBINE_FACTOR_ONE_MINUS_DETAIL_FACTOR))
   {
      m->texture_combine[tmu]=TC_MODE_12;
   }
   else
   {
#ifdef DEBUG
      atuError(FXFALSE,"atrMaterialModify: Unsupported texture mode\n");
#endif
      m->texture_combine[tmu]=TC_MODE_INVALID;
   }
   if ((m->tcuCInvert[tmu]==FXTRUE)||(m->tcuAInvert[tmu]==FXTRUE))
   {
#ifdef DEBUG
      atuError(FXTRUE,"atrMaterialModify: Unsupported texture mode\n");
#endif
      m->texture_combine[tmu]=TC_MODE_INVALID;
   }
/*
grTexCombine(GR_TMU1,
             m->tcuCFunction[1],
             m->tcuCFactor[1],
             m->tcuAFunction[1],
             m->tcuAFactor[1],
             m->tcuCInvert[1],
             m->tcuAInvert[1]);
*/
}

static void atrGetAlphaBlendMode(AtrMaterial *m)
{
   if ((m->abuSrcFactor==GR_BLEND_ONE)&&
       (m->abuDstFactor==GR_BLEND_ZERO))
   {
      m->alpha_blend=AB_MODE_1;
   }
   else if ((m->abuSrcFactor==GR_BLEND_ZERO)&&
            (m->abuDstFactor==GR_BLEND_SRC_COLOR))
   {
      m->alpha_blend=AB_MODE_2;
   }
   else if ((m->abuSrcFactor==GR_BLEND_ONE)&&
            (m->abuDstFactor==GR_BLEND_ONE))
   {
      m->alpha_blend=AB_MODE_3;
   }
   else if ((m->abuSrcFactor==GR_BLEND_SRC_ALPHA)&&
            (m->abuDstFactor==GR_BLEND_ONE_MINUS_SRC_ALPHA))
   {
      m->alpha_blend=AB_MODE_4;
   }
   else if ((m->abuSrcFactor==GR_BLEND_ZERO)&&
            (m->abuDstFactor==GR_BLEND_ZERO))
   {
      m->alpha_blend=AB_MODE_5;
   }
   else if ((m->abuSrcFactor==GR_BLEND_ONE)&&
            (m->abuDstFactor==GR_BLEND_PREFOG_COLOR))
   {
      m->alpha_blend=AB_MODE_6;
   }
   else
   {
      atuError(FXTRUE,"atrMaterialModify: Unsupported alpha blend mode\n");
   }
/*
grAlphaBlendFunction(m->abuSrcFactor,
                     m->abuDstFactor, 
                     GR_BLEND_ZERO,
                     GR_BLEND_ZERO );
*/
}

static void atrGetAlphaCombineMode(AtrMaterial *m)
{
   if ((m->acuFunction == GR_COMBINE_FUNCTION_SCALE_OTHER)&&
       (m->acuFactor   == GR_COMBINE_FACTOR_ONE)&&
       (m->acuLocal    == GR_COMBINE_LOCAL_NONE)&&
       (m->acuOther    == GR_COMBINE_OTHER_TEXTURE)&&
       (m->acuInvert   == FXFALSE))
   {
      m->alpha_combine=AC_MODE_1;
   }
   else
   {
#ifdef DEBUG
      atuError(FXFALSE,"atrMaterialModify: Unsupported alpha combine mode\n");
#endif
      m->alpha_combine=AC_MODE_INVALID;
   }
/*
grAlphaCombine(m->acuFunction,
               m->acuFactor,
               m->acuLocal,
               m->acuOther,
               m->acuInvert);
*/
}

static void atrGetColorCombineMode(AtrMaterial *m)
{
   if (m->ccuLocal==GR_COMBINE_LOCAL_NONE)
   {
      m->color_combine=CC_LOCAL_NONE;
   }
   else if (m->ccuLocal==GR_COMBINE_LOCAL_ITERATED)
   {
      m->color_combine=CC_LOCAL_ITERATED;
   }
   else if (m->ccuLocal==GR_COMBINE_LOCAL_CONSTANT)
   {
      m->color_combine=CC_LOCAL_CONSTANT;
   }
   else
   {
      atuError(FXTRUE,"atrMaterialModify: Unsupported color combine mode\n");
   }
   
   if ((m->ccuFunction == GR_COMBINE_FUNCTION_SCALE_OTHER)&&
       (m->ccuFactor   == GR_COMBINE_FACTOR_ONE)&&
       (m->ccuInvert   == FXFALSE))
   {
      m->color_combine+=CC_MODE_1;
   }
   else if ((m->ccuFunction == GR_COMBINE_FUNCTION_SCALE_OTHER)&&
            (m->ccuFactor   == GR_COMBINE_FACTOR_LOCAL)&&
            (m->ccuInvert   == FXFALSE))
   {
      m->color_combine+=CC_MODE_2;
   }
   else if ((m->ccuFunction == GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL)&&
            (m->ccuFactor   == GR_COMBINE_FACTOR_ONE)&&
            (m->ccuInvert   == FXFALSE))
   {
      m->color_combine+=CC_MODE_3;
   }
   else if ((m->ccuFunction == GR_COMBINE_FUNCTION_BLEND)&&
            (m->ccuFactor   == GR_COMBINE_FACTOR_TEXTURE_ALPHA)&&
            (m->ccuInvert   == FXFALSE))
   {
      m->color_combine+=CC_MODE_4;
   }
   else if ((m->ccuFunction == GR_COMBINE_FUNCTION_LOCAL)&&
            (m->ccuFactor   == GR_COMBINE_FACTOR_NONE)&&
            (m->ccuInvert   == FXFALSE))
   {
      m->color_combine+=CC_MODE_5;
   }
   else
   {
      atuError(FXTRUE,"atrMaterialModify: Unsupported color combine mode\n");
   }

   /*
   if (m->ccuOther  != GR_COMBINE_OTHER_TEXTURE)
   {
      atuError(FXTRUE,"atrMaterialModify: Unsupported color combine mode\n");
   }
   */
/*
grColorCombine(m->ccuFunction,
               m->ccuFactor,
               m->ccuLocal, 
               m->ccuOther,
               m->ccuInvert );
*/
}
