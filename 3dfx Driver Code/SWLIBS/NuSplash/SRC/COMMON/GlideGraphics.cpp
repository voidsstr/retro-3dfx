#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "GlideGraphics.h"
#include "RevPoint.h"
#include <math.h>
#include <assert.h>
#define ASSERT assert

#include <stdio.h>

#define GLIDE_ERROR(a) ASSERT( 0 )

#ifndef M_PI
#define M_PI 3.14159f
#endif

#define SNAP_BIAS ( ( float )( 3L << 18 ) )

#define DEG2RAD(a) ( a * ( M_PI / 180.0f ) )

#define NEAR_Z ( 1.0f )


static inline RevU32 ConvertColor16to24( const RevU16& in )
{
  return ( ( ( RevU32 )in & 0xf800 ) << 8 ) |
    ( ( ( RevU32 )in & 0x07e0 ) << 5 ) |
    ( ( ( RevU32 )in & 0x001f ) << 3 );
}

static inline void MakeIdentMatrix( RevGlideMatrix m )
{
#define M(row,col)  m[col*4+row]
  M(0,0) = 1.0f; M(0,1) = 0.0f; M(0,2) = 0.0f; M(0,3) = 0.0f;
  M(1,0) = 0.0f; M(1,1) = 1.0f; M(1,2) = 0.0f; M(1,3) = 0.0f;
  M(2,0) = 0.0f; M(2,1) = 0.0f; M(2,2) = 1.0f; M(2,3) = 0.0f;
  M(3,0) = 0.0f; M(3,1) = 0.0f; M(3,2) = 0.0f; M(3,3) = 1.0f;
#undef M
}

void GlideGraphics::InitMatrixStacks( void )
{
  int i;

  for( i = 0; i < 3; i++ )
    {
      matrix_stack[i].top_of_stack = 0;
      MakeIdentMatrix( matrix_stack[i].mat[0] );
    }
  current_matrix_stack = &matrix_stack[GLIDE_MODELVIEW];
}

GlideGraphics::GlideGraphics()
{
  InitMatrixStacks();
  RebuildCompositeMatrix();
}

GlideGraphics::~GlideGraphics()
{
}

RevBool GlideGraphics::Open( void *hwnd_ /*HWND hwnd_*/ )
{
  hwnd = ( HWND )hwnd_;
  
#ifndef SPLASH_DLL
  grGlideInit();
  grSstQueryHardware( &hwconfig );
  grSstSelect( 0 );
#endif
  
  RECT  rect;
  GetClientRect( hwnd, &rect );
  device_width  = ( rect.right - rect.left + 1 );
  device_height = ( rect.bottom - rect.top + 1 );
  
  fullscreen = REVFALSE;
  /*
   * Run in a window if you can, otherwise run fullscreen.
   */
#ifndef SPLASH_DLL
#if 1
  if( !grSstWinOpen( ( FxU32 )hwnd, GR_RESOLUTION_NONE,
                     GR_REFRESH_60Hz,
                     GR_COLORFORMAT_ARGB,
                     GR_ORIGIN_LOWER_LEFT,
                     2, 1 ) )
#endif
    {
      grSstWinOpen( ( FxU32 )hwnd, GR_RESOLUTION_640x480,
                    GR_REFRESH_60Hz,
                    GR_COLORFORMAT_ARGB,
                    GR_ORIGIN_LOWER_LEFT,
                    2, 1 );
      fullscreen = REVTRUE;
      device_width = 640;
      device_height = 480;
    }
#endif

  grClipWindow( 0, 0, ( int )device_width, ( int )device_height );
  grBufferClear( 0x0000002F, 0, 0 );
  grBufferSwap( 1 );
  grBufferClear( 0x0000002F, 0, 0 );
  grDepthBufferMode( GR_DEPTHBUFFER_WBUFFER );
  grDepthBufferFunction( GR_CMP_LEQUAL );  
#if 0
  tex_min_address = grTexMinAddress( GR_TMU0 );
  tex_max_address = grTexMaxAddress( GR_TMU0 );
  CreateTexelIDTexture();
  ResetFaceIDs();
#endif
  grTexLodBiasValue( GR_TMU0, GR_LODBIAS_BILINEAR );
#if 0
  FlushLightmaps();
#endif
  grTexFilterMode( GR_TMU0, GR_TEXTUREFILTER_BILINEAR, GR_TEXTUREFILTER_BILINEAR );
  // grHints( GR_HINT_ALLOW_MIPMAP_DITHER, FXTRUE );
  // grTexMipMapMode( GR_TMU0, GR_MIPMAP_NEAREST_DITHER, FXFALSE );
  grTexMipMapMode( GR_TMU0, GR_MIPMAP_NEAREST, FXFALSE );
#if 0
  InitBaseTextures();
#endif

  //  grCullMode( GR_CULL_NEGATIVE );
  //grCullMode( GR_CULL_POSITIVE );
  return REVTRUE;
}

RevBool GlideGraphics::Close( void )
{
  grSstWinClose();
  grGlideShutdown();
  return REVTRUE;
}

void GlideGraphics::Swap( int swapinterval )
{
  grBufferSwap( swapinterval );
}

void GlideGraphics::Clear( RevU32 flags, float r, float g, float b )
{
  FxU32 color;
  FxU16 depth;
  
  if( flags & GLIDE_GRAPHICS_CLEAR_COLOR )
    {
      color = 
        ( ( int )( r * 255.0f ) << 16 ) |
        ( ( int )( g * 255.0f ) << 8 ) |
        ( ( int )( b * 255.0f ) << 0 );
    }
  else
    grColorMask( FXFALSE, FXFALSE );
  
  if( flags & GLIDE_GRAPHICS_CLEAR_DEPTH )
    depth = GR_WDEPTHVALUE_FARTHEST;
  else
    grDepthMask( FXFALSE );
  
  grBufferClear( color, 0x0, depth );
  grDepthMask( FXTRUE );
  grColorMask( FXTRUE, FXFALSE );
}

