/*
 *  tdDraw.c
 *                                                     01/03/95  1.00  NDR
 *                                                     09/01/95  4.00  NDR
 *
 *    Three-D Object Library - Library of 3-D object
 *    manipulation routines.
 *
 *    Draw Module
 *
 *    author: Nathan Drew Robins
 *    email: narobins@es.com
 *
 *
 *    Copyright 1996 by Evans & Sutherland Computer Corporation.  
 *    ALL RIGHTS RESERVED. 
 *    
 *    Permission to use, copy, modify, and distribute this software 
 *    for any purpose and without fee is hereby granted, provided 
 *    that the above copyright notice appear in all copies and that 
 *    both the copyright notice and this notice appear in supporting
 *    documentation.  You may not use the name "Evans & Sutherland 
 *    Computer Corporation" or any trademark of Evans & Sutherland 
 *    Computer Corporation in advertising or publicity pertaining 
 *    to the software without specific, written prior permission. 
 *    
 *    THIS SOFTWARE AND ANY ASSOCIATED MATERIALS IS PROVIDED TO YOU 
 *    "AS-IS" AND WITHOUT REPRESENTATION OR WARRANTY OF ANY KIND, 
 *    EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, 
 *    ANY WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
 *    PURPOSE OR NONINFRINGEMENT OF ANY PATENT, COPYRIGHT, TRADE 
 *    SECRET OR OTHER RIGHT.  EVANS & SUTHERLAND COMPUTER CORPORATION 
 *    DOES NOT WARRANT THAT USE OF THE SOFTWARE WILL BE UNINTERRUPTED
 *    OR ERROR-FREE, OR THAT DEFECTS IN THE SOFTWARE WILL BE CORRECTED.
 *    
 *    IN NO EVENT SHALL EVANS & SUTHERLAND COMPUTER CORPORATION BE 
 *    LIABLE TO YOU OR ANYONE ELSE FOR ANY DIRECT, SPECIAL, INCIDENTAL, 
 *    INDIRECT OR CONSEQUENTIAL  DAMAGES OF ANY KIND, OR ANY DAMAGES 
 *    WHATSOEVER, INCLUDING WITHOUT LIMITATION, LOSS OF PROFIT, LOSS 
 *    OF USE, SAVINGS OR REVENUE, OR THE CLAIMS OF THIRD PARTIES, 
 *    WHETHER OR NOT EVANS & SUTHERLAND COMPUTER CORPORATION  HAS 
 *    BEEN ADVISED OF THE POSSIBILITY OF SUCH LOSS, HOWEVER CAUSED 
 *    AND ON ANY THEORY OF LIABILITY, ARISING OUT OF OR IN CONNECTION 
 *    WITH THE POSSESSION, USE OR PERFORMANCE OF THIS SOFTWARE.
 *    
 *    US Government Users Restricted Rights 
 *    Use, duplication, or disclosure by the Government is subject 
 *    to restricted rights set forth in FAR 52.227.19(c)(2) or 
 *    subparagraph (c)(1)(ii) of the Rights in Technical Data and 
 *    Computer Software clause at DFARS 252.227-7013 and/or in 
 *    similar or successor clauses in the FAR or the DOD or NASA FAR 
 *    Supplement. Contractor/manufacturer is Evans & Sutherland Computer 
 *    Corporation, 600 Komas Drive, Salt Lake City, UT 84108. 
 *    
 *    Use of this software outside the United States is subject to 
 *    US export laws and regulations.  You agree to comply with all 
 *    such laws and regulations.
 *    
 *    This statement is made under and shall be construed and 
 *    governed by the laws of the State of Utah, without regard to 
 *    conflict of laws principles.
 * 
 */


/* includes */
#include "tdPrivate.h"


/* macros */
#define tdVertex(object, face, i)  /* send vertex */ \
  glVertex3fv((GLfloat *)&(object->vertices[face->vertices[(i)]]));

#define tdTexCoord(object, face, i)  /* send texture vertex */ \
  glTexCoord2fv((GLfloat *)&(object->texvertices[face->texvertices[(i)]]));

#define tdNormal(object, face, i)  /* send normal */ \
  glNormal3fv((GLfloat *)&(object->normals[face->normals[(i)]]));


