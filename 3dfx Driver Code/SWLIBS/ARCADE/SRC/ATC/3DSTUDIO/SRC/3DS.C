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
** 3DS Loader - more information here
*/

/*-----------------------------  Includes -----------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <atutil.h>
#include "3ds.h"
#include "3dsinc.h"

#if defined( __DOS__ ) || defined( WIN32 )
# include<fcntl.h>
# include<io.h>
#endif

/*-----------------------------  Defines ------------------------------*/
#define PRINT_SKIPPED_CHUNKS            0
#define PRINT_PROCESSED_CHUNKS          0
#define PRINT_FIRST_PASS_RESULTS        0
#define PRINT_SECOND_PASS_RESULTS       0
#define PRINT_THIRD_PASS_RESULTS        0
#define PRINT_KEY_INFO                  0
#define PRINT_MATRICES                  0

/*-------------------------  Global Variables -------------------------*/
static FILE *error_fp = stderr;
static int pass;
static tds_point *transformed_verts;

/*----------------------------  Functions -----------------------------*/


           /* General 3D Studio File Manipulation Functions */
/*=====================================================================*/
/*=====================================================================*/

/*---------------------------------------------------------------------
  Function: TDSReadChunkHeader
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: reads a chunk header, the header consists of a unique
               id, and the lengh of the chuck; the offset contains
               information on where the chunk resides in the .3ds file
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file, 
           and will reside after the header read
    header - structure to be filled with 3D Studio header information
  Return: see file, header
---------------------------------------------------------------------*/
void TDSReadChunkHeader( TDSFile *file, TDSChunkHeader *header )
{
  header->offset =      ftell( file->fp );
  header->id =          TDSReadUShort( file );
  header->len =         TDSReadUInt( file );
}

/*---------------------------------------------------------------------
  Function: TDSReadChunkHeader
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: skips over unwanted or unnecessary chunks of 3d Studio
               data
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the skip
    header - structure that contains the lengh variable of the chunk
             to be skipped
  Return: see file
---------------------------------------------------------------------*/
void TDSSkipChunk( TDSFile *file, TDSChunkHeader *header )
{
  fseek( file->fp, header->offset + header->len, SEEK_SET );
}

/*----------------------------------------------------------------------*/
/* NOTE: This is a multi-pass program.  It traverses the .3ds file a    */
/* total of three times.  Each function will be labeled as to what pass */
/* happens upon it.  In some cases it will "pass" through a function    */
/* more than once -- this will be noted.                                */
/*----------------------------------------------------------------------*/


                        /* Material Functions */
/*=====================================================================*/
/*=====================================================================*/

/*---------------------------------------------------------------------
  Function: TDSReadMAT_REFLMAP
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: read the 3D Studio reflection map data
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
    parent_header - the header information for the parent chunk
                    (MAT_ENTRY)
  Return: see file
  Pass(s): third pass
---------------------------------------------------------------------*/
void TDSReadMAT_REFLMAP( TDSFile *file, TDSChunkHeader *parent_header )
{
  TDSChunkHeader header;
  tds_material *mat;

  /* check to see what pass of the file the program is on, and skip
     this chunk if necessary */
  if( pass < 3 )
    {
      TDSSkipChunk( file, parent_header);
      return;
    }
  
  /* get the header information for the strength of the reflection map */
  TDSReadChunkHeader( file, &header );

  /* localize the current material value in the function */
  mat = &file->materials[file->current_material];

  /* get the reflcetion map strength, if not found generate an error
     and exit the program */
  if( header.id != INT_PERCENTAGE )
    atuError(FXTRUE, "INT_PERCENTAGE expected in TDSReadMAT_REFLMAP" );
  mat->reflmap.weight = ( float )TDSReadShort( file ) / 100.0f;
  
  /* get the header information for the reflection map name */
  TDSReadChunkHeader( file, &header );

  /* get the reflection map name, if not found generate an error and
     exit the program */
  if( header.id != MAT_MAPNAME )
    atuError(FXTRUE, "MAT_MAPNAME expected in TDSReadMAT_REFLMAP" );
  TDSReadString( file, mat->reflmap.mapname );
}

/*---------------------------------------------------------------------
  Function: TDSReadMAT_AMBIENT_OR_DIFFUSE_OR_SPECULAR
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: reads the 3D Studio ambient, diffuse, and specular
               information for the material
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
    parent_header - the header information for the parent chunk
                    (MAT_ENTRY)
  Return: see file above
  Pass: called from many functions as needed
---------------------------------------------------------------------*/
void TDSReadMAT_AMBIENT_OR_DIFFUSE_OR_SPECULAR( TDSFile *file, TDSChunkHeader *parent_header, tds_ubyte color[3] )
{
  TDSChunkHeader header;
  tds_ubyte color_24[3];
  tds_ubyte lin_color_24[3];
  int has_color_24 = 0, has_lin_color_24 = 0;

  /* read the next chunk header for the type of color information - 
     be it COLOR_24 or LIN_COLOR_24 */
  TDSReadChunkHeader( file, &header );

  /* if the type of color is COLOR_24, get the color information */  
  if( header.id == COLOR_24 )
    {
      color_24[0] = TDSReadUByte( file );
      color_24[1] = TDSReadUByte( file );
      color_24[2] = TDSReadUByte( file );

      /* skip to the end of this chunk and read the next chunk header */
      TDSSkipChunk( file, &header );
      TDSReadChunkHeader( file, &header );

      /* set color found flag */
      has_color_24 = 1;
    }
  
  /* if the type of color is LIN_COLOR_24, get the color information */  
  if( header.id == LIN_COLOR_24 )
    {
      lin_color_24[0] = TDSReadUByte( file );
      lin_color_24[1] = TDSReadUByte( file );
      lin_color_24[2] = TDSReadUByte( file );

      /* skip to the end of this chunk and read the next chunk header */
      TDSSkipChunk( file, &header );
      TDSReadChunkHeader( file, &header );

      /* set color found flag */
      has_lin_color_24 = 1;
    }

  /* compile the color into the correct read format */
  if( has_lin_color_24 )
    memcpy( color, lin_color_24, sizeof( tds_ubyte[3] ) );
  else
    memcpy( color, color_24, sizeof( tds_ubyte[3] ) );
}     

/*---------------------------------------------------------------------
  Function: TDSReadMAT_TEXMAP
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: reads the 3D Studio general texture map data - things
               like name, tiling or not, u and v scale and offset, etc.
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
    parent_header - the header information for the parent chunk
                    (MAT_ENTRY)
  Return: see file
---------------------------------------------------------------------*/
void TDSReadMAT_TEXMAP( TDSFile *file, TDSChunkHeader *parent_header )
{
  TDSChunkHeader header;
  tds_material *mat;

  /* check to see what pass of the file the program is on, and skip
     this chunk if necessary */
  if( pass < 3 )
    {
      TDSSkipChunk( file, parent_header);
      return;
    }

  /* get the header information for texture map */
  TDSReadChunkHeader( file, &header );

  /* localize the current material value in the function */
  mat = &file->materials[file->current_material];

  /* loop through this texture chunk finding the important information */
  while( ftell( file->fp ) < (long)( parent_header->offset + parent_header->len ) )
    {
      /* INT_PERCENTAGE */
      if( header.id == INT_PERCENTAGE )
        {
          /* texture map strength found, set flag and get the strength */
          mat->texmap.strength = TDSReadShort( file );
          mat->texmap.has_strength = 1;

          /* skip to the end of this current chunk and read the next
             chunk header */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );

          /* print the found texture map strength */
#if PRINT_PROCESSED_CHUNKS
          fprintf( error_fp, "Strength: %d\n", ( int )mat->texmap.strength );
#endif
        }

     /* MAT_MAPNAME */
      else if( header.id == MAT_MAPNAME )
        {
          /* texture map name is found, set flag and get name */
          mat->texmap.has_mapname = 1;
          TDSReadString( file, mat->texmap.mapname );

          /* skip to the end of this current chunk and read the next
             chunk header */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );

          /* print the found texture map name */
#if PRINT_PROCESSED_CHUNKS
          fprintf( error_fp, "Mapname: \"%s\"\n", mat->texmap.mapname );
#endif
        }

      /* MAT_MAP_TILING */
      else if( header.id == MAT_MAP_TILING )
        {
          /* texture map tiling found, set flag and get information */
          mat->texmap.has_map_tiling = 1;
          mat->texmap.map_tiling = TDSReadUShort( file );

          /* skip to the end of this current chunk and read the next
             chunk header */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );

          /* print that we found texture map tiling */
#if PRINT_PROCESSED_CHUNKS
          fprintf( error_fp, "Map tiling: 0x%x\n", ( int )mat->texmap.map_tiling );
#endif
        }

      /* MAT_MAT_TEXBLUR */
      else if( header.id == MAT_MAT_TEXBLUR )
        {
          /* texture map blurring found, set the flag and get the strength */
          mat->texmap.has_tex_blur = 1;
          mat->texmap.tex_blur = TDSReadFloat( file );

          /* skip to the end of this current chunk and read the next
             chunk header */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );

          /* print the found texture blurring strength */
#if PRINT_PROCESSED_CHUNKS
          fprintf( error_fp, "Texture blur: %f\n", ( int )mat->texmap.tex_blur );
#endif
        }

      /* MAT_MAP_USCALE */
      else if( header.id == MAT_MAP_USCALE )
        {
          /* texture map u scale found, set flag and get scale value */
          mat->texmap.has_u_scale = 1;
          mat->texmap.u_scale = TDSReadFloat( file );

          /* skip to the end of this current chunk and read the next
             chunk header */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );

          /* print the texture map u scale value */
#if PRINT_PROCESSED_CHUNKS
          fprintf( error_fp, "U scale: %f\n", ( int )mat->texmap.u_scale );
#endif
        }

      /* MAT_MAP_VSCALE */
      else if( header.id == MAT_MAP_VSCALE )
        {
          /* texture map v scale found, set flag and get scale value */
          mat->texmap.has_v_scale = 1;
          mat->texmap.v_scale = TDSReadFloat( file );

          /* skip to the end of this current chunk and read the next
             chunk header */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );

          /* print the texture map v scale value */
#if PRINT_PROCESSED_CHUNKS
          fprintf( error_fp, "V scale: %f\n", ( int )mat->texmap.v_scale );
#endif
        }

      /* MAT_MAP_UOFFSET */
      else if( header.id == MAT_MAP_UOFFSET )
        {
          /* texture map u offset found, set flag and get offset value */
          mat->texmap.has_u_offset = 1;
          mat->texmap.u_offset = TDSReadFloat( file );

          /* skip to the end of this current chunk and read the next
             chunk header */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );

          /* print the texture map u offset value */
#if PRINT_PROCESSED_CHUNKS
          fprintf( error_fp, "U offset: %f\n", ( int )mat->texmap.u_offset );
#endif
        }

      /* MAT_MAP_VOFFSET */
      else if( header.id == MAT_MAP_VOFFSET )
        {
          /* texture map v offset found, set flag and get offset value */
          mat->texmap.has_v_offset = 1;
          mat->texmap.v_offset = TDSReadFloat( file );

          /* skip to the end of this current chunk and read the next
             chunk header */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );

          /* print the texture map v offset value */
#if PRINT_PROCESSED_CHUNKS
          fprintf( error_fp, "V offset: %f\n", ( int )mat->texmap.v_offset );
#endif
        }

      /* MAT_MAP_ANG */
      else if( header.id == MAT_MAP_ANG )
        {
          /* texture map angle found, set flag and get angle */
          mat->texmap.has_angle = 1;
          mat->texmap.angle = TDSReadFloat( file );

          /* skip to the end of this current chunk and read the next
             chunk header */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );

          /* print the texture map angle */
#if PRINT_PROCESSED_CHUNKS
          fprintf( error_fp, "Angle: %f\n", ( int )mat->texmap.angle );
#endif
        }

      /* MAT_MAP_COL1 */
      else if( header.id == MAT_MAP_COL1 )
        {
          /* texture map tint color 1 found, set the flag and get the RGB values */
          mat->texmap.has_col1 = 1;
          mat->texmap.col1[0] = TDSReadUByte( file );
          mat->texmap.col1[1] = TDSReadUByte( file );
          mat->texmap.col1[2] = TDSReadUByte( file );

          /* skip to the end of this current chunk and read the next
             chunk header */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );

          /* print texture map color 1 value */
#if PRINT_PROCESSED_CHUNKS
          fprintf( error_fp, "Col1: %d %d %d\n", ( int )mat->texmap.col1[0], ( int )mat->texmap.col1[1], ( int )mat->texmap.col1[2] );
#endif
        }

      /* MAT_MAP_COL2 */
      else if( header.id == MAT_MAP_COL2 )
        {
          /* texture map tint color 2 found, set flag and get the RGB values */
          mat->texmap.has_col2 = 1;
          mat->texmap.col2[0] = TDSReadUByte( file );
          mat->texmap.col2[1] = TDSReadUByte( file );
          mat->texmap.col2[2] = TDSReadUByte( file );

          /* skip to the end of this current chunk and read the next
             chunk header */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );

          /* print texture map color 2 values */
#if PRINT_PROCESSED_CHUNKS
          fprintf( error_fp, "Col2: %d %d %d\n", ( int )mat->texmap.col2[0], ( int )mat->texmap.col2[1], ( int )mat->texmap.col2[2] );
#endif
        }

      /* MAT_MAP_RCOL */
      else if( header.id == MAT_MAP_RCOL )
        {
          /* texture map red tint color found, set flag and get RGB values */
          mat->texmap.has_rcol = 1;
          mat->texmap.rcol[0] = TDSReadUByte( file );
          mat->texmap.rcol[1] = TDSReadUByte( file );
          mat->texmap.rcol[2] = TDSReadUByte( file );

          /* skip to the end of this current chunk and read the next
             chunk header */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );

          /* print texture map red color values */
#if PRINT_PROCESSED_CHUNKS
          fprintf( error_fp, "Rcol: %d %d %d\n", ( int )mat->texmap.rcol[0], ( int )mat->texmap.rcol[1], ( int )mat->texmap.rcol[2] );
#endif
        }

      /* MAT_MAP_GCOL */
      else if( header.id == MAT_MAP_GCOL )
        {
          /* texture map green tint color found, set flag and get RGB vlaues */
          mat->texmap.has_gcol = 1;
          mat->texmap.gcol[0] = TDSReadUByte( file );
          mat->texmap.gcol[1] = TDSReadUByte( file );
          mat->texmap.gcol[2] = TDSReadUByte( file );

          /* skip to the end of this current chunk and read the next
             chunk header */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );

          /* print the texture map green color values */
#if PRINT_PROCESSED_CHUNKS
          fprintf( error_fp, "Gcol: %d %d %d\n", ( int )mat->texmap.gcol[0], ( int )mat->texmap.gcol[1], ( int )mat->texmap.gcol[2] );
#endif
        }

      /* MAT_MAP_BCOL */
      else if( header.id == MAT_MAP_BCOL )
        {
          /* texture map blue tint color found, set the flag and get RGB values */
          mat->texmap.has_bcol = 1;
          mat->texmap.bcol[0] = TDSReadUByte( file );
          mat->texmap.bcol[1] = TDSReadUByte( file );
          mat->texmap.bcol[2] = TDSReadUByte( file );

          /* skip to the end of this current chunk and read the next
             chunk header */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );

          /* print texture map blue color values */
#if PRINT_PROCESSED_CHUNKS
          fprintf( error_fp, "Bcol: %d %d %d\n", ( int )mat->texmap.bcol[0], ( int )mat->texmap.bcol[1], ( int )mat->texmap.bcol[2] );
#endif
        }
      
      else
        {
          /* skip to the next chunk and read the header data */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );
        }
    }
}

/*---------------------------------------------------------------------
  Function: TDSReadMAT_ENTRY
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: is the material entry point for object, is defines
               coloring and texturing information -- reflection mapping,
               specular, diffuse, and ambient lighting etc.
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
    parent_header - the header information for the parent chunk
  Return: see file
  Pass: passes one and two just count the number of materials found,
        pass 3 collects the information
---------------------------------------------------------------------*/
void TDSReadMAT_ENTRY( TDSFile *file, TDSChunkHeader *parent_header )
{
  /*
   * WARNING!!!  I don't consider all of the possible chunks that can be
   * subordinate to MAT_ENTRY.  Anything that isn't relevant is ignored.
   */
  TDSChunkHeader header;

  /* print the header - tells the user that the object has material 
     information */
#if PRINT_PROCESSED_CHUNKS
  fprintf( error_fp, "MAT_ENTRY\n" );
#endif

  /* check to see what pass of the file the program is on, increment the
     number of materials on passes one and two and then skip to the next 
     chunk */
  if( pass < 3 )
    {
      /* increment the number of materials found */
      file->num_materials++;
      TDSSkipChunk( file, parent_header );
      return;
    }

  /* get the next header under the MAT_ENTRY chunk */
  TDSReadChunkHeader( file, &header );

  /* initialize all the has texture mapping materials to 0 or false */ 
  file->materials[file->current_material].has_texmap = 0;
  file->materials[file->current_material].has_reflmap = 0;
  file->materials[file->current_material].does_self_illum = 0;
  file->materials[file->current_material].two_sided = 0;
  file->materials[file->current_material].shade_type = TDS_SHADE_PHONG;

  /* loop and collect all the material information for the object while
     not at the end of the chunk */
  while( ftell( file->fp ) < ( long )( parent_header->offset + parent_header->len ) )
    {
      /* MAT_TEXMAP */
      if( header.id == MAT_TEXMAP )
        {
          /* set the texture map flag to true and collect the texture map
             information */
          file->materials[file->current_material].has_texmap = 1;
          TDSReadMAT_TEXMAP( file, &header );

          /* skip to the end of the current chunk and read the header
             of the next chunk */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );
        }
      /* MAT_REFLMAP */
      else if( header.id == MAT_REFLMAP )
        {
          /* set the reflection map flag to true and collect the reflection map
             information */
          file->materials[file->current_material].has_reflmap = 1;
          TDSReadMAT_REFLMAP( file, &header );

          /* skip to the end of the current chunk and read the header
             of the next chunk */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );
        }
      /* MAT_NAME */
      else if( header.id == MAT_NAME )
        {
          /* set the texture map name flag to true and get the texture map
             name */
          file->materials[file->current_material].has_name = 1;
          TDSReadString( file, file->materials[file->current_material].name );

          /* skip to the end of the current chunk and read the header
             of the next chunk */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );
        }
      /* MAT_AMBIENT */
      else if( header.id == MAT_AMBIENT )
        {
          /* get the ambient values for the material */
          TDSReadMAT_AMBIENT_OR_DIFFUSE_OR_SPECULAR( file, &header, 
                                                    file->materials[file->current_material].ambient_color );

          /* skip to the end of the current chunk and read the header
             of the next chunk */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );
        }
      /* MAT_DIFFUSE */
      else if( header.id == MAT_DIFFUSE )
        {
          /* get the diffuse values for the material */
          TDSReadMAT_AMBIENT_OR_DIFFUSE_OR_SPECULAR( file, &header, 
                                                    file->materials[file->current_material].diffuse_color );

          /* skip to the end of the current chunk and read the header
             of the next chunk */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );
        }
      /* MAT_SPECULAR */
      else if( header.id == MAT_SPECULAR )
        {
          /* get the specular values for the material */
          TDSReadMAT_AMBIENT_OR_DIFFUSE_OR_SPECULAR( file, &header, 
                                                    file->materials[file->current_material].specular_color );

          /* skip to the end of the current chunk and read the header
             of the next chunk */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );
        }
      /* MAT_SHININESS */
      else if( header.id == MAT_SHININESS )
        {
          /* read the shininess header and get the shininess information for the material */
          TDSReadChunkHeader( file, &header );
          file->materials[file->current_material].shininess = TDSReadShort( file ) / 100.0f;

          /* skip to the end of the current chunk and read the header
             of the next chunk */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );
        }
      /* MAT_SHIN2PCT */
      else if( header.id == MAT_SHIN2PCT )
        {
          /* read the shininess strength header and get the shininess strength for the materail */
          TDSReadChunkHeader( file, &header );
          file->materials[file->current_material].shininess_strength = TDSReadShort( file ) / 100.0f;

          /* skip to the end of the current chunk and read the header
             of the next chunk */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );
        }
      /* MAT_SELF_ILLUM */
      else if( header.id == MAT_SELF_ILLUM )
        {
          /* set the self illumination flag to true */
          file->materials[file->current_material].does_self_illum = 1;

          /* skip to the end of the current chunk and read the header
             of the next chunk */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );
        }
      /* MAT_TWO_SIDE */
      else if( header.id == MAT_TWO_SIDE )
        {
          /* set the two-sided flag to true */
          file->materials[file->current_material].two_sided = 1;

          /* skip to the end of the current chunk and read the header
             of the next chunk */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );
        }
      /* MAT_SHADING */
      else if( header.id == MAT_SHADING )
        {
          /* get the material shading type */
          file->materials[file->current_material].shade_type = TDSReadShort( file );

          /* skip to the end of the current chunk and read the header
             of the next chunk */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );
        }
      else
        {
          /* skip to the next chunk and read the header */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );
        }
    }

  /* skip to the end of the MAT_ENTRY chunk, and set the current material to
     the next material, for the next pass */
  TDSSkipChunk( file, parent_header );
  file->current_material++;
}


                       /* Global Scene Functions */
/*=====================================================================*/
/*=====================================================================*/
/* NOTE!!! most of the following functions are just skip functions to act
as place holders, so that these functions may be implemented later */


/*---------------------------------------------------------------------
  Function: TDSReadMESH_VERSION
  SKIPPED
---------------------------------------------------------------------*/
void TDSReadMESH_VERSION( TDSFile *file )
{
#if PRINT_SKIPPED_CHUNKS
  fprintf( error_fp, "Skipping MESH_VERSION\n" );
#endif
}

/*---------------------------------------------------------------------
  Function: TDSReadMASTER_SCALE
  SKIPPED
---------------------------------------------------------------------*/
void TDSReadMASTER_SCALE( TDSFile *file )
{
#if PRINT_SKIPPED_CHUNKS
  fprintf( error_fp, "Skipping MASTER_SCALE\n" );
#endif
}

/*---------------------------------------------------------------------
  Function: TDSReadVIEWPORT_LAYOUT
  SKIPPED
---------------------------------------------------------------------*/
void TDSReadVIEWPORT_LAYOUT( TDSFile *file )
{
#if PRINT_SKIPPED_CHUNKS
  fprintf( error_fp, "Skipping VIEWPORT_LAYOUT\n" );
#endif
}

/*---------------------------------------------------------------------
  Function: TDSReadDEFAULT_VIEW
  SKIPPED
---------------------------------------------------------------------*/
void TDSReadDEFAULT_VIEW( TDSFile *file )
{
#if PRINT_SKIPPED_CHUNKS
  fprintf( error_fp, "Skipping DEFAULT_VIEW\n" );
#endif
}

/*---------------------------------------------------------------------
  Function: TDSReadLO_SHADOW_BIAS
  SKIPPED
---------------------------------------------------------------------*/
void TDSReadLO_SHADOW_BIAS( TDSFile *file )
{
#if PRINT_SKIPPED_CHUNKS
  fprintf( error_fp, "Skipping LO_SHADOW_BIAS\n" );
#endif
}

/*---------------------------------------------------------------------
  Function: TDSReadHI_SHADOW_BIAS
  SKIPPED
---------------------------------------------------------------------*/
void TDSReadHI_SHADOW_BIAS( TDSFile *file )
{
#if PRINT_SKIPPED_CHUNKS
  fprintf( error_fp, "Skipping HI_SHADOW_BIAS\n" );
#endif
}

