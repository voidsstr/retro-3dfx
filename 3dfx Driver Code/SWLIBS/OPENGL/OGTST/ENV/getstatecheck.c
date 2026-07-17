/*
** Copyright 1992, Silicon Graphics, Inc.
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

#include <string.h>
#include <stdio.h>   /* for sprintf */
#include "ogtst.h"
#include "getstatecheck.h"

#define FLOAT_NEQUALS(x, y)  ((x) != (y))

static void simpleGetCheck(stateRec *);
static void checkEnabled(stateRec *);
static void checkClipPlanes(stateRec *);
static void checkLights(stateRec *);
static void checkEvalMap(stateRec *);
static void checkPixelMap(stateRec *);
static void checkPolygonStipple(stateRec *);
static void checkTexParam(stateRec *);
static void checkTexLevelParams(stateRec *);
static void checkVisual(stateRec *);
static void checkWindow(stateRec *);
static void getFuncCheck(stateRec *);
static void getTargetParameterCheck(stateRec *);
static void getTexFilterHack(GLenum, GLenum, GLfloat *);
#if 0                           /* Not used because Problems with 0 sizes */
static void checkTexLevelExtParams(stateRec *);
#endif

static stateRec state[] = {
    {
	{GL_ACCUM_ALPHA_BITS, OG_STATE_CHECK_NULL}, "ACCUM_ALPHA_BITS", NULL,
	checkVisual, STATEDATA_ONTHEFLY, 1, NULL
    },
    {
	{GL_ACCUM_BLUE_BITS, OG_STATE_CHECK_NULL}, "ACCUM_BLUE_BITS", NULL,
	checkVisual, STATEDATA_ONTHEFLY, 1, NULL
    },
    {
	{GL_ACCUM_GREEN_BITS, OG_STATE_CHECK_NULL}, "ACCUM_GREEN_BITS", NULL,
	checkVisual, STATEDATA_ONTHEFLY, 1, NULL
    },
    {
	{GL_ACCUM_RED_BITS, OG_STATE_CHECK_NULL}, "ACCUM_RED_BITS", NULL,
	checkVisual, STATEDATA_ONTHEFLY, 1, NULL
    },
    {
	{GL_ACCUM_CLEAR_VALUE, OG_STATE_CHECK_NULL}, "ACCUM_CLEAR_VALUE", NULL,
	simpleGetCheck, STATEDATA_PREDEFINED, 4, NULL, {0, 0, 0, 0}
    },
    {
	{GL_ALPHA_BIAS, OG_STATE_CHECK_NULL}, "ALPHA_BIAS", NULL, simpleGetCheck,
	STATEDATA_PREDEFINED, 1, NULL, {0}
    },
    {
	{GL_ALPHA_BITS, OG_STATE_CHECK_NULL}, "ALPHA_BITS", NULL, checkVisual,
	STATEDATA_ONTHEFLY, 1, NULL
    },
    {
	{GL_ALPHA_SCALE, OG_STATE_CHECK_NULL}, "ALPHA_SCALE", NULL, simpleGetCheck,
	STATEDATA_PREDEFINED, 1, NULL, {1} 
    },
    {
	{GL_ALPHA_TEST, OG_STATE_CHECK_NULL}, "ALPHA_TEST", NULL, checkEnabled,
	STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_ALPHA_TEST_FUNC, OG_STATE_CHECK_NULL}, "ALPHA_TEST_FUNC", NULL,
	simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {GL_ALWAYS}
    },
    {
	{GL_ALPHA_TEST_REF, OG_STATE_CHECK_NULL}, "ALPHA_TEST_REF", NULL,
	simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {0}
    },
    {
	{GL_ATTRIB_STACK_DEPTH, OG_STATE_CHECK_NULL}, "ATTRIB_STACK_DEPTH", NULL,
	simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {0}
    },
    {
	{GL_AUTO_NORMAL, OG_STATE_CHECK_NULL}, "AUTO_NORMAL", NULL, checkEnabled,
	STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_AUX_BUFFERS, OG_STATE_CHECK_NULL}, "AUX_BUFFERS", NULL, checkVisual,
	STATEDATA_ONTHEFLY, 1, NULL
    },
    {
	{GL_BLEND, OG_STATE_CHECK_NULL}, "BLEND", NULL, checkEnabled,
	STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_BLEND_DST, OG_STATE_CHECK_NULL}, "BLEND_DST", NULL, simpleGetCheck,
	STATEDATA_PREDEFINED, 1, NULL, {GL_ZERO}
    },
    {
	{GL_BLEND_SRC, OG_STATE_CHECK_NULL}, "BLEND_SRC", NULL, simpleGetCheck,
	STATEDATA_PREDEFINED, 1, NULL, {GL_ONE}
    },
    {
	{GL_BLUE_BIAS, OG_STATE_CHECK_NULL}, "BLUE_BIAS", NULL, simpleGetCheck,
	STATEDATA_PREDEFINED, 1, NULL, {0}
    },
    {
	{GL_BLUE_BITS, OG_STATE_CHECK_NULL}, "BLUE_BITS", NULL, checkVisual,
	STATEDATA_ONTHEFLY, 1, NULL
    },
    {
	{GL_BLUE_SCALE, OG_STATE_CHECK_NULL}, "BLUE_SCALE", NULL,
        simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {1}
    },
    {
	{GL_COLOR_CLEAR_VALUE, OG_STATE_CHECK_NULL}, "COLOR_CLEAR_VALUE", NULL,
	simpleGetCheck, STATEDATA_PREDEFINED, 4, NULL, {0, 0, 0, 0}
    },
    {
	{GL_COLOR_MATERIAL, OG_STATE_CHECK_NULL}, "COLOR_MATERIAL", NULL,
        checkEnabled, STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_COLOR_MATERIAL_FACE, OG_STATE_CHECK_NULL}, "COLOR_MATERIAL_FACE",
        NULL, simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {GL_FRONT_AND_BACK}
    },
    {
	{GL_COLOR_MATERIAL_PARAMETER, OG_STATE_CHECK_NULL},
        "COLOR_MATERIAL_PARAMETER",
	NULL, simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {GL_AMBIENT_AND_DIFFUSE}
    },
    {
	{GL_COLOR_WRITEMASK, OG_STATE_CHECK_NULL}, "COLOR_WRITEMASK", NULL,
	simpleGetCheck, STATEDATA_PREDEFINED, 4, NULL,
        {GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE}
    },
    {
	{GL_CULL_FACE, OG_STATE_CHECK_NULL}, "CULL_FACE", NULL, checkEnabled,
	STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_CULL_FACE_MODE, OG_STATE_CHECK_NULL}, "CULL_FACE_MODE", NULL,
        simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {GL_BACK}
    },
    {
	{GL_CURRENT_COLOR, OG_STATE_CHECK_NULL}, "CURRENT_COLOR", NULL,
        simpleGetCheck, STATEDATA_PREDEFINED, 4, NULL, {1, 1, 1, 1}
    },
    {
	{GL_CURRENT_INDEX, OG_STATE_CHECK_NULL}, "CURRENT_INDEX", NULL,
        simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {1}
    },
    {
	{GL_CURRENT_NORMAL, OG_STATE_CHECK_NULL}, "CURRENT_NORMAL", NULL,
        simpleGetCheck, STATEDATA_PREDEFINED, 3, NULL, {0, 0, 1}
    },
    {
	{GL_CURRENT_RASTER_COLOR, OG_STATE_CHECK_NULL}, "CURRENT_RASTER_COLOR",
        NULL, simpleGetCheck, STATEDATA_PREDEFINED, 4, NULL, {1, 1, 1, 1}
    },
    {
	{GL_CURRENT_RASTER_DISTANCE, OG_STATE_CHECK_NULL}, "CURRENT_RASTER_DISTANCE",
        NULL, simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {0}
    },
    {
	{GL_CURRENT_RASTER_INDEX, OG_STATE_CHECK_NULL}, "CURRENT_RASTER_INDEX",
        NULL, simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {1}
    },
    {
	{GL_CURRENT_RASTER_POSITION, OG_STATE_CHECK_NULL},
        "CURRENT_RASTER_POSITION",
	NULL, simpleGetCheck, STATEDATA_PREDEFINED, 4, NULL, {0, 0, 0, 1}
    },
    {
	{GL_CURRENT_RASTER_POSITION_VALID, OG_STATE_CHECK_NULL},
	"CURRENT_RASTER_POSITION_VALID", NULL, simpleGetCheck,
	STATEDATA_PREDEFINED, 1, NULL, {GL_TRUE}
    },
    {
	{GL_CURRENT_RASTER_TEXTURE_COORDS, OG_STATE_CHECK_NULL},
	"CURRENT_RASTER_TEXTURE_COORDS", NULL, simpleGetCheck,
	STATEDATA_PREDEFINED, 4, NULL, {0, 0, 0, 1}
    },
    {
	{GL_CURRENT_TEXTURE_COORDS, OG_STATE_CHECK_NULL},
	"CURRENT_TEXTURE_COORDS", NULL, simpleGetCheck,
	STATEDATA_PREDEFINED, 4, NULL, {0, 0, 0, 1}
    },
    {
	{GL_DEPTH_CLEAR_VALUE, OG_STATE_CHECK_NULL}, "DEPTH_CLEAR_VALUE", NULL,
        simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {1}
    },
    {
	{GL_DEPTH_BIAS, OG_STATE_CHECK_NULL}, "DEPTH_BIAS", NULL, simpleGetCheck,
	STATEDATA_PREDEFINED, 1, NULL, {0}
    },
    {
	{GL_DEPTH_BITS, OG_STATE_CHECK_NULL}, "DEPTH_BITS", NULL, checkVisual,
	STATEDATA_ONTHEFLY, 1, NULL
    },
    {
	{GL_DEPTH_FUNC, OG_STATE_CHECK_NULL}, "DEPTH_FUNC", NULL, simpleGetCheck,
	STATEDATA_PREDEFINED, 1, NULL, {GL_LESS}
    },
    {
	{GL_DEPTH_RANGE, OG_STATE_CHECK_NULL}, "DEPTH_RANGE", NULL, simpleGetCheck,
	STATEDATA_PREDEFINED, 2, NULL, {0, 1}
    },
    {
	{GL_DEPTH_SCALE, OG_STATE_CHECK_NULL}, "DEPTH_SCALE", NULL, simpleGetCheck,
	STATEDATA_PREDEFINED, 1, NULL, {1}
    },
    {
	{GL_DEPTH_TEST, OG_STATE_CHECK_NULL}, "DEPTH_TEST", NULL, checkEnabled,
	STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_DEPTH_WRITEMASK, OG_STATE_CHECK_NULL}, "DEPTH_WRITEMASK", NULL,
	simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {GL_TRUE}
    },
    {
	{GL_DITHER, OG_STATE_CHECK_NULL}, "DITHER", NULL, simpleGetCheck,
	STATEDATA_PREDEFINED, 1, NULL, {GL_TRUE}
    },
    {
	{GL_DOUBLEBUFFER, OG_STATE_CHECK_NULL}, "DOUBLEBUFFER", NULL, checkVisual,
	STATEDATA_ONTHEFLY, 1, NULL
    },
    {
	{GL_DRAW_BUFFER, OG_STATE_CHECK_NULL}, "DRAW_BUFFER", NULL, checkVisual,
	STATEDATA_ONTHEFLY, 1, NULL
    },
    {
	{GL_EDGE_FLAG, OG_STATE_CHECK_NULL}, "EDGE_FLAG", NULL, simpleGetCheck,
	STATEDATA_PREDEFINED, 1, NULL, {GL_TRUE}
    },
    {
	{GL_FOG, OG_STATE_CHECK_NULL}, "FOG", NULL, checkEnabled,
	STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_FOG_COLOR, OG_STATE_CHECK_NULL}, "FOG_COLOR", NULL, simpleGetCheck,
	STATEDATA_PREDEFINED, 4, NULL, {0, 0, 0, 0}
    },
    {
	{GL_FOG_DENSITY, OG_STATE_CHECK_NULL}, "FOG_DENSITY", NULL, simpleGetCheck,
	STATEDATA_PREDEFINED, 1, NULL, {1}
    },
    {
	{GL_FOG_END, OG_STATE_CHECK_NULL}, "FOG_END", NULL, simpleGetCheck,
	STATEDATA_PREDEFINED, 1, NULL, {1}
    },
    {
	{GL_FOG_HINT, OG_STATE_CHECK_NULL}, "FOG_HINT", NULL, simpleGetCheck,
	STATEDATA_PREDEFINED, 1, NULL, {GL_DONT_CARE}
    },
    {
	{GL_FOG_INDEX, OG_STATE_CHECK_NULL}, "FOG_INDEX", NULL, simpleGetCheck,
	STATEDATA_PREDEFINED, 1, NULL, {0}
    },
    {
	{GL_FOG_MODE, OG_STATE_CHECK_NULL}, "FOG_MODE", NULL, simpleGetCheck,
	STATEDATA_PREDEFINED, 1, NULL, {GL_EXP}
    },
    {
	{GL_FOG_START, OG_STATE_CHECK_NULL}, "FOG_START", NULL, simpleGetCheck,
	STATEDATA_PREDEFINED, 1, NULL, {0}
    },
    {
	{GL_FRONT_FACE, OG_STATE_CHECK_NULL}, "FRONT_FACE", NULL, simpleGetCheck,
	STATEDATA_PREDEFINED, 1, NULL, {GL_CCW}
    },
    {
	{GL_GREEN_BIAS, OG_STATE_CHECK_NULL}, "GREEN_BIAS", NULL, simpleGetCheck,
	STATEDATA_PREDEFINED, 1, NULL, {0}
    },
    {
	{GL_GREEN_BITS, OG_STATE_CHECK_NULL}, "GREEN_BITS", NULL, checkVisual,
	STATEDATA_ONTHEFLY, 1, NULL
    },
    {
	{GL_GREEN_SCALE, OG_STATE_CHECK_NULL}, "GREEN_SCALE", NULL, simpleGetCheck,
	STATEDATA_PREDEFINED, 1, NULL, {1}
    },
    {
	{GL_INDEX_BITS, OG_STATE_CHECK_NULL}, "INDEX_BITS", NULL, checkVisual,
	STATEDATA_ONTHEFLY, 1, NULL
    },
    {
	{GL_INDEX_CLEAR_VALUE, OG_STATE_CHECK_NULL}, "INDEX_CLEAR_VALUE", NULL,
	simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {0}
    },
    {
	{GL_INDEX_MODE, OG_STATE_CHECK_NULL}, "INDEX_MODE", NULL, checkVisual,
	STATEDATA_ONTHEFLY, 1, NULL
    },
    {
	{GL_INDEX_OFFSET, OG_STATE_CHECK_NULL}, "INDEX_OFFSET",
        NULL, simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {0}
    },
    {
	{GL_INDEX_SHIFT, OG_STATE_CHECK_NULL}, "INDEX_SHIFT",
        NULL, simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {0}
    },
    {
	{GL_INDEX_WRITEMASK, OG_STATE_CHECK_NULL}, "INDEX_WRITEMASK",
        NULL, checkVisual, STATEDATA_ONTHEFLY, 1, NULL
    },
    {
	{GL_LIGHTING, OG_STATE_CHECK_NULL}, "LIGHTING", NULL, checkEnabled,
	STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_LIGHT_MODEL_AMBIENT, OG_STATE_CHECK_NULL}, "LIGHT_MODEL_AMBIENT",
        NULL, simpleGetCheck, STATEDATA_PREDEFINED, 4, NULL, {0.2, 0.2, 0.2, 1}
    },
    {
	{GL_LIGHT_MODEL_LOCAL_VIEWER, OG_STATE_CHECK_NULL},
        "LIGHT_MODEL_LOCAL_VIEWER",
	NULL, simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_LIGHT_MODEL_TWO_SIDE, OG_STATE_CHECK_NULL}, "LIGHT_MODEL_TWO_SIDE",
	NULL, simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_LINE_SMOOTH, OG_STATE_CHECK_NULL}, "LINE_SMOOTH", NULL,
	checkEnabled, STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_LINE_SMOOTH_HINT, OG_STATE_CHECK_NULL}, "LINE_SMOOTH_HINT", NULL,
	simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {GL_DONT_CARE}
    },
    {
	{GL_LINE_STIPPLE, OG_STATE_CHECK_NULL}, "LINE_STIPPLE", NULL,
        simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_LINE_STIPPLE_PATTERN, OG_STATE_CHECK_NULL}, "LINE_STIPPLE_PATTERN",
	NULL, simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {65535}
    },
    {
	{GL_LINE_STIPPLE_REPEAT, OG_STATE_CHECK_NULL}, "LINE_STIPPLE_REPEAT",
	NULL, simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {1}
    },
    {
	{GL_LINE_WIDTH, OG_STATE_CHECK_NULL}, "LINE_WIDTH", NULL,
        simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {1}
    },
    {
	{GL_LINE_WIDTH_GRANULARITY, OG_STATE_CHECK_NULL}, "LINE_WIDTH_GRANULARITY",
	NULL, simpleGetCheck, STATEDATA_STATE_GET_TARGET, 1, NULL, {0},
    },
    {
	{GL_LINE_WIDTH_RANGE, OG_STATE_CHECK_NULL}, "LINE_WIDTH_RANGE", NULL,
	simpleGetCheck, STATEDATA_STATE_GET_TARGET, 2, NULL, {0,1},
    },
    {
	{GL_LIST_BASE, OG_STATE_CHECK_NULL}, "LIST_BASE", NULL, simpleGetCheck,
	STATEDATA_PREDEFINED, 1, NULL, {0}
    },
    {
	{GL_LIST_INDEX, OG_STATE_CHECK_NULL}, "LIST_INDEX", NULL, simpleGetCheck,
	STATEDATA_PREDEFINED, 1, NULL, {0}
    },
    {
	{GL_LOGIC_OP, OG_STATE_CHECK_NULL}, "LOGIC_OP", NULL, checkEnabled,
	STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_LOGIC_OP_MODE, OG_STATE_CHECK_NULL}, "LOGIC_OP_MODE", NULL,
	simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {GL_COPY}
    },
    {
	{GL_MAP_COLOR, OG_STATE_CHECK_NULL}, "MAP_COLOR", NULL, simpleGetCheck,
	STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_MAP_STENCIL, OG_STATE_CHECK_NULL}, "MAP_STENCIL",
        NULL, simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_MAP1_COLOR_4, OG_STATE_CHECK_NULL}, "MAP1_COLOR_4",
        NULL, checkEnabled, STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_MAP1_GRID_DOMAIN, OG_STATE_CHECK_NULL}, "MAP1_GRID_DOMAIN", NULL,
	simpleGetCheck, STATEDATA_PREDEFINED, 2, NULL, {0, 1}
    },
    {
	{GL_MAP1_GRID_SEGMENTS, OG_STATE_CHECK_NULL}, "MAP1_GRID_SEGMENTS",
        NULL, simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {1}
    },
    {
	{GL_MAP1_INDEX, OG_STATE_CHECK_NULL}, "MAP1_INDEX", NULL, checkEnabled,
	STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_MAP1_NORMAL, OG_STATE_CHECK_NULL}, "MAP1_NORMAL", NULL, checkEnabled,
	STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_MAP1_TEXTURE_COORD_1, OG_STATE_CHECK_NULL}, "MAP1_TEXTURE_COORD_1", NULL,
	checkEnabled, STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_MAP1_TEXTURE_COORD_2, OG_STATE_CHECK_NULL}, "MAP1_TEXTURE_COORD_2", NULL,
	checkEnabled, STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_MAP1_TEXTURE_COORD_3, OG_STATE_CHECK_NULL}, "MAP1_TEXTURE_COORD_3", NULL,
	checkEnabled, STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_MAP1_TEXTURE_COORD_4, OG_STATE_CHECK_NULL}, "MAP1_TEXTURE_COORD_4", NULL,
	checkEnabled, STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_MAP1_VERTEX_3, OG_STATE_CHECK_NULL}, "MAP1_VERTEX_3",
        NULL, checkEnabled, STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_MAP1_VERTEX_4, OG_STATE_CHECK_NULL}, "MAP1_VERTEX_4",
        NULL, checkEnabled, STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_MAP2_COLOR_4, OG_STATE_CHECK_NULL}, "MAP2_COLOR_4",
        NULL, checkEnabled, STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_MAP2_GRID_DOMAIN, OG_STATE_CHECK_NULL}, "MAP2_GRID_DOMAIN", NULL,
	simpleGetCheck, STATEDATA_PREDEFINED, 4, NULL, {0, 1, 0, 1}
    },
    {
	{GL_MAP2_GRID_SEGMENTS, OG_STATE_CHECK_NULL}, "MAP2_GRID_SEGMENTS", NULL,
	simpleGetCheck, STATEDATA_PREDEFINED, 2, NULL, {1, 1}
    },
    {
	{GL_MAP2_INDEX, OG_STATE_CHECK_NULL}, "MAP2_INDEX", NULL, checkEnabled,
	STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_MAP2_NORMAL, OG_STATE_CHECK_NULL}, "MAP2_NORMAL", NULL, checkEnabled,
	STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_MAP2_TEXTURE_COORD_1, OG_STATE_CHECK_NULL}, "MAP2_TEXTURE_COORD_1",
        NULL, checkEnabled, STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_MAP2_TEXTURE_COORD_2, OG_STATE_CHECK_NULL}, "MAP2_TEXTURE_COORD_2",
        NULL, checkEnabled, STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_MAP2_TEXTURE_COORD_3, OG_STATE_CHECK_NULL}, "MAP2_TEXTURE_COORD_3",
        NULL, checkEnabled, STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_MAP2_TEXTURE_COORD_4, OG_STATE_CHECK_NULL}, "MAP2_TEXTURE_COORD_4",
        NULL, checkEnabled, STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_MAP2_VERTEX_3, OG_STATE_CHECK_NULL}, "MAP2_VERTEX_3", NULL,
        checkEnabled, STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_MAP2_VERTEX_4, OG_STATE_CHECK_NULL}, "MAP2_VERTEX_4", NULL,
        checkEnabled, STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_MATRIX_MODE, OG_STATE_CHECK_NULL}, "MATRIX_MODE", NULL,
        simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {GL_MODELVIEW}
    },
    {
	{GL_MAX_ATTRIB_STACK_DEPTH, OG_STATE_CHECK_NULL},
        "MAX_ATTRIB_STACK_DEPTH",
	NULL, simpleGetCheck, STATEDATA_STATE_GET_TARGET, 1, NULL, {16},
    },
    {
	{GL_MAX_CLIP_PLANES, OG_STATE_CHECK_NULL}, "MAX_CLIP_PLANES", NULL,
	simpleGetCheck, STATEDATA_STATE_GET_TARGET, 1, NULL, {6},
    },
    {
	{GL_MAX_EVAL_ORDER, OG_STATE_CHECK_NULL}, "MAX_EVAL_ORDER", NULL,
        simpleGetCheck, STATEDATA_STATE_GET_TARGET, 1, NULL, {8},
    },
    {
	{GL_MAX_LIGHTS, OG_STATE_CHECK_NULL}, "MAX_LIGHTS", NULL,
        simpleGetCheck, STATEDATA_STATE_GET_TARGET, 1, NULL, {8},
    },
    {
	{GL_MAX_LIST_NESTING, OG_STATE_CHECK_NULL}, "MAX_LIST_NESTING", NULL,
	simpleGetCheck, STATEDATA_STATE_GET_TARGET, 1, NULL, {64},
    },
    {
	{GL_MAX_MODELVIEW_STACK_DEPTH, OG_STATE_CHECK_NULL},
        "MAX_MODELVIEW_STACK_DEPTH",
	NULL, simpleGetCheck, STATEDATA_STATE_GET_TARGET, 1, NULL, {32},
    },
    {
	{GL_MAX_NAME_STACK_DEPTH, OG_STATE_CHECK_NULL},
        "MAX_NAME_STACK_DEPTH", NULL,
	simpleGetCheck, STATEDATA_STATE_GET_TARGET, 1, NULL, {64},
    },
    {
	{GL_MAX_PIXEL_MAP_TABLE, OG_STATE_CHECK_NULL},
        "MAX_PIXEL_MAP_TABLE", NULL,
	simpleGetCheck, STATEDATA_STATE_GET_TARGET, 1, NULL, {32},
    },
    {
	{GL_MAX_PROJECTION_STACK_DEPTH, OG_STATE_CHECK_NULL},
	"MAX_PROJECTION_STACK_DEPTH", NULL, simpleGetCheck,
	STATEDATA_STATE_GET_TARGET, 1, NULL, {2},
    },
    {
	{GL_MAX_TEXTURE_SIZE, OG_STATE_CHECK_NULL}, "MAX_TEXTURE_SIZE", NULL,
	simpleGetCheck, STATEDATA_STATE_GET_TARGET, 1, NULL, {64},
    },
    {
	{GL_MAX_TEXTURE_STACK_DEPTH, OG_STATE_CHECK_NULL},
        "MAX_TEXTURE_STACK_DEPTH",
	NULL, simpleGetCheck, STATEDATA_STATE_GET_TARGET, 1, NULL, {2},
    },
    {
	{GL_MAX_VIEWPORT_DIMS, OG_STATE_CHECK_NULL}, "MAX_VIEWPORT_DIMS", NULL,
	simpleGetCheck, STATEDATA_STATE_GET_TARGET, 2, NULL,
        /* XXXZiv
         * The dimensions should be >= the size of the display being rendered
         * to. It too much hassel to introduce a mechanism that does this, so I
         * just picked 640x480.
         */
        {640, 480},
    },
    {
	{GL_MODELVIEW_MATRIX, OG_STATE_CHECK_NULL},
	"MODELVIEW_MATRIX", NULL, simpleGetCheck,
	STATEDATA_PREDEFINED, 16, NULL, {1, 0, 0, 0,
                                   0, 1, 0, 0,
                                   0, 0, 1, 0,
                                   0, 0, 0, 1}
    },
    {
	{GL_MODELVIEW_STACK_DEPTH, OG_STATE_CHECK_NULL}, "MODELVIEW_STACK_DEPTH",
        NULL, simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {1}
    },
    {
	{GL_NAME_STACK_DEPTH, OG_STATE_CHECK_NULL}, "NAME_STACK_DEPTH", NULL,
	simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {0}
    },
    {
	{GL_NORMALIZE, OG_STATE_CHECK_NULL}, "NORMALIZE", NULL, checkEnabled,
	STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_PACK_ALIGNMENT, OG_STATE_CHECK_NULL}, "PACK_ALIGNMENT", NULL,
        simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {4}
    },
    {
	{GL_PACK_LSB_FIRST, OG_STATE_CHECK_NULL}, "PACK_LSB_FIRST", NULL,
        simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_PACK_ROW_LENGTH, OG_STATE_CHECK_NULL}, "PACK_ROW_LENGTH", NULL,
	simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {0}
    },
    {
	{GL_PACK_SKIP_PIXELS, OG_STATE_CHECK_NULL}, "PACK_SKIP_PIXELS", NULL,
	simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {0}
    },
    {
	{GL_PACK_SKIP_ROWS, OG_STATE_CHECK_NULL}, "PACK_SKIP_ROWS", NULL,
	simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {0}
    },
    {
	{GL_PACK_SWAP_BYTES, OG_STATE_CHECK_NULL}, "PACK_SWAP_BYTES", NULL,
	simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_PERSPECTIVE_CORRECTION_HINT, OG_STATE_CHECK_NULL},
	"PERSPECTIVE_CORRECTION_HINT", NULL, simpleGetCheck,
	STATEDATA_PREDEFINED, 1, NULL, {GL_DONT_CARE}
    },
    {
	{GL_POINT_SIZE, OG_STATE_CHECK_NULL}, "POINT_SIZE", NULL,
	simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {1}
    },
    {
	{GL_POINT_SIZE_GRANULARITY, OG_STATE_CHECK_NULL},
        "POINT_SIZE_GRANULARITY",
	NULL, simpleGetCheck, STATEDATA_STATE_GET_TARGET, 1, NULL, {0},
    },
    {
	{GL_POINT_SIZE_RANGE, OG_STATE_CHECK_NULL}, "POINT_SIZE_RANGE", NULL,
	simpleGetCheck, STATEDATA_STATE_GET_TARGET, 2, NULL, {0, 1},
    },
    {
	{GL_POINT_SMOOTH, OG_STATE_CHECK_NULL}, "POINT_SMOOTH", NULL,
	checkEnabled, STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_POINT_SMOOTH_HINT, OG_STATE_CHECK_NULL}, "POINT_SMOOTH_HINT", NULL,
	simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {GL_DONT_CARE}
    },
    {
	{GL_POLYGON_MODE, OG_STATE_CHECK_NULL}, "POLYGON_MODE", NULL,
	simpleGetCheck, STATEDATA_PREDEFINED, 2, NULL, {GL_FILL, GL_FILL}
    },
    {
	{GL_POLYGON_SMOOTH, OG_STATE_CHECK_NULL}, "POLYGON_SMOOTH", NULL,
	checkEnabled, STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_POLYGON_SMOOTH_HINT, OG_STATE_CHECK_NULL}, "POLYGON_SMOOTH_HINT",
        NULL, simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {GL_DONT_CARE}
    },
    {
	{GL_POLYGON_STIPPLE, OG_STATE_CHECK_NULL}, "POLYGON_STIPPLE", NULL,
	checkEnabled, STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_PROJECTION_MATRIX, OG_STATE_CHECK_NULL}, "PROJECTION_MATRIX",
        NULL, simpleGetCheck, STATEDATA_PREDEFINED, 16, NULL,
        {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}
    },
    {
	{GL_PROJECTION_STACK_DEPTH, OG_STATE_CHECK_NULL},
        "PROJECTION_STACK_DEPTH",
	NULL, simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {1}
    },
    {
	{GL_READ_BUFFER, OG_STATE_CHECK_NULL}, "READ_BUFFER", NULL,
	checkVisual, STATEDATA_ONTHEFLY, 1, NULL
    },
    {
	{GL_RED_BIAS, OG_STATE_CHECK_NULL}, "RED_BIAS", NULL,
	simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {0}
    },
    {
	{GL_RED_BITS, OG_STATE_CHECK_NULL}, "RED_BITS", NULL,
	checkVisual, STATEDATA_ONTHEFLY, 1, NULL
    },
    {
	{GL_RED_SCALE, OG_STATE_CHECK_NULL}, "RED_SCALE", NULL,
	simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {1}
    },
    {
	{GL_RENDER_MODE, OG_STATE_CHECK_NULL}, "RENDER_MODE", NULL,
	simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {GL_RENDER}
    },
    {
	{GL_RGBA_MODE, OG_STATE_CHECK_NULL}, "RGBA_MODE", NULL,
	checkVisual, STATEDATA_ONTHEFLY, 1, NULL
    },
    {
	{GL_SCISSOR_BOX, OG_STATE_CHECK_NULL}, "SCISSOR_BOX", NULL,
	checkWindow, STATEDATA_WINDOW_GET_TARGET, 4, NULL
    },
    {
	{GL_SCISSOR_TEST, OG_STATE_CHECK_NULL}, "SCISSOR_TEST", NULL,
	checkEnabled, STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_SHADE_MODEL, OG_STATE_CHECK_NULL}, "SHADE_MODEL", NULL,
	simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {GL_SMOOTH}
    },
    {
	{GL_STENCIL_BITS, OG_STATE_CHECK_NULL}, "STENCIL_BITS", NULL,
	checkVisual, STATEDATA_ONTHEFLY, 1, NULL
    },
    {
	{GL_STENCIL_CLEAR_VALUE, OG_STATE_CHECK_NULL}, "STENCIL_CLEAR_VALUE",
        NULL, simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {0}
    },
    {
	{GL_STENCIL_FAIL, OG_STATE_CHECK_NULL}, "STENCIL_FAIL", NULL,
	simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {GL_KEEP}
    },
    {
	{GL_STENCIL_FUNC, OG_STATE_CHECK_NULL}, "STENCIL_FUNC", NULL,
	simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {GL_ALWAYS}
    },
    {
	{GL_STENCIL_PASS_DEPTH_FAIL, OG_STATE_CHECK_NULL},
        "STENCIL_PASS_DEPTH_FAIL",
	NULL, simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {GL_KEEP}
    },
    {
	{GL_STENCIL_PASS_DEPTH_PASS, OG_STATE_CHECK_NULL},
        "STENCIL_PASS_DEPTH_PASS",
	NULL, simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {GL_KEEP}
    },
    {
	{GL_STENCIL_REF, OG_STATE_CHECK_NULL}, "STENCIL_REF", NULL,
	simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {0}
    },
    {
	{GL_STENCIL_TEST, OG_STATE_CHECK_NULL}, "STENCIL_TEST", NULL,
	checkEnabled, STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_STENCIL_VALUE_MASK, OG_STATE_CHECK_NULL}, "STENCIL_VALUE_MASK",
        NULL, checkVisual, STATEDATA_ONTHEFLY, 1, NULL
    },
    {
	{GL_STENCIL_WRITEMASK, OG_STATE_CHECK_NULL}, "STENCIL_WRITEMASK",
        NULL, checkVisual, STATEDATA_ONTHEFLY, 1, NULL
    },
    {
	{GL_STEREO, OG_STATE_CHECK_NULL}, "STEREO", NULL,
	checkVisual, STATEDATA_ONTHEFLY, 1, NULL
    },
