/*******************************************************************************
 * 
 * $Header: mgapigeom4.h, 4, 10/11/00 7:32:19 PM, Brent$
 * $Revision: 4$
 * $Date: 10/11/00 7:32:19 PM$
 *
 ******************************************************************************/

/*============================================================================*\

   PROPRIETARY RIGHTS NOTICE: All rights reserved.  This software contains 
   proprietary information and trade secrets of MultiGen Inc. of San Jose, 
   California, and embodies substantial creative efforts as well as 
   confidential information, ideas, and expressions.  No part or all of this 
   software may be reproduced in any form, or by any means of electronic, 
   mechanical, or otherwise, without the written permission of MultiGen Inc.

   COPYRIGHT NOTICE: Copyright(C) 1986-1996 MultiGen Inc., San Jose, California.

\*============================================================================*/

/*----------------------------------------------------------------------------*/

#ifndef MGAPIGEOM4_H_
#define MGAPIGEOM4_H_

/*----------------------------------------------------------------------------*/

#include "mgapibase.h"
#include "mgapicoord.h"

/*----------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*/

extern APIFUNC(icoord) mgCoordDif ( icoord *a, icoord *b );
extern APIFUNC(dline) mgMakeLine ( icoord *ip1, icoord *ip2 );
extern APIFUNC(icoord) mgVectorMove ( icoord *ip, vector *vec, float n );
extern APIFUNC(fcoord) mgi2fcoord ( icoord *ip );
extern APIFUNC(vector) mgDvectorToVector ( dvector *dv );

/*============================================================================*/

#ifdef __cplusplus
}
#endif

#endif
/* DON'T ADD STUFF AFTER THIS #endif */
