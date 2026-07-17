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
** $Date: 10/11/00 7:35:37 PM$
*/
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <dos.h>
#include <stdlib.h>

#include <3dfx.h>

#ifdef __DOS32__
#  include <i86.h>
#  include <fxdpmi.h>
#endif

#include "atb_ipx.h"

#define MULTIPLEX_INTERRUPT 0x2F
#define IPX_INTERRUPT       0x7A

/*
** static variables used by the IPX interface
*/
static struct IpxState
{
   IpxError_t      last_error;
   IpxSocket_t     socketid;
   IpxLocalAddress local_address;
   FxI32           sequence;
   int             current_send_packet;
} ipx_state;

/*
** static packet structures
*/
#ifdef REALMODE
static volatile IpxGenericPacket ipx_send_packet[IPX_NUM_SEND_PACKETS];
static volatile IpxGenericPacket ipx_receive_packet[IPX_NUM_RECEIVE_PACKETS];
static void far                 (*ipx_direct)();
#else
static volatile IpxGenericPacket *ipx_send_packet_rm[IPX_NUM_SEND_PACKETS];
static volatile IpxGenericPacket *ipx_receive_packet_rm[IPX_NUM_RECEIVE_PACKETS];
static DpmiSelector_t             ipx_rcv_sel[IPX_NUM_RECEIVE_PACKETS],
                                  ipx_send_sel[IPX_NUM_SEND_PACKETS];
static FxU16                      ipx_direct_seg, ipx_direct_off;
#endif

/*
** we access packet structures indirectly for portability between
** real mode and protected mode stuff (that way we don't have to
** have a bunch of #ifdef REALMODE type stuff strewn throughout
** our code).
*/
static volatile IpxGenericPacket *ipx_rp[IPX_NUM_RECEIVE_PACKETS];
static volatile IpxGenericPacket *ipx_sp[IPX_NUM_SEND_PACKETS];

/*
** error strings used by IpxErrorString()
*/
static const char *ipx_error_string[] =
{
   "IPX_ERROR_NONE",
   "IPX_ERROR_GENERIC_FAILURE",
   "IPX_ERROR_IPX_NOT_DETECTED",
   "IPX_ERROR_DPMI_DOS_MEM_FAILED",
   "IPX_ERROR_SOCKET_TABLE_FULL",
   "IPX_ERROR_SOCKET_ALREADY_OPEN",
   "IPX_ERROR_PACKET_TOO_LARGE",
   "IPX_ERROR_TIMEOUT",
   "IPX_ERROR_RESEND_PACKET_TOO_OLD",
   "IPX_ERROR_NONEXISTENT_SOCKET",
   "IPX_ERROR_CANNOT_CANCEL",
   "IPX_ERROR_ECB_NOT_BUSY"
};

/*
** node address that denotes BROADCASTING.  This is used externally
** as a parameter to IpxSendPacket( data, size, NODEADDRESS )
*/
static IpxNodeAddress  _IPX_NODE_BROADCAST = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
const  IpxNodeAddress  *IPX_NODE_BROADCAST = &_IPX_NODE_BROADCAST;

/*
** function used to call the IPX driver
*/
static FxBool IpxPresent( void );
static void   IpxCallDriver( union REGS *regs, struct SREGS *sr );
static void   IpxSetError( IpxError_t e );
static void  _IpxSendPacket( volatile IpxGenericPacket *packet, const IpxNodeAddress *target );

/*
** IpxClose
**
** This function should be executed when IPX services are no longer
** needed.  It closes the socket being used then, if in protected mode,
** frees up any real mode memory used
**
** If the program is going to exit then this function really isn't
** necessary since the socket will be closed automatically (in
** theory) the memory will be freed up.  However, in the event that
** someone is constantly calling IpxOpen()/IpxClose() this performs
** the clean up necessary to prevent resource leakage.
*/
void IpxClose( void )
{
   IpxCloseSocket( ipx_state.socketid );

   #ifndef REALMODE
   {
      int i;

      for ( i = 0; i < IPX_NUM_RECEIVE_PACKETS; i++ )
      {
         if ( ipx_send_packet_rm[i] != 0 )
            DpmiFreeDosMem( ipx_send_sel[i] );
         ipx_send_packet_rm[i] = 0;
      }
      for ( i = 0; i < IPX_NUM_RECEIVE_PACKETS; i++ )
      {
         if ( ipx_receive_packet_rm[i] != 0 )
            DpmiFreeDosMem( ipx_rcv_sel[i] );
         ipx_receive_packet_rm[i] = 0;
      }
   }
   #endif
}