#if 0
    /* The spec defines it, but does not say what is does.  So for the time
     * being it's irrelevant.
     */
    {
	{GL_SUBPIXEL_BITS, OG_STATE_CHECK_NULL}, "SUBPIXEL_BITS", NULL,
	simpleGetCheck, STATEDATA_STATE_GET_TARGET, 1, NULL, {4},
    },
#endif
    {
	{GL_TEXTURE_1D, OG_STATE_CHECK_NULL}, "TEXTURE_1D", NULL,
	checkEnabled, STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_TEXTURE_2D, OG_STATE_CHECK_NULL}, "TEXTURE_2D", NULL,
	checkEnabled, STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_TEXTURE_GEN_R, OG_STATE_CHECK_NULL}, "TEXTURE_GEN_R", NULL,
	checkEnabled, STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_TEXTURE_GEN_Q, OG_STATE_CHECK_NULL}, "TEXTURE_GEN_Q", NULL,
	checkEnabled, STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_TEXTURE_GEN_S, OG_STATE_CHECK_NULL}, "TEXTURE_GEN_S", NULL,
	checkEnabled, STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_TEXTURE_GEN_T, OG_STATE_CHECK_NULL}, "TEXTURE_GEN_T", NULL,
	checkEnabled, STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_TEXTURE_MATRIX, OG_STATE_CHECK_NULL}, "TEXTURE_MATRIX", NULL,
	simpleGetCheck, STATEDATA_PREDEFINED, 16, NULL,
        {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}
    },
    {
	{GL_TEXTURE_STACK_DEPTH, OG_STATE_CHECK_NULL}, "TEXTURE_STACK_DEPTH",
        NULL, simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {1}
    },
    {
	{GL_UNPACK_ALIGNMENT, OG_STATE_CHECK_NULL}, "UNPACK_ALIGNMENT", NULL,
	simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {4}
    },
    {
	{GL_UNPACK_LSB_FIRST, OG_STATE_CHECK_NULL}, "UNPACK_LSB_FIRST", NULL,
	simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_UNPACK_ROW_LENGTH, OG_STATE_CHECK_NULL}, "UNPACK_ROW_LENGTH", NULL,
	simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {0}
    },
    {
	{GL_UNPACK_SKIP_PIXELS, OG_STATE_CHECK_NULL}, "UNPACK_SKIP_PIXELS",
        NULL, simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {0}
    },
    {
	{GL_UNPACK_SKIP_ROWS, OG_STATE_CHECK_NULL}, "UNPACK_SKIP_ROWS", NULL,
	simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {0}
    },
    {
	{GL_UNPACK_SWAP_BYTES, OG_STATE_CHECK_NULL}, "UNPACK_SWAP_BYTES",
        NULL, simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_VIEWPORT, OG_STATE_CHECK_NULL}, "VIEWPORT", NULL,
	checkWindow, STATEDATA_WINDOW_GET_TARGET, 4, NULL
    },
    {
	{GL_ZOOM_X, OG_STATE_CHECK_NULL}, "ZOOM_X", NULL,
	simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {1}
    },
    {
	{GL_ZOOM_Y, OG_STATE_CHECK_NULL}, "ZOOM_Y", NULL,
	simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {1}
    },
    {
        /* All clip planes are checked thru checkClipPlanes */
	{GL_CLIP_PLANE0, OG_STATE_CHECK_NULL}, "CLIP_PLANES", NULL,
	checkClipPlanes, STATEDATA_PREDEFINED, 4, NULL, {0, 0, 0, 0}
    },
    {
        /* Parameters of all lights are checked in checkLights */
	{GL_LIGHT0, OG_STATE_CHECK_NULL}, "LIGHT0", NULL, checkLights,
	STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_MAP1_COLOR_4, GL_COEFF, OG_STATE_CHECK_NULL}, "MAP1_COLOR_4",
        "COEFF", checkEvalMap, STATEDATA_PREDEFINED, 4, NULL, {1, 1, 1, 1}
    },
    {
	{GL_MAP1_COLOR_4, GL_DOMAIN, OG_STATE_CHECK_NULL}, "MAP1_COLOR_4",
        "DOMAIN", checkEvalMap, STATEDATA_PREDEFINED, 2, NULL, {0, 1}
    },
    {
	{GL_MAP1_COLOR_4, GL_ORDER, OG_STATE_CHECK_NULL}, "MAP1_COLOR_4",
        "ORDER", checkEvalMap, STATEDATA_PREDEFINED, 1, NULL, {1}
    },
    {
	{GL_MAP1_INDEX, GL_COEFF, OG_STATE_CHECK_NULL}, "MAP1_INDEX",
        "COEFF",checkEvalMap, STATEDATA_PREDEFINED, 1, NULL, {1}
    },
    {
	{GL_MAP1_INDEX, GL_DOMAIN, OG_STATE_CHECK_NULL}, "MAP1_INDEX",
        "DOMAIN", checkEvalMap, STATEDATA_PREDEFINED, 2, NULL, {0, 1}
    },
    {
	{GL_MAP1_INDEX, GL_ORDER, OG_STATE_CHECK_NULL}, "MAP1_INDEX",
        "ORDER", checkEvalMap, STATEDATA_PREDEFINED, 1, NULL, {1}
    },
    {
	{GL_MAP1_NORMAL, GL_COEFF, OG_STATE_CHECK_NULL}, "MAP1_NORMAL",
        "COEFF", checkEvalMap, STATEDATA_PREDEFINED, 3, NULL, {0, 0, 1}
    },
    {
	{GL_MAP1_NORMAL, GL_DOMAIN, OG_STATE_CHECK_NULL}, "MAP1_NORMAL",
        "DOMAIN", checkEvalMap, STATEDATA_PREDEFINED, 2, NULL, {0, 1}
    },
    {
	{GL_MAP1_NORMAL, GL_ORDER, OG_STATE_CHECK_NULL}, "MAP1_NORMAL",
        "ORDER", checkEvalMap, STATEDATA_PREDEFINED, 1, NULL, {1}
    },
    {
	{GL_MAP1_TEXTURE_COORD_1, GL_COEFF, OG_STATE_CHECK_NULL},
        "MAP1_TEXTURE_COORD_1", "COEFF", checkEvalMap,
        STATEDATA_PREDEFINED, 1, NULL, {0}
    },
    {
	{GL_MAP1_TEXTURE_COORD_1, GL_DOMAIN, OG_STATE_CHECK_NULL},
        "MAP1_TEXTURE_COORD_1", "DOMAIN", checkEvalMap,
        STATEDATA_PREDEFINED, 2, NULL, {0, 1}
    },
    {
	{GL_MAP1_TEXTURE_COORD_1, GL_ORDER, OG_STATE_CHECK_NULL},
        "MAP1_TEXTURE_COORD_1", "ORDER", checkEvalMap,
        STATEDATA_PREDEFINED, 1, NULL, {1}
    },
    {
	{GL_MAP1_TEXTURE_COORD_2, GL_COEFF, OG_STATE_CHECK_NULL},
        "MAP1_TEXTURE_COORD_2", "COEFF", checkEvalMap,
        STATEDATA_PREDEFINED, 2, NULL, {0, 0}
    },
    {
	{GL_MAP1_TEXTURE_COORD_2, GL_DOMAIN, OG_STATE_CHECK_NULL},
        "MAP1_TEXTURE_COORD_2", "DOMAIN", checkEvalMap,
        STATEDATA_PREDEFINED, 2, NULL, {0, 1}
    },
    {
	{GL_MAP1_TEXTURE_COORD_2, GL_ORDER, OG_STATE_CHECK_NULL},
        "MAP1_TEXTURE_COORD_2", "ORDER", checkEvalMap,
        STATEDATA_PREDEFINED, 1, NULL, {1}
    },
    {
	{GL_MAP1_TEXTURE_COORD_3, GL_COEFF, OG_STATE_CHECK_NULL},
        "MAP1_TEXTURE_COORD_3", "COEFF", checkEvalMap,
        STATEDATA_PREDEFINED, 3, NULL, {0, 0, 0}
    },
    {
	{GL_MAP1_TEXTURE_COORD_3, GL_DOMAIN, OG_STATE_CHECK_NULL},
        "MAP1_TEXTURE_COORD_3", "DOMAIN", checkEvalMap,
        STATEDATA_PREDEFINED, 2, NULL, {0, 1}
    },
    {
	{GL_MAP1_TEXTURE_COORD_3, GL_ORDER, OG_STATE_CHECK_NULL},
        "MAP1_TEXTURE_COORD_3", "ORDER", checkEvalMap,
        STATEDATA_PREDEFINED, 1, NULL, {1}
    },
    {
	{GL_MAP1_TEXTURE_COORD_4, GL_COEFF, OG_STATE_CHECK_NULL},
        "MAP1_TEXTURE_COORD_4", "COEFF", checkEvalMap,
        STATEDATA_PREDEFINED, 4, NULL, {0, 0, 0, 1}
    },
    {
	{GL_MAP1_TEXTURE_COORD_4, GL_DOMAIN, OG_STATE_CHECK_NULL},
        "MAP1_TEXTURE_COORD_4", "DOMAIN", checkEvalMap,
        STATEDATA_PREDEFINED, 2, NULL, {0, 1}
    },
    {
	{GL_MAP1_TEXTURE_COORD_4, GL_ORDER, OG_STATE_CHECK_NULL},
        "MAP1_TEXTURE_COORD_4", "ORDER", checkEvalMap,
        STATEDATA_PREDEFINED, 1, NULL, {1}
    },
    {
	{GL_MAP1_VERTEX_3, GL_COEFF, OG_STATE_CHECK_NULL},
        "MAP1_VERTEX_3", "COEFF", checkEvalMap,
        STATEDATA_PREDEFINED, 3, NULL, {0, 0, 0}
    },
    {
	{GL_MAP1_VERTEX_3, GL_DOMAIN, OG_STATE_CHECK_NULL},
        "MAP1_VERTEX_3", "DOMAIN", checkEvalMap,
        STATEDATA_PREDEFINED, 2, NULL, {0, 1}
    },
    {
	{GL_MAP1_VERTEX_3, GL_ORDER, OG_STATE_CHECK_NULL},
        "MAP1_VERTEX_3", "ORDER", checkEvalMap,
        STATEDATA_PREDEFINED, 1, NULL, {1}
    },
    {
	{GL_MAP1_VERTEX_4, GL_COEFF, OG_STATE_CHECK_NULL},
        "MAP1_VERTEX_4", "COEFF", checkEvalMap,
        STATEDATA_PREDEFINED, 4, NULL, {0, 0, 0, 1}
    },
    {
	{GL_MAP1_VERTEX_4, GL_DOMAIN, OG_STATE_CHECK_NULL},
        "MAP1_VERTEX_4", "DOMAIN", checkEvalMap,
        STATEDATA_PREDEFINED, 2, NULL, {0, 1}
    },
    {
	{GL_MAP1_VERTEX_4, GL_ORDER, OG_STATE_CHECK_NULL},
        "MAP1_VERTEX_4", "ORDER", checkEvalMap,
        STATEDATA_PREDEFINED, 1, NULL, {1}
    },
    {
	{GL_MAP2_COLOR_4, GL_COEFF, OG_STATE_CHECK_NULL},
        "MAP2_COLOR_4", "COEFF", checkEvalMap,
        STATEDATA_PREDEFINED, 4, NULL, {1, 1, 1, 1}
    },
    {
	{GL_MAP2_COLOR_4, GL_DOMAIN, OG_STATE_CHECK_NULL},
        "MAP2_COLOR_4", "DOMAIN", checkEvalMap,
        STATEDATA_PREDEFINED, 4, NULL, {0, 1, 0, 1}
    },
    {
	{GL_MAP2_COLOR_4, GL_ORDER, OG_STATE_CHECK_NULL},
        "MAP2_COLOR_4", "ORDERGL_ORDER", checkEvalMap,
        STATEDATA_PREDEFINED, 2, NULL, {1, 1}
    },
    {
	{GL_MAP2_INDEX, GL_COEFF, OG_STATE_CHECK_NULL},
        "MAP2_INDEX", "COEFF", checkEvalMap,
        STATEDATA_PREDEFINED, 1, NULL, {1}
    },
    {
	{GL_MAP2_INDEX, GL_DOMAIN, OG_STATE_CHECK_NULL},
        "MAP2_INDEX", "DOMAIN", checkEvalMap,
        STATEDATA_PREDEFINED, 4, NULL, {0, 1, 0, 1}
    },
    {
	{GL_MAP2_INDEX, GL_ORDER, OG_STATE_CHECK_NULL},
        "MAP2_INDEX", "ORDER", checkEvalMap,
        STATEDATA_PREDEFINED, 2, NULL, {1, 1}
    },
    {
	{GL_MAP2_NORMAL, GL_COEFF, OG_STATE_CHECK_NULL},
        "MAP2_NORMAL", "COEFF", checkEvalMap,
        STATEDATA_PREDEFINED, 3, NULL, {0, 0, 1}
    },
    {
	{GL_MAP2_NORMAL, GL_DOMAIN, OG_STATE_CHECK_NULL},
        "MAP2_NORMAL", "DOMAIN", checkEvalMap,
        STATEDATA_PREDEFINED, 4, NULL, {0, 1, 0, 1}
    },
    {
	{GL_MAP2_NORMAL, GL_ORDER, OG_STATE_CHECK_NULL},
        "MAP2_NORMAL", "ORDER", checkEvalMap,
        STATEDATA_PREDEFINED, 2, NULL, {1, 1}
    },
    {
	{GL_MAP2_TEXTURE_COORD_1, GL_COEFF, OG_STATE_CHECK_NULL},
        "MAP2_TEXTURE_COORD_1", " COEFF", checkEvalMap,
        STATEDATA_PREDEFINED, 1, NULL, {0}
    },
    {
	{GL_MAP2_TEXTURE_COORD_1, GL_DOMAIN, OG_STATE_CHECK_NULL},
        "MAP2_TEXTURE_COORD_1", "DOMAIN", checkEvalMap,
        STATEDATA_PREDEFINED, 4, NULL, {0, 1, 0, 1}
    },
    {
	{GL_MAP2_TEXTURE_COORD_1, GL_ORDER, OG_STATE_CHECK_NULL},
        "MAP2_TEXTURE_COORD_1", "ORDER", checkEvalMap,
        STATEDATA_PREDEFINED, 2, NULL, {1, 1}
    },
    {
	{GL_MAP2_TEXTURE_COORD_2, GL_COEFF, OG_STATE_CHECK_NULL},
        "MAP2_TEXTURE_COORD_2", "COEFF", checkEvalMap,
        STATEDATA_PREDEFINED, 2, NULL, {0, 0}
    },
    {
	{GL_MAP2_TEXTURE_COORD_2, GL_DOMAIN, OG_STATE_CHECK_NULL},
        "MAP2_TEXTURE_COORD_2", "DOMAIN", checkEvalMap,
        STATEDATA_PREDEFINED, 4, NULL, {0, 1, 0, 1}
    },
    {
	{GL_MAP2_TEXTURE_COORD_2, GL_ORDER, OG_STATE_CHECK_NULL},
        "MAP2_TEXTURE_COORD_2", "ORDER", checkEvalMap,
        STATEDATA_PREDEFINED, 2, NULL, {1, 1}
    },
    {
	{GL_MAP2_TEXTURE_COORD_3, GL_COEFF, OG_STATE_CHECK_NULL},
        "MAP2_TEXTURE_COORD_3", "COEFF", checkEvalMap,
        STATEDATA_PREDEFINED, 3, NULL, {0, 0, 0}
    },
    {
	{GL_MAP2_TEXTURE_COORD_3, GL_DOMAIN, OG_STATE_CHECK_NULL},
        "MAP2_TEXTURE_COORD_3", "DOMAIN", checkEvalMap,
        STATEDATA_PREDEFINED, 4, NULL, {0, 1, 0, 1}
    },
    {
	{GL_MAP2_TEXTURE_COORD_3, GL_ORDER, OG_STATE_CHECK_NULL},
        "MAP2_TEXTURE_COORD_3", "ORDER", checkEvalMap,
        STATEDATA_PREDEFINED, 2, NULL, {1, 1}
    },
    {
	{GL_MAP2_TEXTURE_COORD_4, GL_COEFF, OG_STATE_CHECK_NULL},
        "MAP2_TEXTURE_COORD_4", "COEFF", checkEvalMap,
        STATEDATA_PREDEFINED, 4, NULL, {0, 0, 0, 1}
    },
    {
	{GL_MAP2_TEXTURE_COORD_4, GL_DOMAIN, OG_STATE_CHECK_NULL},
        "MAP2_TEXTURE_COORD_4", "DOMAIN", checkEvalMap,
        STATEDATA_PREDEFINED, 4, NULL, {0, 1, 0, 1}
    },
    {
	{GL_MAP2_TEXTURE_COORD_4, GL_ORDER, OG_STATE_CHECK_NULL},
        "MAP2_TEXTURE_COORD_4", "ORDER", checkEvalMap,
        STATEDATA_PREDEFINED, 2, NULL, {1, 1}
    },
    {
	{GL_MAP2_VERTEX_3, GL_COEFF, OG_STATE_CHECK_NULL},
        "MAP2_VERTEX_3", "COEFF", checkEvalMap, STATEDATA_PREDEFINED,
        3, NULL, {0, 0, 0}
    },
    {
	{GL_MAP2_VERTEX_3, GL_DOMAIN, OG_STATE_CHECK_NULL},
        "MAP2_VERTEX_3", "DOMAIN", checkEvalMap, STATEDATA_PREDEFINED,
        4, NULL, {0, 1, 0, 1}
    },
    {
	{GL_MAP2_VERTEX_3, GL_ORDER, OG_STATE_CHECK_NULL},
        "MAP2_VERTEX_3", "ORDER", checkEvalMap, STATEDATA_PREDEFINED,
        2, NULL, {1, 1}
    },
    {
	{GL_MAP2_VERTEX_4, GL_COEFF, OG_STATE_CHECK_NULL},
        "MAP2_VERTEX_4", "COEFF", checkEvalMap, STATEDATA_PREDEFINED,
        4, NULL, {0, 0, 0, 1}
    },
    {
	{GL_MAP2_VERTEX_4, GL_DOMAIN, OG_STATE_CHECK_NULL},
        "MAP2_VERTEX_4", "DOMAIN", checkEvalMap, STATEDATA_PREDEFINED,
        4, NULL, {0, 1, 0, 1}
    },
    {
	{GL_MAP2_VERTEX_4, GL_ORDER, OG_STATE_CHECK_NULL},
        "MAP2_VERTEX_4", "ORDER", checkEvalMap, STATEDATA_PREDEFINED,
        2, NULL, {1, 1}
    },
    {
	{GL_BACK, GL_AMBIENT, OG_STATE_CHECK_NULL}, "BACK", "AMBIENT",
	getTargetParameterCheck, STATEDATA_PREDEFINED, 4,
        (void (*) (void)) glGetMaterialfv, {0.2, 0.2, 0.2, 1}
    },
    {
	{GL_BACK, GL_COLOR_INDEXES, OG_STATE_CHECK_NULL}, "BACK",
        "COLOR_INDEXES", getTargetParameterCheck, STATEDATA_PREDEFINED,
        3, (void (*) (void)) glGetMaterialfv, {0, 1, 1}
    },
    {
	{GL_BACK, GL_DIFFUSE, OG_STATE_CHECK_NULL}, "BACK", "DIFFUSE",
	getTargetParameterCheck, STATEDATA_PREDEFINED, 4,
        (void (*) (void)) glGetMaterialfv, {0.8, 0.8, 0.8, 1}
    },
    {
	{GL_BACK, GL_EMISSION, OG_STATE_CHECK_NULL}, "BACK", "EMISSION",
        getTargetParameterCheck, STATEDATA_PREDEFINED,
        4, (void (*) (void)) glGetMaterialfv, {0, 0, 0, 1}
    },
    {
	{GL_BACK, GL_SHININESS, OG_STATE_CHECK_NULL}, "BACK", "SHININESS",
        getTargetParameterCheck, STATEDATA_PREDEFINED, 1,
        (void (*) (void)) glGetMaterialfv, {0}},
    {
	{GL_BACK, GL_SPECULAR, OG_STATE_CHECK_NULL}, "BACK", "SPECULAR",
	getTargetParameterCheck, STATEDATA_PREDEFINED,
        4, (void (*) (void)) glGetMaterialfv, {0, 0, 0, 1}
    },
    {
	{GL_FRONT, GL_AMBIENT, OG_STATE_CHECK_NULL}, "FRONT", "AMBIENT",
	getTargetParameterCheck, STATEDATA_PREDEFINED,
        4, (void (*) (void)) glGetMaterialfv, {0.2, 0.2, 0.2, 1}
    },
    {
	{GL_FRONT, GL_COLOR_INDEXES, OG_STATE_CHECK_NULL}, "FRONT",
        "COLOR_INDEXES", getTargetParameterCheck, STATEDATA_PREDEFINED,
        3, (void (*) (void)) glGetMaterialfv, {0, 1, 1}
    },
    {
	{GL_FRONT, GL_DIFFUSE, OG_STATE_CHECK_NULL}, "FRONT", "DIFFUSE",
	getTargetParameterCheck, STATEDATA_PREDEFINED,
        4, (void (*) (void)) glGetMaterialfv, {0.8, 0.8, 0.8, 1}
    },
    {
	{GL_FRONT, GL_EMISSION, OG_STATE_CHECK_NULL}, "FRONT", "EMISSION",
	getTargetParameterCheck, STATEDATA_PREDEFINED,
        4, (void (*) (void)) glGetMaterialfv, {0, 0, 0, 1}
    },
    {
	{GL_FRONT, GL_SHININESS, OG_STATE_CHECK_NULL}, "FRONT", "SHININESS",
	getTargetParameterCheck, STATEDATA_PREDEFINED, 1,
        (void (*) (void)) glGetMaterialfv, {0}},
    {
	{GL_FRONT, GL_SPECULAR, OG_STATE_CHECK_NULL}, "FRONT", "SPECULAR",
	getTargetParameterCheck, STATEDATA_PREDEFINED,
        4, (void (*) (void)) glGetMaterialfv, {0, 0, 0, 1}
    },
    {
	{GL_PIXEL_MAP_I_TO_A, GL_PIXEL_MAP_I_TO_A_SIZE, OG_STATE_CHECK_NULL},
        "PIXEL_MAP_I_TO_A", "SIZE",
	checkPixelMap, STATEDATA_PREDEFINED, 2, NULL, {0, 1}
    },
    {
	{GL_PIXEL_MAP_I_TO_B, GL_PIXEL_MAP_I_TO_B_SIZE, OG_STATE_CHECK_NULL},
	"PIXEL_MAP_I_TO_B", "SIZE",
	checkPixelMap, STATEDATA_PREDEFINED, 2, NULL, {0, 1}
    },
    {
	{GL_PIXEL_MAP_I_TO_G, GL_PIXEL_MAP_I_TO_G_SIZE, OG_STATE_CHECK_NULL},
	"PIXEL_MAP_I_TO_G", "SIZE",
	checkPixelMap, STATEDATA_PREDEFINED, 2, NULL, {0, 1}
    },
    {
	{GL_PIXEL_MAP_I_TO_I, GL_PIXEL_MAP_I_TO_I_SIZE, OG_STATE_CHECK_NULL},
	"PIXEL_MAP_I_TO_I", "SIZE",
	checkPixelMap, STATEDATA_PREDEFINED, 2, NULL, {0, 1}
    },
    {
	{GL_PIXEL_MAP_I_TO_R, GL_PIXEL_MAP_I_TO_R_SIZE, OG_STATE_CHECK_NULL},
	"PIXEL_MAP_I_TO_R", "SIZE",
	checkPixelMap, STATEDATA_PREDEFINED, 2, NULL, {0, 1}
    },
    {
	{GL_PIXEL_MAP_A_TO_A, GL_PIXEL_MAP_A_TO_A_SIZE, OG_STATE_CHECK_NULL},
	"PIXEL_MAP_A_TO_A", "SIZE",
	checkPixelMap, STATEDATA_PREDEFINED, 2, NULL, {0, 1}
    },
    {
	{GL_PIXEL_MAP_B_TO_B, GL_PIXEL_MAP_B_TO_B_SIZE, OG_STATE_CHECK_NULL},
	"PIXEL_MAP_B_TO_B", "SIZE",
	checkPixelMap, STATEDATA_PREDEFINED, 2, NULL, {0, 1}
    },
    {
	{GL_PIXEL_MAP_G_TO_G, GL_PIXEL_MAP_G_TO_G_SIZE, OG_STATE_CHECK_NULL},
	"PIXEL_MAP_G_TO_G", "SIZE",
	checkPixelMap, STATEDATA_PREDEFINED, 2, NULL, {0, 1}
    },
    {
	{GL_PIXEL_MAP_R_TO_R, GL_PIXEL_MAP_R_TO_R_SIZE, OG_STATE_CHECK_NULL},
	"PIXEL_MAP_R_TO_R", "SIZE",
	checkPixelMap, STATEDATA_PREDEFINED, 2, NULL, {0, 1}
    },
    {
	{GL_PIXEL_MAP_S_TO_S, GL_PIXEL_MAP_S_TO_S_SIZE, OG_STATE_CHECK_NULL},
	"PIXEL_MAP_S_TO_S", "SIZE",
	checkPixelMap, STATEDATA_PREDEFINED, 2, NULL, {0, 1}
    },
    {
	{GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, OG_STATE_CHECK_NULL},
        "TEXTURE_ENV", "COLOR", getTargetParameterCheck,
        STATEDATA_PREDEFINED, 4, (void (*) (void)) glGetTexEnvfv, {0, 0, 0, 0}
    },
    {
	{GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, OG_STATE_CHECK_NULL},
        "TEXTURE_ENV", "MODE", getTargetParameterCheck,
        STATEDATA_PREDEFINED, 1, (void (*) (void)) glGetTexEnvfv, {GL_MODULATE}
    },
    {
	{GL_Q, GL_EYE_PLANE, OG_STATE_CHECK_NULL}, "EYE_PLANE", "Q",
	getTargetParameterCheck, STATEDATA_PREDEFINED,
        4, (void (*) (void)) glGetTexGenfv, {0, 0, 0, 0}
    },
    {
	{GL_Q, GL_OBJECT_PLANE, OG_STATE_CHECK_NULL}, "OBJECT_PLANE", "Q",
	getTargetParameterCheck, STATEDATA_PREDEFINED,
        4, (void (*) (void)) glGetTexGenfv, {0, 0, 0, 0}
    },
    {
	{GL_Q, GL_TEXTURE_GEN_MODE, OG_STATE_CHECK_NULL}, "TEXTURE_GEN_MODE",
        "Q", getTargetParameterCheck, STATEDATA_PREDEFINED,
        1, (void (*) (void)) glGetTexGenfv, {GL_EYE_LINEAR}
    },
    {
	{GL_R, GL_EYE_PLANE, OG_STATE_CHECK_NULL}, "EYE_PLANE", "R",
	getTargetParameterCheck, STATEDATA_PREDEFINED,
        4, (void (*) (void)) glGetTexGenfv, {0, 0, 0, 0}
    },
    {
	{GL_R, GL_OBJECT_PLANE, OG_STATE_CHECK_NULL}, "OBJECT_PLANE", "R",
	getTargetParameterCheck, STATEDATA_PREDEFINED,
        4, (void (*) (void)) glGetTexGenfv, {0, 0, 0, 0}
    },
    {
	{GL_R, GL_TEXTURE_GEN_MODE, OG_STATE_CHECK_NULL}, "TEXTURE_GEN_MODE",
        "R",
	getTargetParameterCheck, STATEDATA_PREDEFINED,
        1, (void (*) (void)) glGetTexGenfv, {GL_EYE_LINEAR}
    },
    {
	{GL_S, GL_EYE_PLANE, OG_STATE_CHECK_NULL}, "EYE_PLANE", "S",
	getTargetParameterCheck, STATEDATA_PREDEFINED,
        4, (void (*) (void)) glGetTexGenfv, {1, 0, 0, 0}
    },
    {
	{GL_S, GL_OBJECT_PLANE, OG_STATE_CHECK_NULL}, "OBJECT_PLANE", "S",
	getTargetParameterCheck, STATEDATA_PREDEFINED,
        4, (void (*) (void)) glGetTexGenfv, {1, 0, 0, 0}
    },
    {
	{GL_S, GL_TEXTURE_GEN_MODE, OG_STATE_CHECK_NULL}, "TEXTURE_GEN_MODE",
        "S", getTargetParameterCheck, STATEDATA_PREDEFINED,
        1, (void (*) (void)) glGetTexGenfv, {GL_EYE_LINEAR}
    },
    {
	{GL_T, GL_EYE_PLANE, OG_STATE_CHECK_NULL}, "TEXTURE_EYE_PLANE", "T",
	getTargetParameterCheck, STATEDATA_PREDEFINED,
        4, (void (*) (void)) glGetTexGenfv, {0, 1, 0, 0}
    },
    {
	{GL_T, GL_OBJECT_PLANE, OG_STATE_CHECK_NULL}, "OBJECT_PLANE", "T",
	getTargetParameterCheck, STATEDATA_PREDEFINED,
        4, (void (*) (void)) glGetTexGenfv, {0, 1, 0, 0}
    },
    {
	{GL_T, GL_TEXTURE_GEN_MODE, OG_STATE_CHECK_NULL}, "TEXTURE_GEN_MODE",
        "T", getTargetParameterCheck, STATEDATA_PREDEFINED,
        1, (void (*) (void)) glGetTexGenfv, {GL_EYE_LINEAR}
    },
    {
	{GL_TEXTURE_1D, GL_TEXTURE_BORDER_COLOR, OG_STATE_CHECK_NULL},
        "TEXTURE_1D", "TEXTURE_BORDER_COLOR",
	checkTexParam, STATEDATA_PREDEFINED, 4, NULL, {0, 0, 0, 0}
    },
    {
	{GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, OG_STATE_CHECK_NULL},
        "TEXTURE_1D", "TEXTURE_MAG_FILTER",
	checkTexParam, STATEDATA_PREDEFINED, 1, NULL, {GL_LINEAR}
    },
    {
	{GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, OG_STATE_CHECK_NULL},
        "TEXTURE_1D", "TEXTURE_MIN_FILTER",
	checkTexParam, STATEDATA_PREDEFINED, 1, NULL, {GL_NEAREST_MIPMAP_LINEAR}
    },
    {
	{GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, OG_STATE_CHECK_NULL}, "TEXTURE_1D",
	"TEXTURE_WRAP_S",
	checkTexParam, STATEDATA_PREDEFINED, 1, NULL, {GL_REPEAT}
    },
    {
	{GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, OG_STATE_CHECK_NULL}, "TEXTURE_1D",
	"TEXTURE_WRAP_T",
	checkTexParam, STATEDATA_PREDEFINED, 1, NULL, {GL_REPEAT}
    },
    {
	{GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, OG_STATE_CHECK_NULL},
        "TEXTURE_2D", "TEXTURE_BORDER_COLOR",
	checkTexParam, STATEDATA_PREDEFINED, 4, NULL, {0, 0, 0, 0}
    },
    {
	{GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, OG_STATE_CHECK_NULL},
        "TEXTURE_2D", "TEXTURE_MAG_FILTER",
	checkTexParam, STATEDATA_PREDEFINED, 1, NULL, {GL_LINEAR}
    },
    {
	{GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, OG_STATE_CHECK_NULL},
        "TEXTURE_2D", "TEXTURE_MIN_FILTER",
	checkTexParam, STATEDATA_PREDEFINED, 1, NULL, {GL_NEAREST_MIPMAP_LINEAR}
    },
    {
	{GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, OG_STATE_CHECK_NULL},
        "TEXTURE_2D", "TEXTURE_WRAP_S",
	checkTexParam, STATEDATA_PREDEFINED, 1, NULL, {GL_REPEAT}
    },
    {
	{GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, OG_STATE_CHECK_NULL}, "TEXTURE_2D",
	"TEXTURE_WRAP_T",
	checkTexParam, STATEDATA_PREDEFINED, 1, NULL, {GL_REPEAT}
    },
    {
	{GL_POLYGON_STIPPLE, OG_STATE_CHECK_NULL}, "POLYGON_STIPPLE",
	NULL, checkPolygonStipple, STATEDATA_PREDEFINED, 128, NULL,
        {255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
         255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
         255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
         255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
         255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
         255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
         255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
         255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
         255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
         255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255}
    },
    {
	{GL_MAX_TEXTURE_SIZE, GL_TEXTURE_1D, OG_STATE_CHECK_NULL},
        "TEXTURE_1D", "MAX_TEXTURE_SIZE",
        checkTexLevelParams, STATEDATA_ONTHEFLY, 1, NULL
    },
    {
	{GL_MAX_TEXTURE_SIZE, GL_TEXTURE_2D, OG_STATE_CHECK_NULL},
        "TEXTURE_2D", "MAX_TEXTURE_SIZE",
        checkTexLevelParams, STATEDATA_ONTHEFLY, 1, NULL
    },
    {
	{OG_STATE_CHECK_NULL}
    }
};

