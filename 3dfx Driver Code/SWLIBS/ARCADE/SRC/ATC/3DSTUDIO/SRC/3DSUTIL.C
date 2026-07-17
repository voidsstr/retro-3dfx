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
** Filename: 3dsutil.c
**
** This file contains the utility function used by the 3D Studio
** loader.
*/

/*-----------------------------  Includes -----------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include "3ds.h"

/*-------------------------  Global Variables -------------------------*/
extern FILE *error_fp = stderr;

/*----------------------------  Functions -----------------------------*/

/*---------------------------------------------------------------------
  Function: TDSSetErrorFP
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: sets the error file pointer to a given file pointer, so
               it may print error information to this given 
               destination
  Arguments: 
    fp - pointer to a given file
  Return: see fp
---------------------------------------------------------------------*/
void TDSSetErrorFP( FILE *fp )
{
  error_fp = fp;
}

/*---------------------------------------------------------------------
  Function: my_malloc
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: malloc wrapper; some compilers will return a zero, this
               handles that return appropriately
  Arguments: 
    size - how much memory needs to be allocated
  Return: pointer to allocate memory, if available
---------------------------------------------------------------------*/
void *my_malloc( size_t size )
{
  if( size == 0 )
    return ( void * )1;
  return malloc( size );
}

#if defined( __DOS__ ) | defined( WIN32 )
#define malloc my_malloc
#endif
