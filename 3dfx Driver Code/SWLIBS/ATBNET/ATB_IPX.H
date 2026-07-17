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
** $Date: 10/11/00 7:35:38 PM$
*/
#ifndef __FXIPX_H__
#define __FXIPX_H__

#ifndef __3DFX_H__
#  include <3dfx.h>
#endif

#ifdef __WATCOMC__
#pragma pack(0)
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
** constants and data types
*/
#define SWAP_BYTES(a) ((FxU16)(((( FxU16 )a)>>8)|(a<<8)))

#define DEFAULT_3DFX_IPX_SOCKET   0xFD30
#define IPX_NULL_SOCKET           0xFFFF
#define IPX_NUM_RECEIVE_PACKETS   10
#define IPX_NUM_SEND_PACKETS      1
#define IPX_MAX_PACKET_DATA_SIZE  512
#define IPX_PACKET_HEADER_SIZE    30

#define IPX_SOCKET_SHORT_LIVED    0x00
#define IPX_SOCKET_LONG_LIVED     0x01

typedef enum
{
   IPX_CONFIG_USE_OPEN_SOCKET    = 0x0001,
   IPX_CONFIG_ALLOW_SEND_TIMEOUT = 0x0002
} IpxConfigFlag_t;

typedef enum
{
   IPX_FNC_OPEN_SOCKET       = 0,
   IPX_FNC_CLOSE_SOCKET      = 1,
   IPX_FNC_GET_LOCAL_TARGET  = 2,
   IPX_FNC_SEND              = 3,
   IPX_FNC_LISTEN            = 4,
   IPX_FNC_SCHEDULE          = 5,
   IPX_FNC_CANCEL            = 6,
   IPX_FNC_GET_LOCAL_ADDRESS = 9
} IpxFunction_t;

typedef enum
{
   IPX_ERROR_NONE,
   IPX_ERROR_GENERIC_FAILURE,

   IPX_ERROR_IPX_NOT_DETECTED,

   IPX_ERROR_DPMI_DOS_MEM_FAILED,

   /*
   ** IPX_FNC_OPEN_SOCKET
   */
   IPX_ERROR_SOCKET_TABLE_FULL,
   IPX_ERROR_SOCKET_ALREADY_OPEN,

   /*
   ** IPX_SEND
   */
   IPX_ERROR_PACKET_TOO_LARGE,
   IPX_ERROR_TIMEOUT,
   IPX_ERROR_RESEND_PACKET_TOO_OLD,

   /*
   ** IPX_FNC_LISTEN
   */
   IPX_ERROR_NONEXISTENT_SOCKET,

   /*
   ** IPX_FNC_CANCEL
   */
   IPX_ERROR_CANNOT_CANCEL,
   IPX_ERROR_ECB_NOT_BUSY
} IpxError_t;

typedef FxU16 IpxSocket_t;

/*
** IpxLocalAddress
*/
typedef struct
{
   FxU8 network[4];          // big-endian
   FxU8 node[6];             // big-endian
} IpxLocalAddress;

/*
** IpxNodeAddress
*/
typedef struct
{
	FxU8 node[6];
} IpxNodeAddress;

/*
** IpxPacketHeader
*/
typedef struct
{
   FxU16 check_sum;
   FxU16 length;
   FxU8  transport_control;
   FxU8  type;

   FxU8  dest_network[4];
   FxU8  dest_node[6];
   FxU8  dest_socket[2];

   FxU8  src_network[4];
   FxU8  src_node[6];
   FxU8  src_socket[2];
} IpxPacketHeader;

/*
** IpxECB
*/
typedef struct
{
   void    *link_address;
   void   (*esr)();
   FxU8    in_use;
   FxU8    completion_code;
   FxU16   ECB_socket;
   FxU8    ipx_workspace[4];
   FxU8    driver_workspace[12];
   FxU8    immediate_address[6];
   FxI16   fragment_count;
   FxU16   fragment_address[2];
   FxI16   fragment_size;
} IpxECB;

/*
** IpxGenericPacket
*/
typedef struct
{
   IpxECB          ecb;
   IpxPacketHeader ipx;
   FxI32           sequence;
   char            data[IPX_MAX_PACKET_DATA_SIZE];
} IpxGenericPacket;

/*
** internal function prototypes
*/
void          IpxCloseSocket( IpxSocket_t sid );
IpxSocket_t   IpxOpenSocket( FxU16 socket_address );
FxBool        IpxListenForPacket( volatile IpxECB *ecb );
void          IpxSetError( IpxError_t e );

/*
** published function prototypes
*/
int    IpxGetLocalAddress( char *name, int length );
FxBool IpxOpen( const void *open_data );
void   IpxClose( void );

FxBool IpxNodesEqual( const IpxNodeAddress *a, const IpxNodeAddress *b );
char  *IpxNodeToString( char buffer[], const IpxNodeAddress *n );

FxBool IpxSocketIsOpen( FxU16 socket_address );
FxBool IpxPacketWaiting( void );
/* Denis. Changed the return value from Bool to int
	(the number of bytes read)
*/
int	IpxGetPacket( void *data );

const char *IpxErrorString( void );
const char *IpxGetErrorString( IpxError_t e );
IpxError_t  IpxError( void );

FxBool      IpxSendPacket( const void *data, int data_length, FxU32 flags );

extern const IpxNodeAddress *IPX_NODE_BROADCAST;

#ifdef __cplusplus
}
#endif

#endif
