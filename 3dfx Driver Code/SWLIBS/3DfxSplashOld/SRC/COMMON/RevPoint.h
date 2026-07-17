#ifndef __REVPOINT_H__
#define __REVPOINT_H__

#include <math.h>
#include <string.h>
#include "RevTypes.h"

class RevPoint
{
  friend class RevFace;

public:
        
  void AbsoluteValue( void )
  {
    if( data[0] < 0.0f )
      data[0] = -data[0];
    if( data[1] < 0.0f )
      data[1] = -data[1];
    if( data[2] < 0.0f )
      data[2] = -data[2];
  }

  void Cross( const RevPoint& a, const RevPoint& b );
  RevPoint() {}
  RevPoint( RevFloat x, RevFloat y, RevFloat z ) 
  { 
    data[0] = x; 
    data[1] = y; 
    data[2] = z; 
  }

  void Set( RevFloat x, RevFloat y, RevFloat z )
  {
    data[0] = x; 
    data[1] = y; 
    data[2] = z; 
  }

  void Set( int index, RevFloat val )
  {
    data[index] = val;
  }

  void SetX( RevFloat x )
  {
    data[0] = x;
  }

  void SetY( RevFloat y )
  {
    data[1] = y;
  }

  void SetZ( RevFloat z )
  {
    data[2] = z;
  }

  RevPoint const &operator=( RevPoint const &old ) 
  { 
    memcpy( data, old.data, sizeof( RevFloat[3] ) ); 
    return *this;
  }

  RevFloat GetX( void ) { return data[0]; }
  RevFloat GetY( void ) { return data[1]; }
  RevFloat GetZ( void ) { return data[2]; }

  RevFloat Get( int index ) { return data[index]; }

  RevFloat GetMagnitude( void ) 
  { 
    return ( RevFloat )sqrt( data[0] * data[0] + data[1] * data[1] + data[2] * data[2] ); 
  }

  void Normalize( void )
  {
    RevFloat oomag;
    oomag = 1.0f / GetMagnitude();
    data[0] *= oomag;
    data[1] *= oomag;
    data[2] *= oomag;
  }

  void Scale( RevFloat scale )
  {
    data[0] *= scale;
    data[1] *= scale;
    data[2] *= scale;
  }

  void Negate( void )
  {
    data[0] = -data[0];
    data[1] = -data[1];
    data[2] = -data[2];
  }

  void Subtract( const RevPoint& a, const RevPoint& b )
  {
    data[0] = a.data[0] - b.data[0];
    data[1] = a.data[1] - b.data[1];
    data[2] = a.data[2] - b.data[2];
  }

  void Add( const RevPoint& a, const RevPoint& b )
  {
    data[0] = a.data[0] + b.data[0];
    data[1] = a.data[1] + b.data[1];
    data[2] = a.data[2] + b.data[2];
  }

  void Add( const RevPoint& b )
  {
    data[0] += b.data[0];
    data[1] += b.data[1];
    data[2] += b.data[2];
  }

  RevFloat Dot( const RevPoint& other )
  {
    return 
      data[0] * other.data[0] +
      data[1] * other.data[1] +
      data[2] * other.data[2];
  }

  int GetMaxAxis( void )
  {
    if( data[0] > data[1] )
      {
        if( data[2] > data[0] )
          return 2;
        else
          return 0;
      }
    else
      {
        if( data[1] > data[2] )
          return 1;
        else
          return 2;
      }
  }

private:
  RevFloat data[3];
};

#endif /* __REVPOINT_H__ */
