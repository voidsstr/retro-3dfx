#ifndef __gl_s3virge_h_
#define __gl_s3virge_h_



#define S3D_BLT_SOURCE_BASE                     (0x0000A4D4L)
#define S3D_BLT_DESTINATION_BASE                (0x0000A4D8L)
#define S3D_BLT_CLIP_LEFT_RIGHT                 (0x0000A4DCL)
#define S3D_BLT_CLIP_TOP_BOTTOM                 (0x0000A4E0L)
#define S3D_BLT_DESTINATION_SOURCE_STRIDE       (0x0000A4E4L)
#define S3D_BLT_MONO_PATTERN_0                  (0x0000A4E8L)
#define S3D_BLT_MONO_PATTERN_1                  (0x0000A4ECL)
#define S3D_BLT_PATTERN_BACKGROUND_COLOR        (0x0000A4F0L)
#define S3D_BLT_PATTERN_FOREGROUND_COLOR        (0x0000A4F4L)
#define S3D_BLT_SOURCE_BACKGROUND_COLOR         (0x0000A4F8L)
#define S3D_BLT_SOURCE_FOREGROUND_COLOR         (0x0000A4FCL)
#define S3D_BLT_COMMAND                         (0x0000A500L)
#define S3D_BLT_WIDTH_HEIGHT                    (0x0000A504L)
#define S3D_BLT_SOURCE_X_Y                      (0x0000A508L)
#define S3D_BLT_DESTINATION_X_Y                 (0x0000A50CL)
#define S3D_DEST_16BPP_1555                     (0x00000004L)

#define S3D_AUTOEXECUTE                         (0x00000001L)
#define S3D_HARDWARE_CLIPPING                   (0x00000002L)

#define S3D_DRAW_ENABLE                         (0x00000020L)
#define S3D_MONOCHROME_SOURCE                   (0x00000040L)
#define S3D_CPU_DATA                            (0x00000080L)
#define S3D_MONOCHROME_PATTERN                  (0x00000100L)
#define S3D_TRANSPARENT                         (0x00000200L)

#define S3D_BYTE_ALIGNED                        (0x00000000L)
#define S3D_WORD_ALIGNED                        (0x00000400L)
#define S3D_DWORD_ALIGNED                       (0x00000800L)

#define S3D_FDO_ENTIRE_DWORD                    (0x00000000L)
#define S3D_FDO_2ND_BYTE                        (0x00001000L)
#define S3D_FDO_3RD_BYTE                        (0x00002000L)
#define S3D_FDO_4TH_BYTE                        (0x00003000L)

#define S3D_ROP_SHIFT                           (0x00000011L)

#define S3D_X_POSITIVE_BLT                      (0x02000000L)
#define S3D_X_NEGATIVE_BLT                      (0x00000000L)
#define S3D_Y_POSITIVE_BLT                      (0x04000000L)
#define S3D_Y_NEGATIVE_BLT                      (0x00000000L)

#define S3D_BITBLT                              (0x00000000L)
#define S3D_RECTANGLE                           (0x10000000L)
#define S3D_LINE_2D                             (0x18000000L)
#define S3D_POLYGON                             (0x28000000L)
#define S3D_NOP                                 (0x78000000L)

//
// Register defs for the S3Virge 3D Engine.
// Base = S3RegBase + 0xB500 (bytes)


// Note, this is B500 / 4, so we can just say 
// s3dEngine = RegBase+VIRGE_TRI_ENGINE_OFFSET

#define VIRGE_TRI_ENGINE_OFFSET  0x2D40  



#define VIRGE_CMD_REG    0x2D40

#define VIRGE_TRI_DXBC   0x2D58
#define VIRGE_TRI_CX     0x2D59
#define VIRGE_TRI_DXAB   0x2D5A
#define VIRGE_TRI_BX     0x2D5B
#define VIRGE_TRI_DXAC   0x2D5C
#define VIRGE_TRI_XSTART 0x2D5D
#define VIRGE_TRI_YSTART 0x2D5E
#define VIRGE_TRI_YCOUNT 0x2D5F

//
// Base = regbase + 0xb4d4