void GlideGraphics::Clear( RevU32 flags, RevU16 clear_color )
{
  FxU32 color;
  FxU16 depth;
  
  if( flags & GLIDE_GRAPHICS_CLEAR_COLOR )
    {
      color = ConvertColor16to24( clear_color );
    }
  else
    grColorMask( FXFALSE, FXFALSE );
  
  if( flags & GLIDE_GRAPHICS_CLEAR_DEPTH )
    depth = GR_WDEPTHVALUE_FARTHEST;
  else
    grDepthMask( FXFALSE );
  
  grBufferClear( color, 0x0, depth );
  grDepthMask( FXTRUE );
  grColorMask( FXTRUE, FXFALSE );
}

void GlideGraphics::Resize( int width, int height )
{
  device_width = viewport_width = width;
  device_height = viewport_height = height;
  grSstControl( GR_CONTROL_RESIZE );
  grClipWindow( 0, 0, 
                device_width, device_height );
}

void GlideGraphics::Resize( void )
{
  RECT rect;
  
  GetClientRect( hwnd, &rect );
  if( !fullscreen )
    {
      device_width = rect.right - rect.left + 1;
      device_height = rect.bottom - rect.top + 1;
    }

#if 0
  viewport_width = rect.right - rect.left + 1;
  viewport_height = rect.bottom - rect.top + 1;
#endif

  // hack
  viewport_width = 640;
  viewport_height = 480;
  
  if( !fullscreen )
    grSstControl( GR_CONTROL_RESIZE );
  grClipWindow( 0, 0, 
                device_width, device_height );
}

void GlideGraphics::Move( void )
{
  if( !fullscreen )
    grSstControl( GR_CONTROL_MOVE );
}

