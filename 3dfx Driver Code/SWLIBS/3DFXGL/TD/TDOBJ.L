%{
/*
 *  tdOBJ.l
 *                                                     02/15/95  1.00  NDR
 *                                                     08/01/95  2.00  NDR
 *                                                     09/01/95  4.00  NDR
 *
 *    Three-D Object Library - Library of 3-D object
 *    manipulation routines.
 *
 *    Wavefront Obj Format 3-D Object File routines
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


/* defines */
#define MAX_ERRORS 10  /* maximum number of errors before bailout */


/* global variables */
static TDobject
   *object;

static TDgroup
  *group,          /* current group */
  *new_group;      /* used for allocating a new group */

static TDface 
  *face,           /* current face */
  *new_face;       /* used for allocating a new face */

static TDmaterial
  *material,       /* current material */
  *new_material;   /* used for allocating a new material */

static int 
  i, v,                 /* counters */
  tmp,                  /* temporary variable */
  skip,                 /* number of characters to skip */
  num_lines,            /* number of lines in the file */
  num_ignored,          /* number of unsupported keywords ignored */
  num_errors,           /* number of parse errors */
  num_pass,             /* which pass are we on (1 or 2)? */
  num_vertices,         /* counter for number of vertices read in so far */
  num_texvertices,      /* counter for number of texvertices read in so far */
  num_normals;          /* counter for number of normals read in so far */

static char buf[256];                  /* general buffer */
static char buffer[256];               /* another buffer */
static char *str;                      /* general string pointer */

static char *material_filename;        /* name of the material file */
static char directory[256];            /* directory of the files */


%}

/* states */
%s FIRSTPASS

%s SECONDPASS

%s MATERIAL


/* definitions */
nl      \n
ws      [ \t]+
float   -?(([0-9]+)|([0-9]*\.[0-9]+)([eE][-+]?[0-9]+)?)
integer -?[0-9]+
string  [^ \t\n]+

%%

 /* all state rules */

 /* white space */
{ws} ;

 /* new line */
{ws}?{nl} num_lines++;  /* count lines */

 /* comment */
#.* ;

 /* contributed message */
$.* ;  


 /* MATERIAL PASS */

 /* new material - newmtl material name */
<MATERIAL>^newmtl{ws}{string} {
    object->num_materials++;

    /* allocate a new material */
    new_material = _tdNewMaterial(&yyOBJtext[7]);
    if(!new_group)
	return(-1);

    /* don't need to check material, because it will
     * initially be set to object->materials (default material - white)
     */
    material->next = new_material;
    material = new_material;
}

 /* ambient - Ka r [g] [b] */
<MATERIAL>^Ka{ws}{float}{ws}?{float}?{ws}?{float}? {
    i = sscanf(&yyOBJtext[2], "%f %f %f",
	       &(material->ambient.r),
	       &(material->ambient.g),
	       &(material->ambient.b));

    /* if only one is specified, make all of them the same intensity */
    if(i == 1)
    {
	material->ambient.g = material->ambient.r; 
	material->ambient.b = material->ambient.r; 
    }
    material->ambient.a = 1.0;
}

 /* diffuse - Kd r [g] [b] */
<MATERIAL>^Kd{ws}{float}{ws}?{float}?{ws}?{float}? {
    i = sscanf(&yyOBJtext[2], "%f %f %f",
	       &(material->diffuse.r),
	       &(material->diffuse.g),
	       &(material->diffuse.b));

    /* if only one is specified, make all of them the same intensity */
    if(i == 1)
    {
	material->diffuse.g = material->diffuse.r; 
	material->diffuse.b = material->diffuse.r; 
    }
    material->diffuse.a = 1.0;
}

 /* specular - Ks r [g] [b] */
<MATERIAL>^Ks{ws}{float}{ws}?{float}?{ws}?{float}? {
    i = sscanf(&yyOBJtext[2], "%f %f %f",
	       &(material->specular.r),
	       &(material->specular.g),
	       &(material->specular.b));

    /* if only one is specified, make all of them the same intensity */
    if(i == 1)
    {
	material->specular.g = material->specular.r; 
	material->specular.b = material->specular.r; 
    }
    material->specular.a = 1.0;
}

 /* specular exponent - Ns num */
