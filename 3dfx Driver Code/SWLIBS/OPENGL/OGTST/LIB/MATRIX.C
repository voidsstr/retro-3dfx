/**************************************************************************
 *									  *
 * 		 Copyright (C) 1990, Silicon Graphics, Inc.		  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *									  *
 **************************************************************************/

/* matrix.c - $Revision: 2$ */

#include "ogtst.h"

static void  ludcmp(float *a, GLint *);
static void  lubksb(float *a, GLint *, float *b);
static float abserr(float, float);
static float relerr(float, float);

static float etol = 0.0;	/* error tolerance for floats */

/*************************************************************
*  ogLibSetEtol()  -  set error tolerance 
*************************************************************/
void 
ogLibSetEtol(float r)
{
    etol = r;
}

/*************************************************************
*  ogLibGetEtol()  -  get error tolerance 
*************************************************************/
float 
ogLibGetEtol(void)
{
    return (etol);
}

/*************************************************************
*  ogLibPntMatrix(m)  - print matrix 
*************************************************************/
void 
ogLibPntMatrix(register float *m, register GLint level, register char *name)
{
    ogEnvLog(level, "  Printing %s Matrix\n", name);
    ogEnvLog(level, "  %14.7e  %14.7e  %14.7e  %14.7e\n", *(m + 0), *(m + 1), *(m + 2), *(m + 3));
    ogEnvLog(level, "  %14.7e  %14.7e  %14.7e  %14.7e\n", *(m + 4), *(m + 5), *(m + 6), *(m + 7));
    ogEnvLog(level, "  %14.7e  %14.7e  %14.7e  %14.7e\n", *(m + 8), *(m + 9), *(m + 10), *(m + 11));
    ogEnvLog(level, "  %14.7e  %14.7e  %14.7e  %14.7e\n", *(m + 12), *(m + 13), *(m + 14), *(m + 15));
}

/*************************************************************
*  ogLibUntMatrix(m)  - set the matrix to the identity matrix
*************************************************************/
void 
ogLibUntMatrix(register float *m)
{
    register float one = 1.0;
    register float zer = 0.0;

    *(m + 0) = one;
    *(m + 1) = zer;
    *(m + 2) = zer;
    *(m + 3) = zer;
    *(m + 4) = zer;
    *(m + 5) = one;
    *(m + 6) = zer;
    *(m + 7) = zer;
    *(m + 8) = zer;
    *(m + 9) = zer;
    *(m + 10) = one;
    *(m + 11) = zer;
    *(m + 12) = zer;
    *(m + 13) = zer;
    *(m + 14) = zer;
    *(m + 15) = one;
}

/*************************************************************
*  ogLibSetMatrix(m,c)  -  set matrix to constant
*************************************************************/
void 
ogLibSetMatrix(register float *m, register float c)
{
    register float ccc = c;

    *(m + 0) = ccc;
    *(m + 1) = ccc;
    *(m + 2) = ccc;
    *(m + 3) = ccc;
    *(m + 4) = ccc;
    *(m + 5) = ccc;
    *(m + 6) = ccc;
    *(m + 7) = ccc;
    *(m + 8) = ccc;
    *(m + 9) = ccc;
    *(m + 10) = ccc;
    *(m + 11) = ccc;
    *(m + 12) = ccc;
    *(m + 13) = ccc;
    *(m + 14) = ccc;
    *(m + 15) = ccc;
}

/*************************************************************
*  ogLibCpyMatrix(mo,m1,m2)  -  copy matrix
*************************************************************/
void 
ogLibCpyMatrix(register float *mo, register float *mi)
{
    *(mo + 0) = *(mi + 0);
    *(mo + 1) = *(mi + 1);
    *(mo + 2) = *(mi + 2);
    *(mo + 3) = *(mi + 3);
    *(mo + 4) = *(mi + 4);
    *(mo + 5) = *(mi + 5);
    *(mo + 6) = *(mi + 6);
    *(mo + 7) = *(mi + 7);
    *(mo + 8) = *(mi + 8);
    *(mo + 9) = *(mi + 9);
    *(mo + 10) = *(mi + 10);
    *(mo + 11) = *(mi + 11);
    *(mo + 12) = *(mi + 12);
    *(mo + 13) = *(mi + 13);
    *(mo + 14) = *(mi + 14);
    *(mo + 15) = *(mi + 15);
}

