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
** $Revision: 5$
** $Date: 10/26/00 7:56:50 AM$
**
** $Log: 
*/

/**************************************************************************
* I N C L U D E S
***************************************************************************/

#include "precomp.h"

#if (DIRECT3D_VERSION >= 0x0800) && (DX >= 8)

#ifdef WINNT
/* V56K-DX8-PORT: the "everything NT needs is in precomp.h" claim below is
** not true for this file.  The dp2Idx* render-function table it selects
** from is declared in d6global.h, which precomp.h never pulls in, so every
** dp2Idx*All[_Orig] here came out as an undeclared identifier.  D7DP2.C
** already includes d3global.h explicitly for the same reason; d6global.h
** needs d3global.h ahead of it. */
#include "d3global.h"
#include "d6global.h"
#endif

#ifndef WINNT
// Everything NT builds need are in precomp.h!
#include "d6fvf.h"
#include "fxglobal.h"
#include "d3contxt.h"
#include "d3txtr.h"
#include "fifomgr.h"
#include "d3tri.h"
#include "d6global.h"
#include "d3contxt.h"
#endif

/**************************************************************************
* D E F I N E S
***************************************************************************/

/* V56K-DX8-PORT: the six K6_2 selections below chose the *_Orig render
** functions, which do not exist on NT -- d6global.h wraps every one of them
** in "#ifndef WINNT  // shouston - this is not implemented in WinNT."  So the
** guards are widened with !defined(WINNT) and NT takes the plain dp2Idx*
** entry points, which is what every other NT render path already uses. */
#define NORMAL_DBG_LEVEL    2

/**************************************************************************
* F U N C T I O N   P R O T O T Y P E S
***************************************************************************/
VOID _D3D_OP_MStream_SetSrc(RC *pRc, DWORD dwStream, DWORD dwVBHandle, DWORD dwStride);
VOID _D3D_OP_MStream_SetSrcUM(RC *pRc, DWORD dwStream, DWORD dwStride, LPDWORD lpVertices, DWORD dwVBSize);
VOID _D3D_OP_MStream_SetIndices(RC *pRc, DWORD dwVBHandle, DWORD dwStride);
VOID _D3D_OP_MStream_DrawPrim(RC *pRc, D3DPRIMITIVETYPE primType, DWORD VStart, DWORD PrimitiveCount);
VOID _D3D_OP_MStream_DrawIndxP(RC *pRc, D3DPRIMITIVETYPE primType, DWORD BaseVertexIndex, DWORD MinIndex, DWORD NumVertices, DWORD StartIndex, DWORD PrimitiveCount);
VOID _D3D_OP_MStream_DrawPrim2(RC *pRc, D3DPRIMITIVETYPE primType, DWORD FirstVertexOffset, DWORD PrimitiveCount);
VOID _D3D_OP_MStream_DrawIndxP2(RC *pRc, D3DPRIMITIVETYPE primType, INT BaseVertexOffset, DWORD MinIndex, DWORD NumVertices, DWORD StartIndexOffset, DWORD PrimitiveCount);
VOID _D3D_OP_MStream_ClipTriFan(RC *pRc, DWORD FirstVertexOffset, DWORD dwEdgeFlags, DWORD PrimitiveCount);
VOID _D3D_OP_MStream_DrawRectSurface(RC *pRc, DWORD Handle, DWORD Flags, PVOID lpPrim);
VOID _D3D_OP_MStream_DrawTriSurface(RC *pRc, DWORD Handle, DWORD Flags, PVOID lpPrim);
HRESULT _D3D_OP_VertexShader_Create(RC *pRc, DWORD dwVtxShaderHandle, DWORD dwDeclSize, DWORD dwCodeSize, BYTE *pShader);
VOID _D3D_OP_VertexShader_Delete(RC *pRc, DWORD dwVtxShaderHandle);
VOID _D3D_OP_VertexShader_Set(RC *pRc, DWORD dwVtxShaderHandle);
VOID _D3D_OP_VertexShader_SetConst(RC *pRc, DWORD dwRegister, DWORD dwConst, DWORD *pdwValues);
HRESULT _D3D_OP_PixelShader_Create(RC *pRc, DWORD dwPxlShaderHandle, DWORD dwCodeSize, BYTE *pShader);
VOID _D3D_OP_PixelShader_Delete(RC *pRc, DWORD dwPxlShaderHandle);
VOID _D3D_OP_PixelShader_Set(RC *pRc, DWORD dwPxlShaderHandle);
VOID _D3D_OP_PixelShader_SetConst(RC *pRc, DWORD dwRegister, DWORD dwCount, DWORD *pdwValues);

VOID UpdateCtxFVFChanges(DWORD pContext, DWORD dwOutputVertexType);