//void GlideGraphics::DrawLine( RevPoint *p1, RevPoint *p2, RevU32 color )
void GlideGraphics::DrawLine( RevVert *p1, RevVert *p2, RevU32 color )
{
  GrVertex a, b;
  RevFloat *modelview_mat = matrix_stack[GLIDE_MODELVIEW].mat[matrix_stack[GLIDE_MODELVIEW].top_of_stack];
  
  /*
   * Transform from world space to view space.
   */
#define M(row,col)  modelview_mat[col*4+row]
#define P(offset)   ( p1->pos.Get( offset ) )
  a.x = M(0,0) * P(0) + M(0,1) * P(1) + M(0,2) * P(2) + M(0,3);
  a.y = M(1,0) * P(0) + M(1,1) * P(1) + M(1,2) * P(2) + M(1,3);
  a.z = M(2,0) * P(0) + M(2,1) * P(1) + M(2,2) * P(2) + M(2,3);
  a.z = -a.z;
#undef P

#define P(offset)   ( p2->pos.Get( offset ) )
  b.x = M(0,0) * P(0) + M(0,1) * P(1) + M(0,2) * P(2) + M(0,3);
  b.y = M(1,0) * P(0) + M(1,1) * P(1) + M(1,2) * P(2) + M(1,3);
  b.z = M(2,0) * P(0) + M(2,1) * P(1) + M(2,2) * P(2) + M(2,3);
  b.z = -b.z;
#undef M
#undef P

  if( a.z < NEAR_Z || b.z < NEAR_Z )
    return;

  /*
   * project
   */
  a.oow = 1.0f / a.z;
  a.tmuvtx[0].sow = p1->texvert.Get( 0 ) * a.oow;
  a.tmuvtx[0].tow = p1->texvert.Get( 1 ) * a.oow;
  a.x = a.x * a.oow;
  a.y = a.y * a.oow;
  b.oow = 1.0f / b.z;
  b.tmuvtx[0].sow = p2->texvert.Get( 0 ) * b.oow;
  b.tmuvtx[0].tow = p2->texvert.Get( 1 ) * b.oow;
  b.x = b.x * b.oow;
  b.y = b.y * b.oow;
  
  /* 
   * viewport transform.
   */
  a.x = a.x * x_scale + x_offset;
  a.y = a.y * y_scale + y_offset;
  a.x -= SNAP_BIAS;
  a.y -= SNAP_BIAS;

  b.x = b.x * x_scale + x_offset;
  b.y = b.y * y_scale + y_offset;
  b.x -= SNAP_BIAS;
  b.y -= SNAP_BIAS;

  RevFloat t;

  if( a.x < 0.0f && b.x < 0.0f )
    return;
  if( a.x > viewport_width && b.x > viewport_width )
    return;
  if( a.y < 0.0f && b.y < 0.0f )
    return;
  if( a.y > viewport_height && b.y > viewport_height )
    return;

  /* clip left. */
  if( a.x < 0.0f )
    {
      t = ( 0.0f - a.x ) / ( b.x - a.x );
      a.x = 0.0f;
      a.y = a.y + ( b.y - a.y ) * t;
      a.oow = a.oow + ( b.oow - a.oow ) * t;
      a.tmuvtx[0].sow = a.tmuvtx[0].sow + ( b.tmuvtx[0].sow - a.tmuvtx[0].sow ) * t;
      a.tmuvtx[0].tow = a.tmuvtx[0].tow + ( b.tmuvtx[0].tow - a.tmuvtx[0].tow ) * t;
    }

  if( b.x < 0.0f )
    {
      t = ( 0.0f - b.x ) / ( a.x - b.x );
      b.x = 0.0f;
      b.y = b.y + ( a.y - b.y ) * t;
      b.oow = b.oow + ( a.oow - b.oow ) * t;
      b.tmuvtx[0].sow = b.tmuvtx[0].sow + ( a.tmuvtx[0].sow - b.tmuvtx[0].sow ) * t;
      b.tmuvtx[0].tow = b.tmuvtx[0].tow + ( a.tmuvtx[0].tow - b.tmuvtx[0].tow ) * t;
    }

  /* clip bottom. */
  if( a.y < 0.0f )
    {
      t = ( 0.0f - a.y ) / ( b.y - a.y );
      a.y = 0.0f;
      a.x = a.x + ( b.x - a.x ) * t;
      a.oow = a.oow + ( b.oow - a.oow ) * t;
      a.tmuvtx[0].sow = a.tmuvtx[0].sow + ( b.tmuvtx[0].sow - a.tmuvtx[0].sow ) * t;
      a.tmuvtx[0].tow = a.tmuvtx[0].tow + ( b.tmuvtx[0].tow - a.tmuvtx[0].tow ) * t;
    }

  if( b.y < 0.0f )
    {
      t = ( 0.0f - b.y ) / ( a.y - b.y );
      b.y = 0.0f;
      b.x = b.x + ( a.x - b.x ) * t;
      b.oow = b.oow + ( a.oow - b.oow ) * t;
      b.tmuvtx[0].sow = b.tmuvtx[0].sow + ( a.tmuvtx[0].sow - b.tmuvtx[0].sow ) * t;
      b.tmuvtx[0].tow = b.tmuvtx[0].tow + ( a.tmuvtx[0].tow - b.tmuvtx[0].tow ) * t;
    }

  /*
   * clip right.
   */
  if( b.x > viewport_width )
    {
      t = ( viewport_width - a.x ) / ( b.x - a.x );
      b.x = ( float )viewport_width;
      b.y = a.y + ( b.y - a.y ) * t;
      b.oow = a.oow + ( b.oow - a.oow ) * t;
      b.tmuvtx[0].sow = a.tmuvtx[0].sow + ( b.tmuvtx[0].sow - a.tmuvtx[0].sow ) * t;
      b.tmuvtx[0].tow = a.tmuvtx[0].tow + ( b.tmuvtx[0].tow - a.tmuvtx[0].tow ) * t;
    }

  if( a.x > viewport_width )
    {
      t = ( viewport_width - b.x ) / ( a.x - b.x );
      a.x = ( float )viewport_width;
      a.y = b.y + ( a.y - b.y ) * t;
      a.oow = b.oow + ( a.oow - b.oow ) * t;
      a.tmuvtx[0].sow = b.tmuvtx[0].sow + ( a.tmuvtx[0].sow - b.tmuvtx[0].sow ) * t;
      a.tmuvtx[0].tow = b.tmuvtx[0].tow + ( a.tmuvtx[0].tow - b.tmuvtx[0].tow ) * t;
    }

  /*
   * clip top.
   */
  if( b.y > viewport_height )
    {
      t = ( viewport_height - a.y ) / ( b.y - a.y );
      b.y = ( float )viewport_height;
      b.x = a.x + ( b.x - a.x ) * t;
      b.oow = a.oow + ( b.oow - a.oow ) * t;
      b.tmuvtx[0].sow = a.tmuvtx[0].sow + ( b.tmuvtx[0].sow - a.tmuvtx[0].sow ) * t;
      b.tmuvtx[0].tow = a.tmuvtx[0].tow + ( b.tmuvtx[0].tow - a.tmuvtx[0].tow ) * t;
    }

  if( a.y > viewport_height )
    {
      t = ( viewport_height - b.y ) / ( a.y - b.y );
      a.y = ( float )viewport_height;
      a.x = b.x + ( a.x - b.x ) * t;
      a.oow = b.oow + ( a.oow - b.oow ) * t;
      a.tmuvtx[0].sow = b.tmuvtx[0].sow + ( a.tmuvtx[0].sow - b.tmuvtx[0].sow ) * t;
      a.tmuvtx[0].tow = b.tmuvtx[0].tow + ( a.tmuvtx[0].tow - b.tmuvtx[0].tow ) * t;
    }

  // hack
  grConstantColorValue( color );
  //  guColorCombineFunction( GR_COLORCOMBINE_CCRGB );

  a.x += SNAP_BIAS;
  a.x -= SNAP_BIAS;

  a.y += SNAP_BIAS;
  a.y -= SNAP_BIAS;

  b.x += SNAP_BIAS;
  b.x -= SNAP_BIAS;

  b.y += SNAP_BIAS;
  b.y -= SNAP_BIAS;


#if 1
  if( a.x < -1.0f || a.x > viewport_width + 1 || a.y < -1.0f || a.y > viewport_height + 1 ||
      b.x < -1.0f || b.x > viewport_width + 1 || b.y < -1.0f || b.y > viewport_height + 1 )
    {
      //      TRACE( "viewport: %f %f\n", ( float )viewport_width, ( float )viewport_height );
      //      ASSERT( 0 );
      return;
    }
#endif

#if 0
  // hack
  a.tmuvtx[0].sow = 0.0f;
  a.tmuvtx[0].tow = 0.0f;
  a.oow = 1.0f;

  b.tmuvtx[0].sow = 255.0f;
  b.tmuvtx[0].tow = 255.0f;
  b.oow = 1.0f;
#endif

  grDrawLine( &a, &b );
}

#if 0
void GlideGraphics::DrawCube( RevFloat len, RevU16 color )
{
  RevVert view_vert;
  GrVertex gvert;
  RevFloat *modelview_mat = matrix_stack[GLIDE_MODELVIEW].mat[matrix_stack[GLIDE_MODELVIEW].top_of_stack];

#define M(row,col)  modelview_mat[col*4+row]
  view_vert.SetX( M(0,3) );
  view_vert.SetY( M(1,3) );
  view_vert.SetZ( -M(2,3) );
#undef M
  if( view_vert.GetZ() < NEAR_Z )
    return;
  gvert.oow = 1.0f / view_vert.GetZ();
  gvert.x = view_vert.GetX() * gvert.oow;
  gvert.y = view_vert.GetY() * gvert.oow;
  gvert.x = gvert.x * x_scale + x_offset;
  gvert.y = gvert.y * y_scale + y_offset;
  gvert.x -= SNAP_BIAS;
  gvert.y -= SNAP_BIAS;
  if( gvert.x < 0.0f || gvert.y < 0.0f ||
      gvert.x > device_width || gvert.y > device_height )
    return;
  grConstantColorValue( ConvertColor16to24( color ) );
  guColorCombineFunction( GR_COLORCOMBINE_CCRGB );
  grDitherMode( GR_DITHER_DISABLE );

  int x, y;

  RevFloat oldx, oldy;
  oldx = gvert.x;
  oldy = gvert.y;

  for( x = -4; x <= 4; x++ )
    for( y = -4; y <= 4; y++ )
      {
        gvert.x = oldx + x;
        gvert.y = oldy + y;
        grDrawPoint( &gvert );
      }
  grDitherMode( GR_DITHER_2x2 );
}
#endif

