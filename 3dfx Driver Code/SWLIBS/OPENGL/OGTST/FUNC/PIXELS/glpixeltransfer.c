#include "ogtst.h"

typedef struct {		/* Pixel Transfer struct */
  /* RGBA component with least nuber of bits != 0 */
  GLint index_shift;
  GLint index_offset;
  GLfloat red_s;
  GLfloat green_s;
  GLfloat blue_s;
  GLfloat alpha_s;
  GLfloat depth_s;
  GLfloat red_b;
  GLfloat green_b;
  GLfloat blue_b;
  GLfloat alpha_b;
  GLfloat depth_b;
} PIXEL_INFO, *PIXEL_INFO_PTR;

typedef struct {
  GLushort *i2i;		/* I_TO_I, S_TO_S */
  GLfloat  *i2c;		/* I_TO_RGBA */
  GLfloat  *c2c;		/* RGBA_TO_RGBA */
  GLint    i2i_size; 
  GLint    i2c_size;
  GLint    c2c_size;
} MAP_INFO, *MAP_INFO_PTR;

static GLboolean isRE = GL_FALSE;
static int hwtype;
static GLboolean isKONA = GL_FALSE, isCRIME = GL_FALSE;
static GLboolean isKonaMultiRGB101010 = GL_FALSE;
static GLfloat *w_pixels = NULL, *t_pixels = NULL, *r_pixels = NULL;

static void
PixelTransferErrors(void)
{
  GLenum error;

  while(glGetError() != GL_NO_ERROR)
    ;
  /* inside begin/end => GL_INVALID_OPERATION */
  glBegin(GL_POINTS);
  glPixelTransferi(GL_MAP_COLOR, GL_FALSE); 
  glEnd();
  error = glGetError();
  if(error != GL_INVALID_OPERATION)
    ogEnvLog(OG_LFAIL, "glPixelTransfer glBegin/glEnd doesn't cause \
GL_INVALID_OPERATION");
  while(glGetError() != GL_NO_ERROR)
    ;
  /* bad first parameter => GL_INVALID_ENUM */
  glPixelTransferi(GL_RED, GL_TRUE);
  error = glGetError();
  if(error != GL_INVALID_ENUM){
    ogEnvLog(OG_LFAIL,"(bad enum Error == 0x%x) != GL_INVALID_ENUM", error);
  }
  while(glGetError() != GL_NO_ERROR)
    ;
}

