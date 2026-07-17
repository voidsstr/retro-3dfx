#include <alloca.h>
#include <stdio.h>
#include <bstring.h>
#include "ogtst.h"

/*
 * de Casteljau for 1D.
 */
void
ogLibDeCasteljau1(float u, int order, int ncomp, float *pts, float *res)
{
    int i, j, k, n;
    float *s;

    s = (float*) alloca(order*sizeof(float)*ncomp);
    memcpy(s, pts, order*sizeof(float)*ncomp);
    
    for (i=order-1; i > 0; i--) {
	for (j=0; j < i; j++) {
	    n = j*ncomp;
	    for (k=0; k < ncomp; k++) {
		s[n+k] = s[n+k]*(1-u) + s[(n+ncomp)+k]*u;
	    }
	}
    }
    for (i=0; i < ncomp; i++) {
	res[i] = s[i];
    }
}

/*
 * de Casteljau for 2D, using the 1D routine above.
 */
void
ogLibDeCasteljau2(float u, int uorder, float v, int vorder, int ncomp,
		   float *pts, float *res)
{
    int i;
    float *s, *r;

    s = (float*) alloca(uorder * vorder * sizeof(float)*ncomp);
    memcpy(s, pts, uorder * vorder * sizeof(float)*ncomp);
    r = (float*) alloca(vorder * sizeof(float)*ncomp);
    
    for (i=0; i < vorder; i++) {
	ogLibDeCasteljau1(u, uorder, ncomp, &s[i*uorder*ncomp], &r[i*ncomp]);
    }
    ogLibDeCasteljau1(v, vorder, ncomp, r, res);
}