/*
 *  tdDraw()
 *
 *    mode - primitive drawing mode.  Can be one of the following:
 *     TD_POINTS          - vertexes only only
 *     TD_LINES           - lines drawn around each polygon
 *     TD_LINE_LOOP       - line loop drawn around each polygon
 *     TD_LINE_STRIP      - line strip drawn around each polygon
 *     TD_TRIANGLES       - triangles (must use tdGenTriangles())
 *     TD_TRIANGLE_STRIP  - triangle strips (UNIMPLEMENTED)
 *     TD_TRIANGLE_FAN    - triangle fan 
 *     TD_QUADS           - quadrilaterals (must use tdGenQuads())
 *     TD_QUAD_STRIP      - quad strips (UNIMPLEMENTED)
 *     TD_POLYGON         - polygons 
 *    mask - a mask indicating what type(s) of data to use to draw with.
 *    A bitwise OR of one or more of the following:
 *     TD_VERTEX                 - send vertex data (default)
 *     TD_NORMAL                 - send normal data
 *     TD_TEXVERTEX              - send texture vertex data
 *     TD_MATERIAL               - send material data
 *     TD_ALL                    - send all of the above
 *
 *    RETURNs TD_TRUE if all went well, TD_FALSE if an error occurred
 */
TDboolean tdDraw(TDobject *object, TDprimitive mode, int mask)
{
    register int i;
    static TDmaterial *material;
    static TDgroup *group;
    static TDface *face;

    /* check for some errors */
    if(!object)
    {
	_tdPrintf(TD_ERROR, "can't draw a null object!");
	return(TD_FALSE);
    }

    if(!object->vertices)
    {
        _tdPrintf(TD_ERROR, "no vertex data in object `%s'!", object->name);
	return(TD_FALSE);
    }
	
    /* check for invalid masks */
    if((mode & TD_TEXVERTEX) && !object->texvertices)
    {
	_tdPrintf(TD_WARNING, "no texture vertices in object `%s'!",
		 object->name);
	mask &= ~TD_TEXVERTEX;
    }

    if((mode & TD_NORMAL) && !object->normals)
    {
	_tdPrintf(TD_WARNING, "no normals in object `%s'!", object->name);
	mask &= ~TD_NORMAL;
    }

    switch(mode)
    {
    case TD_POINTS:
	glBegin(TD_POINTS);
	/* TODO - need to figure out materials for points */
	for(i = 1; i <= object->num_vertices; i++)
	{
	    /* send texture coordinate */
	    if(mask & TD_TEXVERTEX)
		glTexCoord2fv((float *)&(object->texvertices[i]));

	    /* send normal */
	    if(mask & TD_NORMAL)
		glNormal3fv((float *)&(object->normals[i]));

	    /* send vertex */
	    glVertex3fv((float *)&(object->vertices[i]));
	}
	glEnd();
	break;
	
    case TD_TRIANGLES:
	glBegin(TD_TRIANGLES);
 	group = object->groups;
	while(group)
	{
	    material = group->material;
	    if((mask & TD_MATERIAL) && material)
	    {
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, 
			     (GLfloat *)&(material->diffuse));
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, 
			     (GLfloat *)&(material->ambient));
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, 
			     (GLfloat *)&(material->specular));
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 
			    material->shininess);
		if(mask & TD_TEXVERTEX && material->teximage)
		{
		    glEnable(GL_TEXTURE_2D);
		    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,
			      GL_MODULATE);
		    gluBuild2DMipmaps(GL_TEXTURE_2D, 3,
				      material->teximage->width,
				      material->teximage->height,
				      GL_RGB, GL_UNSIGNED_BYTE,
				      material->teximage->data);
		}
	    } 

	    face = group->triangles;
	    while(face)
	    {
		for(i = 0; i < 3; i++)
		{
		    /* send texture vertex */
		    if(mask & TD_TEXVERTEX)
			tdTexCoord(object, face, i);
		    /* send normal */
		    if(mask & TD_NORMAL)
			tdNormal(object, face, i);
		    /* send vertex */
		    tdVertex(object, face, i);
		}
		face = face->next;
	    }
	    group = group->next;
        }
	glEnd();
	break;
#if 0	
      case TD_TRIANGLE_STRIP:
 	group = object->groups;
	while(group)
	{
	    face = group->tristrips;
	    while(face)
	    {
		glBegin(mode);
		for(i = 0; i < face->num_vertices; i++)
		{
		    /* send texture vertex */
		    if(mask & TD_TEXVERTEX)
			tdTexCoord(object, face, i);
		    /* send normal */
		    if(mask & TD_NORMAL)
			tdNormal(object, face, i);
		    /* send vertex */
		    tdVertex(object, face, i);
		}
		glEnd();
		face = face->next;
	    }
	    group = group->next;
        }
	break;
