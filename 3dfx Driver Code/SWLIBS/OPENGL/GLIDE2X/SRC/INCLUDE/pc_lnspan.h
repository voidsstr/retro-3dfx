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
** $Revision: 4$
** $Date: 10/11/00 7:52:26 PM$
*/
#include "fmacros.h"
extern __GLfloat __glFastDitherTable[16];
extern FixedT __glFixedDitherTable[16];

extern GLboolean __glPCPickLineProcs(__GLcontext *gc);

extern GLboolean __glSlowDrawBothLine(__GLcontext *gc);
extern GLboolean __glProcessLine2NW(__GLcontext *gc);

extern GLboolean __glDrawLine_CI8_Flat(__GLcontext *gc);
extern GLboolean __glDrawLine_CI8_Flat_Dither(__GLcontext *gc);
extern GLboolean __glDrawLine_CI8_Smooth(__GLcontext *gc);
extern GLboolean __glDrawLine_CI8_Smooth_Dither(__GLcontext *gc);
extern GLboolean __glDrawStippledLine_CI8_Flat(__GLcontext *gc);
extern GLboolean __glDrawStippledLine_CI8_Flat_Dither(__GLcontext *gc);
extern GLboolean __glDrawStippledLine_CI8_Smooth(__GLcontext *gc);
extern GLboolean __glDrawStippledLine_CI8_Smooth_Dither(__GLcontext *gc);

extern GLboolean __glDrawLine_RGB16_Flat(__GLcontext *gc);
extern GLboolean __glDrawLine_RGB16_Flat_Dither(__GLcontext *gc);
extern GLboolean __glDrawLine_RGB16_Smooth(__GLcontext *gc);
extern GLboolean __glDrawLine_RGB16_Smooth_Dither(__GLcontext *gc);
extern GLboolean __glDrawStippledLine_RGB16_Flat(__GLcontext *gc);
extern GLboolean __glDrawStippledLine_RGB16_Flat_Dither(__GLcontext *gc);
extern GLboolean __glDrawStippledLine_RGB16_Smooth(__GLcontext *gc);
extern GLboolean __glDrawStippledLine_RGB16_Smooth_Dither(__GLcontext *gc);

extern GLboolean __glDrawAALine_RGB_16_Flat_LESS16_SA_MSA(__GLcontext *gc);
extern GLboolean __glDrawAAStippledLine_RGB_16_Flat_LESS16_SA_MSA(__GLcontext *gc);
extern GLboolean __glDrawAALine_RGB_16_Flat_LESS32_SA_MSA(__GLcontext *gc);
extern GLboolean __glDrawAAStippledLine_RGB_16_Flat_LESS32_SA_MSA(__GLcontext *gc);

extern GLboolean __glDrawAALine_RGB_16_Flat_SA_ONE(__GLcontext *gc);
extern GLboolean __glDrawAAStippledLine_RGB_16_Flat_SA_ONE(__GLcontext *gc);

extern GLboolean __glDrawAALine_RGB_16_Smooth_SA_ONE(__GLcontext *gc);
extern GLboolean __glDrawAAStippledLine_RGB_16_Smooth_SA_ONE(__GLcontext *gc);
