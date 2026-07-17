/*
 * The following copyright is included from the source code which this
 * code is based:
 */

/* tgatoppm.c - read a TrueVision Targa file and write a portable pixmap
**
** Partially based on tga2rast, version 1.0, by Ian MacPhedran.
**
** Copyright (C) 1989 by Jef Poskanzer.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

#include <stdlib.h>
#include <stdio.h>
#include <atutil.h>
#include "3dstga.h"

#define MAXCOLORS 16384

static int mapped, rlencoded;

typedef unsigned long pixel;

static pixel ColorMap[MAXCOLORS];
static int RLE_count = 0, RLE_flag = 0;

static void readtga ( FILE* ifp, struct ImageHeader* tgaP );
static void get_map_entry ( FILE* ifp, pixel* Value, int Size );
static void get_pixel ( FILE* ifp, int Size );
static unsigned char getbyte ( FILE* ifp );

static TgaImage *tgaimage;
static TgaPixel *tgascan;

typedef unsigned short pixval;

int ReadTga( const char *filename, TgaImage *tgaimage_ )
    {
    struct ImageHeader tga_head;
    int i;
    unsigned int temp1, temp2;
    FILE* ifp;
    int argn, debug, rows, cols, row, col, realrow, truerow, baserow;
    int maxval;
    pixel** pixels;
    char* usage = " [-debug] [tgafile]";

    tgaimage = tgaimage_;

    argn = 1;
    debug = 0;

    if( !( ifp = fopen( filename, "r" ) ) )
      {
	atuError(FXTRUE, "Not able to read tga file \"%s\"\n", filename );
      }

    /* Read the Targa file header. */
    readtga( ifp, &tga_head );
    rows = ( (int) tga_head.Height_lo ) + ( (int) tga_head.Height_hi ) * 256;
    cols = ( (int) tga_head.Width_lo ) + ( (int) tga_head.Width_hi ) * 256;

    tgaimage->width = cols;
    tgaimage->height = rows;
    
    if( !( tgaimage->data = ( TgaPixel * )malloc( sizeof( TgaPixel ) * ( long )tgaimage->width
						  * ( long )tgaimage->height ) ) )
      {
	atuError(FXTRUE, "Not able to allocate memory for tga image.\n" );
      }

    tgascan = tgaimage->data;

    switch ( tga_head.ImgType )
	{
	case TGA_Map:
	case TGA_RGB:
	case TGA_Mono:
	case TGA_RLEMap:
	case TGA_RLERGB:
	case TGA_RLEMono:
	  break;

	default:
	  atuError(FXTRUE, "unknown Targa image type %d", tga_head.ImgType );
	}

    if ( tga_head.ImgType == TGA_Map ||
	 tga_head.ImgType == TGA_RLEMap ||
	 tga_head.ImgType == TGA_CompMap ||
	 tga_head.ImgType == TGA_CompMap4 )
	{ /* Color-mapped image */
	if ( tga_head.CoMapType != 1 )
	  {
	    atuError(FXTRUE, "mapped image (type %d) with color map type != 1",
		     tga_head.ImgType );
	  }

	mapped = 1;
	/* Figure maxval from CoSize. */
	switch ( tga_head.CoSize )
	    {
	    case 8:
	    case 24:
	    case 32:
	    maxval = 255;
	    break;

	    case 15:
	    case 16:
	    maxval = 31;
	    break;

	    default:
	    atuError(FXTRUE, "unknown colormap pixel size - %d", tga_head.CoSize );
	    }
	}
    else
	{ /* Not colormap, so figure maxval from PixelSize. */
	mapped = 0;
	switch ( tga_head.PixelSize )
	    {
	    case 8:
	    case 24:
	    case 32:
	    maxval = 255;
	    break;

	    case 15:
	    case 16:
	    maxval = 31;
	    break;

	    default:
	    atuError(FXTRUE, "unknown pixel size - %d", tga_head.PixelSize );
	    }
	}

    /* If required, read the color map information. */
    if ( tga_head.CoMapType != 0 )
	{
	temp1 = tga_head.Index_lo + tga_head.Index_hi * 256;
	temp2 = tga_head.Length_lo + tga_head.Length_hi * 256;
	if ( ( temp1 + temp2 + 1 ) >= MAXCOLORS )
	  {
	    atuError(FXTRUE, "too many colors - %d", ( temp1 + temp2 + 1 ) );
	  }
	for ( i = temp1; i < ( temp1 + temp2 ); ++i )
	    get_map_entry( ifp, &ColorMap[i], (int) tga_head.CoSize );
	}

    /* Check run-length encoding. */
    if ( tga_head.ImgType == TGA_RLEMap ||
	 tga_head.ImgType == TGA_RLERGB ||
	 tga_head.ImgType == TGA_RLEMono )
	rlencoded = 1;
    else
	rlencoded = 0;

    /* Read the Targa file body and convert to portable format. */
    truerow = 0;
    baserow = 0;
    for ( row = 0; row < rows; ++row )
	{
	realrow = truerow;
	if ( tga_head.OrgBit == 0 )
	    realrow = rows - realrow - 1;

#if 0
	for ( col = 0; col < cols; ++col )
	    get_pixel( ifp, &(pixels[realrow][col]), (int) tga_head.PixelSize );
#endif
	for ( col = 0; col < cols; ++col )
	    get_pixel( ifp, (int) tga_head.PixelSize );

	if ( tga_head.IntrLve == TGA_IL_Four )
	    truerow += 4;
	else if ( tga_head.IntrLve == TGA_IL_Two )
	    truerow += 2;
	else
	    ++truerow;
	if ( truerow >= rows )
	    truerow = ++baserow;
	}
    fclose( ifp );

    return 1;
    }