/*---------------------------------------------------------------------
  Function: TDSReadSAMPLE_MAP_SIZE
  SKIPPED
---------------------------------------------------------------------*/
void TDSReadSHADOW_MAP_SIZE( TDSFile *file )
{
#if PRINT_SKIPPED_CHUNKS
  fprintf( error_fp, "Skipping SHADOW_MAP_SIZE\n" );
#endif
}

/*---------------------------------------------------------------------
  Function: TDSReadSHADOW_SAMPLES
  SKIPPED
---------------------------------------------------------------------*/
void TDSReadSHADOW_SAMPLES( TDSFile *file )
{
#if PRINT_SKIPPED_CHUNKS
  fprintf( error_fp, "Skipping SHADOW_SAMPLES\n" );
#endif
}

/*---------------------------------------------------------------------
  Function: TDSReadSHADOW_RANGE
  SKIPPED
---------------------------------------------------------------------*/
void TDSReadSHADOW_RANGE( TDSFile *file )
{
#if PRINT_SKIPPED_CHUNKS
  fprintf( error_fp, "Skipping SHADOW_RANGE\n" );
#endif
}

/*---------------------------------------------------------------------
  Function: TDSReadSHADOW_FILTER
  SKIPPED
---------------------------------------------------------------------*/
void TDSReadSHADOW_FILTER( TDSFile *file )
{
#if PRINT_SKIPPED_CHUNKS
  fprintf( error_fp, "Skipping SHADOW_FILTER\n" );
#endif
}

/*---------------------------------------------------------------------
  Function: TDSReadRAY_BIAS
  SKIPPED
---------------------------------------------------------------------*/
void TDSReadRAY_BIAS( TDSFile *file )
{
#if PRINT_SKIPPED_CHUNKS
  fprintf( error_fp, "Skipping RAY_BIAS\n" );
#endif
}

/*---------------------------------------------------------------------
  Function: TDSReadO_CONSTS
  SKIPPED
---------------------------------------------------------------------*/
void TDSReadO_CONSTS( TDSFile *file )
{
#if PRINT_SKIPPED_CHUNKS
  fprintf( error_fp, "Skipping O_CONSTS\n" );
#endif
}

/*---------------------------------------------------------------------
  Function: TDSReadAMBIENT_LIGHT
  SKIPPED
---------------------------------------------------------------------*/
void TDSReadAMBIENT_LIGHT( TDSFile *file )
{
#if PRINT_SKIPPED_CHUNKS
  fprintf( error_fp, "Skipping AMBIENT_LIGHT\n" );
#endif
}


                /* Background and Atmospheric Functions */
/*=====================================================================*/
/*=====================================================================*/
/* NOTE!!! most of the following functions are just skip functions to act
as place holders, so that these functions may be implemented later */


/*---------------------------------------------------------------------
  Function: TDSReadBIT_MAP
  SKIPPED
---------------------------------------------------------------------*/
void TDSReadBIT_MAP( TDSFile *file )
{
#if PRINT_SKIPPED_CHUNKS
  fprintf( error_fp, "Skipping BIT_MAP\n" );
#endif
}

/*---------------------------------------------------------------------
  Function: TDSReadSOLID_BGND
  SKIPPED
---------------------------------------------------------------------*/
void TDSReadSOLID_BGND( TDSFile *file )
{
#if PRINT_SKIPPED_CHUNKS
  fprintf( error_fp, "Skipping SOLID_BGND\n" );
#endif
}

/*---------------------------------------------------------------------
  Function: TDSReadV_GRADIENT
  SKIPPED
---------------------------------------------------------------------*/
void TDSReadV_GRADIENT( TDSFile *file )
{
#if PRINT_SKIPPED_CHUNKS
  fprintf( error_fp, "Skipping V_GRADIENT\n" );
#endif
}

/*---------------------------------------------------------------------
  Function: TDSReadUSE_BIT_MAP
  SKIPPED
---------------------------------------------------------------------*/
void TDSReadUSE_BIT_MAP( TDSFile *file )
{
#if PRINT_SKIPPED_CHUNKS
  fprintf( error_fp, "Skipping USE_BIT_MAP\n" );
#endif
}

/*---------------------------------------------------------------------
  Function: TDSReadUSE_SOLID_BNGD
  SKIPPED
---------------------------------------------------------------------*/
void TDSReadUSE_SOLID_BGND( TDSFile *file )
{
#if PRINT_SKIPPED_CHUNKS
  fprintf( error_fp, "Skipping USE_SOLID_BGND\n" );
#endif
}

/*---------------------------------------------------------------------
  Function: TDSReadUSE_V_GRADIENT
  SKIPPED
---------------------------------------------------------------------*/
void TDSReadUSE_V_GRADIENT( TDSFile *file )
{
#if PRINT_SKIPPED_CHUNKS
  fprintf( error_fp, "Skipping USE_V_GRADIENT\n" );
#endif
}

/*---------------------------------------------------------------------
  Function: TDSReadFOG
  SKIPPED
---------------------------------------------------------------------*/
void TDSReadFOG( TDSFile *file )
{
#if PRINT_SKIPPED_CHUNKS
  fprintf( error_fp, "Skipping FOG\n" );
#endif
}

/*---------------------------------------------------------------------
  Function: TDSReadLAYER_FOG
  SKIPPED
---------------------------------------------------------------------*/
void TDSReadLAYER_FOG( TDSFile *file )
{
#if PRINT_SKIPPED_CHUNKS
  fprintf( error_fp, "Skipping LAYER_FOG\n" );
#endif
}

/*---------------------------------------------------------------------
  Function: TDSReadDISTANCE_CUE
  SKIPPED
---------------------------------------------------------------------*/
void TDSReadDISTANCE_CUE( TDSFile *file )
{
#if PRINT_SKIPPED_CHUNKS
  fprintf( error_fp, "Skipping DISTANCE_CUE\n" );
#endif
}

/*---------------------------------------------------------------------
  Function: TDSReadUSE_FOG
  SKIPPED
---------------------------------------------------------------------*/
void TDSReadUSE_FOG( TDSFile *file )
{
#if PRINT_SKIPPED_CHUNKS
  fprintf( error_fp, "Skipping USE_FOG\n" );
#endif
}

/*---------------------------------------------------------------------
  Function: TDSReadUSE_LAYER_FOG
  SKIPPED
---------------------------------------------------------------------*/
void TDSReadUSE_LAYER_FOG( TDSFile *file )
{
#if PRINT_SKIPPED_CHUNKS
  fprintf( error_fp, "Skipping USE_LAYER_FOG\n" );
#endif
}

/*---------------------------------------------------------------------
  Function: TDSReadUSE_DISTANCE_CUE
  SKIPPED
---------------------------------------------------------------------*/
void TDSReadUSE_DISTANCE_CUE( TDSFile *file )
{
#if PRINT_SKIPPED_CHUNKS
  fprintf( error_fp, "Skipping USE_DISTANCE_CUE\n" );
#endif
}


                       /* Geometry Functions */
/*=====================================================================*/
/*=====================================================================*/

/*---------------------------------------------------------------------
  Function: TDSReadPOINT_ARRAY
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: the array of vertices for an object
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
    parent_header - the header information for the parent chunk
                    ()
  Return: see file
  Pass(s): second, and third pass
---------------------------------------------------------------------*/
void TDSReadPOINT_ARRAY( TDSFile *file, TDSChunkHeader *parent_header )
{
  tds_ushort num_verts;
  int i;

  /* get the number of vertices in the array */
  num_verts = TDSReadUShort( file );
  
  /* a bit of checking, on the number of vertices */
#if PRINT_PROCESSED_CHUNKS 
  fprintf( error_fp, "POINT_ARRAY: " );
  fprintf( error_fp, "%d verts\n", ( int )num_verts );
#endif
  
  /* if the program is on pass two, then the information passed along is
     the nubmer of vertices in the array */
  if( pass == 2 )
    {
      /* save the number of vertices, in the current object */
      file->num_verts_in_object[file->current_object] = num_verts;
      /* skip to the end of this chunk */
      TDSSkipChunk( file, parent_header );
      return;
    }

  /* if the program is on pass three, then the the program get the x, y, and
     z components for each vertices in the array */
  if( pass == 3 )
    {
      for( i = 0; i < num_verts; i++ )
        {
          /* get the x, y, and z information for each vertex */
          TDSReadPoint( file, &( file->world_verts[file->current_object][i] ) );
        }
      /* skip to the end of the current chunk */
      TDSSkipChunk( file, parent_header );
      return;
    }
}

/*---------------------------------------------------------------------
  Function: TDSReadPOINT_FLAG_ARRAY
  SKIPPED
---------------------------------------------------------------------*/
void TDSReadPOINT_FLAG_ARRAY( TDSFile *file )
{
#if PRINT_SKIPPED_CHUNKS
  fprintf( error_fp, "Skipping POINT_FLAG_ARRAY\n" );
#endif
}

/*---------------------------------------------------------------------
  Function: TDSReadMSH_MAT_GROUP
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: the function tells you which material group(s) portions 
               of the current mesh use(s)
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
    parent_header - the header information for the parent chunk
                    ()
  Return: see file
  Pass(s): third pass
---------------------------------------------------------------------*/
void TDSReadMSH_MAT_GROUP( TDSFile *file, TDSChunkHeader *parent_header )
{
  char name[17];
  tds_ushort num_faces;
  tds_ushort face_index;
  int mat_index = -1;
  int i;

  /* get the name of the material group */
  TDSReadString( file, name );
  
  /* print the name of the material group */
#if PRINT_PROCESSED_CHUNKS
  fprintf( error_fp, "MSH_MAT_GROUP: " );
  fprintf( error_fp, "\"%s\"\n", name );
#endif

  /* loop through the materials and assign the new material a new index */
  for( i = 0; i < file->num_materials; i++ )
    {
      if( strcmp( name, file->materials[i].name ) == 0 )
        {
          mat_index = i;
          break;
        }
    }

  /* if the material is not found generate an error */
  if( mat_index == -1 )
    {
      fprintf( error_fp, "TDSReadMSH_MAT_GROUP: material not found\n" );
    }

  /* get the number of faces in the face for this mesh material group */
  num_faces = TDSReadUShort( file );
  
  /* loop through the faces and assign the appropriate material */
  for( i = 0; i < num_faces; i++ )
    {
      face_index = TDSReadUShort( file );
      file->faces[file->current_object][face_index].material_index = mat_index;
  /* print out the face index that just had a material assigned to it */
#if PRINT_PROCESSED_CHUNKS
      fprintf( error_fp, "%d\n", ( int )face_index );
#endif
    }

  /* skip to the end of the current chunk */
  TDSSkipChunk( file, parent_header );
}

/*---------------------------------------------------------------------
  Function: TDSReadMSH_SMOOTH_GROUP
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: function finds all the faces that reside in a smoothing
               group, to increase the impact of shading
               !!!! currently this file finds what smoothing groups
                    faces belong to, but does not do anything with
                    the data
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
    parent_header - the header information for the parent chunk
                    ()
  Return: see file
  Pass(s): third pass
---------------------------------------------------------------------*/
void TDSReadSMOOTH_GROUP( TDSFile *file, TDSChunkHeader *parent_header )
{
  int facenum;

  /* for each face, find out if it is in a smoothing group, if so assign it */
  for( facenum = 0; facenum < file->num_faces_in_object[file->current_object]; facenum++ )
    file->faces[file->current_object][facenum].smoothing_group = TDSReadULong( file );

  /* skip to the end of the current chunk */
  TDSSkipChunk( file, parent_header );
}

/*---------------------------------------------------------------------
  Function: TDSReadMSH_BOXMAP
  SKIPPED
---------------------------------------------------------------------*/
void TDSReadMSH_BOXMAP( TDSFile *file )
{
#if PRINT_SKIPPED_CHUNKS
  fprintf( error_fp, "Skipping MSH_BOXMAP\n" );
#endif
}

/*---------------------------------------------------------------------
  Function: TDSReadFACE_ARRAY
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: get the triangle face information for an object
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
    parent_header - the header information for the parent chunk
                    ()
  Return: see file
  Pass(s): second pass, third pass
---------------------------------------------------------------------*/
void TDSReadFACE_ARRAY( TDSFile *file, TDSChunkHeader *parent_header )
{
  TDSChunkHeader header;
  tds_ushort num_faces;
  int i;
  
  /* get the number of faces contained in the object face array */
  num_faces = TDSReadUShort( file );

  /* print out the number of faces, if necessary */
#if PRINT_PROCESSED_CHUNKS
  fprintf( error_fp, "FACE_ARRAY\n" );
  fprintf( error_fp, "%d faces\n", ( int )num_faces );
#endif

  /* on the second pass, the function just get the number of faces in
     in the object */
  if( pass == 2 )
    {
      file->num_faces_in_object[file->current_object] = num_faces;
      TDSSkipChunk( file, parent_header );
      return;
    }

  /* on the third pass, the function get the vertex and edge information
     for each face in the object */
  if( pass == 3 )
    {
      for( i = 0; i < num_faces; i++ )
        {
          /* get the face and edge information */
          file->faces[file->current_object][i].v1    = TDSReadUShort( file );
          file->faces[file->current_object][i].v2    = TDSReadUShort( file );
          file->faces[file->current_object][i].v3    = TDSReadUShort( file );
          file->faces[file->current_object][i].flags = TDSReadUShort( file );
        }

      /* calculate a flat normal for each face */
      TDSCalcFaceNormals( file, file->current_object );

      /* get to the sub chunks for each face */
      TDSReadChunkHeader( file, &header );
      
      /* loop through and get the optional face information, if any */
      while( ftell( file->fp ) < ( long )( parent_header->offset + parent_header->len ) )
        {
          /* MSH_MAT_GROUP (optional) */
          if( header.id == MSH_MAT_GROUP )
            {
              TDSReadMSH_MAT_GROUP( file, &header );

              /* skip to the end of the current chunk, and read the header
                 for the next chunk to check */
              TDSSkipChunk( file, &header );
              TDSReadChunkHeader( file, &header );
            }
          /* SMOOTH_GROUP (optional) */
          else if( header.id == SMOOTH_GROUP )
            {
              /* smoothing group information found, collect information */
              TDSReadSMOOTH_GROUP( file, &header );

              /* skip to the end of the current chunk, and read the header
                 for the next chunk to check */
              TDSSkipChunk( file, &header );
              TDSReadChunkHeader( file, &header );
            }
          /* MSH_BOXMAP (optional) */
          else if( header.id == MSH_BOXMAP )
            {
              /* boxmap information found, collect the information */
              TDSReadMSH_BOXMAP( file );

              /* skip to the end of the current chunk, and read the header
                 for the next chunk to check */
              TDSSkipChunk( file, &header );
              TDSReadChunkHeader( file, &header );
            }
          else
            {
              /* if weird id found, generate an error message and exit the
                 program */
              atuError( FXTRUE, "Unknown chunk id: %x\n", ( int )header.id );
            }
        }  
      
      /* skip to the end of the current chunk */
      TDSSkipChunk( file, parent_header );
    }
}

/*---------------------------------------------------------------------
  Function: TDSReadTEX_VERTS
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: get the texture vertex information 
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
    parent_header - the header information for the parent chunk
                    ()
  Return: see file
  Pass(s): third pass
---------------------------------------------------------------------*/
void TDSReadTEX_VERTS( TDSFile *file, TDSChunkHeader *parent_header )
{
  tds_ushort num_verts;
  int i;

  /* if not third pass, skip to next chunk */
  if( pass < 3 )
    {
      TDSSkipChunk( file, parent_header );
      return;      
    }

  /* get the number of vertex */
  num_verts = TDSReadUShort( file );

  /* print the number of vertex, if necessary */
#if PRINT_PROCESSED_CHUNKS
  fprintf( error_fp, "TEX_VERTS\n" );
  fprintf( error_fp, "%d verts\n", ( int )num_verts );
#endif

  /* get the u and v values for each vertex */
  for( i = 0; i < num_verts; i++ )
    {
      file->tex_verts[file->current_object][i].u = TDSReadFloat( file );
      file->tex_verts[file->current_object][i].v = TDSReadFloat( file );
    }  

  /* skip to the end of the current chunk */
  TDSSkipChunk( file, parent_header );
}

/*---------------------------------------------------------------------
  Function: TDSReadMESH_MATRIX
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: this function get the 3x4 transform matrix for 
               N_TRI_OBJECT describing the object's orientation 
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
    parent_header - the header information for the parent chunk
                    ()
  Return: see file
  Pass(s): thrid pass
---------------------------------------------------------------------*/
void TDSReadMESH_MATRIX( TDSFile *file )
{
  int i, j;

  /* this check to see if we are on the thrid pass */
  if( pass != 2 )
    return;

  /* print the name of the current object that we are getting the 
     matrices for */
#if PRINT_PROCESSED_CHUNKS 
  printf( "mesh_matrix: \"%s\"\n", file->object_names[file->current_object] );
#endif 

  /* grab the 4x3 matrix */
  for( i = 0; i < 4; i++ )
    {
      for( j = 0; j < 3; j++ )
        {
          file->mesh_matrices[file->current_object][i][j] = TDSReadFloat( file );
        }
      file->mesh_matrices[file->current_object][i][3] = 0.0f;
    }
  file->mesh_matrices[file->current_object][3][3] = 1.0f;

  /* print the 4x3 matrix */
#if PRINT_PROCESSED_CHUNKS 
  TDSPrintMatrixWithPrefix( file->mesh_matrices[file->current_object], "mesh_matrix" );
#endif 

}

/*---------------------------------------------------------------------
  Function: TDSReadMESH_COLOR
  SKIPPED
---------------------------------------------------------------------*/
void TDSReadMESH_COLOR( TDSFile *file )
{
#if PRINT_SKIPPED_CHUNKS
  fprintf( error_fp, "Skipping MESH_COLOR\n" );
#endif
}

/*---------------------------------------------------------------------
  Function: TDSReadMESH_TEXTURE
  SKIPPED
---------------------------------------------------------------------*/
void TDSReadMESH_TEXTURE_INFO( TDSFile *file )
{
#if PRINT_SKIPPED_CHUNKS
  fprintf( error_fp, "Skipping MESH_TEXTURE_INFO\n" );
#endif
}

/*---------------------------------------------------------------------
  Function: TDSReadPROC_NAME
  SKIPPED
---------------------------------------------------------------------*/
void TDSReadPROC_NAME( TDSFile *file )
{
#if PRINT_SKIPPED_CHUNKS
  fprintf( error_fp, "Skipping PROC_NAME\n" );
#endif
}

/*---------------------------------------------------------------------
  Function: TDSReadPROC_DATA
  SKIPPED
---------------------------------------------------------------------*/
void TDSReadPROC_DATA( TDSFile *file )
{
#if PRINT_SKIPPED_CHUNKS
  fprintf( error_fp, "Skipping PROC_DATA\n" );
#endif
}

/*---------------------------------------------------------------------
  Function: TDSCalcVertexNormals
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: this function calculate the vertex normals of the object 
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
    object_number - the unique object id number
  Return: see file
  Pass(s): information is not read from file
---------------------------------------------------------------------*/
void TDSCalcVertexNormals( TDSFile *file, int object_number )
{
  int i;
  tds_face *faces;
  tds_point *verts;
  tds_point *vert_normals;
  tds_point zero;

  /* zero out the vector components */
  zero.x = zero.y = zero.z = 0.0f;

  /* localize faces, verts, and vert_normals for the current object */
  faces = file->faces[object_number];
  verts = file->world_verts[object_number];
  vert_normals = file->vert_normals[object_number];

  /* zero out the vert_normals for accumulation */
  for( i = 0; i < file->num_verts_in_object[object_number]; i++ )
    vert_normals[i] = zero;

  /* calculate the vertex normals for each vertex in the object */
  for( i = 0; i < file->num_faces_in_object[object_number]; i++ )
    {
      TDSVecAdd( &vert_normals[faces[i].v1], &vert_normals[faces[i].v1], &faces[i].normal );
      TDSVecAdd( &vert_normals[faces[i].v2], &vert_normals[faces[i].v2], &faces[i].normal );
      TDSVecAdd( &vert_normals[faces[i].v3], &vert_normals[faces[i].v3], &faces[i].normal );
    }

  /* normalize the vertex normals */
  for( i = 0; i < file->num_verts_in_object[object_number]; i++ )
    TDSVecNorm( &vert_normals[i] );
}

/*---------------------------------------------------------------------
  Function: TDSReadN_TRI_OBJECTS
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: this function gets information on a n_tri_object
               (arbitrary number of triangles object) - specifically
               vertex, face, and mesh information 
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
    parent_header - the header information for the parent chunk
    object_name - name of the n_tri_object
  Return: see file
  Pass(s): first, second, and thrid pass
---------------------------------------------------------------------*/
void TDSReadN_TRI_OBJECT( TDSFile *file, TDSChunkHeader *parent_header, tds_object_name object_name )
{
  TDSChunkHeader header;

  /* prints out that an object was found */
#if PRINT_PROCESSED_CHUNKS
  fprintf( error_fp, "N_TRI_OBJECT\n" );
#endif

  /* on the first pass count the total number of objects in the file */
  if( pass == 1 )
    {
      /* object found, increase the number found */
      file->num_n_tri_objs++;
/*      TDSSkipChunk( file, &header ); */
      return;
    }

  /* on the second pass get the name of the object */
  if( pass == 2 )
    {
      strcpy( file->object_names[file->current_object], object_name );
    }

  /* get the header for the next n_tri_object subchunk */
  TDSReadChunkHeader( file, &header );

  /* loop through until the end of the chunk and collect vertex, face, 
     and mesh information */
  while( ftell( file->fp ) < ( long )( parent_header->offset + parent_header->len ) )
    {
      /* POINT_ARRAY */
      if( header.id == POINT_ARRAY )
        {
          /* get the vertex information for the object */
          TDSReadPOINT_ARRAY( file, &header );
        }

      /* POINT_FLAG_ARRAY */
      else if( header.id == POINT_FLAG_ARRAY )
        {
          /* additional vertex information - not used! */
          TDSReadPOINT_FLAG_ARRAY( file );
        }

      /* FACE_ARRAY */
      else if( header.id == FACE_ARRAY )
        {
          /* get the face information for the object */
          TDSReadFACE_ARRAY( file, &header );
        }

      /* MSH_MAT_GROUP */
      else if( header.id == MSH_MAT_GROUP )
        {
          /* get the mesh information for the object */
          TDSReadMSH_MAT_GROUP( file, &header );
        }

      /* TEX_VERTS */
      else if( header.id == TEX_VERTS )
        {
          /* get the texture mapping coordinate information for the object */
          TDSReadTEX_VERTS( file, &header );
        }

      /* MESH_MATRIX */
      else if( header.id == MESH_MATRIX )
        {
          /* get object orientation information for the object - local space */
          TDSReadMESH_MATRIX( file );
        }

      /* MESH_COLOR */
      else if( header.id == MESH_COLOR )
        {
          /* get the mesh color information - not used! */
          TDSReadMESH_COLOR( file );
        }

      /* MESH_TEXTURE_INFO */
      else if( header.id == MESH_TEXTURE_INFO )
        {
          /* get the mesh texture color information - not used! */
          TDSReadMESH_TEXTURE_INFO( file );
        }

      /* PROC_NAME */
      else if( header.id == PROC_NAME )
        {
          /* get extension information of the file, filename - not used! */
          TDSReadPROC_NAME( file );
        }

      /* PROC_DATA */
      else if( header.id == PROC_DATA )
        {
          /* get the extension information - not used! */
          TDSReadPROC_DATA( file );
        }
      else
        {
          /* generate error - could find a match for the chunk id */
          fprintf( error_fp, "Unknown chunk: 0x%04x\n", ( int )header.id );
        }

      /* skip to the end of the current chunk and get the header information on
         the next chunk to be used */
      TDSSkipChunk( file, &header );
      TDSReadChunkHeader( file, &header );
    }

  /* on the thrid pass calculate the vertex normals */
  if( pass == 3 )
    TDSCalcVertexNormals( file, file->current_object );
  
  /* increase the current object counter, to get the next object */
  file->current_object++;
}

