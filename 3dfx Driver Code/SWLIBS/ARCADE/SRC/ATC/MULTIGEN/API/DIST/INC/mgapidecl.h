/*******************************************************************************
 * 
 * $Header: mgapidecl.h, 4, 10/11/00 7:32:17 PM, Brent$
 * $Revision: 4$
 * $Date: 10/11/00 7:32:17 PM$
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

#ifndef MGAPIDECL_H_
#define MGAPIDECL_H_

#ifdef _WIN32

#ifdef API
#undef APIFUNC
#define APIFUNC(retype)		_declspec(dllexport) retype
#else
#undef APIFUNC
#define APIFUNC(retype)		retype
#endif

#else

#define APIFUNC(retype)		retype

#endif	/* _WIN32 */

#endif
