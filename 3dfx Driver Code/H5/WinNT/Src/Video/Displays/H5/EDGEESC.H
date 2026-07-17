/* -*-c++-*- */
/* $Header: edgeesc.h, 3, 10/11/00 8:57:27 PM, Brent$ */
/*
** Copyright (c) 1999, 3Dfx Interactive, Inc.
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
** File name:   edgeesc.h
**
** Description: Driver defines for 3dx Tools escape calls.
**
** $Revision: 3$
** $Date: 10/11/00 8:57:27 PM$
**
** $Log: 
**  3    3dfx      1.1.3.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  2    3dfx      1.1         12/01/99 Reid Campbell   Updated to use standard
**       file header.
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $ 
*/


#if !defined ESCAPES_DEFINED_H

#define ESCAPES_DEFINED_H

#ifndef STB_GUIDMATCH /* used for comparing guids */
#define STB_GUIDMATCH(guid1, guid2)							\
				(														\
					(((DWORD*)&guid1)[0] == ((DWORD*)&guid2)[0]) &&		\
					(((DWORD*)&guid1)[1] == ((DWORD*)&guid2)[1]) &&		\
					(((DWORD*)&guid1)[2] == ((DWORD*)&guid2)[2]) &&		\
					(((DWORD*)&guid1)[3] == ((DWORD*)&guid2)[3])		\
				)
#endif

#ifdef WINNT
int EdgeEscape(int nEscape, void * lpszInData, void * lpszOutData, PDEV * ppdev);
int EdgeInit(PDEV * ppdev);
#else
int EdgeEscape(int nEscape, void * lpszInData, void * lpszOutData);
int EdgeInit();
#endif

#endif

