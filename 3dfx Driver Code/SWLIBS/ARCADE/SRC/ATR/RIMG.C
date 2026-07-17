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
** $Date: 10/11/00 7:34:07 PM$ 
**
*/


#include "atrender.h"
#include "fxatr.h"
#include "texusint.h"

#include <glide.h>
#include <string.h>

/*
**-----------------------------------------------------------------
** File Scope Data
**-----------------------------------------------------------------
*/

static const char  _dataType[]     = "AtrImg";
static const FxU32 _binaryRevision = 0;

static const char *_fmtString[] = 
{
    "RGB_332",
    "YIQ_422",
    "A_8",
    "I_8",
    "A_I_44",
    "P_8",
    "reserved",
    "reserved",
    "ARGB_8332",
    "AYIQ_8422",
    "RGB_565",
    "ARGB_1555",
    "ARGB_4444",
    "A_I_88",
    "AP_88",
    "reserved",
    "ARGB_8888"
};

/*-------------------------------------------------------------------
  Function: _imgFmtToString
  Date: 6/26
  Implementor(s): jdt
  Library: AT Render
  Description:
    private function that maps formats to constant strings
    Arguments:
      format
  Return:
      pointer to string 
  -------------------------------------------------------------------*/
static const char *_imgFmtToString( AtrImgFormat fmt ) {
#ifdef AT_DEBUGGING
    if ( fmt > 0xe )
      atuError( FXTRUE, "_imgFmtToString: bad format.\n" );
#endif
    return _fmtString[fmt];
}


/*-------------------------------------------------------------------
  Function: atrImgAllocate
  Date: 6/26
  Implementor(s): jdt
  Library: AT Render
  Description: 
    Allocate an array of iniatialzed image data structures.
  Arguments:
    num - number of image structures to allocate.
  Return:
    pointer to array of new structures.
  -------------------------------------------------------------------*/
AtrImg *atrImgAllocate( FxU32 num ) {
    FxU32  *n;
    AtrImg *i;
    FxU32 image;

#ifdef AT_DEBUGGING
    if ( !num ) 
      atuError( FXTRUE, "atuImgAllocate: Tried to allocate 0 img structures.\n" );
#endif

    n = atuMemCalloc( sizeof( AtrImg ) * num + sizeof( FxU32 ), 1 );
    atuMemRef( n );
    *n = num;
    i = (AtrImg*)(n+1);
    for( image = 0; image < num; image++ ) {
        atrImgDefault( i + image );
    }
    return i;
}

/*-------------------------------------------------------------------
  Function: atrImgDeallocate
  Date: 6/26
  Implementor(s): jdt
  Library: AT Render
  Description:
    Deallocate an array of images allocated with atrImgAllocate
  Arguments:
    i - array of images
  Return: 
    none
  -------------------------------------------------------------------*/
void atrImgDeallocate( AtrImg *i ) {
    FxU32 *n;
    FxU32 num, image;

#ifdef AT_DEBUGGING
    if ( !i ) 
      atuError( FXTRUE, "atuImgDeallocate:  invalid parameter.\n" );
#endif

    n = ( FxU32 * ) i;
    n--;
    num = *n;
    for( image = 0; image < num; image++ ) {
        if ( i[image].data ) atuMemDerefFree( i[image].data );
        if ( i[image].table ) atuMemDerefFree( i[image].table );
        if ( i[image].devPrivate ) DRV_FUNC(UnrealizeImg)( &i[image] );
    }
    atuMemFree( n );
    return;
}

/*-------------------------------------------------------------------
  Function: atrImgDefault
  Date: 6/26
  Implementor(s): jdt
  Library: AT Render
  Description:
    Return an image to its default initialization state.
  Arguments:
    i - image to initialize
  Return:
    none
  -------------------------------------------------------------------*/
void atrImgDefault( AtrImg *i ) {
#ifdef AT_DEBUGGING
    if ( !i ) 
      atuError( FXTRUE, "atuImgDefault:  invalid parameter.\n" );
#endif
    i->format = ATR_IMGFMT_NONE;
    i->width  = 0;
    i->height = 0;
    i->nLevels = 1;
    i->name = 0;
    if ( i->data ) {
        atuMemDerefFree( i->data );
        i->data = 0;
    }
    if ( i->table ) {
        atuMemDerefFree( i->table );
        i->table = 0;
    }
    i->devPrivate = NULL;
    return;
}

/*-------------------------------------------------------------------
  Function: _atrImgCalcSize
  Date: 6/26
  Implementor(s): jdt
  Library: AT Render
  Description:
    Calculate the size of an image based on a description in
    and AtrImg structure.
  Arguments:
    i - atr img structure containing dimensions of image
  Return:
    count in bytes of image data size
  -------------------------------------------------------------------*/
