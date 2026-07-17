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

#include <stdio.h>
#include <string.h>
#include "ogtst.h"
#include "env.h"

extern Test tests[];
extern Group groups[];
extern opts_t opts;

extern FILE *logfile;

static void           usage(void);
static void           helpMenu(void);
static void           listGroups(int,char *);

static void 
usage(void)
{
    printf("Usage : ogtst [options]\n  options are:\n");
    printf("    -a test0,test1,...       avoid tests/groups in the list\n");
    printf("    -A file                  avoid tests or groups listed in file\n");
    printf("    -c [abcefmnst]            output formatting\n");
    printf("    -C                       enable context-switch test\n");
#if defined(GLX_SGIX_pbuffer)
    printf("    -d [dpPN[om]]             select drawble type(s) run tests on\n");
#else
    printf("    -d [dp]                  select drawble type(s) run tests on\n");
#endif
    printf("    -D mask                  set debug options\n");
    printf("    -e number                set maximum error number\n");
    printf("    -E                       hold window on error\n");
    printf("    -f ftest[,ltest]         first[,last] test\n");
    printf("    -F logfile               output logfile name\n");
    printf("    -h                       get help\n");
    printf("    -H                       hold window after every test\n");
    printf("    -k [cghmst]              skip conditions\n");
    printf("    -l level                 set ogEnvLog trace level\n");
    printf("    -L mask                  set ogEnvLog trace hex mask\n");
    printf("    -n                       set max color error down\n");
    printf("    -p passScaleFactor       specify a scale factor for number of passes\n");
    printf("    -P sec                   testing for sec seconds\n");
    printf("    -q                       run a 4 copies of ogtst\n");
    printf("    -r                       run tests in random order\n");
    printf("    -R                       disable pixel readback and checksum comparison\n");
    printf("    -s                       use random seed\n");
    printf("    -S seed                  set random seed\n");
    printf("    -t test0,test1,...       run tests/groups in the order listed\n");
    printf("    -T file                  run tests/groups in the order listed in file\n");
    printf("    -u                       generate checksum file\n");
    printf("    -U configID,configID,... run test only for specified fbconfig ids (hex)\n");
    printf("    -v mask                  visual restriction hex mask\n");
    printf("    -V visualID,visualID,... run test only for specified visual ids (hex)\n");
    printf("    -w xmin,xmax,ymin,ymax   geometry of test window (in X coordinates)\n");
    printf("    -x name                  override hardware name (for chksum file)\n");
    printf("    -1                       run on one matching visual for each test\n");
}

