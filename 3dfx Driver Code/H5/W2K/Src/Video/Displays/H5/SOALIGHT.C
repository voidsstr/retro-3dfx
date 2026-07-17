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
** File name: soalight.c
**
** Description: Lighting Code for the Pentium III / Structure of Array format Vertex Buffers
**
** $Revision: 24$
** $Date: 10/11/00 8:45:31 PM$
**
** $Log: 
**  24   3dfx      1.22.2.0    10/11/00 Brent           Forced check in to enforce
**       branching.
**  23   Napalm    1.22        04/24/00 Scott Kephart   Fixed 3D Mark 2000 lighting
**       bug - "Stripes" in game 1
**  22   Napalm    1.21        04/19/00 Scott Kephart   Big lighting change - Part
**       I
**       Lighting is now split into two parts, diffuse and specular. 
**  21   Napalm    1.20        03/17/00 Scott Kephart   Added support for Visual
**       C++ processor pack Beta
**  20   Napalm    1.19        03/14/00 Scott Kephart   Fog fixes, got rid of
**       specular alpha in lighting
**  19   Napalm    1.18        03/08/00 Scott Kephart   Re-shuffle of T&L code.
**  18   Napalm    1.17        03/01/00 Allen Hansen    Fixed vertex fog for T&L
**  17   Napalm    1.16        02/28/00 Allen Hansen    Added vertex fog code (it's
**       rem'd out by a return right now)
**  16   Napalm    1.15        02/28/00 Scott Kephart   Lighting fixes. Fixed
**       specular on point lights, added support for dvFalloff on spotlights.
**  15   Napalm    1.14        02/25/00 Scott Kephart   Spotlights work! (As long
**       as dvFalloff == 1.0)
**  14   Napalm    1.13        02/25/00 Scott Kephart   Early out on dot product in
**       TLLV_PointSOA() -- This is a safer bet than it seems!
**  13   Napalm    1.12        02/25/00 Scott Kephart   Added emms to the end of
**       Xform_Light_4Vertex_SOA
**  12   Napalm    1.11        02/25/00 Scott Kephart   Added needed emms
**       instruction to LightVertexSOA
**  11   Napalm    1.10        02/23/00 Scott Kephart   Specular now works for
**       directional and point lights
**  10   Napalm    1.9         02/22/00 Scott Kephart   Minor cleanup
**  9    Napalm    1.8         02/16/00 Scott Kephart   Point lights work. No
**       specular yet.
**  8    Napalm    1.7         02/15/00 Scott Kephart   Added Diffuse Alpha
**  7    Napalm    1.6         02/15/00 Scott Kephart   Updates for SOA lighting
**  6    Napalm    1.5         02/10/00 Scott Kephart   Data structure cleanup for
**       SOA.H -- we're unionized now!
**  5    Napalm    1.4         02/08/00 Scott Kephart   Really simple device
**       coordinate transform
**  4    Napalm    1.3         02/08/00 Scott Kephart   Updated formatting
**  3    Napalm    1.2         02/08/00 Scott Kephart   Added SOA_ARGB_F2I
**  2    Napalm    1.1         02/01/00 Scott Kephart   More lighting changes.
**       Better SSE matrix multiply code.
**  1    Napalm    1.0         01/28/00 Scott Kephart   
** $
** 
** 2     3/12/00 5:21p Skephart
** Fixed FogVertexSOA to use dFog and fFog
** 
** 1     3/08/00 9:26p Skephart
 * 
 * 2     1/24/00 10:40p Skephart
 * Skeleton for SOA T&L
 * 
 * 1     1/19/00 9:49p Skephart
 * Beginnings of SOA T&L code
*/

#include "precomp.h"

#if( DX >= 7 )
#ifdef TnL_HAL

#ifdef SSECPP

#ifndef WINNT
#include <d3dhal.h>
#include "d6fvf.h"
#include "fxglobal.h"
#include "d3contxt.h"
#include "d3txtr.h"
#include "fifomgr.h"
#include "d3tri.h"
#include "d6global.h"
#include "d3contxt.h"
#endif

#include "dxins.h"

void LightVertexSOA0(RC* pRc);
void LightVertexSOA1_DS(RC* pRc);
void LightVertexSOA2_DS(RC* pRc);
void LightVertexSOA3_DS(RC* pRc);
void LightVertexSOA4_DS(RC* pRc);
void LightVertexSOA5_DS(RC* pRc);
void LightVertexSOA6_DS(RC* pRc);
void LightVertexSOA7_DS(RC* pRc);
void LightVertexSOA8_DS(RC* pRc);

void LightVertexSOA1_D(RC* pRc);
void LightVertexSOA2_D(RC* pRc);
void LightVertexSOA3_D(RC* pRc);
void LightVertexSOA4_D(RC* pRc);
void LightVertexSOA5_D(RC* pRc);
void LightVertexSOA6_D(RC* pRc);
void LightVertexSOA7_D(RC* pRc);
void LightVertexSOA8_D(RC* pRc);

// EAX must point to TL_TMP
#define LIGHT_SETUP_DIFFUSE                                     \
__asm {movaps    xmm5, [eax]_STL.fambEmiss.red};                \
__asm {movaps    xmm6, [eax]_STL.fambEmiss.green};              \
__asm {movaps    xmm7, [eax]_STL.fambEmiss.blue};               \
__asm {movaps    [eax]_STL.fDiffuse.red, xmm5};                 \
__asm {movaps    [eax]_STL.fDiffuse.green, xmm6};               \
__asm {movaps    [eax]_STL.fDiffuse.blue, xmm7};

#define LIGHT_SETUP_DIFFUSE2                                    \
__asm {movaps    xmm5, [eax]_STL.fambEmiss.red};                \
__asm {movaps    xmm6, [eax]_STL.fambEmiss.green};              \
__asm {movaps    xmm7, [eax]_STL.fambEmiss.blue};               


#define LIGHT_SAVE_DIFFUSE                                      \
__asm  {mov eax, pTLD};                                         \
__asm  {movaps  [eax]_STL.fDiffuse.red, xmm5};                  \
__asm  {movaps  [eax]_STL.fDiffuse.green, xmm6};                \
__asm  {movaps  [eax]_STL.fDiffuse.blue, xmm7};                 \


// EAX must point to TL_TMP
#define LIGHT_SETUP_SPECULAR                                    \
__asm  {xorps     xmm0, xmm0};                                  \
__asm  {movaps    [eax]_STL.fSpecular.red, xmm0};               \
__asm  {movaps    [eax]_STL.fSpecular.green, xmm0};             \
__asm  {movaps    [eax]_STL.fSpecular.blue, xmm0};

// EAX must point to TL_TMP
#define LIGHT_SETUP_SPECULAR2                                   \
__asm  {xorps     xmm5, xmm5};                                  \
__asm  {xorps     xmm6, xmm6};                                  \
__asm  {xorps     xmm7, xmm7};                                  

#define LIGHT_SAVE_SPECULAR                                     \
__asm  {mov eax, pTLD};                                         \
__asm  {movaps  [eax]_STL.fSpecular.red, xmm5};                 \
__asm  {movaps  [eax]_STL.fSpecular.green, xmm6};               \
__asm  {movaps  [eax]_STL.fSpecular.blue, xmm7};                \

TLLIGHTFN pLight_Old[] =  {
  &LightVertexSOA0,
  &LightVertexSOA1_Old,
  &LightVertexSOA2_Old,
  &LightVertexSOA3_Old,
  &LightVertexSOA4_Old,
  &LightVertexSOA5_Old,
  &LightVertexSOA6_Old,
  &LightVertexSOA7_Old,
  &LightVertexSOA8_Old,
};

TLLIGHTFN pLight_DS[] =  {
  &LightVertexSOA0,
  &LightVertexSOA1_DS,
  &LightVertexSOA2_DS,
  &LightVertexSOA3_DS,
  &LightVertexSOA4_DS,
  &LightVertexSOA5_DS,
  &LightVertexSOA6_DS,
  &LightVertexSOA7_DS,
  &LightVertexSOA8_DS,
};


TLLIGHTFN pLight_D[] =  {
  &LightVertexSOA0,
  &LightVertexSOA1_D,
  &LightVertexSOA2_D,
  &LightVertexSOA3_D,
  &LightVertexSOA4_D,
  &LightVertexSOA5_D,
  &LightVertexSOA6_D,
  &LightVertexSOA7_D,
  &LightVertexSOA8_D,
};
/*-------------------------------------------------------------------
Function Name:  LightVertexSOA0
Description:    Apply no Light to a vertex
Parameters:   
                RC *pRC -- pointer to the rendering context
Information:    
Return:         
-------------------------------------------------------------------*/
ALIGN32 void LightVertexSOA0(RC* pRc)
{
  TL_TMP *pTLD = pRc->tl.pTL;
  __asm
  {
    mov       eax, pTLD           // point to TL_TMP
    movaps    xmm0, [eax]_STL.fambEmiss.red
    movaps    xmm1, [eax]_STL.fambEmiss.green
    movaps    xmm2, [eax]_STL.fambEmiss.blue

    movaps    [eax]_STL.fDiffuse.red, xmm0 
    movaps    [eax]_STL.fDiffuse.green, xmm1 
    movaps    [eax]_STL.fDiffuse.blue, xmm2 

    xorps     xmm0, xmm0
    movaps    [eax]_STL.fSpecular.red, xmm0
    movaps    [eax]_STL.fSpecular.green, xmm0
    movaps    [eax]_STL.fSpecular.blue, xmm0
  }
}  

/*-------------------------------------------------------------------
Function Name:  LightVertexSOA1_Old
Description:    Apply 1 Light to a vertex with diffuse + specular
Parameters:   
                RC *pRC -- pointer to the rendering context
Information:    
Return:         
-------------------------------------------------------------------*/
ALIGN32 void LightVertexSOA1_Old(RC* pRc)
{
  TLLIGHTING *Ldata = &pRc->tl.lighting;
  TL_TMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pLightArray;

  __asm  {
    mov       eax, pTLD           // point to TL_TMP            
  }
  LIGHT_SETUP_DIFFUSE
  LIGHT_SETUP_SPECULAR
  (pLA[0]->pfnLightVertexSOA)(pRc, pLA[0]);

}  

/*-------------------------------------------------------------------
Function Name:  LightVertexSOA2_Old
Description:    Apply 2 Lights to a vertex with diffuse + specular
Parameters:   
                RC *pRC -- pointer to the rendering context
Information:    
Return:         
-------------------------------------------------------------------*/
ALIGN32 void LightVertexSOA2_Old(RC* pRc)
{
  TLLIGHTING *Ldata = &pRc->tl.lighting;
  TL_TMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pLightArray;

  __asm  {
    mov       eax, pTLD           // point to TL_TMP            
  }
  LIGHT_SETUP_DIFFUSE
  LIGHT_SETUP_SPECULAR

  (pLA[0]->pfnLightVertexSOA)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA)(pRc, pLA[1]);

}



