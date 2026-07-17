/*******************************************************************************
 * 
 * $Header: mgapistruc.h, 4, 10/11/00 7:32:37 PM, Brent$
 * $Revision: 4$
 * $Date: 10/11/00 7:32:37 PM$
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

#ifndef MGAPISTRUC_H_
#define MGAPISTRUC_H_

/*----------------------------------------------------------------------------*/

#ifdef API_LEV5
#include "mgapistruc2.h"
#include "mgapistruc1.h"
#endif

#ifdef API_LEV4
#include "mgapistruc2.h"
#include "mgapistruc1.h"
#endif

#ifdef API_LEV3
#include "mgapistruc2.h"
#include "mgapistruc1.h"
#endif

#ifdef API_LEV2
#include "mgapistruc2.h"
#include "mgapistruc1.h"
#endif

#ifdef API_LEV1
#include "mgapistruc1.h"
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
