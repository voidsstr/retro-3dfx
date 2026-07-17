#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "ogtst.h"

#define ORTHO 1
#define FRUSTUM 2
#define ERROR_TOLERANCE 1.0/1000.0

GLboolean approx_eq(GLdouble a, GLdouble b)
{
    GLdouble denom; /* should be the SMALLER of the two */
    GLdouble aa = ABS(a);
    GLdouble bb = ABS(b);

    if(0==aa) {
        if(bb<ERROR_TOLERANCE)
            return GL_TRUE;
        else
            return GL_FALSE;
    }

    if(0==bb) {
        if(aa<ERROR_TOLERANCE)
            return GL_TRUE;
        else
            return GL_FALSE;
    }

    if(aa>bb)
        denom = bb;
    else
        denom = aa;

    if(ABS(a-b)/denom < ERROR_TOLERANCE)
        return GL_TRUE;
    else
        return GL_FALSE;

}

GLint RandInt(GLint int_max)
{
    return (GLint)(int_max * (rand()/(GLfloat)RAND_MAX));
}

GLvoid RasterPosRand(GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    GLint rand_int, n_func;
    GLfloat  f[4];
    GLdouble d[4];
    GLint    i[4];
    GLshort  s[4];
    GLboolean is_float; /* is it a floating point number */

    is_float = ( x!=(int)x || y!=(int)y || z!=(int)z || w!=(int)w);
    if(is_float) {
        n_func = 12;
	if(w != 1){
	    n_func = 4;
	}
        if(z != 0){
	    n_func = 8;
	}
    } else {
        n_func = 24;
	if(w != 1){
	    n_func = 8;
	}
        if(z != 0){
	    n_func = 16;
	}
    }
    n_func--;
    /* randomly choose a glRasterPos command */
    rand_int = RandInt(n_func);
    if(is_float)
	rand_int = (rand_int/2)*4 + (rand_int%2); /* note integer div */

    switch(rand_int) {
/* 4 parameter cases*/
        case 0: 
	    d[0] = x; d[1] = y; d[2] = z; d[3] = w;
	    ogEnvLog(OG_LINTERNALDEBUG, "\t\tglRasterPos4dv(%g,%g,%g,%g)\n",d[0], d[1], d[2], d[3]);
	    glRasterPos4dv(d);
	break;
        case 1: 
	    f[0] = (GLfloat)x; f[1] = (GLfloat)y; f[2] = (GLfloat)z; f[3] = (GLfloat)w;
	    ogEnvLog(OG_LINTERNALDEBUG, "\t\tglRasterPos4fv(%g,%g,%g,%g)\n",f[0], f[1], f[2], f[3]);
	    glRasterPos4fv(f);
	break;
        case 2: 
	    i[0] = (GLint)x; i[1] = (GLint)y; i[2] = (GLint)z; i[3] = (GLint)w;
	    ogEnvLog(OG_LINTERNALDEBUG, "\t\tglRasterPos4iv(%d,%d,%d,%d)\n",i[0], i[1], i[2], i[3]);
	    glRasterPos4iv(i);
	break;
        case 3: 
	    s[0] = (GLshort)x; s[1] = (GLshort)y; s[2] = (GLshort)z; s[3] = (GLshort)w;
	    ogEnvLog(OG_LINTERNALDEBUG, "\t\tglRasterPos4sv(%d,%d,%d,%d)\n", (int)s[0], (int)s[1], (int)s[2], (int)s[3]);
	    glRasterPos4sv(s);
	break;
	case 4:
	    ogEnvLog(OG_LINTERNALDEBUG, "\t\tglRasterPos4d(%g,%g,%g,%g)\n", x, y, z, w);
	    glRasterPos4d(x,y,z,w);
	break;
	case 5:
	    ogEnvLog(OG_LINTERNALDEBUG, "\t\tglRasterPos4f(%g,%g,%g,%g)\n",(GLfloat)x, (GLfloat)y, (GLfloat)z, (GLfloat)w);
	    glRasterPos4f((GLfloat)x, (GLfloat)y, (GLfloat)z, (GLfloat)w);
	break;
	case 6:
	    ogEnvLog(OG_LINTERNALDEBUG, "\t\tglRasterPos4i(%d,%d,%d,%d)\n", (GLint)x, (GLint)y, (GLint)z, (GLint)w);
	    glRasterPos4i((GLint)x, (GLint)y, (GLint)z, (GLint)w);
	break;
	case 7:
	    ogEnvLog(OG_LINTERNALDEBUG, "\t\tglRasterPos4s(%d,%d,%d,%d)\n", (int)(GLshort)x, (int)(GLshort)y, (int)(GLshort)z, (int)(GLshort)w);
	    glRasterPos4s((GLshort)x, (GLshort)y, (GLshort)z, (GLshort)w);
	break;

/* 3 parameter cases*/
        case 8: 
	    d[0] = x; d[1] = y; d[2] = z;
	    ogEnvLog(OG_LINTERNALDEBUG, "\t\tglRasterPos3dv(%g,%g,%g)\n",d[0], d[1], d[2]);
	    glRasterPos3dv(d);
	break;
        case 9: 
	    f[0] = (GLfloat)x; f[1] = (GLfloat)y; f[2] = (GLfloat)z;
	    ogEnvLog(OG_LINTERNALDEBUG, "\t\tglRasterPos3fv(%g,%g,%g)\n",f[0], f[1], f[2]);
	    glRasterPos3fv(f);
	break;
        case 10: 
	    i[0] = (GLint)x; i[1] = (GLint)y; i[2] = (GLint)z;
	    ogEnvLog(OG_LINTERNALDEBUG, "\t\tglRasterPos3iv(%d,%d,%d)\n",i[0], i[1], i[2]);
	    glRasterPos3iv(i);
	break;
        case 11: 
	    s[0] = (GLshort)x; s[1] = (GLshort)y; s[2] = (GLshort)z;
	    ogEnvLog(OG_LINTERNALDEBUG, "\t\tglRasterPos3sv(%d,%d,%d)\n",(int)s[0], (int)s[1], (int)s[2]);
	    glRasterPos3sv(s);
	break;
	case 12:
	    ogEnvLog(OG_LINTERNALDEBUG, "\t\tglRasterPos3d(%g,%g,%g)\n", x, y, z);
	    glRasterPos3d(x,y,z);
	break;
	case 13:
	    ogEnvLog(OG_LINTERNALDEBUG, "\t\tglRasterPos3f(%g,%g,%g)\n",(GLfloat)x, (GLfloat)y, (GLfloat)z);
	    glRasterPos3f((GLfloat)x, (GLfloat)y, (GLfloat)z);
	break;
	case 14:
	    ogEnvLog(OG_LINTERNALDEBUG, "\t\tglRasterPos3i(%d,%d,%d)\n", (GLint)x, (GLint)y, (GLint)z);
	    glRasterPos3i((GLint)x, (GLint)y, (GLint)z);
	break;
	case 15:
	    ogEnvLog(OG_LINTERNALDEBUG, "\t\tglRasterPos3s(%d,%d,%d)\n", (int)(GLshort)x, (int)(GLshort)y, (int)(GLshort)z);
	    glRasterPos3s((GLshort)x, (GLshort)y, (GLshort)z);
	break;

/* 2 parameter cases */
        case 16: 
	    d[0] = x; d[1] = y;
	    ogEnvLog(OG_LINTERNALDEBUG, "\t\tglRasterPos2dv(%g,%g)\n",d[0], d[1]);
	    glRasterPos2dv(d);
	break;
        case 17: 
	    f[0] = (GLfloat)x; f[1] = (GLfloat)y; 
	    ogEnvLog(OG_LINTERNALDEBUG, "\t\tglRasterPos2fv(%g,%g)\n",f[0], f[1]);
	    glRasterPos2fv(f);
	break;
        case 18: 
	    i[0] = (GLint)x; i[1] = (GLint)y;
	    ogEnvLog(OG_LINTERNALDEBUG, "\t\tglRasterPos2iv(%d,%d)\n",i[0], i[1]);
	    glRasterPos2iv(i);
	break;
        case 19: 
	    s[0] = (GLshort)x; s[1] = (GLshort)y;
	    ogEnvLog(OG_LINTERNALDEBUG, "\t\tglRasterPos2sv(%d,%d)\n", (int)s[0], (int)s[1]);
	    glRasterPos2sv(s);
	break;
	case 20:
	    ogEnvLog(OG_LINTERNALDEBUG, "\t\tglRasterPos2d(%g,%g)\n", x, y);
	    glRasterPos2d(x,y);
	break;
	case 21:
	    ogEnvLog(OG_LINTERNALDEBUG, "\t\tglRasterPos2f(%g,%g)\n",(GLfloat)x, (GLfloat)y);
	    glRasterPos2f((GLfloat)x, (GLfloat)y);
	break;
	case 22:
	    ogEnvLog(OG_LINTERNALDEBUG, "\t\tglRasterPos2i(%d,%d)\n", (GLint)x, (GLint)y);
	    glRasterPos2i((GLint)x, (GLint)y);
	break;
	case 23:
	    ogEnvLog(OG_LINTERNALDEBUG, "\t\tglRasterPos2s(%d,%d)\n", (int)(GLshort)x, (int)(GLshort)y);
	    glRasterPos2s((GLshort)x, (GLshort)y);
	break;
	default:
	    ogEnvLog(OG_LINTERNALERROR,"\t\tbad random number %d\n",rand_int);
	break;
   }
}

