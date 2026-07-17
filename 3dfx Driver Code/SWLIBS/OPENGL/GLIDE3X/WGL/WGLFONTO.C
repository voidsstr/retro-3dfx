/*
** Copyright 1992-1997 Silicon Graphics, Inc.
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
/*****************************************************************************
 * wglUseFontOutlines
 * 
 * Converts a subrange of the glyphs in a TrueType font to OpenGL display
 * lists.
 *
 * TO DO:
 *
 * In an NT server-side environment, calls to GetGlyphOutline() should
 * be replaced with GreGetGlyphOutline().  This requires a new final
 * argument, the client's attribute cache, that's unnecessary in this
 * test environment.
 *
 * Since this code uses a global buffer, we need to add locking to
 * wglUseFontOutlines() in order to make it thread-safe.
 *
 * Might need a more dynamic storage allocation mechanism for the vertex
 * buffer, though none of the test cases so far has used more than a few
 * entries.
 *
 * Need to add support for extrusion.
 *****************************************************************************/

#include "wgllib.h"
#include "GL/glu.h"
#include "math.h"

BOOL WINAPI wglUseFontOutlinesA(HDC hDC, DWORD first, DWORD count,
				DWORD listBase, FLOAT chordalDeviation, FLOAT
				extrusion, INT format, LPGLYPHMETRICSFLOAT
				glyphMetricsFloatArray);

#ifndef DEBUG
#define DEBUG 0
#endif

#if DEBUG
#include <stdio.h>
#include <stdlib.h>
#endif



typedef GLUtesselator *(APIENTRY *gluNewTessProto)(void);
typedef void (APIENTRY *gluDeleteTessProto)(GLUtesselator *tess);
typedef void (APIENTRY *gluTessBeginPolygonProto)(GLUtesselator *tess, void *polygon_data);
typedef void (APIENTRY *gluTessBeginContourProto)(GLUtesselator *tess);
typedef void (APIENTRY *gluTessVertexProto)(GLUtesselator *tess, GLdouble coords[3], void *data);
typedef void (APIENTRY *gluTessEndContourProto)(GLUtesselator *tess);
typedef void (APIENTRY *gluTessEndPolygonProto)(GLUtesselator *tess);
typedef void (APIENTRY *gluTessPropertyProto)(GLUtesselator *tess, GLenum which, GLdouble value);
typedef void (APIENTRY *gluTessNormalProto)(GLUtesselator *tess, GLdouble x, GLdouble y, GLdouble z);
typedef void (APIENTRY *gluTessCallbackProto)(GLUtesselator *tess, GLenum which, void (CALLBACK *)());

static HINSTANCE		gluModuleHandle;
static gluNewTessProto		gluNewTessProc;
static gluDeleteTessProto	gluDeleteTessProc;
static gluTessBeginPolygonProto	gluTessBeginPolygonProc;
static gluTessBeginContourProto	gluTessBeginContourProc;
static gluTessVertexProto	gluTessVertexProc;
static gluTessEndContourProto	gluTessEndContourProc;
static gluTessEndPolygonProto	gluTessEndPolygonProc;
static gluTessPropertyProto	gluTessPropertyProc;
static gluTessNormalProto	gluTessNormalProc;
static gluTessCallbackProto	gluTessCallbackProc;



static HFONT	hNewFont, hOldFont;
static FLOAT	ScaleFactor;



#define LINE_BUF_QUANT 4000
#define VERT_BUF_QUANT 4000



static FLOAT*	LineBuf;
static DWORD	LineBufSize;
static DWORD	LineBufIndex;
static FLOAT*	VertBuf;
static DWORD	VertBufSize;
static DWORD	VertBufIndex;
static GLenum	TessErrorOccurred;



static int AppendToLineBuf(		FLOAT		value);

static int AppendToVertBuf(		FLOAT		value);

static int DrawGlyph(			UCHAR*		glyphBuf,
					DWORD		glyphSize,
					FLOAT		chordalDeviation,
					FLOAT		extrusion,
					INT		format);

static void FreeLineBuf(		void);

static void FreeVertBuf(		void);

static long GetWord(			UCHAR**		p);

static long GetDWord(			UCHAR**		p);

static double GetFixed(			UCHAR**		p);

static int InitLineBuf(			void);

static int InitVertBuf(			void);

static HFONT CreateHighResolutionFont(	HDC		hDC);

static int MakeDisplayListFromGlyph(	DWORD			listName,
					UCHAR*			glyphBuf,
					DWORD			glyphSize,
					LPGLYPHMETRICSFLOAT	glyphMetricsFloat,
					FLOAT			chordalDeviation,
					FLOAT			extrusion,
					INT			format);

static BOOL LoadGLUTesselator(		void);
static BOOL UnloadGLUTesselator(	void);

static int MakeLinesFromArc(		FLOAT		x0,
					FLOAT		y0,
					FLOAT		x1,
					FLOAT		y1,
					FLOAT		x2,
					FLOAT		y2,
					DWORD		vertexCountIndex,
					FLOAT		chordalDeviationSquared);