void GlideGraphics::MatrixMode( RevU32 mode )
{
  current_matrix_stack = &matrix_stack[mode];
}

void GlideGraphics::LoadIdentity( void )
{
  MakeIdentMatrix( current_matrix_stack->mat[current_matrix_stack->top_of_stack] );
  RebuildCompositeMatrix();
}

void GlideGraphics::Perspective( RevFloat fovy, RevFloat aspect, RevFloat zNear,
                                    RevFloat zFar )
{
  RevFloat xmin, xmax, ymin, ymax;
  
  ymax = zNear * ( float )tan( fovy * M_PI / 360.0 );
  ymin = -ymax;
  
  xmin = ymin * aspect;
  xmax = ymax * aspect;
  
  Frustum( xmin, xmax, ymin, ymax, zNear, zFar );
}

void GlideGraphics::Frustum( RevFloat left, RevFloat right, 
                                RevFloat bottom, RevFloat top, 
                                RevFloat nearval, RevFloat farval )
{
  RevFloat x, y, a, b, c, d;
  RevGlideMatrix m;

  if( nearval <= 0.0f || farval <= 0.0f ) 
    {
#if 0
      GLIDE_ERROR(( "GlideGraphics::Frustum",
                  "nearval/farval out of range" ));
#endif
      ASSERT( 0 );
    }

   x = ( 2.0f * nearval ) / ( right - left );
   y = ( 2.0f * nearval ) / ( top - bottom );
   a = ( right + left ) / ( right - left );
   b = ( top + bottom ) / ( top - bottom );
   c = -( farval + nearval ) / ( farval - nearval );
   d = -( 2.0f * farval * nearval ) / ( farval-nearval );

#define M(row,col)  m[col*4+row]
   M(0,0) = x;     M(0,1) = 0.0F;  M(0,2) = a;      M(0,3) = 0.0F;
   M(1,0) = 0.0F;  M(1,1) = y;     M(1,2) = b;      M(1,3) = 0.0F;
   M(2,0) = 0.0F;  M(2,1) = 0.0F;  M(2,2) = c;      M(2,3) = d;
   M(3,0) = 0.0F;  M(3,1) = 0.0F;  M(3,2) = -1.0F;  M(3,3) = 0.0F;
#undef M

   MultMatrix( m );
}

/*
 * Hack: This is hacked to force a 90 fov in x since the Frustum/ortho stuff
 * isn't used yet in the pipeline.
 */
void GlideGraphics::Viewport( RevI32 x, RevI32 y, RevI32 width, RevI32 height )
{
  viewport_x = x;
  viewport_y = y;
  viewport_width = width;
  viewport_height = height;
#if 0
  x_scale = width * 0.5f;
  x_offset = ( x + width * 0.5f ) + SNAP_BIAS;
  //  y_scale = height * 0.5f;
  y_scale = width * 0.5f;
  y_offset = ( y + height * 0.5f ) + SNAP_BIAS;
#endif
  RecalcFOV();
  grClipWindow( x, y, x + width, y + height );
}

void GlideGraphics::Ortho( RevFloat left, RevFloat right, RevFloat bottom, RevFloat top, 
                              RevFloat zNear, RevFloat zFar )
{
  RevGlideMatrix m;
  RevFloat tx, ty, tz;

  tx = -( right + left ) / ( right - left );
  ty = -( top + bottom ) / ( top - bottom );
  tz = -( zFar + zNear ) / ( zFar - zNear );

#define M(row,col)  m[col*4+row]
   M(0,0) = 2.0f / ( right - left ); M(0,1) = 0.0F;                    M(0,2) = 0.0f;                    M(0,3) = tx;
   M(1,0) = 0.0F;                    M(1,1) = 2.0f / ( top - bottom ); M(1,2) = 0.0f;                    M(1,3) = ty;
   M(2,0) = 0.0F;                    M(2,1) = 0.0F;                    M(2,2) = 2.0f / ( zFar - zNear ); M(2,3) = tz;
   M(3,0) = 0.0F;                    M(3,1) = 0.0F;                    M(3,2) = 0.0F;                    M(3,3) = 1.0F;
#undef M

   MultMatrix( m );
}

void GlideGraphics::Translate( RevFloat x, RevFloat y, RevFloat z )
{
  RevGlideMatrix m;

  /*
   * Gee, this could be a little faster.
   */
#define M(row,col)  m[col*4+row]
  M(0,0) = 1.0f; M(0,1) = 0.0f; M(0,2) = 0.0f; M(0,3) = x;
  M(1,0) = 0.0f; M(1,1) = 1.0f; M(1,2) = 0.0f; M(1,3) = y;
  M(2,0) = 0.0f; M(2,1) = 0.0f; M(2,2) = 1.0f; M(2,3) = z;
  M(3,0) = 0.0f; M(3,1) = 0.0f; M(3,2) = 0.0f; M(3,3) = 1.0f;
#undef M
  MultMatrix( m );
}

void GlideGraphics::Rotate( RevFloat angle, RevFloat x, RevFloat y, RevFloat z )
{
  RevGlideMatrix m;
  RevFloat rad;
  RevFloat s, c, omc;

  if( angle == 0.0f )
    return;

  rad = DEG2RAD( angle );
  s = ( float )sin( rad );
  c = ( float )cos( rad );
  omc = 1.0f - c;

#define M(row,col)  m[col*4+row]
  M(0,0) = x*x*omc+c;   M(0,1) = x*y*omc-z*s; M(0,2) = x*z*omc+y*s; M(0,3) = 0.0f;
  M(1,0) = y*x*omc+z*s; M(1,1) = y*y*omc+c;   M(1,2) = y*z*omc-x*s; M(1,3) = 0.0f;
  M(2,0) = x*z*omc-y*s; M(2,1) = y*z*omc+x*s; M(2,2) = z*z*omc+c;   M(2,3) = 0.0f;
  M(3,0) = 0.0f;        M(3,1) = 0.0f;        M(3,2) = 0.0f;        M(3,3) = 1.0f;
#undef M
  MultMatrix( m );
}

