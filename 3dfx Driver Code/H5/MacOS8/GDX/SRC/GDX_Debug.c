/*
** Copyright (c) 1996-1999, 3Dfx Interactive, Inc.
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
** File name:   gdx_debug.c
**
** Description: HAL layer for MacOS Display Driver.
**
** $Header: GDX_Debug.c, 7, 10/11/00 8:33:52 PM, Brent$
**
** $History: gdx_debug.c $
** 
** *****************  Version 2  *****************
** User: Kcd          Date: 7/30/99    Time: 1:02p
** Updated in $/devel/h3/MacOS8/GDX/src
** Code formatting cleanup.
**
** *****************  Version 1  *****************
** User: Kcd          Date: 6/03/99    Time: 6:45p
** Created in $/devel/h3/MacOS8/GDX/src
** MacOS 8 2D Display Driver
**
**
*/


#include "gdx_debug.h"
#include <gdebug.h>
#include <stdio.h>
#include <string.h>
//#define DEBUG 1
#include <DCON.h>
//#undef DEBUG
#include <gestalt.h>
//#include <MacErrors.h>
#include <LowMem.h>

#define GDX_BUFFER_SIZE 64 * 1024

static char dbg_buff[ GDX_BUFFER_SIZE ]; 
static char * dbg_buff_pos = dbg_buff; 
static long dbg_log_file_is_open = false; 
static long dbg_buffer_is_full = false; 



//#if DEBUG



/*----------------------------------------------------------------------
Function name:  gdbg_printf

Description:    

Return:         -
----------------------------------------------------------------------*/
FX_ENTRY void
FX_CALL gdbg_printf (
	const char *	format,
					...)
{
    va_list args;

    va_start(args, format);
    gdx_debug_vprintf( format, args );
    va_end(args);
}




/*----------------------------------------------------------------------
Function name:  gdbg_info

Description:    

Return:         -
----------------------------------------------------------------------*/
FX_ENTRY int
FX_CALL gdbg_info (
	const int		level,
	const char *	format,
					...)
{
	va_list args;

	if ( VERBOSE_DEBUG >= level ) {

	    va_start(args, format);
	    gdx_debug_vprintf( format, args );
	    va_end(args);

    }

    return 0;
}




/*----------------------------------------------------------------------
Function name:  gdbg_info_more

Description:    

Return:         -
----------------------------------------------------------------------*/
FX_ENTRY int
FX_CALL gdbg_info_more (
	const int		level,
	const char *	format,
					...)
{
    va_list args;


	if ( VERBOSE_DEBUG >= level ) {

	    va_start(args, format);
	    gdx_debug_vprintf( format, args );
	    va_end(args);

    }

    return 0;
}




/*----------------------------------------------------------------------
Function name:  gdx_debug_init

Description:    

Return:         -
----------------------------------------------------------------------*/
void gdx_debug_init( void )
{
  gdx_debug_printf( "________________________________________________________________________\n" );
  gdx_debug_printf( "NDRV for SST1 hardware\n\n" );
}




/*----------------------------------------------------------------------
Function name:  gdx_debug_refresh

Description:    

Return:         -
----------------------------------------------------------------------*/
void gdx_debug_refresh( void )
{
#if !DEBUG_USE_SERIAL
  if ( !dbg_log_file_is_open )
  {
    long response;
    
    if ( !Gestalt('DCon', &response))
	{
      char * thePos;
    
      dopen( "SST1_ndrv.dbg" );
      dprintf( "________________________________________________________________\n" );
      dprintf( "SST1_ndrv debugging data\n\n" );
      dbg_log_file_is_open = true;
    
      thePos = dbg_buff;
      while ( thePos < dbg_buff_pos ) {
        dprintf( thePos );
        thePos += strlen( thePos ) + 1;
      }
    }
  }
#endif

}




/*----------------------------------------------------------------------
Function name:  gdx_debug_printf

Description:    

Return:         -
----------------------------------------------------------------------*/
void gdx_debug_printf( const char * inFormat, ...)
{
  va_list					theArgs;

  va_start( theArgs, inFormat );
  gdx_debug_vprintf( inFormat, theArgs );
  va_end( theArgs );
}