GLvoid RasterPosRandv (GLdouble *d_array)
{
    RasterPosRand(d_array[0], d_array[1], d_array[2], d_array[3]);
}

static GLvoid RasterPosErrors(GLvoid)
{
   GLenum 	error;
   GLdouble c_array[4], c_value;
   GLdouble default_index, default_color[4], default_pos[4], 
			default_tex[4];
   GLboolean default_valid, c_valid;

   ogEnvLog(OG_LINTERNALDEBUG,"\tRasterPosErrors()\n"); 

   /* Check default values */
   glGetDoublev(GL_CURRENT_RASTER_INDEX, &default_index);
   glGetDoublev(GL_CURRENT_RASTER_COLOR, default_color);
   glGetDoublev(GL_CURRENT_RASTER_TEXTURE_COORDS, default_tex);
   glGetDoublev(GL_CURRENT_RASTER_POSITION, default_pos);
   glGetBooleanv(GL_CURRENT_RASTER_POSITION_VALID, &default_valid);

   if(default_index != 1)
     ogEnvLog(OG_LFAIL,"Initial raster index is %.2f, expected 1\n", 
		default_index);

    if ( !approx_eq(default_color[0], 1.0) ||
         !approx_eq(default_color[1], 1.0) ||
         !approx_eq(default_color[2], 1.0) ||
         !approx_eq(default_color[3], 1.0))

     ogEnvLog(OG_LFAIL,
		"Initial raster color is (%.2f,%.2f,%.2f,%.2f), expected (1,1,1,1)\n",
		default_color[0], default_color[1], default_color[2],
		default_color[3]);

   if( (default_tex[0] != 0) || (default_tex[1] != 0) ||
        (default_tex[2] != 0) || (default_tex[3] != 1) )
     ogEnvLog(OG_LFAIL,
		"Initial raster texture coords is (%.2f,%.2f,%.2f,%.2f), expected (0,0,0,1)\n",
		default_tex[0], default_tex[1], default_tex[2], default_tex[3]);

   if( (default_pos[0] != 0) || (default_pos[1] != 0) ||
        (default_pos[2] != 0) || (default_pos[3] != 1) )
     ogEnvLog(OG_LFAIL,
		"Initial raster position is (%.2f,%.2f,%.2f,%.2f), expected (0,0,0,1)\n",
		default_pos[0], default_pos[1], default_pos[2], default_pos[3]);

   if(default_valid != GL_TRUE)
     ogEnvLog(OG_LFAIL,"Initial raster position valid is %d, expected 1\n", 
		default_valid);

   /* Test glRaterPos called between glBegin and glEnd 
	  which should not change values */
   glBegin(GL_POINTS);
       RasterPosRand(1,2,3,4);		/* random pick up a glRasterPos */
   glEnd();

   error = glGetError(); 
   if(error != GL_INVALID_OPERATION)
     ogEnvLog(OG_LFAIL,
	 "glRasterPos between glBegin/glEnd doesn't cause GL_INVALID_OPERATION\n"); 

   /* verify if the RasterPos values have been destroyed */
   glGetDoublev(GL_CURRENT_RASTER_INDEX, &c_value);
   if(c_value != default_index)
     ogEnvLog(OG_LFAIL, 
	 "glRasterPos between glBegin/glEnd changes raster index from %f to %f\n",
                default_index, c_value);

   glGetDoublev(GL_CURRENT_RASTER_COLOR, c_array);
   if( (c_array[0] != default_color[0]) || (c_array[1] != default_color[1]) ||
       (c_array[2] != default_color[2]) || (c_array[3] != default_color[3]) )
     ogEnvLog(OG_LFAIL,
     "glRasterPos between glBegin/glEnd changes raster color from (%f,%f,%f,%f) to (%f,%f,%f,%f)\n",
     default_color[0], default_color[1], default_color[2], default_color[3],
     c_array[0], c_array[1], c_array[2], c_array[3]);

   glGetDoublev(GL_CURRENT_RASTER_TEXTURE_COORDS, c_array);
   if( (c_array[0] != default_tex[0]) || (c_array[1] != default_tex[1]) ||
       (c_array[2] != default_tex[2]) || (c_array[3] != default_tex[3]) )
     ogEnvLog(OG_LFAIL,
     "glRasterPos between glBegin/glEnd changes texture coords from (%f,%f,%f,%f) to (%f,%f,%f,%f)\n",
     default_tex[0], default_tex[1], default_tex[2], default_tex[3],
     c_array[0], c_array[1], c_array[2], c_array[3]);

   glGetDoublev(GL_CURRENT_RASTER_POSITION, c_array);
   if( (c_array[0] != default_pos[0]) || (c_array[1] != default_pos[1]) ||
       (c_array[2] != default_pos[2]) || (c_array[3] != default_pos[3]) )
     ogEnvLog(OG_LFAIL,
     "glRasterPos between glBegin/glEnd changes raster position from (%f,%f,%f,%f) to (%f,%f,%f,%f)\n",
     default_pos[0], default_pos[1], default_pos[2], default_pos[3],
     c_array[0], c_array[1], c_array[2], c_array[3]);

   glGetBooleanv(GL_CURRENT_RASTER_POSITION_VALID, &c_valid);
   if(c_valid != default_valid)
     ogEnvLog(OG_LFAIL, 
	 "glRasterPos between glBegin/glEnd changes raster position valid from %d to %d\n",
     default_valid, c_valid);
}

