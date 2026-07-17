/*
** Copyright (c) 1995-1998, 3Dfx Interactive, Inc.
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
** $Revision: 2$
** $Date: 10/11/00 8:42:57 PM$
**
*/

/**************************************************************************
* I N C L U D E S
***************************************************************************/

#include "precomp.h"

#undef __D3TXTR_H__
#define FX_DEFINE_MACROS
#include "d3txtr.h"

/**************************************************************************
* G L O B A L   V A R S
***************************************************************************/

FxU32 _cpu_type;

/**************************************************************************
* P U B L I C   F U N C T I O N S
***************************************************************************/

#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
#else

DWORD __stdcall
mySetRenderTarget32 ( LPD3DHAL_SETRENDERTARGETDATA psrtd )
{
  SETUP_PPDEV(psrtd->dwhContext)
  RC  *pRc;


  D3D_ENTRY("mySetRenderTarget32");

  if (CONTEXT_VALIDATE(psrtd->dwhContext))
  {
    D3DPRINT(255, "mySetRenderTarget32, bad context =0x08lx", psrtd->dwhContext);
    psrtd->ddrval = D3DHAL_CONTEXT_BAD;
    D3D_EXIT(DDHAL_DRIVER_HANDLED);
  }

  D3DPRINT(255, "mySetRenderTarget32, ptsd->dwhContext =%08lx", psrtd->dwhContext);

  pRc = CONTEXT_PTR(psrtd->dwhContext);

  // cannot render into system memory
  if (DDSCAPS_SYSTEMMEMORY & psrtd->lpDDS->ddsCaps.dwCaps)
  {
    psrtd->ddrval = DDERR_CURRENTLYNOTAVAIL;
    D3D_EXIT(DDHAL_DRIVER_HANDLED);
  }

  if (psrtd->lpDDSZ)
  {
    // cannot render if z buffer is in system memory
    if (DDSCAPS_SYSTEMMEMORY & psrtd->lpDDSZ->ddsCaps.dwCaps)
    {
      psrtd->ddrval = DDERR_CURRENTLYNOTAVAIL;
      D3D_EXIT(DDHAL_DRIVER_HANDLED);
    }

    pRc->lpDDSZ = psrtd->lpDDSZ;
  }
  else
  {
    pRc->lpDDSZ = NULL;
  }

  pRc->lpDDS = psrtd->lpDDS;

  D3DPRINT(4, "mySetRenderTarget32, dwhContext=%08lx pDDS =%08lx, pDDSZ =%08lx",
           psrtd->dwhContext,pRc->lpDDS,pRc->lpDDSZ);

  _D3(lastContext) = 0;

  psrtd->ddrval = DD_OK;
  D3D_EXIT(DDHAL_DRIVER_HANDLED);
}

#endif

