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
** $Date: 10/11/00 7:34:40 PM$ 
**
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <texus.h>
#include <atrender.h>
#include <atscenep.h>

/* Definitions */

#define COOL                    0
#define NOT_VALID_FILE            91
#define NOT_CORRECT_VERSION        92
#define NOT_COOL                13

#define COLOR_ONLY        0
#define TEX_PER_SHADER    1
#define TEX_PER_POLY    2

#define TREE_ONLY            1
#define TREE_XFORMS            2
#define TREE_XFORMS_PIVOTS    3

#define LOCAL_SPACE        'L'
#define WORLD_SPACE        'W'

#define RGB_COLOR        'R'
#define TEXTURE_INDEX    'T'

static char *converterName = "atsLoadFromRTG";
static char *ext = "rtg";

/* Global Variables */

/* flags */
static char hierarchy_exists = 0;
static char vertex_norms_exist = 0;
static char texture_coords_exist = 0;
static char polygon_norms_exist = 0;
static char index_counters_exist = 0;
static char texture_table_exists = 0;
static char hier_xform_matrices = 0;

/* modes */
static int texture_mode = COLOR_ONLY;
static int hierarchy_level = TREE_ONLY;
static FxBool use_animation = FXFALSE, usetextures = FXTRUE;
static float vertex_scale = 1.0f;

/* File Pointers */

static FILE *text_in;
static FILE *bin_in;

/* String Variables */

static char tag [ 80 ];
static char value [ 80 ];
static char temp_string [ 512 ];
static char file_name [ 512 ];

static char header_title [ 80 ];
static char header_date  [ 80 ];
static char version [ 15 ];

/* Counters */

static int num_objects;
static int num_vertices;
static int num_polygons;
static int num_table_entries;
static int num_textures;
static int num_top_level;

static float tmp;

static AtsNode *all_shapes;

/* THE RTG BINARY TEXTURE DATA FILE (.bin)
   ---------------------------------------

  the Header:
  -----------

  The header of the texture file contains a text title, a version tag, and
  time and date creation information. This header string is NOT null terminat-
  ed. Use the header byte count found in the texture table start line to
  determine how many bytes to read to clear the header. the version tag has
  a 'ver' descriptor tag just before it and the time and date info has a 'date'
  descriptor tag just before it.

  The header should be parsed to get the version tag and the date in order to
  verify the texture file against the P-data file.

  the Data:
  ---------

  After the header any textures generated for the corresponding polygonal data
  file is dumped pixel by pixel into this file. Each pixel is seperated into
  4 bytes one for each of R, G, B, and A and written out to this file in that
  order. Values for each of RGB and A range from 0 to 255. Pixel values are
  output starting from the bottom-left (texture coord 0,0) and are output
  moving left to right and bottom to top.
*/

static AtsTexture *imageToTexture(char *raw_image, FxBool hasAlpha, 
                           int width, int height) {
    size_t tex_mem_required;
    Gu3dfInfo *info;
    int target_width, target_height; /* values to resize image to */
    AtsTexture *tex;
    AtrImg     *img;

    /* TBD need to handle alpha case */

    FXUNUSED(hasAlpha);

    if ((info=(Gu3dfInfo*)atuMemMalloc(sizeof(Gu3dfInfo)))==NULL)
      atuError(FXTRUE, 
               "Out of memory allocating system memory textures.\n" );

    target_width = width;
    target_height = height;

    tex_mem_required = txInit3dfInfo( info, GR_TEXFMT_RGB_565,
                                      &target_width, &target_height,
                                      -1, TX_AUTORESIZE_GROW );
    /*
     * Make sure txInit3dfInfo didn't fail.
     */

    if ( tex_mem_required == 0 ) {
        atuError( FXTRUE, "Problem with txInit3dfInfo\n" );
    }

    /*
     * Allocate system memory for the texture.
     */

    if ( ( info->data = atuMemMalloc( tex_mem_required )) == NULL ) {
        atuError( FXTRUE, "Out of memory allocating system memory textures.\n" );
    }

    /*
     * Convert to a texture that can be downloaded to the hardware.
     * TBD: the image is upside down
     */

    txConvert( info, GR_TEXFMT_ARGB_8888,
               width, height, raw_image, TX_DITHER_ERR, NULL );
  
    if (( tex = atsTextureNew()) == NULL ) {
        atuError(FXTRUE, "Could not create texture\n");
    }


    img          = (AtrImg *)atsImageNew();
    img->format  = info->header.format;
    img->width   = info->header.width;
    img->height  = info->header.height;
#ifdef GLIDE3
    img->nLevels = 1 + info->header.large_lod - info->header.small_lod;
#else
    img->nLevels = 1 + info->header.small_lod - info->header.large_lod;
#endif
    img->name    = strdup( "radimg" );
    img->data    = info->data;
    img->table   = (AtrImgTable*)&info->table;

    atsTextureAddImage( tex, img );
    atsTextureReset( tex );

    return tex;
}

