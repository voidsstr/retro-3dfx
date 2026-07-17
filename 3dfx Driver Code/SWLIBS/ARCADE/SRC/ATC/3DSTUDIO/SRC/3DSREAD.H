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
** Filename: 3dsread.h
**
** This file contains all the prototypes for the 3dsread.c file.
*/

#ifndef TDSREAD_H__
#define TDSREAD_H__

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------  Includes -----------------------------*/

#include <stdio.h>
#include "3ds.h"

/*----------------------------  Prototypes ----------------------------*/

tds_ubyte  TDSReadUByte( TDSFile *file );
tds_ushort TDSReadUShort( TDSFile *file );
tds_short  TDSReadShort( TDSFile *file );
tds_uint   TDSReadUInt( TDSFile *file );
tds_long   TDSReadLong( TDSFile *file );
tds_ulong  TDSReadULong( TDSFile *file );
tds_float  TDSReadFloat( TDSFile *file );
void       TDSReadString( TDSFile *file, tds_string string );
void       TDSReadPoint( TDSFile *file, tds_point *point );

#ifdef __cplusplus
};
#endif

#endif /* TDSREAD_H__ */
