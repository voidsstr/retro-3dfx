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
** $Date: 10/11/00 7:34:48 PM$ 
**
*/

#ifndef _ATSCENEP_H_
#   define _ATSCENEP_H_

#include <atscene.h>

#define CHECK(_c) {if (!(_c))\
     {atuError(FXTRUE, "FILE I/O Error: %s: line %d\n",__FILE__,__LINE__); \
      return(FXFALSE);}}

extern AtsType *_ats_object_type ;
extern AtsType *_ats_node_type ;
extern AtsType *_ats_material_type ;
extern AtsType *_ats_texture_type ;
extern AtsType *_ats_image_type ;
extern AtsType *_ats_shape_type ;
extern AtsType *_ats_geometry_type ;
extern AtsType *_ats_dict_type ;
extern AtsType *_ats_frame_type ;
extern AtsType *_ats_group_type ;
extern AtsType *_ats_lod_type ;
extern AtsType *_ats_seq_type ;

#define VALIDATE_NODE(np, op, func) \
               ATS_VALIDATE_NODE(np, op, AtsNode, _ats_node_type, func)

#define VALIDATE_GROUP(gp, np, func) \
                    ATS_VALIDATE_NODE(gp, np, AtsGroup, _ats_group_type, func)

#define VALIDATE_LOD(sp, np, func) \
                    ATS_VALIDATE_NODE(sp, np, AtsLOD, _ats_lod_type, func)

#define VALIDATE_MATERIAL(_mp, _op, func) \
               ATS_VALIDATE_NODE(_mp, _op, AtsMaterial, _ats_material_type, func)

#define VALIDATE_TEXTURE(gp, np, func) \
               ATS_VALIDATE_NODE(gp, np, AtsTexture, _ats_texture_type, func)

#define VALIDATE_IMAGE(ip, op, func) \
               ATS_VALIDATE_NODE(ip, op, AtrImg, _ats_image_type, func)

#define VALIDATE_SHAPE(gp, np, func) \
                    ATS_VALIDATE_NODE(gp, np, AtsShape, _ats_shape_type, func)

#define VALIDATE_GEOMETRY(gp, np, func) \
                    ATS_VALIDATE_NODE(gp, np, AtsNode, _ats_geometry_type, func)

#define VALIDATE_TRISET(gp, np, func) \
                ATS_VALIDATE_NODE(gp, np, AtrTriSet, _ats_triset_type, func)

#define VALIDATE_CMESH(_cm, _o, func) \
                ATS_VALIDATE_NODE(_cm, _o, AtmCMesh, _ats_cmesh_type, func)

#define VALIDATE_DICT(gp, np, func) \
                    ATS_VALIDATE_NODE(gp, np, AtsDict, _ats_dict_type, func)

#define VALIDATE_FRAME(gp, np, func) \
                    ATS_VALIDATE_NODE(gp, np, AtsFrame, _ats_frame_type, func)

#define VALIDATE_SEQ(sp, np, func) \
                    ATS_VALIDATE_NODE(sp, np, AtsSeq, _ats_seq_type, func)

FxBool atsImageStore(AtsObject *obj, FILE *stream);

FxBool atsImageLoad(AtsObject *obj, FILE *stream);

void atsInitRTG(void);

AtsObject *atsLoadFromRTG(const char *filename );

void atsTermRTG(void);

void atsInitJBY(void);

AtsObject *atsLoadFromJBY(const char *filename );

void atsTermJBY(void);

void atsInitVQ(void);

AtsObject *atsLoadFromVQ(const char *filename );

void atsTermVQ(void);

void atsInit3DS(void);

AtsObject *atsLoadFrom3DS( const char *_tds_filename );

void atsTerm3DS(void);

void atsInitRad(void);

AtsObject *atsLoadFromRad(const char *filename);

void atsTermRad(void);

AtsObject *atsLoadFromBOF(const char *filename);

FxBool atsStoreToBOF(const char *fileName, AtsObject *node);

void atsInitBOF(void);

void atsTermBOF(void);

FxBool _atsInitConverters(void);

FxBool _atsTermConverters(void);

void _atsBboardInitClass(void);
void _atsLODInitClass(void);
void _atsSeqInitClass(void);
void _atsCMeshInitClass(void);
void _atsTextureInitClass(void);
void _atsImageInitClass(void);
void _atsMaterialInitClass(void);
void _atsTriSetInitClass(void);
void _atsGeometryInitClass(void);
void _atsShapeInitClass(void);
void _atsAnimInitClass(void);
void _atsDictInitClass(void);
void _atsGroupInitClass(void);
void _atsNodeInitClass(void);
void _atsObjectInitClass(void);
void _atsFrameInitClass(void);
FxBool _atsTextureInit(void);
void _atsTextureTerm(void);
void _atsInitXform(void);
void _atsShutdownXform( void );

#ifdef AT_LOADER_MG
void atsInitFlt(void);
AtsObject *atsLoadFromFlt(const char *fileName);
void atsTermFlt(void);
#endif

#define MAGIC(_a, _b, _c, _d) (((_a)<<24)|((_b)<<16)|((_c)<<8)|(_d))
#define BOF_MAGIC MAGIC('B', 'O', 'F', 'M')
#define BOF_VERSION 02 /* last digit is minor version number */

ENDFDECL()
#endif
