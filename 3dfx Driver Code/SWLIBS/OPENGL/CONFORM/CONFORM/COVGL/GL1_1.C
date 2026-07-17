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
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "shell.h"


#if GL_VERSION_1_1

void FailAndDie1_1(void)
{

    Output("\n");
    printf("covgl fail for version 1.1.\n\n");
    tkQuit();
}

void ProbeEnum1_1(void)
{

    if (glGetError() == GL_INVALID_ENUM) {
	FailAndDie1_1();
    }
}

void ProbeError1_1(void (*Func)(void))
{

    glPushAttrib(GL_ALL_ATTRIB_BITS);

    (*Func)();
    if (glGetError() != GL_NO_ERROR) {
	FailAndDie1_1();
    }

    glPopAttrib();
}

void CallPolygonOffset(void)
{

    Output("glPolygonOffset\n");
    glPolygonOffset(1.0f, 0.0f);
    ProbeEnum1_1();
    Output("\n");
}

void CallArrayElement(void)
{

    Output("glArrayElement\n");
    {
	GLfloat buf[4];
	buf[0] = 0.0;
	buf[1] = 0.0;
	buf[2] = 0.0;
	buf[3] = 1.0;
	Output("\tGL_FLOAT\n");
	glColorPointer(4, GL_FLOAT, 0, buf);
    }
    glArrayElement(0);  /*XXX maybe one would be better */
    ProbeEnum1_1();
    Output("\n");
}

void CallColorPointer(void)
{
    int i;

    Output("glColorPointer\n");
    {
	GLbyte buf[4];
	buf[0] = 0;
	buf[1] = 0;
	buf[2] = 0;
	buf[3] = 127;
	Output("\tGL_BYTE\n");
	for (i = 3; i <= 4; i++) {
	    glColorPointer(i, GL_BYTE, 0, buf);
	    ProbeEnum1_1();
	}
    }
    {
	GLubyte buf[4];
	buf[0] = 0;
	buf[1] = 0;
	buf[2] = 0;
	buf[3] = 255;
	Output("\tGL_UNSIGNED_BYTE\n");
	for (i = 3; i <= 4; i++) {
	    glColorPointer(i, GL_UNSIGNED_BYTE, 0, buf);
	    ProbeEnum1_1();
	}
    }
    {
	GLshort buf[4];
	buf[0] = 0;
	buf[1] = 0;
	buf[2] = 0;
	buf[3] = 32767;
	Output("\tGL_SHORT\n");
	for (i = 3; i <= 4; i++) {
	    glColorPointer(i, GL_SHORT, 0, buf);
	    ProbeEnum1_1();
	}
    }
    {
	GLushort buf[4];
	buf[0] = 0;
	buf[1] = 0;
	buf[2] = 0;
	buf[3] = 65535;
	Output("\tGL_UNSIGNED_SHORT\n");
	for (i = 3; i <= 4; i++) {
	    glColorPointer(i, GL_UNSIGNED_SHORT, 0, buf);
	    ProbeEnum1_1();
	}
    }
    {
	GLint buf[4];
	buf[0] = 0;
	buf[1] = 0;
	buf[2] = 0;
	buf[3] = 0;
	Output("\tGL_INT\n");
	for (i = 3; i <= 4; i++) {
	    glColorPointer(i, GL_INT, 0, buf);
	    ProbeEnum1_1();
	}
    }
    {
	GLuint buf[4];
	buf[0] = 0;
	buf[1] = 0;
	buf[2] = 0;
	buf[3] = 0;
	Output("\tGL_UNSIGNED_INT\n");
	for (i = 3; i <= 4; i++) {
	    glColorPointer(i, GL_UNSIGNED_INT, 0, buf);
	    ProbeEnum1_1();
	}
    }
    {
	GLfloat buf[4];
	buf[0] = 0.0f;
	buf[1] = 0.0f;
	buf[2] = 0.0f;
	buf[3] = 1.0f;
	Output("\tGL_FLOAT\n");
	for (i = 3; i <= 4; i++) {
	    glColorPointer(i, GL_FLOAT, 0, buf);
	    ProbeEnum1_1();
	}
    }
    {
	GLdouble buf[4];
	buf[0] = 0.0;
	buf[1] = 0.0;
	buf[2] = 0.0;
	buf[3] = 1.0;
	Output("\tGL_DOUBLE\n");
	for (i = 3; i <= 4; i++) {
	    glColorPointer(i, GL_DOUBLE, 0, buf);
	    ProbeEnum1_1();
	}
    }

    Output("\n");
}

void CallDrawArrays(void)
{
    int i;

    Output("glDrawArrays\n");
    {
	GLfloat buf[4];
	buf[0] = 0.0f;
	buf[1] = 0.0f;
	buf[2] = 0.0f;
	buf[3] = 1.0f;
	glColorPointer(4, GL_FLOAT, 0, buf);
	glVertexPointer(4, GL_FLOAT, 0, buf);
    }

    for (i = 0; enum_BeginMode[i].value != -1; i++) {
	Output("\t%s\n", enum_BeginMode[i].name);

	/* XXX is the zeroth element legal */
	glDrawArrays(enum_BeginMode[i].value, 0, 1); 
	ProbeEnum1_1();
    }
    Output("\n");
}