#define MAT_TEXTURED 0
#define MAT_COLORED  1

typedef struct _rtgMat {
    FxU32 matType;
    int width, height;
    int r, g, b;
    AtsMaterial *atbMaterial;
    struct _rtgMat *next;
} rtgMat;

static rtgMat *matList = NULL;

static AtsMaterial *coloredMaterial(int r, int g, int b) {
    rtgMat *mat;

    /* see if we have a material which matches the desired one */

    for ( mat = matList; mat != NULL; mat = mat->next ) {
         if (( mat->matType == MAT_COLORED ) &&
             ( mat->r == r ) && ( mat->g == g ) && ( mat->b == b ) ) {
             return mat->atbMaterial;
         }
    }

    /* no match, create a new one */

    if ( ( mat = (rtgMat *)atuMemMalloc(sizeof(rtgMat))) == NULL ) {
        atuError(FXTRUE, "unable to allocate material\n");
    }

    if( (mat->atbMaterial = atsMaterialNew()) == NULL ) {
        atuError(FXTRUE, "%s: Unable to allocate space for the Materials\n", 
                 converterName);
    }

    mat->matType = MAT_COLORED;
    mat->r = r;
    mat->g = g;
    mat->b = b;
    mat->next = matList;
    matList = mat;
     
    atrMaterialSetup(&mat->atbMaterial->material, ATR_MAT_GSHADE);

    mat->atbMaterial->material.diffuse.r = (float)r/255.0f;
    mat->atbMaterial->material.diffuse.g = (float)g/255.0f;
    mat->atbMaterial->material.diffuse.b = (float)b/255.0f;

    return mat->atbMaterial;
}

static AtsMaterial *texturedMaterial(int tex_index) {
    rtgMat *mat;

    /* see if we have a material which matches the desired one */

    for ( mat = matList; mat != NULL; mat = mat->next ) {
         if (( mat->matType == MAT_TEXTURED ) && ( mat->r == tex_index )) {
             return mat->atbMaterial;
         }
    }

    /* no match, create a new one */

    if ( ( mat = (rtgMat *)atuMemMalloc(sizeof(rtgMat))) == NULL ) {
        atuError(FXTRUE, "unable to allocate material\n");
    }

    if( (mat->atbMaterial = atsMaterialNew()) == NULL ) {
        atuError(FXTRUE, "%s: Unable to allocate space for the Materials\n", 
                 converterName);
    }

    mat->atbMaterial->material.diffuse.g = 1.0f;
    mat->atbMaterial->material.diffuse.g = 1.0f;
    mat->atbMaterial->material.diffuse.b = 1.0f;

    mat->matType = MAT_TEXTURED;
    mat->r = tex_index;
    mat->next = matList;
    matList = mat;
     
    atrMaterialSetup(&mat->atbMaterial->material, ATR_MAT_DECAL_X_LIGHTING );

    /* we will associate the texture later */

    return mat->atbMaterial;
}

static void associateTexture(int tex_index, FxBool hasAlpha, AtsTexture *tex) {
    rtgMat *mat;

    /* find material with specified texture index */

    for ( mat = matList; mat != NULL; mat = mat->next ) {
        if (( mat->matType == MAT_TEXTURED ) && ( mat->r == tex_index )) {
             atsMaterialTexture(mat->atbMaterial, 0, tex);
             if ( hasAlpha ) {
                 atrMaterialSetup(&mat->atbMaterial->material, 
                     ATR_MAT_DECAL_X_LIGHTING | ATR_MAT_FB_BLEND );
             }
             atsRef(tex);
             return;
         }
    }

    atuError(FXTRUE, "Could not associate texture with material\n");
}

/* Routines */