static int MakeLinesFromGlyph(		UCHAR*		glyphBuf,
					DWORD		glyphSize,
					FLOAT		chordalDeviation);

static int MakeLinesFromTTLine(		UCHAR**		pp,
					DWORD		vertexCountIndex,
					WORD		pointCount);

static int MakeLinesFromTTPolycurve(	UCHAR**		pp,
					DWORD		vertexCountIndex,
					FLOAT		chordalDeviation);

static int MakeLinesFromTTPolygon(	UCHAR**		pp,
					FLOAT		chordalDeviation);

static int MakeLinesFromTTQSpline(	UCHAR**		pp,
					DWORD		vertexCountIndex,
					WORD		pointCount,
					FLOAT		chordalDeviation);

static void CALLBACK TessCombine(	double		coords[3],
					void*		vertex_data[4],
					FLOAT		weight[4],
					void**		outData);

static void CALLBACK TessError(		GLenum		error);

static void CALLBACK TessVertexOutData(	FLOAT		p[3],
					GLfloat 	z);



/*****************************************************************************
 * wglUseFontOutlinesW
 * 
 * Converts a subrange of the glyphs in a TrueType font to OpenGL display
 * lists.
 *****************************************************************************/

WINGDIAPI BOOL APIENTRY
wglUseFontOutlinesW(IN	HDC			hDC,
		    IN	DWORD			first,
		    IN	DWORD			count,
		    IN	DWORD			listBase,
		    IN	FLOAT			chordalDeviation,
		    IN	FLOAT			extrusion,
		    IN	INT			format,
		    OUT	LPGLYPHMETRICSFLOAT	lpgmf)
	{
		return wglUseFontOutlinesA(hDC, first, count, listBase,
				chordalDeviation, extrusion, format, lpgmf);
	}

/*****************************************************************************
 * wglUseFontOutlinesA
 * 
 * Converts a subrange of the glyphs in a TrueType font to OpenGL display
 * lists.
 *****************************************************************************/

WINGDIAPI BOOL APIENTRY
wglUseFontOutlinesA(IN	HDC			hDC,
		    IN	DWORD			first,
		    IN	DWORD			count,
		    IN	DWORD			listBase,
		    IN	FLOAT			chordalDeviation,
		    IN	FLOAT			extrusion,
		    IN	INT			format,
		    OUT	LPGLYPHMETRICSFLOAT	glyphMetricsFloatArray)
	{
	DWORD	glyphIndex;
	UCHAR*	glyphBuf;
	DWORD	glyphBufSize;


	/*
	 * Flush any previous OpenGL errors.  This allows us to check for
	 * new errors so they can be reported via the function return value.
	 */
	while (glGetError() != GL_NO_ERROR)
		;
	
	/*
	 * Make sure that the current font can be sampled accurately.
	 */
	hNewFont = CreateHighResolutionFont(hDC);
	if (!hNewFont)
		return FALSE;

	hOldFont = SelectObject(hDC, hNewFont);
	if (!hOldFont)
		return FALSE;
	
	/*
	 * Preallocate a buffer for the outline data, and track its size:
	 */
	glyphBuf = (UCHAR*) __wglMalloc(glyphBufSize = 10240);
	if (!glyphBuf)
		return FALSE; /*WGL_STATUS_NOT_ENOUGH_MEMORY*/

	/*
	 * Process each glyph in the given range:
	 */
	for (glyphIndex = first; glyphIndex - first < count; ++glyphIndex)
		{
		GLYPHMETRICS	glyphMetrics;
		DWORD		glyphSize;
		static MAT2	matrix =
			{
			{0, 1},		{0, 0},
			{0, 0},		{0, 1}
			};
		LPGLYPHMETRICSFLOAT glyphMetricsFloat =
			&glyphMetricsFloatArray[glyphIndex - first];


		/*
		 * Determine how much space is needed to store the glyph's
		 * outlines.  If our glyph buffer isn't large enough,
		 * resize it.
		 */
		glyphSize = GetGlyphOutline(	hDC,
						glyphIndex,
						GGO_NATIVE,
						&glyphMetrics,
						0,
						NULL,
						&matrix
						);
		if (glyphSize < 0)
			return FALSE; /*WGL_STATUS_FAILURE*/
		if (glyphSize > glyphBufSize)
			{
			__wglFree(glyphBuf);
			glyphBuf = (UCHAR*) __wglMalloc(glyphBufSize = glyphSize);
			if (!glyphBuf)
				return FALSE; /*WGL_STATUS_NOT_ENOUGH_MEMORY*/
			}


		/*
		 * Get the glyph's outlines.
		 */
		if (GetGlyphOutline(	hDC,
					glyphIndex,
					GGO_NATIVE,
					&glyphMetrics,
					glyphBufSize,
					glyphBuf,
					&matrix
					) < 0)
			{
			__wglFree(glyphBuf);
			return FALSE; /*WGL_STATUS_FAILURE*/
			}
		
		glyphMetricsFloat->gmfBlackBoxX =
			(FLOAT) glyphMetrics.gmBlackBoxX * ScaleFactor;
		glyphMetricsFloat->gmfBlackBoxY =
			(FLOAT) glyphMetrics.gmBlackBoxY * ScaleFactor;
		glyphMetricsFloat->gmfptGlyphOrigin.x =
			(FLOAT) glyphMetrics.gmptGlyphOrigin.x * ScaleFactor;
		glyphMetricsFloat->gmfptGlyphOrigin.y =
			(FLOAT) glyphMetrics.gmptGlyphOrigin.y * ScaleFactor;
		glyphMetricsFloat->gmfCellIncX =
			(FLOAT) glyphMetrics.gmCellIncX * ScaleFactor;
		glyphMetricsFloat->gmfCellIncY =
			(FLOAT) glyphMetrics.gmCellIncY * ScaleFactor;
		
		/*
		 * Turn the glyph into a display list:
		 */
		if (!MakeDisplayListFromGlyph(	(glyphIndex - first) + listBase,
						glyphBuf,
						glyphSize,
						glyphMetricsFloat,
						chordalDeviation + ScaleFactor,
						extrusion,
						format))
			{
			__wglFree(glyphBuf);
			return FALSE; /*WGL_STATUS_FAILURE*/
			}
		}


	/*
	 * Clean up temporary storage and return.  If an error occurred,
	 * clear all OpenGL error flags and return FAILURE status;
	 * otherwise just return SUCCESS.
	 */
	__wglFree(glyphBuf);

	SelectObject(hDC, hOldFont);

	if (glGetError() == GL_NO_ERROR)
		return TRUE; /*WGL_STATUS_SUCCESS*/
	else
		{
		while (glGetError() != GL_NO_ERROR)
			;
		return FALSE; /*WGL_STATUS_FAILURE*/
		}
	}



