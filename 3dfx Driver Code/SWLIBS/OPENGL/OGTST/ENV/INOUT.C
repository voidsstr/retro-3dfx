/**************************************************************************
 *									  *
 * 		 Copyright (C) 1990, Silicon Graphics, Inc.		  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *									  *
 **************************************************************************/
#include <stdlib.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <stdio.h>
#include <stdarg.h>
#include <setjmp.h>
#ifndef WIN32
#include <sys/time.h>
#else
#include <time.h>
#endif
#include <string.h>
#include "ogtst.h"
#include "env.h"
#ifdef WIN32
#include "getopt.h"
#endif

#define OPTSTRING "1a:A:b:c:Cd:D:e:Ef:F:hHk:l:L:np:P:rRsS:t:T:uU:v:V:w:x:"

extern opts_t opts;
extern wist_t windata;
extern jmp_buf jmpenv;

extern FILE *logfile;

extern int numrun;		/* set in ogEnvParse for option T */
extern int *runarray;          /* set in ogEnvParse for option T */

static char parapar[4][256];

/* for statistics info */
static int total_pass, total_fail;
static int cur_test_pass = 0;
static int cur_test_skip_flag;
static int total_big_tests_skip = 0;
static int total_majority_pass = 0;
static int total_big_tests = 0;
static int total_big_tests_pass = 0;
int total_skip, total_tests;

static char          *getName(char **);
static int            inList(int, int*, int);

/****************************************************************************
*  getName()  -  return a string terminated by either ' ', ',', or '\0' in the
*                input string sp, and sp will be updated to current location.
****************************************************************************/
static char *
getName(char **sp)
{
    char *str;

    if (*sp == 0)
	return (NULL);
    str = *sp;
    while ((**sp != ' ') && (**sp != ',') && (**sp != '\0'))
	(*sp)++;
    if (**sp == '\0')
	*sp = 0;
    else {
	**sp = '\0';
	(*sp)++;
    }
    return (str);
}

/****************************************************************************
*  inList()  - Check if 'number' is in 'list' 
****************************************************************************/
static int
inList(int number, int *list, int lenght)
{
    int i, found = 0;

    for( i = 0; (!found) && (i < lenght); i++)
      if( number == list[i] )
        found = 1;
    return(found);
}

static int
hexStrToI(const char *str, const char *opt) {
    char *p;
    int val;

    val = strtol(str, &p, 16);
    if (p == str)
        ogEnvLog(OG_LUSERERROR, "Bad numerical argument `%s' to `-%s' option\n",
                 str, opt);
    return val;
}

