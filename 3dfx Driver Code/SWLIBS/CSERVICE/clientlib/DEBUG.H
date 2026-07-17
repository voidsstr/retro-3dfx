/*
** Copyright (c) 1995, 1996, 1997, 2000 3Dfx Interactive, Inc.
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
** $Revision: 26$
** $Date: 10/24/00 9:33:14 AM$
** $Log: 
**  26   Napalm R1.51.17.1.7    10/24/00 Don Fowler      Fixed a book keeping error
**       in previous checkin.. 
**  25   Napalm R1.51.17.1.6    10/24/00 Don Fowler      Added CSTIME structure to
**       debug system and CSGRAPHICALCONTEXT structure. De-coupled
**       CSGRAPHICALCONTEXT from CS_BUFFER_TEXTURE_HEAP type in all functions that
**       deal with this memory type. 
**  24   Napalm R1.51.17.1.5    10/11/00 Brent           Forced check in to enforce
**       branch.
**  23   Napalm R1.51.17.1.4    10/09/00 Don Fowler      removed // DWF from file
**  22   Napalm R1.51.17.1.3    10/09/00 Don Fowler      Changed debug system to
**       get errors from the server when the error code comes from the server. The
**       error code defines are stored in the respecitve servers header files. Call
**       csDEBUGGetErrorString to retrieve the verbose string from the server
**  21   Napalm R1.51.17.1.2    10/09/00 Don Fowler      Added support for the
**       CSSST1 chip type.
**  20   Napalm R1.51.17.1.1    07/20/00 Andrew  Bell    Rolling back all swlibs
**       files to 2nd latest version due to an incorrect checkin.  This should
**       bring things back to normal.
**  19   Napalm R1.51.17.1.0    07/19/00 Dinesh Raja Savari Amirtharaj CSIM for the
**       RTL release_3_2
**  18   3dfx      1.17        03/29/00 Don Fowler      Removed CS_MEMORY_AGP since
**       we now have CS_LOCALE_AGP
**       Added CSALLOC_APIERROR_INVALIDASPECT
**  17   3dfx      1.16        03/19/00 Don Fowler      Added the
**       CSALLOCATIONDESCRIPTOR.u32Locale so that the client can specify
**       CS_LOCALE_FRAMEBUFFER  or CS_LOCALE_AGP separate from tiled or linear
**       memory types.
** 
**       Changed all of the error codes in the API from _ERROR_ to _APIERROR_ so
**       that server codes can have a different base 
** 
**  16   3dfx      1.15        03/09/00 Don Fowler      Added more error types and
**       stricter error checking to csAlloc
**  15   3dfx      1.14        03/07/00 Don Fowler      Added more structures to
**       the debug display system. Added CSSERVERRES.u32ReqID so the debug system
**       would know the id of the request packet after it returns. And for future
**       use.
**  14   3dfx      1.13        03/05/00 Don Fowler      Removed the notion of a
**       device context from the code. Since a graphical context is locked to a
**       window the same as a device context it was a redundant notion. 
**  13   3dfx      1.12        03/03/00 Don Fowler      Incremental development
**  12   3dfx      1.11        03/02/00 Don Fowler      Incremental Development
**  11   3dfx      1.10        03/02/00 Don Fowler      Incremental development
**  10   3dfx      1.9         03/01/00 Don Fowler      Incremental development
**  9    3dfx      1.8         02/29/00 Don Fowler      Incremental development
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
*/

#ifndef _CSDEBUG_H
#define _CSDEBUG_H

#include <stdarg.h>



/* Debug levels for common components */
#define DEBUG_LEVEL_ERROR       1      
#define DEBUG_LEVEL_FUNCTION    100
#define DEBUG_LEVEL_INFO        300
#define DEBUG_LEVEL_STRUCTURE   500


FxVOID FX_CALL csDEBUGInit( FxVOID );
FxVOID FX_CALL csDEBUGSetFile( PFxSz pszFileName );
FxVOID FX_CALL csDEBUGPrint( PFxSz pszFormatString, va_list args );
FxVOID FX_CALL csDEBUGDisplayFormatted( FxU32 u32Level, PFxSz pszFormatString, ... );
FxVOID FX_CALL csDEBUGDisplayStruct( FxU32 u32Level, FxU32 u32StructID, PFxVOID pvStructure );
PFxSz  FX_CALL csDEBUGCreateStringWithIndent( PFxSz pszString, PFxSz pszText, FxU32 u32Indent );