/*****************************************************************************
 * CreateHighResolutionFont
 *
 * Gets metrics for the current font and creates an equivalent font
 * scaled to the design units of the font.
 * 
 *****************************************************************************/

static HFONT
CreateHighResolutionFont(HDC hDC)
	{
	UINT otmSize;
	OUTLINETEXTMETRIC *otm;
	LONG fontHeight, fontWidth, fontUnits;
	LOGFONT logFont;

	otmSize = GetOutlineTextMetrics(hDC, 0, NULL);
	if (otmSize == 0) 
		return NULL;

	otm = (OUTLINETEXTMETRIC *) __wglMalloc(otmSize);
	if (otm == NULL)
		return NULL;

	otm->otmSize = otmSize;
	if (GetOutlineTextMetrics(hDC, otmSize, otm) == 0) 
		return NULL;
	
	fontHeight = otm->otmTextMetrics.tmHeight -
			otm->otmTextMetrics.tmInternalLeading;
	fontWidth = otm->otmTextMetrics.tmAveCharWidth;
	fontUnits = (LONG) otm->otmEMSquare;
	
	ScaleFactor = 1.0F / (FLOAT) fontUnits;

	logFont.lfHeight = - ((LONG) fontUnits);
	logFont.lfWidth = (LONG)
		((FLOAT) (fontWidth * fontUnits) / (FLOAT) fontHeight);
	logFont.lfEscapement = 0;
	logFont.lfOrientation = 0;
	logFont.lfWeight = otm->otmTextMetrics.tmWeight;
	logFont.lfItalic = otm->otmTextMetrics.tmItalic;
	logFont.lfUnderline = otm->otmTextMetrics.tmUnderlined;
	logFont.lfStrikeOut = otm->otmTextMetrics.tmStruckOut;
	logFont.lfCharSet = otm->otmTextMetrics.tmCharSet;
	logFont.lfOutPrecision = OUT_OUTLINE_PRECIS;
	logFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	logFont.lfQuality = DEFAULT_QUALITY;
	logFont.lfPitchAndFamily =
		otm->otmTextMetrics.tmPitchAndFamily & 0xf0;
	strcpy(logFont.lfFaceName,
	       (char *)otm + (int)otm->otmpFaceName);

	hNewFont = CreateFontIndirect(&logFont);
	if (hNewFont == NULL)
		return NULL;

	__wglFree(otm);

	return hNewFont;
	}



/*****************************************************************************
 * MakeDisplayListFromGlyph
 * 
 * Converts the outline of a glyph to an OpenGL display list.
 *
 * Return value is nonzero for success, zero for failure.
 *
 * Does not check for OpenGL errors, so if the caller needs to know about them,
 * it should call glGetError().
 *****************************************************************************/

static int
MakeDisplayListFromGlyph(	IN  DWORD		listName,
				IN  UCHAR*		glyphBuf,
				IN  DWORD		glyphSize,
				IN  LPGLYPHMETRICSFLOAT	glyphMetricsFloat,
				IN  FLOAT		chordalDeviation,
				IN  FLOAT		extrusion,
				IN  INT			format)
	{
	int status;

	glNewList(listName, GL_COMPILE);
		status = DrawGlyph(	glyphBuf,
					glyphSize,
					chordalDeviation,
					extrusion,
					format);
		
	glTranslatef(glyphMetricsFloat->gmfCellIncX,
		     glyphMetricsFloat->gmfCellIncY,
		     0.0F);
	glEndList();

	return status;
	}



