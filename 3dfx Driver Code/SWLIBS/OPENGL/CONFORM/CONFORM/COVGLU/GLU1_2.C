/*
** Copyright 1997, Silicon Graphics, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of Silicon Graphics, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of Silicon Graphics, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
*/

#include <stdio.h>
#include "shell.h"


/* Added for glu 1.2 */

void ProbeEnum1_2(void)
{

    if (glGetError() == GL_INVALID_ENUM) {
        printf("covglu failed for level glu 1.2.\n\n");
        tkQuit();
    }
}

void ProbeError1_2(void (*Func)(void))
{

    (*Func)();
    if (glGetError() != GL_NO_ERROR) {
	printf("covglu failed for glu at level 1.2.\n\n");
	tkQuit();
    }
}

/* Calls which existed under 1.1 but accept new enumerants */

static void beginData(GLenum type, void *polygon_data)
{
    return;
}

static void endData(void *polygon_data)
{
    return;
}

static void combine(GLdouble coords[3], void *vertex_data[4],
		    GLfloat weight[4], void **outData)
{
    return;
}

static void combineData(GLdouble coords[3], void *vertex_data[4],
			GLfloat weight[4], void **outData, void *polygon_data)
{
    return;
}

static void vertexData(void *vertex_data, void *polygon_data)
{
    return;
}

static void errorData(GLenum errno, void *polygon_data)
{
    return;
}

void CallTessCallback1_2(void)
{

    Output("gluTessCallback with 1.2 enumerants\n");

    Output("\tGLU_TESS_BEGIN_DATA\n");
    gluTessCallback(tessObj, GLU_TESS_BEGIN_DATA, beginData);
    ProbeEnum1_2();

    Output("\tGLU_TESS_END_DATA\n");
    gluTessCallback(tessObj, GLU_TESS_END_DATA, endData);
    ProbeEnum1_2();
    
    Output("\tGLU_TESS_COMBINE\n");
    gluTessCallback(tessObj, GLU_TESS_COMBINE, combine);
    ProbeEnum1_2();

    Output("\tGLU_TESS_COMBINE_DATA\n");
    gluTessCallback(tessObj, GLU_TESS_COMBINE_DATA, combineData);
    ProbeEnum1_2();

    Output("\tGLU_TESS_VERTEX_DATA\n");
    gluTessCallback(tessObj, GLU_TESS_VERTEX_DATA, vertexData);
    ProbeEnum1_2();

    Output("\tGLU_TESS_ERROR_DATA\n");
    gluTessCallback(tessObj, GLU_TESS_ERROR_DATA, errorData);
    ProbeEnum1_2();

    Output("\n"); 
}

/* Calls which only exist after glu 1.1 */

void CallTessProperty(void)
{
    long i;

    Output("gluTessProperty\n");

    for (i = 0; enum_TessWindingRule[i].value != -1; i++) {
	Output("\tGLU_TESS_WINDING_RULE %s\n", enum_TessWindingRule[i].name);
	gluTessProperty(tessObj, GLU_TESS_WINDING_RULE, 
			(GLdouble)enum_TessWindingRule[i].value);
	ProbeEnum1_2();
    }

    Output("\tGLU_TESS_BOUNDARY_ONLY, TRUE\n");
    gluTessProperty(tessObj, GLU_TESS_BOUNDARY_ONLY, (GLdouble)GL_TRUE); 
    ProbeEnum1_2();

    Output("\tGLU_TESS_BOUNDARY_ONLY, FALSE\n");
    gluTessProperty(tessObj, GLU_TESS_BOUNDARY_ONLY, (GLdouble)GL_FALSE); 
    ProbeEnum1_2();

    Output("\tGLU_TESS_TOLERANCE\n");
    gluTessProperty(tessObj, GLU_TESS_TOLERANCE,  0.0); 

    Output("\n"); 
}

void CallGetTessProperty(void)
{
    long i;
    double buf;

    Output("gluGetTessProperty\n");

    for (i = 0; enum_TessWindingRule[i].value != -1; i++) {
	Output("\tGLU_TESS_WINDING_RULE, %s\n", enum_TessWindingRule[i].name);
	buf = enum_TessWindingRule[i].value;
	gluGetTessProperty(tessObj, GLU_TESS_WINDING_RULE, &buf);
	ProbeEnum1_2();
    }
    
    Output("\tGLU_TESS_BOUNDARY_ONLY, GL_TRUE\n");
    buf = GL_TRUE;
    gluGetTessProperty(tessObj, GLU_TESS_BOUNDARY_ONLY, &buf); 
    Output("\tGLU_TESS_BOUNDARY_ONLY, GL_FALSE\n");
    buf = GL_FALSE;
    gluGetTessProperty(tessObj, GLU_TESS_BOUNDARY_ONLY, &buf); 
    ProbeEnum1_2();

    Output("\tGLU_TESS_TOLERANCE\n");
    buf = 0.0;
    gluGetTessProperty(tessObj, GLU_TESS_TOLERANCE, &buf);
    ProbeEnum1_2();

    Output("\n"); 
}

void CallTessBeginPolygon(void)
{

    Output("gluTessBeginPolygon\n");

    gluTessBeginPolygon(tessObj, NULL); 
    ProbeEnum1_2();

    Output("\n"); 
}

void CallTessBeginContour(void)
{

    Output("gluTessBeginContour\n");

    gluTessBeginContour(tessObj);
    ProbeEnum1_2();

    Output("\n"); 
}

void CallTessNormal(void)
{

    Output("gluTessNormal\n");

    gluTessNormal(tessObj, 0.0, 0.0, 1.0);  /* takes doubles */
    ProbeEnum1_2();

    Output("\n"); 
}

void CallTessEndContour(void)
{

    Output("gluTessEndContour\n");

    gluTessEndContour(tessObj);
    ProbeEnum1_2();

    Output("\n"); 
}

void CallTessEndPolygon(void)
{

    Output("gluTessEndPolygon\n");

    gluTessEndPolygon(tessObj);
    ProbeEnum1_2();

    Output("\n"); 
}