/*----------------------------------------------------------------------
Function name:  gdx_debug_vprintf

Description:    

Return:         -
----------------------------------------------------------------------*/
void gdx_debug_vprintf( const char * inFormat, va_list inArgs)
{
#if DEBUG_USE_SERIAL
  gdx_scc_vprintf( inFormat, inArgs);
#else
  if ( dbg_log_file_is_open )
    vdprintf( inFormat, inArgs );
  else
  {
    if ( !dbg_buffer_is_full )
    {
      int theStrSize = vsprintf( dbg_buff_pos, inFormat, inArgs);
      dbg_buff_pos += theStrSize + 1;
    
      if ( dbg_buff_pos > ( dbg_buff + GDX_BUFFER_SIZE - 255 ) )
      {
        dbg_buffer_is_full = true;
        
        theStrSize = sprintf( dbg_buff_pos, "############### BUFFER IS FULL ###############\n" );
        dbg_buff_pos += theStrSize + 1;
      }
    }
  }
#endif
}


//#endif


/*
#include <DCon.h>

extern OSErr drvrInitFunc( void );

OSErr drvrInitFunc( void )
{

	dopen("voodoo3.log");

	dprintf("cfm init!\n");

	return 0;
}
*/


/*----------------------------------------------------------------------
Function name:  gdx_debug_printf

Description:    

Return:         -
----------------------------------------------------------------------*/
void gdx_ddc_printf( const char * inFormat, ...)
{
  va_list					theArgs;

  va_start( theArgs, inFormat );
  gdx_ddc_vprintf( inFormat, theArgs );
  va_end( theArgs );
}




static char ddcBuff[2048];

extern void AvengerPrintfDDC(char *string);

/*----------------------------------------------------------------------
Function name:  gdx_debug_vprintf

Description:    

Return:         -
----------------------------------------------------------------------*/
void gdx_ddc_vprintf( const char * inFormat, va_list inArgs)
{
  vsprintf(ddcBuff, inFormat, inArgs);
  AvengerPrintfDDC(ddcBuff);
}


/*----------------------------------------------------------------------
Function name:  gdx_debug_printf

Description:    

Return:         -
----------------------------------------------------------------------*/
void gdx_scc_printf( const char * inFormat, ...)
{
  va_list					theArgs;

  va_start( theArgs, inFormat );
  gdx_scc_vprintf( inFormat, theArgs );
  va_end( theArgs );
}

// SCC

OSStatus	DebugSCCSendData( const void *inDataPtr, ByteCount inDataSize );
OSStatus	DebugSCCRecvData( UInt8 *outData );


static char sccBuff[2048];

/*----------------------------------------------------------------------
Function name:  gdx_debug_vprintf

Description:    

Return:         -
----------------------------------------------------------------------*/
void gdx_scc_vprintf( const char * inFormat, va_list inArgs)
{
  vsprintf(sccBuff, inFormat, inArgs);
  DebugSCCSendData(sccBuff,strlen(sccBuff));
}

// RequireGoto

#define	RequireGoto( X, LABEL )	\
	do {						\
		if( X ) { }				\
		else {					\
			goto LABEL;			\
		}						\
	} while( false )

// RequireMessageGoto