<MATERIAL>^Ns{ws}{float} {
    sscanf(&yyOBJtext[2], "%f", &(material->shininess));

    /* Wavefront shininess is in the range [0, 1000] 
     * so, scale the shininess to something that
     * OpenGL can handle (in the range [0, 128] 
     */
    material->shininess /= 1000;
    material->shininess *= 128;
}

 /* map ambient - map_Ka ... filename */
<MATERIAL>^map_K[ads]{ws}.*{string} {
    /* grab the name of the texture */
    sscanf(&yyOBJtext[7], "%s", buf);

    /* put together a string */
    strncpy(buffer, directory, sizeof(char) * 256);
    strncat(buffer, buf, sizeof(char) * 256 - strlen(directory));

    if(!material->teximage)
	material->teximage = tdReadTeximage(buffer);
}

 /* decal texture - decal ... filename TODO - this is GL_DECAL */
<MATERIAL>^decal{ws}.*{string} {

}


<MATERIAL>^illum{ws}.*     |
<MATERIAL>^Tf{ws}.*        |
<MATERIAL>^d{ws}.*         |
<MATERIAL>^sharpness{ws}.* |
<MATERIAL>^Ni{ws}.*        num_ignored++;


 /* SECOND PASS */

 /* vertex - v x y z [w] */
<SECONDPASS>^v{ws}{float}{ws}{float}{ws}{float}{ws}?{float}? {
    num_vertices++;

    i = sscanf(&yyOBJtext[1], "%f %f %f %f",
	       &(object->vertices[num_vertices].x),
	       &(object->vertices[num_vertices].y),
	       &(object->vertices[num_vertices].z),
	       &(object->vertices[num_vertices].w));
    
    /* no W parameter */
    if(i == 3)
	object->vertices[num_vertices].w = 1.0;  /* defaults to 1.0 */

    /* i < 3 errors aren't handled here, because we specifically
     * look for {float} {float} {float} 
     */
}

 /* texture vertex - vt u v [w] */
<SECONDPASS>^vt{ws}{float}{ws}?{float}?{ws}?{float}? {
    num_texvertices++;

    sscanf(&yyOBJtext[2], "%f %f %f",
           &(object->texvertices[num_texvertices].u),
           &(object->texvertices[num_texvertices].v),
           &(object->texvertices[num_texvertices].w));

    if(i == 2)
	object->texvertices[num_texvertices].w = 0.0;  /* defaults to 0.0 */
    else if(i == 1)
	object->texvertices[num_texvertices].v = 0.0;  /* defaults to 0.0 */
}

 /* vertex normal - vn i j k */
<SECONDPASS>^vn{ws}{float}{ws}{float}{ws}{float} {
    num_normals++;

    sscanf(&yyOBJtext[2], "%f %f %f",
           &(object->normals[num_normals].i),
           &(object->normals[num_normals].j),
           &(object->normals[num_normals].k));

    /* no errors handled here, because we specifically
     * look for {float} {float} {float} 
     */
}

 /* face - f ... */
<SECONDPASS>^f{ws}.*     /* don't do anything this time */

 /* group - g ... */
<SECONDPASS>^g.*  {  /* update the group pointer */
    /* need to do this in case the current group
     * doesn't explicitly set a material, so
     * its supposed to use the last one sent down 
     */
    if(!group->material)
	group->material = material;

    group = group->next;
}    

 /* mtllib - mtllib ... */
<SECONDPASS>^mtllib.*  ;  /* don't do anything this time */

 /* material - usemtl material */
<SECONDPASS>^usemtl{ws}{string} {
    /* get the material name */
    sscanf(&yyOBJtext[6], "%s", buf);  /* index at 6 to skip "usemtl" */

    /* find the material */
    material = object->materials;
    while(material)
    {
	if(!strcmp(material->name, buf))
	    break;

	material = material->next;
    }

    /* make sure the material was found */
    if(!material)
	_tdPrintf(TD_WARNING, "can't find material \"%s\"!", buf);

    /* NOTE - this parser assumes that there is
     * only one usemtl per group!
     */
    group->material = material;
}



 /* FIRST PASS */

 /* vertex - v x y z [w] */
