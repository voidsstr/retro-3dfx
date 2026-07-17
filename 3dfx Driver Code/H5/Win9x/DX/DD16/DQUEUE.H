/* $Header: dqueue.h, 2, 10/11/00 8:51:02 PM, Brent$ */
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
** File name:   dqueue.h
**
** Description: Structure and macros for processing the Dqueue.  This was
**              used early on for device bitmaps.  With the implementation
**              of the memory manager, this appears to no longer be used.
**
** $Revision: 2$
** $Date: 10/11/00 8:51:02 PM$
**
** $History: dqueue.h $
** 
** *****************  Version 2  *****************
** User: Michael      Date: 12/28/98   Time: 11:24a
** Updated in $/devel/h3/Win95/dx/dd16
** Added the 3Dfx/STB unified file/funciton header.  Add VSS keywords to
** files where this was missing.
** 
*/

#ifndef _DQUEUE_H_
#define _DQUEUE_H_

typedef struct dqueue {
   struct dqueue * pLeft;     // Pointer to the left
   struct dqueue * pRight;    // Pointer to the Right
   VOID * pData;              // Pointer to data
   } DQUEUE, * PDQUEUE, FAR * FPDQUEUE;

#define DDELETE(P) \
   P->pLeft->pRight = P->pRight; \
   P->pRight->pLeft = P->pLeft;

#define DINSERT(P,Q) \
   P->pLeft = &Q; \
   P->pRight = Q.pRight; \
   Q.pRight->pLeft = P; \
   Q.pRight = P;

#define DRTRAVERSE(P) \
   P = P->pRight;

#define DLTRAVERSE(P) \
   P = P->pLeft;

#endif
