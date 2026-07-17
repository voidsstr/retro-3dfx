#define WIN32_LEANER_AND_MEANER
#include <windows.h>
#include <mmsystem.h>


#include <stdio.h>
#include <stdlib.h>

#include <assert.h>
#include <conio.h>

#include <math.h>

#ifdef SPLASH_DLL
#define FX_DLL_DEFINITION 1
#define FX_DLL_ENABLE 1
#endif



#include <3dfx.h>
#include <fxdll.h>
#include <glide.h>


#include "GlideGraphics.h"
#include "texcache.h"

#include "..\playback\convexhull.h"

#include "resource.h"


#ifndef M_PI
#define M_PI 3.14159f
#endif


//#define DO_FRONT_BUFFER

#define GRAPHICS_SWAP_CMD 2
#define DRAW_GEOM_CMD 3
#define CAMERA_MATRIX_CMD 4
#define EOF_CMD 5
#define SET_MATERIAL_CMD 6
#define OMNI_LIGHT_CMD 7
#define Z_WRITE_CMD 8
#define XPARENCY_CMD 9
#define PRECALC_EDGES_CMD 10
#define START_CMD 11
#define GRAPHICS_SWAP_NO_TIMEBASE_CMD 12
#define GRAPHICS_SWAP_LAST_FRAME_CMD 13


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
  "XPARENCY_CMD",
  "PRECALC_EDGES_CMD",
  "START_CMD"
};

typedef struct _res_mem_
{
	BYTE *start, *end, *curr;
	DWORD size;
} ResMem;

static FxU32 constant_color = 0x00ffffff;
static FxU32 constant_alpha = 0xff000000;

GlideGraphics *graphics = NULL;
TexCache texcache;
ResMem resource;
char buf[256];
int frame_count;
int frame_rate;
int num_stored_materials;
int num_stored_geometry;

int num_texinfos;
GrTexInfo *texinfos;
int *texinfo_id_to_texcache_id;

int gNormals;

unsigned char *commands;
int commands_size;
unsigned char *cur_cmd;

static int skip_frame = FALSE;

int cur_frame;
BOOL using_sound = FALSE;

int has_opacity_map;

HMODULE gModule;

// this is just like fread, but reads from main memory instead of a file
size_t mread(void *buffer, size_t size, size_t count, ResMem *res_mem)
{
	size_t i;
	BYTE *dest;

	dest = (BYTE *)buffer;
	for (i=size*count; i>0; i--)
	{
		*dest++ = *res_mem->curr++;
	}

	return count;
}

// this is just like fgets, but reads from main memory instead of a file
char *mgets(char *string, int n, ResMem *res_mem)
{
	int i;
	char *dest;

	dest = string;
	for (i=0; i<n; i++)
	{
		*dest++ = *res_mem->curr++;

		if (*(res_mem->curr-1) == '\n')
		{
			*dest = '\0';
			break;
		}
	}

	return string;
}


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
  void PreCalcEdges( void );

  BOOL Read( ResMem *res_mem );
private:
  unsigned int flags;
  int num_AAEdges;
  AAEdge *edges;
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
  AAEdge *edgePtr;


  if (flags & ANTI_ALIAS)
    ClearEdges();

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


      graphics->DrawPolygon( 3, blah ,flags | DO_DRAW);
   }

#if 1

  if (flags & ( ANTI_ALIAS | PRE_CALC))
  {
	  grAlphaCombine( GR_COMBINE_FUNCTION_SCALE_OTHER,
		  GR_COMBINE_FACTOR_ONE,
		  GR_COMBINE_LOCAL_NONE,
		  GR_COMBINE_OTHER_ITERATED,
		  FXFALSE );

	  grAlphaBlendFunction(GR_BLEND_SRC_ALPHA,
		  GR_BLEND_ONE_MINUS_SRC_ALPHA,
		  GR_BLEND_ZERO,
		  GR_BLEND_ZERO);

	  if (flags & ANTI_ALIAS)
		 num_AAEdges = ConvexHull(edge_list,edge_count);

	  edgePtr = (flags & PRE_CALC) ? edges : edge_list;

#ifdef USE_GLIDE3
		grEnable(GR_AA_ORDERED);
#endif

		// draw the AA lines
		for (int i = 0; i < num_AAEdges; i++,edgePtr++) {
		  edgePtr->v0.a = edgePtr->v1.a = (constant_alpha >> 24) * 1.0f;
//		  edgePtr->v0.a = edgePtr->v1.a = 255.0f;
#ifdef USE_GLIDE3
		  grDrawLine(&edgePtr->v0,&edgePtr->v1);
#else
		  grAADrawLine(&edgePtr->v0,&edgePtr->v1);
#endif // USE_GLIDE3
	  }

#ifdef USE_GLIDE3
		grDisable(GR_AA_ORDERED);
#endif
  }

