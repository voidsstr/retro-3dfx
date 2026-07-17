/*
** Copyright (c) 1996-2000, 3Dfx Interactive, Inc.
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
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
**
** File name:   swapbuf.c
**
** Description: Functions for swapping buffers to the display surface
**
** $Log: 
**  21   3dfx      1.13.1.6    10/12/00 Don Fowler      Added type CSDCID for OS
**       independent device contexts
**       Changed all ExtEscape calls to happen from the DC of the desktop because
**       no clients currently known will be multi-monitor compatible.
**  20   3dfx      1.13.1.5    10/11/00 Brent           Forced check in to enforce
**       branch.
**  19   3dfx      1.13.1.4    10/11/00 Ryan Bissell    Modifications needed for
**       OGL.
**  18   3dfx      1.13.1.3    10/09/00 Don Fowler      Removed // DWF left in from
**       previous build 
**  17   3dfx      1.13.1.2    10/09/00 Don Fowler      Changed debug system to get
**       errors from the server when the error code comes from the server. The
**       error code defines are stored in the respecitve servers header files. Call
**       csDEBUGGetErrorString to retrieve the verbose string from the server
**  16   3dfx      1.13.1.1    10/09/00 Don Fowler      Removed #ifdef
**       ENABLE_ESCAPE since we will be working with an ESCAPE enabled library from
**       now on
**  15   3dfx      1.13.1.0    07/06/00 Ryan Bissell    Incremental changes, and
**       bug fixes.
**  14   3dfx      1.13        03/19/00 Don Fowler      Added the
**       CSALLOCATIONDESCRIPTOR.u32Locale so that the client can specify
**       CS_LOCALE_FRAMEBUFFER  or CS_LOCALE_AGP separate from tiled or linear
**       memory types.
** 
**       Changed all of the error codes in the API from _ERROR_ to _APIERROR_ so
**       that server codes can have a different base 
** 
**  13   3dfx      1.12        03/16/00 Don Fowler      Removed u32DestColorFormat
**       from CSSWAPBUFFERTODISPLAY and fixed a bug with a memcopy in
**       csSwapBufferToDisplay
**  12   3dfx      1.11        03/09/00 Don Fowler      Modified include file
**       structure to separate out a server-specific file and a client-specific
**       file. 
**  11   3dfx      1.10        03/08/00 Don Fowler      Changed FxBool to FxU32
**       because of 16 bit structure alignment problems.
**  10   3dfx      1.9         03/08/00 Don Fowler      Changed CSWINDOWID from
**       HWND to DWORD because of 16 bit in the server. Changed HDC to CSHDC which
**       is now a DWORD for the same reason.
**  9    3dfx      1.8         03/08/00 Don Fowler      Added an FxFAR define for
**       16 bit code. Fixed a minor type check in GetClipListFrom DirectDraw
**  8    3dfx      1.7         03/07/00 Don Fowler      Added code to allow clip
**       lists of a size larger than CS_CLIPLIST_MAX to be cut up into multiple
**       calls to the server
**  7    3dfx      1.6         03/07/00 Don Fowler      Added more structures to
**       the debug display system. Added CSSERVERRES.u32ReqID so the debug system
**       would know the id of the request packet after it returns. And for future
**       use.
**  6    3dfx      1.5         03/06/00 Don Fowler      Changed the return value
**       from ExtEscape from a bool to an int because it was wrong as a bool
**  5    3dfx      1.4         03/05/00 Don Fowler      Removed the notion of a
**       device context from the code. Since a graphical context is locked to a
**       window the same as a device context it was a redundant notion. 
**  4    3dfx      1.3         03/02/00 Don Fowler      Incremental development
**  3    3dfx      1.2         03/01/00 Don Fowler      Incremental development
**  2    3dfx      1.1         02/29/00 Don Fowler      Incremental development
**  1    3dfx      1.0         02/28/00 Don Fowler      
** $
**
*/

#include "csclient.h"
#include "csserver.h"

#ifdef WIN32
FxU32 FX_CALL csCopyClipRegionDataFromDirectDraw( LPRGNDATA psDDRegionData,
    PCSCLIPLIST psClipList, FxU32 u32FirstTime );
#endif /* WIN32 */