<FIRSTPASS>^v{ws}{float}{ws}{float}{ws}{float}{ws}?{float}? {
    object->num_vertices++;
}

 /* texture vertex - vt u v [w] */
<FIRSTPASS>^vt{ws}{float}?{ws}?{float}{ws}?{float}? {
    object->num_texvertices++;
}

 /* vertex normal - vn i j k */
<FIRSTPASS>^vn{ws}{float}{ws}{float}{ws}?{float}? {
    object->num_normals++;
}

 /* face - f v/vt/vn ... */
<FIRSTPASS>^f{ws}{integer}[/]{integer}[/]{integer}.* {
    object->num_faces++;
    group->num_faces++;

    /* set the string pointer to the matched text (past the "f") */
    str = &yyOBJtext[1];

    /* count the number of vertex indexes */
    v = 0;
    while(1)
    {
	i = sscanf(str, "%d/%d/%d%n", &tmp, &tmp, &tmp, &skip);
	if(i != 3)
	    break;  /* bail if we didn't read anything (end of line) */

	/* position the string pointer past the indexes
	 * that have already been read 
	 */
	str = &str[skip];
	v++;
    }

    /* make sure we got at least 3 vertices */
    if(v < 3)
    {
	_tdPrintf(TD_ERROR, "**line %d: a face must have at least 3 vertices!",
		  num_lines);
	break;  /* break out -- don't get this face */
    }

    /* malloc a face */
    new_face = _tdNewFace(v, v, v);
    if(!new_face)
	return(-1);
    if(!group->faces)
	group->faces = new_face;
    else
	face->next = new_face;
    face = new_face;
	
    /* reset the string pointer, and fill the new face with data */
    str = &yyOBJtext[1];

    v = 0;
    while(1)
    {
	i = sscanf(str, "%d/%d/%d%n", &(face->vertices[v]),
		   &(face->texvertices[v]), &(face->normals[v]), &skip);
	if(i != 3)
	    break;  /* bail if we didn't read anything (end of line) */

	/* check for negative references */
	if(face->vertices[v] < 0)
	    face->vertices[v] += object->num_vertices + 1;
	if(face->texvertices[v] < 0)
	    face->texvertices[v] += object->num_texvertices + 1;
	if(face->normals[v] < 0)
	    face->normals[v] += object->num_normals + 1;

	/* position the string pointer past the indexes
	 * that have already been read 
	 */
	str = &str[skip];
	v++;
    }

    /* set the number of vertices */
    face->num_vertices    = v;
    face->num_texvertices = v;
    face->num_normals     = v;
}

 /* face - f v/vt ... */
<FIRSTPASS>^f{ws}{integer}[/]{integer}.* {
    object->num_faces++;
    group->num_faces++;

    /* set the string pointer to the matched text (past the "f") */
    str = &yyOBJtext[1];

    /* count the number of vertex indexes */
    v = 0;
    while(1)
    {
	i = sscanf(str, "%d/%d%n", &tmp, &tmp, &skip);
	if(i != 2)
	    break;  /* bail if we didn't read anything (end of line) */

	/* position the string pointer past the indexes
	 * that have already been read 
	 */
	str = &str[skip];
	v++;
    }

    /* make sure we got at least 3 vertices */
    if(v < 3)
    {
	_tdPrintf(TD_ERROR, "**line %d: a face must have at least 3 vertices!",
		  num_lines);
	break;  /* break out -- don't get this face */
    }

    /* malloc a face */
    new_face = _tdNewFace(v, v, 0);
    if(!new_face)
	return(-1);
    if(!group->faces)
	group->faces = new_face;
    else
	face->next = new_face;
    face = new_face;
	
    /* reset the string pointer, and fill the new face with data */
    str = &yyOBJtext[1];

    v = 0;
    while(1)
    {
	i = sscanf(str, "%d/%d%n",
		   &(face->vertices[v]), &(face->texvertices[v]), &skip);
	if(i != 2)
	    break;  /* bail if we didn't read anything (end of line) */

	/* check for negative references */
	if(face->vertices[v] < 0)
	    face->vertices[v] += object->num_vertices + 1;
	if(face->texvertices[v] < 0)
	    face->texvertices[v] += object->num_texvertices + 1;

	/* position the string pointer past the indexes
	 * that have already been read 
	 */
	str = &str[skip];
	v++;
    }

    /* set the number of vertices */
    face->num_vertices    = v;
    face->num_texvertices = v;
}

 /* face - f v//vn ... */
