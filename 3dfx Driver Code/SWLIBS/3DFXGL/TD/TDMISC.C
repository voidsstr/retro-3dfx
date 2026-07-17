/*
 *  tdMisc.c                                           08/26/94  0.00  NDR
 *                                                     09/07/94  1.00  NDR
 *                                                     09/08/94  2.00  NDR
 *                                                     11/15/94  3.00  NDR
 *                                                     01/03/95  3.10  NDR
 *                                                     09/01/95  4.00  NDR
 *
 *    Three-D Object Library - Library of 3-D object
 *    manipulation routines.
 *
 *    Miscellaneous Routines Module
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

#include "tdPrivate.h"


/*
 *  tdNewObject()
 * 
 *    Allocates memory for an object data structure, and fills the new
 *    structure with default values.
 *
 *    name - name of the new object, should not be NULL.
 *
 *    RETURNs a new object pointer if all went well,
 *    otherwise an error message is printed, and NULL returned.
 *
 */
TDobject *tdNewObject(char *name)
{
    TDobject *object;

    object = (TDobject *)malloc(sizeof(TDobject));
    if(object == NULL)
    {
	_tdPrintf(TD_NOMEMORY, "malloc() failed for new object");
	return(NULL);
    }
    
    /* allocate memory and set the object name */
    object->name = malloc(sizeof(char) * (strlen(name) + 1));
    if(object->name == NULL)
    {
	_tdPrintf(TD_NOMEMORY, "malloc() failed for object name");
	return(NULL);
    }
    strcpy(object->name, name);

    /* initialize all elements */
    object->center.x = 0.0;
    object->center.y = 0.0;
    object->center.z = 0.0;
    object->maximum.x = -TD_BIG;  /* make this really small, so when real   */
    object->maximum.y = -TD_BIG;  /* values are read in, they will for sure */
    object->maximum.z = -TD_BIG;  /* be bigger, and we can get correct      */
    object->minimum.x = TD_BIG;   /* values for the bounding box info       */
    object->minimum.y = TD_BIG;
    object->minimum.z = TD_BIG;
    object->width = 0.0;
    object->height = 0.0;
    object->depth = 0.0;

    object->translation.x = 0.0;
    object->translation.y = 0.0;
    object->translation.z = 0.0;
    object->translation.x_delta = 0.0;
    object->translation.y_delta = 0.0;
    object->translation.z_delta = 0.0;
    object->rotation.i = 0.0;
    object->rotation.j = 0.0;
    object->rotation.k = 0.0;
    object->rotation.i_delta = 0.0;
    object->rotation.j_delta = 0.0;
    object->rotation.k_delta = 0.0;

    object->num_vertices = 0;
    object->num_texvertices = 0;
    object->num_normals = 0;

    object->vertices = NULL;
    object->texvertices = NULL;
    object->normals = NULL;

    object->num_materials = 0;

    object->materials = NULL;

    object->num_groups = 0;
    object->num_faces = 0;

    object->groups = NULL;

    object->display_list = 0;

    object->next = NULL;

    return(object);
}

/*
 *  _tdNewGroup()
 * 
 *    Allocates memory for a group data structure, and fills the new
 *    structure with default values.
 *
 *    name - name of the new group, should not be NULL.
 *
 *    RETURNs a new group pointer if all went well,
 *    otherwise an error message is printed, and NULL returned.
 */
TDgroup *_tdNewGroup(char *name)
{
    TDgroup *group;

    group = (TDgroup *)malloc(sizeof(TDgroup));
    if(group == NULL)
    {
	_tdPrintf(TD_NOMEMORY, "malloc() failed for new group");
	return(NULL);
    }
    
    /* initialize all elements */
    group->name = (char *)malloc(sizeof(char) * (strlen(name) + 1));
    if(group->name == NULL)
    {
	_tdPrintf(TD_NOMEMORY, "malloc() failed for group name");
	return(NULL);
    }
    strcpy(group->name, name);

    group->material = 0;

    group->num_faces = 0;
    group->faces = NULL;

    group->num_triangles = 0;
    group->num_quads = 0;

    group->triangles = NULL;
    group->quads = NULL;

    group->dlist = 0;

    group->next = NULL;

    return(group);
}

