/*
 *  tdGenerate.c
 *                                                     01/20/95  1.00  NDR
 *                                                     03/01/95  1.50  NDR
 *                                                     09/01/95  4.00  NDR
 *
 *    Three-D Object Library - Library of 3-D object
 *    manipulation routines.
 *
 *    Generation Module - Generates texture vertices and normals for an object
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
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "tdPrivate.h"


/*
 *  tdGenSmoothNormals()
 *
 *    Generates vertex normals.  Overrides any existing normals in
 *    the object.  Generates smooth normals by calculating the cross
 *    product of two vectors in the same plane as the face (effectively
 *    getting the normal of the face), and uses this normal as the 
 *    normal for all vertices in the face.  Assuming that vertices are
 *    shared between faces, some of the vertex normals for a given face
 *    will be written more than once with different values each time,
 *    which gives a smoother appearance to the object.
 *    
 *    object - a pointer to a valid object with vertices
 *
 *    RETURNs a pointer to the normals if all went well,
 *    otherwise, returns NULL.
 *
 */
TDnormal *tdGenSmoothNormals(TDobject *object)
{
    TDgroup *group;
    TDface *face;
    float length, vx, vy, vz, ux, uy, uz, nx, ny, nz;
    int i;


    /* bail if no object */
    if(object == NULL)
    {
	_tdPrintf(TD_ERROR, "can't generate normals for a null object!");
	return(NULL);
    }

    /* bail if no vertices */
    if(object->vertices == NULL)
    {
	_tdPrintf(TD_ERROR, "can't generate normals without vertices!");
	return(NULL);
    }

    /* warn if there are already normals and nuke the old ones */
    if(object->normals != NULL)
    {
	_tdPrintf(TD_WARNING,
		  "overriding existing vertex normals in object `%s'!",
		  object->name);
	free(object->normals);
    }

    /* allocate memory for normals */
    object->normals = 
	(TDnormal *)malloc(sizeof(TDnormal) * (object->num_vertices + 1));
    if(object->normals == NULL)
    {
	_tdPrintf(TD_NOMEMORY, "malloc() failed for facet normals");
	return(NULL);
    }

    /* do the calculations */
    group = object->groups;
    while(group)
    {
	face = group->faces;
	while(face)
	{
	    /* get the two vectors to cross */
	    vx = object->vertices[face->vertices[0]].x - 
		 object->vertices[face->vertices[1]].x;
	    vy = object->vertices[face->vertices[0]].y - 
		 object->vertices[face->vertices[1]].y;
	    vz = object->vertices[face->vertices[0]].z - 
		 object->vertices[face->vertices[1]].z;
	    
	    ux = object->vertices[face->vertices[1]].x - 
		 object->vertices[face->vertices[2]].x;
	    uy = object->vertices[face->vertices[1]].y - 
		 object->vertices[face->vertices[2]].y;
	    uz = object->vertices[face->vertices[1]].z - 
		 object->vertices[face->vertices[2]].z;
	    
	    /* calculate normal (cross the vectors) */
	    nx = (vy * uz) - (uy * vz);
	    ny = (vz * ux) - (uz * vx);
	    nz = (vx * uy) - (ux * vy);

	    /* normalize the normals */
	    length = sqrt(_tdSqr(nx) + _tdSqr(ny) + _tdSqr(nz));
	    nx /= length;
	    ny /= length;
	    nz /= length;

	    /* put the facet normal in the face */
	    face->normal.i = nx;
	    face->normal.j = ny;
	    face->normal.k = nz;

	    /* get memory for the normal indices */
	    face->num_normals = face->num_vertices;
	    face->normals = 
		(int *)malloc(sizeof(int) * (face->num_normals + 1));
	    if(face->normals == NULL)
	    {
		_tdPrintf(TD_NOMEMORY, "malloc() failed for face normals");
		return(NULL);
	    }
		    
	    /* stick the normal in the object normals
             * list (duplicates alot, and by duplicating,
             * causes a smooth appearance) */
	    for(i = 0; i < face->num_vertices; i++)
	    {
		face->normals[i] = face->vertices[i];
		object->normals[face->normals[i]].i = face->normal.i;
		object->normals[face->normals[i]].j = face->normal.j;
		object->normals[face->normals[i]].k = face->normal.k;
	    }
 
	    face = face->next;
	}

	group = group->next;
    }

    object->num_normals = object->num_vertices;

    _tdPrintf(TD_INFO, "generated %d facet normals", object->num_normals);

    return(object->normals);
}

/*
 *  tdGenSpheremapTexvertices()
 *
 *    Generates texture coordinates according to a spherical projection
 *    of the texture map.  Sometimes referred to as spheremap, or
 *    reflection map texture coordinates.  It generates these by
 *    using the normal to calculate where that vertex would map onto
 *    a sphere.  Since it is impossible to map something flat perfectly
 *    onto something spherical, there is distortion at the poles.  This
 *    particular implementation causes the poles along the X axis to be
 *    distorted.
 *
 *    object - a pointer to a valid object with normals
 *
 *    RETURNs a pointer to the vertices if all went well,
 *    otherwise returns NULL.
 *
 */
