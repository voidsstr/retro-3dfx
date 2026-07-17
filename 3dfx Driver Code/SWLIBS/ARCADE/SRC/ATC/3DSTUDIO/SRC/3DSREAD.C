/*
** Copyright (c) 1995, 3Dfx Interactive, Inc.
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
** ALPHA PRODUCT!
*/

/*
** 3DS Data Read Functions
**
** The following functions read 3DStudio data types.
*/

/*-----------------------------  Includes -----------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include "3ds.h"

/*----------------------------  Functions -----------------------------*/

/*---------------------------------------------------------------------
  Function: TDSReadUByte
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: reads in a 3DS-defined byte
  Arguments: 
    file - pointer to the 3DS file structure
  Return: 3DS byte (unsigned char)
---------------------------------------------------------------------*/
tds_ubyte TDSReadUByte( TDSFile *file )
{
  tds_ubyte ret;

  fread( &ret, 1, 1, file->fp );
  return ret;
}

/*---------------------------------------------------------------------
  Function: TDSReadUShort
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: reads in a 3DS-defined unsigned short
  Arguments: 
    file - pointer to the 3DS file structure
  Return: 3DS unsigned short (unsigned short)
---------------------------------------------------------------------*/
tds_ushort TDSReadUShort( TDSFile *file )
{
  tds_ushort ret;
  tds_ubyte  lsbyte, msbyte;

  fread( &lsbyte, 1, 1, file->fp );
  fread( &msbyte, 1, 1, file->fp );

  ret = lsbyte | ( ( ( tds_ushort )msbyte ) << 8 );
  return ret;
}

/*---------------------------------------------------------------------
  Function: TDSReadShort
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: reads in a 3DS-defined short
  Arguments: 
    file - pointer to the 3DS file structure
  Return: 3DS short (short)
---------------------------------------------------------------------*/
tds_short TDSReadShort( TDSFile *file )
{
  tds_short ret;
  tds_ubyte lsbyte, msbyte;

  fread( &lsbyte, 1, 1, file->fp );
  fread( &msbyte, 1, 1, file->fp );

  *( ( tds_ushort * )&ret ) = ( tds_ushort )lsbyte | ( ( tds_ushort )msbyte << 8 );
  return ret;
}

/*---------------------------------------------------------------------
  Function: TDSReadUInt
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: reads in a 3DS-defined unsigned integer
  Arguments: 
    file - pointer to the 3DS file structure
  Return: 3DS unsigned integer (unsigned int)
---------------------------------------------------------------------*/
tds_uint TDSReadUInt( TDSFile *file )
{
  tds_uint ret;
  tds_ubyte bytes[4];

  fread( &bytes[0], 1, 1, file->fp );
  fread( &bytes[1], 1, 1, file->fp );
  fread( &bytes[2], 1, 1, file->fp );
  fread( &bytes[3], 1, 1, file->fp );

  ret = ( tds_uint )bytes[0] |
        ( ( ( tds_uint )bytes[1] ) << 8 ) |
        ( ( ( tds_uint )bytes[2] ) << 16 ) |
        ( ( ( tds_uint )bytes[3] ) << 24 );
  return ret;
}

/*---------------------------------------------------------------------
  Function: TDSReadLong
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: reads in a 3DS-defined long
  Arguments: 
    file - pointer to the 3DS file structure
  Return: 3DS long (int)
---------------------------------------------------------------------*/
tds_long TDSReadLong( TDSFile *file )
{
  tds_long ret;
  tds_ubyte bytes[4];

  fread( &bytes[0], 1, 1, file->fp );
  fread( &bytes[1], 1, 1, file->fp );
  fread( &bytes[2], 1, 1, file->fp );
  fread( &bytes[3], 1, 1, file->fp );

  *( ( tds_ulong * )&ret ) = ( tds_uint )bytes[0] |
                             ( ( ( tds_uint )bytes[1] ) << 8 ) |
                             ( ( ( tds_uint )bytes[2] ) << 16 ) |
                             ( ( ( tds_uint )bytes[3] ) << 24 );
  return ret;
}

/*---------------------------------------------------------------------
  Function: TDSReadULong
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: reads in a 3DS-defined unsigned long
  Arguments: 
    file - pointer to the 3DS file structure
  Return: 3DS unsigned long (unsigned int)
---------------------------------------------------------------------*/
tds_ulong TDSReadULong( TDSFile *file )
{
  tds_ulong ret;
  tds_ubyte bytes[4];

  fread( &bytes[0], 1, 1, file->fp );
  fread( &bytes[1], 1, 1, file->fp );
  fread( &bytes[2], 1, 1, file->fp );
  fread( &bytes[3], 1, 1, file->fp );

  ret = ( tds_uint )bytes[0] |
        ( ( ( tds_uint )bytes[1] ) << 8 ) |
        ( ( ( tds_uint )bytes[2] ) << 16 ) |
        ( ( ( tds_uint )bytes[3] ) << 24 );
  return ret;
}

/*---------------------------------------------------------------------
  Function: TDSReadFloat
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: reads in a 3DS-defined float
  Arguments: 
    file - pointer to the 3DS file structure
  Return: 3DS float (float)
---------------------------------------------------------------------*/
tds_float TDSReadFloat( TDSFile *file )
{
  tds_float ret;
  tds_ubyte bytes[4];

  fread( &bytes[0], 1, 1, file->fp );
  fread( &bytes[1], 1, 1, file->fp );
  fread( &bytes[2], 1, 1, file->fp );
  fread( &bytes[3], 1, 1, file->fp );

  *( ( tds_ulong * )&ret ) = ( tds_uint )bytes[0] |
                             ( ( ( tds_uint )bytes[1] << 8 ) ) |
                             ( ( ( tds_uint )bytes[2] << 16 ) ) |
                             ( ( ( tds_uint )bytes[3] << 24 ) );
  return ret;
}

/*---------------------------------------------------------------------
  Function: TDSReadString
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: reads in a 3DS-defined string
  Arguments: 
    file - pointer to the 3DS file structure
    string - holds the char string information
  Return: none
---------------------------------------------------------------------*/
void TDSReadString( TDSFile *file, tds_string string )
{
  while( ( *string++ = getc( file->fp ) ) != '\0' )
    ;
}

/*---------------------------------------------------------------------
  Function: TDSReadPoint
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: reads in a 3DS-defined point (3 float values)
  Arguments: 
    file - pointer to the 3DS file structure
    point - 3 float values representing the x, y, and z components of
            a vector
  Return: none
---------------------------------------------------------------------*/
void TDSReadPoint( TDSFile *file, tds_point *point )
{
  point->x = TDSReadFloat( file );
  point->y = TDSReadFloat( file );
  point->z = TDSReadFloat( file );
}
