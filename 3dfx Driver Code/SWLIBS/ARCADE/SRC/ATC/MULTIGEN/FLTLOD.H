/*
** Copyright (c) 1996, 3Dfx Interactive, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of 3Dfx Interactive, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
** rights reserved under the Copyright Laws of the United States.
**
**
** $Revision: 4$ 
** $Date: 10/11/00 7:31:48 PM$ 
**
*/

#ifndef FLTLOD_H
#define FLTLOD_H
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <windows.h>    
#include <glide.h>
#include <texus.h>
#include <atscene.h>    
#include <mgapiall.h>

/* these are things that should be in the API but is not at the moment */

#include <fltcode.h>

typedef struct normcolor {
	float r;
	float g;
	float b;
} normcolor;

typedef struct tagIcoord {
	double x;
	double y;
	double z;
} Icoord;

typedef struct tagVector {
	float i;
	float j;
	float k;
} Vector;

#define COLOR_SET(_c, _r, _g, _b) { _c.r = _r; _c.g = _g; _c.b = _b; }
#define MAT_INC 20

#define MAX_VERTS 100 /* maximum number of vertices in a polygon */

#define MAXMGCOLORINDEX 1024
#define NOTEXTURE               0xffffffff

#define qVertexHasNormal(v)             (v.i != 2.0f)

/*  'type' definitions for mgReadImageHeader/mgReadImage */

#define mgiIntensity    2
#define mgiIntAlpha     3
#define mgiRGB          4
#define mgiRGBA         5

typedef enum tagFVNodeType {
	fvHeader = 0,
	fvGroup = 1,
	fvObject = 2,
	fvPolygon = 3,
	fvVertex = 4,
	fvLodFloat = 5,
	fvLightSource = 6,
        fvDof = 7,
        fvSwitch = 8,
        fvRoad = 9,
        fvPath = 10,
	fvUnknown = 11,
	fvLastNodeType = fvUnknown
} FVNodeType;

typedef struct tagFltNodeMap {
	mgcode		ftAPICode;
	FVNodeType	ftNodeType;
} FltNodeMap;

typedef struct {
    char*   name;
    FxBool  defined;
    AtsTexture *atbTexture;

    /* Texture data */

    FxU8*   pixmap;
    int     type;
    int     width;
    int     height;
    int     extfmt;

    /* Texture-mapping attributes */

    int     imgWrapU;      /* 0=repeat, 1=clamp, 2=obsolete */
    int     imgWrapV;      /* 0=repeat, 1=clamp, 2=obsolete */
    int     imgEnvType;    /* 0=MODULATE, 1= BLEND, 2=DECAL, 3=COLOR */
    int     imgMinFilter;  /* 0=POINT, 1=BILINEAR, 2 =obsolete, see apimissing.h */
    int     imgMagFilter;  /* 0=POINT, 1=BILINEAR, 2 =obsolete, see apimissing.h */
} SgTexture;

typedef struct {
    FxU16 materialCode;
    char *name;
    normcolor amb;
    normcolor dif;
    normcolor spe;
    normcolor emi;
    float shine;
    float alpha;
} SgMaterial;

#define ATMG_TRANSP_NONE    0
#define ATMG_TRANSP_VERTEX  1
#define ATMG_TRANSP_TEXTURE 2
#define ATMG_TRANSP_BLEND   3

typedef struct {
    FxU8 template;
    short textureIndex;
    SgMaterial mat;
    AtsMaterial *atbMaterial;
    FxU8 lightMode;
    FxU32 transpMode;
    float transparency;
    FxU8 facet_r, facet_g, facet_b;
} MatDef;

#endif