void CallDrawElements(void)
{
    int i;

    Output("glDrawElements\n");
    {
	GLfloat buf[4];
	buf[0] = 0.0f;
	buf[1] = 0.0f;
	buf[2] = 0.0f;
	buf[3] = 1.0f;
	glColorPointer(4, GL_FLOAT, 0, buf);
	glVertexPointer(4, GL_FLOAT, 0, buf);
    }

    for (i = 0; enum_BeginMode[i].value != -1; i++) {
	Output("\t%s\n", enum_BeginMode[i].name);

	/* XXX is the zeroeth element legal */
	{
	    GLubyte indices[1];
	    indices[0] = 0;  /* XXX should this be one?*/
	    glDrawElements(enum_BeginMode[i].value, 1, 
		GL_UNSIGNED_BYTE, indices); 
	}
	ProbeEnum1_1();
	{
	    GLushort indices[1];
	    indices[0] = 0;  /* XXX should this be one?*/
	    glDrawElements(enum_BeginMode[i].value, 1, 
		GL_UNSIGNED_SHORT, indices); 
	}
	ProbeEnum1_1();
	{
	    GLuint indices[1];
	    indices[0] = 0;  /* XXX should this be one?*/
	    glDrawElements(enum_BeginMode[i].value, 1, 
		GL_UNSIGNED_INT, indices); 
	}
	ProbeEnum1_1();
    }
    Output("\n");
}

void CallIndexPointer(void)
{

    Output("glIndexPointer\n");
    {
	GLubyte buf[1];
	buf[0] = 0;
	Output("\tGL_UNSIGNED_BYTE\n");
	glIndexPointer(GL_UNSIGNED_BYTE, 0, buf);
    }
    ProbeEnum1_1();
    {
	GLshort buf[1];
	buf[0] = 0;
	Output("\tGL_SHORT\n");
	glIndexPointer(GL_SHORT, 0, buf);
    }
    ProbeEnum1_1();
    {
	GLint buf[1];
	buf[0] = 0;
	Output("\tGL_INT\n");
	glIndexPointer(GL_INT, 0, buf);
    }
    ProbeEnum1_1();
    {
	GLfloat buf[1];
	buf[0] = 0.0f;
	Output("\tGL_FLOAT\n");
	glIndexPointer(GL_FLOAT, 0, buf);
    }
    ProbeEnum1_1();
    {
	GLdouble buf[1];
	buf[0] = 0.0;
	Output("\tGL_DOUBLE\n");
	glIndexPointer(GL_DOUBLE, 0, buf);
    }
    ProbeEnum1_1();
    Output("\n");
}

