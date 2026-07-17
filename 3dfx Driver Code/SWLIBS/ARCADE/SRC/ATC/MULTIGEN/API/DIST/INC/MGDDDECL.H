/*******************************************************************************
 * 
 * $Header: mgdddecl.h, 4, 10/11/00 7:32:47 PM, Brent$
 * $Revision: 4$
 * $Date: 10/11/00 7:32:47 PM$
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

#ifndef MGDDDECL_H_
#define MGDDDECL_H_

#ifdef _WIN32

#ifdef DD
#define DDFUNC(retype)		retype _declspec(dllexport)
#else
#define DDFUNC(retype)		retype
#endif

#else

#define DDFUNC(retype)		retype

#endif	/* _WIN32 */

#endif
/*eof*/
