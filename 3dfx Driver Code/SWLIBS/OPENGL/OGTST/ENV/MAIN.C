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

/* main.c - $Revision: 2$ */

#include <stdlib.h>
#include <strings.h>
#include <stdio.h>
#include <setjmp.h>
#include <time.h>
#include "ogtst.h"
#include "env.h"
#include "architectures.h"

wist_t windata;			/* window data */
opts_t opts;			/* options */
jmp_buf jmpenv;

FILE *logfile;

int numrun;                     /* number of tests to be run */
int *runarray;                  /* which tests to be run */

static int runindex = 0;

Test *ctst;

static void	initialize(int, char **);
static void	runTest(Test *);
int	        skipCheck(Test *, int);
static void	initTests(void);
static Test    *getTest(void);
static void     gethw(void);
static void	setHWConditions(void);
static void     memoryUse(void);
static int      imagedVisual(int);


/****************************************************************************
*  main()
****************************************************************************/
int __cdecl
main(int argc, char *argv[])
{
    GLint bgntime;

    /* initialize gtst data */
    initialize(argc, argv);
    /* get initial time */
    bgntime = time(0);
    {
      /* gross workaround for people who use tcsh; needed for -H option */
      char *cmd = getenv("CMDLINE");
      if (cmd && !strcmp(cmd, "tcsh.exe") && (opts.flags & FLAG_HOLD)) {
	getchar();
      }
    }
    if (opts.flags & FLAG_MEMUSE) {
	fprintf(stderr, "ogtst: calling memoryUse()... \n");
	memoryUse();
    }
    if ((opts.flags & (FLAG_PBUFFERS | FLAG_PBUFFERS_ONLY)) !=
        (FLAG_PBUFFERS | FLAG_PBUFFERS_ONLY)) {
        ogEnvInitStatistics();
        do {
            initTests();
            while (ctst = getTest()) {
                if (skipCheck(ctst, 1)) {
                    runTest(ctst);
                    if (opts.flags & FLAG_MEMUSE)
                        memoryUse();
                }
            }
        } while ((time(0) - bgntime) < opts.runsec);	/* end of do loop */
        if (opts.flags & FLAG_UPDATED)
            ogEnvWriteChecksumData();
        if (opts.flags & FLAG_STATISTICS)
            ogEnvStatistics();
    } else {
        ogEnvLog(OG_LALWAYS, "Skipping windows/pixelmap tests (-d o) option\n");
    }
    if (opts.flags & FLAG_PBUFFERS) {
        ogEnvLog(OG_LALWAYS, "============================ PBUFFER TESTS " \
                 "============================\n");
        opts.doingPbuffer = 1;
        if (!(opts.flags & FLAG_PB_WINDOWS_MAPPED)) {
	    int i;
            for (i = 0; i < windata.num_visual; i++)
                ogEnvUnMapWindow(i);
	}
        ogEnvInitStatistics();
        do {
            initTests();
            while (ctst = getTest()) {
                if (skipCheck(ctst, 1)) {
                    runTest(ctst);
                    if (opts.flags & FLAG_MEMUSE)
                        memoryUse();
                }
            }
        } while ((time(0) - bgntime) < opts.runsec);	/* end of do loop */

        if ((opts.flags & FLAG_UPDATED) && (opts.flags & FLAG_PBUFFERS_ONLY))
            ogEnvWriteChecksumData();

        if (opts.flags & FLAG_STATISTICS)
            ogEnvStatistics();
    }
    ogEnvLog(OG_LALWAYS, "=== OGTST Done ===\n");
    return 0;
}

