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

#include "pc.h"

/*-----------------------------------------------------------------------
 |	Graphics test suite header file  -  $Revision: 2$           |
 -----------------------------------------------------------------------*/
typedef	void	(*TestFunc)(int); /* A test function pointer */
typedef	void	(*CleanupFunc)(void); /* A test cleanup function pointer */

/* group information */
typedef struct _Group {
   char *name;
   int nTests;			/* number of tests in group */
   short *tests;                /* Array of indices of the group tests in
                                 * the tests array
                                 */
} Group;

/* Test Info */
typedef struct {
    /* From the spec file */
    char *name;
    /*
     * HW specific information.
     * Compiled in as an array with an entry for each architecture.
     * At run time entry 0 is replaced by the information for the
     * architecture being tested.
     */
    struct {
        int loop;               /* number of passes */
        float toler;            /* arch specific tolerance */
        int skips;              /* test skip conditions */
    } archSpecs[12];
    unsigned int visuals;        /* visual constraints */
    unsigned int type;           /* checksumed of normal */
    Group *group;
    TestFunc test;              /* the test itself */
    CleanupFunc	cleanup;        /* post-test cleanup function */
    /* Dynamic info */
    unsigned int expectedChksumFB, expectedChksumBB,
		 returnedChksumFB, returnedChksumBB; 
    int	runstatus;              /* cumulative run flags */
} Test;

/* window information */
typedef struct {			/* window state */
   int		wx0, wx1, wy0, wy1;	/* window parameter */
   int		xsize, ysize;		/* window size */
   int		cur_visual;		/* current visual */
   int		num_visual;		/* number of visuals */
} wist_t;

/* all options must be stored in here */
typedef struct {
   int 		errcount;		/* current errors */
   int 		maxerr;			/* maximum errors allowed */
   int		partial;		/* partial pass */
   int 		plvl;			/* ogEnvLog level */
   int 		pmsk;			/* ogEnvLog mask */
   float	passes;			/* test run extended  */
   int 		runsec;			/* test time extender */
   int 		debugged;		/* debugging options */
   int 		seed;			/* seed for tests */
   int 		flags ;			/* user misc flags */
   int 		option;			/* user run  flags */
   int 		kskip;			/* user skip flags */
   int 		hwtype;			/* graphics type */
   char		hwname[64];		/* graphics name */
   int		vmask;			/* visual attributes mask */
   int		doingAuxBuffer;         /* currently testing an aux buffer */
   int		doingPbuffer;           /* currently testing a pbuffer */
} opts_t;

typedef struct {
   int         xsiz, ysiz, zsiz;
   void        *buf;       /* either (Colorindex *) or (GLuint *) */
} gimg;

/*************************************************************
*	Defines for opts.flags
*************************************************************/
#define FLAG_RANDORDER 	           0x0001  /* */
#define FLAG_GSKIP	           0x0002  /* */
#define FLAG_NOTIME 	           0x0004  /* */
#define FLAG_HOLD 	           0x0008  /* */
#define FLAG_ONEVIS	           0x0010  /* */
#define FLAG_LOGGED	           0x0020  /* */
#define FLAG_UPDATED 	           0x0040  /* */
#define FLAG_NODUMP 	           0x0080  /* */
#define FLAG_2NAME 	           0x0100  /* */
#define FLAG_MEMUSE 	           0x0200  /* */
#define FLAG_SIMULATE	           0x0400  /* */
#define FLAG_FORCDUMP	           0x0800  /* */
#define FLAG_PIXMAP	           0x1000  /* */
#define FLAG_ERRHOLD	           0x2000  /* */
#define FLAG_COLOR	           0x8000  /* Turn color error down */
#define FLAG_CTXSW                0x10000  /* Enable context-switch tests */
#define FLAG_NORDBACK             0x20000  /* No pixel readback -> stress */
#define FLAG_STATISTICS           0x40000  /* Print statistic info */
#define FLAG_PBUFFERS             0x80000  /* Running tests on pbuffers as well */
#define FLAG_PBUFFERS_ONLY       0x100000  /* Running tests on pbuffers only */
#define FLAG_NONPRESEVED_PB      0x200000  /* Use non preserved pbuffers */
#define FLAG_PB_WINDOWS_MAPPED   0x400000  /* Use non preserved pbuffers */
#define FLAG_COLORPRINT          0x800000  /* print results in color */
#define FLAG_ALTERNATE           0x1000000 /* print results in another format */
#define FLAG_NUM_VIZ             0x2000000 /* print results for number of visuals passed in a test */

