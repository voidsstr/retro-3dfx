#if 0 /* Perl cannot handle C-style comments */
/*
** Copyright 1996, 1997, Silicon Graphics, Inc.
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
*/
#endif
FR_ZTEST_PROTOTYPE
{
#if FR_ZTEST == NEVER
#if FR_ZSTENCIL
    __GLcontext *gc = tr->gc;
    /* Stencil test is enabled, so continue to the next stage regardless
     * of mask.
     */
    (*gc->procs.afterDepthTest)(0, tr);
#endif
#elif ((FR_ZTEST == ALWAYS) && !FR_ZWRITE)
    __GLcontext *gc = tr->gc;
    /* We should never get here if PickTriangleProcs is smart */
    (*gc->procs.afterDepthTest)(mask, tr);
#else
    __GLcontext *gc = tr->gc;
    int z = tr->z;
    FR_ZTYPE *zp = (FR_ZTYPE *) tr->zp;
#if FR_ZTEST == ALWAYS
    GLbitfield mask_out = mask;
#else
    FR_ZTYPE fz;
    GLbitfield mask_out = ~mask;
    unsigned int bit = HIBIT;
#endif

    while (1) {
	while (((int)mask) < 0) {
#if FR_ZTEST == ALWAYS
#if FR_ZSIZE == 2 /* bytes */
	    *zp = TruncFixed(z, 16);
#else
	    *zp = z;
#endif
#else
#if FR_ZSIZE == 2 /* bytes */
	    fz = TruncFixed(z, 16);
#else
	    fz = z;
#endif
	    if (fz FR_ZTEST_OP *zp) {
#if FR_ZWRITE
		*zp = fz;
#endif
	    } else {
		mask_out |= bit;
	    }
	    bit >>= 1;
#endif
	    mask <<= 1;
	    zp += tr->dx;
	    z += tr->dzdx;
	}

	if (mask == 0) break;

	/* step interpolants */
	mask <<= 1;
	zp += tr->dx;
	z += tr->dzdx;
#if FR_ZTEST != ALWAYS
	bit >>= 1;
#endif
    }

#if FR_ZSTENCIL
    /* Stencil test is enabled, so continue to the next stage regardless
     * of mask.
     */
#if FR_ZTEST == ALWAYS
    (*gc->procs.afterDepthTest)(mask_out, tr);
#else
    (*gc->procs.afterDepthTest)(~mask_out, tr);
#endif /* FR_ZTEST == ALWAYS */
#else /* FR_ZSTENCIL */
#if FR_ZTEST == ALWAYS
    if (mask_out)
	(*gc->procs.afterDepthTest)(mask_out, tr);
#else
    if (~mask_out)
	(*gc->procs.afterDepthTest)(~mask_out, tr);
#endif /* FR_ZTEST == ALWAYS */
#endif /* FR_ZSTENCIL */
#endif /* FR_ZTEST == NEVER */
}