/****************************************************************************
*  initialize()  -  initialization before parse command line
****************************************************************************/
static void
initialize(int argc, char *argv[])
{
    int i;
    int w, h;

    /* initialize options */
    opts.errcount = 0;
    opts.maxerr = 10;
    opts.plvl = 0;
    opts.pmsk = 0;
    opts.passes = 1;
    opts.partial = 0;
    opts.runsec = 0;
    opts.debugged = 0;
    opts.seed = 0;
    opts.flags = 0;
    opts.option = 0;
    opts.kskip = 0;
    opts.hwtype = 0;
    opts.vmask = 0;
    opts.doingPbuffer = 0;
    opts.doingAuxBuffer = 0;
    if (!strcmp(argv[0], "gtsti"))
	opts.flags |= FLAG_SIMULATE;
    numrun = 0;
    runarray = (int *)malloc(nTests * sizeof(int));
    bzero(runarray, nTests * sizeof(int));
    logfile = stdout;
    /* are we logged ? */
    ogEnvWInit();
    /* get hardware info */
    gethw();
    /* Set hardware specific conditions for each test */
    setHWConditions();
    /* default window */
    w = 640;
    h = 512;
    if (opts.hwtype == OG_VG1) {
      h = 480;
    }
    windata.wx0 = (ogEnvXScreenSize() - w) / 2;
    windata.wx1 = windata.wx0 + w - 1;
    windata.wy0 = (ogEnvYScreenSize() - h) / 2;
    windata.wy1 = windata.wy0 + h - 1;
    windata.xsize = w;
    windata.ysize = h;
    /* parse command line */
    if (argc > 1)
	ogEnvParse(argc, argv);
    /* are we logged ? */
    /* dump system data if logged */
    if (opts.flags & FLAG_LOGGED && !(opts.flags & FLAG_SIMULATE)) {
	fprintf(logfile, "Command Data: ");
	for (i = 0; i < argc; i++)
	    fprintf(logfile, "%s ", argv[i]);
	fprintf(logfile, "\n");
    } 
    if (opts.flags & FLAG_LOGGED && !(opts.flags & FLAG_SIMULATE)) 
	fprintf(logfile, "Random  Seed: %d\n\n", opts.seed);
    if ((opts.flags & FLAG_PIXMAP) && (opts.flags & FLAG_PBUFFERS))
        fprintf(logfile, "Warning: both pixmaps and pbuffers requested\n");
    /* read in and attach checksum data */
    ogEnvReadChecksumData();
}

/*
 * Are we in the aux buffer phase of the test.
 */
int
ogEnvDoingAuxBuffer(void)
{
    return opts.doingAuxBuffer;
}

/****************************************************************************
*  runTest()  -  routine to run each test case
****************************************************************************/
static void 
runTest(Test *tst)
{
    float doer;
    int vindex, ranIt = 0, i;
    int ranVis = 0;

#define OG_RAN_FRONT 1
#define OG_RAN_BACK  2

    /* run different applicable visuals */
    for (vindex = 0; vindex < windata.num_visual;) {
        /* reject visuals unsuitable for imaged tests */
        if ((tst->type & OG_IMAGED) && !imagedVisual(vindex))
            goto endLoop;

	/* Run IMAGED tests only once in front and once in back buffer */
	if (tst->type & OG_IMAGED) {
	    int type = ogEnvGLXVisualInfo(vindex, GLX_DOUBLEBUFFER) ?
		OG_RAN_BACK : OG_RAN_FRONT;
	    if (ranVis & type) {
		vindex++;
		continue;
	    }
	    ranVis |= type;
	}

        /*
         * Imaged tests need only run once. However, to enable an imaged
         * test to run on aux buffers, even after the test has run on the
         * normal color buffers, we keep trying to find a visual that will
         * allow it to run on aux buffers.
         */
        for (i = 2 - ranIt, opts.doingAuxBuffer = ranIt; i;
             i--, opts.doingAuxBuffer = !opts.doingAuxBuffer) {
            if (ogEnvLegalVisual(vindex, tst->visuals)) {
                /* print extra test name */
                if (opts.flags & FLAG_2NAME) {
                    if (!(opts.flags & FLAG_NOTIME))
                        ogEnvTime();
                    ogEnvLog(OG_LALWAYS, "Run Test: %s on %s\n", tst->name,
                             ogEnvVisualString(vindex));
                }
                /* clear skip flag */
                opts.flags &= ~FLAG_GSKIP;
                /* initialize error count */
                opts.errcount = 0;
		/* clear partial pass flag */
		opts.partial = 0;
                /* set the error tolerance for the test */
                ogLibSetEtol(tst->archSpecs[0].toler);
                /* set random number seed */
                ogLibSetSeed(opts.seed);
                /* Initialize the visual */
                if (ogEnvInitVisual(vindex)) {
                    ranIt++;
                    /* Check the attribute state before the test is run */
                    if (opts.debugged & 2)
                        ogEnvCheckDefaultState(GL_TRUE);
                    ogLibCheckPerVisualInit();
                    /* iteration count for test */
                    doer = (float) 0.1 * opts.passes * tst->archSpecs[0].loop;
                    /* Enable context-switch flag just before test */
                    /* use setjmp to jump out of test for too many errors */
                    if (setjmp(jmpenv) == 0)
                        tst->test((doer < 1.0) ? 1 : (int) doer);
                    /* cleanup after test (test can force GSKIP) */
                    if (!(opts.flags & FLAG_GSKIP))
                        tst->cleanup();
                    /* Check the attribute state after the test is run */
                    if (opts.debugged & 2)
                        ogEnvCheckDefaultState(GL_FALSE);
                    /* non-simulation mode */
                    if (!(opts.flags & FLAG_SIMULATE)) {
                        /* for non-aborted test */
                        if (!(opts.flags & FLAG_GSKIP)) {
                            /*
                             * for checksum tests, get image and dump it if
			     * checksum fails 
                             */
                            if ((tst->type & OG_IMAGED) &&
                                !(opts.flags & FLAG_NORDBACK))
                                ogEnvImageSnap(tst, 0, 0,
					       ogEnvQuery(OG_XWSIZE)-1,
                                               ogEnvQuery(OG_YWSIZE)-1);
                        }
                        /* print out end message of a test case */
                        ogEnvGend(tst, vindex);
                        /* execute post function of a display mode */
                        ogEnvPostVisual(vindex);
                        /* hold option */
                        if ((opts.flags & FLAG_HOLD) ||
                            opts.errcount && (opts.flags&FLAG_ERRHOLD)) {
                            fprintf(stderr, "Hit <return> to continue");
                            getchar();
                            if (opts.doingPbuffer)
                                ogEnvUnMapWindow(vindex);
                        }
                    }
                }
            }
        }
        /* IMAGED tests only run one visual */
        if (ranIt && (opts.flags & FLAG_ONEVIS)) {
            if (!(tst->visuals & OG_AUXBUF_TOO) || ranIt == 2)
                break;
        } else
            ranIt = 0;
      endLoop:
	/*
	 * Dual personality visuals always start as color index.
	 */
        if (!ogEnvToggleRenderMode(vindex))
            vindex++;
    }
}