void CallInterleavedArrays(void)
{
    GLfloat buf[16];
    GLubyte mixedBuf[16*sizeof(GLfloat)];
    GLubyte *mixBufUbytePtr;
    int i;

    for (i = 0; i < 16; i++) {
	buf[i] = 1.0f;
    }

    Output("glInterleavedArrays\n");
    {
	Output("\tGL_V2F\n");
	glInterleavedArrays(GL_V2F, 0, buf);
    }
    ProbeEnum1_1();
    {
	Output("\tGL_V3F\n");
	glInterleavedArrays(GL_V3F, 0, buf);
    }
    ProbeEnum1_1();
    {
	mixBufUbytePtr = (GLubyte *)mixedBuf;
	for (i = 0; i < 16; i++) {
	    buf[i] = 1.0f;
	}
	for (i = 0; i < 4; i++) {
	    mixBufUbytePtr[i] = 255;
	}
	Output("\tGL_C4UB_V2F\n");
	glInterleavedArrays(GL_C4UB_V2F, 0, buf);
	for (i = 0; i < 16; i++) {
	    buf[i] = 1.0f;
	}

    }
    ProbeEnum1_1();
    {
	mixBufUbytePtr = (GLubyte *)mixedBuf;
	for (i = 0; i < 16; i++) {
	    buf[i] = 1.0f;
	}
	for (i = 0; i < 4; i++) {
	    mixBufUbytePtr[i] = 255;
	}
	Output("\tGL_C4UB_V3F\n");
	glInterleavedArrays(GL_C4UB_V3F, 0, buf);
	for (i = 0; i < 16; i++) {
	    buf[i] = 1.0f;
	}
    }
    ProbeEnum1_1();
    {
	Output("\tGL_C3F_V3F\n");
	glInterleavedArrays(GL_C3F_V3F, 0, buf);
    }
    ProbeEnum1_1();
    {
	Output("\tGL_N3F_V3F\n");
	glInterleavedArrays(GL_N3F_V3F, 0, buf);
    }
    ProbeEnum1_1();
    {
	Output("\tGL_C4F_N3F_V3F\n");
	glInterleavedArrays(GL_C4F_N3F_V3F, 0, buf);
    }
    ProbeEnum1_1();
    {
	Output("\tGL_T2F_V3F\n");
	glInterleavedArrays(GL_T2F_V3F, 0, buf);
    }
    ProbeEnum1_1();
    {
	Output("\tGL_T4F_V4F\n");
	glInterleavedArrays(GL_T4F_V4F, 0, buf);
    }
    ProbeEnum1_1();
    {
	Output("\tGL_T2F_C4UB_V4F\n");
	mixBufUbytePtr = (GLubyte *)&mixedBuf[3];
	for (i = 0; i < 16; i++) {
	    buf[i] = 1.0f;
	}
	for (i = 0; i < 4; i++) {
	    mixBufUbytePtr[i] = 255;
	}
	Output("\tGL_T2F_C4UB_V3F\n");
	glInterleavedArrays(GL_T2F_C4UB_V3F, 0, buf);
	for (i = 0; i < 16; i++) {
	    buf[i] = 1.0f;
	}

    }
    ProbeEnum1_1();
    {
	Output("\tGL_T2F_C3F_V3F\n");
	glInterleavedArrays(GL_T2F_C3F_V3F, 0, buf);
    }
    ProbeEnum1_1();
    {
	Output("\tGL_T2F_N3F_V3F\n");
	glInterleavedArrays(GL_T2F_N3F_V3F, 0, buf);
    }
    ProbeEnum1_1();
    {
	Output("\tGL_T2F_C4F_N3F_V3F\n");
	glInterleavedArrays(GL_T2F_C4F_N3F_V3F, 0, buf);
    }
    ProbeEnum1_1();
    {
	Output("\tGL_T4F_C4F_N3F_V4F\n");
	glInterleavedArrays(GL_T4F_C4F_N3F_V4F, 0, buf);
    }
    ProbeEnum1_1();

    Output("\n");
}

void CallEdgeFlagPointer(void)
{
    GLboolean buf[1];

    buf[0] = GL_TRUE; 

    Output("glEdgeFlagPointer\n");
    glEdgeFlagPointer(0, buf);
    ProbeEnum1_1();
    Output("\n");
}

void CallGetPointer(void)
{
    int i;
    GLvoid *pointer;

    Output("glGetPointerv\n");
    for (i = 0; enum_GetPointerTarget1_1[i].value != -1; i++) {
	Output("\t%s\n", enum_GetPointerTarget1_1[i].name);
	glGetPointerv(enum_GetPointerTarget1_1[i].value, &pointer);
	ProbeEnum1_1();
    }
    Output("\n");
}
 
void CallNormalPointer(void)
{

    Output("glNormalPointer\n");
    {
	GLbyte buf[3];
	buf[0] = 0;
	buf[1] = 0;
	buf[2] = 1;
	Output("\tGL_BYTE\n");
	glNormalPointer(GL_BYTE, 0, buf);
    }
    ProbeEnum1_1();
    {
	GLshort buf[3];
	buf[0] = 0;
	buf[1] = 0;
	buf[2] = 1;
	Output("\tGL_SHORT\n");
	glNormalPointer(GL_SHORT, 0, buf);
    }
    ProbeEnum1_1();
    {
	GLint buf[3];
	buf[0] = 0;
	buf[1] = 0;
	buf[2] = 1;
	Output("\tGL_INT\n");
	glNormalPointer(GL_INT, 0, buf);
    }
    ProbeEnum1_1();
    {
	GLfloat buf[3];
	buf[0] = 0.0f;
	buf[1] = 0.0f;
	buf[2] = 1.0f;
	Output("\tGL_FLOAT\n");
	glNormalPointer(GL_FLOAT, 0, buf);
    }
    ProbeEnum1_1();
    {
	GLdouble buf[3];
	buf[0] = 0.0f;
	buf[1] = 0.0f;
	buf[2] = 1.0f;
	Output("\tGL_DOUBLE\n");
	glNormalPointer(GL_DOUBLE, 0, buf);
    }
    ProbeEnum1_1();
    Output("\n");
}