static int Process_Header_Info ( void )
{
    int version_supported;
    int file_stat, c;

    /* do title ... */

    strcpy ( header_title, "" );
    fscanf ( text_in, "%s", tag );  /* clear HEADER_TITLE tag */

    while ( 1 )
    {
        file_stat = fscanf ( text_in, "%s", tag );
        if ( file_stat == EOF )
            return ( EOF );

        if ( strcmp ( tag, "HEADER_VERSION" ) == 0 )
            break;

        strcat ( header_title, tag );
        strcat ( header_title, " " );
    }

    /* we've cleared the HEADER_VERSION tag already so get value */
    fscanf ( text_in, "%s", version );

    /* verify file ... */

    if ( strcmp ( header_title, "Alias|Wavefront GAMES TEXT OUTPUT " ) != 0 )
    {
        if ( strcmp ( header_title,
             "Alias|Wavefront Real Time Games Output " ) != 0 )
        {
            atuError(FXFALSE,  "Error: file is not an Alias|Wavefront RTG file.\n" );
            return ( NOT_VALID_FILE );
        }
    }

    version_supported = 0;

    if ( strcmp ( version, "v1.0" ) == 0 )
        version_supported = 1;

    if ( strcmp ( version, "v1.1" ) == 0 )
        version_supported = 1;

    if ( strcmp ( version, "v1.75" ) == 0 )
        version_supported = 1;

    if ( ! version_supported )
    {
        atuError ( FXFALSE, "Error: file is not a supported version.\n" );
        return ( NOT_CORRECT_VERSION );
    }

    /* do date ... */

    strcpy ( header_date, "" );
    fscanf ( text_in, "%s", tag );  /* clear HEADER_DATE tag */

    for ( c=0; c < 5; c++ )  /* get 5 tags for date string */
    {
        /* 5 tags are: 1-Day of week
                       2-Month
                       3-Day of month
                       4-Time
                       5-Year
        */
        file_stat = fscanf ( text_in, "%s", tag );
        if ( file_stat == EOF )
            return ( EOF );

        strcat ( header_date, tag );
        strcat ( header_date, " " );
    }

    /* printf ("@ %s %s %s\n", header_title, version, header_date ); */

    /* get counters ... */

    fscanf ( text_in, "%s", tag );           /* clear NUMBER_OF_OBJECTS tag */
    fscanf ( text_in, "%d", &num_objects );  /* get num_objects value */

    /* printf ("@ %d objects\n", num_objects); */

    /* get flags ... */

    fscanf ( text_in, "%s", tag );    /* clear OUTPUT_VERT_NORMS tag */
    fscanf ( text_in, "%s", value );  /* get value */
    if ( strcmp ( value, "on" ) == 0 )
        vertex_norms_exist = 1;
    else
        vertex_norms_exist = 0;

    /* printf ("@ vert_norms %s\n", value); */

    fscanf ( text_in, "%s", tag );    /* clear OUTPUT_TEX_COORDS tag */
    fscanf ( text_in, "%s", value );  /* get value */
    if ( strcmp ( value, "on" ) == 0 )
        texture_coords_exist = 1;
    else
        texture_coords_exist = 0;

    /* printf ("@ tex_coords %s\n", value); */

    fscanf ( text_in, "%s", tag );    /* clear OUTPUT_POLY_NORMS tag */
    fscanf ( text_in, "%s", value );  /* get value */
    if ( strcmp ( value, "on" ) == 0 )
        polygon_norms_exist = 1;
    else
        polygon_norms_exist = 0;

    /* printf ("@ poly_norms %s\n", value); */

    fscanf ( text_in, "%s", tag );    /* clear OUTPUT_HIERARCHY tag */
    fscanf ( text_in, "%s", value );  /* get value */
    if ( strcmp ( value, "on" ) == 0 )
        hierarchy_exists = 1;
    else
        hierarchy_exists = 0;

    /* printf ("@ hierarchy %s\n", value); */

    fscanf ( text_in, "%s", tag );    /* clear SHOW_INDEX_COUNTERS tag */
    fscanf ( text_in, "%s", value );  /* get value */
    if ( strcmp ( value, "on" ) == 0 )
        index_counters_exist = 1;
    else
        index_counters_exist = 0;

    /* printf ("@ index_counters %s\n", value); */

    /* do mode variables ... */

    fscanf ( text_in, "%s", tag );    /* clear TEXTURE_MODE tag */
    fscanf ( text_in, "%s", value );  /* get value */

    texture_mode = COLOR_ONLY;

    if ( strcmp ( value, "per_shader" ) == 0 )
        texture_mode = TEX_PER_SHADER;

    if ( strcmp ( value, "per_polygon" ) == 0 )
        texture_mode = TEX_PER_POLY;

    /* printf ("@ texture_mode %s\n", value); */

    return ( COOL );
}

static void Set_Matrix(AtsNode *n, double m[4][4]) {
    AtrXform atbT;

    atbT.data[0]  = (float)m[0][0];
    atbT.data[1]  = (float)m[2][0];
    atbT.data[2]  = (float)m[1][0];

    atbT.data[3]  = (float)m[3][0];

    atbT.data[4]  = (float)m[0][2];
    atbT.data[5]  = (float)m[2][2];
    atbT.data[6]  = (float)m[1][2];

    atbT.data[7]  = (float)m[3][2];

    atbT.data[8]  = (float)m[0][1];
    atbT.data[9]  = (float)m[2][1];
    atbT.data[10] = (float)m[1][1];

    atbT.data[11] = (float)m[3][1];

    atbT.data[12] = (float)m[0][3]*vertex_scale;
    atbT.data[13] = (float)m[2][3]*vertex_scale;

    atbT.data[14] = (float)m[1][3]*vertex_scale;

    atbT.data[15] = (float)m[3][3];

    if ( atbT.data[3] != 0.0f )
        printf("bad order %f\n", atbT.data[3]);

    if ( atbT.data[7] != 0.0f )
        printf("bad order %f\n", atbT.data[3]);

    if ( atbT.data[11] != 0.0f )
        printf("bad order %f\n", atbT.data[3]);

    atsFrameXform(n, &atbT);
}

