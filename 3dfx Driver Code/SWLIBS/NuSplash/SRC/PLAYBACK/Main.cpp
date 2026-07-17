#define WIN32_LEANER_AND_MEANER
#include <windows.h>
#include <mmsystem.h>

#define FX_DLL_DEFINITION
#include <3dfx.h>
#include <fxdll.h>
#include <glide.h>

#include <stdio.h>
#include <stdlib.h>
#include "GlideGraphics.h"
#include "texcache.h"
#include <assert.h>
#include <conio.h>

#include <math.h>

#ifndef M_PI
#define M_PI 3.14159f
#endif

#define GRAPHICS_SWAP_CMD 2
#define DRAW_GEOM_CMD 3
#define CAMERA_MATRIX_CMD 4
#define EOF_CMD 5
#define SET_MATERIAL_CMD 6
#define OMNI_LIGHT_CMD 7
#define Z_WRITE_CMD 8
#define XPARENCY_CMD 9

#include <largeint.h>

char *cmd_to_string[] = {
  "",
  "",
  "GRAPHICS_SWAP_CMD",
  "DRAW_GEOM_CMD",
  "CAMERA_MATRIX_CMD",
  "EOF_CMD",
  "SET_MATERIAL_CMD",
  "OMNI_LIGHT_CMD",
  "Z_WRITE_CMD",
  "XPARENCY_CMD"
};

static FxU32 constant_color = 0x00ffffff;
static FxU32 constant_alpha = 0xff000000;

GlideGraphics *graphics = NULL;
TexCache texcache;
FILE *fp;
char buf[256];
int frame_count;
int frame_rate;
int num_stored_materials;
int num_stored_geometry;

int num_texinfos;
GrTexInfo *texinfos;
int *texinfo_id_to_texcache_id;

unsigned char *commands;
int commands_size;
unsigned char *cur_cmd;

static int skip_frame = FALSE;

class SubMaterial
{
public:
  SubMaterial() { texcache_id = TEX_CACHE_NULL_ID; }
  float diffuse[3];
  float specular[3];
  TexCacheId_t texcache_id;
};

class Material
{
public:
  int num_submaterials;
  SubMaterial *submaterials;
};

static Material *materials;
static int num_materials = -1;

class Mesh
{
public:
  struct Face
  {
    int vert_ids[3];
  };

  Mesh() {}
  void Draw( void );
  BOOL Read( FILE *fp );
private:
  int num_faces;
  int num_verts;
  int num_tex_verts;
  Face *faces;
  RevVert *verts;
};

Mesh *meshes;

void Mesh::Draw( void )
{
  Face *face, *end_face;
  end_face = faces + num_faces;
  for( face = faces; face < end_face; face++ )
    {
      RevVert *vert[3];

      vert[0] = &verts[face->vert_ids[0]];
      vert[1] = &verts[face->vert_ids[1]];
      vert[2] = &verts[face->vert_ids[2]];

      RevVert blah[3];
      blah[0] = *vert[0];
      blah[1] = *vert[1];
      blah[2] = *vert[2];

      graphics->DrawPolygon( 3, blah );
    }
}

BOOL Mesh::Read( FILE *fp )
{
  // Read the faces.
  fread( &num_faces, sizeof( num_faces ), 1, fp );
  //  printf( "num_faces: %d\n", num_faces );
  int i;
  faces = new Face[num_faces];
  if( !faces )
    return FALSE;
  for( i = 0; i < num_faces; i++ )
    {
      int vertnum;
      for( vertnum = 0; vertnum < 3; vertnum++ )
        {
          fread( &faces[i].vert_ids[vertnum], sizeof( int ), 1, fp );
          //          printf( "vert_ids[%d]: %d\n", vertnum, faces[i].vert_ids[vertnum] );
        }
    }

  // Read the verts
  fread( &num_verts, sizeof( num_verts ), 1, fp );
  //  printf( "num_verts: %d\n", num_verts );
  verts = new RevVert[num_verts];
  for( i = 0; i < num_verts; i++ )
    {
      float x, y, z;
      fread( &x, sizeof( float ), 1, fp );
      fread( &y, sizeof( float ), 1, fp );
      fread( &z, sizeof( float ), 1, fp );
      verts[i].pos.Set( x, y, z );
      //      printf( "vert[%d]: %f %f %f\n", i, x, y, z );
      fread( &x, sizeof( float ), 1, fp );
      fread( &y, sizeof( float ), 1, fp );
      fread( &z, sizeof( float ), 1, fp );
      verts[i].normal.Set( x, y, z );
      //      printf( "normal[%d]: %f %f %f\n", i, x, y, z );
    }

  // Read the texverts
  fread( &num_tex_verts, sizeof( num_tex_verts ), 1, fp );
  //  printf( "num_tex_verts: %d\n", num_tex_verts );
  for( i = 0; i < num_tex_verts; i++ )
    {
      float u, v;
      fread( &u, sizeof( float ), 1, fp );
      fread( &v, sizeof( float ), 1, fp );
      if( i < num_verts )
        {
          verts[i].texvert.Set( u, v, 0.0f );
          //          printf( "texvert[%d]: %f %f\n", i, u, v );
        }
    }
  return TRUE;
}