#ifdef GL_VERSION_1_1
static void nullPointerCheck(stateRec *ptr);

/* OpenGL 1.1 state */
static stateRec state1[] = {
    {
	{GL_INDEX_LOGIC_OP, OG_STATE_CHECK_NULL}, "INDEX_LOGIC_OP", NULL, checkEnabled,
	STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_COLOR_LOGIC_OP, OG_STATE_CHECK_NULL}, "COLOR_LOGIC_OP", NULL, checkEnabled,
	STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_POLYGON_OFFSET_POINT, OG_STATE_CHECK_NULL}, "POLYGON_OFFSET_POINT", NULL, checkEnabled,
	STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_POLYGON_OFFSET_LINE, OG_STATE_CHECK_NULL}, "POLYGON_OFFSET_LINE", NULL, checkEnabled,
	STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_POLYGON_OFFSET_FILL, OG_STATE_CHECK_NULL}, "POLYGON_OFFSET_FILL", NULL, checkEnabled,
	STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_POLYGON_OFFSET_FACTOR, OG_STATE_CHECK_NULL}, "POLYGON_OFFSET_FACTOR", NULL, simpleGetCheck,
	STATEDATA_PREDEFINED, 1, NULL, {0}
    },
    {
	{GL_POLYGON_OFFSET_UNITS, OG_STATE_CHECK_NULL}, "POLYGON_OFFSET_UNITS", NULL, simpleGetCheck,
	STATEDATA_PREDEFINED, 1, NULL, {0}
    },
    {
	{GL_VERTEX_ARRAY, OG_STATE_CHECK_NULL}, "VERTEX_ARRAY", NULL, checkEnabled,
	STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_VERTEX_ARRAY_SIZE, OG_STATE_CHECK_NULL}, "VERTEX_ARRAY_SIZE", NULL, simpleGetCheck,
	STATEDATA_PREDEFINED, 1, NULL, {4}
    },
    {
	{GL_VERTEX_ARRAY_TYPE, OG_STATE_CHECK_NULL}, "VERTEX_ARRAY_TYPE", NULL, simpleGetCheck,
	STATEDATA_PREDEFINED, 1, NULL, {GL_FLOAT}
    },
    {
	{GL_VERTEX_ARRAY_STRIDE, OG_STATE_CHECK_NULL}, "VERTEX_ARRAY_TYPE_STRIDE", NULL, simpleGetCheck,
	STATEDATA_PREDEFINED, 1, NULL, {0}
    },
    {
	{GL_VERTEX_ARRAY_POINTER, OG_STATE_CHECK_NULL}, "VERTEX_ARRAY_POINTER", NULL, nullPointerCheck,
	STATEDATA_PREDEFINED, 1, NULL, {0}
    },
    {
	{GL_NORMAL_ARRAY, OG_STATE_CHECK_NULL}, "NORMAL_ARRAY", NULL, checkEnabled,
	STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_NORMAL_ARRAY_TYPE, OG_STATE_CHECK_NULL}, "NORMAL_ARRAY_TYPE", NULL, simpleGetCheck,
	STATEDATA_PREDEFINED, 1, NULL, {GL_FLOAT}
    },
    {
	{GL_NORMAL_ARRAY_STRIDE, OG_STATE_CHECK_NULL}, "NORMAL_ARRAY_STRIDE", NULL, simpleGetCheck,
	STATEDATA_PREDEFINED, 1, NULL, {0}
    },
    {
	{GL_NORMAL_ARRAY_POINTER, OG_STATE_CHECK_NULL}, "NORMAL_ARRAY_POINTER", NULL, nullPointerCheck,
	STATEDATA_PREDEFINED, 1, NULL, {0}
    },
    {
	{GL_COLOR_ARRAY, OG_STATE_CHECK_NULL}, "COLOR_ARRAY", NULL, checkEnabled,
	STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_COLOR_ARRAY_SIZE, OG_STATE_CHECK_NULL}, "COLOR_ARRAY_SIZE", NULL, simpleGetCheck,
	STATEDATA_PREDEFINED, 1, NULL, {4}
    },
    {
	{GL_COLOR_ARRAY_TYPE, OG_STATE_CHECK_NULL}, "COLOR_ARRAY_TYPE", NULL, simpleGetCheck,
	STATEDATA_PREDEFINED, 1, NULL, {GL_FLOAT}
    },
    {
	{GL_COLOR_ARRAY_STRIDE, OG_STATE_CHECK_NULL}, "COLOR_ARRAY_STRIDE", NULL, simpleGetCheck,
	STATEDATA_PREDEFINED, 1, NULL, {0}
    },
    {
	{GL_COLOR_ARRAY_POINTER, OG_STATE_CHECK_NULL}, "COLOR_ARRAY_POINTER", NULL, nullPointerCheck,
	STATEDATA_PREDEFINED, 1, NULL, {0}
    },
    {
	{GL_INDEX_ARRAY, OG_STATE_CHECK_NULL}, "INDEX_ARRAY", NULL, checkEnabled,
	STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_INDEX_ARRAY_TYPE, OG_STATE_CHECK_NULL}, "INDEX_ARRAY_TYPE", NULL, simpleGetCheck,
	STATEDATA_PREDEFINED, 1, NULL, {GL_FLOAT}
    },
    {
	{GL_INDEX_ARRAY_STRIDE, OG_STATE_CHECK_NULL}, "INDEX_ARRAY_STRIDE", NULL, simpleGetCheck,
	STATEDATA_PREDEFINED, 1, NULL, {0}
    },
    {
	{GL_INDEX_ARRAY_POINTER, OG_STATE_CHECK_NULL}, "INDEX_ARRAY_POINTER", NULL, nullPointerCheck,
	STATEDATA_PREDEFINED, 1, NULL, {0}
    },
    {
	{GL_TEXTURE_COORD_ARRAY, OG_STATE_CHECK_NULL}, "TEXTURE_COORD_ARRAY", NULL, checkEnabled,
	STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_TEXTURE_COORD_ARRAY_SIZE, OG_STATE_CHECK_NULL}, "TEXTURE_COORD_ARRAY_SIZE", NULL, simpleGetCheck,
	STATEDATA_PREDEFINED, 1, NULL, {4}
    },
    {
	{GL_TEXTURE_COORD_ARRAY_TYPE, OG_STATE_CHECK_NULL}, "TEXTURE_COORD_ARRAY_TYPE", NULL, simpleGetCheck,
	STATEDATA_PREDEFINED, 1, NULL, {GL_FLOAT}
    },
    {
	{GL_TEXTURE_COORD_ARRAY_STRIDE, OG_STATE_CHECK_NULL}, "TEXTURE_COORD_ARRAY_STRIDE", NULL, simpleGetCheck,
	STATEDATA_PREDEFINED, 1, NULL, {0}
    },
    {
	{GL_TEXTURE_COORD_ARRAY_POINTER, OG_STATE_CHECK_NULL}, "TEXTURE_COORD_ARRAY_POINTER", NULL, nullPointerCheck,
	STATEDATA_PREDEFINED, 1, NULL, {0}
    },
    {
	{GL_EDGE_FLAG_ARRAY, OG_STATE_CHECK_NULL}, "EDGE_FLAG_ARRAY", NULL, checkEnabled,
	STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}
    },
    {
	{GL_EDGE_FLAG_ARRAY_STRIDE, OG_STATE_CHECK_NULL}, "EDGE_FLAG_ARRAY_STRIDE", NULL, simpleGetCheck,
	STATEDATA_PREDEFINED, 1, NULL, {0}
    },
    {
	{GL_EDGE_FLAG_ARRAY_POINTER, OG_STATE_CHECK_NULL}, "EDGE_FLAG_ARRAY_POINTER", NULL, nullPointerCheck,
	STATEDATA_PREDEFINED, 1, NULL, {0}
    },

    {
	{GL_CLIENT_ATTRIB_STACK_DEPTH, OG_STATE_CHECK_NULL}, "CLIENT_ATTRIB_STACK_DEPTH", NULL,
	simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {0}
    },
    {
	{GL_MAX_CLIENT_ATTRIB_STACK_DEPTH, OG_STATE_CHECK_NULL}, "MAX_CLIENT_ATTRIB_STACK_DEPTH", NULL,
	simpleGetCheck, STATEDATA_STATE_GET_TARGET, 1, NULL, {16},
    },
    {
	{GL_FEEDBACK_BUFFER_SIZE, OG_STATE_CHECK_NULL}, "FEEDBACK_BUFFER_SIZE", NULL,
	simpleGetCheck, STATEDATA_STATE_GET_TARGET, 1, NULL, {0},
    },
    {
	{GL_FEEDBACK_BUFFER_TYPE, OG_STATE_CHECK_NULL}, "FEEDBACK_BUFFER_TYPE", NULL,
	simpleGetCheck, STATEDATA_STATE_GET_TARGET, 1, NULL, {GL_2D},
    },
    {
	{GL_FEEDBACK_BUFFER_POINTER, OG_STATE_CHECK_NULL}, "FEEDBACK_BUFFER_POINTER", NULL,
	nullPointerCheck, STATEDATA_STATE_GET_TARGET, 1, NULL, {0},
    },
    {
	{GL_SELECTION_BUFFER_SIZE, OG_STATE_CHECK_NULL}, "SELECTION_BUFFER_SIZE", NULL,
	simpleGetCheck, STATEDATA_STATE_GET_TARGET, 1, NULL, {0},
    },
    {
	{GL_SELECTION_BUFFER_POINTER, OG_STATE_CHECK_NULL}, "SELECTION_BUFFER_POINTER", NULL,
	nullPointerCheck, STATEDATA_STATE_GET_TARGET, 1, NULL, {0},
    },
    {
	{GL_TEXTURE_BINDING_1D, OG_STATE_CHECK_NULL}, "TEXTURE_BINDING_1D", NULL,
	simpleGetCheck, STATEDATA_STATE_GET_TARGET, 1, NULL, {0},
    },
    {
	{GL_TEXTURE_BINDING_2D, OG_STATE_CHECK_NULL}, "TEXTURE_BINDING_2D", NULL,
	simpleGetCheck, STATEDATA_STATE_GET_TARGET, 1, NULL, {0},
    },
    {
	{GL_TEXTURE_1D, GL_TEXTURE_PRIORITY, OG_STATE_CHECK_NULL}, "TEXTURE_1D", "PRIORITY",
         checkTexParam, STATEDATA_PREDEFINED, 1, NULL, {1},
    },
    {
	{GL_TEXTURE_2D, GL_TEXTURE_PRIORITY, OG_STATE_CHECK_NULL}, "TEXTURE_2D", "PRIORITY",
         checkTexParam, STATEDATA_PREDEFINED, 1, NULL, {1},
    },
    {
	{OG_STATE_CHECK_NULL}
    }
};
#endif

