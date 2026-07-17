#ifndef __UDIAG_H__
#define __UDIAG_H__

/*
** Copyright (c) 1997, 3Dfx Interactive, Inc.
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
** $Revision: 2$
** $Date: 10/11/00 8:12:13 PM$
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <h3.h>
#include <fx64.h>
#include "../csim/h3sim.h"

#ifdef HAL_HSIM
#include "tstbench.h"
#endif

#if defined(_MSC_VER)
#define putenv _putenv
#endif

// define our own set of SET macros so we can accumulate CMD packets
#undef SET8
#undef SET16
#undef SET
#undef SETF

#define SET8(d,s)	diagStore8 (&(d),s)
#define SET16(d,s)	diagStore16(&(d),s)
#define SET(d,s)	diagStore32(&(d),s)
#define SETF(d,s)	diagStore32f(&(d),s)

void diagStore8 (volatile void *, FxU8);
void diagStore16(volatile void *, FxU16);
void diagStore32(volatile void *, FxU32);
void diagStore32f(volatile void *, FxFloat);

//These are shadow registers for the 3d registers
extern SstRegs shadowRegisters3D[32][1 + MAX_NUM_TMUS];


// maximum screen dimension in either direction (used to size arrays)
#ifdef CVG
#define MAXSCREEN 1024
#else
#define MAXSCREEN 2048
#endif

// this struct holds command line argument values
typedef struct diagopts {
    int adjust;		// sub-pixel adjustment
    FxBool bigAssTextures;   // Support for 2048x2048 textures
    int bilinear;	// binlinear filtering
    int clamp;		// s,t clamping
    int checkCrc;	// check CRC value
    int checkEveryTriangle;	// check results every triangle
    FxBool dontCheckEveryTriangle;  //This unforces to default of checkEveryTriangle
                                    //when running the csim alone 
    int columnOf8Align; // randomly align triangle vertices to column-of-8 boundaries
    FxBool compressedTextures;  //Should compressed textures be used?
    int curdrawbuffer;	// current drawbuffer setting
    int cmdFifoEnabled; // 1 if CMDFIFO currently enabled, 0 if disabled
    FxU32 cmdFifoAddress[2];  //Address of command FIFOs
    FxU32 cmdFifoTop;         //Address of last byte of command FIFO
    int drawbuffer;	// front(0)/back(1) buffer
    int diff;		// compare HSIM with CSIM
    int deviceID;	// device ID
    int directExec;	// direct Execute of CMD FIFO
    FxBool aaEnabled;   // This enables anti-aliasing
    FxU32  aaSampleCount;  //Number of subsamples to use for AA; should be 2 or 4
    FxBool sliEnabled;  // This enables Scan Line Interleaving(SLI)
    int errorCount;	// current error count
    int errorLimit;	// error limit before exit
    int flip;		// flip (MIRROR) textures
    int floatSTW;	// use quick CSIM floating point for STW
    FxU32 crc;    	// expected CRC value
    int whichFifo;	// which CMD FIFO to use
    int usePacket6;     // Use packet 6 
    int goodMask;	// good bitmask for pixel compares (signed!)
    int generateCrc;    // compute golden CRC
    int paranoid;       // check crc & memory
    int disableHoles;	// 
    char *infile_name;	// input filename
    int srcKey;		// source color key
    int dstKey;		// dest color key
    int lodbias;	// enable LOD bias
    int loddither;	// enable LOD dither
    int lstipple;	// enable line stipple
    int multiTexBaseAddr;// enable multiple texture base addresses
    int ncc;		// enable NCC texture compression
    int ringSize;	// CMD FIFO ring buffer size
    int oppositeBanks;  // place front/back buffer in opposite banks
    int option;         // general test option 
    int passes;		// number of test passes to run
    int perspective;	// perspective correct
    char *pgm_name;	// program name
    int pixelLimit;	// limits test to a number of pixels
    FxI32 log2BandHeight;   //For 2 pixel per clock rendering, determines each TMU's band height
    FxI32 pixelsPerClock;   //Determines how many pixels per clock are rendered
    int printOpts;      // flag to prompt for option printing
    int dumpPpm;	// dump ppm file at end (-q)
    int dumpMemory;	// dump memory file at end
    char *imgFilename;	// image filename to preload (-Q)
    int rectangular;	// allow rectangular textures
    int randomZA;	// use random ZA modes when texturing
    FxBool randomPlacement;// Place buffers and textures randomly
    FxBool randomCmdFifoPlacement; //Place command fifos anywhere in lower 16meg
    int rgb;		// RGB mode (always TRUE), 15,16,or 32
    int saveBeforeSwap; // save back buffer/memory before each swap
    int seed;		// initial random seed
    int sliBandHeight;  // SLI band height is the height the bands that each chip renders
    int chipCount;      // The number of Napalms to use; they can be either SLI'd or AA'd.
    FxI32 triColumnBand;  //Used to set the width of rendering bands
    int swizzle;        // byte/word swizzling (-1=random,0=none,1=word,10=byte,11=both)
    int tex8;		// 8-bit textures (default is 16)
    int tex32;          // 32-bit textures (default is 16). This takes precedence though
    int trex;		// which TREX chip to test
    int trexStandAlone;	// special hardware diag mode
    int tsize;		// random triangle size
    int tsplit;		// enable TSPLIT
    int dstFormat;	// -u destination format for 2d
    int updatexy;	// enable update XY bits
    int width;		// width of display window
    int writeFifo;	// write the command fifo
    int trexInit0;	// initial value for trexInit0
    int trexInit1;	// initial value for trexInit1
    int triple;         // enable triple buffering
    int ytiled;		// randomly use linear/tiled surfaces
    int vsync;		// wait for vsync
    int videoMaxFrame;  // number of frames for video test
    int videoTest;	// enable video testing
    int yorigin;	// enable old yorigin bits
    int zeroLodFrac;	// enable LOD_ZEROFRAC
    int fbiRevision;	// FBI revision number (to be tested)
    int trexRevision;	// TREX revision number (to be tested)
    int agpEnable;      // Use AGP for cmd fifos
    unsigned int vectorGenMask;  // Each bit turns on a different vector generation stream

    // the items below are not options, but configuration state
    int gui;		// 2d gui test
    int vid;		// video test
    int refreshRate;    // video refresh rate
    int xmaxscreen;	// width of framebuffer
    int ymaxscreen;	// height of framebuffer
    int hasAuxBuffer;	// is there an Aux buffer?

    //Stupid-ass old way to do texture allocation.
    //Kept in for backwards compatibility
    int texMemStart;    // minimum (floor) of texture memory (above 3d buffers)   
    int minTrashMem;	// minimum (floor) of trashable memory (above cmd fifos etc)
    int maxTrashMem;	// maximum (ceiling) trashable memory (below memory fifo etc)


    SstRegs *sst;	          // pointer to SST hardware (fake address)
    SstRegs *sstCSIM;	          // pointer to actual CSIM regs
    SstRegs *sstChildren[3];
    SstRegs *sstChildrenCSIM[3];
    HalInfo *halInfo;	// pointer to HAL info
} Diagopts;

// translation tables for decompressing NCC format
typedef struct {
    unsigned char yRGB[16];
    int iRGB[4][3];
    int qRGB[4][3];
} DiagNccTable;

typedef struct {
    long x,y;		// snapped 12.4 vertex
    float fx,fy;	// floating point (if applicable)
    long r,g,b,a;
    FxI64 z64;
    FxI64 s,t,w;
    float fr,fg,fb,fa,fz,fs,ft,fw,fs1,ft1,fw1;
} Vertex;

// a texture structure which holds all the info about the current
// texture we have downloaded into the system and are working with
typedef struct {
  FxU32 textureBaseAddress;  //Base address for entire set of mipmaps
  FxU32 mipmapBaseAddress;   //Base address of this specific mipmap
  FxU32 lod;                 //lod value for this mipmap 
  FxU32 width;
  FxU32 height;
  FxU32 *data;
  FxU32 nData;               //Number of allocated data 
} Mipmap;

typedef struct {
  int trex;		// which TREX chip I am loaded into
  int lodmin, lodmax;	// which LODs are defined; 8.2 format
  FxU32 tMode;		// hardware textureMode register
  FxU32 tLOD;			// hardware tLOD register
  FxU32 tDetail;		// hardware tDetail register
  FxU32 combMode;		// hardware combineMode register
  FxU32 cmask;		// chromakey mask (HACK)
  FxU32 tChromakey;
  FxU32 tChromarange;
  int tiled, tStride;         // for tiled texture memory
  DiagNccTable ncc;		// NCC table for compressed textures
  FxU32 palette[256];		// 8-bit LUT palette
  Mipmap *mip[16];
  FxU32 nMipmaps;
} Texture;

typedef struct triangle {
    int area;
    long drdx, dgdx, dbdx, dadx;
    FxI64 dzdx64;
    FxI64 dsdx, dtdx, dwdx;
    long drdy, dgdy, dbdy, dady;
    FxI64 dzdy64;
    FxI64 dsdy, dtdy, dwdy;
    float fdrdx, fdgdx, fdbdx, fdadx, fdzdx, fdsdx, fdtdx, fdwdx;
    float fdrdy, fdgdy, fdbdy, fdady, fdzdy, fdsdy, fdtdy, fdwdy;
    Vertex vA;
    Vertex vB;
    Vertex vC;
    struct triangle *next;
    Texture *tex;
    FxU32 currentSampleIndex;   //Used to add when checking multi-sample triangles
} Triangle;

#ifndef CVG
#define MAX_EDGES 32
#define MAX_SPANS MAXSCREEN
// XXX this stuff should all go into diags/lib if we write more than
// only poly diagnostic

typedef struct {
    int x,y;
} Edge;

typedef struct {
    int numVerts;			// number of vertices
    int nLeft, nRight;			// number of edges on left and right
    int xl,xr,yb,yt;			// bounding box
    FxU32 cmd;				// command used to draw it
    Edge leftEdges[MAX_EDGES];
    Edge rightEdges[MAX_EDGES];
    int spanArraysValidFlag;		// tells whether span*Check[] are valid
    int spanLeftCheck[MAX_SPANS];		// minimum
    int spanRightCheck[MAX_SPANS];
} Polygon;
#endif /* #ifndef CVG */