/*-------------------------------------------------------------------
Function Name:  LightVertexSOA3_Old
Description:    Apply 3 Lights to a vertex with Diffuse
Parameters:   
                RC *pRC -- pointer to the rendering context
Information:    
Return:         
-------------------------------------------------------------------*/
ALIGN32 void LightVertexSOA3_Old(RC* pRc)
{
  TLLIGHTING *Ldata = &pRc->tl.lighting;
  TL_TMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pLightArray;

  __asm  {
    mov       eax, pTLD           // point to TL_TMP            
  }
  LIGHT_SETUP_DIFFUSE
  LIGHT_SETUP_SPECULAR

  (pLA[0]->pfnLightVertexSOA)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA)(pRc, pLA[2]);

}  

/*-------------------------------------------------------------------
Function Name:  LightVertexSOA4_Old
Description:    Apply 4 Lights to a vertex with Diffuse
Parameters:   
                RC *pRC -- pointer to the rendering context
Information:    
Return:         
-------------------------------------------------------------------*/
ALIGN32 void LightVertexSOA4_Old(RC* pRc)
{
  TLLIGHTING *Ldata = &pRc->tl.lighting;
  TL_TMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pLightArray;

  __asm  {
    mov       eax, pTLD           // point to TL_TMP            
  }
  LIGHT_SETUP_DIFFUSE
  LIGHT_SETUP_SPECULAR

  (pLA[0]->pfnLightVertexSOA)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA)(pRc, pLA[3]);


}  