/*****************************************************************************
**
**  Function        : csSwapBufferToDisplay
**
**  Parameters      :  PCSGRAPHICALCONTEXT psGraphicalContext - Graphical context 
**                     PCSSWAPBUFFERTODISPLAY psSwapBufferToDisplay - Info to swap with
**
**  Purpose         : Swap a buffer from the back to the visible display
**
**  Return Value    : CSRESULT
**
**  Programmer(s)   : Don Fowler
**
*****************************************************************************/

CSRESULT FX_CALL csSwapBufferToDisplay( PCSGRAPHICALCONTEXT psGraphicalContext, 
                                        PCSSWAPBUFFERTODISPLAY psSwapBufferToDisplay )
{
    RECT rect;
    POINT point;

    PFxSz pszFunctionName = "csSwapBufferToDisplay";
    csDEBUGDisplayFormatted( DEBUG_LEVEL_FUNCTION, "Enter - %s( psGraphicalContext(0x%x), psSwapBufferToDisplay(0x%x) )\n", pszFunctionName, psGraphicalContext, psSwapBufferToDisplay );

    /* Verify function entry */
    CSDEBUG_VALIDATE_PARAMETERS( ( !psGraphicalContext || !psSwapBufferToDisplay ) );
    CSDEBUG_VALIDATE_INITIALIZATION;
    CSDEBUG_VALIDATE_GRAPHICALCONTEXT_PTR( psGraphicalContext );

#ifdef WIN32
    {
        int s32Return;
        CSSERVERREQ sServerReq;
        CSSERVERRES sServerRes;
        HRESULT hResult;
        DWORD dwClipListSize;
        LPRGNDATA psRegionData;
        FxU32 u32GetClipList;
        FxU32 u32CopyClipListFirstTime = TRUE;

        /* Validate idDestWindow */
        if ( !psSwapBufferToDisplay->idDestWindow || ! IsWindow( ( HWND )psSwapBufferToDisplay->idDestWindow ) )
        {
            /* Bad window ID */
            csDEBUGDisplayFormatted( DEBUG_LEVEL_ERROR, "%s::ERROR::CSSWAPBUFFERTODISPLAY_APIERROR_CANTASSOCIATEWINDOWTOCLIP::The given Window ID appears to be invalid::0x%x\n", pszFunctionName, psSwapBufferToDisplay->idDestWindow );
            CSDEBUG_RETURN( DEBUG_LEVEL_FUNCTION, CSSWAPBUFFERTODISPLAY_APIERROR_CANTASSOCIATEWINDOWTOCLIP );
        }

        /* Associate the given window with the clipper, (if it isn't already) */
        if( ( hResult = psGraphicalContext->psDirectDrawClipper->lpVtbl->SetHWnd( 
            psGraphicalContext->psDirectDrawClipper, 0, ( HWND )psSwapBufferToDisplay->idDestWindow ) ) != DD_OK ) 
        {
            /* Can't associate this window with the clipper*/
            csDEBUGDisplayFormatted( DEBUG_LEVEL_ERROR, "%s::ERROR::CSSWAPBUFFERTODISPLAY_APIERROR_CANTASSOCIATEWINDOWTOCLIP::Unable To Associate idDestWindow to The DirectDraw Clipper::0x%x\n", pszFunctionName, hResult );
            CSDEBUG_RETURN( DEBUG_LEVEL_FUNCTION, CSSWAPBUFFERTODISPLAY_APIERROR_CANTASSOCIATEWINDOWTOCLIP );
        }

        /* Get the clip list for the window and store it in the swap buffer structure */
        /* First, the size of the clip region needs to be retrieved */
        if( ( hResult = psGraphicalContext->psDirectDrawClipper->lpVtbl->
            GetClipList( psGraphicalContext->psDirectDrawClipper, NULL, 
            NULL, &dwClipListSize ) ) != DD_OK ) 
        {
            /* Can't get the size of the clip list */
            csDEBUGDisplayFormatted( DEBUG_LEVEL_ERROR, "%s::ERROR::CSSWAPBUFFERTODISPLAY_APIERROR_CANTGETCLIPLISTSIZE::Unable To Get The DirectDraw Clip List Size::0x%x\n", pszFunctionName, hResult );
            CSDEBUG_RETURN( DEBUG_LEVEL_FUNCTION, CSSWAPBUFFERTODISPLAY_APIERROR_CANTGETCLIPLISTSIZE );
        }
        
        /* Allocate memory for the clip list to be stored in */
        if( !( psRegionData = ( LPRGNDATA )malloc( dwClipListSize ) ) )
        {
            /* Can't allocate memory for the clip list for the window */
            csDEBUGDisplayFormatted( DEBUG_LEVEL_ERROR, "%s::ERROR::CSSWAPBUFFERTODISPLAY_APIERROR_CANTGETCLIPLIST::Unable To Allocate Memory For The DirectDraw Clip List\n", pszFunctionName );
            CSDEBUG_RETURN( DEBUG_LEVEL_FUNCTION, CSSWAPBUFFERTODISPLAY_APIERROR_CANTGETCLIPLIST );
        }

        /* Now, get the clip list */
        if( ( hResult = psGraphicalContext->psDirectDrawClipper->lpVtbl->
            GetClipList( psGraphicalContext->psDirectDrawClipper, NULL, 
            psRegionData, &dwClipListSize ) ) != DD_OK ) 
        {
            /* Can't get the clip list for the window */
            csDEBUGDisplayFormatted( DEBUG_LEVEL_ERROR, "%s::ERROR::CSSWAPBUFTODISPLAY_APIERROR_CANTGETCLIPLIST::Unable To Get The DirectDraw Clip List::0x%x\n", pszFunctionName, hResult );
            CSDEBUG_RETURN( DEBUG_LEVEL_FUNCTION, CSSWAPBUFFERTODISPLAY_APIERROR_CANTGETCLIPLIST );
        }

        do
        {
            /* Copy the clip list structure from DirectDraw into the list to send to 
               the server, if there are more regions than can be sent at once then split up the calls */
            u32GetClipList = csCopyClipRegionDataFromDirectDraw( psRegionData, 
                &psSwapBufferToDisplay->sClipList, u32CopyClipListFirstTime );
            u32CopyClipListFirstTime = FALSE;

            /* Display the information about the psSwapBufferToDisplay structure */
            csDEBUGDisplayStruct( DEBUG_LEVEL_STRUCTURE, DEBUG_STRUCT_CSSWAPBUFFERTODISPLAY, psSwapBufferToDisplay ); 

            /* If there are no clip regions in the list then the window is entirely occluded
               and should not be copied to */
            if( psSwapBufferToDisplay->sClipList.u32TotalRegions != 0x00 )
            {
                /* Setup the structure for the request */
                sServerReq.u32ReqID     = CSREQ_SWAPBUFFERTODISPLAY;
                memcpy( &sServerReq.sGraphicalContext, psGraphicalContext, 
                    sizeof( CSGRAPHICALCONTEXT ) );

                /* Copy the swap buffer information into the request packet */
                memcpy( &sServerReq.unionReqData.sSwapBufferToDisplayReq.sSwapBufferToDisplay,
                    psSwapBufferToDisplay, sizeof( CSSWAPBUFFERTODISPLAY ) );
                memcpy( &sServerReq.unionReqData.sSwapBufferToDisplayReq.sSrcAllocationDescriptor,
                    psSwapBufferToDisplay->psSrcBufferAllocationDescriptor, sizeof( CSALLOCATIONDESCRIPTOR ) );

                /* RYAN@000616, initialize sDestClipRegion */
                point.x = point.y = 0;
                GetClientRect((HWND)psSwapBufferToDisplay->idDestWindow, &rect);
                ClientToScreen((HWND)psSwapBufferToDisplay->idDestWindow, &point);
                sServerReq.unionReqData.sSwapBufferToDisplayReq.sSwapBufferToDisplay.sDestClipRegion.u32Top = point.y;
                sServerReq.unionReqData.sSwapBufferToDisplayReq.sSwapBufferToDisplay.sDestClipRegion.u32Left = point.x;
                sServerReq.unionReqData.sSwapBufferToDisplayReq.sSwapBufferToDisplay.sDestClipRegion.u32Right = point.x + rect.right;
                sServerReq.unionReqData.sSwapBufferToDisplayReq.sSwapBufferToDisplay.sDestClipRegion.u32Bottom = point.y + rect.bottom;
        
                /* Display the information from the data structure */
                csDEBUGDisplayStruct( DEBUG_LEVEL_STRUCTURE, DEBUG_STRUCT_CSSERVERREQ, &sServerReq ); 

                /* Call the server to make the request */
                s32Return = ExtEscape( 
                    ( HDC )_sCSClientLibData.hDesktopDC,
                    CS_EXTESCAPE, 
                    sizeof( CSSERVERREQ ), ( LPSTR )&sServerReq, 
                    sizeof( CSSERVERRES ), ( LPSTR )&sServerRes );

                /* Display the information from the data structure */
                sServerRes.u32ReqID = sServerReq.u32ReqID;
                csDEBUGDisplayStruct( DEBUG_LEVEL_STRUCTURE, DEBUG_STRUCT_CSSERVERRES, &sServerRes ); 

                /* Verify the return states of the call */
                if( s32Return <= 0 )
                {
                    csDEBUGDisplayFormatted( DEBUG_LEVEL_ERROR, "%s::ERROR::CS_APIERROR_SYSTEMFAILEDEXTESCAPE::ExtEscape Failed::0x%x\n", pszFunctionName, s32Return );
                    CSDEBUG_RETURN( DEBUG_LEVEL_FUNCTION, CS_APIERROR_SYSTEMFAILEDEXTESCAPE );
                }

                /* Verify that the server did not fail the call */
                if( sServerRes.u32Status )
                {
                    csDEBUGDisplayFormatted( DEBUG_LEVEL_ERROR, "%s::ERROR::Server Returned Error(%x)\n", pszFunctionName, sServerRes.u32Status );
                    CSDEBUG_RETURN( DEBUG_LEVEL_FUNCTION, sServerRes.u32Status );
                }

                /* Copy the swap buffer information back from the result packet */
                memcpy(  psSwapBufferToDisplay, 
                    &sServerRes.unionResData.sSwapBufferToDisplayRes.sSwapBufferToDisplay, 
                    sizeof( CSSWAPBUFFERTODISPLAY ) );
            }
        } while( u32GetClipList );
    }    
#endif /* WIN32 */

    CSDEBUG_RETURN( DEBUG_LEVEL_FUNCTION, CS_SUCCESS );
}