<FIRSTPASS>^f{ws}{integer}[/][/]{integer}.* {
    object->num_faces++;
    group->num_faces++;

    /* set the string pointer to the matched text (past the "f") */
    str = &yyOBJtext[1];

    /* count the number of vertex indexes */
    v = 0;
    while(1)
    {
	i = sscanf(str, "%d//%d%n", &tmp, &tmp, &skip);
	if(i != 2)
	    break;  /* bail if we didn't read anything (end of line) */

	/* position the string pointer past the indexes
	 * that have already been read 
	 */
	str = &str[skip];
	v++;
    }

    /* make sure we got at least 3 vertices */
    if(v < 3)
    {
	_tdPrintf(TD_ERROR, "**line %d: a face must have at least 3 vertices!",
		  num_lines);
	break;  /* break out -- don't get this face */
    }

    /* malloc a face */
    new_face = _tdNewFace(v, 0, v);
    if(!new_face)
	return(-1);
    if(!group->faces)
	group->faces = new_face;
    else
	face->next = new_face;
    face = new_face;
	
    /* reset the string pointer, and fill the new face with data */
    str = &yyOBJtext[1];

    v = 0;
    while(1)
    {
	i = sscanf(str, "%d//%d%n", 
		   &(face->vertices[v]), &(face->normals[v]), &skip);
	if(i != 2)
	    break;  /* bail if we didn't read anything (end of line) */

	/* check for negative references */
	if(face->vertices[v] < 0)
	    face->vertices[v] += object->num_vertices + 1;
	if(face->normals[v] < 0)
	    face->normals[v] += object->num_normals + 1;

	/* position the string pointer past the indexes
	 * that have already been read 
	 */
	str = &str[skip];
	v++;
    }

    /* set the number of vertices */
    face->num_vertices = v;
    face->num_normals  = v;
}

 /* face - f v ... */
<FIRSTPASS>^f{ws}{integer}.* {
    object->num_faces++;
    group->num_faces++;

    /* set the string pointer to the matched text (past the "f") */
    str = &yyOBJtext[1];

    /* count the number of vertex indexes */
    v = 0;
    while(1)
    {
	i = sscanf(str, "%d%n", &tmp, &skip);
	if(i != 1)
	    break;  /* bail if we didn't read anything (end of line) */

	/* position the string pointer past the indexes
	 * that have already been read 
	 */
	str = &str[skip];
	v++;
    }

    /* make sure we got at least 3 vertices */
    if(v < 3)
    {
	_tdPrintf(TD_ERROR, "**line %d: a face must have at least 3 vertices!",
		  num_lines);
	break;  /* break out -- don't get this face */
    }

    /* malloc a face */
    new_face = _tdNewFace(v, 0, 0);
    if(!new_face)
	return(-1);
    if(!group->faces)
	group->faces = new_face;
    else
	face->next = new_face;
    face = new_face;
	
    /* reset the string pointer, and fill the new face with data */
    str = &yyOBJtext[1];

    v = 0;
    while(1)
    {
	i = sscanf(str, "%d%n", &(face->vertices[v]), &skip);
	if(i != 1)
	    break;  /* bail if we didn't read anything (end of line) */

	/* check for negative references */
	if(face->vertices[v] < 0)
	    face->vertices[v] += object->num_vertices + 1;

	/* position the string pointer past the indexes
	 * that have already been read 
	 */
	str = &str[skip];
	v++;
    }

    /* set the number of vertices */
    face->num_vertices = v;
}

 /* group - g group_name ... */
<FIRSTPASS>^g.* {
    object->num_groups++;

    /* do this little trick to make sure
     * that we don't get spaces in our
     * group names
     */
    i = strlen(yyOBJtext);
    if(i > 1)
	i = 2;

    /* allocate a new one */
    new_group = _tdNewGroup(&yyOBJtext[i]);
    if(!new_group)
	return(-1);

    /* don't need to check group, because it will
     * initially be set to object->groups (default group)
     */
    group->next = new_group;
    group = new_group;
}

 /* material library - mtllib filename ... */
