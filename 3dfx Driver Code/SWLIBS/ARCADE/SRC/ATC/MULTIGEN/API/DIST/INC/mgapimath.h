/*******************************************************************************
 * 
 * $Header: mgapimath.h, 4, 10/11/00 7:32:28 PM, Brent$
 * $Revision: 4$
 * $Date: 10/11/00 7:32:28 PM$
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

#ifndef MGAPIMATH_H_
#define MGAPIMATH_H_

/*----------------------------------------------------------------------------*/

#ifdef API_LEV5
#include "mgapimath4.h"
#include "mgapimath1.h"
#endif

#ifdef API_LEV4
#include "mgapimath4.h"
#include "mgapimath1.h"
#endif

#ifdef API_LEV3
#include "mgapimath1.h"
#endif

#ifdef API_LEV2
#include "mgapimath1.h"
#endif

#ifdef API_LEV1
#include "mgapimath1.h"
#endif

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
