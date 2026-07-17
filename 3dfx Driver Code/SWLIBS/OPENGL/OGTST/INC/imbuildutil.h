#ifndef IMBUILDUTIL_H
#define IMBUILDUTIL_H

#define IU_I_TO_FLOAT(i)      ((((GLfloat)(i) * 2) + 1) / (double) 4294966784)
#define IU_UI_TO_FLOAT(ui)    ((GLfloat)(ui) / (double) 4294966784)
#define IU_B_TO_FLOAT(b)      ((((b)<<1) + 1) / 255.0)
#define IU_UB_TO_FLOAT(ub)    ((ub) / 255.0)
#define IU_S_TO_FLOAT(s)      ((((s)<<1) + 1) / 65535.0)
#define IU_US_TO_FLOAT(us)    ((us) / 65535.0)

#define IU_FLOAT_TO_B(f)      ((GLbyte)  (((f) * 255) / 2))
#define IU_FLOAT_TO_UB(f)     ((GLubyte) ((f) * 255 + 0.5))
#define IU_FLOAT_TO_S(f)      ((GLshort) (((f) * 65535) / 2))
#define IU_FLOAT_TO_US(f)     ((GLushort)((f) * 65535 + 0.5))
#define IU_FLOAT_TO_I(f)      ((GLint)   (((f) * (double) 4294966784) / 2))

/*
 * Conversion from float to unsigned int gives different results on
 * N32 systems as compared to -32. Use intermediate casting to (long
 * long) to work around this problem
 */

#if defined(_MIPS_SIM_NABI32) || defined(_MIPS_SIM_ABI64)
#define IU_FLOAT_TO_UI(f)     ((GLuint) ((long long) ((f) * (double) 4294966784 + 0.5)))

#else  /* old 32 bit mode */
#define IU_FLOAT_TO_UI(f)     ((GLuint)  ((f) * (double) 4294966784 + 0.5))
#endif

/* Image type */
#define IU_CHECKER   (0)
#define IU_RAMP      (1)
#define IU_REFERENCE (2)
#define IU_RANDOM    (3)

typedef struct {
    GLsizei width, height;
    GLenum  format, type;
    GLint   alignment, swap, lsb;
    GLvoid *pImage;
} IU_Image;

typedef struct {
    float r,g,b,a;
} IU_FColor;

typedef struct {
    unsigned char r,g,b,a;
} IU_UBColor;

typedef struct {
    char r,g,b,a;
} IU_BColor;

typedef struct {
    unsigned short r,g,b,a;
} IU_USColor;

typedef struct {
    short r,g,b,a;
} IU_SColor;

typedef struct {
    int r,g,b,a;
} IU_IColor;

typedef struct {
    unsigned int r,g,b,a;
} IU_UIColor;

typedef struct {
    IU_FColor fColor;

    IU_UBColor ubColor;
    IU_BColor  bColor;

    IU_USColor usColor;
    IU_SColor sColor;

    IU_IColor iColor;
    IU_UIColor uiColor;

} IU_AllColor;

typedef struct {
    float fIndex;

    unsigned char ubIndex;
    char bIndex;

    unsigned short usIndex;
    short sIndex;

    unsigned int uiIndex;
    int iIndex;
} IU_AllIndex;

typedef struct {
    IU_AllColor color0, color1;
    IU_AllIndex index0, index1;
    GLint xMod, yMod;
} IU_Checker;

typedef struct {
    IU_FColor color0, color1;
    GLfloat index0, index1;
} IU_Ramp;

typedef struct {
    GLfloat x1, y1, x2, y2;
} IU_Rect;

typedef struct {
    IU_Rect screen;
    GLint width, height;
    GLint curXTile, curYTile;
    GLint numHorTile, numVerTile;
} IU_Tile;

extern GLint 
IU_NumCompInPix(GLenum format);

GLint 
IU_NumByteInType(GLenum type);

extern GLint  
IU_NumByteInImage(IU_Image *pI);
extern void   
IU_InitChecker(IU_FColor col0, IU_FColor col1,
	       GLubyte i0, GLubyte i1, 
	       GLint xMod, GLint yMod);
extern void
IU_InitRamp(IU_FColor col0, IU_FColor col1, 
	    GLubyte i0, GLubyte i1);
extern GLint   
IU_BuildImage(IU_Image *pI, GLint imageType);

extern GLenum  
IU_FormatNameToEnum(char * pStr);
extern GLenum  
IU_TypeNameToEnum(char * pStr);
extern char * 
IU_TypeName(GLenum type);
extern char * 
IU_FormatName(GLenum type);


extern IU_Rect 
IU_TileLocation(GLint x, GLint y);
extern IU_Rect 
IU_NextTileLocation(GLvoid);

extern GLvoid    
IU_InitTile(IU_Rect rect, GLint numHorTile, GLint numVertTile);

extern GLvoid
IU_InitBitmapToRGB(IU_FColor col0, IU_FColor col1);

extern GLvoid
IU_InitIndexToRGB(GLvoid);

extern GLvoid   
IU_Verbose(GLint on);

extern GLint
IU_Validate_NumPixInLine(GLint baseNumByteInLine, GLint numByteInLineMod8, 
			 GLint numInvalidByte, 
			 GLenum format, GLenum type, 
			 GLint *pNumPixInLine, GLint *pNumByteInLine);

extern GLint
IU_Validate_Alignment(GLint numByteInLine, GLint numByteInRow, 
		      GLint alignment);


extern int refImage[];

#endif /* IMBUILDUTIL_H */
