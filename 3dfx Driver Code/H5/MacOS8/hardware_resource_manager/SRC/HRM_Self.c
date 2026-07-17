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
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
**
** $Header: HRM_Self.c, 3, 10/11/00 8:35:15 PM, Brent$
** $Log: 
**  3    3dfx      1.1.1.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  2    MacOS Dev Tree1.1         01/28/00 Kenneth Dyke    Fix line endings.
**  1    MacOS Dev Tree1.0         01/28/00 Kenneth Dyke    
** $
** 
** 2     7/02/99 3:23p Kcd
** Added FIFO resource type.
**
*/

#include "hrm_utilities.h"
#include "hrm_self.h"
#include <DCon.h>

#include <CodeFragments.h>

/*
________________________________________________________ Private Definitions ___
*/

#define kHrmName "Hardware Resource Manager"

short             hrmGetResourceInfo(
                          hrmResourceInfo_T *  outResInfo);

short             hrmRequestTermination( void );

short             hrmGetNextResourceInfo(
                          hrmResourceInfo_T *  outResInfo);

short             hrmUpdate(
                          hrmUpdateData_t *    inUpdateData);

short             hrmSaveGlobals(
                          char **              outData);
                          
short             hrmRestoreGlobals(
                          hrmGenericData_t *   inData);

short             hrmRequestAllTerminations( void );


/*
___________________________________________________________ hrmDispatch_self ___
*/

short
hrmDispatch_self(
  hrmCommand_t *       ioCmd)
{
  short                theSuccess = -1;
  
  switch( ioCmd->methodSelector ) {
    case HRM_GET_RESOURCE_INFO:
      theSuccess = hrmGetResourceInfo( &ioCmd->u.resInfo );
      break;
    
    case HRM_REQUEST_TERMINATION:
      theSuccess = hrmRequestTermination();
      break;
      
    case HRM_GET_NEXT_RESOURCE_INFO:
      theSuccess = hrmGetNextResourceInfo( &ioCmd->u.resInfo );          
      break;

    case HRM_UPDATE:
      theSuccess = hrmUpdate( &ioCmd->u.updateData );
      break;

    case HRM_UPDATE_GLOBALS:
      theSuccess = hrmRestoreGlobals( &ioCmd->u.genericData );
      break;

    default:
      dprintf("hrmDispatch_self(): invalid method selector = %d, ( ioCmd @ 0x%08x )\n", ioCmd->methodSelector, ioCmd);
      break;
  }
  
  return theSuccess;   
}


/*
_________________________________________________________ hrmGetResourceInfo ___

Fix me: we are not passing the number of boards.
*/

short
hrmGetResourceInfo(
  hrmResourceInfo_T *  outResInfo)
{
  short                theSuccess = 0;
  
  dprintf("hrmGetResourceInfo(): Get HRM_SELF resource info\n");

#if __UPDATE__
  dprintf("hrmGetResourceInfo(): updated version\n");
#else
  dprintf("hrmGetResourceInfo(): original version\n");
#endif

  {
    hrmBoard_t *       theBoard = &gHdwrData[0];
    hwcBoardInfo *     theInfo = &theBoard->boardInfo;
  }

  outResInfo->ioResourceTag = HRM_SELF;
  outResInfo->outVersion = kHrmVersion;
  strcpy( outResInfo->outName, kHrmName );
  
  return theSuccess;
}


/*
__________________________________________________ hrmRequestAllTerminations ___

*/

short
hrmRequestTermination( void )
{
  return 0;
}

/*
_____________________________________________________ hrmGetNextResourceInfo ___
*/

