/*
** Copyright 1992-1997 Silicon Graphics, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of Silicon Graphics, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of Silicon Graphics, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
*/
/************************************************************************
 *                                                                      *
 *                         GLiNT                                        *
 *                                                                      *
 *                                                                      *
 *          Copyright (C) 1994  3Dlabs Inc. Ltd                         *
 *                                                                      *
 *                                                                      *
 * This software and its associated documentation contains proprietary, *
 * confidential and trade secret information of DuPont Pixel Systems    *
 * Limited and except as provided by written agreement with DuPont      *
 * Pixel Systems Limited                                                *
 *                                                                      *
 * a) no part may be disclosed, distributed, reproduced, transmitted,   *
 *    transcribed, stored in a retieval system, adapted or translated   *
 *    in any form or by any means electronic, mechanical, magnetic,     *
 *    optical, chemical, manual or otherwise,                           *
 *                                                                      *
 *    and                                                               *
 *                                                                      *
 * b) the recipient is not entitled to discover through reverse         *
 *    engineering or reverse compiling or other such techniques or      *
 *    processes the trade secrets contained therein or in the           *
 *    documentation.                                                    *
 *                                                                      *
 ************************************************************************/

/************************************************************************
*
* Name: wgldrawable.h
*
* Description:
*
* This file contains the error codes and messages pertinent to the
* 3D labs OpenGL DLL.
*
* Author: P. Huxley
*
* See SCCS for History.
*
************************************************************************/

#ifndef __wglerror_h_
#define __wglerror_h_
/*
** These comply with the Win32 error code format.
*/	
#define	WGL_ERROR		0xC0000000
#define	WGL_WARNING		0x80000000
#define	WGL_OGL_ID		0x003D3D00
#define	WGL_APP_ERROR		0x20000000

/*
** In a call to either wglCreateContext or wglMakeCurrent an HDC
** handle was passed which had no associated window.
*/
#define	WGL_INVALID_HDC			0x1
/*
** During a call to wglMakeCurrent, the HRC handle passed in was
** Invalid indicating that it had not been successfully created in
** a call to wglCreateContext
*/
#define	WGL_INVALID_HRC			0x2
/*
** During a call to wglUseFontBitmaps it has been noticed that TRUE
** TYPE capabilities are not available on the system.  
*/
#define	WGL_NO_TT_CAPS			0x3
/*
** During a call to wglUseFontBitmaps information about a glyph 
** could not be obtained.
*/
#define	WGL_GET_GLYPH_FAILED		0x4
/*
** There has been a memory allocation error during a call to WGL.
*/
#define	WGL_MALLOC_FAILED		0x5
/*
** During the initialisation phase of the OpenGL DLL there was a
** problem initialising the palette.
*/
#define	WGL_PAL_INIT_FAILED		0x6
/*
** During the initialisation stage of the OpenGL DLL the hook into
** the message stream to monitor window changes could not be made.
*/
#define	WGL_HOOK_FAILED			0x7
/*
** During the initialisation phase of the OpenGL DLL a required 
** palette could not be created.
*/
#define	WGL_CREATE_PALETTE_FAILED	0x8
/*
** In a call to wglCreateContext, the pixel format associated with
** the HDC is wrong.  Perhaps the HDC itself is incorrect.
*/
#define	WGL_BAD_PIXEL_FORMAT		0x9
/*
** In a call to wglCreateContext, the pixel format specified is
** unsupported by this implementation of OpenGL
*/
#define	WGL_UNSUPPORTED_PIXEL_FORMAT	0x10
/*
** In a call to wglCreateContext, the pixel format associated with 
** the HDC could not be described by the system.
*/
#define WGL_UNKNOWN_PIXEL_FORMAT	0x11 
/*
** During a call to create the WGL context, the attempt at registering
** the window with the GLINT driver failed.
*/
#define	WGL_GLINT_INITWIN_FAILED	0x12
/*
** During a call to create the WGL context, the window was registered
** with the driver, but the shared memory area could not be set up.
*/
#define WGL_GLINT_GETMEM_FAILED		0x13
/*
** When the OpenGL library started up it attempted to make a call to
** the GLINT driver extension to get the configuration of the board.
** the call failed.  It's serious.
*/
#define	WGL_DRV_CONFIG_FAILED		0x14
/*
** During the creation of a WGL context the call to create an area of
** shared memory to pass data to the driver failed.
*/
#define	WGL_DRIVER_GSM_FAILED		0x15
/*
** During a the creation of a WGL context, a call was made to the driver
** to hook up to the user allocated shared memory region failed.
*/
#define	WGL_DRIVER_MEMAP_FAILED		0x16
/*
** When the library was being initialised, the mutex could not be created
*/
#define	WGL_CREATE_MUTEX_FAILED		0x17
/*
** Could not get the lock.
*/ 
#define	WGL_MUTEX_LOCK_FAILED		0x18
/*
** In a call to make the making the context current, it was found that the
** context was in use by another thread.
*/
#define	WGL_CONTEXT_IN_USE		0x19
/*
** An attempt was made to use a different pixel format for the creation and
** setting of a context.
*/
#define	WGL_DIFFERENT_P_FMAT		0x1a
/*
** An attempt has been made to release a context which is not being used
** by the current thread.
*/
#define	WGL_NOT_CURRENT_OWNER		0x1b
/*
** An attempt has been made to release a context which is not currently
** set.
*/
#define	WGL_HRC_IS_NOT_CURRENT		0x1c
/*
** At initialisation of the installable driver it has noticed that it isn't
** compatible with the display driver.
*/
#define WGL_INVALID_DRIVER_VERSION	0x1d
/*
** The driver failed to make the context current in a call to wglMakeCurrent.
*/
#define WGL_SET_CONTEXT_FAILED		0x1e
/*
** The extension is not supported by the client driver.
*/					
#define WGL_NO_EXTENSION		0x1f
/*
** An attempt was made to share display list space and the source context was 
** invalid.
*/
#define WGL_SHR_INVALID_SRC_CONTEXT	0x20
/*
** An attempt was made to share display list space and the source context was 
** invalid.
*/
#define WGL_SHR_INVALID_DST_CONTEXT	0x21
/*
** An attempt was made to share a list when the secondary context's list was
** not empty.
*/
#define WGL_SHR_DST_LIST_NOT_EMPTY	0x22

#endif /* __wglerror_h_ */