void GlideGraphics::PushMatrix( void )
{
  if( ( current_matrix_stack->top_of_stack++ ) > GLIDE_GLIDE_MATRIX_STACK_DEPTH )
    {
      GLIDE_ERROR(( "GlideGraphics::PushMatrix",
                 "Stack overflow" ));
    }

  memcpy( current_matrix_stack->mat[current_matrix_stack->top_of_stack],
          current_matrix_stack->mat[current_matrix_stack->top_of_stack-1],          
          sizeof( RevFloat[16] ) );
}

void GlideGraphics::PopMatrix( void )
{
  if( ( current_matrix_stack->top_of_stack-- ) < 0 )
    {
      GLIDE_ERROR(( "GlideGraphics::PopMatrix",
                 "Stack underflow" ));
    }
  RebuildCompositeMatrix();
}

void GlideGraphics::MultMatrix( const RevFloat *m )
{
  RevGlideMatrix tmp;
  RevFloat *left;
  int r, c;

  left = current_matrix_stack->mat[current_matrix_stack->top_of_stack];

#define L(row,col)  m[col*4+row]
#define R(row,col)  left[col*4+row]
#define T(row,col)  tmp[col*4+row]

  for( r = 0; r < 4; r++ ) 
    {
      for( c = 0; c < 4; c++ ) 
        {
          T(r,c) = 
            L(0,c) * R(r,0) +
            L(1,c) * R(r,1) +
            L(2,c) * R(r,2) +
            L(3,c) * R(r,3);
        }
    }
#undef R
#undef L
#undef T

  memcpy( left, tmp, sizeof( RevGlideMatrix ) );
  RebuildCompositeMatrix();
}


void GlideGraphics::GetFloatv( RevU32 pname, RevFloat *params )
{
  switch( pname )
    {
    case GLIDE_MODELVIEW_MATRIX:
      memcpy( params, 
              matrix_stack[GLIDE_MODELVIEW].mat[matrix_stack[GLIDE_MODELVIEW].top_of_stack], 
              sizeof( RevFloat[16] ) );
      break;
    case GLIDE_DEVICE_WIDTH:
      *params = ( float )device_width;
      break;
    case GLIDE_DEVICE_HEIGHT:
      *params = ( float )device_height;
      break;
    default:
      GLIDE_ERROR(( "GlideGraphics::GetFloatv",
                  "unknown pname" ));
      break;
    }
}

void GlideGraphics::MatrixMultiply( RevFloat *dest, const RevFloat *left, RevFloat *right )
{
  RevGlideMatrix tmp;
  int r, c;

  // Hack: this seems backwards.

#define L(row,col)  right[col*4+row]
#define R(row,col)  left[col*4+row]
#define T(row,col)  tmp[col*4+row]

  for( r = 0; r < 4; r++ ) 
    {
      for( c = 0; c < 4; c++ ) 
        {
          T(r,c) = 
            L(0,c) * R(r,0) +
            L(1,c) * R(r,1) +
            L(2,c) * R(r,2) +
            L(3,c) * R(r,3);
        }
    }
#undef R
#undef L
#undef T

  memcpy( dest, tmp, sizeof( RevGlideMatrix ) );
}

void GlideGraphics::RebuildCompositeMatrix( void )
{
  /*
   * composite_matrix = projection * modelview
   * (composite_matrix = projection * worldview * modelview)
   */
  MatrixMultiply( composite_matrix, 
                  matrix_stack[GLIDE_PROJECTION].mat[matrix_stack[GLIDE_PROJECTION].top_of_stack],
                  matrix_stack[GLIDE_MODELVIEW].mat[matrix_stack[GLIDE_MODELVIEW].top_of_stack] );
}

void GlideGraphics::RenderBuffer( RevU32 flags )
{
  if( flags & GLIDE_BUFFER_FRONT )
    grRenderBuffer( GR_BUFFER_FRONTBUFFER );
  if( flags & GLIDE_BUFFER_BACK )
    grRenderBuffer( GR_BUFFER_BACKBUFFER );
}


#define GLIDE_MAX_NUM_FACE_VERTS 100
#define SIDE_OUT 0
#define SIDE_IN  1