/*************************************************************
*  ogLibTnsMatrix(mo,mi)  -  transpose matrix
*************************************************************/
void 
ogLibTnsMatrix(register float *mo, register float *mi)
{
    if (mo != mi) {
	*(mo + 0) = *(mi + 0);
	*(mo + 1) = *(mi + 4);
	*(mo + 2) = *(mi + 8);
	*(mo + 3) = *(mi + 12);
	*(mo + 4) = *(mi + 1);
	*(mo + 5) = *(mi + 5);
	*(mo + 6) = *(mi + 9);
	*(mo + 7) = *(mi + 13);
	*(mo + 8) = *(mi + 2);
	*(mo + 9) = *(mi + 6);
	*(mo + 10) = *(mi + 10);
	*(mo + 11) = *(mi + 14);
	*(mo + 12) = *(mi + 3);
	*(mo + 13) = *(mi + 7);
	*(mo + 14) = *(mi + 11);
	*(mo + 15) = *(mi + 15);
    } else {
	float m[16];
	ogLibCpyMatrix(m, mi);
	ogLibTnsMatrix(mo, m);
    }
}

/*************************************************************
*  ogLibAbsMatrix(mo,mi)  -  absolute value of matrix
*************************************************************/
void 
ogLibAbsMatrix(register float *mo, register float *mi)
{
    *(mo + 0) = ((*(mi + 0) < 0.0) ? -*(mi + 0) : *(mi + 0));
    *(mo + 1) = ((*(mi + 1) < 0.0) ? -*(mi + 1) : *(mi + 1));
    *(mo + 2) = ((*(mi + 2) < 0.0) ? -*(mi + 2) : *(mi + 2));
    *(mo + 3) = ((*(mi + 3) < 0.0) ? -*(mi + 3) : *(mi + 3));
    *(mo + 4) = ((*(mi + 4) < 0.0) ? -*(mi + 4) : *(mi + 4));
    *(mo + 5) = ((*(mi + 5) < 0.0) ? -*(mi + 5) : *(mi + 5));
    *(mo + 6) = ((*(mi + 6) < 0.0) ? -*(mi + 6) : *(mi + 6));
    *(mo + 7) = ((*(mi + 7) < 0.0) ? -*(mi + 7) : *(mi + 7));
    *(mo + 8) = ((*(mi + 8) < 0.0) ? -*(mi + 8) : *(mi + 8));
    *(mo + 9) = ((*(mi + 9) < 0.0) ? -*(mi + 9) : *(mi + 9));
    *(mo + 10) = ((*(mi + 10) < 0.0) ? -*(mi + 10) : *(mi + 10));
    *(mo + 11) = ((*(mi + 11) < 0.0) ? -*(mi + 11) : *(mi + 11));
    *(mo + 12) = ((*(mi + 12) < 0.0) ? -*(mi + 12) : *(mi + 12));
    *(mo + 13) = ((*(mi + 13) < 0.0) ? -*(mi + 13) : *(mi + 13));
    *(mo + 14) = ((*(mi + 14) < 0.0) ? -*(mi + 14) : *(mi + 14));
    *(mo + 15) = ((*(mi + 15) < 0.0) ? -*(mi + 15) : *(mi + 15));
}