/*-------------------------------------------------------------------
Function Name:  LightVertexSOA5_Old
Description:    Apply 5 Lights to a vertex with Diffuse
Parameters:   
                RC *pRC -- pointer to the rendering context
Information:    
Return:         
-------------------------------------------------------------------*/
ALIGN32 void LightVertexSOA5_Old(RC* pRc)
{
  TLLIGHTING *Ldata = &pRc->tl.lighting;
  TL_TMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pLightArray;

  __asm  {
    mov       eax, pTLD           // point to TL_TMP            
  }
  LIGHT_SETUP_DIFFUSE
  LIGHT_SETUP_SPECULAR

  (pLA[0]->pfnLightVertexSOA)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA)(pRc, pLA[4]);

}  


/*-------------------------------------------------------------------
Function Name:  LightVertexSOA6_Old
Description:    Apply 6 Lights to a vertex with Diffuse
Parameters:   
                RC *pRC -- pointer to the rendering context
Information:    
Return:         
-------------------------------------------------------------------*/
ALIGN32 void LightVertexSOA6_Old(RC* pRc)
{
  TLLIGHTING *Ldata = &pRc->tl.lighting;
  TL_TMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pLightArray;

  __asm  {
    mov       eax, pTLD           // point to TL_TMP            
  }
  LIGHT_SETUP_DIFFUSE
  LIGHT_SETUP_SPECULAR

  (pLA[0]->pfnLightVertexSOA)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA)(pRc, pLA[5]);


}  

/*-------------------------------------------------------------------
Function Name:  LightVertexSOA7_Old
Description:    Apply 7 Lights to a vertex with Diffuse
Parameters:   
                RC *pRC -- pointer to the rendering context
Information:    
Return:         
-------------------------------------------------------------------*/
ALIGN32 void LightVertexSOA7_Old(RC* pRc)
{
  TLLIGHTING *Ldata = &pRc->tl.lighting;
  TL_TMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pLightArray;

  __asm  {
    mov       eax, pTLD           // point to TL_TMP            
  }
  LIGHT_SETUP_DIFFUSE
  LIGHT_SETUP_SPECULAR

  (pLA[0]->pfnLightVertexSOA)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA)(pRc, pLA[6]);

}  

/*-------------------------------------------------------------------
Function Name:  LightVertexSOA8_Old
Description:    Apply 8 Lights to a vertex with Diffuse
Parameters:   
                RC *pRC -- pointer to the rendering context
Information:    
Return:         
-------------------------------------------------------------------*/
ALIGN32 void LightVertexSOA8_Old(RC* pRc)
{
  TLLIGHTING *Ldata = &pRc->tl.lighting;
  TL_TMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pLightArray;

  __asm  {
    mov       eax, pTLD           // point to TL_TMP            
  }
  LIGHT_SETUP_DIFFUSE
  LIGHT_SETUP_SPECULAR

  (pLA[0]->pfnLightVertexSOA)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA)(pRc, pLA[7]);

}  


/*-------------------------------------------------------------------
Function Name:  LightVertexSOA1_D
Description:    Apply 1 Light to a vertex with Diffuse
Parameters:   
                RC *pRC -- pointer to the rendering context
Information:    
Return:         
-------------------------------------------------------------------*/
ALIGN32 void LightVertexSOA1_D(RC* pRc)
{
  TLLIGHTING *Ldata = &pRc->tl.lighting;
  TL_TMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pLightArray;

  __asm  {
    mov       eax, pTLD           // point to TL_TMP            
  }
  LIGHT_SETUP_DIFFUSE2
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  LIGHT_SAVE_DIFFUSE

}  

/*-------------------------------------------------------------------
Function Name:  LightVertexSOA2_D
Description:    Apply 2 Lights to a vertex with Diffuse
Parameters:   
                RC *pRC -- pointer to the rendering context
Information:    
Return:         
-------------------------------------------------------------------*/
ALIGN32 void LightVertexSOA2_D(RC* pRc)
{
  TLLIGHTING *Ldata = &pRc->tl.lighting;
  TL_TMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pLightArray;

  __asm  {
    mov       eax, pTLD           // point to TL_TMP            
  }
  LIGHT_SETUP_DIFFUSE2

  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  LIGHT_SAVE_DIFFUSE

}



/*-------------------------------------------------------------------
Function Name:  LightVertexSOA3_D
Description:    Apply 3 Lights to a vertex with Diffuse
Parameters:   
                RC *pRC -- pointer to the rendering context
Information:    
Return:         
-------------------------------------------------------------------*/
ALIGN32 void LightVertexSOA3_D(RC* pRc)
{
  TLLIGHTING *Ldata = &pRc->tl.lighting;
  TL_TMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pLightArray;

  __asm  {
    mov       eax, pTLD           // point to TL_TMP            
  }
  LIGHT_SETUP_DIFFUSE2

  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  LIGHT_SAVE_DIFFUSE

}  

/*-------------------------------------------------------------------
Function Name:  LightVertexSOA4_D
Description:    Apply 4 Lights to a vertex with Diffuse
Parameters:   
                RC *pRC -- pointer to the rendering context
Information:    
Return:         
-------------------------------------------------------------------*/
ALIGN32 void LightVertexSOA4_D(RC* pRc)
{
  TLLIGHTING *Ldata = &pRc->tl.lighting;
  TL_TMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pLightArray;

  __asm  {
    mov       eax, pTLD           // point to TL_TMP            
  }
  LIGHT_SETUP_DIFFUSE2

  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  LIGHT_SAVE_DIFFUSE


}  

/*-------------------------------------------------------------------
Function Name:  LightVertexSOA5_D
Description:    Apply 5 Lights to a vertex with Diffuse
Parameters:   
                RC *pRC -- pointer to the rendering context
Information:    
Return:         
-------------------------------------------------------------------*/
ALIGN32 void LightVertexSOA5_D(RC* pRc)
{
  TLLIGHTING *Ldata = &pRc->tl.lighting;
  TL_TMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pLightArray;

  __asm  {
    mov       eax, pTLD           // point to TL_TMP            
  }
  LIGHT_SETUP_DIFFUSE2

  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  LIGHT_SAVE_DIFFUSE

}  


/*-------------------------------------------------------------------
Function Name:  LightVertexSOA6_D
Description:    Apply 6 Lights to a vertex with Diffuse
Parameters:   
                RC *pRC -- pointer to the rendering context
Information:    
Return:         
-------------------------------------------------------------------*/
ALIGN32 void LightVertexSOA6_D(RC* pRc)
{
  TLLIGHTING *Ldata = &pRc->tl.lighting;
  TL_TMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pLightArray;

  __asm  {
    mov       eax, pTLD           // point to TL_TMP            
  }
  LIGHT_SETUP_DIFFUSE2

  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);

  LIGHT_SAVE_DIFFUSE

}  