void SetupGraphics( int width, int height )
{
  graphics = new GlideGraphics;
  graphics->Open( NULL );
  graphics->Resize();
  graphics->Clear( GLIDE_GRAPHICS_CLEAR_COLOR | GLIDE_GRAPHICS_CLEAR_DEPTH, 
                   0.0f, 0.0f, 0.0f );
  graphics->MatrixMode( GLIDE_PROJECTION );
  graphics->LoadIdentity();
  graphics->Perspective( 90.0f, 1, 1.5, 20000.0f );
  graphics->Viewport( 0, 0, width, height );
  graphics->SetFovX( 90.0f );
  graphics->MatrixMode( GLIDE_MODELVIEW );
  graphics->PushMatrix();
  graphics->LoadIdentity();
  TexCacheInit( &texcache, grTexMinAddress( GR_TMU0 ), grTexMaxAddress( GR_TMU0 ),
                1024, GR_TMU0, "the cache" );
  graphics->Clear( GLIDE_GRAPHICS_CLEAR_COLOR | GLIDE_GRAPHICS_CLEAR_DEPTH, 0.0f, 0.0f, 0.0f );
  graphics->Swap();
  graphics->Clear( GLIDE_GRAPHICS_CLEAR_COLOR | GLIDE_GRAPHICS_CLEAR_DEPTH, 0.0f, 0.0f, 0.0f );
  grCullMode( GR_CULL_NEGATIVE );
}

int ReadCookie( void )
{
  fgets( buf, 256, fp );
  //  printf( "cookie string: \"%s\"\n", buf );
  if( strcmp( buf, "3Dfx Splash Version 1.1\n" ) != 0 )
    {
      fprintf( stderr, "wrong version number\n" );
      return 0;
    }
  return 1;
}

int ReadFrameCount( void )
{
  int retval;
  retval = ( fread( &frame_count, sizeof( frame_count ), 1, fp ) == 1 );
  //  printf( "frame_count: %d\n", frame_count );
  return retval;
}

int ReadFrameRate( void )
{
  int retval;
  retval = ( fread( &frame_rate, sizeof( frame_rate ), 1, fp ) == 1 );
  //  printf( "frame_rate: %d\n", frame_rate );
  return retval;
}

int ReadTexinfos( void )
{
  fread( &num_texinfos, sizeof( num_texinfos ), 1, fp );
  texinfos = new GrTexInfo[num_texinfos];
  texinfo_id_to_texcache_id = new int[num_texinfos];
  int i;
  for( i = 0; i < num_texinfos; i++ )
    texinfo_id_to_texcache_id[i] = TEX_CACHE_NULL_ID;
  if( !texinfos || !texinfo_id_to_texcache_id )
    return FALSE;
  FxU32 size;

  for( i = 0; i < num_texinfos; i++ )
    {
      GrTexInfo *texinfo = &texinfos[i];
      
      fread( texinfo, sizeof( GrTexInfo ), 1, fp );
#if 0
      printf( "texture format: %s\n", 
              ( texinfo->format == GR_TEXFMT_ARGB_4444 ) ? "GR_TEXFMT_ARGB_4444" :
              "GR_TEXFMT_RGB_565" );
#endif
      fread( &size, sizeof( size ), 1, fp );
      texinfo->data = new unsigned char[size];
      if( !texinfo->data )
        return FALSE;
      fread( texinfo->data, size, 1, fp );
    }
  return TRUE;
}