/**************************************************************************
* P U B L I C   F U N C T I O N S
***************************************************************************/
//-----------------------------------------------------------------------------
//
// _D3D_OP_MStream_SetSrc
//
// This function processes the D3DDP2OP_SETSTREAMSOURCE DP2 command token.
//
//-----------------------------------------------------------------------------
VOID _D3D_OP_MStream_SetSrc(RC *pRc, DWORD dwStream, DWORD dwVBHandle, DWORD dwStride)
{
    TXTRHNDL *pSrcStream;

    if (pRc->bSBRecMode)
    {
        if (dwStream == 0)
        {
              pRc->pCurrSB->uc.StreamSource[dwStream].dwStream = dwStream;
              pRc->pCurrSB->uc.StreamSource[dwStream].dwVBHandle = dwVBHandle;
              pRc->pCurrSB->uc.StreamSource[dwStream].dwStride = dwStride;

              SET_STATEBLOCK_STREAM_FLAG(pRc->pCurrSB, 0);
        }
        return;
    }

    if (dwVBHandle != 0)
    {
        if (dwStream == 0)
        {
            // Get the surface structure pointers for stream #0
            pSrcStream = TXTRHNDL_PTR(dwVBHandle);

            if (pSrcStream != NULL)
            {
                DISPDBG((DBGLVL,"Address of VB = 0x%x "
                                "dwVBHandle = %d , dwStride = %d",
                                pSrcStream->fpVidMem, dwVBHandle, dwStride));
                pRc->lpVertices = (LPDWORD) pSrcStream->fpVidMem;
                pRc->dwVerticesStride = dwStride >> 2;

                if (dwStride > 0)
                {
                    // DX8 has mixed types of vertices in one VB, size in bytes
                    // of the vertex buffer must be preserved
                    pRc->dwVBSizeInBytes = pSrcStream->lPitch;

                    // for VBs the wHeight should always be == 1.
                    // dwNumVertices stores the # of vertices in the VB
                    // On Win2K, both wWidth and lPitch are the buffer size
                    // On Win9x, only lPitch is the buffer size, wWidth is 0
                    // The same fact is also true for the index buffer
                    pRc->dwNumVertices = pSrcStream->lPitch / dwStride;

                    DISPDBG((DBGLVL,"dwVBHandle pContext->dwNumVertices = "
                                "pSrcStream->lPitch / dwStride = %d %d %d %d",
                                dwVBHandle,
                                pRc->dwNumVertices, 
                                pSrcStream->lPitch,dwStride));
                    
                    pRc->dwVBHandle = dwVBHandle;
                }
                else
                {
                    pRc->dwVBSizeInBytes = 0;
                    pRc->dwNumVertices = 0;
                    DISPDBG((ERRLVL,"INVALID Stride is 0. VB Size undefined"));
                }
            }
            else
            {
                DISPDBG((ERRLVL,"ERROR Address of VB is NULL, "
                                "dwStream = %d dwVBHandle = %d , dwStride = %d",
                                dwStream, dwVBHandle, dwStride));
            }
        }
        else
        {
            DISPDBG((WRNLVL,"We don't handle other streams than #0"));
        }
    }
    else
    {
        DISPDBG((WRNLVL,"Unsetting a stream: "
                        "dwStream = %d dwVBHandle = %d , dwStride = %d",
                        dwStream, dwVBHandle, dwStride));
    }
} // _D3D_OP_MStream_SetSrc

//-----------------------------------------------------------------------------
//
// _D3D_OP_MStream_SetSrcUM
//
// This function processes the D3DDP2OP_SETSTREAMSOURCEUM DP2 command token.
//
//-----------------------------------------------------------------------------
VOID _D3D_OP_MStream_SetSrcUM(RC *pRc, DWORD dwStream, DWORD dwVerticesStride, LPDWORD lpVertices, DWORD dwVBSize)
{
    if (dwStream == 0)
    {
        // Set the stream # 0 information
        DISPDBG((DBGLVL,"_D3D_OP_MStream_SetSrcUM: "
                        "Setting VB@ 0x%x dwstride=%d", pUMVtx, dwVerticesStride));
        pRc->lpVertices         = lpVertices;
        pRc->dwVerticesStride   = dwVerticesStride >> 2;
        pRc->dwVBSizeInBytes    = dwVBSize * dwVerticesStride;
        pRc->dwNumVertices      = dwVBSize; // comes from the DP2 data structure
    }
    else
    {
        DISPDBG((WRNLVL,"_D3D_OP_MStream_SetSrcUM: "
                        "We don't handle other streams than #0"));
    }
} // _D3D_OP_MStream_SetSrcUM

