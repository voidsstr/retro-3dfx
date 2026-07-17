/* -*-c++-*- */
/* $Header: dfpdisp.h, 4, 10/11/00 8:55:24 PM, Brent$ */
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
** File name:   dfpdisp.h
**
** Description: Structures for digital flat panels.
**
** $Revision: 4$
** $Date: 10/11/00 8:55:24 PM$
**
** $History: dfp.h $
** 
** 
*/
#ifndef __DFPDISP_H__
#define __DFPDISP_H__


#include "dfpctrl.h"

#define MAX_DFP_MODES 6



typedef struct
{
   FxU16 width;
   FxU16 height;
   FxU8  refresh;
}  DFP_MODE;

typedef struct
{
	FxU16    htotal;     // hblank + dispWidth
	FxU16    dispWidth;
	FxU16    hsyncWidth; //
	FxU16    hsyncOfst;
	FxU16    vtotal;     // vblank + dispHeight
	FxU16    dispHeight;
	FxU8     vsyncWidth;  //
	FxU8     vsyncOfst;
	FxU32    dispClock;  // 65mhz = 65000 = 0xfde8
    DFP_MODE stdModes[MAX_DFP_MODES];
    FxU32    FpFlags;
	char     uniqueName[UNIQUENAMELEN];

}  DFP_DISPLAY;



#endif
