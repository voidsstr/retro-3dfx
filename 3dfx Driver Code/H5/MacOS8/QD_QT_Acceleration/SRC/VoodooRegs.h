/********************************************************************************
	VoodooRegs.h
	
	Register reading and writing model for QD/QT acceleration.
	
	Chall Fry
	Critical Path Software
	2/14/00		
	
*/

//
class Swap32
{
	volatile SInt32	mData;
	
public:

	// The conversion operator to SInt32 handles reading from the register.
	inline operator SInt32()
	{
		return __lwbrx((void *) &mData, 0);
	}
	
	// The assignment operator handles writing to the register.
	inline UInt32 operator= (const UInt32& value)
	{
		__stwbrx(value, (void *) &mData, 0);
		return value;
	}
};


typedef struct VoodooIORegs
{     

  // Init Registers
    Swap32 status;                       // aliased status register
    Swap32 pciInit0;
    Swap32 sipMonitor;
    Swap32 lfbMemoryConfig;    //This is more or less a dummy variable now.
                              //It really just controls access to the
                              //lfbMemoryTileCtrl and lfbMemoryTileCompare registers
    Swap32 miscInit0;
    Swap32 miscInit1;
    Swap32 dramInit0;
    Swap32 dramInit1;
    Swap32 agpInit;
    Swap32 tmuGbeInit;
    Swap32 vgaInit0;
    Swap32 vgaInit1;
    Swap32 dramCommand;
    Swap32 dramData;
    Swap32 reservedZ[1];
	Swap32 vidTvOutBlankVCount;

  // PLL Registers
    Swap32 pllCtrl0;
    Swap32 pllCtrl1;
    Swap32 pllCtrl2;

  // DAC Registers
    Swap32 dacMode;
    Swap32 dacAddr;
    Swap32 dacData;

  // Video Registers I
    Swap32 vidMaxRGBDelta;
    Swap32 vidProcCfg;
    Swap32 hwCurPatAddr;
    Swap32 hwCurLoc;
    Swap32 hwCurC0;
    Swap32 hwCurC1;
    Swap32 vidInFormat;
	Swap32 vidTvOutBlankHCount;
    Swap32 vidSerialParallelPort;
    Swap32 vidInXDecimDeltas;
    Swap32 vidInDecimInitErrs;
    Swap32 vidInYDecimDeltas;
    Swap32 vidPixelBufThold;
    Swap32 vidChromaMin;
    Swap32 vidChromaMax;
    Swap32 vidCurrentLine;
    Swap32 vidScreenSize;
    Swap32 vidOverlayStartCoords;
    Swap32 vidOverlayEndScreenCoord;
    Swap32 vidOverlayDudx;
    Swap32 vidOverlayDudxOffsetSrcWidth;
    Swap32 vidOverlayDvdy;

  // VGA Registers
    Swap32 vgaRegister[12];

  // Video Registers II
    Swap32 vidOverlayDvdyOffset;
    Swap32 vidDesktopStartAddr;
    Swap32 vidDesktopOverlayStride;
    Swap32 vidInAddr0;
    Swap32 vidInAddr1;
    Swap32 vidInAddr2;
    Swap32 vidInStride;
    Swap32 vidCurrOverlayStartAddr;

  // The following registers don't exist in IO space where
  // their offset is in this structure.
  Swap32 lfbMemoryTileCtrl;
  Swap32 lfbMemoryTileCompare;
} VoodooIORegs;


typedef struct VoodooCmdFifo {
    Swap32 baseAddrL;
    Swap32 baseSize;
    Swap32 bump;
    Swap32 readPtrL;
    Swap32 readPtrH;
    Swap32 aMin;
    Swap32 unusedA;
    Swap32 aMax;
    Swap32 unusedB;
    Swap32 depth;
    Swap32 holeCount;
    Swap32 reserved;
} VoodooCmdFifo;


// This structure mirrors the placement of the fifo/agp registers on the board. The h3Info struct
// in Utilities.h has a pointer to one of these structs, which is initialized to point directly
// at the registers. 
typedef struct VoodooFifoRegs
{
  // AGP
    Swap32 agpReqSize;
    Swap32 hostAddrLow;
    Swap32 hostAddrHigh;
    Swap32 graphicsAddr;
    Swap32 graphicsStride;
    Swap32 moveCMD;
    Swap32 reservedL[2];

  // CMD FIFO 0,1
    VoodooCmdFifo cmdFifo0;
    VoodooCmdFifo cmdFifo1;

    Swap32 cmdFifoThresh;
    Swap32 cmdHoleInit;
    Swap32 reservedO[6];
    Swap32 reservedP[8];
    Swap32 reservedQ[8];
    Swap32 reservedR[8];
  // misc
    Swap32 yuvBaseAddr;
    Swap32 yuvStride;
    Swap32 reservedS[6];
    Swap32 crc1;
    Swap32 reservedT[3];
    Swap32 crc2;
} VoodooFifoRegs;