static GLboolean
create_and_load_maps(EnvironmentPtr ep, MAP_INFO_PTR mp)
{
  GLuint i, max, bmax, tmax=0;

  mp->i2i = NULL;
  mp->i2c = NULL;
  mp->i2i_size = 0;
  mp->i2c_size = 0;
  /* I_TO_I, S_TO_S */
  glGetIntegerv(GL_MAX_PIXEL_MAP_TABLE, (int *) &tmax);

  if (isRE)
    tmax = 2048;
  if(ep->index_mode || ep->stencil_bits) {
    if (ep->index_mode) {
      bmax = 1 << ep->index_bits;
      max = bmax < tmax ? bmax : tmax;
    } else
      max = tmax;
    if(ep->stencil_bits) {
      bmax = 1 << ep->stencil_bits;
      if (bmax < max)
	max = bmax;
    }
    mp->i2i = (GLushort *) ogLibMalloc(sizeof(GLushort) * max);
    if(mp->i2i == NULL)
      return GL_TRUE;
    mp->i2i_size = max;
    for(i = 0; i < max; i++) {
      mp->i2i[i] = max - i - 1;
    }
    if (ep->index_mode)
      glPixelMapusv(GL_PIXEL_MAP_I_TO_I, mp->i2i_size, mp->i2i);
    if (ep->stencil_bits)
      glPixelMapusv(GL_PIXEL_MAP_S_TO_S, (GLint)mp->i2i_size,
		    mp->i2i);
  }
  if (ep->rgba_mode) { 
    max = ep->rgba_bits[0] < ep->rgba_bits[1] ? 
      (ep->rgba_bits[0] < ep->rgba_bits[2] ?
       ep->rgba_bits[0] : ep->rgba_bits[2]) :
      (ep->rgba_bits[1] < ep->rgba_bits[2] ?
       ep->rgba_bits[1] : ep->rgba_bits[2]);
    if(ep->rgba_bits[3] && ep->rgba_bits[3] < max)
	max = ep->rgba_bits[3];
    max = 1 << max;
    if (max > tmax)
      max = tmax;
    mp->i2c = (GLfloat *) ogLibMalloc(sizeof(GLfloat) * max);
    if(mp->i2c == NULL) {
      if (mp->i2i) {
	ogLibFree(mp->i2i);
	mp->i2i = NULL;
      }
      return GL_TRUE;
    }
    mp->i2c_size = max;
    for(i = 0; i < max; i++)
      mp->i2c[i] = (max - i - 1)/(GLfloat)max;
    glPixelMapfv(GL_PIXEL_MAP_I_TO_R, mp->i2c_size, mp->i2c);
    glPixelMapfv(GL_PIXEL_MAP_R_TO_R, mp->i2c_size, mp->i2c);
    glPixelMapfv(GL_PIXEL_MAP_I_TO_G, mp->i2c_size, mp->i2c);
    glPixelMapfv(GL_PIXEL_MAP_G_TO_G, mp->i2c_size, mp->i2c);
    glPixelMapfv(GL_PIXEL_MAP_I_TO_B, mp->i2c_size, mp->i2c);
    glPixelMapfv(GL_PIXEL_MAP_B_TO_B, mp->i2c_size, mp->i2c);
    if(ep->rgba_bits[3]) {
      glPixelMapfv(GL_PIXEL_MAP_I_TO_A, mp->i2c_size, mp->i2c);
      glPixelMapfv(GL_PIXEL_MAP_A_TO_A, mp->i2c_size, mp->i2c);
    }
  }
  return GL_FALSE;
}

static void
create_and_load_transfer(EnvironmentPtr ep, PIXEL_INFO_PTR pip,
			 MAP_INFO_PTR mip, GLboolean forStencil)
{
  int size, i;

  size = (forStencil ? mip->i2i_size :
	  (ep->rgba_mode ? mip->i2c_size : mip->i2i_size))/2;
  pip->index_offset = ogLibIntRand(-size, size); 

  for (i = 0; size; size >>= 1, i++)
      ;
  pip->index_shift = ogLibIntRand(-i, i);

  pip->red_s = ogLibFloatRand(-1, 1);
  pip->green_s = ogLibFloatRand(-1, 1);
  pip->blue_s = ogLibFloatRand(-1, 1);
  pip->alpha_s = ogLibFloatRand(-1, 1);
  pip->depth_s = ogLibFloatRand(-1, 1);

  pip->red_b = ogLibFloatRand(-1, 1);
  pip->green_b = ogLibFloatRand(-1, 1);
  pip->blue_b = ogLibFloatRand(-1, 1);
  pip->alpha_b = ogLibFloatRand(-1, 1);
  pip->depth_b = ogLibFloatRand(-1, 1);

  glPixelTransferi(GL_INDEX_SHIFT, pip->index_shift);
  glPixelTransferi(GL_INDEX_OFFSET, pip->index_offset);

  glPixelTransferf(GL_RED_SCALE, pip->red_s);
  glPixelTransferf(GL_GREEN_SCALE, pip->green_s);
  glPixelTransferf(GL_BLUE_SCALE, pip->blue_s);
  glPixelTransferf(GL_ALPHA_SCALE, pip->alpha_s);
  glPixelTransferf(GL_DEPTH_SCALE, pip->depth_s);

  glPixelTransferf(GL_RED_BIAS, pip->red_b);
  glPixelTransferf(GL_GREEN_BIAS, pip->green_b);
  glPixelTransferf(GL_BLUE_BIAS, pip->blue_b);
  glPixelTransferf(GL_ALPHA_BIAS, pip->alpha_b);
  glPixelTransferf(GL_DEPTH_BIAS, pip->depth_b);

  ogEnvLog(OG_LPARAMETERS, "glPixelTransferi(GL_INDEX_SHIFT, %d);\n",
	   pip->index_shift);
  ogEnvLog(OG_LPARAMETERS, "glPixelTransferi(GL_INDEX_OFFSET, %d);\n",
	   pip->index_offset);

  ogEnvLog(OG_LPARAMETERS, "glPixelTransferf(GL_RED_SCALE, %g);\n",
	   pip->red_s);
  ogEnvLog(OG_LPARAMETERS, "glPixelTransferf(GL_GREEN_SCALE, %g);\n",
	   pip->green_s);
  ogEnvLog(OG_LPARAMETERS, "glPixelTransferf(GL_BLUE_SCALE, %g);\n",
	   pip->blue_s);
  ogEnvLog(OG_LPARAMETERS, "glPixelTransferf(GL_ALPHA_SCALE, %g);\n",
	   pip->alpha_s);
  ogEnvLog(OG_LPARAMETERS, "glPixelTransferf(GL_DEPTH_SCALE, %g);\n",
	   pip->depth_s);

  ogEnvLog(OG_LPARAMETERS, "glPixelTransferf(GL_RED_BIAS, %g);\n",
	   pip->red_b);
  ogEnvLog(OG_LPARAMETERS, "glPixelTransferf(GL_GREEN_BIAS, %g);\n",
	   pip->green_b);
  ogEnvLog(OG_LPARAMETERS, "glPixelTransferf(GL_BLUE_BIAS, %g);\n",
	   pip->blue_b);
  ogEnvLog(OG_LPARAMETERS, "glPixelTransferf(GL_ALPHA_BIAS, %g);\n",
	   pip->alpha_b);
  ogEnvLog(OG_LPARAMETERS, "glPixelTransferf(GL_DEPTH_BIAS, %g);\n",
	   pip->depth_b);

}

