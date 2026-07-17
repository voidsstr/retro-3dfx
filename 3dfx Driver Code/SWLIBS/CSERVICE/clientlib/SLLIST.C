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
** File name:   sllist.c
**
** Description: Functions to support a singly-linked list
**
** $Log: 
**  6    Napalm R1.51.2.1.2     10/11/00 Brent           Forced check in to enforce
**       branch.
**  5    Napalm R1.51.2.1.1     07/20/00 Andrew  Bell    Rolling back all swlibs
**       files to 2nd latest version due to an incorrect checkin.  This should
**       bring things back to normal.
**  4    Napalm R1.51.2.1.0     07/19/00 Dinesh Raja Savari Amirtharaj CSIM for the
**       RTL release_3_2
**  3    3dfx      1.2         03/03/00 Don Fowler      Incremental development
**  2    3dfx      1.1         03/02/00 Don Fowler      Incremental development
**  1    3dfx      1.0         03/02/00 Don Fowler      
** $
**
*/

#include "csclient.h"

/*****************************************************************************
**
**  Function        : csSLLISTInsert
**
**  Parameters      : PCSSLLISTNODE psRootNode
**                    PCSSLLISTNODE psNodeToInsert
**
**  Purpose         : Insert an element into a singly-linked list
**
**  Return Value    : VOID
**
**  Programmer(s)   : Don Fowler
**
*****************************************************************************/

FxVOID FX_CALL csSLLISTInsert( PCSSLLISTNODE psRootNode, PCSSLLISTNODE psNodeToInsert )
{
    PCSSLLISTNODE psNode;
    PFxSz pszFunctionName = "csSLLISTInsert";
    csDEBUGDisplayFormatted( DEBUG_LEVEL_FUNCTION, "Enter - %s( psRootNode(0x%x), psNodeToInsert( 0x%x ) )\n", pszFunctionName, psRootNode, psNodeToInsert );

    /* Traverse the list until the last node is found */
    psNode = psRootNode;
    while( psNode->psNext )
        psNode = psNode->psNext;

    /* Insert the structure node here */
    psNodeToInsert->psNext = ( PCSSLLISTNODE )0x00;
    psNode->psNext = psNodeToInsert;

    CSDEBUG_RETURNVOID( DEBUG_LEVEL_FUNCTION );
}


/*****************************************************************************
**
**  Function        : csSLLISTRemove
**
**  Parameters      : PCSSLLISTNODE psRootNode
**                    PCSSLLISTNODE psNodeToRemove
**
**  Purpose         : Remove an element from a singly-linked list
**
**  Return Value    : VOID
**
**  Programmer(s)   : Don Fowler
**
*****************************************************************************/

FxVOID FX_CALL csSLLISTRemove( PCSSLLISTNODE psRootNode, PCSSLLISTNODE psNodeToRemove )
{
    PCSSLLISTNODE psNode;
    PFxSz pszFunctionName = "csSLLISTRemove";
    csDEBUGDisplayFormatted( DEBUG_LEVEL_FUNCTION, "Enter - %s( psRootNode(0x%x), psNodeToRemove( 0x%x ) )\n", pszFunctionName, psRootNode, psNodeToRemove );

    /* Traverse the list until the node is found, or the end of the list is reached */
    psNode = psRootNode;
    while( psNode->psNext != psNodeToRemove && psNode->psNext )
        psNode = psNode->psNext;

    /* If no node was found then display an error and return with no harm */
    if( !psNode->psNext )
    {
        csDEBUGDisplayFormatted( DEBUG_LEVEL_ERROR, "%s::ERROR::Unable To Find Node To Remove\n", pszFunctionName );
        CSDEBUG_RETURNVOID( DEBUG_LEVEL_FUNCTION );
    }

    /* Remove the node here */
    psNode->psNext = psNodeToRemove->psNext;
   
    CSDEBUG_RETURNVOID( DEBUG_LEVEL_FUNCTION );
}

/*****************************************************************************
**
**  Function        : csSLLISTGetNext
**
**  Parameters      : PCSSLLISTNODE psRootNode
**
**  Purpose         : Return the next element in a list
**
**  Return Value    : VOID
**
**  Programmer(s)   : Don Fowler
**
*****************************************************************************/

PCSSLLISTNODE FX_CALL csSLLISTGetNext( PCSSLLISTNODE psRootNode )
{
    PFxSz pszFunctionName = "csSLLISTGetNext";
    csDEBUGDisplayFormatted( DEBUG_LEVEL_FUNCTION, "Enter - %s( psRootNode(0x%x) )\n", pszFunctionName, psRootNode );

    /* Return the next node in the list */
    CSDEBUG_RETURN_NOERROR( DEBUG_LEVEL_FUNCTION, psRootNode->psNext );
}