/*---------------------------------------------------------------------
  Function: TDSReadN_DIRECT_LIGHT
  SKIPPED
---------------------------------------------------------------------*/
void TDSReadN_DIRECT_LIGHT( TDSFile *file )
{
#if PRINT_SKIPPED_CHUNKS
  fprintf( error_fp, "Skipping N_DIRECT_LIGHT\n" );
#endif
}

/*---------------------------------------------------------------------
  Function: TDSReadN_CAMERA
  SKIPPED
---------------------------------------------------------------------*/
void TDSReadN_CAMERA( TDSFile *file )
{
#if PRINT_SKIPPED_CHUNKS
  fprintf( error_fp, "Skipping N_CAMERA\n" );
#endif
}

/*---------------------------------------------------------------------
  Function: TDSReadOBJ_HIDDEN
  SKIPPED
---------------------------------------------------------------------*/
void TDSReadOBJ_HIDDEN( TDSFile *file )
{
#if PRINT_SKIPPED_CHUNKS
  fprintf( error_fp, "Skipping OBJECT_HIDDEN\n" );
#endif
}

/*---------------------------------------------------------------------
  Function: TDSReadOBJ_VIS_LOFTER
  SKIPPED
---------------------------------------------------------------------*/
void TDSReadOBJ_VIS_LOFTER( TDSFile *file )
{
#if PRINT_SKIPPED_CHUNKS
  fprintf( error_fp, "Skipping OBJ_VIS_LOFTER\n" );
#endif
}

/*---------------------------------------------------------------------
  Function: TDSReadOBJ_DOESNT_CAST
  SKIPPED
---------------------------------------------------------------------*/
void TDSReadOBJ_DOESNT_CAST( TDSFile *file )
{
#if PRINT_SKIPPED_CHUNKS
  fprintf( error_fp, "Skipping OBJ_DOESNT_CAST\n" );
#endif
}

/*---------------------------------------------------------------------
  Function: TDSReadOBJ_MATTE
  SKIPPED
---------------------------------------------------------------------*/
void TDSReadOBJ_MATTE( TDSFile *file )
{
#if PRINT_SKIPPED_CHUNKS
  fprintf( error_fp, "Skipping OBJ_MATTE\n" );
#endif
}

/*---------------------------------------------------------------------
  Function: TDSReadOBJ_DONT_RCVSHADOW
  SKIPPED
---------------------------------------------------------------------*/
void TDSReadOBJ_DONT_RCVSHADOW( TDSFile *file )
{
#if PRINT_SKIPPED_CHUNKS
  fprintf( error_fp, "Skipping OBJ_DONT_RCVSHADOW\n" );
#endif
}

/*---------------------------------------------------------------------
  Function: TDSReadOBJ_FAST
  SKIPPED
---------------------------------------------------------------------*/
void TDSReadOBJ_FAST( TDSFile *file )
{
#if PRINT_SKIPPED_CHUNKS
  fprintf( error_fp, "Skipping OBJ_FAST\n" );
#endif
}

/*---------------------------------------------------------------------
  Function: TDSReadOBJ_PROCEDURAL
  SKIPPED
---------------------------------------------------------------------*/
void TDSReadOBJ_PROCEDURAL( TDSFile *file )
{
#if PRINT_SKIPPED_CHUNKS
  fprintf( error_fp, "Skipping OBJ_PROCEDURAL\n" );
#endif
}

/*---------------------------------------------------------------------
  Function: TDSReadOBJ_FROZEN
  SKIPPED
---------------------------------------------------------------------*/
void TDSReadOBJ_FROZEN( TDSFile *file )
{
#if PRINT_SKIPPED_CHUNKS
  fprintf( error_fp, "Skipping OBJ_FROZEN\n" );
#endif
}

/*---------------------------------------------------------------------
  Function: TDSReadNAMED_OBJECT
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: this function identifies objects - object, light, 
               and camera, then it collect the "object" information --
               N_TRI_OBJECT is the only one the collects information,
               the other two just skip the information
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
    parent_header - the header information for the parent chunk
  Return: see file
  Pass(s): first, second, and third pass
---------------------------------------------------------------------*/
void TDSReadNAMED_OBJECT( TDSFile *file, TDSChunkHeader *parent_header )
{
  char name[11];
  TDSChunkHeader header;

  /* get the name of the object */
  TDSReadString( file, name );

  /* print the name of the object */
#if PRINT_PROCESSED_CHUNKS
  fprintf( error_fp, "NAMED_OBJECT: " );
  fprintf( error_fp, "\"%s\"\n", name );
#endif

  /* get the information on the "object" subchunks */
  TDSReadChunkHeader( file, &header );

  /* loop through and collect all the information on the 3DS objects */
  while( ftell( file->fp ) < ( long )( parent_header->offset + parent_header->len ) )
    {
      /* N_TRI_OBJECT */
      if( header.id == N_TRI_OBJECT )
        {
          /* found an object, collect information */
          TDSReadN_TRI_OBJECT( file, &header, name );

          /* skip to the end of the current chunk, and read the header
             information for the next chunk */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );
        }

      /* N_DIRECT_LIGHT */
      if( header.id == N_DIRECT_LIGHT )
        {
          /* found a light - not used! */
          TDSReadN_DIRECT_LIGHT( file );

          /* skip to the end of the current chunk, and read the header
             information for the next chunk */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );
        }

      /* N_CAMERA */
      if( header.id == N_CAMERA )
        {
          /* found a camera - not used! */
          TDSReadN_CAMERA( file );

          /* skip to the end of the current chunk, and read the header
             information for the next chunk */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );
        }

      /* OBJECT_HIDDEN (optional) */
      if( header.id == OBJ_HIDDEN )
        {
          /* object is hidden in the scene - not used! */
          TDSReadOBJ_HIDDEN( file );

          /* skip to the end of the current chunk, and read the header
             information for the next chunk */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );
        }

      /* OBJ_VIS_LOFTER (optional) */
      if( header.id == OBJ_VIS_LOFTER )
        {
          /* object is visible in the lofter - not used! */
          TDSReadOBJ_VIS_LOFTER( file );

          /* skip to the end of the current chunk, and read the header
             information for the next chunk */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );
        }

      /* OBJ_DOESNT_CAST (optional) */
      if( header.id == OBJ_DOESNT_CAST )
        {
          /* object does not cast a shadow - not used! */
          TDSReadOBJ_DOESNT_CAST( file );

          /* skip to the end of the current chunk, and read the header
             information for the next chunk */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );
        }

      /* OBJ_MATTE (optional) */
      if( header.id == OBJ_MATTE )
        {
          /* object has a background matte - not used! */
          TDSReadOBJ_MATTE( file );

          /* skip to the end of the current chunk, and read the header
             information for the next chunk */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );
        }

      /* OBJ_DONT_RCVSHADOW (optional) */
      if( header.id == OBJ_DONT_RCVSHADOW )
        {
          /* object does not receive shadows - not used! */
          TDSReadOBJ_DONT_RCVSHADOW( file );

          /* skip to the end of the current chunk, and read the header
             information for the next chunk */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );
        }

      /* OBJ_FAST (optional) */
      if( header.id == OBJ_FAST )
        {
          /* object is fast draw object - not used! */
          TDSReadOBJ_FAST( file );

          /* skip to the end of the current chunk, and read the header
             information for the next chunk */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );
        }

      /* OBJ_PROCEDURAL (optional) */
      if( header.id == OBJ_PROCEDURAL )
        {
          /* object is a procedural object - not used! */
          TDSReadOBJ_PROCEDURAL( file );

          /* skip to the end of the current chunk, and read the header
             information for the next chunk */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );
        }

      /* OBJ_FROZEN (optional) */
      if( header.id == OBJ_FROZEN )
        {
          /* object is a frozen object - not used! */
          TDSReadOBJ_FROZEN( file );

          /* skip to the end of the current chunk, and read the header
             information for the next chunk */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );
        }
    }
}

/*---------------------------------------------------------------------
  Function: TDSReadMDATA
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: this function is the top level function for finding 3DS
               object and material information
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
    parent_header - the header information for the parent chunk
  Return: see file
  Pass(s): first, second, and third pass
---------------------------------------------------------------------*/
void TDSReadMDATA( TDSFile *file, TDSChunkHeader *parent_header )
{
  TDSChunkHeader header;

  /* get the header information on the next material or object chuck */
  TDSReadChunkHeader( file, &header );

  /* loop through and collect all the object and material information
     in the .3ds file */
  while( ftell( file->fp ) < ( long )( parent_header->offset + parent_header->len ) )
    {
      /* MESH_VERSION */
      if( header.id == MESH_VERSION )
        {
          /* get information on the mesh version - not used! */
          TDSReadMESH_VERSION( file );

          /* skip to the end of the current chunk, and read the header
             information for the next chunk */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );
        }

      /* MAT_ENTRY */
      else if( header.id == MAT_ENTRY )
        {
          /* get material information */
          TDSReadMAT_ENTRY( file, &header );

          /* skip to the end of the current chunk, and read the header
             information for the next chunk */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );
        }

      /* MASTER_SCALE */
      else if( header.id == MASTER_SCALE )
        {
          /* get global scale for scene */
          TDSReadMASTER_SCALE( file );

          /* skip to the end of the current chunk, and read the header
             information for the next chunk */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );
        }


      /* VIEWPORT_LAYOUT (optional) */
      else if( header.id == VIEWPORT_LAYOUT )
        {
          /* get viewport layout - not used! */
          TDSReadVIEWPORT_LAYOUT( file );

          /* skip to the end of the current chunk, and read the header
             information for the next chunk */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );
        }

      /* LO_SHADOW_BIAS (optional)  */
      else if( header.id == LO_SHADOW_BIAS )
        {
          /* get shadow bias - not used! */
          TDSReadLO_SHADOW_BIAS( file );

          /* skip to the end of the current chunk, and read the header
             information for the next chunk */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );
        }

      /* HI_SHADOW_BIAS (optional)  */
      else if( header.id == HI_SHADOW_BIAS )
        {
          /* get high shadow bias - not used! */
          TDSReadHI_SHADOW_BIAS( file );

          /* skip to the end of the current chunk, and read the header
             information for the next chunk */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );
        }

      /* SHADOW_MAP_SIZE (optional)  */
      else if( header.id == SHADOW_MAP_SIZE )
        {
          /* get shadow size - not used! */
          TDSReadSHADOW_MAP_SIZE( file );

          /* skip to the end of the current chunk, and read the header
             information for the next chunk */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );
        }

      /* SHADOW_SAMPLES (optional)  */
      else if( header.id == SHADOW_SAMPLES )
        {
          /* get sample size - not used! */
          TDSReadSHADOW_SAMPLES( file );

          /* skip to the end of the current chunk, and read the header
             information for the next chunk */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );
        }

      /* SHADOW_RANGE (optional)  */
      else if( header.id == SHADOW_RANGE )
        {
          /* get shadow sample range - not used! */
          TDSReadSHADOW_RANGE( file );

          /* skip to the end of the current chunk, and read the header
             information for the next chunk */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );
        }

      /* SHADOW_FILTER (optional)  */
      else if( header.id == SHADOW_FILTER )
        {
          /* get shadow filter - not used! */
          TDSReadSHADOW_FILTER( file );

          /* skip to the end of the current chunk, and read the header
             information for the next chunk */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );
        }

      /* RAY_BIAS */
      else if( header.id == RAY_BIAS )
        {
          /* get ray traced shadow bias - not used! */
          TDSReadRAY_BIAS( file );

          /* skip to the end of the current chunk, and read the header
             information for the next chunk */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );
        }

      /* O_CONSTS (optional)  */
      else if( header.id == O_CONSTS )
        {
          /* get construction plane location - not used! */
          TDSReadO_CONSTS( file );

          /* skip to the end of the current chunk, and read the header
             information for the next chunk */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );
        }

      /* AMBIENT_LIGHT (optional) */
      else if( header.id == AMBIENT_LIGHT )
        {
          /* get ambient light information for scene - not used! */
          TDSReadAMBIENT_LIGHT( file );

          /* skip to the end of the current chunk, and read the header
             information for the next chunk */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );
        }

      /* BIT_MAP (optional) */
      else if( header.id == BIT_MAP )
        {
          /* get bit mapped background, if found - not used! */
          TDSReadBIT_MAP( file );

          /* skip to the end of the current chunk, and read the header
             information for the next chunk */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );
        }

      /* SOLID_BGND (optional) */
      else if( header.id == SOLID_BGND )
        {
          /* get solid background, if found - not used! */
          TDSReadSOLID_BGND( file );

          /* skip to the end of the current chunk, and read the header
             information for the next chunk */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );
        }

      /* V_GRADIENT (optional) */
      else if( header.id == V_GRADIENT )
        {
          /* get gradient color background, if found - not used! */
          TDSReadV_GRADIENT( file );

          /* skip to the end of the current chunk, and read the header
             information for the next chunk */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );
        }

      /* USE_BIT_MAP (optional) */
      else if( header.id == USE_BIT_MAP )
        {
          /* set flag to using bit mapped background - not used! */
          TDSReadUSE_BIT_MAP( file );

          /* skip to the end of the current chunk, and read the header
             information for the next chunk */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );
        }

      /* USE_SOLID_BGND (optional) */
      else if( header.id == USE_SOLID_BGND )
        {
          /* set flag to using solid background - not used! */
          TDSReadUSE_SOLID_BGND( file );

          /* skip to the end of the current chunk, and read the header
             information for the next chunk */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );
        }

      /* USE_V_GRADIENT (optional) */
      else if( header.id == USE_V_GRADIENT )
        {
          /* set flag to using gradient background - not used! */
          TDSReadUSE_V_GRADIENT( file );

          /* skip to the end of the current chunk, and read the header
             information for the next chunk */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );
        }

      /* FOG (optional) */
      else if( header.id == FOG )
        {
          /* get fog information - not used! */
          TDSReadFOG( file );

          /* skip to the end of the current chunk, and read the header
             information for the next chunk */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );
        }

      /* LAYER_FOG (optional) */
      else if( header.id == LAYER_FOG )
        {
          /* get layered fog information - not used! */
          TDSReadLAYER_FOG( file );

          /* skip to the end of the current chunk, and read the header
             information for the next chunk */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );
        }

      /* DISTANCE_CUE (optional) */
      else if( header.id == DISTANCE_CUE )
        {
          /* get distance cue definition - not used! */
          TDSReadDISTANCE_CUE( file );

          /* skip to the end of the current chunk, and read the header
             information for the next chunk */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );
        }

      /* USE_FOG (optional) */
      else if( header.id == USE_FOG )
        {
          /* set flag for using fog - not used! */
          TDSReadUSE_FOG( file );

          /* skip to the end of the current chunk, and read the header
             information for the next chunk */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );
        }

      /* USE_LAYER_FOG (optional) */
      else if( header.id == USE_LAYER_FOG )
        {
          /* set flag for using layered fog - not used! */
          TDSReadUSE_LAYER_FOG( file );

          /* skip to the end of the current chunk, and read the header
             information for the next chunk */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );
        }

      /* USE_DISTANCE_CUE (optional) */
      else if( header.id == USE_DISTANCE_CUE )
        {
          /* set flag to use distance cueing - not used! */
          TDSReadUSE_DISTANCE_CUE( file );

          /* skip to the end of the current chunk, and read the header
             information for the next chunk */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );
        }

      /* DEFAULT_VIEW */
      else if( header.id == DEFAULT_VIEW )
        {
          /* get defined rendering view - not used! */
          TDSReadDEFAULT_VIEW( file );

          /* skip to the end of the current chunk, and read the header
             information for the next chunk */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );
        }
      /* NAMED_OBJECT */
      else if( header.id == NAMED_OBJECT )
        {
          /* get information on named object */
          TDSReadNAMED_OBJECT( file, &header );

          /* skip to the end of the current chunk, and read the header
             information for the next chunk */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );
        }

      else
        {
          /* skip to the next chunk, and read the header
             information for the next chunk */
          TDSSkipChunk( file, &header );
          TDSReadChunkHeader( file, &header );
        }
    }
}


                       /* Animation Functions */
/*=====================================================================*/
/*=====================================================================*/

/*---------------------------------------------------------------------
  Function: TDSReadKFHDR
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: get the key frame header information - specifically the
               name and size of the animation
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
    parent_header - the header information for the parent chunk
  Return: see file
  Pass(s): first, second, and third pass
---------------------------------------------------------------------*/
void TDSReadKFHDR( TDSFile *file, TDSChunkHeader *parent_header )
{
  char dummy[30];

  /* revision level of the keyframe section - not used! */
  TDSReadShort( file );

  /* name of the 3ds file - not used */
  TDSReadString( file, dummy );

  /* get the animation length in frames - range [1..32000] */
  file->animation_length = TDSReadLong( file );

  /* skip to the end of the current chunk */
  TDSSkipChunk( file, parent_header );
}

/*---------------------------------------------------------------------
  Function: TDSReadKFSEG
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: gets the active first and last frames of the animation
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
    parent_header - the header information for the parent chunk
  Return: see file
  Pass(s): second and third pass
---------------------------------------------------------------------*/
void TDSReadKFSEG( TDSFile *file, TDSChunkHeader *parent_header )
{
  /* get the active first frame of segment */
  file->first_frame = TDSReadLong( file );
  file->first_frame_set = 1;

  /* get the active last frame of segment */
  file->last_frame = TDSReadLong( file );
  file->last_frame_set = 1;

  /* skip to the end of the current chunk */
  TDSSkipChunk( file, parent_header );
}

/*---------------------------------------------------------------------
  Function: TDSReadNODE_HDR
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: gets the header information for the current animation
               node
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
    parent_header - the header information for the parent chunk
    node_header - the header information for the animation node
  Return: see file
  Pass(s): second, and third pass
---------------------------------------------------------------------*/
void TDSReadNODE_HDR( TDSFile *file, TDSChunkHeader *parent_header,
                      tds_node_header *node_header )
{
  /* get the object name */
  TDSReadString( file, node_header->object_name );

  /* get node flag 1 */
  node_header->flag1 = TDSReadUShort( file );

  /* get node flag 2 */
  node_header->flag2 = TDSReadUShort( file );

  /* get node parent index */
  node_header->parent_index = TDSReadUShort( file );
  
  /* print out the node header information */
#if PRINT_PROCESSED_CHUNKS
  fprintf( error_fp, "object name: \"%s\"\n", node_header->object_name );
  fprintf( error_fp, "flag1: 0x%04x\n", node_header->flag1 );
  fprintf( error_fp, "flag2: 0x%04x\n", node_header->flag2 );
  fprintf( error_fp, "parent index: %d\n", ( int )node_header->parent_index );
#endif

  /* skip to the end of the current chunk */
  TDSSkipChunk( file, parent_header );
}

/*---------------------------------------------------------------------
  Function: TDSReadTrackHeader
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: get the key frame track header information - the item of 
               particular interest is the number of keys in the track
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
    track_header - the header information for the track
  Return: see file
  Pass(s): third pass
---------------------------------------------------------------------*/
void TDSReadTrackHeader( TDSFile *file, tds_track_header *track_header )
{
  /* get the looping flags - not used! */
  track_header->looping_flags = TDSReadUShort( file );

  /* Skip unused data */
  TDSReadULong( file );
  TDSReadULong( file );

  /* get the number of keys in track */  
  track_header->num_keys = TDSReadULong( file );
}

/*---------------------------------------------------------------------
  Function: TDSPrintTrackHeader
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: print the track header information
  Arguments: 
    track_header - the header information for the track
  Return: none
  Pass(s): NA
---------------------------------------------------------------------*/
void TDSPrintTrackHeader( tds_track_header *track_header )
{
  fprintf( error_fp, "flags: 0x%04x\n", track_header->looping_flags );
  fprintf( error_fp, "num_keys: %d\n", ( int )track_header->num_keys );
}

/*---------------------------------------------------------------------
  Function: TDSReadKeyHeader
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: read the key header information - specifically 
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
    key_header - the header information for the track
  Return: see file above
  Pass(s): third
---------------------------------------------------------------------*/
void TDSReadKeyHeader( TDSFile *file, tds_key_header *key_header )
{
  /* get the current frame number */
  key_header->frame_number = TDSReadLong( file );

  /* get the animation interpolation information */
  key_header->spline_term_flags = TDSReadShort( file );

  /* get the tension information, and set it to a default if necessary */
  if( key_header->spline_term_flags & TDS_SPLINE_USE_TENSION )
    key_header->spline_tension = TDSReadFloat( file );
  else
    key_header->spline_tension = 0.0f;

  /* get the continuity information, and set it to a default if necessary */
  if( key_header->spline_term_flags & TDS_SPLINE_USE_CONTINUINITY )
    key_header->spline_continuinity = TDSReadFloat( file );
  else
    key_header->spline_continuinity = 0.0f;

  /* get the bias information, and set it to a default if necessary */
  if( key_header->spline_term_flags & TDS_SPLINE_USE_BIAS )
    key_header->spline_bias = TDSReadFloat( file );
  else
    key_header->spline_bias = 0.0f;

  /* get the ease to information, and set it to a default if necessary */
  if( key_header->spline_term_flags & TDS_SPLINE_USE_EASE_TO )
    key_header->ease_to = TDSReadFloat( file );
  else
    key_header->ease_to = 0.0f;

  /* get the ease from information, and set it to a default if necessary */
  if( key_header->spline_term_flags & TDS_SPLINE_USE_EASE_FROM )
    key_header->ease_from = TDSReadFloat( file );
  else
    key_header->ease_from = 0.0f;
}

/*---------------------------------------------------------------------
  Function: TDSPrintKeyHeader
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: print the key information
  Arguments: 
    key_header - the header information for the key
  Return: none
  Pass(s): NA
---------------------------------------------------------------------*/
void TDSPrintKeyHeader( tds_key_header *key_header )
{
  /* print the current frame number */
  fprintf( error_fp, "frame number: %d\n", ( int )key_header->frame_number );
  
  /* print the heading for spline terms */
  fprintf( error_fp, "spline term flags: 0x%04x\n", ( int )key_header->spline_term_flags );

  /* print spline tension */
  if( key_header->spline_term_flags & TDS_SPLINE_USE_TENSION )
    fprintf( error_fp, "spline tension: %f\n", key_header->spline_tension );

  /* print spline continuity */
  if( key_header->spline_term_flags & TDS_SPLINE_USE_CONTINUINITY )
    fprintf( error_fp, "spline continuinity: %f\n", key_header->spline_continuinity );

  /* print spline bias */
  if( key_header->spline_term_flags & TDS_SPLINE_USE_BIAS )
    fprintf( error_fp, "splin bias: %f\n", key_header->spline_bias );

  /* print spline ease to */
  if( key_header->spline_term_flags & TDS_SPLINE_USE_EASE_TO )
    fprintf( error_fp, "ease to: %f\n", key_header->ease_to );

  /* print spline ease from */
  if( key_header->spline_term_flags & TDS_SPLINE_USE_EASE_FROM )
    fprintf( error_fp, "ease from: %f\n", key_header->ease_from );
}