static GLvoid
compare_pixels(int w, int h, GLenum format, EnvironmentPtr ep)
{
  int i, j;
  float tolerance;
  GLfloat *ptp, *prp;

  switch(format) {
   case GL_COLOR_INDEX:
    i = ep->index_bits;
    break;
   case GL_STENCIL_INDEX:
    i = ep->stencil_bits;
    break;
   case GL_DEPTH_COMPONENT:
    i = ep->depth_bits;
    break;
   case GL_RED:
   case GL_GREEN:
   case GL_BLUE:
   case GL_ALPHA:
    if (isKonaMultiRGB101010) {
      /*
       * In kona the 10/10/10 MS buffer clamps values to 3fc loosing the
       * bottom 2 bits. So we set precision to that. We should actually
       * test only for the clamped values, but ...
       */
      i = 8;
    } else {
      i = ep->rgba_bits[format - GL_RED];
      if (i == 0)
	i = 8;
    }
    break;
  }
  if (i > 24)                 /* Not enough precision in floats */
    i = 24;
  tolerance = 2.0/(0xffffffff >> (32 - i));
  for (ptp = t_pixels, prp = r_pixels, i = 0; i < h; i++)
    for(j = 0; j < w; j++, ptp++, prp++) {
      float diff = fabs(*ptp - *prp);
      if(diff > tolerance) {
	ogEnvLog(OG_LFAIL, "Pixel[%d, %d] format %s, expected %g, got %g (%g > %g)\n",
		 i, j, ogEnvPixelFormatName(format), *ptp, *prp, diff, tolerance);
      }
    }
}

static void
reset_pix_xfer(void)
{
  glPixelTransferf(GL_MAP_COLOR, GL_FALSE);
  glPixelTransferf(GL_MAP_STENCIL, GL_FALSE);

  glPixelTransferi(GL_INDEX_SHIFT,  0);
  glPixelTransferi(GL_INDEX_OFFSET, 0);

  glPixelTransferf(GL_RED_SCALE,   1);
  glPixelTransferf(GL_GREEN_SCALE, 1);
  glPixelTransferf(GL_BLUE_SCALE,  1);
  glPixelTransferf(GL_ALPHA_SCALE, 1);
  glPixelTransferf(GL_DEPTH_SCALE, 1);

  glPixelTransferf(GL_RED_BIAS,   0);
  glPixelTransferf(GL_GREEN_BIAS, 0);
  glPixelTransferf(GL_BLUE_BIAS,  0);
  glPixelTransferf(GL_ALPHA_BIAS, 0);
  glPixelTransferf(GL_DEPTH_BIAS, 0);
}

