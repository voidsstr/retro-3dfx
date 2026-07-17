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
** File name: d7fvfext.c
**
** Description: FVF support code for Software T&L HAL
**
** $Revision: 4$
** $Date: 10/11/00 8:49:42 PM$
**
** $Log: 
**  4    3dfx      1.2.2.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  3    Napalm Shared1.2         01/28/00 Scott Kephart   Big T&L Merge: FVF
**       handling changes
**  2    Napalm Shared1.1         10/27/99 Russ Lind       moved #include
**       "precomp.h" to be before the #ifdef TnL_HAL (as required for NT builds)
**  1    Napalm Shared1.0         10/26/99 Scott Kephart   
** $
 * 
 * 2     1/25/00 10:16p Skephart
 * BobJ Merge
*/

#include "precomp.h"

#ifdef TnL_HAL

#if( DX >= 6 )

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
#include "d7fvfext.h"


void InitFVFExtractor(DWORD qwFVFFlags, PFVFEXTDATA pFVFExtData)
{
   int       i;
   int       iFVFOffset;
   int       iTexND;
   int       iTexCoordCnt;
   
   memset((LPVOID)pFVFExtData, 0, sizeof(FVFEXTDATA) ); // Null out all of the pointers
   
   pFVFExtData->qwFVFFlags = qwFVFFlags;
   
   iFVFOffset = 0;
   
   switch (qwFVFFlags)
   {
   
      case D3DFVF_VERTEX:     // Untransformed/Unlit Vertices
      {
         pFVFExtData->iXYZ         = iFVFOffset;         
         pFVFExtData->iNorm        = iFVFOffset + 3;        
         pFVFExtData->iTexCoord[0] = iFVFOffset + 6;
         
         pFVFExtData->iFVFSize = 32;
         break;
      }
      
      case D3DFVF_LVERTEX:     // Unlit/Transformed Vertex
      {
         pFVFExtData->iXYZ         = iFVFOffset;
         pFVFExtData->iRes1        = iFVFOffset + 3;        
         pFVFExtData->iDiffuse     = iFVFOffset + 4;     
         pFVFExtData->iSpecular    = iFVFOffset + 5;    
         pFVFExtData->iTexCoord[0] = iFVFOffset + 6;
         
         pFVFExtData->iFVFSize = 32;
         
         break;
      }
   
      case D3DFVF_TLVERTEX:     // Transformed/Lit Vertex
      {
         pFVFExtData->iXYZ         = iFVFOffset;         
         pFVFExtData->iRHW         = iFVFOffset + 3;         
         pFVFExtData->iDiffuse     = iFVFOffset + 4;     
         pFVFExtData->iSpecular    = iFVFOffset + 5;    
         pFVFExtData->iTexCoord[0] = iFVFOffset + 6;
         
         pFVFExtData->iFVFSize = 32;
         
         break;
      }
   
      default:
      {
         //
         // This path is taken for lesser used FVF types.  Any path
         // that is common should be coded as a case above !!!
         //
         
         if(D3DFVF_RESERVED0 & qwFVFFlags)
         {                                   
            pFVFExtData->iRes0 = iFVFOffset;
            iFVFOffset++;    
         }
   
         if(D3DFVF_XYZ & qwFVFFlags)
         {
            pFVFExtData->iXYZ = iFVFOffset;
            iFVFOffset += 3;
            pFVFExtData->iRHW = 0;
         }
         else
         {
            if(D3DFVF_XYZRHW & qwFVFFlags)
            {
               pFVFExtData->iXYZ = iFVFOffset;
               iFVFOffset += 3;
               pFVFExtData->iRHW = iFVFOffset;
               iFVFOffset++;
            }
         }
   
         pFVFExtData->iBlendWts = 0; // Ignore this for now.
   
         if(D3DFVF_NORMAL & qwFVFFlags)
         {
            pFVFExtData->iNorm = iFVFOffset;
            iFVFOffset += 3;
         }
   
         if(D3DFVF_RESERVED1 & qwFVFFlags)
         {
            pFVFExtData->iRes1 = iFVFOffset;
            iFVFOffset++;  
         }
   
         if(D3DFVF_DIFFUSE & qwFVFFlags)
         {
            pFVFExtData->iDiffuse = iFVFOffset;
            iFVFOffset++;  
         }
   
         if(D3DFVF_SPECULAR & qwFVFFlags)
         {
            pFVFExtData->iSpecular = iFVFOffset;
            iFVFOffset++;  
         }
   
         iTexCoordCnt = (int)(( qwFVFFlags & D3DFVF_TEXCOUNT_MASK ) >> D3DFVF_TEXCOUNT_SHIFT);
         pFVFExtData->iTexCoordCnt = iTexCoordCnt;
   
         for(i = 0; i < iTexCoordCnt; i++)
         {
            switch (D3DFVF_GETTEXCOORDSIZE(qwFVFFlags, i))
            {
               case D3DFVF_TEXTUREFORMAT2:  iTexND = 2; break;
               case D3DFVF_TEXTUREFORMAT3:  iTexND = 3; break;
               case D3DFVF_TEXTUREFORMAT4:  iTexND = 4; break;
               case D3DFVF_TEXTUREFORMAT1:  iTexND = 1; break;
            }
            
            pFVFExtData->iTexCoord[i] = iFVFOffset;
            iFVFOffset+= iTexND;
            pFVFExtData->iS = iFVFOffset;
         }
         
         pFVFExtData->iFVFSize = iFVFOffset * sizeof(DWORD);
      }
   }
   
} // end InitFVFExtractor()  

#endif
#endif //TnL_HAL
