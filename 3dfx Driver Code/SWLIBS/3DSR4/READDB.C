#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <math.h>
#include <process.h>

#pragma off (unreferenced)
#include "3dsftk.h"
#pragma on (unreferenced)

#include "pxp.h"
#include "kxp.h"

#include "database.h"

#define AT_DEBUGGING
#ifndef GLIDE_HARDWARE
#define GLIDE_HARDWARE
#endif
#include "atrender.h"
#include "atmath.h"

#include "glide.h"
#include "texus.h"
#include "texusint.h"

#include "mymalloc.h"


#define VERTEX_MAX	(5000)

char			home_3ds[512];

typedef struct {
  FxU32			a, b, c;
} Triangle;


typedef struct {
  FxU8			r, g, b, a;
} RGBA;


static int		max_vertices = 0;
static int		max_texture_memory = 0;
static int		num_tmus = 0;
static int 		tmu_size[2] = { 0, 0 };
static int		ntextures[2] = { 0, 0 };

static GammaData	gamma_data;

#ifdef DEBUG_OUTPUT
extern FILE *		output = NULL;
#endif


static void set_camera_parameters (
  Camera *		cam,			
  float * 		target,
  float * 		position,
  float 		roll) {
  
  AtmVector3		direction;
  AtmVector3		up;
  AtmVector3		r1;
  AtrXform *		xform = atrXformAllocate (1);

  put_glide_fp_status();
  atmVector3Sub(direction, target, position);
  atmVector3Normalize(direction, direction);
  put_my_fp_status ();

  if (fabs(direction[2]) > fabs(100.0*direction[0]) &&
      fabs(direction[2]) > fabs(100.0*direction[1])) {
    /* camera is in mostly direction of z axis, things get wierd */
    up[0] = (direction[2] > 0) ? -1 : 1;
    up[1] = 0;
    up[2] = 0;
  }
  else {
    up[0] = 0;
    up[1] = 0;
    up[2] = 1;
  }

  TransformSetRotationAboutAxis (xform, direction, roll);
  atmVectorMatrix4x4Mult (up, up, xform->data);
    
  put_glide_fp_status();  
  atmVector3Cross(r1, direction, up);

  put_my_fp_status();
  cam->up[0] = up[0];
  cam->up[1] = up[1];
  cam->up[2] = up[2];
  cam->dir[0] = direction[0];
  cam->dir[1] = direction[1];
  cam->dir[2] = direction[2];
  cam->horiz[0] = r1[0];
  cam->horiz[1] = r1[1];
  cam->horiz[2] = r1[2];

  put_glide_fp_status();  
  atrXformDeallocate (xform);
  put_my_fp_status ();
}


static void set_lcs_to_wcs (
  AtrXform *		xform,
  float * 		target,
  float * 		position) {

  AtmVector3		direction;
  float *		m = &xform->data[0];
  float *		d = &direction[0];
  AtmVector3		up;
  AtmVector3		r1;
  AtmVector3		r2;

  put_glide_fp_status();
  atmVector3Sub(direction, target, position);
  atmVector3Normalize(direction, direction);

  put_my_fp_status();
  if (fabs(direction[2]) > fabs(100.0*direction[0]) &&
      fabs(direction[2]) > fabs(100.0*direction[1])) {
    /* camera is mostly in direction of z axis, things get wierd */
    up[0] = (direction[2] > 0) ? -1 : 1;
    up[1] = 0;
    up[2] = 0;
  }
  else {
    up[0] = 0;
    up[1] = 0;
    up[2] = 1;
  }
  
  put_glide_fp_status();
  atmVector3Cross(r1, up, direction);
  atmVector3Cross(r2, direction, r1);

  put_my_fp_status();
  m[0] = r1[0]; m[1] = r1[1]; m[2] = r1[2]; m[3] = 0;
  m[4] = r2[0]; m[5] = r2[1]; m[6] = r2[2]; m[7] = 0;
  m[8] = d[0];  m[9] = d[1];  m[10] = d[2]; m[11] = 0;
  m[12] = position[0]; m[13] = position[1]; m[14] = position[2]; m[15] = 1;
}


int matrix_is_identity (float * m) {
  int			ii;
  int			jj;
  int			is_identity = 1;

#define ROWS	4
#define COLUMNS 3  

  if (getenv ("DEBUG_LOCMATRIX")) {
    for (ii = 0; ii < ROWS; ii++) {
      error_printf (__FILE__, __LINE__, "[");
      for (jj = 0; jj < COLUMNS; jj++) {
	error_printf (__FILE__, __LINE__, " %f ", m[ii*COLUMNS + jj]);
	if (ii == jj && m[ii*COLUMNS + jj] != 1.0)
	  is_identity = 0;
	else if (m[ii*COLUMNS + jj] != 0)
	  is_identity = 0;
      }
      error_printf (__FILE__, __LINE__, "]\n");
    }
  }
  else {
    for (ii = 0; ii < ROWS; ii++) {
      for (jj = 0; jj < COLUMNS; jj++) {
	if (ii == jj && m[ii*COLUMNS + jj] != 1.0)
	  is_identity = 0;
	else if (m[ii*COLUMNS + jj] != 0)
	  is_identity = 0;
      }
    }
  }
  
  return is_identity;
}


void up_to_power_of_two(
  int * 		w,
  int * 		h) {
  
  static int		powers[] = {1, 2, 4, 8, 16, 32, 64, 128, 256};
  static int		npowers = sizeof(powers)/sizeof(powers[0]);
  int			ii;

  for (ii = 0; ii < npowers; ii++) {
    if (*w <= powers[ii]) {
      *w = powers[ii];
      break;
    }
  }
  if (ii == npowers)
    *w = powers[npowers - 1];

  for (ii = 0; ii < npowers; ii++) {
    if (*h <= powers[ii]) {
      *h = powers[ii];
      break;
    }
  }
  if (ii == npowers)
    *h = powers[npowers - 1];

  if (*w < *h)
    *w = *h;
  else
    *h = *w;
}


int get_bitmap_info(
  BitmapInfo __far *	bmi,
  char *		name,
  char *		buffer) {
  
  static char		map[512];
  static char		file[12];
  int			status;
  int			ii;
  FILE *		fp = NULL;
  char __near *		nbuffer = (char __near *)buffer;

  for (ii = 0; ii < 249; ii++) {
    gfx_get_paths(GFX_MAP_PATH, ii, map, file);
    if (map[0] == '\0')
      break;

    sprintf (nbuffer, "%s\\%s", map, name);
    if (fp = fopen(nbuffer, "r")) {
      fclose(fp);
      gfx_bitmap_info(nbuffer, bmi, status);
      
      if (status == 1)
	return 1;
    }
  }
  return 0;
}


