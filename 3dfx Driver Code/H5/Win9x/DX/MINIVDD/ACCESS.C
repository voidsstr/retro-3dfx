/* -*-c++-*- */
/* $Header: access.c, 2, 10/11/00 8:53:46 PM, Brent$ */
/*
** Copyright (c) 1995-1999, 3Dfx Interactive, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of 3Dfx Interactive, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
** rights reserved under the Copyright Laws of the United States.
**
** File name:   access.c
**
** Description: The BeginAccess/EndAccess function.
**
** $Revision: 2$
** $Date: 10/11/00 8:53:46 PM$
**
** $History: access.c $
** 
** *****************  Version 3  *****************
** User: Michael      Date: 1/04/99    Time: 1:19p
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
** *****************  Version 2  *****************
** User: Ken          Date: 4/15/98    Time: 6:41p
** Updated in $/devel/h3/win95/dx/minivdd
** added unified header to all files, with revision, etc. info in it
**
*/

//: access.c


#include "thunk32.h"


/*----------------------------------------------------------------------
Function name:  BeginAccess

Description:    The GDI BeginAccess function.

Information:    This function is empty.

Return:         VOID
----------------------------------------------------------------------*/
VOID BeginAccess
(
    BeginAccessParams* pParams
)
{
}


/*----------------------------------------------------------------------
Function name:  EndAccess

Description:    The GDI EndAccess function.

Information:    This function is empty.

Return:         VOID
----------------------------------------------------------------------*/
VOID EndAccess
(
    EndAccessParams* pParams
)
{
}
