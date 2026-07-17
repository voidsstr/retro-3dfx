#ifndef __AVENGER_DISPLAY_DETECTION_H__
#define __AVENGER_DISPLAY_DETECTION_H__

/*
** Copyright (c) 1996-1999, 3Dfx Interactive, Inc.
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
** File name:   avenger_display_detection.h
**
**
** $Header: Avenger_Display_Detection.h, 3, 10/11/00 8:33:44 PM, Brent$
**
** $History: avenger_display_detection.h $
** 
** *****************  Version 2  *****************
** User: Kcd          Date: 7/19/99    Time: 3:47p
** Updated in $/devel/h3/MacOS8/GDX/src
** Added VGA monitor detection code.
** 
** *****************  Version 1  *****************
** User: Kcd          Date: 6/03/99    Time: 6:45p
** Created in $/devel/h3/MacOS8/GDX/src
** MacOS 8 2D Display Driver
** 
**
*/

#include <GraphicsPriv.h>


ExtendedSenseCode	AvengerGetExtendedSenseCode(void);
void				AvengerResetSenseLines(void);
void				AvengerDriveSenseLines(SenseLine senseLine);
UInt32				AvengerReadSenseLines(void);
extern FxI32 AvengerGetMonitorType(void);

#endif /* __AVENGER_DISPLAY_DETECTION_H__ */