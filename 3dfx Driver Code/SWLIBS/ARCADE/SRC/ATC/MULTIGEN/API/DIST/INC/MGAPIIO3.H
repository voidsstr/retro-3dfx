/*******************************************************************************
 * 
 * $Header: mgapiio3.h, 4, 10/11/00 7:32:23 PM, Brent$
 * $Revision: 4$
 * $Date: 10/11/00 7:32:23 PM$
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

#ifndef MGAPIIO3_H_
#define MGAPIIO3_H_

/*----------------------------------------------------------------------------*/

#include "mgapibase.h"
#include "mgapidd.h"

/*----------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif
	
/*============================================================================*/

extern APIFUNC(void) mgRegisterCallback ( siteinfo *site, char *func_name, mgbool (*func)() );

extern APIFUNC(void) mgRegisterPreAction ( siteinfo *site, mgcode code, 
		mgbool (*func)(action_type type, mgrec *par_rec, mgcode code, void* val) );
extern APIFUNC(void) mgRegisterPostAction ( siteinfo *site, mgcode code, 
		mgbool (*func)(action_type type, mgrec *par_rec, mgrec *rec) );

/*============================================================================*/

#ifdef __cplusplus
}
#endif

#endif
/* eof */
