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
** File name:   d6temp.c
**
** Description: Miscellaneous functions
**
** $Revision: 3$
** $Date: 10/11/00 8:48:09 PM$
**
** $Log: 
**  3    3dfx      1.0.1.1     10/11/00 Brent           Forced check in to enforce
**       branching.
**  2    3dfx      1.0.1.0     09/22/00 Johnny Trainor  Preparation for DX8 support
**       in the driver. Modifications so that we can build the Win9x driver using
**       the Win98 DDK and the DX8 DDK. 
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
** 
** 3     7/28/99 11:59p Bseitsin
** Changes to enable DX7 for W9x.
** 
** 2     6/04/99 7:15p Bseitsin
** 24bit Z/8bit stencil mods.
** 
** 1     6/02/99 6:45a Michael
** Branch from H3
** 
** 9     1/26/99 5:29p Peterm
** Added unified header information
** 
** 8     10/15/98 6:35p Artg
** changed ifdef h3 to account for h4
** ifdef h3  --> if defined(h3) || defined(h4)
** 
** 7     9/25/98 6:45p Adrians
** Addtional instrumentation changes.
** 
** 6     8/28/98 1:32p Adrians
** Remove Validate* from this file. Now exists in d6mt.c
** 
** 5     8/05/98 4:36p Adrians
** Add DX6 Texture Stage State validation code.
** 
** 4     7/29/98 7:26p Adrians
** DX6 changes.
** 
** 2     5/18/98 1:43p Adrians
** Wbuffering now uses the full wbuffer range.
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

#if( DX >= 6 )

#include "precomp.h"

// Fix for building with Windows 98 DDK (Must include DDrawI first)
#include "ddrawi.h"
#include <d3dhal.h>
#include "fxglobal.h"
#include "d3global.h"
#include "d3contxt.h"
#include "d3txtr.h"

#include "dxins.h"

#endif