/*
** Separate state array for OpenGL extensions. They must be #ifdef'ed to
** ensure that older systems will be able to compile ogtst
*/
static stateRecEXT stateEXT[] = {
#if defined(GL_EXT_blend_logic_op) || defined(GL_EXT_blend_minmax) || defined(GL_EXT_blend_subtract)
    {
        {{GL_BLEND_EQUATION_EXT, OG_STATE_CHECK_NULL}, "BLEND_EQUATION",
         NULL, simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {GL_FUNC_ADD_EXT},},
        "GL_EXT_blend_minmax" /* not exactly one-to-one */
    },
#endif
#ifdef GL_EXT_blend_color
    {
        {{GL_BLEND_COLOR_EXT, OG_STATE_CHECK_NULL}, "BLEND_COLOR",
         NULL, simpleGetCheck, STATEDATA_PREDEFINED, 4, NULL, {0, 0, 0, 0},},
        "GL_EXT_blend_color"
    },
#endif
#ifdef GL_SGI_color_matrix
    {
        {{GL_COLOR_MATRIX_SGI, OG_STATE_CHECK_NULL}, "COLOR_MATRIX",
         NULL, simpleGetCheck, STATEDATA_PREDEFINED, 16, NULL,
         {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1},},
        "GL_SGI_color_matrix"
    },
    {
        {{GL_COLOR_MATRIX_STACK_DEPTH_SGI, OG_STATE_CHECK_NULL},
         "COLOR_MATRIX_STACK_DEPTH", NULL,
         simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {1},},
        "GL_SGI_color_matrix"
    },
    {
        {{GL_POST_COLOR_MATRIX_RED_SCALE_SGI, OG_STATE_CHECK_NULL},
         "POST_COLOR_MATRIX_RED_SCALE", NULL,
         simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {1},},
        "GL_SGI_color_matrix"
    },
    {
        {{GL_POST_COLOR_MATRIX_RED_BIAS_SGI, OG_STATE_CHECK_NULL},
         "POST_COLOR_MATRIX_RED_BIAS", NULL,
         simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {0},},
        "GL_SGI_color_matrix"
    },
    {
        {{GL_POST_COLOR_MATRIX_GREEN_SCALE_SGI, OG_STATE_CHECK_NULL},
         "POST_COLOR_MATRIX_GREEN_SCALE", NULL,
         simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {1},},
        "GL_SGI_color_matrix"
    },
    {
        {{GL_POST_COLOR_MATRIX_GREEN_BIAS_SGI, OG_STATE_CHECK_NULL},
         "POST_COLOR_MATRIX_GREEN_BIAS", NULL,
         simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {0},},
        "GL_SGI_color_matrix"
    },
    {
        {{GL_POST_COLOR_MATRIX_BLUE_SCALE_SGI, OG_STATE_CHECK_NULL},
         "POST_COLOR_MATRIX_BLUE_SCALE", NULL,
         simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {1},},
        "GL_SGI_color_matrix"
    },
    {
        {{GL_POST_COLOR_MATRIX_BLUE_BIAS_SGI, OG_STATE_CHECK_NULL},
         "POST_COLOR_MATRIX_BLUE_BIAS", NULL,
         simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {0},},
        "GL_SGI_color_matrix"
    },
    {
        {{GL_POST_COLOR_MATRIX_ALPHA_SCALE_SGI, OG_STATE_CHECK_NULL},
         "POST_COLOR_MATRIX_ALPHA_SCALE", NULL,
         simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {1},},
        "GL_SGI_color_matrix"
    },
    {
        {{GL_POST_COLOR_MATRIX_ALPHA_BIAS_SGI, OG_STATE_CHECK_NULL},
         "POST_COLOR_MATRIX_ALPHA_BIAS", NULL,
         simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {0},},
        "GL_SGI_color_matrix"
    },
    {
        {{GL_MAX_COLOR_MATRIX_STACK_DEPTH_SGI, OG_STATE_CHECK_NULL},
         "MAX_COLOR_MATRIX_STACK_DEPTH", NULL,
         simpleGetCheck, STATEDATA_STATE_GET_TARGET, 1, NULL, {2},},
        "GL_SGI_color_matrix"
    },
#endif
#ifdef GL_EXT_convolution
    {
	{{GL_CONVOLUTION_1D_EXT, OG_STATE_CHECK_NULL},
         "CONVOLUTION_1D", NULL, checkEnabled, STATEDATA_PREDEFINED,
         1, NULL, {GL_FALSE},},
        "GL_EXT_convolution"
    },
    {
	{{GL_CONVOLUTION_1D_EXT, GL_CONVOLUTION_FILTER_SCALE_EXT,
          OG_STATE_CHECK_NULL}, "CONVOLUTION_1D", "FILTER_SCALE",
         getTargetParameterCheck, STATEDATA_PREDEFINED, 4,
         (void (*) (void)) glGetConvolutionParameterfvEXT,  {1, 1, 1, 1},},
	"GL_EXT_convolution"
    },
    {
	{{GL_CONVOLUTION_1D_EXT, GL_CONVOLUTION_FILTER_BIAS_EXT,
          OG_STATE_CHECK_NULL}, "CONVOLUTION_1D", "FILTER_BIAS",
         getTargetParameterCheck, STATEDATA_PREDEFINED, 4,
         (void (*) (void)) glGetConvolutionParameterfvEXT, {0, 0, 0, 0},},
        "GL_EXT_convolution"
    },
    {
	{{GL_CONVOLUTION_1D_EXT, GL_CONVOLUTION_FORMAT_EXT,
          OG_STATE_CHECK_NULL},
         "CONVOLUTION_1D", "FILTER_FORMAT",
         getTargetParameterCheck, STATEDATA_PREDEFINED, 1,
         (void (*) (void)) glGetConvolutionParameterfvEXT,  {GL_RGBA},},
        "GL_EXT_convolution"
    },
    {
	{{GL_CONVOLUTION_1D_EXT, GL_CONVOLUTION_WIDTH_EXT,
          OG_STATE_CHECK_NULL}, "CONVOLUTION_1D", "WIDTH",
         getTargetParameterCheck, STATEDATA_PREDEFINED, 1,
         (void (*) (void)) glGetConvolutionParameterfvEXT, {0},},
        "GL_EXT_convolution"
    },
    {
	{{GL_CONVOLUTION_1D_EXT, GL_CONVOLUTION_BORDER_MODE_EXT,
          OG_STATE_CHECK_NULL}, "CONVOLUTION_1D", "BORDER_MODE",
         getTargetParameterCheck, STATEDATA_PREDEFINED, 1,
         (void (*) (void)) glGetConvolutionParameterfvEXT, {GL_REDUCE_EXT},},
	"GL_EXT_convolution"
    },
    {
	{{GL_CONVOLUTION_2D_EXT, OG_STATE_CHECK_NULL}, "CONVOLUTION_2D",
         NULL, checkEnabled, STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE},},
	"GL_EXT_convolution"
    },
    {
	{{GL_SEPARABLE_2D_EXT, OG_STATE_CHECK_NULL}, "SEPARABLE_2D",
         NULL, checkEnabled, STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE},},
	"GL_EXT_convolution"
    },
    {
	{{GL_CONVOLUTION_2D_EXT, GL_CONVOLUTION_FILTER_SCALE_EXT,
          OG_STATE_CHECK_NULL}, "CONVOLUTION_2D", "FILTER_SCALE",
         getTargetParameterCheck, STATEDATA_PREDEFINED, 4,
         (void (*) (void)) glGetConvolutionParameterfvEXT, {1, 1, 1, 1},},
	"GL_EXT_convolution"
    },
    {
	{{GL_CONVOLUTION_2D_EXT, GL_CONVOLUTION_FILTER_BIAS_EXT,
          OG_STATE_CHECK_NULL}, "CONVOLUTION_2D", "FILTER_BIAS",
         getTargetParameterCheck, STATEDATA_PREDEFINED, 4,
         (void (*) (void)) glGetConvolutionParameterfvEXT, {0, 0, 0, 0},},
        "GL_EXT_convolution"
    },
    {
	{{GL_CONVOLUTION_2D_EXT, GL_CONVOLUTION_FORMAT_EXT,
          OG_STATE_CHECK_NULL}, "CONVOLUTION_2D", "FILTER_FORMAT",
         getTargetParameterCheck, STATEDATA_PREDEFINED, 1,
         (void (*) (void)) glGetConvolutionParameterfvEXT, {GL_RGBA},},
        "GL_EXT_convolution"
    },
    {
	{{GL_CONVOLUTION_2D_EXT, GL_CONVOLUTION_WIDTH_EXT,
          OG_STATE_CHECK_NULL}, "CONVOLUTION_2D", "WIDTH",
         getTargetParameterCheck, STATEDATA_PREDEFINED, 1,
         (void (*) (void)) glGetConvolutionParameterfvEXT, {0},},
        "GL_EXT_convolution"
    },
    {
	{{GL_CONVOLUTION_2D_EXT, GL_CONVOLUTION_HEIGHT_EXT,
          OG_STATE_CHECK_NULL}, "CONVOLUTION_2D", "HEIGHT",
         getTargetParameterCheck, STATEDATA_PREDEFINED, 1,
         (void (*) (void)) glGetConvolutionParameterfvEXT, {0},},
	"GL_EXT_convolution"
    },
    {
	{{GL_SEPARABLE_2D_EXT, GL_CONVOLUTION_FORMAT_EXT,
          OG_STATE_CHECK_NULL}, "SEPARABLE_2D", "FILTER_FORMAT",
         getTargetParameterCheck, STATEDATA_PREDEFINED, 1,
         (void (*) (void)) glGetConvolutionParameterfvEXT, {GL_RGBA},},
	"GL_EXT_convolution"
    },
    {
	{{GL_SEPARABLE_2D_EXT, GL_CONVOLUTION_WIDTH_EXT,
          OG_STATE_CHECK_NULL}, "SEPARABLE_2D", "WIDTH",
         getTargetParameterCheck, STATEDATA_PREDEFINED, 1,
         (void (*) (void)) glGetConvolutionParameterfvEXT, {0},},
	"GL_EXT_convolution"
    },
    {
	{{GL_SEPARABLE_2D_EXT, GL_CONVOLUTION_HEIGHT_EXT, OG_STATE_CHECK_NULL}, 
         "SEPARABLE_2D", "HEIGHT",
         getTargetParameterCheck, STATEDATA_PREDEFINED, 1,
         (void (*) (void)) glGetConvolutionParameterfvEXT, {0},},
	"GL_EXT_convolution"
    },
    {
	{{GL_CONVOLUTION_2D_EXT, GL_CONVOLUTION_BORDER_MODE_EXT,
          OG_STATE_CHECK_NULL}, "CONVOLUTION_2D", "BORDER_MODE",
         getTargetParameterCheck, STATEDATA_PREDEFINED, 1,
         (void (*) (void)) glGetConvolutionParameterfvEXT, {GL_REDUCE_EXT},},
	"GL_EXT_convolution"
    },
    {
	{{GL_POST_CONVOLUTION_RED_SCALE_EXT, OG_STATE_CHECK_NULL},
         "POST_CONVOLUTION_RED_SCALE", NULL,
         simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {1},},
	"GL_EXT_convolution"
    },
    {
	{{GL_POST_CONVOLUTION_RED_BIAS_EXT, OG_STATE_CHECK_NULL},
         "POST_CONVOLUTION_RED_SCALE", NULL,
         simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {0},},
	"GL_EXT_convolution"
    },
    {
	{{GL_POST_CONVOLUTION_GREEN_SCALE_EXT, OG_STATE_CHECK_NULL},
         "POST_CONVOLUTION_GREEN_SCALE", NULL,
         simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {1},},
	"GL_EXT_convolution"
    },
    {
	{{GL_POST_CONVOLUTION_GREEN_BIAS_EXT, OG_STATE_CHECK_NULL},
         "POST_CONVOLUTION_GREEN_SCALE", NULL,
         simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {0},},
	"GL_EXT_convolution"
    },
    {
	{{GL_POST_CONVOLUTION_BLUE_SCALE_EXT, OG_STATE_CHECK_NULL},
         "POST_CONVOLUTION_BLUE_SCALE", NULL,
         simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {1},},
	"GL_EXT_convolution"
    },
    {
	{{GL_POST_CONVOLUTION_BLUE_BIAS_EXT, OG_STATE_CHECK_NULL},
         "POST_CONVOLUTION_BLUE_SCALE", NULL,
         simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {0},},
	"GL_EXT_convolution"
    },
    {
	{{GL_POST_CONVOLUTION_ALPHA_SCALE_EXT, OG_STATE_CHECK_NULL},
         "POST_CONVOLUTION_ALPHA_SCALE", NULL,
         simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {1},},
	"GL_EXT_convolution"
    },
    {
	{{GL_POST_CONVOLUTION_ALPHA_BIAS_EXT, OG_STATE_CHECK_NULL},
         "POST_CONVOLUTION_ALPHA_SCALE", NULL,
         simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {0},},
	"GL_EXT_convolution"
    },
    {
	{{GL_CONVOLUTION_1D_EXT, GL_MAX_CONVOLUTION_WIDTH_EXT,
          OG_STATE_CHECK_NULL}, "CONVOLUTION_1D", "MAX_WIDTH",
         getTargetParameterCheck, STATEDATA_STATE_GET_TARGET, 1,
         (void (*) (void)) glGetConvolutionParameterfvEXT, {3},},
	"GL_EXT_convolution"
    },
    {
	{{GL_CONVOLUTION_2D_EXT, GL_MAX_CONVOLUTION_WIDTH_EXT,
          OG_STATE_CHECK_NULL}, "CONVOLUTION_2D", "MAX_WIDTH",
         getTargetParameterCheck, STATEDATA_STATE_GET_TARGET, 1,
         (void (*) (void)) glGetConvolutionParameterfvEXT, {3},},
	"GL_EXT_convolution"
    },
    {
	{{GL_CONVOLUTION_2D_EXT, GL_MAX_CONVOLUTION_HEIGHT_EXT,
          OG_STATE_CHECK_NULL}, "CONVOLUTION_2D", "MAX_HEIGHT",
         getTargetParameterCheck, STATEDATA_STATE_GET_TARGET, 1,
         (void (*) (void)) glGetConvolutionParameterfvEXT, {3},},
	"GL_EXT_convolution"
    },
    {
	{{GL_SEPARABLE_2D_EXT, GL_MAX_CONVOLUTION_HEIGHT_EXT,
          OG_STATE_CHECK_NULL}, "SEPARABLE_2D", "MAX_WIDTH",
         getTargetParameterCheck, STATEDATA_STATE_GET_TARGET, 1,
         (void (*) (void)) glGetConvolutionParameterfvEXT, {3},},
	"GL_EXT_convolution"
    },
    {
	{{GL_SEPARABLE_2D_EXT, GL_MAX_CONVOLUTION_WIDTH_EXT,
          OG_STATE_CHECK_NULL}, "SEPARABLE_2D", "MAX_HEIGHT",
         getTargetParameterCheck, STATEDATA_STATE_GET_TARGET, 1,
         (void (*) (void)) glGetConvolutionParameterfvEXT, {3},},
	"GL_EXT_convolution"
    },
