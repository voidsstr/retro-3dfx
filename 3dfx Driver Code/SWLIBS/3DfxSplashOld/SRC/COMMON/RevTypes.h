#ifndef __REVTYPES_H__
#define __REVTYPES_H__

typedef int RevBool;
#define REVFALSE 0
#define REVTRUE  1

/*
 * basic data types.
 */
typedef unsigned char   RevU8;
typedef signed   char   RevI8;
typedef unsigned short  RevU16;
typedef signed   short  RevI16;
typedef signed   long   RevI32;
typedef unsigned long   RevU32;
typedef int             RevBool;
typedef float           RevFloat;
typedef double          RevDouble;
typedef long            RevLong;

#ifdef USE_GLIDE3

typedef struct {
  float  sow;                   /* s texture ordinate (s over w) */
  float  tow;                   /* t texture ordinate (t over w) */  
  float  oow;                   /* 1/w (used mipmapping - really 0xfff/w) */
}  GrTmuVertex;

typedef struct
{
  float x, y, z;                /* X, Y, and Z of scrn space -- Z is ignored */
  float r, g, b;                /* R, G, B, ([0..255.0]) */
  float ooz;                    /* 65535/Z (used for Z-buffering) */
  float a;                      /* Alpha [0..255.0] */
  float oow;                    /* 1/W (used for W-buffering, texturing) */
  GrTmuVertex  tmuvtx[2];
} GrVertex;

#define GR_VERTEX_X_OFFSET              0
#define GR_VERTEX_Y_OFFSET              1
#define GR_VERTEX_Z_OFFSET              2
#define GR_VERTEX_R_OFFSET              3
#define GR_VERTEX_G_OFFSET              4
#define GR_VERTEX_B_OFFSET              5
#define GR_VERTEX_OOZ_OFFSET            6
#define GR_VERTEX_A_OFFSET              7
#define GR_VERTEX_OOW_OFFSET            8

#endif // USE_GLIDE3

#endif /* __REVTYPES_H__ */
