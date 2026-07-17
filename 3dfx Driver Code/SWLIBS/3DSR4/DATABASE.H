#ifndef _DATABASE_H

#include "pxp.h"
#include "kxp.h"

#define AT_DEBUGGING
#ifndef GLIDE_HARDWARE
#define GLIDE_HARDWARE
#endif
#include "glide.h"
#include "atrender.h"
#include "atmath.h"
#include "texus.h"

#define RHCSFIX_X	(1.0f)
#define RHCSFIX_Y	(1.0f)
#define RHCSFIX_Z	(-1.0f)

#define D2RADIANS	((double)3.141592564/180.0)

extern short	my_fp_status;
extern short	glide_fp_status;

#pragma aux get_my_fp_status =\
  ".586" \
  "fnclex" \
  "fstcw   WORD PTR [my_fp_status]";


#pragma aux put_my_fp_status =\
  ".586" \
  "fnclex" \
  "fldcw   WORD PTR [my_fp_status]";


#pragma aux get_glide_fp_status =\
  ".586" \
  "fnclex" \
  "fstcw   WORD PTR [glide_fp_status]";


#pragma aux put_glide_fp_status =\
  ".586" \
  "fnclex" \
  "fldcw   WORD PTR [glide_fp_status]";



typedef char Name3ds[11];

typedef struct {
  BXPColor __far *	image;
  int			width;
  int			height;
  int			mipmap_size;
  char *		filename;
} Image;

typedef struct {
  Image			image;
  int			size;
  long			tmu;
  char *		pathname; /* full pathname to texture */
  char *		filename; /* just the filename to texture */
  Gu3dfInfo		tex_info;
  AtrImg *		aimage;
} Texture;

typedef struct {
  AtrMaterial *		material;
  Texture *		texture;
  char			name[17];
  int			two_sided;
} Material;

typedef struct {
  AtrTriSet *		mesh; /* array of constituent meshes */
  char			name[11];
  Material * *		material; /* array of Material * */
  int			nmeshes;
  float			min[3];
  float			max[3];
  AtrXform *		xform;
  AtrXform *		locmatrix;
  int			ipas_index;
  int			hidden;
} Object;

typedef struct {
  AtrCamera *		camera;
  char			name[11];
  AtmVector3		up;
  AtmVector3		dir;
  AtmVector3		horiz;
  AtmVector3		position;
  AtmVector3		target;
} Camera;

typedef struct {
  AtrLight *		light;
  AtrLight *		slight;
  char			off;
  char			name[11];
} Light;

typedef struct {
  Object *		objects;
  int			nobjects;
  Camera *		cameras;
  int			ncameras;
  Light *		lights;
  int			nlights;
  Material *		materials;
  int			nmaterials;
  Material * *		textures[2];
  int			ntextures[2];
  int			texture_mode;
  int			screen_width;
  int			screen_height;
  AtrColor		ambient;
  int			nframes;
  int			nnodes;
  int *			root_nodes;
  int			nroots;
  int			start;
  int			end;
  int			num_tmus;
  int			max_tmu_size;
} Database_3Dfx;

enum {
  MIPMAP_NONE = 0,
  MIPMAP_POINT,
  MIPMAP_BI,
  MIPMAP_TRI
};

typedef struct {
  long		resolution;
  long		texturing;
  long		mode;
  long		restrict;
  long		calibrate;
} Ini_State;

#define MAX_TEXTURE_MEMORY	(1 * 1024 * 1024)


extern int 	read_db(Database_3Dfx *, Name3ds *, int);
extern int 	render_db(Database_3Dfx *, char *, int, ViewportInfo *);

extern Database_3Dfx * kxp_database;
extern Database_3Dfx * alloc_db();
extern void	free_db(Database_3Dfx *);

extern void __far *	my_malloc(size_t, char *, int);
extern void		my_free(void __far *, char *, int);
extern void __far *	my_realloc (void __far *, size_t, char *, int);
extern int		malloc_calls;

extern void 	atmVectorMatrix4x4Mult (AtmVector3, AtmVector3, AtmMatrix4x4);

extern Object * find_object_by_name(Database_3Dfx *, char *);

extern void get_my_fp_status(void);
extern void put_my_fp_status(void);
extern void get_glide_fp_status(void);
extern void put_glide_fp_status(void);

extern void TransformSetRotationAboutAxis(AtrXform *, AtmVector3, float);

extern void error_printf(char *, int, char *, ...);

extern int resize_image (BXPColor __far *, int, int,
			 BXPColor __far *, int, int);

extern int write_ppm_file (char *, BXPColor __far *, int, int);
extern void xform_to_axform(float *, AtrXform *);

#define _DATABASE_H
#endif
