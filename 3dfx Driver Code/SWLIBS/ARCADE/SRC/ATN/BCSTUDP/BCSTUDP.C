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
** $Date: 10/11/00 7:33:45 PM$
*/

#include <winsock.h>

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <3dfx.h>

#include "bcstudp.h"
#include "util.h"

static FxBool       __net_initialized;
static ConnectInfo  _gConnectInfo;
static FxBool       _gVerbose = FXFALSE;

/*
** NetOpen
*/
FxBool BuNetOpen( void )
{
    struct sockaddr_in    serv_addr, cli_addr;
    WSADATA	wsaData;
    u_long	inaddr;
    BOOL    optVal = TRUE;

    if ( getenv("VERBOSE") )   {
        _gVerbose = FXTRUE;
        printC( "BuLib: Verbose was found true.\n" );
    }

    /* This is not necessary? since it is not a real windows application ??? */
    if ( _gVerbose )    {
        /* Alloc the Console for doing output (it's a WIN32 application) */
        printC( "BuLib: About to allocate a console.\n" );
        AllocConsole();
    }

    if (WSAStartup(WINSOCK_VERSION, &wsaData) != 0) return FXFALSE;

    _gConnectInfo.myPort = NET_BROADCAST_PORT;

    /* Open a UDP socket (an Internet datagram socket). */
    if ( (_gConnectInfo.sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        if ( _gVerbose ) printC( "BuLib: Can't open datagram socket" );
        return FXFALSE;
    }
    
    /* Set the socket option for BROADCASTING     */
    if ( setsockopt( _gConnectInfo.sockfd, SOL_SOCKET, SO_BROADCAST, 
        (const char *)&optVal, sizeof( optVal ) ) < 0 )    {
        if ( _gVerbose ) printC( "BuLib: Failed to set BROADCAST socket option.\n" );
    } else if ( _gVerbose ) printC( "BuLib: SUCCEED setting BROADCAST option! \n" );

    /* 
    ** Experimental: try setting the socket to NONBLOCKING mode
    ** to see if we can boost the sendto() performance. By default,
    ** it seems like Windows create sockets in a blocking mode 
    ** (equivalent to Berkeley sockets).
    ** In DDerby it was found that it takes ~ 1ms to BuNetSendPacket()
    ** I'm assuming that if the socket is in the NONBLOCKING mode
    ** the sento() call will return immediatly, and then the packet
    ** will be sent a bit later. In the case of a 60 Hz game, if we
    ** only send one packet per frame, that's one packet every 16 ms,
    ** which should leave plenty of time to the OS to send the packet
    ** by the time the game tries to send the next packet.
    ** FIONBIO is for controlling the blocking vs nonblocking
    ** last parameter: non-zero value selects nonblocking
    **
    ** WARNING: if the socket is set to nonblocking, than any call
    ** to recvfrom without any data to be read will cause the call
    ** to fail returning WSAEWOULDBLOCK. 
    ** To sum up: by default, a socket is blocking. so one can call
    ** recvfrom even when there is no data to be read, the call will
    ** simply block. If one wants to avoid blocking, do a call with
    ** select first, using a small timeout, and if it returns that
    ** a socket is ready for reading, then go and use the recvfrom.
    ** If a socket is changed to NONBLOCKING, then you cannot use
    ** the recvfrom unless you know there is something to read.
    **
    ** Big Question: what consequence does this have on read()?
    ** when reading a socket, I usually expect the call to either
    ** return because nothing was there to be read (I usually use a
    ** small timeout), or to return because there was something to be
    ** read. In the case of nonblocking IO, does the read() call always
    ** return, and then I need to use some of the Windows crap to try
    ** to get my network packet????
    ** 
    ** Results: I can't seem to detect any difference in DDerby non-blocking.
    ** 
    */
    #if 0
    {
        u_long argp = 1;
        if ( ioctlsocket ( _gConnectInfo.sockfd, FIONBIO, &argp ) != 0 )    {
            if ( _gVerbose ) printC( "BuLib: Failed to set NONBLOCKING ioctlsocket option.\n" );
        } else {
            if ( _gVerbose ) printC( "BuLib: Succeeding setting NONBLOCKING ioctlsocket option.\n" );
        }
    }
    #endif

    /* Now, we want to bind to our address = * (ANY address) and PORT = MYPORT = NET_BROADCAST_PORT
    ** Bind a local address for us.
    ** It is either the default address (INADDR_ANY) or the the one provided as an
    ** argument to the player. 
    cli_addr.sin_family      = AF_INET;
    cli_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    cli_addr.sin_port        = htons((short) _gConnectInfo.myPort );
        */
    inaddr = INADDR_ANY;
    memcpy( (char *) &cli_addr.sin_addr, (char *) &inaddr,sizeof(inaddr));
    cli_addr.sin_family      = AF_INET;
    cli_addr.sin_port        = htons((short) NET_BROADCAST_PORT );
    if (bind( _gConnectInfo.sockfd, (struct sockaddr *) &cli_addr, sizeof(cli_addr)) < 0)	{
        printC( "BuLib:  can't bind local address" );
        return FXFALSE;
    }

    inaddr = INADDR_BROADCAST;
    memcpy( (char *) &serv_addr.sin_addr, (char *) &inaddr,sizeof(inaddr));
    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_port        = htons((short) NET_BROADCAST_PORT );
    
    _gConnectInfo.serv_addr = serv_addr;

    __net_initialized = FXTRUE;

    if ( _gVerbose ) printC( "BuLib: End of BuNetOpen()\n" );
    return FXTRUE;
}

/*
** NetClose
*/
FxBool BuNetClose( void )
{
    int ret;
    if ( !__net_initialized )
      return FXFALSE;

    ret = WSACleanup();
    if ( ret == 0 ) return FXTRUE;
    else {
        if ( ret == WSANOTINITIALISED )  printC( "BuLib:    WSACleanup returned WSANOTINITIALISED\n" );
        if ( ret == WSAENETDOWN )  printC( "BuLib:    WSACleanup returned WSAENETDOWN\n" );
        if ( ret == WSAEINPROGRESS )  printC( "BuLib:   WSACleanup returned WSAEINPROGRESS\n" );
    }
    return FXFALSE;
}

/*
** NetErrorString
*/
const char *BuNetErrorString( void )
{
    if ( !__net_initialized )
        return "BcstUdp not initialized!";
    return "Need to do something here\n";
}

/*
** NetGetPacket
** 	  Returns the number of bytes read.
**	  Returns -1 if there was an error.
*/
int BuNetGetPacket( char *buffer )
{
//     struct sockaddr  cli_addr;
//    int sizeCli = sizeof( cli_addr );

    if ( !__net_initialized )
     return -1;
    return recvfrom( _gConnectInfo.sockfd, (char *) buffer, 1000, 0, 
       0,0);
//       (LPSOCKADDR)&cli_addr, &sizeCli);
}

/*
** NetPacketWaiting
*/
FxBool BuNetPacketWaiting( void )
{

    int	    res;
    fd_set	fd;
    struct timeval      tv;

    if ( !__net_initialized )
        return FXFALSE;

    tv.tv_sec = 0;
    tv.tv_usec = 1;

    FD_ZERO(&fd);
    FD_SET( _gConnectInfo.sockfd, &fd );
    res = select( _gConnectInfo.sockfd+1, &fd, (fd_set *)0, (fd_set *)0, &tv );
    /* if ( _gVerbose ) printC( "BuLib: Value returned by select=%d \n", res ); */
    if (res < 0)    { /* error reading socket */
            if ( _gVerbose ) printC( "BuLib: Error on select. \n" );
            return FXFALSE;
    } else  {
            if ( res > 0)   { /* data on the socket */
                /* if ( _gVerbose ) printC( "BuLib:  Yeah!!! Data on the socket (from bcstudp.lib) \n" ); */
                return FXTRUE;
            } else { /* we've timed out on the select and we haven't received any data */
                return FALSE;
            }
    }
    
}

/*
** NetSendPacket
*/
int BuNetSendPacket( char *buffer, int buflen, FxU32 flags )
{
   if ( !__net_initialized )
     return -1;

   /*
   if ( _gVerbose ) printC( "BuLib: sending packet %s \n", buffer );
   */
   return sendto( _gConnectInfo.sockfd, (char *)buffer, buflen,
            0, (struct sockaddr*)&_gConnectInfo.serv_addr, sizeof( _gConnectInfo.serv_addr ));

}


int BuNetGetHostname( char *name, int length )
{
    int ret = gethostname( name, length );

    if ( ret < 0 ) {
        if ( _gVerbose ) printC( "BuLib: error on gethostname() \n" );
        return ret;
    } else if ( ret == 0 )  {
        if ( _gVerbose ) printC( "BuLib: found hostname=%s, length=%d\n", name, strlen( name ) );
        return strlen( name );
    } else {
        if ( _gVerbose ) printC( "BuLib: bad error code returned by gethostname() \n" );
        return -1;
    };
}