typedef struct diagframebuffer {
  FxU32 colBufferAddr[3];
  FxU32 colBufferStride[3];
  FxU32 auxBufferAddr;
  FxU32 auxBufferStride;

  //AA secondary Addresses
  FxU32 colBufferAddrSecondary[3];
  FxU32 auxBufferAddrSecondary;

  int inRegister;  // index of colBufferAddr/Stride[] currently in registers
} DiagFrameBuffer;



extern DiagFrameBuffer diagfb;

extern Diagopts diago;

extern void DIAG_BEGIN(int, char**);
extern int  DIAG_STARTPASS(void);
extern void DIAG_LOADIMAGE(char *imgFilename, int buffer, void (*setPixel)(int buf, int x, int y, int r, int g, int b) );
extern void DIAG_SAVEIMAGE(const char *filenameRoot, FxU32 buffer, 
				FxU32 width, FxU32 height, FxU32 imgType);
extern void DIAG_SAVEMEMORY(const char *filenameRoot);
extern int  DIAG_EXCEEDED_PIXEL_LIMIT(void);
extern void DIAG_PASS(int);
extern void DIAG_FAIL(void);
extern void DIAG_END(int);
extern int  DIAG_INCERROR(void);
extern void DIAG_EOS(void);
extern void DIAG_NOPS(void);
extern void DIAG_SWAPBUFFER(void);
extern void DIAG_SWAPBUFFER_EX(int swapInterval, int waitOnVsync, int dontSwap);
extern int  DIAG_TESTREG16(char *,FxU16,FxU16);
extern int  DIAG_TESTREG32(char *,FxU32,FxU32);
extern int  DIAG_TESTPORT32(char *,FxU32,FxU32);
extern int  DIAG_TEST_PIXEL(FxI32 buffer, int x, int y, FxU32 color);
extern int  DIAG_TEST_MEM(FxU32 addr, FxU32 cGood, int nbytes);
extern int  DIAG_TESTSWAP(void);
extern int  DIAG_TESTSCREEN(int xmax, int ymax, FxU32 c);
extern int  DIAG_DIFFSCREEN(int xmax, int ymax);
extern int  DIAG_DIFFMEMORY(void);
extern int  DIAG_TEST_CRC(FxU32 golden);
extern FxU32 DIAG_READ_CRC_COMPOSITE(void);
extern FxU32 DIAG_READ_CRC(void);
extern FxU32 DIAG_CRC_ITER(FxU32 crc, FxU32 data);
extern FxU32 DIAG_CALC_CRC(FxU32 base, FxU32 width, FxU32 height, FxU32 stride, FxU32 bpp);
extern FxU32 DIAG_CALC_CRC_COMPOSITE(void);
extern int  DIAG_TESTLFBREAD32(int x, int y, FxU32 cRead, FxU32 cGood);
extern int  DIAG_TESTLFBREAD16(int x, int y, FxU32 cRead, FxU32 cGood);
extern int  DIAG_COMPARE_PIXEL(char *which, FxI32 buffer, int x, int y,
				FxU32 cRead, FxU32 cGood);