/*
 *  _tdNewFace()
 * 
 *    Allocates memory for a face data structure, and 
 *    fills the fields with default values.
 *
 *    num_vertices    - number of vertices to allocate memory for
 *    num_texvertices - number of texvertices to allocate memory for
 *    num_normals     - number of normals to allocate memory for 
 *
 *    RETURNs a new face pointer if all went well,
 *    otherwise an error message is printed, and NULL returned.
 */
TDface *_tdNewFace(int num_vertices, int num_texvertices, int num_normals)
{
    TDface *face;

    face = (TDface *)malloc(sizeof(TDface));
    if(face == NULL)
    {
	_tdPrintf(TD_NOMEMORY, "malloc() failed for face");
	return(NULL);
    }

    /* clear all pointers */
    face->num_vertices = num_vertices;
    face->num_texvertices = num_texvertices;
    face->num_normals = num_normals;
    face->vertices = NULL;
    face->texvertices = NULL;
    face->normals = NULL;

    face->normal.i = 0.0;
    face->normal.j = 0.0;
    face->normal.k = 0.0;

    face->next = NULL;

    /* allocate memory for vertices */
    if(num_vertices)
    {
	face->vertices = (int *)malloc(sizeof(int) * num_vertices);
	if(face->vertices == NULL)
	{
	    _tdPrintf(TD_NOMEMORY, "malloc() failed for face vertices");
	    return(NULL);
	}
    }

    /* allocate memory for texture vertices */
    if(num_texvertices)
    {
	face->texvertices = (int *)malloc(sizeof(int) * num_texvertices);
	if(face->texvertices == NULL)
	{
	    _tdPrintf(TD_NOMEMORY, "malloc() failed for face texture vertices");
	    return(NULL);
	}
    }

    /* allocate memory for normals */
    if(num_normals)
    {
	face->normals = (int *)malloc(sizeof(int) * num_normals);
	if(face->normals == NULL)
	{
	    _tdPrintf(TD_NOMEMORY, "malloc() failed for face normals");
	    return(NULL);
	}
    }

    return(face);
}

/*
 *  _tdNewMaterial()
 * 
 *    Allocates memory for a material data structure, and 
 *    fills the fields with default values.
 *
 *    name - name of the new group, should not be NULL.
 *
 *    RETURNs a new material pointer if all went well,
 *    otherwise an error message is printed, and NULL returned.
 */
TDmaterial *_tdNewMaterial(char *name)
{
    TDmaterial *material;

    material = (TDmaterial *)malloc(sizeof(TDmaterial));
    if(!material)
    {
	_tdPrintf(TD_NOMEMORY, "malloc() failed for new material");
	return(NULL);
    }

    /* initialize all elements */
    material->name = (char *)malloc(sizeof(char) * (strlen(name) + 1));
    if(!material->name)
    {
	_tdPrintf(TD_NOMEMORY, "malloc() failed for material name");
	return(NULL);
    }
    strcpy(material->name, name);

    material->color.r = 0.0;
    material->color.g = 0.0;
    material->color.b = 0.0;

    material->ambient.r = 0.0;
    material->ambient.g = 0.0;
    material->ambient.b = 0.0;

    material->diffuse.r = 0.0;
    material->diffuse.g = 0.0;
    material->diffuse.b = 0.0;

    material->specular.r = 0.0;
    material->specular.g = 0.0;
    material->specular.b = 0.0;

    material->emmission.r = 0.0;
    material->emmission.g = 0.0;
    material->emmission.b = 0.0;

    material->shininess = 0.0;

    material->teximage = NULL;
    material->dlist    = 0;

    material->next = NULL;

    return(material);
}