static void
readtga( ifp, tgaP )
    FILE* ifp;
    struct ImageHeader* tgaP;
    {
    unsigned char flags;
    ImageIDField junk;

    tgaP->IDLength = getbyte( ifp );
    tgaP->CoMapType = getbyte( ifp );
    tgaP->ImgType = getbyte( ifp );
    tgaP->Index_lo = getbyte( ifp );
    tgaP->Index_hi = getbyte( ifp );
    tgaP->Length_lo = getbyte( ifp );
    tgaP->Length_hi = getbyte( ifp );
    tgaP->CoSize = getbyte( ifp );
    tgaP->X_org_lo = getbyte( ifp );
    tgaP->X_org_hi = getbyte( ifp );
    tgaP->Y_org_lo = getbyte( ifp );
    tgaP->Y_org_hi = getbyte( ifp );
    tgaP->Width_lo = getbyte( ifp );
    tgaP->Width_hi = getbyte( ifp );
    tgaP->Height_lo = getbyte( ifp );
    tgaP->Height_hi = getbyte( ifp );
    tgaP->PixelSize = getbyte( ifp );
    flags = getbyte( ifp );
    tgaP->AttBits = flags & 0xf;
    tgaP->Rsrvd = ( flags & 0x10 ) >> 4;
    tgaP->OrgBit = ( flags & 0x20 ) >> 5;
    tgaP->IntrLve = ( flags & 0xc0 ) >> 6;

    if ( tgaP->IDLength != 0 )
	fread( junk, 1, (int) tgaP->IDLength, ifp );
    }

static void
get_map_entry( ifp, Value, Size )
    FILE* ifp;
    pixel* Value;
    int Size;
    {
    unsigned char j, k, r, g, b;

    /* Read appropriate number of bytes, break into rgb & put in map. */
    switch ( Size )
	{
	case 8:				/* Grey scale, read and triplicate. */
	r = g = b = getbyte( ifp );
	break;

	case 16:			/* 5 bits each of red green and blue. */
	case 15:			/* Watch for byte order. */
	j = getbyte( ifp );
	k = getbyte( ifp );
	r = ( k & 0x7C ) >> 2;
	g = ( ( k & 0x03 ) << 3 ) + ( ( j & 0xE0 ) >> 5 );
	b = j & 0x1F;
	break;

	case 32:
	case 24:			/* 8 bits each of blue green and red. */
	b = getbyte( ifp );
	g = getbyte( ifp );
	r = getbyte( ifp );
	if ( Size == 32 )
	    (void) getbyte( ifp );	/* Read alpha byte & throw away. */
	break;

	default:
	atuError(FXTRUE, "unknown colormap pixel size (#2) - %d", Size );
	}
    }

static void
get_pixel( ifp, Size )
    FILE* ifp;
    int Size;
    {
    static pixval Red, Grn, Blu, Alpha;
    unsigned char j, k;
    static unsigned int l;

    /* Check if run length encoded. */
    if ( rlencoded )
	{
	if ( RLE_count == 0 )
	    { /* Have to restart run. */
	    unsigned char i;
	    i = getbyte( ifp );
	    RLE_flag = ( i & 0x80 );
	    if ( RLE_flag == 0 )
		/* Stream of unencoded pixels. */
		RLE_count = i + 1;
	    else
		/* Single pixel replicated. */
		RLE_count = i - 127;
	    /* Decrement count & get pixel. */
	    --RLE_count;
	    }
	else
	    { /* Have already read count & (at least) first pixel. */
	    --RLE_count;
	    if ( RLE_flag != 0 )
		/* Replicated pixels. */
		goto PixEncode;
	    }
	}
    /* Read appropriate number of bytes, break into RGB. */
    switch ( Size )
	{
	case 8:				/* Grey scale, read and triplicate. */
	Red = Grn = Blu = l = getbyte( ifp );
	break;

	case 16:			/* 5 bits each of red green and blue. */
	case 15:			/* Watch byte order. */
	j = getbyte( ifp );
	k = getbyte( ifp );
	l = ( (unsigned int) k << 8 ) + j;
	Red = ( k & 0x7C ) >> 2;
	Grn = ( ( k & 0x03 ) << 3 ) + ( ( j & 0xE0 ) >> 5 );
	Blu = j & 0x1F;
	break;

	case 32:
	case 24:			/* 8 bits each of blue green and red. */
	Blu = getbyte( ifp );
	Grn = getbyte( ifp );
	Red = getbyte( ifp );
	if ( Size == 32 )
	    Alpha = getbyte( ifp );	/* Read alpha byte & throw away. */
	else
	    Alpha = 255;
	l = 0;
	break;

	default:
	atuError(FXTRUE, "unknown pixel size (#2) - %d", Size );
	}

PixEncode:
#if 0
    if ( mapped )
	*dest = ColorMap[l];
    else
	PPM_ASSIGN( *dest, Red, Grn, Blu );
#endif

    tgascan->r = Red;
    tgascan->g = Grn;
    tgascan->b = Blu;
    tgascan->a = Alpha;

    tgascan++;
    }

static unsigned char
getbyte( ifp )
    FILE* ifp;
    {
    unsigned char c;

    if ( fread( (char*) &c, 1, 1, ifp ) != 1 )
      {
	atuError(FXTRUE, "EOF / read error" );
      }

    return c;
    }
