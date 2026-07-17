/**************************************************************************
 *									  *
 * 		 Copyright (C) 1989, Silicon Graphics, Inc.		  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *									  *
 **************************************************************************/

#include "..\env\pc.h"

/*-----------------------------------------------------------------------
 |	Graphics test suite header file  -  $Revision: 2$           |
 -----------------------------------------------------------------------*/
#include <math.h>               /* for the floor stuff used by the pixel conversion */
#include <string.h>
#include <GL/glx.h>

/*************************************************************
*	Test type - Can be used in the spec file
*************************************************************/
#define OG_NORMAL		1		/* */
#define OG_IMAGED		2		/* */

/*************************************************************
*	Per architecture test skip options:
*************************************************************/
#define	OG_TSTBUG		0x1		/* ogtst bug */
#define	OG_GLBUG		0x2		/* X/GL bug */
#define	OG_CONTEXTBUG		0x4		/* context bug */
#define	OG_CRASHBUG		0x8		/* crash/hang bug */
#define	OG_DONOTRUN		0x10		/* do not run at all */

/*************************************************************
 *	Test specific Visual Properties Indications
 *
 * Unless otherwise indicated, these are attributes that a test
 * needs to be able to run.
 *************************************************************/
#define OG_RGB			0x1
#define OG_RGBA			0x2
#define OG_INDEX		0x4
#define OG_SINGLEBUF		0x8
#define OG_DOUBLEBUF		0x10
#define OG_STEREO		0x20
#define OG_AUXBUF		0x40
#define OG_DEPTH		0x80
#define OG_STENCIL		0x100
#define OG_ACCUM		0x200
#define OG_MULTISAMPLE		0x400
#define OG_OVERLAY		0x800
#define OG_RGB_ORL		0x1000
#define OG_RGB_ORL_A		0x2000
/*
 * The following two require that green and blue are zero sized.
 */
#define OG_LUMINANCE		0x4000
#define OG_LUMINANCE_A		0x8000

#define OG_VIS_CONSTRAINS	16 /* Number of defined visual constrains */
/*
 * The following is not a constraint.  It's meaning is that in addtion to
 * the normal run, for visuals that have AUX buffers, the test should also
 * be run with DRAW/READ_BUFFER set to an AUX buffer (this is done from the
 * main loop).  It does not mean that the visual has to have aux buffer for
 * the test to run on it.
 */
#define OG_AUXBUF_TOO		0x10000
/*
 * While the regular constraints say that the particular property is required
 * the one below specify whether the test can deal with a particular property.
 * If not specified, visuals with these properties will not be used by the
 * test.
 * Currently these are only luminance and rgb overlay which most tests probably
 * can't handle properly.
 */
#define OG_LUMINANCE_OK   	0x20000 /* luminance or luminance_alpah is OK */
#define OG_RGB_OVERLAY_OK	0x40000

/*************************************************************
*	Convenient stuff
*************************************************************/
#define NOCHECK       0xffffffff

/*************************************************************
    Structure used for querying the enviroment
*************************************************************/
typedef struct {
    GLboolean rgba_mode;
    GLboolean index_mode;
    GLboolean stereo;
    GLint index_bits;
    GLint rgba_bits[4];
    GLint depth_bits;
    GLint stencil_bits;
    GLint accum_bits[4];
    GLint aux_buff;
    GLint draw_buff;
    GLint read_buff;
} Environment, *EnvironmentPtr;

/*************************************************************
    Helpful Macros
*************************************************************/
#define ABS(a)          ((a)<0 ? -(a):(a))

/*
 * Conversion of between number formats.
 */
#define OGTST_B_TO_F(b)      ((((b)<<1) + 1) / (float) 0xff)
#define OGTST_F_TO_B(f)      ((GLbyte) floorf(((f) * 0xff) / 2))
#define OGTST_UB_TO_F(ub)    ((ub) / (float) 0xff)
#define OGTST_F_TO_UB(f)     ((GLubyte) ((f) * 0xff + 0.5))
#define OGTST_S_TO_F(s)      ((((s)<<1) + 1) / (float) 0xffff)
#define OGTST_F_TO_S(f)      ((GLshort) floorf(((f) * 0xffff) / 2))
#define OGTST_US_TO_F(us)    ((us) / (float) 0xffff)
#define OGTST_F_TO_US(f)     ((GLushort) ((f) * 0xffff + 0.5))
/*
** Not quite 2^31-1 will be rounded of anyhow in float.  So use
**  (2^24-1) * 2^8
*/
#define __FLOAT_2232_M_1 (((GLfloat) ((1 << 24)-1)) * (1 << 8))
#define OGTST_I_TO_F(i)      ((((GLfloat)(i) * 2) + 1) / __FLOAT_2232_M_1)
#define OGTST_F_TO_I(f)      ((GLint) floorf((f) * __FLOAT_2232_M_1 / 2))
#define OGTST_UI_TO_F(ui)    ((GLfloat)(ui) / __FLOAT_2232_M_1)
#define OGTST_F_TO_UI(f)     ((GLuint) ((f) * __FLOAT_2232_M_1 + 0.5))

