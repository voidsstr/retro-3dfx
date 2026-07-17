/*
** Copyright (c) 1996, 3Dfx Interactive, Inc.
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
**
** $Revision: 4$ 
** $Date: 10/11/00 7:31:20 PM$ 
**
*/


#ifndef _ATAUDIO_H_
#define _ATAUDIO_H_

#        ifdef __cplusplus
extern "C" {
#        endif

#if !defined( _SOUND_IMPLEMENTATION_ )
#if macintosh
    #include "macsound.h"
#else
typedef struct { int unused; } Sound;
#endif
#endif

#include <3dfx.h>

#define ATA_SOUND_LOOP  0x00000001

FxBool ataInit( FxU32 winHandle );
void  ataShutdown();

Sound *ataSoundNew( char *soundFile );
void  ataSoundDelete( Sound *s );

void  ataSoundPlay( Sound *s, int flags );
void  ataSoundStop( Sound *s );

#       ifdef __cplusplus
}
#       endif


#endif