/*---------------------------------------------------------------------
  Function: TDSReadPOS_TRACK_TAG
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: read the position track tag information
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
    parent_header - the header information for the parent chunk
    pos_track_info - contains information on the position track of the 
                     animation dat
  Return: see file, pos_track_info
  Pass(s): second, thrid
---------------------------------------------------------------------*/
void TDSReadPOS_TRACK_TAG( TDSFile *file, TDSChunkHeader *parent_header,
                           tds_pos_track_info *pos_track_info )
{
  tds_ulong  key;

  /* skip function on first pass */
  if( pass != 1 )
    return;

  /* get the position track header information */
  TDSReadTrackHeader( file, &pos_track_info->track_header );

  /* print the track header information */
#if PRINT_PROCESSED_CHUNKS
  TDSPrintTrackHeader( &pos_track_info->track_header );
#endif

   /* allocate an extra track at the end to make searching easier */
  if( !( pos_track_info->pos_track_tags =
         ( tds_pos_track_tag * )malloc( sizeof( tds_pos_track_tag ) *
                                        (pos_track_info->track_header.num_keys + 1)) ) )
    {
      atuError(FXTRUE, "Out of memory in TDSReadPOS_TRACK_TAG" );
    }

  /* get position information for each key in the animation */
  for( key = 0; key < pos_track_info->track_header.num_keys; key++ )
    {
      /* get the key header information */
      TDSReadKeyHeader( file, &pos_track_info->pos_track_tags[key].key_header );

      /* print the key header information */
#if PRINT_PROCESSED_CHUNKS
      TDSPrintKeyHeader( &pos_track_info->pos_track_tags[key].key_header );
#endif
      
      /* get the position */
      TDSReadPoint( file, &pos_track_info->pos_track_tags[key].position );

      /* print the position */
#if PRINT_PROCESSED_CHUNKS
      fprintf( error_fp, "position; %f %f %f\n",
               pos_track_info->pos_track_tags[key].position.x,
               pos_track_info->pos_track_tags[key].position.y,
               pos_track_info->pos_track_tags[key].position.z );
#endif
    }

  /* set frame number to end key */
  pos_track_info->pos_track_tags[pos_track_info->track_header.num_keys].key_header.frame_number = 10000;
}

/*---------------------------------------------------------------------
  Function: TDSReadFOV_TRACK_TAG
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: read the field of view track tag information
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
    parent_header - the header information for the parent chunk
    fov_track_info - contains information on the field of view track of 
                     the animation data
  Return: see file, fov_track_info
  Pass(s): second, thrid
---------------------------------------------------------------------*/
void TDSReadFOV_TRACK_TAG( TDSFile *file, TDSChunkHeader *parent_header,
                           tds_fov_track_info *fov_track_info )
{
  tds_ulong  key;

  /* skip function on first pass */
  if( pass != 1 )
    return;

  /* get the track header information */
  TDSReadTrackHeader( file, &fov_track_info->track_header );

  /* print the track header information */
#if PRINT_PROCESSED_CHUNKS
  TDSPrintTrackHeader( &fov_track_info->track_header );
#endif

   /* allocate an extra track at the end to make searching easier */
  if( !( fov_track_info->fov_track_tags =
         ( tds_fov_track_tag * )malloc( sizeof( tds_fov_track_tag ) *
                                        (fov_track_info->track_header.num_keys + 1) ) ) )
    {
      atuError(FXTRUE, "Out of memory in TDSReadFOV_TRACK_TAG" );
    }

  /* get field of view information for each key in the animation */
  for( key = 0; key < fov_track_info->track_header.num_keys; key++ )
    {
      /* get the key header information */
      TDSReadKeyHeader( file, &fov_track_info->fov_track_tags[key].key_header );

      /* print the key header information */
#if PRINT_PROCESSED_CHUNKS
      TDSPrintKeyHeader( &fov_track_info->fov_track_tags[key].key_header );
#endif
      
      /* get the field of view angle */
      fov_track_info->fov_track_tags[key].fov_degrees = TDSReadFloat( file );

      /* print the field of view angle */
#if PRINT_PROCESSED_CHUNKS
      fprintf( error_fp, "fov: %f (frame: %d)\n", 
              fov_track_info->fov_track_tags[key].fov_degrees,
              ( int )fov_track_info->fov_track_tags[key].key_header.frame_number );
#endif
    }

  /* set frame number to end key */
  fov_track_info->fov_track_tags[fov_track_info->track_header.num_keys].key_header.frame_number = 10000;
}

/*---------------------------------------------------------------------
  Function: TDSReadROLL_TRACK_TAG
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: read the camera roll angle track tag information
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
    parent_header - the header information for the parent chunk
    roll_track_info - contains information on the camera roll angle
                      track of the animation data
  Return: see file, roll_track_info
  Pass(s): second, thrid
---------------------------------------------------------------------*/
void TDSReadROLL_TRACK_TAG( TDSFile *file, TDSChunkHeader *parent_header,
                           tds_roll_track_info *roll_track_info )
{
  tds_ulong  key;

  /* skip the first pass */
  if( pass != 1 )
    return;

  /* get the track header information */
  TDSReadTrackHeader( file, &roll_track_info->track_header );

  /* print the track header information */
#if PRINT_PROCESSED_CHUNKS
  TDSPrintTrackHeader( &roll_track_info->track_header );
#endif

  /* allocate an extra track at the end to make searching easier */
  if( !( roll_track_info->roll_track_tags =
         ( tds_roll_track_tag * )malloc( sizeof( tds_roll_track_tag ) *
                                        (roll_track_info->track_header.num_keys + 1) ) ) )
    {
      atuError(FXTRUE, "Out of memory in TDSReadROLL_TRACK_TAG" );
    }

  /* get camera roll angle information for each key in the animation */ 
  for( key = 0; key < roll_track_info->track_header.num_keys; key++ )
    {
      /* get the key header information */
      TDSReadKeyHeader( file, &roll_track_info->roll_track_tags[key].key_header );

      /* print the key header information */
#if PRINT_PROCESSED_CHUNKS
      TDSPrintKeyHeader( &roll_track_info->roll_track_tags[key].key_header );
#endif
      
      /* get the camera roll angle */
      roll_track_info->roll_track_tags[key].roll_degrees = TDSReadFloat( file );

      /* print the camera roll angle */
#if PRINT_PROCESSED_CHUNKS
      fprintf( error_fp, "roll: %f (frame: %d)\n", 
              roll_track_info->roll_track_tags[key].roll_degrees,
              ( int )roll_track_info->roll_track_tags[key].key_header.frame_number );
#endif
    }

  /* set frame number to end key */
  roll_track_info->roll_track_tags[roll_track_info->track_header.num_keys].key_header.frame_number = 10000;
}

/*---------------------------------------------------------------------
  Function: TDSReadROT_TRACK_TAG
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: read the rotation angle track tag information
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
    parent_header - the header information for the parent chunk
    rot_track_info - contains information on the rotation angle track of the 
                     animation dat
  Return: see file, rot_track_info
  Pass(s): second, thrid
---------------------------------------------------------------------*/
void TDSReadROT_TRACK_TAG( TDSFile *file, TDSChunkHeader *parent_header,
                           tds_rot_track_info *rot_track_info )
{
  tds_ulong  key;

  /* skip this function on the first pass */
  if( pass != 1 )
    return;

  /* get the track header information */
  TDSReadTrackHeader( file, &rot_track_info->track_header );

  /* print the track header information */
#if PRINT_PROCESSED_CHUNKS
  TDSPrintTrackHeader( &rot_track_info->track_header );
#endif

  /* Allocate an extra track at the end to make searching easier */
  if( !( rot_track_info->rot_track_tags =
         ( tds_rot_track_tag * )malloc( sizeof( tds_rot_track_tag ) *
                                        (rot_track_info->track_header.num_keys + 1) ) ) )
    {
      atuError(FXTRUE, "Out of memory in TDSReadROT_TRACK_TAG" );
    }

  /* get rotation angle information for each key in the animation */ 
  for( key = 0; key < rot_track_info->track_header.num_keys; key++ )
    {
      /* get the key header information */
      TDSReadKeyHeader( file, &rot_track_info->rot_track_tags[key].key_header );

      /* print the key header information */
#if PRINT_PROCESSED_CHUNKS
      TDSPrintKeyHeader( &rot_track_info->rot_track_tags[key].key_header );
#endif

      /* get the rotational state information. */
      rot_track_info->rot_track_tags[key].rot_angle     = TDSReadFloat( file );
      rot_track_info->rot_track_tags[key].rot_axis[0]   = TDSReadFloat( file );
      rot_track_info->rot_track_tags[key].rot_axis[1]   = TDSReadFloat( file );
      rot_track_info->rot_track_tags[key].rot_axis[2]   = TDSReadFloat( file );
    }

  /* set frame number to end key */
  rot_track_info->rot_track_tags[rot_track_info->track_header.num_keys].key_header.frame_number = 10000;
}

/*---------------------------------------------------------------------
  Function: TDSReadSCL_TRACK_TAG
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: read the scale track tag information
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
    parent_header - the header information for the parent chunk
    scl_track_info - contains information on the scale track of the 
                     animation data
  Return: see file, scl_track_info
  Pass(s): second, thrid
---------------------------------------------------------------------*/
void TDSReadSCL_TRACK_TAG( TDSFile *file, TDSChunkHeader *parent_header,
                           tds_scl_track_info *scl_track_info )
{
  tds_ulong  key;

  /* skip this function on the first pass */
  if( pass != 1 )
    return;

  /* get the track header information */
  TDSReadTrackHeader( file, &scl_track_info->track_header );

  /* print the track header information */
#if PRINT_PROCESSED_CHUNKS
  TDSPrintTrackHeader( &scl_track_info->track_header );
#endif

  /* Allocate an extra track at the end to make searching easier */
  if( !( scl_track_info->scl_track_tags =
         ( tds_scl_track_tag * )malloc( sizeof( tds_scl_track_tag ) *
                                        (scl_track_info->track_header.num_keys + 1) ) ) )
    {
      atuError(FXTRUE, "Out of memory in TDSReadSCL_TRACK_TAG" );
    }

  /* get scale information for each key in the animation */ 
  for( key = 0; key < scl_track_info->track_header.num_keys; key++ )
    {
      /* get the key header information */
      TDSReadKeyHeader( file, &scl_track_info->scl_track_tags[key].key_header );

      /* get the key header information */
#if PRINT_PROCESSED_CHUNKS
      TDSPrintKeyHeader( &scl_track_info->scl_track_tags[key].key_header );
#endif

      /* get the scale state information. */
      scl_track_info->scl_track_tags[key].scale[0]      = TDSReadFloat( file );
      scl_track_info->scl_track_tags[key].scale[1]      = TDSReadFloat( file );
      scl_track_info->scl_track_tags[key].scale[2]      = TDSReadFloat( file );
    }

  /* set frame number to end key */
  scl_track_info->scl_track_tags[scl_track_info->track_header.num_keys].key_header.frame_number = 10000;
}

/*---------------------------------------------------------------------
  Function: TDSReadMORPH_TRACK_TAG
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: read the morphing track tag information
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
    parent_header - the header information for the parent chunk
    morph_track_info - contains information on the morph track of the 
                       animation data
  Return: see file, morph_track_info
  Pass(s): second, thrid
---------------------------------------------------------------------*/
void TDSReadMORPH_TRACK_TAG( TDSFile *file, TDSChunkHeader *parent_header,
                             tds_morph_track_info *morph_track_info )
{
  tds_ulong  key;

  /* skip this function on the first pass */
  if( pass != 1 )
    return;

  /* get the track header information */
  TDSReadTrackHeader( file, &morph_track_info->track_header );

  /* print the track header information */
#if PRINT_PROCESSED_CHUNKS
  TDSPrintTrackHeader( &morph_track_info->track_header );
#endif

  /* allocate an extra track at the end to make searching easier */
  if( !( morph_track_info->morph_track_tags =
         ( tds_morph_track_tag * )malloc( sizeof( tds_morph_track_tag ) *
                                          (morph_track_info->track_header.num_keys + 1) ) ) )
    {
      atuError(FXTRUE, "Out of memory in TDSReadMORPH_TRACK_TAG" );
    }

  /* get scale information for each key in the animation */ 
  for( key = 0; key < morph_track_info->track_header.num_keys; key++ )
    {
      /* get the key header information */
      TDSReadKeyHeader( file, &morph_track_info->morph_track_tags[key].key_header );

      /* print the key header information */
#if PRINT_PROCESSED_CHUNKS
      TDSPrintKeyHeader( &morph_track_info->morph_track_tags[key].key_header );
#endif

      /* get the scale state information */
      morph_track_info->morph_track_tags[key].named_morph_object;
      TDSReadString( file, morph_track_info->morph_track_tags[key].named_morph_object );
    }

  /* set frame number to end key */
  morph_track_info->morph_track_tags[morph_track_info->track_header.num_keys].key_header.frame_number = 10000;
}

/*---------------------------------------------------------------------
  Function: TDSReadCAMERA_NODE_TAG
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: read the camera key frame animation data
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
    parent_header - the header information for the parent chunk
  Return: see file
  Pass(s): second, thrid
---------------------------------------------------------------------*/
void TDSReadCAMERA_NODE_TAG( TDSFile *file, TDSChunkHeader *parent_header )
{
  TDSChunkHeader header;
  tds_ushort node_id;
  tds_camera_node *camera_node;

  /* skip this function on the first pass */
  if( pass != 1 )
    return;

  /* get the camera NODE_ID (optional) */
  TDSReadChunkHeader( file, &header );
  if( header.id == NODE_ID )
    {
      /* get the camera node id */
      node_id = TDSReadUShort( file );

      /* print the camera node id */
#if PRINT_PROCESSED_CHUNKS
      fprintf( error_fp, "CAMERA_NODE_TAG: node_id: %d\n", ( int )node_id );
#endif
     
      /* skip to the end of the current chunk, and get the header
         information for the next chunk */
      TDSSkipChunk( file, &header );
      TDSReadChunkHeader( file, &header );
    }
  else
    {
       /* If the node id isn't specifically mentioned, assign
          the number that represents the order in which the
          nodes are read to the current node id */
      node_id = file->current_key_node_index;

      /* print the assumed camera node id */
#if PRINT_PROCESSED_CHUNKS
      fprintf( error_fp, "CAMERA_NODE_TAG: assumed node_id: %d\n", ( int )node_id );
#endif
    }

  /* allocate space for the camera node */ 
  if( !( file->key_nodes[node_id] = ( tds_key_node * )malloc( sizeof( tds_key_node ) ) ) )
    atuError(FXTRUE, "Out of memory in TDSReadCAMERA_NODE_TAG" );
 
  /* set the type of key node */
  file->key_nodes[node_id]->type = TDS_CAMERA_NODE_TAG;

  /* localize the camera information */
  camera_node = &file->key_nodes[node_id]->nodes.camera_node;
  camera_node->node_id = node_id;
  
  /* NODE_HDR */
  if( header.id == NODE_HDR )
    {
      /* get the node header information */
      TDSReadNODE_HDR( file, &header, &camera_node->node_header );

      /* skip to the end of the current chunk and get header information
         on the next chunk */
      TDSSkipChunk( file, &header );
      TDSReadChunkHeader( file, &header );
    }
  else
    {
      /* generate an error and exit the program */
      atuError(FXTRUE, "NODE_HDR expected." );
    }

  /* loop through until the end of the chunk and get animation track 
     information */
  while( ftell( file->fp ) < ( long )( parent_header->offset + parent_header->len ) )
    {
      /* POS_TRACK_TAG */
      if( header.id == POS_TRACK_TAG )
        {
          /* get position track information */
          TDSReadPOS_TRACK_TAG( file, &header, &camera_node->pos_track_info );
        }
      /* FOV_TRACK_TAG */
      else if( header.id == FOV_TRACK_TAG )
        {
          /* get field of view track information */
          TDSReadFOV_TRACK_TAG( file, &header, &camera_node->fov_track_info );
        }
      /* ROLL_TRACK_TAG */
      else if( header.id == ROLL_TRACK_TAG )
        {
          /* get roll track informatino */
          TDSReadROLL_TRACK_TAG( file, &header, &camera_node->roll_track_info );
        }

      /* skip to the end of the current chunk, and get header information
         on the next chunk */
      TDSSkipChunk( file, &header );
      TDSReadChunkHeader( file, &header );
    }

  /* skip to the end of this chunk */
  TDSSkipChunk( file, parent_header );
}

/*---------------------------------------------------------------------
  Function: TDSReadTARGET_NODE_TAG
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: read key frame animation information for camera targets
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
    parent_header - the header information for the parent chunk
  Return: see file
  Pass(s): second, thrid
---------------------------------------------------------------------*/
void TDSReadTARGET_NODE_TAG( TDSFile *file, TDSChunkHeader *parent_header )
{
  TDSChunkHeader header;
  tds_ushort node_id;
  tds_target_node *target_node;

  /* skip this function on the first pass */
  if( pass != 1 )
    return;

  /* get the NODE_ID (optional) */
  TDSReadChunkHeader( file, &header );
  if( header.id == NODE_ID )
    {
      /* get the node id */
      node_id = TDSReadUShort( file );
 
      /* print the node id */
#if PRINT_PROCESSED_CHUNKS
      fprintf( error_fp, "TARGET_NODE_TAG: node_id: %d\n", ( int )node_id );
#endif

      /* skip to the end of the current chunk and get header information for
         the next chunk */
      TDSSkipChunk( file, &header );
      TDSReadChunkHeader( file, &header );
    }
  else
    {
      /* If the node id isn't specifically mentioned, assign
         the number that represents the order in which the
         nodes are read to the current node id */
      node_id = file->current_key_node_index;

      /* print the assumed node id */
#if PRINT_PROCESSED_CHUNKS
      fprintf( error_fp, "TARGET_NODE_TAG: assumed node_id: %d\n", ( int )node_id );
#endif
    }

  /* allocate space for the camera node */ 
  if( !( file->key_nodes[node_id] = ( tds_key_node * )malloc( sizeof( tds_key_node ) ) ) )
    atuError(FXTRUE, "Out of memory in TDSReadTARGET_NODE_TAG" );

  /* set the type of key node */
  file->key_nodes[node_id]->type = TDS_TARGET_NODE_TAG;

  /* localize the target information */
  target_node = &file->key_nodes[node_id]->nodes.target_node;
  target_node->node_id = node_id;
  
  /* NODE_HDR */
  if( header.id == NODE_HDR )
    {
      /* get the node header information */
      TDSReadNODE_HDR( file, &header, &target_node->node_header );

      /* skip to the end of the current chunk and get header information for
         next chunk */
      TDSSkipChunk( file, &header );
      TDSReadChunkHeader( file, &header );
    }
  else
    {
      /* generate and error and exit the program */
      atuError(FXTRUE, "NODE_HDR expected." );
    }

  /* loop through and get position information on the camera target */
  while( ftell( file->fp ) < ( long )( parent_header->offset + parent_header->len ) )
    {
      /* POS_TRACK_TAG */
      if( header.id == POS_TRACK_TAG )
        {
          /* get the position information */
          TDSReadPOS_TRACK_TAG( file, &header, &target_node->pos_track_info );
        }

      /* skip to the end of the current chunk, and get header information for
         the next chunk */
      TDSSkipChunk( file, &header );
      TDSReadChunkHeader( file, &header );
    }

  /* skip to the end of the current chunk */
  TDSSkipChunk( file, parent_header );
}

/*---------------------------------------------------------------------
  Function: TDSReadOBJECT_NODE_TAG
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: read key frame animation for mesh objects
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
    parent_header - the header information for the parent chunk
  Return: see file
  Pass(s): second, thrid
---------------------------------------------------------------------*/
void TDSReadOBJECT_NODE_TAG( TDSFile *file, TDSChunkHeader *parent_header )
{
  TDSChunkHeader header;
  tds_ushort node_id;
  tds_object_node *object_node;

  /* skip this fucntion on the first pass */
  if( pass != 1 )
    return;

  /* get the NODE_ID (optional) */
  TDSReadChunkHeader( file, &header );
  if( header.id == NODE_ID )
    {
      /* get the node id */
      node_id = TDSReadUShort( file );

      /* print the node id */
#if PRINT_PROCESSED_CHUNKS
      fprintf( error_fp, "read node_id: %d\n", ( int )node_id );
#endif

      /* skip to the end of the current chunk and get the header information
         for the next chunk */
      TDSSkipChunk( file, &header );
      TDSReadChunkHeader( file, &header );
    }
  else
    {
      /* If the node id isn't specifically mentioned, assign
         the number that represents the order in which the
         nodes are read to the current node id */
      node_id = file->current_key_node_index;

      /* print the assumed node id */
#if PRINT_PROCESSED_CHUNKS
      fprintf( error_fp, "assumed node_id: %d\n", ( int )node_id );
#endif
    }

  /* allocate space for the camera node */ 
  if( !( file->key_nodes[node_id] = ( tds_key_node * )malloc( sizeof( tds_key_node ) ) ) )
    atuError(FXTRUE, "Out of memory in TDSReadOBJECT_NODE_TAG" );

  /* set the type of key node */
  file->key_nodes[node_id]->type = TDS_OBJECT_NODE_TAG;

  /* localize the object node information */
  object_node = &file->key_nodes[node_id]->nodes.object_node;
  object_node->node_id = node_id;

  /* NODE_HDR */
  if( header.id == NODE_HDR )
    {
      /* get the node header information */
      TDSReadNODE_HDR( file, &header, &object_node->node_header );

      /* skip to the end of the currrent chunk and get header information
         for the next chunk */
      TDSSkipChunk( file, &header );
      TDSReadChunkHeader( file, &header );
    }
  else
    {
      /* generate error and exit program */
      atuError(FXTRUE, "NODE_HDR expected." );
    }

  /** set a default pivot */
  object_node->pivot.x = object_node->pivot.y = object_node->pivot.z = 0.0f;

  /* Set a default instance name */
  object_node->instance_name[0] = '\0';

  /* loop through until end of chun and get pivot, instance name, boundbox,
     pos_track_tag, rot_track_tag, scl_track_tag, morph_track_tag, hide_track_tag,
     and morph_smooth informtation */
  while( ftell( file->fp ) < ( long )( parent_header->offset + parent_header->len ) )
    {
      /* PIVOT */
      if( header.id == PIVOT )
        {
          /* get the point information */
          TDSReadPoint( file, &object_node->pivot );

          /* print the point information */
#if PRINT_PROCESSED_CHUNKS
          fprintf( error_fp, "pivot: %f %f %f\n", object_node->pivot.x,
                   object_node->pivot.y, object_node->pivot.z );
#endif
        }

      /* INSTANCE_NAME */
      else if( header.id == INSTANCE_NAME )
        {
          /* get the instance name information */
          TDSReadString( file, object_node->instance_name );

          /* print the instance name information */
#if PRINT_PROCESSED_CHUNKS
          fprintf( error_fp, "instance name: \"%s\"\n", object_node->instance_name );
#endif
        }
      
      /* BOUNDBOX */
      else if( header.id == BOUNDBOX )
        {
          /* get the bounding box information */
          TDSReadPoint( file, &object_node->bbox.min );
          TDSReadPoint( file, &object_node->bbox.max );
          object_node->bbox_set = 1;

          /* print the bounding box information */
#if PRINT_PROCESSED_CHUNKS
          fprintf( error_fp, "bbox.min: %f %f %f\n", object_node->bbox.min.x,
                   object_node->bbox.min.y, object_node->bbox.min.z );
          fprintf( error_fp, "bbox.max: %f %f %f\n", object_node->bbox.max.x,
                   object_node->bbox.max.y, object_node->bbox.max.z );
#endif
        }
      
      /* POS_TRACK_TAG */
      else if( header.id == POS_TRACK_TAG )
        {
          /* get the position track information */
          TDSReadPOS_TRACK_TAG( file, &header, &object_node->pos_track_info );
        }
      /* ROT_TRACK_TAG */
      else if( header.id == ROT_TRACK_TAG )
        {
          /* get the rotation track information */
          TDSReadROT_TRACK_TAG( file, &header, &object_node->rot_track_info );
        }
      /* SCL_TRACK_TAG */
      else if( header.id == SCL_TRACK_TAG )
        {
          /* get the scale track infromation */
          TDSReadSCL_TRACK_TAG( file, &header, &object_node->scl_track_info );
        }
      /* MORPH_TRACK_TAG */
      else if( header.id == MORPH_TRACK_TAG )
        {
          /* get the morph track information */
          TDSReadMORPH_TRACK_TAG( file, &header, &object_node->morph_track_info );
        }
      /* HIDE_TRACK_TAG */
      else if( header.id == HIDE_TRACK_TAG )
        {
          /* get the hide track information - not used! */
#if PRINT_SKIPPED_CHUNKS
          fprintf( error_fp, "Skipping HIDE_TRACK_TAG\n" );
#endif
        }
      /* MORPH_SMOOTH */
      else if( header.id == MORPH_SMOOTH )
        {
          /* get the morph smooth information - not used! */
#if PRINT_PROCESSED_CHUNKS
          fprintf( error_fp, "MORPH_SMOOTH\n" );
#endif
          object_node->morph_smooth = TDSReadFloat( file );
        }

      /* skip to the end of the current chunk and get header information
         for the next chunk */
      TDSSkipChunk( file, &header );
      TDSReadChunkHeader( file, &header );
    }

  /* skip to the end of the chunk */
  TDSSkipChunk( file, parent_header );
}

