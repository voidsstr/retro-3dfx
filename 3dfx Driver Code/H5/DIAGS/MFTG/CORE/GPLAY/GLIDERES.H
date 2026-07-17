/* Structures and constants copied from glide implementation */

#if !defined(glideres__INCLUDED_)
#define glideres__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/* Types of data in strips */
#define GR_FLOAT        0
#define GR_U8           1

/* Parameters for strips */
/* Parameters for strips */
#define GR_PARAM_XY       0x01
#define GR_PARAM_Z        0x02
#define GR_PARAM_W        0x03
#define GR_PARAM_Q        0x04
#define GR_PARAM_FOG_EXT  0x05

#define GR_PARAM_A        0x10

#define GR_PARAM_RGB      0x20

#define GR_PARAM_PARGB    0x30

#define GR_PARAM_ST0      0x40
#define GR_PARAM_ST1      GR_PARAM_ST0+1
#define GR_PARAM_ST2      GR_PARAM_ST0+2

#define GR_PARAM_Q0       0x50
#define GR_PARAM_Q1       GR_PARAM_Q0+1
#define GR_PARAM_Q2       GR_PARAM_Q0+2

#define GR_PARAM_DISABLE  0x00
#define GR_PARAM_ENABLE   0x01



/* from ditex.c */
/* vertex layout sizes */
/* GR_PARAM_XY: */
#define SIZE_vertexInfo	2

/* GR_PARAM_Z: */
#define SIZE_zInfo		1

/* GR_PARAM_W: */
#define SIZE_wInfo		1

/* GR_PARAM_FOG_EXT: */
#define SIZE_fogInfo	1

/* GR_PARAM_A: */
#define SIZE_aInfo		1
    
/* GR_PARAM_RGB: */
#define SIZE_rgbInfo	3
    
/* GR_PARAM_PARGB: */
#define SIZE_pargbInfo  1
    
/* GR_PARAM_ST0: */
#define SIZE_st0Info	2
    
/* GR_PARAM_ST1: */
#define SIZE_st1Info	2
    
/* GR_PARAM_Q: */
#define SIZE_qInfo		1
    
/* GR_PARAM_Q0: */
#define SIZE_q0Info		1
    
/* GR_PARAM_Q1: */
#define SIZE_q1Info		1


/* copied from FXGlide.h */
	typedef struct {
  FxU32
    mode;                       /* enable / disable */
  FxI32
    offset;                     /* offset to the parameter data */
} GrVParamInfo;

// copied from FXGlide.h
	typedef struct  {
  GrVParamInfo
    vertexInfo,          /* xy */
    zInfo,               /* z(ooz) */
    wInfo,               /* w(oow) */
    aInfo,               /* a float */
    fogInfo,             /* fog */
    rgbInfo,             /* rgb float */
    pargbInfo,           /* pargb byte */
    st0Info,             /* st0 */
    st1Info,             /* st1 */
    qInfo,               /* q */
    q0Info,              /* q0 */
    q1Info;              /* q1 */
  FxU32
    vStride,             /* vertex stride */
    vSize;               /* vertex size */
  FxU32
   colorType;           /* float or byte */
} GrVertexLayoutInfo;

#endif