extern int  DIAG_COMPARE_MEM(char *which, FxU32 addr, FxU32 cRead, FxU32 cGood);
extern void DIAG_FORCE_PIXEL(FxI32 buffer, int x, int y, FxU32 color);
extern void DIAG_FORCE_MEM(FxU32 addr, FxU32 data, int nbytes);
extern void DIAG_FORCE_RECT(FxI32 buffer, int x, int y, int w, int h, FxU32 color);
extern void DIAG_CHECKPIXSTATS(void);
extern  int DIAG_TIME(void);
extern void DIAG_IDLE(void);
extern void GUI_CHECK(void);

extern FxI32 diagTestMemory(FxI32 chipIndex, FxU32 addr, FxU32 cGood, FxI32 nBytes);
extern FxI32 diagTestPixel(FxI32 sampleIndex, FxI32 buffer, FxI32 x, FxI32 y, FxU32 color);

extern float fix2float(int *fixed, int fracbits);
extern int float2fix(float fdata, int fracbits);
extern FxI64 float2fix64(float fdata, int fracbits);
extern char *printFix(char *fmt, int val, int fracbits);
extern char *printFix64(char *fmt, FxI64 val, int fracbits);
void hb_selectFifo(int which);
void hb_restoreFifo(void);
void hb_resetAll(void);
extern void hb_histFlushAll(void);
extern void hb_moveCmd(  SstRegs *sst,
		 FxU32 sizeBytes,
		 FxU32 srcWidth,
		 FxU32 srcStride,
		 FxU32 *moveData,
		 FxU32 fbOffset,
		 FxU32 dstStride,
		 FxU32 space);