#endif
#ifdef GL_EXT_histogram
    {
	{{GL_HISTOGRAM_EXT, OG_STATE_CHECK_NULL}, "HISTOGRAM", NULL,
         checkEnabled, STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE},},
	"GL_EXT_histogram"
    },
    {
	{{GL_HISTOGRAM_EXT, GL_HISTOGRAM_WIDTH_EXT, OG_STATE_CHECK_NULL},
         "HISTOGRAM", "WIDTH", getTargetParameterCheck, STATEDATA_PREDEFINED,
         1, (void (*) (void)) glGetHistogramParameterfvEXT, {0},},
        "GL_EXT_histogram"
    },
    {
	{{GL_HISTOGRAM_EXT, GL_HISTOGRAM_FORMAT_EXT, OG_STATE_CHECK_NULL},
         "HISTOGRAM", "FORMAT", getTargetParameterCheck, STATEDATA_PREDEFINED,
         1, (void (*) (void)) glGetHistogramParameterfvEXT, {GL_RGBA},},
	"GL_EXT_histogram"
    },
#if 0 /* As internal sizes can not be reset to 0 this can not be tested */
    {
	{{GL_HISTOGRAM_EXT, GL_HISTOGRAM_RED_SIZE_EXT, OG_STATE_CHECK_NULL},
         "HISTOGRAM", "RED_SIZE", getTargetParameterCheck, STATEDATA_PREDEFINED,
         1, (void (*) (void)) glGetHistogramParameterfvEXT, {0},},
	"GL_EXT_histogram"
    },
    {
	{{GL_HISTOGRAM_EXT, GL_HISTOGRAM_GREEN_SIZE_EXT, OG_STATE_CHECK_NULL},
         "HISTOGRAM", "GREEN_SIZE", getTargetParameterCheck, STATEDATA_PREDEFINED,
         1, (void (*) (void)) glGetHistogramParameterfvEXT, {0},},
	"GL_EXT_histogram"
    },
    {
	{{GL_HISTOGRAM_EXT, GL_HISTOGRAM_BLUE_SIZE_EXT, OG_STATE_CHECK_NULL},
         "HISTOGRAM", "BLUE_SIZE", getTargetParameterCheck, STATEDATA_PREDEFINED,
         1, (void (*) (void)) glGetHistogramParameterfvEXT, {0},},
	"GL_EXT_histogram"
    },
    {
	{{GL_HISTOGRAM_EXT, GL_HISTOGRAM_ALPHA_SIZE_EXT, OG_STATE_CHECK_NULL},
         "HISTOGRAM", "ALPHA_SIZE", getTargetParameterCheck, STATEDATA_PREDEFINED,
         1, (void (*) (void)) glGetHistogramParameterfvEXT, {0},},
	"GL_EXT_histogram"
    },
    {
	{{GL_HISTOGRAM_EXT, GL_HISTOGRAM_LUMINANCE_SIZE_EXT,
          OG_STATE_CHECK_NULL}, "HISTOGRAM", "LUMINANCE_SIZE",
         getTargetParameterCheck, STATEDATA_PREDEFINED, 1,
         (void (*) (void)) glGetHistogramParameterfvEXT, {0},},
	"GL_EXT_histogram"
    },
#endif /* Skip histogram internal sizes test */
    {
	{{GL_HISTOGRAM_EXT, GL_HISTOGRAM_SINK_EXT, OG_STATE_CHECK_NULL},
         "HISTOGRAM", "SINK", getTargetParameterCheck, STATEDATA_PREDEFINED,
         1, (void (*) (void)) glGetHistogramParameterfvEXT, {GL_FALSE},},
	"GL_EXT_histogram"
    },
    {
	{{GL_MINMAX_EXT, OG_STATE_CHECK_NULL}, "MINMAX", NULL,
         checkEnabled, STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE},},
	"GL_EXT_histogram"
    },
    {
	{{GL_MINMAX_EXT, GL_MINMAX_FORMAT_EXT, OG_STATE_CHECK_NULL},
         "MINMAX", "FORMAT", getTargetParameterCheck, STATEDATA_PREDEFINED,
         1, (void (*) (void)) glGetMinmaxParameterfvEXT, {GL_RGBA},},
	"GL_EXT_histogram"
    },
    {
	{{GL_MINMAX_EXT, GL_MINMAX_SINK_EXT, OG_STATE_CHECK_NULL},
         "MINMAX", "SINK", getTargetParameterCheck, STATEDATA_PREDEFINED,
         1, (void (*) (void)) glGetMinmaxParameterfvEXT, {GL_FALSE},},
	"GL_EXT_histogram"
    },
#endif
#ifdef GL_EXT_polygon_offset
    {
        {{GL_POLYGON_OFFSET_EXT, OG_STATE_CHECK_NULL}, "POLYGON_OFFSET",
         NULL, checkEnabled, STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE},},
        "GL_EXT_polygon_offset"
    },
    {
        {{GL_POLYGON_OFFSET_FACTOR_EXT, OG_STATE_CHECK_NULL},
         "POLYGON_OFFSET_FACTOR", NULL,
         simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {0},},
        "GL_EXT_polygon_offset"
    },
    {
        {{GL_POLYGON_OFFSET_BIAS_EXT, OG_STATE_CHECK_NULL},
         "POLYGON_OFFSET_BIAS", NULL,
         simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {0},},
        "GL_EXT_polygon_offset"
    },
#endif
#ifdef GL_SGIS_multisample
    {
        {{GL_MULTISAMPLE_SGIS, OG_STATE_CHECK_NULL}, "MULTISAMPLE", NULL,
         checkEnabled, STATEDATA_PREDEFINED, 1, NULL, {GL_TRUE},},
        "GL_SGIS_multisample"
    },
    {
        {{GL_SAMPLE_ALPHA_TO_MASK_SGIS, OG_STATE_CHECK_NULL},
         "SAMPLE_ALPHA_TO_MASK", NULL,
         checkEnabled, STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE},},
        "GL_SGIS_multisample"
    },
    {
        {{GL_SAMPLE_ALPHA_TO_ONE_SGIS, OG_STATE_CHECK_NULL},
         "SAMPLE_ALPHA_TO_ONE", NULL,
         checkEnabled, STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE},},
        "GL_SGIS_multisample"
    },
    {
        {{GL_SAMPLE_MASK_SGIS, OG_STATE_CHECK_NULL}, "SAMPLE_MASK", NULL,
         checkEnabled, STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE},},
        "GL_SGIS_multisample"
    },
    {
        {{GL_SAMPLE_MASK_VALUE_SGIS, OG_STATE_CHECK_NULL}, "SAMPLE_MASK_VALUE",
         NULL, simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {1},},
        "GL_SGIS_multisample"
    },
    {
        {{GL_SAMPLE_MASK_INVERT_SGIS, OG_STATE_CHECK_NULL},
         "SAMPLE_MASK_INVERT", NULL,
         simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE},},
        "GL_SGIS_multisample"
    },
    {
        {{GL_SAMPLE_PATTERN_SGIS, OG_STATE_CHECK_NULL}, "SAMPLE_PATTRN", NULL,
         simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {GL_1PASS_SGIS},},
        "GL_SGIS_multisample"
    },
    {
        {{GL_SAMPLE_BUFFERS_SGIS, OG_STATE_CHECK_NULL}, "SAMPLE_BUFFERS", NULL,
         checkVisual, STATEDATA_ONTHEFLY, 1, NULL, {0},},
        "GL_SGIS_multisample"
    },
    {
        {{GL_SAMPLES_SGIS, OG_STATE_CHECK_NULL}, "SAMPLES", NULL,
         checkVisual, STATEDATA_ONTHEFLY, 1, NULL, {0},},
        "GL_SGIS_multisample"
    },
#endif
#ifdef GL_SGIS_fog_function
    {
        {{GL_FOG_FUNC_POINTS_SGIS, OG_STATE_CHECK_NULL}, "FOG_FUNC_POINTS", NULL,
         simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {1},},
        "GL_SGIS_fog_function"
    },
    {
        {{GL_FOG_FUNC_SGIS, OG_STATE_CHECK_NULL}, "FOG_FUNC", NULL,
         simpleGetCheck, STATEDATA_PREDEFINED, 2, NULL, {0.0, 1.0},},
        "GL_SGIS_fog_function"
    },
#endif
#ifdef GL_SGIX_fog_offset
    {
	{{GL_FOG_OFFSET_SGIX, OG_STATE_CHECK_NULL}, "FOG_OFFSET_SGIX", NULL,
         checkEnabled, STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}},
        "GL_SGIX_fog_offset"
    },
    {
        {{GL_FOG_OFFSET_VALUE_SGIX, OG_STATE_CHECK_NULL}, "FOG_OFFSET_VALUE",
         NULL, simpleGetCheck, STATEDATA_PREDEFINED, 4, NULL,
         {0.0, 0.0, 0.0, 0.0},},
        "GL_SGIX_fog_offset"
    },