/*-------------------------------------------------------------------
Function Name:  LightVertexSOA7_D
Description:    Apply 7 Lights to a vertex with Diffuse
Parameters:   
                RC *pRC -- pointer to the rendering context
Information:    
Return:         
-------------------------------------------------------------------*/
ALIGN32 void LightVertexSOA7_D(RC* pRc)
{
  TLLIGHTING *Ldata = &pRc->tl.lighting;
  TL_TMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pLightArray;

  __asm  {
    mov       eax, pTLD           // point to TL_TMP            
  }
  LIGHT_SETUP_DIFFUSE2

  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  LIGHT_SAVE_DIFFUSE

}  

/*-------------------------------------------------------------------
Function Name:  LightVertexSOA8_D
Description:    Apply 8 Lights to a vertex with Diffuse
Parameters:   
                RC *pRC -- pointer to the rendering context
Information:    
Return:         
-------------------------------------------------------------------*/
ALIGN32 void LightVertexSOA8_D(RC* pRc)
{
  TLLIGHTING *Ldata = &pRc->tl.lighting;
  TL_TMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pLightArray;

  __asm  {
    mov       eax, pTLD           // point to TL_TMP            
  }
  LIGHT_SETUP_DIFFUSE2

  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  LIGHT_SAVE_DIFFUSE

}  




/*-------------------------------------------------------------------
Function Name:  LightVertexSOA1_DS
Description:    Apply 1 Light to a vertex with both Diffuse and Specular
Parameters:   
                RC *pRC -- pointer to the rendering context
Information:    
Return:         
-------------------------------------------------------------------*/
ALIGN32 void LightVertexSOA1_DS(RC* pRc)
{
  TLLIGHTING *Ldata = &pRc->tl.lighting;
  TL_TMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pLightArray;

  __asm  {
    mov       eax, pTLD           // point to TL_TMP            
  }
  LIGHT_SETUP_DIFFUSE2
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  LIGHT_SAVE_DIFFUSE

  __asm  {
    mov       eax, pTLD           // point to TL_TMP            
  }
  LIGHT_SETUP_SPECULAR2
  if (pLA[0]->dwNonZeroDot)
    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  LIGHT_SAVE_SPECULAR

}  

/*-------------------------------------------------------------------
Function Name:  LightVertexSOA2_DS
Description:    Apply 2 Lights to a vertex with both Diffuse and Specular
Parameters:   
                RC *pRC -- pointer to the rendering context
Information:    
Return:         
-------------------------------------------------------------------*/
ALIGN32 void LightVertexSOA2_DS(RC* pRc)
{
  TLLIGHTING *Ldata = &pRc->tl.lighting;
  TL_TMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pLightArray;

  __asm  {
    mov       eax, pTLD           // point to TL_TMP            
  }
  LIGHT_SETUP_DIFFUSE2

  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  LIGHT_SAVE_DIFFUSE

  pLA = pRc->tl.lighting.pLightArray;
  __asm  {
    mov       eax, pTLD           // point to TL_TMP            
  }
  LIGHT_SETUP_SPECULAR2
  if (pLA[0]->dwNonZeroDot)
    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);

  if (pLA[1]->dwNonZeroDot)
    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  LIGHT_SAVE_SPECULAR

}



/*-------------------------------------------------------------------
Function Name:  LightVertexSOA3_DS
Description:    Apply 3 Lights to a vertex with both Diffuse and Specular
Parameters:   
                RC *pRC -- pointer to the rendering context
Information:    
Return:         
-------------------------------------------------------------------*/
ALIGN32 void LightVertexSOA3_DS(RC* pRc)
{
  TLLIGHTING *Ldata = &pRc->tl.lighting;
  TL_TMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pLightArray;

  __asm  {
    mov       eax, pTLD           // point to TL_TMP            
  }
  LIGHT_SETUP_DIFFUSE2

  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  LIGHT_SAVE_DIFFUSE

  pLA = pRc->tl.lighting.pLightArray;
  __asm  {
    mov       eax, pTLD           // point to TL_TMP            
  }
  LIGHT_SETUP_SPECULAR2
  if (pLA[0]->dwNonZeroDot)
    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);

  if (pLA[1]->dwNonZeroDot)
    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);

  if (pLA[2]->dwNonZeroDot)
    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  LIGHT_SAVE_SPECULAR

}  

/*-------------------------------------------------------------------
Function Name:  LightVertexSOA4_DS
Description:    Apply 4 Lights to a vertex with both Diffuse and Specular
Parameters:   
                RC *pRC -- pointer to the rendering context
Information:    
Return:         
-------------------------------------------------------------------*/
ALIGN32 void LightVertexSOA4_DS(RC* pRc)
{
  TLLIGHTING *Ldata = &pRc->tl.lighting;
  TL_TMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pLightArray;

  __asm  {
    mov       eax, pTLD           // point to TL_TMP            
  }
  LIGHT_SETUP_DIFFUSE2

  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);

  LIGHT_SAVE_DIFFUSE

  pLA = pRc->tl.lighting.pLightArray;
  __asm  {
    mov       eax, pTLD           // point to TL_TMP            
  }
  LIGHT_SETUP_SPECULAR2
  if (pLA[0]->dwNonZeroDot)
    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);

  if (pLA[1]->dwNonZeroDot)
    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);

  if (pLA[2]->dwNonZeroDot)
    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);

  if (pLA[3]->dwNonZeroDot)
    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);
  LIGHT_SAVE_SPECULAR

}  

/*-------------------------------------------------------------------
Function Name:  LightVertexSOA5_DS
Description:    Apply 5 Lights to a vertex with both Diffuse and Specular
Parameters:   
                RC *pRC -- pointer to the rendering context
Information:    
Return:         
-------------------------------------------------------------------*/
ALIGN32 void LightVertexSOA5_DS(RC* pRc)
{
  TLLIGHTING *Ldata = &pRc->tl.lighting;
  TL_TMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pLightArray;

  __asm  {
    mov       eax, pTLD           // point to TL_TMP            
  }
  LIGHT_SETUP_DIFFUSE2

  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);

  LIGHT_SAVE_DIFFUSE

  pLA = pRc->tl.lighting.pLightArray;
  __asm  {
    mov       eax, pTLD           // point to TL_TMP            
  }
  LIGHT_SETUP_SPECULAR2
  if (pLA[0]->dwNonZeroDot)
    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);

  if (pLA[1]->dwNonZeroDot)
    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);

  if (pLA[2]->dwNonZeroDot)
    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);

  if (pLA[3]->dwNonZeroDot)
    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);

  if (pLA[4]->dwNonZeroDot)
    (pLA[4]->pfnLightVertexSOA_Spec)(pRc, pLA[4]);
  LIGHT_SAVE_SPECULAR


}  