/****************************************************************************
*  ogEnvPrintHelp()  - 
****************************************************************************/
void 
ogEnvPrintHelp(void)
{
    char string[32];

    helpMenu();
    printf("?-> ");
    fflush(stdout);
    while (fgets(string, 31, stdin) != NULL) {
	switch (string[0]) {
          case '\n':
            break;
          case '1':
	    usage();
	    break;
          case '2':
	    listGroups(0,"");
	    break;
          case '3':
	    listGroups(1,"");
	    break;
          case '4':
	    printf("Please enter group's name - ") ;
            fflush(stdout);
	    gets(string) ;
	    listGroups(2,string);
	    break;
          case '9':
          case 'q':
          case 'Q':
	    return;
          case '?':
            helpMenu();
            break;
          default:
            printf("No such option!\n");
            helpMenu();
            break;
          case '-':
            switch (string[1]) {
              case '1':
                printf("Option:1 - Run each test on a single visual only\n");
                break;
              case 'a':
                printf("Option:a - Specify tests/groups to avoid. There are \
two categories of input:\n");
                printf("           individual tests and groups of tests;\n");
                printf("                tests: avoid running the specified tests(s). To review the\n");
                printf("                       tests available use the ogtst -h (3) option.\n");
                printf("               groups: avoid running the specified test group(s). To review\n");
                printf("                       the tests in each group, use the ogtst -h (2,3) option.\n");
                printf("                       Group names should always start with a capital.\n");
                printf("           Parameters are separated by commas.  Mixing of groups and names\n");
                printf("           is allowed.\n");
                printf("           e.g., -a pixel      (avoid only the pixel test)\n");
                printf("           e.g., -a Xform      (avoid the Xform group of tests)\n");
                printf("           e.g., -a Xform,pixel(avoid both of the above)\n");
                break;
              case 'A':
                printf("Option:A - Specify a file that contains a list of \
tests/groups to avoid.  The\n");
                printf("           file may contain group/test names listed \
one per line, comment\n");
                printf("           lines beginning with a '#', \
and blank lines.\n");
                printf("           See the -a option for more details.\n");
                printf("           e.g., -A badTests\n");
                break;
              case 'c':
                printf("Option:c - Options for output formatting. Options are:\n");
                printf("                a - Print test name first -- another format\n");
                printf("                b - Do not dump failed images\n");
		printf("                c - Print results in color\n");
                printf("                e - Print test name before running test\n");
                printf("                f - Force an image dump\n");
                printf("                m - Print memory usage between tests\n");
                printf("                n - Print number of visuals passed per test\n");
                printf("                s - Print statistics of results\n");
                printf("                t - Do not print the time string\n");
                printf("           e.g., -c t\n");
                break;
              case 'C':
                printf("Option:C - Enable context switching tests.\n");
                printf("           ogtst forks off another process which runs in \
its own context every\n");
                printf("           time ogEnvLog() is invoked.\n");
                break;
              case 'd':
                printf("Option:d - Select drawble type:\n");
                printf("           d       - windows (default)\n");	   
                printf("           p       - pixmaps\n");
#if defined(GLX_SGIX_pbuffer)
                printf("           P/N[om] - pbuffers P(reserved) or N(non) preserved.\n");
                printf("              Optional: 'o' for pbuffers only.\n");
                printf("                        'm' for pbuffers testing with mapped windows (may \n");
                printf("                        cause frame buffer swapping).\n");
#endif
                break;
              case 'D':
                printf("Option:D - Specify hex debug mask. Bits are:\n");
                printf("              0x1    pause ogtst whenever an \
error gets reported.\n");
                printf("              0x2    check OpenGL state before \
and after test is run.\n");
                printf("              0x8    create a fresh context for \
each test.\n");
                printf("           e.g., -D 2, -D 3\n");
                break;
              case 'e':
                printf("Option:e - Set the error limit for aborting a ogtst. \
When the accumulated\n");
                printf("           errors for a given test exceeds this limit, \
that test is aborted.\n");
                printf("           The default is 10.\n");
                printf("           e.g., -e 20\n");
                break;
              case 'E':
                printf("Option:E - Hold the test window if there is an error\n");
                break;
              case 'f':
                printf("Option:f - Specify the first(,last) test(s) to be run \
according to the order\n");
                printf("           the test are listed in spec.ogtst. The last \
test is optional\n");
                printf("           and defaults to the last test in spec.ogtst.\n");
                printf("           e.g., -f rotate, -f rotate,tstrip\n");
                break;
              case 'F':
                printf("Option:F - Specify the file where the output of the tests should be written\n");
                printf("           e.g., -F /usr/tmp/tstLog\n");
                break;
              case 'h':
                printf("Option:h - Help menu mode. The world according to ogtst.\n");
                break;
              case 'H':
                printf("Option:H - Hold window after each test completes. Allows you to examine the\n");
                printf("           final state of the ogtst until a key is pressed.\n");
                printf("           No parameters.\n");
                break;
              case 'k':
                printf("Option:k - Test skip conditions.  The spec.ogtst \
database contains additional\n");
                printf("           information about the tests, e.g., tests \
that fail due to OpenGL\n");
                printf("           bugs, test that may crash certain system, \
etc.  This option\n");
                printf("           instructs ogtst to avoid running tests \
flagged with certain bugs,\n");
                printf("           or tests of particular types.\n");
                printf("           Options are:\n");
                printf("                c - avoid tests that fail under context switching\n");
                printf("                g - avoid tests that fail due to OpenGL bugs\n");
                printf("                h - avoid checksum tests\n");
                printf("                m - only run checksum tests\n");
                printf("                s - avoid tests that cause a crash\n");
                printf("                t - avoid tests that fail due to ogtst bugs\n");
                printf("           e.g., -k c      e.g., -k tcg\n");
                break;
              case 'l':
                printf("Option:l - Specify the integer log (information) level printed for each test.\n");
                printf("           Log levels range from 1 to 32, although most tests only use 1 and 2.\n");
                printf("           All log levels less that or equal to the input will be printed.\n");
                printf("           e.g., -l 3\n");
                break;
              case 'L':
                printf("Option:L - Specify a hexadecimal log (information) mask that will determine the\n");
                printf("           information levels printed for each test. Bits 1-32 determine which\n");
                printf("           information levels get printed.\n");
                printf("           e.g., -L 0x4 (log level 3 only)  e.g., -L f (log levels 1,2,3,4 only)\n");
                break;
              case 'n':
                printf("Option:n - Sets the Initial maxError for the color checks to 0. In effect it\n");
                printf("           tells the test that Dithering is off so we get errors for color\n");
                printf("           differences of 1 or greater.\n");
                break;
              case 'p':
                printf("Option:p - Specify a float factor by which to scale the number of\n");
                printf("           iterations performed for each test.\n");
                printf("           e.g., -p 2, or -p 0.4, or -p 1.5\n");
                break;
              case 'P':
                printf("Option:P - Specify a time in seconds that will determine how long the testing\n");
                printf("           is to be done for. Tests(s) are repeated until the time is up.\n");
                printf("           e.g., -P 20\n");
                break;
              case 'q':
                printf("Option:q - Run a 4 copies of ogtst in windows tiling the screen\n");
                break;
              case 'r':
                printf("Option:r - Run the tests in a random order. Order may be changed with the -s\n");
                printf("           and -S options.\n");
                printf("           No parameters.\n");
                break;
              case 'R':
                printf("Option:R - Disable pixel readback and checksum comparisons.\n");
                printf("           Speeds up imaged-tests considerably. The \
main purpose is to \n");
                printf("           make the tests serve as stress tests.   With \
this option all\n");
                printf("           imaged tests always pass\n");
                break;
              case 's':
                printf("Option:s - Generate a random seed that will be used as the initial seed for all\n");
                printf("           tests. The seed is generated from the current time. Will also change\n");
                printf("           the random test order when using the -r option.\n");
                printf("           No parameters.\n");
                break;
              case 'S':
                printf("Option:S - Specify an integer seed that will be used as the initial seed for\n");
                printf("           all tests. Will also change the random test order with -r option.\n");
                printf("           e.g., -S 345\n");
                break;
              case 't':
                printf("Option:t - Run specified tests/groups.  There are \
two categories of input:\n");
                printf("           individual tests and groups of tests;\n");
                printf("                tests: only the specified tests(s) will be run. To review\n");
                printf("                       the tests available use the ogtst -h (3) option.\n");
                printf("               groups: only the specified test groups will be run. To review\n");
                printf("                       the tests in each group, use the ogtst -h (2,3) option.\n");
                printf("                       Group names should always start with a capital.\n");
                printf("           Parameters are separated by commas.  Mixing of groups and names\n");
                printf("           is allowed.\n");
                printf("           e.g., -t scale        (run the scale test only)\n");
                printf("           e.g., -t Prims        (run the Prims group of tests only)\n");
                printf("           e.g., -t Xform,tstrip (run Xform group and tstrip test)\n");
                break;
              case 'T':
                printf("Option:T - Specify a file that contains a list \
of tests/groups to run.  The\n");
                printf("           file may contain group/test names listed \
one per line, comment\n");
                printf("           lines beginning with a '#', \
and blank lines.\n");
                printf("           See the -t option for more details.\n");
                printf("           e.g., -T filename\n");
                break;
              case 'u':
                printf("Option:u - Dump checksum data after ogtst completion.  \
The checksum data\n");
                printf("           for all checksum tests will be dumped into \
the file gtst.checksum.\n");
                printf("           Note: If the test was not run, its checksum \
data will be zero.\n");
                printf("           No parameters.\n");
                break;
              case 'w':
                printf("Option:w - Use the specified X window geometry \
as xmin,xmax,ymin,ymax.\n");
                printf("           e.g., -w 320,959,256,767\n");
                break;
              case 'v':
                printf("Option:v - Specify visual restriction mask in hexadecimal.\n");
                printf("           RGB                   0x1\n");
                printf("           RGBA                  0x2\n");
                printf("           INDEX                 0x4\n");
                printf("           SINGLEBUFFER          0x8\n");
                printf("           DOUBLEBUFFER          0x10\n");
                printf("           STEREOBUFFER          0x20\n");
                printf("           AUXBUFFER             0x40\n");
                printf("           DEPTHBUFFER           0x80\n");
                printf("           STENCILBUFFER         0x100\n");
                printf("           ACCUMULATION          0x200\n");
                printf("           MULTISAMPLE           0x400\n");
                printf("           OVERLAY               0x800\n");
                printf("           RGB OR L(Red only)    0x1000\n");
                printf("           RGB OR L_A (R,A)      0x2000\n");
                break;
#if defined(GLX_SGIX_fbconfig)
              case 'U':
                printf(" Option:U - Limit visuals used to those with specified fbconfig ids (hex)\n");
                printf("            Ids are separated by commas.\n");
                break;
#endif
              case 'V':
                printf(" Option:V - Limit visuals used to those with specified X visual ids (hex)\n");
                printf("            Ids are separated by commas.\n");
                break;
              default:
                printf("%s: No such command line option\n", string);
            }
        }
        printf("?-> ");
        fflush(stdout);
    }
}