#endif

}


void Mesh::PreCalcEdges( void )
{
  Face *face, *end_face;
  end_face = faces + num_faces;

  ClearEdges();

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

        graphics->DrawPolygon( 3, blah ,flags | ANTI_ALIAS);
    }

	num_AAEdges = ConvexHull(edge_list,edge_count);
	for (int i = 0; i < num_AAEdges; i++)
		edges[i] = edge_list[i];
		  
}




BOOL Mesh::Read( ResMem *res_mem )
{
  // Read the flags
  mread(&flags, sizeof(flags), 1, res_mem);

  if (flags & PRE_CALC) {
	  edges = new AAEdge[1024];

  }

  // Read the faces.
  mread( &num_faces, sizeof( num_faces ), 1, res_mem );
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
          mread( &faces[i].vert_ids[vertnum], sizeof( int ), 1, res_mem );
          //          printf( "vert_ids[%d]: %d\n", vertnum, faces[i].vert_ids[vertnum] );
        }
    }

  // Read the verts
  mread( &num_verts, sizeof( num_verts ), 1, res_mem );
  //  printf( "num_verts: %d\n", num_verts );
  verts = new RevVert[num_verts];
  for( i = 0; i < num_verts; i++ )
    {
      float x, y, z;
      mread( &x, sizeof( float ), 1, res_mem );
      mread( &y, sizeof( float ), 1, res_mem );
      mread( &z, sizeof( float ), 1, res_mem );
      verts[i].pos.Set( x, y, z );
      //      printf( "vert[%d]: %f %f %f\n", i, x, y, z );
      mread( &x, sizeof( float ), 1, res_mem );
      mread( &y, sizeof( float ), 1, res_mem );
      mread( &z, sizeof( float ), 1, res_mem );
      verts[i].normal.Set( x, y, z );
      //      printf( "normal[%d]: %f %f %f\n", i, x, y, z );
    }

  // Read the texverts
  mread( &num_tex_verts, sizeof( num_tex_verts ), 1, res_mem );
  //  printf( "num_tex_verts: %d\n", num_tex_verts );
  for( i = 0; i < num_tex_verts; i++ )
    {
      float u, v;
      mread( &u, sizeof( float ), 1, res_mem );
      mread( &v, sizeof( float ), 1, res_mem );
      if( i < num_verts )
        {
          verts[i].texvert.Set( u, v, 0.0f );
          //          printf( "texvert[%d]: %f %f\n", i, u, v );
        }
    }
#if 0
  // Read the antialiased stuff
  int numAAEdges;
  mread( &numAAEdges, sizeof( int ), 1, res_mem );
//  printf( "numAAEdges: %d\n", numAAEdges );
  for( i = 0; i < numAAEdges; i++ )
    {
      int id[3];
      mread( &id[0], sizeof( int ), 1, res_mem );
      mread( &id[1], sizeof( int ), 1, res_mem );
      mread( &id[2], sizeof( int ), 1, res_mem );
    }
#endif


  return TRUE;
}

