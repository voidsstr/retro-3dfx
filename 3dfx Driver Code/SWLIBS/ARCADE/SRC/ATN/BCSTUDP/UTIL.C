/* 
** util.c
**
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
** $Date: 10/11/00 7:33:47 PM$ 
**
*/

#include <windows.h>
#include <3dfx.h>
#include "util.h"
#include <fxos.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>



void printC( char *fmt, ... )
{
  int howMany = 0;
  va_list argptr;
  static char buf[1000];
  
  va_start( argptr, fmt );
  vsprintf( buf, fmt, argptr );
  va_end( argptr );
#ifdef __WIN32__
  WriteConsole( GetStdHandle(STD_OUTPUT_HANDLE), buf, strlen(buf), &howMany, NULL);
#else
  fprintf( stderr, "%s", buf );
#endif
}



void    getC(char *c)
{

#ifdef __WIN32__
    int howMany = 0;
    int len = 1;
    ReadConsole( GetStdHandle(STD_INPUT_HANDLE), c, len, &howMany, NULL);
#endif
#ifdef __DOS32__
    *c = getchar();
#endif __DOS32__
}




