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

#include "ctk.h"
#include <GL/glu.h>


#define WINSIZE 100


typedef struct _enumTestRec {
    GLenum value;
    char name[40];
} enumTestRec;


extern GLUnurbsObj *nurbObj;
extern GLUtriangulatorObj *tessObj;
extern GLUquadricObj *quadObj;
extern enumTestRec enum_Error[];
extern enumTestRec enum_NurbCallBack[];
extern enumTestRec enum_NurbContour[];
extern enumTestRec enum_NurbDisplay[];
extern enumTestRec enum_NurbProperty[];
extern enumTestRec enum_NurbType[];
extern enumTestRec enum_PixelFormat[];
extern enumTestRec enum_PixelType[];
extern enumTestRec enum_QuadricCallBack[];
extern enumTestRec enum_QuadricDraw[];
extern enumTestRec enum_QuadricNormal[];
extern enumTestRec enum_QuadricOrientation[];
extern enumTestRec enum_foo[];
extern enumTestRec enum_TessCallBack[];
extern enumTestRec enum_TextureTarget[];


extern void ProbeEnum(void);
extern void ProbeError(void (*)());
extern void Output(char *, ...);
extern void ZeroBuf(GLenum, long, void *);

extern void VerifyEnums(void);
extern void CallBeginCurve(void);
extern void CallBeginPolygon(void);
extern void CallBeginSurface(void);
extern void CallBeginTrim(void);
extern void CallBuild1DMipmaps(void);
extern void CallBuild2DMipmaps(void);
extern void CallCylinder(void);
extern void CallDeleteNurbsRenderer(void);
extern void CallDeleteQuadric(void);
extern void CallDeleteTess(void);
extern void CallDisk(void);
extern void CallEndCurve(void);
extern void CallEndPolygon(void);
extern void CallEndSurface(void);
extern void CallEndTrim(void);
extern void CallErrorString(void);
extern void CallGetNurbsProperty(void);
extern void CallLoadSamplingMatrices(void);
extern void CallLookAt(void);
extern void CallNewNurbsRenderer(void);
extern void CallNewQuadric(void);
extern void CallNewTess(void);
extern void CallNextContour(void);
extern void CallNurbsCallback(void);
extern void CallNurbsCurve(void);
extern void CallNurbsProperty(void);
extern void CallNurbsSurface(void);
extern void CallOrtho2D(void);
extern void CallPartialDisk(void);
extern void CallPerspective(void);
extern void CallPickMatrix(void);
extern void CallProject(void);
extern void CallPwlCurve(void);
extern void CallQuadricCallback(void);
extern void CallQuadricDrawStyle(void);
extern void CallQuadricNormals(void);
extern void CallQuadricOrientation(void);
extern void CallQuadricTexture(void);
extern void CallScaleImage(void);
extern void CallSphere(void);
extern void CallTessCallback(void);
extern void CallTessVertex(void);
extern void CallUnProject(void);

extern void ProbeEnum(void);
extern void ProbeError(void (*)(void));

/* glu 1.1 only */
#ifdef GLU_VERSION_1_1
extern void ProbeEnum1_1(void);
extern void ProbeError1_1(void (*)(void));
extern void CallGetString(void);
#endif

/* glu 1.2 only */
#ifdef GLU_VERSION_1_2
extern enumTestRec enum_TessProperty[];
extern enumTestRec enum_TessWindingRule[];

extern void ProbeEnum1_2(void);
extern void ProbeError1_2(void (*Func)(void));

extern void CallTessCallback1_2(void);
extern void CallTessProperty(void);
extern void CallGetTessProperty(void);
extern void CallTessBeginPolygon(void);
extern void CallTessBeginContour(void);
extern void CallTessNormal(void);
extern void CallTessEndContour(void);
extern void CallTessEndPolygon(void);
#endif
