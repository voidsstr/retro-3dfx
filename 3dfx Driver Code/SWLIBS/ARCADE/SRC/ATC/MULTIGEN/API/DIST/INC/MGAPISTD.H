/*******************************************************************************
 * 
 * $Header: mgapistd.h, 4, 10/11/00 7:32:36 PM, Brent$
 * $Revision: 4$
 * $Date: 10/11/00 7:32:36 PM$
 *
 ******************************************************************************/

/*============================================================================*\

   PROPRIETARY RIGHTS NOTICE: All rights reserved.  This software contains 
   proprietary information and trade secrets of MultiGen Inc. of San Jose, 
   California, and embodies substantial creative efforts as well as 
   confidential information, ideas, and expressions.  No part or all of this 
   software may be reproduced in any form, or by any means of electronic, 
   mechanical, or otherwise, without the written permission of MultiGen Inc.

   COPYRIGHT NOTICE: Copyright (C) 1986-1996 MultiGen Inc., San Jose, California.

\*============================================================================*/

/*----------------------------------------------------------------------------*/

#ifndef MGAPISTD_H_
#define MGAPISTD_H_

/*----------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*/

#ifndef mgNULL
#define mgNULL 0
#endif

typedef enum {
	mgFALSE = 0,
	mgTRUE = 1
} mgbool;

typedef struct mgrec mgrec;

typedef int mgcode;

typedef struct mgdb mgdb;

typedef struct mgbead mgbead;

struct mgdb {
	struct mgbead* topib;
};

typedef enum tagtype {
	mgtype_null = 0,			/* undefined */
	mgtype_cval = 1,			/* char int */
	mgtype_ucval = 2,			/* unsigned char int */
	mgtype_sval = 3,			/* short */
	mgtype_usval = 4,			/* unsigned short */
	mgtype_ival = 5,			/* int */
	mgtype_uival = 6,			/* unsigned int */
	mgtype_fval = 7,			/* float */
	mgtype_dval = 8,			/* double */
	mgtype_flag = 9,			/* flag */
	mgtype_enum = 10,			/* bit integer */
	mgtype_text = 11,			/* text string */
	mgtype_root = 12,			/* db header */
	mgtype_bead = 13,			/* has ifmt bead */
	mgtype_rec = 14,			/* generic record */
	mgtype_data = 15,			/* data buffer */
	mgtype_xfm = 16			/* xformll record  (internal) */
} tagtype;

/*============================================================================*/

#ifdef __cplusplus
}
#endif

#endif
/* DON'T ADD STUFF AFTER THIS #endif */