/****************************************************************************
*  ogEnvParse()  -  parse arguments in command line
****************************************************************************/
void 
ogEnvParse(int largc, char *largv[])
{
    extern char *optarg;
    char c, *str, str1[256], *p;
    int d;
    int i, j, k, flag, visList[64];
    FILE *fp;
    time_t t;

    /* q option: output file names */
#ifdef WIN32
    DWORD nSize = 256;
    if (FALSE == GetComputerName(str1, &nSize)) {
	*str1 = '\0';
    }

#else
    gethostname(str1, 64);
#endif
    sprintf(parapar[0], "-w 0,639,0,511       > %s.ul 2>&1 &", str1);
    sprintf(parapar[1], "-w 0,639,512,1023    > %s.ll 2>&1 &", str1);
    sprintf(parapar[2], "-w 640,1279,0,511    > %s.ur 2>&1 &", str1);
    sprintf(parapar[3], "-w 640,1279,512,1023 > %s.lr 2>&1 &", str1);

    /* -q option : ship off 4 parallel gtsts (without -q option) */
    for (i = 0; i < largc; i++)
	if (strcmp(largv[i], "-q") == 0) {
            int cmdlen;
            
            fprintf(stderr, "ogtst: forking 4 ogtst's..");
            fflush(stderr);
            for (str1[0] = '\0', k = 0; k < largc; k++)
                if (k != i) {
                    strcat(str1, largv[k]);
                    strcat(str1, " ");
                }
            cmdlen = strlen(str1);
	    for (j = 0; j < 4; j++) {
		sprintf(&str1[cmdlen],"%s", parapar[j]);
                fprintf(stderr, "%d ", j + 1);
                fflush(stderr);
                system(str1);
	    }
	    fprintf(stderr, "done\n");
            exit(0);
	}
    optind = 1;
    while (((c = getopt(largc, largv, OPTSTRING)) != (char) -1))
	switch (c) {
	case '1':
	    /* run on one matching visual */
	    opts.flags |= FLAG_ONEVIS;
	    break;
	case 'a':
	    /* specify test cases to be avoided */
	    p = optarg;
	    while ((str = getName(&p)) != NULL) {
		if ((j = ogEnvFindTest(str)) >= 0) {
		    opts.option |= OPTION_a;
		    tests[j].runstatus |= OPTION_a;
		} else if ((j = ogEnvFindGroup(str)) >= 0) {
		    opts.option |= OPTION_ga;
		    for (i = 0; i < groups[j].nTests; i++)
			tests[groups[j].tests[i]].runstatus |= OPTION_ga;
		} else
		    ogEnvLog(OG_LUSERERROR, "%s: unknown test/group\n", str);
	    }
	    break;
	case 'A':
	    /* avoid test cases as specified in the file */
	    if ((fp = fopen(optarg, "r")) == NULL)
		ogEnvLog(OG_LUSERERROR, "Can not open avoid file %s\n", optarg);

	    while (fscanf(fp, "%s", str1) != EOF) {
		/* ignore comment and black lines */
		if (str1[0] == '#') {
		    do
			d = fgetc(fp);
		    while (d != '\n' && d != EOF);
		    continue;
		} else if (str1[0] == '\0')
		    continue;
		if (strlen(str1) > 74) 
		    ogEnvLog(OG_LINTERNALERROR, "%s: string too long\n", str1);
		if ((j = ogEnvFindTest(str1)) >= 0) {
		    opts.option |= OPTION_A;
		    tests[j].runstatus |= OPTION_A;
		} else if ((j = ogEnvFindGroup(str1)) >= 0) {
		    opts.option |= OPTION_gA;
		    for (i = 0; i < groups[j].nTests; i++)
			tests[groups[j].tests[i]].runstatus |= OPTION_gA;
		} else
		    ogEnvLog(OG_LUSERERROR, "%s: unknown test/group\n",
                             str1);
	    }
	    fclose(fp);
	    break;
	case 'c':
	    /* Output formatting */
	    p = optarg;
	    str = getName(&p);
	    for (i = 0; str[i] != 0; i++)
		switch (str[i]) {
		case 'a':
		    opts.flags |= FLAG_ALTERNATE;
		    break;
		case 'b':
		    opts.flags |= FLAG_NODUMP;
		    break;
		case 'c':
		    opts.flags |= FLAG_COLORPRINT;
		    break;
		case 'e':
		    opts.flags |= FLAG_2NAME;
		    break;
		case 'f':
		    opts.flags |= FLAG_FORCDUMP;
		    break;
		case 'm':
		    opts.flags |= FLAG_MEMUSE;
		    break;
		case 'n':
		    opts.flags |= FLAG_NUM_VIZ;
		    break;
                case 's':
                    opts.flags |= FLAG_STATISTICS;
                    break;
		case 't':
		    opts.flags |= FLAG_NOTIME;
		    break;
		default:
		    ogEnvLog(OG_LUSERERROR, "Invalid format (-c) option `%c'\n",
                             str[i]);
		    break;
		}
	    break;
	case 'C':
	    /* Enable context-switching */
	    opts.flags |= FLAG_CTXSW;
	    break;
        case 'd':
            p = optarg;
	    str = getName(&p);
	    for (i = 0; str[i] != '\0'; i++)
		switch (str[i]) {
                  case 'd':
                    break;
                  case 'p':
                    /* do pixmap rendering */
                    opts.flags |= FLAG_PIXMAP;
                    break;
#ifdef GLX_SGIX_pbuffer
                  case 'N':
                    if (str[i] == 'N')
                        opts.flags |= FLAG_NONPRESEVED_PB;
                    /* FALLTHRU */
                  case 'P':
                    /* Pbuffer testing */
                    if (!ogEnvHasPbuffers())
                        ogEnvLog(OG_LUSERERROR,
                                 "System does not support pbuffers (-d p/P option)\n");
                    opts.flags |= FLAG_PBUFFERS;
                    break;
                  case 'o':
                    opts.flags |= FLAG_PBUFFERS_ONLY;
                    break;
                  case 'm':
                    opts.flags |= FLAG_PB_WINDOWS_MAPPED;
                    break;
#endif
                  default:
		    ogEnvLog(OG_LUSERERROR, "Invalid drawable (-d) option `%c'\n",
                             str[i]);
		    break;
                }
            if ((opts.flags & (FLAG_PBUFFERS_ONLY | FLAG_PB_WINDOWS_MAPPED)) &&
                !(opts.flags & FLAG_PBUFFERS)) {
                ogEnvLog(OG_LALWAYS,
                         "WARNING: `-d' option(s) `o/m' specified, but pbuffer \
testing not requested.\n         Ignored.\n");
            }
            break;
	case 'D':
	    /* debug mode */
	    opts.debugged = hexStrToI(optarg, "D");
	    break;
	case 'e':
	    /* set maximum number of errors before a test gets aborted */
	    opts.maxerr = atoi(optarg);
	    break;
	case 'E':
	    /* Hold the window if there is any error */
	    opts.flags |= FLAG_ERRHOLD;
	    break;
	case 'f':
	    /* specify first,last test of specified (elsewhere) series to run */
	    flag = j = 0;
	    k = nTests - 1;

	    p = optarg;
	    while ((str = getName(&p)) != NULL) {
		if (flag == 0)
		    j = ogEnvFindTest(str);
		else if (flag == 1)
		    k = ogEnvFindTest(str);
		else {
		    ogEnvLog(OG_LUSERERROR, "Too many options in -f\n");
		    break;
		}
		flag++;
	    }

	    if (j < 0 || k < 0)
		ogEnvLog(OG_LUSERERROR, "Unknown test module(s) in -f option\n");
	    else {
		opts.option |= OPTION_FIRST_LAST;
		for (i = j; i <= k; i++)
		    tests[i].runstatus |= OPTION_FIRST_LAST;
	    }
	    break;
	case 'F':
	    if ((logfile = fopen(optarg, "w")) == NULL) {
		perror(optarg);
		exit(1);
	    }
	    break;
	case 'h':
	    /* HEEEEEEEEEEELLPPPPPP !!!!!!!! */
	    ogEnvPrintHelp();
	    exit(0);
	    break;
	case 'H':
	    /* hold window after every test */
	    opts.flags |= FLAG_HOLD;
	    break;
	case 'k':
	    /* skip conditions */
	    p = optarg;
	    str = getName(&p);
	    for (i = 0; str[i] != 0; i++)
		switch (str[i]) {
		case 'c':
		    opts.kskip |= OG_CONTEXTBUG;
		    break;
		case 'g':
		    opts.kskip |= OG_GLBUG;
		    break;
		case 'h':
		    for (j = 0; j < nTests; j++)
                        if (tests[j].type == OG_IMAGED)
                            tests[j].runstatus |= OPTION_SKIP_IT;
		    break;
		case 'm':
		    for (j = 0; j < nTests; j++)
                        if (tests[j].type != OG_IMAGED)
                            tests[j].runstatus |= OPTION_SKIP_IT;
		    break;
		case 's':
		    opts.kskip |= OG_CRASHBUG;
		    break;
		case 't':
		    opts.kskip |= OG_TSTBUG;
		    break;
		default:
		    ogEnvLog(OG_LUSERERROR, "Invalid skip condition\n");
		    break;
		}
	    break;
	case 'l':
	    /* set level for using ogEnvLog() */
	    opts.plvl = atoi(optarg);
	    if (opts.plvl == OG_LNEVER);
	    else if (opts.plvl < 1) {
		ogEnvLog(OG_LUSERERROR, "%d is not a valid log level, reset to 1\n", opts.plvl);
		opts.plvl = 1;
	    } else if (opts.plvl > 32) {
		ogEnvLog(OG_LUSERERROR, "%d is not a valid log level, reset to 32\n", opts.plvl);
		opts.plvl = 32;
	    }
	    break;
	case 'L':
	    /* set mask for using ogEnvLog() */
	    opts.pmsk = hexStrToI(optarg, "L");
	    break;
        case 'n':
            /* Turn the max color error down */
            opts.flags |= FLAG_COLOR;
            break;
	case 'p':
	    /* specify number of passes to run */
	    opts.passes = atof(optarg);
	    if (opts.passes < 0.0) {
		ogEnvLog(OG_LUSERERROR, "Pass multiplier cannot be negative! Reset to 1.0\n");
		opts.passes = 1.0;
	    }
	    break;
	case 'P':
	    /* specify seconds to run the test */
	    opts.runsec = atoi(optarg);
	    break;
	case 'r':
	    /* run test in random order */
	    opts.flags |= FLAG_RANDORDER;
	    break;
	case 'R':
	    /* Disable pixel readback and checksum comparison */
	    opts.flags |= FLAG_NORDBACK;
	    break;
	case 's':
	    /* using randomly generated seed for random number generator */
	    (void) time(&t);
	    opts.seed = (int) t;
	    break;
	case 'S':
	    /* set seed for random number generator */
	    opts.seed = atoi(optarg);
	    break;
	case 't':
	    /* specify test cases to be run */
	    p = optarg;

	    while ((str = getName(&p)) != NULL) {
		if ((j = ogEnvFindTest(str)) >= 0) {
		    opts.option |= OPTION_t;
		    tests[j].runstatus |= OPTION_t;
                    /* run tests in the same order that they are listed */
                    if(!inList(j, runarray, numrun))
                        runarray[numrun++] = j;
		} else if ((j = ogEnvFindGroup(str)) >= 0) {
		    opts.option |= OPTION_gt;
		    for (i = 0; i < groups[j].nTests; i++) {
			tests[groups[j].tests[i]].runstatus |= OPTION_gt;
                        /*
                         * run tests in the same order that they are
                         * listed in the group
                         */
                        if(!inList(groups[j].tests[i], runarray, numrun))
                            runarray[numrun++] = groups[j].tests[i];
                    }
		} else
		    ogEnvLog(OG_LUSERERROR, "%s: unknown test/group\n", str);
	    }
	    break;
	case 'T':		
	    /* run test cases as specified in the file */
	    if ((fp = fopen(optarg, "r")) == NULL) 
		ogEnvLog(OG_LUSERERROR, "Can not open test file %s\n",
                         optarg);

	    while (fscanf(fp, "%s", str1) != EOF) {
		/* ignore comment and black lines */
		if (str1[0] == '#') {
		    do
			d = fgetc(fp);
		    while (d != '\n' && d != EOF);
		    continue;
		} else if (str1[0] == '\0')
		    continue;

		if (strlen(str1) > 74)
		    ogEnvLog(OG_LINTERNALERROR, "%s: string too long\n", str1);

		if ((j = ogEnvFindTest(str1)) >= 0) {
		    opts.option |= OPTION_T;
		    tests[j].runstatus |= OPTION_T;
                    /* run tests in the same order that they are listed in the file */
                    if( !inList(j, runarray, numrun) )
                        runarray[numrun++] = j;
		} else if ((j = ogEnvFindGroup(str1)) >= 0) {
		    opts.option |= OPTION_gT;
		    for (i = 0; i < groups[j].nTests; i++) {
			tests[groups[j].tests[i]].runstatus |= OPTION_gT;
                        /* run tests in the same order that they are
                         * listed in the group
                         */
                        if(!inList(groups[j].tests[i], runarray, numrun))
                            runarray[numrun++] = groups[j].tests[i];
		    }
		} else
		    ogEnvLog(OG_LUSERERROR, "%s: unknown test/group/glcall\n",
                             str1);
	    }
	    fclose(fp);
	    break;
	case 'u':
	    opts.flags |= FLAG_UPDATED;
	    break;
	case 'v':
	    opts.vmask = hexStrToI(optarg, "v");
	    break;
#if defined(GLX_SGIX_fbconfig)
        case 'U':
#endif
        case 'V':
            if (c == 'U' && !ogEnvHasFBConfigs())
		ogEnvLog(OG_LUSERERROR, "Can not use the '-U' option "
			 "FBConfigs are not supported\n");
            /* Can't use ogLibMalloc yet */
            if (strlen(optarg) > 256)
                ogEnvLog(OG_LINTERNALERROR,
                         "visual list for -%c option too long (> 256 chars)\n", c);
            strcpy(str1, optarg);
            p = strtok(str1, ",");
            i = 0;
            do 
                visList[i++] = hexStrToI(optarg, "V");
            while ((p = strtok(NULL, ",")) != NULL && i < 64);
            if (p != NULL)
		ogEnvLog(OG_LUSERERROR, "More than 64 visuals in -V option\n");
            ogEnvSelectVisuals(visList, i, c == 'V');
            break;
	case 'w':
	    /* specify dimension of default window to run test */
	    p = optarg;

	    if ((str = getName(&p)) == NULL)
		ogEnvLog(OG_LUSERERROR, "-w with wrong parameters\n");
	    windata.wx0 = atoi(str);
	    if ((str = getName(&p)) == NULL)
		ogEnvLog(OG_LUSERERROR, "-w with wrong parameters\n");
	    windata.wx1 = atoi(str);
	    if ((str = getName(&p)) == NULL)
		ogEnvLog(OG_LUSERERROR, "-w with wrong parameters\n");
	    windata.wy0 = atoi(str);
	    if ((str = getName(&p)) == NULL)
		ogEnvLog(OG_LUSERERROR, "-w with wrong parameters\n");
	    windata.wy1 = atoi(str);

	    windata.xsize = windata.wx1 - windata.wx0 + 1;
	    windata.ysize = windata.wy1 - windata.wy0 + 1;
	    break;
	case 'x':
	    strncpy(opts.hwname, optarg, sizeof opts.hwname);
	    break;
	default:
	    ogEnvLog(OG_LUSERERROR, "Invalid user option\n");
	    break;
	}
}