void DownloadTextures( void )
{
  int i;
  
  for( i = 0; i < num_texinfos; i++ )
    {
      texinfo_id_to_texcache_id[i] = TexCacheInsertTexture( &texcache, &texinfos[i] );
    }
}

int ReadStoredMaterials( void )
{
  if( !ReadTexinfos() )
    return FALSE;

  DownloadTextures();

  fread( &num_stored_materials, sizeof( num_stored_materials ), 1, fp );
  //  printf( "num_stored_materials: %d\n", num_stored_materials );

  materials = new Material[num_stored_materials];
  if( !materials )
    return FALSE;

  int i, j;

  for( i = 0; i < num_stored_materials; i++ )
    {
      Material *material = &materials[i];
      fread( &material->num_submaterials, sizeof( int ), 1, fp );
      //      printf( "num_submaterials: %d\n", material->num_submaterials );
      material->submaterials = new SubMaterial[material->num_submaterials];
      if( !material->submaterials )
        return FALSE;
      for( j = 0; j < material->num_submaterials; j++ )
        {
          SubMaterial *submaterial = &material->submaterials[j];
          fread( submaterial->diffuse, sizeof( submaterial->diffuse ), 1, fp );
          fread( submaterial->specular, sizeof( submaterial->specular ), 1, fp );
          int texture_index;
          fread( &texture_index, sizeof( texture_index ), 1, fp );
          if( texture_index != -1 )
            submaterial->texcache_id = texinfo_id_to_texcache_id[texture_index];
#if 0
          printf( "diffuse: %f %f %f\n", diffuse[0], diffuse[1], diffuse[2] );
          printf( "specular: %f %f %f\n", specular[0], specular[1], specular[2] );
          printf( "texture_index: %d\n", texture_index );
#endif
        }
    }

  return TRUE;
}

int ReadStoredGeometry( void )
{
  int retval;
  retval = ( fread( &num_stored_geometry, sizeof( num_stored_geometry ), 1, fp ) == 1 );
  //  printf( "num_stored_geometry: %d\n", num_stored_geometry );
  if( !retval )
    return FALSE;

  // allocate space for the geometry
  meshes = new Mesh[num_stored_geometry];
  if( !meshes )
    return FALSE;

  int i;
  while( 1 )
    {
      fread( &i, sizeof( i ), 1, fp );
      //      printf( "geometry index: %d\n", i );
      if( i == -1 )
        break;
      assert( i < num_stored_geometry );
      meshes[i].Read( fp );
    };
  return TRUE;
}

int ReadCommands( void )
{
  long curpos, endpos;

  curpos = ftell( fp );
  fseek( fp, 0, SEEK_END );
  endpos = ftell( fp );
  fseek( fp, curpos, SEEK_SET );
  commands_size = endpos - curpos;
  commands = new unsigned char[commands_size];
  if( !commands )
    return FALSE;
  // make sure the memory is resident for the commands.
  int j;
  int i;
  for( j = 0; j < 5; j++ )
    {
      for( i = 0; i < commands_size; i++ )
        {
          commands[i] = commands[commands_size-i-1];
        }
    }
  fread( commands, commands_size, 1, fp );
  return 1;
}

void DoGraphicsSwapCmd( void )
{
  if( skip_frame )
    return;
  graphics->Swap( 1 );
  grDepthMask( FXTRUE );
  graphics->Clear( GLIDE_GRAPHICS_CLEAR_COLOR | GLIDE_GRAPHICS_CLEAR_DEPTH, 0.0f, 0.0f, 0.0f );
}

void DoDrawGeomCmd( void )
{
  int mesh_id;

  mesh_id = *( int * )cur_cmd;
  cur_cmd += sizeof( mesh_id );
  graphics->PushMatrix();
  graphics->MultMatrix( ( float * )cur_cmd );
  cur_cmd += sizeof( float[16] );
  if( !skip_frame )
    meshes[mesh_id].Draw();
  graphics->PopMatrix();
}