<FIRSTPASS>^mtllib{ws}{string}.* {
    /* get the library name */
    sscanf(&yyOBJtext[6], "%s", buf);

    material_filename = (char *)malloc(sizeof(char) * (strlen(buf) + 1));
    strcpy(material_filename, buf);

    /* TODO - get all the material files, not just the first one */
}

 /* usemtl - usemtl ... */
<FIRSTPASS>^usemtl{ws}{string}.*  ;  /* don't do anything this time */

 /* smoothing group - s num */
^s{ws}.*          ;

 /* unsupported keywords */
^vp{ws}.*         |
^cstype{ws}.*     |
^deg{ws}.*        |
^bmat{ws}.*       |
^step{ws}.*       |
^p{ws}.*          |
^l{ws}.*          |
^curv{ws}.*       |
^curv2{ws}.*      |
^surf{ws}.*       |
^parm{ws}.*       |
^trim{ws}.*       |
^hole{ws}.*       |
^scrv{ws}.*       |
^sp{ws}.*         |
^end{ws}.*        |
^con{ws}.*        |
^mg{ws}.*         |
^bevel{ws}.*      |
^c_interp{ws}.*   |
^d_interp{ws}.*   |
^lod{ws}.*        |
^mtllib{ws}.*     |
^maplib{ws}.*     |
^shadow_obj{ws}.* |
^trace_obj{ws}.*  |
^ctech{ws}.*      |
^stech{ws}.*      |
^call{ws}.*       |
^scmp{ws}.*       |
^csh{ws}.*        |
^usemap{ws}.*     num_ignored++;

 /* parse errors */
.* {
    if(num_pass == 1)  /* don't display on second pass */
    {
	num_errors++;
	
	_tdPrintf(TD_ERROR, "can't parse line %d: %s", num_lines, yyOBJtext);
	
	if(num_errors > MAX_ERRORS)
	{
	    _tdPrintf(TD_FATAL, "too many parse errors!");
	    exit(0);
	}
    }
}

%%

int yyOBJwrap()
{
    return(1);
}


/*
 *  tdReadOBJObject()
 * 
 *    Reads in object data in .obj (Wavefront) file format.
 *
 *    pathname - filename of the object to be read in.  It must be of the
 *    form path + name + ".obj".  Where path is the name of the path
 *    (full pathname if needed, but will go from current working directory)
 *    and name is the actual name of the object.
 *
 *    RETURNs a pointer to the object if all went well,
 *    otherwise, returns NULL.
 */
