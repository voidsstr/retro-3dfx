/*
** Copyright 1991-1997, Silicon Graphics, Inc.
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
** $Date: 10/11/00 8:02:54 PM$
*/

#include "context.h"
#include "s3vconsts.h"

void
__glS3InitRGB(__GLcolorBuffer *cfb, __GLcontext *gc)
{
    __glInitRGB(cfb, gc);

    /* S3 accelerates only 16-bit 5550 */

    cfb->buf.elementSize = 2;	/* 16 bits */

    cfb->redMax = __GL_S3_RGB_COMPONENT_SCALE_RED;
    cfb->greenMax = __GL_S3_RGB_COMPONENT_SCALE_GREEN;
    cfb->blueMax = __GL_S3_RGB_COMPONENT_SCALE_BLUE;

    cfb->redScale = cfb->redMax;
    cfb->greenScale = cfb->greenMax;
    cfb->blueScale = cfb->blueMax;

    cfb->iRedScale = cfb->redScale;
    cfb->iGreenScale = cfb->greenScale;
    cfb->iBlueScale = cfb->blueScale;
}
