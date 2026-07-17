/*
** Copyright (c) 1995, 3Dfx Interactive, Inc.
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
** $Revision: 4$
** $Date: 10/11/00 7:35:39 PM$
*/
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <dos.h>
#include <stdlib.h>

#include <3dfx.h>

#if defined ( __WATCOMC__ ) && defined ( __DOS32__ )
#  include <i86.h>
#  include <fxdpmi.h>
#  include "atb_ipx.h"
#endif

#include <atb_net.h>

typedef struct
{
   FxBool      (*open)( const void *open_data );
   FxBool      (*close)(void);
   FxBool      (*send_packet)( const void *buffer, int buflen, FxU32 flags );
   /* Denis. Changed the prototype of the get_packet function.
   		Returns the number of bytes read
   */
   int		(*get_packet)( const void *buffer );
   FxBool      (*packet_waiting)(void);
   const char *(*error_string)(void);
} AtbNetDriver;

static FxBool       __net_initialized;
static AtbNetDriver __net_driver;

/*
** NetOpen
*/
FxBool NetOpen( void )
{
#ifdef __DOS32__
   const FxU32 open_data = DEFAULT_3DFX_IPX_SOCKET;

   __net_driver.open           = IpxOpen;
   __net_driver.close          = IpxClose;
   __net_driver.send_packet    = IpxSendPacket;
   __net_driver.packet_waiting = IpxPacketWaiting;
   __net_driver.get_packet     = IpxGetPacket;
#else
#  error Not defined
#endif

   __net_initialized = FXTRUE;

   return __net_driver.open( ( const void * ) &open_data );
}

/*
** NetClose
*/
FxBool NetClose( void )
{
   if ( !__net_initialized )
      return FXFALSE;
   return __net_driver.close();
}

/*
** NetErrorString
*/
const char *NetErrorString( void )
{
   if ( !__net_initialized )
      return "AtbNet not initialized!";
   return __net_driver.error_string();
}

/*
** NetGetPacket
** Denis. Changed the return value from Bool to int.
** 	  Returns the number of bytes read.
**	  Returns -1 if there was an error.
*/
int NetGetPacket( const void *buffer )
{
   if ( !__net_initialized )
     return -1;
   return __net_driver.get_packet( buffer );
}

/*
** NetPacketWaiting
*/
FxBool NetPacketWaiting( void )
{
   if ( !__net_initialized )
     return FXFALSE;
   return __net_driver.packet_waiting();
}

/*
** NetSendPacket
*/
FxBool NetSendPacket( const void *buffer, int buflen, FxU32 flags )
{
   if ( !__net_initialized )
     return FXFALSE;
   return __net_driver.send_packet( buffer, buflen, flags );
}


/* int NetGetHostname( char *name, int length );
** length is the length of the name buffer that will contain
** the name of the local host computer.
** returns the actual length of the name if success,
** returns -1 otherwise 
** Note: for IPX, I don't think there is a local host name
** so this call returns indeed the IPX address and node.
** This call is essentially meant to be used as a way to
** uniquely identify a machine on a local network.
**
** name is the buffer that will contain the address as a string
** length is the length of the name buffer. Must be > 6.
** returns the length of the local address (should always be 6)
*/
int NetGetHostname( char *name, int length )
{
  /* Make sure that the length of the name buffer is long enough
  ** 6 is the size of the node field in the IpxLocalAddress structure.
  */
  if ( length < 6 ) {
      printf( "ATB_NET error: NetGetHostname must be called with a name and length > 6 bytes\n" );
      return -1;
  }
  return IpxGetLocalAddress( name, length);
}

