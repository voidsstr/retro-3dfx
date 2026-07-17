#ifndef __CSIM_H__
#define __CSIM_H__

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
** $Revision: 6$
** $Date: 10/11/00 8:08:43 PM$
*/

// if windows.h is not include then define HWND
#ifndef _WINDOWS_
#define HDC  FxU32
#define HWND FxU32
#endif

#include "trexfunc.h"

#ifdef WINSIM
extern HalInfo halInfo;
#define HSIM_HW_SIMULATION              BIT(0)
#endif // WINSIM

//Ugly f'n global variables. Had to be added to add a hack on.
//The csim is so f'ed that it shouldn't be noticeable though.
extern SstRegs *globalSST;
extern SstRegs *globalChildrenSST[3];
extern SstRegs *currentRenderingChip;


#define SST_CHIP_NUMBER(sst) ((((FxU32)sst) >> 10) & 0xF)
#define CSIM_PRIVATE(sst) ((CsimPrivate *)((sst)->reservedD[1]))
#define CSIMG_PRIVATE(sstg) ((CsimPrivate *)((sstg)->unused0))
#define TMU_PRIVATE(sst) ((TmuData *)((sst)->reservedD[0]))
#define COL_BUFFER_ADDR_PRIMARY(sst) ((sst)->reservedE[2])
#define AUX_BUFFER_ADDR_PRIMARY(sst) ((sst)->reservedE[3])
#define COL_BUFFER_ADDR_SECONDARY(sst) ((sst)->reservedE[0])
#define AUX_BUFFER_ADDR_SECONDARY(sst) ((sst)->reservedE[1])

#define CSIM_VIEW_WINDOW_NAME "Viewer"

// CMD FIFO defines
#define NEW_PACKET	-1
#define BOGUS_DISCARD	-2

// translation tables for decompressing NCC format
typedef struct {
    unsigned char yRGB[16];
    int iRGB[4][3];
    int qRGB[4][3];
} NccTable;

// 256 color palette tables
typedef struct {
    unsigned long argb[256];
} Pal256;

typedef struct {			// a TSU vertex
	FxU32   x, y, z, w;
	FxU32   r, g, b, a;
	FxU32   s0, t0, w0;
	FxU32	s1, t1, w1;
} SVertex;

typedef struct {			// Triangle Setup Unit data
    FxU32 pingpong;
    FxU32 vertexCount;
    SVertex vArray[3];
} TsuData;

// for stepping multiple pixels
#define NUM_PIXELS		4
#define PSTAMP_W		2
#define PSTAMP_LOG2W	1
#define PSTAMP_H		2
#define PSTAMP_LOG2H	1

typedef struct {
                                        // INTERNAL regs for the simulator
    FxI64 s64;                          // 16.32 format
    FxI64 dsdx64;                       // these receive the values from PCI
    FxI64 dsdy64;			// S,T aren't need but are here for
    FxI64 t64;				// conformity with TmuData
    FxI64 dtdx64;
    FxI64 dtdy64;
    FxI64 w64;
    FxI64 dwdx64;
    FxI64 dwdy64;
    FxI64 z64;
    FxI64 dzdx64;
    FxI64 dzdy64;

    struct spanFbiRec {			// FBI span regs
	// data for the reference pixel (the upper left pixel of the 2x2 stamp)
	long x;				// 12.
	long y;				// 12.
	long r;				// 12.12
	long g;				// 12.12
	long b;				// 12.12
        FxI64 z64;                      // 4.24.20
	long a;				// 12.12
	FxI64 w64;			// 16.32

    } spanFbi;
    struct spanFbiRec spanFbiSave;

    struct edgeRec {
	long e, eSave;			// 26.4
	long dedx;			// 14.4
	long dedy;			// 14.4
    } edge0;				// the major edge (AC)
    struct edgeRec edge1;		// the minor edge (AB,BC)

    unsigned long lfbFBZmode;		// modified fbzMode for LFB access
    unsigned long lfbRenderMode;	// modified renderMode for LFB access
    unsigned long lfbRGBdata;		// linear frame buffer access data
    unsigned short lfbAdata;
    unsigned long lfbZdata;
    unsigned char trexIn[4];		// input from TMU 0
} FbiData;

