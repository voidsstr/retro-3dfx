#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <math.h>
#include <3dfx.h>
#include "joy.h"

FxBool BadVersion(void);

#ifdef __WATCOMC__
#   pragma aux clearInterrupts =\
            ".586" \
            "cli";

#   pragma aux restoreInterrupts =\
            ".586" \
            "sti";


#else  // Assume MSVC
#   define clearInterrupts() _asm { cli }
#   define restoreInterrupts() _asm { sti }
#endif

int minx, miny, maxx, maxy, centerx, centery;
float dead_spot = 0.0f;

#ifndef __WATCOMC__
int temp_outp( unsigned short port, int databyte )
{
  unsigned char b;

  b = ( unsigned char )databyte;

  _asm  mov     al, b
  _asm  mov     dx, port
  _asm  out     dx, al
  return databyte;
}

int temp_inp( unsigned short addr )
{
  char return_val;

  _asm  mov     dx, addr
  _asm  in      al, dx
  _asm  mov     return_val, al
  return ( int )return_val;
}

#define inp(X)       temp_inp(X)
#define outp(X,Y)    temp_outp(X,Y)
#endif


int joystick_detect(void)
{ 
  unsigned char k;
  int i = 0;

#ifdef __WIN32__
   if (BadVersion())
     return FXFALSE;
#endif

  outp(0x201, 0);
  k = inp(0x201);
  while ((k & 3) != 0 && i < 5000)
    {
      k = inp(0x201);  
      i++;
    }

  if (i == 5000)
    return 0;
  else 
    return 1;
}

void joystick_read_raw( int *x, int *y, unsigned char *buttons )
{ 
  unsigned char k;
  unsigned short jx, jy;

  jx = 0; jy = 0;

  clearInterrupts();
  outp(0x201, 0);
  k = inp(0x201);
  while ((k & 3) != 0)
    {
      if (k & 1) jx++; 
      if (k & 2) jy++;
      k = inp(0x201);
    }
    restoreInterrupts();
//  _asm  sti

  k = inp(0x201);

#if 0
  *b1 = (k&0x10)?0:1;
  *b2 = (k&0x20)?0:1;
#endif
  *x = jx;
  *y = jy;
  *buttons = k;
}


void joystick_calibrate( void )
{
  int x, y;
  unsigned char buttons;
  FILE *fp;

  fp = fopen( "c:\\joy.dat", "r" );
  if( fp )
    {
      fprintf( stderr, "Warning: reading joystick calibration data from \"c:\\joy.dat\".\n" );
      fprintf( stderr, "Remove this file if you wish to recalibrate.\n" );
      fscanf( fp, "%u %u %u %u %u %u", &minx, &miny, &maxx, &maxy, &centerx, &centery );
      fclose( fp );
      return;
    }

  fprintf( stderr, "Center joystick and hit fire button.\n" );
  
  while( 1 )
    {
      joystick_read_raw( &x, &y, &buttons );
      if( !( buttons & 0x10 ) )
        break;
    }

  joystick_read_raw( &centerx, &centery, &buttons );
    
  /*
   * Wait for the button to come back up.
   */
  while( 1 )
    {
      joystick_read_raw( &x, &y, &buttons );
      if( buttons & 0x10 )
        break;
    }

  fprintf( stderr, "Move joystick throughout its range and hit fire button.\n" );
    
  joystick_read_raw( &x, &y, &buttons );
  minx = maxx = x;
  miny = maxy = y;

  while( 1 )
    {
      joystick_read_raw( &x, &y, &buttons );
      if( !( buttons & 0x10 ) )
        break;
      if( x < minx )
        minx = x;
      if( x > maxx )
        maxx = x;
      if( y < miny )
        miny = y;
      if( y > maxy )
        maxy = y;
/*      fprintf( stderr, "%d %d %d %d\n", minx, miny, maxx, maxy ); */
    }

  /*
   * Wait for the button to come back up.
   */
  while( 1 )
    {
      joystick_read_raw( &x, &y, &buttons );
      if( buttons & 0x10 )
        break;
    }

  fprintf( stderr, "Center joystick again and hit fire.\n" );
  
  while( 1 )
    {
      joystick_read_raw( &x, &y, &buttons );
      if( !( buttons & 0x10 ) )
        break;
    }

  joystick_read_raw( &x, &y, &buttons );
    
  /*
   * Average the initial and the final sample of the center
   * position.
   */
  centerx = ( x + centerx ) / 2;
  centery = ( y + centery ) / 2;

  fp = fopen( "c:\\joy.dat", "w" );
  if( !fp )
    return;
  fprintf( fp, "%u %u %u %u %u %u\n", minx, miny, maxx, maxy, centerx, centery );
  fclose( fp );
}

void joystick_set_deadspot( float amount )
{
   dead_spot = amount;
}

void joystick_read( float *ret_x, float *ret_y, int *button1, int *button2, 
                    int *button3, int *button4 )
{
  int x, y;
  unsigned char buttons;

  joystick_read_raw( &x, &y, &buttons );

  if( x < centerx )
    {
      *ret_x = -( float )( x - centerx ) / ( minx - centerx );
    }
  else
    {
      *ret_x = ( float )( x - centerx ) / ( maxx - centerx );
    }
  if( y < centery )
    {
      *ret_y = ( float )( y - centery ) / ( miny - centery );
    }
  else
    {
      *ret_y = -( float )( y - centery ) / ( maxy - centery );
    }

  /*
   * Make the center absolutely still.
   */
  if( fabs( *ret_x ) < dead_spot )
    *ret_x = 0.0f;
  if( fabs( *ret_y ) < dead_spot )
    *ret_y = 0.0f;

  *button1 = ( buttons & 0x10 ) ? 0 : 1;
  *button2 = ( buttons & 0x20 ) ? 0 : 1;
  *button3 = ( buttons & 0x40 ) ? 0 : 1;
  *button4 = ( buttons & 0x80 ) ? 0 : 1;
}

#ifdef __DOS32__
FxBool BadVersion(void)
{
   return FXTRUE;
}
#endif

