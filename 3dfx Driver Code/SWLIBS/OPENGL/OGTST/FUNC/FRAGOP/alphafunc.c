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

/* afunction.c - $Revision: 2$ */

/*
 * This program tests the afunction command using points to check. 
 */

#include "ogtst.h"		/* include test environment		 */

TESTMOD(alphafunc) {
    int x, y, xmax, ymax, alpha;
    GLfloat z, alphaf;
    unsigned int col2, check_col;
    GLint i, j, k, fi, order[8], nloop, af;
    GLboolean used, first;

    xmax = ogEnvQuery(OG_XWSIZE) - 1;
    ymax = ogEnvQuery(OG_YWSIZE) - 1;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0., ogEnvQuery(OG_XWSIZE), 0., ogEnvQuery(OG_YWSIZE), -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);

	first = GL_TRUE;
    while (pass--) {
if( first )
ogEnvLog(1, "first = true\n");
else
ogEnvLog(1, "first = false\n");

	  if( first ) {
		nloop = 8;	
	
		/* randomly order the enums to make sure we test them all */
/* ogLibOrderRand(order, nloop); */
		for (j = 0; j < 8; j++){
	    used = GL_TRUE;
	    order[j] = -1;
	    while(used){
		used = GL_FALSE;
		fi = ogLibIntRand(0, 7);
		for (k = 0; k <= j; k++){
		    if (fi == order[k])
			used = GL_TRUE;
		}
	    }
	    order[j] = fi;
	  	}
	  }
	else
	  nloop = 1;
	
	/* test all possible functions on one pass */
	for (i = 0; i < nloop; i++){
	    glClearColor(0., 0., 0., 0.);
	    glClear(GL_COLOR_BUFFER_BIT);

	    col2 = ogLibBitRand(32);
	    alpha = col2 & 0xff;
	    glColor4ub((col2>>24)&0xff, (col2 >>16)&0xff, (col2>>8)&0xff, alpha);
	    ogEnvLog(OG_LPARAMETERS, 
			"glColor4ub(0x%02x,0x%02x,0x%02x,0x%02x);\n", (col2>>24)&0xff, (col2>>16)&0xff, (col2>>8)&0xff, alpha);
	    
	    z = ogLibBitRand(8) / 255.;
	    alphaf = alpha / 255.;
	    
	    glEnable(GL_ALPHA_TEST);
	    ogEnvLog(OG_LPARAMETERS, "glEnable(GL_ALPHA_TEST);\n");

		if( first )
			af = order[i];
		else
			af = ogLibBitRand(3);

	    switch (af) {
	      case 0:
		glAlphaFunc(GL_NEVER, z);
		ogEnvLog(OG_LPARAMETERS, "glAlphaFunc(GL_NEVER, %f);\n", z);
		check_col = 0;
		break;
	      case 1:
		glAlphaFunc(GL_LESS, z);
		ogEnvLog(OG_LPARAMETERS, "glAlphaFunc(GL_LESS, %f);\n", z);
		check_col = ((alphaf < z) ? col2 : 0);
		break;
	      case 2:
		glAlphaFunc(GL_EQUAL, z);
		ogEnvLog(OG_LPARAMETERS, "glAlphaFunc(GL_EQUAL, %f);\n", z);
		check_col = ((alphaf == z) ? col2 : 0);
		break;
	      case 3:
		glAlphaFunc(GL_LEQUAL, z);
		ogEnvLog(OG_LPARAMETERS, "glAlphaFunc(GL_LEQUAL, %f);\n", z);
		check_col = ((alphaf <= z) ? col2 : 0);
		break;
	      case 4:
		glAlphaFunc(GL_GREATER, z);
		ogEnvLog(OG_LPARAMETERS, "glAlphaFunc(GL_GREATER, %f);\n", z);
		check_col = ((alphaf > z) ? col2 : 0);
		break;
	      case 5:
		glAlphaFunc(GL_NOTEQUAL, z);
		ogEnvLog(OG_LPARAMETERS, "glAlphaFunc(GL_NOTEQUAL, %f);\n", z);
		check_col = ((alphaf != z) ? col2 : 0);
		break;
	      case 6:
		glAlphaFunc(GL_GEQUAL, z);
		ogEnvLog(OG_LPARAMETERS, "glAlphaFunc(GL_GEQUAL, %f);\n", z);
		check_col = ((alphaf >= z) ? col2 : 0);
		break;
	      case 7:
		glAlphaFunc(GL_ALWAYS, z);
		ogEnvLog(OG_LPARAMETERS, "glAlphaFunc(GL_ALWAYS, %f);\n", z);
		check_col = col2;
		break;
	    }
	    
	    x = 1 + ogLibIntRand(0,xmax - 2);
	    y = 1 + ogLibIntRand(0,ymax - 2);

	    ogLibDrawFragments(x, y);
	    
	    /* glRecti(x+1,y+1, x+5,y+5); */
    	ogEnvLog(OG_LINTERNALDEBUG, "Alphas: current %f, ref %f\n", alphaf, z);
    	ogEnvLog(OG_LINTERNALDEBUG, "Color should be: 0x%08x\n", check_col);
	    ogLibRectCheck(x, y, x, y, check_col, 0);
	  }
	  first = GL_FALSE;
    }
}

CLEANUP(alphafunc) {
    glAlphaFunc(GL_ALWAYS,0);
    glDisable(GL_ALPHA_TEST);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    ogLibSetDefaultColors();
}