/*---------------------------------------------------------------------
  Function: TDSReadKFDATA
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: read key frame animation information
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
    parent_header - the header information for the parent chunk
  Return: see file
  Pass(s): first, second, thrid
---------------------------------------------------------------------*/
void TDSReadKFDATA( TDSFile *file, TDSChunkHeader *parent_header )
{
  TDSChunkHeader header;

  /* set the flag for true, that the file has animation information */
  file->has_animation = 1;

  /* print the header, representing the fact that there is animation
     information in the file */
#if PRINT_PROCESSED_CHUNKS
  fprintf( error_fp, "KFDATA\n" );
#endif

  /* get the next chunk, which must be KFHDR */
  /* KFHDR */
  ASSERT_CHUNK( KFHDR, "KFHDR" );
  TDSReadKFHDR( file, &header );

  /*
   * WARNING!!
   * There may be a VIEWPORT_LAYOUT here which isn't in the spec.
   * Just skip it.
   */
  TDSReadChunkHeader( file, &header );

  /* get the KFSEG information next, if it is the VIEWPORT_LAYOUT,
     then skip it */
  if( header.id != KFSEG )
    {
      /* skip to the end of the current chunk and get the header information
         for the next chunk */
      TDSSkipChunk( file, &header );
      TDSReadChunkHeader( file, &header );
    }

  /* KFSEG */
  if( header.id == KFSEG )
    {
#if PRINT_PROCESSED_CHUNKS
      fprintf( error_fp, "KFSEG\n" );
#endif
      /* get the active frames for the animation */
      TDSReadKFSEG( file, &header );

      /* skip to the end of the current chunk and get the header information
         for the next chunk */
      TDSSkipChunk( file, &header );
      TDSReadChunkHeader( file, &header );
    }

  /* KFCURTIME */
  if( header.id == KFCURTIME )
    {
#if PRINT_PROCESSED_CHUNKS
      fprintf( error_fp, "KFCURTIME\n" );
#endif
      /* get the active frame */
      file->current_frame = TDSReadLong( file );
      file->current_frame_set = 1;

      /* skip to the end of the current chunk and get the header information
         for the next chunk */
      TDSSkipChunk( file, &header );
      TDSReadChunkHeader( file, &header );
    }

  /* loop through and get of the node information */
  while( ftell( file->fp ) < ( long )( parent_header->offset + parent_header->len )  )
    {
      tds_key_node key_node;

      /* OBJECT_NODE_TAG */
      if( header.id == OBJECT_NODE_TAG )
        {
#if PRINT_PROCESSED_CHUNKS
          fprintf( error_fp, "OBJECT_NODE_TAG\n" );
#endif
          key_node.type = TDS_OBJECT_NODE_TAG;

          /* get object node tag information */
          TDSReadOBJECT_NODE_TAG( file, &header );
        }
      /* CAMERA_NODE_TAG */
      else if( header.id == CAMERA_NODE_TAG )
        {
#if PRINT_PROCESSED_CHUNKS
          fprintf( error_fp, "CAMERA_NODE_TAG\n" );
#endif
          key_node.type = TDS_CAMERA_NODE_TAG;

          /* get camera node information */
          TDSReadCAMERA_NODE_TAG( file, &header );
        }
      /* TARGET_NODE_TAG */
      else if( header.id == TARGET_NODE_TAG )
        {
#if PRINT_PROCESSED_CHUNKS
          fprintf( error_fp, "TARGET_NODE_TAG\n" );
#endif
          key_node.type = TDS_TARGET_NODE_TAG;

          /* get target node information */
          TDSReadTARGET_NODE_TAG( file, &header );
        }
      /* LIGHT_NODE_TAG */
      else if( header.id == LIGHT_NODE_TAG )
        {
          /* get light node information - not used! */
#if PRINT_SKIPPED_CHUNKS
          fprintf( error_fp, "Skipping LIGHT_NODE_TAG\n" );
#endif
        }
      /* SPOLIGHT_NODE_TAG */
      else if( header.id == SPOTLIGHT_NODE_TAG )
        {
          /* get spotlight information - not used! */
#if PRINT_SKIPPED_CHUNKS
          fprintf( error_fp, "Skipping SPOLIGHT_NODE_TAG\n" );
#endif
        }
      /* L_TARGET_NODE_TAG */
      else if( header.id == L_TARGET_NODE_TAG )
        {
          /* get target information - not used! */
#if PRINT_SKIPPED_CHUNKS
          fprintf( error_fp, "Skipping L_TARGET_NODE_TAG\n" );
#endif
        }
      /* AMBIENT_NODE_TAG */
      else if( header.id == AMBIENT_NODE_TAG )
        {
          /* get ambient information - not used! */
#if PRINT_SKIPPED_CHUNKS
          fprintf( error_fp, "Skipping AMBIENT_NODE_TAG\n" );
#endif
        }
      /* Unknown! */
      else
        {
          /* unknown node - generate and error and exit program */
          fprintf( error_fp, "id: 0x%04x\n", ( int )header.id );
          fprintf( error_fp, "len: %d\n", ( int )header.len );
          atuError( FXTRUE, "Unknown chunk in TDSReadKFDATA: 0x%04x\n", ( int )header.id );
        }

      /* increase the current node index */
      file->current_key_node_index++;

      /* skip to the end of the current chunk and get header information for
         the next chunk */
      TDSSkipChunk( file, &header );
      TDSReadChunkHeader( file, &header );
    }

  /* skip to the end of the current chunk */
  TDSSkipChunk( file, parent_header );
}

/*---------------------------------------------------------------------
  Function: TDSCalcPosKeyFramesToInterp
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: calculate the position frames to interpret
  Arguments: 
    pos_track_info - position track information
    frame - current frame
    lower_pos_key_index - lower position key indexes 
    higher_pos_key_index - higher position key indexes
  Return: see variables
  Pass(s): NA
---------------------------------------------------------------------*/
void TDSCalcPosKeyFramesToInterp( tds_pos_track_info *pos_track_info, int frame, 
                                 int *lower_pos_key_index, int *higher_pos_key_index )
{
  int lower, higher;

  /* set the starting point */
  lower = 0;

  /* traverse up key frame data to find the current frame  */
  while( pos_track_info->pos_track_tags[lower].key_header.frame_number <= frame )
    lower++;

  /* adjust 1 backward to compensate for the extra ++ */
  lower--;

  while( lower >= ( int )pos_track_info->track_header.num_keys  )
    lower--;
  higher = lower + 1;
  while( higher >= ( int )pos_track_info->track_header.num_keys )
    higher--;

  /* send out the lower and higher key indexes to interpolate between */
  *lower_pos_key_index = lower;
  *higher_pos_key_index = higher;
}

/*---------------------------------------------------------------------
  Function: TDSCalcRotKeyFramesToInterp
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: calculate the rotation frames to interpret
  Arguments: 
    rot_track_info - rotation track information
    frame - current frame
    lower_rot_key_index - lower rotation key indexes 
    higher_rot_key_index - higher rotation key indexes
  Return: see variables
  Pass(s): NA
---------------------------------------------------------------------*/
void TDSCalcRotKeyFramesToInterp( tds_rot_track_info *rot_track_info, int frame, 
                                 int *lower_rot_key_index, int *higher_rot_key_index )
{
  int lower, higher;

  /* set the starting point */
  lower = 0;

  /* traverse up key frame data to find the current frame */
  while( rot_track_info->rot_track_tags[lower].key_header.frame_number <= frame )
    lower++;

  /* adjust 1 backward to compensate for the extra++ */
  lower--;
  while( lower >= ( int )rot_track_info->track_header.num_keys )
    lower--;
  higher = lower + 1;
  while( higher >= ( int )rot_track_info->track_header.num_keys )
    higher--;

  /* send out the lower and higher key indexes to interpolate between */
  *lower_rot_key_index = lower;
  *higher_rot_key_index = higher;
}

/*---------------------------------------------------------------------
  Function: TDSCalcSclKeyFramesToInterp
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: calculate the scale frames to interpret
  Arguments: 
    scl_track_info - scale track information
    frame - current frame
    lower_scl_key_index - lower scale key indexes 
    higher_scl_key_index - higher scale key indexes
  Return: see variables
  Pass(s): NA
---------------------------------------------------------------------*/
void TDSCalcSclKeyFramesToInterp( tds_scl_track_info *scl_track_info, int frame, 
                                 int *lower_scl_key_index, int *higher_scl_key_index )
{
  int lower, higher;

  /* set the starting point */
  lower = 0;

  /* traverse up key frame data to find the current frame */
  while( scl_track_info->scl_track_tags[lower].key_header.frame_number <= frame )
    lower++;

  /* adjust 1 backward to compensate for the extra++ */
  lower--;
  while( lower >= ( int )scl_track_info->track_header.num_keys )
    lower--;
  higher = lower + 1;
  while( higher >= ( int )scl_track_info->track_header.num_keys )
    higher--;

  /* send out the lower and higher key indexes to interpolate between */
  *lower_scl_key_index = lower;
  *higher_scl_key_index = higher;
}

/*---------------------------------------------------------------------
  Function: TDSCalcFovKeyFramesToInterp
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: calculate the field of view frames to interpret
  Arguments: 
    fov_track_info - field of view track information
    frame - current frame
    lower_fov_key_index - lower field of view key indexes 
    higher_fov_key_index - higher field of view key indexes
  Return: see variables
  Pass(s): NA
---------------------------------------------------------------------*/
void TDSCalcFovKeyFramesToInterp( tds_fov_track_info *fov_track_info, int frame, 
                                 int *lower_fov_key_index, int *higher_fov_key_index )
{
  int lower, higher;

  /* set the starting point */
  lower = 0;

  /* traverse up key frame data to find the current frame */
  while( fov_track_info->fov_track_tags[lower].key_header.frame_number <= frame )
    lower++;

  /* adjust 1 backward to compensate for the extra++ */
  lower--;
  while( lower >= ( int )fov_track_info->track_header.num_keys )
    lower--;
  higher = lower + 1;
  while( higher >= ( int )fov_track_info->track_header.num_keys )
    higher--;

  /* send out the lower and higher key indexes to interpolate between */
  *lower_fov_key_index = lower;
  *higher_fov_key_index = higher;
}

/*---------------------------------------------------------------------
  Function: TDSCalcRollKeyFramesToInterp
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: calculate the roll angle frames to interpret
  Arguments: 
    roll_track_info - roll angle track information
    frame - current frame
    lower_roll_key_index - lower roll angle key indexes 
    higher_roll_key_index - higher roll angle key indexes
  Return: see variables
  Pass(s): NA
---------------------------------------------------------------------*/
void TDSCalcRollKeyFramesToInterp( tds_roll_track_info *roll_track_info, int frame, 
                                 int *lower_roll_key_index, int *higher_roll_key_index )
{
  int lower, higher;

  /* set the starting point */
  lower = 0;

  /* traverse up key frame data to find the current frame */
  while( roll_track_info->roll_track_tags[lower].key_header.frame_number <= frame )
    lower++;

  /* adjust 1 backward to compensate for the extra++ */
  lower--;
  while( lower >= ( int )roll_track_info->track_header.num_keys )
    lower--;
  higher = lower + 1;
  while( higher >= ( int )roll_track_info->track_header.num_keys )
    higher--;

  /* send out the lower and higher key indexes to interpolate between */
  *lower_roll_key_index = lower;
  *higher_roll_key_index = higher;
}

/*---------------------------------------------------------------------
  Function: TDSInterpFrameScale
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: calculate the frame scale for the scene
  Arguments: 
    S - scale matrix
    frame - current frame
    lower - lower frame
    upper - upper frame
    previous - previous frame
    following - following frame
  Return: see variables
  Pass(s): NA
---------------------------------------------------------------------*/
void TDSInterpFrameScale( tds_matrix S, int frame, tds_scl_track_tag *lower, 
                          tds_scl_track_tag *upper, tds_scl_track_tag *previous,
                          tds_scl_track_tag *following )
{
  int lower_frame, upper_frame, previous_frame, following_frame;
  float interp_factor;

  /* figure out the frame numbers for the start and stop of the sequence that
     we are in */
  previous_frame = previous->key_header.frame_number;
  lower_frame =  lower->key_header.frame_number;
  upper_frame = upper->key_header.frame_number;
  following_frame = following->key_header.frame_number;

  if( lower_frame != upper_frame )
    interp_factor = ( float )( frame - lower_frame ) / ( upper_frame - lower_frame );
  else
    interp_factor = 0.0f;

  /* build the matrix which will represent this scale */
  TDSMatMakeIdent( S );

  /* linear interpretation of change in scale */
#if LINEAR_INTERP
  S[0][0] = TDSLinearInterpolate( lower->scale[0], upper->scale[0], interp_factor );
  S[1][1] = TDSLinearInterpolate( lower->scale[1], upper->scale[1], interp_factor );
  S[2][2] = TDSLinearInterpolate( lower->scale[2], upper->scale[2], interp_factor );
#endif

  /* hermite interp of change in scale */
  S[0][0] = TDSHermiteInterpolate( previous->scale[0], lower->scale[0], upper->scale[0], following->scale[0],
                                   lower->key_header.spline_tension, lower->key_header.spline_continuinity,
                                   lower->key_header.spline_bias, upper->key_header.spline_tension, 
                                   upper->key_header.spline_continuinity, upper->key_header.spline_bias,
                                   interp_factor );
  S[1][1] = TDSHermiteInterpolate( previous->scale[1], lower->scale[1], upper->scale[1], following->scale[1],
                                   lower->key_header.spline_tension, lower->key_header.spline_continuinity,
                                   lower->key_header.spline_bias, upper->key_header.spline_tension, 
                                   upper->key_header.spline_continuinity, upper->key_header.spline_bias,
                                   interp_factor );
  S[2][2] = TDSHermiteInterpolate( previous->scale[2], lower->scale[2], upper->scale[2], following->scale[2],
                                   lower->key_header.spline_tension, lower->key_header.spline_continuinity,
                                   lower->key_header.spline_bias, upper->key_header.spline_tension, 
                                   upper->key_header.spline_continuinity, upper->key_header.spline_bias,
                                   interp_factor );
}

/*---------------------------------------------------------------------
  Function: TDSInterpFrameRot
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: calculate the frame rotation for the scene
  Arguments: 
    R - rotation matrix
    frame - current frame
    lower - lower frame
    upper - upper frame
    previous - previous frame
    following - following frame
  Return: see variables
  Pass(s): NA
---------------------------------------------------------------------*/
void TDSInterpFrameRot( tds_matrix R, int frame, tds_rot_track_tag *lower, tds_rot_track_tag *upper,
                        tds_rot_track_tag *previous, tds_rot_track_tag *following )
{
  int lower_frame, upper_frame, previous_frame, following_frame;
  float interp_factor;
  float angle;

  /* figure out the frame numbers for the start and stop of the sequence that
     we are in */
  lower_frame =  lower->key_header.frame_number;
  upper_frame = upper->key_header.frame_number;
  previous_frame = previous->key_header.frame_number;
  following_frame = following->key_header.frame_number;

  if( lower_frame != upper_frame )
    interp_factor = ( float )( frame - lower_frame ) / ( upper_frame - lower_frame );
  else
    interp_factor = 0.0f;

  /* linear interpretation of change in rotation */
#if LINEAR_INTERP
  angle = TDSLinearInterpolate( 0.0f, upper->rot_angle, interp_factor );
#endif

  /* hermite interpretation of change in rotation */
  angle = TDSHermiteInterpolate( 0.0f, 0.0f, upper->rot_angle, 0.0f, lower->key_header.spline_tension,
                                 lower->key_header.spline_continuinity, lower->key_header.spline_bias,
                                 upper->key_header.spline_tension, upper->key_header.spline_continuinity,
                                 upper->key_header.spline_bias, interp_factor );

  /* build the matrix which will represent this rotation */
  TDSBuildRotMatrixAboutAxis( R, upper->rot_axis, angle );
}

/*---------------------------------------------------------------------
  Function: TDSInterpFrameFov
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: calculate the frame field of view for the scene
  Arguments: 
    frame - current frame
    lower - lower frame
    upper - upper frame
    previous - previous frame
    following - following frame
  Return: see variables
  Pass(s): NA
---------------------------------------------------------------------*/
float TDSInterpFrameFov( int frame, tds_fov_track_tag *lower, tds_fov_track_tag *upper,
                         tds_fov_track_tag *previous, tds_fov_track_tag *following )
{
  int lower_frame, upper_frame, previous_frame, following_frame;
  float interp_factor;
  float angle;

  /* figure out the frame numbers for the start and stop of the sequence that
     we are in */
  lower_frame =  lower->key_header.frame_number;
  upper_frame = upper->key_header.frame_number;
  previous_frame = previous->key_header.frame_number;
  following_frame = following->key_header.frame_number;

  if( lower_frame != upper_frame )
    interp_factor = ( float )( frame - lower_frame ) / ( upper_frame - lower_frame );
  else
    interp_factor = 0.0f;

  /* linear interpretation of field of view */
#if LINEAR_INTERP
  angle = TDSLinearInterpolate( lower->fov_degrees, upper->fov_degrees, interp_factor );
#endif

  /* hermite interpretation of field of view */
  angle = TDSHermiteInterpolate( previous->fov_degrees, lower->fov_degrees, 
                                 upper->fov_degrees, following->fov_degrees,
                                 lower->key_header.spline_tension, lower->key_header.spline_continuinity,
                                 lower->key_header.spline_bias, upper->key_header.spline_tension,
                                 upper->key_header.spline_continuinity, upper->key_header.spline_bias,
                                 interp_factor );

  /* field of view angle */
  return angle;
}

/*---------------------------------------------------------------------
  Function: TDSInterpFrameRoll
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: calculate the frame roll angle for the scene
  Arguments: 
    frame - current frame
    lower - lower frame
    upper - upper frame
    previous - previous frame
    following - following frame
  Return: see variables
  Pass(s): NA
---------------------------------------------------------------------*/
float TDSInterpFrameRoll( int frame, tds_roll_track_tag *lower, tds_roll_track_tag *upper,
                          tds_roll_track_tag *previous, tds_roll_track_tag *following )
{
  int lower_frame, upper_frame, previous_frame, following_frame;
  float interp_factor;
  float angle;

  /* figure out the frame numbers for the start and stop of the sequence that
     we are in */
  lower_frame =  lower->key_header.frame_number;
  upper_frame = upper->key_header.frame_number;
  previous_frame = previous->key_header.frame_number;
  following_frame = following->key_header.frame_number;

  if( lower_frame != upper_frame )
    interp_factor = ( float )( frame - lower_frame ) / ( upper_frame - lower_frame );
  else
    interp_factor = 0.0f;

  /* linear interpretation of roll angle */
#if LINEAR_INTERP
  angle = TDSLinearInterpolate( lower->roll_degrees, upper->roll_degrees, interp_factor );
#endif

  /* hermite interpretation of roll angle */
  angle = TDSHermiteInterpolate( previous->roll_degrees, lower->roll_degrees, 
                                 upper->roll_degrees, following->roll_degrees,
                                 lower->key_header.spline_tension, lower->key_header.spline_continuinity,
                                 lower->key_header.spline_bias, upper->key_header.spline_tension,
                                 upper->key_header.spline_continuinity, upper->key_header.spline_bias,
                                 interp_factor );

  /* roll angle */
  return angle;
}

/*---------------------------------------------------------------------
  Function: TDSInterpFrameTrans
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: calculate the frame roll angle for the scene
  Arguments: 
    T - translation matrix
    frame - current frame
    lower - lower frame
    upper - upper frame
    previous - previous frame
    following - following frame
  Return: see variables
  Pass(s): NA
---------------------------------------------------------------------*/
void TDSInterpFrameTrans( tds_matrix T, int frame, tds_pos_track_tag *lower, tds_pos_track_tag *upper,
                          tds_pos_track_tag *previous, tds_pos_track_tag *following )
{
  int lower_frame, upper_frame, previous_frame, following_frame;
  float interp_factor;

  /* figure out the frame numbers for the start and stop of the sequence that
     we are in */
  lower_frame = lower->key_header.frame_number;
  upper_frame = upper->key_header.frame_number;
  previous_frame = previous->key_header.frame_number;
  following_frame = following->key_header.frame_number;

  if( lower_frame != upper_frame )
    interp_factor = ( float )( frame - lower_frame ) / ( upper_frame - lower_frame );
  else
    interp_factor = 0.0f;

  /* build the matrix which will represent this rotation */
  TDSMatMakeIdent( T );

  /* linear interpretation of translation */
#if LINEAR_INTERP
  T[3][0] = TDSLinearInterpolate( lower->position.x, upper->position.x, interp_factor );
  T[3][1] = TDSLinearInterpolate( lower->position.y, upper->position.y, interp_factor );
  T[3][2] = TDSLinearInterpolate( lower->position.z, upper->position.z, interp_factor );
#endif

  /* hermite interpretation of translation */
  T[3][0] = TDSHermiteInterpolate( previous->position.x, lower->position.x, upper->position.x,
                                   following->position.x, lower->key_header.spline_tension,
                                   lower->key_header.spline_continuinity, lower->key_header.spline_bias,
                                   upper->key_header.spline_tension, upper->key_header.spline_continuinity,
                                   upper->key_header.spline_bias, interp_factor );
  T[3][1] = TDSHermiteInterpolate( previous->position.y, lower->position.y, upper->position.y,
                                   following->position.y, lower->key_header.spline_tension,
                                   lower->key_header.spline_continuinity, lower->key_header.spline_bias,
                                   upper->key_header.spline_tension, upper->key_header.spline_continuinity,
                                   upper->key_header.spline_bias, interp_factor );
  T[3][2] = TDSHermiteInterpolate( previous->position.z, lower->position.z, upper->position.z,
                                   following->position.z, lower->key_header.spline_tension,
                                   lower->key_header.spline_continuinity, lower->key_header.spline_bias,
                                   upper->key_header.spline_tension, upper->key_header.spline_continuinity,
                                   upper->key_header.spline_bias, interp_factor );
}