TDobject *_tdReadOBJObject(char *pathname)
{
    char *str;
    char filename[256];
    char objectname[256];
    FILE *file, *material_file;

    /* initialize variables */
    group = NULL; face = NULL; material_filename = NULL;
    num_ignored = 0; num_errors = 0;
    num_lines = 1;

    /* extract the directory name */
    strncpy(directory, pathname, sizeof(char) * 256);
    str = strrchr(directory, '/');
    if(str)
	str[1] = '\0';  /* terminate the directory[] at the '/' */
    else
	directory[0] = '\0';  /* if a '/' wasn't found, directory[] = NULL */

    /* extract the object name */
    str = strrchr(pathname, '/');
    if(str)
	strncpy(objectname, &str[1], 256);  /* copy all but the leading '/' */
    else
	strncpy(objectname, pathname, 256);   /* if no '/' found, just copy */

    /* allocate memory for the object */
    object = tdNewObject(objectname);
    if(!object)
        return(NULL);

    /* allocate memory for default group */
    new_group = _tdNewGroup("default");
    if(!new_group)
        return(NULL);
    object->groups = new_group;
    group = new_group;
    object->num_groups++;

    /* status */
    _tdPrintf(TD_INFO, "reading object file `%s'", objectname);


    /* set the start state to first pass */
    BEGIN FIRSTPASS;

    /* initially, count vertices, allocate groups, fill faces */
    file = fopen(pathname, "r");
    if(!file)
    {
        _tdPrintf(TD_ERROR, "can't open open object file `%s'!", pathname);
        return(NULL);
    }
    yyOBJin = file;
    if(yyOBJlex() == -1)
	return(NULL);
    fclose(file);

    /* allocate the vertex, vertex normal, and texture vertex memory */ 
    object->vertices = 
	(TDvertex *)malloc(sizeof(TDvertex) * (object->num_vertices + 1));
    if(!object->vertices)
    {
	_tdPrintf(TD_NOMEMORY, "malloc() failed for object vertices");
	return(NULL);
    }
    if(object->num_texvertices)
    {
	object->texvertices = 
	    (TDtexvertex *)malloc(sizeof(TDtexvertex) * 
				  (object->num_texvertices + 1));
	if(!object->texvertices)
	{
	    _tdPrintf(TD_NOMEMORY, 
		     "malloc() failed for object texture vertices");
	    return(NULL);
	}
    }
    if(object->num_normals)
    {
	object->normals = (TDnormal *)malloc(sizeof(TDnormal) * 
					     (object->num_normals + 1));
	if(!object->normals)
	{
	    _tdPrintf(TD_NOMEMORY, "malloc() failed for object normals");
	    return(NULL);
	}
    }


    /* read the material file (if one) */
    if(material_filename)
    {
	/* allocate memory for default material */
	new_material = _tdNewMaterial("default");
	if(!new_material)
	    return(NULL);
	object->materials = new_material;
	material = new_material;
	object->num_materials++;

	/* put the filename together */
	strncpy(filename, directory, sizeof(char) * 256);
	strncat(filename, material_filename,
		sizeof(char) * 256 - strlen(directory));
	
	/* status */
	_tdPrintf(TD_INFO, "reading material file `%s'", material_filename);

	/* set the state to material, so we only get that stuff */
	BEGIN MATERIAL;

	material_file = fopen(filename, "r");
	if(!material_file)
	{
	    _tdPrintf(TD_ERROR, "can't open material file `%s'!", filename);
	    return(NULL);
	}
	yyOBJin = material_file;
	if(yyOBJlex() == -1)
	    return(NULL);
	fclose(material_file);

	/* remember to free the filename (we allocated it in yyOBJlex()) */
	free(material_filename);
    }

    /* status report */
    if(num_ignored)
	_tdPrintf(TD_INFO, "ignored %d unsupported keywords", num_ignored);
    _tdPrintf(TD_INFO, "%d vertices, %d texvertices, %d normals",
	   object->num_vertices, object->num_texvertices, object->num_normals);
    _tdPrintf(TD_INFO, "%d materials, %d groups, %d faces", 
	      object->num_materials, object->num_groups, object->num_faces);

    /* (re)initialize some variables */
    num_vertices = num_texvertices = num_normals = 0;
    num_lines = 1; num_pass = 2;
    group = object->groups;


    /* okay, this time, read it in for real */
    BEGIN SECONDPASS;

    file = fopen(pathname, "r");
    if(file == NULL)
    {
	_tdPrintf(TD_ERROR, "can't open object file `%s'!", pathname);
        return(NULL);
    }
    yyOBJin = file;
    if(yyOBJlex() == -1)
      return(NULL);
    fclose(file);

    /* compute the bounding box */
    _tdBoundingBox(object);

    return(object);
}

/*
 *  _tdWriteOBJObject()
 *
 *    Writes out object data in .obj (Wavefront) file format to the stdout.
 *
 *    object - pointer to a valid object
 *
 *    pathname - filename of the object to be read in.  It must be of the
 *    form path + name + ".obj".  Where path is the name of the path
 *    (full pathname if needed, but will go from current working directory)
 *    and name is the actual name of the object.
 *
 *    RETURNs a TD_TRUE if all went well, otherwise, returns TD_FALSE.
 */