#if 0
void GlideGraphics::DrawMaxMesh( Mesh *mesh )
{
  /*
   * Get info about the mesh.
   */
  int num_faces = mesh->getNumFaces();
  int num_verts = mesh->getNumVerts();
  int num_tex_verts = mesh->getNumTVerts();
  Point3 *model_verts = mesh->getVertPtr( 0 );
  Point3 *tex_verts = mesh->getTVertPtr( 0 );
  Face *faces = mesh->faces;
  Face *end_faces = mesh->faces + num_faces;

  /*
   * Get info about the current transform.
   */
  RevFloat *modelview_mat = matrix_stack[GLIDE_MODELVIEW].mat[matrix_stack[GLIDE_MODELVIEW].top_of_stack];
  static GrVertex gverts[GLIDE_MAX_NUM_FACE_VERTS];

  if( !model_verts )
    return;

  if( tex_verts )
    {
      ASSERT( num_tex_verts >= num_verts );
    }

  Face *face;
  for( face = faces; face < end_faces; face++ )
    {
      int vertnum;
      int num_verts = 3; // always triangles with Max.
      static RevVert unclipped_verts[GLIDE_MAX_NUM_FACE_VERTS];
      static RevVert clipped_verts[GLIDE_MAX_NUM_FACE_VERTS];
      static int sides[GLIDE_MAX_NUM_FACE_VERTS];
      int counts[2];

      counts[SIDE_OUT] = counts[SIDE_IN] = 0;

      for( vertnum = 0; vertnum < num_verts; vertnum++ )
        {
          Point3 *model_vert = &model_verts[face->getVert( vertnum )];
          Point3 *tex_vert = ( tex_verts ) ? &tex_verts[face->getVert( vertnum )] : NULL;
          GrVertex *gvert = &gverts[vertnum % num_verts]; // Why the %??
          RevVert *unclipped_vert = &unclipped_verts[vertnum];

          /*
           * Transform from world space to view space.
           */
#define M(row,col)  modelview_mat[col*4+row]
#define P(offset)   ( (*model_vert)[offset] )
          unclipped_vert->pos.Set( model_vert->x, model_vert->y, model_vert->z );
          if( tex_verts )
            unclipped_vert->texvert.Set( smult * tex_vert->x, 
                                         tmult * tex_vert->y, 
                                         0.0f );
          unclipped_vert->pos.SetX( M(0,0) * P(0) + M(0,1) * P(1) + M(0,2) * P(2) + M(0,3) );
          unclipped_vert->pos.SetY( M(1,0) * P(0) + M(1,1) * P(1) + M(1,2) * P(2) + M(1,3) );
          unclipped_vert->pos.SetZ( M(2,0) * P(0) + M(2,1) * P(1) + M(2,2) * P(2) + M(2,3) );
          unclipped_vert->pos.SetZ( -unclipped_vert->pos.GetZ() );
#undef M
#undef P

          if( unclipped_vert->GetZ() < NEAR_Z )
            {
              sides[vertnum] = SIDE_OUT;
              counts[SIDE_OUT]++;
            }
          else
            {
              sides[vertnum] = SIDE_IN;
              counts[SIDE_IN]++;
            }
        }

      sides[vertnum] = sides[0];

      /*
       * Trivial reject.
       */
#if 0
      if( !counts[SIDE_IN] )
        {
          return;
        }
#endif

      int clipped_vertnum;
      int unclipped_vertnum;
      
      clipped_vertnum = 0;
      for( unclipped_vertnum = 0; 
           unclipped_vertnum < num_verts; 
           unclipped_vertnum++ )
        {
          /*
           * If the current vert is on the inside, the
           * store it.
           */
          if( sides[unclipped_vertnum] == SIDE_IN )
            { 
              clipped_verts[clipped_vertnum] = 
                unclipped_verts[unclipped_vertnum];
              clipped_vertnum++;
            }

          /*
           * If the next vert is on the surface, or if the next
           * vert is on the same side as the current vert, then
           * continue with the next iteration.
           */
          if( sides[unclipped_vertnum + 1] == sides[unclipped_vertnum] )
            {
              continue;
            }

          RevVert *p1, *p2, *dest;
          RevFloat t;

          /*
           * Grab the next input vert so that we can create a split
           * point.
           */
          p1 = &unclipped_verts[unclipped_vertnum];
          p2 = &unclipped_verts[( unclipped_vertnum + 1 ) % num_verts];

          if( p2->GetZ() - p1->GetZ() == 0.0f )
            {
#if 0
              GLIDE_WARNING(( "GlideGraphics::DrawLightmaps",
                            "Potential divide by zero\n" ));
#endif
              return;
            }
          
          dest = &clipped_verts[clipped_vertnum];
          t = ( NEAR_Z - p1->GetZ() ) / ( p2->GetZ() - p1->GetZ() );
          dest->SetX( p1->GetX() + ( p2->GetX() - p1->GetX() ) * t );
          dest->SetY( p1->GetY() + ( p2->GetY() - p1->GetY() ) * t );
          dest->SetZ( p1->GetZ() + ( p2->GetZ() - p1->GetZ() ) * t );
          dest->SetU( p1->GetU() + ( p2->GetU() - p1->GetU() ) * t );
          dest->SetV( p1->GetV() + ( p2->GetV() - p1->GetV() ) * t );
          clipped_vertnum++;
        }
      

      for( vertnum = 0; vertnum < clipped_vertnum; vertnum++ )
        {
          GrVertex *gvert = &gverts[vertnum];
          RevVert *clipped_vert = &clipped_verts[vertnum];

          gvert->oow = 1.0f / clipped_vert->GetZ();
          gvert->x = clipped_vert->GetX() * gvert->oow;
          gvert->y = clipped_vert->GetY() * gvert->oow;
          gvert->tmuvtx[0].sow = clipped_vert->GetU() * gvert->oow;
          gvert->tmuvtx[0].tow = clipped_vert->GetV() * gvert->oow;

          gvert->x = gvert->x * x_scale + x_offset;
          gvert->y = gvert->y * y_scale + y_offset;
          gvert->x -= SNAP_BIAS;
          gvert->y -= SNAP_BIAS;
        }
      
      guDrawPolygonVertexListWithClip( clipped_vertnum, gverts );
    }
}
#endif

