/*
 *  tdOFF.c
 *                                                     09/18/95  1.00  NDR
 *                                                     09/01/95  4.00  NDR
 *
 *    Three-D Object Library - Library of 3-D object
 *    manipulation routines.
 *
 *    DEC Object File Format (OFF) 3-D Object File Manipulation Routines
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
#include <string.h>

#include "tdPrivate.h"


/*
 *  tdReadOFFObject()
 * 
 *    Reads in object data in OFF (DEC Object File Format) format
 *    RETURNs a pointer to the object if all went well, `NULL' otherwise.
 */
TDobject *tdReadOFFObject(char *objectname)
{
    char f[256], *filename = f;
    FILE *file = NULL;
    int i, j;
    TDobject *object = NULL;
    TDgroup *group = NULL;
    TDface *face = NULL, *new_face = NULL;
    int face_vertices;

    /* allocate memory for object */
    object = tdNewObject("");
    if(object == NULL)
        return(NULL);

    /* set the object name */
    object->name = (char *)malloc(sizeof(char) * strlen(objectname) + 1);
    if(object->name == NULL)
    {
	_tdPrintf(TD_NOMEMORY, "malloc() failed for object name");
	return(NULL);
    }
    strcpy(object->name, objectname);

    /* allocate memory for default group */
    group = _tdNewGroup("default");
    if(group == NULL)
        return(NULL);
    object->groups = group;
    object->num_groups++;

    /* add appropriate extension to objectname */
    strcpy(filename, objectname);
    strcat(filename, ".geom");

    /* status */
    _tdPrintf(TD_INFO, "reading object data file `%s'", filename);

    /* initially, just count the number of things we need + allocate groups */
    file = fopen(filename, "r");
    if(file == NULL)
    {
        _tdPrintf(TD_ERROR, "can't open object file `%s'!", filename);
        return(NULL);
    }

    /* get a few numbers to allocate memory */
    fscanf(file, "%d\t%d\t%d\n",
	   &object->num_vertices, &group->num_faces, &i);

    object->num_normals = object->num_vertices;

    fclose(file);

    /* status report */
    _tdPrintf(TD_INFO,
	     "%d vertices, %d texvertices, %d normals, %d groups",
	     object->num_vertices, object->num_texvertices, 
	     object->num_normals, object->num_groups);

    /* allocate the vertex, vertex normal, and texture vertex memory */ 
    object->vertices = 
	(TDvertex *)malloc(sizeof(TDvertex) * (object->num_vertices + 1));
    if(object->vertices == NULL)
    {
	_tdPrintf(TD_NOMEMORY, "malloc() failed for object vertices");
	return(NULL);
    }
    if(object->num_texvertices)
    {
	object->texvertices = 
	    (TDtexvertex *)malloc(sizeof(TDtexvertex) * 
				  (object->num_texvertices + 1));
	if(object->texvertices == NULL)
	{
	    _tdPrintf(TD_NOMEMORY, 
		     "malloc() failed for object texture vertices");
	    return(NULL);
	}
    }
    if(object->num_normals)
    {
	object->normals = 
	    (TDnormal *)malloc(sizeof(TDnormal) * (object->num_normals + 1));
	if(object->normals == NULL)
	{
	    _tdPrintf(TD_NOMEMORY, "malloc() failed for object normals");
	    return(NULL);
	}
    }

#if 0
    /* allocate the materials memory */
    tdAllocMem(object->materials, TDmaterial, object->num_materials + 1);
    if(!object->materials)
	return(NULL);
#endif

    /* reset some variables */
    group = object->groups;
    face = NULL;

    /* okay, this time, read it in for real */
    file = fopen(filename, "r");
    if(file == NULL)
    {
        _tdPrintf(TD_ERROR, "can't open object file `%s'!", filename);
        return(NULL);
    }

    /* get the vertex and face data */
    fscanf(file, "%d\t%d\t%d\n",
	   &object->num_vertices, &group->num_faces, &i);

    for(i = 1; i < object->num_vertices + 1; i++)
    {
 	fscanf(file, "%f\t%f\t%f\n",
	       &(object->vertices[i].x),
	       &(object->vertices[i].y),
	       &(object->vertices[i].z));
    }

    for(i = 0; i < group->num_faces; i++)
    {
	fscanf(file, "%d", &face_vertices);
    
	/* allocate memory for face */
	new_face = _tdNewFace(face_vertices, 0, face_vertices);
	if(new_face == NULL)
	    return(NULL);
	if(!group->faces)
	    group->faces = new_face;
	else
	    face->next = new_face;
	face = new_face;
	
	for(j = 0; j < face_vertices; j++)
	{
	    fscanf(file, "\t%d", &face->vertices[j]);
	    face->normals[j] = face->vertices[j];
	}
	fscanf(file, "\n");
    }

    fclose(file);

    /* add appropriate extension to objectname */
    strcpy(filename, objectname);
    strcat(filename, ".vnorm");

    file = fopen(filename, "r");
    if(file == NULL)
    {
        _tdPrintf(TD_ERROR, "can't open object file `%s'!", filename);
        return(NULL);
    }

    /* get the normal data */
    fscanf(file, "%d", &object->num_normals);

    for(i = 1; i < object->num_normals + 1; i++)
    {
 	fscanf(file, "%f\t%f\t%f\n",
	       &(object->normals[i].i),
	       &(object->normals[i].j),
	       &(object->normals[i].k));
    }

    fclose(file);

    /* compute the bounding box of the object */
    _tdBoundingBox(object);

    return(object);
}
