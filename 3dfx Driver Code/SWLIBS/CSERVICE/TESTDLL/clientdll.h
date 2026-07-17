
#ifndef _CLIENTDLL_H
#define _CLIENTDLL_H

#include "csclient.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */ 

#ifdef _DLL_EXPORT 

CSRESULT __declspec(dllexport) csDLLGetProtocolRevision( CSDCID idDeviceContext,
                            FxU32 *pu32Major, 
                            FxU32 *pu32Minor );
CSRESULT __declspec(dllexport) csDLLGetGraphicalContext( CSDCID idDeviceContext,
                            PCSGRAPHICALCONTEXT psGraphicalContext );
CSRESULT __declspec(dllexport) csDLLFree           ( PCSGRAPHICALCONTEXT psGraphicalContext, 
                            PCSALLOCATIONDESCRIPTOR psAllocationDescriptor );
CSRESULT __declspec(dllexport) csDLLSetVideoMode   ( PCSGRAPHICALCONTEXT psGraphicalContext,  
                            PCSVIDEOMODE        psVideoMode, CSWINDOWID idWindow ); 
CSRESULT __declspec(dllexport) csDLLAcquireOverlay ( PCSGRAPHICALCONTEXT psGraphicalContext,
                            PCSOVERLAY          psOverlay );
CSRESULT __declspec(dllexport) csDLLExecuteCommands( PCSGRAPHICALCONTEXT psGraphicalContext,  
                            PCSALLOCATIONDESCRIPTOR psStateAllocationDescriptor,
                            PCSALLOCATIONDESCRIPTOR psCommandAllocationDescriptor,
                            PCSALLOCATIONDESCRIPTOR psSentinelAllocationDescriptor,
                            FxU32 u32StateOffset,
                            FxU32 u32CommandOffset,
                            FxU32 u32StateSize,
                            FxU32 u32CommandSize,
                            FxU32 u32SentinelSerial );
CSRESULT __declspec(dllexport) csDLLLock           ( PCSGRAPHICALCONTEXT psGraphicalContext, 
                            PCSALLOCATIONDESCRIPTOR psAllocationDescriptor );
CSRESULT __declspec(dllexport) csDLLUnlock         ( PCSGRAPHICALCONTEXT psGraphicalContext, 
                            PCSALLOCATIONDESCRIPTOR psAllocationDescriptor );
CSRESULT __declspec(dllexport) csDLLDeviceSpecificCommunication( PCSGRAPHICALCONTEXT psGraphicalContext, 
                            FxU32 u32RequestID);

CSRESULT __declspec(dllexport) csDLLReleaseGraphicalContext( PCSGRAPHICALCONTEXT psGraphicalContext );
CSRESULT __declspec(dllexport) csDLLAlloc          ( PCSGRAPHICALCONTEXT psGraphicalContext, 
                            PCSALLOCATIONDESCRIPTOR psAllocationDescriptor );
PFxSz    __declspec(dllexport)            csDLLDEBUGGetStringForError( CSRESULT idError );
CSRESULT __declspec(dllexport) csDLLSwapBufferToDisplay( PCSGRAPHICALCONTEXT psGraphicalContext, 
                            PCSSWAPBUFFERTODISPLAY psSwapBufferToDisplay );

#else

CSRESULT __declspec(dllimport) csDLLGetProtocolRevision( CSDCID idDeviceContext,
                            FxU32 *pu32Major, 
                            FxU32 *pu32Minor );
CSRESULT __declspec(dllimport) csDLLGetGraphicalContext( CSDCID idDeviceContext,
                            PCSGRAPHICALCONTEXT psGraphicalContext );
CSRESULT __declspec(dllimport) csDLLFree           ( PCSGRAPHICALCONTEXT psGraphicalContext, 
                            PCSALLOCATIONDESCRIPTOR psAllocationDescriptor );
CSRESULT __declspec(dllimport) csDLLSetVideoMode   ( PCSGRAPHICALCONTEXT psGraphicalContext,  
                            PCSVIDEOMODE        psVideoMode, CSWINDOWID idWindow ); 
CSRESULT __declspec(dllimport) csDLLAcquireOverlay ( PCSGRAPHICALCONTEXT psGraphicalContext,
                            PCSOVERLAY          psOverlay );
CSRESULT __declspec(dllimport) csDLLExecuteCommands( PCSGRAPHICALCONTEXT psGraphicalContext,  
                            PCSALLOCATIONDESCRIPTOR psStateAllocationDescriptor,
                            PCSALLOCATIONDESCRIPTOR psCommandAllocationDescriptor,
                            PCSALLOCATIONDESCRIPTOR psSentinelAllocationDescriptor,
                            FxU32 u32StateOffset,
                            FxU32 u32CommandOffset,
                            FxU32 u32StateSize,
                            FxU32 u32CommandSize,
                            FxU32 u32SentinelSerial );
CSRESULT __declspec(dllimport) csDLLLock           ( PCSGRAPHICALCONTEXT psGraphicalContext, 
                            PCSALLOCATIONDESCRIPTOR psAllocationDescriptor );
CSRESULT __declspec(dllimport) csDLLUnlock         ( PCSGRAPHICALCONTEXT psGraphicalContext, 
                            PCSALLOCATIONDESCRIPTOR psAllocationDescriptor );
CSRESULT __declspec(dllimport) csDLLReleaseGraphicalContext( PCSGRAPHICALCONTEXT psGraphicalContext );
CSRESULT __declspec(dllimport) csDLLAlloc          ( PCSGRAPHICALCONTEXT psGraphicalContext, 
                            PCSALLOCATIONDESCRIPTOR psAllocationDescriptor );
PFxSz    __declspec(dllimport)            csDLLDEBUGGetStringForError( CSRESULT idError );
CSRESULT __declspec(dllimport) csDLLSwapBufferToDisplay( PCSGRAPHICALCONTEXT psGraphicalContext, 
                            PCSSWAPBUFFERTODISPLAY psSwapBufferToDisplay );
CSRESULT __declspec(dllimport) csDLLDeviceSpecificCommunication( PCSGRAPHICALCONTEXT psGraphicalContext, 
                            FxU32 u32RequestID);

#endif


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _CLIENTDLL_H */