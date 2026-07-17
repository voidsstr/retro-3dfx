#include "ogtst.h"

/*
 * Set default read and write visuals for the visual.  Usefull for cleanup
 * stage of tests that muck around with these.
 */
void
ogLibSetDefaultBuffers(void)
{
    GLenum defBuf;

    if (ogEnvDoingAuxBuffer()) {
        defBuf = GL_AUX0;
    } else if (ogEnvCurVisualInfo(GLX_DOUBLEBUFFER)) {
        defBuf = GL_BACK;
    } else {
        defBuf = GL_FRONT;
    }
    glReadBuffer(defBuf);
    glDrawBuffer(defBuf);
}

void
ogLibSetDefaultColors(void)
{
    glColor4f(1, 1, 1, 1);
    glIndexf(1);
}

void
ogLibSetDefaultClears(void)
{
    glClearColor(0, 0, 0, 0);
    glClearIndex(0);
    glClearDepth(1);
    glClearStencil(0);
    glClearAccum(0, 0, 0, 0);
}

void
ogLibSetDefaultLight(void)
{
    static const  float def_lm_amb[4] = {.2, .2, .2,  1.};
    static const  float def_lm_localviewer = 0;
    static const  float def_mat_emis[4] = {0, 0, 0, 1};
    static const  float def_mat_amb[4] = {.2,.2, .2, 1};
    static const  float def_mat_diff[4] = {.8,.8,.8, 1.};
    static const  float def_mat_spec[4] = {.0,.0,.0,1.};
    static const  float def_mat_shin = 0;
    static const  float def_lt_pos[4] = {0, 0, 1, 0};
    static const  float def_lt_amb[4] = {0, 0, 0, 1};
    static const  float def_lt_diff[4] = {1, 1, 1, 1};
    static const  float def_lt_spec[4] = {1, 1, 1, 1};

    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, def_lm_amb);
    glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER, def_lm_localviewer);
    glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, def_lm_localviewer);

    glMaterialfv(GL_FRONT, GL_EMISSION, def_mat_emis);
    glMaterialfv(GL_FRONT, GL_AMBIENT, def_mat_amb);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, def_mat_diff);
    glMaterialfv(GL_FRONT, GL_SPECULAR, def_mat_spec);
    glMaterialf(GL_FRONT, GL_SHININESS, def_mat_shin);

    glLightfv(GL_LIGHT0, GL_POSITION, def_lt_pos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, def_lt_amb);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, def_lt_diff);
    glLightfv(GL_LIGHT0, GL_SPECULAR, def_lt_spec);

    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);
}

void
ogLibSetDefaultTextures(void)
{
    int texLevel = 0;
    GLint maxTexSize = 0; 

    glDisable(GL_TEXTURE_1D);
    glDisable(GL_TEXTURE_2D);

    glTexCoord4f(0,0,0,1);

    glGetIntegerv (GL_MAX_TEXTURE_SIZE, &maxTexSize);

    for (texLevel = 0; maxTexSize > 1; texLevel++)
        maxTexSize >>= 1;
    

    while (texLevel >= 0)
    {
	glTexImage2D(GL_TEXTURE_2D, texLevel, 1, 0, 0, 0, GL_LUMINANCE,
		GL_UNSIGNED_BYTE, NULL);
	texLevel--;
    }

    /* set the filter after nullifying the images to prevent a readback
       on systems that must reconfigure texture memory on a transition
       such as minfilter=linear to minfilter=mipmap.  the readback will
       slow ogtst down.
    */
    glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,
		    GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
}

void
ogLibSetDefaultDetail(void) {
#ifdef GL_SGIS_detail_texture
    static GLfloat points[] = { 0., 0., -4., 1., };
    glTexParameteri(GL_TEXTURE_2D, GL_DETAIL_TEXTURE_MODE_SGIS, GL_ADD);
    glDetailTexFuncSGIS(GL_TEXTURE_2D, 2, points);
#endif
}

void
ogLibSetDefaultSharpen(void) {
#ifdef GL_SGIS_sharpen_texture
    static GLfloat points[] = { 0., 0., -4., 1., };
    glSharpenTexFuncSGIS(GL_TEXTURE_2D, 2, points);
#endif
}

static float vertexMap[] = { 0, 0, 0, 1 };
static float textureMap[] = { 0, 0, 0, 1 };
static float colorMap[] = { 1, 1, 1, 1 };
static float normalMap[] = { 0, 0, 1 };
static float indexMap[] = { 1 };