//-----------------------------------------------------------------------------
//
// _D3D_OP_MStream_SetIndices
//
// This function processes the D3DDP2OP_SETINDICES DP2 command token.
//
//-----------------------------------------------------------------------------             
VOID _D3D_OP_MStream_SetIndices(RC *pRc, DWORD dwIndexHandle, DWORD dwIndicesStride)
{
    TXTRHNDL *pIndxStream;
    
    if (pRc->bSBRecMode)
    {
        pRc->pCurrSB->uc.StreamIndices.dwVBHandle = dwIndexHandle;
        pRc->pCurrSB->uc.StreamIndices.dwStride = dwIndicesStride;

        SET_STATEBLOCK_STREAM_FLAG(pRc->pCurrSB, 8);
        return;
    }

    // NULL dwIndexHandle just means that the Index should be unset
    if (dwIndexHandle != 0)
    {
        // Get the indices surface structure pointer
        pIndxStream = TXTRHNDL_PTR(dwIndexHandle);

        if (pIndxStream)
        {
            DISPDBG((DBGLVL,"Address of IB = 0x%x", pIndxStream->fpVidMem));

            pRc->lpIndices          = (LPDWORD) pIndxStream->fpVidMem;
            pRc->dwIndicesStride    = dwIndicesStride;  // 2 or 4 for 16/32bit indices
            pRc->dwIndexHandle      = dwIndexHandle;    // Index buffer handle
        }
        else
        {
            DISPDBG((ERRLVL,"ERROR Address of Index Surface is NULL, "
                            "dwIndexHandle = %d , dwIndicesStride = %d",
                             dwIndexHandle, dwIndicesStride));
        }
    }
} // _D3D_OP_MStream_SetIndices

//-----------------------------------------------------------------------------
//
// _D3D_OP_MStream_DrawPrim
//
// This function processes the D3DDP2OP_DRAWPRIMITIVE DP2 command token.
//
//-----------------------------------------------------------------------------
VOID _D3D_OP_MStream_DrawPrim(RC *pRc, D3DPRIMITIVETYPE primType, DWORD VStart, DWORD PrimitiveCount)
{
    DISPDBG((DBGLVL,"_D3D_OP_MStream_DrawPrim "
               "primType=0x%x VStart=%d PrimitiveCount=%d", 
               primType, VStart, PrimitiveCount));

   _D3D_OP_MStream_DrawPrim2(pRc, 
                             primType,
                             VStart * fvfOffsetTable[pRc->dwVertexType].size,
                             PrimitiveCount);
} // _D3D_OP_MStream_DrawPrim

//-----------------------------------------------------------------------------
//
// _D3D_OP_MStream_DrawIndxP
//
// This function processes the D3DDP2OP_DRAWINDEXEDPRIMITIVE DP2 command token.
//
//-----------------------------------------------------------------------------
VOID _D3D_OP_MStream_DrawIndxP(RC *pRc, D3DPRIMITIVETYPE primType, DWORD BaseVertexIndex, DWORD MinIndex, DWORD NumVertices, DWORD StartIndex, DWORD PrimitiveCount)
{
    DISPDBG((DBGLVL,"_D3D_OP_MStream_DrawIndxP " 
               "primType=0x%x BaseVertexIndex=%d MinIndex=%d"
               "NumVertices =%d StartIndex=%d PrimitiveCount=%d", 
               primType, BaseVertexIndex, MinIndex,
               NumVertices, StartIndex, PrimitiveCount));

    _D3D_OP_MStream_DrawIndxP2(pRc, 
                               primType,
                               BaseVertexIndex * fvfOffsetTable[pRc->dwVertexType].size,
                               MinIndex,
                               NumVertices,
                               StartIndex * pRc->dwIndicesStride,
                               PrimitiveCount);
} // _D3D_OP_MStream_DrawIndxP

