#include <stdlib.h>
#include <stdio.h>
#include "dump.h"

void DumpFrameBufferToPPM( int minx, int miny, int maxx, int maxy, const char *filename )
{
  volatile FxU16 *fb;
  FILE *fp;
  int x, y;
  unsigned char r, g, b;
  FxU16 col;

  fp = fopen( filename, "wb" );
  if( !fp )
    {
      fprintf( stderr, "Not able to open %s.\n", filename );
      exit( -1 );
    }

  fprintf( fp, "P6\n%d %d\n255\n", ( int )( maxx - minx + 1), ( int )( maxy - miny + 1 ) );
  fflush( fp );
  
  /*
   * Make sure that the SST is idle before reading from the framebuffer.
   */
  while( grSstIsBusy() )
    ;

  fb = ( FxU16 * )grLfbGetReadPtr( GR_BUFFER_FRONTBUFFER );
  for( y = maxy; y >= miny; y-- )
    {
      for( x = minx; x <= maxx; x++ )
        {
          col = fb[x + y * 1024];
          r = ( col & 0xf800 ) >> 8;
          g = ( col & 0x07e0 ) >> 3;
          b = ( col & 0x001f ) << 3;

          fwrite( &r, 1, 1, fp );
          fwrite( &g, 1, 1, fp );
          fwrite( &b, 1, 1, fp );
        }
    }
  fclose( fp );
}
