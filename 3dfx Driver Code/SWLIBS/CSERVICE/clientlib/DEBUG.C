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
** File name:   debug.c
**
** Description: Debugging functions for csclient.lib
**
** $Log: 
**  39   3dfx      1.28.1.9    10/24/00 Don Fowler      Added CSTIME structure to
**       debug system and CSGRAPHICALCONTEXT structure. De-coupled
**       CSGRAPHICALCONTEXT from CS_BUFFER_TEXTURE_HEAP type in all functions that
**       deal with this memory type. 
**  38   3dfx      1.28.1.8    10/12/00 Don Fowler      Added buffer type
**       CS_BUFFER_TEXTUREHEAP to the allocation system. This texture type is not a
**       child of a graphical context and can be allocated any time after csInit is
**       called
**  37   3dfx      1.28.1.7    10/11/00 Brent           Forced check in to enforce
**       branch.
**  36   3dfx      1.28.1.6    10/11/00 Ryan Bissell    Modifications needed for
**       OGL.
**  35   3dfx      1.28.1.5    10/10/00 Don Fowler      Fixed a bug in
**       csGetErrorStringForError where the server code wasn't returning the error
**  34   3dfx      1.28.1.4    10/09/00 Don Fowler      Removed // DWF left in from
**       previous build 
**  33   3dfx      1.28.1.3    10/09/00 Don Fowler      Changed debug system to get
**       errors from the server when the error code comes from the server. The
**       error code defines are stored in the respecitve servers header files. Call
**       csDEBUGGetErrorString to retrieve the verbose string from the server
**  32   3dfx      1.28.1.2    10/09/00 Don Fowler      Added support for the
**       CSSST1 chip type.
**  31   3dfx      1.28.1.1    07/13/00 Don Fowler      Changed csExecuteCommands
**       so that the server could be responsible for adding the sentinel buffer
**       write packets to the end of the command buffer. This required the sentinel
**       serial number and the sentinel buffer allocation descriptor to be passed
**       to the api and then on to the server.
**  30   3dfx      1.28.1.0    07/06/00 Ryan Bissell    Incremental changes, and
**       bug fixes.
**  29   3dfx      1.28        03/29/00 Don Fowler      Added
**       CSALLOC_APIERROR_INVALIDASPECT 
**  28   3dfx      1.27        03/24/00 Ryan Bissell    renamed "u32SurfacePitch"
**       to "u32LinearStride"
**  27   3dfx      1.26        03/19/00 Don Fowler      Added the
**       CSALLOCATIONDESCRIPTOR.u32Locale so that the client can specify
**       CS_LOCALE_FRAMEBUFFER  or CS_LOCALE_AGP separate from tiled or linear
**       memory types.
** 
**       Changed all of the error codes in the API from _ERROR_ to _APIERROR_ so
**       that server codes can have a different base 
** 
**  26   3dfx      1.25        03/16/00 Don Fowler      Removed u32DestColorFormat
**       from CSSWAPBUFFERTODISPLAY and fixed a bug with a memcopy in
**       csSwapBufferToDisplay
**  25   3dfx      1.24        03/16/00 Don Fowler      Added u32PhysicalStride
**       field to CSALLOCATIONDESCRIPTOR
**  24   3dfx      1.23        03/10/00 Don Fowler      Re moved csRegisterRead and
**       csRegisterWrite and added csDeviceSpecificCoummunication  to supercede
**       these functions.
** 
**       Added register reading and writing information to CSSST2
**  23   3dfx      1.22        03/09/00 Don Fowler      Modified csAlloc to take
**       u32StateSize and u32CommandSize. These changes went into the debug system
**       and into CSALLOCREQ and CSALLOCRES
**  22   3dfx      1.21        03/09/00 Don Fowler      Added new debug types and
**       strings
**  21   3dfx      1.20        03/09/00 Don Fowler      Added more error types and
**       stricter error checking to csAlloc
**  20   3dfx      1.19        03/09/00 Don Fowler      Modified include file
**       structure to separate out a server-specific file and a client-specific
**       file. 
**  19   3dfx      1.18        03/08/00 Don Fowler      Changed FxBool to FxU32
**       because of 16 bit structure alignment problems.
**  18   3dfx      1.17        03/07/00 Don Fowler      Removed the display of the
**       CSDEVICESPECIFICDATA from the CSDEVICECONFIG
**  17   3dfx      1.16        03/07/00 Don Fowler      Added more structures to
**       the debug display system. Added CSSERVERRES.u32ReqID so the debug system
**       would know the id of the request packet after it returns. And for future
**       use.
**  16   3dfx      1.15        03/05/00 Don Fowler      Removed the notion of a
**       device context from the code. Since a graphical context is locked to a
**       window the same as a device context it was a redundant notion. 
**  15   3dfx      1.14        03/03/00 Don Fowler      Incremental development
**  14   3dfx      1.13        03/03/00 Don Fowler      Incremental development
**  13   3dfx      1.12        03/02/00 Don Fowler      Incremental Development
**  12   3dfx      1.11        03/02/00 Don Fowler      Incremental development
**  11   3dfx      1.10        03/01/00 Don Fowler      Incremental development
**  10   3dfx      1.9         02/29/00 Don Fowler      Incremental development
**  9    3dfx      1.8         02/28/00 Don Fowler      Incremental development
**  8    3dfx      1.7         02/28/00 Don Fowler      Incremental development
**  7    3dfx      1.6         02/28/00 Don Fowler      Incremental development
**  6    3dfx      1.5         02/27/00 Don Fowler      Incremental development
** 
**  5    3dfx      1.4         02/27/00 Don Fowler      Incremental development
**  4    3dfx      1.3         02/27/00 Don Fowler      Incremental development
**  3    3dfx      1.2         02/27/00 Don Fowler      Incremental development
**  2    3dfx      1.1         02/27/00 Don Fowler      Completed Rev 1 of the
**       debuging system
**  1    3dfx      1.0         02/27/00 Don Fowler      
** $
**
*/

#include "csclient.h"
#include "csserver.h"

/* Static data for this module */

/* String to precede the debug messages to indicate the system */
static char* pszSystemStamp = "cs"; 

/* Current debug level */
static FxU32 u32DEBUGLevel;   

/* File to output debug messages to */
static FILE* psOutputFile = ( FILE * )0;   

/* Flag to indicate a file is to be used */
static FxU32 u32OutputToFile = FXFALSE;

/* Flag to indicate that the debug system is initialized */
static FxU32 u32DEBUGInitialized = FXFALSE;


/*****************************************************************************
**
**  Function        : csDEBUGInit
**
**  Parameters      : FxVOID
**
**  Purpose         : Initialize the debugging system for CSCLIENT.LIB
**
**  Return Value    : FxVOID
**
**  Programmer(s)   : Don Fowler
**
*****************************************************************************/

FxVOID FX_CALL csDEBUGInit( FxVOID )
{
    static FxU32 u32Executed = FXFALSE;
    PFxSz pszEnvironment;

    /* Set the flag to indicate that the debug system is initialized */
    u32DEBUGInitialized = FXTRUE;

    /* if we've already executed this function then skip it */
    if( u32Executed )
        return;
    u32Executed = FXTRUE;
  
    /* Initialize to level 0 */
    u32DEBUGLevel = 0;

    /* Set the debug output pipe */
    pszEnvironment = getenv( "CSDEBUG_FILE" );
    if( pszEnvironment != NULL ) 
        csDEBUGSetFile( pszEnvironment );

    /* Set the output level */
    pszEnvironment = getenv( "CSDEBUG_LEVEL" );
    if( pszEnvironment == NULL ) 
        pszEnvironment = "0";
 
    /* Convert the environment setting from ascii to integer */
    u32DEBUGLevel = atoi( pszEnvironment );

    /* Display the debug level */
    csDEBUGDisplayFormatted( DEBUG_LEVEL_ERROR,"csDEBUGInit()::u32DEBUGLevel = %d\n", u32DEBUGLevel );
}