#ifdef WIN32
/*****************************************************************************
**
**  Function        : csCopyClipRegionDataFromDirectDraw
**
**  Parameters      : LPRGNDATA - Pointer to the region data structure with the clip list
**                    PCSCLIPLIST - Pointer to the CS clip structure to store to
**                    u32NewClipList - TRUE if a new clip list is being started.
**                      This function allows clip lists to be cut up into smaller pieces
**
**  Purpose         : Copy and reformat the clip information from the Direct Draw
**                      clip information list to the CSCLIPLIST  
**
**  Return Value    : FxU32 - FxTRUE - More data in the clip list
**
**  Programmer(s)   : Don Fowler
**
*****************************************************************************/

static FxU32 FX_CALL csCopyClipRegionDataFromDirectDraw( LPRGNDATA psDDRegionData,
    PCSCLIPLIST psClipList, FxU32 u32NewClipList )
{
    FxU32 u32ListIndex;
    static LPRECT psRect;
    static FxU32 u32TotalRegions;  
    FxU32 u32RegionsCopied;

    PFxSz pszFunctionName = "csCopyClipRegionDataFromDirectDraw";
    csDEBUGDisplayFormatted( DEBUG_LEVEL_FUNCTION, "Enter - %s( psDDRegionData(0x%x), psClipList(0x%x) )\n", pszFunctionName, psDDRegionData, psClipList );

    /* Setup the initial data for copying the clip regions */
    if( u32NewClipList )
    {
        u32TotalRegions = psDDRegionData->rdh.nCount;
        psRect = ( LPRECT )psDDRegionData->Buffer;
    }

    /* Copy until there are no more regions, or there is no more space to copy to */
    u32ListIndex = 0x00;
    u32RegionsCopied = 0x00;
    while( u32RegionsCopied < CS_CLIPREGIONS_MAX && u32TotalRegions )
    {
        psClipList->sClipRegion[ u32ListIndex ].u32Left = psRect->left;
        psClipList->sClipRegion[ u32ListIndex ].u32Top = psRect->top;
        psClipList->sClipRegion[ u32ListIndex ].u32Right = psRect->right;
        psClipList->sClipRegion[ u32ListIndex ].u32Bottom = psRect->bottom;

        /* Move to the next clip region in each list */
        u32ListIndex++;
        psRect++;
        u32TotalRegions--;
        u32RegionsCopied++;
    }

    /* Copy the data about the list */
    psClipList->u32Flags = 0x00;
    psClipList->u32TotalRegions = u32RegionsCopied;

    CSDEBUG_RETURN_NOERROR( DEBUG_LEVEL_FUNCTION, u32TotalRegions );
}


#endif /* WIN32 */