TDtexvertex *tdGenSpheremapTexvertices(TDobject *object)
{
    TDgroup *group;
    TDface *face;
    float theta, phi, rho, x, y, z, r;
    int i;

    /* bail if no object */
    if(object == NULL)
    {
	_tdPrintf(TD_ERROR,
		  "can't generate texture vertices for null object!");
	return(NULL);
    }

    /* bail if no normals */
    if(object->normals == NULL)
    {
	_tdPrintf(TD_ERROR, 
		  "can't generate texture vertices without normals!");
	return(NULL);
    }

    /* warn if there are texvertices already there and free them */
    if(object->texvertices)
    {
	_tdPrintf(TD_WARNING,
		  "overriding existing texture vertices in object `%s'",
		  object->name);
	free(object->texvertices);
    }

    /* allocate memory for texvertices */
    object->texvertices = 
	(TDtexvertex *)malloc(sizeof(TDtexvertex) * (object->num_normals + 1));
    if(object->texvertices == NULL)
    {
	_tdPrintf(TD_NOMEMORY,
		  "malloc() failed for spheremap texture vertices");
	return(NULL);
    }

    /* do the calculations */
    for(i = 1; i <= object->num_normals; i++)
    {
	z = object->normals[i].i;   /* re-arrange for pole distortion */
	y = object->normals[i].j;
	x = object->normals[i].k;
	r = sqrt(_tdSqr(x) + _tdSqr(y));
	rho = sqrt(_tdSqr(r) + _tdSqr(z));

	if(r == 0.0)
	{
	    theta = 0.0;
	    phi = 0.0;
	}	    
	else
	{
	    if(z == 0.0)
		phi = (TD_PI_HALF);
	    else
		phi = acos(z / rho);

#if WE_DONT_NEED_THIS_CODE
	    if(x == 0.0)
		theta = TD_PI_HALF;  /* asin(y / r); */
	    else
		theta = acos(x / r);
#endif

	    if(y == 0.0)
		theta = TD_PI_HALF;  /* acos(x / r); */
	    else
		theta = asin(y / r) + (TD_PI_HALF);
	}

	object->texvertices[i].v = (theta / TD_PI);
	object->texvertices[i].u = (phi / TD_PI);
    }

    /* go through and put texvertex indices in all the faces */
    group = object->groups;
    while(group)
    {
	face = group->faces;
	while(face)
	{
	    /* get memory for the texture vertex indices */
	    face->num_texvertices = face->num_normals;
	    face->texvertices = 
		(int *)malloc(sizeof(int) * (face->num_texvertices + 1));
	    if(face->texvertices == NULL)
	    {
		_tdPrintf(TD_NOMEMORY,
			  "malloc() failed for spheremap texture vertices");
		return(NULL);
	    }
		    

	    /* stick the texture vertex index into the face list */
	    for(i = 0; i < face->num_texvertices; i++)
		face->texvertices[i] = face->normals[i];


	    face = face->next;
	}

	group = group->next;
    }

    object->num_texvertices = object->num_normals;

    _tdPrintf(TD_INFO, "generated %d spheremap texture vertices",
	      object->num_texvertices);

    return(object->texvertices);
}

/*
 *  tdGenLinearTexvertices()
 *
 *    Generates texture coordinates according to a linear projection
 *    of the texture map.  It generates these by linearly mapping the
 *    vertices onto a square.
 *
 *    object - a pointer to a valid object with normals
 *
 *    RETURNs a pointer to the vertices if all went well,
 *    otherwise returns NULL.
 */
TDtexvertex *tdGenLinearTexvertices(TDobject *object)
{
    TDgroup *group;
    TDface *face;
    float x, y, scalefactor;
    int i;

    if(object == NULL)
    {
	_tdPrintf(TD_ERROR, 
		  "can't generate texture vertices for null object!");
	return(NULL);
    }

    /* warn if there are texvertices already there and free them */
    if(object->texvertices)
    {
	_tdPrintf(TD_WARNING, "object already has texture vertices!");
	free(object->texvertices);
    }

    /* allocate memory for texvertices */
    object->texvertices = 
       (TDtexvertex *)malloc(sizeof(TDtexvertex) * (object->num_vertices + 1));
    if(object->texvertices == NULL)
    {
	_tdPrintf(TD_NOMEMORY, "malloc() failed for linear texture vertices");
	return(NULL);
    }

    scalefactor = 2.0 / _tdAbs(_tdMax(_tdMax(object->width, object->height),
				      object->depth));

    /* do the calculations */
    for(i = 1; i <= object->num_vertices; i++)
    {
	x = (object->vertices[i].x - object->center.x) * scalefactor;
	y = (object->vertices[i].y - object->center.y) * scalefactor;

	object->texvertices[i].v = ((y + 1.0) / 2.0);
	object->texvertices[i].u = ((x + 1.0) / 2.0);
    }

    /* go through and put texvertex indices in all the faces */
    group = object->groups;
    while(group)
    {
	face = group->faces;
	while(face)
	{
	    /* get memory for the texture vertex indices */
	    face->num_texvertices = face->num_vertices;
	    face->texvertices = 
		(int *)malloc(sizeof(int) * (face->num_texvertices + 1));
	    if(face->texvertices == NULL)
	    {
		_tdPrintf(TD_NOMEMORY, 
			  "malloc() failed for linear texture vertices");
		return(NULL);
	    }
		    

	    /* stick the texture vertex index into the face list */
	    for(i = 0; i < face->num_texvertices; i++)
		face->texvertices[i] = face->vertices[i];


	    face = face->next;
	}

	group = group->next;
    }

    object->num_texvertices = object->num_vertices;

    _tdPrintf(TD_INFO, "generated %d linear texture vertices", 
	      object->num_texvertices);

    return(object->texvertices);
}