void SetupGraphics( int width, int height )
{
  graphics = new GlideGraphics;
	if (!graphics)
	{
		MessageBox(GetFocus(), "couldn't allocate", "bad", MB_OK);
		exit(0xdeadbeef);
	}
  graphics->Open( NULL );
  graphics->Resize(width, height);
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
#if 1
//  printf( "max: %ld\n", ( long )grTexMaxAddress( GR_TMU0 ) );
  TexCacheInit( &texcache, grTexMinAddress( GR_TMU0 ), grTexMaxAddress( GR_TMU0 ),
                1024, GR_TMU0, "the cache" );
#else
  TexCacheInit( &texcache, grTexMinAddress( GR_TMU0 ), 1000000,
                1024, GR_TMU0, "the cache" );
#endif
  graphics->Clear( GLIDE_GRAPHICS_CLEAR_COLOR | GLIDE_GRAPHICS_CLEAR_DEPTH, 0.0f, 0.0f, 0.0f );
  graphics->Swap();
  graphics->Clear( GLIDE_GRAPHICS_CLEAR_COLOR | GLIDE_GRAPHICS_CLEAR_DEPTH, 0.0f, 0.0f, 0.0f );
  grCullMode( GR_CULL_NEGATIVE );
}

int ReadCookie( void )
{
  mgets( buf, 256, &resource );
  //  printf( "cookie string: \"%s\"\n", buf );
  if( strcmp( buf, "3Dfx Splash Version 1.2\n" ) != 0 )
    {
      fprintf( stderr, "wrong version number\n" );
      return 0;
    }
  return 1;
}

int ReadFrameCount( void )
{
  int retval;
  retval = ( mread( &frame_count, sizeof( frame_count ), 1, &resource ) == 1 );
  //  printf( "frame_count: %d\n", frame_count );
  return retval;
}

int ReadFrameRate( void )
{
  int retval;
  retval = ( mread( &frame_rate, sizeof( frame_rate ), 1, &resource ) == 1 );
  //  printf( "frame_rate: %d\n", frame_rate );
  return retval;
}

int totalSize = 0;

