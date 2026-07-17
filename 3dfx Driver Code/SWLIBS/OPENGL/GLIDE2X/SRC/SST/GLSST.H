#ifndef __gl_sstvertex_h
#define __gl_sstvertex_h

struct __GLSSTtmuVertexRec {
  float s, t, w;
};

typedef struct __GLSSTtmuVertexRec __GLSSTtmuVertex;

struct __GLSSTvertexRec {
  float x, y, z;
  float r, g, b;
  float ooz;
  float a;
  float oow;
  __GLSSTtmuVertex tmuvtx[2];
};

typedef struct __GLSSTvertexRec __GLSSTvertex;


typedef struct _IVert {
    float x;            // 0  Post projection, usable directly by Glide
    float y;            // 4
    float z;            // 8  Unused both by GL and Glide
    float r;            // 12
    float g;            // 16
    float b;            // 20
    float ooz;          // 24 The real z used by Glide for zbuffering
    float a;            // 28
    float oow;          // 32

    float t0_sow;       // 36 Always sow * oow above
    float t0_tow;       // 40 Always tow * oow above
    float t0_oow;       // 44

    float t1_sow;       // 48
    float t1_tow;       // 52
    float t1_oow;       // 56

    unsigned flags;     // 60 clip flags.
    float X;            // 64 in homogeneous coords, this and below
    float Y;            // 68
    float Z;            // 72
    float W;            // 76, for a total of 80 bytes.
} IVert;

typedef struct {
  float s;
  unsigned long sBias;
  float t;
  unsigned long tBias;
} TexCoordData;

typedef struct {
    float r, g, b, a;
} Color;

#define MAX_TEXTURE_UNITS 2

typedef struct {
    Color        clearColor;
    Color        color;
    TexCoordData tc[MAX_TEXTURE_UNITS];
    IVert        vertex [3];
    IVert        *vA, *vB, *vC;
    long         vState;
    int          interpolate;
} SSTPrimState;

#endif