/****************************************************************************
*  skipCheck()  -  check test skip conditions
*
*                       rules:
*                           priority (high --> low)
*                            high   1. run > avoid
*                             |     2. command_line > file
*                            low    3. test > group
*                       flag values:
*                           -1 : not specified by options
*                            1 : specified to run
*                            0 : specified to avoid
*
****************************************************************************/
int 
skipCheck(Test *tst, int fromMainLoop)
{
    int fila, glcl, final;
    int cmd_test, cmd_group, file_test, file_group;

    if (tst->runstatus & OPTION_SKIP_IT)
        return 0;
    /* initialize */
    cmd_test = cmd_group = file_test = file_group = -1;
    final = 1;
    /* first/last options */
    if (opts.option & OPTION_FIRST_LAST)
	fila = ((tst->runstatus & OPTION_FIRST_LAST) ? 1 : 0);
    else
	fila = -1;
    /* gl call options */
    if (opts.option & OPTION_lt)
	glcl = ((tst->runstatus & OPTION_lt) ? 1 : 0);
    else if (opts.option & OPTION_la)
	glcl = ((tst->runstatus & OPTION_la) ? 0 : -1);
    else if (opts.option & OPTION_lT)
	glcl = ((tst->runstatus & OPTION_lT) ? 1 : 0);
    else if (opts.option & OPTION_lA)
	glcl = ((tst->runstatus & OPTION_lA) ? 0 : -1);
    else
	glcl = -1;
    /* test options */
    if( (opts.option & OPTION_t) || (opts.option & OPTION_a) ) {
        cmd_test = ((tst->runstatus & OPTION_a) ? 0 : cmd_test);
        cmd_test = ((tst->runstatus & OPTION_t) ? 1 : cmd_test);
    }
    if( (opts.option & OPTION_T) || (opts.option & OPTION_A) ) {
    	file_test = ((tst->runstatus & OPTION_A) ? 0 : file_test);
    	file_test = ((tst->runstatus & OPTION_T) ? 1 : file_test);
    }
    /* group options */
    if( (opts.option & OPTION_gt) || (opts.option & OPTION_ga) ) {
    	cmd_group = ((tst->runstatus & OPTION_ga) ? 0 : cmd_group);
    	cmd_group = ((tst->runstatus & OPTION_gt) ? 1 : cmd_group);
    }
    if( (opts.option & OPTION_gT) || (opts.option & OPTION_gA) ) {
    	file_group = ((tst->runstatus & OPTION_gA) ? 0 : file_group);
    	file_group = ((tst->runstatus & OPTION_gT) ? 1 : file_group);
    }
    /* set final according to the rules */
    if( cmd_test != -1 )
    	final = cmd_test;
    else if( cmd_group != -1 )
    	final = cmd_group;
    else if( file_test != -1 )
    	final = file_test;
    else if( file_group != -1 )
    	final = file_group;
    final *= glcl * fila;
    if (final) {
        char reason[16];
        extern int total_skip, total_tests;

        reason[0] = '\0';
        if (tst->archSpecs[0].skips & OG_DONOTRUN)
            /* spec say not to never run the test */
            strcpy(reason, "specfile option");
        else if (opts.kskip & tst->archSpecs[0].skips)
            strcpy(reason, "user option"); /* user avoid options */
        if (reason[0] != '\0') {
            if (fromMainLoop) {
                ogEnvLog(OG_LALWAYS,
                         "                           %sSKIPPED\033[0m: %-15s " \
			 "         %s\n",
                         opts.flags & FLAG_COLORPRINT ? "\033[30m\033[43m" : "",
                         reason, tst->name);
                total_tests++;
                total_skip++;
            }
            return (0);
        }
    }
    return (final ? 1 : 0);
}