// This structure mirrors the placement of the 2D registers on the board. The h3Info struct
// in Utilities.h has a pointer to one of these structs, which is initialized to point directly
// at the registers. 
typedef struct Voodoo2DRegs
{
	Swap32 status;                      
	Swap32 unused0;
	Swap32 clip0min;
	Swap32 clip0max;
	Swap32 dstBaseAddr;
	Swap32 dstFormat;
	Swap32 srcColorkeyMin;
	Swap32 srcColorkeyMax;
	Swap32 dstColorkeyMin;
	Swap32 dstColorkeyMax;
	Swap32 bresError0;
	Swap32 bresError1;
	Swap32 rop;
	Swap32 srcBaseAddr;
	Swap32 commandEx;
	Swap32 lineStipple;
	Swap32 lineStyle;
	Swap32 pattern0alias;
	Swap32 pattern1alias;
	Swap32 clip1min;
	Swap32 clip1max;
	Swap32 srcFormat;
	Swap32 srcSize;
	Swap32 srcXY;
	Swap32 colorBack;
	Swap32 colorFore;
	Swap32 dstSize;
	Swap32 dstXY;
	Swap32 command;
	Swap32 reserved[3];
	Swap32 launch[32];
	Swap32 colorPattern[64];
#if COLORTRANSLUT
	Swap32 colorTransLut[256];
#endif
} Voodoo2DRegs;	
	
// Again, this is cribbed from h3regs.h, however, the member types have been changes to 
// Swap32s, which perform the correct byte swapping for reads and writes for use with the PowerPC.

//----------------- SST chip 3D layout -------------------------
// registers are in groups of 8 for easy decode
typedef struct Voodoo_vertex_Rec {
    Swap32 x;            // 12.4 format
    Swap32 y;            // 12.4
} Voodoo_vertex_Rec;

