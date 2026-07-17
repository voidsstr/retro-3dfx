/* util.c */

#include "pxp.h"

#include "database.h"

#include <string.h>
#include <stdarg.h>
#include <math.h>

short		my_fp_status;
short		glide_fp_status;

int write_ppm_file(
  char *		filename,
  BXPColor __far *	incoming_image,
  int			w,
  int			h) {
  
  FILE *		ppm;
  
  ppm = fopen(filename, "wb");
  if (!ppm)
    return 0;

  fprintf (ppm, "P6\n%d %d\n%d\n", w, h, 255);

  /* error_printf (__FILE__, __LINE__,  "write_ppm(%s,%d,%d)\n",
		filename, w, h); */

  fwrite ((void __near *)incoming_image, 3, w * h, ppm);
  
  fclose(ppm);
  return 1;
}


void error_printf(
  char *		file,
  int			line,
  char *		format, ...) {

  va_list		args;

  va_start (args, format);

  fprintf (stdout, "%s(%d): ", file, line);
  vfprintf (stdout, format, args);
}


void atmVectorMatrix4x4Mult (
  AtmVector3		dest,
  AtmVector3		src,
  AtmMatrix4x4		m) {
  AtmVector3		tmp;

  put_my_fp_status();

  tmp[0] = m[0]*src[0] + m[4]*src[1] + m[8]*src[2] + m[12];
  tmp[1] = m[1]*src[0] + m[5]*src[1] + m[9]*src[2] + m[13];
  tmp[2] = m[2]*src[0] + m[6]*src[1] + m[10]*src[2] + m[14];
  dest[0] = tmp[0]; dest[1] = tmp[1]; dest[2] = tmp[2];
}


Object * find_object_by_name(
  Database_3Dfx *	db,
  char *		name) {
  int			ii;

  for (ii = 0; ii < db->nobjects; ii++) {
    Object *		o = db->objects + ii;
    
    if (strcmp(name, o->name) == 0)
      return o;
  }
  return NULL;
}

extern int		malloc_calls = 0;

void __far * my_realloc (void __far * ptr, size_t new_size,
			 char * file, int line) {

  void __far *		ret_ptr;
  
  ret_ptr = realloc (ptr, new_size);
  return ret_ptr;
}


void __far * my_malloc (size_t size, char * file, int line) {
  void __far *		ptr;

  malloc_calls++;
  
  ptr = malloc (size);
  if (ptr == NULL)
    fprintf (stdout, "%s(%d):  malloc(%d) failed!\n", file, line, size);
  return ptr;
}


void my_free(void __far * ptr, char * file, int line) {
  malloc_calls--;

  if (ptr != NULL)
    free ((void __near *) ptr);
  else
    fprintf (stdout, "%s(%d):  WARNING: free got 0!\n", file, line);    
}


#include "mymalloc.h"


Database_3Dfx * alloc_db(void) {
  return (Database_3Dfx *) malloc (sizeof(Database_3Dfx));
}


void free_db(Database_3Dfx * db) {
  int			ii;

  if (db->ntextures > 0)
    free (db->textures);

  if (db->nmaterials > 0) {
    for (ii = 0; ii < db->nmaterials; ii++) {
      Material *			mat = db->materials + ii;
      
      if (mat->material->texture[0]) {
	atrTexDeleteHandle (mat->material->texture[0]);
      }
      
      if (mat->texture) {
	Texture *			texture = mat->texture;
	
	free (texture->pathname);
	free (texture->tex_info.data);
	if (texture->image.image != NULL)
	  free (texture->image.image);
	if (texture->image.filename != NULL)
	  free (texture->image.filename);
	free (texture);
      }
    }
    atrMaterialDeallocate(db->materials[0].material);
    free (db->materials);
  }

  if (db->nlights > 0) {
    atrLightDeallocate(db->lights[0].light);
    free (db->lights);
  }

  if (db->ncameras > 0) {
    atrCameraDeallocate(db->cameras[0].camera);
    free (db->cameras);
  }

  if (db->nobjects > 0) {
    for (ii = 0; ii < db->nobjects; ii++) {
      Object *		o = db->objects + ii;

      atrTriSetDeallocate (o->mesh);
      free (o->material);
    }
    free (db->objects);
  } 

  free(db);
}


void TransformSetRotationAboutAxis(
  AtrXform *		xform,
  AtmVector3		axis,
  float			angle) {
  double		ar;
  float			cosine;
  float			sine;
  float			t;
  float			x;
  float			y;
  float			z;
  float			a;
  float			b;
  float			g;
  float			d;
  float			e;
  float			f;
  float *		m;

  put_my_fp_status();
  
  m = (xform)->data;
  
  ar = D2RADIANS * (angle);  
  sine = sin (ar);
  cosine = cos (ar);
  t = 1 - cosine;
  x = (axis)[0];
  y = (axis)[1];
  z = (axis)[2];
  a = t * x;
  b = t * y;
  g = t * z;
  d = sine * x;
  e = sine * y;
  f = sine * z;
  m[0] = a*x + cosine; m[1] = a*y + f; m[2] = a*z - e; m[3] = 0;
  m[4] = b*x - f; m[5] = b*y + cosine; m[6] = b*z + d; m[7] = 0;
  m[8] = g*x + e; m[9] = g*y - d; m[10]= g*z + cosine; m[11]= 0;
  m[12]= 0;       m[13]= 0;       m[14]= 0;       m[15]= 1;
}
  

void xform_to_axform(
  float *			xform,
  AtrXform *			axform) {

  int				ii, jj;

  for (jj = 0; jj < 4; jj++) {
    for (ii = 0; ii < 3; ii++) {
      axform->data[ii + 4*jj] = xform[ii + 3*jj];
    }
    axform->data[3 + 4*jj] = (jj == 3) ? 1 : 0;
  }
}