static GLvoid RasterPosTypical(GLvoid)
{
    GLdouble id_array[4] = {0.25, 0.25, -0.25, 2.0};
    GLdouble oned_array[4] = {1.0,1.0,1.0,1.0};
    GLdouble zerod_array[4] = {0,0,0,0};
    GLdouble tid_array[4]; /* transformed input array */
    GLdouble od_array[4], default_color[4];
    GLdouble od_index, default_index;
    
    GLboolean rgba_mode, index_mode;
    GLdouble vp[4]; /* view port */
    GLdouble dr[2] = {0.0, 0.0} ; /* depth range */
    GLboolean crp_valid;
    GLdouble L=-1, R=1, B=-1, T=1, N=0.1, F=1.1;

    ogEnvLog(OG_LINTERNALDEBUG,"\tRasterPosTypical()\n");
    rgba_mode = ogEnvCurVisualInfo(GLX_RGBA);
    index_mode = !rgba_mode;
    glGetDoublev(GL_CURRENT_RASTER_INDEX, &default_index);
    glGetDoublev(GL_CURRENT_RASTER_COLOR, default_color);

/* test using Ortho projection */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(L,R,B,T,N,F);
    crp_valid = GL_FALSE;
    glColor4dv(zerod_array);
    glIndexdv(zerod_array);
    glTexCoord4dv(zerod_array);
    RasterPosRandv(id_array);
    glFinish();
    glGetBooleanv(GL_CURRENT_RASTER_POSITION_VALID, &crp_valid);
    if(GL_TRUE != crp_valid)
	ogEnvLog(OG_LFAIL,"GL_CURRENT_RASTER_POSITION_VALID = GL_FALSE\n");
    
    if(GL_TRUE == rgba_mode) {
        ogEnvLog(OG_LINTERNALDEBUG,"\trgba mode\n");
    	glGetDoublev(GL_CURRENT_RASTER_INDEX, &od_index);
	if(od_index != default_index)
	     ogEnvLog(OG_LFAIL, "In rgba_mode, GL_CURRENT_RASTER_INDEX, changes but it shouldn't\n");
    	glGetDoublev(GL_CURRENT_RASTER_COLOR, od_array);
    	if(!(approx_eq(zerod_array[0], od_array[0])
      	  && approx_eq(zerod_array[1], od_array[1])
      	  && approx_eq(zerod_array[2], od_array[2])
      	  && approx_eq(zerod_array[3], od_array[3]) ))
	     ogEnvLog(OG_LFAIL,"expected CURRENT_RASTER_COLOR to be all 0's\n"); 
    } else if(GL_TRUE == index_mode) {
        ogEnvLog(OG_LINTERNALDEBUG,"\tindex mode\n");
    	glGetDoublev(GL_CURRENT_RASTER_INDEX, &od_index);
	if(od_index != zerod_array[0])
	     ogEnvLog(OG_LFAIL,"CURRENT_RASTER_INDEX %g != 0\n",od_index);
	
    	glGetDoublev(GL_CURRENT_RASTER_COLOR, od_array);
    	if(!(approx_eq(default_color[0], od_array[0])
      	  && approx_eq(default_color[1], od_array[1])
      	  && approx_eq(default_color[2], od_array[2])
      	  && approx_eq(default_color[3], od_array[3]) ))
	     ogEnvLog(OG_LFAIL, "GL_CURRENT_RASTER_COLOR isn't all 0's\n");
    } else {
	ogEnvLog(OG_LFAIL, "unknown mode\n");
    }
    
    od_array[0] = 1;
    glGetDoublev(GL_CURRENT_RASTER_TEXTURE_COORDS, od_array);
    if(!(approx_eq(zerod_array[0], od_array[0])
      && approx_eq(zerod_array[1], od_array[1])
      && approx_eq(zerod_array[2], od_array[2])
      && approx_eq(zerod_array[3], od_array[3]) ))
        ogEnvLog(OG_LFAIL ,"GL_CURRENT_RASTER_TEXTURE_COORDS isn't 0's\n"); 

    /* calculate expected Raster Position */
    /* simulate matrix mult p119 */
    tid_array[0] = id_array[0] * 2.0/(R-L) - id_array[3]*(R+L)/(R-L);
    tid_array[1] = id_array[1] * 2.0/(T-B) - id_array[3]*(T+B)/(T-B);
    tid_array[2] = id_array[2] *-2.0/(F-N) - id_array[3]*(F+N)/(F-N);
    tid_array[3] = id_array[3] * 1.0;

    /* simulate the perspective division */ 
    tid_array[0] /= tid_array[3];
    tid_array[1] /= tid_array[3];
    tid_array[2] /= tid_array[3];

    /* simulate viewport transformation - derived from p302 */
    glGetDoublev(GL_VIEWPORT, vp);    /* x,y,w,h */
    glGetDoublev(GL_DEPTH_RANGE, dr); /* n,f */

    tid_array[0] = (tid_array[0] + 1.0)*(vp[2]/2.0);
    tid_array[1] = (tid_array[1] + 1.0)*(vp[3]/2.0);
    tid_array[2] =((tid_array[2] + 1.0)*((dr[1]-dr[0])/2.0) + dr[0]);
    tid_array[3] =  tid_array[3];

    glGetDoublev(GL_CURRENT_RASTER_POSITION, od_array);
    if(!(approx_eq(tid_array[0], od_array[0])
      && approx_eq(tid_array[1], od_array[1])
      && approx_eq(tid_array[2], od_array[2])
      && approx_eq(tid_array[3], od_array[3]) ))
	ogEnvLog(OG_LFAIL,"GL_CURRENT_RASTER_POSITION is not where expected\n");

/* test using Perspective projection */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(L,R,B,T,N,F);
    crp_valid = GL_FALSE;
    glColor4dv(oned_array);
    glIndexdv(oned_array);
    glTexCoord4dv(oned_array);
    RasterPosRandv(id_array);
    glFinish();
    glGetBooleanv(GL_CURRENT_RASTER_POSITION_VALID, &crp_valid);
    if(GL_TRUE != crp_valid)
	ogEnvLog(OG_LFAIL,"GL_CURRENT_RASTER_POSITION_VALID = GL_FALSE\n");

    if(GL_TRUE == rgba_mode) {
        ogEnvLog(OG_LINTERNALDEBUG,"\trgba mode\n");
        glGetDoublev(GL_CURRENT_RASTER_INDEX, &od_index);
        if(od_index != default_index)
             ogEnvLog(OG_LINTERNALDEBUG, "Color isn't expected value\n");
        glGetDoublev(GL_CURRENT_RASTER_COLOR, od_array);
        if(!(approx_eq(oned_array[0], od_array[0])
          && approx_eq(oned_array[1], od_array[1])
          && approx_eq(oned_array[2], od_array[2])
          && approx_eq(oned_array[3], od_array[3]) ))
             ogEnvLog(OG_LFAIL, "CURRENT_RASTER_COLOR isn't written value\n" );
    } else if(GL_TRUE == index_mode) {
        ogEnvLog(OG_LINTERNALDEBUG,"\tindex mode\n");
        glGetDoublev(GL_CURRENT_RASTER_INDEX, &od_index);
        if(od_index != oned_array[0])
             ogEnvLog(OG_LFAIL,"CURRENT_RASTER_INDEX %g != 1\n", od_index);
        
        glGetDoublev(GL_CURRENT_RASTER_COLOR, od_array);
        if(!(approx_eq(default_color[0], od_array[0])
          && approx_eq(default_color[1], od_array[1])
          && approx_eq(default_color[2], od_array[2])
          && approx_eq(default_color[3], od_array[3]) ))
             ogEnvLog(OG_LFAIL, "CURRENT_RASTER_COLOR isn't written value\n" );
    } else {
        ogEnvLog(OG_LINTERNALDEBUG, "unknown mode\n");
    }

    od_array[0] = 0;
    glGetDoublev(GL_CURRENT_RASTER_TEXTURE_COORDS, od_array);
    if(!(approx_eq(oned_array[0], od_array[0])
      && approx_eq(oned_array[1], od_array[1])
      && approx_eq(oned_array[2], od_array[2])
      && approx_eq(oned_array[3], od_array[3]) ))
        ogEnvLog(OG_LFAIL, "GL_CURRENT_RASTER_TEXTURE_COORDS isn't expected value\n"); 

    /* calculate expected Raster position */

    /* simulate matrix mult p131 */
    tid_array[0] = id_array[0] * (2.0*N)/(R-L) + id_array[2]*(R+L)/(R-L);
    tid_array[1] = id_array[1] * (2.0*N)/(T-B) + id_array[2]*(T+B)/(T-B);
    tid_array[2] = id_array[2] *-(F+N)/(F-N)   + id_array[3]*(-2.0*F*N/(F-N));
    tid_array[3] = id_array[2] *-1.0;

    /* simulate the perspective division */
    tid_array[0] /= tid_array[3];
    tid_array[1] /= tid_array[3];
    tid_array[2] /= tid_array[3];

    /* simulate viewport transformation - derived from p302 */
    glGetDoublev(GL_VIEWPORT, vp); /* x,y,w,h */
    glGetDoublev(GL_DEPTH_RANGE, dr); /* n,f */
    tid_array[0] = (tid_array[0] + 1.0)*(vp[2]/2.0);
    tid_array[1] = (tid_array[1] + 1.0)*(vp[3]/2.0);
    tid_array[2] = (tid_array[2] + 1.0)*((dr[1]-dr[0])/2.0) + dr[0];
    tid_array[3] =  tid_array[3];

    glGetDoublev(GL_CURRENT_RASTER_POSITION, od_array);
    ogEnvLog(OG_LINTERNALDEBUG, "\tfrustum test\n");
    if(!(approx_eq(tid_array[0], od_array[0])
      && approx_eq(tid_array[1], od_array[1])
      && approx_eq(tid_array[2], od_array[2])
      && approx_eq(tid_array[3], od_array[3]) ))
	ogEnvLog(OG_LFAIL, "RasterPos should be (%g, %g, %g,%g)  but read (%g %g %g %g)\n",
	tid_array[0], tid_array[1], tid_array[2], tid_array[3],
	od_array[0], od_array[1], od_array[2], od_array[3]);
}

