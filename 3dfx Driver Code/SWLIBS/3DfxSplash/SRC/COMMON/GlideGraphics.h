#ifndef __GLIDEGRAPHICS_H__
#define __GLIDEGRAPHICS_H__

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <glide.h>
#include "RevTypes.h"
#include "RevPoint.h"

class RevFace;

#define GLIDE_GRAPHICS_CLEAR_DEPTH 0x01
#define GLIDE_GRAPHICS_CLEAR_COLOR 0x02
#define GLIDE_GRAPHICS_CLEAR_MASK  0x03

#define GLIDE_GRAPHICS_POINT_SAMPLE    0x01
#define GLIDE_GRAPHICS_BILINEAR_FILTER 0x02

/*
 * Matrix stack modes.
 */
#define GLIDE_MODELVIEW      0x00
#define GLIDE_PROJECTION     0x01
#define GLIDE_TEXTURE        0x02

/*
 * "Get" enum
 */
#define GLIDE_MODELVIEW_MATRIX 0x01
#define GLIDE_DEVICE_WIDTH     0x02
#define GLIDE_DEVICE_HEIGHT    0x03

/*
 * Buffer enum
 */
#define GLIDE_BUFFER_FRONT 1
#define GLIDE_BUFFER_BACK  2

struct RevVert
{
  RevPoint pos;
  RevPoint normal;
  RevPoint texvert;
  RevPoint color;

  float GetU() { return texvert.GetX(); }
  float GetV() { return texvert.GetY(); }
  float GetX() { return pos.GetX(); }
  float GetY() { return pos.GetY(); }
  float GetZ() { return pos.GetZ(); }
  void SetX( float x ) { pos.SetX( x ); }
  void SetY( float y ) { pos.SetY( y ); }
  void SetZ( float z ) { pos.SetZ( z ); }
  void SetU( float u ) { texvert.SetX( u ); };
  void SetV( float v ) { texvert.SetY( v ); };
};

typedef RevFloat RevGlideMatrix[16];

#define GLIDE_GLIDE_MATRIX_STACK_DEPTH 32

typedef struct
{
  RevGlideMatrix mat[GLIDE_GLIDE_MATRIX_STACK_DEPTH];
  int top_of_stack;
} RevGlideMatrixStack;

class GlideGraphics
{
public:
  void RenderBuffer( RevU32 flags );
  /*
   * ----------------------------------------------------------------------
   * Constructors/destructors
   * ----------------------------------------------------------------------
   */
  GlideGraphics();
  ~GlideGraphics();

  /*
   * ----------------------------------------------------------------------
   * Buffer management stuff.
   * ----------------------------------------------------------------------
   */
  RevBool Open( void * /*HWND hwnd_ */ );
  RevBool Close( void );
  void Swap( int swapinterval = 1 );
  void Clear( RevU32 flags, float r, float g, float b );
  void FadeToBlack(void);
  void Resize( int width, int height );
  void Move( void );
	void FadeIn(float fade, float r, float g, float b);
	void SetClipWindow(int x, int y, int width, int height);

  /*
   * ----------------------------------------------------------------------
   * Rendering stuff
   * ----------------------------------------------------------------------
   */
  void DrawLine( RevVert *p1, RevVert *p2, RevU32 color );
  void DrawPolygon( int num_verts, RevVert *vert_ptrs[], unsigned int flags);

  /*
   * ----------------------------------------------------------------------
   * Matrix/viewport stuff.
   * ----------------------------------------------------------------------
   */
private:
  void RecalcFOV();
public:
  void ObjectToView( RevPoint& object_point, RevPoint& view_point );
  void SetFovX( RevFloat fovx );
  void SetFovY( RevFloat fovy );
  void SetFov( RevFloat fovx, RevFloat fovy );
  void MatrixMode( RevU32 mode );
  void LoadIdentity( void );
  void Perspective( RevFloat fovy, RevFloat aspect, RevFloat zNear,
                    RevFloat zFar );
  void Frustum( RevFloat left, RevFloat right, 
                RevFloat bottom, RevFloat top, 
                RevFloat zNear, RevFloat zFar );
  void Viewport( RevI32 x, RevI32 y, RevI32 width, RevI32 height );
  void Ortho( RevFloat left, RevFloat right, RevFloat bottom, RevFloat top, 
              RevFloat zNear, RevFloat zFar );
  void Translate( RevFloat x, RevFloat y, RevFloat z );
  void Rotate( RevFloat angle, RevFloat x, RevFloat y, RevFloat z );
  void PushMatrix( void );
  void PopMatrix( void );
  void MultMatrix( const RevFloat *m );

  void SetTexCoordFactors( float smult_, float tmult_ )
  {
    smult = smult_;
    tmult = tmult_;
  }

  void SetOmniLight( RevFloat pos[3], float color[3] )
  {

	omnilight_position[0] = pos[0];
	omnilight_position[1] = pos[1];
	omnilight_position[2] = pos[2];

	omnilight_color[0] = color[0] * 255.0f;
	omnilight_color[1] = color[1] * 255.0f;
	omnilight_color[2] = color[2] * 255.0f;

  }

	void SetDirLight( RevFloat dir[3], float color[3] )
	{
		dirlight_direction[0] = dir[0];
		dirlight_direction[1] = dir[1];
		dirlight_direction[2] = dir[2];

		dirlight_color[0] = color[0] * 255.0f;
		dirlight_color[1] = color[1] * 255.0f;
		dirlight_color[2] = color[2] * 255.0f;
	}

	void ComputeLighting(float *diffuse, float *specular, RevVert *vert, float light_dir[3]);

  /*
   * ----------------------------------------------------------------------
   * Get stuff.
   * ----------------------------------------------------------------------
   */
  void GetFloatv( RevU32 pname, RevFloat *params );

	void StoreGlideState();
	void RestoreGlideState();
	void SetSplashGlideState();

public:

protected:
  float smult, tmult;
  int device_width, device_height;
  int viewport_width, viewport_height;
#ifdef USE_GLIDE3
  void *oldState;
	void *oldVertState;
#else
	GrState oldState;
#endif // USE_GLIDE3

#ifdef USE_GLIDE3
	GrContext_t context;
#else
  GrHwConfiguration hwconfig;
#endif // USE_GLIDE3
  RevGlideMatrix composite_matrix;
  RevGlideMatrixStack matrix_stack[3];
  RevGlideMatrixStack *current_matrix_stack;
  /*
   * ----------------------------------------------------------------------
   * Viewport junk.
   * ----------------------------------------------------------------------
   */
  RevFloat x_scale;
  RevFloat x_offset;
  RevFloat y_scale;
  RevFloat y_offset;

  RevFloat fovx, fovy;

  RevI32 viewport_x, viewport_y;

  float omnilight_color[3];
	float dirlight_color[3];

  RevFloat omnilight_position[3];
  RevFloat dirlight_direction[3];

  void InitMatrixStacks( void );
  void RebuildCompositeMatrix( void );
  void MatrixMultiply( RevFloat *dest, const RevFloat *left, RevFloat *right );

protected:
  RevBool fullscreen;
  HWND hwnd;
};

#endif /* __REVGLIDEGRAPHICS_H__ */