static GLvoid
emulate_color_xfer(GLint size, GLfloat scale, GLfloat bias, GLboolean mapped,
		   MAP_INFO_PTR mip, int bufBits)
{
    GLfloat *ptp, *pwp;
    GLint i;

    for(ptp = t_pixels, pwp = w_pixels, i=0; i < size; i++, ptp++, pwp++) {
	if (bufBits) {
	    *ptp = *pwp * scale + bias;
	    /* clamping */
	    if(*ptp < 0)
		*ptp = 0;
	    else if(*ptp > 1)
		*ptp = 1;
	    if(mapped){
		*ptp *= mip->i2c_size - 1;
		*ptp = mip->i2c[(GLint)(*ptp+0.5)];
	    }
	} else {
	    *ptp = 0.0;
	}
   }
}

/*ARGSUSED*/
static GLvoid
emulate_pix_xfer(GLenum source_format, GLenum target_format,
		 GLboolean mapped, int size, MAP_INFO_PTR mip,
		 PIXEL_INFO_PTR pip, EnvironmentPtr ep)
{
  int i;
  GLuint mask;
  GLfloat *ptp, *pwp, *w_image;

  switch(source_format) {
   case GL_DEPTH_COMPONENT:
    for(ptp = t_pixels, pwp = w_pixels, i=0; i < size; i++, ptp++, pwp++) {
      *ptp = *pwp * pip->depth_s + pip->depth_b;
      /* clamping */
      if(*ptp < 0)
	*ptp = 0;
      else if(*ptp > 1)
	*ptp = 1;
    }
    break;
   case GL_RED:
    emulate_color_xfer(size, pip->red_s, pip->red_b, mapped, mip,
		       ep->rgba_bits[0]);
    break;
   case GL_GREEN:
    emulate_color_xfer(size, pip->green_s, pip->green_b, mapped, mip,
		       ep->rgba_bits[1]);
    break;
   case GL_BLUE:
    emulate_color_xfer(size, pip->blue_s, pip-> blue_b, mapped, mip,
		       ep->rgba_bits[2]);
    break;
   case GL_ALPHA:
    emulate_color_xfer(size, pip->alpha_s, pip->alpha_b, mapped, mip,
		       ep->rgba_bits[3]);
    break;
   case GL_COLOR_INDEX:
    mask = (ep->index_mode ? (mapped ? mip->i2i_size : (1 << ep->index_bits)) :
	    mip->i2c_size) - 1;
    if (isKONA) {
      /* Kona does float->fixed conversion with rounding */
      for (ptp = t_pixels, pwp = w_pixels, i=0; i < size; i++, ptp++, pwp++)
	*ptp = (GLuint) rint(*pwp);
      w_image = t_pixels;
    } else
      w_image = w_pixels;
    if (pip->index_shift > 0) {
      for (ptp = t_pixels, pwp = w_image, i=0; i < size; i++, ptp++, pwp++)
	*ptp = (((GLuint) *pwp << pip->index_shift) + pip->index_offset) & mask;
    } else {
      for(ptp = t_pixels, pwp = w_image, i=0; i < size; i++, ptp++, pwp++)
	*ptp = (((GLuint) *pwp >> -pip->index_shift) + pip->index_offset) & mask;
    }
    if(ep->rgba_mode)
      /* I_to_RGBA lookup */
      for(ptp = t_pixels, i = 0; i < size; i++, ptp++)
	*ptp = mip->i2c[(GLint)(*ptp)];
    else if(mapped)
      for(ptp = t_pixels, i = 0; i < size; i++, ptp++)
	*ptp = mip->i2i[(GLint) *ptp] & mask;
    break;
   case GL_STENCIL_INDEX:
    mask = (mapped ? mip->i2i_size : (1 << ep->stencil_bits)) - 1;
    if (isKONA) {
      /* Kona does float->fixed conversion with rounding */
      for (ptp = t_pixels, pwp = w_pixels, i=0; i < size; i++, ptp++, pwp++)
	*ptp = (GLuint) rint(*pwp);
      w_image = t_pixels;
    } else
      w_image = w_pixels;
    /* shift and offset */
    if(pip->index_shift > 0)
      for(ptp = t_pixels, pwp = w_image, i=0; i < size; i++, ptp++, pwp++)
	*ptp = (((GLuint)*pwp << pip->index_shift) + pip->index_offset) & mask;
    else 
      for(ptp = t_pixels, pwp = w_image, i=0; i < size; i++, ptp++, pwp++)
	*ptp = (((GLuint)*pwp >> -pip->index_shift) + pip->index_offset) & mask;
    if (mapped) {
      /* S_to_S lookup */
      for(ptp = t_pixels, i = 0; i < size; i++, ptp++)
	*ptp = mip->i2i[(GLuint) *ptp];
    }
    break;
   default:
    ogEnvLog(OG_LINTERNALERROR, "bad case\n");
  }
}