static GLboolean VV_clip_test_valid(GLint mode, GLdouble x, GLdouble y, GLdouble z, GLdouble w) 
{
    GLdouble id_array[4];
    GLboolean crp_valid;
    GLdouble L=-1, R=1, B=-1, T=1, N=0.1, F=1.1;

    ogEnvLog(OG_LINTERNALDEBUG,"\tVV_clip_test_valid()\n"); 

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    switch(mode) {
	case ORTHO:
    	    glOrtho(L,R,B,T,N,F);
	break;
	case FRUSTUM:
    	    glFrustum(L,R,B,T,N,F);
	break;
	default:
	    ogEnvLog(OG_LFAIL, "mode unknown in VV_clip_test_valid\n");
    }
    id_array[0] = x; id_array[1] = y; id_array[2] = z; id_array[3] = w;
    RasterPosRandv(id_array);
    glFinish();
    glGetBooleanv(GL_CURRENT_RASTER_POSITION_VALID, &crp_valid);
    return crp_valid;
}

static GLvoid VVOrthoRasterPosClipTest(GLvoid)
{
    ogEnvLog(OG_LINTERNALDEBUG,"\tOrthoRasterPosVVClipTest()\n"); 
    
    if(GL_TRUE != VV_clip_test_valid(ORTHO, 0.5, 0.5, -0.5, 1.0))
	ogEnvLog(OG_LFAIL, "Rasterpos mistakenly clipped for Ortho\n");

/* gross clipping */
    if(GL_TRUE == VV_clip_test_valid(ORTHO, 0.5, 0.5, 0, 1.0))
	ogEnvLog(OG_LFAIL,"glRasterPos doesn't near clip for Ortho\n");
    
    if(GL_TRUE == VV_clip_test_valid(ORTHO, 0.5, 0.5, -1.5, 1.0))
	ogEnvLog(OG_LFAIL,"glRasterPos doesn't far clip for Ortho\n");

    if(GL_TRUE == VV_clip_test_valid(ORTHO, -1.5, 0.5, -0.5, 1.0))
	ogEnvLog(OG_LFAIL,"glRasterPos doesn't left clip for Ortho\n");

    if(GL_TRUE == VV_clip_test_valid(ORTHO, 1.5, 0.5, -0.5, 1.0))
	ogEnvLog(OG_LFAIL,"glRasterPos doesn't right clip for Ortho\n");

    if(GL_TRUE == VV_clip_test_valid(ORTHO, 0.5, 1.5, -0.5, 1.0))
	ogEnvLog(OG_LFAIL,"glRasterPos doesn't top clip for Ortho\n");

    if(GL_TRUE == VV_clip_test_valid(ORTHO, 0.5,-1.5, -0.5, 1.0))
	ogEnvLog(OG_LFAIL,"glRasterPos doesn't bottom clip for Ortho\n");
   
/* points on clip plane should test as in */
    if(GL_FALSE == VV_clip_test_valid(ORTHO, 
				    0.5, 0.5, -0.1-ERROR_TOLERANCE, 1.0))
	ogEnvLog(OG_LFAIL, "point ON near clip plane isn't IN for Ortho\n");
    
    if(GL_FALSE == VV_clip_test_valid(ORTHO, 
				    0.5, 0.5, -1.1+ERROR_TOLERANCE, 1.0))
	ogEnvLog(OG_LFAIL, "point ON far clip plane isn't IN for Ortho\n");

    if(GL_FALSE == VV_clip_test_valid(ORTHO, 
				    -1.0+ERROR_TOLERANCE, 0.5, -0.5, 1.0))
	ogEnvLog(OG_LFAIL, "point ON left clip plane isn't IN for Ortho\n");

    if(GL_FALSE == VV_clip_test_valid(ORTHO, 
				    1.0-ERROR_TOLERANCE, 0.5, -0.5, 1.0))
	ogEnvLog(OG_LFAIL, "point ON right clip plane isn't IN for Ortho\n");

    if(GL_FALSE == VV_clip_test_valid(ORTHO, 
				    0.5, 1.0-ERROR_TOLERANCE, -0.5, 1.0))
	ogEnvLog(OG_LFAIL, "point ON top clip plane isn't IN for Ortho\n");

    if(GL_FALSE == VV_clip_test_valid(ORTHO, 
				    0.5,-1.0+ERROR_TOLERANCE, -0.5, 1.0))
	ogEnvLog(OG_LFAIL, "point ON bottom clip plane isn't IN for Ortho\n");
}

