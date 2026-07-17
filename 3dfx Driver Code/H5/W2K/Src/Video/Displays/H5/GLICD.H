/******************************Module*Header*******************************\
* Module Name: glicd.h
*
* Data structures and constant definitions for OpenGL ICD
*
* Copyright (c) 1993-1996 Microsoft Corporation
\**************************************************************************/

#ifdef  OPENGL_ICD

// Input structure for OPENGL_CMD ExtEscape.

typedef struct _WNDOBJ WNDOBJ;
typedef struct _XLATEOBJ XLATEOBJ;

typedef struct _OPENGLCMD
{
    ULONG    ulSubEsc;
    FLONG    fl;
    WNDOBJ   *pwo;
    XLATEOBJ *pxo;
} OPENGLCMD, *POPENGLCMD;

#if WINVER >= 0x500

#define OPENGLCMD_MAXMULTI WGL_SWAPMULTIPLE_MAX

typedef struct _OPENGLCMDMULTI
{
    ULONG ulSubEsc;
    FLONG fl;
    ULONG cMulti;
    XLATEOBJ *pxo;
} OPENGLCMDMULTI, *POPENGLCMDMULTI;

#endif // WINVER >= 0x500

// Flags for OPENGL_CMD ExtEscape.

#define OGLCMD_NEEDWNDOBJ       0x01
#define OGLCMD_NEEDXLATEOBJ     0x02

#if WINVER >= 0x500
#define OGLCMD_MULTIWNDOBJ      0x04
#endif // WINVER >= 0x500

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

//===========================================================
#endif  // OPENGL_ICD
