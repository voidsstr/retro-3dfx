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
** 3DS Loader Header File
*/

#ifndef TDS_H__
#define TDS_H__

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------  Includes -----------------------------*/

#include <stdio.h>

/*-----------------------------  Defines ------------------------------*/

#define MAX_VERTS_PER_FILE              100000
#define TDS_SPLINE_USE_TENSION          1
#define TDS_SPLINE_USE_CONTINUINITY     2
#define TDS_SPLINE_USE_BIAS             4
#define TDS_SPLINE_USE_EASE_TO          8
#define TDS_SPLINE_USE_EASE_FROM        16

/*---------------------  Data Types and Structures --------------------*/

typedef unsigned char   tds_ubyte;
typedef unsigned short  tds_ushort;
typedef short           tds_short;
typedef unsigned int    tds_uint;
typedef int             tds_long;
typedef unsigned int    tds_ulong;
typedef float           tds_float;
typedef char           *tds_string;

/* 3DS Loader Matrix Structure */
typedef float           tds_matrix[4][4];

/* 3DS Loader Point Structure */
typedef struct
{
  float x, y, z;
} tds_point;

/* 3DS Loader Texture Vertex Structure */
typedef struct
{
  float u, v;
} tds_tex_vert;

/* 3DS Loader Face Structure */
typedef struct
{
  short v1, v2, v3, flags;
  short material_index;
  tds_point normal;
  tds_ulong smoothing_group;
} tds_face;

/* 3DS Chunk Header */
typedef struct
{
  tds_ushort    id;
  tds_uint      len;
  tds_long      offset;
} TDSChunkHeader;

/* 3DS Loader Texture Map Structure */
typedef struct
{
  char          mapname[13];
  int           has_mapname;

  tds_short     strength;
  int           has_strength;

  tds_ushort    map_tiling;
  int           has_map_tiling;

  tds_float     tex_blur;
  int           has_tex_blur;

  tds_float     u_scale;
  int           has_u_scale;

  tds_float     v_scale;
  int           has_v_scale;

  tds_float     u_offset;
  int           has_u_offset;

  tds_float     v_offset;
  int           has_v_offset;

  tds_float     angle;
  int           has_angle;

  tds_ubyte     col1[3];
  int           has_col1;

  tds_ubyte     col2[3];
  int           has_col2;

  tds_ubyte     rcol[3];
  int           has_rcol;

  tds_ubyte     gcol[3];
  int           has_gcol;

  tds_ubyte     bcol[3];
  int           has_bcol;
} tds_texmap;

/* 3DS Loader Reflection Map Structure */
typedef struct
{
  float weight;
  char mapname[13];
} tds_reflmap;

/* 3DS Shading Types */
enum { TDS_SHADE_WIRE, TDS_SHADE_FLAT, TDS_SHADE_GOURAUD, TDS_SHADE_PHONG, TDS_SHADE_METAL };

/* 3DS Loader Material Structure */
typedef struct
{
  char          name[22];
  int           has_name;

  tds_ubyte     ambient_color[3];
  tds_ubyte     diffuse_color[3];
  tds_ubyte     specular_color[3];
  tds_float     shininess_strength; /* [0..1] */
  tds_float     shininess; /* [0..1] */
  int           does_self_illum; /* 0 for on, 1 for off */

  tds_texmap    texmap;
  int           has_texmap;

  tds_reflmap   reflmap;
  int           has_reflmap;

  int           two_sided;
  int           shade_type;
} tds_material;

/* 3DS Animation Track Header */
typedef struct
{
  tds_ushort    looping_flags;
  tds_ulong     num_keys;
} tds_track_header;

/* 3DS Animation Key Header */
typedef struct
{
  tds_long      frame_number;           /* the frame number where the key occurs */
  tds_short     spline_term_flags;      /* a set of bits that determind which spline
                                           terms are present */
  tds_float     spline_tension;         /* spline tension value ranges from -1.0 to
                                           1.0.  Only present if bit 0 set. */
  tds_float     spline_continuinity;    /* spline continuinity value ranges from -1.0
                                           to 1.0.  Only present if bit 2 set. */
  tds_float     spline_bias;            /* spline bias value ranges from 1.0 to 1.0.
                                           Only present if bit 2 set. */
  tds_float     ease_to;                /* ease to value ranges from 0.0 to 1.0.
                                           Only present if bit 3 set. */
  tds_float     ease_from;              /* ease from value ranges from 0.0 to 1.0.
                                           Only present if bit 4 set. */
} tds_key_header;

/* 3DS Loader Bounding Box Structure */
typedef struct
{
  tds_point min;
  tds_point max;
} tds_bbox;