static GLvoid VVFrustumRasterPosClipTest(GLvoid)
{
    ogEnvLog(OG_LINTERNALDEBUG,"\tFrustumRasterPosClipTest()\n"); 

    if(GL_TRUE != VV_clip_test_valid(FRUSTUM, 0.5, 0.5, -0.5, 1.0))
	ogEnvLog(OG_LFAIL,"RasterPos mistakenly clipped for Frustum\n");

/* gross clipping */
    if(GL_TRUE == VV_clip_test_valid(FRUSTUM, 0, 0, 0, 1.0))
	ogEnvLog(OG_LFAIL,"glRasterPos doesn't near clip to view Frustum\n");
    
    if(GL_TRUE == VV_clip_test_valid(FRUSTUM, 0, 0, -1.5, 1.0))
	ogEnvLog(OG_LFAIL,"glRasterPos doesn't far clip to view Frustum\n");

    if(GL_TRUE == VV_clip_test_valid(FRUSTUM, -50.0, 0.5, -0.5, 1.0))
	ogEnvLog(OG_LFAIL,"glRasterPos doesn't left clip to view Frustum\n");

    if(GL_TRUE == VV_clip_test_valid(FRUSTUM, 50.0, 0.5, -0.5, 1.0))
	ogEnvLog(OG_LFAIL,"glRasterPos doesn't right clip to view Frustum\n");

    if(GL_TRUE == VV_clip_test_valid(FRUSTUM, 0.5, 50.0, -0.5, 1.0))
	ogEnvLog(OG_LFAIL,"glRasterPos doesn't top clip to view Frustum\n");

    if(GL_TRUE == VV_clip_test_valid(FRUSTUM, 0.5,-50.0, -0.5, 1.0))
	ogEnvLog(OG_LFAIL,"glRasterPos doesn't bottom clip to view Frustum\n");
   
/* points on clip plane should test as in */
    if(GL_FALSE == VV_clip_test_valid(FRUSTUM, 
				    0.5, 0.5, -0.1-ERROR_TOLERANCE, 1.0))
	ogEnvLog(OG_LFAIL, "point ON near clip plane isn't IN for Frustum\n");
    
    if(GL_FALSE == VV_clip_test_valid(FRUSTUM, 
				    0.5, 0.5, -1.1+ERROR_TOLERANCE, 1.0))
	ogEnvLog(OG_LFAIL, "point ON far clip plane isn't IN for Frustum\n");

    if(GL_FALSE == VV_clip_test_valid(FRUSTUM, 
				    -1.0+ERROR_TOLERANCE, 0.5, -0.5, 1.0))
	ogEnvLog(OG_LFAIL, "point ON left clip plane isn't IN for Frustum\n");

    if(GL_FALSE == VV_clip_test_valid(FRUSTUM, 
				    1.0-ERROR_TOLERANCE, 0.5, -0.5, 1.0))
	ogEnvLog(OG_LFAIL, "point ON right clip plane isn't IN for Frustum\n");

    if(GL_FALSE == VV_clip_test_valid(FRUSTUM, 
				    0.5, 1.0-ERROR_TOLERANCE, -0.5, 1.0))
	ogEnvLog(OG_LFAIL, "point ON top clip plane isn't IN for Frustum\n");

    if(GL_FALSE == VV_clip_test_valid(FRUSTUM, 
				    0.5,-1.0+ERROR_TOLERANCE, -0.5, 1.0))
	ogEnvLog(OG_LFAIL, "point ON bottom clip plane isn't IN for Frustum\n");
}