void CallTexCoordPointer(void)
{
    int i;

    Output("glTexCoordPointer\n");
    {
	GLshort buf[4];
	buf[0] = 0;
	buf[1] = 0;
	buf[2] = 0;
	buf[3] = 1;
	Output("\tGL_SHORT\n");
	for (i = 1; i <= 4; i++) {
	    glTexCoordPointer(i, GL_SHORT, 0, buf);
	    ProbeEnum1_1();
	}
    }
    {
	GLint buf[4];
	buf[0] = 0;
	buf[1] = 0;
	buf[2] = 0;
	buf[3] = 1;
	Output("\tGL_INT\n");
	for (i = 1; i <= 4; i++) {
	    glTexCoordPointer(i, GL_INT, 0, buf);
	    ProbeEnum1_1();
	}
    }
    {
	GLfloat buf[4];
	buf[0] = 0.0f;
	buf[1] = 0.0f;
	buf[2] = 0.0f;
	buf[3] = 1.0f;
	Output("\tGL_FLOAT\n");
	for (i = 1; i <= 4; i++) {
	    glTexCoordPointer(i, GL_FLOAT, 0, buf);
	    ProbeEnum1_1();
	}
    }
    {
	GLdouble buf[4];
	buf[0] = 0.0f;
	buf[1] = 0.0f;
	buf[2] = 0.0f;
	buf[3] = 1.0f;
	Output("\tGL_DOUBLE\n");
	for (i = 1; i <= 4; i++) {
	    glTexCoordPointer(i, GL_DOUBLE, 0, buf);
	    ProbeEnum1_1();
	}
    }

    Output("\n");
}

void CallVertexPointer(void)
{
    int i;

    Output("glVertexPointer\n");
    {
	GLshort buf[4];
	buf[0] = 0;
	buf[1] = 0;
	buf[2] = 0;
	buf[3] = 1;
	Output("\tGL_SHORT\n");
	for (i = 2; i <= 4; i++) {
	    glVertexPointer(i, GL_SHORT, 0, buf);
	    ProbeEnum1_1();
	}
    }
    {
	GLint buf[4];
	buf[0] = 0;
	buf[1] = 0;
	buf[2] = 0;
	buf[3] = 1;
	Output("\tGL_INT\n");
	for (i = 2; i <= 4; i++) {
	    glVertexPointer(i, GL_INT, 0, buf);
	    ProbeEnum1_1();
	}
    }
    {
	GLfloat buf[4];
	buf[0] = 0.0f;
	buf[1] = 0.0f;
	buf[2] = 0.0f;
	buf[3] = 1.0f;
	Output("\tGL_FLOAT\n");
	for (i = 2; i <= 4; i++) {
	    glVertexPointer(i, GL_FLOAT, 0, buf);
	    ProbeEnum1_1();
	}
    }
    {
	GLdouble buf[4];
	buf[0] = 0.0f;
	buf[1] = 0.0f;
	buf[2] = 0.0f;
	buf[3] = 1.0f;
	Output("\tGL_DOUBLE\n");
	for (i = 2; i <= 4; i++) {
	    glVertexPointer(i, GL_DOUBLE, 0, buf);
	    ProbeEnum1_1();
	}
    }

    Output("\n");
}

void CallEnableDisableClientState(void)
{
    long i;

    Output("glEnableClientState, ");
    Output("glDisableClientState\n");
    for (i = 0; enum_EnableClientState1_1[i].value != -1; i++) {
	Output("\t%s\n", enum_EnableClientState1_1[i].name);
	glEnableClientState(enum_EnableClientState1_1[i].value);
	glDisableClientState(enum_EnableClientState1_1[i].value);
	ProbeEnum1_1();
    }
    Output("\n");
}

void CallPushPopClientAttrib(void)
{
    Output("glPushClientAttrib, ");
    Output("glPopClientAttrib\n ");

    Output("\tGL_CLIENT_VERTEX_ARRAY_BIT\n");
    glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
    glPopClientAttrib();
    ProbeEnum1_1();

    Output("\tGL_CLIENT_PIXEL_STORE_BIT\n");
    glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);
    glPopClientAttrib();
    ProbeEnum1_1();

    Output("\tGL_CLIENT_ALL_ATTRIB_BITS\n");
    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    glPopClientAttrib();
    ProbeEnum1_1();
    Output("\n");
}

void CallAreTexturesResident(void)
{
    GLboolean tmp, residences[2];
    GLuint textures[2];

    Output("glAreTexturesResident\n");

    textures[0] = 1;
    textures[1] = 2;

    glBindTexture(GL_TEXTURE_1D, 1);
    glBindTexture(GL_TEXTURE_2D, 2);
    tmp = glAreTexturesResident(2, textures, residences);
    glDeleteTextures(2, textures);
    ProbeEnum1_1();
    Output("\n");
}

void CallBindDeleteTexture(void)
{
    GLuint buf[2];

    Output("BindTexture, ");
    Output("DeleteTexture\n");

    buf[0] = 2;
    buf[1] = 1;

    Output("\tGL_TEXTURE_2D\n");
    glBindTexture(GL_TEXTURE_2D, buf[0]);

    Output("\tGL_TEXTURE_1D\n");
    glBindTexture(GL_TEXTURE_1D, buf[1]);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindTexture(GL_TEXTURE_1D, 0);

    glDeleteTextures(2, buf);

    ProbeEnum1_1();
    Output("\n");
}