/*****************************************************************************
 * DrawGlyph
 * 
 * Converts the outline of a glyph to OpenGL drawing primitives, tessellating
 * as needed, and then draws the glyph.  Tessellation of the quadratic splines
 * in the outline is controlled by "chordalDeviation", and the drawing
 * primitives (lines or polygons) are selected by "format".
 *
 * Return value is nonzero for success, zero for failure.
 *
 * Does not check for OpenGL errors, so if the caller needs to know about them,
 * it should call glGetError().
 *****************************************************************************/

static int
DrawGlyph(	IN  UCHAR*	glyphBuf,
		IN  DWORD	glyphSize,
		IN  FLOAT	chordalDeviation,
		IN  FLOAT	extrusion,
		IN  INT		format)
	{
	INT			status = 0;
	FLOAT*			p;
	DWORD			loop;
	DWORD			point;
	GLUtesselator*		tess = NULL;


	/*
	 * Initialize the global buffer into which we place the outlines:
	 */
	if (!InitLineBuf())
		goto exit;


	/*
	 * Convert the glyph outlines to a set of polyline loops.
	 * (See MakeLinesFromGlyph() for the format of the loop data
	 * structure.)
	 */
	if (!MakeLinesFromGlyph(glyphBuf, glyphSize, chordalDeviation))
		goto exit;
	p = LineBuf;


	/*
	 * Now draw the loops in the appropriate format:
	 */
	if (format == WGL_FONT_LINES)
		{
		/*
		 * This is the easy case.  Just draw the outlines.
		 */
		for (loop = (DWORD) *p++; loop; --loop)
			{
			glBegin(GL_LINE_LOOP);
				for (point = (DWORD) *p++; point; --point)
					{
					glVertex2fv(p);
					p += 2;
					}
			glEnd();
			}
		status = 1;
		}

	else if (format == WGL_FONT_POLYGONS)
		{
		double v[3];
		FLOAT *save_p = p;
		GLfloat z_value;
		
		/*
		 * This is the hard case.  We have to set up a tessellator
		 * to convert the outlines into a set of polygonal
		 * primitives, which the tessellator passes to some
		 * auxiliary routines for drawing.
		 */
		if (!LoadGLUTesselator())
			goto exit;
		if (!InitVertBuf())
			goto exit;
		if (!(tess = gluNewTessProc()))
			goto exit;
		gluTessCallbackProc(tess,	GLU_BEGIN,	(void(CALLBACK *)()) glBegin);
		gluTessCallbackProc(tess,	GLU_TESS_VERTEX_DATA,
				    (void(CALLBACK *)()) TessVertexOutData);
		gluTessCallbackProc(tess,	GLU_END,	(void(CALLBACK *)()) glEnd);
		gluTessCallbackProc(tess,	GLU_ERROR,	(void(CALLBACK *)()) TessError);
		gluTessCallbackProc(tess,	GLU_TESS_COMBINE, (void(CALLBACK *)()) TessCombine);
		gluTessNormalProc(tess,	0.0F, 0.0F, 1.0F);

		TessErrorOccurred = 0;
		glNormal3f(0.0f, 0.0f, 1.0f);
		v[2] = 0.0;
		z_value = 0.0f;

		gluTessBeginPolygonProc(tess, (void *)*(int *)&z_value);
			for (loop = (DWORD) *p++; loop; --loop)
				{
				gluTessBeginContourProc(tess);
				
				for (point = (DWORD) *p++; point; --point)
					{
					v[0] = p[0];
					v[1] = p[1];
					gluTessVertexProc(tess, v, p);
					p += 2;
					}

				gluTessEndContourProc(tess);
				}
		gluTessEndPolygonProc(tess);

		status = !TessErrorOccurred;

		/* Extrusion code */
		if (extrusion) {
			DWORD loops;
			GLfloat thickness = (GLfloat) -extrusion;
			FLOAT *vert, *vert2;
			DWORD count;

			p = save_p;
			loops = (DWORD) *p++;

			for (loop = 0; loop < loops; loop++) {
				GLfloat dx, dy, len;
				DWORD last;

				count = (DWORD) *p++;
				glBegin(GL_QUAD_STRIP);

				/* Check if the first and last vertex are identical
				 * so we don't draw the same quad twice.
				 */
				vert = p + (count-1)*2;
				last = (p[0] == vert[0] && p[1] == vert[1]) ? count-1 : count;

				for (point = 0; point <= last; point++) {
					vert  = p + 2 * (point % last);
					vert2 = p + 2 * ((point+1) % last);

					dx = vert[0] - vert2[0];
					dy = vert[1] - vert2[1];
					len = (GLfloat)sqrt(dx * dx + dy * dy);

					glNormal3f(dy / len, -dx / len, 0.0f);
					glVertex3f((GLfloat) vert[0],
							   (GLfloat) vert[1], thickness);
					glVertex3f((GLfloat) vert[0],
							   (GLfloat) vert[1], 0.0f);
				}

				glEnd();
				p += count*2;
			}

			/* Draw the back face */
			p = save_p;
			v[2] = thickness;
			glNormal3f(0.0f, 0.0f, -1.0f);
			gluTessNormalProc(tess,	0.0F, 0.0F, -1.0F);

			gluTessBeginPolygonProc(tess, (void *)*(int *)&thickness);

			for (loop = (DWORD) *p++; loop; --loop)
			{
				count = (DWORD) *p++;

				gluTessBeginContourProc(tess);
				
				for (point = 0; point < count; point++)
				{
					vert = p + ((count-point-1)<<1);
					v[0] = vert[0];
					v[1] = vert[1];
					gluTessVertexProc(tess, v, vert);
				}
				p += count*2;

				gluTessEndContourProc(tess);
			}
			gluTessEndPolygonProc(tess);
		}

#if DEBUG
	if (TessErrorOccurred)
		printf("Tessellation error %s\n",
			gluErrorString(TessErrorOccurred));
#endif
		}


exit:
	FreeLineBuf();
	if (tess)
		gluDeleteTessProc(tess);
	// UnloadGLUTesselator();
	FreeVertBuf();
	return status;
	}



