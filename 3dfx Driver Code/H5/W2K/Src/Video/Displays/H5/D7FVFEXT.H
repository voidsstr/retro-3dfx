/*
** Copyright (c) 1999, 3Dfx Interactive, Inc.
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
** File name: d7fvfext.h
**
** Description: FVF support for software T&L HAL
**
** $Revision: 3$
** $Date: 10/11/00 8:45:09 PM$
**
** $Log: 
**  3    3dfx      1.1.1.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  2    Napalm Shared1.1         01/28/00 Scott Kephart   Big T&L Merge: FVF
**       handling changes
**  1    Napalm Shared1.0         10/26/99 Scott Kephart   
** $
 * 
 * 2     1/25/00 10:16p Skephart
 * BobJ Merge
*/

#ifdef TnL_HAL
#ifndef __FVFEXT_H__
#define __FVFEXT_H__

#include "d3global.h"

typedef struct _tagFVFEXTDATA
{
   unsigned int qwFVFFlags;            // Original FVF Flags
   int    iFVFSize;              // Size of FVF Buffer in bytes
   int    iTexCoordCnt;          // number of textures in this FVF structure
   int    iRes0;                 // Offset to Reserved0 in FVF structure      
   int    iXYZ;                  // Offset to X,Y,Z in FVF structure
   int    iRHW;                  // Offset to RHW in FVF structure
   int    iBlendWts;             // Offset to matrix blend weights in FVF structure
   int    iNorm;                 // Offset to Normals in FVF structure
   int    iRes1;                 // Offset to Reserved1 field in FVF structure
   int    iDiffuse;              // Offset to Diffuse Color in FVF structure
   int    iSpecular;             // Offset to Specular Color in FVF structure
   int    iTexCoord[D3DHAL_TSS_MAXSTAGES+1]; // Offset to each of the 
                                 //texture coordinates in FVF structure
   int    iS;                    // Offset to S
   int    iEyeNorm;              // Offset to First Eye Normal
   int    iEyeXYZ;               // Offset to First Eye XYZ
} FVFEXTDATA, *PFVFEXTDATA;


//
// Macro definition for Extractors.  
// FDE is FVFDataExtractor, iTS is TextureStage, and iTC is Texture Coordinate (0 to 3)
// pV is a pointer to an FVF Data.
//

#define FVFEXT_GET_SIZE(FDE)           ((FDE).iFVFSize)

#define FVFEXT_GET_X(pV,FDE)           (D3DVALUE)*((LPD3DVALUE)(pV) + (FDE).iXYZ)
#define FVFEXT_GET_Y(pV,FDE)           (D3DVALUE)*((LPD3DVALUE)(pV) + (FDE).iXYZ + 1)
#define FVFEXT_GET_Z(pV, FDE)          (D3DVALUE)*((LPD3DVALUE)(pV) + (FDE).iXYZ + 2)
#define FVFEXT_GET_RHW(pV, FDE)        (D3DVALUE)*((LPD3DVALUE)(pV) + (FDE).iRHW)
#define FVFEXT_GETPTR_XYZ(pV, FDE)     (LPD3DVALUE)((LPD3DVALUE)(pV) + (FDE).iXYZ)

#define FVFEXT_GET_NORMX(pV, FDE)      (D3DVALUE)*((LPD3DVALUE)(pV) + (FDE).iNorm)
#define FVFEXT_GET_NORMY(pV, FDE)      (D3DVALUE)*((LPD3DVALUE)(pV) + (FDE).iNorm + 1)
#define FVFEXT_GET_NORMZ(pV, FDE)      (D3DVALUE)*((LPD3DVALUE)(pV) + (FDE).iNorm + 2)
#define FVFEXT_GETPTR_NORMXYZ(pV, FDE) (LPD3DVALUE)((LPD3DVALUE)(pV) + (FDE).iNorm)


#define FVFEXT_GET_RESERVED0(pV, FDE)  (DWORD)*((LPDWORD)(pV) + (FDE).iRes0)
#define FVFEXT_GET_RESERVED1(pV, FDE)  (DWORD)*((LPDWORD)(pV) + (FDE).iRes1)

#define FVFEXT_GET_DIFFUSE(pV, FDE)    (DWORD)*((LPDWORD)(pV) + (FDE).iDiffuse)
#define FVFEXT_GET_SPECULAR(pV, FDE)   (DWORD)*((LPDWORD)(pV) + (FDE).iSpecular)

#define FVFEXT_GET_TEX_COORD(FDE, iTS, iTC) (D3DVALUE)*((LPD3DVALUE)(pV) + (FDE).iTexCoord[iTexStage] + iTexCoord)
#define FVFEXT_GETPTR_TEX_COORD(FDE, iTS)   (LPD3DVALUE)((LPD3DVALUE)(pV) + (FDE).iTexCoord[iTexStage])

#define FVFEXT_GET_S(pV, FDE)          (D3DVALUE)((LPD3DVALUE)(pV) + (FDE).iS)
#define FVFEXT_GETPTR_S(pV, FDE)       (LPD3DVALUE)((LPD3DVALUE)(pV) + (FDE).iS)

#define FVFEXT_GET_EYENORM(pV, FDE, iTC) (D3DVALUE)*((LPD3DVALUE)(pV) + (FDE).iEyeNorm + iTC)
#define FVFEXT_GETPTR_EYENORM(pV, FDE)   (LPD3DVALUE)((LPD3DVALUE)(pV) + (FDE).iEyeNorm)

#define FVFEXT_GET_EYEXYZ(pV, FDE, iTC)  (D3DVALUE)*((LPD3DVALUE)(pV) + (FDE).iEyeXYZ + iTC)
#define FVFEXT_GETPTR_EYEXYZ(pV, FDE)    (LPD3DVALUE)((LPD3DVALUE)(pV) + (FDE).iEyeXYZ)


//
// Helper macro that determines how many Coordinates are in a Texture Stage
//

#define D3DFVF_GETTEXCOORDSIZE(FVF, CoordIndex) ((FVF >> (CoordIndex*2 + 16)) & 0x3)


//
// Function Prototypes
//

void InitFVFExtractor(DWORD, PFVFEXTDATA);

#endif

#endif //TnL_HAL