void CallCopyTexImage1D(void)
{
    int i, j;

    Output("glCopyTexImage1D\n");
    for (i = 0; enum_CopyTextureInternalFormat[i].value != -1; i++) {
	for (j = 0; enum_TextureBorder[j].value != -1; j++) {
	    Output("\t%s, %s\n",
		   enum_CopyTextureInternalFormat[i].name, 
		   enum_TextureBorder[j].name);
	    glCopyTexImage1D(GL_TEXTURE_1D, 0, 
		             enum_CopyTextureInternalFormat[i].value, 
		             0, 0,
			     (enum_TextureBorder[j].value) ? 3 : 1,
		             enum_TextureBorder[j].value);
	}
    }
    ProbeEnum1_1();
    Output("\n");
}

void CallCopyTexImage2D(void)
{
    int i, j;

    Output("glCopyTexImage2D\n");
    for (i = 0; enum_CopyTextureInternalFormat[i].value != -1; i++) {
	for (j = 0; enum_TextureBorder[j].value != -1; j++) {
	    Output("\t%s, %s\n",
		   enum_CopyTextureInternalFormat[i].name, 
		   enum_TextureBorder[j].name);
	    glCopyTexImage2D(GL_TEXTURE_2D, 0, 
		             enum_CopyTextureInternalFormat[i].value, 
		             0, 0, 
		             (enum_TextureBorder[j].value) ? 3 : 1,
		             (enum_TextureBorder[j].value) ? 3 : 1,
		             enum_TextureBorder[j].value);
	}
    }
    ProbeEnum1_1();
    Output("\n");
}

void CallGenTextures(void)
{
    GLuint tex[2];

    Output("glGenTextures\n");
    glGenTextures(2, tex);
    ProbeEnum1_1();
    Output("\n");
}

void CallIsTexture(void)
{
    GLboolean tmp;

    Output("glIsTexture\n");
    tmp = glIsTexture(100);
    ProbeEnum1_1();
    Output("\n");
}

void CallPrioritizeTextures(void)
{
    GLuint tex[1];
    GLclampf priorities[1];

    Output("glPrioritizeTextures\n");

    tex[0] = 0;
    priorities[0] = 0.0f;

    glPrioritizeTextures(1, tex, priorities);

    ProbeEnum1_1();
    Output("\n");
}

void CallTexSubImage1D(void)
{
    GLubyte buf[1000];
    int i, j, k;

    Output("glTexSubImage1D\n");
    for (i = 0; enum_TextureFormat1_1[i].value != -1; i++) {
	for (j = 0; enum_TextureType1_1[j].value != -1; j++) {
	    for (k = 0; enum_TextureBorder[k].value != -1; k++) {

		Output("\t%s, %s, %s non-Null texture\n", 
		       enum_TextureFormat1_1[i].name, 
		       enum_TextureType1_1[j].name, 
		       enum_TextureBorder[k].name);
		ZeroBuf(enum_TextureType1_1[j].value, 100, buf);
		glTexImage1D(GL_TEXTURE_1D, 0, 4, 
		             (enum_TextureBorder[k].value) ? 3 : 1, 
		             enum_TextureBorder[k].value, 
		             enum_TextureFormat1_1[i].value, 
		             enum_TextureType1_1[j].value, buf);
		glTexSubImage1D(GL_TEXTURE_1D, 0, 
		                -enum_TextureBorder[k].value, 1,
		                enum_TextureFormat1_1[i].value, 
		                enum_TextureType1_1[j].value, buf);

		ProbeEnum1_1();

		Output("\t%s, %s, %s Null texture\n", 
		       enum_TextureFormat1_1[i].name, 
		       enum_TextureType1_1[j].name, 
		       enum_TextureBorder[k].name);
		glTexImage1D(GL_TEXTURE_1D, 0, 4, 
		             ((enum_TextureBorder[k].value) ? 3 : 1), 
		             enum_TextureBorder[k].value, 
		             enum_TextureFormat1_1[i].value, 
		             enum_TextureType1_1[j].value, 
		             (const GLvoid *)NULL);
		glTexSubImage1D(GL_TEXTURE_1D, 0, 
		                -enum_TextureBorder[k].value, 1,
		                enum_TextureFormat1_1[i].value, 
		                enum_TextureType1_1[j].value, buf);
		ProbeEnum1_1();
	    }
	}
    }
    Output("\n");
}