/* snap x onto a (2n+1)/16 grid */
#define SNAP(x) (((int)(x * 8.0) / 8.0) + (1/16.0))

#define SNAP_CRM(x) (((int)(x * 16.0) / 16.0) + (1/32.0))

static GLvoid
do_test(GLenum format, EnvironmentPtr ep, PIXEL_INFO_PTR pip,
	MAP_INFO_PTR mip, GLboolean mapped, int winWidth, int winHeight)
{
  GLint i;
  GLfloat *ptr;
  GLfloat x, y;
  GLfloat maxColor;
  GLint ix, iy;
  int w, h;
   

  w = ogLibIntRand(16, winWidth/8);
  h = ogLibIntRand(16, winHeight/8);
  x = ogLibFloatRand(0, winWidth - w - 1);
  y = ogLibFloatRand(0, winHeight - h - 1);
  if (isRE || isKONA) {
    x = SNAP(x);
    y = SNAP(y);
  } else if (isCRIME) {
      x = SNAP_CRM(x);
      y = SNAP_CRM(y);
  }
  glRasterPos2f(x, y);
  ogEnvLog(OG_LPARAMETERS, "Rast Post %g %g\n", x, y);
  ix = ceilf(x + 0.5) - 1;
  iy = ceilf(y + 0.5) - 1;

  switch (format) {
   case GL_COLOR_INDEX:
    maxColor = (ep->index_mode ? mip->i2i_size : mip->i2c_size) - 1;
    break;
   case GL_STENCIL_INDEX:
    maxColor = mip->i2i_size - 1;
    break;
   case GL_DEPTH_COMPONENT:
    w = h = 8;
   default:
    maxColor = 1;
    break;
  }
  for (ptr = w_pixels, i = w * h; i; i--, ptr++)
      *ptr = ogLibFloatRand(0, maxColor);
  ogEnvLog(OG_LPARAMETERS, "glDrawPixels(%d, %d, %s, GL_FLOAT, pixels);\n",
	   w, h, ogEnvPixelFormatName(format));
  glDrawPixels(w, h, format, GL_FLOAT, w_pixels);
	
  ogEnvLog(OG_LPARAMETERS, "Reset Tranfer Modes\n");
  reset_pix_xfer();

  /* the acutal glReadPixels */
  if(format == GL_COLOR_INDEX) {
    if(ep->rgba_mode) {
      ogEnvLog(OG_LPARAMETERS,"I_to_R:\n");
      glReadPixels(ix, iy, w, h, GL_RED, GL_FLOAT, r_pixels);
      emulate_pix_xfer(GL_COLOR_INDEX, GL_RED, mapped, w * h, mip, pip, ep);
      compare_pixels(w, h, GL_RED, ep);
      ogEnvLog(OG_LPARAMETERS,"I_to_G:\n");
      glReadPixels(ix, iy, w, h, GL_GREEN, GL_FLOAT, r_pixels);
      emulate_pix_xfer(GL_COLOR_INDEX, GL_GREEN, mapped, w * h, mip, pip, ep);
      compare_pixels(w, h, GL_GREEN, ep);
      ogEnvLog(OG_LPARAMETERS,"I_to_B:\n");
      glReadPixels(ix, iy, w, h, GL_BLUE, GL_FLOAT, r_pixels);
      emulate_pix_xfer(GL_COLOR_INDEX, GL_BLUE, mapped, w * h, mip, pip, ep);
      compare_pixels(w, h, GL_BLUE, ep);
      if(ep->rgba_bits[3]){
	ogEnvLog(OG_LPARAMETERS,"I_to_A:\n");
	glReadPixels(ix, iy, w, h, GL_ALPHA, GL_FLOAT, r_pixels);
	emulate_pix_xfer(GL_COLOR_INDEX, GL_ALPHA, mapped, w * h, mip, pip, ep);
	compare_pixels(w, h, GL_ALPHA, ep);
      }
    } else {
      ogEnvLog(OG_LPARAMETERS,"Color_Index\n");
      glReadPixels(ix, iy, w, h, GL_COLOR_INDEX, GL_FLOAT, r_pixels);
      emulate_pix_xfer(format, format, mapped, w * h, mip, pip, ep);
      compare_pixels(w, h, format, ep);
    }
  } else {
    glReadPixels(ix, iy, w, h, format, GL_FLOAT, r_pixels);
    emulate_pix_xfer(format, format, mapped, w * h, mip, pip, ep);
    compare_pixels(w, h, format, ep);
  }
}

