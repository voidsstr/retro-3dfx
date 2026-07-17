/* Structures and constants copied from glide implementation */

#if !defined(glideres__INCLUDED_)
#define glideres__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/* from ditex.c */
static const int _grMipMapHostWH[(0x3-GR_ASPECT_LOG2_1x8)+1][(0x8-GR_LOD_LOG2_1)+1][2] =
{
   { 
     { 256 , 32 }, 
     { 128 , 16 }, 
     { 64  , 8 }, 
     { 32  , 4 }, 
     { 16  , 2 }, 
     { 8   , 1 }, 
     { 4   , 1 }, 
     { 2   , 1 }, 
     { 1   , 1 }
   },
   { 
     { 256 , 64 }, 
     { 128 , 32 }, 
     { 64  , 16 }, 
     { 32  , 8 }, 
     { 16  , 4 }, 
     { 8   , 2 },
     { 4   , 1 }, 
     { 2   , 1 }, 
     { 1   , 1 }
   } ,
   { 
     { 256 , 128 }, 
     { 128 , 64 }, 
     { 64  , 32 }, 
     { 32  , 16 },
     { 16  , 8 },
     { 8   , 4 }, 
     { 4   , 2 }, 
     { 2   , 1 }, 
     { 1   , 1 }
   },
   { 
     { 256 , 256 }, 
     { 128 , 128 }, 
     { 64  , 64 },
     { 32  , 32 },
     { 16  , 16 },
     { 8   , 8 }, 
     { 4   , 4 }, 
     { 2   , 2 }, 
     { 1   , 1 }
   },
   { 
     { 128, 256 },
     { 64,  128 },
     { 32,  64  },
     { 16,  32  },
     { 8,   16  }, 
     { 4,   8   }, 
     { 2,   4   }, 
     { 1,   2   }, 
     { 1,   1 }
   },
   {
     { 64,  256 },
     { 32,  128 },
     { 16,  64  },
     { 8,   32  },
     { 4,   16  },
     { 2,   8   }, 
     { 1,   4   }, 
     { 1,   2   }, 
     { 1,   1 }
   },
   { 
     { 32,  256 },
     { 16,  128 },
     { 8,   64  },
     { 4,   32  },
     { 2,   16  },
     { 1,   8   }, 
     { 1,   4   }, 
     { 1,   2   },
     { 1,   1 }
   }
};

/* size in texels  */
/* from ditex.c */
static const FxU32 _grMipMapHostSize[4][16] = 
{
  {                             /* 1:1 aspect ratio */
    65536,                      /* 0 : 256x256  */
    16384,                      /* 1 : 128x128  */
    4096,                       /* 2 :  64x64   */
    1024,                       /* 3 :  32x32   */
    256,                        /* 4 :  16x16   */
    64,                         /* 5 :   8x8    */
    16,                         /* 6 :   4x4    */
    4,                          /* 7 :   2x2    */
    1,                          /* 8 :   1x1    */
  },
  {                             /* 2:1 aspect ratio */
    32768,                      /* 0 : 256x128  */
    8192,                       /* 1 : 128x64   */
    2048,                       /* 2 :  64x32   */
    512,                        /* 3 :  32x16   */
    128,                        /* 4 :  16x8    */
    32,                         /* 5 :   8x4    */
    8,                          /* 6 :   4x2    */
    2,                          /* 7 :   2x1    */
    1,                          /* 8 :   1x1    */
  },
  {                             /* 4:1 aspect ratio */
    16384,                      /* 0 : 256x64   */
    4096,                       /* 1 : 128x32   */
    1024,                       /* 2 :  64x16   */
    256,                        /* 3 :  32x8    */
    64,                         /* 4 :  16x4    */
    16,                         /* 5 :   8x2    */
    4,                          /* 6 :   4x1    */
    2,                          /* 7 :   2x1    */
    1,                          /* 8 :   1x1    */
  },
  {                             /* 8:1 aspect ratio */
    8192,                       /* 0 : 256x32   */
    2048,                       /* 1 : 128x16   */
    512,                        /* 2 :  64x8    */
    128,                        /* 3 :  32x4    */
    32,                         /* 4 :  16x2    */
    8,                          /* 5 :   8x1    */
    4,                          /* 6 :   4x1    */
    2,                          /* 7 :   2x1    */
    1,                          /* 8 :   1x1    */
  }
}; 

/* from ditex.c */
static const FxU32 _gr_aspect_index_table[] =
{
   3,
   2,
   1,
   0,
   1,
   2,
   3,
};


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