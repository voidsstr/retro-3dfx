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
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
**
** $Header: gmovie.c, 2, 10/11/00 8:21:45 PM, Brent$
** $Log: 
**  2    3dfx      1.0.1.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
 * 
 * 6     8/30/97 5:59p Tarolli
 * cleanups
 * 
 * 5     5/27/97 1:16p Peter
 * Basic cvg, w/o cmd fifo stuff. 
 * 
 * 4     5/21/97 6:05a Peter
 * 
 * 3     3/09/97 10:31a Dow
 * Added GR_DIENTRY for di glide functions
**
*/
#include <string.h>

#include <3dfx.h>
#define FX_DLL_DEFINITION
#include <fxdll.h>
#include <glide.h>
#include "fxglide.h"

GR_DIENTRY(guMovieStart, void, ( void ))
{
   GrErrorCallback( "guMovieStart:  unsupported", FXFALSE );
}

GR_DIENTRY(guMovieStop, void, ( void ))
{
   GrErrorCallback( "guMovieStop:  unsupported", FXFALSE );
}

GR_DIENTRY(guMovieSetName, void, ( const char *name ))
{
   GrErrorCallback( "guMovieSetName:  unsupported", FXFALSE );
}
