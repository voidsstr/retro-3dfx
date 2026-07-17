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
** $Date: 10/11/00 7:33:24 PM$ 
**
*/

#include <glide.h>

#include <atutil.h>
#include <atrender.h>
#include <atinput.h>
#include <atdemop.h>
#include <conio.h>

FxBool
AppInitGraphics(void) {
    /* initialize rendering library 
       TBD: need to review these values and create ATB versions
     */

    _atGlobals.driverInfo.hWnd = _atGlobals.hWndMain;
    _atGlobals.driverInfo.emulation = _atGlobals.emulation;
    _atGlobals.driverInfo.refreshRate = GR_REFRESH_60Hz, 
    _atGlobals.driverInfo.numBuffers = 2; /* double buffer */
    _atGlobals.driverInfo.smoothingMode = GR_SMOOTHING_ENABLE;
    _atGlobals.driverInfo.fullScreen = _atGlobals.fullScreen;
    _atGlobals.driverInfo.info = NULL;

    if ( (_atGlobals.ctx = atrInit( _atGlobals.driverName, 
                                    &_atGlobals.driverInfo) ) == NULL ) {
        return FXFALSE;
    }

    atrQueryDriverCaps( &_atGlobals.caps );
    _atGlobals.fullScreen = _atGlobals.caps.fullScreen;

    if ( _atGlobals.oneTMU )
        _atGlobals.caps.numTex = 1;

    _atGlobals.graphicsEnabled = FXTRUE;
    return FXTRUE;
}

void
AppTermGraphics(void) {
    atrShutdown();
    _atGlobals.graphicsEnabled = FXTRUE;
}