BXPColor __far * invert_image (
  BXPColor __far *	image,
  int			w,
  int			h) {

  BXPColor __far *	new_image = (BXPColor __far *)
    malloc (sizeof (BXPColor) * w * h);
  int			ii;
  int			jj;

  put_my_fp_status();

  for (ii = 0; ii < h; ii++) {
    for (jj = 0; jj < w; jj++) {
      new_image[(h-1-ii)*w + jj] = image[ii*w + jj];
    }
  }
  free (image);
  
  return new_image;
}


int gammify(BXPColor __far * map, int w, int h)
{
  Color_48 __far *	himap;
  Color_48 __far *	hp;
  BXPColor __far *	m;
  int 			ii;
  int			kk;

  put_my_fp_status();

  if ((himap=malloc(w*sizeof(Color_48)))==NULL) {
    gfx_continu_line("No RAM for gammifying output image");
    return(0);
  }

  for (ii = 0; ii < h; ii++) {
    hp = himap;
    m = &map[ii*w];
    for(kk=0; kk < w; ++kk, ++m, ++hp)
      {
	hp->r=((unsigned int)m->r)<<8;
	hp->g=((unsigned int)m->g)<<8;
	hp->b=((unsigned int)m->b)<<8;
      }

    gfx_gamma(himap, w, 1.01, himap);
    
    hp = himap;
    m = &map[ii*w];
    for(kk=0; kk < w; ++kk, ++m, ++hp)
      {
	m->r=(unsigned char)(hp->r>>8);
	m->g=(unsigned char)(hp->g>>8);
	m->b=(unsigned char)(hp->b>>8);
      }
  }

  free (himap);
  return (1);
}


int load_bitmap_data(
  Texture *		texture) {
  
  static BitmapInfo	bmi;
  int			status;
  static BXPColor __far *	image = NULL;
  int			w;
  int			h;
  int			neww;
  int			newh;
  static BXPColor __far *	newimage;
  static char		buffer[2048];
  char *		image_pathname = buffer;
  
  put_my_fp_status();
  bmi.width = bmi.height = 0;
  get_bitmap_info(&bmi, texture->image.filename, image_pathname);
  neww = w = bmi.width;
  newh = h = bmi.height;
  if (w <= 0 || h <= 0 || w > 10240 || h > 10240) {
    error_printf (__FILE__, __LINE__,  "FAILED: load_bitmap(%s,%d,%d)\n",
		  buffer, w, h);
    return 0;
  }
  /*error_printf (__FILE__, __LINE__,  "load_bitmap(%s,%d,%d)\n",
		buffer, w, h);*/
  image = (BXPColor __far *) malloc(sizeof(BXPColor) * w * h);
  gfx_load_bitmap(image_pathname, w, h, image, status);

  if (status <= 0) {
    error_printf (__FILE__, __LINE__, "Could not load texture bitmap (%s)!\n",
		  buffer);
    free (image);
    return 0;
  }
  up_to_power_of_two(&neww, &newh);
    
  if (((float) neww / newh > 8.0)) {
    newh = neww / 8;
  }
  else if (((float) newh / neww > 8.0)) {
    neww = newh / 8;
  }
				     
  if (neww != w || newh != h) {
    newimage = (BXPColor __far *) malloc (sizeof(BXPColor) * neww * newh);
    status = resize_image (image, w, h, newimage, neww, newh);
    /*error_printf (__FILE__, __LINE__, "load_bitmap(%s) resize to (%d,%d)\n",
		  buffer, neww, newh);*/
    if (!status)
      error_printf (__FILE__, __LINE__, 
		    "Could not resize texture bitmap (%s)!\n",
		    buffer);
    free(image);
    image = newimage;
    w = neww;
    h = newh;
  }

  image = invert_image (image, w, h);
  texture->image.image = image;
  texture->image.width = w;
  texture->image.height = h;
  texture->image.mipmap_size = (w > h) ? w : h;
    
  put_glide_fp_status();
  texture->size = txInit3dfInfo (&texture->tex_info,
				 GR_TEXFMT_RGB_565,
				 &texture->image.width,
				 &texture->image.height,
				 -1, TX_AUTORESIZE_DISABLE);

  if ((texture->image.width != w) && (texture->image.height != h)) {
    error_printf (__FILE__, __LINE__, "txInit3dfInfo changed w & h!\n");
  }
  put_my_fp_status();
  return 1;
}


long get_current_tmu(int size) {
  static int		choose = 0;
  
  if (num_tmus != 2) {
    return GR_TMU0;
  }
  else {
    if (tmu_size[0] + size > max_texture_memory) {
      if (tmu_size[1] + size > max_texture_memory) {
	if (choose == 0) {
	  choose = 1;
	  return GR_TMU0;
	}
	else {
	  choose = 0;
	  return GR_TMU1;
	}
      }
      else {
	return GR_TMU1;
      }
    }
    else {
      return GR_TMU0;
    }
  }
}


