/* -*-c++-*- */
/* $Header: h3gblaxs.c, 2, 10/11/00 8:54:22 PM, Brent$ */
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
** File name:   h3gblaxs.c
**
** Description: The GETGBL_dwOvlOffset function.
**
** $Revision: 2$
** $Date: 10/11/00 8:54:22 PM$
**
** $History: h3gblaxs.c $
** 
** *****************  Version 6  *****************
** User: Xingc        Date: 9/03/99    Time: 10:38a
** Updated in $/devel/h5/Win9x/dx/minivdd
** delete functions to get pfnVMIPLDRead and pfnVMIPLDWrite from
** GLOBALDATA
** 
** 
** *****************  Version 5  *****************
** User: Lpost        Date: 8/25/99    Time: 2:15p
** Updated in $/devel/h5/Win9x/dx/minivdd
** V3TV code merge into H5
** 
** *****************  Version 3  *****************
** User: Xingc        Date: 4/08/99    Time: 5:27p
** Updated in $/devel/h3/Win95/dx/minivdd
** Add a function to get KMVTBUFF pointer
** 
** *****************  Version 2  *****************
** User: Michael      Date: 1/07/99    Time: 1:27p
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
**
*/
#include "thunk32.h"


/*----------------------------------------------------------------------
Function name:  GETGBL_dwOvlOffset

Description:    Get the lpDriverData->dwOvlOffset from GLOBALDATA.
                
Information:

Return:         DWORD   Return lpDriverData->dwOvlOffset
----------------------------------------------------------------------*/
DWORD GETGBL_dwOvlOffset(DWORD lpDriverData)
{
   return( ((GLOBALDATA *)lpDriverData)->dwOvlOffset );
}

/*----------------------------------------------------------------------
Function name:  GETGBL_KMVTBuff

Description:    Get the lpDriverData->KMVTBuff from GLOBALDATA.
                
Information:

Return:         DWORD   Return lpDriverData->KMVTBuff
----------------------------------------------------------------------*/
DWORD GETGBL_KMVTBuff(DWORD lpDriverData)
{
   return( ((GLOBALDATA *)lpDriverData)->KMVTBuff );
}