TESTMOD (PixelTransfer)
{
    Environment  env;
    MAP_INFO     maps;
    PIXEL_INFO   pi;
    int          winWidth = ogEnvQuery(OG_XWSIZE);
    int          winHeight = ogEnvQuery(OG_YWSIZE);

    w_pixels = (GLfloat *) ogLibMalloc((winWidth/8)*(winHeight/8)*
                                       sizeof(GLfloat));
    t_pixels = (GLfloat *) ogLibMalloc((winWidth/8)*(winHeight/8)*
                                       sizeof(GLfloat));
    r_pixels = (GLfloat *) ogLibMalloc((winWidth/8)*(winHeight/8)*
                                       sizeof(GLfloat));
    if(w_pixels == NULL || t_pixels == NULL || r_pixels == NULL) {
    ogEnvLog(OG_LINTERNALERROR,
             "Can not malloc pixel memory for PixelTransfer test\n");
    /* LINTERNAL exits */
    }
    ogLibGetEnvironment(&env);
    ogLibShowEnvironment();
    ogEnvMultiSamplingState(GL_FALSE);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, winWidth, 0, winHeight, -1, 1);
    if (create_and_load_maps(&env, &maps)) {
        ogEnvLog(OG_LINTERNALERROR,
                 "Can not malloc pixmap memory for PixelTransfer test\n");
        /* LINTERNAL exits */
    }
    glDisable(GL_DITHER);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);

    PixelTransferErrors();
    while(pass--) { 
        /* Color */
        if(env.rgba_mode) {
            /* TEST RED_SCALE, RED_BIAS WITH MAP COLOR OFF */
	    ogEnvLog(OG_LPARAMETERS,"***RED TEST UNMAPPED***\n");
            create_and_load_transfer(&env, &pi, &maps, GL_FALSE);
	    glPixelTransferf(GL_MAP_COLOR, GL_FALSE);
            do_test(GL_RED, &env, &pi, &maps, GL_FALSE, winWidth, winHeight);

            /* TEST RED_SCALE, RED_BIAS WITH MAP COLOR ON */
	    ogEnvLog(OG_LPARAMETERS,"***RED TEST MAPPED***\n");
            create_and_load_transfer(&env, &pi, &maps, GL_FALSE);
	    glPixelTransferf(GL_MAP_COLOR, GL_TRUE);
            do_test(GL_RED, &env, &pi, &maps, GL_TRUE, winWidth, winHeight);

            /* TEST GREEN_SCALE, GREEN_BIAS WITH MAP COLOR OFF */
	    ogEnvLog(OG_LPARAMETERS,"***GREEN TEST UNMAPPED***\n");
            create_and_load_transfer(&env, &pi, &maps, GL_FALSE);
	    glPixelTransferf(GL_MAP_COLOR, GL_FALSE);
            do_test(GL_GREEN, &env, &pi, &maps, GL_FALSE, winWidth, winHeight);

            /* TEST GREEN_SCALE, GREEN_BIAS WITH MAP COLOR ON */
	    ogEnvLog(OG_LPARAMETERS,"***GREEN TEST MAPPED***\n");
            create_and_load_transfer(&env, &pi, &maps, GL_FALSE);
	    glPixelTransferf(GL_MAP_COLOR, GL_TRUE);
            do_test(GL_GREEN, &env, &pi, &maps, GL_TRUE, winWidth, winHeight);

            /* TEST BLUE_SCALE, BLUE_BIAS WITH MAP COLOR OFF */
	    ogEnvLog(OG_LPARAMETERS,"***BLUE TEST UNMAPPED***\n");
            create_and_load_transfer(&env, &pi, &maps, GL_FALSE);
	    glPixelTransferf(GL_MAP_COLOR, GL_FALSE);
            do_test(GL_BLUE, &env, &pi, &maps, GL_FALSE, winWidth, winHeight);

            /* TEST BLUE_SCALE, BLUE_BIAS WITH MAP COLOR ON */
	    ogEnvLog(OG_LPARAMETERS,"***BLUE TEST MAPPED***\n");
            create_and_load_transfer(&env, &pi, &maps, GL_FALSE);
	    glPixelTransferf(GL_MAP_COLOR, GL_TRUE);
            do_test(GL_BLUE, &env, &pi, &maps, GL_TRUE, winWidth, winHeight);

	    if(env.rgba_bits[3]){
                /* TEST ALPHA_SCALE, ALPHA_BIAS WITH MAP COLOR OFF */
	        ogEnvLog(OG_LPARAMETERS,"***ALPHA TEST UNMAPPED***\n");
                create_and_load_transfer(&env, &pi, &maps, GL_FALSE);
	        glPixelTransferf(GL_MAP_COLOR, GL_FALSE);
                do_test(GL_ALPHA, &env, &pi, &maps, GL_FALSE,
			winWidth, winHeight);

                /* TEST ALPHA_SCALE, ALPHA_BIAS WITH MAP COLOR ON */
	        ogEnvLog(OG_LPARAMETERS,"***ALPHA TEST MAPPED***\n");
                create_and_load_transfer(&env, &pi, &maps, GL_FALSE);
	        glPixelTransferf(GL_MAP_COLOR, GL_TRUE);
                do_test(GL_ALPHA, &env, &pi, &maps, GL_TRUE,
			winWidth, winHeight);
	    }

	    /* Index With Map Off */
	    ogEnvLog(OG_LPARAMETERS,"***I_to_RGBA TEST UNMAPPED***\n");
            create_and_load_transfer(&env, &pi, &maps, GL_FALSE);
	    glPixelTransferf(GL_MAP_COLOR, GL_FALSE);
            do_test(GL_COLOR_INDEX, &env, &pi, &maps, GL_FALSE,
		    winWidth, winHeight);

            /* Index With Map On (should be no difference) */
	    ogEnvLog(OG_LPARAMETERS,"***I_to_RGBA TEST MAPPED***\n");
            create_and_load_transfer(&env, &pi, &maps, GL_FALSE);
	    glPixelTransferf(GL_MAP_COLOR, GL_TRUE);
            do_test(GL_COLOR_INDEX, &env, &pi, &maps, GL_TRUE,
		    winWidth, winHeight);
        } else {
            /* Index mode */
            /* Map off */
	    ogEnvLog(OG_LPARAMETERS,"***INDEX MODE TEST UNMAPPED***\n");
            create_and_load_transfer(&env, &pi, &maps, GL_FALSE);
	    glPixelTransferf(GL_MAP_COLOR, GL_FALSE);
            do_test(GL_COLOR_INDEX, &env, &pi, &maps, GL_FALSE,
		    winWidth, winHeight);

            /* Map on, I_to_I */
	    ogEnvLog(OG_LPARAMETERS,"***INDEX MODE TEST MAPPED***\n");
            create_and_load_transfer(&env, &pi, &maps, GL_FALSE);
	    glPixelTransferf(GL_MAP_COLOR, GL_TRUE);
            do_test(GL_COLOR_INDEX, &env, &pi, &maps, GL_TRUE,
		    winWidth, winHeight);
        }
	/* Depth */
        if (env.depth_bits) {
	    /* TEST DEPTH WITH MAP COLOR OFF */
	    ogEnvLog(OG_LPARAMETERS, "***DEPTH TEST UNMAPPED***\n");
            create_and_load_transfer(&env, &pi, &maps, GL_FALSE);
	    glPixelTransferf(GL_MAP_COLOR, GL_FALSE);
            do_test(GL_DEPTH_COMPONENT, &env, &pi, &maps, GL_FALSE,
		    winWidth, winHeight);

 	    /* TEST DEPTH WITH MAP COLOR ON */
	    ogEnvLog(OG_LPARAMETERS,"***DEPTH TEST MAPPED***\n");
            create_and_load_transfer(&env, &pi, &maps, GL_FALSE);
	    glPixelTransferf(GL_MAP_COLOR, GL_TRUE);
            do_test(GL_DEPTH_COMPONENT, &env, &pi, &maps, GL_TRUE,
		    winWidth, winHeight); 
        }
        /* Stencil */
        if(env.stencil_bits) {
            /* Stencil with Map Stencil off */
            ogEnvLog(OG_LPARAMETERS,"***STENCIL TEST UNMAPPED***\n");
            create_and_load_transfer(&env, &pi, &maps, GL_TRUE);
            glPixelTransferf(GL_MAP_STENCIL, GL_FALSE);
            do_test(GL_STENCIL_INDEX, &env, &pi, &maps, GL_FALSE,
                    winWidth, winHeight);
            /* Stencil with Map Stencil on */
            ogEnvLog(OG_LPARAMETERS,"***STENCIL TEST MAPPED***\n");
            create_and_load_transfer(&env, &pi, &maps, GL_TRUE);
            glPixelTransferf(GL_MAP_STENCIL, GL_TRUE);
            do_test(GL_STENCIL_INDEX, &env, &pi, &maps, GL_TRUE,
                    winWidth, winHeight);
        }
    }
    ogLibFree(maps.i2i);
    ogLibFree(maps.i2c);
    ogLibFree(t_pixels);
    ogLibFree(r_pixels);
    ogLibFree(w_pixels);
}

