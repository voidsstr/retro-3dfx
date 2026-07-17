/* -*-c++-*- */
/* $Header: p6stuff.h, 3, 10/11/00 8:54:51 PM, Brent$ */
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
** File name:   p6stuff.h
**
** Description: iP6 externally defined functions.
**
** $Revision: 3$
** $Date: 10/11/00 8:54:51 PM$
**
** $History: p6stuff.h $
** 
** *****************  Version 4  *****************
** User: Jw           Date: 1/22/99    Time: 3:53p
** Updated in $/devel/h3/Win95/dx/minivdd
** Add AMD K6/K7 MTRR support.
** 
** *****************  Version 3  *****************
** User: Michael      Date: 1/12/99    Time: 9:22a
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
** *****************  Version 2  *****************
** User: Ken          Date: 4/15/98    Time: 6:42p
** Updated in $/devel/h3/win95/dx/minivdd
** added unified header to all files, with revision, etc. info in it
**
*/

#ifndef _P6STUFF_H_
#define _P6STUFF_H_


#ifdef WINNT	// OLD CPUID CODE
// Return value defines for CheckForP6

#define	P6_NO_MTRRS                   0	// MTRR's not supported
#define	P6_INTELCPU_WITH_MTRRS        1	// Intel CPU, P6 or higher
#define	P6_AMDK6_MTRRS                4	// AMD K6-style MTRRs
#define	P6_NONINTEL_WITH_INTEL_MTRRS  8	// PentiumII-style MTRRs (K7)

extern void CheckForP6(void);
#endif
// Function Prototypes

extern void GetMTRR(FxU32 msrNum, FxU32 *mtrrLo, FxU32 *mtrrHi);
extern void SetMTRR(FxU32 msrNum, FxU32 mtrrLo, FxU32 mtrrHi);

#endif