/*****************************************************************************
**
**  Function        : csDEBUGSetFile
**
**  Parameters      : PFxSz pszFileName - Name of the file to output to
**
**  Purpose         : Open the file to be used to store the debug information CSCLIENT.LIB
**
**  Return Value    : FxVOID
**
**  Programmer(s)   : Don Fowler
**
*****************************************************************************/

FxVOID FX_CALL csDEBUGSetFile( PFxSz pszFileName )
{
    /* If the file name is debug then output to the console, otherwise open the file
       to output to */
    if( !strcmp( pszFileName, "DEBUG" ) )
    {
        u32OutputToFile = FXFALSE;
    }
    else 
    {
        psOutputFile = fopen( pszFileName, "w" );             
        u32OutputToFile = FXTRUE;
    }

    return;
}


/*****************************************************************************
**
**  Function        : csDEBUGPrint
**
**  Parameters      : PFxSz pszFormatString - Formatting string
**                    va_list args - Variable argument list 
**
**  Purpose         : Display the list of arguments, using the formatting string,
**                    to the appropriate output path    
**
**  Return Value    : FxVOID
**
**  Programmer(s)   : Don Fowler
**
*****************************************************************************/

FxVOID FX_CALL csDEBUGPrint( PFxSz pszFormatString, va_list args )
{
    /* Check to see if the system is initialized, if not then initialize it */
    if( !u32DEBUGInitialized )
        csDEBUGInit();

    /* If the output is to a file and the file is not null then output to the file */
    if( u32OutputToFile && psOutputFile )
    {
        vfprintf( psOutputFile, pszFormatString, args );
        fflush( psOutputFile );
        return;
    }

    /* Output through OutputDebugString */   
    if( u32OutputToFile == FXFALSE )
    {
      static FxSz szBuffer[ 1024 ];
      vsprintf( ( char * )&szBuffer, pszFormatString, args );
      OutputDebugString( ( const char * )&szBuffer );
      return;
    } 

    return;
}


/*****************************************************************************
**
**  Function        : csDEBUGDisplayFormatted
**
**  Parameters      : FxU32 u32Level - Debug level of the output 
**                    PFxSz pszFormatString - Formatting string
**
**  Purpose         : Display the list of arguments, using the formatting string,
**                    to the appropriate output path    
**
**  Return Value    : FxVOID
**
**  Programmer(s)   : Don Fowler
**
*****************************************************************************/

FxVOID FX_CALL csDEBUGDisplayFormatted( FxU32 u32Level, PFxSz pszFormatString, ... )
{
    va_list args;
    FxSz szNewFormat[ 4095 ];

    /* If the level is above the global debug level then exit */
    if( ( u32Level > u32DEBUGLevel ) || ( u32DEBUGInitialized != FXTRUE ) )
        return;
        
    va_start( args, pszFormatString );
    sprintf( ( char * )&szNewFormat, "%s.%3d:\t", pszSystemStamp, u32Level );
    strcat( ( char * )&szNewFormat, pszFormatString );
    csDEBUGPrint( ( PFxSz )&szNewFormat, args );
    va_end( args );

    return;
}

/*****************************************************************************
**
**  Function        : csDEBUGGetStringForError
**
**  Parameters      : CSRESULT idError - Error to return a string for
**
**  Purpose         : Return a string for the error code passed in
**
**  Return Value    : PFxSz - String
**
**  Programmer(s)   : Don Fowler
**
*****************************************************************************/

typedef struct csdebugerrorcodestring_s
{
    CSRESULT    idError;        /* Error code */
    PFxSz       pszErrorString; /* String for the error code */

} CSDEBUGERRORCODESTRING,*PCSDEBUGERRORCODESTRING;

/* Array of error codes associated with their strings */
static CSDEBUGERRORCODESTRING sDEBUGErrorCodeStrings[] = 
{
    { CS_SUCCESS,                     "Success"                     },   
    { CS_APIERROR_INVALIDCONTEXT,        "Invalid Graphical Context"   },
    { CS_APIERROR_OUTOFMEMORY,           "Out Of Memory"               },            
    { CS_APIERROR_INVALIDPARAM,          "Invalid Parameter"           },           
    { CS_APIERROR_IMPOSSIBILITY,         "Impossibility"               },          
    { CS_APIERROR_FULLSCREENACTIVE,      "Full Screen Context Active"  },       
    { CS_APIERROR_SURFACELOST,           "Surface Lost"                },            
    { CS_APIERROR_NOTSUPPORTED,          "UnSupported"},           
    { CS_APIERROR_SYSTEMNOTINITIALIZED,"System Not Intialized"},   
    { CS_APIERROR_INVALIDCHIPTYPE,"Invalid Chip Type Specifier"}, 
    { CS_APIERROR_UNKNOWN,"Unknown"},  
    { CSALLOC_APIERROR_INVALIDMEMORYTYPE, "csAlloc::Invalid Memory Type"},   
    { CSALLOC_APIERROR_INVALIDCOMB,"csAlloc::Invalid Combination"}, 
    { CSALLOC_APIERROR_INVALIDSIZE,"csAlloc::Invalid Size"}, 
    { CSALLOC_APIERROR_INVALIDASPECT,"csAlloc::Invalid Aspect Ratio"}, 
    { CSALLOC_APIERROR_INVALIDBUFFERTYPE,"csAlloc::Invalid Buffer Type"}, 
    { CSALLOC_APIERROR_INVALIDLOCALE,"csAlloc::Invalid Memory Locale"}, 
    { CSSETVIDEOMODE_STATUS_BADREFRESH,"csSetVideoMode::Bad Refresh Rate"},     
    { CSSETVIDEOMODE_APIERROR_NOTFULLSCREEN,"csSetVideoMode::Not A Full Screen Context"},   
    { CSSETVIDEOMODE_APIERROR_UNSUPPORTEDRES,"csSetVideoMode::UnSupported Resolution"},  
    { CSSETVIDEOMODE_APIERROR_AAUNSUPPORTED,"csSetVideoMode::Anti-Aliasing UnSupported"},   
    { CSSETVIDEOMODE_APIERROR_OVERLAYBUSY,"csSetVideoMode::Overlay Hardware Busy" },     
    { CSEXECUTEBUF_APIERROR_SERVERBUSY, "csExecuteBuffer::Server Is Currently Busy"},  
    { CSSWAPBUFFERTODISPLAY_APIERROR_UNSUPPORTEDCONV, "csSwapBufferToDisplay::UnSupported Conversion Type" },        
    { CSSWAPBUFFERTODISPLAY_APIERROR_UNSUPPORTEDSCALE, "csSwapBufferToDisplay::UnSupported Scale" }, 
    { CSLOCK_APIERROR_UNSUPPORTEDLOCK, "csLock::UnSupported Lock" }, 
    { CSLOCK_APIERROR_NOLOCKAVAILABLE, "csLock:;No Lock Available" }, 
    { CSUNLOCK_APIERROR_NOTLOCKED, "csUnLock::Buffer Not Locked" }, 
    { CSINIT_APIERROR_ALREADYINITIALIZED, "csInit::System Already Initialized" }, 
    { CS_APIERROR_GRAPHICALCONTEXTNOTINITIALIZED, "CSGRAPHICALCONTEXT Not Initialized" },
    { CS_APIERROR_GRAPHICALCONTEXTINITIALIZED, "CSGRAPHICALCONTEXT Already Initialized" },
    { CS_APIERROR_ALLOCATIONDESCRIPTORINITIALIZED, "CSALLOCATIONDESCRIPTOR Already Initialized" },
    { CS_APIERROR_ALLOCATIONDESCRIPTORNOTINITIALIZED, "CSALLOCATIONDESCRIPTOR Not Initialized" },
    { CS_APIERROR_INVALIDSTRUCTUREPARAM, "Invalid Structure Parameter" },
    { CSFREE_APIERROR_STILLLOCKED, "csFree::Descriptor Still Locked" },
    { CS_APIERROR_OVERLAYINITIALIZED, "CSOVERLAY Already Initialized" },
    { CS_APIERROR_OVERLAYINITIALIZED, "CSOVERLAY Already Initialized" },
    { CS_APIERROR_SYSTEMFAILEDEXTESCAPE, "System Failed ExtEscape Call" },
    { CS_APIERROR_SERVERFAILEDEXTESCAPE, "Server Failed ExtEscape Call" },
    { CSSWAPBUFFERTODISPLAY_APIERROR_CANTGETCLIPLISTSIZE, "Unable To Get The DirectDraw Clip List Size" },
    { CSSWAPBUFFERTODISPLAY_APIERROR_CANTGETCLIPLIST, "Unable To Get The DirectDraw Clip List" },
#ifdef WIN32 
    { CS_APIERROR_DIRECTDRAWFAILED, "DirectDraw Initialization Failed" },
    { CSGETGRAPHICALCONTEXT_APIERROR_CANTCREATECLIPPER, "Unable To Create DirectDraw Clipper" },
    { CSSWAPBUFFERTODISPLAY_APIERROR_CANTASSOCIATEWINDOWTOCLIP, "Unable To Associate idDestWindow To DirectDraw Clipper" },
#endif /* WIN32 */
    { 0x00, ( PFxSz )0x00 }
};

