/* -*-c++-*- */
/* $Header: gramp.h, 3, 10/11/00 8:51:13 PM, Brent$ */
/*
** Copyright (c) 1998, 3Dfx Interactive, Inc.
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
** File name:   gramp.h
**
** Description: Support functions for gamma ramp.
**
** $Revision: 3$
** $Date: 10/11/00 8:51:13 PM$
**
** $History: gramp.h $
** 
** *****************  Version 3  *****************
** User: Michael      Date: 12/29/98   Time: 11:02a
** Updated in $/devel/h3/Win95/dx/dd16
** Implement the 3Dfx/STB unified header.
** 
** *****************  Version 2  *****************
** User: Michael      Date: 9/16/98    Time: 5:35p
** Updated in $/devel/h3/Win95/dx/dd16
** Fred's (igx) assign_gammaramp() implementation and a minor gamma fix.
** 
** *****************  Version 1  *****************
** User: Michael      Date: 8/20/98    Time: 3:39p
** Created in $/devel/h3/Win95/dx/dd16
** New file for GammaRamp.
**
*/

BOOL FAR PASCAL _loadds OEMGammaRamp(
    LPDIBENGINE lpPDevice,
    WORD wGetSet,
    LPDDGAMMARAMP lpGammaRamp);

void set_gammaramp(BOOL);
void init_gammaramp(void);
void assign_gammaramp(int, int, int, int, BOOL);
