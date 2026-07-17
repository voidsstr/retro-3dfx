/**************************************************************************
 *									  *
 * 		 Copyright (C) 1989, Silicon Graphics, Inc.		  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *									  *
 **************************************************************************/

/* rand.c - $Revision: 2$ */

#include "ogtst.h"
#include <stdlib.h>

static GLint seed = 1;
static void swap(int *p, int *q);

/*************************************************************
*  ogLibGetSeed() - get current seed
*************************************************************/
GLint 
ogLibGetSeed(void)
{
    return(seed);
}

/*************************************************************
*  ogLibSetSeed() - set current seed
*************************************************************/
void 
ogLibSetSeed(int s)
{
    seed = s;
}

/*************************************************************
*  ogLibBitRand() - integer 'n' bit random number
*************************************************************/
GLint 
ogLibBitRand(int n)
{
	GLint rslt;

    if (n <= 0 || n > 32) {
      ogEnvLog(OG_LINTERNALERROR,"ogLibBitRand(%d): invalid arg!\n",n); 
      /*NOTREACHED*/
      rslt = 0;
      }
    else if (n < 32) {
	seed = seed * 1103515245 + 12345;
	/*
	 * return the most-significant n bits; they are the random ones (see
	 * Knuth, Vol 2) 
	 */
	rslt = (seed & 0x7fffffff) >> (31 - n);
        }
    else rslt = (ogLibBitRand(16)<<16) | ogLibBitRand(16);
	ogEnvLog(25, "ogLibBitRand returns %d\n", rslt);
	return rslt;
}

/*************************************************************
*  ogLibIntRand() - integer in range a -> b random number
*************************************************************/
GLint 
ogLibIntRand(GLint a, GLint b)
{
    GLint r,sign=1;
    int mySmall,delta,bits = 0;
	GLint rslt;

    if (a > b) {
      mySmall = b;
      delta = a - b;
      }
    else {
      mySmall = a;
      delta = b - a;
      }

    if (delta == 0) {
        rslt = a;
		ogEnvLog(25, "ogLibIntRand returns %d \n", rslt);
        return(rslt);
	}
    else if (delta < 0) {
       sign = -1;
       delta = -delta;
       }

    delta &= 0x7fffffff;
    for (r = delta; r > 0; r >>= 1)
	bits++;

    do r = ogLibBitRand(bits);
    while (r > delta);

    rslt = mySmall + r*sign;
	ogEnvLog(25, "ogLibIntRand returns %d \n", rslt);
    return(rslt);
}

/*************************************************************
*  ogLibExpRand() - float random number
*************************************************************/
float 
ogLibExpRand(int exp)
{
    float tmp;
	float rslt;

    static float etable[73] = {
        1e-36, 1e-35, 1e-34, 1e-33, 1e-32, 1e-31, 1e-30, 1e-29, 1e-28, 1e-27,
        1e-26, 1e-25, 1e-24, 1e-23, 1e-22, 1e-21, 1e-20, 1e-19, 1e-18, 1e-17,
        1e-16, 1e-15, 1e-14, 1e-13, 1e-12, 1e-11, 1e-10,1e-9, 1e-8, 1e-7,
        1e-6, 1e-5, 1e-4, 1e-3, 1e-2, 1e-1, 1e0,  1e1,  1e2,  1e3,  1e4,
        1e5,  1e6,  1e7,  1e8,  1e9, 1e10, 1e11, 1e12, 1e13, 1e14, 1e15,
        1e16, 1e17, 1e18, 1e19, 1e20, 1e21, 1e22, 1e23, 1e24, 1e25, 1e26,
        1e27, 1e28, 1e29, 1e30, 1e31, 1e32, 1e33, 1e34, 1e35, 1e36,
    };

    /* DON'T chose the power of 10 randomly */
    if (-37 < exp && exp < 37)  {
        tmp = (float) ogLibBitRand(31) / (float) 0x7fffffff;
        rslt = (ogLibBitRand(1) ? tmp : -tmp) * etable[36 + exp];
        ogEnvLog(25, "ogLibExpRand returns %f \n", rslt);
        return rslt; 
    }
    else
        ogEnvLog(OG_LINTERNALERROR,"ogLibExpRand(%f): invalid arg!\n",exp);
    return 0;                   /* Damn compiler lint */
}

