/*
 *		Filtered Image Rescaling
 *
 *		  by Dale Schumacher
 */

/* modified by Tanvir Hassan for use in my IPAS routine */

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <math.h>

#include "pxp.h"

#include "database.h"

#include "mymalloc.h"

typedef struct {
  int			xsize;	/* horizontal size of the image in Pixels */
  int			ysize;	/* vertical size of the image in Pixels */
  BXPColor __far *	data;	/* pointer to first scanline of image */
  int			span;	/* byte offset between two scanlines */
} Bitmap;

#define	WHITE_PIXEL	(255)
#define	BLACK_PIXEL	(0)
#define PI		(3.1415962)

#define CLAMP(n,min,max)	((n)>=(max) ? (max) : ((n)<(min) ? min : n))


static int		success = 1;


Bitmap * new_image(
  int			xsize,
  int			ysize) {
  
  Bitmap *		image;

  if((image = (Bitmap *)malloc(sizeof(Bitmap)))
     && (image->data = (BXPColor *) calloc (ysize, xsize * 3))) {
    image->xsize = xsize;
    image->ysize = ysize;
    image->span = xsize * 3;
    memset ((char __near *)image->data, 0x7F, xsize * ysize * 3);
  }
  else {
    success = 0;
  }

  return(image);
}


Bitmap * new_image_with_data(
  int			xsize,
  int			ysize,
  BXPColor __far *	data) {

  Bitmap *		image;

  image = (Bitmap *) malloc (sizeof(Bitmap));
  image->data = data;
  image->xsize = xsize;
  image->ysize = ysize;
  image->span = xsize * 3;

  return image;
}

void free_image_with_data (
  Bitmap *		image) {

  free (image);
}


void free_image(
  Bitmap *		image) {
  
  free ((char __near *)image->data);
  free (image);
}


BXPColor * get_pixel(
  Bitmap *		image,
  int 			x,
  int			y) {
  
  static Bitmap *	im = NULL;
  static int 		yy = -1;
  static BXPColor *	p = NULL;

  if((x < 0) || (x >= image->xsize) || (y < 0) || (y >= image->ysize)) {
    success = 0;
    return NULL;
  }
  
  if((im != image) || (yy != y)) {
    im = image;
    yy = y;
    p = (BXPColor *) (((char *)image->data) + (y * image->span));
  }
  
  return &p[x];
}


void get_row(
  BXPColor *		row,
  Bitmap *		image,
  int			y) {
  
  if((y < 0) || (y >= image->ysize)) {
    success = 0;
    return;
  }
  
  memcpy(row,
	 ((char *)image->data) + (y * image->span),
	 (sizeof(BXPColor) * image->xsize));
}


void get_column(
  BXPColor *		column,
  Bitmap *		image,
  int			x) {
  
  int 			i;
  int			d;
  BXPColor __far *	p;

  if((x < 0) || (x >= image->xsize)) {
    success = 0;
    return;
  }
  
  d = image->span;
  for(i = image->ysize, p = image->data + x; i-- > 0;
      p = (BXPColor __far *)(((char *)p)+d)) {
    *column++ = *p;
  }
}


void put_pixel(
  Bitmap *		image,
  int 			x,
  int			y,
  BXPColor *		data) {
  static Bitmap *	im = NULL;
  static int 		yy = -1;
  static BXPColor *	p = NULL;


  if((x < 0) || (x >= image->xsize) || (y < 0) || (y >= image->ysize)) {
    success = 0;
    return;
  }
  if((im != image) || (yy != y)) {
    im = image;
    yy = y;
    p = (BXPColor *) (((char *)image->data) + (y * image->span));
  }
  p[x] = *data;
}


/*
 *	filter function definitions
 */

#define	filter_support		(1.0)

double
filter(t)
     double t;
{
  /* f(t) = 2|t|^3 - 3|t|^2 + 1, -1 <= t <= 1 */
  if(t < 0.0) t = -t;
  if(t < 1.0) return((2.0 * t - 3.0) * t * t + 1.0);
  return(0.0);
}

#define	box_support		(0.5)

double
box_filter(t)
     double t;
{
  if((t > -0.5) && (t <= 0.5)) return(1.0);
  return(0.0);
}

#define	triangle_support	(1.0)

double
triangle_filter(t)
     double t;
{
  if(t < 0.0) t = -t;
  if(t < 1.0) return(1.0 - t);
  return(0.0);
}

#define	bell_support		(1.5)

double
bell_filter(t)		/* box (*) box (*) box */
     double t;
{
  if(t < 0) t = -t;
  if(t < .5) return(.75 - (t * t));
  if(t < 1.5) {
    t = (t - 1.5);
    return(.5 * (t * t));
  }
  return(0.0);
}

#define	B_spline_support	(2.0)