typedef struct {
    int myNumber;			// my TREX number

					// INTERNAL regs for the simulator
    FxI64 s64;				// 16.32 format
    FxI64 dsdx64;			// these receive the values from PCI
    FxI64 dsdy64;
    FxI64 t64;
    FxI64 dtdx64;
    FxI64 dtdy64;
    FxI64 w64;
    FxI64 dwdx64;
    FxI64 dwdy64;

    struct spanTrexRec {		// TREX span regs
	FxI64 s64;			// 16.32
	FxI64 t64;			// 16.32
	FxI64 w64;			// 16.32
    } spanTrex;
    struct spanTrexRec spanTrexSave;


    int u,v;				// texture map indicies, .4 format
    int lod;				// LOD mipmap level-of-detail, 8.8 format
    int detail_lod;			// LOD for detail blending (before bias)
    int nccValid0;			// valid flag for ncc table 1
    int nccValid1;			// valid flag for ncc table 2
    NccTable ncc0;
    NccTable ncc1;
    Pal256   pal256;
    TRX_STLOD_STRUCT st;		// large struct from trexfunc.h
} TmuData;

typedef struct csimWindow {
    struct csimWindow *next;		// pointer to next window
    struct csimPrivate *cp;		// pointer back to CsimPrivate
    HDC  hDC;
    HWND hWnd;
    HWND hWndDisplaySettings;		// the display settings dialog box
    int pixFormat;			// VSD data
    int tiled;                          // 1 if tiled addressing, 0 if linear
    int base;
    int stride;
    int endBase;			// the extent in memory of this window
    int videoFilter;			// video filtering is enabled
    int zoom;				// zoom factor
    int width,height;			// width and height of visible area
    char hex;				// hex (or dec) display
    char statusBar;			// status bar is visible
    int mx, my;				// last mouse position
    char fileSaveName[256];		// file save name
} CsimWindow;

typedef struct cmdFifoPriv {
    FxI32 state;
    FxU32 packetWord;			// the original packet word
    FxU32 wordCount;			// word count or mask
    FxU32 vertexCount;			// current vertex count
    FxU32 vertexLimit;			// vertex limit
    FxU32 bogusCount;			// bogus word count
    FxU32 registerPtr;			// current register pointer
    FxU32 vertexData[16];
} CmdFifoPriv;

typedef struct {
	int x,y,err, dx;
	int xinc1, xinc2;
	int yinc1, yinc2;
	int einc1, einc2;
} BresEdge;

// 512-entry DAC color lookup table
typedef struct {
    FxU32 rgb[512];
} Clut512;