/****************************************************************************
*  ogEnvLog()  -  print out function used in driver
****************************************************************************/
void 
ogEnvLog(GLint pl, char *fmt, ...) {
    va_list args;

    va_start(args, fmt);

    if (pl > 32 || pl < -5) {
	fprintf(logfile, "ERROR: log level of excessive size %d\n", pl);
	fflush(logfile);
	exit(1);
    } else if (pl == OG_LUSERERROR) {
	fprintf(logfile, "USER ERROR:");
	vfprintf(logfile, fmt, args);
	fflush(logfile);
	exit(1);
    } else if (pl == OG_LINTERNALERROR) {
	fprintf(logfile, "INTERNAL ERROR:");
	vfprintf(logfile, fmt, args);
	fflush(logfile);
	exit(1);
    } else if (pl == OG_LSKIP) {
	fprintf(logfile, "SKIP: ");
	vfprintf(logfile, fmt, args);
	fflush(logfile);
	opts.flags |= FLAG_GSKIP;
	longjmp(jmpenv, 1);
    } else if (pl == OG_LALWAYS) {
	vfprintf(logfile, fmt, args);
	fflush(logfile);
    } else if (opts.plvl == OG_LNEVER) {
	if (pl == OG_LFAIL)
	    opts.errcount++;
    } else if (pl == OG_LFAIL) {
	fprintf(logfile, "ERROR: ");
	vfprintf(logfile, fmt, args);
	fflush(logfile);

	if (opts.debugged & 1) {
	    fprintf(logfile, "Hit <return> to continue");
	    fflush(logfile);
	    (void) getchar();
	}
	if (++opts.errcount > opts.maxerr) {
	    fprintf(logfile, "Test cancelled, too many errors %d.\n",\
                    opts.errcount);
	    fflush(logfile);
	    ogLibFreeAll();
	    longjmp(jmpenv, -1);
	}
    } else if (ogEnvWillOutput(pl)) {
        fprintf(logfile, "INFO.%d: ", pl);
        vfprintf(logfile, fmt, args);
        fflush(logfile);
    }
    va_end(args);

    ogEnvCtxswHook();
}