typedef struct __glS3VMisc3DRegsRec {
    GLuint zBufferOffset;
    GLuint destOffset;
    GLuint clipRectLR;      // ClipRect left/Right Format U16x2
    GLuint clipRectTB;      // ClipRect top/bottom Format U16x2
    GLuint destSrcStride; // format U 16x2 8-byte aligned
    GLuint zStride;
    GLuint texOffset;
    GLuint texBorderColor;
    GLuint fogColor;
    GLuint color0;
    GLuint color1;
} __glS3VMisc3DRegs;


// Base = regBase + 0xb144
// Notice there are no textured lines
typedef struct __glS3VLineEngineRegistersRec {
    GLuint dBGY;
    GLuint dARY;
    GLuint BGStart;
    GLuint ARStart;
    GLuint reserved0;
    GLint dZY;
    GLint ZStart;
    GLuint reserved1;
    GLuint reserved2;
    GLuint reserved3;
    GLuint endPoints;
    GLint dXY;
    GLint XStart;
    GLint YStart;
    GLuint YCount;
} __glS3VLineEngineRegisters;



// Base = regBase + 0xb500
typedef struct __glS3VTriEngineRegistersRec {
    GLuint cmd;         // Command register
    GLuint texelVBase;  // format 12.8
    GLuint texelUBase;
    GLint      dWX;         // Homogenous component delta wrt X. format S12.19
    GLint      dWY;         // Homogenous component delta wrt Y.
    GLint      WStart;      // A.W value
    GLint      dDX;         // D (mip map select) delta wrt X
    GLint      dVX;         // V (vertical texture coord, or t2), delta wrt X
                          // U (horizonal texture coord, or t1), delta wrt X
    GLint      dUX;         //        format S24.8
    GLint      dDY;         // D (mip map select) delta wrt X
    GLint      dVY;         // V (vertical texture coord, or t2), delta wrt Y
                          // U (horizonal texture coord, or t1), delta wrt Y
    GLint      dUY;         //        format S24.8
    GLint      DStart;      // initial value of D 
    GLint      VStart;      // initial value of V coord
    GLint      UStart;      // initial value of U 
    GLuint dBGX;        // delta for B and G wrt X. Format S8.7 x 2.
    GLuint dARX;        // delta for A and R wrt X. Format S8.7 x 2.
    GLuint dBGY;        // delta for B and G wrt Y. Format S8.7.
    GLuint dARY;        // delta for A and R wrt Y. Format S8.7.
    GLuint BGStart;     // start value for B and G. Format S8.7.
    GLuint ARStart;     // start value A and R. Format S8.7.

    GLint dZX;              // Z Value delta wrt X. Format S16.15
    GLint dZY;              // Z Value delta wrt Y 
    GLint ZStart;           // Z Value delta wrt Y 
    GLint dXBC;             // slope of the edge between pts B and C wrt Y
    GLint XBCStart;         // X coord start pixel of BC side. format S11.20
    GLint dXAB;             // slope of the edge between pts A and B wrt Y
    GLint XABStart;         // X coord of first pixel of AB Sid. Format S11.20
    GLint dXAC;             // slope of the edge between A and C wrt Y
    GLint XACStart;         // X coord of first pixel, left side.
    GLint AY;               // Y coord of pt A
    GLuint  Heights;    // start values A and R. Format S8.7 x 2.
} __glS3VTriEngineRegisters;


/*
 * Command register defines for the 2D line and clear engine
 * registers.
 */

#define VIRGE_AUTO_EXEC       0x00000001
#define VIRGE_HW_CLIP         0x00000002

#define VIRGE_DEPTH8          0x00000000
#define VIRGE_DEPTH16         0x00000004
#define VIRGE_DEPTH24         0x00000008

#define VIRGE_DRAW_ENABLE     0x00000020
#define VIRGE_MONO_SRC        0x00000040

#define VIRGE_IMG_SRC_VRAM    0x00000000
#define VIRGE_IMG_SRC_DATA    0x00000080

#define VIRGE_MONO_PAT        0x00000100
#define VIRGE_PAT_STIP        0x00000200

#define VIRGE_IMG_ALIGN_BYTE  0x00000000
#define VIRGE_IMG_ALIGN_WORD  0x00000400
#define VIRGE_IMG_ALIGN_GLuint 0x00000800