int setup_3ds_texture(
  mapset3ds * 		texture3ds,
  Material * 		mat) {
  
  AtrMaterial *		amat = mat->material;
  bitmap3ds *		map = &texture3ds->map;
  static char 		buffer[1024];
  static char		name[9];
  int			ii;
  FILE *		fp = NULL;
  char *		filename = NULL;

  /*amat->texture_contribution = map->percent;*/

  for (ii = 0; ii < 8 && map->name[ii] != '.'; ii++) {
    name[ii] = map->name[ii];
  }
  name[ii] = '\0';

  sprintf(buffer, "%s\\3dfx\\%s.3df", home_3ds, name);
  fp = fopen (buffer, "r");
  if (fp)
    fclose (fp);

  mat->texture = (Texture *) malloc (sizeof(Texture));
  mat->texture->pathname = (char *) malloc (strlen(buffer) + 1);
  strcpy (mat->texture->pathname, buffer);
  mat->texture->image.filename = (char *) malloc (strlen (map->name) + 1);
  strcpy (mat->texture->image.filename, map->name);
  mat->texture->tex_info.data = NULL;
  mat->texture->tex_info.mem_required = 0;
  
  filename = mat->texture->pathname + strlen(mat->texture->pathname);
  while (*filename != '\\')
    filename--;

  mat->texture->filename = filename + 1;

  if (fp == NULL) {
    Texture *			texture = mat->texture;

    if (!load_bitmap_data (texture)) {
      free (texture->pathname);
      free (texture->image.filename);
      free (texture);
      mat->texture = NULL;
      amat->texture[0] = amat->texture[1] = NULL;

      return 0;
    }
    texture->tmu = get_current_tmu(texture->size);
    amat->texture[0] = amat->texture[1] = NULL;
  }
  else {
    mat->texture->image.image = NULL;
    
    put_glide_fp_status();
    mat->texture->aimage = atrImgAllocate (1);
  
    amat->texture[1] = NULL;
    
    if (!atrImgCreateFrom3df (mat->texture->aimage, mat->texture->pathname)) {
      error_printf (__FILE__, __LINE__,
		    "Could not create texture (%s.3df).\n", name);
    }

    gu3dfGetInfo (mat->texture->pathname, &mat->texture->tex_info);
    mat->texture->size = mat->texture->tex_info.mem_required;
    
    amat->texture[0] = atrTexNewHandle (
	mat->texture->tmu = get_current_tmu(mat->texture->size));
    if (amat->texture[0] == NULL)
      error_printf (__FILE__, __LINE__,
		    "TextureAllocate failed on %s!\n", name);
    atrTexAssociate (amat->texture[0], mat->texture->aimage);     
  }

  return 1;
  /* TODO: filter, decal, alpha, uscale, uoffset, rotation, ?tints? */
}

#ifndef USE_TEXUS
static char * texus_environment[] = {
  "DOS4G=quiet",
  NULL, /* PATH= will go here */
  NULL
};
#endif

#ifndef USE_TEXUS
#pragma off (unreferenced)
#endif
int write_3df (
  Gu3dfInfo *		tex_info,
  char *		filename,
  BXPColor __far *	image,
  int			w,
  int			h) {
  
#ifdef USE_TEXUS
  TxMip 		tmpmip;
  char			tmpchar;
  char *		file;
  int			original_pos;

  file = filename;
  tmpchar = file[original_pos = strlen(file)-4];
  file[original_pos] = '\0';

  //txConvert does this...
  //tex_info->data = (void *) malloc (tex_info->mem_required);

  put_glide_fp_status();
  txConvert (tex_info, GR_TEXFMT_RGB_888, w, h, image, TX_DITHER_NONE, NULL,
	     &tmpmip);

  txMipWrite (&tmpmip, file, ".3df", 0 /* dont split */);

  file[original_pos] = tmpchar;

  return 1;
#else
  static char		ppmfile[2048];
  static char		buffer[4096];
  int			len = strlen (filename);
  char *		path = NULL;
  static char		wstr[128];
  static char		hstr[128];

  strcpy (ppmfile, filename);
  /* replace .3df with .ppm */
  ppmfile[len-1] = 'm';
  ppmfile[len-2] = 'p';
  ppmfile[len-3] = 'p';

  write_ppm_file (ppmfile, image, w, h);
  path = getenv ("PATH");
  sprintf (buffer, "PATH=%s", path);
  texus_environment[1] = buffer;
  sprintf (wstr, "%d", w);
  sprintf (hstr, "%d", h);
  spawnlpe (P_WAIT, "texus.exe", "texus.exe", "-if", ppmfile, "-of", filename,
	    "-ocf", "rgb565", "-minw", "1", "-minh", "1", "-maxw", wstr,
	    "-maxh", hstr, NULL, texus_environment);
  
  txInit3dfInfo (tex_info,
		 GR_TEXFMT_RGB_565,
		 &w, &h,
		 -1, TX_AUTORESIZE_DISABLE);
  return 1;
#endif  
}
#ifndef USE_TEXUS
#pragma on (unreferenced)
#endif


void resize_texture (
  Material *		material,
  int			new_size) {
  
  AtrMaterial *		amat = material->material;
  Texture *		texture = material->texture;
  int			size = texture->size;
  Image *		image = &texture->image;
  int			w;
  int			h;
  int			mmsize;
  static BXPColor __far *	newimage = NULL;
  int			status;

  if (image->image == NULL) {
    load_bitmap_data (texture);
  }
  w = image->width;
  h = image->height;
  mmsize = image->mipmap_size;
  
  while (size > new_size) {
    if (w > h)
      w = w >> 1;
    else
      h = h >> 1;

    if (((float) w / h > 8.0)) {
      h = w / 8;
    }
    else if (((float) h / w > 8.0)) {
      w = h / 8;
    }
    
    mmsize = (w>h) ? w : h;

    put_glide_fp_status();
    {
      int		dummy_w = w;
      int		dummy_h = h;
      
      size = txInit3dfInfo (&texture->tex_info,
			    GR_TEXFMT_RGB_565,
			    &dummy_w,
			    &dummy_h,
			    -1, TX_AUTORESIZE_DISABLE);

      if (dummy_w != w || dummy_h != h) {
	error_printf (__FILE__, __LINE__, "txInit3dfInfo not honoring w&h!");
      }
    }

    if (size == 0) {
      error_printf (__FILE__, __LINE__, "txInit3dfInfo returned 0 size!\n"
		    "original:(%d,%d) new:(%d,%d)",
		    image->width, image->height, w, h);
    }
  }

  newimage = (BXPColor __far *) malloc (sizeof (BXPColor) * w * h);
  
  put_my_fp_status();
  status = resize_image (image->image, image->width, image->height,
			 newimage, w, h);

  if (!status) {
    error_printf (__FILE__, __LINE__, "Could not resize texture bitmap!\n");
    free (newimage);
    return;
  }

  /*  error_printf (__FILE__, __LINE__, "resize_texture(%s,%d,%d) --> (%d,%d)\n", texture->pathname,
	   image->width,
	   image->height,
	   w,
	   h); */
  
  if (write_3df (&texture->tex_info, texture->pathname, newimage, w, h)) {
    put_glide_fp_status();

    if (amat->texture[0] != NULL)
      atrTexDeleteHandle (amat->texture[0]);
    if (texture->aimage != NULL)
      atrImgDeallocate (texture->aimage);
    
    texture->aimage = atrImgAllocate (1);
    atrImgCreateFrom3df (texture->aimage, texture->pathname);
    amat->texture[0] = atrTexNewHandle (texture->tmu);
    if (amat->texture[0] == NULL)
      error_printf (__FILE__, __LINE__, "TextureAllocate failed on %s!\n",
		    texture->filename);
  
    atrTexAssociate (amat->texture[0], texture->aimage);
    // TODO:  WHAT HAPPENS IF THIS FAILS
  }
  put_my_fp_status ();
  free (newimage);
}