PFxSz FX_CALL csDEBUGGetStringForError( CSRESULT idError )
{
    PCSDEBUGERRORCODESTRING psErrorCodeString = ( PCSDEBUGERRORCODESTRING )&sDEBUGErrorCodeStrings;
    static FxU8 szError[ SERVER_ERROR_STRING_SIZE ];

    /* If the flag is set to indicate this is a server error then call the server to get
       the error, else search the list of strings for the error */
    if( idError & CSDEBUG_ERROR_TYPE_SERVER )
    {
#ifdef WIN32
        int s32Return;
        CSSERVERREQ sServerReq;
        CSSERVERRES sServerRes;

        /* Setup the structure for the request */
        sServerReq.u32ReqID     = CSREQ_GETSERVERERRORSTRING;
        sServerReq.unionReqData.sGetServerErrorStringReq.idError      = idError;

        /* Call the server to make the request */
        s32Return = ExtEscape( 
            ( HDC )GetDC(NULL),
            CS_EXTESCAPE, 
            sizeof( CSSERVERREQ ), ( LPSTR )&sServerReq, 
            sizeof( CSSERVERRES ), ( LPSTR )&sServerRes );

        /* Copy the string into the local string array */
        memcpy( &szError, &sServerRes.unionResData.sGetServerErrorStringRes.szError, 
            strlen( ( const char * )&sServerRes.unionResData.sGetServerErrorStringRes.szError ) );

        /* Return the error string */
        return( ( PFxSz )&szError );
#endif /* WIN32 */
    }
    else
    {
        /* Go through the list of error types until the correct one is found and return the
           string associated with that error, if no error is found then return the string 
           indicating that the error code was not in the list */
        while( psErrorCodeString->pszErrorString )
        {
            if( psErrorCodeString->idError == idError )
                return( psErrorCodeString->pszErrorString );
            psErrorCodeString++;
        }
    }

    // Return that there was no error string found
    return( "csDEBUGGetDebugStringForError::No Error String For Error Code" );
}


/*****************************************************************************
**
**  Function        : csDEBUGCreateStringWithIndent
**
**  Parameters      : PFxSz pszString - String to add indent to
**                    PFxSz pszText - Text to be added to the end of the string
**                    FxU32 u32Indent - Number of space to index
**
**  Purpose         : Add space to the start of a string for indention and 
**                    concatonate another string to the end
**
**  Return Value    : PFxSz - pszString
**
**  Programmer(s)   : Don Fowler
**
*****************************************************************************/

PFxSz FX_CALL csDEBUGCreateStringWithIndent( PFxSz pszString, PFxSz pszText,
                                             FxU32 u32Indent )
{
    FxU32 u32Index;

    /* Concatonate space to the start of the string, one for each indent */
    pszString[ 0x00 ] = 0x00;
    for( u32Index = 0x00; u32Index < u32Indent; u32Index++ )
        strcat( pszString, " " );

    /* Add the text to the end of the string */
    strcat( pszString, pszText );

    /* Return a pointer to the string */
    return( pszString );
}

/*****************************************************************************
**
**  Function        : csDEBUGDisplayStruct
**
**  Parameters      : FxU32 u32Level - Debug level of the output 
**                    FxU32 u32StructID - Debug ID of the structure to display
**                    PFxVOID pvStructure - Data for the structure to display 
**
**  Purpose         : Display the specific structure type to the debug output
**
**  Return Value    : FxVOID
**
**  Programmer(s)   : Don Fowler
**
*****************************************************************************/

/* Array of names for each of the structures in the system that the debug output
   knows how to display */
static PFxSz pszDEBUGStructNames[] = 
{
    "CSCLIENTLIBDATA",
    "CSSERVERREQ",
    "CSSERVERRES",
    "CSGRAPHICALCONTEXT",
    "CSALLOCATIONDESCRIPTOR",
    "CSSWAPBUFFERTODISPLAY",
    "CSRECT",
    "CSCLIPLIST",
    "CSINIT",
    "CSCHIPSPECIFICDATA",
    "CSSST1",
    "CSSST2",
    "CSVIDEOMODE",
    "CSOVERLAY",
    "CSDEVICECONFIG",
#ifdef WIN32
    "OSVERSIONINFO",
#endif /* WIN32 */
    /* The following strings must be kept in order */
    "CSGETPROTOCOLREVISIONREQ",
    "CSGETPROTOCOLREVISIONRES",
    "CSGETGRAPHICALCONTEXTREQ",
    "CSGETGRAPHICALCONTEXTRES",
    "CSRELEASEGRAPHICALCONTEXTREQ",
    "CSRELEASEGRAPHICALCONTEXTRES",
    "CSALLOCREQ",
    "CSALLOCRES",
    "CSFREEREQ",
    "CSFREERES",
    "CSSETVIDEOMODEREQ",
    "CSSETVIDEOMODERES",
    "CSSWAPBUFFERTODISPLAYREQ",
    "CSSWAPBUFFERTODISPLAYRES",
    "CSEXECUTECOMMANDSREQ",
    "CSEXECUTECOMMANDSRES",
    "CSLOCKREQ",
    "CSLOCKRES",
    "CSUNLOCKREQ",
    "CSUNLOCKRES",
    "CSGETSERVERERRORCODEREQ",
    "CSGETSERVERERRORCODERES",
    "CSTIME"
};

/* Array of strings for each of the CSSERVERREQ/CSSERVERRES request ID */
static PFxSz pszDEBUGServerReqIDString[] = 
{
    "CSREQ_NOTAREQUEST",                     
    "CSREQ_GETPROTOCOLREVISION",             
    "CSREQ_GETGRAPHICALCONTEXT", 
    "CSREQ_RELEASEGRAPHICALCONTEXT",
    "CSREQ_ALLOC",                           
    "CSREQ_FREE",                            
    "CSREQ_SETVIDEOMODE",                    
    "CSREQ_ACQUIREOVERLAY",                  
    "CSREQ_SWAPBUFFERTODISPLAY",             
    "CSREQ_EXECUTECOMMANDS",                 
    "CSREQ_LOCK",                            
    "CSREQ_UNLOCK",
    "CSREQ_GETSERVERERRORSTRING"
  
};