//-----------------------------------------------------------------------------
//
// _D3D_OP_MStream_DrawPrim2
//
// This function processes the D3DDP2OP_DRAWPRIMITIVE2 DP2 command token.
//
//-----------------------------------------------------------------------------
VOID _D3D_OP_MStream_DrawPrim2(RC *pRc, D3DPRIMITIVETYPE primType, DWORD FirstVertexOffset, DWORD PrimitiveCount)
{
//  BOOL bError;
    WORD wVStart;
    DWORD dwFillMode = (DWORD) pRc->fillMode;
    LPBYTE lpVertices;
    DWORD dwNumVertices;

    DISPDBG((DBGLVL ,"_D3D_OP_MStream_DrawPrim2 "
               "primType=0x%x FirstVertexOffset=%d PrimitiveCount=%d", 
               primType, FirstVertexOffset, PrimitiveCount));

    if ((pRc->dwVerticesStride == 0) || 
        (fvfOffsetTable[pRc->dwVertexType].size == 0))
    {
        DISPDBG((ERRLVL,"The zerot'h stream is doesn't have a valid VB set"));
        return;        
    }

    if (pRc->dwVerticesStride < fvfOffsetTable[pRc->dwVertexType].size)
    {
        DISPDBG((ERRLVL,"The stride set for the vertex stream is "
                        "less than the FVF vertex size"));
        return;
    }

    if (NULL == pRc->lpVertices)
    {
        DISPDBG((ERRLVL,"Pointer to vertex buffer is null"));
        return;       
    }

    lpVertices = ((LPBYTE)pRc->lpVertices) + FirstVertexOffset;
    dwNumVertices = pRc->dwVBSizeInBytes - FirstVertexOffset;
    dwNumVertices /= (pRc->dwVerticesStride << 2);
    wVStart = 0;
    
    switch(primType)
    {
        case D3DPT_POINTLIST:
            {
                D3DHAL_DP2POINTS dp2Points;
                dp2Points.wVStart = wVStart;
                dp2Points.wCount = (WORD)PrimitiveCount;
                
#if DX8_POINTSPRITES
                if(IS_POINSTPRITE_ACTIVE(pRc))
                {
/*                  _D3D_R3_DP2_PointsSprite(pRc,
                                       1,
                                       (LPBYTE)&dp2Points,
                                       (LPD3DTLVERTEX)lpVertices,
                                       dwNumVertices,
                                       &bError); */
                }
                else
#endif // DX8_POINTSPRITES
                {
/*                  _D3D_R3_DP2_Points(pRc,
                                       1,
                                       (LPBYTE)&dp2Points,
                                       (LPD3DTLVERTEX)lpVertices,
                                       dwNumVertices,
                                       &bError); */
                }  
            
            }
            break;
                
        case D3DPT_LINELIST:
            {
/*              _D3D_R3_DP2_LineList(pRc,
                                     (WORD)PrimitiveCount,
                                     (LPBYTE)&wVStart,
                                     (LPD3DTLVERTEX)lpVertices,
                                     dwNumVertices,
                                     &bError); */
            }
            break;        
            
        case D3DPT_LINESTRIP:
            {  
/*              _D3D_R3_DP2_LineStrip(pRc,
                                     (WORD)PrimitiveCount,
                                     (LPBYTE)&wVStart,
                                     (LPD3DTLVERTEX)lpVertices,
                                     dwNumVertices,
                                     &bError); */
            }
            break;   
            
        case D3DPT_TRIANGLELIST:        
        
            if (dwFillMode == D3DFILL_SOLID)
            {
                dp2TriangleAll(pRc, (WORD) PrimitiveCount, NULL, (LPDWORD) lpVertices, pRc->dwVertexType);
            }
            else
            {
                if (dwFillMode == D3DFILL_POINT)
                {
#if DX8_POINTSPRITES
                    if (IS_POINSTPRITE_ACTIVE(pRc))
                    {
/*                      _D3D_R3_DP2_TriangleListPointSprite(pRc,
                                                            (WORD)PrimitiveCount,
                                                            (LPBYTE)&wVStart,
                                                            (LPD3DTLVERTEX)lpVertices,
                                                            dwNumVertices,
                                                            &bError); */
                    }
                    else
#endif // DX8_POINTSPRITES   
                    {
                        dp2TriangleAllFill(pRc, (WORD) PrimitiveCount, NULL, (LPDWORD) lpVertices, pRc->dwVertexType);
                    }
                }
                else
                {
                    dp2TriangleAllFill(pRc, (WORD) PrimitiveCount, NULL, (LPDWORD) lpVertices, pRc->dwVertexType);
                }
            }
            break;  
            
        case D3DPT_TRIANGLESTRIP:
        
            if (dwFillMode == D3DFILL_SOLID)
            {
                dp2StripAll(pRc, (WORD) PrimitiveCount, NULL, (LPDWORD) lpVertices, pRc->dwVertexType);
            }
            else
            {
                if (dwFillMode == D3DFILL_POINT)
                {
#if DX8_POINTSPRITES
                    if (IS_POINSTPRITE_ACTIVE(pRc))
                    {
/*                      _D3D_R3_DP2_TriangleStripPointSprite(pRc,
                                                             (WORD)PrimitiveCount,
                                                             (LPBYTE)&wVStart,
                                                             (LPD3DTLVERTEX)lpVertices,
                                                             dwNumVertices,
                                                             &bError); */
                    }
                    else
#endif //DX8_POINTSPRITES
                    {
                        dp2StripAllFill(pRc, (WORD) PrimitiveCount, NULL, (LPDWORD) lpVertices, pRc->dwVertexType);
                    }
                }
                else
                {
                    dp2StripAllFill(pRc, (WORD) PrimitiveCount, NULL, (LPDWORD) lpVertices, pRc->dwVertexType);
                }
            }
            break; 
            
        case D3DPT_TRIANGLEFAN:
        
            if (dwFillMode == D3DFILL_SOLID)
            {
                dp2FanAll(pRc, (WORD) PrimitiveCount, NULL, (LPDWORD) lpVertices, pRc->dwVertexType);
            }
            else
            {
                if (dwFillMode == D3DFILL_POINT)
                {
#if DX8_POINTSPRITES
                    if (IS_POINSTPRITE_ACTIVE(pRc))
                    {
/*                      _D3D_R3_DP2_TriangleFanPointSprite(pRc,
                                                           (WORD)PrimitiveCount,
                                                           (LPBYTE)&wVStart,
                                                           (LPD3DTLVERTEX)lpVertices,
                                                           dwNumVertices,
                                                           &bError); */
                    } 
                    else
#endif // DX8_POINTSPRITES
                    {
                        dp2FanAllFill(pRc, (WORD) PrimitiveCount, NULL, (LPDWORD) lpVertices, pRc->dwVertexType);
                    }
                }
                else
                {
                    dp2FanAllFill(pRc, (WORD) PrimitiveCount, NULL, (LPDWORD) lpVertices, pRc->dwVertexType);
                }        
            }
            break;         
    }
} // _D3D_OP_MStream_DrawPrim2