/*-------------------------------------------------------------------
Function Name:  LightVertexSOA6_DS
Description:    Apply 6 Lights to a vertex with both Diffuse and Specular
Parameters:   
                RC *pRC -- pointer to the rendering context
Information:    
Return:         
-------------------------------------------------------------------*/
ALIGN32 void LightVertexSOA6_DS(RC* pRc)
{
  TLLIGHTING *Ldata = &pRc->tl.lighting;
  TL_TMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pLightArray;

  __asm  {
    mov       eax, pTLD           // point to TL_TMP            
  }
  LIGHT_SETUP_DIFFUSE2

  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);

  LIGHT_SAVE_DIFFUSE

  pLA = pRc->tl.lighting.pLightArray;
  __asm  {
    mov       eax, pTLD           // point to TL_TMP            
  }
  LIGHT_SETUP_SPECULAR2
  if (pLA[0]->dwNonZeroDot)
    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);

  if (pLA[1]->dwNonZeroDot)
    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);

  if (pLA[2]->dwNonZeroDot)
    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);

  if (pLA[3]->dwNonZeroDot)
    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);

  if (pLA[4]->dwNonZeroDot)
    (pLA[4]->pfnLightVertexSOA_Spec)(pRc, pLA[4]);

  if (pLA[5]->dwNonZeroDot)
    (pLA[5]->pfnLightVertexSOA_Spec)(pRc, pLA[5]);
  LIGHT_SAVE_SPECULAR


}  

/*-------------------------------------------------------------------
Function Name:  LightVertexSOA7_DS
Description:    Apply 7 Lights to a vertex with both Diffuse and Specular
Parameters:   
                RC *pRC -- pointer to the rendering context
Information:    
Return:         
-------------------------------------------------------------------*/
ALIGN32 void LightVertexSOA7_DS(RC* pRc)
{
  TLLIGHTING *Ldata = &pRc->tl.lighting;
  TL_TMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pLightArray;

  __asm  {
    mov       eax, pTLD           // point to TL_TMP            
  }
  LIGHT_SETUP_DIFFUSE2

  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);

  LIGHT_SAVE_DIFFUSE

  pLA = pRc->tl.lighting.pLightArray;
  __asm  {
    mov       eax, pTLD           // point to TL_TMP            
  }
  LIGHT_SETUP_SPECULAR2
  if (pLA[0]->dwNonZeroDot)
    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);

  if (pLA[1]->dwNonZeroDot)
    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);

  if (pLA[2]->dwNonZeroDot)
    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);

  if (pLA[3]->dwNonZeroDot)
    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);

  if (pLA[4]->dwNonZeroDot)
    (pLA[4]->pfnLightVertexSOA_Spec)(pRc, pLA[4]);

  if (pLA[5]->dwNonZeroDot)
    (pLA[5]->pfnLightVertexSOA_Spec)(pRc, pLA[5]);

  if (pLA[6]->dwNonZeroDot)
    (pLA[6]->pfnLightVertexSOA_Spec)(pRc, pLA[6]);
  LIGHT_SAVE_SPECULAR



}  

/*-------------------------------------------------------------------
Function Name:  LightVertexSOA8_DS
Description:    Apply 8 Lights to a vertex with both Diffuse and Specular
Parameters:   
                RC *pRC -- pointer to the rendering context
Information:    
Return:         
-------------------------------------------------------------------*/
ALIGN32 void LightVertexSOA8_DS(RC* pRc)
{
  TLLIGHTING *Ldata = &pRc->tl.lighting;
  TL_TMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pLightArray;

  __asm  {
    mov       eax, pTLD           // point to TL_TMP            
  }
  LIGHT_SETUP_DIFFUSE2

  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  LIGHT_SAVE_DIFFUSE

  pLA = pRc->tl.lighting.pLightArray;
  __asm  {
    mov       eax, pTLD           // point to TL_TMP            
  }
  LIGHT_SETUP_SPECULAR2
  if (pLA[0]->dwNonZeroDot)
    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);

  if (pLA[1]->dwNonZeroDot)
    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);

  if (pLA[2]->dwNonZeroDot)
    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);

  if (pLA[3]->dwNonZeroDot)
    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);

  if (pLA[4]->dwNonZeroDot)
    (pLA[4]->pfnLightVertexSOA_Spec)(pRc, pLA[4]);

  if (pLA[5]->dwNonZeroDot)
    (pLA[5]->pfnLightVertexSOA_Spec)(pRc, pLA[5]);

  if (pLA[6]->dwNonZeroDot)
    (pLA[6]->pfnLightVertexSOA_Spec)(pRc, pLA[6]);

  if (pLA[7]->dwNonZeroDot)
    (pLA[7]->pfnLightVertexSOA_Spec)(pRc, pLA[7]);
  LIGHT_SAVE_SPECULAR

}  