/*************************************************************
*       Default Log levels for oglog
*************************************************************/
#define OG_LINTERNALDEBUG	 5	/* print debugging messages */
#define	OG_LDLIMM		 3	/* print display-list or immediate message */
#define OG_LPARAMETERS		 2	/* print GL parameters */
#define OG_LGENERAL		 1	/* print general messages */
#define	OG_LALWAYS		 0	/* always print message */
#define	OG_LFAIL		-1	/* test failure */
#define OG_LSKIP		-2	/* skip test due to failure */
#define OG_LINTERNALERROR	-3	/* abort test due to failure */
#define OG_LUSERERROR		-4	/* abort test due to failure */
#define	OG_LNEVER 		-5	/* silent mode */

/*************************************************************
*	Immediate or display-list modes
*************************************************************/
#define DL_PROLOG() int obj

#define START_DL_OR_IM(dl)                  				\
    glGetIntegerv(GL_LIST_INDEX,&obj);					\
    if (!obj && !glIsList(dl)) {       					\
      obj = ogLibDlistRand();						\
      switch(obj) {							\
      case 0:								\
	ogEnvLog(OG_LDLIMM,"immediate mode\n");                         \
        break;								\
      case 1:								\
        glNewList(dl, GL_COMPILE);					\
        ogEnvLog(OG_LDLIMM,"display-list compile mode\n");		\
	break;								\
      case 2:								\
	glNewList(dl, GL_COMPILE_AND_EXECUTE);				\
	ogEnvLog(OG_LDLIMM,"display-list compile-and-execute mode\n");	\
	break;								\
      }	                                				\
    } else { obj = 0; ogEnvLog(OG_LDLIMM,"immediate mode\n"); }        	\

#define FINIS_DL_OR_IM(dl)				\
    if (obj) {						\
       glEndList();					\
       if (obj == 1) glCallList(dl);			\
       glDeleteLists(dl,1);				\
       ogEnvLog(OG_LDLIMM,"end of display-list mode\n");\
    }                                   		\

/*************************************************************
* Macro to exit if Extension isn't supported
* exten_name should be a string or string constant with the
* extension name as returned by glGetString()
*************************************************************/

#define SUPPORTED_EXTENSION(exten_name)                      \
          if(!glGetString(GL_EXTENSIONS) ||                  \
          !strstr((const char *)glGetString(GL_EXTENSIONS), exten_name))   \
	  ogEnvLog(OG_LSKIP, "Unsupported Extension: %s\n",  \
	           exten_name);                              \

#define IS_ONEONE()	(ogEnvVersion() > 0)

/*************************************************************
*       Macro for test entry points
*************************************************************/
#if defined(__STDC__)
#define TESTMOD(mod)            void mod ## _test(int pass)
#define CLEANUP(mod)            void mod ## _clean()
#else
#define TESTMOD(mod)            void mod/**/_test(int pass)
#define CLEANUP(mod)            void mod/**/_clean()
#endif

/*************************************************************
*	Defines for HW type
*************************************************************/
#define	OG_GENERIC		0x0001
#define	OG_VG1		        0x0002

/*************************************************************
*	Defines for current test attributes
*************************************************************/
#define OG_XWSIZE		0		/* */
#define OG_YWSIZE		1		/* */
#define OG_XW0			2		/* */
#define OG_XW1			3		/* */
#define OG_YW0			4		/* */
#define OG_YW1			5		/* */
#define OG_HW			6		/* */
#define OG_FINISH               7               /* for stress_ debugs */

/*************************************************************
 *	env prototypes - used by files outside env
 *************************************************************/
