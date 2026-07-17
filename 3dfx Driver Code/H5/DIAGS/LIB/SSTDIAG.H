#ifndef __SSTDIAG_H__
#define __SSTDIAG_H__

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
** $Date: 10/11/00 8:12:02 PM$
*/

// some useful defines
#define XY_ONE (1<<SST_XY_FRACBITS)
#define XY_HALF (1<<(SST_XY_FRACBITS-1))
#define MAX(x,y) ((x)>(y)?(x):(y))
#define MIN(x,y) ((x)<(y)?(x):(y))
#define CEIL(x,y) ( ((x)+(y)-1) / (y) )  // divide x/y, rounding up
#define FLOOR(x,y) ( (x)/(y) )           // divide x/y, truncating

// a bit slow, but it makes it easy to write the code
// these first two have to deal with the integer registers
#define SST_Z_FRACBITS (diago.rgb==32 ? SST_Z_32BPP_FRACBITS : SST_Z_16BPP_FRACBITS)
#define SST_Z_INTBITS (diago.rgb==32 ? SST_Z_32BPP_INTBITS : SST_Z_16BPP_INTBITS)
// this one deals with the internal 48-bit register
#define SST_Z64_FRACBITS (diago.rgb==32 ? SST_Z64_FRACBITS_32BPP : SST_Z64_FRACBITS_16BPP)

#define FOG_TABLE_SIZE 64

/*
**  Here's the stuff to do P6 Fencing.  This is required for the
**  certain things on the P6
*/
#if defined(__MSC__)
  #define P6FENCE {FxU32 p6FenceVar; _asm xchg eax, p6FenceVar}
#elif defined(__unix__)
  #define P6FENCE
#else
  #error "P6 Fencing in-line assembler code needs to be added for this compiler"
#endif


extern void inhibitTwoPixelsPerClock(void);
extern FxBool isTwoPixelsPerClockInhibited(void);
FxU32 sstBitsPerTexel(FxU32 textureMode);
FxU32 calculateTiledTexturePortAddress(FxU32 textureMode, FxU32 tLOD, FxU32 lod, FxU32 s, FxU32 t, FxU32 wordSelect);

long sstDownLoadTexture(SstRegs *, int trex, long mipmapBaseAddress, 
			int tiled, int tStride,
			unsigned long texmode,
			int ar,	int slod, int tlod, int bpt, FxU32 tLOD, unsigned long *data);

void sstTiledToXY(FxU32 addr, FxU32 base, FxU32 tStride, FxU32 depth, int *x, int *y, int *rem);
void sstBoundingBox(int amin, int amax, int tStride, int depth, 
		    int *base, int *xmax, int *ymax, int *r);
void sstAlloc2DFrameBuffers(SstRegs *sst);
void sstAlloc3DFrameBuffers(SstRegs *sst);
void sstInitVideoOverlay(
    SstRegs *sst,     
    FxU32 enable,		// 1=enable Overlay surface (OS), 1=disable
    FxU32 stereo,		// 1=enable OS stereo, 0=disable
    FxU32 horizScaling,		// 1=enable horizontal scaling, 0=disable
    FxU32 dudx,			// horizontal scale factor (ignored if not scaling)
    FxU32 verticalScaling,	// 1=enable vertical scaling, 0=disable
    FxU32 dvdy,			// vertical scale factor (ignored if not scaling)
    FxU32 filterMode,		// duh
    FxU32 tiled,		// 0=OS linear, 1=tiled
    FxU32 pixFmt,		// pixel format of OS
    FxU32 clutBypass,		// bypass clut for OS?
    FxU32 clutSelect,		// 0=lower 256 CLUT entries, 1=upper 256
    FxU32 startAddress,		// board address of beginning of OS
    FxU32 stride,		// stride
    FxU32 width);               // overlay width in bytes
void sstInitVideoDesktop(
    SstRegs *sst,
    FxU32 enable,		// 1=enable desktop surface (DS), 1=disable
    FxU32 tiled,		// 0=DS linear, 1=tiled
    FxU32 pixFmt,		// pixel format of DS
    FxU32 clutBypass,		// bypass clut for DS?
    FxU32 clutSelect,		// 0=lower 256 CLUT entries, 1=upper 256
    FxU32 startAddress,		// board address of beginning of DS
    FxU32 stride);		// stride