void DoCameraMatrixCmd( void )
{
  float fov;

  fov = *( float * )cur_cmd;

  graphics->SetFovX( fov * 180.0f / M_PI );
  cur_cmd += sizeof( float );
  graphics->LoadIdentity();
  graphics->MultMatrix( ( float * )cur_cmd );
  cur_cmd += sizeof( float[16] );
}

void SetMaterialCmd( void )
{
  int material_id;

  material_id = *( int * )cur_cmd;
  //  printf( "material_id: %d\n", material_id );
  cur_cmd += sizeof( int );

  if( skip_frame )
    return;
  
  Material *material = &materials[material_id];
  if( material->submaterials[0].texcache_id != TEX_CACHE_NULL_ID )
    {
      TexCacheSetCurrent( &texcache, 
                          material->submaterials[0].texcache_id, 
                          FXFALSE );
      //      guColorCombineFunction( GR_COLORCOMBINE_DECAL_TEXTURE );
      //      guColorCombineFunction( GR_COLORCOMBINE_TEXTURE_TIMES_ITRGB );
      //      guColorCombineFunction( GR_COLORCOMBINE_ITRGB );
      //      grTexCombineFunction( GR_TMU0, GR_TEXTURECOMBINE_DECAL );
      /* Set up Render State - Decal Texture - alpha blend */
      grTexCombine( GR_TMU0,
                    GR_COMBINE_FUNCTION_LOCAL,
                    GR_COMBINE_FACTOR_NONE,
                    GR_COMBINE_FUNCTION_LOCAL,
                    GR_COMBINE_FACTOR_NONE,
                    FXFALSE, FXFALSE );

#if 1
      // texture * iterated color
      grColorCombine( GR_COMBINE_FUNCTION_SCALE_OTHER,
                      GR_COMBINE_FACTOR_LOCAL,
                      GR_COMBINE_LOCAL_ITERATED,
                      GR_COMBINE_OTHER_TEXTURE,
                      FXFALSE );
#endif
#if 0
      // texture * constant color
      grColorCombine( GR_COMBINE_FUNCTION_SCALE_OTHER,
                      GR_COMBINE_FACTOR_LOCAL,
                      GR_COMBINE_LOCAL_CONSTANT,
                      GR_COMBINE_OTHER_TEXTURE,
                      FXFALSE );
#endif

#if 1
      // alpha blend on texture alpha
      grAlphaCombine( GR_COMBINE_FUNCTION_SCALE_OTHER,
                      GR_COMBINE_FACTOR_ONE,
                      GR_COMBINE_LOCAL_NONE,
                      GR_COMBINE_OTHER_TEXTURE,
                      FXFALSE );
#endif
#if 0
      // alpha blend on constant alpha
      grAlphaCombine( GR_COMBINE_FUNCTION_LOCAL,
                      GR_COMBINE_FACTOR_NONE,
                      GR_COMBINE_LOCAL_CONSTANT,
                      GR_COMBINE_OTHER_NONE,
                      FXFALSE );
#endif

      grAlphaBlendFunction( GR_BLEND_SRC_ALPHA,
                            GR_BLEND_ONE_MINUS_SRC_ALPHA,
                            GR_BLEND_ZERO,
                            GR_BLEND_ZERO );

      constant_color = 
        ( ( int )( material->submaterials[0].diffuse[0] * 255.0f ) ) << 16 |
        ( ( int )( material->submaterials[0].diffuse[1] * 255.0f ) ) << 8 |
        ( ( int )( material->submaterials[0].diffuse[2] * 255.0f ) ) << 0;
      grConstantColorValue( constant_color | constant_alpha );
    }
  else
    {
      // constant color * iterated color.
      grColorCombine( GR_COMBINE_FUNCTION_SCALE_OTHER,
                      GR_COMBINE_FACTOR_LOCAL,   
                      GR_COMBINE_LOCAL_CONSTANT, 
                      GR_COMBINE_OTHER_ITERATED, 
                      FXFALSE );       

      //      guColorCombineFunction( GR_COLORCOMBINE_ITRGB );
      //      grConstantColorValue( 0xffffffff );
      //      material->submaterials[0].diffuse[3];
      constant_color = 
        ( ( int )( material->submaterials[0].diffuse[0] * 255.0f ) ) << 16 |
        ( ( int )( material->submaterials[0].diffuse[1] * 255.0f ) ) << 8 |
        ( ( int )( material->submaterials[0].diffuse[2] * 255.0f ) ) << 0;
      grConstantColorValue( constant_color | constant_alpha );
      //      grConstantColorValue( 0xffff0000 );
      // alpha blend on constant alpha
      grAlphaCombine( GR_COMBINE_FUNCTION_LOCAL,
                      GR_COMBINE_FACTOR_NONE,
                      GR_COMBINE_LOCAL_CONSTANT,
                      GR_COMBINE_OTHER_NONE,
                      FXFALSE );
#if 1
      grAlphaBlendFunction( GR_BLEND_SRC_ALPHA,
                            GR_BLEND_ONE_MINUS_SRC_ALPHA,
                            GR_BLEND_ZERO,
                            GR_BLEND_ZERO );
#else
      grAlphaBlendFunction( GR_BLEND_ONE,
                            GR_BLEND_ZERO,
                            GR_BLEND_ZERO,
                            GR_BLEND_ZERO );
#endif
    }
}