/* inout.c */
extern void		ogEnvLog(int, char *, ...);
extern int		ogEnvWillOutput(int);
extern void		ogEnvPartialPass(void);
extern const char *	ogEnvColorString(GLuint color);
extern const char *	ogEnvDataTypeName(GLenum type);
extern const char *	ogEnvPixelFormatName(GLenum format);
/* main.c */
extern int		ogEnvQuery(int);
extern int		ogEnvDoingAuxBuffer(void);
/* wind.c */
extern void		ogEnvMultiSamplingState(int mode);
#ifdef WIN32
extern HWND		ogEnvWindow(void);
extern HPALETTE		ogEnvColormap(void);
#else
extern Display *	ogEnvDisplay(void);
extern Window		ogEnvWindow(void);
extern Colormap		ogEnvColormap(void);
extern Visual *		ogEnvVisual(int);
extern Visual *		ogEnvCurVisualVisual(void);
#endif
extern char *		ogEnvVisualString(int vindex);
extern char *		ogEnvCurVisualString(void);
extern int		ogEnvGLXVisualInfo(int vindex, int attr);
extern int		ogEnvCurVisualInfo(int attr);
extern int		ogEnvIsCIMode(void);
extern int		ogEnvIsMultiSampled(void);
extern int		ogEnvIsDoubleBuffered(void);
extern int		ogEnvIsStereo(void);
extern int		ogEnvIsTransparent(void);
extern int		ogEnvIsDualPersonality(void); 
extern void		ogEnvColorBits(int *, int *, int *, int *, int *);
extern void		ogEnvSwapBuffers(void);
extern void		ogEnvInitFont(char *, int *, int *);
extern int		ogEnvVersion(void);

/*************************************************************
*	lib prototypes
*************************************************************/
/* alloc.c */
extern GLvoid	ogLibMemInit(GLvoid);
extern GLvoid*	ogLibMalloc(GLint size);
extern GLvoid	ogLibFree(GLvoid *data);
extern GLvoid	ogLibFreeAll(GLvoid);
extern GLint	ogLibCheckAllFreed(GLvoid);
extern GLvoid   ogLibCleanMem(int level);

/* check.c */
extern void	ogLibCheckPerVisualInit(void);
extern int	ogLibColCheck(GLuint,GLuint,int);
extern int	ogLibPixelCheck(int,int,unsigned int);
extern int	ogLibRectCheck(int,int,int,int,unsigned int, unsigned int);
extern int	ogLibRectEdgeCheck(int,int,int,int,unsigned int, unsigned int);
extern int	ogLibRectBufCheck(GLuint *buff, int x1,int y1, int x2, int y2,
                                  GLuint inc, GLuint outc, GLboolean filled,
                                  GLuint inError, GLuint outError);
extern int 	ogLibRangeCheck(unsigned int, unsigned int, unsigned int);
extern int	ogLibPointCheck(int,int,unsigned int,unsigned int,
                                unsigned int);
extern int	ogLibLineBufCheck(GLuint *buff, int x1, int y1, int x2, int y2,
                                  int size, GLuint inc, GLuint outc,
                                  GLuint inError, GLuint outError);
extern void	ogLibCheckIgnoreMultiSampling(GLboolean mode);

/* defaultState.c */
extern void	ogLibSetDefaultBuffers(void);
extern void	ogLibSetDefaultColors(void);
extern void	ogLibSetDefaultClears(void);
extern void	ogLibSetDefaultLight(void);
extern void	ogLibSetDefaultTextures(void);
extern void	ogLibSetDefaultDetail(void);
extern void	ogLibSetDefaultSharpen(void);
extern void	ogLibSetDefaultEvaluators(void);
extern void	ogLibSetDefaultRasterPos(void);
extern void     ogLibSetDefaultBlend(void);
extern void     ogLibSetDefaultMatrices(void);
extern void     ogLibSetDefaultFog(void);
extern void     ogLibSetDefaultConvolution(void);
extern void	ogLibSetDefaultPixelStore(void);

/* getenv.c */
extern void	ogLibGetEnvironment(EnvironmentPtr ep);
extern void	ogLibShowEnvironment(void);
/* gltst.c */
extern void	ogLibClear(GLuint);
extern GLuint	ogLibColor(void);
extern GLuint	ogLibClearColor(void);
extern void	ogLibDrawFragments(GLint, GLint);
extern void	ogLibDefaultState(void);
extern const char *ogLibGLError(GLenum error);
/* matrix.c */
extern void	ogLibSetEtol(float); /* set error tolerance */
extern float	ogLibGetEtol(void);  /* get error tolerance */
extern void	ogLibPntMatrix(float *, GLint, char *); /* print matrix */
extern void	ogLibUntMatrix(float *); /* identity matrix */
extern void	ogLibSetMatrix(float *, float); /* set to constant */
extern void	ogLibCpyMatrix(float *, float *); /* copy matrix */
extern void	ogLibTnsMatrix(float *, float *); /* transpose matrix */
extern void	ogLibAbsMatrix(float *, float *); /* absolute value */
extern void	ogLibAdsMatrix(float *, float *, float); /* add constant */
extern void	ogLibAddMatrix(float *, float *, float *); /* add matrices */
extern void	ogLibSubMatrix(float *, float *, float *); /* subtract matrices */
extern void	ogLibMusMatrix(float *, float *, float); /* multiply by constant */
extern void	ogLibMulMatrix(float *, float *, float *); /* multiply matrices */
extern void	ogLibVecMatrix(float *, float *, float *); /* mul vector by matrix */
extern void	ogLibInvMatrix(float *, float *); /* inverse matrix */
extern float	ogLibDetMatrix(float *); /* matrix determinant */
extern void	ogLibRcmMatrix(float *, float *); /* relative matrix err */
extern void	ogLibAcmMatrix(float *, float *); /* absolute matrix err */
extern void	ogLibRcmVector(float *, float *); /* relative vector err */
extern void	ogLibAcmVector(float *, float *); /* absolute vector err */
extern void	ogLibFCompare(char *, float, float); /* relative float err */
/* rand.c */
extern GLint	ogLibGetSeed(void);
extern void	ogLibSetSeed(int);
extern GLint	ogLibBitRand(int);
extern GLint	ogLibIntRand(GLint,GLint);
extern float	ogLibExpRand(int);
extern float	ogLibFloatRand(float,float);
extern GLint	ogLibDlistRand(void);
extern void	ogLibOrderRand(int *order, int size);
/* readwrite.c */
extern void	ogLibReadPixels(int,int,int,int,GLuint *);
/* set.c */
extern int	ogLibSetVertex(int,int,int,int);
extern int	ogLibSetColor(int,int,int,int);
extern void	ogLibSetRead(GLvoid (*)(const GLvoid *), GLenum,
                                     GLfloat *, GLvoid *);