int sstRollFwdBufId(int buf);
// machine dependent ONSCREEN check
// this eventually needs to handle different screen sizes
#define ONSCREEN(x,y) ((x)>=0 && (x)<diago.xmaxscreen && (y)>=0 && (y)<diago.ymaxscreen)
#define ONSCREEN_FRAC(x,y) ((x)>=0 && (x)<diago.xmaxscreen*XY_ONE && (y)>=0 && (y)<diago.ymaxscreen*XY_ONE)
typedef int (* PFNTRICHECK)(Triangle *, int,int,unsigned long);

extern SstRegs *SST_BEGIN(int, char**);
extern SstRegs *SST_BEGIN2d(int, char**);

extern FxU32 lfbOffset(FxU32 x, FxU32 y);
extern int sst_drawpixel(SstRegs *, int x, int y, unsigned long color, int lfbOK);
extern void sst_drawspan(SstRegs *, int x, int y, int spanlen);
extern void sst_setclip(SstRegs *, unsigned long x1, unsigned long y1,
				   unsigned long x2, unsigned long y2);

extern void sst96_exclclip(SstRegs *, unsigned long x1, unsigned long y1,
				   unsigned long x2, unsigned long y2);
extern void sst_idle(SstRegs *);
extern void sst_idle_really(SstRegs *);
extern void sst_vsync_active(SstRegs *);
extern void sst_vsync_inactive(SstRegs *);
extern unsigned long sst_dit565_2(unsigned long col, int x, int y);
extern unsigned long sst_dit565_4(unsigned long col, int x, int y);
extern unsigned long wBufferValue( unsigned long X );
extern unsigned long wFloat64(FxI64 oow);
extern unsigned long drawbufferRandom(void);
extern int goodAlphaMode(FxU32 amode, FxU32 fbzMode);
extern int goodCcuPath(unsigned long ccu, int fbiOnly);
extern int goodLfbMode(unsigned long lm);
extern int lfbAuxCheck(FxU32 lmode);
extern void sst_random_fog_table(SstRegs *, FxU32 fogTable[]);
extern FxU16 sst_random_fog_table_entry(void);
extern FxU32 sst_argb_form_result(FxU32 c, int endit, int x, int y);
extern void setBand(SstRegs *sst);
extern void setPixelsPerClock(SstRegs *sst);
extern void texturingSetPixelsPerClock(SstRegs *sst, FxI32 activeTMU);
extern void sstYab422to8888(DiagNccTable *ncc, unsigned char *c888, unsigned char __yiq);
extern void sstAi44to8888 (unsigned char c8888[4], unsigned char c44);
extern void sstRgba332to8888 (unsigned char c8888[4], unsigned char c332);
extern void sstRgba565to8888 (unsigned char c8888[4], unsigned short c565);
extern void sstRgba555to8888 (unsigned char c8888[4], unsigned short c555);
extern void sstRgba1555to8888 (unsigned char c8888[4], unsigned short c1555);
extern void sstRgba4444to8888 (unsigned char c8888[4], unsigned short c4444);
extern void sstRgba6666to8888 (unsigned char c8888[4], unsigned long c6666);
extern int sstDit5(int,int);
extern int sstDit6(int,int);
extern void fastfill_bug_workaround(SstRegs *sst, FxU32 fbzMode,
			     FxU32 zaColor, FxU32 c1,
			     FxU32 auxBufferAddr, FxU32 auxBufferStride,
			     FxU32 colBufferAddr, FxU32 colBufferStride,
			     unsigned int fastfill_cmd);

//----------------------------------------------------------------------
// the following are from ssttri.c
extern void randomTriangle(Triangle *t, int size, int onscreen);
extern void randomTriangle1(Triangle *t, int size, int onscreen);
extern void randomStressTriangle(Triangle *t, int size, int onscreen, int type);
extern void randomFloatTriangle(Triangle *t, int size, int onscreen);
extern void randomRgbaTriangle(Triangle *t);
extern void randomFloatRgbaTriangle(Triangle *t);
extern void randomZTriangle(Triangle *t);
extern void randomFloatZTriangle(Triangle *t);
extern void randomFloatWTriangle(Triangle *t);
extern void randomFloatStwTriangle_Setup(Triangle *t);
extern void areaTriangle(Triangle *t);
extern void printTriangle(int level, Triangle *t, int RGB, int A, int Z);
extern void printTriangleSlopes(int level, Triangle *t, int RGB, int A, int Z);
extern void setupTriangle(Triangle *t, int RGB, int A, int Z);
extern void setupFloatTriangle(Triangle *t, int RGB, int A, int Z, int W);
extern  int sortTriangle(Triangle *t);
extern void drawTriangle(SstRegs *sst, Triangle *t, int RGB, int A, int Z);
extern void drawAltTriangle(SstRegs *sst, Triangle *t, int RGB, int A, int Z);
extern void drawFloatTriangle(SstRegs *sst, Triangle *t, int RGB, int A, int Z, int W);
extern void drawAltFloatTriangle(SstRegs *sst, Triangle *t, int RGB, int A, int Z, int W);
extern void eraseTriangle(SstRegs *sst, Triangle *t, int RGB, int A, int Z);
extern  int insideTriangle(Triangle *t, int x, int y, unsigned long col);
extern void checkTriangle(Triangle *t, unsigned long csrc, int slop, int subP,
				PFNTRICHECK, int RGB, int A, int Z);