static void Add_Shape(AtsNode *n, char *object_name) {
    AtsNode *c;

    /* printf("adding shape %s to node %s\n", object_name, atsNodeGetName(n)); 
     */

    if ((c = atsFind(all_shapes, object_name, NULL)) == NULL ) {
        atuError(FXTRUE, "%s: unknown child %s\n", converterName, object_name);
    }

    atsGroupAddChild(n, c);
}

#define MAX_LEVELS 100

static int Process_Hierarchy ( AtsNode *root ) {
    double localTM [4][4];
    double worldTM [4][4];
    AtsNode *n, *parent[MAX_LEVELS];
    char object_name[256];

    double tx, ty, tz;
    double rx, ry, rz;
    double sx, sy, sz;

    double prx, pry, prz;
    double psx, psy, psz;

    /* int c; <-- unused */ 

    int done, level;
    int file_stat;
    char type;

    /* printf ("@ PROCESSING HIERARCHY ... "); */

    parent[0] = root;

    fscanf ( text_in, "%s", tag );   /* clear HIERARCHY_LIST tag */
    fscanf ( text_in, "%s", tag );   /* get hierarchy data level tag */

    if ( strcmp ( tag, "MAT4x4" ) == 0 ) {
        /* means that matrices have been output */

        hier_xform_matrices = 1;
    } else {

        atuError(FXFALSE, "HXP format not supported yet\n");
        hierarchy_level = TREE_ONLY;

        if ( strcmp ( tag, "HX" ) == 0 )
            hierarchy_level = TREE_XFORMS;

        if ( strcmp ( tag, "HXP" ) == 0 )
            hierarchy_level = TREE_XFORMS_PIVOTS;
    }

    /* get number of top level dags */
    fscanf ( text_in, "%d", &num_top_level );

    /* clear top_level tag */
    fscanf ( text_in, "%s", tag );

    done = 0;
    while ( !done ) {
        file_stat = fscanf ( text_in, "%s", tag );

        if ( file_stat == EOF )
        {
            atuError(FXFALSE, "Error: premature EOF found, expecting further data.\n");
            return ( EOF );
        }

        /* check to see if we got the END_HIERARCHY_LIST tag */
        if ( tag[0] == 'E' )
        {
            done = 1;
            continue;
        }
        else  /* otherwise we have a node level number */
        {
            level = atoi ( tag );
        }

        if ( level >= (MAX_LEVELS-2) )
            atuError(FXTRUE,"%s: object nesting level too deep: max %d\n",
                     MAX_LEVELS);

        fscanf ( text_in, "%s %s", value, object_name );  /* do node line */
        type = value[0];

        if ( hier_xform_matrices ) {
            /* clear localTM: tag */
            fscanf ( text_in, "%s", tag );

            /* get first row of LOCAL transformation matrix */
            fscanf ( text_in, "%lf %lf %lf %lf", localTM[0]+0, localTM[0]+1,
                                    localTM[0]+2, localTM[0]+3  );
            /* get second row of LOCAL transformation matrix */
            fscanf ( text_in, "%lf %lf %lf %lf", localTM[1]+0, localTM[1]+1,
                                    localTM[1]+2, localTM[1]+3  );
            /* get third row of LOCAL transformation matrix */
            fscanf ( text_in, "%lf %lf %lf %lf", localTM[2]+0, localTM[2]+1,
                                    localTM[2]+2, localTM[2]+3  );
            /* get fourth row of LOCAL transformation matrix */
            fscanf ( text_in, "%lf %lf %lf %lf", localTM[3]+0, localTM[3]+1,
                                    localTM[3]+2, localTM[3]+3  );

            /* clear worldTM: tag */
            fscanf ( text_in, "%s", tag );

            /* get first row of WORLD transformation matrix */
            fscanf ( text_in, "%lf %lf %lf %lf", worldTM[0]+0, worldTM[0]+1,
                                    worldTM[0]+2, worldTM[0]+3  );
            /* get second row of WORLD transformation matrix */
            fscanf ( text_in, "%lf %lf %lf %lf", worldTM[1]+0, worldTM[1]+1,
                                    worldTM[1]+2, worldTM[1]+3  );
            /* get third row of WORLD transformation matrix */
            fscanf ( text_in, "%lf %lf %lf %lf", worldTM[2]+0, worldTM[2]+1,
                                    worldTM[2]+2, worldTM[2]+3  );
            /* get fourth row of WORLD transformation matrix */
            fscanf ( text_in, "%lf %lf %lf %lf", worldTM[3]+0, worldTM[3]+1,
                                    worldTM[3]+2, worldTM[3]+3  );
        }
        else
        {
            if ( ( hierarchy_level == TREE_XFORMS ) ||
                 ( hierarchy_level == TREE_XFORMS_PIVOTS ) )
            {
                /* clear tran: tag */
                fscanf ( text_in, "%s", tag );

                /* get translation values */
                fscanf ( text_in, "%lf %lf %lf", &tx, &ty, &tz );

                /* clear rot: tag */
                fscanf ( text_in, "%s", tag );

                /* get rotation values */
                fscanf ( text_in, "%lf %lf %lf", &rx, &ry, &rz );

                /* clear scal: tag */
                fscanf ( text_in, "%s", tag );

                /* get scale values */
                fscanf ( text_in, "%lf %lf %lf", &sx, &sy, &sz );
            }

            if ( hierarchy_level == TREE_XFORMS_PIVOTS )
            {
                /* clear sPiv: tag */
                fscanf ( text_in, "%s", tag );

                /* get scale pivot values */
                fscanf ( text_in, "%lf %lf %lf", &psx, &psy, &psz );

                /* clear rPiv: tag */
                fscanf ( text_in, "%s", tag );

                /* get rotation pivot values */
                fscanf ( text_in, "%lf %lf %lf", &prx, &pry, &prz );
            }
        }

        /* Do something here with all this node transform data ... */

        if ( root == NULL )
           continue; /* first pass */

        if (( n = atsFrameNew()) == NULL ) {
           atuError(FXTRUE, "Could not allocate node in %s\n", converterName);
        }

        atsNodeName(n, object_name);

        Set_Matrix(n, localTM);

        if ( type == 'P' )
            Add_Shape(n, object_name);

        atsGroupAddChild(parent[level], n);
        parent[level+1] = n;
    }

    return ( COOL );
}

