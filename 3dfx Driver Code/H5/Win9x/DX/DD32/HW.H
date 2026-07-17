/*
** Copyright (c) 1997, 3Dfx Interactive, Inc.
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
** $Log: 
**  2    3dfx      1.0.1.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
** 
** 6     6/29/99 3:56p Edwin
** Remove obsolete Banshee ifdefs.
** 
** 4     4/09/99 12:38p Stb_bseitsin
** Added Napalm registers. Added ifdef H5.
** 
** 3     10/16/98 4:32p Artg
** change ifdef h3 to if defned(h3) || defined (H4)
** 
** 2     1/21/98 11:37a Suninn
** rid of all cmdfifo macros.  now they are in inc\fifomgr.h
 * 
 * 1     12/05/97 11:12p Ken
 * the real banshee build tree
 * 
 * 3     11/13/97 7:21p Miriam
 * Scan line interleave support.
 * 
 * 2     10/14/97 2:16p Suninn
 * prepare for h3
 * 
 * 1     10/14/97 11:23a Adrians
 * Needed for H3 build environment.
*/

//#ifndef __HW_H__
//#define __HW_H__

    #include "h3.h"
    #define FTOL( X ) ((FxU32)(X))

//#endif // __HW_H__