static FxU32 _atrImgCalcSize( const AtrImg *i ) {
    FxU32 lvl;
    FxU32 width;
    FxU32 height;
    FxU32 size;
    FxU32 bpp;

    lvl    = 0;
    size   = 0;
    width  = i->width;
    height = i->height;

    if ( i->format < ATR_IMGFMT_16_BIT )
      bpp = 1;
    else if ( i->format < ATR_IMGFMT_32_BIT )
      bpp = 2;
    else
      bpp = 4;

    while( lvl < i->nLevels ) {
        size   += ( width * height * bpp );
        width  = (width + 1)  >> 1;
        height = (height + 1) >> 1;
        lvl++;
    }
    return size;
}

/*-------------------------------------------------------------------
  Function: atrImgStore
  Date: 6/26
  Implementor(s): jdt
  Library: AT Render
  Description:
    Store an image out to an I/O stream
  Arguments:
   i - image to store
   stream - stream to write image to 
  Return:
    FXTRUE - success
    FXFALSE - failure
  -------------------------------------------------------------------*/
FxBool atrImgStore( const AtrImg *i, FILE *stream ) {
    FxU32 count;
    FxU32 size = 0;
#ifdef AT_DEBUGGING
    if ( !i || !stream ) 
        atuError( FXTRUE, "atrImgStore(): Invalid parameter.\n" );
#endif    
    /* Header */
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
    /* Struct Data */
    count = fwrite( i, sizeof( AtrImg ), 1, stream );
    if ( count != 1 ) return FXFALSE;

    /* name */
    if ( i->name )
      size = strlen( i->name ) + 1;
    else 
      size = strlen( "no_name" ) + 1;

    count = fwrite( &size, sizeof( size ), 1, stream );
    if ( count != 1 ) return FXFALSE;
    
    if ( i->name )
      count = fwrite( i->name, sizeof( char ) * size, 1, stream );
    else 
      count = fwrite( "no_name", sizeof( char ) * size, 1, stream );
    if ( count != 1 ) return FXFALSE;

    /* Image Data */
    if ( i->data ) {
        count = fwrite( i->data, _atrImgCalcSize( i ), 1, stream );
        if ( count != 1 ) return FXFALSE;
    }
    /* Table Data */
    if ( i->table ) {
        count = fwrite( i->table, sizeof( AtrImgTable ), 1, stream );
        if ( count != 1 ) return FXFALSE;
    }

    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: atrImgLoad
  Date: 6/26
  Implementor(s): jdt
  Library: AT Render
  Description:
    load an image from an I/O stream 
  Arguments:
    i - image structure to fill with data from stream
    stream - i/o stream from which to read image
  Return:
    FXTRUE - success
    FXFALSE - failure
  -------------------------------------------------------------------*/
FxBool atrImgLoad( AtrImg *i, FILE *stream ) {
    FxU32 rev;
    char  type[64];
    FxU32 count;
    FxU32 size;

#ifdef AT_DEBUGGING
    if ( !i || !stream ) 
        atuError( FXTRUE, "atrImgLoad(): Invalid parameter.\n" );
#endif    

     /* Header */
   count = fread( &rev, sizeof( _binaryRevision ), 1, stream );
    if ( count != 1 ) return FXFALSE;
    if ( rev != _binaryRevision ) return FXFALSE;
    count = fread( type, sizeof( _dataType ), 1, stream );
    if ( count != 1 ) return FXFALSE;
    if ( strcmp( type, _dataType ) ) return FXFALSE;

    /* Struct Data */
    count = fread( i, sizeof( AtrImg ), 1, stream );
    if ( count != 1 ) return FXFALSE;

    /* name data */
    count = fread( &size, sizeof( size ), 1, stream );
    if ( count != 1 ) return FXFALSE;

    i->name = atuMemCalloc( sizeof( char ), size );
    
    count = fread( i->name, sizeof( char ) * size, 1, stream );
    if ( count != 1 ) return FXFALSE;

    /* Image Data */
    if ( i->data ) {
        i->data = atuMemCalloc( _atrImgCalcSize( i ), 1 );
        count = fread( i->data, _atrImgCalcSize( i ), 1, stream );
        if ( count != 1 ) return FXFALSE;
    }
    /* Table Data */
    if ( i->table ) {
        i->table = atuMemCalloc( sizeof( AtrImgTable ), 1 );
        count = fread( i->table, sizeof( AtrImgTable ), 1, stream );
        if ( count != 1 ) return FXFALSE;
    }

    i->name = 0;
    i->devPrivate = NULL;
    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: atrImgCreateFromFile
  Date: 10/13
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Create an image from a file
    AtuLoadPath is search for the filename provided.
  Arguments:
    i - image structure to fill
    filename - filename to read
  Return:
    none
  -------------------------------------------------------------------*/

FxBool atrImgCreateFromFile(AtrImg *i, const char *filename) {
    char full_path[512];
    char   tmp[256];
    TxMip txMip;

    strcpy( tmp, filename );

    /* if no file type is specified default to 3df */

    if ( !strchr( tmp, '.' ) ) {
        strcat( tmp, ".3df" );
    }

    if ( atuFileLocate(tmp, full_path) == NULL ) {
        return FXFALSE;
    }

    if (!txMipRead(&txMip, full_path, GR_TEXFMT_ANY)) {
		atuError(FXFALSE, "can't load image %s\n", filename);
		return FXFALSE;
	}
  
    if (  i->data ) {
        atuMemDerefFree( i->data );
        i->data = 0;
    }

    if ( i->table ) {
        atuMemDerefFree( i->table );
        i->table = 0;
    }

    if ( i->devPrivate ) {
        DRV_FUNC(UnrealizeImg)( i );
    }

    i->format  = txMip.format;
    i->width = txMip.width;
	i->height = txMip.height;
    i->nLevels = txMip.depth;
    i->name = strdup(filename);
    i->data    = txMip.data[0];

    /* initialize palette if it has one */

    switch( i->format ) {
    case ATR_IMGFMT_YIQ_422:
    case ATR_IMGFMT_AYIQ_8422:
      i->table = atuMemMalloc( sizeof( AtrImgTable ) );
      txPalToNcc((GuNccTable *)&i->table->nccTable, txMip.pal);
      break;
    case ATR_IMGFMT_P_8:
    case ATR_IMGFMT_AP_88:
      i->table = atuMemMalloc( sizeof( AtrImgTable ) );
      atuMemRef( i->table );
      memcpy(i->table->palette, txMip.pal, sizeof(AtrPalette));
      break;
    default:
      i->table = 0;
    }

    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: atrImgClone
  Date: 6/26
  Implementor(s): jdt
  Library: AT Render
  Description:
    Allocate a new memory copy of an image.
  Arguments:
    src - image to clone
  Return:
    newly allocated clone of image
  -------------------------------------------------------------------*/
AtrImg *atrImgClone( const AtrImg *src ) {
    AtrImg *dst;

#ifdef AT_DEBUGGING
    if ( !src ) 
      atuError( FXTRUE, "atrImgClone: invalid parameter.\n" );
#endif

    dst = atrImgAllocate( 1 );
    *dst = *src;
    if ( src->data ) {
        dst->data = atuMemMalloc( _atrImgCalcSize( src ) );
        atuMemRef( dst->data );
        memcpy( dst->data, src->data, _atrImgCalcSize( src ) );
    }
    if ( src->table ) {
        dst->table = atuMemMalloc( sizeof( AtrImgTable ) );
        atuMemRef( dst->table );
        *dst->table = *src->table;
    }
    if ( src->devPrivate ) {
        DRV_FUNC(CloneImg)( dst, src );
    }
    return dst;
}

/*-------------------------------------------------------------------
  Function: atrImgAssign
  Date: 6/26
  Implementor(s): jdt
  Library: AT Render
  Description:
    Assign one image to another pre-allocate image structure
  Arguments:
    dst - destination memory region for image
    src - source image
  Return:
    none
  -------------------------------------------------------------------*/
void atrImgAssign( AtrImg *dst, const AtrImg *src ) {
#ifdef AT_DEBUGGING
    if ( !src || !dst ) 
      atuError( FXTRUE, "atrImgAssign: invalid parameter.\n" );
#endif

    if ( dst->data ) {
      atuMemDerefFree( dst->data );
      dst->data = 0;
    }
    if ( dst->table ) {
      atuMemDerefFree( dst->table );
      dst->table = 0;
    }

    if ( src->data ) {
        dst->data = atuMemMalloc( _atrImgCalcSize( src ) );
        atuMemRef( dst->data );
        memcpy( dst->data, src->data, _atrImgCalcSize( src ) );
    }
    if ( src->table ) {
        dst->table = atuMemMalloc( sizeof( AtrImgTable ) );
        atuMemRef( dst->table );
        *dst->table = *src->table;
    }
    return;
}

/*-------------------------------------------------------------------
  Function: atrImgPrint
  Date: 6/26
  Implementor(s): jdt
  Library: AT Render
  Description:
    Print out brief debugging information for an image.
  Arguments:
    i - image to describe
    stream - stream to output ascii
    indent - level of indentation of output
  Return:
    none
  -------------------------------------------------------------------*/
void atrImgPrint( const AtrImg *i,
                  FILE   *stream, 
                  FxU32  indent ) {
#ifdef AT_DEBUGGING
    if ( !i || !stream )
      atuError( FXTRUE, "atrImgPrint: invalid parameter.\n" );
#endif

    fprintf( stream, "%*sImg:\n", indent, " " );
    fprintf( stream, 
             "%*sformat: %s\n", 
             indent, 
             " ", 
             _imgFmtToString(i->format) );
    fprintf( stream, 
            "%*sw: %d h: %d\n", 
            indent, " ", 
            i->width, i->height );
    fprintf( stream,
            "%*slevels: %d\n", 
            indent, " ", i->nLevels );
    return;
}
