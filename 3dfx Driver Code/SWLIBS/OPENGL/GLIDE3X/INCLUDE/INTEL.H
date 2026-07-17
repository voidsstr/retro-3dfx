#ifndef __INTEL_H__
#define __INTEL_H__

/*
** Copyright 1996,1997 Silicon Graphics, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of Silicon Graphics, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of Silicon Graphics, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
**
** $Revision: 2$
** $Date: 10/11/00 7:57:24 PM$
*/

/*
** __GLvertex offsets
*/
#define V_HAS      0x30
#define V_CLIP_X   0x20
#define V_CLIP_Y   0x24
#define V_CLIP_Z   0x28
#define V_CLIP_W   0x2C
#define V_CLIPCODE 0x30
#define V_WINDOW_X 0x38
#define V_WINDOW_Y 0x3C
#define V_WINDOW_Z 0x40
#define V_WINDOW_W 0x44

/*
** __GLviewport offsets
*/
#define VP_XSCALE  0x20
#define VP_YSCALE  0x28
#define VP_ZSCALE  0x30
#define VP_XCENTER 0x24
#define VP_YCENTER 0x2C
#define VP_ZCENTER 0x34

#endif
