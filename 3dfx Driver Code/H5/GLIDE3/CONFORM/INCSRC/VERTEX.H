/*
** Copyright (c) 1998, 3Dfx Interactive, Inc.
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
*/

/*
 * The vertex definition. Note that I added 'w' to the GrVertex
 * structure, which shifted the tmuvtx[] addresses down.
 */


#ifndef _VERTEX_H_
#define _VERTEX_H_

typedef struct {
  float  sow;         /* s texture ordinate (s over w) */
  float  tow;         /* t texture ordinate (t over w) */  
  float  oow;         /* 1/w (used mipmapping - really 0xfff/w) */
}  GrTmuVertex;

#define GRVERTEX_NUM_TMU 3
typedef struct
{
  float x, y;         /* X and Y in screen space */
  float ooz;          /* 65535/Z (used for Z-buffering) */
  float oow;          /* 1/W (used for W-buffering, texturing) */
  float r, g, b, a;   /* R, G, B, A [0..255.0] */
  float z;            /* Z is ignored */
  float w;            /* W (used when in clip coords) */
  GrTmuVertex  tmuvtx[GRVERTEX_NUM_TMU];
  float fog;          /* value used for FOGCOORD extension */
} GrVertex;

#define GR_VERTEX_X_OFFSET              0
#define GR_VERTEX_Y_OFFSET              1
#define GR_VERTEX_OOZ_OFFSET            2
#define GR_VERTEX_OOW_OFFSET            3
#define GR_VERTEX_R_OFFSET              4
#define GR_VERTEX_G_OFFSET              5
#define GR_VERTEX_B_OFFSET              6
#define GR_VERTEX_A_OFFSET              7
#define GR_VERTEX_Z_OFFSET              8
#define GR_VERTEX_W_OFFSET              9 

#define GR_VERTEX_SOW_TMU0_OFFSET       10 
#define GR_VERTEX_TOW_TMU0_OFFSET       11
#define GR_VERTEX_OOW_TMU0_OFFSET       12

#define GR_VERTEX_SOW_TMU1_OFFSET       13
#define GR_VERTEX_TOW_TMU1_OFFSET       14
#define GR_VERTEX_OOW_TMU1_OFFSET       15

#define GR_VERTEX_SOW_TMU2_OFFSET       16
#define GR_VERTEX_TOW_TMU2_OFFSET       17
#define GR_VERTEX_OOW_TMU2_OFFSET       18

#define GR_VERTEX_FOG_OFFSET             19

#define MAX_TMU 16

typedef struct {
    int min_x, max_x, min_y, max_y;
} BBox;

typedef struct {
    double a, r, g, b;             /* per iteration fragment A, R, G, B  */
    double s, t, q;                /* per iteration fragment, s, t, q */
    double da, dr, dg, db;
    double ds, dt, dq;
} TmuFragment;

typedef struct {
    double x, y, z, q;             /* X, Y, Z and Q */
    double w;                      /* w */
    double f;                      /* fog coordinate */
    double dz, dq, dw, df;
    TmuFragment tmu[MAX_TMU];
} Fragment;

typedef struct {
    float a, r, g, b;             
    float s, t, q;                
    FxU32 argb;                   /* packed argb */
} TmuVertex;

typedef struct {
    float x, y, z, q;             
    float w;                      
    float f;                      /* fog coordinate */
    TmuVertex tmu[MAX_TMU];
} Vertex;

typedef struct {
    double drdx, dgdx, dbdx, dadx;
    double dsdx, dtdx, dqdx;
    double drdy, dgdy, dbdy, dady;
    double dsdy, dtdy, dqdy;
} TmuGradient;

typedef struct {
    double dzdx, dqdx, dwdx, dfdx; 
    double dzdy, dqdy, dwdy, dfdy; 
    TmuGradient tmu[MAX_TMU];
} Gradient;

typedef struct {
    double dedx;
    double dedy;
    double c;
    double scale;
} Edge;

#ifndef BIT
#define BIT(n)  (1UL<<(n))
#endif

typedef FxU32 VertexMode;      /* controls interpretaion of vertex */
#define VERTEX_CLIP_COORDS     BIT(0)  /* vertex is in clip coordinates */
#define VERTEX_Z               BIT(1)  /* vertex includes z */
#define VERTEX_W               BIT(2)  /* vertex includes w */
#define VERTEX_Q               BIT(3)  /* vertex includes q */
#define VERTEX_FOG             BIT(4)  /* vertex includes fog */

typedef struct {
    VertexMode mode;      /* controls interpretaion of vertex */ 
    FxU32 aTMUMode;       /* a float (1 bit per iteration) */
    FxU32 rgbTMUMode;     /* rgb float (1 bit per iteration) */
    FxU32 pargbTMUMode;   /* pargb byte (1 bit per iteration) */
    FxU32 stTMUMode;      /* st float (1 bit per iteration) */
    FxU32 qTMUMode;       /* q float (1 bit per iteration) */
} VertexInfo;

#define X_MAJOR 0  // x major line
#define Y_MAJOR 1  // y major line
#define N_MAJOR 2  // degenerate line ( vertices coincide )

typedef struct {
    Vertex vA;
    Vertex vB;
    int major;
    double dedx;
    double dedy;
    double c;
    double error;
    BBox bbox;
    double dzdp, dqdp, dwdp, dfdp; 
    struct {
      double drdp, dgdp, dbdp, dadp;
      double dsdp, dtdp, dqdp;
    } tmu[MAX_TMU];
} Line;

typedef struct {
    Vertex vA;
    Vertex vB;
    Vertex vC;
    double area;
    double ooarea;
    Edge  eAB;
    Edge  eAC;
    Edge  eBC;
    BBox bbox;
    Gradient grd;
    double dAB, dAC, dBC; // vaulue of error term for last point checked
} Triangle;

#endif //_VERTEX_H_
