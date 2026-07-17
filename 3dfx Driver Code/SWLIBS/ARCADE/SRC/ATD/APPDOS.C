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
** $Date: 10/11/00 7:33:18 PM$ 
**
*/

#include <glide.h>

#include <atutil.h>
#include <atrender.h>
#include <atinput.h>
#include <ataudio.h>
#include <atdemop.h>
#include <conio.h>
#include <stdarg.h>


/* DOS frontend for demo applications so that they can run in different OS
   environments (WIN95, DOS, MacOS etc. without requiring code changes.
   Assumes a simple model of 1 display surface. Handles input from a 
   single window
 */


/*
 * Create a default window, parse the command line for options and call main
 */

int
main(int argc, char **argv) {
    /*
     * Initialize the global variables
     */

    _atdInitGlobals();

    /* Parse the command line and search for standard options */
    
    _atdParseCmdLine( argc, argv );

#ifdef __WIN32__
    _atGlobals.hInstApp = GetModuleHandle(NULL);

    return atdWinMain();
#else
    atuErrorSetCallback( atdErrorCallback );

    if (!AppInitGraphics()) {
       atuError(FXTRUE, "Can't initialize graphics\n");
    }

    /* initialize input library */

    if (!atiInit(NULL)) {
       atuError(FXTRUE, "Can't initialize input\n");
    }

    /* TBD: no audio under DOS yet */
    /* ataInit( (FxU32)_atGlobals.hWndMain ); */

    /* run the application */

    AppMain(_atGlobals.argc, _atGlobals.argv);

    /* cleanup */

    atiShutdown();
    AppTermGraphics();
    return 0;

#endif
}

/* if we are really running under windows use the windows based
   functions defined in appwin.c
 */

#ifndef __WIN32__

FxBool atdPause(FxBool flag) {
    if ( _atGlobals.graphicsEnabled ) {
        _atGlobals.bPaused = flag;           
        return atrPause(flag);
    }

    return FXTRUE;
}
void atdSetName(char *name) {
}

HINSTANCE atdGetInstance(void) {
    return NULL;
}

/*-------------------------------------------------------------------
  Function: atdErrorCallback
  Date: 10/11/96
  Implementor(s): mlwp
  Library: AT Demo
  Description:
    DOS version of general purpose error callback and default.  Just creates
    a message box with the specified message and conditionally exits with 
    the exit API.
  Arguments:
    fatal - if true then error is not recoverable, so exit
    format - printf style format string
    ... - arguments determined by format string
  Return:
    none
  -------------------------------------------------------------------*/
static void 
atdErrorCallback( FxBool fatal, const char *format, ... ) {
    va_list args;

    atdPause(FXTRUE);
    va_start( args, format );
    vfprintf( stdout, format, args );
    va_end( args );
    
    if ( fatal ) {
        exit( -1 );
    }
    atdPause(FXFALSE);
}

void atdSetFocus(void) {
}

/*-------------------------------------------------------------------
  Function: atdPrintf
  Date: 10/11/96
  Implementor(s): mlwp
  Library: AT Demo
  Description:
    Multi platform printf function
  Arguments:
    format - printf style format string
    ... - arguments determined by format string
  Return:
    none
  -------------------------------------------------------------------*/

void
atdPrintf( const char *format, ... ) {
    va_list args;
    va_start( args, format );
    vfprintf( stdout, format, args );
    va_end( args );
}

/*-------------------------------------------------------------------
  Function: atdGetString
  Date: 10/11/96
  Implementor(s): mlwp
  Library: AT Demo
  Description:
    Multi platform get string function
  Arguments:
    c        - buffer to receieve string
    buffSize - size of buffer
  Return:
    String read, or NULL on error
  -------------------------------------------------------------------*/

char *
atdGetString(char *c, int buffSize) {
    return fgets(c, buffSize, stdin);
}

/*-------------------------------------------------------------------
  Function: atdGetChar
  Date: 10/11/96
  Implementor(s): mlwp
  Library: AT Demo
  Description:
    Multi platform get character function
  Arguments:
    None
  Return:
    Character read
  -------------------------------------------------------------------*/

int
atdGetChar(void) {
    return getchar();
}
#endif
