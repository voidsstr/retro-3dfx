/*******************************************************************************
 * 
 * $Header: mgapibase1.h, 4, 10/11/00 7:32:11 PM, Brent$
 * $Revision: 4$
 * $Date: 10/11/00 7:32:11 PM$
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

#ifndef MGAPIBASE1_H_
#define MGAPIBASE1_H_

/*----------------------------------------------------------------------------*/

#include "mgapistd.h"
#include "mgapidecl.h"

/*----------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*/

extern APIFUNC(mgcode) mgGetCode ( mgrec* rec );
extern APIFUNC(mgcode) mgGetMyCode ( mgrec* rec );
extern APIFUNC(mgbool) mgIsTypeUcval ( mgrec* rec );
extern APIFUNC(mgbool) mgIsTypeSval ( mgrec* rec );
extern APIFUNC(mgbool) mgIsTypeUsval ( mgrec* rec );
extern APIFUNC(mgbool) mgIsTypeFval ( mgrec* rec );
extern APIFUNC(mgbool) mgIsTypeIval ( mgrec* rec );
extern APIFUNC(mgbool) mgIsTypeUival ( mgrec* rec );
extern APIFUNC(mgbool) mgIsTypeDval ( mgrec* rec );
extern APIFUNC(mgbool) mgIsTypeFlag ( mgrec* rec );
extern APIFUNC(mgbool) mgIsTypeText ( mgrec* rec );
extern APIFUNC(mgbool) mgIsTypeRec ( mgrec* rec );
extern APIFUNC(mgbead) *mgGetBead ( mgrec* rec );
extern APIFUNC(mgbool) mgIsCode ( mgrec* rec, mgcode code );
extern APIFUNC(void) *mgGetUserData ( mgrec* rec );
extern APIFUNC(void) mgSetUserData ( mgrec* rec, void* user );

/*============================================================================*/

#ifdef __cplusplus
}
#endif

#endif
/* DON'T ADD STUFF AFTER THIS #endif */