/*
** IpxCloseSocket
**
** Closes the specified IPX socket.
*/
void IpxCloseSocket( IpxSocket_t sid )
{
   union REGS r;

   if ( ipx_state.socketid != IPX_NULL_SOCKET )
   {
      REGW( r, bx ) = IPX_FNC_CLOSE_SOCKET;
      REGW( r, dx ) = sid;

      IpxCallDriver( &r, 0 );
   }
}

/*
** IpxNodesEqual
**
** Determines if two IpxNodeAddress structures are identical
*/
FxBool IpxNodesEqual( const IpxNodeAddress *a, const IpxNodeAddress *b )
{
   return ( FxBool ) ( memcmp( &a->node, &b->node, sizeof( a->node ) ) == 0 );
}

/*
** IpxConfigurePacketPointers
**
** This function is responsible for making sure all the various
** packet pointers point to the appropriate chunks of memory.
**
** If running in protected mode then chunks of real-mode DOS memory
** have to be allocated, since the packets will be accessed directly
** by a real mode routine (i.e. the IPX driver).
**
** IMPORTANT:  packets MUST be zeroed out before use or their contents
** will be incorrectly interpreted by the IPX driver, leading to all
** kinds of bizarre bugs and unexpected behaviour.
*/
FxBool IpxConfigurePacketPointers( void )
{
   int i;

   #ifdef REALMODE

   /*
   ** assign pointers
   */
   for ( i = 0; i < IPX_NUM_RECEIVE_PACKETS; i++ )
      ipx_sp[i] = &ipx_send_packet[i];
   for ( i = 0; i < IPX_NUM_RECEIVE_PACKETS; i++ )
      ipx_rp[i] = &ipx_receive_packet[i];

   #else

   /*
   ** allocate real-mode memory
   */
   for ( i = 0; i < IPX_NUM_SEND_PACKETS; i++ )
   {
     if ( ( ipx_send_packet_rm[i] = ( IpxGenericPacket * ) DpmiAllocDosMem( sizeof( IpxGenericPacket ), &ipx_send_sel[i] ) ) == 0 )
     {
        IpxSetError( IPX_ERROR_DPMI_DOS_MEM_FAILED );
        return FXFALSE;
     }
   }
   for ( i = 0; i < IPX_NUM_RECEIVE_PACKETS; i++ )
   {
     if ( ( ipx_receive_packet_rm[i] = ( IpxGenericPacket * ) DpmiAllocDosMem( sizeof( IpxGenericPacket ), &ipx_rcv_sel[i] ) ) == 0 )
     {
        IpxSetError( IPX_ERROR_DPMI_DOS_MEM_FAILED );
        return FXFALSE;
     }
   }

   /*
   ** assign pointers
   */
   for ( i = 0; i < IPX_NUM_RECEIVE_PACKETS; i++ )
      ipx_rp[i] = ipx_receive_packet_rm[i];

   for ( i = 0; i < IPX_NUM_SEND_PACKETS; i++ )
      ipx_sp[i] = ipx_send_packet_rm[i];

   #endif

   /*
   ** clear out memory
   */
   for ( i = 0; i < IPX_NUM_RECEIVE_PACKETS; i++ )
   {
      memset( ( void * ) ipx_rp[i], 0, sizeof( IpxGenericPacket ) );
   }
   for ( i = 0; i < IPX_NUM_SEND_PACKETS; i++ )
   {
      memset( ( void * ) ipx_sp[i], 0, sizeof( IpxGenericPacket ) );
   }

   return FXTRUE;
}

/*
** IpxError
**
** Returns the last error generated internally then resets
** the last error to IPX_ERROR_NONE.  A string describing
** the nature of a particular error can be retrieved using:
**
** const char *string = IpxErrorString( IpxError() );
*/
IpxError_t IpxError( void )
{
   IpxError_t e = ipx_state.last_error;

   ipx_state.last_error = IPX_ERROR_NONE;

   return e;
}

/*
** IpxGetErrorString
**
** Returns a string describing the nature of a particular
** error.  Used for debugging.
*/
const char *IpxGetErrorString( IpxError_t e )
{
   return ipx_error_string[e];
}

/*
** IpxErrorString
*/
const char *IpxErrorString( void )
{
   return IpxGetErrorString( IpxError() );
}