/*
 * Returns whether a given info level will cause output.
 * For tests for which preparing the message might be costly and wastefull
 * if the output is not going to be printed.
 */
int
ogEnvWillOutput(int printLevel)
{
    switch (printLevel) {
      case OG_LALWAYS:
        return 1;
      case OG_LNEVER:
        return 0;
      default:
        if (opts.pmsk == 0) {
            return opts.plvl >= printLevel;
        } else
            return (opts.pmsk & (1 << (printLevel - 1))) &&
                (opts.plvl == 0 || opts.plvl >= printLevel);
    }
}

/****************************************************************************
*  ogEnvPartialPass()  -  mark current test as a partial pass
****************************************************************************/
void ogEnvPartialPass(void) {
    opts.partial = 1;
}

/***************************************************************************
*   Majority Pass -- Majority Pass => Pass
**************************************************************************/
void ogEnvMajorityPass(int num_times)
{
  total_big_tests++;
  if (cur_test_skip_flag) total_big_tests_skip++;
  if (cur_test_pass > TEST_PASSING_GRADE * num_times) {
  total_majority_pass += num_times;
  total_big_tests_pass++;
  }
  ogEnvLog(OG_LALWAYS, " passed %d of %d visuals\n", 
                 cur_test_pass, num_times);
  cur_test_pass = 0;
}
  