/* Enumeration for each structure type in the system to display */
enum
{
    DEBUG_STRUCT_CSCLIENTLIBDATA = 0x00,
    DEBUG_STRUCT_CSSERVERREQ,
    DEBUG_STRUCT_CSSERVERRES,
    DEBUG_STRUCT_CSGRAPHICALCONTEXT,
    DEBUG_STRUCT_CSALLOCATIONDESCRIPTOR,
    DEBUG_STRUCT_CSSWAPBUFFERTODISPLAY,
    DEBUG_STRUCT_CSRECT,
    DEBUG_STRUCT_CSCLIPLIST,
    DEBUG_STRUCT_CSINIT,
    DEBUG_STRUCT_CSCHIPSPECIFICDATA,
    DEBUG_STRUCT_CSSST1,
    DEBUG_STRUCT_CSSST2,
    DEBUG_STRUCT_CSVIDEOMODE,
    DEBUG_STRUCT_CSOVERLAY,
    DEBUG_STRUCT_CSDEVICECONFIG,
#ifdef WIN32 
    DEBUG_STRUCT_OSVERSIONINFO,
#endif /* WIN32 */
    /* These values need to stay at the end of the and contiguous in order */
    DEBUG_STRUCT_CSGETPROTOCOLREVISIONREQ,
    DEBUG_STRUCT_CSGETPROTOCOLREVISIONRES,
    DEBUG_STRUCT_CSGETGRAPHICALCONTEXTREQ,
    DEBUG_STRUCT_CSGETGRAPHICALCONTEXTRES,
    DEBUG_STRUCT_CSRELEASEGRAPHICALCONTEXTREQ,
    DEBUG_STRUCT_CSRELEASEGRAPHICALCONTEXTRES,
    DEBUG_STRUCT_CSALLOCREQ,
    DEBUG_STRUCT_CSALLOCRES,
    DEBUG_STRUCT_CSFREEREQ,
    DEBUG_STRUCT_CSFREERES,
    DEBUG_STRUCT_CSSETVIDEOMODEREQ,
    DEBUG_STRUCT_CSSETVIDEOMODERES,
    DEBUG_STRUCT_CSSWAPBUFFERTODISPLAYREQ,
    DEBUG_STRUCT_CSSWAPBUFFERTODISPLAYRES,
    DEBUG_STRUCT_CSEXECUTECOMMANDSREQ,
    DEBUG_STRUCT_CSEXECUTECOMMANDSRES,
    DEBUG_STRUCT_CSLOCKREQ,
    DEBUG_STRUCT_CSLOCKRES,
    DEBUG_STRUCT_CSUNLOCKREQ,
    DEBUG_STRUCT_CSUNLOCKRES,
    DEBUG_STRUCT_CSREGISTERREADREQ,
    DEBUG_STRUCT_CSREGISTERREADRES,
    DEBUG_STRUCT_CSREGISTERWRITEREQ,
    DEBUG_STRUCT_CSREGISTERWRITERES,
    DEBUG_STRUCT_CSGETSERVERERRORSTRINGREQ,
    DEBUG_STRUCT_CSGETSERVERERRORSTRINGRES,
    DEBUG_STRUCT_CSTIME
};


/* Macros to return from functions and display debug information */
#define CSDEBUG_RETURNVOID(level)\
{\
    csDEBUGDisplayFormatted(level,"Exit - %s\n",pszFunctionName);\
    return;\
}

#define CSDEBUG_RETURN(level,value)\
{\
    csDEBUGDisplayFormatted(level,"Exit(%s) - %s\n", csDEBUGGetStringForError( value ), pszFunctionName);\
    return(value);\
}

#define CSDEBUG_RETURN_NOERROR(level,value)\
{\
    csDEBUGDisplayFormatted(level,"Exit(0x%x) - %s\n", value, pszFunctionName);\
    return(value);\
}

/* Macro to validate the initialization of the system and return if it has not been done */
#define CSDEBUG_VALIDATE_INITIALIZATION \
{\
    if( !( _sCSClientLibData.u32Flags & CSCLIENTLIBDATA_FLAGS_INITIALIZED ) )\
    {\
        csDEBUGDisplayFormatted( DEBUG_LEVEL_ERROR, "%s::ERROR::CS_APIERROR_SYSTEMNOTINITIALIZED\n", pszFunctionName );\
        CSDEBUG_RETURN( DEBUG_LEVEL_FUNCTION, CS_APIERROR_SYSTEMNOTINITIALIZED );\
    }\
}

/* Macro to validate that parameters passed to a function */
#define CSDEBUG_VALIDATE_PARAMETERS(boolean)\
{\
    if( boolean )\
    {\
        csDEBUGDisplayFormatted( DEBUG_LEVEL_ERROR, "%s::ERROR::CS_APIERROR_INVALIDPARAM\n",pszFunctionName );\
        CSDEBUG_RETURN( DEBUG_LEVEL_FUNCTION, CS_APIERROR_INVALIDPARAM );\
    }\
}