/*---------------------------------------------------------------------
  Function: TDSCalcKeyRotationMatrices
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: calculate the rotation matrices for the animation
  Arguments: 
    rot_track_info - rotation track information
  Return: see variables
  Pass(s): NA
---------------------------------------------------------------------*/
void TDSCalcKeyRotationMatrices( tds_rot_track_info *rot_track_info )
{
  int i;
  tds_matrix m;

  /* build rotation matrix about given rotation axis */
  TDSBuildRotMatrixAboutAxis( rot_track_info->rot_track_tags[0].rot_matrix,
                             rot_track_info->rot_track_tags[0].rot_axis,
                             rot_track_info->rot_track_tags[0].rot_angle );

  /* traverse through the rotation keys and create the rotation
     matrices based on the change in the key values */
  for( i = 1; i < ( int )rot_track_info->track_header.num_keys; i++ )
    {
      TDSBuildRotMatrixAboutAxis( m,
                                 rot_track_info->rot_track_tags[i].rot_axis,
                                 rot_track_info->rot_track_tags[i].rot_angle );
      TDSMatMult( rot_track_info->rot_track_tags[i].rot_matrix, 
                 rot_track_info->rot_track_tags[i-1].rot_matrix, 
                 m );
    }
}

/*---------------------------------------------------------------------
  Function: TDSMatrixProcessCameraNode
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: calculates camera movements
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
    camera_node - contains the camera node information
  Return: see variables
  Pass(s): NA
---------------------------------------------------------------------*/
void TDSMatrixProcessCameraNode( TDSFile *file, tds_camera_node *camera_node )
{
  int i;
  char *target_name;
  int target_node_index = -1;
  tds_target_node *target_node;
  int frame;
  tds_matrix camera_trans, target_trans, tmpmat;

  /* allocate space for the state information associated with this camera */
  if( !( file->camera_state[camera_node->node_id] = ( tds_camera_state * )malloc( sizeof( tds_camera_state ) * ( file->animation_length + 1 ) ) ) )
    {
      atuError(FXTRUE, "Not able to allocate camera state.\n" );
    }

  /* figure out what camera target goes with this node */
  target_name = camera_node->node_header.object_name;

  for( i = 0; i < 65536; i++ )
    {
      if( file->key_nodes[i] && file->key_nodes[i]->type == TDS_TARGET_NODE_TAG )
        {
          /*
           * The current node is at least a target node.  Now test if it is the one
           * that goes with the current camera node.
           */
          if( strcmp( target_name, file->key_nodes[i]->nodes.target_node.node_header.object_name ) == 0 )
            {
              target_node_index = i;
              break;
            }
        }
    }

  /* no target node was found, generate error and exit program */
  if( target_node_index == -1 )
    {
      atuError(FXTRUE, "ERROR: Camera \"%s\" does not have a target node.\n", target_name );
    }

  /* localize the target node */
  target_node = &file->key_nodes[target_node_index]->nodes.target_node;

  /* loop through the animation and interp camera movement */
  for( frame = 0; frame < file->animation_length; frame++ )
    {
      int camera_lower_pos_key_index, camera_higher_pos_key_index;
      int camera_previous_pos_key_index, camera_following_pos_key_index;
      int target_lower_pos_key_index, target_higher_pos_key_index;
      int target_previous_pos_key_index, target_following_pos_key_index;
      int lower_fov_key_index, higher_fov_key_index;
      int previous_fov_key_index, following_fov_key_index;
      int lower_roll_key_index, higher_roll_key_index;
      int previous_roll_key_index, following_roll_key_index;
      float fov, roll;

      /* target following and previous never used */
      target_previous_pos_key_index = target_following_pos_key_index = 0;

      /* initiate Previous and Following FOV and Roll */
      if( frame == 0 )
        {
          TDSCalcFovKeyFramesToInterp( &camera_node->fov_track_info, frame, 
                                       &lower_fov_key_index, &higher_fov_key_index );
          previous_fov_key_index = lower_fov_key_index;
          TDSCalcRollKeyFramesToInterp( &camera_node->roll_track_info, frame, 
                                       &lower_roll_key_index, &higher_roll_key_index );
          previous_roll_key_index = lower_roll_key_index;
          TDSCalcPosKeyFramesToInterp( &camera_node->pos_track_info,  frame, 
                                       &camera_lower_pos_key_index, &camera_higher_pos_key_index );
          camera_previous_pos_key_index = camera_lower_pos_key_index;;

          TDSCalcFovKeyFramesToInterp( &camera_node->fov_track_info, ( frame + 1 ),
                                       &higher_fov_key_index, &following_fov_key_index );
          TDSCalcRollKeyFramesToInterp( &camera_node->roll_track_info, ( frame + 1 ),
                                       &higher_roll_key_index, &following_roll_key_index );
          TDSCalcPosKeyFramesToInterp( &camera_node->pos_track_info, ( frame + 1 ), 
                                       &camera_higher_pos_key_index, &camera_following_pos_key_index );
        }
      else if( frame == ( file->animation_length - 1 ) )
        {
          TDSCalcFovKeyFramesToInterp( &camera_node->fov_track_info, ( frame - 1 ),
                                       &previous_fov_key_index, &lower_fov_key_index );
          TDSCalcRollKeyFramesToInterp( &camera_node->roll_track_info, ( frame - 1 ),
                                       &previous_roll_key_index, &lower_roll_key_index );
          TDSCalcPosKeyFramesToInterp( &camera_node->pos_track_info, ( frame - 1 ), 
                                       &camera_previous_pos_key_index, &camera_lower_pos_key_index );

          TDSCalcFovKeyFramesToInterp( &camera_node->fov_track_info, frame, 
                                       &lower_fov_key_index, &higher_fov_key_index );
          following_fov_key_index = higher_fov_key_index;
          TDSCalcRollKeyFramesToInterp( &camera_node->roll_track_info, frame, 
                                       &lower_roll_key_index, &higher_roll_key_index );
          following_roll_key_index = higher_roll_key_index;
          TDSCalcPosKeyFramesToInterp( &camera_node->pos_track_info,  frame, 
                                       &camera_lower_pos_key_index, &camera_higher_pos_key_index );
          camera_following_pos_key_index = camera_higher_pos_key_index;;
        }
      else
        {
          TDSCalcFovKeyFramesToInterp( &camera_node->fov_track_info, ( frame - 1 ),
                                       &previous_fov_key_index, &lower_fov_key_index );
          TDSCalcRollKeyFramesToInterp( &camera_node->roll_track_info, ( frame - 1 ),
                                       &previous_roll_key_index, &lower_roll_key_index );
          TDSCalcPosKeyFramesToInterp( &camera_node->pos_track_info, ( frame - 1 ), 
                                       &camera_previous_pos_key_index, &camera_lower_pos_key_index );

          TDSCalcFovKeyFramesToInterp( &camera_node->fov_track_info, ( frame + 1 ),
                                       &higher_fov_key_index, &following_fov_key_index );
          TDSCalcRollKeyFramesToInterp( &camera_node->roll_track_info, ( frame + 1 ),
                                       &higher_roll_key_index, &following_roll_key_index );
          TDSCalcPosKeyFramesToInterp( &camera_node->pos_track_info, ( frame + 1 ), 
                                       &camera_higher_pos_key_index, &camera_following_pos_key_index );
        }
      
      /*
       * Figure out the keyframes for both the camera and the target which should be
       * used to interpolate between for this frame.
       */
      TDSCalcPosKeyFramesToInterp( &camera_node->pos_track_info, frame, 
                                  &camera_lower_pos_key_index, &camera_higher_pos_key_index );
      TDSCalcPosKeyFramesToInterp( &target_node->pos_track_info, frame, 
                                  &target_lower_pos_key_index, &target_higher_pos_key_index );



      /*
       * Figure out which FOV (field of view) keyframes to interpolate between for 
       * the camera.
       */
      TDSCalcFovKeyFramesToInterp( &camera_node->fov_track_info, frame,
                                  &lower_fov_key_index, &higher_fov_key_index );

      /*
       * Figure out which roll keyframes to interpolate between for the camera.
       */
      TDSCalcRollKeyFramesToInterp( &camera_node->roll_track_info, frame,
                                   &lower_roll_key_index, &higher_roll_key_index );

      /*
       * Figure out a translation matrix for both the camera and the target for this frame.
       * Note that these matrices will not be used as is, but instead the translation part
       * will be yanked out to build a "lookat" matrix given the two positions.
       * Note that these matrices are relative to the parents of the target and camera
       * if they have them.
       */
      TDSInterpFrameTrans( camera_trans, frame,
                          &camera_node->pos_track_info.pos_track_tags[camera_lower_pos_key_index],
                          &camera_node->pos_track_info.pos_track_tags[camera_higher_pos_key_index],
                          &camera_node->pos_track_info.pos_track_tags[camera_previous_pos_key_index],
                          &camera_node->pos_track_info.pos_track_tags[camera_following_pos_key_index] );
      TDSInterpFrameTrans( target_trans, frame,
                          &target_node->pos_track_info.pos_track_tags[target_lower_pos_key_index],
                          &target_node->pos_track_info.pos_track_tags[target_higher_pos_key_index],
                          &target_node->pos_track_info.pos_track_tags[target_previous_pos_key_index],
                          &target_node->pos_track_info.pos_track_tags[target_following_pos_key_index] );

      /*
       * Figure out interpolated camera FOV for this frame.
       */
      fov = TDSInterpFrameFov( frame,
                              &camera_node->fov_track_info.fov_track_tags[lower_fov_key_index],
                              &camera_node->fov_track_info.fov_track_tags[higher_fov_key_index],
                              &camera_node->fov_track_info.fov_track_tags[previous_fov_key_index],
                              &camera_node->fov_track_info.fov_track_tags[following_fov_key_index] );

      /*
       * Figure out interpolated camera roll for this frame.
       */
      roll = TDSInterpFrameRoll( frame,
                                &camera_node->roll_track_info.roll_track_tags[lower_roll_key_index],
                                &camera_node->roll_track_info.roll_track_tags[higher_roll_key_index],
                                &camera_node->roll_track_info.roll_track_tags[previous_roll_key_index],
                                &camera_node->roll_track_info.roll_track_tags[following_roll_key_index] );

      /*
       * WARNING!!!  I'm assuming that the parents pivot point for either the 
       * camera or the target does not effect the position of the camera or
       * the target.  This is probably an incorrect assumption.
       */

      /*
       * Figure out the positions of the camera and target within the world.
       * We already know the position relative to their parents.
       */
      if( camera_node->node_header.parent_index == 65535 )
        {
          /*
           * The camera does not have a parent - not used!
           */
        }
      else
        {
          /*
           * The camera does have a parent. . . modify the position of the camera to 
           * reflect this.
           */
          TDSMatMult( tmpmat, camera_trans, 
                     file->world_matrices[camera_node->node_header.parent_index][frame] );
          memcpy( camera_trans, tmpmat, sizeof( tds_matrix ) );
        }

      if( target_node->node_header.parent_index == 65535 )
        {
          /*
           * The target does not have a parent - not used!
           */
        }
      else
        {
          /*
           * The target does have a parent.
           */
          TDSMatMult( tmpmat, target_trans, 
                     file->world_matrices[target_node->node_header.parent_index][frame] );
          memcpy( target_trans, tmpmat, sizeof( tds_matrix ) );
        }

      /* print camera information */
#if 0
      fprintf( error_fp, "camera position: %f %f %f\n", camera_trans[3][0], 
              camera_trans[3][1], camera_trans[3][2] );
      fprintf( error_fp, "target position: %f %f %f\n", target_trans[3][0], 
              target_trans[3][1], target_trans[3][2] );
      fprintf( error_fp, "camera fov: %f (frame %d)\n", fov, ( int )frame );
      fprintf( error_fp, "camera roll: %f (frame %d)\n", roll, ( int )frame );
#endif

      /*
       * Store the matrix for the position of the camera where the
       * world transform matrix for the camera should go so that we can draw
       * the point where the camera is supposed to be. This is used for debugging.
       */
      file->camera_state[camera_node->node_id][frame].camera_position.x = camera_trans[3][0];
      file->camera_state[camera_node->node_id][frame].camera_position.y = camera_trans[3][1];
      file->camera_state[camera_node->node_id][frame].camera_position.z = camera_trans[3][2];
      file->camera_state[camera_node->node_id][frame].target_position.x = target_trans[3][0];
      file->camera_state[camera_node->node_id][frame].target_position.y = target_trans[3][1];
      file->camera_state[camera_node->node_id][frame].target_position.z = target_trans[3][2];
      file->camera_state[camera_node->node_id][frame].roll_degrees = roll;
      file->camera_state[camera_node->node_id][frame].fov_degrees = fov;
    }  
}

/*---------------------------------------------------------------------
  Function: TDSMatrixProcessObjectNode
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: calculates object movements
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
    object_node - contains the object node information
  Return: see variables
  Pass(s): NA
---------------------------------------------------------------------*/
void TDSMatrixProcessObjectNode( TDSFile *file, tds_object_node *object_node )
{
  tds_matrix P, Pinv, Sframe, Rinterp, T, Rkey, ParentP, ParentPinv;
  tds_matrix tmpmat, tmpmat2, finalmat, ident;
  int frame;

  TDSMatMakeIdent( ident );

  /*
   * Allocate space for the matrices associated with this object.
   */
  if( !( file->local_matrices[object_node->node_id] = 
        ( tds_matrix * )malloc( sizeof( tds_matrix ) * ( file->animation_length + 1 ) ) ) )
    atuError(FXTRUE, "Out of memory in TDSMatrixProcessObjectNode" );

  if( !( file->world_matrices[object_node->node_id] = 
        ( tds_matrix * )malloc( sizeof( tds_matrix ) * ( file->animation_length + 1 ) ) ) )
    atuError(FXTRUE, "Out of memory in TDSMatrixProcessObjectNode" );
  
  /*
   * Calculate the rotation matrices at each of the keyframes.
   */
  TDSCalcKeyRotationMatrices( &object_node->rot_track_info );

  /*
   * Iterate through each frame of animation for the object and figure
   * out what it's local and global transform matrices should be.
   *
   * The transform for a particular object is built as follow:
   * 
   * P-1 * Sframe * Rkey * Rinterp * P * T
   * 
   * where:
   * 
   * P = matrix consisting of a translation which is equal to the pivot point.
   * Pinv = the inverse of P
   * Sframe = the scale matrix for the object at the particular frame.
   * Rkey = the cumulative rotation at the "low" key frame.
   * Rinterp = the rotation incurred since the last key frame.
   * Tkey = the translation value for the object at the particular frame.
   *
   * NOTE: In 3ds files, translation stated for each key frame as the
   * position of the object at that time, while rotation is cumulative
   * and any rotation keys are to build on previous rotations.
   */
  for( frame = 0; frame < file->animation_length; frame++ )
    {
      int lower_pos_key_index, higher_pos_key_index;
      int previous_pos_key_index, following_pos_key_index; 
      int lower_rot_key_index, higher_rot_key_index;
      int previous_rot_key_index, following_rot_key_index; 
      int lower_scl_key_index, higher_scl_key_index;
      int previous_scl_key_index, following_scl_key_index; 


      /* calculate previous and following indexes */
      if( frame == 0 )
        {
          TDSCalcPosKeyFramesToInterp( &object_node->pos_track_info, frame, 
                                       &lower_pos_key_index, &higher_pos_key_index );
          previous_pos_key_index = lower_pos_key_index;
          TDSCalcRotKeyFramesToInterp( &object_node->rot_track_info, frame, 
                                       &lower_rot_key_index, &higher_rot_key_index );
          previous_rot_key_index = lower_rot_key_index;
          TDSCalcSclKeyFramesToInterp( &object_node->scl_track_info, frame, 
                                       &lower_scl_key_index, &higher_scl_key_index );
          previous_scl_key_index = lower_scl_key_index;
          
          TDSCalcPosKeyFramesToInterp( &object_node->pos_track_info, ( frame + 1 ),
                                       &higher_pos_key_index, &following_pos_key_index );
          TDSCalcRotKeyFramesToInterp( &object_node->rot_track_info, ( frame + 1 ), 
                                       &higher_rot_key_index, &following_rot_key_index );
          TDSCalcSclKeyFramesToInterp( &object_node->scl_track_info, ( frame + 1 ), 
                                       &higher_scl_key_index, &following_scl_key_index );
        }
      else if( frame == ( file->animation_length - 1 ) )
        {
          TDSCalcPosKeyFramesToInterp( &object_node->pos_track_info, ( frame - 1 ),
                                       &previous_pos_key_index, &lower_pos_key_index );
          TDSCalcRotKeyFramesToInterp( &object_node->rot_track_info, ( frame - 1 ), 
                                       &previous_rot_key_index, &lower_rot_key_index );
          TDSCalcSclKeyFramesToInterp( &object_node->scl_track_info, ( frame - 1 ), 
                                       &previous_scl_key_index, &lower_scl_key_index );

          TDSCalcPosKeyFramesToInterp( &object_node->pos_track_info, frame, 
                                       &lower_pos_key_index, &higher_pos_key_index );
          following_pos_key_index = higher_pos_key_index;
          TDSCalcRotKeyFramesToInterp( &object_node->rot_track_info, frame, 
                                       &lower_rot_key_index, &higher_rot_key_index );
          following_rot_key_index = higher_rot_key_index;
          TDSCalcSclKeyFramesToInterp( &object_node->scl_track_info, frame, 
                                       &lower_scl_key_index, &higher_scl_key_index );
          following_scl_key_index = higher_scl_key_index;
        }
      else
        {
          TDSCalcPosKeyFramesToInterp( &object_node->pos_track_info, ( frame - 1 ),
                                       &previous_pos_key_index, &lower_pos_key_index );
          TDSCalcRotKeyFramesToInterp( &object_node->rot_track_info, ( frame - 1 ), 
                                       &previous_rot_key_index, &lower_rot_key_index );
          TDSCalcSclKeyFramesToInterp( &object_node->scl_track_info, ( frame - 1 ), 
                                       &previous_scl_key_index, &lower_scl_key_index );

          TDSCalcPosKeyFramesToInterp( &object_node->pos_track_info, ( frame + 1 ),
                                       &higher_pos_key_index, &following_pos_key_index );
          TDSCalcRotKeyFramesToInterp( &object_node->rot_track_info, ( frame + 1 ), 
                                       &higher_rot_key_index, &following_rot_key_index );
          TDSCalcSclKeyFramesToInterp( &object_node->scl_track_info, ( frame + 1 ), 
                                       &higher_scl_key_index, &following_scl_key_index );
        }

      TDSCalcPosKeyFramesToInterp( &object_node->pos_track_info, frame, 
                                  &lower_pos_key_index, &higher_pos_key_index );
      
      TDSCalcRotKeyFramesToInterp( &object_node->rot_track_info, frame, 
                                  &lower_rot_key_index, &higher_rot_key_index );

      TDSCalcSclKeyFramesToInterp( &object_node->scl_track_info, frame, 
                                  &lower_scl_key_index, &higher_scl_key_index );
      
      /* print the object movement results */
#if 0
      fprintf( error_fp, "pos: frame: %d lower: %d higher: %d\n", frame,
               object_node->pos_track_info.pos_track_tags[lower_pos_key_index].key_header.frame_number,
               object_node->pos_track_info.pos_track_tags[higher_pos_key_index].key_header.frame_number );
      fprintf( error_fp, "rot: frame: %d lower: %d higher: %d\n", frame,
               object_node->rot_track_info.rot_track_tags[lower_rot_key_index].key_header.frame_number,
               object_node->rot_track_info.rot_track_tags[higher_rot_key_index].key_header.frame_number );
      fprintf( error_fp, "scl: frame: %d lower: %d higher: %d\n", frame,
               object_node->scl_track_info.scl_track_tags[lower_scl_key_index].key_header.frame_number,
               object_node->scl_track_info.scl_track_tags[higher_scl_key_index].key_header.frame_number );
#endif

      /*
       * Build the matrices to translate the object's origin from its centroid
       * to the given pivot point and back.
       */
      TDSMatMakeIdent( P );
      TDSMatMakeIdent( Pinv );
      P[3][0]    =  object_node->pivot.x;
      P[3][1]    =  object_node->pivot.y;
      P[3][2]    =  object_node->pivot.z;
      Pinv[3][0] = -object_node->pivot.x;
      Pinv[3][1] = -object_node->pivot.y;
      Pinv[3][2] = -object_node->pivot.z;
      

      TDSMatMakeIdent( ParentP );
      TDSMatMakeIdent( ParentPinv );
      if( object_node->node_header.parent_index != 65535 && 
         file->key_nodes[object_node->node_header.parent_index]->type == TDS_OBJECT_NODE_TAG )
        {
          tds_point *pivot;

          /* 
           * WARNING!!!!  This assumes that the parent is an object. . . this won't
           * work if the parent is not an object.  Does this ever happen?
           */
          pivot = &file->key_nodes[object_node->node_header.parent_index]->nodes.object_node.pivot;
          ParentP[3][0] = pivot->x;
          ParentP[3][1] = pivot->y;
          ParentP[3][2] = pivot->z;
          ParentPinv[3][0] = -pivot->x;
          ParentPinv[3][1] = -pivot->y;
          ParentPinv[3][2] = -pivot->z;
        }

#if PRINT_MATRICES
      fprintf( error_fp, "Pinv:\n" );
      TDSPrintMatrix( Pinv );
      fprintf( error_fp, "P:\n" );
      TDSPrintMatrix( P );
#endif

      /*
       * Figure out Sframe.
       */
      TDSInterpFrameScale( Sframe, frame,
                          &object_node->scl_track_info.scl_track_tags[lower_scl_key_index],
                          &object_node->scl_track_info.scl_track_tags[higher_scl_key_index],
                          &object_node->scl_track_info.scl_track_tags[previous_scl_key_index],
                          &object_node->scl_track_info.scl_track_tags[following_scl_key_index] );
      
#if PRINT_MATRICES
      fprintf( error_fp, "Sframe:\n" );
      TDSPrintMatrix( Sframe );
#endif

      /*
       * figure out Rinterp. ( cab )
       */
      TDSInterpFrameRot( Rinterp, frame, 
                        &object_node->rot_track_info.rot_track_tags[lower_rot_key_index],
                        &object_node->rot_track_info.rot_track_tags[higher_rot_key_index],
                        &object_node->rot_track_info.rot_track_tags[previous_rot_key_index],
                        &object_node->rot_track_info.rot_track_tags[following_rot_key_index] );

#if PRINT_MATRICES
      fprintf( error_fp, "Rinterp:\n" );
      TDSPrintMatrix( Rinterp );
#endif

      /*
       * Figure out T.
       */
      TDSInterpFrameTrans( T, frame, 
                          &object_node->pos_track_info.pos_track_tags[lower_pos_key_index],
                          &object_node->pos_track_info.pos_track_tags[higher_pos_key_index],
                          &object_node->pos_track_info.pos_track_tags[previous_pos_key_index],
                          &object_node->pos_track_info.pos_track_tags[following_pos_key_index] );

#if PRINT_MATRICES
      fprintf( error_fp, "T:\n" );
      TDSPrintMatrix( T );
#endif

      /*
       * Make a copy of the rotation matrix for the starting key.
       */
      memcpy( Rkey, &object_node->rot_track_info.rot_track_tags[lower_rot_key_index].rot_matrix,
             sizeof( tds_matrix ) );

#if PRINT_MATRICES
      fprintf( error_fp, "Rkey:\n" );
      TDSPrintMatrix( Rkey );
#endif

      /*
       * Multiply these matrices together to get the total matrix for the
       * current frame for this object. 
       *
       * Pinv * Rkey * Rinterp * Sframe * P * ParentP * T * Pinv
       */
#if 0 /* this one works! */
      TDSMatMult( tmpmat,  Pinv,     Rkey );
      TDSMatMult( tmpmat2, tmpmat,   Rinterp );
      TDSMatMult( tmpmat,  tmpmat2,  Sframe );
      TDSMatMult( tmpmat2, tmpmat,   P );
      TDSMatMult( tmpmat,  tmpmat2,  ParentP );
      TDSMatMult( tmpmat2, tmpmat,   T );
      TDSMatMult( finalmat, tmpmat2, Pinv );
#endif

#if 1
      TDSMatMult( tmpmat,  Pinv,     Rkey );
      TDSMatMult( tmpmat2, tmpmat,   Rinterp );
      TDSMatMult( tmpmat,  tmpmat2,  Sframe );
      TDSMatMult( tmpmat2, tmpmat,   P );
      TDSMatMult( tmpmat,  tmpmat2,  ParentP );
      TDSMatMult( tmpmat2, tmpmat,   T );
      TDSMatMult( finalmat, tmpmat2, Pinv );
#endif

#if PRINT_MATRICES
      fprintf( error_fp, "finalmat %d\n", ( int )object_node->node_id );
      TDSPrintMatrix( finalmat );
#endif

      /*
       * Store the resulting local matrix.
       */
      memcpy( &file->local_matrices[object_node->node_id][frame], finalmat, sizeof( tds_matrix ) );

      /*
       * Now the the local matrix for the object is made, left multiply the parents
       * matrix to that to give the world matrix for the object.
       */
      if( object_node->node_header.parent_index == 65535 )
        {
          /*
           * Since there is no parent, just copy the local matrix to the world
           * matrix.
           */
          memcpy( &file->world_matrices[object_node->node_id][frame], 
                 &file->local_matrices[object_node->node_id][frame], sizeof( tds_matrix ) );
        }
      else
        {
          /*
           * There is a parent, so tack the new local transformation onto 
           * the already calculates transformation for the parent to 
           * get the world transformation for this object.
           */
          TDSMatMult( file->world_matrices[object_node->node_id][frame],
                     file->local_matrices[object_node->node_id][frame],
                     file->world_matrices[object_node->node_header.parent_index][frame] );
        }
    }
}


