/******************************Module*Header*******************************\
* Module Name: esc.h
*
* OpenGL driver escape functions and structures.
*
* Created: 04-Jan-1994 23:36:29
* Author: Gilman Wong [gilmanw]
*
* Copyright (c) 1994 Microsoft Corporation
*
\**************************************************************************/

#ifndef _DRVESC_H_
#define _DRVESC_H_

#ifdef _WIN95_

#include <dciman.h>

typedef struct _DCIINFO_ {
    HDC hdc;
    LPDCISURFACEINFO pDCISurfInfo;
} DCIINFO;

//
// Keep these per-process so that the thunk code in Control will not
// need to keep grabbing them.
//

extern DWORD hmodOGLSRV;
extern DWORD lpfnEscapeOGLSRV;
#endif

/*******************************************************************/

/* to satisfy the compiler */
typedef void	WNDOBJ;
typedef void	XLATEOBJ;

/* The defines for the escapes */
#define	OPENGL_CMD	4352
#define	OPENGL_GETINFO	4353

// Input structure for OPENGL_CMD ExtEscape.

typedef struct _OPENGLCMD
{
    ULONG    ulSubEsc;
    FLONG    fl;
    WNDOBJ   *pwo;
    XLATEOBJ *pxo;
} OPENGLCMD, *POPENGLCMD;

// Flags for OPENGL_CMD ExtEscape.

#define OGLCMD_NEEDWNDOBJ       0x01
#define OGLCMD_NEEDXLATEOBJ     0x02

// OPENGL_GETINFO ExtEscape sub-escape numbers.  They are defined by Microsoft.

#define OPENGL_GETINFO_DRVNAME  0

// Input structure for OPENGL_GETINFO ExtEscape.

typedef struct _OPENGLGETINFO
{
    ULONG   ulSubEsc;
} OPENGLGETINFO, *POPENGLGETINFO;

// Input structure for OPENGL_GETINFO_DRVNAME ExtEscape.

typedef struct _GLDRVNAME
{
    OPENGLGETINFO   oglget;
} GLDRVNAME, *PGLDRVNAME;

// Output structure for OPENGL_GETINFO_DRVNAME ExtEscape.

typedef struct _GLDRVNAMERET
{
    ULONG   ulVersion;              // must be 1 for this version
    ULONG   ulDriverVersion;        // driver specific version number
    WCHAR   awch[MAX_PATH+1];
} GLDRVNAMERET, *PGLDRVNAMERET;


/*******************************************************************/


//
// OpenGL sub-escape commands for the OPENGL_CMD escape.  All escapes are
// used with ExtEscape except for CMD_SETCONTEXT and CMD_ATTENTION which use
// DrawEscape.
//
// These sub-escape commands are optional and specific to the OpenGL
// installable driver implementation.
//

#define CMD_SETCONTEXT          1
#define CMD_ATTENTION           2
#define CMD_CREATECONTEXT       3
#define CMD_DELETECONTEXT       4
#define CMD_RELEASECONTEXT      5
#define CMD_DUPLICATESECTION    6
#define CMD_SHARELISTS          7

#ifdef OVERLAY_PLANES

#define CMD_COPYCONTEXT            8
#define CMD_CREATELAYERCONTEXT     9
#define CMD_DESCRIBELAYERPLANE     10
#define CMD_GETLAYERPALETTEENTRIES 11
#define CMD_REALIZELAYERPALETTE    12
#define CMD_SETLAYERPALETTEENTRIES 13
#define CMD_SWAPLAYERBUFFERS       14

#ifdef _WIN95_
#define CMD_DESCRIBEPIXELFORMAT    15
#define CMD_SETPIXELFORMAT         16
#define CMD_SWAPBUFFERS            17
#endif

#else

#ifdef _WIN95_
#define CMD_DESCRIBEPIXELFORMAT 8
#define CMD_SETPIXELFORMAT      9
#define CMD_SWAPBUFFERS         10
#endif

#endif

typedef struct _OGLCMDHEADER_
{
    OPENGLCMD   oglcmd;

#ifdef _WIN95_
    PVOID       pvFramebufferBits;
    LONG        lFramebufferStride;
    DWORD       hmod;
    DWORD       lpfnEscape;

    DCIINFO     *pdcii;
#endif

} OGLCMDHEADER, *POGLCMDHEADER;

#define OPENGLCMD_SUBESC(pv)   ( ((POGLCMDHEADER)(pv))->oglcmd.ulSubEsc )

//
// Structures passed to ExtEscape and DrawEscape via input buffer for
// each of the OpenGL sub-escape commands.
//
// These structures are specific to a given OpenGL installable driver
// implementation.
//


typedef struct _GLMAPREG_
{
    OGLCMDHEADER och;

    UINT base;
    UINT length;
} GLMAPREG;

typedef struct _GLMAPREGRET_
{
    void *addr;
} GLMAPREGRET;

#endif /* _DRVESC_H_ */
