
#include "clientdll.h"



static HINSTANCE hInstance;

BOOL WINAPI DllMain( HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved ) 
{
    static CSINIT sCSInit;

    hInstance = ( HINSTANCE )hInst;

	switch( ul_reason_for_call ) 
	{
		case DLL_PROCESS_DETACH:
            csUnInit();
    		break;
		case DLL_PROCESS_ATTACH:
            memset( &sCSInit, 0x00, sizeof( CSINIT ) );
            sCSInit.hDLLInstance = hInstance;
            csInit( &sCSInit );
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
		default:
			break;
	}
  
	return( TRUE );
}


CSRESULT __declspec(dllexport) csDLLGetProtocolRevision( CSDCID idDeviceContext,
                            FxU32 *pu32Major, 
                            FxU32 *pu32Minor )
{
    return( csGetProtocolRevision( idDeviceContext,pu32Major,pu32Minor ) );
}
CSRESULT __declspec(dllexport) csDLLGetGraphicalContext( CSDCID idDeviceContext,
                            PCSGRAPHICALCONTEXT psGraphicalContext )
{
    return( csGetGraphicalContext( idDeviceContext, psGraphicalContext ));
}
CSRESULT __declspec(dllexport) csDLLFree           ( PCSGRAPHICALCONTEXT psGraphicalContext, 
                            PCSALLOCATIONDESCRIPTOR psAllocationDescriptor )
{
    return(   csFree( psGraphicalContext, 
                psAllocationDescriptor ) );
}
CSRESULT __declspec(dllexport) csDLLSetVideoMode   ( PCSGRAPHICALCONTEXT psGraphicalContext,  
                            PCSVIDEOMODE        psVideoMode,
                            CSWINDOWID          idWindow )
{
    return( csSetVideoMode( psGraphicalContext, psVideoMode, idWindow ));
}
CSRESULT __declspec(dllexport) csDLLAcquireOverlay ( PCSGRAPHICALCONTEXT psGraphicalContext,
                            PCSOVERLAY          psOverlay )
{
    return( csAcquireOverlay ( psGraphicalContext,
                            psOverlay ));
}

CSRESULT __declspec(dllexport) csDLLExecuteCommands( PCSGRAPHICALCONTEXT psGraphicalContext,  
                            PCSALLOCATIONDESCRIPTOR psStateAllocationDescriptor,
                            PCSALLOCATIONDESCRIPTOR psCommandAllocationDescriptor,
                            PCSALLOCATIONDESCRIPTOR psSentinelAllocationDescriptor,
                            FxU32 u32StateOffset,
                            FxU32 u32CommandOffset,
                            FxU32 u32StateSize,
                            FxU32 u32CommandSize,
                            FxU32 u32SentinelSerial )
{
    return( csExecuteCommands( psGraphicalContext,  
                            psStateAllocationDescriptor,
                            psCommandAllocationDescriptor,
                            psSentinelAllocationDescriptor,
                            u32StateOffset,
                            u32CommandOffset,
                            u32StateSize,
                            u32CommandSize,
                            u32SentinelSerial ));
}

CSRESULT __declspec(dllexport) csDLLDeviceSpecificCommunication( PCSGRAPHICALCONTEXT psGraphicalContext, 
                            FxU32 u32RequestID)
{
    return( csDeviceSpecificCommunication( psGraphicalContext, 
              u32RequestID ) );
}
CSRESULT __declspec(dllexport) csDLLLock           ( PCSGRAPHICALCONTEXT psGraphicalContext, 
                            PCSALLOCATIONDESCRIPTOR psAllocationDescriptor )
{
    return( csLock( psGraphicalContext, 
              psAllocationDescriptor ) );
}
CSRESULT __declspec(dllexport) csDLLUnlock         ( PCSGRAPHICALCONTEXT psGraphicalContext, 
                            PCSALLOCATIONDESCRIPTOR psAllocationDescriptor )
{
    return( csUnlock( psGraphicalContext, 
              psAllocationDescriptor ));
}
CSRESULT __declspec(dllexport) csDLLReleaseGraphicalContext( PCSGRAPHICALCONTEXT psGraphicalContext )
{
    return( csReleaseGraphicalContext( psGraphicalContext ));
}
CSRESULT __declspec(dllexport) csDLLAlloc          ( PCSGRAPHICALCONTEXT psGraphicalContext, 
                            PCSALLOCATIONDESCRIPTOR psAllocationDescriptor )
{
    return(csAlloc          ( psGraphicalContext, 
                            psAllocationDescriptor ) );
}
PFxSz    __declspec(dllexport)            csDLLDEBUGGetStringForError( CSRESULT idError )
{
    return( csDEBUGGetStringForError( idError ));
}

CSRESULT __declspec(dllexport) csDLLSwapBufferToDisplay( PCSGRAPHICALCONTEXT psGraphicalContext, 
                            PCSSWAPBUFFERTODISPLAY psSwapBufferToDisplay )
{
    return( csSwapBufferToDisplay( psGraphicalContext, 
                            psSwapBufferToDisplay ));
}