/*---------------------------------------------------------------------
  Function: TDSMatrixProcessKeyNode
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: processes the object and camera nodes 
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
    key_node - contains the key node information
  Return: see variables
  Pass(s): NA
---------------------------------------------------------------------*/
void TDSMatrixProcessKeyNode( TDSFile *file, tds_key_node *key_node )
{
  switch( key_node->type )
    {
    case TDS_OBJECT_NODE_TAG:
      /* process the object node information */
      TDSMatrixProcessObjectNode( file, &key_node->nodes.object_node );
      break;
    case TDS_CAMERA_NODE_TAG:
      /* process the camera node information */
      TDSMatrixProcessCameraNode( file, &key_node->nodes.camera_node );
      break;
    default:
      break;
    }
}

/*---------------------------------------------------------------------
  Function: TDSMatrixBuild
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: build animation matrices based on the key nodes 
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
  Return: see variables
  Pass(s): NA
---------------------------------------------------------------------*/
void TDSMatrixBuild( TDSFile *file )
{
  int i;

  /* loop through all the keys and calculate animation matrices */
  for( i = 0; i < 65536; i++ )
    {
      if( !file->key_nodes[i] )
        continue;
      TDSMatrixProcessKeyNode( file, file->key_nodes[i] );
    }
}

/*---------------------------------------------------------------------
  Function: TDSWorldToLocal
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: move all of the objects from the .3ds file in their 
               world starting locations to their local coord system
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
  Return: see variables
  Pass(s): NA
---------------------------------------------------------------------*/
void TDSWorldToLocal( TDSFile *file )
{
  int           obj;
  int           vert;
  tds_matrix    invmat;

  /* move world coords to local corrds per object, so that animation
     information may be calculated */
  for( obj = 0; obj < file->num_n_tri_objs; obj++ )
    {
      TDSMatInvert4x4( invmat, file->mesh_matrices[obj] );

      /* print the inverse matrix */
#if PRINT_PROCESSED_CHUNKS
      printf( "matrix:\n" );
      TDSPrintMatrix( file->mesh_matrices[obj] );
#endif

      /* apply the inverse matrix */
      for( vert = 0; vert < file->num_verts_in_object[obj]; vert++ )
        {
          TDSPointMatMult( &file->local_verts[obj][vert], &file->world_verts[obj][vert], invmat );
        }
    }
}

/*---------------------------------------------------------------------
  Function: TDSMapKeysToObjects
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: map animation matrices to proper object 
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
  Return: see variables
  Pass(s): NA
---------------------------------------------------------------------*/
void TDSMapKeysToObjects( TDSFile *file )
{
  int i, j;
  int match;

  /* loop through all the animation and map it to the appropriate object */
  for( i = 0; i < 65536; i++ )
    {
      /* check for object node type */
      if( !file->key_nodes[i] || file->key_nodes[i]->type != TDS_OBJECT_NODE_TAG )
        continue;
      match = 0;

      /* found object node, try to match it to all n_tri_objects */
      for( j = 0; j < file->num_n_tri_objs; j++ )
        {
          /* check for a match */
          if( strcmp( file->key_nodes[i]->nodes.object_node.node_header.object_name,
                      file->object_names[j] ) == 0 )
            {
              /* found match */
              file->key_node_to_object_index[i] = j;
              match = 1;
              break;
            }
        }
      if( !match )
        {
          file->key_node_to_object_index[i] = -1;
#if 0
          fprintf( error_fp, "NO MATCH FOR \"%s\"\n",
                   file->key_nodes[i]->nodes.object_node.node_header.object_name );
#endif
        }
    }
}


       /* .3ds File Header and Pass & Printed Results Functions */
/*=====================================================================*/
/*=====================================================================*/

/*---------------------------------------------------------------------
  Function: TDSPrintMaterial
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: print material information
  Arguments: 
    mat - the material to print out the information on
  Return: none
  Pass(s): NA
---------------------------------------------------------------------*/
void TDSPrintMaterial( tds_material *mat )
{
  /* print material characteristics */
  if( mat->has_name )
    fprintf( error_fp, "name: \"%s\"\n", mat->name );
  if( mat->texmap.has_mapname )
    fprintf( error_fp, "mapname: \"%s\"\n", mat->texmap.mapname );
  if( mat->texmap.has_strength )
    fprintf( error_fp, "strength: %d\n", ( int )mat->texmap.strength );
  if( mat->texmap.has_map_tiling )
    fprintf( error_fp, "map_tiling: 0x%04x\n", ( int )mat->texmap.map_tiling );
  if( mat->texmap.has_tex_blur )
    fprintf( error_fp, "tex_blur: %f\n", mat->texmap.tex_blur );
  if( mat->texmap.has_u_scale )
    fprintf( error_fp, "u_scale: %f\n", mat->texmap.u_scale );
  if( mat->texmap.has_v_scale )
    fprintf( error_fp, "v_scale: %f\n", mat->texmap.v_scale );
  if( mat->texmap.has_u_offset )
    fprintf( error_fp, "u_offset: %f\n", mat->texmap.u_offset );
  if( mat->texmap.has_v_offset )
    fprintf( error_fp, "v_offset: %f\n", mat->texmap.v_offset );
  if( mat->texmap.has_angle )
    fprintf( error_fp, "angle: %f\n", mat->texmap.angle );
  if( mat->texmap.has_col1 )
    fprintf( error_fp, "col1: %d %d %d\n", ( int )mat->texmap.col1[0], ( int )mat->texmap.col1[1], ( int )mat->texmap.col1[2] );
  if( mat->texmap.has_col2 )
    fprintf( error_fp, "col2: %d %d %d\n", ( int )mat->texmap.col2[0], ( int )mat->texmap.col2[1], ( int )mat->texmap.col2[2] );
  if( mat->texmap.has_rcol )
    fprintf( error_fp, "rcol: %d %d %d\n", ( int )mat->texmap.rcol[0], ( int )mat->texmap.rcol[1], ( int )mat->texmap.rcol[2] );
  if( mat->texmap.has_gcol )
    fprintf( error_fp, "gcol: %d %d %d\n", ( int )mat->texmap.gcol[0], ( int )mat->texmap.gcol[1], ( int )mat->texmap.gcol[2] );
  if( mat->texmap.has_bcol )
    fprintf( error_fp, "bcol: %d %d %d\n", ( int )mat->texmap.bcol[0], ( int )mat->texmap.bcol[1], ( int )mat->texmap.bcol[2] );
}

/*---------------------------------------------------------------------
  Function: TDSReadFileHeader
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: reads the .3ds file header information
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
  Return: see file
  Pass(s): first
---------------------------------------------------------------------*/
void TDSReadFileHeader( TDSFile *file )
{
  TDSChunkHeader header;

  /* M3DMAGIC */
  ASSERT_CHUNK( M3DMAGIC, "M3DMAGIC" );

  /* M3D_VERSION */
  ASSERT_CHUNK( M3D_VERSION, "M3D_VERSION" );
  TDSSkipChunk( file, &header );

  /* MDATA */
  ASSERT_CHUNK( MDATA, "MDATA" );
  /* get the geometry and material information */
  TDSReadMDATA( file, &header );
  TDSSkipChunk( file, &header );

  /* KFDATA */
  TDSReadChunkHeader( file, &header );
  if( header.id != KFDATA )
  {
    if( pass == 1 )
      fprintf( error_fp, "WARNING: No animation data in file.\n" );
  }
  else
    /* get the animation information */
    TDSReadKFDATA( file, &header );
}

/*---------------------------------------------------------------------
  Function: TDSSetupForFirstPass
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: setup to collect all first pass information - number
               of materials and objects
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
  Return: see file
  Pass(s): first
---------------------------------------------------------------------*/
void TDSSetupForFirstPass( TDSFile *file )
{
  /* set the pass value */
  pass = 1;

  /* initialize the number of materials and objects */
  file->num_materials = file->num_n_tri_objs = 0;

  /* initialize the key frame information */
  file->current_key_node_index = 0;
  memset( file->key_nodes, 0, 65536 * sizeof( tds_key_node * ) );
}

/*---------------------------------------------------------------------
  Function: TDSPrintFirstPassResults
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: print first pass statistics
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
  Return: none
  Pass(s): NA
---------------------------------------------------------------------*/
void TDSPrintFirstPassResults( TDSFile *file )
{
  /* print first pass results */
  fprintf( error_fp, "RESULTS OF FIRST PASS:\n" );
  fprintf( error_fp, "num_materials: %d\n", file->num_materials );
  fprintf( error_fp, "num_n_tri_objs: %d\n", file->num_n_tri_objs );
}

/*---------------------------------------------------------------------
  Function: TDSSetupForSecondPass
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: setup to collect all second pass information - number
               of vertices and faces in object
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
  Return: see file
  Pass(s): second
---------------------------------------------------------------------*/
void TDSSetupForSecondPass( TDSFile *file )
{
  /* set the pass value */
  pass = 2;

  /* reset the file pointer */
  rewind( file->fp );

  /* allocate memory for vertices, faces, object names, and matrices
     based on number of materials and objects found in the first pass */
  if( !( file->num_verts_in_object = ( int * )malloc( file->num_n_tri_objs * sizeof( int ) ) ) )
    atuError(FXTRUE, "Out of memory in TDSSetupForSecondPass" );

  if( !( file->num_faces_in_object = ( int * )malloc( file->num_n_tri_objs * sizeof( int ) ) ) )
    atuError(FXTRUE, "Out of memory in TDSSetupForSecondPass" );

  if( !( file->object_names = ( tds_object_name * )malloc( file->num_n_tri_objs * sizeof( tds_object_name ) ) ) )
    atuError(FXTRUE, "Out of memory in TDSSetupForSecondPass" );

  if( !( file->mesh_matrices = ( tds_matrix * )malloc( file->num_n_tri_objs * sizeof( tds_matrix ) ) ) )
    atuError(FXTRUE, "Out of memory in TDSSetupForSecondPass" );

  /* initialize the current object and material */
  file->current_object = 0;
  file->current_material = 0;
}

/*---------------------------------------------------------------------
  Function: TDSPrintSecondPassResults
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: print the second pass results
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
  Return: none
  Pass(s): NA
---------------------------------------------------------------------*/
void TDSPrintSecondPassResults( TDSFile *file )
{
  int i;

  /* print the second pass results, the number of verts and
     faces per object */
  fprintf( error_fp, "RESULTS OF SECOND PASS:\n" );
  for( i = 0; i < file->num_n_tri_objs; i++ )
    {
      fprintf( error_fp, "num_verts_in_object[%d] = %d\n", i, file->num_verts_in_object[i] );
      fprintf( error_fp, "num_faces_in_object[%d] = %d\n", i, file->num_faces_in_object[i] );
    }
}

/*---------------------------------------------------------------------
  Function: TDSSetupForThirdPass
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: setup to collect all third pass information - 
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
  Return: see file
  Pass(s): third
---------------------------------------------------------------------*/
void TDSSetupForThirdPass( TDSFile *file )
{
  int i;

  /* set the pass value */
  pass = 3;

  /* reset the file pointer */
  rewind( file->fp );

  /* reset the current object and material pointers */
  file->current_object = 0;
  file->current_material = 0;

  /* allocate memory for world verts, local verts, etc. based on first
     and second pass findings */
  if( !( file->world_verts = ( tds_point ** )malloc( sizeof( tds_point * ) * file->num_n_tri_objs ) ) )
    atuError(FXTRUE, "Out of memory in TDSSetupForThirdPass." );
    
  if( !( file->local_verts = ( tds_point ** )malloc( sizeof( tds_point * ) * file->num_n_tri_objs ) ) )
    atuError(FXTRUE, "Out of memory in TDSSetupForThirdPass." );
    
  if( !( file->vert_normals = ( tds_point ** )malloc( sizeof( tds_point * ) * file->num_n_tri_objs ) ) )
    atuError(FXTRUE, "Out of memory in TDSSetupForThirdPass." );
    
  if( !( file->tex_verts = ( tds_tex_vert ** )malloc( sizeof( tds_tex_vert * ) * file->num_n_tri_objs ) ) )
    atuError(FXTRUE, "Out of memory in TDSSetupForThirdPass." );
    
  if( !( file->faces = ( tds_face ** )malloc( sizeof( tds_face * ) * file->num_n_tri_objs ) ) )
    atuError(FXTRUE, "Out of memory in TDSSetupForThirdPass." );

  if( !( file->materials = ( tds_material * )malloc( sizeof( tds_material ) * file->num_materials ) ) )
    atuError(FXTRUE, "Out of memory in TDSSetupForThirdPass." );

  memset( file->materials, 0, sizeof( tds_material ) * file->num_materials );

  for( i = 0; i < file->num_n_tri_objs; i++ )
    {
      if( !( file->world_verts[i] = ( tds_point * )malloc( sizeof( tds_point ) *
                                                     file->num_verts_in_object[i] ) ) )
        atuError(FXTRUE, "Out of memory in TDSSetupForThirdPass." );

      if( !( file->local_verts[i] = ( tds_point * )malloc( sizeof( tds_point ) *
                                                     file->num_verts_in_object[i] ) ) )
        atuError(FXTRUE, "Out of memory in TDSSetupForThirdPass." );

      if( !( file->vert_normals[i] = ( tds_point * )malloc( sizeof( tds_point ) *
                                                           file->num_verts_in_object[i] ) ) )
        atuError(FXTRUE, "Out of memory in TDSSetupForThirdPass." );

      if( !( file->tex_verts[i] = ( tds_tex_vert * )malloc( sizeof( tds_tex_vert ) *
                                                               file->num_verts_in_object[i] ) ) )
        atuError(FXTRUE, "Out of memory in TDSSetupForThirdPass." );

      if( !( file->faces[i] = ( tds_face * )malloc( sizeof( tds_face ) *
                                                     file->num_faces_in_object[i] ) ) )
        atuError(FXTRUE, "Out of memory in TDSSetupForThirdPass." );
    }
}

/*---------------------------------------------------------------------
  Function: TDSPrintThirdPassResults
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: print third pass results 
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
  Return: none
  Pass(s): NA
---------------------------------------------------------------------*/
void TDSPrintThirdPassResults( TDSFile *file )
{
  int i, j;

  fprintf( error_fp, "RESULTS OF THIRD PASS:\n" );
  
  /* print the third pass results - world verts, texture verts,
     face information */
  for( i = 0; i < file->num_n_tri_objs; i++ )
    {
      fprintf( error_fp, "\"%s\":\n", file->object_names[i] );
      for( j = 0; j < file->num_verts_in_object[i]; j++ )
        {
          fprintf( error_fp, "vert: %f %f %f ", file->world_verts[i][j].x,
                   file->world_verts[i][j].y, file->world_verts[i][j].z );
          fprintf( error_fp, "uv: %f %f\n", file->tex_verts[i][j].u,
                   file->tex_verts[i][j].v );
        }
      for( j = 0; j < file->num_faces_in_object[i]; j++ )
        {
          fprintf( error_fp, "f: %d %d %d (flags: 0x%04x) (material: %d)\n",
                   ( int )file->faces[i][j].v1, 
                   ( int )file->faces[i][j].v2, 
                   ( int )file->faces[i][j].v3, 
                   ( int )file->faces[i][j].flags,
                   ( int )file->faces[i][j].material_index ); 
        }
    }

  /* print material information */
  for( i = 0; i < file->num_materials; i++ )
    {
      tds_material *mat = &file->materials[i];
      fprintf( error_fp, "MATERIAL %d\n", i );
      TDSPrintMaterial( mat );
    }
}

/*---------------------------------------------------------------------
  Function: TDSReadFileFromFP
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: setup for a .3ds info file read from memory (fp)   
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
    fp - file pointer
  Return: see file above
  Pass(s): first
---------------------------------------------------------------------*/
void TDSReadFileFromFP( TDSFile *file, FILE *fp )
{
  /* set memory to 0 size of the given file */
  memset( file, 0, sizeof( TDSFile) );

  /* map file's file pointer to given fp */
  file->fp = fp;

#if defined( __DOS__ ) || defined( WIN32 )
  /* Make sure that someone cannot pass down an ASCII mode fp */
  setmode( fileno( fp ), O_BINARY );
#endif

  /* error checking for bad file pointer */
  if( !fp )
    {
      atuError( FXTRUE, "TDSReadFileFromFP: Handed bad file pointer.\n" );
    }
  
  /* some initialization */
  file->filename = strdup( "fp" );
  file->has_animation = 0;

  /*
   * For the first pass, figure out how many of the following there
   * are in the 3DS file:
   *
   * 1. materials
   * 2. objects
   */
  TDSSetupForFirstPass( file );
  TDSReadFileHeader( file );
#if PRINT_FIRST_PASS_RESULTS
  TDSPrintFirstPassResults( file );
#endif

  /*
   * During the second pass, figure out how many vertices and faces are
   * in each object.
   */
  TDSSetupForSecondPass( file );
  TDSReadFileHeader( file );
#if PRINT_SECOND_PASS_RESULTS
  TDSPrintSecondPassResults( file ); 
#endif

  /*
   * Allocate all necessary data structures before the third pass, and
   * then read all appropriate data.
   */
  TDSSetupForThirdPass( file );
  TDSReadFileHeader( file );
#if PRINT_THIRD_PASS_RESULTS
  TDSPrintThirdPassResults( file ); 
#endif

#if PRINT_KEY_INFO
  TDSPrintKeyInfo( file );
#endif

  /* if there was animation map it to the appropriate objects */
  TDSMapKeysToObjects( file );
  TDSWorldToLocal( file );
}

/*---------------------------------------------------------------------
  Function: TDSReadFile
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: setup for a .3ds info file read from a file
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
    filename - name of file to get data from
  Return: see file above
  Pass(s): first
---------------------------------------------------------------------*/
int TDSReadFile( TDSFile *file, const char *filename )
{
  /* clear memory size of the file */
  memset( file, 0, sizeof( TDSFile) );

  /* open the file with error checking */
#if defined( __DOS__ ) | defined( WIN32 )
  if( !( file->fp = fopen( filename, "rb" ) ) )
#else
  if( !( file->fp = fopen( filename, "r" ) ) )
#endif
    {
      atuError( FXTRUE, "TDSReadFile: Not able to open input file: \"%s\"\n", filename );
    }

  /* initial data */
  file->filename = strdup( filename );
  file->has_animation = 0;

  /*
   * For the first pass, figure out how many of the following there
   * are in the 3DS file:
   *
   * 1. materials
   * 2. objects
   */
  TDSSetupForFirstPass( file );
  TDSReadFileHeader( file );
#if PRINT_FIRST_PASS_RESULTS
  TDSPrintFirstPassResults( file );
#endif

  /*
   * During the second pass, figure out how many vertices and faces are
   * in each object.
   */
  TDSSetupForSecondPass( file );
  TDSReadFileHeader( file );
#if PRINT_SECOND_PASS_RESULTS
  TDSPrintSecondPassResults( file ); 
#endif

  /*
   * Allocate all necessary data structures before the third pass, and
   * then read all appropriate data.
   */
  TDSSetupForThirdPass( file );
  TDSReadFileHeader( file );
#if PRINT_THIRD_PASS_RESULTS
  TDSPrintThirdPassResults( file ); 
#endif

#if PRINT_KEY_INFO
  TDSPrintKeyInfo( file );
#endif

  /* if animation information, map it appropriately */
  TDSMapKeysToObjects( file );
  TDSWorldToLocal( file );
  return 1;
}

/*---------------------------------------------------------------------
  Function: TDSUseAnimation
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: specify whether or not you want the animation data
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
  Return: see file above
  Pass(s): first
---------------------------------------------------------------------*/
void TDSUseAnimation( TDSFile *file )
{
#if 0
  TDSWorldToLocal( file ); 
#endif
  TDSMatrixBuild( file );
}

/*---------------------------------------------------------------------
  Function: TDSInit
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: initializing the max number of vertices to be used by
               .3ds file, and checking memory
  Arguments: 
    max_num_verts_in_obj - maximum number of verts in object 
  Return: see file above
  Pass(s): NA
---------------------------------------------------------------------*/
void TDSInit( long max_num_verts_in_obj )
{
  if( !( transformed_verts = ( tds_point * )malloc( sizeof( tds_point ) * max_num_verts_in_obj ) ) )
    {
      atuError(FXTRUE, "Not able to allocate space for transformed verts\n" );
    }
}

/*---------------------------------------------------------------------
  Function: TDSPrintNodeHeader
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: print node header  information
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
    node_header - node header information
  Return: none
  Pass(s): NA
---------------------------------------------------------------------*/
void TDSPrintNodeHeader( TDSFile *file, tds_node_header *node_header )
{
  /* print the node header information */
  fprintf( error_fp, "object_name: \"%s\"\n", node_header->object_name );
  fprintf( error_fp, "flag1: 0x%04x\n", node_header->flag1 );
  fprintf( error_fp, "flag2: 0x%04x\n", node_header->flag2 );
  fprintf( error_fp, "parent_index: %d\n", node_header->parent_index );
}

