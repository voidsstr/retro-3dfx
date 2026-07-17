/*******************************************************************************
 * 
 * $Header: mgapixform.h, 4, 10/11/00 7:32:45 PM, Brent$
 * $Revision: 4$
 * $Date: 10/11/00 7:32:45 PM$
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

#ifndef _MGAPIXFORM_H
#define _MGAPIXFORM_H

/*----------------------------------------------------------------------------*/

/***************************************************
*																	*
*	transformation linked list op codes 				*
*																	*
***************************************************/

typedef enum	mgxfllcode {

	XLL_TRANSLATE	= 'T',
	XLL_SCALE		= 'S',
	XLL_ROTEDGE		= 'E',
	XLL_ROTPT		= 'R',
	XLL_PUT			= 'P',
	XLL_TOPOINT		= 'p',
	XLL_GENERAL		= 'M'

} mgxfllcode;

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