/* 3DS Animation Node Header */
typedef struct
{
  char object_name[11];
  tds_ushort flag1;
  tds_ushort flag2;
  tds_ushort parent_index;
} tds_node_header;

/* 3DS Animation Position Header */
typedef struct
{
  tds_key_header        key_header;
  tds_point             position;
} tds_pos_track_tag;

/* 3DS Animation Field of View Header */
typedef struct
{
  tds_key_header        key_header;
  float                 fov_degrees;
} tds_fov_track_tag;

/* 3DS Animation Roll Angle Header */
typedef struct
{
  tds_key_header        key_header;
  float                 roll_degrees;
} tds_roll_track_tag;

/* 3DS Animation Rotation Angle Header */
typedef struct
{
  tds_key_header        key_header;
  float                 rot_axis[3];
  float                 rot_angle;
  /*
   * Store a rotation matrix with the key which represents the
   * calculated rotation of the object up until the time of
   * this key.  This information is not read from the 3ds file
   * but is rather calculated after reading.
   */
  tds_matrix            rot_matrix;
} tds_rot_track_tag;

/* 3DS Animation Scale Header */
typedef struct
{
  tds_key_header        key_header;
  tds_float             scale[3];
} tds_scl_track_tag;

/* 3DS Animation Morphing Header */
typedef struct
{
  tds_key_header        key_header;
  char                  named_morph_object[11];
} tds_morph_track_tag;

/* 3DS Animation Hot Spot Header */
typedef struct
{
  tds_key_header        key_header;
  tds_float             angle;
} tds_hot_track_tag;

/* 3DS Animation Fall Off Header */
typedef struct
{
  tds_key_header        key_header;
  tds_float             angle;
} tds_fall_track_tag;

/*
 * WARNING!!: The format of this structure is not completely specified
 * in the docs.  Will have to make sure that this is the actual format
 * by experiment - Hide Track Header 
 */
typedef struct
{
  tds_key_header        key_header;
} tds_hide_track_tag;

/* 3DS Animation Position Information Header */
typedef struct
{
  tds_track_header      track_header;
  tds_pos_track_tag     *pos_track_tags;
} tds_pos_track_info;

/* 3DS Animation Rotation Information Header */
typedef struct
{
  tds_track_header      track_header;
  tds_rot_track_tag     *rot_track_tags;
} tds_rot_track_info;

/* 3DS Animation Scale Information Header */
typedef struct
{
  tds_track_header      track_header;
  tds_scl_track_tag     *scl_track_tags;
} tds_scl_track_info;

/* 3DS Animation Field of View Information Header */
typedef struct
{
  tds_track_header      track_header;
  tds_fov_track_tag     *fov_track_tags;
} tds_fov_track_info;

/* 3DS Animation Roll Angle Information Header */
typedef struct
{
  tds_track_header      track_header;
  tds_roll_track_tag    *roll_track_tags;
} tds_roll_track_info;

/* 3DS Animation Morphing Information Header */
typedef struct
{
  tds_track_header      track_header;
  tds_morph_track_tag   *morph_track_tags;
} tds_morph_track_info;

/* 3DS Animation Hidden Track Information Header */
typedef struct
{
  tds_track_header      track_header;
  tds_hide_track_tag    *hide_track_tags;
} tds_hide_track_info;

/* 3DS Animation Object Node Structure */
typedef struct
{
  tds_ushort            node_id;
  tds_node_header       node_header;
  tds_point             pivot;
  char                  instance_name[11];
  tds_bbox              bbox;
  int                   bbox_set;

  tds_pos_track_info    pos_track_info;
  tds_rot_track_info    rot_track_info;
  tds_scl_track_info    scl_track_info;
  tds_morph_track_info  morph_track_info;
  tds_hide_track_info   hide_track_info;
  
  tds_float             morph_smooth;
} tds_object_node;

/* 3DS Animation Camera Node Structure */
typedef struct
{
  tds_ushort            node_id;
  tds_node_header       node_header;

  tds_pos_track_info    pos_track_info;
  tds_fov_track_info    fov_track_info;
  tds_roll_track_info   roll_track_info;
} tds_camera_node;

/* 3DS Animation Target Node Structure */
typedef struct
{
  tds_ushort            node_id;
  tds_node_header       node_header;

  tds_pos_track_info    pos_track_info;
} tds_target_node;

/* Key Node Enumeration Types */
typedef enum { TDS_OBJECT_NODE_TAG,
               TDS_CAMERA_NODE_TAG,
               TDS_TARGET_NODE_TAG, 
               TDS_LIGHT_NODE_TAG, 
               TDS_SPOTLIGHT_NODE_TAG, 
               TDS_L_TARGET_NODE_TAG, 
               TDS_AMBIENT_NODE_TAG } tds_key_node_type;

