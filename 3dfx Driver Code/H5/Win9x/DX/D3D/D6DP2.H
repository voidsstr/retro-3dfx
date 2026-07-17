/*
** Copyright (c) 1998, 3Dfx Interactive, Inc.
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
** File name:   d6dp2.h
**
** Description: DrawPrimitive2 ddi header
**
** $Revision: 2$
** $Date: 10/11/00 8:48:08 PM$
**
** $Log: 
**  2    3dfx      1.0.1.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
** 
** 2     7/28/99 11:59p Bseitsin
** Changes to enable DX7 for W9x.
** 
** 1     6/02/99 6:45a Michael
** Branch from H3
** 
** 4     1/26/99 5:28p Peterm
** Added unified header information
** 
** 3     10/15/98 6:35p Artg
** changed ifdef h3 to account for h4
** ifdef h3  --> if defined(h3) || defined(h4)
** 
** 2     7/24/98 1:37p Hohn
** 
** 1     5/06/98 6:23p Adrians
** New DX6 files.
** 
** 2     5/01/98 4:11p Adrians
** Compile options for dx5 and dx6.
** Removed redundent returns.
** 
** 
** 
** 1     4/29/98 6:31p Adrians
** Created
*/

#include <d3dhal.h>

#if( DX >= 6 )

extern PFND3DPARSEUNKNOWNCOMMAND dp2Callback;

#endif // DX >= 6
