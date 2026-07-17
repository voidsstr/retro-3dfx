/**************************************************************************
 *									  *
 * 		 Copyright (C) 1989, Silicon Graphics, Inc.		  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *									  *
 **************************************************************************/

/* set.c - $Revision: 2$ */

#include "ogtst.h"

static float maxValue(GLenum);

/*************************************************************
*  ogLibSetVertex()  -  set co-ord position
*************************************************************/
int 
ogLibSetVertex(int x, int y, int z, int w)
{
    int n;
    short ts[4];
    int ti[4];
    float tf[4];
    double td[4];

    ts[0] = (short) x;
    ts[1] = (short) y;
    ts[2] = (short) z;
    ts[3] = (short) w;
    ti[0] = (int) x;
    ti[1] = (int) y;
    ti[2] = (int) z;
    ti[3] = (int) w;
    tf[0] = (float) x;
    tf[1] = (float) y;
    tf[2] = (float) z;
    tf[3] = (float) w;
    td[0] = (double) x;
    td[1] = (double) y;
    td[2] = (double) z;
    td[3] = (double) w;

    switch (n = ogLibIntRand(0,23)) {
    case 0:
	glVertex2sv(ts);
	ogEnvLog(1, "glVertex2sv(%d,%d)      \n", ts[0], ts[1]);
	break;
    case 1:
	glVertex2iv(ti);
	ogEnvLog(1, "glVertex2iv(%d,%d)      \n", ti[0], ti[1]);
	break;
    case 2:
	glVertex2fv(tf);
	ogEnvLog(1, "glVertex2fv(%f,%f)      \n", tf[0], tf[1]);
	break;
    case 3:
	glVertex2dv(td);
	ogEnvLog(1, "glVertex2dv(%f,%f)      \n", td[0], td[1]);
	break;

    case 4:
	glVertex2s(ts[0],ts[1]);
	ogEnvLog(1, "glVertex2s(%d,%d)      \n", ts[0], ts[1]);
	break;
    case 5:
	glVertex2i(ti[0],ti[1]);
	ogEnvLog(1, "glVertex2i(%d,%d)      \n", ti[0], ti[1]);
	break;
    case 6:
	glVertex2f(tf[0],tf[1]);
	ogEnvLog(1, "glVertex2f(%f,%f)      \n", tf[0], tf[1]);
	break;
    case 7:
	glVertex2d(td[0],td[1]);
	ogEnvLog(1, "glVertex2d(%f,%f)      \n", td[0], td[1]);
	break;

    case 8:
	glVertex3sv(ts);
	ogEnvLog(1, "glVertex3sv(%d,%d,%d)   \n", ts[0], ts[1], ts[2]);
	break;
    case 9:
	glVertex3iv(ti);
	ogEnvLog(1, "glVertex3iv(%d,%d,%d)   \n", ti[0], ti[1], ti[2]);
	break;
    case 10:
	glVertex3fv(tf);
	ogEnvLog(1, "glVertex3fv(%f,%f,%f)   \n", tf[0], tf[1], tf[2]);
	break;
    case 11:
	glVertex3dv(td);
	ogEnvLog(1, "glVertex3dv(%f,%f,%f)   \n", td[0], td[1], td[2]);
	break;

    case 12:
	glVertex3s(ts[0],ts[1],ts[2]);
	ogEnvLog(1, "glVertex3s(%d,%d,%d)   \n", ts[0], ts[1], ts[2]);
	break;
    case 13:
	glVertex3i(ti[0],ti[1],ti[2]);
	ogEnvLog(1, "glVertex3i(%d,%d,%d)   \n", ti[0], ti[1], ti[2]);
	break;
    case 14:
	glVertex3f(tf[0],tf[1],tf[2]);
	ogEnvLog(1, "glVertex3f(%f,%f,%f)   \n", tf[0], tf[1], tf[2]);
	break;
    case 15:
	glVertex3d(td[0],td[1],td[2]);
	ogEnvLog(1, "glVertex3d(%f,%f,%f)   \n", td[0], td[1], td[2]);
	break;

    case 16:
	glVertex4sv(ts);
	ogEnvLog(1, "glVertex4sv(%d,%d,%d,%d)\n", ts[0], ts[1], ts[2], ts[3]);
	break;
    case 17:
	glVertex4iv(ti);
	ogEnvLog(1, "glVertex4iv(%d,%d,%d,%d)\n", ti[0], ti[1], ti[2], ti[3]);
	break;
    case 18:
	glVertex4fv(tf);
	ogEnvLog(1, "glVertex4fv(%f,%f,%f,%f)\n", tf[0], tf[1], tf[2], tf[3]);
	break;
    case 19:
	glVertex4dv(td);
	ogEnvLog(1, "glVertex4dv(%f,%f,%f,%f)\n", td[0], td[1], td[2], td[3]);
	break;

    case 20:
	glVertex4s(ts[0],ts[1],ts[2],ts[3]);
	ogEnvLog(1, "glVertex4s(%d,%d,%d,%d)\n", ts[0], ts[1], ts[2], ts[3]);
	break;
    case 21:
	glVertex4i(ti[0],ti[1],ti[2],ti[3]);
	ogEnvLog(1, "glVertex4i(%d,%d,%d,%d)\n", ti[0], ti[1], ti[2], ti[3]);
	break;
    case 22:
	glVertex4f(tf[0],tf[1],tf[2],tf[3]);
	ogEnvLog(1, "glVertex4f(%f,%f,%f,%f)\n", tf[0], tf[1], tf[2], tf[3]);
	break;
    case 23:
	glVertex4d(td[0],td[1],td[2],td[3]);
	ogEnvLog(1, "glVertex4d(%f,%f,%f,%f)\n", td[0], td[1], td[2], td[3]);
	break;

    default:
        ogEnvLog(OG_LINTERNALERROR,"ogLibSetVertex()\n");
        break;
    }

    /* return dimension */
    return (2 + (n >> 3));
}