void GlideGraphics::DrawPolygon( int num_verts, RevVert *verts )
{
  /*
   * Get info about the current transform.
   */
  RevFloat *modelview_mat = matrix_stack[GLIDE_MODELVIEW].mat[matrix_stack[GLIDE_MODELVIEW].top_of_stack];
  static GrVertex gverts[GLIDE_MAX_NUM_FACE_VERTS];

  int vertnum;
  static RevVert unclipped_verts[GLIDE_MAX_NUM_FACE_VERTS];
  static RevVert clipped_verts[GLIDE_MAX_NUM_FACE_VERTS];
  static int sides[GLIDE_MAX_NUM_FACE_VERTS];
  int counts[2];

  counts[SIDE_OUT] = counts[SIDE_IN] = 0;

  for( vertnum = 0; vertnum < num_verts; vertnum++ )
    {
      RevVert *model_vert = &verts[vertnum];
      GrVertex *gvert = &gverts[vertnum % num_verts]; // Why the %??
      RevVert *unclipped_vert = &unclipped_verts[vertnum];

      /*
       * Transform from object space to view space.
       */
#define M(row,col)  modelview_mat[col*4+row]
#define P(offset)   ( model_vert->pos.Get( offset ) )
      //      *unclipped_vert = verts[vertnum];
      unclipped_vert->texvert.Set( verts[vertnum].texvert.GetX() * smult,
                                   verts[vertnum].texvert.GetY() * tmult, 0.0f );
      unclipped_vert->pos.SetX( M(0,0) * P(0) + M(0,1) * P(1) + M(0,2) * P(2) + M(0,3) );
      unclipped_vert->pos.SetY( M(1,0) * P(0) + M(1,1) * P(1) + M(1,2) * P(2) + M(1,3) );
      unclipped_vert->pos.SetZ( M(2,0) * P(0) + M(2,1) * P(1) + M(2,2) * P(2) + M(2,3) );
      unclipped_vert->pos.SetZ( -unclipped_vert->pos.GetZ() );
#undef M
#undef P

      /*
       * Transform normal from object space to view space.
       */
#define M(row,col)  modelview_mat[col*4+row]
#define P(offset)   ( model_vert->normal.Get( offset ) )
      unclipped_vert->normal.SetX( M(0,0) * P(0) + M(0,1) * P(1) + M(0,2) * P(2) );
      unclipped_vert->normal.SetY( M(1,0) * P(0) + M(1,1) * P(1) + M(1,2) * P(2) );
      unclipped_vert->normal.SetZ( M(2,0) * P(0) + M(2,1) * P(1) + M(2,2) * P(2) );
      unclipped_vert->normal.SetZ( -unclipped_vert->normal.GetZ() );
#undef M
#undef P

      float mag;
      mag = 
        ( float )sqrt( unclipped_vert->normal.GetX() * unclipped_vert->normal.GetX() +
                       unclipped_vert->normal.GetY() * unclipped_vert->normal.GetY() +
                       unclipped_vert->normal.GetZ() * unclipped_vert->normal.GetZ() );

#if 1
      unclipped_vert->normal.SetX( unclipped_vert->normal.GetX() * ( 1.0f / mag ) );
      unclipped_vert->normal.SetY( unclipped_vert->normal.GetY() * ( 1.0f / mag ) );
      unclipped_vert->normal.SetZ( unclipped_vert->normal.GetZ() * ( 1.0f / mag ) );
#endif

#if 1
      // light the vert.
      unclipped_vert->color.Set( ( unclipped_vert->normal.GetX() + 1.0f ) / 2.0f,
                                 ( unclipped_vert->normal.GetX() + 1.0f ) / 2.0f,
                                 ( unclipped_vert->normal.GetX() + 1.0f ) / 2.0f );
#endif

#if 0
      if( unclipped_vert->color.GetX() < 0.8f )
        {
          unclipped_vert->color.Set( 0.0f, 0.0f, 0.0f );
        }
#endif

#if 0
      /*
       * The omni stuff needs to be made directional as well as distance
       * based.
       */
      float omni_delta[3];
      omni_delta[0] = -( unclipped_vert->pos.GetX() - omnilight_position[0] );
      omni_delta[1] = -( unclipped_vert->pos.GetY() - omnilight_position[1] );
      omni_delta[2] = -( unclipped_vert->pos.GetZ() - omnilight_position[2] );

      mag =
        ( float )sqrt( omni_delta[0] * omni_delta[0] +
                       omni_delta[1] * omni_delta[1] +
                       omni_delta[2] * omni_delta[2] );

      float oomag = 1.0f / mag;

      omni_delta[0] *= oomag;
      omni_delta[1] *= oomag;
      omni_delta[2] *= oomag;

      float dot;

      dot = 
        omni_delta[0] * unclipped_vert->normal.GetX() +
        omni_delta[1] * unclipped_vert->normal.GetY() +
        omni_delta[2] * unclipped_vert->normal.GetZ();

      dot *= 2000.0f / ( mag * mag );

      if( dot < 0.0f )
        dot = 0.0f;

#if 1
      if( dot > 1.0f )
        dot = 1.0f;
#endif

#if 0
      dot *= dot;
      dot *= dot;
      dot *= dot;
#endif

#if 0
      float denom =
        omni_delta[0] * omni_delta[0] +
        omni_delta[1] * omni_delta[1] +
        omni_delta[2] * omni_delta[2]; 
      float omni_mag = 
        500000.0f / ( denom );
#endif

      //      printf( "%f\n", omni_mag );

#if 0
      unclipped_vert->color.Set( unclipped_vert->color.GetX() + dot,
                                 unclipped_vert->color.GetY() + dot,
                                 unclipped_vert->color.GetZ() );
#endif

#endif
#if 0
      unclipped_vert->color.Set( dot,
                                 dot,
                                 dot );
#endif

      if( unclipped_vert->GetZ() < NEAR_Z )
        {
          sides[vertnum] = SIDE_OUT;
          counts[SIDE_OUT]++;
        }
      else
        {
          sides[vertnum] = SIDE_IN;
          counts[SIDE_IN]++;
        }
    }

  sides[vertnum] = sides[0];

  /*
   * Trivial reject.
   */
#if 0
  if( !counts[SIDE_IN] )
    {
      return;
    }
#endif

  int clipped_vertnum;
  int unclipped_vertnum;
      
  clipped_vertnum = 0;
  for( unclipped_vertnum = 0; 
       unclipped_vertnum < num_verts; 
       unclipped_vertnum++ )
    {
      /*
       * If the current vert is on the inside, the
       * store it.
       */
      if( sides[unclipped_vertnum] == SIDE_IN )
        { 
          clipped_verts[clipped_vertnum] = 
            unclipped_verts[unclipped_vertnum];
          clipped_vertnum++;
        }

      /*
       * If the next vert is on the surface, or if the next
       * vert is on the same side as the current vert, then
       * continue with the next iteration.
       */
      if( sides[unclipped_vertnum + 1] == sides[unclipped_vertnum] )
        {
          continue;
        }

      RevVert *p1, *p2, *dest;
      RevFloat t;

      /*
           * Grab the next input vert so that we can create a split
           * point.
           */
      p1 = &unclipped_verts[unclipped_vertnum];
      p2 = &unclipped_verts[( unclipped_vertnum + 1 ) % num_verts];

      if( p2->GetZ() - p1->GetZ() == 0.0f )
        {
#if 0
          GLIDE_WARNING(( "GlideGraphics::DrawLightmaps",
                        "Potential divide by zero\n" ));
#endif
          return;
        }
          
      dest = &clipped_verts[clipped_vertnum];
      t = ( NEAR_Z - p1->GetZ() ) / ( p2->GetZ() - p1->GetZ() );
      dest->SetX( p1->GetX() + ( p2->GetX() - p1->GetX() ) * t );
      dest->SetY( p1->GetY() + ( p2->GetY() - p1->GetY() ) * t );
      dest->SetZ( p1->GetZ() + ( p2->GetZ() - p1->GetZ() ) * t );
      dest->SetU( p1->GetU() + ( p2->GetU() - p1->GetU() ) * t );
      dest->SetV( p1->GetV() + ( p2->GetV() - p1->GetV() ) * t );
      dest->color.SetX( p1->color.GetX() + ( p2->color.GetX() - p1->color.GetX() ) * t );
      dest->color.SetY( p1->color.GetY() + ( p2->color.GetY() - p1->color.GetY() ) * t );
      dest->color.SetZ( p1->color.GetZ() + ( p2->color.GetZ() - p1->color.GetZ() ) * t );
      clipped_vertnum++;
    }
      

  for( vertnum = 0; vertnum < clipped_vertnum; vertnum++ )
    {
      GrVertex *gvert = &gverts[vertnum];
      RevVert *clipped_vert = &clipped_verts[vertnum];

      gvert->oow = 1.0f / clipped_vert->GetZ();
      gvert->x = clipped_vert->GetX() * gvert->oow;
      gvert->y = clipped_vert->GetY() * gvert->oow;
      gvert->tmuvtx[0].sow = clipped_vert->GetU() * gvert->oow;
      gvert->tmuvtx[0].tow = clipped_vert->GetV() * gvert->oow;
#if 0
      float mag;
      mag = 
        sqrt( clipped_vert->color.GetX() * clipped_vert->color.GetX() +
              clipped_vert->color.GetY() * clipped_vert->color.GetY() +
              clipped_vert->color.GetZ() * clipped_vert->color.GetZ() );
#endif
        
      gvert->r = clipped_vert->color.GetX() * 255.0f;
      gvert->g = clipped_vert->color.GetY() * 255.0f;
      gvert->b = clipped_vert->color.GetZ() * 255.0f;

#if 1
      if( gvert->r < 0.0f )
        gvert->r = 0.0f;
      if( gvert->g < 0.0f )
        gvert->g = 0.0f;
      if( gvert->b < 0.0f )
        gvert->b = 0.0f;
#endif

#if 1
      if( gvert->r > 255.0f )
        gvert->r = 255.0f;
      if( gvert->g > 255.0f )
        gvert->g = 255.0f;
      if( gvert->g > 255.0f )
        gvert->g = 255.0f;
#endif

      gvert->x = gvert->x * x_scale + x_offset;
      gvert->y = gvert->y * y_scale + y_offset;
      gvert->x -= SNAP_BIAS;
      gvert->y -= SNAP_BIAS;
    }
      
  guDrawPolygonVertexListWithClip( clipped_vertnum, gverts );
}

