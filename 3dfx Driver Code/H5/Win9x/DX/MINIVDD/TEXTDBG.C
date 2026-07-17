/* -*-c++-*- */
/* $Header: textdbg.c, 2, 10/11/00 8:55:12 PM, Brent$ */
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
** File name:   textdbg.c
**
** Description: Support for debugging text routines.
**              THIS FILE IS NOT USED!
**
** $Revision: 2$
** $Date: 10/11/00 8:55:12 PM$
**
** $History: textdbg.c $
** 
** *****************  Version 4  *****************
** User: Michael      Date: 1/15/99    Time: 7:02a
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
** *****************  Version 3  *****************
** User: Ken          Date: 4/15/98    Time: 6:42p
** Updated in $/devel/h3/win95/dx/minivdd
** added unified header to all files, with revision, etc. info in it
**
*/


/*****************************************************************************
 *                                                                           *
 * textdbg.c                                                                 *
 *                                                                           *
 * Copyright (C) 1998, 3Dfx Interactive, Inc.                                *
 * All Rights Reserved.                                                      *
 *                                                                           *
 * Written by F. Weigel (Intelligraphics) for 3Dfx                           *
 * Actually, borrowed from code written by ArtG                              *
 *                                                                           *
 * Text debug functions                                                      *
 *                                                                           *
 *****************************************************************************/


#include "h3.h"
#include "thunk32.h"

#include "textout.h"


/* This function is called on an ASSERT() failure. It prints out
 * the assertion failure, and then traps to the debugger so that
 * the failure can be seen.
 */
void assert_(char *exp, char *filename, unsigned linenumber)
{
    DEBUG_FIX;

    dpf("ASSERT failed: %s (%s:%d)\n", exp, filename, linenumber);
    DEBUG_BREAK;
}