#define VIRGE_FGLuint_4BYTES        0x00000000
#define VIRGE_FGLuint_3BYTES        0x00001000
#define VIRGE_FGLuint_2BYTES        0x00002000
#define VIRGE_FGLuint_1BYTE         0x00003000

#define VIRGE_BLOCKWRITE           0x00000000
#define VIRGE_BLOCKWRITE_DISABLE   0x00010000

#define VIRGE_ROP_COPY             0x01980000

#define VIRGE_DIRC_LR              0x02000000
#define VIRGE_DIRC_TB              0x04000000

#define VIRGE_CMD_BLT              0x00000000
#define VIRGE_CMD_FILLRECT         0x10000000
#define VIRGE_CMD_LINE             0x18000000
#define VIRGE_CMD_POLY             0x28000000
#define VIRGE_CMD_NOOP             0x78000000

#define VIRGE_CMD_2D               0x00000000
#define VIRGE_CMD_3D               0x80000000

#define VIRGE_CMD_LINE3D             0x40000000L
#define VIRGE_CLEAR_RECT ( \
 VIRGE_IMG_SRC_DATA | \
 VIRGE_DEPTH16     | \
 VIRGE_DRAW_ENABLE | \
 VIRGE_ROP_COPY    | \
 VIRGE_DIRC_LR     | \
 VIRGE_DIRC_TB     | \
 VIRGE_CMD_FILLRECT )





/*
 * Command register defines for the 3D Line and triangle engine command 
 * registers.
 */

#define S3D_TEXTURE_FILTER_MIP_NEAREST          (0x00000000L)
#define S3D_TEXTURE_FILTER_LINEAR_MIP_NEAREST   (0x00001000L)
#define S3D_TEXTURE_FILTER_MIP_LINEAR           (0x00002000L)
#define S3D_TEXTURE_FILTER_LINEAR_MIP_LINEAR    (0x00003000L)
#define S3D_TEXTURE_FILTER_NEAREST              (0x00004000L)
#define S3D_TEXTURE_FILTER_YU_VU                (0x00005000L)
#define S3D_TEXTURE_FILTER_LINEAR               (0x00006000L)
#define S3D_TEXTURE_FILTER_RESERVED             (0x00007000L)

#define S3D_TEXTURE_BLEND_COMPLEX_REFLECTION    (0x00000000L)
#define S3D_TEXTURE_BLEND_MODULATE              (0x00008000L)
#define S3D_TEXTURE_BLEND_DECAL                 (0x00010000L)

#define S3D_FOG_ENABLE                          (0x00020000L)

#define S3D_ALPHA_BLEND_NONE                    (0x00000000L)
#define S3D_ALPHA_BLEND_TEXTURE_ALPHA           (0x00080000L)
#define S3D_ALPHA_BLEND_SOURCE_ALPHA            (0x000C0000L)


#define S3D_GOURAUD_SHADED_TRIANGLE             (0x00000000L)
#define S3D_LIT_TEXTURED_TRIANGLE               (0x08000000L)
#define S3D_UNLIT_TEXTURED_TRIANGLE             (0x10000000L)
#define S3D_PERSPECTIVE_LIT_TEXTURED_TRIANGLE   (0x28000000L)
#define S3D_PERSPECTIVE_UNLIT_TEXTURED_TRIANGLE (0x30000000L)
#define S3D_LINE_3D                             (0x40000000L)

#define S3D_TEXTURE_WRAP_ENABLE                 (0x04000000L)

#define S3D_COMMAND_3D                          (0x80000000L)


#define VIRGE3D_AUTOEXEC             0x00000001
#define VIRGE3D_HWCLIP               0x00000002
#define VIRGE3D_PIXELFORMAT_16BITS   0x00000004
#define VIRGE3D_PIXELFORMAT_24BITS   0x00000008

#define VIRGE3D_TXTEL

#define VIRGE_ZFUNC_NEVER       0x00000000
#define VIRGE_ZFUNC_GREATER     0x00100000
#define VIRGE_ZFUNC_EQUAL       0x00200000
#define VIRGE_ZFUNC_GEQUAL      0x00300000
#define VIRGE_ZFUNC_LESS        0x00400000
#define VIRGE_ZFUNC_NEQUAL      0x00500000
#define VIRGE_ZFUNC_LEQUAL      0x00600000
#define VIRGE_ZFUNC_ALWAYS      0x00700000

