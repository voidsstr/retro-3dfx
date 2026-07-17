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
#ifndef __BCSTUDP_H__
#define __BCSTUDP_H__

#include <winsock.h>
#include <3dfx.h>

#define NET_ID_BROADCAST    0xFFFFFFFFL
#define NET_MAX_PACKET_SIZE 512
#define	WINSOCK_VERSION	    0x0101

#define NET_BROADCAST_PORT  2700

/*  Current Network State */
typedef	struct	{
    int	sockfd;
    struct sockaddr_in  serv_addr;
    FxU8    myId;
    FxU32   mySeq;
    int	    myPort;
} ConnectInfo;


FxBool BuNetOpen( void );
FxBool BuNetClose( void );
int BuNetSendPacket( char *buffer, int buflen, FxU32 flags );
int BuNetGetPacket( char *buffer );
FxBool BuNetPacketWaiting( void );

/* int BuNetGetHostname( char *name, int length );
** length is the length of the name buffer that will contain
** the name of the local host computer.
** returns the actual length of the name if success,
** returns -1 otherwise 
*/
int BuNetGetHostname( char *name, int length );

const char *BuNetErrorString( void );

#endif