FxVOID FX_CALL csDEBUGDisplayStruct( FxU32 u32Level, FxU32 u32StructID, PFxVOID pvStructure )
{
    static FxSz szTemp1[ 512 ];
    static FxSz szTemp2[ 512 ];
    static FxU32 u32IndentDepth = 0x00;
    PCSCLIENTLIBDATA psCSClientLibData = ( PCSCLIENTLIBDATA )pvStructure;
    PCSSERVERREQ psCSServerReq = ( PCSSERVERREQ )pvStructure;
    PCSSERVERRES psCSServerRes = ( PCSSERVERRES )pvStructure;
    PCSGRAPHICALCONTEXT psCSGraphicalContext = ( PCSGRAPHICALCONTEXT )pvStructure;
    PCSALLOCATIONDESCRIPTOR psCSAllocationDescriptor = ( PCSALLOCATIONDESCRIPTOR )pvStructure;
    PCSGETPROTOCOLREVISIONREQ psCSGetProtocolRevisionReq =
        ( PCSGETPROTOCOLREVISIONREQ )pvStructure;
    PCSGETPROTOCOLREVISIONRES psCSGetProtocolRevisionRes = 
        ( PCSGETPROTOCOLREVISIONRES )pvStructure;
    PCSSWAPBUFFERTODISPLAY psCSSwapBufferToDisplay = ( PCSSWAPBUFFERTODISPLAY )pvStructure;
    PCSRECT psCSRect = ( PCSRECT )pvStructure;
    PCSCLIPLIST psCSClipList = ( PCSCLIPLIST )pvStructure;
    PCSINIT psCSInit = ( PCSINIT )pvStructure;
    PCSSST1 psCSsst1 = ( PCSSST1 )pvStructure;
    PCSSST2 psCSsst2 = ( PCSSST2 )pvStructure;
    PCSCHIPSPECIFICDATA psCSChipSpecificData = ( PCSCHIPSPECIFICDATA )pvStructure;
    PCSVIDEOMODE psCSVideoMode = ( PCSVIDEOMODE )pvStructure;
    PCSOVERLAY psCSOverlay = ( PCSOVERLAY )pvStructure;
    PCSDEVICECONFIG psCSDeviceConfig = ( PCSDEVICECONFIG )pvStructure;
    PCSGETGRAPHICALCONTEXTREQ psCSGetGraphicalContextReq = ( PCSGETGRAPHICALCONTEXTREQ )pvStructure;
    PCSGETGRAPHICALCONTEXTRES psCSGetGraphicalContextRes = ( PCSGETGRAPHICALCONTEXTRES )pvStructure;
    PCSRELEASEGRAPHICALCONTEXTREQ psCSReleaseGraphicalContextReq = ( PCSRELEASEGRAPHICALCONTEXTREQ )pvStructure;
    PCSRELEASEGRAPHICALCONTEXTRES psCSReleaseGraphicalContextRes = ( PCSRELEASEGRAPHICALCONTEXTRES )pvStructure;
    PCSALLOCREQ psCSAllocReq = ( PCSALLOCREQ )pvStructure;
    PCSALLOCRES psCSAllocRes = ( PCSALLOCRES )pvStructure;
    PCSFREEREQ psCSFreeReq = ( PCSFREEREQ )pvStructure;
    PCSFREERES psCSFreeRes = ( PCSFREERES )pvStructure;
    PCSSETVIDEOMODEREQ psCSSetVideoModeReq = ( PCSSETVIDEOMODEREQ )pvStructure;
    PCSSETVIDEOMODERES psCSSetVideoModeRes = ( PCSSETVIDEOMODERES )pvStructure;
    PCSSWAPBUFFERTODISPLAYREQ psCSSwapBufferToDisplayReq = ( PCSSWAPBUFFERTODISPLAYREQ )pvStructure;
    PCSSWAPBUFFERTODISPLAYRES psCSSwapBufferToDisplayRes = ( PCSSWAPBUFFERTODISPLAYRES )pvStructure;
    PCSEXECUTECOMMANDSREQ psCSExecuteCommandsReq = ( PCSEXECUTECOMMANDSREQ )pvStructure;
    PCSEXECUTECOMMANDSRES psCSExecuteCommandsRes = ( PCSEXECUTECOMMANDSRES )pvStructure;
    PCSLOCKREQ psCSLockReq = ( PCSLOCKREQ )pvStructure;
    PCSLOCKRES psCSLockRes = ( PCSLOCKRES )pvStructure;
    PCSUNLOCKREQ psCSUnLockReq = ( PCSUNLOCKREQ )pvStructure;
    PCSUNLOCKRES psCSUnLockRes = ( PCSUNLOCKRES )pvStructure;
    PCSGETSERVERERRORSTRINGREQ psCSGetServerErrorStringReq = ( PCSGETSERVERERRORSTRINGREQ )pvStructure;
    PCSGETSERVERERRORSTRINGRES psCSGetServerErrorStringRes = ( PCSGETSERVERERRORSTRINGRES )pvStructure;
    PCSTIME psCSTime = ( PCSTIME )pvStructure;

#ifdef WIN32
    OSVERSIONINFO * psOSVersionInfo = ( OSVERSIONINFO * )pvStructure;
#endif /* WIN32 */

    /* To speed things up, check to see if the debug level is appropriate */
    if( ( u32Level > u32DEBUGLevel ) || ( u32DEBUGInitialized != FXTRUE ) )
        return;

    /* Add to the indent depth. This is used to display nested structures */
    u32IndentDepth += 0x03;

    /* Structure name and formatting */
    csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "struct %s\n", u32IndentDepth - 0x03 );
    csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1,
        pszDEBUGStructNames[ u32StructID ] );
    csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "{\n", u32IndentDepth - 0x03 );
    csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1 );

    /* check to see which type of structure to display */
    switch( u32StructID )
    {
        case DEBUG_STRUCT_CSCLIENTLIBDATA:

            szTemp1[ 0x00 ] = 0x00;
            if( psCSClientLibData->u32Flags & CSCLIENTLIBDATA_FLAGS_INITIALIZED )
                strcat( ( char * )( PFxSz )&szTemp1, "CSCLIENTLIBDATA_FLAGS_INITIALIZED" );
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp2, "u32Flags = %s\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp2, ( PFxSz )&szTemp1 );
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "psInit = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSClientLibData->psInit );            
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "psInit = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSClientLibData->psInit );            

            if( psCSClientLibData->psInit )
            {
                csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSINIT, 
                    ( PFxVOID )psCSClientLibData->psInit );
            }

#ifdef WIN32
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "sProcessTimeStamp\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1 );            
            csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSTIME, 
                ( PFxVOID )&psCSClientLibData->sProcessTimeStamp );
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "hDesktopDC = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSClientLibData->hDesktopDC );            
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "psDirectDraw = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSClientLibData->psDirectDraw );            
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "hWindowSnoopHook = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSClientLibData->hWindowSnoopHook );
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "hKeyboardSnoopHook = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSClientLibData->hKeyboardSnoopHook );
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "sOSVersionInfo = \n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1 );
            csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_OSVERSIONINFO, 
                ( PFxVOID )&psCSClientLibData->sOSVersionInfo );
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32SnoopHooksEnabled = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSClientLibData->u32SnoopHooksEnabled );            
#endif /* WIN32 */
            break;

        case DEBUG_STRUCT_CSSERVERREQ:
#ifdef WIN32
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32Flags = UNDEFINED\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1 ); 
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32ReqID = %s\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, 
                pszDEBUGServerReqIDString[ psCSServerReq->u32ReqID ] );
#endif
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "sGrahicalContext = \n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1 );
            csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSGRAPHICALCONTEXT, 
                ( PFxVOID )&psCSServerReq->sGraphicalContext ); 

            switch( psCSServerReq->u32ReqID )
            {
                case CSREQ_GETGRAPHICALCONTEXT:
                    csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSGETGRAPHICALCONTEXTREQ,
                        ( PFxVOID )&psCSServerReq->unionReqData.sLockReq ); 
                    break;
                case CSREQ_GETPROTOCOLREVISION:
                    csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSGETPROTOCOLREVISIONREQ,
                        ( PFxVOID )&psCSServerReq->unionReqData.sLockReq ); 
                    break;
                case CSREQ_RELEASEGRAPHICALCONTEXT:
                    csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSRELEASEGRAPHICALCONTEXTREQ,
                        ( PFxVOID )&psCSServerReq->unionReqData.sLockReq ); 
                    break;
                case CSREQ_ALLOC:
                    csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSALLOCREQ,
                        ( PFxVOID )&psCSServerReq->unionReqData.sLockReq ); 
                    break;
                case CSREQ_FREE:
                    csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSFREEREQ,
                        ( PFxVOID )&psCSServerReq->unionReqData.sLockReq ); 
                    break;
                case CSREQ_SETVIDEOMODE:
                    csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSSETVIDEOMODEREQ,
                        ( PFxVOID )&psCSServerReq->unionReqData.sLockReq ); 
                    break;
                case CSREQ_SWAPBUFFERTODISPLAY:
                    csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSSWAPBUFFERTODISPLAYREQ,
                        ( PFxVOID )&psCSServerReq->unionReqData.sLockReq ); 
                    break;
                case CSREQ_EXECUTECOMMANDS:
                    csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSEXECUTECOMMANDSREQ,
                        ( PFxVOID )&psCSServerReq->unionReqData.sLockReq ); 
                    break;
                case CSREQ_LOCK:
                    csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSLOCKREQ,
                        ( PFxVOID )&psCSServerReq->unionReqData.sLockReq ); 
                    break;
                case CSREQ_UNLOCK:
                    csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSUNLOCKREQ,
                        ( PFxVOID )&psCSServerReq->unionReqData.sLockReq ); 
                    break;
                case CSREQ_GETSERVERERRORSTRING:
                    csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSGETSERVERERRORSTRINGREQ,
                        ( PFxVOID )&psCSServerReq->unionReqData.sGetServerErrorStringReq ); 
                    break;
            }

            break;

        case DEBUG_STRUCT_CSSERVERRES:
#ifdef WIN32
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32Flags = UNDEFINED\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1 ); 
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32Status = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSServerRes->u32Status );
#endif
            csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSGRAPHICALCONTEXT, 
                ( PFxVOID )&psCSServerRes->sGraphicalContext ); 

            switch( psCSServerRes->u32ReqID )
            {
                case CSREQ_GETGRAPHICALCONTEXT:
                    csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSGETGRAPHICALCONTEXTRES,
                        ( PFxVOID )&psCSServerRes->unionResData.sLockRes ); 
                    break;
                case CSREQ_GETPROTOCOLREVISION:
                    csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSGETPROTOCOLREVISIONREQ,
                        ( PFxVOID )&psCSServerRes->unionResData.sLockRes ); 
                    break;
                case CSREQ_RELEASEGRAPHICALCONTEXT:
                    csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSRELEASEGRAPHICALCONTEXTRES,
                        ( PFxVOID )&psCSServerRes->unionResData.sLockRes ); 
                    break;
                case CSREQ_ALLOC:
                    csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSALLOCRES,
                        ( PFxVOID )&psCSServerRes->unionResData.sLockRes ); 
                    break;
                case CSREQ_FREE:
                    csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSFREERES,
                        ( PFxVOID )&psCSServerRes->unionResData.sLockRes ); 
                    break;
                case CSREQ_SETVIDEOMODE:
                    csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSSETVIDEOMODERES,
                        ( PFxVOID )&psCSServerRes->unionResData.sLockRes ); 
                    break;
                case CSREQ_SWAPBUFFERTODISPLAY:
                    csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSSWAPBUFFERTODISPLAYRES,
                        ( PFxVOID )&psCSServerRes->unionResData.sLockRes ); 
                    break;
                case CSREQ_EXECUTECOMMANDS:
                    csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSEXECUTECOMMANDSRES,
                        ( PFxVOID )&psCSServerRes->unionResData.sLockRes ); 
                    break;
                case CSREQ_LOCK:
                    csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSLOCKRES,
                        ( PFxVOID )&psCSServerRes->unionResData.sLockRes ); 
                    break;
                case CSREQ_UNLOCK:
                    csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSUNLOCKRES,
                        ( PFxVOID )&psCSServerRes->unionResData.sLockRes ); 
                    break;
                case CSREQ_GETSERVERERRORSTRING:
                    csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSGETSERVERERRORSTRINGRES,
                        ( PFxVOID )&psCSServerRes->unionResData.sGetServerErrorStringRes ); 
                    break;
            }

            break;

        case DEBUG_STRUCT_CSGRAPHICALCONTEXT:

            szTemp1[ 0x00 ] = 0x00;
            if( psCSGraphicalContext->u32Flags & CSGRAPHICALCONTEXT_FLAGS_INITIALIZED )
                strcat( ( char * )( PFxSz )&szTemp1, "CSGRAPHICALCONTEXT_FLAGS_INITIALIZED" );
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp2, "u32Flags = %s\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp2, ( PFxSz )&szTemp1 );
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32ContextID = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, 
                psCSGraphicalContext->u32ContextID );
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "idWindow = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, 
                psCSGraphicalContext->idWindow );
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "idProcess = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, 
                psCSGraphicalContext->idProcess );
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "sProcessTimeStamp\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1 );            
            csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSTIME, 
                ( PFxVOID )&psCSClientLibData->sProcessTimeStamp );
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "idThread = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, 
                psCSGraphicalContext->idThread );

            /* Display the video mode structures */

            /* Display the overlay structure */

            csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSDEVICECONFIG, 
                ( PFxVOID )&psCSGraphicalContext->sDeviceConfig ); 
#ifdef WIN32
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "hDC = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, 
                psCSGraphicalContext->hDC );
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "psDirectDrawClipper = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, 
                psCSGraphicalContext->psDirectDrawClipper );