/* Macro to validate the state of the graphical context structure*/
#define CSDEBUG_VALIDATE_GRAPHICALCONTEXT_PTR(context)\
{\
    csDEBUGDisplayStruct( DEBUG_LEVEL_STRUCTURE, DEBUG_STRUCT_CSGRAPHICALCONTEXT, context );\
    if( !( context->u32Flags & CSGRAPHICALCONTEXT_FLAGS_INITIALIZED ) )\
    {\
        csDEBUGDisplayFormatted( DEBUG_LEVEL_ERROR, "%s::ERROR::CS_APIERROR_GRAPHICALCONTEXTNOTINITIALIZED\n", pszFunctionName );\
        CSDEBUG_RETURN( DEBUG_LEVEL_FUNCTION, CS_APIERROR_GRAPHICALCONTEXTNOTINITIALIZED );\
    }\
}
#define CSDEBUG_VALIDATE_GRAPHICALCONTEXT(context)\
{\
    csDEBUGDisplayStruct( DEBUG_LEVEL_STRUCTURE, DEBUG_STRUCT_CSGRAPHICALCONTEXT, &context );\
    if( !( context.u32Flags & CSGRAPHICALCONTEXT_FLAGS_INITIALIZED ) )\
    {\
        csDEBUGDisplayFormatted( DEBUG_LEVEL_ERROR, "%s::ERROR::CS_APIERROR_GRAPHICALCONTEXTNOTINITIALIZED\n", pszFunctionName );\
        CSDEBUG_RETURN( DEBUG_LEVEL_FUNCTION, CS_APIERROR_GRAPHICALCONTEXTNOTINITIALIZED );\
    }\
}

/* Macro to validate the state of the allocation descriptor */
#define CSDEBUG_VALIDATE_ALLOCATIONDESCRIPTOR_PTR(context)\
{\
    csDEBUGDisplayStruct( DEBUG_LEVEL_STRUCTURE, DEBUG_STRUCT_CSALLOCATIONDESCRIPTOR, context );\
    if( !( context->u32Flags & CSALLOCATIONDESCRIPTOR_FLAGS_INITIALIZED ) )\
    {\
        csDEBUGDisplayFormatted( DEBUG_LEVEL_ERROR, "%s::ERROR::CS_APIERROR_ALLOCATIONDESCRIPTORNOTINITIALIZED\n", pszFunctionName );\
        CSDEBUG_RETURN( DEBUG_LEVEL_FUNCTION, CS_APIERROR_ALLOCATIONDESCRIPTORNOTINITIALIZED );\
    }\
    if( ( context->u32MemType < CS_MEMORY_LINEAR ) ||\
        ( context->u32MemType > CS_MEMORY_TILED ) )\
    {\
        csDEBUGDisplayFormatted( DEBUG_LEVEL_ERROR, "%s::ERROR::CSALLOC_APIERROR_INVALIDMEMORYTYPE::Invalid Memory Type::0x%x\n", pszFunctionName, context->u32MemType );\
        CSDEBUG_RETURN( DEBUG_LEVEL_FUNCTION, CSALLOC_APIERROR_INVALIDMEMORYTYPE );\
    }\
    if( ( context->u32BufferType < CS_BUFFER_FIFO ) ||\
        ( context->u32BufferType > CS_BUFFER_ZBUFFER ) )\
    {\
        csDEBUGDisplayFormatted( DEBUG_LEVEL_ERROR, "%s::ERROR::CSALLOC_APIERROR_INVALIDBUFFERTYPE::Invalid Buffer Type::0x%x\n", pszFunctionName, context->u32BufferType );\
        CSDEBUG_RETURN( DEBUG_LEVEL_FUNCTION, CSALLOC_APIERROR_INVALIDBUFFERTYPE );\
    }\
}
#define CSDEBUG_VALIDATE_ALLOCATIONDESCRIPTOR(context)\
{\
    csDEBUGDisplayStruct( DEBUG_LEVEL_STRUCTURE, DEBUG_STRUCT_CSALLOCATIONDESCRIPTOR, &context );\
    if( !( context.u32Flags & CSALLOCATIONDESCRIPTOR_FLAGS_INITIALIZED ) )\
    {\
        csDEBUGDisplayFormatted( DEBUG_LEVEL_ERROR, "%s::ERROR::CS_APIERROR_ALLOCATIONDESCRIPTORNOTINITIALIZED\n", pszFunctionName );\
        CSDEBUG_RETURN( DEBUG_LEVEL_FUNCTION, CS_APIERROR_ALLOCATIONDESCRIPTORNOTINITIALIZED );\
    }\
    if( ( context.u32MemType < CS_MEMORY_LINEAR ) ||\
        ( context.u32MemType > CS_MEMORY_TILED ) )\
    {\
        csDEBUGDisplayFormatted( DEBUG_LEVEL_ERROR, "%s::ERROR::CSALLOC_APIERROR_INVALIDMEMORYTYPE::Invalid Memory Type::0x%x\n", pszFunctionName, context.u32MemType );\
        CSDEBUG_RETURN( DEBUG_LEVEL_FUNCTION, CSALLOC_APIERROR_INVALIDMEMORYTYPE );\
    }\
    if( ( context.u32BufferType < CS_BUFFER_FIFO ) ||\
        ( context.u32BufferType > CS_BUFFER_ZBUFFER ) )\
    {\
        csDEBUGDisplayFormatted( DEBUG_LEVEL_ERROR, "%s::ERROR::CSALLOC_APIERROR_INVALIDBUFFERTYPE::Invalid Buffer Type::0x%x\n", pszFunctionName, context.u32BufferType );\
        CSDEBUG_RETURN( DEBUG_LEVEL_FUNCTION, CSALLOC_APIERROR_INVALIDBUFFERTYPE );\
    }\
}

#endif /* !_CSDEBUG_H */
