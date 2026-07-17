/**************************************************************************
 *									  *
 * 	 Copyright (C) 1996 Silicon Graphics, Inc.			  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *									  *
 **************************************************************************/

#define OGTST_LIGHT				0x11111111
#define OGTST_MATERIAL				0x22222222
#define OGTST_MODEL				0x33333333

#define OGTST_LIGHT_AMBIENT			0x00000001
#define OGTST_LIGHT_DIFFUSE			0x00000002
#define OGTST_LIGHT_SPECULAR			0x00000004
#define OGTST_LIGHT_POSITION			0x00000008
#define OGTST_LIGHT_SPOT_DIRECTION		0x00000010
#define OGTST_LIGHT_SPOT_EXPONENT		0x00000020
#define OGTST_LIGHT_SPOT_CUTOFF			0x00000040
#define OGTST_LIGHT_CONSTANT_ATTENUATION	0x00000080
#define OGTST_LIGHT_LINEAR_ATTENUATION		0x00000100
#define OGTST_LIGHT_QUADRATIC_ATTENUATION	0x00000200

#define OGTST_MATERIAL_AMBIENT			0x00001000
#define OGTST_MATERIAL_DIFFUSE			0x00002000
#define OGTST_MATERIAL_SPECULAR			0x00004000
#define OGTST_MATERIAL_EMISSION			0x00008000
#define OGTST_MATERIAL_SHININESS		0x00010000
#define OGTST_MATERIAL_AMBIENT_AND_DIFFUSE	0x00020000
#define OGTST_MATERIAL_COLOR_INDEXES		0x00040000

#define OGTST_MODEL_AMBIENT			0x00100000
#define OGTST_MODEL_LOCAL_VIEWER		0x00200000
#define OGTST_MODEL_TWO_SIDE			0x00400000

#define OGTST_DEBUG_MASK			0xffffffff

int		backfacing(void);
void		reset_all(void);
unsigned int	LightVertex(int);
void		ogtst_lmgen(int,int,int);
void		dumpster(unsigned int,int,int);
void		eye_init(void);
void		object_init(int);
void		enable(int);
void		disable(int);

