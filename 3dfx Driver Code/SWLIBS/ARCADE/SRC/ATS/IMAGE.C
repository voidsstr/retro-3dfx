
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
** $Date: 10/11/00 7:34:48 PM$ 
**
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "atscenep.h"
#include "texusint.h"
#include "texus.h"

static AtsType image_type;
AtsType *_ats_image_type = &image_type;
static const char  _dataType[]     = "AtrImg";
static const FxU32 _binaryRevision = 1;

/*-------------------------------------------------------------------
  Function: _atsImgCalcSize
  Date: 10/25/96
  Implementor(s): mlwp
  Library: AT Scene Manager
  Description:
    Calculate the size of an image based on a description in
    and AtrImg structure.
  Arguments:
    i - atr img structure containing dimensions of image
  Return:
    count in bytes of image data size
  -------------------------------------------------------------------*/
static FxU32 _atsImgCalcSize( const AtrImg *i ) {
    FxU32 lvl;
    FxU32 width;
    FxU32 height;
    FxU32 size;

    lvl    = 0;
    size   = 0;
    width  = i->width;
    height = i->height;

    while( lvl < i->nLevels ) {
        size   += ( width * height );
        width  = (width + 1)  >> 1;
        height = (height + 1) >> 1;
        lvl++;
    }
    return size;
}

/* Write NCC table */
static FxBool
atsWriteNCCTable (AtrNCCTable *t, FILE *stream) {
    CHECK(atuWrite8(t->yRGB, 16, stream));
    CHECK(atuWrite16(t->iRGB, 12, stream));
    CHECK(atuWrite16(t->qRGB, 12, stream));
    CHECK(atuWrite32(t->packedData, 12, stream));

    return FXTRUE;
}

/* Read NCC table */
static FxBool
atsReadNCCTable (AtrNCCTable *t, FILE *stream) {
    CHECK(atuRead8(t->yRGB, 16, stream));
    CHECK(atuRead16(t->iRGB, 12, stream));
    CHECK(atuRead16(t->qRGB, 12, stream));
    CHECK(atuRead32(t->packedData, 12, stream));

    return FXTRUE;
}

static FxBool
atsWritePalTable (FxU32 *pal, FILE *stream) {
    CHECK(atuWrite32(pal, 256, stream));

    return FXTRUE;
}

