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
** $Revision: 3$
** $Date: 10/11/00 7:37:04 PM$
*/
#ifndef _FXAGP_H_
#define _FXAGP_H_

#define FXAGPAPIFUNC_1          1 /* My that's descriptive */
#define FXAGPAPIFUNC_2          2
#define FXAGPGARTRESERVE        3
#define FXAGPGARTFREE           4
#define FXAGPGARTCOMMIT         5
#define FXAGPGARTUNCOMMIT       6
#define FXAGPCMLOCATE		7

#ifdef __cplusplus
extern "C" {
#endif

FxBool
fxAGPInit(void);

FxBool
fxAGPReserve(FxU32 nPages, FxU32 *linearAddress, FxU32 *physAddr, FxU32 pciID);

FxBool
fxAGPCommit(FxU32 linearAddr, FxU32 offset, FxU32 nPages);

FxBool
fxAGPUncommit(FxU32 linearAddr, FxU32 offset, FxU32 nPages);

FxBool
fxAGPFree(FxU32 linearAddr);

void
fxAGPShutdown();

char *
fxAGPGetErrorString(void);  

#ifdef __cplusplus
}
#endif

#endif