double
B_spline_filter(t)	/* box (*) box (*) box (*) box */
     double t;
{
  double tt;

  if(t < 0) t = -t;
  if(t < 1) {
    tt = t * t;
    return((.5 * tt * t) - tt + (2.0 / 3.0));
  } else if(t < 2) {
    t = 2 - t;
    return((1.0 / 6.0) * (t * t * t));
  }
  return(0.0);
}

double
sinc(x)
     double x;
{
  x *= PI;
  if(x != 0) return(sin(x) / x);
  return(1.0);
}

#define	Lanczos3_support	(3.0)

double
Lanczos3_filter(t)
     double t;
{
  if(t < 0) t = -t;
  if(t < 3.0) return(sinc(t) * sinc(t/3.0));
  return(0.0);
}

#define	Mitchell_support	(2.0)

#define	B	(1.0 / 3.0)
#define	C	(1.0 / 3.0)

double
Mitchell_filter(t)
     double t;
{
  double tt;

  tt = t * t;
  if(t < 0) t = -t;
  if(t < 1.0) {
    t = (((12.0 - 9.0 * B - 6.0 * C) * (t * tt))
	 + ((-18.0 + 12.0 * B + 6.0 * C) * tt)
	 + (6.0 - 2 * B));
    return(t / 6.0);
  } else if(t < 2.0) {
    t = (((-1.0 * B - 6.0 * C) * (t * tt))
	 + ((6.0 * B + 30.0 * C) * tt)
	 + ((-12.0 * B - 48.0 * C) * t)
	 + (8.0 * B + 24 * C));
    return(t / 6.0);
  }
  return(0.0);
}

/*
 *	image rescaling routine
 */

typedef struct {
  int		pixel;
  double	weight;
} CONTRIB;


typedef struct {
  int		n;		/* number of contributors */
  CONTRIB	*p;		/* pointer to list of contributions */
} CLIST;


static CLIST	*	contrib;	/* array of contribution lists */