/*
 *  _tdDeleteMaterials()
 * 
 *    Frees memory allocated for a material data structures
 *
 *    material - pointer to a list of materials to be freed.
 *
 *    NOTE that this routine goes through the WHOLE LIST of
 *    materials (until it hits a NULL) and frees ALL of them
 *     
 */
void _tdDeleteMaterials(TDmaterial *material)
{
    TDmaterial *free_material;

    while(material)
    {
	free_material = material;
	material = material->next;

	if(free_material->name)
	    free(free_material->name);
	
	if(free_material->teximage)
	{
	    if(free_material->teximage->data)
		free(free_material->teximage->data);
	    free(free_material->teximage);
	}
	
	/* don't free the OpenGL display list, 
         * because the user may still be using it,
         * just reset it
         */
	free_material->dlist = 0;

	free(free_material);
    }
}

/*
 *  _tdDeleteFaces()
 * 
 *    Frees memory allocated for a face data structures
 *
 *    face - pointer to a list of faces to be freed.
 *
 *    NOTE that this routine goes through the WHOLE LIST of
 *    faces (until it hits a NULL) and frees ALL of them
 */
void _tdDeleteFaces(TDface *face)
{
    TDface *free_face;

    while(face)
    {
	free_face = face;
	face = face->next;

	if(free_face->vertices)
	    free(free_face->vertices);
	free_face->num_vertices = 0;
	if(free_face->texvertices)
	    free(free_face->texvertices);
	free_face->num_texvertices = 0;
	if(free_face->normals)
	    free(free_face->normals);
	free_face->num_normals = 0;
	free(free_face);
    }
}

/*
 *  _tdDeleteGroups()
 * 
 *    Frees memory allocated for a group data structures
 *
 *    group - pointer to a list of groups to be freed.
 *
 *    NOTE that this routine goes through the WHOLE LIST of
 *    groups (until it hits a NULL) and frees ALL of them
 *    including any memory associated with them (such as faces)
 */
void _tdDeleteGroups(TDgroup *group)
{
    TDgroup *free_group;

    while(group)
    {
	free_group = group;
	group = group->next;

	if(free_group->name)
	    free(free_group->name);

	/* only reset this pointer (don't free it),
         * since the real data lives in the object
         */
	free_group->material = NULL;

	/* free all the faces */
	_tdDeleteFaces(free_group->triangles);
	_tdDeleteFaces(free_group->quads);

	free_group->num_triangles = 0;
	free_group->num_quads = 0;

	_tdDeleteFaces(free_group->faces);
	free_group->num_faces = 0;

	free(free_group);
    }
}

/*
 *  tdDeleteObject()
 * 
 *    Frees memory allocated for an object data structure
 *
 *    object - pointer to an object to be freed.
 *
 */
void tdDeleteObject(TDobject *object)
{
    TDobject *free_object = NULL;

    /* null out the next field, as we don't use it */
    object->next = NULL;
    while(object)  /* make sure it hasn't already been freed */
    {
	free_object = object;
	object = object->next;

	/* don't free the OpenGL display list, 
         * because the user may still be using it,
	 * just reset it
         */
	free_object->display_list = 0;

	/* free all the groups */
	_tdDeleteGroups(free_object->groups);

	/* free all the materials */
	_tdDeleteMaterials(free_object->materials);

	/* free memory for the vertex arrays */
	if(free_object->vertices)
	    free(free_object->vertices);

	if(free_object->texvertices)
	    free(free_object->texvertices);

	if(free_object->normals)
	    free(free_object->normals);

	free_object->num_vertices = 0;
	free_object->num_texvertices = 0;
	free_object->num_normals = 0;
	free_object->num_groups = 0;
	free_object->num_faces = 0;
	free_object->num_materials = 0;

	if(free_object->name) 
	    free(free_object->name);

	free(free_object);
    }
}

