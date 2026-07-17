/****************************************************************************
*
*  File              : client.c
*
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <dos.h>
#include <string.h>

#include <bcstudp.h>

/*  Common header to client and server */
#include "common.h"

/*  Bandwith stuff */
unsigned long   gTotalBytes = 0, gTotalMessages = 0;
int             gClientId = 99;

/*  Controls how long we wait between each packet */
int gMaxTime = MAX_TIME;

void   DoSend( void );
void   DoSendAndReceive( void );

void main( int argc, const char *argv[] )
{
    char	name[255];
    int		length;

    if ( !BuNetOpen() )
    {
        exit( 1 );
    }

    if ( argc > 1 ) {
        gMaxTime = atoi( argv[1] );
    }
    printf( "Delay between packets:%d\n", gMaxTime );

    /* Test the gethostname call */
    length = BuNetGetHostname( &name, 255 );
    printf( "BuNetGethostname returned %d, and the name=%s\n", length, name );

    /* DoSend(); */
    DoSendAndReceive();

    if ( kbhit() ) getch();

    BuNetClose();
}



void DoSendAndReceive()
{
    char buffer[MAX_BUFFER];
    int  size = 0;
    int     nBytes = 0;
    FxBool   done = FXFALSE;

    while ( !done )  {

#ifdef CLIENT
        /* send */
        sprintf( buffer, "This is message nb %d from player nb %d.\n", 
            gTotalMessages, gClientId );
        printC( "Sending message %s \n", buffer );
        if ( BuNetSendPacket( buffer, strlen( buffer ) , NET_ID_BROADCAST ) != strlen( buffer ) )
        {
            printf( "DoSendAndReceive() - %s\n", BuNetErrorString() );
            return;
        } else {
            gTotalBytes += size;
            gTotalMessages++;
        }
        if ( gTotalBytes > MAX_BYTES ) done = FXTRUE;
#endif
#ifdef SERVER
        /* receive */
        if ( BuNetPacketWaiting()  )
        {
            nBytes = BuNetGetPacket( buffer );
            if ( nBytes == -1 ) {
                int error = WSAGetLastError(); 
                printf( "Error on BuNetGetPacket.\n" );
                switch ( error )    {
                case WSANOTINITIALISED :
                    printf( "A successful WSAStartup must occur before using this function.\n" );
                    break;
                case WSAENETDOWN :
                    printf( "The Windows Sockets implementation has detected that the network subsystem has failed.\n" );
                    break;
                case WSAEFAULT	:
                    printf( "The fromlen argument was invalid: the from buffer was too small to accommodate the peer address.\n" );
                    break;
                case WSAEINTR	:
                    printf( "The (blocking) call was canceled using WSACancelBlockingCall.\n" );
                    break;
                case WSAEINPROGRESS	:
                    printf( "A blocking Windows Sockets operation is in progress.\n" );
                    break;
                case WSAEINVAL	:
                    printf( "The socket has not been bound with bind.\n" );
                    break;
                case WSAENOTCONN	:
                    printf( "The socket is not connected (SOCK_STREAM only).\n" );
                    break;
                case WSAENOTSOCK	:
                    printf( "The descriptor is not a socket.\n" );
                    break;
                case WSAEOPNOTSUPP	:
                    printf( "MSG_OOB was specified, but the socket is not of type SOCK_STREAM.\n" );
                    break;
                case WSAESHUTDOWN	:
                    printf( "The socket has been shut down; it is not possible to recvfrom on a socket after shutdown has been invoked with how set to 0 or 2.\n" );
                    break;
                case    WSAEWOULDBLOCK	:
                    printf( "The socket is marked as nonblocking and the recvfrom operation would block.\n" );
                    break;
                case WSAEMSGSIZE	:
                    printf( "The datagram was too large to fit into the specified buffer and was truncated.\n" );
                    break;
                case WSAECONNABORTED	:
                    printf( "The virtual circuit was aborted due to timeout or other failure.\n" );
                    break;
                case WSAECONNRESET	:
                    printf( "The virtual circuit was reset by the remote side.\n" );
                    break;
                default:
                    printf( "Unknown WSA error\n" );

                }
                exit( -1 );
            } else {
                gTotalBytes += nBytes;
                gTotalMessages++;
                printf( "Received my first BROADCAST PACKET!: %s\n", buffer );
            }
            if ( gTotalBytes > MAX_BYTES - 500 ) done = FXTRUE;
        }
#endif
    }
}

void DoSend( void )
{
   char buffer[MAX_BUFFER];
   int  size = 0;
   FxBool   done = FXFALSE;
   float    startTime = 0.0f, endTime = 0.0f;
   int      buyTime = 0;

   strcpy( buffer, MESSAGE_STRING );
   size = strlen( buffer );

   startTime = timer();
   
   while ( !done )
   {
      if ( BuNetSendPacket( buffer, size , NET_ID_BROADCAST ) != size )
      {
         printf( "DoSend() - %s\n", BuNetErrorString() );
         return;
      } else {
          gTotalBytes += size;
          gTotalMessages++;
      }
      if ( gTotalBytes > MAX_BYTES ) done = FXTRUE;

      /* Wait a little bit before sending the next packet */
      buyTime = 0;
      while ( buyTime++ < gMaxTime ) ;

   }

    endTime = timer();

    /*
    **    Print the bandwith results
    */
    {
      float delta = endTime - startTime;
      float bytes_per_sec, kbytes_per_sec, bits_per_sec, kilobits_per_sec;
      printf( "----------------------------------------------------\n" );
      printf( "Nb of bytes sent: %d\n", gTotalBytes );
      printf( "Delta time = %f \n", delta );
      printf( " Bytes/sec = %f\n", gTotalBytes / delta );
      printf( " KiloBytes/sec = %f\n", ( gTotalBytes / 1000 ) / delta );
      //printf( " Bits/sec = %f\n", ( gTotalBytes * 8 ) / delta );
      //printf( " KiloBits/sec = %f\n", ( gTotalBytes * 8 / 1000 ) / delta );
      printf( " MBits/sec = %f\n", ( gTotalBytes * 8 / 1000000 ) / delta );
      printf( " Messages/sec = %f  (Message size=%d bytes)\n", ( gTotalMessages / delta ), size );
      printf( "\n" );
    }

}