void printHistoryBuffer(FxU32 bufferIndex);

// these all come from random.c
extern void setSeed(unsigned long);
extern unsigned long getSeed(void);
extern unsigned long byteRandom(int);
extern unsigned long colRandom16(void);
extern unsigned long colRandom24(void);
extern FxU32 colRandom32(void);
extern double aRandom(void);
extern unsigned int iRandom(unsigned int);
extern int rRandom(int, int);
extern void xyRandom(long *x, long *y);
extern float fRandom(int);
extern float fexpRandom(int loExp, int hiExp);
extern float rfRandom(int,int);
extern FxU64 iRandom64(FxU64);
extern FxI64 rRandom64(FxI64,FxI64);
extern void scrambleRandom(int n, int *array);
extern void ckeyRandom565(int a, int b, int c, FxU32 csrc, FxU32 *ckey, FxU32 *crng);
extern void ckeyRandom888(int a, int b, int c, FxU32 csrc, FxU32 *ckey, FxU32 *crng);

extern void register32test(volatile void *reg, FxU32 maskRead, FxU32 maskWrite);

extern Texture* buildTexture(FxU32 width, FxU32 height);
extern Triangle* buildTriangle(FxU32 textureWidth, FxU32 textureHeight);
extern void copyTexture(Texture *destination, Texture *source);
extern void copyTriangle(Triangle *destination, Triangle *source);
extern void deleteTexture(Texture *texture);
extern void deleteTriangle(Triangle *triangle);

extern FxU32 getMipmapData(Texture *texture, FxU32 lod, FxU32 u, FxU32 v);
extern void setMipmapData(Texture *texture, FxU32 lod, FxU32 u, FxU32 v, FxU32 data);

extern void convertFromMicroTile(FxU32 textureMode, FxI32 *u, FxI32 *v);
extern void convertToMicroTile(FxU32 textureMode, FxI32 *u, FxI32 *v);
extern FxI32 roundDownDivide(FxI32 n, FxI32 divisor);

extern FxBool isMultiTexturing(void);
void diagAssert(char *expression, char *filename, int line);

#ifdef assert
#undef assert
#endif
#define assert(exp) (void)( (exp) || (diagAssert(#exp, __FILE__, __LINE__), 0) )

#endif /* !__UDIAG_H__ */