static int Process_Object ( AtsNode **sreturn ) {
    char object_name[ 128 ];
    FxU16 vnum, nnum, tnum, pnum;
    int  coord_space;
    int  file_stat, c, a;
    AtsMaterial *mat;
    AtsPrim *prim ;

    double vx, vy, vz;
    double nx, ny, nz;
    double u, v;

    double Nx, Ny, Nz;

    int vindex ;
    int nindex ;
    int tindex ;

    char texture_type;
    int  tex_index;
    int  num_verts;
    int  r, g, b;
    int  index;
    int  maxVerts;

    /* get object info ... */

    fscanf ( text_in, "%s", tag );          /* clear OBJECT_START tag */
    fscanf ( text_in, "%s", object_name );  /* get object's name */

    /* printf ("@ PROCESSING OBJECT %s ...\n", object_name); */

    /* get list counters ... */

    vnum = 0;
    nnum = 0;
    tnum = 0;
    pnum = 0;

    /* we assume that the vertex list is always the first list for an  */
    /* object and that each object will be guaranteed to have a vertex */
    /* list ... so we get counter tags until we hit the VERTEX tag of  */
    /* the vertex list                                                 */

    while ( strcmp ( tag, "VERTEX" ) != 0 ) {
        file_stat = fscanf ( text_in, "%s", tag );
        if ( file_stat == EOF ) {
            atuError (FXFALSE, 
                      "Error: premature EOF found, expecting further data.\n");
            return ( EOF );
        }

        switch ( tag[0] ) {
        case 'v':  /* for vertex counter */
            vnum = (FxU16)atoi ( tag+1 );
            break;

        case 'n':  /* for vertex normal counter */
            nnum = (FxU16)atoi ( tag+1 );
            break;

        case 't':  /* for vertex texture coordinate counter */
            tnum = (FxU16)atoi ( tag+1 );
            break;

        case 'p':  /* for polygon counter */
            pnum = (FxU16)atoi ( tag+1 );
            break;

        case 'V':  /* for VERTEX tag */
            break;

        default:
            atuError (FXFALSE, "Error: not a valid counter.\n");
            return ( NOT_COOL );
        }
    }

    /* if we got here we have already cleared the VERTEX tag of the */
    /* vertex list so process vertex list here ...                  */

    fscanf ( text_in, "%s", tag );  /* get coordinate space tag */
    if ( strcmp ( tag, "local" ) == 0 )
        coord_space = LOCAL_SPACE;
    else
        coord_space = WORLD_SPACE;

    maxVerts = ATM_MAX(vnum, nnum);
    maxVerts = ATM_MAX(tnum, maxVerts);

    prim = atsShapeBuildInit((FxU32)maxVerts, 50);
    prim->flags = ATS_PRIM_CW;
    prim->lBind = ATS_BIND_INDEXED_VERTEX;
    prim->nBind = vertex_norms_exist ? ATS_BIND_INDEXED_VERTEX : ATS_BIND_NONE; 
    prim->cBind = ATS_BIND_NONE;
    prim->tBind = texture_coords_exist ? ATS_BIND_INDEXED_VERTEX : ATS_BIND_NONE; 

    /* do vertex list ... */

    /* printf ("@     processing vertex list ...\n"); */

    for ( c=0; c < vnum; c++ ) {
        /* get index counter if it's there */
        if ( index_counters_exist )
            fscanf ( text_in, "%d", &index );

        /* get position of vertex */
        fscanf ( text_in, "%lf %lf %lf", &vx, &vy, &vz );

        /* Do something here with values vertex position vx, vy, vz */

        prim->locations[c][0] = vertex_scale*(float)vx;
        prim->locations[c][1] = vertex_scale*(float)vz;
        prim->locations[c][2] = vertex_scale*(float)vy;
    }

    /* do Vertex Normal list (if there is a list) ... */

    if ( vertex_norms_exist ) {
        /* printf ("@     processing vertex normal list ...\n"); */

        /* clear NORMAL tag of normal list */
        fscanf ( text_in, "%s", tag );

        for ( c=0; c < nnum; c++ ) {
            /* get index counter if it's there */
            if ( index_counters_exist )
                fscanf ( text_in, "%d", &index );

            /* get normal values */
            fscanf ( text_in, "%lf %lf %lf", &nx, &ny, &nz );

            /* Do something here with values nx, ny, nz */

            prim->normals[c][0] = (float)nx;
            prim->normals[c][1] = (float)nz;
            prim->normals[c][2] = (float)ny;
        }
    }

    /* do texture coordinate list (if there is one) ... */

    if ( texture_coords_exist ) {
        /* printf ("@     processing tex coord list ...\n"); */

        /* clear TEXCOORD tag of texture coord list */
        fscanf ( text_in, "%s", tag );

        for ( c=0; c < tnum; c++ ) {
            /* get index counter if it's there */
            if ( index_counters_exist )
                fscanf ( text_in, "%d", &index );

            /* get tex coord values */
            fscanf ( text_in, "%lf %lf", &u, &v );

            /* Do something here with values u, v */

            prim->texCoords[c][0] = (float)v;
            prim->texCoords[c][1] = (float)(1.0-u);
        }
    }

    /* do polygon list ... */

    /* clear POLYGON tag of polygon list */
    fscanf ( text_in, "%s", tag );
    /* printf ("@     processing polygon list ...\n"); */

    for ( c=0; c < pnum; c++ ) {
        /* get index counter if it's there */
        if ( index_counters_exist )
            fscanf ( text_in, "%d", &index );

        /* get the number of points in poly */
        fscanf ( text_in, "%d", &num_verts );
        prim->numVerts = num_verts;

        /* clear 'v' vertex index list tag */
        fscanf ( text_in, "%s", tag );

        /* get the values for the vertex index array */
        for ( a=0; a < num_verts; a++ ) {
            fscanf ( text_in, "%d", &vindex );
            prim->lIdx[a] = (FxU16)vindex;
        }

        if ( vertex_norms_exist ) {
            /* clear 'n' normal index list tag */
            fscanf ( text_in, "%s", tag );

            /* get the values for the vertex normal index array */
            for ( a=0; a < num_verts; a++ ) {
                fscanf ( text_in, "%d", &nindex );
                prim->nIdx[a] = (FxU16)nindex;
            }
        }

        if ( texture_coords_exist ) {
            /* clear 't' tex coord index list tag */
            fscanf ( text_in, "%s", tag );

            /* get the values for the vertex texture coordinate index array */
            for ( a=0; a < num_verts; a++ ) {
                fscanf ( text_in, "%d", &tindex );
                prim->tIdx[a] = (FxU16)tindex;
            }
        }

        if ( polygon_norms_exist ) {
            /* clear 'N' polygon normal tag */
            fscanf ( text_in, "%s", tag );

            /* get the values for the Polygon's Normal */
            fscanf ( text_in, "%lf %lf %lf", &Nx, &Ny, &Nz );

            /* Do something with the polygon normal here */
        }

        /* texture or color entry ... */

        /* get texture type tag either T or RGB */
        fscanf ( text_in, "%s", tag );

        texture_type = tag[0];

        switch ( texture_type ) {
        case RGB_COLOR:
            fscanf ( text_in, "%d %d %d", &r, &g, &b );
            mat = coloredMaterial(r, g, b);
            break;

        case TEXTURE_INDEX:
            fscanf ( text_in, "%d", &tex_index );

            /* if we find a texture index there will be texture table */

            mat = texturedMaterial(tex_index);
            texture_table_exists = 1;
            break;

        default:
            atuError (FXFALSE, "Error: unknown texture type found.\n");
            return ( NOT_COOL );
        }

        /* Do something here with the texture info */

        atsShapeMaterial(mat);

        if (!atsShapeAddPrim() )
            return ( NOT_COOL );
    }

    /* clear OBJECT_END tag and object name at end of object data section */

    fscanf ( text_in, "%s %s", tag, value );

    *sreturn = atsShapeConstruct(FXFALSE);

    atsNodeName(*sreturn, object_name);

    return ( COOL );
}