/*************************************************************
*	Defines for opts.bufferMode
*************************************************************/
#define NORMAL_BUFFERS	0
#define AUX_BUFFER	1
#define PBUFFERS	2

/* Please note ANSI's acpp treats ## as concatenate and # as a way to get "" */
/* __STDC__ implies ANSI */
/*************************************************************
*	Macro for test groups
*************************************************************/
#define BEGINLIST		atmp_t atmpdata[] = {
#if defined(__STDC__)
#define	ADDMOD(mod) { (TestFunc)mod ## _test,(CleanupFunc)mod ## _clean,# mod },
#else
#define	ADDMOD(mod) { (TestFunc)mod/**/_test,(CleanupFunc)mod/**/_clean, "mod" },
#endif
#define ENDLIST			0,0,0 };

/*************************************************************
*	Runtime flags
*************************************************************/
#define	OPTION_FIRST_LAST	0x0001
#define	OPTION_SKIP_IT		0x0002

#define	OPTION_A		0x0010
#define	OPTION_T		0x0020
#define	OPTION_a		0x0040
#define	OPTION_t		0x0080

#define	OPTION_gA		0x0100
#define	OPTION_gT		0x0200
#define	OPTION_ga		0x0400
#define	OPTION_gt		0x0800

#define	OPTION_lA		0x1000
#define	OPTION_lT		0x2000
#define	OPTION_la		0x4000
#define	OPTION_lt		0x8000


/* Test Passing Criterion by Majority -- What Fraction of Visuals */
#define	TEST_PASSING_GRADE	0.5

/* From spec.c */
extern Test tests[];		/* Tests info */
extern Group groups[];          /* Test groups info */
extern int nTests, nGroups;

/*************************************************************
*	env prototypes - used only by files in env
*************************************************************/
/* ctxsw.c */
extern int		ogEnvInitDummyContext(void);
extern void		ogEnvCtxswHook(void);
/* getstatecheck.c */
extern void ogEnvInitStateChecker(void);
extern void ogEnvCheckDefaultState(GLboolean);
/* imutil.c */
extern GLuint		ogEnvImageSnap(Test *, GLint, GLint, GLint, GLint);
extern void		ogEnvImageSave(char *, gimg *);
/* info.c */
extern void		ogEnvPrintHelp(void);
/* inout.c */
extern void		ogEnvParse(int,char **);
extern void		ogEnvGend(Test *, int);
extern void             ogEnvStatistics(void);
extern void             ogEnvInitStatistics(void);
extern int		ogEnvReadSpecData(void);
extern int		ogEnvReadAtmpData(void);
extern void		ogEnvReadChecksumData(void);
extern void		ogEnvWriteChecksumData(void);
/* main.c */
extern int		ogEnvFindGroup(char *);
extern int		ogEnvFindTest(char *);
extern int		ogEnvHexConvert(char *);
extern void		ogEnvCheckSum(Test *, unsigned int, gimg *);
/* wind.c */
extern int		ogEnvInitVisual(int);
extern void             ogEnvUnMapWindow(int);
extern void		ogEnvRgbaMode(int);
extern GLboolean	ogEnvToggleRenderMode(int);
extern void		ogEnvPostVisual(int);
extern int		ogEnvXScreenSize(void);
extern int		ogEnvYScreenSize(void);
extern void		ogEnvWInit(void);
extern void		ogEnvSelectVisuals(int *, int, GLboolean);
extern int		ogEnvHasPbuffers(void);
extern int		ogEnvHasFBConfigs(void);
extern int		ogEnvLegalVisual(int, int);
extern void		ogEnvTime(void);
extern void             ogEnvMajorityPass(int);