/*************************************************************
*  ogLibSetColor()  -  set color
*************************************************************/
int ogLibSetColor(int r,int g,int b,int a)
{
   int mode;
   GLshort cs[4];
   GLushort cus[4];
   GLint ci[4];
   GLuint cui[4];
   GLfloat cf[4];
   GLdouble cd[4];
   GLbyte cb[4];
   GLubyte cub[4];

   if (ogEnvIsCIMode())
     switch (ogLibBitRand(3)) {
       case 0:
         glIndexs((GLshort)r);
         ogEnvLog(1, "glIndexs(0x%x)\n", r);
         break;
       case 1:
         glIndexi((GLint)r);
         ogEnvLog(1, "glIndexi(0x%x)\n", r);
         break;
       case 2:
         glIndexf((GLfloat)r);
         ogEnvLog(1, "glIndexf(0x%x)\n", r);
         break;
       case 3:
         glIndexd((GLdouble) r);
         ogEnvLog(1, "glIndexd(0x%x)\n", r);
         break;
       case 4:
	 cs[0] = (GLshort)r;
         glIndexsv(cs);
         ogEnvLog(1, "glIndexsv(0x%x)\n", r);
         break;
       case 5:
	 ci[0] = (GLint)r;
         glIndexiv(ci);
         ogEnvLog(1, "glIndexiv(0x%x)\n", r);
         break;
       case 6:
	 cf[0] = (GLfloat)r;
         glIndexfv(cf);
         ogEnvLog(1, "glIndexfv(0x%x)\n", r);
         break;
       case 7:
	 cd[0] = (GLdouble)r;
         glIndexdv(cd);
         ogEnvLog(1, "glIndexdv(0x%x)\n", r);
         break;
       default:
         ogEnvLog(OG_LINTERNALERROR,"ogLibSetColor()\n");
         break;
       }
   else  {
     cub[0] = (GLubyte) r;
     cub[1] = (GLubyte) g;
     cub[2] = (GLubyte) b;
     cub[3] = (GLubyte) a;

     cb[0] = (GLbyte) r / 2;
     cb[1] = (GLbyte) g / 2;
     cb[2] = (GLbyte) b / 2;
     cb[3] = (GLbyte) a / 2;

     cus[0] = (GLushort) (r * 65535) / 255;
     cus[1] = (GLushort) (g * 65535) / 255;
     cus[2] = (GLushort) (b * 65535) / 255;
     cus[3] = (GLushort) (a * 65535) / 255;
 
     cs[0] = (GLshort) (r * 32767) / 255;
     cs[1] = (GLshort) (g * 32767) / 255;
     cs[2] = (GLshort) (b * 32767) / 255;
     cs[3] = (GLshort) (a * 32767) / 255;

     cui[0] = (GLuint) r << 24;
     cui[1] = (GLuint) g << 24;
     cui[2] = (GLuint) b << 24;
     cui[3] = (GLuint) a << 24;

     ci[0] = (GLint) r << 23;
     ci[1] = (GLint) g << 23;
     ci[2] = (GLint) b << 23;
     ci[3] = (GLint) a << 23;

     cf[0] = (float) r / 255.0;
     cf[1] = (float) g / 255.0;
     cf[2] = (float) b / 255.0;
     cf[3] = (float) a / 255.0;

     cd[0] = (double) r / 255.0;
     cd[1] = (double) g / 255.0;
     cd[2] = (double) b / 255.0;
     cd[3] = (double) a / 255.0;
 
     if (a == 0xff)
       mode = ogLibBitRand(5); 
     else mode = 16 + ogLibBitRand(4); 

     switch (mode) {
       case 0:
         glColor3b(cb[0], cb[1], cb[2]);
         ogEnvLog(1, "glColor3b(0x%x,0x%x,0x%x)\n", cb[0], cb[1], cb[2]);
         break;
       case 1:
         glColor3ub(cub[0], cub[1], cub[2]);
         ogEnvLog(1, "glColor3ub(0x%x,0x%x,0x%x)\n", cub[0], cub[1], cub[2]);
         break;
       case 2:
         glColor3s(cs[0], cs[1], cs[2]);
         ogEnvLog(1, "glColor3s(0x%x,0x%x,0x%x)\n", cs[0], cs[1], cs[2]);
         break;
       case 3:
         glColor3us(cus[0], cus[1], cus[2]);
         ogEnvLog(1, "glColor3us(0x%x,0x%x,0x%x)\n", cus[0], cus[1], cus[2]);
         break;
       case 4:
         glColor3i(ci[0], ci[1], ci[2]);
         ogEnvLog(1, "glColor3i(0x%x,0x%x,0x%x)\n", ci[0], ci[1], ci[2]);
         break;
       case 5:
         glColor3ui(cui[0], cui[1], cui[2]);
         ogEnvLog(1, "glColor3ui(0x%x,0x%x,0x%x)\n", cui[0], cui[1], cui[2]);
         break;
       case 6:
         glColor3f(cf[0], cf[1], cf[2]);
         ogEnvLog(1, "glColor3f(%f,%f,%f)\n", cf[0], cf[1], cf[2]);
         break;
       case 7:
         glColor3d(cd[0], cd[1], cd[2]);
         ogEnvLog(1, "glColor3d(%f,%f,%f)\n", cd[0], cd[1], cd[2]);
         break;
 
       case 8:
         glColor3bv(cb);
         ogEnvLog(1, "glColor3bv(0x%x,0x%x,0x%x)\n", cb[0], cb[1], cb[2]);
         break;
       case 9:
         glColor3ubv(cub);
         ogEnvLog(1, "glColor3ubv(0x%x,0x%x,0x%x)\n", cub[0], cub[1], cub[2]);
         break;
       case 10:
         glColor3sv(cs);
         ogEnvLog(1, "glColor3sv(0x%x,0x%x,0x%x)\n", cs[0], cs[1], cs[2]);
         break;
       case 11:
         glColor3usv(cus);
         ogEnvLog(1, "glColor3usv(0x%x,0x%x,0x%x)\n", cus[0], cus[1], cus[2]);
         break;
       case 12:
         glColor3iv(ci);
         ogEnvLog(1, "glColor3iv(0x%x,0x%x,0x%x)\n", ci[0], ci[1], ci[2]);
         break;
       case 13:
         glColor3uiv(cui);
         ogEnvLog(1, "glColor3uiv(0x%x,0x%x,0x%x)\n", cui[0], cui[1], cui[2]);
         break;
       case 14:
         glColor3fv(cf);
         ogEnvLog(1, "glColor3fv(%f,%f,%f)\n", cf[0], cf[1], cf[2]);
         break;
       case 15:
         glColor3dv(cd);
         ogEnvLog(1, "glColor3dv(%f,%f,%f)\n", cd[0], cd[1], cd[2]);
         break;
 
       case 16:
         glColor4b(cb[0], cb[1], cb[2], cb[3]);
         ogEnvLog(1, "glColor4b(0x%x,0x%x,0x%x,0x%x)\n", cb[0], cb[1], cb[2], cb[3]);
         break;
       case 17:
         glColor4ub(cub[0], cub[1], cub[2], cub[3]);
         ogEnvLog(1, "glColor4ub(0x%x,0x%x,0x%x,0x%x)\n", cub[0], cub[1], cub[2], cub[3]);
         break;
       case 18:
         glColor4s(cs[0], cs[1], cs[2], cs[3]);
         ogEnvLog(1, "glColor4s(0x%x,0x%x,0x%x,0x%x)\n", cs[0], cs[1], cs[2], cs[3]);
         break;
       case 19:
         glColor4us(cus[0], cus[1], cus[2], cus[3]);
         ogEnvLog(1, "glColor4us(0x%x,0x%x,0x%x,0x%x)\n", cus[0], cus[1], cus[2], cus[3]);
         break;
       case 20:
         glColor4i(ci[0], ci[1], ci[2], ci[3]);
         ogEnvLog(1, "glColor4i(0x%x,0x%x,0x%x,0x%x)\n", ci[0], ci[1], ci[2], ci[3]);
         break;
       case 21:
         glColor4ui(cui[0], cui[1], cui[2], cui[3]);
         ogEnvLog(1, "glColor4ui(0x%x,0x%x,0x%x,0x%x)\n", cui[0], cui[1], cui[2], cui[3]);
         break;
       case 22:
         glColor4f(cf[0], cf[1], cf[2], cf[3]);
         ogEnvLog(1, "glColor4f(%f,%f,%f,%f)\n", cf[0], cf[1], cf[2], cf[3]);
         break;
       case 23:
         glColor4d(cd[0], cd[1], cd[2], cd[3]);
         ogEnvLog(1, "glColor4d(%f,%f,%f,%f)\n", cd[0], cd[1], cd[2], cd[3]);
         break;
  
       case 24:
         glColor4bv(cb);
         ogEnvLog(1, "glColor4bv(0x%x,0x%x,0x%x,0x%x)\n", cb[0], cb[1], cb[2], cb[3]);
         break;
       case 25:
         glColor4ubv(cub);
         ogEnvLog(1, "glColor4ubv(0x%x,0x%x,0x%x,0x%x)\n", cub[0], cub[1], cub[2], cub[3]);
         break;
       case 26:
         glColor4sv(cs);
         ogEnvLog(1, "glColor4sv(0x%x,0x%x,0x%x,0x%x)\n", cs[0], cs[1], cs[2], cs[3]);
         break;
       case 27:
         glColor4usv(cus);
         ogEnvLog(1, "glColor4usv(0x%x,0x%x,0x%x,0x%x)\n", cus[0], cus[1], cus[2], cus[3]);
         break;
       case 28:
         glColor4iv(ci);
         ogEnvLog(1, "glColor4iv(0x%x,0x%x,0x%x,0x%x)\n", ci[0], ci[1], ci[2], ci[3]);
         break;
       case 29:
         glColor4uiv(cui);
         ogEnvLog(1, "glColor4uiv(0x%x,0x%x,0x%x,0x%x)\n", cui[0], cui[1], cui[2], cui[3]);
         break;
       case 30:
         glColor4fv(cf);
         ogEnvLog(1, "glColor4fv(%f,%f,%f,%f)\n", cf[0], cf[1], cf[2], cf[3]);
         break;
       case 31:
         glColor4dv(cd);
         ogEnvLog(1, "glColor4dv(%f,%f,%f,%f)\n", cd[0], cd[1], cd[2], cd[3]);
         break;
       default:
         ogEnvLog(OG_LINTERNALERROR,"ogLibSetColor()\n");
         break;
       }
     }
  
   return((mode<16)?3:4);
}