/*
** IpxGetLocalAddress
**
** name is the buffer that will contain the address as a string
** length is the length of the name buffer. Must be > 6.
** returns the length of the local address (should always be 6)
**
*/
int IpxGetLocalAddress( char *name, int length)
{
  int          i;
  union  REGS  r;
  struct SREGS sr;

  #ifdef REALMODE
  REGW( r, si ) = RM_OFF( &ipx_state.local_address );
  sr.es         = RM_SEG( &ipx_state.local_address );
  REGW( r, bx ) = IPX_FNC_GET_LOCAL_ADDRESS;

  IpxCallDriver( &r, &sr );

  #else
  void           *ptr;
  DpmiSelector_t  sel;

  /* Make sure that the length of the name buffer is long enough
  ** 6 is the size of the node field in the IpxLocalAddress structure.
  */
  if ( length < 6 ) {
      printf( "ATB_IPX error: IpxGetLocalAddress must be called with a name and length > 6 bytes\n" );
      return -1;
  }
    
  /*
  ** the IPX driver expects ES:SI to point to a buffer where the local
  ** address can be stored.  This means we have to allocate a real
  ** mode memory block temporarily so that the driver has somewhere to
  ** stick the address.
  */
  if ( ( ptr = DpmiAllocDosMem( sizeof( IpxLocalAddress ), &sel ) ) == 0 )
  {
     IpxSetError( IPX_ERROR_DPMI_DOS_MEM_FAILED );
     return FXFALSE;
  }

  REGW( r, si ) = RM_OFF( ptr );
  sr.es         = RM_SEG( ptr );
  REGW( r, bx ) = IPX_FNC_GET_LOCAL_ADDRESS;

  IpxCallDriver( &r, &sr );

  /*
  ** copy from the temp buffer back out to our global address buffer
  */
  memcpy( &ipx_state.local_address, ptr, sizeof( ipx_state.local_address ) );

  /*
  ** be a good boy and free up the memory
  */
  DpmiFreeDosMem( sel );

  #endif

  printf( "IpxGetLocalAddress() = " );
  for ( i = 0; i < 6; i++ )
  {
     printf( "%02x ", ipx_state.local_address.node[i] );
     /* fill in the name parameter */
     name[i] = ipx_state.local_address.node[i];
  }
  printf( "\n" );

  return 6;
}

/*
** IpxGetPacket
**
** Finds a full receive packet, copies its data into the buffer provided
** by the application, copies the source node into the source node buffer
** provided by the application, then returns.
**
** Denis
** returns the number of bytes read if data was found and retrieved
** returns -1 if there were no waiting packets
*/
int IpxGetPacket( void *data )
{
   int i;

   for ( i = 0; i < IPX_NUM_RECEIVE_PACKETS; i++ )
   {
      if ( !ipx_rp[i]->ecb.in_use )
      {
         // IpxPacketHeader::length is big-endian so we have to byte swap
         FxU32 length = SWAP_BYTES( ipx_rp[i]->ipx.length ) - sizeof( IpxPacketHeader ) - sizeof( FxI32 );

         memcpy( data, ipx_rp[i]->data, length );

         // tell the IPX driver to start listening for packets again
         IpxListenForPacket( &ipx_rp[i]->ecb );

         return length;
      }
   }
   return -1;
}

/*
** IpxListenForPacket
**
** Tells the IPX driver to start listening for incoming data using the
** given ECB.
*/
FxBool IpxListenForPacket( volatile IpxECB *ecb )
{
   union REGS   r;
   struct SREGS sr;

   REGW( r, si ) = RM_OFF( ecb );
   sr.es         = RM_SEG( ecb );
   REGW( r, bx ) = IPX_FNC_LISTEN;

   IpxCallDriver( &r, &sr );

   if ( REGB( r, al ) == 0 )
      return FXTRUE;

   switch ( REGB( r, al ) )
   {
      case 0xFF:
         IpxSetError( IPX_ERROR_NONEXISTENT_SOCKET );
         break;
      default:
         IpxSetError( IPX_ERROR_GENERIC_FAILURE );
         break;
   }
   return FXFALSE;
}

/*
** IpxNodeToString
*/
char *IpxNodeToString( char buffer[], const IpxNodeAddress *n )
{
   sprintf( buffer, "%02x %02x %02x %02x %02x %02x", n->node[0],
                                                     n->node[1],
                                                     n->node[2],
                                                     n->node[3],
                                                     n->node[4],
                                                     n->node[5] );
   return buffer;
}