// Csim Private data is stored here, as well as FBI data (since there's only 1)
typedef struct csimPrivate {
    FxDeviceInfo *info;			// points back to halInfo data
    CsimWindow *mainWindow;		// the main control window
    CsimWindow *windows;		// linked list of all windows

    //========================================================================
    struct {				// environmental controls
	signed char recipFlag;		// controls reciprocal alg. used
	signed char fastFill;		// optimize fast fils
	double loderr;
	int statsVerboseLevel;

	int   paused;			// Go or Break?
	int   pauseAfterNextSwap;	// break after next swap
	int   pauseAfterNextTriangle;	// break after next triangle
	int   flushOnCommands;		// flush dirty pixels after each command
	int   saveAfterSwap;		// save image file after all swaps
	FxI16 flushCount;		// flush count
	FxI16 curFlushCount;		// current flush counter
	int   resetPixelsIn;		// reset hwStats.pixelIn every frame
	int   skipRendering;		// go fast mode (real fast)

	FxBool valid;			// has any of the dynamic text change
	FxU32 curBufferSwapCount;
	FxU32 pauseAfterSwapNum;
	FxU32 pauseAfterTriangleNum;
	FxU32 curTriangleCount;
	FxU32 curTrianglePerFrameCount;
        FxBool parentDevice;            // Is this the primary Napalm?
        FxBool aaEnabled;               //Is anti-aliasing enabled?
        FxBool aaPrimaryBuffers;        //Are we drawing into the primary buffers
        FxU32  aaSampleCount;           //2 or 4 subsample AA
        FxBool sliEnabled;              //Are we running in SLI?        
        FxU32 sliBandHeight;            //The height of the band that each chip renders
        FxU32 chipCount;                //The number of chips
        FxU32 chipIndex;                //Index of this particular Napalm
        FxBool detectNopError;         //This determines if the csim should check to see
                                        //if 12 NOPs are written to both TMUs immediately before
                                        //switching from 1ppc to 2ppc
        FxI32 triangleOffsetX;          //Perturbation for all triangle locations
        FxI32 triangleOffsetY;          //Perturbation for all triangle locations
        FxBool allowAccessesToCommandFifoRegion;   //Only allow when doing LFBs 
                                                   // to/from command fifo
        char name[32];
    } environment;

    // non chip-related info goes here
    unsigned char *hsim_memory;		// the HSIM memory array
    volatile unsigned char *memory;	// all of my memories...
    unsigned char *compositeMemory;  //For multichip, this contains the blended image
    int memorySizeInBytes;		// size of memory
    int numColorBuffers;		// the number color buffers
    int numBuffers;			// number of buffers, color+aux
    int bufferAddrHack[2];		// GMT: stores buffer addresses
    int bufferStrideHack[2];		// GMT: stores buffer strides
    unsigned int chipMask;		// bitmask of valid chips
    int cmdCode;
    int inCmdFifoExecMode;		// executing from CMD FIFO
    int launched;			// 2D launch mode
    FxU32 pixelsOut2d;			// count of 2D pixels drawn
    float ooSTWfracbits;		// 1.0/(1<<SST_ST(w)_FRACBITS)
    BresEdge bLeft, bRight;		// 2 bresenham iterators
    int lastY;				// last polygon Y coordinate
    Clut512 clut512;                    // video output color lookup table

    FxU8 *agpVirtAddr;                  // addr where AGP is mapped into memory

    TsuData tsuData;			// internal TSU data;
    FbiData fbiData;			// internal FBI chip data
    TmuData *tmuPrivate[MAX_NUM_TMUS];	// helps a lot in debugging
    SstRegs trex[MAX_NUM_TMUS];		// the TREX chips
    SstGRegs gui;			// the 2D GUI registers
    SstIORegs io;                       // the remapped I/O registers
    SstCRegs cmd;                       // the CMD/AGP registers
    SstPCIConfigRegs pciConfigRegs;            // PCI configuration space registers
    CmdFifoPriv fifo0data;		// private CMD FIFO data
    CmdFifoPriv fifo1data;
    vectorFiles vectorf;	        // handles for all vector files
 
} CsimPrivate;

// All information about a register is stored here
typedef struct {
	char *name;			// register name (for printing)
	FxU32 mask;			// bitmask to reduce to proper size
	int special;			// if >0 special, <0 implies not maskable
	int signExtend;
	int cmdCode;			// if non-zero then GO
	int floatConversion;		// if > 0, then float->int: fracbits
					// if < 0, then int->int64: shift count
	int readable;			// if > 0 then readable, else hack offset
	int altMap;			// offset for alternate mapping
} RegInfo;


// CSIM/HAL interface
void csimShutdown( SstRegs *sst );
void csimRegisterInfoInit( SstRegs *sst );
void csimFbiSetRevision( SstRegs *sst );
void csimFbiSetMemory( SstRegs *sst );
void csimTmuSetMemory( SstRegs *sst );
void csimFbiSetBuffers( SstRegs *sst );

void csimStore8 ( SstRegs *sst, FxU32 addr, FxU8  data );
void csimStore16( SstRegs *sst, FxU32 addr, FxU16 data );
void csimStore32( SstRegs *sst, FxU32 addr, FxU32 data );
void csimSnoop32( SstRegs *sst, FxU32 addr, FxU32 data );
FxU8  csimLoad8 ( SstRegs *sst, FxU32 addr );
FxU16 csimLoad16( SstRegs *sst, FxU32 addr );
FxU32 csimLoad32( SstRegs *sst, FxU32 addr );

void  csimCfgInitialize(SstRegs *sst);
FxU32 csimCfgLoad32(SstRegs *sst, FxU32 registerOffset);
void  csimCfgStore32(SstRegs *sst, FxU32 registerOffset, FxU32 data);


void csimOutPort32( SstRegs *sst, FxU16 port, FxU32 data );
FxU32 csimInPort32( SstRegs *sst, FxU16 port );

