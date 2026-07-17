/*
** Copyright (c) 1996, 3Dfx Interactive, Inc.
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
**
** $Revision: 4$ 
** $Date: 10/11/00 7:33:20 PM$ 
**
*/

#include <glide.h>

#include <atutil.h>
#include <atrender.h>
#include <atinput.h>
#include <atdemop.h>
#if !macintosh
#include <conio.h>
#endif

#ifdef __DOS32__
#define lstrcmp strcmp
#endif

/*
 * _atdInitGlobals
 * Called once at program initialization to initialize global variables.
 */

AtGlobals _atGlobals;     /* collection of global variables */

void
_atdInitGlobals(void) {
    memset(&_atGlobals, 0, sizeof(_atGlobals));
    strcpy(_atGlobals.appName, "ATB Demo");
    _atGlobals.texture = FXTRUE;
    _atGlobals.bilinear = FXTRUE;
    _atGlobals.mipMap = FXTRUE;
    _atGlobals.plug = FXTRUE;
    _atGlobals.historyFileName = "app.evt";
    _atGlobals.sync = FXTRUE;
    _atGlobals.dithering = FXTRUE;
    _atGlobals.fullScreen = FXTRUE;
    _atGlobals.fullView = FXTRUE;
    _atGlobals.emulation = FXFALSE;
    _atGlobals.eventMode = ATI_EM_DEFAULT;
#if macintosh
    _atGlobals.sound = FXTRUE;
#endif
#ifdef BENCHMARK
    /* initialize the parameters in the benchmark structure */
    memset( &_atBenchmarkInfo, 0, sizeof( _atBenchmarkInfo ) );
    _atBenchmarkInfo.playbackOn     = FXFALSE;
    _atBenchmarkInfo.playbackDone   = FXFALSE;
    _atBenchmarkInfo.recordOn       = FXFALSE;
    _atBenchmarkInfo.dumpStats      = FXFALSE;
    _atBenchmarkInfo.filename       = '\0';
    _atBenchmarkInfo.numBenchmarks  = 1;
    _atBenchmarkInfo.currentBenchmarks = 1;
    _atBenchmarkInfo.averagedBenchmarks = ( double )0.0;

    /* turn off v-sync for benchmark numbers */
    _atGlobals.sync = FXFALSE;
#endif
}


/*
 * Parse the command line and seach for one of the following options:
 *    -d[river] <name> Name of driver
 *    -emulation       Do not use hardware
 *    -n count         Number of times to repeat script
 *    -f history       Input script
 *    -nosync          Do not wait for vertical retrace
 * These arguments are removed from the command line before being passed to
 * the application.
 */

#if !macintosh
void _atdParseCmdLine( int argc, char **argv ) {
    int i;

    _atGlobals.argc = 0;
    _atGlobals.argv = argv;

    for ( i = 0; i < argc; i++ ) {
        if ((strcmp(argv[i], "-window") == 0) || 
            (strcmp(argv[i], "-win") == 0)) {
            _atGlobals.fullScreen = FXFALSE;
        }
    }

    /* set default dimensions for view surface */

    _atGlobals.driverInfo.width = ( _atGlobals.fullScreen ) ? SCREEN_X : WIN_X;
    _atGlobals.driverInfo.height = ( _atGlobals.fullScreen ) ? SCREEN_Y : WIN_Y;

    for ( i = 0; i < argc; i++ ) {
        if (!lstrcmp(argv[i], "-device") || !lstrcmp(argv[i], "-d")) {
            _atGlobals.driverName = argv[i+1]; i++;
        } else if (!lstrcmp(argv[i], "-emulation")) {
            _atGlobals.emulation = FXTRUE;
        } else if ( !lstrcmp(argv[i], "-n" ) ) {
            _atGlobals.repeatCount = atoi(argv[i+1]); i++;
            _atGlobals.eventMode = ATI_EM_PLAYBACK ;
        } else if ( !lstrcmp(argv[i], "-f" ) ) {
            _atGlobals.historyFileName  = argv[i+1]; i++;
        } else if ( !lstrcmp(argv[i], "-w" ) || !lstrcmp(argv[i], "-width" ) ) {
            i++;
            _atGlobals.driverInfo.width = atoi(argv[i]);
        } else if (!lstrcmp(argv[i], "-h" ) || !lstrcmp(argv[i], "-height" )) {
            i++;
            _atGlobals.driverInfo.height = atoi(argv[i]);
        } else if (strcmp(argv[i], "-sound") == 0) {
            _atGlobals.sound = FXTRUE;
        } else if (strcmp(argv[i], "-nosync") == 0) {
            _atGlobals.sync = FXFALSE;
        } else if (strcmp(argv[i], "-onetmu") == 0) {
            _atGlobals.oneTMU = FXTRUE;
        } else if (strcmp(argv[i], "-console") == 0) {
            _atGlobals.hasConsole = FXTRUE;
        } else if ((strcmp(argv[i], "-window") == 0) || 
            (strcmp(argv[i], "-win") == 0)) {
        } else if (strcmp(argv[i], "-debug") == 0) {
            _atGlobals.debugFile = fopen(argv[++i], "w");
        } else if (strcmp(argv[i], "-nodither") == 0 ) {
            _atGlobals.dithering = FXFALSE;
        } else {
            _atGlobals.argv[_atGlobals.argc++] = argv[i];
        }
    }

    return;
}
#endif