void load_texture (
  Material *		material) {
  
  AtrMaterial *		amat = material->material;
  Texture *		texture = material->texture;
  Image *		image = &texture->image;
  BXPColor __far *	image3ds = image->image;

  /*  error_printf (__FILE__, __LINE__, "load_texture(%s,%d,%d)\n",
      texture->pathname,
      image->width,
      image->height);	*/

  if (write_3df (&texture->tex_info, texture->pathname, image3ds,
		 image->width, image->height)) {
    put_glide_fp_status();

    texture->aimage = atrImgAllocate (1);
    atrImgCreateFrom3df (texture->aimage, texture->pathname);
    amat->texture[0] = atrTexNewHandle (texture->tmu);
    if (amat->texture[0] == NULL)
      error_printf (__FILE__, __LINE__, "atrTexNewHandle failed on %s!\n",
		    texture->filename);
    atrTexAssociate (amat->texture[0], texture->aimage);
    if (0) { // how to tell if the assoc fails?
      error_printf (__FILE__, __LINE__, "Could not create texture (%s).\n",
		    texture->filename);
      atrImgDeallocate (texture->aimage);
      atrTexDeleteHandle (amat->texture[0]);
      amat->texture[0] = NULL;
      put_my_fp_status();
      
      return;
    }
  
    texture->size = texture->tex_info.mem_required;

    put_my_fp_status();
  }
  else {
    error_printf (__FILE__, __LINE__,  "Could not write texture (%s)!\n",
		  texture->pathname);
  }
}
  

// This is where fancy texture allocation algorithms can be implemented.
// Right now there are no fancy algorithms.
void fixup_textures (
  Database_3Dfx *	db) {
  
  int			jj;
  int			ii;

  put_my_fp_status();
  gfx_show_gasgauge ("Converting Textures...Please Wait");
  for (ii = 0; ii < num_tmus; ii++) {
    if (tmu_size[ii] > max_texture_memory) {
      int			per_texture_size =
	max_texture_memory /(float) ntextures[ii] + 0.5;
    
      for (jj = 0; jj < ntextures[ii]; jj++) {
	Material *		material = db->textures[ii][jj];
	Texture *		texture = material->texture;

	if (texture->size > per_texture_size)
	  resize_texture (material, per_texture_size);
	else if (material->material->texture[0] == NULL)
	  load_texture (material);
	gfx_set_gasgauge ("", jj, ntextures[ii] - 1);
      }
    }
    else {
      for (jj = 0; jj < ntextures[ii]; jj++) {
	Material *		material = db->textures[ii][jj];

	if (material->material->texture[0] == NULL) {
	  load_texture (material);
	}
	gfx_set_gasgauge ("", jj, ntextures[ii] - 1);
      }
    }
  }
  gfx_put_hole ();
}


Material * find_material_by_name(
  char * 	matname,
  Material * 	mat,
  int		mat_count) {
  
  int		jj;
  
  if (matname != NULL) {
    for (jj = 0; jj < mat_count; jj++) {
      if (strcmp(matname, mat[jj].name) == 0) {
	return mat + jj;
      }
    }
  }
  error_printf (__FILE__, __LINE__, "Cannot find mat %s!\n", matname);
  return NULL;
}


void set_bounding_box (
  float			min[3],
  float			max[3],
  AtrVertex *		v) {

  put_my_fp_status();
  if (v->x < min[0])
    min[0] = v->x;
  else if (v->x > max[0])
    max[0] = v->x;

  if (v->y < min[1])
    min[1] = v->y;
  else if (v->y > max[1])
    max[1] = v->y;

  if (v->z < min[2])
    min[2] = v->z;
  else if (v->z > max[2])
    max[2] = v->z;
}


void make_atb_face (
  AtrTriSet *		amesh,
  AtrVertex *		v,
  int			a,
  int			b,
  int			c) {
  
  atrTriSetVertex(amesh, v + a);
  atrTriSetVertex(amesh, v + b);
  atrTriSetVertex(amesh, v + c);
}


void face_to_mesh(
  mesh3ds *		mesh3ds,		  
  objmat3ds *		fmat,
  AtrTriSet *		amesh,
  AtrVertex *		v,
  int			two_sided) {
  
  int			nfaces = fmat->nfaces;
  ushort3ds *		faceindex = fmat->faceindex;
  int			ii;
  face3ds *		faces3ds = mesh3ds->facearray;
  face3ds *		face3ds = faces3ds + faceindex[0];

  put_glide_fp_status();
  if (!two_sided) {
#ifdef DEBUG_OUTPUT
    fprintf (output, "atrTriSetBegin (meshes);\n", 0);
    fflush (output);
#endif    
    atrTriSetBegin(amesh);
    for (ii = 0; ii < nfaces; ii++) {
      face3ds = faces3ds + faceindex[ii];

      make_atb_face (amesh, v, face3ds->v3, face3ds->v2, face3ds->v1);
#ifdef DEBUG_OUTPUT
      fprintf (output, "atrTriSetVertex(meshes, v + %d);\n", face3ds->v3);
      fprintf (output, "atrTriSetVertex(meshes, v + %d);\n", face3ds->v2);
      fprintf (output, "atrTriSetVertex(meshes, v + %d);\n", face3ds->v1);
#endif      
    }
    atrTriSetCalcNormals();
    atrTriSetEnd (amesh);

#ifdef DEBUG_OUTPUT
    fprintf (output, "atrTriSetCalcNormals();\n");
    fprintf (output, "atrTriSetEnd (meshes);\n");
#endif      
  }
  else {
    atrTriSetBegin(amesh);
    for (ii = 0; ii < nfaces; ii++) {
      face3ds = faces3ds + faceindex[ii];
    
      make_atb_face (amesh, v, face3ds->v1, face3ds->v2, face3ds->v3);
    }
    atrTriSetCalcNormals();
    atrTriSetEnd (amesh);
  }
  put_my_fp_status();
}  


