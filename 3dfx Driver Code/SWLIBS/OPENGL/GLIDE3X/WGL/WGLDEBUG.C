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
#if defined(__WGL_USE_DIRECTDRAW)
/************************************************************************
 *                                                                      *
 *                               GLiNT                                  *
 *                                                                      *
 *                                                                      *
 *                   Copyright (C) 1994  3Dlabs Inc. Ltd.               *
 *                                                                      *
 *                                                                      *
 * This software and its associated documentation contains proprietary, *
 * confidential and trade secret information of 3Dlabs Ltd. and except  *
 * as provided by written agreement with 3Dlabs Ltd.                    *
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

#include <wgllib.h>
#include <ddraw.h>

#define NUM_DDRAW_ERRORS (sizeof(DDRAWERR) / sizeof(DDRAWERR[0]))
static struct
{
    HRESULT dderval;
    char* descriptor;
}
DDRAWERR[] =
{
    DDERR_ALREADYINITIALIZED,          "DDERR_ALREADYINITIALIZED",
    DDERR_CANNOTATTACHSURFACE,         "DDERR_CANNOTATTACHSURFACE",
    DDERR_CANNOTDETACHSURFACE,         "DDERR_CANNOTDETACHSURFACE",
    DDERR_CURRENTLYNOTAVAIL,           "DDERR_CURRENTLYNOTAVAIL",
    DDERR_EXCEPTION,                   "DDERR_EXCEPTION",
    DDERR_GENERIC,                     "DDERR_GENERIC",
    DDERR_HEIGHTALIGN,                 "DDERR_HEIGHTALIGN",
    DDERR_INCOMPATIBLEPRIMARY,         "DDERR_INCOMPATIBLEPRIMARY",
    DDERR_INVALIDCAPS,                 "DDERR_INVALIDCAPS",
    DDERR_INVALIDCLIPLIST,             "DDERR_INVALIDCLIPLIST",
    DDERR_INVALIDMODE,                 "DDERR_INVALIDMODE",
    DDERR_INVALIDOBJECT,               "DDERR_INVALIDOBJECT",
    DDERR_INVALIDPARAMS,               "DDERR_INVALIDPARAMS",
    DDERR_INVALIDPIXELFORMAT,          "DDERR_INVALIDPIXELFORMAT",
    DDERR_INVALIDRECT,                 "DDERR_INVALIDRECT",
    DDERR_LOCKEDSURFACES,              "DDERR_LOCKEDSURFACES",
    DDERR_NO3D,                        "DDERR_NO3D",
    DDERR_NOALPHAHW,                   "DDERR_NOALPHAHW",
    DDERR_NOCLIPLIST,                  "DDERR_NOCLIPLIST",
    DDERR_NOCOLORCONVHW,               "DDERR_NOCOLORCONVHW",
    DDERR_NOCOOPERATIVELEVELSET,       "DDERR_NOCOOPERATIVELEVELSET",
    DDERR_NOCOLORKEY,                  "DDERR_NOCOLORKEY",
    DDERR_NOCOLORKEYHW,                "DDERR_NOCOLORKEYHW",
    DDERR_NODIRECTDRAWSUPPORT,         "DDERR_NODIRECTDRAWSUPPORT",
    DDERR_NOEXCLUSIVEMODE,             "DDERR_NOEXCLUSIVEMODE",
    DDERR_NOFLIPHW,                    "DDERR_NOFLIPHW",
    DDERR_NOGDI,                       "DDERR_NOGDI",
    DDERR_NOMIRRORHW,                  "DDERR_NOMIRRORHW",
    DDERR_NOTFOUND,                    "DDERR_NOTFOUND",
    DDERR_NOOVERLAYHW,                 "DDERR_NOOVERLAYHW",
    DDERR_NORASTEROPHW,                "DDERR_NORASTEROPHW",
    DDERR_NOROTATIONHW,                "DDERR_NOROTATIONHW",
    DDERR_NOSTRETCHHW,                 "DDERR_NOSTRETCHHW",
    DDERR_NOT4BITCOLOR,                "DDERR_NOT4BITCOLOR",
    DDERR_NOT4BITCOLORINDEX,           "DDERR_NOT4BITCOLORINDEX",
    DDERR_NOT8BITCOLOR,                "DDERR_NOT8BITCOLOR",
    DDERR_NOTEXTUREHW,                 "DDERR_NOTEXTUREHW",
    DDERR_NOVSYNCHW,                   "DDERR_NOVSYNCHW",
    DDERR_NOZBUFFERHW,                 "DDERR_NOZBUFFERHW",
    DDERR_NOZOVERLAYHW,                "DDERR_NOZOVERLAYHW",
    DDERR_OUTOFCAPS,                   "DDERR_OUTOFCAPS",
    DDERR_OUTOFMEMORY,                 "DDERR_OUTOFMEMORY",
    DDERR_OUTOFVIDEOMEMORY,            "DDERR_OUTOFVIDEOMEMORY",
    DDERR_OVERLAYCANTCLIP,             "DDERR_OVERLAYCANTCLIP",
    DDERR_OVERLAYCOLORKEYONLYONEACTIVE,"DDERR_OVERLAYCOLORKEYONLYONEACTIVE",
    DDERR_PALETTEBUSY,                 "DDERR_PALETTEBUSY",
    DDERR_COLORKEYNOTSET,              "DDERR_COLORKEYNOTSET",
    DDERR_SURFACEALREADYATTACHED,      "DDERR_SURFACEALREADYATTACHED",
    DDERR_SURFACEALREADYDEPENDENT,     "DDERR_SURFACEALREADYDEPENDENT",
    DDERR_SURFACEBUSY,                 "DDERR_SURFACEBUSY",
    DDERR_SURFACEISOBSCURED,           "DDERR_SURFACEISOBSCURED",
    DDERR_SURFACELOST,                 "DDERR_SURFACELOST",
    DDERR_SURFACENOTATTACHED,          "DDERR_SURFACENOTATTACHED",
    DDERR_TOOBIGHEIGHT,                "DDERR_TOOBIGHEIGHT",
    DDERR_TOOBIGSIZE,                  "DDERR_TOOBIGWIDTH",
    DDERR_UNSUPPORTED,                 "DDERR_UNSUPPORTED",
    DDERR_UNSUPPORTEDFORMAT,           "DDERR_UNSUPPORTEDFORMAT",
    DDERR_UNSUPPORTEDMASK,             "DDERR_UNSUPPORTEDMASK",
    DDERR_VERTICALBLANKINPROGRESS,     "DDERR_VERTICALBLANKINPROGRESS",
    DDERR_WASSTILLDRAWING,             "DDERR_WASSTILLDRAWING",
    DDERR_XALIGN,                      "DDERR_XALIGN",
    DDERR_INVALIDDIRECTDRAWGUID,       "DDERR_INVALIDDIRECTDRAWGUID",
    DDERR_DIRECTDRAWALREADYCREATED,    "DDERR_DIRECTDRAWALREADYCREATED",
    DDERR_NODIRECTDRAWHW,              "DDERR_NODIRECTDRAWHW",
    DDERR_PRIMARYSURFACEALREADYEXISTS, "DDERR_PRIMARYSURFACEALREADYEXISTS",
    DDERR_NOEMULATION,                 "DDERR_NOEMULATION",
    DDERR_REGIONTOOSMALL,              "DDERR_REGIONTOOSMALL",
    DDERR_CLIPPERISUSINGHWND,          "DDERR_CLIPPERISUSINGHWND",
    DDERR_NOCLIPPERATTACHED,           "DDERR_NOCLIPPERATTACHED",
    DDERR_NOHWND,                      "DDERR_NOHWND",
    DDERR_HWNDSUBCLASSED,              "DDERR_HWNDSUBCLASSED",
    DDERR_HWNDALREADYSET,              "DDERR_HWNDALREADYSET",
    DDERR_NOPALETTEATTACHED,           "DDERR_NOPALETTEATTACHED",
    DDERR_NOPALETTEHW,                 "DDERR_NOPALETTEHW",
    DDERR_BLTFASTCANTCLIP,             "DDERR_BLTFASTCANTCLIP",
    DDERR_NOBLTHW,                     "DDERR_NOBLTHW",
    DDERR_NODDROPSHW,                  "DDERR_NODDROPSHW",
    DDERR_OVERLAYNOTVISIBLE,           "DDERR_OVERLAYNOTVISIBLE",
    DDERR_NOOVERLAYDEST,               "DDERR_NOOVERLAYDEST",
    DDERR_INVALIDPOSITION,             "DDERR_INVALIDPOSITION",
    DDERR_NOTAOVERLAYSURFACE,          "DDERR_NOTAOVERLAYSURFACE",
    DDERR_EXCLUSIVEMODEALREADYSET,     "DDERR_EXCLUSIVEMODEALREADYSET",
    DDERR_NOTFLIPPABLE,                "DDERR_NOTFLIPPABLE",
    DDERR_CANTDUPLICATE,               "DDERR_CANTDUPLICATE",
    DDERR_NOTLOCKED,                   "DDERR_NOTLOCKED",
    DDERR_CANTCREATEDC,                "DDERR_CANTCREATEDC",
    DDERR_NODC,                        "DDERR_NODC",
    DDERR_WRONGMODE,                   "DDERR_WRONGMODE",
    DDERR_IMPLICITLYCREATED,           "DDERR_IMPLICITLYCREATED",
    DDERR_NOTPALETTIZED,               "DDERR_NOTPALETTIZED",
    DDERR_UNSUPPORTEDMODE,             "DDERR_UNSUPPORTEDMODE"
};

/*******************************************************************
**
** Function: DrawError(HRESULT ddrval)
**
** Returns: void
**
** Description
**
** A helper routine to display correct Direct Draw debug messages.
**
** Created: chris.maughan@3Dlabs.com
**
*******************************************************************/
void
__wglDDrawError(const char *str, HRESULT ddrval)
{
    char *descriptor = "Unknown";
    char errorString[256];
    int i;

    for (i = 0; i < NUM_DDRAW_ERRORS; i++) {
	if (ddrval == DDRAWERR[i].dderval) {
	    descriptor = DDRAWERR[i].descriptor;
	    break;
	}
    }
    wsprintf(errorString,"WGL: %s: DDERROR = %s\n", str, descriptor);
    __wglMessage(errorString);
}
#endif /* __WGL_USE_DIRECTDRAW */
