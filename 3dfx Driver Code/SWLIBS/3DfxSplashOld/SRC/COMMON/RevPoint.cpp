#if 0
#include "util.h"
#include "utilapi.h"
#endif
#include "RevPoint.h"


void RevPoint::Cross( const RevPoint& a, const RevPoint& b )
{
  static RevPoint tmp;
  RevPoint *result;

  if( &a == this || &b == this )
    {
      result = &tmp;
    }
  else
    {
      result = this;
    }

  result->data[0] = a.data[1] * b.data[2] -
    a.data[2] * b.data[1];
  result->data[1] = a.data[2] * b.data[0] -
    a.data[0] * b.data[2];
  result->data[2] = a.data[0] * b.data[1] -
    a.data[1] * b.data[0];

  if( result == &tmp )
    {
      memcpy( data, tmp.data, sizeof( RevFloat[3] ) );
    }
}