#endif
#ifdef GL_EXT_texture
#if 0 /* As internal sizes can not be reset to 0 this can not be tested */
    {
	{{GL_TEXTURE_1D, GL_TEXTURE_RED_SIZE_EXT, OG_STATE_CHECK_NULL},
         "TEXTURE_1D", "TEXTURE_RED_SIZE",
         checkTexLevelExtParams, STATEDATA_PREDEFINED, 1, NULL, {0},},
	"GL_EXT_texture"
    },
    {
	{{GL_TEXTURE_1D, GL_TEXTURE_GREEN_SIZE_EXT, OG_STATE_CHECK_NULL},
         "TEXTURE_1D", "TEXTURE_GREEN_SIZE",
         checkTexLevelExtParams, STATEDATA_PREDEFINED, 1, NULL, {0},},
	"GL_EXT_texture"
    },
    {
	{{GL_TEXTURE_1D, GL_TEXTURE_BLUE_SIZE_EXT, OG_STATE_CHECK_NULL},
         "TEXTURE_BLUE_SIZE", NULL,
         checkTexLevelExtParams, STATEDATA_PREDEFINED, 1, NULL, {0},},
	"GL_EXT_texture"
    },
    {
	{{GL_TEXTURE_1D, GL_TEXTURE_ALPHA_SIZE_EXT, OG_STATE_CHECK_NULL},
         "TEXTURE_1D", "TEXTURE_ALPHA_SIZE",
         checkTexLevelExtParams, STATEDATA_PREDEFINED, 1, NULL, {0},},
	"GL_EXT_texture"
    },
    {
	{{GL_TEXTURE_1D, GL_TEXTURE_LUMINANCE_SIZE_EXT, OG_STATE_CHECK_NULL},
         "TEXTURE_1D", "TEXTURE_LUMINANCE_SIZE",
         checkTexLevelExtParams, STATEDATA_PREDEFINED, 1, NULL, {0},},
	"GL_EXT_texture"
    },
    {
	{{GL_TEXTURE_1D, GL_TEXTURE_INTENSITY_SIZE_EXT, OG_STATE_CHECK_NULL},
         "TEXTURE_1D", "TEXTURE_INTENSITY_SIZE",
         checkTexLevelExtParams, STATEDATA_PREDEFINED, 1, NULL, {0},},
	"GL_EXT_texture"
    },
    {
	{{GL_TEXTURE_2D, GL_TEXTURE_RED_SIZE_EXT, OG_STATE_CHECK_NULL},
         "TEXTURE_2D", "TEXTURE_RED_SIZE",
         checkTexLevelExtParams, STATEDATA_PREDEFINED, 1, NULL, {0},},
	"GL_EXT_texture"
    },
    {
	{{GL_TEXTURE_2D, GL_TEXTURE_GREEN_SIZE_EXT, OG_STATE_CHECK_NULL},
         "TEXTURE_2D", "TEXTURE_GREEN_SIZE",
         checkTexLevelExtParams, STATEDATA_PREDEFINED, 1, NULL, {0},},
	"GL_EXT_texture"
    },
    {
	{{GL_TEXTURE_2D, GL_TEXTURE_BLUE_SIZE_EXT, OG_STATE_CHECK_NULL},
         "TEXTURE_2D", "TEXTURE_BLUE_SIZE",
         checkTexLevelExtParams, STATEDATA_PREDEFINED, 1, NULL, {0},},
	"GL_EXT_texture"
    },
    {
	{{GL_TEXTURE_2D, GL_TEXTURE_ALPHA_SIZE_EXT, OG_STATE_CHECK_NULL},
         "TEXTURE_2D", "TEXTURE_ALPHA_SIZE",
         checkTexLevelExtParams, STATEDATA_PREDEFINED, 1, NULL, {0},},
	"GL_EXT_texture"
    },
    {
	{{GL_TEXTURE_2D, GL_TEXTURE_LUMINANCE_SIZE_EXT, OG_STATE_CHECK_NULL},
         "TEXTURE_2D", "TEXTURE_LUMINANCE_SIZE",
         checkTexLevelExtParams, STATEDATA_PREDEFINED, 1, NULL, {0},},
	"GL_EXT_texture"
    },
    {
	{{GL_TEXTURE_2D, GL_TEXTURE_INTENSITY_SIZE_EXT, OG_STATE_CHECK_NULL},
         "TEXTURE_2D", "TEXTURE_INTENSITY_SIZE",
         checkTexLevelExtParams, STATEDATA_PREDEFINED, 1, NULL, {0},},
	"GL_EXT_texture"
    },
    {
	{{GL_PROXY_TEXTURE_1D_EXT, GL_TEXTURE_RED_SIZE_EXT,
          OG_STATE_CHECK_NULL}, "PROXY_TEXTURE_1D", "TEXTURE_RED_SIZE",
         checkTexLevelExtParams, STATEDATA_PREDEFINED, 1, NULL, {0},},
	"GL_EXT_texture"
    },
    {
	{{GL_PROXY_TEXTURE_1D_EXT, GL_TEXTURE_GREEN_SIZE_EXT,
          OG_STATE_CHECK_NULL}, "PROXY_TEXTURE_1D", "TEXTURE_GREEN_SIZE",
         checkTexLevelExtParams, STATEDATA_PREDEFINED, 1, NULL, {0},},
	"GL_EXT_texture"
    },
    {
	{{GL_PROXY_TEXTURE_1D_EXT, GL_TEXTURE_BLUE_SIZE_EXT,
          OG_STATE_CHECK_NULL}, "PROXY_TEXTURE_1D", "TEXTURE_BLUE_SIZE",
         checkTexLevelExtParams, STATEDATA_PREDEFINED, 1, NULL, {0},},
	"GL_EXT_texture"
    },
    {
	{{GL_PROXY_TEXTURE_1D_EXT, GL_TEXTURE_ALPHA_SIZE_EXT,
          OG_STATE_CHECK_NULL}, "PROXY_TEXTURE_1D", "TEXTURE_ALPHA_SIZE",
         checkTexLevelExtParams, STATEDATA_PREDEFINED, 1, NULL, {0},},
	"GL_EXT_texture"
    },
    {
	{{GL_PROXY_TEXTURE_1D_EXT, GL_TEXTURE_LUMINANCE_SIZE_EXT,
          OG_STATE_CHECK_NULL}, "PROXY_TEXTURE_1D", "TEXTURE_LUMINANCE_SIZE",
         checkTexLevelExtParams, STATEDATA_PREDEFINED, 1, NULL, {0},},
	"GL_EXT_texture"
    },
    {
	{{GL_PROXY_TEXTURE_1D_EXT, GL_TEXTURE_INTENSITY_SIZE_EXT,
          OG_STATE_CHECK_NULL}, "PROXY_TEXTURE_1D", "TEXTURE_INTENSITY_SIZE",
         checkTexLevelExtParams, STATEDATA_PREDEFINED, 1, NULL, {0},},
	"GL_EXT_texture"
    },
    {
	{{GL_PROXY_TEXTURE_2D_EXT, GL_TEXTURE_RED_SIZE_EXT,
          OG_STATE_CHECK_NULL}, "PROXY_TEXTURE_2D", "TEXTURE_RED_SIZE",
         checkTexLevelExtParams, STATEDATA_PREDEFINED, 1, NULL, {0},},
	"GL_EXT_texture"
    },
    {
	{{GL_PROXY_TEXTURE_2D_EXT, GL_TEXTURE_GREEN_SIZE_EXT,
          OG_STATE_CHECK_NULL}, "PROXY_TEXTURE_2D", "TEXTURE_GREEN_SIZE",
         checkTexLevelExtParams, STATEDATA_PREDEFINED, 1, NULL, {0},},
	"GL_EXT_texture"
    },
    {
	{{GL_PROXY_TEXTURE_2D_EXT, GL_TEXTURE_BLUE_SIZE_EXT,
          OG_STATE_CHECK_NULL}, "PROXY_TEXTURE_2D", "TEXTURE_BLUE_SIZE",
         checkTexLevelExtParams, STATEDATA_PREDEFINED, 1, NULL, {0},},
	"GL_EXT_texture"
    },
    {
	{{GL_PROXY_TEXTURE_2D_EXT, GL_TEXTURE_ALPHA_SIZE_EXT,
          OG_STATE_CHECK_NULL}, "PROXY_TEXTURE_2D", "TEXTURE_ALPHA_SIZE",
         checkTexLevelExtParams, STATEDATA_PREDEFINED, 1, NULL, {0},},
	"GL_EXT_texture"
    },
    {
	{{GL_PROXY_TEXTURE_2D_EXT, GL_TEXTURE_LUMINANCE_SIZE_EXT,
          OG_STATE_CHECK_NULL}, "PROXY_TEXTURE_2D", "TEXTURE_LUMINANCE_SIZE",
         checkTexLevelExtParams, STATEDATA_PREDEFINED, 1, NULL, {0},},
	"GL_EXT_texture"
    },
    {
	{{GL_PROXY_TEXTURE_2D_EXT, GL_TEXTURE_INTENSITY_SIZE_EXT,
          OG_STATE_CHECK_NULL}, "PROXY_TEXTURE_2D", "TEXTURE_INTENSITY_SIZE",
         checkTexLevelExtParams, STATEDATA_PREDEFINED, 1, NULL, {0},},
	"GL_EXT_texture"
    },
#endif /* Skip texture internal sizes test */
#endif
#ifdef GL_SGIX_texture_add_env
    {
        {{GL_TEXTURE_ENV, GL_TEXTURE_ENV_BIAS_SGIX, OG_STATE_CHECK_NULL},
         "TEXTURE_ENV", "BIAS", 
         getTargetParameterCheck, STATEDATA_PREDEFINED,
         4, (void (*) (void)) glGetTexEnvfv, {0.0, 0.0, 0.0, 0.0},},
        "GL_SGIX_texture_add_env"
    },
#endif
#ifdef GL_SGIX_clipmap
    {
	{{GL_TEXTURE_2D, GL_TEXTURE_CLIPMAP_CENTER_SGIX, OG_STATE_CHECK_NULL},
         "TEXTURE_2D", "CLIPMAP_CENTER",
         checkTexParam, STATEDATA_PREDEFINED, 2, NULL, {0, 0},},
	"GL_SGIX_clipmap"
    },
    {
	{{GL_TEXTURE_2D, GL_TEXTURE_CLIPMAP_OFFSET_SGIX, OG_STATE_CHECK_NULL},
         "TEXTURE_2D", "CLIPMAP_OFFSET",
         checkTexParam, STATEDATA_PREDEFINED, 2, NULL, {0, 0},},
	"GL_SGIX_clipmap"
    },
    {
	{{GL_TEXTURE_2D, GL_TEXTURE_CLIPMAP_FRAME_SGIX, OG_STATE_CHECK_NULL},
         "TEXTURE_2D", "CLIPMAP_FRAME",
         checkTexParam, STATEDATA_PREDEFINED, 1, NULL, {0},},
	"GL_SGIX_clipmap"
    },
    {
	{{GL_MAX_CLIPMAP_DEPTH_SGIX, OG_STATE_CHECK_NULL},
         "MAX_CLIPMAP_DEPTH", NULL,
         simpleGetCheck, STATEDATA_STATE_GET_TARGET, 1, NULL, {10},},
	"GL_SGIX_clipmap"
    },
#endif
#ifdef GL_SGIS_detail_texture
#ifdef GL_EXT_texture_object
    {
	{{GL_DETAIL_TEXTURE_2D_BINDING_SGIS, OG_STATE_CHECK_NULL},
         "TEXTURE_2D", "DETAIL_TEXTURE_2D_BINDING",
         simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {0},},
	"GL_SGIS_detail_texture"
    },
#endif
    {
	{{GL_TEXTURE_2D, GL_DETAIL_TEXTURE_LEVEL_SGIS, OG_STATE_CHECK_NULL},
         "TEXTURE_2D", "DETAIL_TEXTURE_LEVEL",
         checkTexParam, STATEDATA_PREDEFINED, 1, NULL, {-4},},
	"GL_SGIS_detail_texture"
    },
    {
	{{GL_TEXTURE_2D, GL_DETAIL_TEXTURE_MODE_SGIS, OG_STATE_CHECK_NULL},
         "TEXTURE_2D", "DETAIL_TEXTURE_MODE",
         checkTexParam, STATEDATA_PREDEFINED, 1, NULL, {GL_ADD},},
	"GL_SGIS_detail_texture"
    },
    {
	{{GL_TEXTURE_2D, GL_DETAIL_TEXTURE_FUNC_POINTS_SGIS, OG_STATE_CHECK_NULL},
         "TEXTURE_2D", "DETAIL_TEXTURE_FUNC_POINTS",
         checkTexParam, STATEDATA_PREDEFINED, 1, NULL, {2},},
	"GL_SGIS_detail_texture"
    },
    {
	{{GL_TEXTURE_2D, OG_STATE_CHECK_NULL},
         "TEXTURE_2D", "DETAIL_TEXTURE_FUNC", getFuncCheck, STATEDATA_PREDEFINED,
         4, (void (*) (void)) glGetDetailTexFuncSGIS, {0, 0, -4, 1},},
	"GL_SGIS_detail_texture"
    },
#endif
#ifdef GL_SGIS_sharpen_texture
    {
	{{GL_TEXTURE_2D, GL_SHARPEN_TEXTURE_FUNC_POINTS_SGIS, OG_STATE_CHECK_NULL},
         "TEXTURE_2D", "SHARPEN_TEXTURE_FUNC_POINTS",
         checkTexParam, STATEDATA_PREDEFINED, 1, NULL, {2},},
	"GL_SGIS_sharpen_texture"
    },
    {
	{{GL_TEXTURE_2D, OG_STATE_CHECK_NULL},
         "TEXTURE_2D", "SHARPEN_TEXTURE_FUNC", getFuncCheck, STATEDATA_PREDEFINED,
         4, (void (*) (void)) glGetSharpenTexFuncSGIS, {0, 0, -4, 1},},
	"GL_SGIS_sharpen_texture"
    },
#endif
#ifdef GL_SGIS_texture_filter4
    {
	{{GL_TEXTURE_2D, GL_TEXTURE_FILTER4_SIZE_SGIS, OG_STATE_CHECK_NULL},
         "TEXTURE_2D", "TEXTURE_FILTER4_SIZE",
         getTargetParameterCheck, STATEDATA_STATE_GET_TARGET, 1,
         (void (*) (void)) glGetTexParameterfv, {17},},
	"GL_SGIS_texture_filter4"
    },
    {
	{{GL_TEXTURE_2D, GL_FILTER4_SGIS, OG_STATE_CHECK_NULL},
         "TEXTURE_2D", "TEXTURE_FILTER4_FUNC",
         getTargetParameterCheck, STATEDATA_STATE_GET_TARGET,
         /*
          * Size is implementation dependent and is initialized upon
          * startup.
          */
         0, (void (*) (void)) getTexFilterHack,},
	"GL_SGIS_texture_filter4"
    },
#endif
#ifdef GL_EXT_texture3D
    {
	{{GL_UNPACK_SKIP_IMAGES_EXT, OG_STATE_CHECK_NULL}, "UNPACK_SKIP_IMAGES",
         NULL, simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {0},},
	"GL_EXT_texture3D"
    },
    {
	{{GL_UNPACK_IMAGE_HEIGHT_EXT, OG_STATE_CHECK_NULL}, "UNPACK_IMAGE_HEIGHT",
         NULL, simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {0},},
	"GL_EXT_texture3D"
    },
    {
	{{GL_PACK_SKIP_IMAGES_EXT, OG_STATE_CHECK_NULL}, "PACK_SKIP_IMAGES", NULL,
         simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {0},},
	"GL_EXT_texture3D"
    },
    {
	{{GL_PACK_IMAGE_HEIGHT_EXT, OG_STATE_CHECK_NULL}, "PACK_IMAGE_HEIGHT",
         NULL, simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {0},},
	"GL_EXT_texture3D"
    },
    {
	{{GL_TEXTURE_3D_EXT, GL_TEXTURE_WRAP_R_EXT, OG_STATE_CHECK_NULL},
         "TEXTURE_3D", "TEXTURE_WRAP_R",
         checkTexParam, STATEDATA_PREDEFINED, 1, NULL, {GL_REPEAT},},
	"GL_EXT_texture3D"
    },
#if 0
    {
	{{GL_TEXTURE_DEPTH_EXT, GL_TEXTURE_3D_EXT, OG_STATE_CHECK_NULL},
         "TEXTURE_3D", "TEXTURE_DEPTH",
         checkTexLevelParams, STATEDATA_PREDEFINED, 1, NULL, {0},},
	"GL_EXT_texture3D"
    },
#endif
    {
	{{GL_MAX_3D_TEXTURE_SIZE_EXT, OG_STATE_CHECK_NULL},
         "TEXTURE_3D", "MAX_3D_TEXTURE_SIZE",
         simpleGetCheck, STATEDATA_STATE_GET_TARGET, 1, NULL, {16},},
	"GL_EXT_texture3D"
    },
#endif
#ifdef GL_EXT_texture_object
    {
	{{GL_TEXTURE_1D_BINDING_EXT, OG_STATE_CHECK_NULL}, "TEXTURE_1D_BINDING",
         NULL, simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {0},},
	"GL_EXT_texture_object"
    },
    {
	{{GL_TEXTURE_2D_BINDING_EXT, OG_STATE_CHECK_NULL}, "TEXTURE_2D_BINDING",
         NULL, simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {0},},
	"GL_EXT_texture_object"
    },
#ifdef GL_EXT_texture3D
    {
	{{GL_TEXTURE_3D_BINDING_EXT, OG_STATE_CHECK_NULL}, "TEXTURE_3D_BINDING",
         NULL, simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {0},},
	"GL_EXT_texture_object"
    },
#endif
    {
	{{GL_TEXTURE_2D, GL_TEXTURE_PRIORITY_EXT, OG_STATE_CHECK_NULL},
         "TEXTURE_2D", "PRIORITY",
         checkTexParam, STATEDATA_PREDEFINED, 1, NULL, {1},},
	"GL_EXT_texture_object"
    },
#endif
#ifdef GL_SGIS_texture_lod
    {
	{{GL_TEXTURE_1D, GL_TEXTURE_MIN_LOD_SGIS, OG_STATE_CHECK_NULL},
         "TEXTURE_1D", "MIN_LOD",
         checkTexParam, STATEDATA_PREDEFINED, 1, NULL, {-1000},},
	"GL_SGIS_texture_lod"
    },
    {
	{{GL_TEXTURE_1D, GL_TEXTURE_MAX_LOD_SGIS, OG_STATE_CHECK_NULL},
         "TEXTURE_1D", "MAX_LOD",
         checkTexParam, STATEDATA_PREDEFINED, 1, NULL, {1000},},
	"GL_SGIS_texture_lod"
    },
    {
	{{GL_TEXTURE_1D, GL_TEXTURE_BASE_LEVEL_SGIS, OG_STATE_CHECK_NULL},
         "TEXTURE_1D", "BASE_LEVEL",
         checkTexParam, STATEDATA_PREDEFINED, 1, NULL, {0},},
	"GL_SGIS_texture_lod"
    },
    {
	{{GL_TEXTURE_1D, GL_TEXTURE_MAX_LEVEL_SGIS, OG_STATE_CHECK_NULL},
         "TEXTURE_1D", "MAX_LEVEL",
         checkTexParam, STATEDATA_PREDEFINED, 1, NULL, {1000},},
	"GL_SGIS_texture_lod"
    },
    {
	{{GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD_SGIS, OG_STATE_CHECK_NULL},
         "TEXTURE_2D", "MIN_LOD",
         checkTexParam, STATEDATA_PREDEFINED, 1, NULL, {-1000},},
	"GL_SGIS_texture_lod"
    },
    {
	{{GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD_SGIS, OG_STATE_CHECK_NULL},
         "TEXTURE_2D", "MAX_LOD",
         checkTexParam, STATEDATA_PREDEFINED, 1, NULL, {1000},},
	"GL_SGIS_texture_lod"
    },
    {
	{{GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL_SGIS, OG_STATE_CHECK_NULL},
         "TEXTURE_2D", "BASE_LEVEL",
         checkTexParam, STATEDATA_PREDEFINED, 1, NULL, {0},},
	"GL_SGIS_texture_lod"
    },
    {
	{{GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL_SGIS, OG_STATE_CHECK_NULL},
         "TEXTURE_2D", "MAX_LEVEL",
         checkTexParam, STATEDATA_PREDEFINED, 1, NULL, {1000},},
	"GL_SGIS_texture_lod"
    },
#ifdef GL_EXT_texture3D
    {
	{{GL_TEXTURE_3D_EXT, GL_TEXTURE_MIN_LOD_SGIS, OG_STATE_CHECK_NULL},
         "TEXTURE_3D", "MIN_LOD",
         checkTexParam, STATEDATA_PREDEFINED, 1, NULL, {-1000},},
	"GL_SGIS_texture_lod"
    },
    {
	{{GL_TEXTURE_3D_EXT, GL_TEXTURE_MAX_LOD_SGIS, OG_STATE_CHECK_NULL},
         "TEXTURE_3D", "MAX_LOD",
         checkTexParam, STATEDATA_PREDEFINED, 1, NULL, {1000},},
	"GL_SGIS_texture_lod"
    },
    {
	{{GL_TEXTURE_3D_EXT, GL_TEXTURE_BASE_LEVEL_SGIS, OG_STATE_CHECK_NULL},
         "TEXTURE_3D", "BASE_LEVEL",
         checkTexParam, STATEDATA_PREDEFINED, 1, NULL, {0},},
	"GL_SGIS_texture_lod"
    },
    {
	{{GL_TEXTURE_3D_EXT, GL_TEXTURE_MAX_LEVEL_SGIS, OG_STATE_CHECK_NULL},
         "TEXTURE_3D", "MAX_LEVEL",
         checkTexParam, STATEDATA_PREDEFINED, 1, NULL, {1000},},
	"GL_SGIS_texture_lod"
    },