typedef struct Voodoo3DRegs {       // THE 3D CHIP
                                        // EXTERNAL registers
    Swap32 status;               // chip status, Read Only
    Swap32 intrCtrl;             // interrupt control
    Voodoo_vertex_Rec vA;                  // Vertex A,B,C
    Voodoo_vertex_Rec vB;
    Voodoo_vertex_Rec vC;

    Swap32 r;             // 12.12        Parameters
    Swap32 g;             // 12.12
    Swap32 b;             // 12.12
    Swap32 z;             // 20.12 in 16bpp, 28.4 in 32bpp (there is an ugly hack in csimio.c, search "//EVIL:")
    Swap32 a;             // 12.12
    Swap32 s;             // 14.18
    Swap32 t;             // 14.18
    Swap32 w;             //  2.30

    Swap32 drdx;                  // X Gradients
    Swap32 dgdx;
    Swap32 dbdx;
    Swap32 dzdx;  //20.12 in 16bpp, 28.4 in 32bpp (there is an ugly hack in csimio.c, search "//EVIL:")
    Swap32 dadx;
    Swap32 dsdx;
    Swap32 dtdx;
    Swap32 dwdx;

    Swap32 drdy;                  // Y Gradients
    Swap32 dgdy;
    Swap32 dbdy;
    Swap32 dzdy;  //20.12 in 16bpp, 28.4 in 32bpp (there is an ugly hack in csimio.c, search "//EVIL:")
    Swap32 dady;
    Swap32 dsdy;
    Swap32 dtdy;
    Swap32 dwdy;

    Swap32 triangleCMD;  // execute a triangle command (float)
    Swap32 reservedA;
    Voodoo_vertex_Rec FvA;                 // floating point version
    Voodoo_vertex_Rec FvB;
    Voodoo_vertex_Rec FvC;

    Swap32 Fr;                    // floating point version
    Swap32 Fg;
    Swap32 Fb;
    Swap32 Fz;
    Swap32 Fa;
    Swap32 Fs;
    Swap32 Ft;
    Swap32 Fw;

    Swap32 Fdrdx;
    Swap32 Fdgdx;
    Swap32 Fdbdx;
    Swap32 Fdzdx;
    Swap32 Fdadx;
    Swap32 Fdsdx;
    Swap32 Fdtdx;
    Swap32 Fdwdx;

    Swap32 Fdrdy;
    Swap32 Fdgdy;
    Swap32 Fdbdy;
    Swap32 Fdzdy;
    Swap32 Fdady;
    Swap32 Fdsdy;
    Swap32 Fdtdy;
    Swap32 Fdwdy;

    Swap32 FtriangleCMD;         // execute a triangle command
    Swap32 fbzColorPath;         // color select and combine
    Swap32 fogMode;              // fog Mode
    Swap32 alphaMode;            // alpha Mode
    Swap32 fbzMode;              // framebuffer and Z mode
    Swap32 lfbMode;              // linear framebuffer Mode
    Swap32 clipLeftRight;        // (6)10(6)10
    Swap32 clipBottomTop;        // (6)10(6)10

    Swap32 nopCMD;       // execute a nop command
    Swap32 fastfillCMD;  // execute a fast fill command
    Swap32 swapbufferCMD;// execute a swapbuffer command
    Swap32 fogColor;             // (8)888
    Swap32 zaColor;              // 8.24
    Swap32 chromaKey;            // (8)888
    Swap32 chromaRange;
    Swap32 userIntrCmd;

    Swap32 stipple;              // 32 bits, MSB masks pixels
    Swap32 c0;                   // 8.8.8.8 (ARGB)
    Swap32 c1;                   // 8.8.8.8 (ARGB)
    struct {                            // statistic gathering variables
        Swap32 fbiPixelsIn;
        Swap32 fbiChromaFail;
        Swap32 fbiZfuncFail;
        Swap32 fbiAfuncFail;
        Swap32 fbiPixelsOut;
    } stats;

    Swap32 fogTable[32];         // 64 entries, 2 per word, 2 bytes each

    Swap32 renderMode;		// new 32bpp and 1555 modes
    Swap32 stencilMode;
    Swap32 stencilOp;
    Swap32 colBufferAddr;        //This is the primary colBufferAddr
    Swap32 colBufferStride;    
    Swap32 auxBufferAddr;        //This is the primary auxBufferAddr
    Swap32 auxBufferStride;
    Swap32 fbiStencilFail;

    Swap32 clipLeftRight1;
    Swap32 clipBottomTop1;
    Swap32 combineMode;
    Swap32 sliCtrl;
    Swap32 aaCtrl;
    Swap32 chipMask;
    Swap32 leftDesktopBuf;
    Swap32 reservedD[2];         // NOTE: used to store TMUprivate ptr  (reservedD[0])
                                        // NOTE: used to store CSIMprivate ptr (reservedD[1])

    Swap32 reservedE[7];         // NOTE: reservedE[0] stores the secondary colBufferAddr
                                        // NOTE: reservedE[1] stores the secondary auxBufferAddr  
                                        // NOTE: reservedE[2] stores the primary colBufferAddr  
                                        // NOTE: reservedE[3] stores the primary auxBufferAddr  
                      
    Swap32 reservedF[3];  
    Swap32 swapBufferPend;
    Swap32 leftOverlayBuf;
    Swap32 rightOverlayBuf;
    Swap32 fbiSwapHistory;
    Swap32 fbiTrianglesOut;      // triangles out counter

    Swap32 sSetupMode;
    Swap32 sVx;
    Swap32 sVy;
    Swap32 sARGB;
    Swap32 sRed;
    Swap32 sGreen;
    Swap32 sBlue;
    Swap32 sAlpha;

    Swap32 sVz;
    Swap32 sOowfbi;
    Swap32 sOow0;
    Swap32 sSow0;
    Swap32 sTow0;
    Swap32 sOow1;
    Swap32 sSow1;
    Swap32 sTow1;

    Swap32 sDrawTriCMD;
    Swap32 sBeginTriCMD;
    Swap32 reservedG[6];

    Swap32 reservedH[8];

    Swap32 reservedI[8];

    Swap32 textureMode;          // texture Mode
    Swap32 tLOD;                 // texture LOD settings
    Swap32 tDetail;              // texture detail settings
    Swap32 texBaseAddr;          // current texture base address
    Swap32 texBaseAddr1;
    Swap32 texBaseAddr2;
    Swap32 texBaseAddr38;
    Swap32 trexInit0;            // hardware init bits
    Swap32 trexInit1;            // hardware init bits
   
    Swap32 nccTable0[12];        // NCC decode tables, bits are packed
    Swap32 nccTable1[12];        // 4 words Y, 4 words I, 4 words Q

} Voodoo3DRegs;