#define	RequireMessageGoto( X, MESSAGE, LABEL )								\
	do {																	\
		if( X ) { }															\
		else {																\
			DebugPrintAssertMacro( #X, ( MESSAGE ), __FILE__, __LINE__ );	\
			goto LABEL;														\
		}																	\
	} while( false )

// RequireAction

#define	RequireAction( X, ACTION )	\
	{								\
		if( X ) { }					\
		else {						\
			{ ACTION; }				\
		}							\
	}

// RequireMessageAction

#define	RequireMessageAction( X, MESSAGE, ACTION )							\
	{																		\
		if( X ) { }															\
		else {																\
			DebugPrintAssertMacro( #X, ( MESSAGE ), __FILE__, __LINE__ );	\
			{ ACTION; }														\
		}																	\
	}

// RequireActionGoto

#define	RequireActionGoto( X, ACTION, LABEL )	\
	{											\
		if( X ) { }								\
		else {									\
			{ ACTION; }							\
			goto LABEL;							\
		}										\
	}

// RequireMessageActionGoto

#define	RequireMessageActionGoto( X, MESSAGE, ACTION, LABEL )				\
	{																		\
		if( X ) { }															\
		else {																\
			DebugPrintAssertMacro( #X, ( MESSAGE ), __FILE__, __LINE__ );	\
			{ ACTION; }														\
			goto LABEL;														\
		}																	\
	}
	
// RequireEventGoto

#define	RequireEventGoto( X, A, B, C, D, LABEL )			\
	do {													\
		if( X ) { }											\
		else {												\
			DebugEvent( kRequireDebugLevel, A, B, C, D );	\
			goto LABEL;										\
		}													\
	} while( false )

// RequireEventMessageGoto

#define	RequireEventMessageGoto( X, A, B, C, D, MESSAGE, LABEL )						\
	do {																				\
		if( X ) { }																		\
		else {																			\
			DebugEvent( kRequireDebugLevel, A, B, C, D );								\
			DebugPrintAssertMacro( kNilOptions, #X, ( MESSAGE ), __FILE__, __LINE__ );	\
			goto LABEL;																	\
		}																				\
	} while( false )

// RequireEventAction

#define	RequireEventAction( X, A, B, C, D, ACTION )			\
	{														\
		if( X ) { }											\
		else {												\
			DebugEvent( kRequireDebugLevel, A, B, C, D );	\
			{ ACTION; }										\
		}													\
	}

// RequireEventMessageAction

#define	RequireEventMessageAction( X, A, B, C, D, MESSAGE, ACTION )			\
	{																		\
		if( X ) { }															\
		else {																\
			DebugEvent( kRequireDebugLevel, A, B, C, D );					\
			DebugPrintAssertMacro( #X, ( MESSAGE ), __FILE__, __LINE__ );	\
			{ ACTION; }														\
		}																	\
	}

// RequireEventActionGoto

#define	RequireEventActionGoto( X, A, B, C, D, ACTION, LABEL )		\
	{																\
		if( X ) { }													\
		else {														\
			DebugEvent( kRequireDebugLevel, A, B, C, D );			\
			{ ACTION; }												\
			goto LABEL;												\
		}															\
	}	

// RequireEventMessageActionGoto

#define	RequireEventMessageActionGoto( X, A, B, C, D, MESSAGE, ACTION, LABEL )		\
	{																				\
		if( X ) { }																	\
		else {																		\
			DebugEvent( kRequireDebugLevel, A, B, C, D );							\
			DebugPrintAssertMacro( #X, ( MESSAGE ), __FILE__, __LINE__ );			\
			{ ACTION; }																\
			goto LABEL;																\
		}																			\
	}

/*==================================================================================================
	Macro aliases
==================================================================================================*/

#define	AssertNoErrEvent( X, A, B, C, D )	\
	AssertEvent( ( X ) == noErr, A, B, C, D )

#define	RequireNoErrEventAction( X, A, B, C, D, ACTION )	\
	RequireEventAction( ( X ) == noErr, A, B, C, D, ACTION )
	
#define	RequireNoErrEventGoto( X, A, B, C, D, LABEL )	\
	RequireEventGoto( ( X ) == noErr, A, B, C, D, LABEL )

#define	RequireNoErrEventActionGoto( X, A, B, C, D, ACTION, LABEL )	\
	RequireEventActionGoto( ( X ) == noErr, A, B, C, D, ACTION, LABEL )



// SCC

static void		DebugSCCConfigure( void );
static OSStatus	DebugSCCWaitTransmitterEmpty( void );
static OSStatus	DebugSCCWaitAllSent( void );
static UInt8	DebugSCCReadControlRegister( UInt8 inRegisterIndex );
static void		DebugSCCWriteControlRegister( UInt8 inRegisterIndex, UInt8 inData );

// SCC

#define	kWallstreetSCCBaseAddress		( ( volatile UInt8 * ) 0xF3012000 )
#define	kNewWorldSCCBaseAddress			( ( volatile UInt8 * ) 0x80812000 )
#define	kCore99SCCBaseAddress			( ( volatile UInt8 * ) 0x80012000 )

// SCC

#if( !defined( DEBUG_OUTPUT_SCC_BASE_ADDRESS ) )
	#define	DEBUG_OUTPUT_SCC_BASE_ADDRESS		kNewWorldSCCBaseAddress
#endif

#define SynchronizeIO __sync

enum
{
	kSCCCmdOffsetA	   		= 2, 
	kSCCCmdOffsetModem   	= kSCCCmdOffsetA, 
	kSCCCmdOffsetB	   		= 0, 
	kSCCCmdOffsetPrinter	= kSCCCmdOffsetB, 
	kSCCDataOffset	   		= 4
};


// General

typedef OSType							DebugOutputKind;
typedef UInt32							DebugLevel;

// General

typedef	struct	DebugGlobals			DebugGlobals;
struct	DebugGlobals
{
	UInt8		buffer[ 512 ];
};

// SCC

typedef	struct	DebugSCCGlobals			DebugSCCGlobals;
struct	DebugSCCGlobals
{
	volatile UInt8 *		baseAddress;
	volatile UInt8 *		controlRegister;
	volatile UInt8 *		dataRegister;
	Boolean					initialized;
};

static DebugGlobals					gDebug;

// SCC

static DebugSCCGlobals				gDebugSCC = 
{
	DEBUG_OUTPUT_SCC_BASE_ADDRESS,											// baseAddress
	DEBUG_OUTPUT_SCC_BASE_ADDRESS + kSCCCmdOffsetA,							// controlRegister
	DEBUG_OUTPUT_SCC_BASE_ADDRESS + kSCCCmdOffsetA + kSCCDataOffset,		// dataRegister
	false																	// initialized
};


#if( !TARGET_CPU_68K )

/*==================================================================================================
	DebugDisableInterrupts
==================================================================================================*/

static UInt16	DebugDisableInterrupts( void )
{	
	static const UInt16		kDebugDisableInterrupts68KCode[] = 
	{
		0x40C0, 			// move		sr,d0			; Save off SR for output.
		0x007C, 0x0700, 	// ori.w	#$0700,sr		; Set interrupt level to 7.
		0x4E75				// rts						; Return to caller.
	};

	return( CallUniversalProc( ( UniversalProcPtr ) kDebugDisableInterrupts68KCode,
							   kRegisterBased | 
							   RESULT_SIZE( kTwoByteCode ) | 
							   REGISTER_RESULT_LOCATION( kRegisterD0 ) ) );
}

/*==================================================================================================
	DebugRestoreInterrupts
==================================================================================================*/

static void	DebugRestoreInterrupts( UInt16 inOldState )
{
	static const UInt16		kDebugRestoreInterrupts68KCode[] = 
	{
		0x40C0, 			// move		sr,d0			; Get current SR.
		0x0240, 0xF8FF, 	// andi.w	#$F8FF,d0		; Mask out current interrupt level.
		0x0241, 0x0700, 	// andi.w	#$0700,d1		; Mask all but interrupt level on input.
		0x8041, 			// or.w		d1,d0			; Or new interrupt level and old SR data.
		0x46C0, 			// move		d0,sr			; Set interrupt level to input level.
		0x4E75				// rts						; Reture to caller.
	};

	CallUniversalProc( ( UniversalProcPtr ) kDebugRestoreInterrupts68KCode, 
					   kRegisterBased | 
					   REGISTER_ROUTINE_PARAMETER( 1, kRegisterD1, kTwoByteCode ), 
					   inOldState );
}

#endif


/*==================================================================================================
	DebugSCCSendData
==================================================================================================*/

OSStatus	DebugSCCSendData( const void *inDataPtr, ByteCount inDataSize )
{
	OSStatus			err;
	const UInt8 *		dataPtr;
	UInt32				dataIndex;
	
	if( !inDataPtr || !inDataSize )
	{
		return( paramErr );
	}
	
	DebugSCCConfigure();
	
	dataPtr = ( const UInt8 * ) inDataPtr;
	for( dataIndex = 0; dataIndex < inDataSize; ++dataIndex )
	{
		UInt8		data;
		
		err = DebugSCCWaitTransmitterEmpty();
		RequireAction( err == noErr, return( err ) );
		
		data = dataPtr[ dataIndex ];
		if( ( data == '\r' ) || ( data == '\n' ) )
		{
			// Special case for CR or LR and send out both.
			
			*( gDebugSCC.dataRegister ) = '\r';
			err = DebugSCCWaitTransmitterEmpty();
			RequireAction( err == noErr, return( err ) );
			*( gDebugSCC.dataRegister ) = '\n';
		}
		else
		{
			*( gDebugSCC.dataRegister ) = dataPtr[ dataIndex ];
		}
		SynchronizeIO();
	}
	err = DebugSCCWaitAllSent();
	return( err );
}

/*==================================================================================================
	DebugSCCRecvData
==================================================================================================*/

OSStatus	DebugSCCRecvData( UInt8 *outData )
{
	//DebugTimeoutState		timeoutState;
	
	SynchronizeIO();
	DebugSCCConfigure();
	
	// Wait for data.
	
	//DebugStartTimeout( &timeoutState );
	while( true )
	{
		OSStatus		err = noErr;
		UInt8			temp;
		
		//err = DebugCheckTimeout( &timeoutState );
		if( err != noErr ) return( err );
		
		temp = *gDebugSCC.controlRegister;
		SynchronizeIO();
			
		if( temp & 0x01 )
		{
			// Get character.
			
			SynchronizeIO();
			temp = *gDebugSCC.dataRegister;
			SynchronizeIO();
			*outData = temp;
			
			// Echo character.
			
			err = DebugSCCSendData( &temp, 1 );
			if( err != noErr ) return( err );	
			break;
		}
	}
	return( noErr );
}

/*==================================================================================================
	DebugSCCConfigure
==================================================================================================*/

// Shut down so we can restore it again on power up.
void gdx_scc_shutdown(void)
{
  gDebugSCC.initialized = 0;
}

static void	DebugSCCConfigure( void )
{
	UInt16		oldInterruptState;
	
	if( !gDebugSCC.initialized )
	{
	    //dprintf("initializing SCC\n");
		if( LMGetSCCWr() )
		{
            //dprintf("got SCC registers\n");
			gDebugSCC.baseAddress 		= ( volatile UInt8 * ) LMGetSCCWr();
			gDebugSCC.controlRegister 	= gDebugSCC.baseAddress + kSCCCmdOffsetA;
			gDebugSCC.dataRegister	 	= gDebugSCC.baseAddress + kSCCCmdOffsetA + kSCCDataOffset;
			gDebugSCC.initialized 		= true;
		}
	} else {
	    //dprintf("SCC already initialized: %08lx\n",gDebugSCC.baseAddress);
	}
	oldInterruptState = DebugDisableInterrupts();
	
	DebugSCCWriteControlRegister( 9, 	0x80 );	// Reset port A (modem)
	DebugSCCWriteControlRegister( 4, 	0x44 );	// 16x clock, 1 stop bit, no parity.
	DebugSCCWriteControlRegister( 3, 	0xC0 );	// 8-bit receive, disable.
	DebugSCCWriteControlRegister( 5, 	0xE2 );	// 8-bit transmit, DTS/RTS asserted, disable.
	DebugSCCWriteControlRegister( 2, 	0x00 );	// Clear interrupt vector.
	DebugSCCWriteControlRegister( 10, 	0x00 );	// NRZ encoding.
	DebugSCCWriteControlRegister( 11, 	0x50 );	// Baud rate generator clock.
	DebugSCCWriteControlRegister( 12, 	0x00 );	// Baud rate.lo = 57.6KB/sec.
	//DebugSCCWriteControlRegister( 12, 	0x01 );	// Baud rate.lo = 38.4KB/sec.
	//DebugSCCWriteControlRegister( 12, 	0x04 );	// Baud rate.lo = 19.2KB/sec.
	DebugSCCWriteControlRegister( 13,	0x00 );	// Baud rate.hi = 0.
	DebugSCCWriteControlRegister( 3, 	0xC1 );	// 8-bit receive and enable.
	DebugSCCWriteControlRegister( 5, 	0xEA );	// 8-bit transmit, DTS/RTS asserted, enable.
	DebugSCCWriteControlRegister( 14, 	0x01 );	// Enable baud rate generator.
	DebugSCCWriteControlRegister( 15, 	0x00 );	// Disable break/abort interrupts.
	DebugSCCWriteControlRegister( 0, 	0x10 );	// Reset ext/status interrupts.
	DebugSCCWriteControlRegister( 0, 	0x10 );	// Reset ext/status interrupts (again).
	DebugSCCWriteControlRegister( 1, 	0x00 );	// No external interrupts on this channel.
	DebugSCCWriteControlRegister( 9, 	0x0A );	// SCC interrupts enabled (MIE)
	
	DebugRestoreInterrupts( oldInterruptState );
}

/*==================================================================================================
	DebugSCCReadControlRegister
	
	Note: This should be called with interrupts disabled.
==================================================================================================*/

static UInt8	DebugSCCReadControlRegister( UInt8 inRegisterIndex )
{
	UInt8		data;
	
	*gDebugSCC.controlRegister = inRegisterIndex;
	SynchronizeIO();
	
	data = *gDebugSCC.controlRegister;
	SynchronizeIO();
	
	return( data );
}

/*==================================================================================================
	DebugSCCWriteControlRegister
	
	Note: This should be called with interrupts disabled.
==================================================================================================*/

static void	DebugSCCWriteControlRegister( UInt8 inRegisterIndex, UInt8 inData )
{
	*gDebugSCC.controlRegister = inRegisterIndex;
	SynchronizeIO();
	
	*gDebugSCC.controlRegister = inData;
	SynchronizeIO();
}

/*==================================================================================================
	DebugSCCWaitTransmitterEmpty
==================================================================================================*/

static OSStatus	DebugSCCWaitTransmitterEmpty( void )
{
	//DebugTimeoutState		timeoutState;
	Boolean					triedToConfigure;
	UInt8					sccRR0;
	
	triedToConfigure = false;
	//DebugStartTimeout( &timeoutState );
	do
	{
		OSStatus		err = noErr;
		
		//err = DebugCheckTimeout( &timeoutState );
		if( err != noErr )
		{
			if( triedToConfigure )
			{
				return( err );
			}
			else
			{
				triedToConfigure = true;
				DebugSCCConfigure();
			}
		}
		sccRR0 = *gDebugSCC.controlRegister;
		SynchronizeIO();
		
	}	while( !( sccRR0 & 0x04 ) );
	return( noErr );
}

/*==================================================================================================
	DebugSCCWaitAllSent
==================================================================================================*/

static OSStatus	DebugSCCWaitAllSent( void )
{
	//DebugTimeoutState		timeoutState;
	UInt16					oldInterruptState;
	UInt8					temp;

	//DebugStartTimeout( &timeoutState );
	oldInterruptState = DebugDisableInterrupts();
	do
	{
		OSStatus		err = noErr;
		
		//err = DebugCheckTimeout( &timeoutState );
		if( err != noErr ) return( err );
		
		temp = DebugSCCReadControlRegister( 1 );
		
	}	while( !( temp & 0x1 ) );
	DebugRestoreInterrupts( oldInterruptState );
	return( noErr );
}

