/*
** Copyright 1992, Silicon Graphics, Inc.
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
*/

#define STATEDATA_MAX_SIZE 129
#define MAX_ENUMS 3  	/* We use at most 2 enums with room for NULL 
			   terminator */
#define OG_STATE_CHECK_NULL	0xdeadbeef

/* type of data returned from glGet*() */
#define     STATEDATA_BOOLEAN    1 /* value returned is boolean */
#define     STATEDATA_DATA       2 /* value returned is a number */
#define     STATEDATA_ENUM       3 /* value returned is an enumerated type */
#define     STATEDATA_MATRIX     4 /* value returned is a matrix */
#define     STATEDATA_STIPPLE    5 /* value returned is a stipple pattern */

/* how to initialize comparison data */
#define     STATEDATA_PREDEFINED  	 7
#define     STATEDATA_STATE_GET_TARGET   8
#define     STATEDATA_WINDOW_GET_TARGET  9
#define     STATEDATA_ONTHEFLY  	10

typedef struct _stateRec {
    GLenum value[MAX_ENUMS];    /* values to pass to glGet*() */
    const char *valueString1;   /* name to use in error message if test fails */
    const char *valueString2;
    void (*GetFunc)(struct _stateRec *); /* function to call with data */
    GLenum dataType; /* how data is initialized */
    int dataCount; /* number of data elements */
    /* Function to use for non simple gets that can be done via a single
     * funciion call (either GetXXXParam or some special get funcs, 
     * e.g., glGetSharpenTexFuncSGIS).
     */
    void (*SpecialGetFunc) (void);
    GLfloat DefaultData[STATEDATA_MAX_SIZE]; /* data to be compared with */ 
} stateRec;

typedef struct _stateRecEXT { /* nearly same as _stateRec */
    stateRec standard;
    const char *extensionString; /* don't run test unless string is there */
    GLboolean supported;        /* Extension is supported */
} stateRecEXT;