void OmniLightCmd( void )
{
  float color[3];
  float intensity;
  float position[3];
  memcpy( color, cur_cmd, sizeof( color ) );
  cur_cmd += sizeof( color );
  intensity = *( float * )cur_cmd;
  cur_cmd += sizeof( float );
  memcpy( position, cur_cmd, sizeof( position ) );
  cur_cmd += sizeof( position );
  graphics->SetOmniLight( position );
  //  printf( "omni: %f %f %f\n", position[0], position[1], position[2] );
}

void ZWriteCmd( void )
{
  //  grDepthMask( *cur_cmd );
  cur_cmd += 1;
}


void XparencyCmd( void )
{
  //  printf( "0x%08x\n", ( int )*( FxU32 * )cur_cmd );
#if 1
  if( ( *( FxU32 * )cur_cmd == 0 ) || ( *( FxU32 * )cur_cmd == 0xff000000 ) )
    {
      constant_alpha = 0xff000000;
      grConstantColorValue( constant_color | constant_alpha );
    }
  else
    {
      constant_alpha = *( FxU32 * )cur_cmd;
      grConstantColorValue( constant_color | constant_alpha );
    }
#endif
  cur_cmd += sizeof( FxU32 );
}

int DoCommandLoop( void )
{
  unsigned char *start, *end;

  start = commands;
  end = commands + commands_size;
  cur_cmd = start;

  int cur_frame = 0, next_frame = 0;

  LARGE_INTEGER timer_freq;

  if( !QueryPerformanceFrequency( &timer_freq ) )
    {
      assert( 0 );
    }

#if 0
  printf( "timer_freq: %ld %ld\n", ( long )timer_freq.HighPart, 
          ( long )timer_freq.LowPart );
#endif

  LARGE_INTEGER start_time, cur_time;

  QueryPerformanceCounter( &start_time );

  while( cur_cmd < end )
    {
      unsigned char cmd;

      cmd = *cur_cmd++;
      //      printf( "cmd: %s\n", cmd_to_string[cmd] );
      switch( cmd )
        {
        case GRAPHICS_SWAP_CMD:
          DoGraphicsSwapCmd();
          QueryPerformanceCounter( &cur_time );
          cur_frame++;

          LARGE_INTEGER big_delta;
          big_delta = LargeIntegerSubtract( cur_time, start_time );

          LARGE_INTEGER dummy_large;

          big_delta = ExtendedIntegerMultiply( big_delta, frame_rate );
          next_frame = LargeIntegerDivide( big_delta, timer_freq, &dummy_large ).LowPart;

          if( next_frame > cur_frame )
            skip_frame = TRUE;
          else
            skip_frame = FALSE;
#if 0
          skip_frame = FALSE;
#endif
          break;
        case DRAW_GEOM_CMD:
          DoDrawGeomCmd();
          break;
        case CAMERA_MATRIX_CMD:
          DoCameraMatrixCmd();
          break;
        case SET_MATERIAL_CMD:
          SetMaterialCmd();
          break;
        case EOF_CMD:
          return TRUE;
          break;
        case OMNI_LIGHT_CMD:
          OmniLightCmd();
          break;
        case Z_WRITE_CMD:
          ZWriteCmd();
          break;
        case XPARENCY_CMD:
          XparencyCmd();
          break;
        default:
          fprintf( stdout, "Unknown command: 0x%x\n", ( int )cmd );
          assert( 0 );
          break;
        }
    }
  return 1;
}