#endif
    case TD_LINES:
 	group = object->groups;
	while(group)
	{
	    material = group->material;
	    if((mask & TD_MATERIAL) && material)
	    {
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, 
			     (GLfloat *)&(material->diffuse));
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, 
			     (GLfloat *)&(material->ambient));
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, 
			     (GLfloat *)&(material->specular));
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 
			    material->shininess);
		if(mask & TD_TEXVERTEX && material->teximage)
		{
		    glEnable(GL_TEXTURE_2D);
		    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,
			      GL_MODULATE);
		    gluBuild2DMipmaps(GL_TEXTURE_2D, 3,
				      material->teximage->width,
				      material->teximage->height,
				      GL_RGB, GL_UNSIGNED_BYTE,
				      material->teximage->data);
		}
	    } 

	    face = group->faces;
	    while(face)
	    {
		glBegin(TD_LINES);
		for(i = 0; i < face->num_vertices; i++)
		{
		    /* send texture vertex */
		    if(mask & TD_TEXVERTEX)
			tdTexCoord(object, face, i);
		    /* send normal */
		    if(mask & TD_NORMAL)
			tdNormal(object, face, i);
		    /* send vertex */
		    tdVertex(object, face, i);

		    /* send texture vertex */
		    if(mask & TD_TEXVERTEX)
			tdTexCoord(object, face, 
				(i == face->num_vertices - 1 ? 0 : (i + 1)));
		    /* send normal */
		    if(mask & TD_NORMAL)
			tdNormal(object, face,
				(i == face->num_vertices - 1 ? 0 : (i + 1)));
		    /* send vertex */
		    tdVertex(object, face,
			     (i == face->num_vertices - 1 ? 0 : (i + 1)));
		}
		glEnd();
		face = face->next;
	    }
	    group = group->next;
        }
	break;

    case TD_LINE_LOOP:
 	group = object->groups;
	while(group)
	{
	    material = group->material;
	    if((mask & TD_MATERIAL) && material)
	    {
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, 
			     (GLfloat *)&(material->diffuse));
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, 
			     (GLfloat *)&(material->ambient));
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, 
			     (GLfloat *)&(material->specular));
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 
			    material->shininess);
		if(mask & TD_TEXVERTEX && material->teximage)
		{
		    glEnable(GL_TEXTURE_2D);
		    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,
			      GL_MODULATE);
		    gluBuild2DMipmaps(GL_TEXTURE_2D, 3,
				      material->teximage->width,
				      material->teximage->height,
				      GL_RGB, GL_UNSIGNED_BYTE,
				      material->teximage->data);
		}
	    } 

	    face = group->faces;
	    while(face)
	    {
		glBegin(mode);
		for(i = 0; i < face->num_vertices; i++)
		{
		    /* send texture vertex */
		    if(mask & TD_TEXVERTEX)
			tdTexCoord(object, face, i);
		    /* send normal */
		    if(mask & TD_NORMAL)
			tdNormal(object, face, i);
		    /* send vertex */
		    tdVertex(object, face, i);
		}
		/* finish the loop */
		/* send texture vertex */
		if(mask & TD_TEXVERTEX)
		    tdTexCoord(object, face, 0);
		/* send normal */
		if(mask & TD_NORMAL)
		    tdNormal(object, face, 0);
		/* send vertex */
		tdVertex(object, face, 0);
		glEnd();
		face = face->next;
	    }
	    group = group->next;
        }
	break;

    case TD_LINE_STRIP:
    case TD_TRIANGLE_FAN:
    case TD_POLYGON:
 	group = object->groups;
	while(group)
	{
	    material = group->material;
	    if((mask & TD_MATERIAL) && material)
	    {
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, 
			     (GLfloat *)&(material->diffuse));
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, 
			     (GLfloat *)&(material->ambient));
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, 
			     (GLfloat *)&(material->specular));
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 
			    material->shininess);
		if(mask & TD_TEXVERTEX && material->teximage)
		{
		    glEnable(GL_TEXTURE_2D);
		    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,
			      GL_MODULATE);
		    gluBuild2DMipmaps(GL_TEXTURE_2D, 3,
				      material->teximage->width,
				      material->teximage->height,
				      GL_RGB, GL_UNSIGNED_BYTE,
				      material->teximage->data);