//-----------------------------------------------------------------------------
//
// _D3D_OP_MStream_DrawIndxP2
//
// This function processes the D3DDP2OP_DRAWINDEXEDPRIMITIVE2 DP2 command token.
//
//-----------------------------------------------------------------------------
typedef VOID (*R3_DP2_PRIM_TYPE) (RC *pRc, DWORD count, LPVOID idx, LPDWORD vertices, DWORD vertexType);
VOID _D3D_OP_MStream_DrawIndxP2(RC *pRc, D3DPRIMITIVETYPE primType, INT BaseVertexOffset, DWORD MinIndex, DWORD NumVertices, DWORD StartIndexOffset, DWORD PrimitiveCount)
{
    INT      BaseIndexOffset;
    LPDWORD  lpVertices;
    LPBYTE   lpIndices;
    DWORD dwFillMode = (DWORD) pRc->fillMode;

    R3_DP2_PRIM_TYPE pRenderFunc;
    
    DISPDBG((DBGLVL,"_D3D_OP_MStream_DrawIndxP2 "
               "primType=0x%x BaseVertexOffset=%d MinIndex=%d "
               "NumVertices=%d StartIndexOffset=%d PrimitiveCount=%d", 
               primType, BaseVertexOffset, MinIndex,
               NumVertices, StartIndexOffset, PrimitiveCount));

    if ((pRc->dwVerticesStride == 0) || 
        (fvfOffsetTable[pRc->dwVertexType].size == 0))
    {
        DISPDBG((ERRLVL,"The zero'th stream is doesn't have a valid VB set"));
        return;        
    }

    if (pRc->dwVerticesStride < fvfOffsetTable[pRc->dwVertexType].size)
    {
        DISPDBG((ERRLVL,"The stride set for the vertex stream is "
                        "less than the FVF vertex size"));
        return;
    }

    if (NULL == pRc->lpIndices)
    {
        DISPDBG((ERRLVL,"Pointer to index buffer is null"));
        return;    
    }

    if (NULL == pRc->lpVertices)
    {
        DISPDBG((ERRLVL,"Pointer to vertex buffer is null"));
        return;       
    }

    // The MinIndex and NumVertices parameters specify the range of vertex i
    // ndices used for each DrawIndexedPrimitive call. These are used to 
    // optimize vertex processing of indexed primitives by processing a 
    // sequential range of vertices prior to indexing into these vertices

    // **********                IMPORTANT NOTE               **********
    //
    // BaseVertexOffset is a signed quantity (INT) unlike the other parameters
    // to this call which are DWORDS. This may appear strange. Why would
    // the offset into the vertex buffer be negative? Clearly you cannot access
    // vertex data before the start of the vertex buffer, and indeed, you never
    // do. When you have a negative BaseVertexOffset you will also receive
    // indices which are large enough that when applied to the start pointer
    // (obtained from adding a negative BaseVertexOffset to the vertex data
    // pointer) which fall within the correct range of the vertices in the
    // actual vertex buffer, i.e., the indices "undo" any negative vertex offset
    // and vertex accesses will end up being in the legal range for that vertex
    // buffer.
    //
    // Hence, you must write your driver code with this in mind. For example,
    // you can't assume that given an index i and with a current vertex buffer
    // of size v:
    //
    // ((StartIndexOffset + i) >= 0) && ((StartIndexOffset + i) < v)
    //
    // Your code needs to take into account that your indices are not offsets
    // from the start of the vertex buffer but rather from the start of the
    // vertex buffer plus BaseVertexOffset and that furthermore BaseVertexOffset
    // may be negative.
    //
    // The reason BaseVertexOffset can be negative is that it provides a
    // significant advantage to the runtime in certain vertex processing scenarios.

    lpVertices = (LPDWORD) ((LPBYTE) pRc->lpVertices + BaseVertexOffset);

    lpIndices =  (LPBYTE) pRc->lpIndices + StartIndexOffset;

    // Select the appropriate rendering function
    pRenderFunc = NULL;
    
    if (pRc->dwIndicesStride == 2)
    {   
        // Handle 16 bit indices
                                      
        switch(primType)
        {                    
            case D3DPT_LINELIST:
//              pRenderFunc = _D3D_DP2_IndexedLineList_16IND;
                break;        
                
            case D3DPT_LINESTRIP:
//              pRenderFunc = _D3D_DP2_IndexedLineStrip_16IND;            
                break;   
                
            case D3DPT_TRIANGLELIST:

                if (dwFillMode == D3DFILL_SOLID)
#if defined(K6_2) && !defined(WINNT)   /* V56K-DX8-PORT: K6-2/3DNow! */
                    pRenderFunc = dp2Idx16TriangleAll_Orig;
#else
                    pRenderFunc = dp2Idx16TriangleAll;
#endif
                else
                    pRenderFunc = dp2Idx16TriangleAllFill;

                break;
                
            case D3DPT_TRIANGLESTRIP:

                if (dwFillMode == D3DFILL_SOLID)
#if defined(K6_2) && !defined(WINNT)   /* V56K-DX8-PORT: K6-2/3DNow! */
                    pRenderFunc = dp2Idx16StripAll_Orig;
#else
                    pRenderFunc = dp2Idx16StripAll;
#endif
                else
                    pRenderFunc = dp2Idx16StripAllFill;

                break;            
                
            case D3DPT_TRIANGLEFAN:

                if (dwFillMode == D3DFILL_SOLID)
#if defined(K6_2) && !defined(WINNT)   /* V56K-DX8-PORT: K6-2/3DNow! */
                    pRenderFunc = dp2Idx16FanAll_Orig;
#else
                    pRenderFunc = dp2Idx16FanAll;
#endif
                else
                    pRenderFunc = dp2Idx16FanAllFill;

                break;                        
        }
    }
    else
    {
        // Handle 32 bit indices

        switch(primType)
        {                    
            case D3DPT_LINELIST:
//              pRenderFunc = _D3D_DP2_IndexedLineList_32IND;
                break;        
                
            case D3DPT_LINESTRIP:
//              pRenderFunc = _D3D_DP2_IndexedLineStrip_32IND;            
                break;   
                
            case D3DPT_TRIANGLELIST:

                if (dwFillMode == D3DFILL_SOLID)
#if defined(K6_2) && !defined(WINNT)   /* V56K-DX8-PORT: K6-2/3DNow! */
                    pRenderFunc = dp2Idx32TriangleAll_Orig;
#else
                    pRenderFunc = dp2Idx32TriangleAll;
#endif
                else
                    pRenderFunc = dp2Idx32TriangleAllFill;

                break;
                
            case D3DPT_TRIANGLESTRIP:

                if (dwFillMode == D3DFILL_SOLID)
#if defined(K6_2) && !defined(WINNT)   /* V56K-DX8-PORT: K6-2/3DNow! */
                    pRenderFunc = dp2Idx32StripAll_Orig;
#else
                    pRenderFunc = dp2Idx32StripAll;
#endif
                else
                    pRenderFunc = dp2Idx32StripAllFill;

                break;            
                
            case D3DPT_TRIANGLEFAN:

                if (dwFillMode == D3DFILL_SOLID)
#if defined(K6_2) && !defined(WINNT)   /* V56K-DX8-PORT: K6-2/3DNow! */
                    pRenderFunc = dp2Idx32FanAll_Orig;
#else
                    pRenderFunc = dp2Idx32FanAll;
#endif
                else
                    pRenderFunc = dp2Idx32FanAllFill;

                break;                        
        }        
    }

    // Call our rendering function
    if (pRenderFunc)
    {
        // As mentioned above, the actual range of indices seen by the driver
        // doesn't necessarily lie within the range zero to one less than
        // the number of vertices in the vertex buffer due to BaseVertexOffset.
        // If BaseVertexOffset is positive the range of valid indices is
        // smaller than the size of the vertex buffer (the vertices that
        // lie in the vertex buffer before the BaseVertexOffset are not
        // considered). Furthermore, if BaseVertexOffset a valid index can
        // actually by greater than the number of vertices in the vertex
        // buffer.
        //
        // To assist with the validation performed by the rendering functions
        // we here compute a minimum and maximum index which take into
        // account the value of BaseVertexOffset. Thus a test for a valid
        // index becomes:
        //
        // ((BaseIndexOffset + StartIndexOffset + Index) >= 0) &&
        // ((BaseIndexOffset + StartIndexOffset + Index) <  VertexCount)

        BaseIndexOffset = (BaseVertexOffset / (int) (pRc->dwVerticesStride << 2));

        DISPDBG((DBGLVL,"_D3D_OP_MStream_DrawIndxP2 BaseIndexOffset = %d",
            BaseIndexOffset));

        (*pRenderFunc)(pRc,
                       PrimitiveCount,
                       (LPBYTE)lpIndices,
                       (LPDWORD)lpVertices,
//                       BaseIndexOffset,
                       pRc->dwNumVertices);
    }
} // _D3D_OP_MStream_DrawIndxP2