/****************************************************************************
*  ogEnvFindGroup  -  find group name in group data base
****************************************************************************/
int 
ogEnvFindGroup(char *name)
{
    int j;

    for (j = 0; j < nGroups; j++)
	if (strcmp(groups[j].name, name) == 0)
	    return (j);
    return (-1);
}

/****************************************************************************
*  ogEnvFindTest()  -  find test name in spec data base
****************************************************************************/
int 
ogEnvFindTest(char *name)
{
    int j;

    for (j = 0; j < nTests; j++)
	if (strcmp(tests[j].name, name) == 0)
	    return (j);
    return (-1);
}

/*
 * initTests()
 * Reset number of tests run and order the tests in runarray if
 * not done in ogEnvParse.
 */
static void
initTests(void)
{
    int i, j;

    runindex = 0;
    if (numrun)		/* runarray and numrun already set in ogEnvParse */
        return;
    numrun = nTests;
    if (opts.flags & FLAG_RANDORDER) { /* random order */
        for (i = 0; i < nTests; i++)
            runarray[i] = -1;
        ogLibSetSeed(opts.seed);
        for (i = 0; i < nTests; i++) {
            j = ogLibIntRand(0, nTests);
            while (runarray[j % nTests] != -1)
                j++;
            runarray[j % nTests] = i;
        }
    } else                      /* linear order */
        for (i = 0; i < nTests; i++)
            runarray[i] = i;
}

/****************************************************************************
*  getTest()  - the next test from list of tests to run
****************************************************************************/
static Test *
getTest(void)
{
    if (runindex >= numrun)
        return (NULL);
    else
        return (&tests[runarray[runindex++]]);
}

/****************************************************************************
* ogEnvQuery() -
****************************************************************************/
int
ogEnvQuery(int opt)
{
   int dat;

   switch (opt) {
     case OG_XWSIZE:
       dat = windata.xsize;
       break;
     case OG_YWSIZE:
       dat = windata.ysize;
       break;
     case OG_XW0:
       dat = windata.wx0;
       break;
     case OG_XW1:
       dat = windata.wx1;
       break;
     case OG_YW0:
       dat = windata.wy0;
       break;
     case OG_YW1:
       dat = windata.wy1;
       break;
     case OG_HW:
       dat = opts.hwtype;
       break;
     case OG_FINISH:
       dat = 4 & opts.debugged; 
       break;
     default:
       dat = -1;
       break;
     }

   return(dat);
}

static void 
gethw(void)
{
   extern const char *gtst_renderer;
   char tmp[128], override_hw;

   /*
    * If the hardware name string was set via command line options, we go
    * through the motions of assigning the name, and at the end we just reset
    * it to the supplied name.
    */
   if (opts.hwname[0] != '\0') {
       strcpy(tmp, opts.hwname);
       override_hw = 1;
   } else
       override_hw = 0;

   if (!strncmp("Generic", gtst_renderer, 7)) {
       opts.hwtype = OG_GENERIC;
       strcpy(opts.hwname, "Generic");
   } else if (!strncmp("3Dfx", gtst_renderer, 4)) {
       opts.hwtype = OG_VG1;
       strcpy(opts.hwname, "VG1");
   } else
       ogEnvLog(OG_LINTERNALERROR, "Unknown RENDERER `%s'\n", gtst_renderer);

   if (override_hw)
       strcpy(opts.hwname, tmp);
}

