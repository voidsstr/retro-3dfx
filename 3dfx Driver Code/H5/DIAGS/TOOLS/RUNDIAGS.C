/*
** Copyright (c) 1995, 3Dfx Interactive, Inc.
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
** $Revision: 3$
** $Date: 10/11/00 8:18:49 PM$
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <wchar.h>
#include <process.h>
#include <io.h>
#include <errno.h>

#define ACCESS_MODE_EXISTENCE 0
#define ISBLANK(s) (s==' ' || s=='\t' || s=='\0')

main(int argc, char **argv)
{
        int i;
        char buffer[256], tmpBuffer[256];
        char cmdName[256];
        char cmdStreamName[100];
        char otherArgs[256];
        FILE *cmdStream;
        int execRetVal;
        int bailOnFailure = 1;
        int loopMode = 0;
        int loopCount = 0;
        int tests, passed, failed;
        int rndmSeed = -1;
        int grxClkFreq = -1;
        int grxClkFreqMin = 40;
        int grxClkFreqMax = 55;
        int maxGrxClkUsed = 0;
        int minGrxClkUsed = 1000;
        char * diagname;
        char diagargs[100];
        char *ps;
        int retVal = 0;

        tests = passed = failed = 0;
        cmdStreamName[0] = (char) NULL;
        otherArgs[0] = (char) NULL;
        for(i=1; i<argc; i++) {
                if(!(strcmp(argv[i], "-f")))
                        strcpy(cmdStreamName, argv[++i]);
                else if(!(strcmp(argv[i], "-i")))
                        bailOnFailure = 0;
                else if(!(strcmp(argv[i], "-l")))
                        loopMode = 1;
                else if(!(strcmp(argv[i], "-s")))
                        rndmSeed = atoi(argv[++i]);
                else if(!(strcmp(argv[i], "-clk")))
                        grxClkFreq = 1;
                else if(!(strcmp(argv[i], "-min")))
                        grxClkFreqMin = atoi(argv[++i]);
                else if(!(strcmp(argv[i], "-max")))
                        grxClkFreqMax = atoi(argv[++i]);
                else {
                        strcat(otherArgs, " ");
                        strcat(otherArgs, argv[i]);
                }
        }
        if(!cmdStreamName[0]) {
                printf("usage: rundiags [-i] [-l] [-s val] [-clk] [-min val] [-max val] -f <command list> [args to diags]\n");
                printf("         -i:   Do not stop for failed diags\n");
                printf("         -l:   Run all tests until failure is found\n");
                printf("         -s:   Run all tests with seed value specified\n");
                printf("         -clk: Increment clock frequency for each pass\n");
                printf("         -min: set minimum clock frequency to value specified\n");
                printf("         -max: set maximum clock frequency to value specified\n");
                exit(0);
        }
        if(grxClkFreq > 0)
                grxClkFreq = grxClkFreqMin;

        // Setup proper Environment...
	//        _putenv("GSIM_WIN32=");
	//        _putenv("GSIM_SST=1");
	//        _putenv("SST_REALHW=1");
	_putenv("SST_DUALHEAD=1");
	_putenv("HAL_VIDEO=0");
        _putenv("HAL_HW=1");
        _putenv("GDBG_LEVEL=0");

rerun:

        if(!(cmdStream = fopen(cmdStreamName, "r"))) {
                printf("ERROR: Could not open command list file '%s'\n", cmdStreamName);
                exit(1);
        }

        while(fgets(cmdName, 256, cmdStream)) {
                cmdName[strlen(cmdName)-1] = (char) NULL;       // strip newline
                if(cmdName[0] != '#' && isalpha(cmdName[0])) {
                        if(rndmSeed != -1) {
                                sprintf(buffer, " -s %d", rndmSeed);
                                strcat(cmdName, buffer);
                        }
                        strcat(cmdName, otherArgs);
                        printf("rundiags INFO: LoopCount:%d TestCount:%d Passed:%d Failed:%d\n",
                                loopCount, tests, passed, failed);
                        if(grxClkFreq > 0) {
                                if(grxClkFreq > maxGrxClkUsed)
                                        maxGrxClkUsed = grxClkFreq;
                                if(grxClkFreq < minGrxClkUsed)
                                        minGrxClkUsed = grxClkFreq;
                                printf("rundiags INFO: grxClk=%d MHz (MinRun:%d MHz, MaxRun:%d MHz)\n",
                                        grxClkFreq, minGrxClkUsed, maxGrxClkUsed);
                                sprintf(tmpBuffer, "SSTH3_GRXCLK=%d", grxClkFreq);
                                _putenv(tmpBuffer);
                        }
                        printf("rundiags: Launching command '%s'\n\n", cmdName);
                        tests++;

                        // Launch specified executable...

                        // separate cmd name from arguments
                        diagname = ps = &cmdName[0];
                        while (ISBLANK(*ps)) ps++;
                        while (!ISBLANK(*ps) ) ps++;
			if ( *ps != '\0' )
			  *ps++ = '\0';
			strcpy (diagargs, ps);
                        strcat (diagname, ".exe");

                        execRetVal = _spawnlp(_P_WAIT, diagname, diagname, diagargs, NULL);

                        if (execRetVal != 0) {
                          printf("rundiags STOP: Command Stream '%s' FAILED...\n", cmdName);
                          if (errno)
                            printf ("        ERROR: %s\n", strerror(errno));
                          failed++;
                          // exit(1);
                          if(bailOnFailure) {
                            retVal = 1;
                            break;
                          }
                        } else
                          passed++;
                }
        }
        fclose(cmdStream);

        if(loopMode && (bailOnFailure == 0 || failed == 0)) {
                if(rndmSeed != -1)
                        rndmSeed++;
                loopCount++;
                if(grxClkFreq > 0) {
                        if(++grxClkFreq > grxClkFreqMax)
                                grxClkFreq = grxClkFreqMin;
                }
                goto rerun;
        }

        printf("\n");
        printf("**********************************************************\n");
        printf("* rundiags Summary:\n");
        printf("* Total Tests: %d\n", tests);
        printf("* Number Passes: %d\n", loopCount+1);
        printf("* Passed: %d\n", passed);
        printf("* Failed: %d\n", failed);

        return retVal;
}