/*****************************************************************************
 * LoadGLUTesselator
 *
 * Maps the glu32.dll module and gets function pointers for the 
 * tesselator functions.
 *****************************************************************************/

static BOOL
LoadGLUTesselator(void)
	{
	if (gluModuleHandle != NULL)
		return TRUE;

	{
		extern HINSTANCE hInstanceOpenGL;
		char *gluName = "GLU.DLL";
		char name[256];
		char *ptr;
		int len;

		len = GetModuleFileName(hInstanceOpenGL, name, 255);
		if (len != 0)
			{
			ptr = name+len-1;
			while (ptr > name && *ptr != '\\')
				ptr--;
			if (*ptr == '\\')
				ptr++;
			if (!stricmp(ptr, "cosmogl.dll"))
				{
				gluName = "COSMOGLU.DLL";
				}
			else if (!stricmp(ptr, "opengl32.dll"))
				{
				gluName = "GLU32.DLL";
				}
			}

		if ((gluModuleHandle = LoadLibrary(gluName)) == NULL)
			return FALSE;
	}

	if ((gluNewTessProc = (gluNewTessProto)
		GetProcAddress(gluModuleHandle, "gluNewTess")) == NULL)
		return FALSE;
	
	if ((gluDeleteTessProc = (gluDeleteTessProto)
		GetProcAddress(gluModuleHandle, "gluDeleteTess")) == NULL)
		return FALSE;
	
	if ((gluTessBeginPolygonProc = (gluTessBeginPolygonProto)
		GetProcAddress(gluModuleHandle, "gluTessBeginPolygon")) == NULL)
		return FALSE;
	
	if ((gluTessBeginContourProc = (gluTessBeginContourProto)
		GetProcAddress(gluModuleHandle, "gluTessBeginContour")) == NULL)
		return FALSE;
	
	if ((gluTessVertexProc = (gluTessVertexProto)
		GetProcAddress(gluModuleHandle, "gluTessVertex")) == NULL)
		return FALSE;
	
	if ((gluTessEndContourProc = (gluTessEndContourProto)
		GetProcAddress(gluModuleHandle, "gluTessEndContour")) == NULL)
		return FALSE;
	
	if ((gluTessEndPolygonProc = (gluTessEndPolygonProto)
		GetProcAddress(gluModuleHandle, "gluTessEndPolygon")) == NULL)
		return FALSE;
	
	if ((gluTessPropertyProc = (gluTessPropertyProto)
		GetProcAddress(gluModuleHandle, "gluTessProperty")) == NULL)
		return FALSE;
	
	if ((gluTessNormalProc = (gluTessNormalProto)
		GetProcAddress(gluModuleHandle, "gluTessNormal")) == NULL)
		return FALSE;
	
	if ((gluTessCallbackProc = (gluTessCallbackProto)
		GetProcAddress(gluModuleHandle, "gluTessCallback")) == NULL)
		return FALSE;

	return TRUE;
	}



/*****************************************************************************
 * UnloadGLUTesselator
 *
 * Unmaps the glu32.dll module.
 *****************************************************************************/

static BOOL
UnloadGLUTesselator(void)
	{
	if (gluModuleHandle != NULL)
	    if (FreeLibrary(gluModuleHandle) == FALSE)
		return FALSE;
	gluModuleHandle = NULL;
	}



/*****************************************************************************
 * TessVertexOut
 *
 * Used by tessellator to handle output vertexes.
 *****************************************************************************/
 