// internal interfaces
void csimRecomputeBusy(SstRegs *sst);
void csimCmdFifoExec(SstRegs *sst, CmdFifo *fifo, CmdFifoPriv *fifopriv, FxU32 data);
void csimCmdFifoExecuteN(SstRegs *sst, CmdFifo *fifo, CmdFifoPriv *fifopriv, FxU32 n);
void csimCmdFifoExecuteSome( SstRegs *sst );
void csimCmdFifoWrite( SstRegs *sst, CmdFifo *fifo, CmdFifoPriv *fifopriv, FxU32 addr, FxU32 data );
void csimExecuteMoveCmd(SstRegs *sst);
FxU32 csimFbiLfbRead( SstRegs *sst, FxU32 addr );
FxU32 csimFbiLfbSize(SstRegs *sst);
void  csimFbiLfbWrite( SstRegs *sst, FxU32 addr, FxU32 data, int words );
RegInfo *csimRegisterIoInfo( FxU32 addr );
RegInfo *csimRegisterCmdInfo( FxU32 addr );
RegInfo *csimRegister2dInfo( FxU32 addr );
RegInfo *csimRegister3dInfo( FxU32 addr );


FX_ENTRY FxU32 FX_CALL csimColorConvert(SstGRegs *, FxU32 csrc);

// routines for reading/writing pixels, and then memory
#define CSIM_BUF_2D_STRETCH_SRC	-3
#define CSIM_BUF_2D_SRC		-2
#define CSIM_BUF_2D_DST		-1
#define CSIM_BUF_3D_FRONT	0
#define CSIM_BUF_3D_BACK	1
#define CSIM_BUF_3D_AUX1	2
#define CSIM_BUF_3D_AUX2	3
#define CSIM_BUF_3D_COLOR	4
#define CSIM_BUF_DESKTOP	5
#define CSIM_BUF_OVERLAY	6
#define CSIM_BUF_CURSOR		7
#define CSIM_BUF_YUV		8
#define CSIM_BUF_3D_TRIPLE      9

//Tiled stuff
typedef struct 
{
  FxI32 uoff;
  FxI32 voff;
  FxI32 utot;
  FxI32 vtot;
} tiledStruct;


// GD
#ifndef WINSIM
// This is the 'old' set of prototypes...

FX_ENTRY SstRegs* FX_CALL csimInit(FxU32 bn);
FX_ENTRY void FX_CALL csimMakeMultiFunctionDevice(SstRegs *sst);

FX_ENTRY FxU32 FX_CALL tiledAddress(FxU32 base, FxU32 stride, FxU32 depth, int x, int y);
FX_ENTRY FxU32 FX_CALL csimPixelAddress2dSrc(SstGRegs *, FxU32 base, int x, int y);
FX_ENTRY FxU32 FX_CALL csimPixelAddress(SstRegs *, FxI32 buffer, int x, int y);
FX_ENTRY FxU32 FX_CALL csimReadPixel2d(SstGRegs *, FxI32 buffer, int x, int y);
FX_ENTRY void FX_CALL csimWritePixel2d(SstGRegs *, int x, int y, FxU32 data);

FX_ENTRY FxU32 FX_CALL csimReadPixel(SstRegs *, FxI32 buffer, int x, int y);
FX_ENTRY void FX_CALL csimWritePixel(SstRegs *, FxI32 buffer, int x, int y, FxU32 data);

FX_ENTRY FxU32 FX_CALL readPixel(SstRegs *, FxI32 buffer, int x, int y, volatile FxU8 * mem, char *where);
FX_ENTRY void FX_CALL writePixel(SstRegs *, FxI32 buffer, int x, int y, FxU32 data, volatile FxU8 * mem, char *where);

FX_ENTRY void FX_CALL csimRawLfbWrite(SstRegs *, FxU32 addr, FxU32 data, int nbytes );
FX_ENTRY FxU32 FX_CALL csimRawLfbRead(SstRegs *, FxU32 addr, int nbytes );

FX_ENTRY FxU32 FX_CALL readMem8 (volatile FxU8 *memory, int offset, int memorySize, char *where);
FX_ENTRY FxU32 FX_CALL readMem16(volatile FxU8 *memory, int offset, int memorySize, char *where);
FX_ENTRY FxU32 FX_CALL readMem24(volatile FxU8 *memory, int tiled, int offset, int memorySize, char *where);
FX_ENTRY FxU32 FX_CALL readMem32(volatile FxU8 *memory, int offset, int memorySize, char *where);

FX_ENTRY void FX_CALL writeMem8 (volatile FxU8 *memory, int offset, FxU32 data, int memorySize, char *where);
FX_ENTRY void FX_CALL writeMem16(volatile FxU8 *memory, int offset, FxU32 data, int memorySize, char *where);
FX_ENTRY void FX_CALL writeMem24(volatile FxU8 *memory, int tiled, int offset, FxU32 data, int memorySize, char *where);
FX_ENTRY void FX_CALL writeMem32(volatile FxU8 *memory, int offset, FxU32 data, int memorySize, char *where);