/*************************************************************
*  ogLibAdsMatrix(mo,mi,s)  -  add constant to matrix
*************************************************************/
void 
ogLibAdsMatrix(register float *mo, register float *mi, register float s)
{
    *(mo + 0) = *(mi + 0) + s;
    *(mo + 1) = *(mi + 1) + s;
    *(mo + 2) = *(mi + 2) + s;
    *(mo + 3) = *(mi + 3) + s;
    *(mo + 4) = *(mi + 4) + s;
    *(mo + 5) = *(mi + 5) + s;
    *(mo + 6) = *(mi + 6) + s;
    *(mo + 7) = *(mi + 7) + s;
    *(mo + 8) = *(mi + 8) + s;
    *(mo + 9) = *(mi + 9) + s;
    *(mo + 10) = *(mi + 10) + s;
    *(mo + 11) = *(mi + 11) + s;
    *(mo + 12) = *(mi + 12) + s;
    *(mo + 13) = *(mi + 13) + s;
    *(mo + 14) = *(mi + 14) + s;
    *(mo + 15) = *(mi + 15) + s;
}

/*************************************************************
*  ogLibAddMatrix(mo,m1,m2)  -  add matrices
*************************************************************/
void 
ogLibAddMatrix(register float *mo, register float *m1, register float *m2)
{
    *(mo + 0) = *(m1 + 0) + *(m2 + 0);
    *(mo + 1) = *(m1 + 1) + *(m2 + 1);
    *(mo + 2) = *(m1 + 2) + *(m2 + 2);
    *(mo + 3) = *(m1 + 3) + *(m2 + 3);
    *(mo + 4) = *(m1 + 4) + *(m2 + 4);
    *(mo + 5) = *(m1 + 5) + *(m2 + 5);
    *(mo + 6) = *(m1 + 6) + *(m2 + 6);
    *(mo + 7) = *(m1 + 7) + *(m2 + 7);
    *(mo + 8) = *(m1 + 8) + *(m2 + 8);
    *(mo + 9) = *(m1 + 9) + *(m2 + 9);
    *(mo + 10) = *(m1 + 10) + *(m2 + 10);
    *(mo + 11) = *(m1 + 11) + *(m2 + 11);
    *(mo + 12) = *(m1 + 12) + *(m2 + 12);
    *(mo + 13) = *(m1 + 13) + *(m2 + 13);
    *(mo + 14) = *(m1 + 14) + *(m2 + 14);
    *(mo + 15) = *(m1 + 15) + *(m2 + 15);
}

/*************************************************************
*  SubMatrix(mo,m1,m2)  -  subtract matrices
*************************************************************/
void 
ogLibSubMatrix(register float *mo, register float *m1, register float *m2)
{
    *(mo + 0) = *(m1 + 0) - *(m2 + 0);
    *(mo + 1) = *(m1 + 1) - *(m2 + 1);
    *(mo + 2) = *(m1 + 2) - *(m2 + 2);
    *(mo + 3) = *(m1 + 3) - *(m2 + 3);
    *(mo + 4) = *(m1 + 4) - *(m2 + 4);
    *(mo + 5) = *(m1 + 5) - *(m2 + 5);
    *(mo + 6) = *(m1 + 6) - *(m2 + 6);
    *(mo + 7) = *(m1 + 7) - *(m2 + 7);
    *(mo + 8) = *(m1 + 8) - *(m2 + 8);
    *(mo + 9) = *(m1 + 9) - *(m2 + 9);
    *(mo + 10) = *(m1 + 10) - *(m2 + 10);
    *(mo + 11) = *(m1 + 11) - *(m2 + 11);
    *(mo + 12) = *(m1 + 12) - *(m2 + 12);
    *(mo + 13) = *(m1 + 13) - *(m2 + 13);
    *(mo + 14) = *(m1 + 14) - *(m2 + 14);
    *(mo + 15) = *(m1 + 15) - *(m2 + 15);
}

