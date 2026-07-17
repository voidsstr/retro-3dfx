#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <dos.h>
#include <string.h>

#include <atb_net.h>

int sending;

void   DoSend( void );
void   DoListen( void );

void main( int argc, const char *argv[] )
{
   if ( !NetOpen() )
   {
      exit( 1 );
   }

   if ( argc == 2 )
   {
      if ( strcmp( argv[1], "-s" ) == 0 )
         sending = 1;
   }

   if ( sending )
      DoSend();
   else
      DoListen();

   if ( kbhit() ) getch();

   NetClose();
}

void DoSend( void )
{
   char buffer[80];
   int  i = 0;

   while ( !kbhit() )
   {
      itoa( i++, buffer, 10 );
      if ( !NetSendPacket( buffer, sizeof( buffer ), NET_ID_BROADCAST ) )
      {
         printf( "DoSend() - %s\n", NetErrorString() );
         return;
      }
      printf( "s" );
      fflush( stdout );
      sleep( 1 );
   }
}

void DoListen( void )
{
   puts("Listening" );
   while ( !kbhit() )
   {
      char buffer[512];

      sleep( 1 );

      printf( "." );
      fflush( stdout );

      if ( NetPacketWaiting() )
      {
         NetGetPacket( buffer );
         printf( "%s\n", buffer );
      }
   }
}