/*
 *  _tdBoundingBox()
 *
 *    Computes the bounding box around an object, and
 *    the dimensions (volumetric) of the object.
 *
 *    object - a pointer to a valid object
 *
 */
void _tdBoundingBox(TDobject *object)
{
    int i;

    if(!object)
    {
	_tdPrintf(TD_ERROR, "can't compute bounding box of null object!");
	return;
    }

    /* reset the max and min values
     * make the max's really negative, and the
     * min's really positive, so that when we get
     * new values, the max/min will be overridden easily
     */
    object->maximum.x = -TD_BIG;
    object->maximum.y = -TD_BIG;
    object->maximum.z = -TD_BIG;
    object->minimum.x = TD_BIG;
    object->minimum.y = TD_BIG;
    object->minimum.z = TD_BIG;

    /* compute bounding box info */
    for(i = 1; i <= object->num_vertices; i++)
    {
        if(object->vertices[i].x > object->maximum.x) 
	    object->maximum.x = object->vertices[i].x;
        else if(object->vertices[i].x < object->minimum.x) 
	    object->minimum.x = object->vertices[i].x;
        if(object->vertices[i].y > object->maximum.y)
	    object->maximum.y = object->vertices[i].y;
        else if(object->vertices[i].y < object->minimum.y)
	    object->minimum.y = object->vertices[i].y;
        if(object->vertices[i].z > object->maximum.z)
	    object->maximum.z = object->vertices[i].z;
        else if(object->vertices[i].z < object->minimum.z)
	    object->minimum.z = object->vertices[i].z;
    }

    /* compute center of object */
    object->center.x = (object->maximum.x + object->minimum.x) / 2.0;
    object->center.y = (object->maximum.y + object->minimum.y) / 2.0;
    object->center.z = (object->maximum.z + object->minimum.z) / 2.0;

    /* compute dimensions of object */
    object->width = _tdAbs(object->minimum.x) + _tdAbs(object->maximum.x);
    object->height = _tdAbs(object->minimum.y) + _tdAbs(object->maximum.y);
    object->depth = _tdAbs(object->minimum.z) + _tdAbs(object->maximum.z);
}

/*
 *  tdSize()
 *
 *    Translates an object to the origin (0,0,0) and
 *    scales it by a scalefactor.
 *
 *    object - a pointer to a valid object
 *    scalefactor - factor to scale object by. If it 
 *    is zero (0.0), then the object is scaled by the
 *    2.0 / maximum dimension of the object (this causes the
 *    object to be scaled to between -1.0 and 1.0 in all
 *    dimensions uniformly (that is, the aspect ratio is
 *    preserved).
 *
 *    RETURNs the scalefactor used.  If object is NULL,
 *    0.0 is returned.
 *
 */
float tdSize(TDobject *object, float scalefactor)
{
    int i;

    /* make sure we have an object */
    if(object == NULL)
    {
	_tdPrintf(TD_ERROR, "can't scale a null object!");
	return(0.0);
    }

    if(!scalefactor)  /* this is a bit dangerous, since it is a float */
    {
	scalefactor = 
	    2.0 / _tdAbs(_tdMax(_tdMax(object->width, object->height),
				object->depth));
	_tdPrintf(TD_INFO, "scaling object by %f", scalefactor);
    }

    /* translate to the origin */
    for(i = 1; i <= object->num_vertices; i++)
    {
	object->vertices[i].x -= object->center.x;
	object->vertices[i].y -= object->center.y;
	object->vertices[i].z -= object->center.z;
    }

    /* scale all the vertices */
    for(i = 1; i <= object->num_vertices; i++)
    {
	object->vertices[i].x *= scalefactor;
	object->vertices[i].y *= scalefactor;
	object->vertices[i].z *= scalefactor;
    }

    /* calculate the bounding box */
    _tdBoundingBox(object);

    return(scalefactor);
}