extern void align8Fixed(long *x);
extern void align8Float(float *fx);

//----------------------------------------------------------------------
// the following are from tsumath.c
void tsu_clamp(FxU32 *value);
void tsu_floatsub (FxU32 a, FxU32 b, FxU32 *s);
void tsu_floatmul(FxU32 a, FxU32 b, FxU32 *p);
void tsu_floatrecip(FxU32 in, FxU32 *out);

//----------------------------------------------------------------------
// the following are from sstgdiag.c
#ifdef CVG
void sstg_init_random_screen(SstRegs *sstg, FxU32 screen[MAXSCREEN][MAXSCREEN]);
void sstg_init_random_memory(SstRegs *sstg, FxU16 *mem, int size);
void sstg_idle(SstRegs *sst);
void sstg_random_command_bits(FxU32 *command);
void sstg_print_stuff(FxU32 command, FxU32 rop, FxU32 cFore, FxU32 cBack);
int sstg_colorkey(FxU32 col, FxU32 min, FxU32 max);
int sstg_random_colors(SstRegs *sstg, FxU32 *cfore, FxU32 *cback, FxU32 cdest);
FxU32 sstg_pick_rop(FxU32 cmdops, FxU32 rop, FxU32 csrc, FxU32 cdst);
FxU32 sstg_rop2(FxU32 rop, FxU32 src, FxU32 dst);
int sstg_clipped(int x, int y);
FxU32 sstg_destination_mask(FxU32 color);
void sstg_drawblt(SstRegs *sstg, int xs, int ys, int xd, int yd, int w, int h, FxU32 cmd);
void sstg_drawrect(SstRegs *sstg, int x, int y, int w, int h, FxU32 cmd);
FxU32 sstg_check_pixel(long xc, long yc, FxU32 cmdops, FxU32 rop, FxU32 csrc, FxU32 cdest);

#else

typedef struct {
    FxU32 cmd;
    int x,y,err, dx,dy;
    int xinc1, xinc2;
    int yinc1, yinc2;
    int einc1, einc2;
} DiagBresEdge;

FxU8 *sstg_alloc_shadow_memory();
void sstg_init_random_memory(FxU8 *mem);
void sstg_init_random_screen(SstGRegs *sstg, FxU32 screen[MAXSCREEN][MAXSCREEN]);
void sstg_idle(SstRegs *sst);
void sstg_random_command_bits(FxU32 *command, FxU32 *commandX);
void sstgCheckForIdle(SstRegs *sst, FxU32 commandExtra);
void sstg_print_stuff(FxU32 command, FxU32 commandX, 
			FxU32 rop, FxU32 srcFormat, FxU32 cFore, FxU32 cBack);
int sstg_src_colorkey(FxU32 col);
int sstg_dst_colorkey(FxU32 col);
int sstg_colorkey(FxU32 col, FxU32 fmt, FxU32 min, FxU32 max);
int sstg_random_colors(SstGRegs *sstg, FxU32 cfore, FxU32 *cback, FxU32 cdest);
FxU32 sstg_pick_rop(FxU32 cmdops, FxU32 cmdXops, FxU32 rop, FxU32 csrc, FxU32 cdst, int srcKey);
FxU32 sstg_rop2(FxU32 rop, FxU32 src, FxU32 dst);
FxU32 sstg_rop3(FxU32 rop, FxU32 pat, FxU32 src, FxU32 dst);
FxU32 sstg_getpattern(int x, int y, int *visible);
void sstg_setpattern_random(SstGRegs *sstg);
void sstg_setclut_random(SstGRegs *sstg);
int sstg_clipped(int x, int y);
FxU32 sstg_destination_mask(FxU32 color);
void sstg_drawblt(SstGRegs *sstg, int xs, int ys, int xd, int yd, int w, int h, FxU32 cmd, FxU32 *cmdXops);
void sstg_drawrect(SstGRegs *sstg, int x, int y, int w, int h, FxU32 cmd);
void sstg_drawline(SstGRegs *sstg, int x1, int y1, int x2, int y2, FxU32 cmd);
FxU32 sstg_check_pixel(long xc, long yc, FxU32 cmdops, FxU32 cmdXops,
			FxU32 rop, FxU32 csrc, FxU32 cdest, int srcKey);