static GLboolean arb_clip_test_valid(GLint mode, GLdouble x, GLdouble y, GLdouble z, GLdouble w) 
{
    GLdouble id_array[4];
    GLboolean crp_valid;
    GLdouble L=-1, R=1, B=-1, T=1, N=0.1, F=1.1;
    GLdouble cp0[4] = { 1, 0, 0, 0.5};	/* in if x >= -0.5 */
    GLdouble cp1[4] = {-1, 0, 0, 0.5};	/* in if x <=  0.5 */
    GLdouble cp2[4] = { 0, 1, 0, 0.5};	/* in if y >= -0.5 */
    GLdouble cp3[4] = { 0,-1, 0, 0.5};	/* in if y <=  0.5 */
    GLdouble cp4[4] = { 0, 0, 1, 0.75};	/* in if z >= -0.75 */
    GLdouble cp5[4] = { 0, 0,-1,-0.25};	/* in if z <= -0.25 */

    ogEnvLog(OG_LINTERNALDEBUG,"\tarb_clip_test_valid()\n"); 

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    switch(mode) {
	case ORTHO:
    	    glOrtho(L,R,B,T,N,F);
	break;
	case FRUSTUM:
    	    glFrustum(L,R,B,T,N,F);
	break;
	default:
	    ogEnvLog(OG_LFAIL, "mode unknown in arb_clip_test_valid\n");
    }
    glClipPlane(GL_CLIP_PLANE0,cp0); glEnable(GL_CLIP_PLANE0);
    glClipPlane(GL_CLIP_PLANE1,cp1); glEnable(GL_CLIP_PLANE1);
    glClipPlane(GL_CLIP_PLANE2,cp2); glEnable(GL_CLIP_PLANE2);
    glClipPlane(GL_CLIP_PLANE3,cp3); glEnable(GL_CLIP_PLANE3);
    glClipPlane(GL_CLIP_PLANE4,cp4); glEnable(GL_CLIP_PLANE4); 
    glClipPlane(GL_CLIP_PLANE5,cp5); glEnable(GL_CLIP_PLANE5);
    id_array[0] = x; id_array[1] = y; id_array[2] = z; id_array[3] = w;
    RasterPosRandv(id_array);
    glFinish();
    glGetBooleanv(GL_CURRENT_RASTER_POSITION_VALID, &crp_valid);
    return crp_valid;
}