static int Process_Texture_Table ( void ) {
    int  bytesPerLine;
    int  image_files;
    int  bytes_in_header;
    int  row, col;
    char *pTmp;
    int  tex_entries, c, index;
    int  xres, yres, num_bytes;
    char tex_file_name [ 80 ], full_path[256];
    char imageFileName [ 80 ];
    FILE *textureFile = NULL;
    char buff[256];
    char *imageBuff;
    FxBool hasAlpha = FXFALSE;
    int min_alpha = 256, max_alpha = 0;

    /* tag gets the TEXTURE_TABLE identifier string */
    /* tex_file_name gets the name of the associated texture binary file */
    /* tex_entries gets the number of entries there are in the table */
    /* value string gets the 'entries' descriptor tag */

    fscanf ( text_in, "%s %s %d %s", tag, tex_file_name, &tex_entries, value );

    /* get header byte info ... */
    /* tag clears the '(header:' descriptor tag and value clears the */
    /*     'bytes)' descriptor tag, bytes_in_header gets integer value */
    /* in the case of generation of image files instead of RTG .bin file */
    /* tag will clear a '(' descriptor and value will clear a ')' descriptor */

    fscanf ( text_in, "%s %d %s", tag, &bytes_in_header, value );

    /* if bytes_in_header is zero it means that image files were generated */
    /* instead of an RTG .bin file ... */
    if ( bytes_in_header == 0 )
        image_files = 1;
    else {
        image_files = 0;
        if ( ( atuFileLocate(tex_file_name, full_path) == NULL ) || 
             ((textureFile = fopen(full_path, "rb")) == NULL )) {
            atuError(FXTRUE, "Could not open bin file %s\n", tex_file_name);
        }

        /* skip header */

        if ( fread( buff, bytes_in_header, 1, textureFile ) != 1 ) {
            atuError(FXTRUE, "Could not read bin header\n");
        }
    }

    for ( c=0; c < tex_entries; c++ ) {
        hasAlpha = FXFALSE;
        /* get index counter if it's there */
        if ( index_counters_exist )
            fscanf ( text_in, "%d", &index );

        /* This version has only one format for texture table entry which    */
        /* is:  RGBA [Xres] [Yres] [source_shader_name] [extra_info]         */
        /*  */
        /*  If an RTG binary is being generated then the extra_info field    */
        /*  will contain a "bytes_in_bin_file" value which should be equal   */
        /*  to Xres * Yres * 4, and currently only RGBA tag type is          */
        /*  supported.                                                       */
        /*  */
        /*  NOTE: if image files were generated instead of the RTG .bin file */
        /*        then the extra_info field will actually contain            */
        /*        the name of the image file that was generated for the      */
        /*        texture entry (full path to it not included).              */

        if ( image_files ) {
            fscanf ( text_in, "%s %d %d %s %d", tag, &xres, &yres, temp_string,
                     imageFileName );
            /* tag clears the RGBA tag and temp_string gets the source */
            /* shader name. The imageFileName string gets the name of  */
            /* the image file created for that texture - note that it  */
            /* will not contain the path to it.                        */

            num_bytes = 0;
        } else {
            fscanf ( text_in, "%s %d %d %s %d", tag, &xres, &yres, temp_string,
                     &num_bytes );
            /* tag clears the RGBA tag and temp_string gets the source */
            /* shader name. */

            imageFileName[0] = '\0';

            if (( imageBuff = atuMemMalloc(num_bytes)) == NULL ) {
                atuError(FXTRUE, "Could not allocate image buffer\n");
            }

            /* images are upside down */

            bytesPerLine = xres<<2;

            if ( bytesPerLine*yres != num_bytes )
                atuError(FXTRUE, "Invalid image length\n");

            for ( row = 0; row < yres; row++ ) {
                char *pBuff = imageBuff+((yres-row-1)*bytesPerLine);
                if ( fread( pBuff, bytesPerLine, 1, textureFile ) != 1 ) {
                    atuError(FXTRUE, "Could not read texture data\n");
                }
                for ( col = 0, pTmp = pBuff; col < xres; col++, pTmp += 4 ) {
                    char c;
                    /* 0 blue, 1 green, 2 red, 3 alpha */

                    if ( pTmp[3] != 0 )
                        hasAlpha = FXTRUE;

                    if ( pTmp[3] < min_alpha ) min_alpha = pTmp[3];
                    if ( pTmp[3] > max_alpha ) max_alpha = pTmp[3];

                    pTmp[3] = (FxU8)255; hasAlpha = FXFALSE;
                   
                    /* pTmp[3] =255; */
                    c = pTmp[0]; pTmp[0] = pTmp[2]; pTmp[2] = c;
                    /*
                    pTmp[2] = 255;
                    c = pTmp[3]; pTmp[3] = pTmp[2]; 
                    pTmp[2] = pTmp[1]; pTmp[1] = pTmp[0]; pTmp[0] = 255;
                    */
            
                }
                /* memset(pBuff, 255, bytesPerLine); */
            }

            associateTexture(c, hasAlpha, imageToTexture(imageBuff, hasAlpha, xres, yres));
        }

        /* Do something with this texture table entry:  */
        /* during this processing the associated .bin file can also be */
        /* opened and processed as well for pixel data as you go through */
        /* each texture entry. OR if image files were generated you can */
        /* process those as you go. */
    }

    /* clear the END_TEXTURE_TABLE tag and clear the repeated .bin file name */

    fscanf ( text_in, "%s %s", tag, tex_file_name );
    return 0; /* jdt - don't know how this is interpreted. */
}