static void CALLBACK
TessVertexOut(FLOAT	p[3])
	{
	    GLfloat v[2];

	    v[0] = p[0] * ScaleFactor;
	    v[1] = p[1] * ScaleFactor;
	    glVertex2fv(v);
	}

static void CALLBACK
TessVertexOutData(FLOAT	p[3], GLfloat z)
{
    GLfloat v[3];

    v[0] = (GLfloat) p[0];
    v[1] = (GLfloat) p[1];
    v[2] = z;
    glVertex3fv(v);
}


/*****************************************************************************
 * TessCombine
 *
 * Used by tessellator to handle self-intersecting contours and degenerate
 * geometry.
 *****************************************************************************/
 
static void CALLBACK
TessCombine(double	coords[3],
	    void*	vertex_data[4],
	    FLOAT	weight[4],
	    void**	outData)
	{
	if (!AppendToVertBuf((FLOAT) coords[0])
	 || !AppendToVertBuf((FLOAT) coords[1])
	 || !AppendToVertBuf((FLOAT) coords[2]))
		TessErrorOccurred = GL_OUT_OF_MEMORY;
	*outData = VertBuf + (VertBufIndex - 3);
	}



/*****************************************************************************
 * TessError
 *
 * Saves the last tessellator error code in the global TessErrorOccurred.
 *****************************************************************************/
 
static void CALLBACK
TessError(GLenum error)
	{
	TessErrorOccurred = error;
	}



/*****************************************************************************
 * MakeLinesFromGlyph
 * 
 * Converts the outline of a glyph from the TTPOLYGON format to a simple
 * array of floating-point values containing one or more loops.
 *
 * The first element of the output array is a count of the number of loops.
 * The loop data follows this count.  Each loop consists of a count of the
 * number of vertices it contains, followed by the vertices.  Each vertex
 * is an X and Y coordinate.  For example, a single triangle might be
 * described by this array:
 *
 *	1.,	3.,	0., 0.,		1., 0.,		0., 1.
 *       ^	 ^	 ^    ^		 ^    ^		 ^    ^
 *     #loops	#verts	 x1   y1	 x2   y2	 x3   y3
 *
 * A two-loop glyph would look like this:
 *
 *	2.,	3.,  0.,0.,  1.,0.,  0.,1.,	3.,  .2,.2,  .4,.2,  .2,.4
 *
 * Line segments from the TTPOLYGON are transferred to the output array in
 * the obvious way.  Quadratic splines in the TTPOLYGON are converted to
 * collections of line segments
 *****************************************************************************/

static int
MakeLinesFromGlyph(IN  UCHAR*	glyphBuf,
		   IN  DWORD	glyphSize,
		   IN  FLOAT	chordalDeviation)
	{
	UCHAR*	p;
	int	status = 0;


	/*
	 * Pick up all the polygons (aka loops) that make up the glyph:
	 */
	if (!AppendToLineBuf(0.0F))	/* loop count at LineBuf[0] */
		goto exit;

	p = glyphBuf;
	while (p < glyphBuf + glyphSize)
		{
		if (!MakeLinesFromTTPolygon(&p, chordalDeviation))
			goto exit;
		LineBuf[0] += 1.0F;	/* increment loop count */
		}

	status = 1;

exit:
	return status;
	}



/*****************************************************************************
 * MakeLinesFromTTPolygon
 *
 * Converts a TTPOLYGONHEADER and its associated curve structures into a
 * single polyline loop in the global LineBuf.
 *****************************************************************************/

static int
MakeLinesFromTTPolygon(	IN OUT	UCHAR**	pp,
			IN	FLOAT	chordalDeviation)
	{
	DWORD	polySize;
	UCHAR*	polyStart;
	DWORD	vertexCountIndex;

	/*
	 * Record where the polygon data begins, and where the loop's
	 * vertex count resides:
	 */
	polyStart = *pp;
	vertexCountIndex = LineBufIndex;
	if (!AppendToLineBuf(0.0F))
		return 0;

	/*
	 * Extract relevant data from the TTPOLYGONHEADER:
	 */
	polySize = GetDWord(pp);
	if (GetDWord(pp) != TT_POLYGON_TYPE)	/* polygon type */
		return 0;
	if (!AppendToLineBuf((FLOAT) GetFixed(pp)))	/* first X coord */
		return 0;
	if (!AppendToLineBuf((FLOAT) GetFixed(pp)))	/* first Y coord */
		return 0;
	LineBuf[vertexCountIndex] += 1.0F;

	/*
	 * Process each of the TTPOLYCURVE structures in the polygon:
	 */
	while (*pp < polyStart + polySize)
		if (!MakeLinesFromTTPolycurve(	pp,
						vertexCountIndex,
						chordalDeviation))
		return 0;

	return 1;
	}



/*****************************************************************************
 * MakeLinesFromTTPolyCurve
 *
 * Converts the lines and splines in a single TTPOLYCURVE structure to points
 * in the global LineBuf.
 *****************************************************************************/