int LoadSplashData( void )
{
  // read the cookie and the version number
  if( !ReadCookie() )
    return 0;

  // read the frame count.
  if( !ReadFrameCount() )
    return 0;

  // read the frame rate.
  if( !ReadFrameRate() )
    return 0;

  // read the stored materials
  if( !ReadStoredMaterials() )
    return 0;

  // read the stored geometry
  if( !ReadStoredGeometry() )
    return 0;

  // read the rendering command from the data file.
  if( !ReadCommands() )
    return 0;

  return TRUE;
}

#ifdef SPLASH_DLL
#define DllExport    __declspec( dllexport )

extern "C" DllExport FX_ENTRY void FX_CSTYLE
fxSplash( FxU32 hWind, FxU32 scrWidth, FxU32 scrHeight, FxU32 nAuxBuffers )
{
  static char system_dir[MAX_PATH];

  if( nAuxBuffers == 0 )
    return;

  GetSystemDirectory( system_dir, MAX_PATH );

  if( !( fp = fopen( "3dfxsplash.dat", "rb" ) ) )
    {
      char tmp[256];
      sprintf( tmp, "%s\\3dfxsplash.dat", system_dir );
      if( !( fp = fopen( tmp, "rb" ) ) )
        {
          return;
        }
    }

  SetupGraphics( scrWidth, scrHeight );

  Sleep( 1000 );

  if( !LoadSplashData() )
    {
      fclose( fp );
      return;
    }

  BOOL using_sound;
  
#if 0
  FILE *wave_fp;

  if( !( wave_fp = fopen( "3dfxsplash.wav", "rb" ) ) )
    {
      char tmp[256];
      sprintf( tmp, "%s\\system\\3dfxsplash.wav", windir );
      if( !( wave_fp = fopen( tmp, "rb" ) ) )
        {
          sprintf( tmp, "%s\\system32\\3dfxsplash.wav", windir );
          if( !( wave_fp = fopen( tmp, "rb" ) ) )
            {
              using_sound = FALSE;
            }
        }
    }

  unsigned char *sound_buffer;
  if( using_sound )
    {
      int wave_size;

      fseek( wave_fp, 0, SEEK_END );
      wave_size = ftell( wave_fp ) + 1;
      fseek( wave_fp, 0, SEEK_SET );
      sound_buffer = new unsigned char[wave_size];
      using_sound = PlaySound( ( const char * )sound_buffer, NULL, SND_MEMORY | SND_ASYNC );
    }

#else
  // Play the sound.
  using_sound = PlaySound( "3dfxsplash.wav", NULL, SND_FILENAME | SND_ASYNC );
#endif

  // Do the main event loop to render the animation
  DoCommandLoop();

  Sleep( 1000 );

  // Make sure that sound is turned off so we don't screw up the app.
  if( using_sound )
    PlaySound( NULL, NULL, NULL );

  fclose( fp );
  return;
}
#else
int main( int argc, char **argv )
{
  BOOL using_sound;

  if( argc != 2 )
    return 0;
  if( !( fp = fopen( argv[1], "rb" ) ) )
    return 0;

  SetupGraphics( 640, 480 );

  Sleep( 1000 );

  if( !LoadSplashData() )
    {
      fclose( fp );
      return 0;
    }

  // Play the sound.
  using_sound = PlaySound( "test.wav", NULL, SND_FILENAME | SND_ASYNC );

  // Do the main event loop to render the animation
  DoCommandLoop();

  Sleep( 1000 );

  // Make sure that sound is turned off so we don't screw up the app.
  if( using_sound )
    PlaySound( NULL, NULL, NULL );

  fclose( fp );
  return 1;
}
#endif
