/**********************************************************************
 *<
	FILE: util.h

	DESCRIPTION:

	CREATED BY: Rolf Berteig

	HISTORY:

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#ifndef __UTIL__H
#define __UTIL__H

#include "Max.h"
#include "resource.h"

TCHAR *GetString(int id);

extern ClassDesc* GetColorClipDesc();
extern ClassDesc* GetAsciiOutDesc();
extern ClassDesc* GetUtilTestDesc();
extern ClassDesc* GetAppDataTestDesc();
extern ClassDesc* GetTestSoundObjDescriptor();
extern ClassDesc* GetSplashUtilDesc();

extern HINSTANCE hInstance;

#endif