static FxBool
atsReadPalTable (FxU32 *pal, FILE *stream) {
    CHECK(atuRead32(pal, 256, stream));

    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: ImgStore
  Date: 10/25/96
  Implementor(s): mlwp
  Library: AT Scene Manager
  Description:
    Store an image out to an I/O stream MSB first
  Arguments:
   i - image to store
   stream - stream to write image to 
  Return:
    FXTRUE - success
    FXFALSE - failure
  -------------------------------------------------------------------*/
FxBool 
atsImageStore(AtsObject *obj, FILE *stream) {
    FxU32 size ;
    char *name;
    AtrImg *i;

    VALIDATE_IMAGE(i, obj, "atsImageStore"); 

#ifdef AT_DEBUGGING
    if ( !i || !stream ) 
        atuError( FXTRUE, "atsImgStore(): Invalid parameter.\n" );
#endif    
    /* Header */
    CHECK(atuWrite32( &_binaryRevision, 1, stream ));
    CHECK(atuWrite8( _dataType, sizeof( _dataType ), stream ));
    CHECK(atuWrite32( &i->format, 1, stream ));
    CHECK(atuWrite32( &i->width, 1, stream ));
    CHECK(atuWrite32( &i->height, 1, stream ));
    CHECK(atuWrite32( &i->nLevels, 1, stream ));

    /* name */
    name = ( i->name ) ? i->name : "no name";
    size = strlen( name ) + 1;
    CHECK(atuWrite32( &size, 1, stream ));
    CHECK(atuWrite8( name, size, stream ));

    /* Image Data */
    size = ( i->data ) ? _atsImgCalcSize( i ) : 0 ;
    CHECK(atuWrite32( &size, 1, stream ));

    if ( size > 0 ) {
        if ( i->format < ATR_IMGFMT_16_BIT )
            CHECK(atuWrite8( i->data, size, stream))
        else if ( i->format < ATR_IMGFMT_32_BIT )
            CHECK(atuWrite16( i->data, size, stream))
        else
            CHECK(atuWrite32( i->data, size, stream));
    } 

    /* write out table data if necessary */

    if ((i->format == ATR_IMGFMT_YIQ_422) || 
        (i->format == ATR_IMGFMT_AYIQ_8422)) {
        CHECK(atsWriteNCCTable (&(i->table->nccTable), stream));
    } else if ((i->format == ATR_IMGFMT_P_8) || 
               (i->format == ATR_IMGFMT_AP_88)) {
        CHECK(atsWritePalTable (i->table->palette, stream));
    }

    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: ImgLoad
  Date: 10/25/96
  Implementor(s): mlwp
  Library: AT Scene Manager
  Description:
    load an image from an I/O stream MSB first
  Arguments:
    i - image structure to fill with data from stream
    stream - i/o stream from which to read image
  Return:
    FXTRUE - success
    FXFALSE - failure
  -------------------------------------------------------------------*/

FxBool 
atsImageLoad(AtsObject *obj, FILE *stream){
    FxU32 rev;
    char  type[64];
    FxU32 size;
    AtrImg *i;

    VALIDATE_IMAGE(i, obj, "atsImageLoad"); 

#ifdef AT_DEBUGGING
    if ( !i || !stream ) 
        atuError( FXTRUE, "atsImgLoad(): Invalid parameter.\n" );
#endif    

     /* Header */
    CHECK(atuRead32( &rev, 1, stream ));
    CHECK ( rev == _binaryRevision ) ;
    CHECK(atuRead8( type, sizeof( _dataType ), stream ));
    CHECK ( !strcmp( type, _dataType ) );

    /* Struct Data */
    CHECK(atuRead32( &i->format, 1, stream ));
    CHECK(atuRead32( &i->width, 1, stream ));
    CHECK(atuRead32( &i->height, 1, stream ));
    CHECK(atuRead32( &i->nLevels, 1, stream ));

    /* name data */
    CHECK(atuRead32( &size, 1, stream ));

    i->name = atuMemCalloc( sizeof( char ), size );
    
    CHECK(atuRead8( i->name, size, stream ));
  
    /* image data */

    CHECK(atuRead32( &size, 1, stream ));

    if ( size > 0 ) {
        if ( i->format < ATR_IMGFMT_16_BIT ) {
            i->data = atuMemCalloc( size, 1 );
            CHECK( i->data != NULL );
            CHECK(atuRead8(i->data, size, stream));
        } else if ( i->format < ATR_IMGFMT_32_BIT ) {
            i->data = atuMemCalloc( size, 2 );
            CHECK( i->data != NULL );
            CHECK(atuRead16(i->data, size, stream));
        } else {
            i->data = atuMemCalloc( size, 4 );
            CHECK( i->data != NULL );
            CHECK(atuRead32(i->data, size, stream));
        }
    } else i->data = NULL;

    /* read in table data if necessary */

    if ((i->format == ATR_IMGFMT_YIQ_422) || (i->format == ATR_IMGFMT_AYIQ_8422)) {
        i->table = atuMemCalloc( sizeof( AtrImgTable ), 1 );
        CHECK(atsReadNCCTable (&(i->table->nccTable), stream));
    } else if ((i->format == ATR_IMGFMT_P_8) || (i->format == ATR_IMGFMT_AP_88)) {
        i->table = atuMemCalloc( sizeof( AtrImgTable ), 1 );
        CHECK(atsReadPalTable (i->table->palette, stream));
    } else i->table = NULL;

    i->devPrivate = NULL;
    return FXTRUE;
}


AtsObject *atsImageCreateFromFile( const char *fileName ) {
    AtsObject   *img;

    if (( img = atsImageNew()) == NULL ) {
        atuError(FXTRUE, "Could not create image\n");
    }

    if (!atrImgCreateFromFile( (AtrImg *)img, fileName )) {
		atuError(FXTRUE, "Could not find image %s\n", fileName);
	}

    return img;
}

AtsObject *_atsImageNew( char *where, FxU32 line ) {
    return _atsNew(_ats_image_type, where, line );
}

static void ImageInit(AtsObject *obj) {
    AtrImg *img;

    VALIDATE_IMAGE(img, obj, "ImageInit"); 

    atrImgDefault(img);

    ATS_PARENT_CALL(_ats_image_type, Init)(obj);
}

static void ImageDelete(AtsObject *obj) {
    AtrImg *img;

    VALIDATE_IMAGE(img, obj, "ImageDelete"); 

    atuMemFree(obj);
}

static AtsObject *ImageClone(AtsObject* obj, FxU32 mode) {
    AtrImg *src, *dst = 0;

    VALIDATE_IMAGE(src, obj, "ImageClone"); 

    switch ( mode ) {
      case ATS_CLONE_HIERARCHY: /* clone only the hierarchy */
        atsRef(obj);
        return obj;
      case ATS_CLONE_ALL:       /* clone the hierarchy and geometry */
        dst = atrImgClone(src);
        atuMemType(dst, _ats_image_type->index);
        return (AtsObject *)dst;
      default:
        atuError( FXTRUE, "ImageClone(): unknown clone method %d\n", mode );
        return NULL;
    }
}

static FxBool ImagePrint(const AtsObject* obj, FILE *stream, 
                               FxU32 indent, FxU32 verbose) {
    AtrImg *img;

    FXUNUSED(verbose);

    VALIDATE_IMAGE(img, obj, "ImagePrint"); 

    atrImgPrint(img, stream, indent);

    return FXTRUE;
}

void _atsImageInitClass(void) {
    atsObjectNewType(_ats_image_type);
    _ats_image_type->name = "Image";
    _ats_image_type->size = sizeof(AtrImg);
    _ats_image_type->parent = _ats_object_type;

    /* initialize methods */

    _ats_image_type->Init   = ImageInit;
    _ats_image_type->Delete = ImageDelete;
    _ats_image_type->Clone  = ImageClone;
    _ats_image_type->Print  = ImagePrint;
    _ats_image_type->Load   = atsImageLoad;
    _ats_image_type->Store  = atsImageStore;
    return ;
}