short
hrmGetNextResourceInfo(
  hrmResourceInfo_T *  outResInfo)
{
  short                theSuccess = -1;
  
  if ( ( outResInfo->ioResourceTag < HRM_SELF )
                    || ( outResInfo->ioResourceTag >= HRM_LAST_RESOURCE_TAG ) ) {
    outResInfo->ioResourceTag = HRM_SELF;
  }
  
  outResInfo->ioResourceTag = hrmGetNextResourceTag( outResInfo->ioResourceTag );

  if ( outResInfo->ioResourceTag != (hrmResourceTag_e) 0 ) {
    hrmCommand_t     theCmd;
      
    theCmd.resourceSelector = outResInfo->ioResourceTag;
    theCmd.methodSelector = HRM_GET_RESOURCE_INFO;
    theCmd.u.resInfo.ioResourceTag = HRM_SELF;
      
    theSuccess = hrmDispatcher( &theCmd );
    *outResInfo = theCmd.u.resInfo;
   
  }
  
  return theSuccess;
}


/*
______________________________________________________ hrmGetNextResourceTag ___
*/

hrmResourceTag_e
hrmGetNextResourceTag(
  hrmResourceTag_e     inResTag)
{
  hrmResourceTag_e     theResourceList[] = {
                                             HRM_SELF,
                                             HRM_MEMORY_MANAGER,
                                             HRM_OVERLAY_SURFACE,
                                             HRM_FIFO,
                                                    
                                             (hrmResourceTag_e) 0
                                           };
  long                 theIndex = sizeof( theResourceList )
                                              / sizeof ( hrmResourceTag_e ) - 2;


  while ((theIndex >= 0) && (theResourceList[ theIndex ] != inResTag)) theIndex--;
  return (theIndex >= 0) ? theResourceList[ theIndex + 1 ] : (hrmResourceTag_e) 0;    
}


/*
__________________________________________________________________ hrmUpdate ___

This update algorithm is leaking memory (we don't return the unused HRM). Since
we only need this feature to debug, it is harmless enough...

*/

short
hrmUpdate(
  hrmUpdateData_t *    inUpdateData)
{
  Handle               theNewCFragH = (Handle) inUpdateData->opaqueData;
  CFragConnectionID    theHRMLib;
  hrmDispatcherPtr     theDispatcherUpgrade;
//  short                (*theDispatcherUpgrade)(hrmCommand_t * ioCmd);
  Str255               theError;
  char *               theSavedGlobalData;
  short                theSuccess = -1;
  
  dprintf("hrmUpdate(): attempting to update...\n");
  dprintf("hrmUpdate(): MemFrag @ 0x%08x\n", *inUpdateData->opaqueData );
                                                      
  theSuccess = hrmRequestAllTerminations();
  
  if ( theSuccess == 0 ) {

    theSuccess = hrmSaveGlobals( &theSavedGlobalData );
                      
    if ( theSuccess == 0 ) {
      dprintf("hrmUpdate(): new fragment @ 0x%08x\n", theNewCFragH);
      theSuccess = GetMemFragment( *theNewCFragH, GetHandleSize( theNewCFragH ),
                          (StringPtr)"\p3DfxHrdwResMgrUpdate", kPrivateCFragCopy,
                                       &theHRMLib, (Ptr *) &theDispatcherUpgrade,
                                                                       theError);

      if ( theSuccess == noErr ) {
        hrmCommand_t   theCmd;
       
        theCmd.resourceSelector = HRM_SELF;
        theCmd.methodSelector = HRM_UPDATE_GLOBALS;
        theCmd.u.genericData.dataVersion = kHrmVersion;
        theCmd.u.genericData.dataSize = sizeof( hrmUpdateGlobalsBucket_t );
        theCmd.u.genericData.dataPtr = theSavedGlobalData;
      
        theSuccess = theDispatcherUpgrade( &theCmd );

        if ( theSuccess == noErr ) {
          hrmCommand_t           theCmd;
          FxU32                  theCurrentHRM;
        
          dprintf("hrmUpdate(): new code fragment validated\n");
          theCmd.resourceSelector = HRM_SELF;
          theCmd.methodSelector = HRM_GET_RESOURCE_INFO;
          theCmd.u.resInfo.ioResourceTag = HRM_SELF;
      
          hrmDispatcher( &theCmd );
          theCurrentHRM = theCmd.u.resInfo.outVersion;
          theSuccess = (*theDispatcherUpgrade)( &theCmd );
          if ( (theSuccess == noErr)
                           && (theCmd.u.resInfo.outVersion >= theCurrentHRM) ) {
            dprintf("hrmUpdate(): new dispatcher @ 0x%08x\n", theDispatcherUpgrade);
            gDispatcherUpgrade = theDispatcherUpgrade;

          } else { 
            theSuccess = -14;
            dprintf("hrmUpdate(): ### Failure! udpate HRM is an older!\n");
          }

        } else { 
          theSuccess = -13;
          dprintf("hrmUpdate(): ### Failure! could not restore data!\n");
        }

      } else { 
        theSuccess = -12;
        dprintf("hrmUpdate(): ### Failure! could not find the code fragment!\n");
      }

    } else { 
      theSuccess = -11;
      dprintf("hrmUpdate(): ### Failure! could not save data!\n");
    }
  
  } else { 
    dprintf("hrmUpdate(): ### Failure! a resource could not be terminated\n");
  }
  
  return theSuccess;
}