/*
** IpxOpen
**
** Initialize the IPX library using the given socket address.
*/
FxBool IpxOpen( const void *open_data )
{
   int i, j;
   FxU32 socket_address = * ( const FxU32 * ) open_data;
   char name[255];

   /*
   ** Determine if IPX is present
   */
   if ( IpxPresent() == FXFALSE )
   {
      IpxSetError( IPX_ERROR_IPX_NOT_DETECTED );
      return FXFALSE;
   }

   /*
   ** configure our packet pointers
   */
   IpxConfigurePacketPointers();

   /*
   ** open a socket for sending and receiving
   */
   if ( ( ipx_state.socketid = IpxOpenSocket( socket_address /*, use_open_socket */ ) ) == IPX_NULL_SOCKET )
      return FXFALSE;

   /*
   ** get machine's local address
   */
   if ( !IpxGetLocalAddress( &name, 6 ) )
      return FXFALSE;

   /*
   ** set up receiving ECBs
   */
   for ( i = 0; i < IPX_NUM_RECEIVE_PACKETS; i++ )
   {
      ipx_rp[i]->ecb.in_use         = 0;
      ipx_rp[i]->ecb.ECB_socket     = ipx_state.socketid;
      ipx_rp[i]->ecb.fragment_count = 1;

      ipx_rp[i]->ecb.fragment_address[0] = RM_OFF( &ipx_rp[i]->ipx );
      ipx_rp[i]->ecb.fragment_address[1] = RM_SEG( &ipx_rp[i]->ipx );

		ipx_rp[i]->ecb.fragment_size = sizeof( IpxGenericPacket ) - sizeof( IpxECB );

      if ( !IpxListenForPacket( &ipx_rp[i]->ecb ) )
         return FXFALSE;
   }

   /*
   ** set up a sending ECBs
   */
   for ( i = 0; i < IPX_NUM_SEND_PACKETS; i++ )
   {
      ipx_sp[i]->ecb.ECB_socket          = ipx_state.socketid;
      ipx_sp[i]->ecb.fragment_count      = 1;
      ipx_sp[i]->ecb.fragment_address[0] = RM_OFF( &ipx_sp[i]->ipx );
      ipx_sp[i]->ecb.fragment_address[1] = RM_SEG( &ipx_sp[i]->ipx );

      for ( j = 0; j < 4; j++ )
         ipx_sp[i]->ipx.dest_network[j] = ipx_state.local_address.network[j];
      ipx_sp[i]->ipx.dest_socket[0] = ipx_state.socketid & 0xFF;
      ipx_sp[i]->ipx.dest_socket[1] = ipx_state.socketid >> 8;
   }

   return FXTRUE;
}

/*
** IpxOpenSocket
**
** Opens an IPX socket for use
*/
IpxSocket_t IpxOpenSocket( FxU16 value /*, FxBool use_open_socket */ )
{
   union REGS    r;
   IpxSocket_t sid;

   // the driver expects a big-endian socket number
   value = SWAP_BYTES( value );

   // use a short lived socket since we won't it open after program
   // termination
   REGB( r, al ) = IPX_SOCKET_SHORT_LIVED;
   REGW( r, bx ) = IPX_FNC_OPEN_SOCKET;
   REGW( r, dx ) = value;

   IpxCallDriver( &r, 0 );

   if ( REGB( r, al ) != 0x00 )
   {
      switch ( REGB( r, al ) )
      {
         case 0xFE:
            IpxSetError( IPX_ERROR_SOCKET_TABLE_FULL );
            break;
         case 0xFF:
            IpxSetError( IPX_ERROR_SOCKET_ALREADY_OPEN );
            break;
         default:
            IpxSetError( IPX_ERROR_GENERIC_FAILURE );
            break;
      }
      sid = IPX_NULL_SOCKET;
   }
   else
   {
     sid = REGW( r, dx );
   }

   return sid;
}

/*
** IpxPacketWaiting
**
** returns FXTRUE if there is a packet waiting to have its contents
** copied out, FXFALSE otherwise.
*/
FxBool IpxPacketWaiting( void )
{
   int i;

   for ( i = 0; i < IPX_NUM_RECEIVE_PACKETS; i++ )
   {
      if ( !ipx_rp[i]->ecb.in_use )
      {
         /*
         ** if the originator is the same as the destination ignore
         */
         if ( memcmp( ipx_rp[i]->ipx.src_node, &ipx_state.local_address.node, sizeof( ipx_state.local_address.node ) ) == 0 )
         {
            IpxListenForPacket( &ipx_rp[i]->ecb );
            return FXFALSE;
         }
         return FXTRUE;
      }
   }

   return FXFALSE;
}

