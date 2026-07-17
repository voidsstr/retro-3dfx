/*
 *  tdError.c
 *                                                     12/23/94  0.00  NDR
 *                                                     12/28/94  1.00  NDR
 *                                                     03/01/95  2.00  NDR
 *                                                     09/01/95  4.00  NDR
 *
 *    Three-D Object Library - Library of 3-D object
 *    manipulation routines.
 *
 *    Error message functions module
 *
 *    author: Nathan Drew Robins
 *    email: narobins@es.com
 *
 *
 *    Copyright 1996 by Evans & Sutherland Computer Corporation.  
 *    ALL RIGHTS RESERVED. 
 *    
 *    Permission to use, copy, modify, and distribute this software 
 *    for any purpose and without fee is hereby granted, provided 
 *    that the above copyright notice appear in all copies and that 
 *    both the copyright notice and this notice appear in supporting
 *    documentation.  You may not use the name "Evans & Sutherland 
 *    Computer Corporation" or any trademark of Evans & Sutherland 
 *    Computer Corporation in advertising or publicity pertaining 
 *    to the software without specific, written prior permission. 
 *    
 *    THIS SOFTWARE AND ANY ASSOCIATED MATERIALS IS PROVIDED TO YOU 
 *    "AS-IS" AND WITHOUT REPRESENTATION OR WARRANTY OF ANY KIND, 
 *    EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, 
 *    ANY WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
 *    PURPOSE OR NONINFRINGEMENT OF ANY PATENT, COPYRIGHT, TRADE 
 *    SECRET OR OTHER RIGHT.  EVANS & SUTHERLAND COMPUTER CORPORATION 
 *    DOES NOT WARRANT THAT USE OF THE SOFTWARE WILL BE UNINTERRUPTED
 *    OR ERROR-FREE, OR THAT DEFECTS IN THE SOFTWARE WILL BE CORRECTED.
 *    
 *    IN NO EVENT SHALL EVANS & SUTHERLAND COMPUTER CORPORATION BE 
 *    LIABLE TO YOU OR ANYONE ELSE FOR ANY DIRECT, SPECIAL, INCIDENTAL, 
 *    INDIRECT OR CONSEQUENTIAL  DAMAGES OF ANY KIND, OR ANY DAMAGES 
 *    WHATSOEVER, INCLUDING WITHOUT LIMITATION, LOSS OF PROFIT, LOSS 
 *    OF USE, SAVINGS OR REVENUE, OR THE CLAIMS OF THIRD PARTIES, 
 *    WHETHER OR NOT EVANS & SUTHERLAND COMPUTER CORPORATION  HAS 
 *    BEEN ADVISED OF THE POSSIBILITY OF SUCH LOSS, HOWEVER CAUSED 
 *    AND ON ANY THEORY OF LIABILITY, ARISING OUT OF OR IN CONNECTION 
 *    WITH THE POSSESSION, USE OR PERFORMANCE OF THIS SOFTWARE.
 *    
 *    US Government Users Restricted Rights 
 *    Use, duplication, or disclosure by the Government is subject 
 *    to restricted rights set forth in FAR 52.227.19(c)(2) or 
 *    subparagraph (c)(1)(ii) of the Rights in Technical Data and 
 *    Computer Software clause at DFARS 252.227-7013 and/or in 
 *    similar or successor clauses in the FAR or the DOD or NASA FAR 
 *    Supplement. Contractor/manufacturer is Evans & Sutherland Computer 
 *    Corporation, 600 Komas Drive, Salt Lake City, UT 84108. 
 *    
 *    Use of this software outside the United States is subject to 
 *    US export laws and regulations.  You agree to comply with all 
 *    such laws and regulations.
 *    
 *    This statement is made under and shall be construed and 
 *    governed by the laws of the State of Utah, without regard to 
 *    conflict of laws principles.
 * 
 */


/* includes */
#include <stdio.h>
#include <stdarg.h>

#include "tdPrivate.h"


/* global variable */
static TDboolean TD_Verbose = TD_TRUE;  /* verbosity level (TD_TRUE = all) */
static TDmessage TD_Error = TD_INFO;    /* current error (if one) */

/*
 *  _tdPrintf()
 *
 *    Prints out information in a standard way e.g., 
 *    TD-Library: INFO - this is an information message.
 *    Prints out error messages on the stderr, warning and
 *    info messages on stdout.
 *
 *    Uses the global variable TD_Verbose to mask out certain
 *    message printing.
 *
 *    type - type of message to be printed, one of the following:
 *    TD_INFO     - informative message
 *    TD_WARNING  - warning message
 *    TD_ERROR    - error message
 *    TD_NOMEMORY - out of memory message
 *    TD_FATAL    - fatal error message
 */
void _tdPrintf(
  TDmessage type,     /* type of message */
  char *format, ...   /* format e.g., "%s %s\n", "hello", "world" */
	       )
{
    va_list args;
    char buffer[256], *message = NULL;
    
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);

    switch(type)
    {
    case TD_ERROR:
	message = "ERROR";
	break;

    case TD_FATAL:
	message = "FATAL ERROR";
	break;

    case TD_NOMEMORY:
	message = "OUT OF MEMORY";
	break;

    case TD_WARNING:
	message = "WARNING";
	break;

    case TD_INFO:
	message = "INFO";
	break;

    default:
	break;

    }

    if(type > TD_WARNING)
	fprintf(stderr, "%s: %s - %s\n", TD_NAME, message, buffer);
    else
    {
	if(message && TD_Verbose)
	    printf("%s: %s - %s\n", TD_NAME, message, buffer);
    }

    /* set the current error */
    TD_Error = type;
}

/*
 *  tdVerbose()
 *
 *    Set the verbosity level (all messages on by default)
 *
 *    verbose - boolean value, one of the following:
 *    GL_TRUE   - print out everything (default)
 *    GL_FALSE  - only print out error messages (no warning or info)
 */
void tdVerbose(TDboolean verbose)
{
    TD_Verbose = verbose;
}

/*
 *  tdGetError()
 *
 *    Get the last error (if one)
 *
 */
TDmessage tdGetError()
{
    return(TD_Error);
}
