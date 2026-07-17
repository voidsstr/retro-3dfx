/*
** Copyright (c) 1996-1999, 3Dfx Interactive, Inc.
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
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
**
** File name:   H3modeinit.h
**
** Description: h3 mode initialization code (derived from h3vdd in win9x)
**
**
*/

#ifndef __H3MODEINIT_H__
#define __H3MODEINIT_H__

#include <3dfx.h>
#include "macos8shim.h"

#include "h3g.h"
#include "devtable.h"

#define CRTC_TABLE_SIZE    24

extern FxU16 crtc_table[CRTC_TABLE_SIZE];

VOID ds_Calc_CRTC_table( VidProcConfig *pVpc, PDEVTABLE pDev );

#endif /* __H3MODEINIT_H__ */