FX_ENTRY FxU32 FX_CALL csimReadMem8 (CsimPrivate *cp, int offset);
FX_ENTRY FxU32 FX_CALL csimReadMem16(CsimPrivate *cp, int offset);
FX_ENTRY FxU32 FX_CALL csimReadMem24(CsimPrivate *cp, int tiled, int offset);
FX_ENTRY FxU32 FX_CALL csimReadMem32(CsimPrivate *cp, int offset);
FX_ENTRY FxU32* FX_CALL csimReadMem64(CsimPrivate *cp, int offset, FxU32 *block);
FX_ENTRY FxU32* FX_CALL csimReadMem128(CsimPrivate *cp, int offset, FxU32 *block);

FX_ENTRY void FX_CALL csimWriteMem8 (CsimPrivate *cp, int offset, FxU32 data);
FX_ENTRY void FX_CALL csimWriteMem16(CsimPrivate *cp, int offset, FxU32 data);
FX_ENTRY void FX_CALL csimWriteMem24(CsimPrivate *cp, int tiled, int offset, FxU32 data);
FX_ENTRY void FX_CALL csimWriteMem32(CsimPrivate *cp, int offset, FxU32 data);

FX_ENTRY FxBool FX_CALL csimPicSave( CsimPrivate *, const char *filename,
			FxU32 buffer, FxU32 w, FxU32 h, FxU32 imgtype );
FX_ENTRY void FX_CALL csimVideo( CsimPrivate *, FxBool flag );

FX_ENTRY FxBool FX_CALL csimDecompressTexel(FxU32 *texelBlock, FxU32 tMode, FxU32 s, FxU32 t, FxU8 *result);
FX_ENTRY FxBool FX_CALL csimIsMultiTexturing(SstRegs *sst);
FX_ENTRY FxBool FX_CALL csimIsMultiTexturing2(FxU32 fbiFbzColorPath, FxU32 fbiCombineMode,
					      FxU32 tmu0TextureMode, FxU32 tmu0CombineMode);
FX_ENTRY FxI32 FX_CALL csimActiveTMUs(SstRegs *sst);
FX_ENTRY FxI32 FX_CALL csimActiveTMUs2(FxU32 fbiFbzColorPath, FxU32 fbiCombineMode,
				       FxU32 tmu0TextureMode, FxU32 tmu0CombineMode);

FX_ENTRY FxBool FX_CALL csimChipOwnsPixel(SstRegs *sst, int y, int *yPrime);
FX_ENTRY SstRegs* FX_CALL csimFindPixelOwner(FxI32 y, FxI32 *sliY);
FX_ENTRY SstRegs* FX_CALL csimFindSecondaryPixelOwner(FxI32 y, FxI32 *sliY);

FX_ENTRY FxBool FX_CALL csimConvertFramebufferFormatTo8888(FxU32 renderMode, FxU32 color, FxU32 *r, FxU32 *g, FxU32 *b, FxU32 *a);
FX_ENTRY FxBool FX_CALL csimConvertFramebufferFormatTo8888_2(FxU32 renderMode, FxU32 frameBufferColor, FxU32 *ARGB);
FX_ENTRY FxBool FX_CALL csimConvert8888ToFramebufferFormat(FxU32 renderMode, FxU32 r, FxU32 g, FxU32 b, FxU32 a, FxU32 *result);
FX_ENTRY FxBool FX_CALL csimConvert8888ToFramebufferFormat_2(FxU32 renderMode, FxU32 ARGB, FxU32 *frameBufferColor);


//////////////////////////////////////////////////////////////
//
//                  Mipmap crap
//
//////////////////////////////////////////////////////////////

///Linear stuff
FX_ENTRY FxI32 FX_CALL sstLinearMipMapOffset(FxU32 lod, FxU32 logAspectRatio, FxU32 bitsPerTexel, 
						FxBool bigAssTexture, FxBool wideTexture, FxBool compressedTexture,
						FxBool splitTextures);
FX_ENTRY FxI32 FX_CALL sstLinearMipMapOffset2(FxU32 lod, FxU32 tLOD, FxU32 textureMode);

FX_ENTRY FxI32 FX_CALL sstLinearMipMapSize(FxU32 lod, FxU32 logAspectRatio, FxU32 bitsPerTexel, 
					      FxBool bigAssTexture, FxBool wideTexture, FxBool compressedTexture);