void
setHWConditions(void)
{
    int i, hwIndex;

    switch (ogEnvQuery(OG_HW))  {
      case OG_GENERIC:
	hwIndex = ARCH_Generic;
      case OG_VG1:
	hwIndex = ARCH_VG1;
    }
    for (i = 0; i < nTests; i++) {
        tests[i].archSpecs[0].loop = tests[i].archSpecs[hwIndex].loop;
        tests[i].archSpecs[0].toler = tests[i].archSpecs[hwIndex].toler;
        tests[i].archSpecs[0].skips = tests[i].archSpecs[hwIndex].skips;
    }
}

/****************************************************************************
*  ogEnvCheckSum()  -
****************************************************************************/
void 
ogEnvCheckSum(Test *tst, unsigned int chksum, gimg *img)
{
    int db;
    char fname[64];

    db = ogEnvCurVisualInfo(GLX_DOUBLEBUFFER);

    if (db) {
	tst->returnedChksumBB = chksum;
    } else {
	tst->returnedChksumFB = chksum;
    }

    if (opts.flags & FLAG_FORCDUMP ||
        (db && (tst->returnedChksumBB != tst->expectedChksumBB)) ||
        (!db && (tst->returnedChksumFB != tst->expectedChksumFB))) {
	if (opts.flags & FLAG_NODUMP)
	    ogEnvLog(OG_LALWAYS,
                     "User Option: Image file for test %s not dumped\n",
                     tst->name);
	else {
	    if (db) {
		sprintf(fname, "B\\%s_%s.img", tst->name, opts.hwname);
	    } else {
		sprintf(fname, "F\\%s_%s.img", tst->name, opts.hwname);
	    }
	    ogEnvImageSave(fname, img);
	    ogEnvLog(OG_LALWAYS,
                     "Image file for test %s dumped into file %s\n",
                     tst->name, fname);
	}
    }
}

/****************************************************************************
*  memoryUse()  -
****************************************************************************/
static void 
memoryUse(void)
{
    int j;
    char *c, ch[256];
    FILE *fd;

    system("ps -le | grep ogtst > .tmp");
    if ((fd = fopen(".tmp", "r")) == NULL)
	return;
    for (j = 0; j < 3; j++) {
	for (c = ch, *c = getc(fd); *c != (char) EOF && *c != '\n';
	     *++c = getc(fd));

	if (strstr(ch, "sh -c") || strstr(ch, "grep"));
	else {
	    *c = '\0';
	    fprintf(logfile, "%s\n", ch);
	    break;
	}
    }
    fclose(fd);
}

/****************************************************************************
*  imagedVisual() - Test if visual is adequate for an imaged (checksumed) test.
****************************************************************************/
static int
imagedVisual(int vindex)
{
   int sum;
#if OVERLAYS_ALLOWED
   int layer;
#endif
   int rgba = ogEnvGLXVisualInfo(vindex, GLX_RGBA);

   if (rgba)
       sum = ogEnvGLXVisualInfo(vindex, GLX_RED_SIZE) +
           ogEnvGLXVisualInfo(vindex, GLX_GREEN_SIZE) +
           ogEnvGLXVisualInfo(vindex, GLX_BLUE_SIZE) +
           ogEnvGLXVisualInfo(vindex, GLX_ALPHA_SIZE);
   else
       sum = ogEnvGLXVisualInfo(vindex, GLX_BUFFER_SIZE);
#if OVERLAYS_ALLOWED
   layer = ogEnvGLXVisualInfo(vindex, GLX_LEVEL);
#endif
   switch(opts.hwtype) {
     case OG_GENERIC:
	 if (!rgba) {
	     return sum == 8;
	 } else {
	     switch (sum) {
	     case 8:
	     case 15:
	     case 16:
	     case 24:
	     case 32:
		 return 1;
	     default:
		 return 0;
	     }
	 }
     case OG_VG1:
         return sum == 16;
     default:
       ogEnvLog(OG_LINTERNALERROR,
                "unsupported hardware in imagedVisual(0x%08x)\n", opts.hwtype);
       break;
   }
   return 0;
}