static int
MakeLinesFromTTPolycurve(	IN OUT	UCHAR**	pp,
				IN	DWORD	vertexCountIndex,
				IN	FLOAT	chordalDeviation)
	{
	WORD type;
	WORD pointCount;


	/*
	 * Pick up the relevant fields of the TTPOLYCURVE structure:
	 */
	type = (WORD) GetWord(pp);
	pointCount = (WORD) GetWord(pp);

	/*
	 * Convert the "curve" to line segments:
	 */
	if (type == TT_PRIM_LINE)
		return MakeLinesFromTTLine(	pp,
						vertexCountIndex,
						pointCount);
	else if (type == TT_PRIM_QSPLINE)
		return MakeLinesFromTTQSpline(	pp,
						vertexCountIndex,
						pointCount,
						chordalDeviation);
	else
		return 0;
	}



/*****************************************************************************
 * MakeLinesFromTTLine
 *
 * Converts points from the polyline in a TT_PRIM_LINE structure to
 * equivalent points in the global LineBuf.
 *****************************************************************************/
static int
MakeLinesFromTTLine(	IN OUT	UCHAR**	pp,
			IN	DWORD	vertexCountIndex,
			IN	WORD	pointCount)
	{
	/*
	 * Just copy the line segments into the line buffer (converting
	 * type as we go):
	 */
	LineBuf[vertexCountIndex] += pointCount;
	while (pointCount--)
		{
		if (!AppendToLineBuf((FLOAT) GetFixed(pp))	/* X coord */
		 || !AppendToLineBuf((FLOAT) GetFixed(pp)))	/* Y coord */
			return 0;
		}

	return 1;
	}



/*****************************************************************************
 * MakeLinesFromTTQSpline
 *
 * Converts points from the poly quadratic spline in a TT_PRIM_QSPLINE
 * structure to polyline points in the global LineBuf.
 *****************************************************************************/

static int
MakeLinesFromTTQSpline(	IN OUT	UCHAR**	pp,
			IN	DWORD	vertexCountIndex,
			IN	WORD	pointCount,
			IN	FLOAT	chordalDeviation)
	{
	FLOAT x0, y0, x1, y1, x2, y2;
	WORD point;

	/*
	 * Process each of the non-interpolated points in the outline.
	 * To do this, we need to generate two interpolated points (the
	 * start and end of the arc) for each non-interpolated point.
	 * The first interpolated point is always the one most recently
	 * stored in LineBuf, so we just extract it from there.  The
	 * second interpolated point is either the average of the next
	 * two points in the QSpline, or the last point in the QSpline
	 * if only one remains.
	 */
	for (point = 0; point < pointCount - 1; ++point)
		{
		x0 = LineBuf[LineBufIndex - 2];
		y0 = LineBuf[LineBufIndex - 1];

		x1 = (FLOAT) GetFixed(pp);
		y1 = (FLOAT) GetFixed(pp);

		if (point == pointCount - 2)
			{
			/*
			 * This is the last arc in the QSpline.  The final
			 * point is the end of the arc.
			 */
			x2 = (FLOAT) GetFixed(pp);
			y2 = (FLOAT) GetFixed(pp);
			}
		else
			{
			/*
			 * Peek at the next point in the input to compute
			 * the end of the arc:
			 */
			x2 = 0.5F * (x1 + (FLOAT) GetFixed(pp));
			y2 = 0.5F * (y1 + (FLOAT) GetFixed(pp));
			/*
			 * Push the point back onto the input so it will
			 * be reused as the next off-curve point:
			 */
			*pp -= 8;
			}

		if (!MakeLinesFromArc(	x0, y0,
					x1, y1,
					x2, y2,
					vertexCountIndex,
					chordalDeviation * chordalDeviation))
			return 0;
		}

	return 1;
	}



/*****************************************************************************
 * MakeLinesFromArc
 *
 * Subdivides one arc of a quadratic spline until the chordal deviation
 * tolerance requirement is met, then places the resulting set of line
 * segments in the global LineBuf.
 *****************************************************************************/