/*************************************************************
*  ogLibMusMatrix(mo,mi,s)  -  multiply matrix by constant
*************************************************************/
void 
ogLibMusMatrix(register float *mo, register float *mi, register float s)
{
    *(mo + 0) = *(mi + 0) * s;
    *(mo + 1) = *(mi + 1) * s;
    *(mo + 2) = *(mi + 2) * s;
    *(mo + 3) = *(mi + 3) * s;
    *(mo + 4) = *(mi + 4) * s;
    *(mo + 5) = *(mi + 5) * s;
    *(mo + 6) = *(mi + 6) * s;
    *(mo + 7) = *(mi + 7) * s;
    *(mo + 8) = *(mi + 8) * s;
    *(mo + 9) = *(mi + 9) * s;
    *(mo + 10) = *(mi + 10) * s;
    *(mo + 11) = *(mi + 11) * s;
    *(mo + 12) = *(mi + 12) * s;
    *(mo + 13) = *(mi + 13) * s;
    *(mo + 14) = *(mi + 14) * s;
    *(mo + 15) = *(mi + 15) * s;
}

/*************************************************************
*  ogLibMulMatrix(mo,m1,m2)  -  multiply matrices
*************************************************************/
void 
ogLibMulMatrix(register float *mo, register float *m1, register float *m2)
{
    if (mo != m1 && mo != m2) {
	*(mo + 0) = *(m1 + 0) * *(m2 + 0) + *(m1 + 1) * *(m2 + 4) + *(m1 + 2) * *(m2 + 8) + *(m1 + 3) * *(m2 + 12);
	*(mo + 1) = *(m1 + 0) * *(m2 + 1) + *(m1 + 1) * *(m2 + 5) + *(m1 + 2) * *(m2 + 9) + *(m1 + 3) * *(m2 + 13);
	*(mo + 2) = *(m1 + 0) * *(m2 + 2) + *(m1 + 1) * *(m2 + 6) + *(m1 + 2) * *(m2 + 10) + *(m1 + 3) * *(m2 + 14);
	*(mo + 3) = *(m1 + 0) * *(m2 + 3) + *(m1 + 1) * *(m2 + 7) + *(m1 + 2) * *(m2 + 11) + *(m1 + 3) * *(m2 + 15);
	*(mo + 4) = *(m1 + 4) * *(m2 + 0) + *(m1 + 5) * *(m2 + 4) + *(m1 + 6) * *(m2 + 8) + *(m1 + 7) * *(m2 + 12);
	*(mo + 5) = *(m1 + 4) * *(m2 + 1) + *(m1 + 5) * *(m2 + 5) + *(m1 + 6) * *(m2 + 9) + *(m1 + 7) * *(m2 + 13);
	*(mo + 6) = *(m1 + 4) * *(m2 + 2) + *(m1 + 5) * *(m2 + 6) + *(m1 + 6) * *(m2 + 10) + *(m1 + 7) * *(m2 + 14);
	*(mo + 7) = *(m1 + 4) * *(m2 + 3) + *(m1 + 5) * *(m2 + 7) + *(m1 + 6) * *(m2 + 11) + *(m1 + 7) * *(m2 + 15);
	*(mo + 8) = *(m1 + 8) * *(m2 + 0) + *(m1 + 9) * *(m2 + 4) + *(m1 + 10) * *(m2 + 8) + *(m1 + 11) * *(m2 + 12);
	*(mo + 9) = *(m1 + 8) * *(m2 + 1) + *(m1 + 9) * *(m2 + 5) + *(m1 + 10) * *(m2 + 9) + *(m1 + 11) * *(m2 + 13);
	*(mo + 10) = *(m1 + 8) * *(m2 + 2) + *(m1 + 9) * *(m2 + 6) + *(m1 + 10) * *(m2 + 10) + *(m1 + 11) * *(m2 + 14);
	*(mo + 11) = *(m1 + 8) * *(m2 + 3) + *(m1 + 9) * *(m2 + 7) + *(m1 + 10) * *(m2 + 11) + *(m1 + 11) * *(m2 + 15);
	*(mo + 12) = *(m1 + 12) * *(m2 + 0) + *(m1 + 13) * *(m2 + 4) + *(m1 + 14) * *(m2 + 8) + *(m1 + 15) * *(m2 + 12);
	*(mo + 13) = *(m1 + 12) * *(m2 + 1) + *(m1 + 13) * *(m2 + 5) + *(m1 + 14) * *(m2 + 9) + *(m1 + 15) * *(m2 + 13);
	*(mo + 14) = *(m1 + 12) * *(m2 + 2) + *(m1 + 13) * *(m2 + 6) + *(m1 + 14) * *(m2 + 10) + *(m1 + 15) * *(m2 + 14);
	*(mo + 15) = *(m1 + 12) * *(m2 + 3) + *(m1 + 13) * *(m2 + 7) + *(m1 + 14) * *(m2 + 11) + *(m1 + 15) * *(m2 + 15);
    } else {
	float m[16];
	ogLibMulMatrix(m, m1, m2);
	ogLibCpyMatrix(mo, m);
    }
}