int ReadTexinfos( void )
{
  mread( &num_texinfos, sizeof( num_texinfos ), 1, &resource );
  texinfos = new GrTexInfo[num_texinfos];
  texinfo_id_to_texcache_id = new int[num_texinfos];

  if( !texinfos || !texinfo_id_to_texcache_id )
    return FALSE;

  int i;
  for( i = 0; i < num_texinfos; i++ )
    texinfo_id_to_texcache_id[i] = TEX_CACHE_NULL_ID;
  FxU32 size;

  for( i = 0; i < num_texinfos; i++ )
    {
      GrTexInfo *texinfo = &texinfos[i];
      
      mread( texinfo, sizeof( GrTexInfo ), 1, &resource );
#if 0
      printf( "texture format: %s\n", 
              ( texinfo->format == GR_TEXFMT_ARGB_4444 ) ? "GR_TEXFMT_ARGB_4444" :
              "GR_TEXFMT_RGB_565" );
#endif
      mread( &size, sizeof( size ), 1, &resource );
      texinfo->data = new unsigned char[size];
	  totalSize +=size;
      if( !texinfo->data )
        return FALSE;
#ifdef USE_GLIDE3
		texinfo->largeLodLog2 = 8-texinfo->largeLodLog2;
		texinfo->aspectRatioLog2 = 3-texinfo->aspectRatioLog2;
	  texinfo->smallLodLog2 = texinfo->largeLodLog2;   // hack on gary
#else
	  texinfo->smallLod = texinfo->largeLod;   // hack on gary
#endif // USE_GLIDE3
      mread( texinfo->data, size, 1, &resource );
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

  mread( &num_stored_materials, sizeof( num_stored_materials ), 1, &resource );
  //  printf( "num_stored_materials: %d\n", num_stored_materials );

  materials = new Material[num_stored_materials];
  if( !materials )
    return FALSE;

  int i, j;

  for( i = 0; i < num_stored_materials; i++ )
    {
      Material *material = &materials[i];
      mread( &material->num_submaterials, sizeof( int ), 1, &resource );
      //      printf( "num_submaterials: %d\n", material->num_submaterials );
      material->submaterials = new SubMaterial[material->num_submaterials];
      if( !material->submaterials )
        return FALSE;
      for( j = 0; j < material->num_submaterials; j++ )
        {
          SubMaterial *submaterial = &material->submaterials[j];
          mread( submaterial->diffuse, sizeof( submaterial->diffuse ), 1, &resource );
          mread( submaterial->specular, sizeof( submaterial->specular ), 1, &resource );
          int texture_index;
          mread( &texture_index, sizeof( texture_index ), 1, &resource );
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
  retval = ( mread( &num_stored_geometry, sizeof( num_stored_geometry ), 1, &resource ) == 1 );
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
      mread( &i, sizeof( i ), 1, &resource );
      //      printf( "geometry index: %d\n", i );
      if( i == -1 )
        break;
      assert( i < num_stored_geometry );
      meshes[i].Read( &resource );

  };
  return TRUE;
}

int ReadCommands( void )
{
	/*
  long curpos, endpos;

  curpos = ftell( fp );
  fseek( fp, 0, SEEK_END );
  endpos = ftell( fp );
  fseek( fp, curpos, SEEK_SET );
  commands_size = endpos - curpos;
	*/
	commands_size = resource.end - resource.curr;

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
  mread( commands, commands_size, 1, &resource );
  return 1;
}


void PreCalcEdgesCmd( void )
{
  int mesh_id;

  mesh_id = *( int * )cur_cmd;
  cur_cmd += sizeof( mesh_id );
  graphics->PushMatrix();
  graphics->MultMatrix( ( float * )cur_cmd );
  cur_cmd += sizeof( float[16] );

  meshes[mesh_id].PreCalcEdges();
  graphics->PopMatrix();
}



void DoGraphicsSwapCmd( void )
{
  if( skip_frame )
    return;
#ifndef DO_FRONT_BUFFER
  graphics->Swap( 1 );
#endif
  grDepthMask( FXTRUE );
  graphics->Clear( GLIDE_GRAPHICS_CLEAR_COLOR | GLIDE_GRAPHICS_CLEAR_DEPTH, 0.0f, 0.0f, 0.0f );


}



float gLightPos[3], gXformedLightPos[3];
float duck_mat[16];
int DUCK = 0;

void DoDrawGeomCmd( void )
{
	int mesh_id;

	mesh_id = *( int * )cur_cmd;
	cur_cmd += sizeof( mesh_id );
	graphics->PushMatrix();
	graphics->MultMatrix( ( float * )cur_cmd );
	///////////////////////////////////////////
	/*
	if (mesh_id == 15) // accelerated pops in
		DUCK = 1;
	if (DUCK && mesh_id == 2) // store the FX matrix
	{
		memcpy(duck_mat, cur_cmd, sizeof(float)*16);
		DUCK = 2;
	}
	*/
	///////////////////////////////////////////
	cur_cmd += sizeof( float[16] );

	//  if (mesh_id != 5)  // 4 = tm
//	if (mesh_id == 26) // quad that covers the "accelerated" quad in front of the comet
	if( !skip_frame )
		meshes[mesh_id].Draw();


	graphics->PopMatrix();
	//  getch();
}

float gXformedCameraDir[3];

void DoCameraMatrixCmd( void )
{
  float fov;

  fov = *( float * )cur_cmd;

  graphics->SetFovX( fov * 180.0f / M_PI );
  cur_cmd += sizeof( float );
  graphics->LoadIdentity();
  graphics->MultMatrix( ( float * )cur_cmd );

#define M(row,col)  ((float *)cur_cmd)[col*4+row]
  // get the camera's n vector from the camera matrix
	gXformedCameraDir[0] = M(0,2);
	gXformedCameraDir[1] = M(1,2);
	gXformedCameraDir[2] = -M(2,2);
	float mag_inv = 1.0f/(float)sqrt(gXformedCameraDir[0]*gXformedCameraDir[0] +
									 gXformedCameraDir[1]*gXformedCameraDir[1] +
									 gXformedCameraDir[2]*gXformedCameraDir[2]);
	gXformedCameraDir[0] *= mag_inv;
	gXformedCameraDir[1] *= mag_inv;
	gXformedCameraDir[2] *= mag_inv;
#undef M

  cur_cmd += sizeof( float[16] );

  float f = 200.0f*(float)sin(0.25f*(2.0f*M_PI)*(0.001f*timeGetTime()));
	gLightPos[0] = f;
//	gLightPos[2] = -150.0f;
//	gLightPos[1] = -100.0f;
	gLightPos[2] = 150.0f;
	gLightPos[1] = -100.0f;
	graphics->DrawX(gLightPos[0], gLightPos[1], gLightPos[2]);
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
	  	  
	  if (0)//!has_opacity_map)
	  {
		  grColorCombine( GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
			  GR_COMBINE_FACTOR_LOCAL_ALPHA,
			  GR_COMBINE_LOCAL_ITERATED,
			  GR_COMBINE_OTHER_TEXTURE,
			  FXFALSE );
		  
		  grAlphaCombine(GR_COMBINE_FUNCTION_LOCAL, 
			  GR_COMBINE_FACTOR_NONE,
			  GR_COMBINE_LOCAL_ITERATED, 
			  GR_COMBINE_OTHER_NONE,
			  FXFALSE );
		  grAlphaBlendFunction(GR_BLEND_ONE, GR_BLEND_ZERO, GR_BLEND_ZERO, GR_BLEND_ZERO);
	  }
	  else
	  {
		  		 
		  // texture * iterated color
		  grColorCombine( GR_COMBINE_FUNCTION_SCALE_OTHER,
			  GR_COMBINE_FACTOR_LOCAL,
			  GR_COMBINE_LOCAL_ITERATED,
			  GR_COMBINE_OTHER_TEXTURE,
			  FXFALSE );
		  
		  
		  // alpha blend on constant alpha * texture alpha
		  grAlphaCombine(GR_COMBINE_FUNCTION_SCALE_OTHER, 
			  GR_COMBINE_FACTOR_LOCAL,
			  GR_COMBINE_LOCAL_CONSTANT, 
			  GR_COMBINE_OTHER_TEXTURE,
			  FXFALSE );
		  
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
	  
  }
  else
  {
	  if (0)//!has_opacity_map)
	  {
		  grColorCombine( GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
			  GR_COMBINE_FACTOR_LOCAL_ALPHA,
			  GR_COMBINE_LOCAL_ITERATED,
			  GR_COMBINE_OTHER_CONSTANT,
			  FXFALSE );

		  grAlphaCombine(GR_COMBINE_FUNCTION_LOCAL, 
			  GR_COMBINE_FACTOR_NONE,
			  GR_COMBINE_LOCAL_ITERATED, 
			  GR_COMBINE_OTHER_NONE,
			  FXFALSE );
		  grAlphaBlendFunction(GR_BLEND_ONE, GR_BLEND_ZERO, GR_BLEND_ZERO, GR_BLEND_ZERO);
		  
	  }
	  else
	  {
		  // constant color * iterated color.
		  grColorCombine( GR_COMBINE_FUNCTION_SCALE_OTHER,
			  GR_COMBINE_FACTOR_LOCAL,
			  GR_COMBINE_LOCAL_ITERATED,
			  GR_COMBINE_OTHER_CONSTANT, 
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
		  grAlphaBlendFunction( GR_BLEND_SRC_ALPHA,
			  GR_BLEND_ZERO,
			  GR_BLEND_ZERO,
			  GR_BLEND_ZERO );
	  }
	  
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


  graphics->SetOmniLight( position, color );
  //  printf( "omni: %f %f %f\n", position[0], position[1], position[2] );
}

void ZWriteCmd( void )
{
 //   grDepthMask(*cur_cmd);
	has_opacity_map = !*cur_cmd;
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

void DrawMyTriangle(void)
{

	GrVertex v0,v1,v2;

	grColorCombine(GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_ZERO, GR_COMBINE_LOCAL_CONSTANT, GR_COMBINE_OTHER_NONE, FXFALSE);
	grAlphaBlendFunction(GR_BLEND_ONE,GR_BLEND_ONE_MINUS_SRC_ALPHA,GR_BLEND_ZERO,GR_BLEND_ZERO);

	v0.oow = v1.oow = v2.oow = 1.0f;
	grConstantColorValue(0xffff0000);

	v0.x = 320.0f - 100.0f;
	v0.y = 240.0f - 90.0f;
	
	v1.x = v0.x + 200.0f;
	v1.y = v0.y;
	
	v2.x = v0.x + 100.0f;
	v2.y = v0.y + 180.0f;

	v0.a = v1.a = v2.a = 255.0f;

	grDrawTriangle(&v0,&v1,&v2);


}



LARGE_INTEGER timer_freq;

int DoCommandLoop( void )
{
	unsigned char *start, *end;

	start = commands;
	end = commands + commands_size;
	cur_cmd = start;

	int next_frame = 0;


	if( !QueryPerformanceFrequency( &timer_freq ) )
    {
		assert( 0 );
    }

#if 0
	printf( "timer_freq: %ld %ld\n", ( long )timer_freq.HighPart, 
		( long )timer_freq.LowPart );
#endif

	LARGE_INTEGER start_time, cur_time;

#ifdef DO_FRONT_BUFFER
	graphics->RenderBuffer( GLIDE_BUFFER_FRONT );
#endif

	while( cur_cmd < end)// && DUCK!=2)
    {
		unsigned char cmd;

		cmd = *cur_cmd++;
		//      printf( "cmd: %s\n", cmd_to_string[cmd] );
		switch( cmd )
        {

		case PRECALC_EDGES_CMD:
			PreCalcEdgesCmd();
			break;

		case GRAPHICS_SWAP_LAST_FRAME_CMD:
			grBufferSwap(1);		// last frame - dont clear so fade to black works
			break;

		case GRAPHICS_SWAP_NO_TIMEBASE_CMD:
			DoGraphicsSwapCmd();   // allow the clear but don't skip the next frame (second to last frame);
			skip_frame = FALSE;
			break;

        case GRAPHICS_SWAP_CMD:
			DoGraphicsSwapCmd();
			cur_frame++;

			// Check the current time and wait if we are ahead of the 
			// desired animation rate.
#ifdef REALTIME
			do
            {
				QueryPerformanceCounter( &cur_time );

				LARGE_INTEGER big_delta;
				big_delta = LargeIntegerSubtract( cur_time, start_time );

				LARGE_INTEGER dummy_large;

				big_delta = ExtendedIntegerMultiply( big_delta, frame_rate );
				next_frame = LargeIntegerDivide( big_delta, timer_freq, &dummy_large ).LowPart;
            } while( next_frame < cur_frame );
#endif

#ifdef REALTIME
			// Make sure that we don't get behind
			if( next_frame > cur_frame )
				skip_frame = TRUE;
			else
				skip_frame = FALSE;
#else
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
			graphics->FadeToBlack();

			return 1;
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

		case START_CMD:
			// Play the sound.
//			using_sound = PlaySound( "3dfxsplash.wav", NULL, SND_FILENAME | SND_ASYNC );
			using_sound = PlaySound( (char *)IDR_WAVE1, gModule, SND_RESOURCE | SND_ASYNC );
			QueryPerformanceCounter( &start_time );
			gNormals = 0;
			cur_frame = 0;
			has_opacity_map = 0;
			break;

        default:
			fprintf( stdout, "Unknown command: 0x%x\n", ( int )cmd );
			return 0;
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

/*
*/

#ifdef SPLASH_DLL

extern "C" FX_ENTRY int FX_CALL
fxSplash( FxU32 hWind,FxU32 scrWidth,FxU32 scrHeight,FxU32 nAuxBuffers );

BOOL WINAPI DllMain(HANDLE hModule, DWORD reason_for_call, void *reserved)
{
	gModule = hModule;

	switch (reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			break;

		case DLL_PROCESS_DETACH:
			break;

		case DLL_THREAD_ATTACH:
			break;

		case DLL_THREAD_DETACH:
			break;
	}

	return TRUE;
}

FX_EXPORT int FX_CSTYLE
fxSplash( FxU32 hWind,FxU32 scrWidth,FxU32 scrHeight,FxU32 nAuxBuffers )
{
  if( nAuxBuffers == 0 )
    return 0;

	/*
  static char system_dir[MAX_PATH];

  GetSystemDirectory( system_dir, MAX_PATH );

  if( !( fp = fopen( "3dfxsplash.dat", "rb" ) ) )
    {
      char tmp[256];
      sprintf( tmp, "%s\\3dfxsplash.dat", system_dir );
      if( !( fp = fopen( tmp, "rb" ) ) )
        {
          return 0;
        }
    }
	*/

	HRSRC rsrc;

	rsrc = FindResource(gModule, (char *)IDR_ANIM1, "ANIM");
	if (!rsrc)
	{
		return 0;
	}

	resource.size = SizeofResource(gModule, rsrc);

	resource.start = (BYTE *)LoadResource(gModule, rsrc);
	if (!resource.start)
	{
		return 0;
	}

	resource.curr = resource.start;
	resource.end = resource.start + resource.size;


  SetupGraphics( scrWidth, scrHeight );

  Sleep( 1000 );

  if( !LoadSplashData() )
  {
	  graphics->Close();
//      fclose( fp );
      return 0;
  }

  // Do the main event loop to render the animation
  DoCommandLoop();

  Sleep( 1000 );

  // Make sure that sound is turned off so we don't screw up the app.
  if( using_sound )
    PlaySound( NULL, NULL, NULL );

  graphics->Close();

//  fclose( fp );
  return 1;
}
#else
int gIndex0=0, gIndex1=1, gIndex2=2;
int gInv0=0, gInv1=0, gInv2=0;

int main( int argc, char **argv )
{
	/*
	if( argc != 2 )
		return 0;
	if( !( &res_mem = fopen( argv[1], "rb" ) ) )
		return 0;
	*/

	HRSRC rsrc;

	gModule = GetModuleHandle(argv[0]);

	rsrc = FindResource(gModule, (char *)IDR_ANIM1, "ANIM");
	if (!rsrc)
	{
		return 0;
	}

	resource.size = SizeofResource(gModule, rsrc);

	resource.start = (BYTE *)LoadResource(gModule, rsrc);
	if (!resource.start)
	{
		return 0;
	}

	resource.curr = resource.start;
	resource.end = resource.start + resource.size;

	SetupGraphics( 640, 480 );

	Sleep( 1000 );

	if( !LoadSplashData() )
    {
		graphics->Close();
//		fclose( fp );
		return 0;
    }




	// Do the main event loop to render the animation
	DoCommandLoop();

	Sleep( 1000 );

/*
	gLightPos[0] = 0.0f;
	gLightPos[2] = 0.0f;
	gLightPos[1] = 0.0f;
	int done = 0;
	while (!done)
	{
		if (_kbhit())
		{
			int c = _getch();
			switch (c)
			{
				case 'q':
					done = 1;
					break;

				case 'a':
					gLightPos[0] -= 10.0f;
					break;

				case 'd':
					gLightPos[0] += 10.0f;
					break;

				case 'w':
					gLightPos[2] -= 10.0f;
					break;

				case 'x':
					gLightPos[2] += 10.0f;
					break;

				case 'e':
					gLightPos[1] -= 10.0f;
					break;

				case 'c':
					gLightPos[1] += 10.0f;
					break;

				case 'n':
					gNormals ^= 1;
					break;

				case '1':
					gIndex0 = (gIndex0+1)%3;
					break;

				case '2':
					gIndex1 = (gIndex1+1)%3;
					break;

				case '3':
					gIndex2 = (gIndex2+1)%3;
					break;

				case '4':
					gInv0 ^= 1;
					break;

				case '5':
					gInv1 ^= 1;
					break;

				case '6':
					gInv2 ^= 1;
					break;
			}
		}

		grBufferClear(0, 0, 0xffff);

		graphics->DrawX(gLightPos[0], gLightPos[1], gLightPos[2]);

		graphics->PushMatrix();
		graphics->MultMatrix(duck_mat);

		grColorCombine(GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE, GR_COMBINE_LOCAL_ITERATED, GR_COMBINE_OTHER_NONE, FXFALSE);
		grAlphaCombine(GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE, GR_COMBINE_LOCAL_ITERATED, GR_COMBINE_OTHER_NONE, FXFALSE);
		grAlphaBlendFunction(GR_BLEND_ONE, GR_BLEND_ZERO, GR_BLEND_ZERO, GR_BLEND_ZERO);

		meshes[0].Draw();
		meshes[1].Draw();
		meshes[2].Draw();
		meshes[3].Draw();

		graphics->PopMatrix();

		grBufferSwap(1);
	}

	grSstWinClose();
	grGlideShutdown();
*/

	// Make sure that sound is turned off so we don't screw up the app.
	if( using_sound )
		PlaySound( NULL, NULL, NULL );

	graphics->Close();

//	fclose( fp );
	return 1;
}
#endif