/*-------------------------------------------------------------------
Function Name:  LightVertexSOA
Description:    Apply all lights to a vertex
Parameters:     
                RC *pRC -- pointer to the rendering context
Information:    
                Currently a maximum of 8 lights are supported. This is
                hard-wired into the code to reduce the number of 
                expensive branch mispredictions in the code.

Return:         
-------------------------------------------------------------------*/
ALIGN32 void LightVertexSOA(RC* pRc)
{
  TLLIGHTING *Ldata = &pRc->tl.lighting;
  TLLIGHT  *pLight;
  TL_TMP *pTLD = pRc->tl.pTL;


  // We'll need to add color vertex support here later

  // Initialize the diffuse color

  // Initialize the diffuse and specular color
  //pTLD->fSpecular = 0.0;
  //pTLD->fDiffuse = pTLD->fambEmiss;
  __asm
  {
    mov       eax, pTLD           // point to TL_TMP
    movaps    xmm0, [eax]_STL.fambEmiss.red
    movaps    xmm1, [eax]_STL.fambEmiss.green
    movaps    xmm2, [eax]_STL.fambEmiss.blue
//    movaps    xmm3, [eax]_XTL.fambEmiss.alpha
    movaps    [eax]_STL.fDiffuse.red, xmm0 
    movaps    [eax]_STL.fDiffuse.green, xmm1 
    movaps    [eax]_STL.fDiffuse.blue, xmm2 
//    movaps    [eax]_XTL.fDiffuse.alpha, xmm3 

    xorps     xmm0, xmm0
    movaps    [eax]_STL.fSpecular.red, xmm0
    movaps    [eax]_STL.fSpecular.green, xmm0
    movaps    [eax]_STL.fSpecular.blue, xmm0
//    movaps    [eax]_XTL.fSpecular.alpha, xmm0
  }

  //
  // In a loop accumulate color from the activated lights
  //
  pLight = Ldata->pActiveLights;

  // Light 0
  if (pLight)
  {
    if (pLight->dwFlags & TLLIGHT_READY)
      pLight->pfnLightVertexSOA(pRc,
          pLight);

        pLight = pLight->Next;
  }
  else
  {
    goto LightFinished;
  }


  // Light 1
  if (pLight)
  {
    if (pLight->dwFlags & TLLIGHT_READY)
      pLight->pfnLightVertexSOA(pRc,
          pLight);


        pLight = pLight->Next;
  }
  else
  {
    goto LightFinished;
  }

  // Light 2
  if (pLight)
  {
    if (pLight->dwFlags & TLLIGHT_READY)
      pLight->pfnLightVertexSOA(pRc,
          pLight);


        pLight = pLight->Next;
  }
  else
  {
    goto LightFinished;
  }

  // Light 3
  if (pLight)
  {
    if (pLight->dwFlags & TLLIGHT_READY)
      pLight->pfnLightVertexSOA(pRc,
          pLight);


        pLight = pLight->Next;
  }
  else
  {
    goto LightFinished;
  }


  // Light 4
  if (pLight)
  {
    if (pLight->dwFlags & TLLIGHT_READY)
      pLight->pfnLightVertexSOA(pRc,
          pLight);


        pLight = pLight->Next;
  }
  else
  {
    goto LightFinished;
  }

  // Light 5
  if (pLight)
  {
    if (pLight->dwFlags & TLLIGHT_READY)
      pLight->pfnLightVertexSOA(pRc,
          pLight);


        pLight = pLight->Next;
  }
  else
  {
    goto LightFinished;
  }

  // Light 6
  if (pLight)
  {
    if (pLight->dwFlags & TLLIGHT_READY)
    goto LightFinished;
  }

  // Light 7
  if (pLight)
  {
    if (pLight->dwFlags & TLLIGHT_READY)
      pLight->pfnLightVertexSOA(pRc,
          pLight);


        pLight = pLight->Next;
  }

LightFinished:;


}


/*-------------------------------------------------------------------
Function Name:  FogVertexSOA
Description:    Performs fogging calculation on the input vertex
                Alpha component of pv->lighting.outSpecular is set
Parameters:   
                RC *pRC -- pointer to the rendering context
Information:    
Return:         
-------------------------------------------------------------------*/

#define FOG_HIGH_PRECISION_RSQRT
#define CLAMP_FOG

