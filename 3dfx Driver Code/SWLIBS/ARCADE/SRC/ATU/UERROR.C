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
** $Date: 10/11/00 7:34:53 PM$ 
**
*/

#include "atutil.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#if macintosh
#include <Types.h> /* for DebugStr() */
unsigned char err_buffer[2048];
#endif

/*-------------------------------------------------------------------
  Function: _atuStdErrorCallback
  Date: 3/16/96    
  Implementor(s): jdt
  Library: AT Utility
  Description:
    General purpose error callback and default.  Just prints out
    the message to stdout and conditionally exits with the exit API.    
  Arguments:
    fatal - if true then error is not recoverable, so exit
    format - printf style format string
    ... - arguments determined by format string
  Return:
    none
  -------------------------------------------------------------------*/
void _atuStdErrorCallback( FxBool fatal, const char *format, ... ) {
    va_list args;
    #if macintosh
    int len;
    extern int debugger_avail;

    if(debugger_avail) {
        va_start( args, format );
        vsprintf((void*)(err_buffer+1), format, args);
        va_end( args );
        len = strlen((void*)(err_buffer+1));
        if(len > 255) {
            len = 255;
        }
        err_buffer[0] = (unsigned char)len;
        DebugStr(err_buffer);
    }
    #else
    va_start( args, format );
    vfprintf( stdout, format, args );
    va_end( args );
    #endif

    if ( fatal ) 
      {
        exit( -1 );
      }
    return;
}

AtuErrorCallback atuError = &_atuStdErrorCallback;

/*-------------------------------------------------------------------
  Function: atuErrorSetCallback
  Date: 3/15/96
  Implementor(s): jdt
  Library: AT Utility ( error handling )
  Description:
    Sets the default error callback.
  Arguments:
    c - function pointer to user error callback
  Return:
    none
  -------------------------------------------------------------------*/
void atuErrorSetCallback( AtuErrorCallback c  ) {
    atuError = c;
    return;
}