/*************************************************************
*  ogLibFloatRand() - float in range (a,b) random number
*************************************************************/
float 
ogLibFloatRand(float a, float b)
{
	float rslt;

    if (a > b)
		rslt = (b + (a - b) * ((float) ogLibBitRand(31) / (float) 0x7fffffff));
    else
	 	rslt = (a + (b - a) * ((float) ogLibBitRand(31) / (float) 0x7fffffff));
	ogEnvLog(25, "ogLibFloatRand returns %f\n", rslt);
	return(rslt);
}

/******************************************************************
* ogLibDlistRand() - return 0 (don't display list) or 
* 1 (compile) or 2 (compile and execute). Has 4 "modern" modes and 2
* which are included for backward compatability:
* random (the default -- switches between any of the three)
* immediate (never display list)
* compile (always compile, then execute)
* compile and execute (always compile and execute)
* Archaic modes:
* yes (random between compile and (compile and execute)) 
* no (same as immediate)
******************************************************************/

GLint
ogLibDlistRand(void)
{
  char *dlistenv;
  GLint rslt;

  /* controls whether ogtst dlists are always on, always off, or random */

  dlistenv = getenv("OGTST_DLISTS");
  ogEnvLog(25, "getenv(OGTST_DLISTS) in ogLibDlistRand returns %s\n", dlistenv);

  if(dlistenv) { /* if environment is defined */

    if((dlistenv[0] == 'y') || (dlistenv[0] == 'Y')) {
      rslt = 1+ogLibBitRand(1); /* [Yy]es: dlists should always be on */
	  ogEnvLog(25, "ogLibDlistRand returns %d\n", rslt);
	  return(rslt);
	}

    if(!strncmp(dlistenv,"on",2) || 
       !strncmp(dlistenv,"On",2) ||
       !strncmp(dlistenv,"ON",2)) {
      rslt = 1+ogLibBitRand(1); /* dlists should always be "on" */
	  ogEnvLog(25, "ogLibDlistRand returns %d\n", rslt);
	  return(rslt);
	}
    
    if((dlistenv[0] == 'n') || (dlistenv[0] == 'N') ||
       !strncmp(dlistenv,"of",2) || !strncmp(dlistenv,"Of",2) ||
       !strncmp(dlistenv,"OF",2)) {
        /* dlists should never be on */
        /* Make the random call to keep the random sequence as if
           display lists were fully enabled */
        (void) ogLibIntRand(0, 2);  
	    ogEnvLog(25, "ogLibDlistRand returns %d\n", 0);
        return 0;
    }
    if(!strncmp(dlistenv, "IMMEDIATE", 9)) {
        (void) ogLibIntRand(0, 2);  
	    ogEnvLog(25, "ogLibDlistRand returns %d\n", 0);
        return 0; /* no dlists */
    }
    if(!strncmp(dlistenv, "COMPILE_AND_EXECUTE", 19)) {
        (void) ogLibIntRand(0, 2);  
	    ogEnvLog(25, "ogLibDlistRand returns %d\n", 2);
        return 2; /* dlists should always be "compile and execute" */
    }
    if(!strncmp(dlistenv, "COMPILE", 7)) {
        (void) ogLibIntRand(0, 2);  
	    ogEnvLog(25, "ogLibDlistRand returns %d\n", 1);
        return 1; /* dlists should always be "compile" */
    }
  }
  rslt = ogLibIntRand(0, 2); /* default behavior: dlists should be random */
  ogEnvLog(25, "ogLibDlistRand returns %d\n", rslt);
  return(rslt); 
}

/************************************************
 * swap() - a simple functions to swap two values
 ***********************************************/
void
swap(int *p, int *q)
{
    int tmp;

    tmp = *p;
    *p = *q;
    *q = tmp;

}

/*************************************************************************
 * ogLibOrderRand() - shuffle a list of integers and return a pointer
 *  to the randomly ordered list (0 , (size-1))
 *  Note: remember to allocate enough space for the order list
 ***********************************************************************/
void
ogLibOrderRand(int *order, int size)
{
    int i, j;

    if(order == NULL)
	ogEnvLog(OG_LINTERNALERROR,"ogLibOrderRand(): Space hasn't been allocated for the ordered list\n");
    
    ogEnvLog(OG_LINTERNALDEBUG, "Ordered list\n");
    for(i = 0; i < size; ++i){
	order[i] = i;
	ogEnvLog(OG_LINTERNALDEBUG, "order[%i]: %i\n", i, order[i]);
    }
    size--;
    for(i = 0; i <= size; ++i){
	j = ogLibIntRand(0, size);
	swap(&order[i], &order[j]);
    }
}