void CallTexSubImage2D(void)
{
    GLubyte buf[1000];
    int i, j, k;

    Output("glTexSubImage2D\n");
    for (i = 0; enum_TextureFormat1_1[i].value != -1; i++) {
	for (j = 0; enum_TextureType1_1[j].value != -1; j++) {
	    for (k = 0; enum_TextureBorder[k].value != -1; k++) {

		Output("\t%s, %s, %s non-Null texture\n", 
		       enum_TextureFormat1_1[i].name, 
		       enum_TextureType1_1[j].name, 
		       enum_TextureBorder[k].name);
		ZeroBuf(enum_TextureType1_1[j].value, 100, buf);
		glTexImage2D(GL_TEXTURE_2D, 0, 4, 
		             (enum_TextureBorder[k].value) ? 3 : 1, 
		             (enum_TextureBorder[k].value) ? 3 : 1, 
		             enum_TextureBorder[k].value, 
		             enum_TextureFormat1_1[i].value, 
		             enum_TextureType1_1[j].value, buf);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 
		                -enum_TextureBorder[k].value, 
		                -enum_TextureBorder[k].value, 
		                1, 1, 
		                enum_TextureFormat1_1[i].value, 
		                enum_TextureType1_1[j].value, buf);

		ProbeEnum1_1();

		Output("\t%s, %s, %s Null texture\n", 
		       enum_TextureFormat1_1[i].name, 
		       enum_TextureType1_1[j].name, 
		       enum_TextureBorder[k].name);
		glTexImage2D(GL_TEXTURE_2D, 0, 4, 
		             (enum_TextureBorder[k].value) ? 3 : 1, 
		             (enum_TextureBorder[k].value) ? 3 : 1, 
		             enum_TextureBorder[k].value, 
		             enum_TextureFormat1_1[i].value, 
		             enum_TextureType1_1[j].value,
			     (const GLvoid *)NULL);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 
		                -enum_TextureBorder[k].value, 
		                -enum_TextureBorder[k].value, 
		                1, 1,
		                enum_TextureFormat1_1[i].value, 
		                enum_TextureType1_1[j].value, buf);
		ProbeEnum1_1();
	    }
	}
    }
    Output("\n");
}

void CallCopyTexSubImage1D(void)
{
    GLubyte buf[1000];
    int i;

    Output("glCopyTexSubImage1D\n");
    ZeroBuf(GL_UNSIGNED_BYTE, 100, buf);
    for (i = 0; enum_TextureBorder[i].value != -1; i++) {
	Output("\t%s non-Null Texture\n", enum_TextureBorder[i].name);
	glTexImage1D(GL_TEXTURE_1D, 0,  4,
	             (enum_TextureBorder[i].value) ? 3 : 1, 
	             enum_TextureBorder[i].value, GL_RGBA,
	             GL_UNSIGNED_BYTE, buf);
	glCopyTexSubImage1D(GL_TEXTURE_1D, 0, -enum_TextureBorder[i].value,
			    0, 0, 1);
	ProbeEnum1_1();

	Output("\t%s Null Texture\n", enum_TextureBorder[i].name);
	glTexImage1D(GL_TEXTURE_1D, 0,  4,
	             (enum_TextureBorder[i].value) ? 3 : 1, 
	             enum_TextureBorder[i].value, GL_RGBA,
	             GL_UNSIGNED_BYTE, (const GLvoid *)NULL);
	glCopyTexSubImage1D(GL_TEXTURE_1D, 0, -enum_TextureBorder[i].value,
			    0, 0, 1);
	ProbeEnum1_1();
    }
    Output("\n");
}

void CallCopyTexSubImage2D(void)
{
    GLubyte buf[1000];
    int i;

    Output("glCopyTexSubImage2D\n");
    ZeroBuf(GL_UNSIGNED_BYTE, 100, buf);
    for (i = 0; enum_TextureBorder[i].value != -1; i++) {
	Output("\t%s non-Null Texture\n", enum_TextureBorder[i].name);
	glTexImage2D(GL_TEXTURE_2D, 0,  4,
	             (enum_TextureBorder[i].value) ? 3 : 1, 
	             (enum_TextureBorder[i].value) ? 3 : 1, 
	             enum_TextureBorder[i].value, 
	             GL_RGBA,
	             GL_UNSIGNED_BYTE, buf);
	glCopyTexSubImage2D(GL_TEXTURE_2D, 0, -enum_TextureBorder[i].value,
			    -enum_TextureBorder[i].value, 0, 0, 1, 1);
	ProbeEnum1_1();
	Output("\t%s Null Texture\n", enum_TextureBorder[i].name);
	glTexImage2D(GL_TEXTURE_2D, 0,  4,
	             (enum_TextureBorder[i].value) ? 3 : 1, 
	             (enum_TextureBorder[i].value) ? 3 : 1, 
	             enum_TextureBorder[i].value, GL_RGBA,
	             GL_UNSIGNED_BYTE, (const GLvoid *)NULL);
	glCopyTexSubImage2D(GL_TEXTURE_2D, 0, -enum_TextureBorder[i].value,
			    -enum_TextureBorder[i].value, 0, 0, 1, 1);
	ProbeEnum1_1();
    }
    Output("\n");
}