FX_ENTRY FxI32 FX_CALL sstLinearMipMapSize2(FxU32 lod, FxU32 tLOD, FxU32 textureMode);

//Tiled stuff
FX_ENTRY tiledStruct FX_CALL sstTiledMipMapOffset(FxU32 lod, FxU32 logAspectRatio, FxU32 bitsPerTexel,
						     FxBool bigAssTexture, FxBool wideTexture, FxBool compressedTexture);
FX_ENTRY tiledStruct FX_CALL sstTiledMipMapOffset2(FxU32 lod, FxU32 tLOD, FxU32 textureMode);


void guiKeepAlive(int);
void guiApplyToAllWindows(CsimPrivate *, void (*)(CsimWindow *));
void guiDrawPixel(CsimPrivate *, int x, int y, int addr, FxU32 r, FxU32 g, FxU32 b);
void guiRefresh(CsimPrivate *);
void guiChange(CsimPrivate *,int);
FxU32 yuvTOrgbFixed(FxU32 yuv);

#else // WINSIM
// This set of prototypes is needed to compile csim with the Windows Simulation Environment

SstRegs* FX_CSTYLE csimInit(FxU32 bn);
void FX_CSTYLE csimMakeMultiFunctionDevice(SstRegs *sst);

FxU32 FX_CSTYLE tiledAddress(FxU32 base, FxU32 stride, FxU32 depth, int x, int y);
FxU32 FX_CSTYLE csimPixelAddress2dSrc(SstGRegs *, FxU32 base, int x, int y);
FxU32 FX_CSTYLE csimPixelAddress(SstRegs *, FxI32 buffer, int x, int y);
FxU32 FX_CSTYLE csimReadPixel2d(SstGRegs *, FxI32 buffer, int x, int y);
void FX_CSTYLE csimWritePixel2d(SstGRegs *, int x, int y, FxU32 data);

FxU32 FX_CSTYLE csimReadPixel(SstRegs *, FxI32 buffer, int x, int y);
void FX_CSTYLE csimWritePixel(SstRegs *, FxI32 buffer, int x, int y, FxU32 data);

FxU32 FX_CSTYLE readPixel(SstRegs *, FxI32 buffer, int x, int y, volatile FxU8 * mem, char *where);
void FX_CSTYLE writePixel(SstRegs *, FxI32 buffer, int x, int y, FxU32 data, volatile FxU8 * mem, char *where);

void FX_CSTYLE csimRawLfbWrite(SstRegs *, FxU32 addr, FxU32 data, int nbytes );
FxU32 FX_CSTYLE csimRawLfbRead(SstRegs *, FxU32 addr, int nbytes );

FxU32 FX_CSTYLE readMem8 (volatile FxU8 *memory, int offset, int memorySize, char *where);

FxU32 FX_CSTYLE readMem16(volatile FxU8 *memory, int offset, int memorySize, char *where);
FxU32 FX_CSTYLE readMem24(volatile FxU8 *memory, int tiled, int offset, int memorySize, char *where);
FxU32 FX_CSTYLE readMem32(volatile FxU8 *memory, int offset, int memorySize, char *where);

void FX_CSTYLE writeMem8 (volatile FxU8 *memory, int offset, FxU32 data, int memorySize, char *where);
void FX_CSTYLE writeMem16(volatile FxU8 *memory, int offset, FxU32 data, int memorySize, char *where);
void FX_CSTYLE writeMem24(volatile FxU8 *memory, int tiled, int offset, FxU32 data, int memorySize, char *where);
void FX_CSTYLE writeMem32(volatile FxU8 *memory, int offset, FxU32 data, int memorySize, char *where);

FxU32 FX_CSTYLE csimReadMem8 (CsimPrivate *cp, int offset);
FxU32 FX_CSTYLE csimReadMem16(CsimPrivate *cp, int offset);
FxU32 FX_CSTYLE csimReadMem24(CsimPrivate *cp, int tiled, int offset);
FxU32 FX_CSTYLE csimReadMem32(CsimPrivate *cp, int offset);
FxU32* FX_CSTYLE csimReadMem64(CsimPrivate *cp, int offset, FxU32 *block);
FxU32* FX_CSTYLE csimReadMem128(CsimPrivate *cp, int offset, FxU32 *block);