static AtsNode* Process_File ( const char *filename, FxBool first ) {
    AtmSphere bsphere;
    AtsNode *g, *h;
    AtsNode *s;
    int c;
    char full_path[256];

    if ( ( atuFileLocate(filename, full_path) == NULL ) || 
         ((text_in = fopen( full_path, "rt" )) == NULL )) {
        atuError( FXTRUE, "rtgtoats: Not able to open RTG file \"%s\"\n", filename );
    }

    if ((g = atsGroupNew()) == NULL ) {
         atuError(FXFALSE, "%s: could not allocate group\n", converterName);
         return NULL;
    }

    texture_table_exists = 0;

    /* Process */

    if ( text_in )
    {
        if ( Process_Header_Info () != COOL ) {
            fclose ( text_in );
            return NULL;
        }

        if ( hierarchy_exists )
        {
            if ( Process_Hierarchy (NULL) != COOL )
            {
                fclose ( text_in );
                return NULL;
            }
        }

        for ( c=0; c < num_objects; c++ )
        {
            if ( Process_Object (&s) != COOL )
            {
                fclose ( text_in );
                return NULL;
            }

            atsGroupAddChild(g, s);
        }

        if ( first && texture_table_exists )
            Process_Texture_Table ();

        /* second pass for heirarchy */

        if ( !hierarchy_exists )
            goto done;
        
        if ((h = atsGroupNew()) == NULL ) {
             atuError(FXFALSE, "%s: could not allocate group\n", converterName);
             return NULL;
        }

        rewind(text_in);

        all_shapes = g;

        if ( Process_Header_Info () != COOL ) {
            fclose ( text_in );
            return NULL;
        }

        if ( Process_Hierarchy (h) != COOL ) {
            fclose ( text_in );
            return NULL;
        }

        g = h;

    } else {
        atuError (FXTRUE, "Error: unable to open '%s' for input.\n", filename );
        return NULL;
    }

done:
    if ( text_in )
        fclose ( text_in );

    atsComputeBSphere(g, &bsphere);

    atsNodeBSphere(g, &bsphere);

    return g;
}