#endif
#endif
#ifdef GL_SGIX_sprite
    {
	{{GL_SPRITE_SGIX, OG_STATE_CHECK_NULL}, "SPRITE_SGIX", NULL,
         checkEnabled, STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE}},
        "GL_SGIX_sprite"
    },
    {
	{{GL_SPRITE_MODE_SGIX, OG_STATE_CHECK_NULL}, "SPRITE_MODE", NULL,
         simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {GL_SPRITE_AXIAL_SGIX},},
	"GL_SGIX_sprite"
    },
    {
	{{GL_SPRITE_TRANSLATION_SGIX, OG_STATE_CHECK_NULL}, "SPRITE_TRANSLATION",
         NULL, simpleGetCheck, STATEDATA_PREDEFINED, 3, NULL, {0, 0, 0},},
	"GL_SGIX_sprite"
    },
    {
	{{GL_SPRITE_AXIS_SGIX, OG_STATE_CHECK_NULL}, "SPRITE_AXIS", NULL,
         simpleGetCheck, STATEDATA_PREDEFINED, 3, NULL, {0, 0, 1},},
	"GL_SGIX_sprite"
    },
#endif
#ifdef GL_SGIS_point_parameters
    {
	{{GL_POINT_SIZE_MIN_SGIS, OG_STATE_CHECK_NULL}, "POINT_SIZE_MIN",
         NULL, simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {0.0},},
	"GL_SGIS_point_parameters"
    },
    {
	{{GL_POINT_SIZE_MAX_SGIS, OG_STATE_CHECK_NULL}, "POINT_SIZE_MAX",
         NULL, simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {256.0},},
	"GL_SGIS_point_parameters"
    },
    {
	{{GL_POINT_FADE_THRESHOLD_SIZE_SGIS, OG_STATE_CHECK_NULL},
         "POINT_FADE_THRESHOLD_SIZE", NULL,
         simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {1.0},},
	"GL_SGIS_point_parameters",
    },
    {
	{{GL_DISTANCE_ATTENUATION_SGIS, OG_STATE_CHECK_NULL},
         "DISTANCE_ATTENUATION", NULL,
         simpleGetCheck, STATEDATA_PREDEFINED, 3, NULL, {1.0,0.0,0.0},},
	"GL_SGIS_point_parameters"
    },
#endif
#ifdef GL_SGIX_polynomial_ffd
    {
	{{GL_GEOMETRY_DEFORMATION_SGIX, GL_ORDER}, "GEOMETRY_DEFORMATION",
         "ORDER", checkEvalMap, STATEDATA_PREDEFINED, 3, NULL, {2, 2, 2},},
	"GL_SGIX_polynomial_ffd",
    },
    {
	{{GL_GEOMETRY_DEFORMATION_SGIX, GL_DOMAIN}, "GEOMETRY_DEFORMATION",
         "DOMAIN", checkEvalMap, STATEDATA_PREDEFINED, 6,
         NULL, {0, 1, 0, 1, 0, 1},},
	"GL_SGIX_polynomial_ffd",
    },
    {
	{{GL_GEOMETRY_DEFORMATION_SGIX, GL_COEFF}, "GEOMETRY_DEFORMATION",
         "COEFF", checkEvalMap, STATEDATA_PREDEFINED, 3, NULL,
         {0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 1,
          1, 0, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1},},
	"GL_SGIX_polynomial_ffd",
    },
    {
	{{GL_TEXTURE_DEFORMATION_SGIX, GL_ORDER}, "TEXTURE_DEFORMATION",
         "ORDER", checkEvalMap, STATEDATA_PREDEFINED, 3, NULL, {2, 2, 2}, },
	"GL_SGIX_polynomial_ffd",
    },
    {
	{{GL_TEXTURE_DEFORMATION_SGIX, GL_DOMAIN}, "TEXTURE_DEFORMATION",
         "DOMAIN", checkEvalMap, STATEDATA_PREDEFINED, 6, NULL,
         {0, 1, 0, 1, 0, 1},},
	"GL_SGIX_polynomial_ffd",
    },
    {
	{{GL_TEXTURE_DEFORMATION_SGIX, GL_COEFF}, "TEXTURE_DEFORMATION",
         "COEFF", checkEvalMap, STATEDATA_PREDEFINED, 3, NULL,
         {0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 1,
          1, 0, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1},},
	"GL_SGIX_polynomial_ffd",
    },
    {
	{{GL_DEFORMATIONS_MASK_SGIX, OG_STATE_CHECK_NULL}, "DEFORMATIONS_MASK",
         NULL, simpleGetCheck, STATEDATA_PREDEFINED, 1, NULL, {0},},
	"GL_SGIX_polynomial_ffd",
    },
    {
	{{GL_MAX_DEFORMATION_ORDER_SGIX, OG_STATE_CHECK_NULL},
         "MAX_DEFORMATION_ORDER", NULL, simpleGetCheck,
         STATEDATA_STATE_GET_TARGET, 1, NULL, {2},},
	"GL_SGIX_polynomial_ffd",
    },
#endif
#ifdef GL_SGIX_texture_scale_bias    
    {
	{{GL_TEXTURE_1D, GL_POST_TEXTURE_FILTER_BIAS_SGIX, OG_STATE_CHECK_NULL},
         "TEXTURE_1D", "POST_TEXTURE_FILTER_BIAS",
         checkTexParam, STATEDATA_PREDEFINED, 4, NULL, {0, 0, 0, 0},},
	"GL_SGIX_texture_scale_bias"
    },
    {
	{{GL_TEXTURE_1D, GL_POST_TEXTURE_FILTER_SCALE_SGIX, OG_STATE_CHECK_NULL},
         "TEXTURE_1D", "POST_TEXTURE_FILTER_SCALE",
         checkTexParam, STATEDATA_PREDEFINED, 4, NULL, {1, 1, 1, 1},},
	"GL_SGIX_texture_scale_bias"
    },
    {
	{{GL_TEXTURE_2D, GL_POST_TEXTURE_FILTER_BIAS_SGIX, OG_STATE_CHECK_NULL},
         "TEXTURE_2D", "POST_TEXTURE_FILTER_BIAS",
         checkTexParam, STATEDATA_PREDEFINED, 4, NULL, {0, 0, 0, 0},},
	"GL_SGIX_texture_scale_bias"
    },
    {
	{{GL_TEXTURE_2D, GL_POST_TEXTURE_FILTER_SCALE_SGIX, OG_STATE_CHECK_NULL},
	"TEXTURE_2D", "POST_TEXTURE_FILTER_SCALE",
	checkTexParam, STATEDATA_PREDEFINED, 4, NULL, {1, 1, 1, 1},},
	"GL_SGIX_texture_scale_bias"
    },
#ifdef GL_EXT_texture3D
    {
	{{GL_TEXTURE_3D_EXT, GL_POST_TEXTURE_FILTER_BIAS_SGIX,
          OG_STATE_CHECK_NULL},
	"TEXTURE_3D", "POST_TEXTURE_FILTER_BIAS",
	checkTexParam, STATEDATA_PREDEFINED, 4, NULL, {0, 0, 0, 0},},
	"GL_SGIX_texture_scale_bias"
    },
    {
	{{GL_TEXTURE_3D_EXT, GL_POST_TEXTURE_FILTER_SCALE_SGIX,
          OG_STATE_CHECK_NULL},
	"TEXTURE_3D", "POST_TEXTURE_FILTER_SCALE",
	checkTexParam, STATEDATA_PREDEFINED, 4, NULL, {1, 1, 1, 1},},
	"GL_SGIX_texture_scale_bias"
    },
    {
	{{GL_POST_TEXTURE_FILTER_BIAS_RANGE_SGIX, OG_STATE_CHECK_NULL},
         "POST_TEXTURE_FILTER_BIAS_RANGE", NULL, simpleGetCheck,
         STATEDATA_STATE_GET_TARGET, 2, NULL, {-10, 1},},
	"GL_SGIX_texture_scale_bias",
    },
    {
	{{GL_POST_TEXTURE_FILTER_SCALE_RANGE_SGIX, OG_STATE_CHECK_NULL},
         "POST_TEXTURE_FILTER_SCALE_RANGE", NULL, simpleGetCheck,
         STATEDATA_STATE_GET_TARGET, 2, NULL, {-10, 1},},
	"GL_SGIX_texture_scale_bias",
    },
#endif
#endif
#ifdef GL_SGIX_reference_plane
    {
	{{GL_REFERENCE_PLANE_SGIX, OG_STATE_CHECK_NULL},
         "GL_REFERENCE_PLANE_SGIX", NULL, checkEnabled,
         STATEDATA_PREDEFINED, 1, NULL, {GL_FALSE},},
	"GL_SGIX_reference_plane",
    },
    {
	{{GL_REFERENCE_PLANE_EQUATION_SGIX, OG_STATE_CHECK_NULL},
         "GL_REFERENCE_PLANE_EQUATION_SGIX", NULL, simpleGetCheck,
         STATEDATA_PREDEFINED, 4, NULL, {0, 0, 1, 0},},
	"GL_SGIX_reference_plane",
    },
#endif
    {
        {{OG_STATE_CHECK_NULL}, NULL, NULL}
    }
};

static void
checkErrorPrintOneLine(stateRec *ptr, GLfloat *tmpBuf)
{
    int i;
    GLboolean firstError = GL_TRUE;

    for ( i = 0; i < ptr->dataCount; i++) {
	if (FLOAT_NEQUALS(tmpBuf[i], ptr->DefaultData[i]) ) {
            if (firstError) {
                ogEnvLog(OG_LALWAYS, "ERROR: %s, parameter %s is not default\n", 
                         ptr->valueString1, ptr->valueString2);
                ogEnvLog(OG_LALWAYS, "ERROR: value(s) "); 
                firstError = GL_FALSE;
            }
	    ogEnvLog(OG_LALWAYS, "  [%d] (%g != %g)", i,
                     tmpBuf[i], ptr->DefaultData[i]);
	}
    }
    if (!firstError)
        ogEnvLog(OG_LALWAYS, "\n");
}

static void
checkErrorPrintMultiLine(stateRec *ptr, GLfloat *tmpBuf, const char *extraDesc)
{
    int i;
    GLboolean firstError = GL_TRUE;

    for (i = 0; i < ptr->dataCount; i++) {
	if (FLOAT_NEQUALS(tmpBuf[i], ptr->DefaultData[i]) ) {
            if (firstError) {
                if (ptr->valueString2 == NULL)
                    ogEnvLog(OG_LALWAYS, "ERROR: %s %s(current != default)\n",
                             ptr->valueString1, extraDesc);
                else
                    ogEnvLog(OG_LALWAYS,
                             "ERROR: %s parameter of %s %s(current != default)\n",
                             ptr->valueString2, ptr->valueString1, extraDesc);
                firstError = GL_FALSE;
            }
            ogEnvLog(OG_LALWAYS,
                     "       %s[%d]: %g != %g (0x%08x != 0x%08x)\n",
                     (ptr->valueString2 != NULL ?
                      ptr->valueString2 : ptr->valueString1),
                     i, tmpBuf[i], ptr->DefaultData[i], 
                     *(GLuint *) &tmpBuf[i], *(GLuint *) &ptr->DefaultData[i]);
	}
    }
}

/*****************************************************************************/

static void simpleGetCheck(stateRec *ptr)
{
    GLfloat tmpBuf[STATEDATA_MAX_SIZE];

    glGetFloatv(ptr->value[0], tmpBuf);
    checkErrorPrintMultiLine(ptr, tmpBuf, "");
}

static void
checkEnabled(stateRec *ptr)
{
    GLfloat tmpBuf;

    glGetFloatv(ptr->value[0], &tmpBuf);
    checkErrorPrintOneLine(ptr, &tmpBuf);
    /* Check IsEnabled call as well */
    if ((tmpBuf = glIsEnabled(ptr->value[0])) != ptr->DefaultData[0])
        ogEnvLog(OG_LALWAYS, "ERROR: IsEnabled(%s) returns %sabled != default\n",
                 ptr->valueString1, tmpBuf ? "En" : "Dis");
}

static void
checkClipPlanes(stateRec *ptr)
{
    GLdouble tmpBuf[4];
    int i, j, numPlanes;

    glGetIntegerv(GL_MAX_CLIP_PLANES, &numPlanes);

    for (i =0; i < numPlanes; i++) {
	glGetClipPlane(GL_CLIP_PLANE0 + i, tmpBuf);
	for (j = 0; j < 4; j++) {
	    if (tmpBuf[j] != ptr->DefaultData[j]) {
		ogEnvLog(OG_LALWAYS, "ERROR: CLIP_PLANE%d Coefficent %d \
is not default %g != %g (0x%x != 0x%x)\n", i, j, tmpBuf[j], ptr->DefaultData[j],
                         *(unsigned int *) &tmpBuf[j],
                         *(unsigned int *) &ptr->DefaultData[j]);
	    }
	}
        if (glIsEnabled(GL_CLIP_PLANE0 + i))
            ogEnvLog(OG_LALWAYS,"ERROR: CLIP_PLANE%d is enabled\n", i);
    }
}

typedef struct {
    GLenum name;
    char nameString[40];
    int size;
    GLfloat defState[4];
} LightState;
    
static void
singleLightState(int index, LightState *ls)
{
    GLfloat retData[4];
    unsigned int *pInt = (unsigned int *) &retData[0];
    GLboolean firstError = GL_TRUE;
    int i;

    pInt[0] = pInt[1] = pInt[2] = pInt[3] = 0xdeadbeef;
    glGetLightfv(GL_LIGHT0 + index, ls->name, retData);
    for (i = 0; i < ls->size; i++) {
	if (FLOAT_NEQUALS(retData[i], ls->defState[i]) ) {
            if (firstError) {
                ogEnvLog(OG_LALWAYS,
                         "ERROR: %s parameter of LIGHT%d (current != default)\n",
                         ls->nameString, index);
                firstError = GL_FALSE;
            }
            ogEnvLog(OG_LALWAYS,
                     "       %s[%d]: %g != %g (0x%08x != 0x%08x)\n",
                     ls->nameString, i, retData[i], ls->defState[i], 
                     *(GLuint *) &retData[i], *(GLuint *) &ls->defState[i]);
	}
    }
}

/*ARGSUSED*/
static void
checkLights(stateRec *ptr)
{
    int nLights;
    int i, j;
    static LightState commonState[] = {
        {GL_AMBIENT, "AMBIENT", 4, {0, 0, 0, 1}},
        {GL_CONSTANT_ATTENUATION, "CONSTANT_ATTENUATION", 1, {1}},
	{GL_LINEAR_ATTENUATION, "LINEAR_ATTENUATION", 1, {0}},
        {GL_POSITION, "POSITION", 4, {0, 0, 1, 0}},
        {GL_QUADRATIC_ATTENUATION, "QUADRATIC_ATTENUATION", 1, {0}},
        {GL_SPOT_CUTOFF, "SPOT_CUTOFF", 1, {180}},
        {GL_SPOT_DIRECTION, "SPOT_DIRECTION", 3, {0, 0, -1}},
        {GL_SPOT_EXPONENT, "SPOT_EXPONENT", 1, {0}}
    };
    static LightState light0[] = {
	{GL_DIFFUSE, "DIFFUSE", 4, {1, 1, 1, 1}},
	{GL_SPECULAR, "SPECULAR", 4, {1, 1, 1, 1}}
    };
    static LightState lightI[] = {
	{GL_DIFFUSE, "DIFFUSE", 4, {0, 0, 0, 1}},
	{GL_SPECULAR, "SPECULAR", 4, {0, 0, 0, 1}}
    };

    glGetIntegerv(GL_MAX_LIGHTS, &nLights);

    if (nLights < 2) {
        ogEnvLog(OG_LALWAYS, "ERROR: Number of lights (%d) < 2, not checking \
any light state\n", nLights);
        return;
    }
    /*
     * State that's different between light 0 and the others.
     */
    for (i = sizeof(light0)/sizeof(LightState) - 1; i >= 0; i--) {
        singleLightState(0, &light0[i]);
        for (j = 1; j < nLights; j++)
            singleLightState(j, &lightI[i]);
    }
    /*
     * Common state
     */
    for (i = 0; i < nLights; i++) {
        GLfloat ret;
        
        ret = -9999;
        glGetFloatv(GL_LIGHT0 + i, &ret);
        if (ret != 0)
            ogEnvLog(OG_LALWAYS, "ERROR: get(LIGHT%d)=(%g,0x%x) != 0 \n",
                     i, ret, *(unsigned int *) &ret);
        if (glIsEnabled(GL_LIGHT0 + i))
            ogEnvLog(OG_LALWAYS, "ERROR: IsEnabled(LIGHT%d) returns TRUE\n", i);
        for (j = sizeof(commonState)/sizeof(LightState) - 1; j >= 0; j--) {
           singleLightState(i, &commonState[j]);
        }
    }
}
        
    
static void
checkEvalMap(stateRec *ptr)
{
    GLfloat tmpBuf[24];

    switch (ptr->value[1]) {
      case GL_ORDER:
      case GL_DOMAIN:
        glGetMapfv(ptr->value[0], ptr->value[1], tmpBuf);
        checkErrorPrintOneLine(ptr, tmpBuf);
	break;
      case GL_COEFF:
	/*
	** query the order first, because that determines the size of
	** the returned map
	*/
	glGetMapfv(ptr->value[0], GL_ORDER, tmpBuf);
#ifndef WIN32
	if (ptr->value[0] == GL_GEOMETRY_DEFORMATION_SGIX ||
            ptr->value[0] == GL_TEXTURE_DEFORMATION_SGIX) {
            if (tmpBuf[0] != 2 || tmpBuf[1] != 2 || tmpBuf[2] != 2)
                goto badOrder;
        } else if (tmpBuf[0] != 1 ||
                   (ptr->valueString1[3] == '2' && tmpBuf[1] != 1))
            goto badOrder;
#else
        if (tmpBuf[0] != 1 ||
                   (ptr->valueString1[3] == '2' && tmpBuf[1] != 1))
            goto badOrder;
#endif
        glGetMapfv(ptr->value[0], ptr->value[1], tmpBuf);
        checkErrorPrintMultiLine(ptr, tmpBuf, "");
        return;
     badOrder:
        ogEnvLog(OG_LALWAYS,
                 "ERROR: %s parameter GL_ORDER is not default, \
can't test COEFF\n", ptr->valueString1);
	break;
      default:
	ogEnvLog(OG_LALWAYS, "ERROR: Bad glGetMap query.\n");
	break;
    }
}

static void checkPixelMap(stateRec *ptr)
{
    GLfloat Currentdata;

    glGetFloatv(ptr->value[1], &Currentdata);
    if ( FLOAT_NEQUALS(Currentdata, ptr->DefaultData[1]) ) {
	ogEnvLog(OG_LALWAYS,"ERROR: %s is %g\n", ptr->valueString2,
                 Currentdata);
    } else {
        glGetPixelMapfv(ptr->value[0], &Currentdata);
	if ( FLOAT_NEQUALS(Currentdata, ptr->DefaultData[0]) ) {
	    ogEnvLog(OG_LALWAYS,"ERROR: %s %g != default %g\n",
                     ptr->valueString1, ptr->DefaultData[0], Currentdata);
	}
    }
}