void FX_CSTYLE csimWriteMem8 (CsimPrivate *cp, int offset, FxU32 data);
void FX_CSTYLE csimWriteMem16(CsimPrivate *cp, int offset, FxU32 data);
void FX_CSTYLE csimWriteMem24(CsimPrivate *cp, int tiled, int offset, FxU32 data);
void FX_CSTYLE csimWriteMem32(CsimPrivate *cp, int offset, FxU32 data);

FxBool FX_CSTYLE csimPicSave( CsimPrivate *, const char *filename,
			FxU32 buffer, FxU32 w, FxU32 h, FxU32 imgtype );
void FX_CSTYLE csimVideo( CsimPrivate *, FxBool flag );

FxBool FX_CSTYLE csimDecompressTexel(FxU32 *texelBlock, FxU32 tMode, FxU32 s, FxU32 t, FxU8 *result);
FxBool FX_CSTYLE csimIsMultiTexturing(SstRegs *sst);
FxBool FX_CSTYLE csimIsMultiTexturing2(FxU32 fbiFbzColorPath, FxU32 fbiCombineMode,
				     FxU32 tmu0TextureMode, FxU32 tmu0CombineMode);

FxI32 FX_CSTYLE csimActiveTMUs(SstRegs *sst);
FxI32 FX_CSTYLE csimActiveTMUs2(FxU32 fbiFbzColorPath, FxU32 fbiCombineMode,
				       FxU32 tmu0TextureMode, FxU32 tmu0CombineMode);

FxBool FX_CSTYLE csimChipOwnsPixel(SstRegs *sst, int y, int *yPrime);
SstRegs* FX_CSTYLE csimFindPixelOwner(FxI32 y, FxI32 *sliY);
SstRegs* FX_CSTYLE csimFindSecondaryPixelOwner(FxI32 y, FxI32 *sliY);

FxBool FX_CSTYLE csimConvertFramebufferFormatTo8888(FxU32 renderMode, FxU32 color, FxU32 *r, FxU32 *g, FxU32 *b, FxU32 *a);
FxBool FX_CSTYLE csimConvertFramebufferFormatTo8888_2(FxU32 renderMode, FxU32 frameBufferColor, FxU32 *ARGB);
FxBool FX_CSTYLE csimConvert8888ToFramebufferFormat(FxU32 renderMode, FxU32 r, FxU32 g, FxU32 b, FxU32 a, FxU32 *result);
FxBool FX_CSTYLE csimConvert8888ToFramebufferFormat_2(FxU32 renderMode, FxU32 ARGB, FxU32 *frameBufferColor);

//////////////////////////////////////////////////////////////
//
//                  Mipmap crap
//
//////////////////////////////////////////////////////////////

///Linear stuff
FxI32 FX_CSTYLE sstLinearMipMapOffset(FxU32 lod, FxU32 logAspectRatio, FxU32 bitsPerTexel, 
						FxBool bigAssTexture, FxBool wideTexture, FxBool compressedTexture,
						FxBool splitTextures);
FxI32 FX_CSTYLE sstLinearMipMapOffset2(FxU32 lod, FxU32 tLOD, FxU32 textureMode);

FxI32 FX_CSTYLE sstLinearMipMapSize(FxU32 lod, FxU32 logAspectRatio, FxU32 bitsPerTexel, 
					      FxBool bigAssTexture, FxBool wideTexture, FxBool compressedTexture);
FxI32 FX_CSTYLE sstLinearMipMapSize2(FxU32 lod, FxU32 tLOD, FxU32 textureMode);

//Tiled stuff
tiledStruct FX_CSTYLE sstTiledMipMapOffset(FxU32 lod, FxU32 logAspectRatio, FxU32 bitsPerTexel,
						     FxBool bigAssTexture, FxBool wideTexture, FxBool compressedTexture);
tiledStruct FX_CSTYLE sstTiledMipMapOffset2(FxU32 lod, FxU32 tLOD, FxU32 textureMode);


void guiKeepAlive(int);
void guiApplyToAllWindows(CsimPrivate *, void (*)(CsimWindow *));
void guiDrawPixel(CsimPrivate *, int x, int y, int addr, FxU32 r, FxU32 g, FxU32 b);
void guiRefresh(CsimPrivate *);
void guiChange(CsimPrivate *,int);
FxU32 yuvTOrgbFixed(FxU32 yuv);

#endif // WINSIM

#endif /* !__CSIM_H__ */