/****************************************************************************
*  ogEnvGend()  -  print out end message of test case
****************************************************************************/
void
ogEnvGend(Test *test, int vindex)
{
    char tmst[20], *visualString;
    char passedString[28], failedString[27], skippedString[28];

    total_tests++;
    if (opts.flags & FLAG_NUM_VIZ) {
      cur_test_skip_flag = 0;
    }
    if (opts.flags & FLAG_NOTIME)
	tmst[0] = '\0';
    else {
	time_t tim;
	struct tm *tm;

	tim = time(0);
	tm = localtime(&tim);
	sprintf(tmst, "%02d:%02d:%02d %02d-%02d-%02d",
		tm->tm_hour, tm->tm_min, tm->tm_sec, tm->tm_mon + 1, 
		tm->tm_mday, tm->tm_year);
    }
    visualString = ogEnvVisualString(vindex);
    if (opts.flags & FLAG_COLORPRINT) {
        (void) sprintf(passedString, "\033[30m\033[42mPASSED%s\033[0m",
                       opts.partial ? "*" : " ");
        (void) strcpy(failedString, "\033[37m\033[41mFAILED\033[0m");
        (void) strcpy(skippedString, "\033[30m\033[43mSKIPPED\033[0m");
    } else {
        (void) sprintf(passedString, "PASSED%s",
                       opts.partial ? "*" : " ");
        (void) strcpy(failedString, "FAILED");
        (void) strcpy(skippedString, "SKIPPED");
    }
    if (!(opts.flags & FLAG_ALTERNATE)) {
      if (opts.flags & FLAG_GSKIP) {
        total_skip++;
      if (opts.flags & FLAG_NUM_VIZ) {
        cur_test_skip_flag = 1;
      }
        ogEnvLog(OG_LALWAYS, "%18s %12s %s                    %s\n", 
                 tmst, visualString, skippedString, test->name);
      } else if (opts.errcount > 0) {
        total_fail++;
        ogEnvLog(OG_LALWAYS, "%18s %11s %s   error: %8d        %s\n",
                 tmst, visualString, failedString, opts.errcount,
                 test->name);
      } else if (test->type == OG_NORMAL) {
      if (opts.flags & FLAG_NUM_VIZ) {
	cur_test_pass++;
      }
        total_pass++;
        ogEnvLog(OG_LALWAYS, "%18s %11s %s                   %s\n", 
                 tmst, visualString, passedString, test->name);
      } else if (test->type == OG_IMAGED) {
	int db = ogEnvGLXVisualInfo(vindex, GLX_DOUBLEBUFFER);
        if (opts.flags & FLAG_NORDBACK ||
            (db && (test->expectedChksumBB == test->returnedChksumBB)) ||
	    (!db && (test->expectedChksumFB == test->returnedChksumFB))) {
	    if (opts.flags & FLAG_NUM_VIZ) {
		cur_test_pass++;
	    }
            total_pass++;
	    ogEnvLog(OG_LALWAYS, "%18s %11s %s   %08x: %08x (%s)  %s\n", 
		     tmst, visualString, passedString,
		     (db) ? test->returnedChksumBB : test->returnedChksumFB,
		     (db) ? test->expectedChksumBB : test->expectedChksumFB,
		     (db) ? "B" : "F", test->name);
        } else {
            total_fail++;
            ogEnvLog(OG_LALWAYS, "%18s %11s %s   %08x : %08x (%s)  %s\n",
                     tmst, visualString, failedString,
		     (db) ? test->returnedChksumBB : test->returnedChksumFB,
		     (db) ? test->expectedChksumBB : test->expectedChksumFB,
		     (db) ? "B" : "F", test->name);
	    opts.errcount = 1; /* for hold on error option */
        }
      } else
        ogEnvLog(OG_LINTERNALERROR, "fucked up in ogEnvGend()\n");
    } else {
      if (opts.flags & FLAG_GSKIP) {
        total_skip++;
        if (opts.flags & FLAG_NUM_VIZ) {
          cur_test_skip_flag = 1;
	}
        ogEnvLog(OG_LALWAYS, "%-25s%8s %s                      %s\n",
                 test->name, visualString, skippedString, tmst);
      } else if (opts.errcount > 0) {
        total_fail++;
        ogEnvLog(OG_LALWAYS, "%-25s%8s %s  error: %8d      %s\n",
                 test->name, visualString, failedString,
                 opts.errcount, tmst);
      } else if (test->type == OG_NORMAL) {
        if (opts.flags & FLAG_NUM_VIZ) {
	  cur_test_pass++;
	}
        total_pass++;
        ogEnvLog(OG_LALWAYS, "%-25s%8s %s                      %s\n",
                test->name, visualString, passedString, tmst);
      } else if (test->type == OG_IMAGED) {
	int db = ogEnvGLXVisualInfo(vindex, GLX_DOUBLEBUFFER);
        if ((opts.flags & FLAG_NORDBACK) ||
            (db && (test->expectedChksumBB == test->returnedChksumBB)) ||
	    (!db && (test->expectedChksumFB == test->returnedChksumFB))) {
          if (opts.flags & FLAG_NUM_VIZ) {
	    cur_test_pass++;
	  }
          total_pass++;
          ogEnvLog(OG_LALWAYS, "%-25s%8s %s  %08x: %08x (%s)     %s\n",
                   test->name, visualString, passedString,
		   (db) ? test->returnedChksumBB : test->returnedChksumFB,
		   (db) ? test->expectedChksumBB : test->expectedChksumFB,
		   (db) ? "B" : "F", tmst);
        } else {
          total_fail++;
          ogEnvLog(OG_LALWAYS, "%-25s%8s %s  %08x : %08x (%s)     %s\n",
                   test->name, visualString, failedString,
		   (db) ? test->returnedChksumBB : test->returnedChksumFB,
		   (db) ? test->expectedChksumBB : test->expectedChksumFB,
		   (db) ? "B" : "F", tmst);
          opts.errcount = 1; /* for hold on error option */
        }
      } else 
	  ogEnvLog(OG_LINTERNALERROR, "fucked up in ogEnvGend()\n");
    }
    fflush(logfile);
}

