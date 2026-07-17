/* -*-c++-*- */
/* $Header: textout.h, 2, 10/11/00 8:55:16 PM, Brent$ */
/*
** Copyright (c) 1995-1999, 3Dfx Interactive, Inc.
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
** File name:   textout.h
**
** Description: Support for ExtTextOut.
**
** $Revision: 2$
** $Date: 10/11/00 8:55:16 PM$
**
** $History: textout.h $
** 
** *****************  Version 10  *****************
** User: Michael      Date: 1/15/99    Time: 8:40a
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
** *****************  Version 9  *****************
** User: Ken          Date: 4/27/98    Time: 5:35p
** Updated in $/devel/h3/win95/dx/minivdd
** complete (well, almost) enter/leave implementation for gdi32
** 
** *****************  Version 8  *****************
** User: Ken          Date: 4/24/98    Time: 9:43p
** Updated in $/devel/h3/win95/dx/minivdd
** cleaned up some functions, added R32 macros, 
** 
** *****************  Version 7  *****************
** User: Ken          Date: 4/15/98    Time: 6:42p
** Updated in $/devel/h3/win95/dx/minivdd
** added unified header to all files, with revision, etc. info in it
**
*/


/*****************************************************************************
 *                                                                           *
 * textout.h                                                                 *
 *                                                                           *
 * Copyright (C) 1998, 3Dfx Interactive, Inc.                                *
 * All Rights Reserved.                                                      *
 *                                                                           *
 * Written by F. Weigel (Intelligraphics) for 3Dfx                           *
 *                                                                           *
 * Defines for textout.                                                      *
 *                                                                           *
 *****************************************************************************/


#ifndef TEXTOUT_INCLUDED
#define TEXTOUT_INCLUDED 1


#define PARANOID_FLAG   0
#define ASSERT_FLAG     0
#define STATISTICS_FLAG 0


/* Optimization for Microsoft compiler. We will use these functions
 * when possible, but will wrap them in case another compiler is
 * used. These functions will generate in-line code, *not* function
 * calls (and we aren't linking with the RTL anyway).
 */

#include <string.h>     /* for memcmp, memcpy, memset,
                         *     strcpy, strcat, strlen, strcmp, strset
                         */
#include <stdlib.h>     /* for abs, labs (fabs also possible) */

#pragma intrinsic(memcmp, memcpy, memset)
#pragma intrinsic(strcpy, strcat, strlen, strcmp, strset)
#pragma intrinsic(abs, labs)


/* Boolean constants */
#define YES 1
#define NO 0


/* We want paranoid and statistics code to go away in production releases */

#ifndef PARANOID_FLAG
#   ifdef DEBUG
#       define PARANOID_FLAG 1
#   else
#       define PARANOID_FLAG 0
#   endif
#endif


#ifndef ASSERT_FLAG
#   ifdef DEBUG
#       define ASSERT_FLAG 1
#   else
#       define ASSERT_FLAG 0
#   endif
#endif


#ifndef NO_ENGINE_FLAG
#   define NO_ENGINE_FLAG 0
#endif


#ifndef STATISTICS_FLAG
#   ifdef DEBUG
#       define STATISTICS_FLAG 1
#   else
#       define STATISTICS_FLAG 0
#   endif
#endif


#ifndef DEBUG_FLAG
#   ifdef DEBUG
#       define DEBUG_FLAG 1
#   else
#       define DEBUG_FLAG 0
#   endif
#endif


/* Compile type assert. If the compile time assertion is violated,
 * generate an error message. (arrays of size 0 are not allowed)
 */
#define ASSERT_AT_COMPILE(x) { typedef int _c_assert[(x) != 0]; }


/* Make DEBUG code go away in production release */
#if DEBUG_FLAG
#   define DEBUG_CODE(d) d
#else
#   define DEBUG_CODE(d)
#endif


/* Paranoia
 *
 * P(c,x) asserts condition c. If c is false, does x (usually return)
 */
#if PARANOID_FLAG
#   define PARANOID(c,x) { ASSERT((c)); if (!(c)) x; }
#else
#   define PARANOID(c,x)
#endif /* PARANOID_FLAG */


/* Debug break */
#define DEBUG_BREAK DEBUG_CODE( { __asm int 3 } )


/* Disable simulator macros, but still do syntax checking */
#if NO_ENGINE_FLAG
#   undef CMDFIFO_PROLOG
#   undef CMDFIFO_SETUP
#   undef CMDFIFO_CHECKROOM
#   undef CMDFIFO_EPILOG
#   undef BUMP
#   undef SETPH
#   undef SET
#   define CMDFIFO_PROLOG(p)
#   define CMDFIFO_SETUP(p)
#   define CMDFIFO_CHECKROOM(p1,p2) { int r_ = (p2); }
#   define CMDFIFO_EPILOG(p)
#   define BUMP(p)                  { int r_ = (p); }
#   define SETPH(p1, p2)            { int r_ = (p2); }
#   define SET(p1, p2, p3)          { int r_ = ((p2), (p3)); }
#endif /* NO_ENGINE_FLAG */


/* Convert pointer from FAR to FLAT. FarToFlat is wrapped in CVP to handle
 * NULL pointer cases correctly.
 */
#define FLATTEN_POINTER(p) { if ( lp->p == 0 ) p = NULL; else FarToFlat(lp->p, p); }


/* Local variable whose address will be taken */
#define TAKE_ADDRESS static


/* Declare flattened pointer */
#define DEFINE_FLAT_POINTER(t,n) t *n

/* Run-time assertion */

#if ASSERT_FLAG
#   define ASSERT(exp) ((exp) ? (void)0 : assert_(#exp, __FILE__, __LINE__))
#else
#   define ASSERT(exp) ((void)0)
#endif


/* min and max macros */
#define max(a, b)  (((a) > (b)) ? (a) : (b))
#define min(a, b)  (((a) < (b)) ? (a) : (b))


#if STATISTICS_FLAG
#   define STATISTIC(x) x
#else
#   define STATISTIC(x)
#endif


/* Prototypes */

void assert_(char *exp, char *filename, unsigned linenumber);


/* Convert board offset to host address */
#define LFB_ADDRESS(x)  (lpDriverData->lfbBase + (x))


/* Wait for engine idle */
#define WAIT_FOR_IDLE {while ( GET(lpIOregs->status) & SST_BUSY ) ;}


/* Compare bytes, return 1 if equal */
#define compare_memory(s1,s2,n) (memcmp(s1,s2,n) == 0)


/* Copy bytes */
#define copy_memory(dst,src,n)  (memcpy(dst,src,n))


/* Zero bytes */
#define clear_memory(dst,n)     (memset(dst,0,n))



#endif /* TEXTOUT_INCLUDED */
