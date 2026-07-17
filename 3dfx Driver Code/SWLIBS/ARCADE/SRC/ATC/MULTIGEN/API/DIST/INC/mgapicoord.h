/*******************************************************************************
 * 
 * $Header: mgapicoord.h, 4, 10/11/00 7:32:14 PM, Brent$
 * $Revision: 4$
 * $Date: 10/11/00 7:32:14 PM$
 *
 ******************************************************************************/

/*============================================================================*\

   PROPRIETARY RIGHTS NOTICE: All rights reserved.  This software contains 
   proprietary information and trade secrets of MultiGen Inc. of San Jose, 
   California, and embodies substantial creative efforts as well as 
   confidential information, ideas, and expressions.  No part or all of this 
   software may be reproduced in any form, or by any means of electronic, 
   mechanical, or otherwise, without the written permission of MultiGen Inc.

   COPYRIGHT NOTICE: Copyright (C) 1986-1996 MultiGen Inc., San Jose, California.

\*============================================================================*/

/*----------------------------------------------------------------------------*/

#ifndef _MGAPICOORD_H
#define _MGAPICOORD_H


/*============================================================================*\
	public types
\*============================================================================*/


typedef struct vector {
	float i, j, k;
} vector, *vectorpt;

typedef struct dvector {
	double i, j, k;
} dvector, *dvectorpt;

typedef struct fcoord {
   float x, y, z;
} fcoord, *fcoordpt;

typedef struct icoord {	
	double x, y, z;
} icoord, *icoordpt;

typedef struct dline {
	icoord p1, p2;
} dline, *dlinept;


/*----------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*/

/*============================================================================*/

#ifdef __cplusplus
}
#endif

#endif
/* DON'T ADD STUFF AFTER THIS #endif */