static GLboolean checkPixelStoreDefaults(void)
{
    GLboolean retVal = GL_TRUE;
    GLint tmpInt;

    glGetIntegerv(GL_PACK_ALIGNMENT, &tmpInt);
    if (tmpInt != 4) {
	ogEnvLog(OG_LALWAYS,"ERROR: GL_PACK_ROW_LENGTH is %d != 4\n", tmpInt);
	retVal = GL_FALSE;
    }
    glGetIntegerv(GL_PACK_LSB_FIRST, &tmpInt);
    if (tmpInt) {
	ogEnvLog(OG_LALWAYS,"ERROR: GL_PACK_LSB_FIRST is GL_TRUE\n");
	retVal = GL_FALSE;
    }
    glGetIntegerv(GL_PACK_ROW_LENGTH, &tmpInt);
    if ( tmpInt != 0 ) {
	ogEnvLog(OG_LALWAYS,"ERROR: GL_PACK_ROW_LENGTH is %d != 0\n", tmpInt);
	retVal = GL_FALSE;
    }
    glGetIntegerv(GL_PACK_SKIP_ROWS, &tmpInt);
    if ( tmpInt != 0 ) {
	ogEnvLog(OG_LALWAYS,"ERROR: GL_PACK_SKIP_ROWS is %d != 0\n", tmpInt);
	retVal = GL_FALSE;
    }
    glGetIntegerv(GL_PACK_SKIP_PIXELS, &tmpInt);
    if ( tmpInt != 0 ) {
	ogEnvLog(OG_LALWAYS,"ERROR: GL_PACK_SKIP_PIXELS is %d != 0\n", tmpInt);
	retVal = GL_FALSE;
    }
    glGetIntegerv(GL_PACK_SWAP_BYTES, &tmpInt);
    if (tmpInt) {
	ogEnvLog(OG_LALWAYS,"ERROR: GL_PACK_SWAP_BYTES is GL_TRUE\n");
	retVal = GL_FALSE;
    }
    return retVal;
}

static void checkPolygonStipple(stateRec *ptr)
{
    GLubyte tmpBuf[128];
    GLint i;

    /*CONSTANTCONDITION*/
    if ( checkPixelStoreDefaults() ) {
	for (i = 0; i < 128; i++) {
	    tmpBuf[i] = 0x0;
	}
	glGetPolygonStipple((GLubyte *)tmpBuf);
	for (i = 0; i < 128; i++) {
	    if ( FLOAT_NEQUALS(ptr->DefaultData[i], (float)tmpBuf[i]) ) {
		ogEnvLog(OG_LALWAYS,"ERROR: %s is non-default\n", 
					ptr->valueString1);
		return;
	    }
	}
    } else {
	ogEnvLog(OG_LALWAYS,"ERROR: %s, PixelStore modes are non-default\n",
				ptr->valueString1);
    }
}

static void checkTexParam(stateRec *ptr)
{
    GLfloat tmpBuf[4];

    glGetTexParameterfv(ptr->value[0], ptr->value[1], tmpBuf);
    checkErrorPrintMultiLine(ptr, tmpBuf, "");
}

static void checkTexLevelDefaults(GLint level, stateRec *ptr)
{
    GLint     tmpInt;

    glGetTexLevelParameteriv(ptr->value[1], level, GL_TEXTURE_WIDTH, &tmpInt);
    if ( tmpInt != 0 ) {
	ogEnvLog(OG_LALWAYS,"ERROR: %s level %d GL_TEXTURE_WIDTH isn't zero\n",
				ptr->valueString2, level);
	ogEnvLog(OG_LALWAYS,"ERROR: the value is %d\n", tmpInt);
    }
    glGetTexLevelParameteriv( ptr->value[1], level, GL_TEXTURE_HEIGHT, 
				&tmpInt);
    if ( tmpInt != 0 ) {
	ogEnvLog(OG_LALWAYS,"ERROR: %s level %d GL_TEXTURE_HEIGHT isn't zero\n",
				ptr->valueString2, level);
	ogEnvLog(OG_LALWAYS,"ERROR: the value is %d\n", tmpInt);
    }
    glGetTexLevelParameteriv( ptr->value[1], level, GL_TEXTURE_BORDER, 
				&tmpInt);
    if ( tmpInt != 0 ) {
	ogEnvLog(OG_LALWAYS,"ERROR: %s level %d GL_TEXTURE_BORDER isn't zero\n",
				ptr->valueString2, level);
	ogEnvLog(OG_LALWAYS,"ERROR: the value is %d\n", tmpInt);
    }
    glGetTexLevelParameteriv( ptr->value[1], level, GL_TEXTURE_COMPONENTS, 
				&tmpInt);
    if ( tmpInt != 1) {
	ogEnvLog(OG_LALWAYS,"ERROR: %s level %d GL_TEXTURE_COMPONENTS is not one\n",
                 ptr->valueString2, level);
	ogEnvLog(OG_LALWAYS,"ERROR: the value is %d\n", tmpInt);
    }
    
}

static void checkTexLevelParams(stateRec *ptr)
{
    int maxWidth, i;
    unsigned int maxLevel;

    glGetFloatv(ptr->value[0], ptr->DefaultData );
    maxWidth = (int)ptr->DefaultData[0];
    if ( maxWidth & (maxWidth - 1) ) {
	ogEnvLog(OG_LALWAYS, "ERROR: %s returned non-power of 2, ie %d\n", 
                 ptr->valueString1, maxWidth);
	return;
    }
    
    maxLevel = ffs(maxWidth);
    
    for (i = 0; i < maxLevel; i++) {
	checkTexLevelDefaults(i, ptr);
    }
}

static void
getFuncCheck(stateRec *ptr) {
    GLfloat tmpBuf[STATEDATA_MAX_SIZE];
    unsigned int *pInt = (unsigned int *) &tmpBuf[0];
    int i;

    for (i=0; i < STATEDATA_MAX_SIZE; i++)
	pInt[i] = 0xdeadbeef;

    ((void (*) (GLenum, GLfloat *)) ptr->SpecialGetFunc)(ptr->value[0], tmpBuf);
    checkErrorPrintMultiLine(ptr, tmpBuf, "");
}

static void checkVisual(stateRec *ptr)
{
    GLint value, GetRetVal;

    glGetIntegerv(ptr->value[0], &GetRetVal);

    switch (ptr->value[0]) {
	case GL_ACCUM_ALPHA_BITS:
	  value = ogEnvCurVisualInfo(GLX_ACCUM_ALPHA_SIZE);
	  break;
	case GL_ACCUM_BLUE_BITS:
	  value = ogEnvCurVisualInfo(GLX_ACCUM_BLUE_SIZE);
	  break;
	case GL_ACCUM_GREEN_BITS:
	  value = ogEnvCurVisualInfo(GLX_ACCUM_GREEN_SIZE);
	  break;
	case GL_ACCUM_RED_BITS:
	  value = ogEnvCurVisualInfo(GLX_ACCUM_RED_SIZE);
	  break;
	case GL_ALPHA_BITS:
	  value = ogEnvCurVisualInfo(GLX_ALPHA_SIZE);
	  /*XXXblythe hack around KONA dual personality visuals */
	  if (ogEnvIsCIMode() && value == 16) value = 0;
	  break;
	case GL_AUX_BUFFERS:
	  value = ogEnvCurVisualInfo(GLX_AUX_BUFFERS);
	  break;
	case GL_BLUE_BITS:
	  value = ogEnvCurVisualInfo(GLX_BLUE_SIZE);
	  break;
	case GL_DEPTH_BITS:
	  value = ogEnvCurVisualInfo(GLX_DEPTH_SIZE);
	  break;
	case GL_DOUBLEBUFFER:
	  value = ogEnvCurVisualInfo(GLX_DOUBLEBUFFER);
	  break;
	case GL_DRAW_BUFFER:
	case GL_READ_BUFFER:
          value = ogEnvDoingAuxBuffer() ? GL_AUX0 :
              ogEnvCurVisualInfo(GLX_DOUBLEBUFFER) ? GL_BACK : GL_FRONT;
	  break;
	case GL_GREEN_BITS:
	  value = ogEnvCurVisualInfo(GLX_GREEN_SIZE);
	  break;
	case GL_INDEX_BITS:
          /*
           * XXXZiv:
           * As currently glx loads modes indexBits with BUFFER_SIZE, we
           * never get 0 and as most core sides get this info off the
           * modes struct it will be wrong.  To prevent noise we will
           * therefore always check against buffer size.
           */
	  value =
#if 0
              ogEnvCurVisualInfo(GLX_RGBA) ? 0 :
#endif
              ogEnvCurVisualInfo(GLX_BUFFER_SIZE);
	  break;
	case GL_INDEX_MODE:
	  value = !ogEnvCurVisualInfo(GLX_RGBA);
	  break;
	case GL_INDEX_WRITEMASK:
          value = ogEnvCurVisualInfo(GLX_RGBA) ? 0 : 
	       (1 << ogEnvCurVisualInfo(GLX_BUFFER_SIZE)) - 1;
	  break;
	case GL_RED_BITS:
	  value = ogEnvCurVisualInfo(GLX_RED_SIZE);
	  /*XXXblythe hack around KONA dual personality visuals */
	  if (ogEnvIsCIMode() && value == 16) value = 0;
	  break;
	case GL_RGBA_MODE:
	  value = ogEnvCurVisualInfo(GLX_RGBA);
	  break;
	case GL_STENCIL_BITS:
	  value = ogEnvCurVisualInfo(GLX_STENCIL_SIZE);
	  break;
	case GL_STENCIL_VALUE_MASK:
	  value = ogEnvCurVisualInfo(GLX_STENCIL_SIZE);
	  value = (1 << value) - 1;
	  break;
	case GL_STENCIL_WRITEMASK:
	  value = ogEnvCurVisualInfo(GLX_STENCIL_SIZE);
	  value = (1 << value) - 1;
	  break;
	case GL_STEREO:
	  value = ogEnvCurVisualInfo(GLX_STEREO);
	  break;
#ifdef GL_SGIS_multisample
        case GL_SAMPLE_BUFFERS_SGIS:
          value = ogEnvCurVisualInfo(GLX_SAMPLE_BUFFERS_SGIS);
          break;
        case GL_SAMPLES_SGIS:
          value = ogEnvCurVisualInfo(GLX_SAMPLES_SGIS);
          break;
#endif
	default:
	  ogEnvLog(OG_LALWAYS, "Unsupported visual attribute.\n");
	  break;
    }
    if ( value != GetRetVal ) {
	ogEnvLog(OG_LALWAYS,"ERROR: %s is %d, should be %d\n",
                 ptr->valueString1, GetRetVal, value);
    }
}

/*
 * Check window dependent state
 */
static void
checkWindow(stateRec *ptr)
{
    GLfloat tmp[4];

    glGetFloatv(ptr->value[0], tmp);
    ptr->DefaultData[2] = ogEnvQuery(OG_XWSIZE);
    ptr->DefaultData[3] = ogEnvQuery(OG_YWSIZE);
    checkErrorPrintMultiLine(ptr, tmp, "");
}

static void
checkEnables(void)
{
    if (glIsEnabled(GL_ALPHA_TEST)) 
      ogEnvLog(OG_LALWAYS,"ERROR: GL_ALPHA_TEST is enabled\n");
}

/*
 * scan the first 32 texture objects and make sure they don't exist
 */
static void
checkTextureObjs(void) {
#if GL_EXT_texture_object
    if (strstr((const char *)glGetString(GL_EXTENSIONS), "GL_EXT_texture_object")) {
	GLuint i;
	for(i = 1; i <= 32; i++) {
	    if (glIsTextureEXT(i)) {
		  ogEnvLog(OG_LALWAYS,"ERROR: TextureObject %d exists but should not\n", i);
	    }
	}
    }
#endif
}

/*****************************************************************************/

/*
** Iterate over state (core GL attributes) and stateEXT (GL extension
** attributes) arrays, calling glGet*() calls and comparing against
** default values.
*/
void ogEnvCheckDefaultState(GLboolean beforeTest)
{
    stateRec *ptr;
    stateRecEXT *ptrEXT;
    GLenum err;

    ogEnvLog(OG_LALWAYS,"\n&&&&&& %s-test state checking &&&&&&\n",
             beforeTest ? "Pre" : "Post");
    while ((err = glGetError()) != GL_NO_ERROR)
        ogEnvLog(OG_LALWAYS,"ERROR: glGetError == 0x%x (%s)\n",
                 err, ogLibGLError(err));

    for (ptr = state; ptr->value[0] != OG_STATE_CHECK_NULL; ptr++) {
	(*ptr->GetFunc)(ptr);
#if 0
        if ((err = glGetError()) != GL_NO_ERROR) {
            ogEnvLog(OG_LALWAYS,"At %s glGetError(0x%x,%s)\n",
                     ptr->valueString1,
		     err,
                     ogLibGLError(err));
            if (ptr->valueString2)
                ogEnvLog(OG_LALWAYS,"------ Parameter is %s\n",
                         ptr->valueString2);
        }
#endif
    }
#ifdef GL_VERSION_1_1
    if (IS_ONEONE()) {
	for (ptr = state1; ptr->value[0] != OG_STATE_CHECK_NULL; ptr++) {
	    (*ptr->GetFunc)(ptr);
	}
    }
#endif

    for (ptrEXT = stateEXT; ptrEXT->standard.value[0] != OG_STATE_CHECK_NULL;
         ptrEXT++) {
        if (ptrEXT->supported) {
            (*ptrEXT->standard.GetFunc)((stateRec *)ptrEXT);
#if 0
            if ((err = glGetError()) != GL_NO_ERROR) {
                ogEnvLog(OG_LALWAYS,"At %s glGetError(0x%x,%s)\n",err,
                         ptrEXT->standard.valueString1,
                         ogLibGLError(err));
                if (ptrEXT->standard.valueString2)
                    ogEnvLog(OG_LALWAYS,"------ Parameter is %s\n",
                             ptrEXT->standard.valueString2);
            }
#endif
        }
    }
    checkEnables();
    checkTextureObjs();
    while ((err = glGetError()) != GL_NO_ERROR)
        ogEnvLog(OG_LALWAYS,"ERROR: After checks glGetError == 0x%x (%s)\n",
                 err, ogLibGLError(err));

    ogEnvLog(OG_LALWAYS,"||||||| END OF STATE CHECK |||||||\n");
}

static void
initState(void *gPtr, GLboolean isExtension) {
    int i;
    stateRec *ptr;
    GLfloat tmpBuf[STATEDATA_MAX_SIZE + 1];
    unsigned int *pInt;
    GLboolean firstError;
   
    if (isExtension) {
        stateRecEXT *ePtr = (stateRecEXT *) gPtr;
        ePtr->supported = (glGetString(GL_EXTENSIONS) &&
                           strstr((const char *)glGetString(GL_EXTENSIONS),
                                  ePtr->extensionString));
        if (!ePtr->supported)
            return;
        ptr = &ePtr->standard;
    } else
        ptr = (stateRec *) gPtr;
    
    for (i = ptr->dataCount, pInt = (unsigned int *) &ptr->DefaultData[i];
         i < STATEDATA_MAX_SIZE; i++, pInt++)
        *pInt = 0xdeadbeef;
    switch (ptr->dataType) {
      case STATEDATA_WINDOW_GET_TARGET:
        ptr->DefaultData[0] = ptr->DefaultData[1] = 0;
        /* The width and height depend on the actual window size and
           are therefore dynamically checked */
        break;
      case STATEDATA_STATE_GET_TARGET:
        /*
         * For implementation dependent values we sometimes need to check
         * that we meet the spec defined minimums.  For the cases where there
         * is no spec'ed minimums, the array will store values that are
         * guaranteed to be smaller than the implementation dependent values.
         *
         */
        for (i = 0, pInt = (unsigned int *) &tmpBuf[0];
             i < STATEDATA_MAX_SIZE + 1; i++, pInt++)
            *pInt = 0xdeadbeef;
        if (ptr->GetFunc == simpleGetCheck)
            glGetFloatv(ptr->value[0], tmpBuf);
        else if (ptr->GetFunc == getTargetParameterCheck ||
                 ptr->GetFunc == checkTexParam) {
#ifdef GL_SGIS_texture_filter4
            /*
             * For filter 4 we need to find out the size from the
             * implementation.
             */
            if (ptr->value[1] == GL_FILTER4_SGIS)
                glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_FILTER4_SIZE_SGIS,
                                    &ptr->dataCount);
#endif
            ((void (*) (GLenum, GLenum, GLfloat *)) ptr->SpecialGetFunc)
                (ptr->value[0], ptr->value[1], tmpBuf);
        } else if (ptr->GetFunc == getFuncCheck) {
            ((void (*) (GLenum, GLfloat *)) ptr->SpecialGetFunc)
                (ptr->value[0], tmpBuf);
        } else
            ogEnvLog(OG_LINTERNALERROR,
                     "Don't know how to get implemenation dependent state for %s %s\n",
                     ptr->valueString1, ptr->valueString2 ? ptr->valueString2 : "");
        firstError = GL_TRUE;
        for (i = 0; i < ptr->dataCount; i++) {
            if (tmpBuf[i] < ptr->DefaultData[i]) {
                if (firstError) {
                    if (ptr->valueString2 == NULL)
                        ogEnvLog(OG_LALWAYS, "ERROR: %s implemenation dependent \
state < min spec'ed\n", ptr->valueString1);
                    else
                        ogEnvLog(OG_LALWAYS,
                                 "ERROR: %s parameter of %s implemenation dependent \
state < min spec'ed\n",
                                 ptr->valueString2, ptr->valueString1);
                    firstError = GL_FALSE;
                }
                ogEnvLog(OG_LALWAYS,
                         "       %s[%d]: %g < %g (0x%08x < 0x%08x)\n",
                         (ptr->valueString2 != NULL ?
                          ptr->valueString2 : ptr->valueString1),
                         i, tmpBuf[i], ptr->DefaultData[i], 
                         *(GLuint *) &tmpBuf[i],
                         *(GLuint *) &ptr->DefaultData[i]);
            }
            ptr->DefaultData[i] = tmpBuf[i];
        }
        if (*(unsigned int *) &tmpBuf[ptr->dataCount] != 0xdeadbeef) {
            char msg[256];
            i = sprintf(msg, "ERROR: StateInit - Too many values return for \
initial state for ");
            if (ptr->valueString2 != NULL)
                i += sprintf(&msg[i], "%s parameter of ", ptr->valueString2);
            sprintf(&msg[i], "%s\n", ptr->valueString1);
            ogEnvLog(OG_LALWAYS, msg);
        }
        break;
    }
}

/*
** Fill in missing DefaultData array elements in in _state and 
** _stateEXT structure arrays
*/
void ogEnvInitStateChecker(void)
{
    stateRec *ptr;
    stateRecEXT *ptrExt;

    for (ptr = state; ptr->value[0] != OG_STATE_CHECK_NULL; ptr++)
        initState(ptr, GL_FALSE);
    for (ptrExt = stateEXT;  ptrExt->standard.value[0] != OG_STATE_CHECK_NULL;
         ptrExt++)
        initState(ptrExt, GL_TRUE);
}

/******************************************************************************/

/* Extension gets */

static void
getTargetParameterCheck(stateRec *ptrOrig)
{
    int i;
    GLfloat tmpBuf[STATEDATA_MAX_SIZE];
    stateRec  *ptr = &((stateRecEXT *) ptrOrig)->standard;

    for (i=0; i < STATEDATA_MAX_SIZE; i++)
	tmpBuf[i] = 9999;

    ((void (*) (GLenum, GLenum, GLfloat *)) ptr->SpecialGetFunc)
        (ptr->value[0], ptr->value[1], tmpBuf);
    checkErrorPrintMultiLine(ptr, tmpBuf, "");
}

/*
 * This hack is to allow for code compiled in 6.2+ trees to run on 5.3
 * systems.  The problem is that if we use the pointer to glGetTexFilterFuncSGIS
 * in the array, loading fails as it tries to resolve the reference.  However,
 * if it appears just as a function call rld does not get upset unless you
 * try to call an unresolve referece, which we will not when the extension is
 * not supported.
 */

static void
getTexFilterHack(GLenum target, GLenum filter, GLfloat *weights)
{
#ifndef WIN32
    glGetTexFilterFuncSGIS(target, filter, weights);
#endif
}

#if 0 /* As internal sizes can not be reset to 0 currently not used */
static void checkTexLevelExtParams(stateRec *ptr)
{
    GLfloat tmpBuf[4];
    int maxWidth;
    unsigned int level, maxLevel;
    char levelName[10];

    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxWidth );
    maxLevel = ffs(maxWidth);
    
    for (level = 0; level < maxLevel; level++) {
      glGetTexLevelParameterfv( ptr->value[0], level, ptr->value[1], tmpBuf);
      sprintf(levelName, "level %d ", level);
      checkErrorPrintMultiLine(ptr, tmpBuf, levelName);
    }
}
#endif /* 0 */

#ifdef GL_VERSION_1_1
static void nullPointerCheck(stateRec *ptr) {
    void *tmp;
    glGetPointerv(ptr->value[0], &tmp);
    if (tmp) {
	ogEnvLog(OG_LALWAYS, "ERROR: %s, parameter %s is not default\n", 
		 ptr->valueString1, ptr->valueString2);
	ogEnvLog(OG_LALWAYS, "ERROR: value(s) "); 
	ogEnvLog(OG_LALWAYS, "  (0x%p != 0x%p)\n", tmp, ptr->DefaultData[0]);
    }
}
#endif
