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
** $Date: 10/11/00 7:34:50 PM$ 
**
** TBD: need to modify converters to support either TMU.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "atscenep.h"
#include "texusint.h"
#include "texus.h"

static AtsTMUFunc _tmuFunc = NULL;
static numTextures = 0;
static AtsType texture_type;
AtsType *_ats_texture_type = &texture_type;

static const FxU32 _binaryRevision = 2;

#define TEXTURE_INC 100

typedef struct {
  AtsTexture *texture;
  char *filename;
} _TextureInfo;

static int num_textures = 0;
static _TextureInfo *textures;
static max_num_textures;

#define IMG_EXT 5

static void TextureExtendImageArray(AtsTexture *t, int n) {

    if (( t->numImages+n ) > t->maxImages ) {
        t->maxImages += n+IMG_EXT;
        t->img = (AtrImg **)atuMemRealloc(t->img,t->maxImages*sizeof(AtrImg *));

        if ( t->img == NULL ) {
            atuError(FXTRUE, "AtsTexture: could not extend image array\n");
        }
    }
}

/*-------------------------------------------------------------------
  Function: atsTextureAddImage
  Date: 7/31/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Add an image to a texture object
  Arguments:
    tex - the texture
    img - the image
  Return:
    Nothing
  -------------------------------------------------------------------*/

void 
atsTextureAddImage(AtsObject *obj, AtrImg *img) {
    AtsTexture *t;

    VALIDATE_TEXTURE(t, obj, "TextureAddImage"); 

    TextureExtendImageArray(t, 1);
    t->img[t->numImages++] = img;
    atuMemRef(img);
}

/*-------------------------------------------------------------------
  Function: atsTextureUpdate
  Date: 7/31/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Advance to the next image in a texture movie, wraps to zero if
    we are at the end of the image sequence.
  Arguments:
    tex - the texture
  Return:
    FXTRUE if we wrapped, FXFALSE otherwise
  -------------------------------------------------------------------*/

FxBool 
atsTextureUpdate( AtsObject *obj) {
    AtsTexture *t;
    FxBool result = FXFALSE;

    VALIDATE_TEXTURE(t, obj, "TextureUpdate"); 

    t->curImage++;

    if ( t->curImage >= t->numImages ) {
        t->curImage = 0;
        result = FXTRUE;
    }

    atrTexAssociate( t->handle, t->img[t->curImage] );

    return result;
}

/*-------------------------------------------------------------------
  Function: atsTMUFunc
  Date: 7/31/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Specify a callback function to assign a tmu to associate with a
    texture.
  Arguments:
    tex - the texture
  Return:
    Nothing
  -------------------------------------------------------------------*/

void 
atsTMUFunc( AtsTMUFunc func ) {

    _tmuFunc = func;
}

/*-------------------------------------------------------------------
  Function: atsTextureTMU
  Date: 7/31/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Change the TMU a texture is associated with
  Arguments:
    tex - the texture
    tmu - the tmu
  Return:
    FXTRUE on success, FXFALSE on error
  -------------------------------------------------------------------*/