void FogVertexSOA(RC* pRc)
{
  TL_TMP *pTL = pRc->tl.pTL;
  //return;   // Disable this whole thing for now

  //
  // Calculate the distance
  //
  // Vertex is already transformed to the camera space
  if (pRc->tl.dwTLState & TLPV_RANGEFOG) {
    _asm {
      mov     eax, pTL
      movaps  xmm0, [eax]_STL.cv.x
      movaps  xmm1, [eax]_STL.cv.y
      movaps  xmm2, [eax]_STL.cv.z
      mulps   xmm0, xmm0              // x*x
      mulps   xmm1, xmm1              // y*y
      mulps   xmm2, xmm2              // z*z
      addps   xmm0, xmm1              // x*x + y*y
      addps   xmm0, xmm2              // x*x + y*y + z*z = d
      rsqrtps   xmm1, xmm0      ; 1/sqrt(d)
#ifdef FOG_HIGH_PRECISION_RSQRT
          // 0.5 * rsqrtps * (3 - x * rsqrtps(x) * rsqrtps(x))
      movaps  xmm3, [TL_soa_0pt5]     // 0.5f
      mulps   xmm0, xmm1              // d * 1/sqrt(d)
      movaps  xmm2, [TL_soa_neg_3]    // -3.0f
      mulps   xmm3, xmm1              // 1/2 * 1/sqrt(d)
      mulps   xmm0, xmm1              // d * 1/sqrt(d) * 1/sqrt(d)
      addps   xmm0, xmm2              // 3 - d * 1/sqrt(d) * 1/sqrt(d)
      mulps   xmm0, xmm3              // 1/2 * 1/sqrt(d) * (3 - d * 1/sqrt(d) * 1/sqrt(d))
#endif
      rcpps   xmm0, xmm1              // sqrt(d)
    }
  }
  else {
    _asm {
      mov       eax, pTL
      movaps    xmm0, [eax]_STL.cv.z
    }
  }

  /*
  if (pRc->tl.lighting.fog_mode == D3DFOG_LINEAR) {
  if (dist < pRc->tl.lighting.fog_start)      RRPV_SET_ALPHA(pRc->tl.lighting.outSpecular, 255);
  else if (dist >= pRc->tl.lighting.fog_end)  RRPV_SET_ALPHA(pRc->tl.lighting.outSpecular, 0);
  else  RRPV_SET_ALPHA(pRc->tl.lighting.outSpecular, (int)((pRc->tl.lighting.fog_end - dist) * pRc->tl.lighting.fog_factor));
  } else {
  D3DVALUE tmp = dist * pRc->tl.lighting.fog_density;
  if (pRc->tl.lighting.fog_mode == D3DFOG_EXP2)
  tmp *= tmp;
  RRPV_SET_ALPHA( pRc->tl.lighting.outSpecular, (int) (exp(-tmp) * 255.0f) )
  }
  */


  if (pRc->tl.lighting.fog_mode == D3DFOG_LINEAR) {
    _asm {
      mov     edx, pRc
      movss   xmm1, [edx]RC.tl.lighting.fog_start
      movss   xmm2, [edx]RC.tl.lighting.fog_end
      movss   xmm3, [edx]RC.tl.lighting.fog_factor
      shufps  xmm1, xmm1, 0                       // broadcast start
      shufps  xmm2, xmm2, 0                       // broadcast end
      shufps  xmm3, xmm3, 0                       // broadcast fog_factor
      movaps  xmm4, xmm1
      movaps  xmm5, xmm2

      // clear the fog_factor if it's out of range
      cmpps   xmm1, xmm0, SSE_LT                  // (dist < start) ? 0 : -1
      cmpps   xmm2, xmm0, SSE_GE                  // (dist >= end ) ? 0 : -1
      andps   xmm3, xmm1                          // (dist < start) ? 0 : fog_factor
      andps   xmm3, xmm2                          // ((dist < start) || (dist >= end)) ? 0 : fog_factor  

      // if fog is in range, fog = (end - dist) * fog_factor (else this result will be zero)
      subps   xmm5, xmm0                          // end - dist
      mulps   xmm5, xmm3                          // (end - dist) * (((dist < start) || (dist >= end)) ? 0 : fog_factor)

      // if (dist < start) fog = 255
      cmpps   xmm0, xmm4, SSE_LT                  // (dist < start) ? -1 : 0
      andps   xmm0, [TL_soa_255]                  // (dist < start) ? 255 : 0
      orps    xmm0, xmm5                          // (dist < start) ? 255 : (end - dist) * ((dist >= end) ? 0 : fog_factor)
    }
  }
  else {
    _asm {
      mov     edx, pRc
      movss   xmm1, [edx]RC.tl.lighting.fog_density // density
      shufps  xmm1, xmm1, 0
      mulps   xmm0, xmm1                          // distance * density
      movaps  xmm2, [TL_hi_bits]                  // 0x80000000

      test    [edx]RC.tl.lighting.materialDiffAlpha, D3DFOG_EXP2
      jne     SoaFogNotExp2

      mulps   xmm0, xmm0                          // (distance * density)^2

SoaFogNotExp2:
      xorps   xmm0, xmm2                          // -(distance * density)^2
      call    KniExp                              // xmm0 = exp(xmm0)
      movaps  xmm1, [TL_soa_255]
      mulps   xmm0, xmm1
    }
    /*
    int f;
    D3DVALUE tmp = dist * pRc->tl.lighting.fog_density;
    if (pRc->tl.lighting.fog_mode == D3DFOG_EXP2)
    {
    tmp *= tmp;
    }
    tmp = (D3DVALUE)exp(-tmp) * 255.0f;
    f = FTOI(tmp);
    RRPV_SET_ALPHA( pRc->tl.lighting.outSpecular, f )
    }
    */
  }

  /* Write the fog value back out (do we have to clamp to 255???) */
  _asm
  {
#ifdef CLAMP_FOG
    xorps     xmm1, xmm1                          // all zero's
    movaps    xmm2, [TL_soa_255]                  // 255.0
    maxps     xmm0, xmm1                          // clamp negative numbers to zero
    minps     xmm0, xmm2                          // clamp over +255 to 255
#endif
    mov       eax, pRc                            // eax = pointer to pRc
    mov       edx, [eax]RC.tl.pTL                 // edx = pointer to pRc->tl
    movaps    [edx]_STL.fFog, xmm0

    // Convert to int.
    cvtps2pi  mm0, xmm0
    shufps    xmm0, xmm0, 0xe
    cvtps2pi  mm1, xmm0
    pslld     mm0, 24                             // move alpha to hi byte
    pslld     mm1, 24                             // move alpha to hi byte
    movq    [edx]_STL.dFog.m.lo, mm0
    movq    [edx]_STL.dFog.m.hi, mm1


  }

  _asm emms // not needed but I can't shut up the Intel compiler warning otherwise
} // end of FogVertexSOA()
/*-------------------------------------------------------------------
Function Name:  SOA_ARGB_F2I
Description:    Convert float ARGB to integer in SOA format
Parameters:   
Information:    
Return:         
-------------------------------------------------------------------*/

__declspec(align(32)) void SOA_ARGB_F2I(SOA_RGBA *pin, SOA_DWORD *pout)
{
  _asm 
  {
    mov       eax, pin
    mov       ebx, pout

    pxor      mm6, mm6
    movlps    xmm0, [eax]_SC4.alpha.m.lo
    cvttps2pi mm0, xmm0
    movlps    xmm1, [eax]_SC4.alpha.m.hi
    cvttps2pi mm1, xmm1

    movlps    xmm2, [eax]_SC4.red.m.lo
    cvttps2pi mm4, xmm2
    movlps    xmm3, [eax]_SC4.red.m.hi
    cvttps2pi mm3, xmm3

    packuswb  mm0, mm1        //mm0 = 00 A3 00 A2 00 A1 00 A0
    packuswb  mm0, mm6        //mm0 = 00 00 00 00 A3 A2 A1 A0 

    packuswb  mm4, mm3        //mm4 = 00 R3 00 R2 00 R1 00 R0
    packuswb  mm4, mm6        //mm4 = 00 00 00 00 R3 R2 R1 R0 
    punpcklbw mm4, mm0        //mm4 = A3 R3 A2 R2 A1 R1 A0 R0
  
    movlps    xmm0, [eax]_SC4.green.m.lo
    cvttps2pi mm0, xmm0
    movlps    xmm1, [eax]_SC4.green.m.hi
    cvttps2pi mm1, xmm1

    movlps    xmm2, [eax]_SC4.blue.m.lo
    cvttps2pi mm2, xmm2
    movlps    xmm3, [eax]_SC4.blue.m.hi
    cvttps2pi mm3, xmm3

    packuswb  mm0, mm1        //mm0 = 00 G3 00 G2 00 G1 00 G0
    packuswb  mm0, mm6        //mm0 = 00 00 00 00 G3 G2 G1 G0 
    packuswb  mm2, mm3        //mm2 = 00 B3 00 B2 00 B1 00 B0
    packuswb  mm2, mm6        //mm2 = 00 00 00 00 B3 B2 B1 B0 
    punpcklbw mm2, mm0        //mm2 = G3 B3 G2 B2 G1 B1 G0 B0
    movq      mm3, mm2        //mm5 = G3 B3 G2 B2 G1 B1 G0 B0

    punpcklwd mm2, mm4        //mm2 = A1 R1 G1 B1 A0 R0 G0 B0

    punpckhwd mm3, mm4        //mm3 = A3 R3 G3 B3 A2 R2 G2 B2
    movq      [ebx]MMX2.lo, mm2
    movq      [ebx]MMX2.hi, mm3
    emms
  }  
}  