void zoom(
  Bitmap *		dst,		/* destination image structure */
  Bitmap *		src,		/* source image structure */
  double 		(*filterf)(),	/* filter function */
  double 		fwidth) {	/* filter width (support) */

  Bitmap *		tmp;		/* intermediate image */
  double 		xscale, yscale;	/* zoom scale factors */
  int 			i, j, k;	/* loop variables */
  int 			n;		/* pixel number */
  double 		center, left, right;
  double 		width, fscale, weight;
  BXPColor *		raster;	/* a row or column of pixels */

  /* create intermediate image to hold horizontal zoom */
  tmp = new_image (dst->xsize, src->ysize);
  xscale = (double) dst->xsize / (double) src->xsize;
  yscale = (double) dst->ysize / (double) src->ysize;

  /* pre-calculate filter contributions for a row */
  contrib = (CLIST *)calloc(dst->xsize, sizeof(CLIST));
  if(xscale < 1.0) {
    width = fwidth / xscale;
    fscale = 1.0 / xscale;
    for(i = 0; i < dst->xsize; ++i) {
      center = (double) i / xscale;
      left = ceil(center - width);
      right = floor(center + width);

      contrib[i].n = 0;
      contrib[i].p = (CONTRIB *)calloc(right - left + 1, sizeof(CONTRIB));

      for(j = left; j <= right; ++j) {
	weight = center - (double) j;
	weight = (*filterf)(weight / fscale) / fscale;
	if(j < 0) {
	  n = -j;
	} else if(j >= src->xsize) {
	  n = (src->xsize - j) + src->xsize - 1;
	} else {
	  n = j;
	}
	k = contrib[i].n++;
	contrib[i].p[k].pixel = n;
	contrib[i].p[k].weight = weight;
      }
    }
  } else {
    for(i = 0; i < dst->xsize; ++i) {
      center = (double) i / xscale;
      left = ceil(center - fwidth);
      right = floor(center + fwidth);

      contrib[i].n = 0;
      contrib[i].p = (CONTRIB *)calloc(right - left + 1, sizeof(CONTRIB));
      for(j = left; j <= right; ++j) {
	weight = center - (double) j;
	weight = (*filterf)(weight);
	if(j < 0) {
	  n = -j;
	} else if(j >= src->xsize) {
	  n = (src->xsize - j) + src->xsize - 1;
	} else {
	  n = j;
	}
	k = contrib[i].n++;
	contrib[i].p[k].pixel = n;
	contrib[i].p[k].weight = weight;
      }
    }
  }

  /* apply filter to zoom horizontally from src to tmp */
  raster = (BXPColor *) calloc(src->xsize, sizeof(BXPColor));
  for(k = 0; k < tmp->ysize; ++k) {
    get_row(raster, src, k);
    for(i = 0; i < tmp->xsize; ++i) {
      BXPColor		pixel;
      double		wt[3];
      
      wt[0] = wt[1] = wt[2] = 0.0;
      
      for(j = 0; j < contrib[i].n; ++j) {
	wt[0] += raster[contrib[i].p[j].pixel].r * contrib[i].p[j].weight;
	wt[1] += raster[contrib[i].p[j].pixel].g * contrib[i].p[j].weight;
	wt[2] += raster[contrib[i].p[j].pixel].b * contrib[i].p[j].weight;
      }

      pixel.r = CLAMP(wt[0], BLACK_PIXEL, WHITE_PIXEL);
      pixel.g = CLAMP(wt[1], BLACK_PIXEL, WHITE_PIXEL);
      pixel.b = CLAMP(wt[2], BLACK_PIXEL, WHITE_PIXEL);
      put_pixel(tmp, i, k, &pixel);
    }
  }
  free(raster);

  /* free the memory allocated for horizontal filter weights */
  for(i = 0; i < tmp->xsize; ++i) {
    free(contrib[i].p);
  }
  free(contrib);

  /* pre-calculate filter contributions for a column */
  contrib = (CLIST *)calloc(dst->ysize, sizeof(CLIST));
  if(yscale < 1.0) {
    width = fwidth / yscale;
    fscale = 1.0 / yscale;
    for(i = 0; i < dst->ysize; ++i) {
      center = (double) i / yscale;
      left = ceil(center - width);
      right = floor(center + width);

      contrib[i].n = 0;
      contrib[i].p = (CONTRIB *)calloc(right - left + 1, sizeof(CONTRIB));
      for(j = left; j <= right; ++j) {
	weight = center - (double) j;
	weight = (*filterf)(weight / fscale) / fscale;
	if(j < 0) {
	  n = -j;
	} else if(j >= tmp->ysize) {
	  n = (tmp->ysize - j) + tmp->ysize - 1;
	} else {
	  n = j;
	}
	k = contrib[i].n++;
	contrib[i].p[k].pixel = n;
	contrib[i].p[k].weight = weight;
      }
    }
  } else {
    for(i = 0; i < dst->ysize; ++i) {
      center = (double) i / yscale;
      left = ceil(center - fwidth);
      right = floor(center + fwidth);

      contrib[i].n = 0;
      contrib[i].p = (CONTRIB *)calloc(right - left + 1, sizeof(CONTRIB));
      for(j = left; j <= right; ++j) {
	weight = center - (double) j;
	weight = (*filterf)(weight);
	if(j < 0) {
	  n = -j;
	} else if(j >= tmp->ysize) {
	  n = (tmp->ysize - j) + tmp->ysize - 1;
	} else {
	  n = j;
	}
	k = contrib[i].n++;
	contrib[i].p[k].pixel = n;
	contrib[i].p[k].weight = weight;
      }
    }
  }

  /* apply filter to zoom vertically from tmp to dst */
  raster = (BXPColor *)calloc(tmp->ysize, sizeof(BXPColor));
  for(k = 0; k < dst->xsize; ++k) {
    get_column(raster, tmp, k);
    for(i = 0; i < dst->ysize; ++i) {
      BXPColor		pixel;
      double		wt[3];
      
      wt[0] = wt[1] = wt[2] = 0.0;
      for(j = 0; j < contrib[i].n; ++j) {
	wt[0] += raster[contrib[i].p[j].pixel].r * contrib[i].p[j].weight;
	wt[1] += raster[contrib[i].p[j].pixel].g * contrib[i].p[j].weight;
	wt[2] += raster[contrib[i].p[j].pixel].b * contrib[i].p[j].weight;
      }

      pixel.r = CLAMP(wt[0], BLACK_PIXEL, WHITE_PIXEL);
      pixel.g = CLAMP(wt[1], BLACK_PIXEL, WHITE_PIXEL);
      pixel.b = CLAMP(wt[2], BLACK_PIXEL, WHITE_PIXEL);

      put_pixel(dst, k, i, &pixel);
    }
  }
  free(raster);

  /* free the memory allocated for vertical filter weights */
  for(i = 0; i < dst->ysize; ++i) {
    free(contrib[i].p);
  }
  free(contrib);

  free_image(tmp);
}


extern int resize_image (
  BXPColor __far *	src,
  int			sw,
  int			sh,
  BXPColor __far *	dst,
  int			dw,
  int			dh) {

  Bitmap *		isrc;
  Bitmap *		idst;

  gfx_resize_bitmap (src, sw, sh, dst, dw, dh, success);
  return success;
  
  success = 1;
  
  isrc = new_image_with_data (sw, sh, src);
  idst = new_image_with_data (dw, dh, dst);

  zoom (idst, isrc, Mitchell_filter, Mitchell_support);
  /* zoom (idst, isrc, box_filter, box_support); */

  free_image_with_data (isrc);
  free_image_with_data (idst);

  return success;
}