static int
MakeLinesFromArc(	IN	FLOAT	x0,
			IN	FLOAT	y0,
			IN	FLOAT	x1,
			IN	FLOAT	y1,
			IN	FLOAT	x2,
			IN	FLOAT	y2,
			IN	DWORD	vertexCountIndex,
			IN	FLOAT	chordalDeviationSquared)
	{
	FLOAT	x01;
	FLOAT	y01;
	FLOAT	x12;
	FLOAT	y12;
	FLOAT	midPointX;
	FLOAT	midPointY;
	FLOAT	deltaX;
	FLOAT	deltaY;

	/*
	 * Calculate midpoint of the curve by de Casteljau:
	 */
	x01 = 0.5F * (x0 + x1);
	y01 = 0.5F * (y0 + y1);
	x12 = 0.5F * (x1 + x2);
	y12 = 0.5F * (y1 + y2);
	midPointX = 0.5F * (x01 + x12);
	midPointY = 0.5F * (y01 + y12);


	/*
	 * Estimate chordal deviation by the distance from the midpoint
	 * of the curve to its non-interpolated control point.  If this
	 * distance is greater than the specified chordal deviation
	 * constraint, then subdivide.  Otherwise, generate polylines
	 * from the three control points.
	 */
	deltaX = midPointX - x1;
	deltaY = midPointY - y1;
	if (deltaX * deltaX + deltaY * deltaY > chordalDeviationSquared)
		{
		MakeLinesFromArc(	x0, y0,
					x01, y01,
					midPointX, midPointY,
					vertexCountIndex,
					chordalDeviationSquared);
		
		MakeLinesFromArc(	midPointX, midPointY,
					x12, y12,
					x2, y2,
					vertexCountIndex,
					chordalDeviationSquared);
		}
	else
		{
		/*
		 * The "pen" is already at (x0, y0), so we don't need to
		 * add that point to the LineBuf.
		 */
		if (!AppendToLineBuf(x1)
		 || !AppendToLineBuf(y1)
		 || !AppendToLineBuf(x2)
		 || !AppendToLineBuf(y2))
			return 0;
		LineBuf[vertexCountIndex] += 2.0F;
		}

	return 1;
	}



/*****************************************************************************
 * InitLineBuf
 *
 * Initializes the global LineBuf and its associated size and current-element
 * counters.
 *****************************************************************************/

static int
InitLineBuf(void)
	{
	if (!(LineBuf = (FLOAT*)
		__wglMalloc((LineBufSize = LINE_BUF_QUANT) * sizeof(FLOAT))))
			return 0;
	LineBufIndex = 0;
	return 1;
	}



/*****************************************************************************
 * InitVertBuf
 *
 * Initializes the global VertBuf and its associated size and current-element
 * counters.
 *****************************************************************************/

static int
InitVertBuf(void)
	{
	if (!(VertBuf = (FLOAT*)
		__wglMalloc((VertBufSize = VERT_BUF_QUANT) * sizeof(FLOAT))))
			return 0;
	VertBufIndex = 0;
	return 1;
	}



/*****************************************************************************
 * AppendToLineBuf
 *
 * Appends one floating-point value to the global LineBuf array.  Return value
 * is non-zero for success, zero for failure.
 *****************************************************************************/

static int
AppendToLineBuf(FLOAT value)
	{
	if (LineBufIndex >= LineBufSize)
		{
		FLOAT* f;
		
		f = (FLOAT*) __wglRealloc(LineBuf,
			(LineBufSize += LINE_BUF_QUANT) * sizeof(FLOAT));
		if (!f)
			return 0;
		LineBuf = f;
		}
	LineBuf[LineBufIndex++] = value;
	return 1;
	}



/*****************************************************************************
 * AppendToVertBuf
 *
 * Appends one floating-point value to the global VertBuf array.  Return value
 * is non-zero for success, zero for failure.
 *
 * Note that we can't realloc this one, because the tessellator is using
 * pointers into it.
 *****************************************************************************/

static int
AppendToVertBuf(FLOAT value)
	{
	if (VertBufIndex >= VertBufSize)
		return 0;
	VertBuf[VertBufIndex++] = value;
	return 1;
	}



/*****************************************************************************
 * FreeLineBuf
 *
 * Cleans up vertex buffer structure.
 *****************************************************************************/

static void
FreeLineBuf(void)
	{
	if (LineBuf)
		{
		__wglFree(LineBuf);
		LineBuf = NULL;
		}
	}



/*****************************************************************************
 * FreeVertBuf
 *
 * Cleans up vertex buffer structure.
 *****************************************************************************/

static void
FreeVertBuf(void)
	{
	if (VertBuf)
		{
		__wglFree(VertBuf);
		VertBuf = NULL;
		}
	}



/*****************************************************************************
 * GetWord
 *
 * Fetch the next 16-bit word from a little-endian byte stream, and increment
 * the stream pointer to the next unscanned byte.
 *****************************************************************************/

static long GetWord(UCHAR** p)
	{
	long value;

	value = ((*p)[1] << 8) + (*p)[0];
	*p += 2;
	return value;
	}



/*****************************************************************************
 * GetDWord
 *
 * Fetch the next 32-bit word from a little-endian byte stream, and increment
 * the stream pointer to the next unscanned byte.
 *****************************************************************************/

static long GetDWord(UCHAR** p)
	{
	long value;

	value = ((*p)[3] << 24) + ((*p)[2] << 16) + ((*p)[1] << 8) + (*p)[0];
	*p += 4;
	return value;
	}




/*****************************************************************************
 * GetFixed
 *
 * Fetch the next 32-bit fixed-point value from a little-endian byte stream,
 * convert it to floating-point, and increment the stream pointer to the next
 * unscanned byte.
 *****************************************************************************/

static double GetFixed(UCHAR** p)
	{
	long hiBits, loBits;
	double value;

	loBits = GetWord(p);
	hiBits = GetWord(p);
	value = (double) ((hiBits << 16) | loBits) / 65536.0;
	return value * ScaleFactor;
	}