#endif /* WIN32 */

            csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSCHIPSPECIFICDATA, 
                ( PFxVOID )&psCSGraphicalContext->sChipSpecificData ); 
            
            break;

        case DEBUG_STRUCT_CSALLOCATIONDESCRIPTOR:
            szTemp1[ 0x00 ] = 0x00;
            if( psCSAllocationDescriptor->u32Flags & CSALLOCATIONDESCRIPTOR_FLAGS_INITIALIZED )
                strcat( ( char * )( PFxSz )&szTemp1, "CSALLOCATIONDESCRIPTOR_FLAGS_INITIALIZED" );
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp2, "u32Flags = %s\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp2, ( PFxSz )&szTemp1 );
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32BufferID = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, 
                psCSAllocationDescriptor->u32BufferID );
            szTemp1[ 0x00 ] = 0x00;
            if( psCSAllocationDescriptor->u32BufferType == CS_BUFFER_FIFO )
                strcat( ( char * )( PFxSz )&szTemp1, "CS_BUFFER_FIFO " );
            if( psCSAllocationDescriptor->u32BufferType == CS_BUFFER_PERSISTENT )
                strcat( ( char * )( PFxSz )&szTemp1, "CS_BUFFER_PERSISTENT " );
            if( psCSAllocationDescriptor->u32BufferType == CS_BUFFER_RENDER )
                strcat( ( char * )( PFxSz )&szTemp1, "CS_BUFFER_RENDER " );
            if( psCSAllocationDescriptor->u32BufferType == CS_BUFFER_TEXTUREHEAP )
                strcat( ( char * )( PFxSz )&szTemp1, "CS_BUFFER_TEXTUREHEAP " );
            if( psCSAllocationDescriptor->u32BufferType == CS_BUFFER_TEXTURE )
                strcat( ( char * )( PFxSz )&szTemp1, "CS_BUFFER_TEXTURE " );
            if( psCSAllocationDescriptor->u32BufferType == CS_BUFFER_ZBUFFER )
                strcat( ( char * )( PFxSz )&szTemp1, "CS_BUFFER_ZBUFFER " );
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp2, "u32BufferType = %s\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp2, ( PFxSz )&szTemp1 );
            szTemp1[ 0x00 ] = 0x00;
            if( psCSAllocationDescriptor->u32MemType == CS_MEMORY_LINEAR )
                strcat( ( char * )( PFxSz )&szTemp1, "CS_MEMORY_LINEAR " );
            if( psCSAllocationDescriptor->u32MemType == CS_MEMORY_TILED )
                strcat( ( char * )( PFxSz )&szTemp1, "CS_MEMORY_TILED " );
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp2, "u32MemType = %s\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp2, ( PFxSz )&szTemp1 );
            szTemp1[ 0x00 ] = 0x00;
            if( psCSAllocationDescriptor->u32Locale == CS_LOCALE_FRAMEBUFFER )
                strcat( ( char * )( PFxSz )&szTemp1, "CS_LOCALE_FRAMEBUFFER " );
            if( psCSAllocationDescriptor->u32Locale == CS_LOCALE_AGP )
                strcat( ( char * )( PFxSz )&szTemp1, "CS_LOCALE_AGP " );
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp2, "u32Locale = %s\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp2, ( PFxSz )&szTemp1 );
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32Size = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, 
                psCSAllocationDescriptor->u32Size );
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32Width = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, 
                psCSAllocationDescriptor->u32Width );
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32Height = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, 
                psCSAllocationDescriptor->u32Height );
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32Depth = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, 
                psCSAllocationDescriptor->u32Depth );
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32PhysicalOffset = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, 
                psCSAllocationDescriptor->u32PhysicalOffset );
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "pvLinearAddress = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, 
                psCSAllocationDescriptor->pvLinearAddress );
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32LockCount = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, 
                psCSAllocationDescriptor->u32LockCount );
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32LockFlags = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, 
                psCSAllocationDescriptor->u32LockFlags );
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32LinearStride = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, 
                psCSAllocationDescriptor->u32LinearStride );
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32LFBDepth = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, 
                psCSAllocationDescriptor->u32LFBDepth );
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32PhysicalStride = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, 
                psCSAllocationDescriptor->u32PhysicalStride );
            csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSCHIPSPECIFICDATA, 
                ( PFxVOID )&psCSAllocationDescriptor->sChipSpecificData ); 
            break;
            
        case DEBUG_STRUCT_CSSWAPBUFFERTODISPLAY:

            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32Flags = UNDEFINED\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1 );
            csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSALLOCATIONDESCRIPTOR, 
                ( PFxVOID )psCSSwapBufferToDisplay->psSrcBufferAllocationDescriptor );
            csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSRECT, 
                ( PFxVOID )&psCSSwapBufferToDisplay->sDestClipRegion );
//          csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32DestX = 0x%x\n", u32IndentDepth );
//          csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSSwapBufferToDisplay->u32DestX );
//          csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32DestY = 0x%x\n", u32IndentDepth );
//          csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSSwapBufferToDisplay->u32DestY );
//          csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSRECT, ( PFxVOID )&psCSSwapBufferToDisplay->sSrcClipRegion );
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32DestWindow = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSSwapBufferToDisplay->idDestWindow );
//          csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32SrcColorFormat = 0x%x\n", u32IndentDepth );
//          csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSSwapBufferToDisplay->u32SrcColorFormat );
//          csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32InterpolationType = 0x%x\n", u32IndentDepth );
//          csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSSwapBufferToDisplay->u32InterpolationType );
            csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSCLIPLIST, 
                ( PFxVOID )&psCSSwapBufferToDisplay->sClipList );

            break;

        case DEBUG_STRUCT_CSCLIPLIST:
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32Flags = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSClipList->u32Flags );
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32TotalRegions = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSClipList->u32TotalRegions );
            {
                FxU32 u32Index = 0x00;
                while( u32Index < psCSClipList->u32TotalRegions && u32Index < CS_CLIPREGIONS_MAX )
                {
                    csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSRECT, 
                        ( PFxVOID )&psCSClipList->sClipRegion[ u32Index++ ] );
                }
            }       
            break;

        case DEBUG_STRUCT_CSRECT:
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32Left = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSRect->u32Left );
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32Top = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSRect->u32Top );
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32Right = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSRect->u32Right );
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32Bottom = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSRect->u32Bottom );
            break;

        case DEBUG_STRUCT_CSTIME:
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32Year = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSTime->u32Year );
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32Month = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSTime->u32Month );
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32Day = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSTime->u32Day );
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32DayOfWeek = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSTime->u32DayOfWeek );
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32Hour = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSTime->u32Hour );
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32Minute = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSTime->u32Minute );
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32Second = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSTime->u32Second );
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32Milliseconds = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSTime->u32Milliseconds );

            break;

        case DEBUG_STRUCT_CSINIT:
#ifdef WIN32
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "hDLLInstance = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSInit->hDLLInstance );
#endif /* WIN32 */
            break;

        case DEBUG_STRUCT_CSGETPROTOCOLREVISIONREQ:
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32Minor = UNDEFINED\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1 );
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32Major = UNDEFINED\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1 );
            break;

        case DEBUG_STRUCT_CSGETPROTOCOLREVISIONRES:
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32Minor = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSGetProtocolRevisionRes->u32Minor );
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32Major = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSGetProtocolRevisionRes->u32Major );
            break;

        case DEBUG_STRUCT_CSGETSERVERERRORSTRINGREQ:
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "idError = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSGetServerErrorStringReq->idError );
            break;

        case DEBUG_STRUCT_CSGETSERVERERRORSTRINGRES:
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "idError = 0x%s\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&psCSGetServerErrorStringRes->szError, 0x00 );
            break;

        case DEBUG_STRUCT_CSSST1:
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32Flags = UNDEFINED\n", u32IndentDepth );
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32SLIAvailable = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSsst1->u32SLIAvailable );
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32NumSLIChips = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSsst1->u32NumSLIChips );
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32UseSLI = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSsst1->u32UseSLI );
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32DeviceRev = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSsst1->u32DeviceRev );
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32LFBRam = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSsst1->u32LFBRam );
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32AGPRam = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSsst1->u32AGPRam );
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32Antialiased = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSsst1->u32Antialiased );
            break;

        case DEBUG_STRUCT_CSSST2:
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32Flags = UNDEFINED\n", u32IndentDepth );
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32SLIAvailable = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSsst2->u32SLIAvailable );
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32NumSLIChips = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSsst2->u32NumSLIChips );
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32UseSLI = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSsst2->u32UseSLI );
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32DeviceRev = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSsst2->u32DeviceRev );
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32LFBRam = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSsst2->u32LFBRam );
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32AGPRam = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSsst2->u32AGPRam );
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32Antialiased = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSsst2->u32Antialiased );
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32CAMEntry = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSsst2->u32CAMEntry );
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32TileMode = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSsst2->u32TileMode );
            break;

        case DEBUG_STRUCT_CSCHIPSPECIFICDATA:

            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32Flags = UNDEFINED\n", u32IndentDepth );
            csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32ChipType = 0x%x\n", u32IndentDepth );
            csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSChipSpecificData->u32ChipType );

            switch( psCSChipSpecificData->u32ChipType )
            {
                case CSCHIPSPECIFICDATA_CHIPTYPE_SST2:
                    csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSSST2, 
                        ( PFxVOID )&psCSChipSpecificData->unionChipType.sSST2 ); 
                    break;

                case CSCHIPSPECIFICDATA_CHIPTYPE_SST1:
                    csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSSST1, 
                        ( PFxVOID )&psCSChipSpecificData->unionChipType.sSST1 ); 
                    break;
            }


            break;

            case DEBUG_STRUCT_CSVIDEOMODE:

                csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32Flags = UNDEFINED\n", u32IndentDepth );
                csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32Width = 0x%x\n", u32IndentDepth );
                csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSVideoMode->u32Width );
                csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32Height = 0x%x\n", u32IndentDepth );
                csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSVideoMode->u32Height );
                csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32Width = 0x%x\n", u32IndentDepth );
                csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSVideoMode->u32Width );

                szTemp1[ 0x00 ] = 0x00;
                if( psCSVideoMode->u32ColorFormat == CS_COLORFORMAT_8BPP )
                    strcat( ( char * )( PFxSz )&szTemp1, "CS_COLORFORMAT_8BPP " );
                if( psCSVideoMode->u32ColorFormat == CS_COLORFORMAT_DVD )
                    strcat( ( char * )( PFxSz )&szTemp1, "CS_COLORFORMAT_DVD " );
                if( psCSVideoMode->u32ColorFormat == CS_COLORFORMAT_RGB565 )
                    strcat( ( char * )( PFxSz )&szTemp1, "CS_COLORFORMAT_RGB565 " );
                if( psCSVideoMode->u32ColorFormat == CS_COLORFORMAT_RGB32 )
                    strcat( ( char * )( PFxSz )&szTemp1, "CS_COLORFORMAT_RGB32 " );
                if( psCSVideoMode->u32ColorFormat == CS_COLORFORMAT_UYVY )
                    strcat( ( char * )( PFxSz )&szTemp1, "CS_COLORFORMAT_UYVY " );
                if( psCSVideoMode->u32ColorFormat == CS_COLORFORMAT_YUYV )
                    strcat( ( char * )( PFxSz )&szTemp1, "CS_COLORFORMAT_YUYV " );                
                if( psCSVideoMode->u32ColorFormat == CS_COLORFORMAT_RGB1555 )
                    strcat( ( char * )( PFxSz )&szTemp1, "CS_COLORFORMAT_RGB1555 " );                
                if( psCSVideoMode->u32ColorFormat == CS_COLORFORMAT_YUVA )
                    strcat( ( char * )( PFxSz )&szTemp1, "CS_COLORFORMAT_YUVA " );                
                csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp2, "u32ColorFormat = %s\n", u32IndentDepth );
                csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp2, ( PFxSz )&szTemp1 );
                
                csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32RefreshRate = 0x%x\n", u32IndentDepth );
                csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSVideoMode->u32RefreshRate );
 
                csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSCHIPSPECIFICDATA, 
                    ( PFxVOID )&psCSVideoMode->sChipSpecificData ); 


                break;

            case DEBUG_STRUCT_CSOVERLAY:

                csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32Flags = UNDEFINED\n", u32IndentDepth );
                szTemp1[ 0x00 ] = 0x00;
                if( psCSOverlay->u32ColorFormat == CS_COLORFORMAT_8BPP )
                    strcat( ( char * )( PFxSz )&szTemp1, "CS_COLORFORMAT_8BPP " );
                if( psCSOverlay->u32ColorFormat == CS_COLORFORMAT_DVD )
                    strcat( ( char * )( PFxSz )&szTemp1, "CS_COLORFORMAT_DVD " );
                if( psCSOverlay->u32ColorFormat == CS_COLORFORMAT_RGB565 )
                    strcat( ( char * )( PFxSz )&szTemp1, "CS_COLORFORMAT_RGB565 " );
                if( psCSOverlay->u32ColorFormat == CS_COLORFORMAT_RGB32 )
                    strcat( ( char * )( PFxSz )&szTemp1, "CS_COLORFORMAT_RGB32 " );
                if( psCSOverlay->u32ColorFormat == CS_COLORFORMAT_UYVY )
                    strcat( ( char * )( PFxSz )&szTemp1, "CS_COLORFORMAT_UYVY " );
                if( psCSOverlay->u32ColorFormat == CS_COLORFORMAT_YUYV )
                    strcat( ( char * )( PFxSz )&szTemp1, "CS_COLORFORMAT_YUYV " );                
                if( psCSOverlay->u32ColorFormat == CS_COLORFORMAT_RGB1555 )
                    strcat( ( char * )( PFxSz )&szTemp1, "CS_COLORFORMAT_RGB1555 " );                
                if( psCSOverlay->u32ColorFormat == CS_COLORFORMAT_YUVA )
                    strcat( ( char * )( PFxSz )&szTemp1, "CS_COLORFORMAT_YUVA " );                
                csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp2, "u32ColorFormat = %s\n", u32IndentDepth );
                csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp2, ( PFxSz )&szTemp1 );

                csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32SrcWidth = 0x%x\n", u32IndentDepth );
                csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSOverlay->u32SrcWidth );
                csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32SrcHeight = 0x%x\n", u32IndentDepth );
                csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSOverlay->u32SrcHeight );
                csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32DestX = 0x%x\n", u32IndentDepth );
                csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSOverlay->u32DestX );
                csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32DestY = 0x%x\n", u32IndentDepth );
                csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSOverlay->u32DestY );
                csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32DestWidth = 0x%x\n", u32IndentDepth );
                csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSOverlay->u32DestWidth );
                csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32DestHeight = 0x%x\n", u32IndentDepth );
                csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSOverlay->u32DestHeight );

                csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSCHIPSPECIFICDATA, 
                    ( PFxVOID )&psCSOverlay->sChipSpecificData ); 
                break;

            case DEBUG_STRUCT_CSDEVICECONFIG:

                csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32Flags = UNDEFINED\n", u32IndentDepth );
                csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1 );
                csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32DeviceID = 0x%x\n", u32IndentDepth );
                csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSDeviceConfig->u32DeviceID );
                break;

#ifdef WIN32
            case DEBUG_STRUCT_OSVERSIONINFO:

                csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "dwOSVersionInfoSize = 0x%x\n", u32IndentDepth );
                csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psOSVersionInfo->dwOSVersionInfoSize );
                csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "dwMajorVersion = 0x%x\n", u32IndentDepth );
                csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psOSVersionInfo->dwMajorVersion );
                csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "dwMinorVersion = 0x%x\n", u32IndentDepth );
                csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psOSVersionInfo->dwMinorVersion );
                csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "dwBuildNumber = 0x%x\n", u32IndentDepth );
                csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psOSVersionInfo->dwBuildNumber );
                csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "dwPlatformId = 0x%x\n", u32IndentDepth );
                csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psOSVersionInfo->dwPlatformId );
                csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "dwOSVersionInfoSize = %s\n", u32IndentDepth );
                csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, &psOSVersionInfo->szCSDVersion );

                break;