//-----------------------------------------------------------------------------
//
// _D3D_OP_MStream_ClipTriFan
//
// This function processes the D3DDP2OP_CLIPPEDTRIANGLEFAN DP2 command token.
//
//-----------------------------------------------------------------------------
VOID _D3D_OP_MStream_ClipTriFan(RC *pRc, DWORD FirstVertexOffset, DWORD dwEdgeFlags, DWORD PrimitiveCount)
{   
//  BOOL bError;
    
    DISPDBG((DBGLVL,"_D3D_OP_MStream_ClipTriFan "
               "FirstVertexOffset=%d dwEdgeFlags=0x%x PrimitiveCount=%d", 
               FirstVertexOffset, dwEdgeFlags, PrimitiveCount));

    if (pRc->fillMode == D3DFILL_WIREFRAME)
    {
        D3DHAL_DP2TRIANGLEFAN_IMM dp2TriFanWire;

        dp2TriFanWire.dwEdgeFlags = dwEdgeFlags;

/*      _D3D_R3_DP2_TriangleFanImmWire(pRc,
                                       (WORD) PrimitiveCount,
                                       (LPBYTE) &dp2TriFanWire,
                                       (LPD3DTLVERTEX) pRc->lpVertices,
                                       pRc->dwNumVertices,
                                       &bError); */
    }
    else
    {
       _D3D_OP_MStream_DrawPrim2(pRc,
                                 D3DPT_TRIANGLEFAN,
                                 FirstVertexOffset,
                                 PrimitiveCount);
    }
} // _D3D_OP_MStream_ClipTriFan