void
ogLibSetDefaultEvaluators(void)
{
    glDisable(GL_MAP1_COLOR_4);
    glDisable(GL_MAP1_INDEX);
    glDisable(GL_MAP1_NORMAL);
    glDisable(GL_MAP1_TEXTURE_COORD_1);
    glDisable(GL_MAP1_TEXTURE_COORD_2);
    glDisable(GL_MAP1_TEXTURE_COORD_3);
    glDisable(GL_MAP1_TEXTURE_COORD_4);
    glDisable(GL_MAP1_VERTEX_3);
    glDisable(GL_MAP1_VERTEX_4);
    glDisable(GL_MAP2_COLOR_4);
    glDisable(GL_MAP2_INDEX);
    glDisable(GL_MAP2_NORMAL);
    glDisable(GL_MAP2_TEXTURE_COORD_1);
    glDisable(GL_MAP2_TEXTURE_COORD_2);
    glDisable(GL_MAP2_TEXTURE_COORD_3);
    glDisable(GL_MAP2_TEXTURE_COORD_4);
    glDisable(GL_MAP2_VERTEX_3);
    glDisable(GL_MAP2_VERTEX_4);
    glDisable(GL_AUTO_NORMAL);

    glMap1f(GL_MAP1_COLOR_4, 0, 1, 4, 1, colorMap);
    glMap1f(GL_MAP1_INDEX, 0, 1, 1, 1, indexMap);
    glMap1f(GL_MAP1_NORMAL, 0, 1, 3, 1, normalMap);
    glMap1f(GL_MAP1_TEXTURE_COORD_1, 0, 1, 1, 1, textureMap);
    glMap1f(GL_MAP1_TEXTURE_COORD_2, 0, 1, 2, 1, textureMap);
    glMap1f(GL_MAP1_TEXTURE_COORD_3, 0, 1, 3, 1, textureMap);
    glMap1f(GL_MAP1_TEXTURE_COORD_4, 0, 1, 4, 1, textureMap);
    glMap1f(GL_MAP1_VERTEX_3, 0, 1, 3, 1, vertexMap);
    glMap1f(GL_MAP1_VERTEX_4, 0, 1, 4, 1, vertexMap);

    glMap2f(GL_MAP2_COLOR_4, 0, 1, 4, 1, 0, 1, 4, 1, colorMap);
    glMap2f(GL_MAP2_INDEX, 0, 1, 1, 1, 0, 1, 1, 1, indexMap);
    glMap2f(GL_MAP2_NORMAL, 0, 1, 3, 1, 0, 1, 3, 1, normalMap);
    glMap2f(GL_MAP2_TEXTURE_COORD_1, 0, 1, 1, 1, 0, 1, 1, 1, textureMap);
    glMap2f(GL_MAP2_TEXTURE_COORD_2, 0, 1, 2, 1, 0, 1, 2, 1, textureMap);
    glMap2f(GL_MAP2_TEXTURE_COORD_3, 0, 1, 3, 1, 0, 1, 3, 1, textureMap);
    glMap2f(GL_MAP2_TEXTURE_COORD_4, 0, 1, 4, 1, 0, 1, 4, 1, textureMap);
    glMap2f(GL_MAP2_VERTEX_3, 0, 1, 3, 1, 0, 1, 3, 1, vertexMap);
    glMap2f(GL_MAP2_VERTEX_4, 0, 1, 4, 1, 0, 1, 4, 1, vertexMap);

    glMapGrid1f(1, 0, 1);
    glMapGrid2f(1, 0, 1, 1, 0, 1);
}

void
ogLibSetDefaultRasterPos(void)
{
    /* in order to set the raster position and
       raster distance correctly we need some
       matrix hackery */
    glMatrixMode(GL_PROJECTION);
    glTranslatef(0., 0., -1.);
    glRasterPos3f(-1, -1, 0.);
    glTranslatef(0., 0., 1.);
    glMatrixMode(GL_MODELVIEW);
}

void
ogLibSetDefaultBlend(void)
{
    glEnable(GL_DITHER);
    glDisable(GL_BLEND);
    glBlendFunc(GL_ONE,GL_ZERO);
#ifndef WIN32
    glBlendEquationEXT(GL_FUNC_ADD_EXT);
#endif
    glLogicOp(GL_COPY);
}