/*************************************************************
*  ogLibVecMatrix(vo,vi,m)  -  multiply vector by matrix
*************************************************************/
void 
ogLibVecMatrix(register float *vo, register float *vi, register float *m)
{
    register float ox, oy, oz, ow;

    ox = vi[0];
    oy = vi[1];
    oz = vi[2];
    ow = vi[3];

    vo[0] = *(m + 0) * ox + *(m + 4) * oy + *(m + 8) * oz + *(m + 12) * ow;
    vo[1] = *(m + 1) * ox + *(m + 5) * oy + *(m + 9) * oz + *(m + 13) * ow;
    vo[2] = *(m + 2) * ox + *(m + 6) * oy + *(m + 10) * oz + *(m + 14) * ow;
    vo[3] = *(m + 3) * ox + *(m + 7) * oy + *(m + 11) * oz + *(m + 15) * ow;
}

/*************************************************************
*  ogLibInvMatrix(mo,mi)  -  calculate inverse matrix by LU decomp
*                       as described in Numerical Recipes p.38
*************************************************************/
void 
ogLibInvMatrix(register float *mo, register float *mi)
{
    float d, a[16];
    GLint index[4];

    d = ogLibDetMatrix(mi);

    if (d > -0.001 && d < 0.001)
	ogEnvLog(0, "  InvMatrix(): Matrix probably singular\n");

    ogLibCpyMatrix(a, mi);

    ludcmp(a, index);
    lubksb(a, index, mo);
}

/*************************************************************
*  ogLibDetMatrix(m)  -  calculate matrix determinant
*************************************************************/
float 
ogLibDetMatrix(register float *m)
{
    register float a, b, c, d;
             float k[6];

    k[0] = *(m + 10) * *(m + 15) - *(m + 14) * *(m + 11);
    k[1] = *(m + 9) * *(m + 15) - *(m + 13) * *(m + 11);
    k[2] = *(m + 9) * *(m + 14) - *(m + 13) * *(m + 10);
    k[3] = *(m + 8) * *(m + 15) - *(m + 12) * *(m + 11);
    k[4] = *(m + 8) * *(m + 14) - *(m + 12) * *(m + 10);
    k[5] = *(m + 8) * *(m + 13) - *(m + 12) * *(m + 9);

    a = *(m + 5) * k[0] - *(m + 6) * k[1] + *(m + 7) * k[2];
    b = *(m + 4) * k[0] - *(m + 6) * k[3] + *(m + 7) * k[4];
    c = *(m + 4) * k[1] - *(m + 5) * k[3] + *(m + 7) * k[5];
    d = *(m + 4) * k[2] - *(m + 5) * k[4] + *(m + 6) * k[5];

    return (*(m + 0) * a - *(m + 1) * b + *(m + 2) * c - *(m + 3) * d);
}