// hack: have to set this stuff after setting the viewport

void GlideGraphics::RecalcFOV( void )
{
#if 0
  x_scale = viewport_width * 0.5f;
  x_offset = ( viewport_x + viewport_width * 0.5f ) + SNAP_BIAS;
  y_scale = viewport_width * 0.5f;
  y_offset = ( viewport_y + viewport_height * 0.5f ) + SNAP_BIAS;
#else
  //  printf( "%f %f\n", fovx, fovy );
  //90 -> 1.0f
  //0  -> inf
  //180 -> 0

  x_scale = .5f * viewport_width * ( 1.0f / ( float )tan( fovx * .5f * M_PI / 180.0f ) );
  x_offset = ( viewport_x + viewport_width * 0.5f ) + SNAP_BIAS;
  //  y_scale = viewport_width * 0.5f * tan( .5f * fovx * M_PI / 180.0f );
  y_scale = x_scale;
  y_offset = ( viewport_y + viewport_height * 0.5f ) + SNAP_BIAS;
#endif
}

void GlideGraphics::SetFovX( RevFloat fovx_ )
{
  fovx = fovx_;
  fovy = ( float )viewport_height / ( float )viewport_width * fovx;
  RecalcFOV();
}

void GlideGraphics::SetFovY( RevFloat fovy_ )
{
  fovy = fovy_;
  fovx = ( float )viewport_width / ( float )viewport_height * fovy;
  RecalcFOV();
}

void GlideGraphics::SetFov( RevFloat fovx_, RevFloat fovy_ )
{
  fovx = fovx_;
  fovy = fovy_;
  RecalcFOV();
}

void GlideGraphics::ObjectToView( RevPoint& object_point, RevPoint& view_point )
{
  RevFloat *modelview_mat = matrix_stack[GLIDE_MODELVIEW].mat[matrix_stack[GLIDE_MODELVIEW].top_of_stack];
      /*
       * Transform from object space to view space.
       */
#define M(row,col)  modelview_mat[col*4+row]
#define P(offset)   ( object_point.Get( offset ) )
  view_point.SetX( M(0,0) * P(0) + M(0,1) * P(1) + M(0,2) * P(2) + M(0,3) );
  view_point.SetY( M(1,0) * P(0) + M(1,1) * P(1) + M(1,2) * P(2) + M(1,3) );
  view_point.SetZ( M(2,0) * P(0) + M(2,1) * P(1) + M(2,2) * P(2) + M(2,3) );
  view_point.SetZ( -view_point.GetZ() );
#undef M
#undef P
}