/* Retest the old routine using new enumerants */
void CallGet1_1(void)
{
    long i;

    Output("glGetBooleanv, ");
    Output("glGetIntegerv, ");
    Output("glGetFloatv, ");
    Output("glGetDoublev\n");
    for (i = 0; enum_GetTarget1_1[i].value != -1; i++) {
	Output("\t%s\n", enum_GetTarget1_1[i].name);
	{
	    GLubyte buf[100];
	    glGetBooleanv(enum_GetTarget1_1[i].value, buf);
	}
	{
	    GLint buf[100];
	    glGetIntegerv(enum_GetTarget1_1[i].value, buf);
	}
	{
	    GLfloat buf[100];
	    glGetFloatv(enum_GetTarget1_1[i].value, buf);
	}
	{
	    GLdouble buf[100];
	    glGetDoublev(enum_GetTarget1_1[i].value, buf);
	}
	ProbeEnum1_1();
    }
    Output("\n");
}

void CallTexImage2D1_1(void)
{
    GLubyte buf[1000];
    GLint i, j, k, l, m;

    Output("glTexImage2D\n");
    for (i = 0; enum_TexTarget2D1_1[i].value != -1; i++) {
	for (j = 0; enum_TextureFormat1_1[j].value != -1; j++) {
	    for (k = 0; enum_TextureType1_1[k].value != -1; k++) {
		for (l = 0; enum_TextureBorder[l].value != -1; l++) {
		    for (m = 0;
			 enum_TextureInternalFormat[m].value != -1;
			 m++) {
			Output("\t%s, %s, %s, %s, %s, with texture\n", 
			       enum_TexTarget2D1_1[i].name, 
			       enum_TextureInternalFormat[m].name, 
			       enum_TextureFormat1_1[j].name, 
			       enum_TextureType1_1[k].name, 
			       enum_TextureBorder[l].name);
			ZeroBuf(enum_TextureType1_1[k].value, 100, buf);
			glTexImage2D(enum_TexTarget2D1_1[i].value, 0, 
			             enum_TextureInternalFormat[m].value, 
			             (enum_TextureBorder[l].value) ? 3 : 1, 
			             (enum_TextureBorder[l].value) ? 3 : 1, 
			             enum_TextureBorder[l].value, 
			             enum_TextureFormat1_1[j].value, 
			             enum_TextureType1_1[k].value, buf);
			ProbeEnum1_1();

			Output("\t%s, %s, %s, %s, %s, NULL texture\n", 
			       enum_TexTarget2D1_1[i].name, 
			       enum_TextureInternalFormat[m].name, 
			       enum_TextureFormat1_1[j].name, 
			       enum_TextureType1_1[k].name, 
			       enum_TextureBorder[l].name);
			glTexImage2D(enum_TexTarget2D1_1[i].value, 0, 
			             enum_TextureInternalFormat[m].value, 
			             (enum_TextureBorder[l].value) ? 3 : 1, 
			             (enum_TextureBorder[l].value) ? 3 : 1, 
			             enum_TextureBorder[l].value, 
			             enum_TextureFormat1_1[j].value, 
			             enum_TextureType1_1[k].value,
				     (const GLvoid *)NULL);
			ProbeEnum1_1();
		    }
		}
	    }
	}
    }
    Output("\n");
}

void CallTexImage1D1_1(void)
{
    GLubyte buf[1000];
    GLint i, j, k, l, m;

    Output("glTexImage1D \n");
    for (i = 0; enum_TexTarget1D1_1[i].value != -1; i++) {
	for (j = 0; enum_TextureFormat1_1[j].value != -1; j++) {
	    for (k = 0; enum_TextureType1_1[k].value != -1; k++) {
		for (l = 0; enum_TextureBorder[l].value != -1; l++) {
		    for (m = 0;
			 enum_TextureInternalFormat[m].value != -1;
			 m++) {

			Output("\t%s, %s, %s, %s, %s\n", 
			       enum_TexTarget1D1_1[i].name, 
			       enum_TextureInternalFormat[m].name, 
			       enum_TextureFormat1_1[j].name, 
			       enum_TextureType1_1[k].name, 
			       enum_TextureBorder[l].name);
			ZeroBuf(enum_TextureType1_1[k].value, 100, buf);
			glTexImage1D(enum_TexTarget1D1_1[i].value, 0, 
			             enum_TextureInternalFormat[m].value, 
			             (enum_TextureBorder[l].value) ? 3 : 1, 
			             enum_TextureBorder[l].value, 
			             enum_TextureFormat1_1[j].value, 
			             enum_TextureType1_1[k].value, buf);
			ProbeEnum1_1();
		    }
		}
	    }
	}
    }
    Output("\n");
}

void CallTexEnv1_1(void)
{

    Output("glTexEnvi, ");
    Output("glTexEnvf\n");

    Output("\tGL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE\n");
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, (GLfloat)GL_REPLACE);
    ProbeEnum1_1();
    Output("\n");

    Output("glTexEnviv, ");
    Output("glTexEnvfv\n");
    Output("\tGL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE\n");
    {
	GLint buf[1];
	buf[0] = (GLint)GL_REPLACE;
	glTexEnviv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, buf);
    }
    {
	GLfloat buf[1];
	buf[0] = (GLfloat)GL_REPLACE;
	glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, buf);
    }
    ProbeEnum1_1();
    Output("\n");
}

