/******************************Module*Header*******************************\
* Module Name: precomp.h
*
* Common headers used throughout the display driver.  This entire include
* file will typically be pre-compiled.
*
* Copyright (c) 1993-1996 Microsoft Corporation
\**************************************************************************/

#include <stddef.h>
#include <stdarg.h>
#include <limits.h>
#include <windef.h>

#if (_WIN32_WINNT >= 0x0500)
#define _CRTIMP

#if (DX == 8) // This must be defined before any DX DDK files are included
/* V56K-DX8-DX7HAL: the DX8 DDK hides every legacy D3DHAL_* alias behind
** this switch (dx95type.h: "#ifdef DX8DDK_DX7HAL_DEFINES").  This driver
** uses the DX7 HAL names throughout, so without it D3DHAL_TSS_MAXSTAGES,
** D3DHAL_DP2SETSTREAMSOURCE etc. all vanish under DX=8. */
#define DX8DDK_DX7HAL_DEFINES
#define DIRECTDRAW_VERSION  0x0800 
#define DIRECT3D_VERSION    0x0800 
#endif 

#if (DX == 8) 
#include <d3d8.h>
/* V56K-DX8-INCORDER: at DIRECT3D_VERSION 0x0800 the DX7 body of d3d.h is
** guarded out (#if DIRECT3D_VERSION < 0x0800), and with it the chain that
** reached ddrawint.h -> dx95type.h.  driver.h (included below, BEFORE
** ddfxnt95.h) needs the D3DHAL_* / PFND3DPARSEUNKNOWNCOMMAND aliases that
** dx95type.h defines for NT, so pull it in explicitly here. */
#define NODXMALLOC
#include "dx95type.h"
#endif 

#include <d3d.h>

#endif

#include <winerror.h>
#include <wingdi.h>
#include <winddi.h>
#include <devioctl.h>
#include <ntddvdeo.h>
#include <ioaccess.h>

#include "logfile.h"

#include "3dfx.h"
#include "h3g.h"
#include "h3gdefs.h"
#include "h3defs.h"
#include "h3regs.h"

#include "lines.h"

#ifdef USE_DDRAW_CODE
#include "ddvpe32.h"
#endif

#include "driver.h"
#include "hw.h"
#include "debug.h"

#include "memcheck.h"

#ifdef USE_DDRAW_CODE
#include "ddfxnt95.h"
#endif

#include "hwcext.h"
#include "glicd.h"
#include "fnproto.h"

#if (_WIN32_WINNT >= 0x0500)
#include "gdi.h"    // For GradientFill
#endif