/*****************************************************************************
ogEnvStatistic()  -  print out statistic status
****************************************************************************/
void
ogEnvStatistics(void)
{
    ogEnvLog(OG_LALWAYS,
            "\n----------------------------------------\n");
    if (opts.flags & FLAG_NUM_VIZ) {
      ogEnvLog(OG_LALWAYS, "Passed a majority of visuals in %8d of %8d tests:  (%6.2f%s)\n",
            total_big_tests_pass, total_big_tests, 100.00*(float)total_big_tests_pass/(float)total_big_tests, "%");
      ogEnvLog(OG_LALWAYS, "Total Passed Tests based on Majority:  %8d (%6.2f%s)\n",
            total_majority_pass, 100.00*(float)total_majority_pass/(float)total_tests, "%");
      ogEnvLog(OG_LALWAYS, "%d Full Tests were Skipped (%6.2f%s)\n",
            total_big_tests_skip, 100.00*(float)total_big_tests_skip/
	     (float)total_tests, "%");
    ogEnvLog(OG_LALWAYS, "Total Passed Tests - Discounting Skips:  %8d (%6.2f%s)\n",
            total_pass, 100.00*(float)total_pass/(float)(total_tests - total_skip), "%");
    ogEnvLog(OG_LALWAYS, "Total Failed Tests - Discounting Skips:  %8d (%6.2f%s)\n",
            total_fail, 100.00*(float)total_fail/(float)(total_tests - total_skip), "%");
    }
    ogEnvLog(OG_LALWAYS, "Total Passed Tests:  %8d (%6.2f%s)\n",
            total_pass, 100.00*(float)total_pass/(float)total_tests, "%");
    ogEnvLog(OG_LALWAYS, "Total Failed Tests:  %8d (%6.2f%s)\n",
            total_fail, 100.00*(float)total_fail/(float)total_tests, "%");
    ogEnvLog(OG_LALWAYS, "Total Skipped Tests: %8d (%6.2f%s)\n",
            total_skip, 100.00*(float)total_skip/(float)total_tests, "%");
    ogEnvLog(OG_LALWAYS,
            "----------------------------------------\n");
    ogEnvLog(OG_LALWAYS, "Total Run Tests:     %8d (100.00%s)\n\n",
                total_tests, "%");
}

void
ogEnvInitStatistics(void)
{
    total_pass = total_fail = total_skip = total_tests = 0;
    cur_test_pass = total_majority_pass = 0;
    total_big_tests = total_big_tests_pass = 0;
    total_big_tests_skip = 0;
}

/****************************************************************************
*  ogEnvTime()  -  print out time stamp
****************************************************************************/
void
ogEnvTime(void)
{
    char tmst[20];
    
	time_t tim;
	struct tm *tm;

	tim = time(0);
	tm = localtime(&tim);
	sprintf(tmst, "%02d:%02d:%02d %02d-%02d-%02d",
		tm->tm_hour, tm->tm_min, tm->tm_sec, tm->tm_mon + 1, tm->tm_mday, tm->tm_year);
    
    ogEnvLog(OG_LALWAYS, "%18s ", tmst); 
}

static void
GetChecksumFileName(char *str, int front, int new)
{
    extern const char *gtst_renderer;

    if (front) {
	sprintf(str, "F\\");
    } else {
	sprintf(str, "B\\");
    }
    switch (ogEnvCurVisualInfo(GLX_BUFFER_SIZE)) {
	case 8:
	    strcat(str, "chksum_RGB332");
	    break;
	case 16:
	    if (ogEnvCurVisualInfo(GLX_GREEN_SIZE) == 6) {
		strcat(str, "chksum_RGB565");
	    } else {
		strcat(str, "chksum_RGB5");
	    }
	    break;
	case 24:
	    strcat(str, "chksum_RGB8");
	    break;
	case 32:
	    strcat(str, "chksum_XRGB8");
	    break;
    }
    if (strstr(gtst_renderer, "MMX")) {
	strcat(str, "_MMX");
    }
    strcat(str, ".");
    if (new) {
	strcat(str, "new");
    } else {
	strcat(str, opts.hwname);
    }
}

