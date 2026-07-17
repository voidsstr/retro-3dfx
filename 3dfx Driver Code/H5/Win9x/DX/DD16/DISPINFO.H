/* $Header: dispinfo.h, 2, 10/11/00 8:51:00 PM, Brent$ */
/*
** Copyright (c) 1995-1999, 3Dfx Interactive, Inc.
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
** File name:   dispinfo.h
**
** Description: DISPLAYINFO structure.
**
** $Revision: 2$
** $Date: 10/11/00 8:51:00 PM$
**
** $History: dispinfo.h $
** 
** *****************  Version 2  *****************
** User: Michael      Date: 12/28/98   Time: 11:24a
** Updated in $/devel/h3/Win95/dx/dd16
** Added the 3Dfx/STB unified file/funciton header.  Add VSS keywords to
** files where this was missing.
** 
*/

#ifndef _DISPINFO_H_
#define _DISPINFO_H_

// FIX_ME .. this is missing the new VGA flag

// The following information was stolen from the minivdd.inc
//
//
//Following are values and explanations for the diInfoFlags word in DISPLAYINFO:
//
//RETURNED_DATA_IS_STALE, if set, means that this call to VDD_GET_DISPLAY_CONFIG
//or VDD_GetDisplayInfo (which are the Ring 3 and Ring 0 methods by which a
//program would get the DISPLAYINFO structure returned to him) caused the VDD
//to return data that was read in a previous call to VDD_GET_DISPLAY_CONFIG
//insted of actually going out and reading "fresh" data from the Registry.
//
//This flag brings to light the fact that there are some circumstances when the
//VDD cannot go out and read the registry in response to the call to
//VDD_GET_DISPLAY_CONFIG or VDD_GetDisplayInfo (due to system multi-tasking
//considerations).  In this case, this flag will be set to a 1 to indicate that
//the information being returned isn't "fresh" -- that is -- it may be
//incorrect and obsolete.  The caller should respond accordingly if this flag
//is set.
//
#define RETURNED_DATA_IS_STALE			0x01
//
//MINIVDD_FAILED_TO_LOAD if set, indicates that for some reason (typically
//that the MiniVDD didn't match the chipset installed in the machine), the
//MiniVDD didn't load.  Callers can examine this flag and act accordingly.
//
#define MINIVDD_FAILED_TO_LOAD			0x02
//
//MINIVDD_CHIP_ID_DIDNT_MATCH means that although the MiniVDD did load
//successfully, when the ChipID that the MiniVDD calculated was compared
//against the value saved in the registry, they didn't match.  An example of
//when this would happen is when the user is happily using an S3-911 card
//and then decides to upgrade his display card to an S3-864.  Since both
//cards use S3.VXD, the MiniVDD will load, however, since the card model
//is different, the VDD will return a defect to configuration manager and
//set this flag.  Callers of the GET_DISPLAY_CONFIG functions can use this
//flag to take appropriate actions to make sure that the user gets his
//configuration correct.
//
#define MINIVDD_CHIP_ID_DIDNT_MATCH		0x04
//
//REGISTRY_BPP_NOT_VALID if set, means that we failed to obtain the BPP value
//from the registry when the VDD tried to read it.
//
#define REGISTRY_BPP_NOT_VALID			0x08
//
//REGISTRY_RESOLUTION_NOT_VALID if set, means that we failed to obtain the
//resolution value from the registry when the VDD tried to read it.
//
#define REGISTRY_RESOLUTION_NOT_VALID		0x10
//
//REGISTRY_DPI_NOT_VALID if set, means that we failed to obtain the
//DPI value from the registry when the VDD tried to read it.
//
#define REGISTRY_DPI_NOT_VALID			0x20
//
//MONITOR_DEVNODE_NOT_ACTIVE is set if someone tries to make a call to the
//GET_DISPLAY_CONFIG function before the monitor DevNode has been created.
//This is certainly not fatal by any means.  It simply means that the
//monitor refresh rate info in the DISPLAYINFO data structure is totally
//invalid!
//
#define MONITOR_DEVNODE_NOT_ACTIVE		0x40
//
//MONITOR_INFO_NOT_VALID indicates that something within the code which
//retrieves and calculates the refresh rate data has failed.  This indicates
//that the values in diRefreshRateMax through diVertSyncPolarity are not
//valid and could contain random data.
//
#define MONITOR_INFO_NOT_VALID			0x80
//
//MONITOR_INFO_DISABLED_BY_USER indicates that the either the RefreshRate=
//string in SYSTEM.INI had a negative number in it or that the string in
//the display's software key RefreshRate = string was 0 or a negative number.
// APS... Now the hero in our saga enters the story from stage left
// Actually on OSR2 if you put in a -1 then you actually get the monitor
// info structure parsed out and passed on to you
//
#define MONITOR_INFO_DISABLED_BY_USER		0x100
//
//REFRESH_RATE_MAX_ONLY indicates that there was no diLowHorz, diHighHorz,
//diLowVert, diHighVert, or sync polarity data in the registry.	The
//value returned in diRefreshRateMax is the only refresh rate data that
//we have available.  This was derived either from RefreshRate= in SYSTEM.INI
//or the display software key RefreshRate = string in the registry.
//
#define REFRESH_RATE_MAX_ONLY			0x200
//
//CARD_VDD_LOADED_OK indicates that a second MiniVDD (which is useful for
//display card manufacturers wishing to extend the capabilities of the chip level
//MiniVDD's ) has successfully been loaded and initialized.
//
#define CARD_VDD_LOADED_OK			0x400

typedef   struct {
        WORD diHdrSize;
        WORD diInfoFlags;
        DWORD diDevNodeHandle;
        BYTE diDriverName[16];
        WORD diXRes;
        WORD diYRes;
        WORD diDPI;
        BYTE diPlanes;
        BYTE diBpp;
        WORD diRefreshRateMax;
        WORD diRefreshRateMin;
        WORD diLowHorz;
        WORD diHighHorz;
        WORD diLowVert;
        WORD diHighVert;
        DWORD diMonitorDevNodeHandle;
        BYTE diHorzSyncPolarity;
        BYTE diVertSyncPolarity;
} DISPLAYINFO, FAR * LPDISPLAYINFO, * PDISPLAYINFO;

extern int FAR GetDisplayInfo(LPDISPLAYINFO lpDisplayInfo);

#endif