#endif /* WIN32 */

            case DEBUG_STRUCT_CSGETGRAPHICALCONTEXTREQ:

                csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSGRAPHICALCONTEXT, 
                    ( PFxVOID )&psCSGetGraphicalContextReq->sGraphicalContext );                 
                break;

            case DEBUG_STRUCT_CSGETGRAPHICALCONTEXTRES:

                csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSGRAPHICALCONTEXT, 
                    ( PFxVOID )&psCSGetGraphicalContextRes->sGraphicalContext );                 
                break;

            case DEBUG_STRUCT_CSRELEASEGRAPHICALCONTEXTREQ:
                csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSGRAPHICALCONTEXT, 
                    ( PFxVOID )&psCSReleaseGraphicalContextReq->sGraphicalContext ); 
                break;

            case DEBUG_STRUCT_CSRELEASEGRAPHICALCONTEXTRES:
                csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSGRAPHICALCONTEXT, 
                    ( PFxVOID )&psCSReleaseGraphicalContextRes->sGraphicalContext ); 
                break;

            case DEBUG_STRUCT_CSALLOCREQ:
                csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSALLOCATIONDESCRIPTOR, 
                    ( PFxVOID )&psCSAllocReq->sAllocationDescriptor ); 
                break;
            case DEBUG_STRUCT_CSALLOCRES:
                csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSALLOCATIONDESCRIPTOR, 
                    ( PFxVOID )&psCSAllocRes->sAllocationDescriptor ); 
                break;
            case DEBUG_STRUCT_CSFREEREQ:
                csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSALLOCATIONDESCRIPTOR, 
                    ( PFxVOID )&psCSFreeReq->sAllocationDescriptor ); 
                break;
            case DEBUG_STRUCT_CSFREERES:
                csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSALLOCATIONDESCRIPTOR, 
                    ( PFxVOID )&psCSFreeRes->sAllocationDescriptor ); 
                break;
            case DEBUG_STRUCT_CSSETVIDEOMODERES:
                csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSVIDEOMODE, 
                    ( PFxVOID )&psCSSetVideoModeRes->sVideoMode ); 
                break;
            case DEBUG_STRUCT_CSSETVIDEOMODEREQ:
                csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSVIDEOMODE, 
                    ( PFxVOID )&psCSSetVideoModeReq->sVideoMode ); 
                break;
            case DEBUG_STRUCT_CSSWAPBUFFERTODISPLAYREQ:
                csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSSWAPBUFFERTODISPLAY, 
                    ( PFxVOID )&psCSSwapBufferToDisplayReq->sSwapBufferToDisplay ); 
                break;
            case DEBUG_STRUCT_CSSWAPBUFFERTODISPLAYRES:
                csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSSWAPBUFFERTODISPLAY, 
                    ( PFxVOID )&psCSSwapBufferToDisplayRes->sSwapBufferToDisplay ); 
                break;
            case DEBUG_STRUCT_CSEXECUTECOMMANDSRES:
                csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "sStateAllocationDescriptor =\n", u32IndentDepth );
                csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1 );
                csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSALLOCATIONDESCRIPTOR, 
                    ( PFxVOID )&psCSExecuteCommandsRes->sStateAllocationDescriptor ); 
                csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "sCommandAllocationDescriptor =\n", u32IndentDepth );
                csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1 );
                csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSALLOCATIONDESCRIPTOR, 
                    ( PFxVOID )&psCSExecuteCommandsRes->sCommandAllocationDescriptor ); 
                csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "sSentinelAllocationDescriptor =\n", u32IndentDepth );
                csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1 );
                csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSALLOCATIONDESCRIPTOR, 
                    ( PFxVOID )&psCSExecuteCommandsRes->sSentinelAllocationDescriptor ); 
                csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32StateOffset = 0x%x\n", u32IndentDepth );
                csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSExecuteCommandsRes->u32StateOffset );
                csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32CommandOffset = 0x%x\n", u32IndentDepth );
                csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSExecuteCommandsRes->u32CommandOffset );

                csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32StateSize = 0x%x\n", u32IndentDepth );
                csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSExecuteCommandsRes->u32StateSize );
                csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32CommandSize = 0x%x\n", u32IndentDepth );
                csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSExecuteCommandsRes->u32CommandSize );
                csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32SentinelSerial = 0x%x\n", u32IndentDepth );
                csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSExecuteCommandsRes->u32SentinelSerial );

                break;
            case DEBUG_STRUCT_CSEXECUTECOMMANDSREQ:
                csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "sStateAllocationDescriptor =\n", u32IndentDepth );
                csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1 );
                csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSALLOCATIONDESCRIPTOR, 
                    ( PFxVOID )&psCSExecuteCommandsReq->sStateAllocationDescriptor ); 
                csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "sCommandAllocationDescriptor =\n", u32IndentDepth );
                csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1 );
                csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSALLOCATIONDESCRIPTOR, 
                    ( PFxVOID )&psCSExecuteCommandsReq->sCommandAllocationDescriptor ); 
                csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "sSentinelAllocationDescriptor =\n", u32IndentDepth );
                csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1 );
                csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSALLOCATIONDESCRIPTOR, 
                    ( PFxVOID )&psCSExecuteCommandsReq->sSentinelAllocationDescriptor ); 
                csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32StateOffset = 0x%x\n", u32IndentDepth );
                csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSExecuteCommandsReq->u32StateOffset );
                csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32CommandOffset = 0x%x\n", u32IndentDepth );
                csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSExecuteCommandsReq->u32CommandOffset );

                csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32StateSize = 0x%x\n", u32IndentDepth );
                csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSExecuteCommandsReq->u32StateSize );
                csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32CommandSize = 0x%x\n", u32IndentDepth );
                csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSExecuteCommandsReq->u32CommandSize );
                csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "u32SentinelSerial = 0x%x\n", u32IndentDepth );
                csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1, psCSExecuteCommandsReq->u32SentinelSerial );


                break;

            case DEBUG_STRUCT_CSLOCKRES:
                csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSALLOCATIONDESCRIPTOR, 
                    ( PFxVOID )&psCSLockRes->sAllocationDescriptor ); 
                break;
            case DEBUG_STRUCT_CSLOCKREQ:
                csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSALLOCATIONDESCRIPTOR, 
                    ( PFxVOID )&psCSLockReq->sAllocationDescriptor ); 
                break;
            case DEBUG_STRUCT_CSUNLOCKRES:
                csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSALLOCATIONDESCRIPTOR, 
                    ( PFxVOID )&psCSUnLockRes->sAllocationDescriptor ); 
                break;
            case DEBUG_STRUCT_CSUNLOCKREQ:
                csDEBUGDisplayStruct( u32Level, DEBUG_STRUCT_CSALLOCATIONDESCRIPTOR, 
                    ( PFxVOID )&psCSUnLockReq->sAllocationDescriptor ); 
                break;
    }

    csDEBUGCreateStringWithIndent( ( PFxSz )&szTemp1, "}\n", u32IndentDepth - 0x03 );
    csDEBUGDisplayFormatted( u32Level, ( PFxSz )&szTemp1 );  

    /* Decrement the indent depth to allow the next structure to be displayed a level back */
    u32IndentDepth -= 0x03;
}