static void
LoadChecksumData(FILE *fd, int front)
{
    int i;
    char str[80];

    while (fscanf(fd, "%s", str) > 0 && str[0] != '!') {
	i = ogEnvFindTest(str);
	if (i < 0) {
	    ogEnvLog(OG_LINTERNALERROR,
                     "Unknown test %s in checksum data file\n", str);
	}
	if (tests[i].type != OG_IMAGED) {
	    ogEnvLog(OG_LINTERNALERROR, "test %s is not an image test\n", str);
	}
	if (front) {
	    fscanf(fd, "%x", &tests[i].expectedChksumFB);
	} else {
	    fscanf(fd, "%x", &tests[i].expectedChksumBB);
	}
    }
}

void 
ogEnvReadChecksumData(void)
{
    int i;
    FILE *fd;
    char name[32];

    GetChecksumFileName(name, 1, 0);
    if (!(fd = fopen(name, "r"))) {
	ogEnvLog(OG_LALWAYS, "WARNING: NO CHECKSUM DATA AVAILABLE FOR \"%s\", "
		 "CHECKSUM TESTS WILL NOT BE RUN!!!\n", name);
	for (i = 0; i < nTests; i++) {
	    if (tests[i].type == OG_IMAGED) {
		tests[i].runstatus |= OPTION_SKIP_IT;
	    }
        }
	return;
    }
    LoadChecksumData(fd, 1);
    fclose(fd);

    GetChecksumFileName(name, 0, 0);
    if (!(fd = fopen(name, "r"))) {
	ogEnvLog(OG_LALWAYS, "WARNING: NO CHECKSUM DATA AVAILABLE FOR \"%s\", "
		 "CHECKSUM TESTS WILL NOT BE RUN!!!\n", name);
	for (i = 0; i < nTests; i++) {
	    if (tests[i].type == OG_IMAGED) {
		tests[i].runstatus |= OPTION_SKIP_IT;
	    }
        }
	return;
    }
    LoadChecksumData(fd, 0);
    fclose(fd);

    for (i = 0; i < nTests; i++) {
        if (tests[i].type == OG_IMAGED &&
	    (tests[i].expectedChksumFB == 0xdeadbeef ||
	    tests[i].expectedChksumBB == 0xdeadbeef)) {
            ogEnvLog(OG_LALWAYS,
                     "WARNING: test %-24s has no valid checksum data!\n",
                     tests[i].name);
	}
    }
}

/****************************************************************************
*  ogEnvWriteChecksumData()  - 
****************************************************************************/
void 
ogEnvWriteChecksumData(void)
{
    FILE *fd;
    int i;
    char name[32];
    extern int skipCheck(Test *, int);

    GetChecksumFileName(name, 1, 1);
    if (!(fd = fopen(name, "w"))) {
	ogEnvLog(OG_LALWAYS,
                 "Error: Cannot open %s file for writing!\n", name);
        return;
    }
    for (i = 0; i < nTests; i++) {
	if (tests[i].type == OG_IMAGED && skipCheck(&tests[i], 0)) {
	    fprintf(fd, "%-18s 0x%08x\n",tests[i].name,
                    tests[i].returnedChksumFB);
	}
    }
    fprintf(fd, "!");
    fclose(fd);
    ogEnvLog(OG_LALWAYS, "Checksum data written into file %s\n", name);

    GetChecksumFileName(name, 0, 1);
    if (!(fd = fopen(name, "w"))) {
	ogEnvLog(OG_LALWAYS,
                 "Error: Cannot open %s file for writing!\n", name);
        return;
    }
    for (i = 0; i < nTests; i++) {
	if (tests[i].type == OG_IMAGED && skipCheck(&tests[i], 0)) {
	    fprintf(fd, "%-18s 0x%08x\n",tests[i].name,
                    tests[i].returnedChksumBB);
	}
    }
    fprintf(fd, "!");
    fclose(fd);
    ogEnvLog(OG_LALWAYS, "Checksum data written into file %s\n", name);
}

/***************************************************************************
* Yes, yes, it's a gross hack. But consider the alternatives:
*
* + pass a pointer: akward, and callee must know size of char array to pass
* + malloc: callee must free string, slow
* + print it directly: doesn't work with ogEnvLog calls that redirect output
*
* ogEnvColorString() - convert a color in packed int format into a string
*                      showing each color component as a 3 digit integer
*                      (0-255). String is returned a a pointer to 1 element
*                      of a pool of static memory. Can't use more than
*                      STRING_INDEXS strings in the same call.
****************************************************************************/


#define STRING_INDICES 10

const char *
ogEnvColorString(GLuint color)
{
  static char strings[STRING_INDICES][sizeof("(rrr,ggg,bbb,aaa)")];
  static unsigned int index = STRING_INDICES - 1;

  index = (index + 1) % STRING_INDICES;
  if (ogEnvCurVisualInfo(GLX_RGBA)) {
      GLubyte r,g,b,a;
      ogLibParseColor(color, &r, &g, &b, &a);
      sprintf(strings[index], "(%2x,%2x,%2x,%2x)",r,g,b,a);
  }
  else
      sprintf(strings[index], "%4x", color);
  return strings[index];
}