/*
** IpxPresent
**
** Determines if an IPX driver is loaded.  It does this by executing
** the DOS multiplex interrupt with AH = 0x7A and AL = 0x00.  Upon
** return AL is expected to be 0xFF upon success.
**
** A pointer to a directly accessible real mode procedure is returned
** by the DOS interrupt in ES:DI.  This procedure can be used instead
** of the IPX interrupt 0x7A.
*/
FxBool IpxPresent( void )
{
   #ifdef REALMODE
   union  REGS  r;
   struct SREGS sr;

   r.h.ah = 0x7a;
   r.h.al = 0x00;
   INT86X( MULTIPLEX_INTERRUPT, &r, &r, &sr );

   if ( REGB( r, al ) != 0xFF )
      return FXFALSE;

   ipx_direct = MK_FP( sr.es, r.x.di );

   #else
   DpmiRMI rmi;

   memset( &rmi, 0, sizeof( rmi ) );

   rmi.EAX = 0x7A00;
   DpmiExecuteRealModeInterrupt( MULTIPLEX_INTERRUPT, &rmi );

   if ( ( rmi.EAX & 0xFF ) != 0xFF )
      return FXFALSE;

   ipx_direct_seg = rmi.ES;
   ipx_direct_off = rmi.EDI;

   #endif

   return FXTRUE;
}

/*
** IpxResendPacket
*/
FxBool IpxResendPacket( FxI32 sequence, const IpxNodeAddress *target )
{
   int i;

   /*
   ** see if we still have the data and resend it if we do
   */
   for ( i = 0; i < IPX_NUM_SEND_PACKETS; i++ )
   {
      if ( ipx_sp[i]->sequence == sequence )
      {
         _IpxSendPacket( ipx_sp[i], target );
         return FXTRUE;
      }
   }

   IpxSetError( IPX_ERROR_RESEND_PACKET_TOO_OLD );

   return FXFALSE;
}

/*
** IpxSendPacket
**
** Sends a chunk of data to the given target address.  The target address
** can be set to IPX_NODE_BROADCAST if we want to blurt this out to the
** universe.
*/
FxBool IpxSendPacket( const void *data, int data_length, FxU32 flags  )
{
   clock_t                    poll_start;
   volatile IpxGenericPacket *free_ipx_sp = 0;
   const IpxNodeAddress      *target      = IPX_NODE_BROADCAST;

   /*
   ** make sure we're not sending too much data
   */
   if ( data_length > IPX_MAX_PACKET_DATA_SIZE )
   {
      IpxSetError( IPX_ERROR_PACKET_TOO_LARGE );
      return FXFALSE;
   }

   /*
   ** wait until the next packet is available
   ** some type of time out should probably be stuck in here
   */
   poll_start = clock();

   while ( ipx_sp[ipx_state.current_send_packet]->ecb.in_use )
   {
      if ( ( clock() - poll_start ) > CLK_TCK )
      {
         IpxSetError( IPX_ERROR_TIMEOUT );
         return FXFALSE;
      }
   }
   free_ipx_sp = ipx_sp[ipx_state.current_send_packet];
   ipx_state.current_send_packet = ( ipx_state.current_send_packet + 1 ) % IPX_NUM_SEND_PACKETS;

   /*
   ** copy data into packet data area, update the sequence, compute
   ** fragment size, and set type to 0.
   */
   memcpy( free_ipx_sp->data, data, data_length );
   free_ipx_sp->ecb.fragment_size = data_length + sizeof( IpxPacketHeader ) + sizeof( FxI32 );
   free_ipx_sp->sequence          = ipx_state.sequence++;
   free_ipx_sp->ipx.type          = 0;

   /*
   ** tell it to go
   */
   _IpxSendPacket( free_ipx_sp, target );

   return FXTRUE;
}

/*
** _IpxSendPacket
**
** Internal helper function to toss out packets
*/
static void _IpxSendPacket( volatile IpxGenericPacket *packet, const IpxNodeAddress *target )
{
   int          j;
   union REGS   r;
   struct SREGS sr;

   /*
   ** set destination node
   */
   for ( j = 0; j < 6; j++ )
      packet->ipx.dest_node[j] = packet->ecb.immediate_address[j] = target->node[j];

   /*
   ** send the packet
   */
   sr.es         = RM_SEG( packet );
   REGW( r, si ) = RM_OFF( packet );
   REGW( r, bx ) = IPX_FNC_SEND;

   IpxCallDriver( &r, &sr );
}

