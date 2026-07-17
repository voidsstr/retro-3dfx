/****************************************************************************
*
*						MegaGraph Graphics Library
*
*                   Copyright (C) 1996 SciTech Software.
*							All rights reserved.
*
* Filename:		$Workfile: fgl.h$
* Version:		$Revision: 4$
*
* Language:		ANSI C
* Environment:	IBM PC (MS DOS)
*
* Description:	Header file for the MGL OpenGL fullscreen interface.
*
* $Date: 10/11/00 7:22:06 PM$ $Author: Brent$
*
****************************************************************************/

#ifndef	__FGL_H
#define	__FGL_H

#ifndef __DEBUG_H
#include "debug.h"
#endif

/*---------------------- Macro and type definitions -----------------------*/

#pragma pack(1)

#ifndef	__MGRAPH_H

/* Palette entry structure	*/

typedef struct {
	uchar	blue;				/* Blue component of color				*/
	uchar	green;				/* Green component of color				*/
	uchar	red;				/* Blue component of color				*/
	uchar	alpha;				/* Alpha or alignment byte				*/
	} palette_t;

#endif

typedef struct {
	int    	redSize;            /* Size of direct color red mask        */
	int    	redPos;             /* Bit posn of lsb of red mask          */
	int    	greenSize;          /* Size of direct color green mask      */
	int    	greenPos;           /* Bit posn of lsb of green mask        */
	int    	blueSize;           /* Size of direct color blue mask       */
	int    	bluePos;            /* Bit posn of lsb of blue mask         */
	int    	rsvdSize;           /* Size of direct color res mask        */
	int   	rsvdPos;            /* Bit posn of lsb of res mask          */
	} pixelFormat;

/* The following defines the hardware information returned by fglDetect */

typedef struct _FGL_hwInfo {
	char	driverName[40];			/* Name of driver (ie: 3DFX.DRV)	*/
	int		videoMemSize;			/* Amount of video memory			*/
	int		zbufferSize;			/* Amount of z-buffer memory		*/
	int		textureSize;			/* Amount of texture memory			*/
	} FGL_hwInfo;

/* The following defines the mode information returned by fglDetect */

typedef struct _FGL_modeInfo {
	int				xRes;			/* Device x resolution - 1			*/
	int				yRes;			/* Device y resolution - 1		  	*/
	int				bitsPerPixel;	/* Number of bits per pixel			*/
	int				bytesPerLine;	/* Number of bytes in a line		*/
    int				maxBuffers;		/* Number of buffers				*/
	pixelFormat		pf;				/* Pixel format information 		*/
	} FGL_modeInfo;

/* Defines for passing to fglBeginDirectAccess */

#define	FGL_FRONT_BUFFER	0
#define	FGL_BACK_BUFFER		1
#define	FGL_Z_BUFFER		2
#define	FGL_TEXTURE_BUFFER	3

#pragma pack()

/*------------------------- Function Prototypes ---------------------------*/

/* Fullscreen OpenGL driver bindings */

#ifndef	BUILD_MGL
bool	WINAPI fglDetect(int *numModes,FGL_hwInfo *hwInfo,FGL_modeInfo *modes);
bool	WINAPI fglSetVideoMode(int mode);
void	WINAPI fglSetFocus(bool active);
void	WINAPI fglRestoreMode(void);
void * 	WINAPI fglBeginDirectAccess(int buffer);
void	WINAPI fglEndDirectAccess(void);
bool  	WINAPI fglSwapBuffers(bool waitVRT);
void 	WINAPI fglRealizePalette(palette_t *pal,int numColors,int startIndex,int waitVRT);
#endif

#endif	/* __FGL_H */