int read_db(
  Database_3Dfx * 	db3dfx,
  Name3ds *		selection_list,
  int			nselections) {
  
  int			screen_width = db3dfx->screen_width;
  int			screen_height = db3dfx->screen_height;
  file3ds *		file = NULL;
  database3ds *		db = NULL;
  int			mesh_count = 0;
  int			object_count = 0;
  int			mat_count = 0;
  int			cam_count = 0;
  int			olight_count = 0;
  int			slight_count = 0;
  int			light_count = 0;
  int			ii = 0;
  Object *		objects = NULL;
  AtrMaterial *		amaterials = NULL;
  Material *		materials = NULL;
  Material * *		textures[2] = { NULL, NULL };
  AtrCamera *		acameras = NULL;
  Camera *		cameras = NULL;
  Light *		lights = NULL;
  AtrLight *		alights = NULL;
  AtrLight *		aslights = NULL;
  static char		buffer[1024];
  AtrVertex *		v = NULL;

  if (db3dfx->num_tmus > 2)
    num_tmus = 2;
  else
    num_tmus = db3dfx->num_tmus;
  
  max_texture_memory = db3dfx->max_tmu_size * 1024 * 1024;
  tmu_size[0] = tmu_size[1] = 0;

  gfx_gamma_info (&gamma_data);

#ifdef DEBUG_OUTPUT
  output = fopen ("c:\\test.c", "wb");
  fprintf (output, "void main() {\natrInit (640, 480);\n");
#endif  

  put_my_fp_status();
  gfx_get_paths(GFX_HOME_PATH, 0, home_3ds, buffer);

  sprintf (buffer, "%s\\3dfx", home_3ds);
  
  sprintf (buffer, "%s\\3dfx\\temp.3ds", home_3ds);
  file = OpenFile3ds(buffer, "r");
  if (file == NULL)
    error_printf (__FILE__, __LINE__, "OpenFile3ds failed!\n");
  
  InitDatabase3ds(&db);
  CreateDatabase3ds(file, db);

  /* do ambient light--use IPAS for this */
  {
    int				count;
    int				status;
    static ItemData		tmp_item_data;
    ItemData *			item = &tmp_item_data;

    db3dfx->ambient.r = -1;
    
    pxp_get_item_count (count);
    
    for (ii = 0; ii < count; ii++) {
      pxp_get_item (ii, item, status);
      if (item->type == PXPAMBIENT) {
	db3dfx->ambient.r = item->item.a.color.r;
	db3dfx->ambient.g = item->item.a.color.g;
	db3dfx->ambient.b = item->item.a.color.b;
      }	
    }

    if (db3dfx->ambient.r == -1) { 
	db3dfx->ambient.r = 0;
	db3dfx->ambient.g = 0;
	db3dfx->ambient.b = 0;
    }     
  }

  db3dfx->nmaterials = mat_count = GetMaterialCount3ds(db);
  if (mat_count > 0) {
    put_glide_fp_status();
    amaterials = atrMaterialAllocate(mat_count);
    put_my_fp_status();
    if (amaterials == NULL)
      error_printf (__FILE__, __LINE__,
		    "atrMaterialAllocate(mat_count) failed!\n");
  
    db3dfx->materials = materials = (Material *)
      malloc (sizeof(Material) * mat_count);
    for (ii = 0; ii < 2; ii++) {
      db3dfx->textures[ii] = textures[ii] = (ii > num_tmus) ?
	NULL :	(Material * *) malloc (sizeof(Material *) * mat_count);
      ntextures[ii] = 0;
    }      
  }
  else {
    amaterials = NULL;
    db3dfx->materials = NULL;
  }

  gfx_show_gasgauge ("Setting up Materials...Please Wait");
  for (ii = 0; ii < mat_count; ii++) {
    AtrMaterial *		amat = amaterials + ii;
    material3ds * 		mat3ds = NULL;
    Material *			mat = db3dfx->materials + ii;
    
    gfx_set_gasgauge ("", ii, mat_count - 1);
    
    mat->material = amat;

    GetMaterialByIndex3ds(db, ii, &mat3ds);

    if (mat3ds->texture.map.percent > 0.0) {
      BXPColor			diffuse;
      float			tex_percent = mat3ds->texture.map.percent;

      diffuse.r = (1 - tex_percent) * mat3ds->diffuse.r + tex_percent * 1.0f;
      diffuse.g = (1 - tex_percent) * mat3ds->diffuse.g + tex_percent * 1.0f;
      diffuse.b = (1 - tex_percent) * mat3ds->diffuse.b + tex_percent * 1.0f;
      
      amat->diffuse.r = diffuse.r * (1 - mat3ds->selfillumpct);
      amat->diffuse.g = diffuse.g * (1 - mat3ds->selfillumpct);
      amat->diffuse.b = diffuse.b * (1 - mat3ds->selfillumpct);

      amat->emissive.r = diffuse.r * mat3ds->selfillumpct;
      amat->emissive.g = diffuse.g * mat3ds->selfillumpct;
      amat->emissive.b = diffuse.b * mat3ds->selfillumpct;    

      amat->specular.r = mat3ds->specular.r;
      amat->specular.g = mat3ds->specular.g;
      amat->specular.b = mat3ds->specular.b;

      put_glide_fp_status();

      atrMaterialSetup (amat, ATR_MAT_DECAL_X_LIGHTING);

      amat->texMMMode[0] =  (db3dfx->texture_mode == MIPMAP_NONE) ?
	ATR_TEXMIPMAP_DISABLE : ATR_TEXMIPMAP_NEAREST;
      
      if (mat3ds->texture.map.tiling == Tile ||
	  mat3ds->texture.map.tiling == Both)
	amat->texSClamp[0] = amat->texTClamp[0] = ATR_TEXCLAMP_WRAP;
      else
	amat->texSClamp[0] = amat->texTClamp[0] = ATR_TEXCLAMP_CLAMP;
      
      put_my_fp_status();
      if (!setup_3ds_texture(&mat3ds->texture, mat)) {
	put_glide_fp_status();
	atrMaterialSetup (amat, ATR_MAT_GSHADE);
	put_my_fp_status();
	amat->texture[0] = amat->texture[1] = NULL;
	mat->texture = NULL;
      }
      else {
	if (mat->texture->tmu == GR_TMU1) {
	  textures[1][ntextures[1]++] = mat;
	  tmu_size[1] += mat->texture->size;
	}
	else {
	  textures[0][ntextures[0]++] = mat;
	  tmu_size[0] += mat->texture->size;
	}
      }
    }
    else {
      amat->diffuse.r = mat3ds->diffuse.r * (1 - mat3ds->selfillumpct);
      amat->diffuse.g = mat3ds->diffuse.g * (1 - mat3ds->selfillumpct);
      amat->diffuse.b = mat3ds->diffuse.b * (1 - mat3ds->selfillumpct);

      amat->emissive.r = mat3ds->diffuse.r * mat3ds->selfillumpct;
      amat->emissive.g = mat3ds->diffuse.g * mat3ds->selfillumpct;
      amat->emissive.b = mat3ds->diffuse.b * mat3ds->selfillumpct;
    
      amat->specular.r = mat3ds->specular.r;
      amat->specular.g = mat3ds->specular.g;
      amat->specular.b = mat3ds->specular.b;

      if (mat3ds->shading != Flat) {
	put_glide_fp_status();
	atrMaterialSetup (amat, ATR_MAT_GSHADE);
	put_my_fp_status();
	amat->texture[0] = amat->texture[1] = NULL;
	mat->texture = NULL;
      }
      else {
	put_glide_fp_status();
	atrMaterialSetup (amat, ATR_MAT_GSHADE);
	put_my_fp_status();
	/*amat->texture_contribution = 0.0;*/
	amat->texture[0] = amat->texture[1] = NULL;
	mat->texture = NULL;
      }

#ifdef DEBUG_OUTPUT
      fprintf (output, "amat = atrMaterialAllocate (1);\n");
      fprintf (output, "amat->diffuse.r = %f;\n",
	       mat3ds->diffuse.r * (1 - mat3ds->selfillumpct));
      fprintf (output, "amat->diffuse.g = %f;\n",
	       mat3ds->diffuse.g * (1 - mat3ds->selfillumpct));
      fprintf (output, "amat->diffuse.b = %f;\n",
	       mat3ds->diffuse.b * (1 - mat3ds->selfillumpct));

      fprintf (output, "amat->emissive.r = %f;\n",
	       mat3ds->diffuse.r * mat3ds->selfillumpct);
      fprintf (output, "amat->emissive.g = %f;\n",
	       mat3ds->diffuse.g * mat3ds->selfillumpct);
      fprintf (output, "amat->emissive.b = %f;\n",
	       mat3ds->diffuse.b * mat3ds->selfillumpct);
    
      fprintf (output, "amat->specular.r = %f;\n", mat3ds->specular.r);
      fprintf (output, "amat->specular.g = %f;\n", mat3ds->specular.g);
      fprintf (output, "amat->specular.b = %f;\n", mat3ds->specular.b);

      if (mat3ds->shading != Flat) {
	fprintf (output, "atrMaterialSetup (amat, ATR_MAT_GSHADE);\n");
	fprintf (output, "amat->texture[0] = amat->texture[1] = NULL;\n");
	fprintf (output, "mat->texture = NULL;\n");
      }
      else {
	fprintf (output, "atrMaterialSetup (amat, ATR_MAT_GSHADE);\n");
	fprintf (output, "amat->texture[0] = amat->texture[1] = NULL;\n");
	fprintf (output, "mat->texture = NULL;\n");
      }
#endif      
    }
    
    /*amat->specular_contribution = mat3ds->shinstrength;*/
    amat->specExponent = 50.0 * mat3ds->shininess;

    /*amat->transparency = mat3ds->transparency;*/
    amat->isTwoSided = 0;
    mat->two_sided = mat3ds->twosided;

    if (getenv ("DEBUG_NO_TWO_SIDED")) {
      mat->two_sided = 0;
      amat->isTwoSided = 0;
    }

    if (getenv ("DEBUG_MATERIAL")) {
      atrMaterialPrint (amat, stdout, 4);
    }

#ifdef DEBUG_OUTPUT
    fprintf (output, "amat->specExponent = %f;\n",
	     50.0 * mat3ds->shininess);

    fprintf (output, "amat->isTwoSided = 0;\n");
    fprintf (output, "\n");
#endif    

    strcpy(mat->name, mat3ds->name);

    ReleaseMaterial3ds(&mat3ds);
  }
  gfx_put_hole ();

  db3dfx->ntextures[0] = ntextures[0];
  db3dfx->ntextures[1] = ntextures[1];
  
  for (ii = 0; ii < num_tmus; ii++) {
    if (ntextures[ii] == 0) {
      free (db3dfx->textures[ii]);
      db3dfx->textures[ii] = NULL;
    }
  }
  
  if (ntextures[0] != 0 || ntextures[1] != 0)
    fixup_textures (db3dfx);

  db3dfx->nlights = light_count = (olight_count = GetOmnilightCount3ds(db)) +
    (slight_count = GetSpotlightCount3ds(db));

  if (light_count > 0) {
    put_glide_fp_status();
    db3dfx->lights = lights = (Light *) malloc (sizeof(Light) * light_count);

    if (light_count > 0) {
      alights = atrLightAllocate(light_count);
      if (alights == NULL)
	error_printf (__FILE__, __LINE__,
		      "atrLightAllocate(light_count) failed!\n");
    }
    else
      alights = NULL;
    
    if (slight_count > 0) {
      aslights = atrLightAllocate (slight_count);
      if (alights == NULL)
	error_printf (__FILE__, __LINE__,
		      "atrLightAllocate(slight_count) failed!\n");
    }
    else
      aslights = NULL;
    
    put_my_fp_status();
  }
  else {
    db3dfx->lights = lights = NULL;
    alights = NULL;
  }

  for (ii = 0; ii < olight_count; ii++) {
    light3ds *			l3ds = NULL;
    Light *			light = lights + ii;
    AtrLight *			alight = alights + ii;
    AtmVector3			position;
    AtmVector3			target;

    GetOmnilightByIndex3ds(db, ii, &l3ds);
    alight->color.r = l3ds->color.r;
    alight->color.g = l3ds->color.g;
    alight->color.b = l3ds->color.b;
    alight->flags = ATR_LIGHT_POSITIONAL;
    alight->attenuation[ATR_LIGHT_QUADRATIC] = 0.0f;
    alight->attenuation[ATR_LIGHT_LINEAR]    = 0.0f;
    alight->attenuation[ATR_LIGHT_CONSTANT]  = 0.0f;

    /* use -position here because set_lcs_to_wcs uses rot about
     * vector from origin?
     */
    atmVector3Set(position,
		  RHCSFIX_X * l3ds->pos.x,
		  RHCSFIX_Y * l3ds->pos.y,
		  RHCSFIX_Z * l3ds->pos.z);
    atmVector3Set(target, RHCSFIX_X * 0.0, RHCSFIX_Y * 0.0, RHCSFIX_Z * 0.0);
    set_lcs_to_wcs (&alight->lcsToWCS, position, target);

    /* todo: multiplier, attenuation, exclude */
    light->light = alight;
    light->slight = NULL;
    light->off = l3ds->dloff;
    strcpy(light->name, l3ds->name);

    ReleaseLight3ds (&l3ds);
  }

  for (ii = 0; ii < slight_count; ii++) {
    light3ds *			l3ds = NULL;
    Light *			light = lights + olight_count + ii;
    AtrLight *			alight = alights + olight_count + ii;
    AtrLight *			aslight = aslights + ii;
    AtmVector3			position;
    AtmVector3			target;
    
    GetSpotlightByIndex3ds(db, ii, &l3ds);
    alight->color.r = l3ds->color.r;
    alight->color.g = l3ds->color.g;
    alight->color.b = l3ds->color.b;
    alight->flags = ATR_LIGHT_POSITIONAL;
    alight->attenuation[ATR_LIGHT_QUADRATIC] = 0.0f;
    alight->attenuation[ATR_LIGHT_LINEAR]    = 0.0f;
    alight->attenuation[ATR_LIGHT_CONSTANT]  = 0.0f;

    aslight->color.r = l3ds->color.r * 0.3;
    aslight->color.g = l3ds->color.g * 0.3;
    aslight->color.b = l3ds->color.b * 0.3;
    aslight->flags = ATR_LIGHT_DIRECTED;
    aslight->attenuation[ATR_LIGHT_QUADRATIC] = 0.0f;
    aslight->attenuation[ATR_LIGHT_LINEAR]    = 0.0f;
    aslight->attenuation[ATR_LIGHT_CONSTANT]  = 0.0f;

    /* ATB does not support this yet
    alight->flags = ATR_LIGHT_SPOT;

    / * todo: multiplier, attenuation, exclude, shadows, cone, projector * /
    alight->spot_exponent = l3ds->spot->hotspot;
    alight->spot_cutoff = l3ds->spot->falloff;
    alight->projected_focus_angle = l3ds->spot->falloff;
    alight->projected_texture = NULL;
    alight->pureAmbient = 0;
    alight->distantObserver = 0;
    */

    /* position does not seem right, almost as if we need to negate it...so*/
    atmVector3Set(position,
		  RHCSFIX_X * l3ds->pos.x,
		  RHCSFIX_Y * l3ds->pos.y,
		  RHCSFIX_Z * l3ds->pos.z);
    atmVector3Set(target,
		  RHCSFIX_X * l3ds->spot->target.x,
		  RHCSFIX_Y * l3ds->spot->target.y,
		  RHCSFIX_Z * l3ds->spot->target.z);
    set_lcs_to_wcs (&alight->lcsToWCS, position, target);
    set_lcs_to_wcs (&aslight->lcsToWCS, position, target);

    light->light = alight;
    light->slight = NULL;
    light->off = l3ds->dloff;
    strcpy(light->name, l3ds->name);

    ReleaseLight3ds (&l3ds);
  }

  db3dfx->ncameras = cam_count = GetCameraCount3ds(db);

  if (cam_count > 0) {
    db3dfx->cameras = cameras = (Camera *) malloc (sizeof(Camera) * cam_count);
    put_glide_fp_status();
    acameras = atrCameraAllocate(cam_count);
    put_my_fp_status();
    if (acameras == NULL)
      error_printf (__FILE__, __LINE__,
		    "atrCameraAllocate(camera_count) failed!\n");
  }
  else {
    db3dfx->cameras = cameras = NULL;
    acameras = NULL;
  }
  
  for (ii = 0; ii < cam_count; ii++) {
    camera3ds *			cam3ds = NULL;
    Camera *			cam = cameras + ii;
    AtrCamera *			acam = acameras + ii;
    
    GetCameraByIndex3ds(db, ii, &cam3ds);

    acam->aspectRatio = ((float)screen_width)/screen_height;
    acam->fovRadians = D2RADIANS * cam3ds->fov;
    acam->nearClip = cam3ds->ranges.cam_near;
    acam->farClip = cam3ds->ranges.cam_far;
    
    atmVector3Set(cam->target,
		  cam3ds->target.x,
		  cam3ds->target.y,
		  cam3ds->target.z);
    atmVector3Set(cam->position,
		  cam3ds->position.x,
		  cam3ds->position.y,
		  cam3ds->position.z);
    set_camera_parameters (cam, cam->target, cam->position, cam3ds->roll);
    
    cam->camera = acam;
    strcpy(cam->name, cam3ds->name);

    ReleaseCamera3ds(&cam3ds);
  }

  max_vertices = VERTEX_MAX;
  put_glide_fp_status();
  v = atrVertexAllocate (VERTEX_MAX);
  put_my_fp_status();
  
  mesh_count = GetMeshCount3ds(db);

  if (mesh_count > 0) {
    if (selection_list != NULL) {
      db3dfx->nobjects = object_count = nselections;
      db3dfx->objects = objects = (Object *)
	malloc (sizeof(Object) * nselections);
    }
    else {
      db3dfx->nobjects = mesh_count;
      db3dfx->objects = objects = (Object *)
	malloc (sizeof(Object) * mesh_count);
    }
  }
  else {
    db3dfx->objects = objects = NULL;
  }

  gfx_show_gasgauge ("Converting Meshes...Please wait");

  for (ii = 0; ii < mesh_count; ii++) {
    mesh3ds *		mesh = NULL;
    Object *		o = NULL;
    int			jj;
    int			nmats;
    objmat3ds *		fmats;

    gfx_set_gasgauge ("", ii, mesh_count - 1);
    GetMeshByIndex3ds(db, ii, &mesh);

    if (selection_list != NULL) {
      int		matches = 0;
      
      for (jj = 0; jj < nselections; jj++) {
	if (strcmp (selection_list[jj], mesh->name) == 0) {
	  o = objects + --object_count;
	  matches = 1;
	  break;
	}
      }
      if (!matches)
	continue; /* go to the next mesh */
    }
    else {
      o = objects + ii;
    }

    strcpy (o->name, mesh->name);

    o->ipas_index = -1;

    if (!matrix_is_identity (mesh->locmatrix)) {
      if (getenv ("DEBUG_LOCMATRIX")) {
	error_printf (__FILE__, __LINE__,
		    "%s has non identity locmatrix\n", o->name);
      }

      o->locmatrix = atrXformAllocate (1);
      xform_to_axform (mesh->locmatrix, o->locmatrix);
      atrXformInvert (o->locmatrix, o->locmatrix);
    }
    else {
      o->locmatrix = NULL;
    }

    if (mesh->nvertices > max_vertices) {
      max_vertices = mesh->nvertices + 100;
      put_glide_fp_status();
      atrVertexDeallocate (v);
      v = atrVertexAllocate (max_vertices);
      put_my_fp_status();
    }

    nmats = mesh->nmats;
    fmats = mesh->matarray;
    o->material = NULL;
    o->xform = NULL;

    o->min[0] = o->max[0] =
      RHCSFIX_X * mesh->vertexarray[mesh->facearray->v1].x;
    o->min[1] = o->max[1] =
      RHCSFIX_Y * mesh->vertexarray[mesh->facearray->v1].y;
    o->min[2] = o->max[2] =
      RHCSFIX_Z * mesh->vertexarray[mesh->facearray->v1].z;

#ifdef DEBUG_OUTPUT
    fprintf (output, "v = atrVertexAllocate (%d);\n", mesh->nvertices);
#endif    

    for (jj = 0; jj < mesh->nvertices; jj++) {
      AtrVertex *			ov = v + jj;

      ov->x = RHCSFIX_X * mesh->vertexarray[jj].x;
      ov->y = RHCSFIX_Y * mesh->vertexarray[jj].y;
      ov->z = RHCSFIX_Z * mesh->vertexarray[jj].z;
      ov->s0 = mesh->textarray[jj].u;
      ov->t0 = mesh->textarray[jj].v;   
      set_bounding_box (o->min, o->max, ov);

#ifdef DEBUG_OUTPUT
      fprintf (output, "v[%d].x = %f; v[%d].y = %f; v[%d].z = %f;\n",
	       jj, RHCSFIX_X * mesh->vertexarray[jj].x,
	       jj, RHCSFIX_Y * mesh->vertexarray[jj].y,
	       jj, RHCSFIX_Z * mesh->vertexarray[jj].z);
      fprintf (output, "v[%d].s0 = %f; v[%d].t0 = %f;\n",
	       jj, mesh->textarray[jj].u,
	       jj, mesh->textarray[jj].v);
#endif      
    }
	
    if (nmats > 1) {
      int			kk = 0;
      
      o->nmeshes = nmats;
      o->material = (Material * *) malloc(sizeof(Material *) * nmats * 2);
      for (jj = 0; jj < nmats; jj++) {
	o->material[kk] = find_material_by_name(fmats[jj].name, materials,
						mat_count);
	if (o->material[kk] && o->material[kk]->two_sided) {
	  o->nmeshes++;
	  kk++;
	  o->material[kk] = o->material[kk-1];
	}
	kk++;
      }

      put_glide_fp_status();
      o->mesh = atrTriSetAllocate(o->nmeshes);
#ifdef DEBUG_OUTPUT
      fprintf (output, "meshes = atrTriSetAllocate (%d);\n",
	       o->nmeshes);
#endif      
      put_my_fp_status();

      if (o->mesh == NULL)
	error_printf (__FILE__, __LINE__,
		      "atrTriSetAllocate(nmats) failed!\n");

      for (jj = 0, kk = 0; kk < nmats; kk++, jj++) {
	face_to_mesh (mesh, fmats + kk, o->mesh + jj, v, 0);
	
	if (o->material[jj] && o->material[jj]->two_sided) {
	  jj++;
	  face_to_mesh (mesh, fmats + kk, o->mesh + jj, v, 1);
	}
      }
    } else {
      AtrTriSet *		m = NULL;
      
      put_glide_fp_status();
      put_my_fp_status();

      o->material = (Material * *) malloc(2 * sizeof (Material *));
      if (nmats == 1) {
	o->material[0] = o->material[1] =
	  find_material_by_name(mesh->matarray[0].name, materials, mat_count);
      }
      else {
	o->material[0] = o->material[1] = NULL;
      }

      put_glide_fp_status();
      if (o->material[0] == NULL || !o->material[0]->two_sided) {
	o->nmeshes = 1;
	m = o->mesh = atrTriSetAllocate(1);
	if (o->mesh == NULL)
	  error_printf (__FILE__, __LINE__,
			"atrTriSetAllocate(1) failed!\n");

	atrTriSetBegin (m);
	for (jj = 0; jj < mesh->nfaces; jj++) {
	  face3ds *		studio_face = mesh->facearray + jj;
      
	  make_atb_face (m, v,
			 studio_face->v3, studio_face->v2, studio_face->v1);
	}
	atrTriSetCalcNormals ();
	atrTriSetEnd (m);
      }
      else {
	o->nmeshes = 2;
	m = o->mesh = atrTriSetAllocate(2);
	if (o->mesh == NULL)
	  error_printf (__FILE__, __LINE__,
			"atrTriSetAllocate(2) failed!\n");

	atrTriSetBegin (m);
	for (jj = 0; jj < mesh->nfaces; jj++) {
	  face3ds *		studio_face = mesh->facearray + jj;
      
	  make_atb_face (m, v,
			 studio_face->v3, studio_face->v2, studio_face->v1);
	}
	atrTriSetCalcNormals ();
	atrTriSetEnd (m);
	  
	atrTriSetBegin (m+1);
	for (jj = 0; jj < mesh->nfaces; jj++) {
	  face3ds *		studio_face = mesh->facearray + jj;
      
	  make_atb_face (m + 1, v,
			 studio_face->v1, studio_face->v2, studio_face->v3);
	}
	atrTriSetCalcNormals ();
	atrTriSetEnd (m+1);
      }
      
      if (getenv ("DEBUG_VERTICES")) {
	for (jj = 0; jj < mesh->nfaces; jj++) {
	  face3ds *		studio_face = mesh->facearray + jj;
	
	  atrVertexPrint (v + studio_face->v3, stdout, 4);
	  atrVertexPrint (v + studio_face->v2, stdout, 4);
	  atrVertexPrint (v + studio_face->v1, stdout, 4);
	}
      }
      put_my_fp_status();
    }
    RelMeshObj3ds(&mesh);
  }
  gfx_put_hole ();

  ReleaseDatabase3ds (&db);
  CloseAllFiles3ds ();
  
  put_glide_fp_status();
  atrVertexDeallocate (v);
  put_my_fp_status();

  return 1;
}