static GLvoid ArbOrthoRasterPosClipTest(GLvoid)
{
    ogEnvLog(OG_LINTERNALDEBUG,"\tArbOrthoRasterPosClipTest()\n"); 

    if(GL_TRUE != arb_clip_test_valid(ORTHO, 0, 0, -0.5, 1))
	ogEnvLog(OG_LFAIL, "Rasterpos mistakenly clipped for arbitrary Ortho\n");

/* gross clipping */
    if(GL_TRUE == arb_clip_test_valid(ORTHO, 0, 0, -0.125, 1.0))
	ogEnvLog(OG_LFAIL, "glRasterPos doesn't near arbitrary clip for Ortho\n");

    if(GL_TRUE == arb_clip_test_valid(ORTHO, 0, 0, -.85, 1.0))
	ogEnvLog(OG_LFAIL, "glRasterPos doesn't far arbitrary clip for Ortho\n");

    if(GL_TRUE == arb_clip_test_valid(ORTHO, -0.75, 0, -0.5, 1.0))
	ogEnvLog(OG_LFAIL, "glRasterPos doesn't left arbitrary clip for Ortho\n");

    if(GL_TRUE == arb_clip_test_valid(ORTHO, 0.75, 0, -0.5, 1.0))
	ogEnvLog(OG_LFAIL, "glRasterPos doesn't right arbitrary clip for Ortho\n");

    if(GL_TRUE == arb_clip_test_valid(ORTHO, 0, 0.75, -0.5, 1.0))
	ogEnvLog(OG_LFAIL, "glRasterPos doesn't top arbitrary clip for Ortho\n");

    if(GL_TRUE == arb_clip_test_valid(ORTHO, 0,-0.75, -0.5, 1.0))
	ogEnvLog(OG_LFAIL, "glRasterPos doesn't bottom arbitrary clip for Ortho\n");
   
/* points on clip plane should test as in */
    if(GL_FALSE == arb_clip_test_valid(ORTHO, 
				    0, 0, -0.25-ERROR_TOLERANCE, 1.0))
	ogEnvLog(OG_LFAIL, "point on arbitrary near clip plane isn't IN for Ortho\n");
    
    if(GL_FALSE == arb_clip_test_valid(ORTHO, 
				    0, 0, -0.75+ERROR_TOLERANCE, 1.0))
	ogEnvLog(OG_LFAIL, "point on arbitrary far clip plane isn't IN for Ortho\n");

    if(GL_FALSE == arb_clip_test_valid(ORTHO, 
				    -0.5+ERROR_TOLERANCE, 0, -0.5, 1.0))
	ogEnvLog(OG_LFAIL, "point on arbitrary left clip plane isn't IN for Ortho\n");

    if(GL_FALSE == arb_clip_test_valid(ORTHO, 
				    0.5-ERROR_TOLERANCE, 0, -0.5, 1.0))
	ogEnvLog(OG_LFAIL, "point on arbitrary right clip plane isn't IN for Ortho\n");

    if(GL_FALSE == arb_clip_test_valid(ORTHO, 
				    0, 0.5-ERROR_TOLERANCE, -0.5, 1.0))
	ogEnvLog(OG_LFAIL, "point on arbitrary top clip plane isn't IN for Ortho\n");

    if(GL_FALSE == arb_clip_test_valid(ORTHO, 
				    0 , -0.5+ERROR_TOLERANCE, -0.5, 1.0))
	ogEnvLog(OG_LFAIL, "point on arbitrary bottom clip plane isn't IN for Ortho\n");
}