void sstg_bres_setup(SstGRegs *sstg, DiagBresEdge *pe, int x1, int y1, int x2, int y2);
void sstg_bres_iterate(DiagBresEdge *pe);
FxU32 sstg_checkline(SstGRegs *sstg, int x1, int y1, int x2, int y2,
			FxU32 cmd, FxU32 cmdX,	FxU32 cfore, FxU32 cback,
			FxU32 style, int neighbors);
FxU32 sstg_get_screen_color(FxI32 x, FxI32 y, FxU8 *memory);
int sstg_test_screen_pixel(FxI32 x, FxI32 y, FxU32 color);
int sstg_set_screen_color(FxI32 x, FxI32 y, FxU8 *memory, FxU32 data);
FxU32 sstg_random_srcBaseAddr(SstGRegs *sstg, FxU32 srcFormat, FxU32 dstFormat,
				FxU32 baseAddr, FxU32 overlap);
FxU32 sstg_random_srcBaseAddr2(FxU32 destFormat, FxU32 destBA, FxU32 dstWidth, FxU32 dstHeight);
FxU32 sstg_src_sizeInBytes(int buffer, FxU32 srcBaseAddr, FxU32 srcFormat, FxU32 w, FxU32 h);
void sstg_random_src_rect(int buffer, FxU32 srcFormat, FxU32 srcBase,
			  long *xsOut, long *ysOut, long *wOut, long *hOut);
void sstg_draw_R_blt(SstGRegs *sstg, int xs, int ys, int xd, int yd,
			int w, int h, FxU32 cmd);
void sstg_draw_S_blt(SstGRegs *sstg, int xs, int ys, int sw, int sh,
			int xd, int yd,	int dw, int dh, FxU32 cmd);

extern int sstTrashMem(FxU32 base,unsigned sizeBytes);
extern FxU32 sstFbMemRalloc(unsigned sizeBytes);
extern void cmdP5Start(FxU32 space,FxU32 maskW2,FxU32 maskWN,FxU32 nWords,FxU32 dstAddr,FxU32 data);
extern void cmdP5Data(FxU32 data);
extern void sstg_random_poly(Polygon *poly, long xd, long yd, FxBool firstPt);
extern FxBool sstg_is_valid_polygon(Polygon *poly);
extern void sstg_print_poly(Polygon *poly);
extern void sstg_draw_poly(SstGRegs *sstg, Polygon *poly, FxU32 cmdops,
				     FxBool reversible);
extern int sstg_inside_poly(Polygon *poly, int x, int y);
extern void sstg_draw_poly_rectangle(SstGRegs *sstg, FxI32 baseX, FxI32 baseY,
				     FxI32 width, FxI32 height, FxU32 cmdops);
extern void sstg_draw_triangle(SstGRegs * sstg,
			FxI32 p1x, FxI32 p1y,
			FxI32 p2x, FxI32 p2y,
			FxI32 p3x, FxI32 p3y,
			       FxU32 cmdops);
extern FxU32 sstg_area(FxI32 p1x, FxI32 p1y,
		       FxI32 p2x, FxI32 p2y,
		       FxI32 p3x, FxI32 p3y);
extern int sstg_colinear(FxI32 p1x, FxI32 p1y,
			 FxI32 p2x, FxI32 p2y,
			 FxI32 p3x, FxI32 p3y);
extern FxU32 yuvTOrgbFixed(FxU32);
extern FxU32 sstg_compute_blit_address(FxU32 srcFormat, FxU32 base, int x, int y, int xsize);
extern void sendBltDataPacked(SstGRegs *sstg, FxU32 align, FxU32 byte, 
			      int xd, int yd, int w, int h, int ydir,
			      FxU32 srcFormat,
			      FxU32 host_pixels[]);
extern void sendBltData0(SstGRegs *sstg, FxU32 byte, FxU32 bit,
			 int xd, int yd, int w, int h, int ydir,
			 FxU32 srcFormat, FxU32 host_pixels[]);
extern int sstg_compatible_src_dst(FxU32 src, FxU32 dst);
extern FxU32 sstg_random_srcFormat(FxU32 dstFormat, int src_is_tiled);
extern int sstg_compatible_srcfmt_pack(FxU32 srcFormat, FxU32 pack);

#endif /* #ifdef CVG */

#endif /* !__SSTDIAG_H__ */