static void 
helpMenu(void)
{
    fprintf(logfile, "Please enter option:\n");
    fprintf(logfile, "       1  list command line options\n");
    fprintf(logfile, "       2  list Groups\n");
    fprintf(logfile, "       3  list Groups with tests\n");
    fprintf(logfile, "       4  list Tests for specified group\n");
    fprintf(logfile, "  9/Q/^D  Quit\n");
    fprintf(logfile, "  -[a-Z]  command line option information\n");
    fprintf(logfile, "       ?  print this Menu\n");
    fprintf(logfile, "\n");
}

/****************************************************************************
*  listGroups()  -  list group data
****************************************************************************/
static void 
listGroups(int flag,char *gname)
{
   int i, j;
   Group *gn;

   switch (flag)  {
     case 0:                    /* group names */
       printf("\
Group Name (#Tests)\n\
====================\n");
       for (i = 0; i < nGroups; i++) 
         printf("%s (%d)\n", groups[i].name, groups[i].nTests);
       break;
     case 1:                    /* group+test names */
       for (gn = groups, i = 0; i < nGroups; gn++, i++) {
         printf("%s\n", groups[i].name);
         for (j = 0; j < gn->nTests; j++)
           printf("  %3d. %s%s\n",
                  j + 1, tests[gn->tests[j]].name,
                  tests[gn->tests[j]].type == OG_IMAGED ? " (I)" : "");
       }
       break;
     /* test names for specified group */
     case 2:
       if ((i = ogEnvFindGroup(gname)) >= 0) {
           gn = &groups[i];
           for (i = 0; i < gn->nTests; i++)
               printf("  %3d. %s%s\n",
                      i + 1, tests[gn->tests[i]].name,
                      tests[gn->tests[i]].type == OG_IMAGED ? " (I)" : "");
       } else
           printf("Group `%s' does not exist\n", gname);
       break;
     default:
       ogEnvLog(OG_LINTERNALERROR,"listGroups(%d,%s)", flag, gname);
       break;
   }
}