/* TODO - figure some way to get all the textures in a dlist */
#if 0
		    if(material->dlist)
		    {
			glCallList(material->dlist);
			printf("called list\n");
		    }
		    else
		    {
			material->dlist = glGenLists(1);
			glNewList(material->dlist, GL_COMPILE_AND_EXECUTE);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,
				  GL_MODULATE);
			gluBuild2DMipmaps(GL_TEXTURE_2D, 3,
					  material->teximage->width,
					  material->teximage->height,
					  GL_RGB, GL_UNSIGNED_BYTE,
					  material->teximage->data);
			glEndList();
			printf("built list %d\n", material->dlist);
		    }
#endif
		}
	    } 

	    face = group->faces;
	    while(face)
	    {
		glBegin(mode);
		for(i = 0; i < face->num_vertices; i++)
		{
		    /* send texture vertex */
		    if(mask & TD_TEXVERTEX)
			tdTexCoord(object, face, i);
		    /* send normal */
		    if(mask & TD_NORMAL)
			tdNormal(object, face, i);
		    /* send vertex */
		    tdVertex(object, face, i);
		}
		glEnd();
		face = face->next;
	    }

	    if(material)
	    {
		if(mask & TD_TEXVERTEX && material->teximage)
		    glDisable(GL_TEXTURE_2D);
	    }
	    group = group->next;
        }
	break;

      case TD_QUADS:
	glBegin(TD_QUADS);
 	group = object->groups;
	while(group)
	{
	    material = group->material;
	    if((mask & TD_MATERIAL) && material)
	    {
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, 
			     (GLfloat *)&(material->diffuse));
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, 
			     (GLfloat *)&(material->ambient));
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, 
			     (GLfloat *)&(material->specular));
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 
			    material->shininess);
		if(mask & TD_TEXVERTEX && material->teximage)
		{
		    glEnable(GL_TEXTURE_2D);
		    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,
			      GL_MODULATE);
		    gluBuild2DMipmaps(GL_TEXTURE_2D, 3,
				      material->teximage->width,
				      material->teximage->height,
				      GL_RGB, GL_UNSIGNED_BYTE,
				      material->teximage->data);
		}
	    } 

	    face = group->quads;
	    while(face)
	    {
		for(i = 0; i < 4; i++)
		{
		    /* send texture vertex */
		    if(mask & TD_TEXVERTEX)
			tdTexCoord(object, face, i);
		    /* send normal */
		    if(mask & TD_NORMAL)
			tdNormal(object, face, i);
		    /* send vertex */
		    tdVertex(object, face, i);
		}
		face = face->next;
	    }
	    group = group->next;
        }
	glEnd();
	break;
#if 0
    case TD_QUAD_STRIP:
	break;
#endif

    default:    /* none of the above */
	return(TD_FALSE);
	break;
    }
    return(TD_TRUE);
}


/*
 *  tdGenDList()
 * 
 *    Generates a display list out of an object
 *
 *    mode - primitive drawing mode.  Can be one of the following:
 *     TD_POINTS          - vertexes only only
 *     TD_LINES           - lines drawn around each polygon
 *     TD_LINE_LOOP       - line loop drawn around each polygon
 *     TD_LINE_STRIP      - line strip drawn around each polygon
 *     TD_TRIANGLES       - triangles (must use tdGenTriangles())
 *     TD_TRIANGLE_STRIP  - triangle strips (UNIMPLEMENTED)
 *     TD_TRIANGLE_FAN    - triangle fan 
 *     TD_QUADS           - quadrilaterals (must use tdGenQuads())
 *     TD_QUAD_STRIP      - quad strips (UNIMPLEMENTED)
 *     TD_POLYGON         - polygons 
 *    mask - a mask indicating what type(s) of data to use to draw with.
 *    A bitwise OR of one or more of the following:
 *     TD_VERTEX                 - send vertex data (default)
 *     TD_NORMAL                 - send normal data
 *     TD_TEXVERTEX              - send texture vertex data
 *     TD_MATERIAL               - send material data
 *     TD_ALL                    - send all of the above
 *
 *    RETURNs the list number allocated if all went well,
 *    otherwise, returns 0.
 */
int tdGenDList(TDobject *object, TDprimitive mode, int mask)
{
    int list;
    int status;

    list = glGenLists(1);

    /* create a list */
    glNewList(list, GL_COMPILE);
      status = tdDraw(object, mode, mask);
    glEndList();

    if(status == TD_FALSE)  /* couldn't build a display list */
    {
	glDeleteLists(list, 1);
	return(0);
    }

    /* stick the display list into the object */
    object->display_list = list;

    return(list);
}