void CallGetTexParameter1_1(void)
{
    long i, j;

    Output("glGetTexParameteriv, ");
    Output("glGetTexParameterfv\n");
    for (j = 0; enum_GetTextureParameter1_1[j].value != -1; j++) {
	for (i = 0; enum_TexTarget2D1_1[i].value != -1; i++) {

	    if (enum_TexTarget2D1_1[i].value == GL_PROXY_TEXTURE_2D) {
		continue;
	    }

	    Output("\t%s, %s\n", enum_TexTarget2D1_1[i].name, 
		   enum_GetTextureParameter1_1[j].name);
	    {
		GLint buf[100];
		glGetTexParameteriv(enum_TexTarget2D1_1[i].value, 
		                    enum_GetTextureParameter1_1[j].value, buf);
	    }
	    {
		GLfloat buf[100];
		glGetTexParameterfv(enum_TexTarget2D1_1[i].value, 
		                    enum_GetTextureParameter1_1[j].value, buf);
	    }
	    ProbeEnum();
	}
	for (i = 0; enum_TexTarget1D1_1[i].value != -1; i++) {

	    if (enum_TexTarget1D1_1[i].value == GL_PROXY_TEXTURE_1D) {
		continue;
	    }

	    Output("\t%s, %s\n", enum_TexTarget1D1_1[i].name, 
		   enum_GetTextureParameter1_1[j].name);
	    {
		GLint buf[100];
		glGetTexParameteriv(enum_TexTarget1D1_1[i].value, 
		                    enum_GetTextureParameter1_1[j].value, buf);
	    }
	    {
		GLfloat buf[100];
		glGetTexParameterfv(enum_TexTarget1D1_1[i].value, 
		                    enum_GetTextureParameter1_1[j].value, buf);
	    }
	    ProbeEnum();
	}
    }
    Output("\n");
}

void CallGetTexLevelParameter1_1(void)
{
    long i, j, l;

    Output("glGetTexLevelParameteriv, ");
    Output("glGetTexLevelParameterfv\n");
    for (j = 0; enum_GetTextureLevelParameter1_1[j].value != -1; j++) {
	for (l = 0; l <= 2; l++) {
	    for (i = 0; enum_TexTarget2D1_1[i].value != -1; i++) {
		Output("\t%s, level %d,  %s\n", enum_TexTarget2D1_1[i].name,
		       l, enum_GetTextureLevelParameter1_1[j].name);
		{
		    GLint buf[100];
		    glGetTexLevelParameteriv(enum_TexTarget2D1_1[i].value, l,
				enum_GetTextureLevelParameter1_1[j].value, buf);
		}
		{
		    GLfloat buf[100];
		    glGetTexLevelParameterfv(enum_TexTarget2D1_1[i].value, l,
				enum_GetTextureLevelParameter1_1[j].value, buf);
		}
		ProbeEnum();
	    }
	    for (i = 0; enum_TexTarget1D1_1[i].value != -1; i++) {
		Output("\t%s, level %d,  %s\n", enum_TexTarget1D1_1[i].name, l,
		       enum_GetTextureLevelParameter1_1[j].name);
		{
		    GLint buf[100];
		    glGetTexLevelParameteriv(enum_TexTarget1D1_1[i].value, l,
				enum_GetTextureLevelParameter1_1[j].value, buf);
		}
		{
		    GLfloat buf[100];
		    glGetTexLevelParameterfv(enum_TexTarget1D1_1[i].value, l,
				enum_GetTextureLevelParameter1_1[j].value, buf);
		}
		ProbeEnum();
	    }
	}
    }
    Output("\n");
}

void CallEnableIsEnableDisable1_1(void)
{
    long i, x;

    Output("glEnable, ");
    Output("glIsEnabled, ");
    Output("glDisable\n");
    for (i = 0; enum_Enable1_1[i].value != -1; i++) {

	if (enum_Enable1_1[i].value == GL_TEXTURE_GEN_R) {
	    continue;
	} else if (enum_Enable1_1[i].value == GL_TEXTURE_GEN_Q) {
	    continue;
	}

	Output("\t%s\n", enum_Enable1_1[i].name);
	glEnable(enum_Enable1_1[i].value);
	x = glIsEnabled(enum_Enable1_1[i].value);
	glDisable(enum_Enable1_1[i].value);
	ProbeEnum1_1();
    }
    Output("\n");
}

void CallIndex1_1(void)
{
    float ci;

    ci = 255.0;

    Output("glIndexub, ");
    Output("glIndexubv, ");

    glIndexub((GLubyte)ci);

    {
	GLubyte buf[1];
	buf[0] = (GLubyte)ci;
	glIndexubv(buf);
    }

    Output("\n");
}

#endif  /* GL_VERSION_1_1 */