/*
____________________________________________________ hrmSaveGlobals ___

*/

short
hrmSaveGlobals(
  char **              outData)
{
  short                theSuccess = -1;  

#if __UPDATE__
  dprintf("hrmSaveGlobals(): ... (from updated version)\n");
#else
  dprintf("hrmSaveGlobals(): ... (from original version)\n");
#endif

  *outData = NewPtrSys( sizeof ( hrmUpdateGlobalsBucket_t ) );
  
  if ( *outData ) {
    hrmUpdateGlobalsBucket_t * theData = (hrmUpdateGlobalsBucket_t*) *outData;
    short i;
    
    i = 0;
    while ( i < kHrmMaxNumberOfTargets ) {
      theData->hdwrData[i] = gHdwrData[i];
      i++;
    }
  
    theSuccess = 0;
  }
  
  dprintf("hrmSaveGlobals(): done! (@ 0x%08x)\n", *outData);
  
  return theSuccess;
}

/*
_________________________________________________ hrmRestoreGlobals ___

*/

short
hrmRestoreGlobals(
  hrmGenericData_t *   inData)
{
  short                theSuccess = -1;
  
#if __UPDATE__
  dprintf("hrmRestoreGlobals(): ... (from updated version, @ 0x%08x)\n", inData->dataPtr);
#else
  dprintf("hrmRestoreGlobals(): ... (from original version)\n");
#endif


  if ( inData->dataSize == sizeof ( hrmUpdateGlobalsBucket_t ) ) {

    if ( inData ) {
      hrmUpdateGlobalsBucket_t * theData = (hrmUpdateGlobalsBucket_t*) inData->dataPtr;
      short i;
      
      i = 0;
      while ( i < kHrmMaxNumberOfTargets ) {
        gHdwrData[i] = theData->hdwrData[i];
        i++;
      }
      
      DisposePtr( (Ptr) inData );
      theSuccess = 0;
    }
  } else {
    dprintf("hrmRestoreGlobals(): ### failed,"
                      " the size of the data is different\n");
  }
  
  dprintf("hrmRestoreGlobals(): done!\n");
  
  return theSuccess;
}


/*
__________________________________________________ hrmRequestAllTerminations ___

*/

short
hrmRequestAllTerminations( void )
{
  hrmCommand_t         theCmd;
  hrmResourceTag_e     theResourceTag;
  short                theSuccess = 0;

  theCmd.methodSelector = HRM_REQUEST_TERMINATION;
      
  theResourceTag = HRM_SELF;  /*we assume this is the first resource tag */
  while ( theResourceTag != (hrmResourceTag_e)0 && theSuccess == 0) {
  
    theCmd.resourceSelector = theResourceTag;
    
    theSuccess = hrmDispatcher( &theCmd );
   
    theResourceTag = hrmGetNextResourceTag( theResourceTag );
  }


  return theSuccess;
}