/*-------------------------------------------------------------------
Function Name:  taylorpowSSE
Description:    Fairly inaccurate pow() function for SSE
Parameters:     xmm0 = base
Information:    
Return:         xmm0 = pow(fbase, fexp);
-------------------------------------------------------------------*/

__declspec(align(32)) __m128 taylorpowSSE( float *fexp)
{
/*
   float temp;
   float mb1, mb2, mb3, mb5, mb7, mb9;
   float t1, t2, t3, t4, t5;
   
   mb1 = (fbase - 1.0f)/(fbase + 1.0);
   mb2 = mb1 * mb1;
   mb3 = mb2 * mb1;
   mb5 = mb3 * mb2;
   mb7 = mb5 * mb2;
   mb9 = mb7 * mb2;

   temp = 2.0f*( mb1 + mb3*(1.0f/3.0f) + mb5*(1.0f/5.0f) + mb7*(1.0f/7.0f) + mb9*(1.0f/9.0f) );

   temp = -fexp * temp;
   
   t1 = temp;
   t2 = t1 * temp;
   t3 = t2 * temp;
   t4 = t3 * temp;
   t5 = t4 * temp;
   
   fresult = 1.0f + t1 + (t2*(1.0f/2.0f)) + (t3*(1.0f/6.0f)) + 
      (t4*(1.0f/24.0f)) +(t5*(1.0f/120.0f));
   
   return(1.0f/fresult);
*/
//   float  *tmpBase, *tmpExp, *tmpRes;
   
//   tmpExp  = (float *)fexp;
//   tmpBase = (float *)fbase;
//   tmpRes  = (float *)fresult;
   
   _asm 
   {
      mov    edx, [fexp]
//      mov    eax, [fbase]
      xorps   xmm6, xmm6
      
      movaps  xmm1, xmm0          // xmm0 = xmm1 = base
      cmpneqps xmm6, xmm0
      subps  xmm0, [TL_soa_1]     // xmm0 = fbase - 1.0f
      addps  xmm1, [TL_soa_1]     // xmm1 = fbase + 1.0f
      rcpps  xmm2, xmm1           // xmm2 = 1.0f/(fbase + 1.0)
      mulps  xmm0, xmm2           // xmm0 = mb1 = (fbase - 1.0f)/(fbase + 1.0f)
      
      movaps xmm7, xmm0           // xmm7 = mb1
      mulps  xmm7, xmm0           // xmm7 = mb2 = mb1 * mb1
    
      movaps xmm1, xmm7           // xmm1 = mb2
      mulps  xmm1, xmm0           // xmm1 = mb3 = mb2 * mb1
      movaps xmm2, xmm1           // xmm2 = mb3
      mulps  xmm2, xmm7           // xmm2 = mb5 = mb3 * mb2
      movaps xmm3, xmm2           // xmm3 = mb5
      mulps  xmm3, xmm7           // xmm3 = mb7 = mb5 * mb2
      movaps xmm4, xmm3           // xmm4 = mb7
      mulps  xmm4, xmm7           // xmm4 = mb9 = mb7 * mb2

      mulps  xmm1, [TL_soa_r3]    // xmm1 = mb3/3.0f
      mulps  xmm2, [TL_soa_r5]    // xmm2 = mb5/5.0f
      mulps  xmm3, [TL_soa_r7]    // xmm3 = mb7/7.0f
      mulps  xmm4, [TL_soa_r9]    // xmm4 = mb9/9.0f
      addps  xmm0, xmm1           // xmm0 = mb1 + mb3/3.0f
      addps  xmm0, xmm2           // xmm0 = mb1 + mb3/3.0f + mb5/5.0f
      addps  xmm0, xmm3           // xmm0 = mb1 + mb3/3.0f + mb5/5.0f + mb7/7.0f
      addps  xmm0, xmm4           // xmm0 = mb1 + mb3/3.0f + mb5/5.0f + mb7/7.0f + mb9/9.0f
      mulps  xmm0, [TL_soa_2]     // xmm0 = ln(fbase) = 2.0f*(mb1 + mb3/3.0f + mb5/5.0f + 
                                  //                       mb7/7.0f + mb9/9.0f)
                                    
      movss xmm7, [edx]           // xmm7 = fexp
      shufps  xmm7, xmm7, 0       // broadcast
      
      mulps  xmm0, xmm7           // xmm0 = fexp * ln(fbase)
      
      //
      // We can negate fexp*ln(fbase) here because it will make the
      // exp portion of the code converge more quickly.  This trick
      // only works because 0.0 < fbase < 1.0 which makes the ln always 
      // negative.  Remember to fix the negation you need to take the 
      // reciprical of the exp at the end.
      //
      
      mulps  xmm0, [TL_soa_neg_1] // xmm0 = temp = t1 = -fexp * ln(fbase)
      movaps xmm1, xmm0           // xmm1 = t1  
      mulps  xmm1, xmm0           // xmm1 = t2 = t1 * temp
      movaps xmm2, xmm1           // xmm2 = t2 
      mulps  xmm2, xmm0           // xmm2 = t3 = t2 * temp
      movaps xmm3, xmm2           // xmm3 = t3
      mulps  xmm3, xmm0           // xmm3 = t4 = t3 * temp
      movaps xmm4, xmm3           // xmm4 = t4
      mulps  xmm4, xmm0           // xmm4 = t5 = t4 * temp
      
      mulps  xmm1, [TL_soa_r2f]   // xmm1 = t2/2!
      mulps  xmm2, [TL_soa_r3f]   // xmm2 = t3/3!
      mulps  xmm3, [TL_soa_r4f]   // xmm3 = t4/4!
      mulps  xmm4, [TL_soa_r5f]   // xmm4 = t5/5!
      
      addps  xmm0, [TL_soa_1]     // xmm0 = 1 + t1
      addps  xmm0, xmm1           // xmm0 = 1 + t1 + t2/2!
      addps  xmm0, xmm2           // xmm0 = 1 + t1 + t2/2! + t3/3! 
      addps  xmm0, xmm3           // xmm0 = 1 + t1 + t2/2! + t3/3! + t4/4!
      addps  xmm0, xmm4           // xmm0 = 1 + t1 + t2/2! + t3/3! + t4/4! + t5/5!

      rcpps  xmm1, xmm0           // xmm1 = fresult = 1/xmm0 to remove negation
      movaps xmm0, xmm1
      andps xmm0, xmm6
      
   }
}

#endif
#endif
#endif