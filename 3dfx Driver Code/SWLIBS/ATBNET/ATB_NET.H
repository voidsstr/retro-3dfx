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
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
*/
#ifndef __ATB_NET_H__
#define __ATB_NET_H__

#include <3dfx.h>

typedef FxI32 NET_ID;

#define NET_ID_BROADCAST    0xFFFFFFFFL
#define NET_MAX_PACKET_SIZE 512

typedef enum
{
   NET_SEND_GUARANTEE = 0x0000001
} NetSendFlag;

FxBool NetOpen( void );
FxBool NetClose( void );
FxBool NetSendPacket( const void *buffer, int buflen, FxU32 flags );
/* Denis. Changed the return value from Bool to int.
** 	  Returns the number of bytes read.
**	  Returns -1 if there was an error.
*/
int NetGetPacket( const void *buffer );
FxBool NetPacketWaiting( void );

/* int NetGetHostname( char *name, int length );
** length is the length of the name buffer that will contain
** the name of the local host computer.
** returns the actual length of the name if success,
** returns -1 otherwise 
** Note: for IPX, I don't think there is a local host name
** so this call returns indeed the IPX address and node.
** This call is essentially meant to be used as a way to
** uniquely identify a machine on a local network.
*/
int NetGetHostname( char *name, int length );

const char *NetErrorString( void );

#endif