/* 3DS Animation Key Node Structure */
typedef struct
{
  tds_key_node_type             type;
  union
  {
    tds_object_node             object_node;
    tds_camera_node             camera_node;
    tds_target_node             target_node;
#if 0
    tds_light_node              light_node;
    tds_spotlight_node          spotlight_node;
    tds_l_target_node           l_target_node;
    tds_ambient_node            ambient_node;
#endif
  } nodes;
} tds_key_node;

/* 3DS Object Name */
typedef char            tds_object_name[11];

/* 3DS Animation Camera State Structure */
typedef struct
{
  tds_point camera_position;
  tds_point target_position;
  float roll_degrees;
  float fov_degrees;
} tds_camera_state;

/* 3DS File Information Structure */
typedef struct
{
  FILE                  *fp;
  char                  *filename;
  int                   num_materials;
  int                   num_n_tri_objs;
  tds_matrix            *mesh_matrices;
  int                   *num_verts_in_object;
  int                   *num_faces_in_object;
  tds_object_name       *object_names;
  tds_point             **world_verts; /* world_verts[object_index][vert_num] */
  tds_point             **local_verts; /* local_verts[object_index][vert_num] */
  tds_point             **vert_normals;
  tds_tex_vert          **tex_verts;
  tds_face              **faces;
  tds_material          *materials;
  int                   current_object;
  int                   current_material;

  /*
   * keyframe stuff.
   */
  tds_long              animation_length;
  tds_long              first_frame;
  tds_long              last_frame;
  tds_long              current_frame;
  tds_ushort            current_key_node_index;
  tds_key_node          *key_nodes[65536];
  int                   key_node_to_object_index[65536];        

  /*
   * matrix per frame per keyframed object which relates the object to
   * its parent spatially.
   */
  tds_matrix            *local_matrices[65536];

  /*
   * matrix per frame per keyframed object which relates the object to
   * the world spatially.
   */
  tds_matrix            *world_matrices[65536];

  /*
   * State information for each frame of animation for each camera.
   * camera_state[65536][frame]
   */
  tds_camera_state      *camera_state[65536];

  int                   first_frame_set;
  int                   last_frame_set;
  int                   current_frame_set;

  int                   has_animation;
} TDSFile;

/*
** 3DS Loader Function Prototypes
*/

int TDSReadFile( TDSFile *file, const char *filename );
void TDSReadFileFromFP( TDSFile *file, FILE *fp );
void TDSPointMatMult( tds_point *result, tds_point *p, tds_matrix m );
void TDSPrintMatrix( tds_matrix mat );
void TDSVecSub( tds_point *result, tds_point *a, tds_point *b );
float TDSVecDot( tds_point *a, tds_point *b );
void TDSVecNorm( tds_point *v );
void TDSVecCross( tds_point *v, tds_point *a, tds_point *b );
void TDSMatMakeIdent( tds_matrix m );
void TDSMatMakeXRot( tds_matrix m, float radians );
void TDSMatMakeYRot( tds_matrix m, float radians );
void TDSMatMakeZRot( tds_matrix m, float radians );
float TDSGetModelRadiusAtFrame( TDSFile *file, int frame );
void TDSInit( long max_num_verts_in_obj );
void TDSUseAnimation( TDSFile *file );
void TDSHackTransforms( TDSFile *file, const char *anifilename );
void TDSHackGeometry( TDSFile *file, const char *geomfilename );
int TDSFindNodeIndexByName( TDSFile *file, const char *name );
void TDSCalcVertexNormals( TDSFile *file, int object_number );
void TDSCalcFaceNormals( TDSFile *file, int object_number );
void TDSCalcBoundingBoxes( TDSFile *file );
float TDSGetNodeRadiusAtFrame( TDSFile *file, int frame, int node_index );
int TDSCountMaterialsInObject( TDSFile *tdsfile, int objnum, int **_mesh_material_to_object_material,
                               int **_object_material_to_mesh_material );
float TDSHermiteSourceDerivative( float tension, float continuity,
                                  float bias, float key_pt_left, 
                                  float key_pt, float key_pt_right );
float TDSHermiteDestinationDerivative( float tension, float continuity,
                                       float bias, float key_pt_left,
                                       float key_pt, float key_pt_right );
float TDSHermiteInterpolate( float src_pt_left, float src_pt,
                             float dest_pt, float dest_pt_right,
                             float src_tension, float src_continuity,
                             float src_bias, float dest_tension,
                             float dest_continuity, float dest_bias,
                             float u );

void TDSConvertToATBCoordSystem( TDSFile *tdsfile );

void TDSSetErrorFP( FILE *fp );

#ifdef __cplusplus
};
#endif

#endif

