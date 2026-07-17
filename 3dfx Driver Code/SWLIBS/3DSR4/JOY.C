#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <math.h>
#include "joy.h"
#include "pxp.h"
#include "kxp.h"

#include "dialog.h"
#include "joycal.3de"

static int minx, miny, maxx, maxy, centerx, centery;
static float dead_spot = 0;
static int joystick_calibrated = 0;

void joystick_calibrate( void )
{
  int x, y;
  unsigned char buttons;
  FILE *fp;
  int	status;

  init_dialog( Joycal1, NULL, NULL);
  ready_dialog( Joycal1, NULL, NULL, NULL, NULL, NULL, NULL );
  center_dialog( Joycal1 );
  save_under_dialog( Joycal1 );
  draw_dialog( Joycal1 );
  
  while( 1 ) {
    joystick_read_raw( &x, &y, &buttons );
    if( !( buttons & 0x10 ) )
      break;
  }

  joystick_read_raw( &centerx, &centery, &buttons );
    
  /*
   * Wait for the button to come back up.
   */
  while( 1 ) {
    joystick_read_raw( &x, &y, &buttons );
    if( buttons & 0x10 )
      break;
  }
  
  restore_under_dialog ();
  
  init_dialog( Joycal2, NULL, NULL);
  ready_dialog( Joycal2, NULL, NULL, NULL, NULL, NULL, NULL );
  center_dialog( Joycal2 );
  save_under_dialog( Joycal2 );
  draw_dialog( Joycal2 );
      
  joystick_read_raw( &x, &y, &buttons );
  minx = maxx = x;
  miny = maxy = y;

  while( 1 ) {
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
  }

  /*
   * Wait for the button to come back up.
   */
  while( 1 ) {
    joystick_read_raw( &x, &y, &buttons );
    if( buttons & 0x10 )
      break;
  }

  restore_under_dialog ();
  
  init_dialog( Joycal3, NULL, NULL);
  ready_dialog( Joycal3, NULL, NULL, NULL, NULL, NULL, NULL );
  center_dialog( Joycal3 );
  save_under_dialog( Joycal3 );
  draw_dialog( Joycal3 );
  
  while( 1 ) {
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
  if (fp) {
    fprintf( fp, "%u %u %u %u %u %u\n",
	     minx, miny, maxx, maxy, centerx, centery );
    fclose( fp );
  }

  restore_under_dialog ();
}


int joystick_detect(void)
{ 
  unsigned char k;
  int i = 0;
  FILE * fp = NULL;

  fp = fopen( "c:\\joy.dat", "r" );
  if( fp ) {
      fscanf( fp, "%u %u %u %u %u %u",
	      &minx, &miny, &maxx, &maxy, &centerx, &centery );
      fclose( fp );
      joystick_calibrated = 1;
  }

  outp(0x201, 0);
  k = inp(0x201);
  while ((k & 3) != 0 && i < 5000) {
    k = inp(0x201);  
    i++;
  }

  if (i == 5000)
    return 0;
  else {
    if (!joystick_calibrated) {
      return 2;
    }
    return 1;
  }
}

void joystick_read_raw( int *x, int *y, unsigned char *buttons )
{ 
  unsigned char k;
  unsigned short jx, jy;

  jx = 0; jy = 0;

  clearInterrupts();
  outp(0x201, 0);
  k = inp(0x201);
  while ((k & 3) != 0) {
    if (k & 1) jx++; 
    if (k & 2) jy++;
    k = inp(0x201);
  }
  restoreInterrupts();

  k = inp(0x201);

  *x = jx;
  *y = jy;
  *buttons = k;
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

  if( x < centerx ) {
    *ret_x = -( float )( x - centerx ) / ( minx - centerx );
  }
  else {
    *ret_x = ( float )( x - centerx ) / ( maxx - centerx );
  }
  if( y < centery ) {
    *ret_y = ( float )( y - centery ) / ( miny - centery );
  }
  else {
    *ret_y = -( float )( y - centery ) / ( maxy - centery );
  }

  /*
   * Make the center absolutely still.
   */
  if( fabs( *ret_x ) < dead_spot )
    *ret_x = 0;
  if( fabs( *ret_y ) < dead_spot )
    *ret_y = 0;

  *button1 = ( buttons & 0x10 ) ? 0 : 1;
  *button2 = ( buttons & 0x20 ) ? 0 : 1;
  *button3 = ( buttons & 0x40 ) ? 0 : 1;
  *button4 = ( buttons & 0x80 ) ? 0 : 1;
}
