/*
 *  tdDecimate.c                                       08/26/94  0.00  NDR
 *                                                     09/07/94  1.00  NDR
 *                                                     09/08/94  2.00  NDR
 *                                                     11/15/94  3.00  NDR
 *                                                     01/03/95  3.10  NDR
 *                                                     09/01/95  4.00  NDR
 *
 *    Three-D Object Library - Library of 3-D object
 *    manipulation routines.
 *
 *    Decimation Module
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


/*
 *  tdGenTriangles()
 *
 *    Generates triangles from polygonal data.  It does this by
 *    marching along the edge of the polygon, and connects the first
 *    vertex to the next two, then connects the first vertex to 
 *    vertexes 3 and 4 and so on. (Essentially making a triangle fan
 *    out of each polygon.)  This algorithm assumes that the
 *    polygons are convex.
 *
 *    object - a pointer to a valid object with polygonal data
 *
 *    RETURNs the number of triangles the object was broken into if
 *    all went well, otherwise, returns 0.
 */
int tdGenTriangles(TDobject *object)
{
    int i, num_triangles = 0;
    TDgroup *group;
    TDface *face, *triangle, *new_triangle;

    /* bail if no object */
    if(!object)
    {
	_tdPrintf(TD_ERROR, "can't generate triangles for a null object!");
	return(0);
    }

    group = object->groups;
    while(group)
    {
        face = group->faces;
        while(face)
        {
            for(i = 2; i < face->num_vertices; i++)
            {
                new_triangle = 
		    _tdNewFace(3,
			       face->num_texvertices ? 3 : 0,
			       face->num_normals ? 3 : 0);
		/* bail if tdNewFace failed (probably no memory) */
		if(!new_triangle)
		    return(0);
                if(!group->triangles)
                {
                    group->triangles = new_triangle;
                    triangle = new_triangle;
                }
                else
                {
                    triangle->next = new_triangle;
                    triangle = triangle->next;
                }

                triangle->vertices[0] = face->vertices[0];
                triangle->vertices[1] = face->vertices[i - 2 + 1];
                triangle->vertices[2] = face->vertices[i - 2 + 2];
                if(face->num_texvertices >= i)
                {
                    triangle->texvertices[0] = face->texvertices[0];
                    triangle->texvertices[1] = face->texvertices[i - 2 + 1];
                    triangle->texvertices[2] = face->texvertices[i - 2 + 2];
                }
                if(face->num_normals >= i)
                {
                    triangle->normals[0] = face->normals[0];
                    triangle->normals[1] = face->normals[i - 2 + 1];
                    triangle->normals[2] = face->normals[i - 2 + 2];
                }
                group->num_triangles++;
            }
            face = face->next;
        }
        num_triangles += group->num_triangles;
        group = group->next;
    }

    _tdPrintf(TD_INFO, "generated %d triangles", num_triangles);

    return(num_triangles);
}

/*
 *  tdGenQuads()
 *
 *    Generates quadrilaterals (quads) from polygonal data.  It does this by
 *    marching along the edge of the polygon, and connects the first
 *    vertex to the next three, then connects the first vertex to 
 *    vertexes 4 and 5 and so on.  Unfortunately, since a polygon can
 *    consist of an odd number of vertices, this algorithm generates
 *    some degenerate quads, that is, quads that have repeated vertices.
 *    (Basically a triangle with the last vertex repeated).  This algorithm
 *    assumes that the polygons are convex.
 *
 *    object - a pointer to a valid object with polygonal data
 *
 *    RETURNs the number of quads the object was broken into.
 */
int tdGenQuads(TDobject *object)
{
    int i, num_quads = 0, vertices_left;
    TDgroup *group;
    TDface *face, *quad, *new_quad;


    /* bail if no object */
    if(!object)
    {
	_tdPrintf(TD_ERROR, "can't generate quads for a null object!");
	return(0);
    }

    group = object->groups;
    while(group)
    {
        face = group->faces;
        while(face)
        {
	    for(i = 0; i < face->num_vertices; i++)
	    {
                new_quad = 
		    _tdNewFace(4,
			       face->num_texvertices ? 4 : 0,
			       face->num_normals ? 4 : 0);
		/* bail if tdNewFace failed (probably no memory) */
		if(!new_quad)
		    return(0);

                if(!group->quads)
                    group->quads = new_quad;
                else
                    quad->next = new_quad;
		quad = new_quad;

		vertices_left = face->num_vertices - 1;
		for(i = 2; i < face->num_vertices; i += 2)
		{
		    quad->vertices[0] = face->vertices[0];
		    quad->vertices[1] = face->vertices[i - 2 + 1];
		    quad->vertices[2] = face->vertices[i - 2 + 2];
		    if(face->num_texvertices >= i)
		    {
			quad->texvertices[0] = face->texvertices[0];
			quad->texvertices[1] = face->texvertices[i - 2 + 1];
			quad->texvertices[2] = face->texvertices[i - 2 + 2];
		    }
		    if(face->num_normals >= i)
		    {
			quad->normals[0] = face->normals[0];
			quad->normals[1] = face->normals[i - 2 + 1];
			quad->normals[2] = face->normals[i - 2 + 2];
		    }

		    vertices_left -= 2;
		    if(vertices_left == 0) /* use the last one (degenerate) */
		    {
			quad->vertices[3] = face->vertices[i - 2 + 2];
			if(face->num_texvertices >= i)
			   quad->texvertices[3] = face->texvertices[i - 2 + 2];
			if(face->num_normals >= i)
			    quad->normals[3] = face->normals[i - 2 + 2];
		    }
		    else
		    {
			quad->vertices[3] = face->vertices[i - 2 + 3];
			if(face->num_texvertices >= i)
			   quad->texvertices[3] = face->texvertices[i - 2 + 3];
			if(face->num_normals >= i)
			    quad->normals[3] = face->normals[i - 2 + 3];
		    }
		    group->num_quads++;
		}
	    }
            face = face->next;
        }
        num_quads += group->num_quads;
        group = group->next;
    }

    _tdPrintf(TD_INFO, "generated %d quads", num_quads);

    return(num_quads);
}
