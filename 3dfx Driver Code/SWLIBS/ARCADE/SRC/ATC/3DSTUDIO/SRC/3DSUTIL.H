/*
** Copyright (c) 1995, 3Dfx Interactive, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of 3Dfx Interactive, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
** rights reserved under the Copyright Laws of the United States.
**
** ALPHA PRODUCT!
*/

/*
** Filename: 3dsutil.h
**
** This file contains all the prototypes for the 3dsread.c file.
*/

#ifndef TDSUTIL_H__
#define TDSUTIL_H__

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------  Includes -----------------------------*/

#include <stdio.h>
#include "3ds.h"

/*----------------------------  Prototypes ----------------------------*/

void TDSSetErrorFP( FILE *fp );
void *my_malloc( size_t size );
void TDSBail( const char *err_msg );

#ifdef __cplusplus
};
#endif

#endif /* TDSUTIL_H__ */
