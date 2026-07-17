#include <stdlib.h>
#include <stdio.h>
#include "joy.h"

int main( void )
{
  float x, y;
  int b1, b2, b3, b4;

  if( !joystick_detect() )
    {
      fprintf( stderr, "No joystick detected.\n" );
      exit( -1 );
    }

  joystick_calibrate();

  while( 1 )
    {
      joystick_read( &x, &y, &b1, &b2, &b3, &b4 );
      fprintf( stderr, "%f %f %d %d %d %d\n", x, y, b1, b2, b3, b4 );
    }

  return 1;
}