#define S3D_Z_COMP_NEVER                        (0x00000000L)
#define S3D_Z_COMP_S_GT_B                       (0x00100000L)
#define S3D_Z_COMP_S_EQ_B                       (0x00200000L)
#define S3D_Z_COMP_S_GE_B                       (0x00300000L)
#define S3D_Z_COMP_S_LT_B                       (0x00400000L)
#define S3D_Z_COMP_S_NE_B                       (0x00500000L)
#define S3D_Z_COMP_S_LE_B                       (0x00600000L)
#define S3D_Z_COMP_ALWAYS                       (0x00700000L)

#define S3D_Z_BUFFER_UPDATE                     (0x00800000L)

#define S3D_Z_BUFFER_NORMAL                     (0x00000000L)


#define VIRGE_DEPTHMASK         0x00000000
#define VIRGE_DEPTHWRITE        0x00800000

#define VIRGE_FLAT_TRI   0x837c0004
#define VIRGE_FLAT_Z_TRI 0x800c0004

#define VIRGE_FLAT_TXT_TRI (\
 VIRGE_IMG_SRC_DATA | \
 VIRGE_DEPTH16     | \
 VIRGE_DRAW_ENABLE | \
 VIRGE_ROP_COPY    | \
 VIRGE_DIRC_LR     | \
 VIRGE_DIRC_TB     | \
 VIRGE_CMD_FILLRECT )

#define VIRGE_FLAT_TXT_Z_TRI (\
 VIRGE_IMG_SRC_DATA | \
 VIRGE_DEPTH16     | \
 VIRGE_DRAW_ENABLE | \
 VIRGE_ROP_COPY    | \
 VIRGE_DIRC_LR     | \
 VIRGE_DIRC_TB     | \
 VIRGE_CMD_FILLRECT )


#define VIRGE_LIRP_TRI (\
 VIRGE_IMG_SRC_DATA | \
 VIRGE_DEPTH16     | \
 VIRGE_DRAW_ENABLE | \
 VIRGE_ROP_COPY    | \
 VIRGE_DIRC_LR     | \
 VIRGE_DIRC_TB     | \
 VIRGE_CMD_FILLRECT )

#define VIRGE_LIRP_Z_TRI (\
 VIRGE_IMG_SRC_DATA | \
 VIRGE_DEPTH16     | \
 VIRGE_DRAW_ENABLE | \
 VIRGE_ROP_COPY    | \
 VIRGE_DIRC_LR     | \
 VIRGE_DIRC_TB     | \
 VIRGE_CMD_FILLRECT )

#define VIRGE_LIRP_TXT_TRI (\
 VIRGE_IMG_SRC_DATA | \
 VIRGE_DEPTH16     | \
 VIRGE_DRAW_ENABLE | \
 VIRGE_ROP_COPY    | \
 VIRGE_DIRC_LR     | \
 VIRGE_DIRC_TB     | \
 VIRGE_CMD_FILLRECT )

#define VIRGE_LIRP_TXT_Z_TRI (\
 VIRGE_IMG_SRC_DATA | \
 VIRGE_DEPTH16     | \
 VIRGE_DRAW_ENABLE | \
 VIRGE_ROP_COPY    | \
 VIRGE_DIRC_LR     | \
 VIRGE_DIRC_TB     | \
 VIRGE_CMD_FILLRECT )


#define MmioRegWrite(mmReg,value)   (*((volatile GLuint *)((hwcx->regBase)+mmReg)))=value
#define MmioRegRead(mmReg)          (*((volatile GLuint *)((hwcx->regBase)+mmReg)))

#define WaitForQueue(x) { \
        {while(((MmioRegRead(0x8504) & 0x1F00) >> 8) < x);} \
}

#define WaitFifoEmpty() { \
        {while(((MmioRegRead(0x8504) & 0x1F00) >> 8) != 16 );} \
}

#define WaitForEngineIdle()         {while(!(MmioRegRead(0x8504) & 0x2000));}

#endif /* __gl_s3virge_h_ */
