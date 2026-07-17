/*******************************************************************************
 * 
 * $Header: mgapiio1.h, 4, 10/11/00 7:32:21 PM, Brent$
 * $Revision: 4$
 * $Date: 10/11/00 7:32:21 PM$
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

#ifndef MGAPIIO1_H_
#define MGAPIIO1_H_

/*----------------------------------------------------------------------------*/

#include "mgapibase.h"

/*----------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif
	
/*============================================================================*/

extern APIFUNC(void) mgInit ( int* argc, char* argv [] );
extern APIFUNC(mgrec) *mgNewDb ( char* fname );
extern APIFUNC(mgrec) *mgOpenDb ( char* fname );
extern APIFUNC(mgbool) mgCloseDb ( mgrec* db );
extern APIFUNC(mgbool) mgExit ( void );


/*============================================================================*/

#ifdef __cplusplus
}
#endif

#endif
/* DON'T ADD STUFF AFTER THIS #endif */