/*************************************************************
*  ludcmp()  - 
*************************************************************/
static void 
ludcmp(float *a, GLint *indx)
{
    register float sum, dum, big;
             float vv[4];
    register GLint j, k, imax, i;

    for (i = 0; i < 4; i++) {
	for (big = 0.0, j = 0; j < 4; j++) {
	    dum = ((a[4 * i + j] < 0.0) ? -a[4 * i + j] : a[4 * i + j]);
	    big = ((dum > big) ? dum : big);
	}

	if (big == 0.0)
	    ogEnvLog(0, "Singular Matrix\n");

	vv[i] = 1.0 / big;
    }

    for (j = 0; j < 4; j++) {
	for (i = 0; i < j; i++) {
	    sum = a[4 * i + j];

	    for (k = 0; k < i; k++)
		sum = sum - a[4 * i + k] * a[4 * k + j];

	    a[4 * i + j] = sum;
	}

	for (big = 0.0, i = j; i < 4; i++) {
	    sum = a[4 * i + j];

	    for (k = 0; k < j; k++)
		sum = sum - a[4 * i + k] * a[4 * k + j];

	    a[4 * i + j] = sum;

	    dum = vv[i] * ((sum < 0.0) ? -sum : sum);

	    if (dum > big) {
		big = dum;
		imax = i;
	    }
	}

	if (j != imax) {
	    for (k = 0; k < 4; k++) {
		dum = a[4 * imax + k];
		a[4 * imax + k] = a[4 * j + k];
		a[4 * j + k] = dum;
	    }
	    vv[imax] = vv[j];
	}
	indx[j] = imax;

	if (a[4 * j + j] == 0.0)
	    a[4 * j + j] = 1e-20;

	if (j != 3) {
	    dum = 1.0 / a[4 * j + j];

	    for (i = j + 1; i < 4; i++)
		a[4 * i + j] *= dum;
	}
    }
}

/*************************************************************
*  lubksb()  - 
*************************************************************/
static void 
lubksb(float *a, GLint *indx, float *m)
{
    register GLint i, j, k, ii, ip;
    register float sum;
             float b[4];

    ogLibUntMatrix(m);

    for (k = 0; k < 4; k++) {
	b[0] = m[0 + k];
	b[1] = m[4 + k];
	b[2] = m[8 + k];
	b[3] = m[12 + k];
	ii = -1;

	for (i = 0; i < 4; i++) {
	    ip = indx[i];
	    sum = b[ip];
	    b[ip] = b[i];

	    if (ii != -1) {
		for (j = ii; j < i; j++)
		    sum = sum - a[4 * i + j] * b[j];
	    } else if (sum != 0.0)
		ii = i;

	    b[i] = sum;
	}

	for (i = 3; i >= 0; i--) {
	    sum = b[i];

	    if (i < 3)
		for (j = i + 1; j < 4; j++)
		    sum = sum - a[4 * i + j] * b[j];

	    b[i] = sum / a[4 * i + i];
	}

	m[0 + k] = b[0];
	m[4 + k] = b[1];
	m[8 + k] = b[2];
	m[12 + k] = b[3];
    }
}

/*************************************************************
*  ogLibRcmMatrix()  -  compare matrices using relative error
*************************************************************/
void 
ogLibRcmMatrix(register float *m1, register float *m2)
{
    float maxerr, em[16];
    int i;

    /* compute max relative error for all elements */
    for (maxerr = 0.0, i = 0; i < 16; i++) {
	if (*(m1 + i) < 1e-6 && *(m2 + i) < 1e-6)
	    *(em + i) = 0.0;
	else
	    *(em + i) = relerr(*(m1 + i), *(m2 + i));

	if (*(em + i) > maxerr)
	    maxerr = *(em + i);
    }

    if (maxerr > etol) {
	ogEnvLog(-1, "matrix compare maximum relative error is %g\n", maxerr);
	ogLibPntMatrix(m2, 0, "Matrix is");
	ogLibPntMatrix(m1, 0, "Should be");
	ogLibPntMatrix(em, 0, "Relative error");
    }
}