/*
** IpxSetError
**
** Sets the current error.
*/
static void IpxSetError( IpxError_t e )
{
   printf( "IpxError(%d): ('%s')\n", e, ipx_error_string[e] );
   ipx_state.last_error = e;
}

/*
** IpxSocketIsOpen
**
** Returns FXTRUE if the specified socket is open.  The way I wrote this
** SEEMS to be valid, but this is more of a hack than anything else and
** it may not even work.  I'm pretty sure it does, though.
*/
FxBool IpxSocketIsOpen( FxU16 value )
{
   IpxSocket_t sid;

   if ( ( sid = IpxOpenSocket( value /*, FXFALSE */ ) ) != IPX_NULL_SOCKET )
   {
      IpxCloseSocket( sid );
      return FXFALSE;
   }
   else
   {
      if ( IpxError() == IPX_ERROR_SOCKET_ALREADY_OPEN )
        return FXTRUE;
      return FXFALSE;
   }
}

/*
** IpxCallDriver
**
** Calls the IPX driver.  There are four basic ways of calling the
** IPX driver:
**
**   real-mode int 0x7A
**   prot-mode int 0x7A
**   real-mode direct procedure call
**   prot-mode direct procedure call
**
** Of these the first three are implemented.  The last one has not been
** implemented because it requires DPMI function 0x301, which is not
** present in DOS4GW (but it IS present in DOS4GW/Pro).  I'm assuming
** that the last one will be implemented when we get DOS4GW/Pro.
**
** Using the IPX interrupt works, but it is no longer officially supported
** by Novell.  I don't really think this is a concern for our particular
** applications.
*/
#define USE_IPX_INT
#ifdef REALMODE
void IpxCallDriver( union REGS *r, struct SREGS *sr )
{
   FxU16 new_ax = REGW( *r, ax ),
         new_bx = REGW( *r, bx ),
         new_cx = REGW( *r, cx ),
         new_dx = REGW( *r, dx ),
         new_si = REGW( *r, si ),
         new_di = REGW( *r, di ),
         new_es = 0;

   #ifdef USE_IPX_INT
   if ( sr != 0 )
      INT86X( IPX_INTERRUPT, r, r, sr );
   else
      INT86( IPX_INTERRUPT, r, r );
   #else
   if ( sr != 0 )
      new_es = sr->es;

   asm mov  ax, new_ax
   asm mov  bx, new_bx
   asm mov  cx, new_cx
   asm mov  dx, new_dx
   asm mov  si, new_si
   asm mov  di, new_di
   asm mov  es, new_es

   asm push bp

   ipx_direct();

   asm pop bp

   asm mov  new_ax, ax
   asm mov  new_bx, bx
   asm mov  new_cx, cx
   asm mov  new_dx, dx
   asm mov  new_si, si
   asm mov  new_di, di

   REGW( *r, ax ) = new_ax;
   REGW( *r, bx ) = new_bx;
   REGW( *r, cx ) = new_cx;
   REGW( *r, dx ) = new_dx;
   REGW( *r, si ) = new_si;
   REGW( *r, di ) = new_di;

   #endif
}
#else
void IpxCallDriver( union REGS *r, struct SREGS *sr )
{
   DpmiRMI rmi;

   memset( &rmi, 0, sizeof( rmi ) );

   rmi.EAX = r->w.ax;
   rmi.EBX = r->w.bx;
   rmi.ECX = r->w.cx;
   rmi.EDX = r->w.dx;
   rmi.ESI = r->w.si;
   rmi.EDI = r->w.di;

   #ifdef USE_IPX_INT
   if ( sr != 0 )
   {
      rmi.ES  = sr->es;
   }
   DpmiExecuteRealModeInterrupt( IPX_INTERRUPT, &rmi );
   #else
   puts( "Ipx direct call not implemented" );
   exit( 1 );
   if ( sr != 0 )
   {
      rmi.ES = sr->es;
   }
//   DpmiExecuteRealModeProcedure( ipx_proc_seg, ipx_proc_&rmi );
   #endif

   r->w.ax = rmi.EAX;
   r->w.bx = rmi.EBX;
   r->w.cx = rmi.ECX;
   r->w.dx = rmi.EDX;
   r->w.si = rmi.ESI;
   r->w.di = rmi.EDI;
}
#endif