//-----------------------------------------------------------------------------
//
// _D3D_OP_MStream_DrawRectPatch
//
// This function processes the D3DDP2OP_DRAWRECTPATCH DP2 command token.
//
//-----------------------------------------------------------------------------
VOID _D3D_OP_MStream_DrawRectPatch(RC *pRc, DWORD Handle, DWORD Flags, PVOID lpPrim)
{
    // High order surfaces are only supported for hw/drivers with
    // TnL support and 1.0 vertex shader support
    
} // _D3D_OP_MStream_DrawRectPatch

//-----------------------------------------------------------------------------
//
// _D3D_OP_MStream_DrawTriPatch
//
// This function processes the D3DDP2OP_DRAWTRIPATCH DP2 command token.
//
//-----------------------------------------------------------------------------                                     
VOID _D3D_OP_MStream_DrawTripatch(RC *pRc, DWORD Handle, DWORD Flags, PVOID lpPrim)
{
    // High order surfaces are only supported for hw/drivers with
    // TnL support and 1.0 vertex shader support

} // _D3D_OP_MStream_DrawTriPatch

//-----------------------------------------------------------------------------
//
// _D3D_OP_VertexShader_Create
//
// This function processes the D3DDP2OP_CREATEVERTEXSHADER DP2 command token.
//
//-----------------------------------------------------------------------------
HRESULT _D3D_OP_VertexShader_Create(RC *pRc, DWORD dwVtxShaderHandle, DWORD dwDeclSize, DWORD dwCodeSize, BYTE *pShader)
{
    // Here we would use the data passed by the vertex shader
    // creation block in order to instantiate or compile the
    // given vertex shader. Since this hardware can't support
    // vertex shaders at this time, we just skip the data.

    //
    
    return DD_OK;
    
} // _D3D_OP_VertexShader_Create

//-----------------------------------------------------------------------------
//
// _D3D_OP_VertexShader_Delete
//
// This function processes the D3DDP2OP_DELETEVERTEXSHADER DP2 command token.
//
//-----------------------------------------------------------------------------
VOID _D3D_OP_VertexShader_Delete(RC *pRc, DWORD dwVtxShaderHandle)
{
    // Here we would use the data passed by the vertex shader
    // delete block in order to destroy the given vertex shader.
    // Since this hardware can't support vertex shaders at 
    // this time, we just skip the data.
    
} // _D3D_OP_VertexShader_Delete

#define RDVSD_ISLEGACY(ShaderHandle) !(ShaderHandle & D3DFVF_RESERVED0)