/*---------------------------------------------------------------------
  Function: TDSPrintPosTrackInfo
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: print position track information
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
    pos_track_info - position track information
  Return: none
  Pass(s): NA
---------------------------------------------------------------------*/
void TDSPrintPosTrackInfo( TDSFile *file, tds_pos_track_info *pos_track_info )
{
  int i;

  /* print the position track information */
  fprintf( error_fp, "\nPOS_TRACK_INFO\n" );
  TDSPrintTrackHeader( &pos_track_info->track_header );
  for( i = 0; i < ( int )pos_track_info->track_header.num_keys; i++ )
    {
      TDSPrintKeyHeader( &pos_track_info->pos_track_tags[i].key_header );
      fprintf( error_fp, "%d position: %f %f %f\n", i, 
               pos_track_info->pos_track_tags[i].position.x,
               pos_track_info->pos_track_tags[i].position.y,
               pos_track_info->pos_track_tags[i].position.z );
    }
}

/*---------------------------------------------------------------------
  Function: TDSPrintNodeHeader
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: print node header  information
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
    node_header - node header information
  Return: none
  Pass(s): NA
---------------------------------------------------------------------*/
void TDSPrintRotTrackInfo( TDSFile *file, tds_rot_track_info *rot_track_info )
{
  int i;

  fprintf( error_fp, "\nROT_TRACK_INFO\n" );
  TDSPrintTrackHeader( &rot_track_info->track_header );
  for( i = 0; i < ( int  )rot_track_info->track_header.num_keys; i++ )
    {
      TDSPrintKeyHeader( &rot_track_info->rot_track_tags[i].key_header );
      fprintf( error_fp, "rot_angle: %f rot_axis: %f %f %f\n",
               rot_track_info->rot_track_tags[i].rot_angle,
               rot_track_info->rot_track_tags[i].rot_axis[0],
               rot_track_info->rot_track_tags[i].rot_axis[1],
               rot_track_info->rot_track_tags[i].rot_axis[2] );
    }
}

/*---------------------------------------------------------------------
  Function: TDSPrintSclTrackInfo
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: print scale track information
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
    scl_track_info - scale track information
  Return: none
  Pass(s): NA
---------------------------------------------------------------------*/
void TDSPrintSclTrackInfo( TDSFile *file, tds_scl_track_info *scl_track_info )
{
  int i;

  /* print the scale track information */
  fprintf( error_fp, "\nSCL_TRACK_INFO\n" );
  TDSPrintTrackHeader( &scl_track_info->track_header );
  for( i = 0; i < ( int )scl_track_info->track_header.num_keys; i++ )
    {
      TDSPrintKeyHeader( &scl_track_info->scl_track_tags[i].key_header );
      fprintf( error_fp, "scale: %f %f %f\n",
               scl_track_info->scl_track_tags[i].scale[0],
               scl_track_info->scl_track_tags[i].scale[1],
               scl_track_info->scl_track_tags[i].scale[2] );
    }
}

/*---------------------------------------------------------------------
  Function: TDSPrintObjectNode
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: print object node information
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
    object_node - object node information
  Return: none
  Pass(s): NA
---------------------------------------------------------------------*/
void TDSPrintObjectNode( TDSFile *file, tds_object_node *object_node )
{

  /* print the object node information */
  fprintf( error_fp, "node_id: %d\n", ( int )object_node->node_id );
  TDSPrintNodeHeader( file, &object_node->node_header );
  fprintf( error_fp, "pivot: %f %f %f\n",
           object_node->pivot.x, 
           object_node->pivot.y, 
           object_node->pivot.z );
  fprintf( error_fp, "instance_name:\"%s\"\n", object_node->instance_name );
  fprintf( error_fp, "bbox: (%f %f %f) (%f %f %f)\n",
           object_node->bbox.min.x,
           object_node->bbox.min.y,
           object_node->bbox.min.z,
           object_node->bbox.max.x,
           object_node->bbox.max.y,
           object_node->bbox.max.z );
  TDSPrintPosTrackInfo( file, &object_node->pos_track_info );
  TDSPrintRotTrackInfo( file, &object_node->rot_track_info );
  TDSPrintSclTrackInfo( file, &object_node->scl_track_info );
}

/*---------------------------------------------------------------------
  Function: TDSPrintCameraNode
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: print camera node information
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
    camera_node - camera node information
  Return: none
  Pass(s): NA
---------------------------------------------------------------------*/
void TDSPrintCameraNode( TDSFile *file, tds_camera_node *camera_node )
{
  /* print the camera node information */
  fprintf( error_fp, "node_id: %d\n", ( int )camera_node->node_id );
  TDSPrintNodeHeader( file, &camera_node->node_header );
  TDSPrintPosTrackInfo( file, &camera_node->pos_track_info );
}

/*---------------------------------------------------------------------
  Function: TDSPrintKeyNode
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: print key node information
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
    key_node - key node information
  Return: none
  Pass(s): NA
---------------------------------------------------------------------*/
void TDSPrintKeyNode( TDSFile *file, tds_key_node *key_node )
{
  /* print the key_node information */
  switch( key_node->type )
    {
    case TDS_OBJECT_NODE_TAG:
      TDSPrintObjectNode( file, &key_node->nodes.object_node );
      break;
    case TDS_CAMERA_NODE_TAG:
      TDSPrintCameraNode( file, &key_node->nodes.camera_node );
      break;
    default:
      break;
    }
}

/*---------------------------------------------------------------------
  Function: TDSPrintKeyInfo
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: print key  information
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
  Return: none
  Pass(s): NA
---------------------------------------------------------------------*/
void TDSPrintKeyInfo( TDSFile *file )
{
  int i;

  /* print the key information */
  for( i = 0; i < 65536; i++ )
    {
      if( !file->key_nodes[i] )
        continue;
      TDSPrintKeyNode( file, file->key_nodes[i] );
      fprintf( error_fp, "\n" );
    }
}


             /* Geometry Utility Functions and Hacks!!! */
/*=====================================================================*/
/*=====================================================================*/

/*---------------------------------------------------------------------
  Function: TDSGetModelRadusAtFrame
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: find maximum model radius at frame
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
    frame - current frame
  Return: floating point radius
  Pass(s): NA
---------------------------------------------------------------------*/
float TDSGetModelRadiusAtFrame( TDSFile *file, int frame )
{
  int i, j;
  int object_index;
  tds_tex_vert *tex_verts;
  tds_matrix *m;
  tds_point *verts;
  int num_verts;
  float max_radius = -1.0f;

  /* loop through animation data and find max radius of objects
     in frame */
  for( i = 0; i < 65536; i++ )
    {
      /* int tmp; */

      /* check for a key node */
      if( file->key_nodes[i] )
        {
          /* check specifically for an object node */
          if( file->key_nodes[i]->type == TDS_OBJECT_NODE_TAG )
            {
              /*
               * Transform vertices.
               */
              object_index = file->key_node_to_object_index[i]; 
              if( object_index == -1 )
                continue;

              /* localize variables */
              m = &file->world_matrices[i][frame];

              verts = file->world_verts[object_index];
              tex_verts = file->tex_verts[object_index];
              num_verts = file->num_verts_in_object[object_index];

              /* find max radius */
              for( j = 0; j < num_verts; j++ )
                {
                  float tmp;

                  TDSPointMatMult( &transformed_verts[j], &verts[j], *m );
                  if( ( tmp = transformed_verts[j].x * transformed_verts[j].x +
                        transformed_verts[j].y * transformed_verts[j].y +
                        transformed_verts[j].z * transformed_verts[j].z ) > max_radius )
                    max_radius = tmp;
                }
            }
        }
    }

  /* return max radius */
  return ( float )sqrt( max_radius );
}

/*---------------------------------------------------------------------
  Function: TDSGetNodeRadusAtFrame
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: find maximum node radius at frame 
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
    frame - current frame
  Return: floating point radius
  Pass(s): NA
---------------------------------------------------------------------*/
float TDSGetNodeRadiusAtFrame( TDSFile *tdsfile, int frame, int node_index )
{
  int object_index;
  tds_matrix m;
  tds_point *verts;
  int num_verts;
  int i;
  float max_radius;

  /* localize object */
  object_index = tdsfile->key_node_to_object_index[node_index];
  if( object_index == -1 )
    return 0.0f;

  memcpy( &m, tdsfile->world_matrices[node_index][frame], sizeof( tds_matrix ) );
  
  /*
   * Get rid of any translation in the matrix.
   */
  m[3][0] = m[3][1] = m[3][2] = 0.0f;

  /* localize variables */
  verts = tdsfile->local_verts[object_index];
  num_verts = tdsfile->num_verts_in_object[object_index];

  /* set an initial max radius */
  max_radius = -1.0f;

  /* loop through and find the max radius */
  for( i = 0; i < num_verts; i++ )
    {
      tds_point tmp_point;
      float tmp;

      TDSPointMatMult( &tmp_point, &verts[i], m );
      if( ( tmp = tmp_point.x * tmp_point.x +
           tmp_point.y * tmp_point.y +
           tmp_point.z * tmp_point.z ) > max_radius )
        max_radius = tmp;
    }

  /* return max radius */
  return ( float )sqrt( max_radius );
}

/*---------------------------------------------------------------------
  Function: TDSFindNodeIndexByName
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: find a node by passing a name value in 
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
    name - name of a node
  Return: integer node indexed by name 
  Pass(s): NA
---------------------------------------------------------------------*/
int TDSFindNodeIndexByName( TDSFile *file, const char *name )
{
  int i;
  int found;
  char *node_name;
  int match_node_index;

  /* find node index for the current node */
  found = 0;

  /* loop through and find a node indexed by the given name, match with
     instance and object names */
  for( i = 0; i < 65536; i++ )
    {
      if( file->key_nodes[i] && ( file->key_nodes[i]->type == TDS_OBJECT_NODE_TAG ) )
        {
          if( *file->key_nodes[i]->nodes.object_node.instance_name != '\0' )
            node_name = file->key_nodes[i]->nodes.object_node.instance_name;
          else
            node_name = file->object_names[file->key_node_to_object_index[i]];

          if( strcmp( node_name, name ) == 0 )
            {
#if 0
              fprintf( error_fp, "Match: %s\n", name );
#endif
              /* match found */
              found = 1;
              match_node_index = i;
            }
        }
    }

  /* match not found, generate an error and exit the program */
  if( !found )
    {
      atuError(FXTRUE, "Error: \"%s\" not found.\n", name );
    }

  /* return the node index, indexed by the name */
  return match_node_index;
}





/*---------------------------------------------------------------------
  HACK!!!!  3DStudio gives you the geometry in world coord, so the 
            vertex must be calculated back to local to perform animation
            rotations, etc. However, 3DStudio mirrors by just negating the 
            appropriate axis, this is incorrect and reveals weird result
            when calculating inverse matrices

  Function: TDSHackTransforms
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: read in world matrics, as opposed to inverting
               them  
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
    anifilename - animation file name
  Return: see file above
  Pass(s): NA
---------------------------------------------------------------------*/
void TDSHackTransforms( TDSFile *file, const char *anifilename )
{
  FILE *fp;
  int segment_start, segment_end;
  char buf[100];
  char *ptr, *ptr2;
  int match_node_index;
  int frame;
  
  /* open animation file name */
  if( !( fp = fopen( anifilename, "r" ) ) )
    atuError(FXTRUE, "Not able to open transform input file." );

  fgets( buf, 100, fp );
  sscanf( buf, "segment: %d %d", &segment_start, &segment_end );
  fprintf( error_fp, "segment: %d %d\n", segment_start, segment_end );

  fgets( buf, 100, fp );

  while( strncmp( "object:", buf, 7 ) == 0 )
    {
      fprintf( error_fp, "innerloop\n" );

      /*
       * Get the name only of the object.
       */
      ptr = buf;
      while( *ptr++ != '<' )
        ;
      ptr2 = ptr;
      while( *ptr2 != '>' )
        ptr2++;
      *ptr2 = '\0';

      /*
       * ptr now points at the null terminated object name.
       */
      match_node_index = TDSFindNodeIndexByName( file, ptr );

      /*
       * Node index which correspondes to the named node is now in 
       * "match_nodex_index".
       */
      for( frame = segment_start; frame <= segment_end; frame++ )
        {
          fgets( buf, 100, fp );
          sscanf( buf, "%f %f %f", 
                 &file->world_matrices[match_node_index][frame][0][0],
                 &file->world_matrices[match_node_index][frame][0][1],
                 &file->world_matrices[match_node_index][frame][0][2] );
          fgets( buf, 100, fp );
          sscanf( buf, "%f %f %f", 
                 &file->world_matrices[match_node_index][frame][1][0],
                 &file->world_matrices[match_node_index][frame][1][1],
                 &file->world_matrices[match_node_index][frame][1][2] );
          fgets( buf, 100, fp );
          sscanf( buf, "%f %f %f", 
                 &file->world_matrices[match_node_index][frame][2][0],
                 &file->world_matrices[match_node_index][frame][2][1],
                 &file->world_matrices[match_node_index][frame][2][2] );
          fgets( buf, 100, fp );
          sscanf( buf, "%f %f %f", 
                 &file->world_matrices[match_node_index][frame][3][0],
                 &file->world_matrices[match_node_index][frame][3][1],
                 &file->world_matrices[match_node_index][frame][3][2] );
          file->world_matrices[match_node_index][frame][0][3]= 0.0f;
          file->world_matrices[match_node_index][frame][1][3]= 0.0f;
          file->world_matrices[match_node_index][frame][2][3]= 0.0f;
          file->world_matrices[match_node_index][frame][3][3]= 1.0f;
        }

      fgets( buf, 100, fp );
    }
}

/*---------------------------------------------------------------------
  HACK!!!!  see above

  Function: TDSHackGeometry
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: read in correct geometry as opposed to inverting
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
    geomfilename - geometry file name
  Return: none 
  Pass(s): NA
---------------------------------------------------------------------*/
void TDSHackGeometry( TDSFile *file, const char *geomfilename )
{
  FILE *fp;
  char buf[200];
  int num_objects;
  int obj;
  char objname[200];

  /* open geometry file */
  if( !( fp = fopen( geomfilename, "r" ) ) )
    atuError(FXTRUE, "Not able to open geometry input file." );

  fgets( buf, 100, fp );

  sscanf( buf, "objects: %d", &num_objects );

  for( obj = 0; obj < num_objects; obj++ )
    {
      int num_verts, num_faces;
      int vert, face;
      float v[3];
      int f[3];
      int node_index;

      fgets( buf, 100, fp );
      sscanf( buf, "object: <%s", objname );

      objname[strlen(objname) - 1] = '\0';

      node_index = TDSFindNodeIndexByName( file, objname );

      fgets( buf, 100, fp );
      sscanf( buf, "verts: %d", &num_verts );

#if 0
      printf( "messed object: %s\n", file->object_names[obj] );
#endif

      for( vert = 0; vert < num_verts; vert++ )
        {
          fgets( buf, 100, fp );
          sscanf( buf, "%f %f %f", &v[0], &v[1], &v[2] );

          file->local_verts[file->key_node_to_object_index[node_index]][vert].x = v[0];
          file->local_verts[file->key_node_to_object_index[node_index]][vert].y = v[1];
          file->local_verts[file->key_node_to_object_index[node_index]][vert].z = v[2];
        }

      fgets( buf, 100, fp );
      sscanf( buf, "faces: %d", &num_faces );

      for( face = 0; face < num_faces; face++ )
        {
          fgets( buf, 100, fp );
          sscanf( buf, "%d %d %d", &f[0], &f[1], &f[2] );
        }
    }
  fclose( fp );
}

/*---------------------------------------------------------------------
  Function: TDSCalcFaceNormals
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: calculate the face normals
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
    object number - number of objects
  Return: none 
  Pass(s): NA
---------------------------------------------------------------------*/
void TDSCalcFaceNormals( TDSFile *file, int object_number )
{
  int i;

  for( i = 0; i < file->num_faces_in_object[object_number]; i++ )
    {
      tds_point a, b, c;
      
      /*
       * Calculate the normal for the face.
       */
      a = file->world_verts[object_number][file->faces[object_number][i].v1];
      b = file->world_verts[object_number][file->faces[object_number][i].v2];
      TDSVecSub( &a, &b, &a );
      
      b = file->world_verts[object_number][file->faces[object_number][i].v1];
      c = file->world_verts[object_number][file->faces[object_number][i].v3];
      TDSVecSub( &b, &c, &b );
      
      TDSVecCross( &file->faces[object_number][i].normal, &a, &b );
      TDSVecNorm( &file->faces[object_number][i].normal ); 
    }
}

/*---------------------------------------------------------------------
  Function: TDSCalcNodeBoundingBox
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: calculate bounding box for a node
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
    node_index - node index
  Return: none 
  Pass(s): NA
---------------------------------------------------------------------*/
void TDSCalcNodeBoundingBox( TDSFile *tdsfile, int node_index )
{
  int object_index;
  int num_verts;
  int i;
  tds_bbox *b;

  object_index = tdsfile->key_node_to_object_index[node_index];

  if( object_index < 0 )
    return;

  num_verts = tdsfile->num_verts_in_object[object_index];
  b = &tdsfile->key_nodes[node_index]->nodes.object_node.bbox;

  /* initialize box min and max */  
  b->min.x = tdsfile->world_verts[object_index][0].x;
  b->min.y = tdsfile->world_verts[object_index][0].y;
  b->min.z = tdsfile->world_verts[object_index][0].z;
  b->max.x = tdsfile->world_verts[object_index][0].x;
  b->max.y = tdsfile->world_verts[object_index][0].y;
  b->max.z = tdsfile->world_verts[object_index][0].z;

  /* find box min and max */
  for( i = 1; i < num_verts; i++ )
    {
      if( tdsfile->world_verts[object_index][i].x < b->min.x )
        b->min.x = tdsfile->world_verts[object_index][i].x;
      if( tdsfile->world_verts[object_index][i].y < b->min.y )
        b->min.y = tdsfile->world_verts[object_index][i].y;
      if( tdsfile->world_verts[object_index][i].z < b->min.z )
        b->min.z = tdsfile->world_verts[object_index][i].z;
      
      if( tdsfile->world_verts[object_index][i].x > b->max.x )
        b->max.x = tdsfile->world_verts[object_index][i].x;
      if( tdsfile->world_verts[object_index][i].y > b->max.y )
        b->max.y = tdsfile->world_verts[object_index][i].y;
      if( tdsfile->world_verts[object_index][i].z > b->max.z )
        b->max.z = tdsfile->world_verts[object_index][i].z;
    }
}

/*---------------------------------------------------------------------
  Function: TDSCalcBoundingBoxes
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: calculate bounding box for a node that don't have them
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
  Return: none 
  Pass(s): NA
---------------------------------------------------------------------*/
void TDSCalcBoundingBoxes( TDSFile *tdsfile )
{
  int i;

  /* loop through the nodes and find objects, and calculate bounding
     boxes for them */
  for( i = 0; i < 65536; i++ )
    {
      if( tdsfile->key_nodes[i] && ( tdsfile->key_nodes[i]->type == TDS_OBJECT_NODE_TAG ) )
        {
#if 0
          if( tdsfile->key_nodes[i]->nodes.object_node.bbox_set )
            continue;
#endif
          TDSCalcNodeBoundingBox( tdsfile, i );
        }
    }
}

/*---------------------------------------------------------------------
  Function: TDSCountMaterialsInObject
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: count the number of materials in an object
  Arguments: 
    file - contains information on the .3ds file, specific to this
           function, where the file pointer resides in the .3ds file,
           and will reside after the read or skip
    objnum - object number
    mesh_material_to_object_material - 
    object_material_to_mesh_material - 
  Return: integer number of mesh materials
  Pass(s): NA
---------------------------------------------------------------------*/
int TDSCountMaterialsInObject( TDSFile *tdsfile, int objnum, int **_mesh_material_to_object_material,
                               int **_object_material_to_mesh_material )
{
  int i;
  int num_faces;
  int *mesh_material_to_object_material;
  int *object_material_to_mesh_material;
  int mesh_material_index;
  tds_face *faces;
  int num_mesh_materials;

  /* allocate space to meshes and materials, size of the number of 
     materials, with error checking */
  mesh_material_to_object_material = ( int * )malloc( sizeof( int ) * tdsfile->num_materials );
  object_material_to_mesh_material = ( int * )malloc( sizeof( int ) * tdsfile->num_materials );
  if( !mesh_material_to_object_material || !object_material_to_mesh_material )
    {
      atuError(FXTRUE, "Out of memory in TDSCountMaterialsInObject\n" );
    }

  /* initialize the mesh to object and vice versa */
  for( i = 0; i < tdsfile->num_materials; i++ )
    {
      mesh_material_to_object_material[i] = -1;
      object_material_to_mesh_material[i] = -1;
    }

  /* localize the variables */
  num_faces = tdsfile->num_faces_in_object[objnum];
  faces = tdsfile->faces[objnum];

  /* set the initial index number */
  mesh_material_index = 0;

  /* loop through the faces and find all the materials associated with
     the faces per object */
  for( i = 0; i < num_faces; i++ )
    {
      if( object_material_to_mesh_material[faces[i].material_index] == -1 )
        {
          object_material_to_mesh_material[faces[i].material_index] = mesh_material_index;
          mesh_material_to_object_material[mesh_material_index] = faces[i].material_index;
#if 0
          fprintf( error_fp, "mesh_material_to_object_material[%d] = %d\n", ( int )mesh_material_index, ( int )faces[i].material_index );
#endif
          mesh_material_index++;
        }
    }

  num_mesh_materials = mesh_material_index;

  *_mesh_material_to_object_material = mesh_material_to_object_material;
  *_object_material_to_mesh_material = object_material_to_mesh_material;

  /* return the number of materials found in the object */
  return num_mesh_materials;
}

/*---------------------------------------------------------------------
  Function: TDSConvertToATBCoordSystem
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: convert from a right handed system to left handed by
               swapping the y and z values, and change facing 
               appropriately
  Arguments: 
    tdsfile - contains information on the .3ds file, specific to this
              function, where the file pointer resides in the .3ds file,
              and will reside after the read or skip
  Return: see tdsfile above
  Pass(s): NA
---------------------------------------------------------------------*/
void TDSConvertToATBCoordSystem( TDSFile *tdsfile )
{
  int vertnum, facenum, objnum;
  float flttmp;
  int inttmp;

  for( objnum = 0; objnum < tdsfile->num_n_tri_objs; objnum++ )
    {
      for( vertnum = 0; vertnum < tdsfile->num_verts_in_object[objnum]; vertnum++ )
        {
          /*
           * Swap Y and Z.
           */
          flttmp = tdsfile->world_verts[objnum][vertnum].y;
          tdsfile->world_verts[objnum][vertnum].y = tdsfile->world_verts[objnum][vertnum].z;
          tdsfile->world_verts[objnum][vertnum].z = flttmp;
        }
      /*
       * Switch the facing of all the polys.
       */
      for( facenum = 0; facenum < tdsfile->num_faces_in_object[objnum]; facenum++ )
        {
          /*
           * Swap v2 and v3.
           */
          inttmp = tdsfile->faces[objnum][facenum].v2;
          tdsfile->faces[objnum][facenum].v2 = tdsfile->faces[objnum][facenum].v3;
          tdsfile->faces[objnum][facenum].v3 = inttmp;
        }
    }
}