/* Main Routine */

AtsObject *atsLoadFromRTG(const char *filename) {
    AtsNode *g, *c;
    int frameNumber;
    AtmSphere bsphere;
    char buff[80];
    int startFrame = 0, endFrame = 0;
    AtsType *targetType;
    FxBool loadSequence = FXFALSE;

    /* float x = (float)sqrt(1.0f); /* TBD keep msdev happy */

    atsConverterGetIntAttr(NULL, ATS_CATTR_LOAD_TEXTURES, &usetextures);

    if ( !atsConverterGetFloatAttr(NULL, ATS_CATTR_VERTEX_SCALE, &vertex_scale) )
        vertex_scale = 1.0f;

    if ((g = atsSeqNew()) == NULL ) {
         atuError(FXFALSE, "%s: could not allocate group\n", converterName);
         return NULL;
    }

    if ( atsConverterGetAttr(NULL, ATS_CATTR_TARGET_TYPE, (void *)&targetType) ) {
        if (atsIsSubClassOf(targetType, atsGetTypeFromName("Seq"))) {
            loadSequence = FXTRUE;
        }
    }

    if ( loadSequence ) {
        atsConverterGetIntAttr(NULL, ATS_CATTR_START_FRAME, &startFrame);
        atsConverterGetIntAttr(NULL, ATS_CATTR_END_FRAME, &endFrame);
        for ( frameNumber= startFrame; frameNumber<= endFrame; frameNumber++ ) {
            sprintf(buff, filename, frameNumber);
            c = Process_File ( buff, (frameNumber == startFrame) ) ;

            if ( c != NULL ) {
                atsGroupAddChild(g, c);
            } else {
                atuError(FXFALSE, "error in processing file %s\n", buff);
                return NULL;
            }
        }
    } else return Process_File ( filename, FXTRUE ) ;

    atsComputeBSphere(g, &bsphere);

    atsNodeBSphere(g, &bsphere);

    return g;
}

void atsInitRTG(void) {
}

void atsTermRTG(void) {
}