extern int	ogLibSetReadSet(GLvoid (*)(const GLfloat *), 
                                GLvoid (*)(const GLvoid *),
                                GLenum, GLenum, GLfloat *, GLvoid *,
                                GLint, GLboolean);
/* texsim.c */
extern GLint	ogLib4Ub(char *func, GLubyte rubt, GLubyte gubt, GLubyte bubt,
                         GLubyte aubt, float crf, float cgf, float cbf,
                         float caf);
extern GLint	ogLib4Us(char *func, GLushort rust, GLushort gust,
                         GLushort bust, GLushort aust, float crf,
                         float cgf, float cbf, float caf);
extern GLint	ogLib3Ub(char *func, GLubyte rubt, GLubyte gubt, GLubyte bust,
                         float crf, float cgf, float cbf, float caf); 
extern GLint	ogLib4Ui(char *func, GLuint ruit, GLuint guit, GLuint buit,
                         GLuint auit, float crf, float cgf, float cbf,
                         float caf);
extern GLint	ogLib3Us(char *func, GLushort rust, GLushort gust,
                         GLushort bust, float crf, float cgf, float cbf,
                         float caf);
extern GLint	ogLib3Ui(char *func, GLuint ruit, GLuint guit, GLuint buit,
                         float crf, float cgf, float cbf, float caf);
extern GLint	ogLib2Ub(char *func, GLubyte lubt, GLubyte aubt, float crf,
                         float cgf, float cbf, float caf);
extern GLint	ogLib2Ui(char *func, GLuint luit, GLuint auit, float crf,
                         float cgf, float cbf, float caf);
extern GLint	ogLib2Us(char *func, GLushort lust, GLushort aust,
                         float crf, float cgf, float cbf, float caf);
extern GLint	ogLib1Ub(char *func, GLubyte lubt, float crf, float cgf,
                         float cbf, float caf);
extern GLint	ogLib1Ui(char *func, GLuint luit, float crf, float cgf,
                         float cbf, float caf);
extern GLint	ogLib1Us(char *func, GLushort lust, float crf, float cgf,
                         float cbf, float caf);
/* evaluator.c */
extern void	ogLibDeCasteljau1(float u, int order, int ncomp, float *pts,
				  float *res);
extern void	ogLibDeCasteljau2(float u, int uorder, float v, int vorder,
				  int ncomp, float *pts, float *res);

/* util.c */
extern void ogLibParseColor(GLuint, GLubyte *, GLubyte *, GLubyte *, GLubyte *);
extern void ogCheckErrors(void);

/* pixutil.c */
extern int ogLibBytesPerComp(GLenum type);
extern int ogLibCompsPerPixel(GLenum format);
extern int ogLibImageSize(int width, int height, GLenum type, GLenum format);

/* texutil.c */
extern void *ogLibGenTriTex(int, int, GLenum,  GLenum, unsigned char *,
                            unsigned char *);
extern void *ogLibGenTriTex(int, int, GLenum, GLenum, unsigned char *,
                            unsigned char *);
extern void *ogLibGenSolidTex(int, int, GLenum, GLenum, unsigned char *);
extern void *ogLibGenFrameTex(int, int, GLenum, GLenum, unsigned char *,
                              unsigned char *);
extern void *ogLibGenLabelTex(int, int, GLenum, GLenum, unsigned char *,
                              unsigned char *, char *);