CLEANUP (PixelTransfer)
{
    GLuint hugeMap = 0;
    
    reset_pix_xfer();
    glPixelMapuiv(GL_PIXEL_MAP_I_TO_I, 1, &hugeMap);
    glPixelMapuiv(GL_PIXEL_MAP_I_TO_R, 1, &hugeMap);
    glPixelMapuiv(GL_PIXEL_MAP_I_TO_G, 1, &hugeMap);
    glPixelMapuiv(GL_PIXEL_MAP_I_TO_B, 1, &hugeMap);
    glPixelMapuiv(GL_PIXEL_MAP_I_TO_A, 1, &hugeMap);
    glPixelMapuiv(GL_PIXEL_MAP_R_TO_R, 1, &hugeMap);
    glPixelMapuiv(GL_PIXEL_MAP_G_TO_G, 1, &hugeMap);
    glPixelMapuiv(GL_PIXEL_MAP_B_TO_B, 1, &hugeMap);
    glPixelMapuiv(GL_PIXEL_MAP_A_TO_A, 1, &hugeMap);
    glPixelMapuiv(GL_PIXEL_MAP_S_TO_S, 1, &hugeMap);
    glEnable(GL_DITHER);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    ogLibSetDefaultRasterPos();
    if (ogEnvIsMultiSampled())
        ogEnvMultiSamplingState(GL_TRUE);
    ogEnvMultiSamplingState(GL_TRUE);
    glDisable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
}