void
ogLibSetDefaultMatrices(void)
{
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void
ogLibSetDefaultFog(void)
{
    GLfloat junk[4];

    glDisable(GL_FOG);
    junk[0] = junk[1] = junk[2] = junk[3] = 0.;
    glFogfv(GL_FOG_COLOR, junk);
    glFogf(GL_FOG_DENSITY, 1);
    glFogf(GL_FOG_START, 0);
    glFogf(GL_FOG_END, 1);
    glFogf(GL_FOG_MODE, GL_EXP);

    glHint(GL_FOG_HINT, GL_DONT_CARE);
}

void
ogLibSetDefaultConvolution(void)
{
#ifdef GL_EXT_convolution
    GLfloat stuff[4];
    int hwType = ogEnvQuery(OG_HW);

    glDisable(GL_CONVOLUTION_1D_EXT);
    glDisable(GL_CONVOLUTION_2D_EXT);
    glDisable(GL_SEPARABLE_2D_EXT);

    stuff[0] = stuff[1] = stuff[2] = stuff[3] = 1;
    glConvolutionParameterfvEXT(GL_CONVOLUTION_1D_EXT,
				GL_CONVOLUTION_FILTER_SCALE_EXT, stuff);
    glConvolutionParameterfvEXT(GL_CONVOLUTION_2D_EXT,
                                GL_CONVOLUTION_FILTER_SCALE_EXT, stuff);
    glConvolutionParameterfvEXT(GL_SEPARABLE_2D_EXT,
                                GL_CONVOLUTION_FILTER_SCALE_EXT, stuff);

    stuff[0] = stuff[1] = stuff[2] = stuff[3] = 0;

    glConvolutionParameterfvEXT(GL_CONVOLUTION_1D_EXT,
				GL_CONVOLUTION_FILTER_BIAS_EXT, stuff);
    glConvolutionParameterfvEXT(GL_CONVOLUTION_2D_EXT,
                                GL_CONVOLUTION_FILTER_BIAS_EXT, stuff);
    glConvolutionParameterfvEXT(GL_SEPARABLE_2D_EXT,
                                GL_CONVOLUTION_FILTER_BIAS_EXT, stuff);

    glConvolutionParameteriEXT(GL_CONVOLUTION_1D_EXT,
			       GL_CONVOLUTION_BORDER_MODE_EXT, GL_REDUCE_EXT);
    glConvolutionParameteriEXT(GL_CONVOLUTION_2D_EXT,
                               GL_CONVOLUTION_BORDER_MODE_EXT, GL_REDUCE_EXT);
    glConvolutionParameteriEXT(GL_SEPARABLE_2D_EXT,
                               GL_CONVOLUTION_BORDER_MODE_EXT, GL_REDUCE_EXT);

    glConvolutionFilter1DEXT(GL_CONVOLUTION_1D_EXT, GL_RGBA, 0, GL_RGBA,
			     GL_UNSIGNED_BYTE, NULL);
    glConvolutionFilter2DEXT(GL_CONVOLUTION_2D_EXT, GL_RGBA, 0, 0, GL_RGBA,
                             GL_UNSIGNED_BYTE, NULL);
    glSeparableFilter2DEXT(GL_SEPARABLE_2D_EXT, GL_RGBA, 0, 0, GL_RGBA,
                           GL_UNSIGNED_BYTE, NULL, NULL);

    glPixelTransferf(GL_POST_CONVOLUTION_RED_SCALE_EXT, 1);
    glPixelTransferf(GL_POST_CONVOLUTION_GREEN_SCALE_EXT, 1);
    glPixelTransferf(GL_POST_CONVOLUTION_BLUE_SCALE_EXT, 1);
    glPixelTransferf(GL_POST_CONVOLUTION_ALPHA_SCALE_EXT, 1);

    glPixelTransferf(GL_POST_CONVOLUTION_RED_BIAS_EXT, 0);
    glPixelTransferf(GL_POST_CONVOLUTION_GREEN_BIAS_EXT, 0);
    glPixelTransferf(GL_POST_CONVOLUTION_BLUE_BIAS_EXT, 0);
    glPixelTransferf(GL_POST_CONVOLUTION_ALPHA_BIAS_EXT, 0);
#endif /* def GL_EXT_convolution */
}

void
ogLibSetDefaultPixelStore(void)
{
    glPixelStorei(GL_PACK_SWAP_BYTES,		0);
    glPixelStorei(GL_UNPACK_SWAP_BYTES,		0);
    glPixelStorei(GL_PACK_LSB_FIRST,		0);
    glPixelStorei(GL_UNPACK_LSB_FIRST,		0);
    glPixelStorei(GL_PACK_ROW_LENGTH,		0);
    glPixelStorei(GL_UNPACK_ROW_LENGTH,		0);
    glPixelStorei(GL_PACK_SKIP_PIXELS,		0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS,	0);
    glPixelStorei(GL_PACK_SKIP_ROWS,		0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS,		0);
    glPixelStorei(GL_PACK_ALIGNMENT,		4);
    glPixelStorei(GL_UNPACK_ALIGNMENT,		4);
#ifdef GL_EXT_texture3D
    if (glGetString(GL_EXTENSIONS) && 
        strstr((const char *)glGetString(GL_EXTENSIONS), "GL_EXT_texture3D"))
       {
	   glPixelStorei(GL_PACK_IMAGE_HEIGHT_EXT, 0);
	   glPixelStorei(GL_UNPACK_IMAGE_HEIGHT_EXT, 0);
	   glPixelStorei(GL_PACK_SKIP_IMAGES_EXT, 0);
	   glPixelStorei(GL_UNPACK_SKIP_IMAGES_EXT, 0);
       }
#endif
#ifdef GL_SGIS_texture4D
    if (glGetString(GL_EXTENSIONS) && 
        strstr((const char *)glGetString(GL_EXTENSIONS), "GL_SGIS_texture4D"))
       {
	   glPixelStorei(GL_PACK_IMAGE_DEPTH_SGIS, 0);
	   glPixelStorei(GL_UNPACK_IMAGE_DEPTH_SGIS, 0);
	   glPixelStorei(GL_PACK_SKIP_VOLUMES_SGIS, 0);
	   glPixelStorei(GL_UNPACK_SKIP_VOLUMES_SGIS, 0);
       }
#endif
}