/*
 * Return the name of a GLenum data type.
 */
static const char *typeNames[] = {
    "GL_BITMAP",
    /* Separate enum range */
    "GL_BYTE",
    "GL_UNSIGNED_BYTE",
    "GL_SHORT",
    "GL_UNSIGNED_SHORT",
    "GL_INT",
    "GL_UNSIGNED_INT",
    "GL_FLOAT",
    "GL_2_BYTES",
    "GL_3_BYTES",
    "GL_4_BYTES",
#ifdef GL_DOUBLE_EXT
    "GL_DOUBLE_EXT",
#endif
    /* Separate enum range */
#ifdef GL_EXT_packed_pixels
    "GL_UNSIGNED_BYTE_3_3_2_EXT",
    "GL_UNSIGNED_SHORT_4_4_4_4_EXT",
    "GL_UNSIGNED_SHORT_5_5_5_1_EXT",
    "GL_UNSIGNED_INT_8_8_8_8_EXT",
    "GL_UNSIGNED_INT_10_10_10_2_EXT",
#endif
};

const char *
ogEnvDataTypeName(GLenum type)
{
    static char errString[80];

    if (type == GL_BITMAP)
        return typeNames[0];
    if (GL_BYTE <= type && type <=
#ifdef GL_DOUBLE_EXT
        GL_DOUBLE_EXT
#else
        GL_4_BYTES
#endif
        )
        return typeNames[type - GL_BYTE +
                         GL_BITMAP - GL_BITMAP + 1];
#ifdef GL_EXT_packed_pixels
    if (GL_UNSIGNED_BYTE_3_3_2_EXT <= type &&
        type <= GL_UNSIGNED_INT_10_10_10_2_EXT)
        return typeNames[type - GL_UNSIGNED_BYTE_3_3_2_EXT +
#ifdef GL_DOUBLE_EXT
                         1 +
#endif
                         GL_4_BYTES - GL_BYTE + 1 +
                         GL_BITMAP - GL_BITMAP + 1];
                          
#endif
    sprintf(errString, "Illegal Type 0x%x", type);
    return errString;
}

static const char *pixelFormats[] = {
    "GL_COLOR_INDEX",
    "GL_STENCIL_INDEX",
    "GL_DEPTH_COMPONENT",
    "GL_RED",
    "GL_GREEN",
    "GL_BLUE",
    "GL_ALPHA",
    "GL_RGB",
    "GL_RGBA",
    "GL_LUMINANCE",
    "GL_LUMINANCE_ALPHA",
    /* Separate enum range */
#ifdef GL_EXT_abgr
    "GL_ABGR_EXT",
#endif
    /* Separate enum range */
#ifdef GL_EXT_texture
    "GL_ALPHA4_EXT",
    "GL_ALPHA8_EXT",
    "GL_ALPHA12_EXT",
    "GL_ALPHA16_EXT",
    "GL_LUMINANCE4_EXT",
    "GL_LUMINANCE8_EXT",
    "GL_LUMINANCE12_EXT",
    "GL_LUMINANCE16_EXT",
    "GL_LUMINANCE4_ALPHA4_EXT",
    "GL_LUMINANCE6_ALPHA2_EXT",
    "GL_LUMINANCE8_ALPHA8_EXT",
    "GL_LUMINANCE12_ALPHA4_EXT",
    "GL_LUMINANCE12_ALPHA12_EXT",
    "GL_LUMINANCE16_ALPHA16_EXT",
    "GL_INTENSITY_EXT",
    "GL_INTENSITY4_EXT",
    "GL_INTENSITY8_EXT",
    "GL_INTENSITY12_EXT",
    "GL_INTENSITY16_EXT",
    "GL_RGB2_EXT",
    "GL_RGB4_EXT",
    "GL_RGB5_EXT",
    "GL_RGB8_EXT",
    "GL_RGB10_EXT",
    "GL_RGB12_EXT",
    "GL_RGB16_EXT",
    "GL_RGBA2_EXT",
    "GL_RGBA4_EXT",
    "GL_RGB5_A1_EXT",
    "GL_RGBA8_EXT",
    "GL_RGB10_A2_EXT",
    "GL_RGBA12_EXT",
    "GL_RGBA16_EXT",
#endif
};
  
/*
 * Return the name of a GLenum pixel format.
 */
const char *
ogEnvPixelFormatName(GLenum format)
{
    static char errString[80];

    if (GL_COLOR_INDEX <= format && format <= GL_LUMINANCE_ALPHA)
        return pixelFormats[format - GL_COLOR_INDEX];
#ifdef GL_EXT_abgr
    if (format == GL_ABGR_EXT)
        return pixelFormats[format - GL_ABGR_EXT + /* For consistency */
                            GL_LUMINANCE_ALPHA - GL_COLOR_INDEX + 1];
#endif /* def GL_EXT_abgr  */
#ifdef GL_EXT_texture
    if (GL_ALPHA4_EXT <= format && format <= GL_RGBA16_EXT)
        return pixelFormats[format - GL_ALPHA4_EXT +
                            GL_ABGR_EXT - GL_ABGR_EXT + 1 +
                            GL_LUMINANCE_ALPHA - GL_COLOR_INDEX + 1];
#endif
    /* It's an error */
    sprintf(errString, "Illegal Format 0x%x", format);
    return errString;
}