/*************************************************************
*  ogLibAcmMatrix()  -  compare matrices using absolute error
*************************************************************/
void 
ogLibAcmMatrix(register float *m1, register float *m2)
{
    float maxerr, em[16];
    int i;

    ogLibSubMatrix(em, m1, m2);
    ogLibAbsMatrix(em, em);

    /* compute max absolute error for all element */
    for (maxerr = 0.0, i = 0; i < 16; i++)
	if (*(em + i) > maxerr)
	    maxerr = *(em + i);

    if (maxerr > etol) {
	ogEnvLog(-1, "matrix compare maximum absolute error is %g\n", maxerr);
	ogLibPntMatrix(m2, 0, "Matrix is");
	ogLibPntMatrix(m1, 0, "Should be");
	ogLibPntMatrix(em, 0, "Absolute error");
    }
}

/*************************************************************
*  ogLibRcmVector()  -  compare vectors using relative error
*************************************************************/
void 
ogLibRcmVector(register float *v1, register float *v2)
{
    int j;
    float maxerr, er[4];

    /* compute relative error for each element */
    for (maxerr = 0.0, j = 0; j < 4; j++) {
	er[j] = relerr(v1[j], v2[j]);
	if (er[j] > maxerr)
	    maxerr = er[j];
    }

    if (maxerr > etol) {
	ogEnvLog(-1, "vector compare maximum error is %g\n", maxerr);
	ogEnvLog(0, "  vector is: %g  %g  %g  %g\n", v1[0], v1[1], v1[2], v1[3]);
	ogEnvLog(0, "  should be: %g  %g  %g  %g\n", v2[0], v2[1], v2[2], v2[3]);
	ogEnvLog(0, "  rel error: %g  %g  %g  %g\n", er[0], er[1], er[2], er[3]);
    }
}

/*************************************************************
*  ogLibAcmVector()  -  compare vectors using absolute error
*************************************************************/
void 
ogLibAcmVector(register float *v1, register float *v2)
{
    int j;
    float maxerr, er[4];

    /* compute relative error for each element */
    for (maxerr = 0.0, j = 0; j < 4; j++) {
	er[j] = abserr(v1[j], v2[j]);
	if (er[j] > maxerr)
	    maxerr = er[j];
    }

    if (maxerr > etol) {
	ogEnvLog(-1, "vector compare maximum error is %g\n", maxerr);
	ogEnvLog(0, "  vector is: %g  %g  %g  %g\n", v1[0], v1[1], v1[2], v1[3]);
	ogEnvLog(0, "  should be: %g  %g  %g  %g\n", v2[0], v2[1], v2[2], v2[3]);
	ogEnvLog(0, "  abs error: %g  %g  %g  %g\n", er[0], er[1], er[2], er[3]);
    }
}

/*************************************************************
*  abserr()  -  absolute error 
*************************************************************/
static float 
abserr(register float a, register float b)
{
    register float s;

    s = a - b;
    return ((s < 0.0) ? -s : s);
}

/*************************************************************
*  relerr()  -  relative error 
*************************************************************/
static float 
relerr(register float a, register float b)
{
    register float aa, ab, s;

    aa = ((a < 0.0) ? -a : a);
    ab = ((b < 0.0) ? -b : b);

    if (a == b)
	return (0.0);
    else if (a == 0.0)
	return (ab);
    else if (b == 0.0)
	return (aa);
    else {
	s = a - b;
	return (((s < 0.0) ? -s : s) / (aa + ab));
    }
}

/*************************************************************
*  ogLibFCompare()  -  compare 2 floats
*************************************************************/
void 
ogLibFCompare(char *msg, float ans, float pst)
{
    register float err;

    err = relerr(ans, pst);

    if (err > etol)
	ogEnvLog(-1, "%s should be %g but is %g, relative error is %g\n", msg, ans, pst, err);
}
