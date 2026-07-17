/* -*-c++-*- */
/* $Header: polygon.h, 2, 10/11/00 8:54:58 PM, Brent$ */
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
** File name:   polygon.h
**
** Description: Structures, macros, externs to support the
**              GDI polygon routines.
**
** $Revision: 2$
** $Date: 10/11/00 8:54:58 PM$
**
** $History: polygon.h $
** 
** *****************  Version 5  *****************
** User: Michael      Date: 1/12/99    Time: 11:00a
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
** *****************  Version 4  *****************
** User: Ken          Date: 4/15/98    Time: 6:42p
** Updated in $/devel/h3/win95/dx/minivdd
** added unified header to all files, with revision, etc. info in it
**
*/

extern DWORD HandleBrush(DIB_Brush8 * lpBrush, DWORD Rop, 
					DRAWMODE  * lpDrawMode);

extern DWORD HandleFormat(DIBENGINE FAR * Src);

typedef DIB_Pen * LPpen;

LONG hwOsPolyLine
(
	OutputParams* pParams,
	LPDIBENGINE   lpDestDev,
	LPRECT        lpClipRect
);


#define	DONT_INTERSECT   -1
#define	DO_INTERSECT      1
#define COLLINEAR         0

#define MINUS            -1
#define PLUS              1

#define COUNTERCLOCKWISE -1
#define CLOCKWISE         1
#define NOTSIMPLECONVEX   0

/* Local variable whose address will be taken */
#define TAKE_ADDRESS static


LONG    
hwOsAltPolygon
(
    OutputParams * pParams,
    LPDIBENGINE    lpDestDev,
    LPRECT         lpClipRect
);

LONG
SpeedyPGon(
	OutputParams * pParams,
	LPDIBENGINE    lpDestDev,
	LPRECT         lpClipRect,
    LPPOINT        lpDstPoints
);

LONG 
SimpleConvexPolygon 
(
	OutputParams * pParams,
	LPDIBENGINE    lpDestDev,
	LPRECT         lpClipRect,
    LPPOINT        lpDstPoints,
    int            NumVerts,
    LONG           direction
);


LONG
ComplexQuad(
	OutputParams * pParams,
	LPDIBENGINE    lpDestDev,
	LPRECT         lpClipRect,
    LPPOINT        lpDstPoints
);

LONG
SimpleConcaveQuad(
	OutputParams * pParams,
	LPDIBENGINE    lpDestDev,
	LPRECT         lpClipRect,
    LPPOINT        lpDstPoints,
    DWORD          breakvertex
);


LONG    
Intersect
(
    LPPOINT p1,                 
    LPPOINT p2,                 
    LPPOINT p3,                 
    LPPOINT p4,                 
    LPPOINT intersection        
);

INLINE LONG 
XProduct
(
    LPPOINT p0, 
    LPPOINT p1, 
    LPPOINT p2
);

LONG
SimpleConvexCheck 
(
    LPPOINT       lpDstPoints,
    int           NumVtxs
);

LONG 
RenderTrianglePair 
(
	OutputParams     * pParams,
	LPDIBENGINE        lpDestDev,
	LPRECT             lpClipRect,
    LPPOINT            t1p1,
    LPPOINT            t1p2,
    LPPOINT            t1p3,
    LPPOINT            t2p1,
    LPPOINT            t2p2,
    LPPOINT            t2p3
);



#define SWAP_PT(a,b,t) t = a; a = b; b = t;

#define SORT_TRI(p1,p2,p3,Temp) \
    if (p1->y > p2->y) {        \
        SWAP_PT(p1,p2,Temp);    \
    }                           \
    if (p2->y > p3->y) {        \
        SWAP_PT(p2,p3,Temp);    \
    }                           \
    if (p1->y > p2->y) {        \
        SWAP_PT(p1,p2,Temp);    \
    }                           