FxBool 
atsTextureTMU( AtsObject *obj, AtrTexelFx tmu ) {
    AtsTexture *t;
    AtrTexInfo tinfo;

    VALIDATE_TEXTURE(t, obj, "atsTextureTMU"); 

#ifdef AT_DEBUGGING
    if ((tmu != ATR_TEXELFX_0) && ( tmu != ATR_TEXELFX_1) &&
        (tmu != ATR_TEXELFX_BOTH)) {
        atuError(FXFALSE, "TextureTMU: invalid tmu %d\n", tmu);
        return FXFALSE;
    }
#endif

    atrTexInfo(t->handle, &tinfo);

    if ( tinfo.tmu != tmu ) {
        atrTexDeleteHandle(t->handle);
        t->handle = atrTexNewHandle( tmu );
    }

    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: atsTextureGetTMU
  Date: 7/31/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Query the tmu(s) associated with a texture
  Arguments:
    tex - the texture
  Return:
    the tmu(s) associated with a texture
  -------------------------------------------------------------------*/

AtrTexelFx 
atsTextureGetTMU( AtsObject *obj ) {
    AtsTexture *t;
    AtrTexInfo tinfo;

    VALIDATE_TEXTURE(t, obj, "atsTextureTMU"); 
    
    atrTexInfo(t->handle, &tinfo);

    return tinfo.tmu;
}

/*-------------------------------------------------------------------
  Function: atsTextureNumFrames
  Date: 7/29/96
  Implementor(s): mlwp, jdt
  Library: AT Scene Manager Library
  Description: 
    Return number of images in texture
  Arguments:
    tex - the texture
  Return:
    number of images in texture
  -------------------------------------------------------------------*/

int 
atsTextureNumFrames( AtsObject *obj ) {
    AtsTexture *t;

    VALIDATE_TEXTURE(t, obj, "TextureNumFrames"); 

    return t->numImages;
}

/*-------------------------------------------------------------------
  Function: atsTextureFrame
  Date: 7/29/96
  Implementor(s): mlwp, jdt
  Library: AT Scene Manager Library
  Description: 
    Set the current frame
  Arguments:
    tex   - the texture
    frame - which frame to display
  Return:
    Nothing
  -------------------------------------------------------------------*/

void 
atsTextureFrame( AtsObject *obj, int frame ) {
    AtsTexture *t;

    VALIDATE_TEXTURE(t, obj, "TextureNumFrames"); 

#ifdef AT_DEBUGGING
    if ((frame < 0 ) || ( frame >= t->numImages ))
        atuError(FXFALSE, "atsTextureFrame: texture out of range %d, max %d\n",
                 frame, t->numImages);
#endif

    t->curImage = frame;

    atrTexAssociate( t->handle, t->img[t->curImage] );
}

/*-------------------------------------------------------------------
  Function: atsTextureReset
  Date: 7/23/96
  Implementor(s): mlwp, jdt
  Library: AT Scene Manager Library
  Description: 
    reset to start of movie sequence
  Arguments:
    tex - the texture
  Return:
    Nothing
  -------------------------------------------------------------------*/

void 
atsTextureReset( AtsObject *obj) {
    AtsTexture *t;

    VALIDATE_TEXTURE(t, obj, "TextureReset"); 

    t->curImage = 0;

    atrTexAssociate( t->handle, t->img[t->curImage] );
}

/*-------------------------------------------------------------------
  Function: atsTextureDownload
  Date: 4/24/96
  Implementor(s): mlwp, jdt
  Library: AT Scene Manager Library
  Description: 
    Download a texture use a 3df or tga version if its available
  Arguments:
  Return:
    New texture pointer
  -------------------------------------------------------------------*/
AtsObject* 
atsTextureDownload( const char *filename_ ) {
    AtsTexture *texture;
    char filename[200];
    char *str = filename;
  
    strcpy( filename, filename_ );

    /*
     * First see if we have a 3df file of the same name 
     */

    str = filename+strlen(filename);
   
    while ( ( str != filename ) && ( *str != '.' ))
        str--;

    strcpy( str, ".3df" );

    if (( texture = (AtsTexture*)atsTextureCreateFromFile( filename )) == 0 ) {
        strcpy( str, ".tga" );

        if ((( texture = (AtsTexture*)atsTextureCreateFromFile( filename )) == 0 ) &&
           (( texture = (AtsTexture*)atsTextureCreateFromFile(filename_)) ==0)){
            atuError(FXTRUE, "Could not load texture %s\n", filename);
        }
    }

    return texture;
}

/*-------------------------------------------------------------------
  Function: atsTextureMovie
  Date: 7/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
   Create a texture from a sequence of movie images
  Arguments:
    filename - the filename template in the form of a sprintf format   
               e.g. "fire%d.3df". There should be 1 integer format
               descriptor which will be replaced by the current frame
               number.
    start    - start frame number
    end      - end frame number
  Return:
    New image
  -------------------------------------------------------------------*/
AtsObject* 
atsTextureMovie( const char *filename, int start, int end ) {
    AtsTexture *texture = atsTextureNew( );
    AtrImg *img;
    int i;
    char buff[256];
    
#ifdef AT_DEBUGGING
    if (( filename == NULL ) || ( start < 0 ) || ( end < start )) {
        atuError(FXTRUE, "atsTextureMovie: invalid arguments\n");
    }
#endif

    if ( texture == NULL ) { 
        atuError(FXTRUE, "atsTextureMovie: Could not create texture\n");
    }

    TextureExtendImageArray(texture, end-start+1);

    for ( i = start; i <= end; i++ ) {
        sprintf(buff, filename, i);

        if ((img = atsImageCreateFromFile( buff)) == NULL ) {
            atuError(FXTRUE, "atsTextureMovie: Could not load image %s\n",
                     buff);
        }
    
        atsTextureAddImage(texture, img);
    }
        
    atsTextureReset( texture );

    return texture;
}

/*-------------------------------------------------------------------
  Function: atsTextureCreateFromFile
  Date: 10/25/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
   Create a texture from a file
  Arguments:
    filename - the filename 
  Return:
    New texture
  -------------------------------------------------------------------*/
AtsObject *
atsTextureCreateFromFile( const char *fileName ) {
    AtsTexture   *tex;
    AtrImg       *img;

    if (( tex = atsTextureNew()) == NULL ) {
        atuError(FXTRUE, "Could not create texture\n");
    }

    img = atsImageCreateFromFile( fileName );
    atsTextureAddImage(tex, img);
    atrTexAssociate( tex->handle, img );

    return tex;
}

/*-------------------------------------------------------------------
  Function: atsTextureFindOrDownload
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Check if file already downloaded, if not download it
  Arguments:
    filename_  - texture file
    flags      - flags for texture
  Return:
    pointer to new texture
  -------------------------------------------------------------------*/

AtsObject *
atsTextureFindOrDownload( const char *filename ) { 
    int i;
    AtsTexture *texture;
  
    /*
     * Has the texture already been downloaded?  If so, then return a 
     * handle to it.
     */
    for ( i = 0; i < num_textures; i++ ) {
        if( atuStringCompare( filename, textures[i].filename ) ) {
            return textures[i].texture;
        }
    }

    /*
     * Sanity check to make sure we haven't exceeded the 
     * init time limit on the number of textures.
     */
    if( num_textures == max_num_textures ) {
        max_num_textures += TEXTURE_INC;
       if( !( textures = ( _TextureInfo * )atuMemRealloc( textures, 
                               sizeof( _TextureInfo ) * max_num_textures ) ) )
       atuError( FXTRUE, "Out of memory in atsTextureFindOrDownload\n" );
    }

    /*
     * Store the name of the texture and download the texture.
     */

    textures[num_textures].filename  = strdup( filename );
    texture = textures[num_textures++].texture = 
      atsTextureDownload( filename );
    return texture;
}

/*-------------------------------------------------------------------
  Function: atsTextureNew
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Create a new texture object
  Arguments:
    None
  Return:
    pointer to new texture
  -------------------------------------------------------------------*/

AtsObject *
_atsTextureNew( char *where, FxU32 line ) {
    return _atsNew(_ats_texture_type, where, line );
}

static void 
TextureInit(AtsObject *obj) {
    AtsTexture *t;
    AtrTexelFx tmu = ATR_TEXELFX_0;

    VALIDATE_TEXTURE(t, obj, "TextureInit"); 

    if ( _tmuFunc != NULL ) {
        tmu = (* _tmuFunc)();
#ifdef AT_DEBUGGING
        if ((tmu != ATR_TEXELFX_0) && ( tmu != ATR_TEXELFX_1) &&
            (tmu != ATR_TEXELFX_BOTH)) {
            atuError(FXTRUE, "TextureInit: invalid tmu %d\n", tmu);
        }
#endif
    }

    t->handle = atrTexNewHandle( tmu );

    numTextures++;
    t->img = NULL;
    t->numImages = 0;
    t->maxImages = 0;
    t->curImage = 0;

    ATS_PARENT_CALL(_ats_texture_type, Init)(obj);
}

static void 
TextureDelete(AtsObject *obj) {
    AtsTexture *m;

    VALIDATE_TEXTURE(m, obj, "TextureDelete"); 

    atuError( FXTRUE, "TextureDelete(): Not Implemented yet.\n" );

    atrTexDeleteHandle( m->handle ) ;
}

static AtsObject *
TextureClone(AtsObject* obj, FxU32 mode) {
    AtsTexture *msrc, *mdst = 0;

    VALIDATE_TEXTURE(msrc, obj, "TextureClone"); 

    switch ( mode ) {
      case ATS_CLONE_HIERARCHY: /* clone only the hierarchy */
        atsRef(obj);
        return obj;
      case ATS_CLONE_ALL:       /* clone the hierarchy and geometry */
        atuError(FXTRUE, "TextureClone not implemented\n");
        atuMemType(mdst, _ats_texture_type->index);
        return (AtsObject *)mdst;
      default:
        atuError( FXTRUE, "TextureClone(): unknown clone method %d\n", mode );
        return NULL;
    }
}

static FxBool 
TexturePrint(const AtsObject* obj, FILE *stream, FxU32 indent, FxU32 verbose) {
    AtsTexture *tex;
    AtrTexInfo info;

    FXUNUSED(verbose);

    VALIDATE_TEXTURE(tex, obj, "TexturePrint"); 

    atrTexInfo( tex->handle, &info );
    /* Need to determine relevant information to print */

    fprintf( stream, "%*sTexture: %s\n", indent, " ", 
            (info.img->name)?info.img->name:"no_name" );

    return FXTRUE;
}

static FxBool 
TextureLoad(AtsObject *obj, FILE *stream){
    AtsTexture *t;
    FxU32 version;
    int i;
    int numImages;
    AtrTexelFx tmu;
    void *id;

    VALIDATE_TEXTURE(t, obj, "TextureLoad"); 

#ifdef AT_DEBUGGING
    if ( !t || !stream )
        atuError( FXTRUE, "TextureLoad(): Invalid parameter.\n" );
#endif

    CHECK(atuRead32( &version, 1, stream ) );

    CHECK ( version == _binaryRevision ) ;

    CHECK( atuRead32( &tmu, 1, stream ));

    if ( _tmuFunc != NULL ) {
        tmu = (* _tmuFunc)();
    }

#ifdef AT_DEBUGGING
    if ((tmu != ATR_TEXELFX_0) && ( tmu != ATR_TEXELFX_1) &&
        (tmu != ATR_TEXELFX_BOTH)) {
        atuError(FXTRUE, "TextureLoad: invalid tmu %d\n", tmu);
    }
#endif

    t->handle = atrTexNewHandle( tmu );

    /* read all the images */
    
    CHECK ( atuRead32( &numImages, 1, stream ) );

    if ( numImages > 0 ) {
        TextureExtendImageArray(t, numImages);

        for ( i = 0; i < numImages; i++ ) {
            CHECK ( atuRead32( &id, 1, stream ));

            t->img[t->numImages] = id;
            t->numImages++;
        }
    }


    return FXTRUE;
}

static FxBool 
TextureStore(AtsObject *obj, FILE *stream) {
    AtsTexture *t;
    int i;
    AtrTexInfo tinfo;
    void *id;

    VALIDATE_TEXTURE(t, obj, "TextureStore"); 

#ifdef AT_DEBUGGING
    if ( !t || !stream )
        atuError( FXTRUE, "TextureStore(): Invalid parameter.\n" );
#endif

    CHECK( atuWrite32( &_binaryRevision, 1, stream ) );
  
    /* output tmu associated with this texture */
    
    atrTexInfo(t->handle, &tinfo);

#ifdef AT_DEBUGGING
    if ((tinfo.tmu != ATR_TEXELFX_0) && ( tinfo.tmu != ATR_TEXELFX_1) &&
        (tinfo.tmu != ATR_TEXELFX_BOTH)) {
        atuError(FXTRUE, "TextureStore: invalid tmu %d\n", tinfo.tmu);
    }
#endif

    CHECK ( atuWrite32( &tinfo.tmu, 1, stream ) );

    /* output all the images */
    
    CHECK ( atuWrite32( &t->numImages, 1, stream ) );

    for ( i = 0; i < t->numImages; i++ ) {
        id = atsPointerToID( t->img[i] );
        CHECK ( atuWrite32( &id, 1, stream ) );
    }

    return FXTRUE;
}

static void 
TextureEnumerateReferences(AtsObject *obj) {
    AtsTexture *t;
    int i;

    VALIDATE_TEXTURE(t, obj, "TextureEnumerateReferences"); 

    for ( i = 0; i < t->numImages; i++ ) {
        ATS_NODE_CALL(t->img[i], EnumerateReferences)(t->img[i]);
    }
}

static FxBool 
TextureFixup(AtsObject *obj) {
    AtsTexture *t;
    int i;

    VALIDATE_TEXTURE(t, obj, "TextureFixup"); 

    for ( i = 0; i < t->numImages; i++ ) {
        t->img[i] = (AtrImg *)atsIDToPointer( t->img[i] ) ;
        atuMemRef(t->img[i]);
    }

    if ( t->numImages > 0 )
        atrTexAssociate( t->handle, t->img[0] );

    return FXTRUE;
}

void 
_atsTextureInitClass(void) {
    atsObjectNewType(_ats_texture_type);
    _ats_texture_type->name = "Texture";
    _ats_texture_type->size = sizeof(AtsTexture);
    _ats_texture_type->parent = _ats_object_type;

    /* initialize methods */

    _ats_texture_type->Init   = TextureInit;
    _ats_texture_type->Delete = TextureDelete;
    _ats_texture_type->Clone  = TextureClone;
    _ats_texture_type->Print  = TexturePrint;
    _ats_texture_type->Load   = TextureLoad;
    _ats_texture_type->Fixup  = TextureFixup;
    _ats_texture_type->Store  = TextureStore;
    _ats_texture_type->EnumerateReferences = TextureEnumerateReferences;
    return ;
}


/*-------------------------------------------------------------------
  Function: _atsTextureInit
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Initialize texture Manager
  Arguments:
    None
  Return:
    FXTRUE if initialization successful
  -------------------------------------------------------------------*/

/*
 * Allocate space for information pertaining to a max number of textures.
 */

FxBool 
_atsTextureInit( ) {
    max_num_textures = TEXTURE_INC;

    if( !( textures = ( _TextureInfo * )atuMemMalloc( sizeof( _TextureInfo ) * max_num_textures ) ) ) {
        atuError(FXTRUE, "Not able to allocate textures in _atsTexturesInit\n");
        return FXFALSE;
    }
    return FXTRUE;
}

void _atsTextureTerm() {
    printf("num textures created %d\n", numTextures);
}