static GLvoid ArbFrustumRasterPosClipTest(GLvoid)
{
    ogEnvLog(OG_LINTERNALDEBUG,"\tArbFrustumRasterPosClipTest()\n"); 

    if(GL_TRUE != arb_clip_test_valid(FRUSTUM, 0, 0, -0.5, 1))
	ogEnvLog(OG_LFAIL, "Rasterpos mistakenly clipped for arbitrary Frustum\n");

/* gross clipping */
    if(GL_TRUE == arb_clip_test_valid(FRUSTUM, 0, 0, -0.125, 1.0))
	ogEnvLog(OG_LFAIL, "glRasterPos doesn't near arbitrary clip for Frustum\n");

    if(GL_TRUE == arb_clip_test_valid(FRUSTUM, 0, 0, -.85, 1.0))
	ogEnvLog(OG_LFAIL, "glRasterPos doesn't far arbitrary clip for Frustum\n");

    if(GL_TRUE == arb_clip_test_valid(FRUSTUM, -0.75, 0, -0.5, 1.0))
	ogEnvLog(OG_LFAIL, "glRasterPos doesn't left arbitrary clip for Frustum\n");

    if(GL_TRUE == arb_clip_test_valid(FRUSTUM, 0.75, 0, -0.5, 1.0))
	ogEnvLog(OG_LFAIL, "glRasterPos doesn't right arbitrary clip for Frustum\n");

    if(GL_TRUE == arb_clip_test_valid(FRUSTUM, 0, 0.75, -0.5, 1.0))
	ogEnvLog(OG_LFAIL, "glRasterPos doesn't top arbitrary clip for Frustum\n");

    if(GL_TRUE == arb_clip_test_valid(FRUSTUM, 0,-0.75, -0.5, 1.0))
	ogEnvLog(OG_LFAIL, "glRasterPos doesn't bottom arbitrary clip for Frustum\n");

/* points on clip plane should test as in */
    if(GL_FALSE == arb_clip_test_valid(FRUSTUM, 
					0, 0, -0.25-ERROR_TOLERANCE, 1.0))
	ogEnvLog(OG_LFAIL, "point on arbitrary near clip plane isn't IN for Frustum\n");

    if(GL_FALSE == arb_clip_test_valid(FRUSTUM, 
					0, 0, -0.75+ERROR_TOLERANCE, 1.0))
	ogEnvLog(OG_LFAIL, "point on arbitrary far clip plane isn't IN for Frustum\n");

    if(GL_FALSE == arb_clip_test_valid(FRUSTUM, 
					-0.5+ERROR_TOLERANCE, 0, -0.5, 1.0))
	ogEnvLog(OG_LFAIL, "point on arbitrary left clip plane isn't IN for Frustum\n");

    if(GL_FALSE == arb_clip_test_valid(FRUSTUM, 
					0.5-ERROR_TOLERANCE, 0, -0.5, 1.0))
	ogEnvLog(OG_LFAIL, "point on arbitrary right clip plane isn't IN for Frustum\n");

    if(GL_FALSE == arb_clip_test_valid(FRUSTUM, 
					0, 0.5-ERROR_TOLERANCE, -0.5, 1.0))
	ogEnvLog(OG_LFAIL, "point on arbitrary top clip plane isn't IN for Frustum\n");

    if(GL_FALSE == arb_clip_test_valid(FRUSTUM, 
					0 , -0.5+ERROR_TOLERANCE, -0.5, 1.0))
	ogEnvLog(OG_LFAIL, "point on arbitrary bottom clip plane isn't IN for Frustum\n");
}

CLEANUP (RasterPos)
{
    GLdouble cp[4] = { 0, 0, 0, 0};
    glPopAttrib();
    glEnable(GL_DITHER);
    glClipPlane(GL_CLIP_PLANE0,cp); glDisable(GL_CLIP_PLANE0);
    glClipPlane(GL_CLIP_PLANE1,cp); glDisable(GL_CLIP_PLANE1);
    glClipPlane(GL_CLIP_PLANE2,cp); glDisable(GL_CLIP_PLANE2);
    glClipPlane(GL_CLIP_PLANE3,cp); glDisable(GL_CLIP_PLANE3);
    glClipPlane(GL_CLIP_PLANE4,cp); glDisable(GL_CLIP_PLANE4); 
    glClipPlane(GL_CLIP_PLANE5,cp); glDisable(GL_CLIP_PLANE5);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    ogLibSetDefaultRasterPos();
}

/*ARGSUSED*/
TESTMOD (RasterPos)
{
    
    ogEnvLog(OG_LINTERNALDEBUG,"RasterPosTest\n"); 
    srand(1);                   /* make repeatable random numbers's */

    glPushAttrib(GL_CURRENT_BIT);
    glDisable(GL_DITHER);
    RasterPosErrors();		/* test error conditions */
    RasterPosTypical();			/* test simple case (not clipped) */
    VVOrthoRasterPosClipTest();		/* view volume clipping */
    VVFrustumRasterPosClipTest();	/* view volume clipping */
    ArbOrthoRasterPosClipTest();	/* arbitrary clipping */
    ArbFrustumRasterPosClipTest();	/* arbitrary clipping */
}