TDboolean _tdWriteOBJObject(TDobject *object, char *pathname)
{
    TDgroup *group;
    TDface *face;
    TDmaterial *material;
    FILE *file;
    char *str;
    int i;

    if(!object)
    {
        _tdPrintf(TD_ERROR, "can't write a null object!");
        return(TD_FALSE);
    }
    
    file = fopen(pathname, "w");
    if(!file)
    {
	_tdPrintf(TD_ERROR, "can't open file for writing `%s'!", pathname);
	return(NULL);
    }

    _tdPrintf(TD_INFO, "writing object file `%s'", pathname);

    fprintf(file, "##\n"); 
    fprintf(file, "##  Three-D Library generated .obj file\n");
    fprintf(file, "##  Object: %s\n", object->name);
    fprintf(file, "##\n\n"); 

    /* write vertices */
    for(i = 1; i < object->num_vertices + 1; i++)
      fprintf(file, "v %f %f %f\n",
	      object->vertices[i].x,
	      object->vertices[i].y, 
	      object->vertices[i].z);
    fprintf(file, "# %d vertices\n\n", object->num_vertices);

    /* write normals (if any) */
    if(object->num_normals)
        for(i = 1; i < object->num_normals + 1; i++)
            fprintf(file, "vn %f %f %f\n",
		    object->normals[i].i,
		    object->normals[i].j,
		    object->normals[i].k);
    fprintf(file, "# %d vertex normals\n\n", object->num_normals);

    /* write texture vertices (if any) */
    if(object->num_texvertices)
        for(i = 1; i < object->num_texvertices + 1; i++)
            fprintf(file, "vt %f %f\n", object->texvertices[i].u,
                   object->texvertices[i].v);
    fprintf(file, "# %d texture vertices\n\n", object->num_texvertices);

    /* write all the group data */
    group = object->groups;
    while(group)
    {
        fprintf(file, "g %s\n", group->name);
	if(group->material)
	    fprintf(file, "usemtl %s\n", group->material->name);

	/* write face data for this group */
        face = group->faces;
        while(face)
        {
	    fprintf(file, "f");
	    for(i = 0; i < face->num_vertices; i++)
	    {
		if(!face->num_normals && !face->num_texvertices)
		    fprintf(file, " %d", face->vertices[i]);
		else if(face->num_normals && !face->num_texvertices)
		    fprintf(file, " %d//%d", 
			    face->vertices[i],
			    face->normals[i]);
		else if(face->num_texvertices && !face->num_normals)
		    fprintf(file, " %d/%d",
			    face->vertices[i],
			    face->texvertices[i]);
		else
		    fprintf(file, " %d/%d/%d",
			    face->vertices[i],
			    face->texvertices[i],
			    face->normals[i]);
	    }
	    fprintf(file, "\n");
	    
	    face = face->next;
	}
        fprintf(file, "# %d faces\n\n", group->num_faces);
	
        group = group->next;
    }

    fclose(file);

    /* bail now if we don't have any material
     * stuff to write out
     */
    if(!object->materials)
	return(TD_TRUE);

    /* write the material file */

    /* find the .obj in the pathname, and replace it
     * with .mtl for the material file name
     */
    pathname[strlen(pathname) - 3] = 'm';
    pathname[strlen(pathname) - 2] = 't';
    pathname[strlen(pathname) - 1] = 'l';

    file = fopen(pathname, "w");
    if(!file)
    {
	_tdPrintf(TD_ERROR, "can't open file for writing `%s'!", pathname);
	return(NULL);
    }

    _tdPrintf(TD_INFO, "writing material file `%s'.", pathname);

    fprintf(file, "##\n"); 
    fprintf(file, "##  Three-D Library generated .mtl file\n");
    fprintf(file, "##  Material Library for Object: %s\n", object->name);
    fprintf(file, "##\n\n"); 

    /* write materials */
    material = object->materials;

    /* skip the default material */
    if(material->next)
	material = material->next;

    while(material)
    {
	fprintf(file, "newmtl %s\n", material->name);
	fprintf(file, "Ka %f %f %f\n",
		material->ambient.r,
		material->ambient.g,
		material->ambient.b);
	fprintf(file, "Kd %f %f %f\n",
		material->diffuse.r,
		material->diffuse.g,
		material->diffuse.b);
	fprintf(file, "Ks %f %f %f\n",
		material->specular.r,
		material->specular.g,
		material->specular.b);
	if(material->shininess > 0.0)
	    fprintf(file, "Ns %f\n", material->shininess * 1000 / 128);
	if(material->teximage)
	    fprintf(file, "map_Kd %s\n", material->teximage);

	fprintf(file, "\n");

	material = material->next;
    }

    fclose(file);

    /* put the pathname back the way we found it */
    pathname[strlen(pathname) - 3] = 'o';
    pathname[strlen(pathname) - 2] = 'b';
    pathname[strlen(pathname) - 1] = 'j';

    return(TD_TRUE);
}