//-----------------------------------------------------------------------------
//
// _D3D_OP_VertexShader_Set
//
// This function processes the D3DDP2OP_SETVERTEXSHADER DP2 command token.
//
//-----------------------------------------------------------------------------
VOID _D3D_OP_VertexShader_Set(RC *pRc, DWORD dwVtxShaderHandle)
{
    // Here we would use the data passed by the vertex shader
    // set block in order to setup the given vertex shader.
    // Since this hardware can't support vertex shaders at 
    // this time, we just skip the data.

    DISPDBG((DBGLVL,"Setting up shader # 0x%x",dwVtxShaderHandle));

    if (pRc->bSBRecMode)
    {
        pRc->pCurrSB->uc.dwShaderHandle = dwVtxShaderHandle;

        SET_STATEBLOCK_STREAM_FLAG(pRc->pCurrSB, 9);
        return;
    }

    // Zero is a special handle that tells the driver to
    // invalidate the currently set shader.
    if (dwVtxShaderHandle == 0)
    {
        DISPDBG((WRNLVL,"Invalidating the currently set shader"));
        return ;
    }    
    pRc->dwShaderHandle = dwVtxShaderHandle;

    if( RDVSD_ISLEGACY(dwVtxShaderHandle) )
    {
        // Make it parse the FVF 
        UpdateCtxFVFChanges((DWORD) pRc, dwVtxShaderHandle);
    }  
    else
    {
        DISPDBG((ERRLVL,"_D3D_OP_VertexShader_Set: Illegal shader handle "
                        "(This driver cant do vertex processing)"));
    }
    
} // _D3D_OP_VertexShader_Set

//-----------------------------------------------------------------------------
//
// _D3D_OP_VertexShader_SetConst
//
// This function processes the D3DDP2OP_SETVERTEXSHADERCONST DP2 command token.
//
//-----------------------------------------------------------------------------
VOID _D3D_OP_VertexShader_SetConst(RC *pRc, DWORD dwRegister, DWORD dwConst, DWORD *pdwValues)
{
    // Here we would use the data passed by the vertex shader
    // constant block in order to set up the constant entry.
    // Since this hardware can't support vertex shaders at 
    // this time, we just skip the data.
    
} // _D3D_OP_VertexShader_SetConst

//-----------------------------------------------------------------------------
//
// _D3D_OP_PixelShader_Create
//
// This function processes the D3DDP2OP_CREATEPIXELSHADER DP2 command token.
//
//-----------------------------------------------------------------------------
HRESULT _D3D_OP_PixelShader_Create(RC *pRc, DWORD dwPxlShaderHandle, DWORD dwCodeSize, BYTE *pShader)
{
    // Here we would use the data passed by the pixel shader
    // creation block in order to instantiate or compile the
    // given pixel shader. 

    // Since this hardware can't support pixel shaders at this 
    // time, we fail the call in case we're called to create a
    // 255.255 version shader!

    return D3DERR_DRIVERINVALIDCALL;
    
} // _D3D_OP_PixelShader_Create

//-----------------------------------------------------------------------------
//
// _D3D_OP_PixelShader_Delete
//
// This function processes the D3DDP2OP_DELETEPIXELSHADER DP2 command token.
//
//-----------------------------------------------------------------------------
VOID _D3D_OP_PixelShader_Delete(RC *pRc, DWORD dwPxlShaderHandle)
{
    // Here we would use the data passed by the pixel shader
    // delete block in order to destroy the given pixel shader.
    // Since this hardware can't support pixel shaders at 
    // this time, we just skip the data.
    
} // _D3D_OP_PixelShader_Delete

//-----------------------------------------------------------------------------
//
// _D3D_OP_PixelShader_Set
//
// This function processes the D3DDP2OP_SETPIXELSHADER DP2 command token.
//
//-----------------------------------------------------------------------------
VOID _D3D_OP_PixelShader_Set(RC *pRc, DWORD dwPxlShaderHandle)
{
    // Here we would use the data passed by the pixel shader
    // set block in order to setup the given pixel shader.
    // Since this hardware can't support pixel shaders at 
    // this time, we just skip the data.
    
} // _D3D_OP_PixelShader_Set

//-----------------------------------------------------------------------------
//
// _D3D_OP_PixelShader_SetConst
//
// This function processes the D3DDP2OP_SETPIXELSHADERCONST DP2 command token.
//
//-----------------------------------------------------------------------------
VOID _D3D_OP_PixelShader_SetConst(RC *pRc, DWORD dwRegister, DWORD dwCount, DWORD *pdwValues)
{
    // Here we would use the data passed by the pixel shader
    // set block in order to setup the given pixel shader constants.
    // Since this hardware can't support pixel shaders at 
    // this time, we just skip the data.
    
} // _D3D_OP_PixelShader_SetConst

/**************************************************************************
* S T A T I C   F U N C T I O N S
***************************************************************************/

#endif //DX8