/*************************************************************
*  ogLibSetRead() - sets the other value, then reads the 
*  float value.  Used for later testing state set with other 
*  value against float value.
*/
void ogLibSetRead(GLvoid (*other_set)(const GLvoid *),
		  GLenum param_name, GLfloat *float_ptr, GLvoid *other_ptr)
{
    (*other_set)(other_ptr);
    glGetFloatv(param_name, float_ptr);
}


/*************************************************************
*  ogLibSetReadSet()  -  set the float value, read in using 
*  the other value, set using the other value, and read as a 
*  float.  The end result of all this is that when the parameter
*  is set using the non-float value, the get value should be
*  close to the float value.
*  float_set	--	pointer to function to set the value 
*	using a GLfloat * as an arguement (for example, 
*	glNormal3fv)
*  other_set	--	pointer to a function to set the value
* 	using a GLother * as an arguement (for example,
*	glNormal3sv)
*  param_name	--	parameter name to be used in glGet calls
*	(for example, GL_CURRENT_NORMAL)
*  other_type	--	type name of other pointer
*	(for example, GL_SHORT)
*  float_ptr	--	as input, points to desired value as a 
*	float.  as output, points to a value which was the result of
*	setting using the other type.
*  other_ptr	--	(output only) points to the value which
*	is the equivalent of float_ptr using the given set and get
*	commands.
*  ncomponents -- the size of the arrays (max == 4) (for example, 3)
*  scale -- should the values be scaled to be between 0 or -1 and 1 for floats
*	and doubles and between 0 or the most negative value and the
*	most positive value for other types.  This paramter should be TRUE
*	for normals and colors (for example), but FALSE for texture coordinates
* 	(for example).  
*  Function returns TRUE if the input and output values of float_set
*  are exactly equal, FALSE otherwise.
*************************************************************/
int ogLibSetReadSet(GLvoid (*float_set)(const GLfloat *),
		    GLvoid (*other_set)(const GLvoid *),
		    GLenum param_name, GLenum other_type,
		    GLfloat *float_ptr, GLvoid *other_ptr, GLint ncomponents,
		    GLboolean scale)
{
    GLfloat tmp[4];
    int special_type;
    int i;

    if (ncomponents < 1 || ncomponents > 4) {
	ogEnvLog(OG_LINTERNALERROR, 
		 "ogLibSetReadSet():  invalid ncomponents:  %d\n", 
		 ncomponents);
	return 0;
    }

    /* copy the float value into the temporay buffer -- will be used to
     * determine the return value at the end of the function */
    for (i = 0; i < ncomponents; i++) {
	tmp[i] = float_ptr[i];
    }

    /* set the parameter using the float value */
    (*float_set)(float_ptr);

    /* read the parameter back in using the other type */
    /* special_type == 1 if the paramter can be read in other_type.  if this
     * is not the case, some conversion will have to happen at the end. */
    special_type = 1;
    switch(other_type) {
#ifdef GL_DOUBLE_EXT
      case GL_DOUBLE_EXT:
	glGetDoublev(param_name, other_ptr);
	break;
#endif
      case GL_FLOAT:
	glGetFloatv(param_name, other_ptr);
	break;
      case GL_INT:
	glGetIntegerv(param_name, other_ptr);
	break;
      default:
	special_type = 0;
	glGetFloatv(param_name, float_ptr);
	break;
    }

    /* do type conversion if necessary */
    if (!special_type) {
	for (i = 0; i < ncomponents; i++) {
	    /* may have to do some scaling... */
	    if (scale) float_ptr[i] *= (float)maxValue(other_type);
	    switch(other_type) {
	      case GL_BYTE:
		((GLbyte *)other_ptr)[i] = float_ptr[i];
		break;
	      case GL_UNSIGNED_BYTE:
		((GLubyte *)other_ptr)[i] = float_ptr[i];
		break;
	      case GL_SHORT:
		((GLshort *)other_ptr)[i] = float_ptr[i];
		break;
	      case GL_UNSIGNED_SHORT:
		((GLshort *)other_ptr)[i] = float_ptr[i];
		break;
	      case GL_UNSIGNED_INT:
		((GLuint *)other_ptr)[i] = float_ptr[i];
		break;
	      default:
		/* should have hit everything by now... */
		ogEnvLog(OG_LINTERNALERROR, 
			 "ogLibSetReadSet():  Unrecognized type %d.\n",
			 (int)other_type);
		return 0;
	    }
	} 
    }

    /* set the parameter using the other type */
    (*other_set)(other_ptr);

    /* read the paramter back in as a float */
    glGetFloatv(param_name, float_ptr);


    /* do exact compares to get return value */
    for (i = 0; i < ncomponents; i++) {
	if (float_ptr[i] != tmp[i]) return 0;
    }
    
    return 1;
}

/*************************************************************
* maxValue() - maximum value of an OpenGL type
*************************************************************/
static float maxValue(GLenum type) 
{
    GLuint all_ones = 0xffffffff;
    GLint almost_all_ones = 0x7fffffff;
    switch(type) {
      case GL_BYTE:
	return *((GLbyte *)&almost_all_ones);
      case GL_UNSIGNED_BYTE:
	return *((GLubyte *)&all_ones);
      case GL_SHORT:
	return *((GLshort *)&almost_all_ones);
      case GL_UNSIGNED_SHORT:
	return *((GLushort *)&all_ones);
      case GL_INT:
	return *((GLint *)&almost_all_ones);
      case GL_UNSIGNED_INT:
	return *((GLuint *)&all_ones);
      case GL_FLOAT:
	return 1;
#ifdef GL_DOUBLE_EXT
      case GL_DOUBLE_EXT:
	return 1;
#endif
      default:
	ogEnvLog(OG_LINTERNALERROR, "maxValue():  invalid enum %d", (int)type);
	return 0;
    }
}
