#undef USE_SOUND
#define SOUND_FILE "3dfxsplash.wav"
#define ANIM_FILE  "3dfxsplash.dat"
#define PLUG_FILE "3dfxplug.tga"


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

#include "convexhull.h"

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
#define DIRECT_LIGHT_CMD 14


#include <largeint.h>

#ifdef PRINT_DEBUG
FILE *fp;
#endif // PRINT_DEBUG

// global variables
int gCurrMaterialID, gCurrGroupID;
FxBool gSingleFrame;
GrColorFormat_t gColorFormat;
int gNormals, gWireframe;
SplashTexture gSplashPlugTexture;

static FxU8 gCurrAlpha;

extern long gDepthMinMax[2];

GlideGraphics *graphics = NULL;
TexCache texcache;
ResMem *gAnimResource, *gPlugResource;
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

int cur_frame;
BOOL using_sound = FALSE;

int has_opacity_map;

HMODULE gModule;

// function prototypes
void DrawAAEdges();
void SetMaterial(int material_id);
int HandleCommand(unsigned char cmd);


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
  for( face = faces; face < end_face; face++ )
	{
		RevVert *vert_ptrs[3];

		vert_ptrs[0] = &verts[face->vert_ids[0]];
		vert_ptrs[1] = &verts[face->vert_ids[1]];
		vert_ptrs[2] = &verts[face->vert_ids[2]];

		graphics->DrawPolygon(3, vert_ptrs,flags | DO_DRAW);
	}
}


void Mesh::PreCalcEdges( void )
{
  Face *face, *end_face;
  end_face = faces + num_faces;

  ClearEdges();

  for( face = faces; face < end_face; face++ )
	{
		RevVert *vert_ptrs[3];

		vert_ptrs[0] = &verts[face->vert_ids[0]];
		vert_ptrs[1] = &verts[face->vert_ids[1]];
		vert_ptrs[2] = &verts[face->vert_ids[2]];

		graphics->DrawPolygon( 3, vert_ptrs,flags | ANTI_ALIAS);
	}

	num_AAEdges = ConvexHull(edge_list,edge_count);
	for (int i = 0; i < num_AAEdges; i++)
		edges[i] = edge_list[i];
}




BOOL Mesh::Read( ResMem *res_mem )
{
  // Read the flags
  mread(&flags, sizeof(flags), 1, res_mem);

	flags |= ANTI_ALIAS;
	flags |= ENVMAP;
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

FxBool SetupGraphics(HWND hwnd, int width, int height )
{
#ifdef PRINT_DEBUG
	fprintf(fp, "entered SetupGraphics\n");
	fflush(fp);
#endif // PRINT_DEBUG

  graphics = new GlideGraphics;
	if (!graphics)
		return FXFALSE;
	if (!graphics->Open( hwnd ))
		return FXFALSE;
	graphics->Resize(width, height);
	graphics->Clear( GLIDE_GRAPHICS_CLEAR_COLOR | GLIDE_GRAPHICS_CLEAR_DEPTH, 0.0f, 0.0f, 0.0f );
	graphics->MatrixMode( GLIDE_PROJECTION );
	graphics->LoadIdentity();
	graphics->Perspective( 90.0f, 1, 1.5, 20000.0f );
	graphics->Viewport( 0, 0, width, height );
	graphics->SetFovX( 90.0f );
	graphics->MatrixMode( GLIDE_MODELVIEW );
	graphics->PushMatrix();
	graphics->LoadIdentity();

  TexCacheInit( &texcache, grTexMinAddress( GR_TMU0 ), grTexMaxAddress( GR_TMU0 ), 1024, GR_TMU0, "the cache" );

#ifdef PRINT_DEBUG
	fprintf(fp, "SetupGraphics succeeded!!!\n");
	fflush(fp);
#endif // PRINT_DEBUG

	return FXTRUE;
}

void CleanupGraphics()
{
	if (graphics)
	{
		graphics->Close();
		delete graphics;
		graphics = NULL;
	}
}

int ReadCookie( void )
{
  mgets( buf, 256, gAnimResource );
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
  retval = ( mread( &frame_count, sizeof( frame_count ), 1, gAnimResource ) == 1 );
  //  printf( "frame_count: %d\n", frame_count );
  return retval;
}

int ReadFrameRate( void )
{
  int retval;
  retval = ( mread( &frame_rate, sizeof( frame_rate ), 1, gAnimResource ) == 1 );
  //  printf( "frame_rate: %d\n", frame_rate );
  return retval;
}

int totalSize = 0;

int ReadTexinfos( void )
{
  mread( &num_texinfos, sizeof( num_texinfos ), 1, gAnimResource );
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
      
      mread( texinfo, sizeof( GrTexInfo ), 1, gAnimResource );
#if 0
      printf( "texture format: %s\n", 
              ( texinfo->format == GR_TEXFMT_ARGB_4444 ) ? "GR_TEXFMT_ARGB_4444" :
              "GR_TEXFMT_RGB_565" );
#endif
      mread( &size, sizeof( size ), 1, gAnimResource );
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
      mread( texinfo->data, size, 1, gAnimResource );
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

  mread( &num_stored_materials, sizeof( num_stored_materials ), 1, gAnimResource );
  //  printf( "num_stored_materials: %d\n", num_stored_materials );

  materials = new Material[num_stored_materials];
  if( !materials )
    return FALSE;

  int i, j;

  for( i = 0; i < num_stored_materials; i++ )
    {
      Material *material = &materials[i];
      mread( &material->num_submaterials, sizeof( int ), 1, gAnimResource );
      //      printf( "num_submaterials: %d\n", material->num_submaterials );
      material->submaterials = new SubMaterial[material->num_submaterials];
      if( !material->submaterials )
        return FALSE;
      for( j = 0; j < material->num_submaterials; j++ )
        {
          SubMaterial *submaterial = &material->submaterials[j];
          mread( submaterial->diffuse, sizeof( submaterial->diffuse ), 1, gAnimResource );
          mread( submaterial->specular, sizeof( submaterial->specular ), 1, gAnimResource );
          int texture_index;
          mread( &texture_index, sizeof( texture_index ), 1, gAnimResource );
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
  retval = ( mread( &num_stored_geometry, sizeof( num_stored_geometry ), 1, gAnimResource ) == 1 );
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
      mread( &i, sizeof( i ), 1, gAnimResource );
      //      printf( "geometry index: %d\n", i );
      if( i == -1 )
        break;
      assert( i < num_stored_geometry );
      meshes[i].Read( gAnimResource );

  };
  return TRUE;
}

int ReadCommands( void )
{
#ifdef READ_FROM_FILE
  long curpos, endpos;

  curpos = ftell( gAnimResource );
  fseek( gAnimResource, 0, SEEK_END );
  endpos = ftell( gAnimResource );
  fseek( gAnimResource, curpos, SEEK_SET );
  commands_size = endpos - curpos;
#else
	commands_size = gAnimResource->end - gAnimResource->curr;
#endif // READ_FROM_FILE

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
  mread( commands, commands_size, 1, gAnimResource );
  return 1;
}


void PreCalcEdgesCmd( void )
{
  int mesh_id;

  mesh_id = *( int * )cur_cmd;
  cur_cmd += sizeof( mesh_id );
  graphics->PushMatrix();
  graphics->MultMatrix( ( float * )cur_cmd );
  cur_cmd += 16*sizeof( float );

  meshes[mesh_id].PreCalcEdges();
  graphics->PopMatrix();
}



void DoGraphicsSwapCmd( void )
{
	float fade;

  if( skip_frame )
    return;

	if (!gSingleFrame)
	{
		const int start_fade_in_frame = 24;
		const int end_fade_in_frame = 32;
		if (cur_frame < end_fade_in_frame)
		{
			if (cur_frame < start_fade_in_frame)
				fade = 1.0f;
			else
			{
				fade = 1.0f - (float)(cur_frame - start_fade_in_frame)/(float)(end_fade_in_frame - start_fade_in_frame);
			}
			graphics->FadeIn(fade, 0.0f, 0.0f, 0.0f);
		}
	}

	DrawAAEdges();

#ifndef DO_FRONT_BUFFER
	if (!gSingleFrame)
	  graphics->Swap( 1 );
#endif

  grDepthMask( FXTRUE );
	if (!gSingleFrame)
	  graphics->Clear( GLIDE_GRAPHICS_CLEAR_COLOR | GLIDE_GRAPHICS_CLEAR_DEPTH, 0.0f, 0.0f, 0.0f );
}



float duck_mat[16];
int DUCK = 0;

void DoDrawGeomCmd( void )
{
	int mesh_id, group_id;
	float *matrix;

	mesh_id = *( int * )cur_cmd;
	cur_cmd += sizeof( mesh_id );

	group_id = *(int *)cur_cmd;
	cur_cmd += sizeof(group_id);

	matrix = (float *)cur_cmd;
	cur_cmd += 16*sizeof(float);

	// if the group of objects has changed, draw the previous
	// group's AA edges and clear the edge list for this new group
	if (group_id != gCurrGroupID)
	{
		gCurrGroupID = group_id;
		DrawAAEdges();
	}

	graphics->PushMatrix();
	graphics->MultMatrix( matrix );
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

	// hack
	//  if (mesh_id != 5)  // 4 = tm
//	if (mesh_id == 26) // quad that covers the "accelerated" quad in front of the comet
	if( !skip_frame )
	{
		meshes[mesh_id].Draw();
	}

	graphics->PopMatrix();
	//  getch();
}

float gXformedCameraDir[3];

void DoCameraMatrixCmd( void )
{
  float fov;

  fov = *( float * )cur_cmd;

#ifdef PRINT_DEBUG
	fprintf(fp, "fov from file = %f\n", fov);
	fflush(fp);
#endif // PRINT_DEBUG

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

  cur_cmd += 16*sizeof(float);
}

void DrawAAEdges()
{
	int i, store_material_id, curr_material_id;
  AAEdge *edgePtr;

	if (!edge_count)
		return;

	store_material_id = gCurrMaterialID;
	curr_material_id = -1;

//	edge_count = ConvexHull(edge_list, edge_count);

#ifdef USE_GLIDE3
	grEnable(GR_AA_ORDERED);
#endif

	// draw the AA lines
	for (i=0; i<edge_count; i++)
	{
		edgePtr = &edge_list[i];

		// set the material if it changed
		if (edgePtr->material_id != curr_material_id)
		{
			curr_material_id = edgePtr->material_id;
			SetMaterial(curr_material_id);
		}

		// draw the aa line
#ifdef USE_GLIDE3
		grDrawLine(&edgePtr->v0, &edgePtr->v1);
#else
		grAADrawLine(&edgePtr->v0, &edgePtr->v1);
#endif // USE_GLIDE3
	}

#ifdef USE_GLIDE3
	grDisable(GR_AA_ORDERED);
#endif

	// restore the material
	SetMaterial(store_material_id);

	ClearEdges();
}

void SetMaterialCmd( void )
{
  int material_id;

  material_id = *( int * )cur_cmd;
  cur_cmd += sizeof( int );

//	if (material_id == gCurrMaterialID) // no changes
//		return;

  //  printf( "material_id: %d\n", material_id );

  if( skip_frame )
    return;

	SetMaterial(material_id);
}

void SetMaterial(int material_id)
{
  Material *material;
	FxU8 red, green, blue;

	gCurrMaterialID = material_id;

	material = &materials[material_id];
	if (!gSingleFrame && material->submaterials[0].texcache_id != TEX_CACHE_NULL_ID)
	{
		TexCacheSetCurrent(&texcache, material->submaterials[0].texcache_id, FXFALSE);

		// texture * iterated color
		grColorCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
									 GR_COMBINE_FACTOR_LOCAL,
									 GR_COMBINE_LOCAL_ITERATED,
									 GR_COMBINE_OTHER_TEXTURE,
									 FXFALSE);

		// alpha blend on iterated alpha * texture alpha
		grAlphaCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
									 GR_COMBINE_FACTOR_LOCAL,
									 GR_COMBINE_LOCAL_ITERATED,
									 GR_COMBINE_OTHER_TEXTURE,
									 FXFALSE);
	}
	else
	{
		// constant color * iterated color.
		grColorCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
									 GR_COMBINE_FACTOR_LOCAL,
									 GR_COMBINE_LOCAL_ITERATED,
									 GR_COMBINE_OTHER_CONSTANT,
									 FXFALSE);

		// alpha blend on iterated alpha
		grAlphaCombine(GR_COMBINE_FUNCTION_LOCAL,
									 GR_COMBINE_FACTOR_NONE,
									 GR_COMBINE_LOCAL_ITERATED,
									 GR_COMBINE_OTHER_NONE,
									 FXFALSE);
  }

	grAlphaBlendFunction(GR_BLEND_SRC_ALPHA, GR_BLEND_ONE_MINUS_SRC_ALPHA, GR_BLEND_ZERO, GR_BLEND_ZERO);

	red = (FxU8)(255.0f*material->submaterials[0].diffuse[0]);
	green = (FxU8)(255.0f*material->submaterials[0].diffuse[1]);
	blue = (FxU8)(255.0f*material->submaterials[0].diffuse[2]);

	// set the constant color value based on the color format
	if (gColorFormat == GR_COLORFORMAT_ARGB)
	{
		grConstantColorValue((gCurrAlpha<<24) | (red<<16) | (green<<8) | (blue));
	}
	else if (gColorFormat == GR_COLORFORMAT_ABGR)
	{
		grConstantColorValue((gCurrAlpha<<24) | (blue<<16) | (green<<8) | (red));
	}
	else if (gColorFormat == GR_COLORFORMAT_RGBA)
	{
		grConstantColorValue((red<<24) | (green<<16) | (blue<<8) | (gCurrAlpha));
	}
	else if (gColorFormat == GR_COLORFORMAT_BGRA)
	{
		grConstantColorValue((blue<<24) | (green<<16) | (red<<8) | (gCurrAlpha));
	}
}

void OmniLightCmd( void )
{
  float color[3];
  float intensity;
  float position[3];

  memcpy( color, cur_cmd, sizeof( color ) );
	int ii = sizeof(color);
  cur_cmd += sizeof( color );
  intensity = *( float * )cur_cmd;
  cur_cmd += sizeof( float );
  memcpy( position, cur_cmd, sizeof( position ) );
  cur_cmd += sizeof( position );

  graphics->SetOmniLight( position, color );
  //  printf( "omni: %f %f %f\n", position[0], position[1], position[2] );
}

void DirLightCmd( void )
{
  float color[3];
  float intensity;
  float direction[3];

  memcpy( color, cur_cmd, sizeof( color ) );
  cur_cmd += sizeof( color );
  intensity = *( float * )cur_cmd;
  cur_cmd += sizeof( float );
  memcpy( direction, cur_cmd, sizeof( direction ) );
  cur_cmd += sizeof( direction );

  graphics->SetDirLight( direction, color );
  //  printf( "dir: %f %f %f\n", direction[0], direction[1], direction[2] );
}

void ZWriteCmd( void )
{
 //   grDepthMask(*cur_cmd);
	has_opacity_map = !*cur_cmd;
	cur_cmd += 1;
}


void XparencyCmd( void )
{
	if( ( *( FxU32 * )cur_cmd == 0 ) || ( *( FxU32 * )cur_cmd == 0xff000000 ) )
	{
		gCurrAlpha = 255;
	}
	else
	{
		gCurrAlpha = (FxU8)((*(FxU32 *)cur_cmd)>>24);
	}

  cur_cmd += sizeof( FxU32 );
}

// draws the nth frame number
// n must be a number between 0 and frame_count-1
int DrawNthFrame(int n)
{
	unsigned char cmd, *start, *end;

	start = commands;
	end = commands + commands_size;
	cur_cmd = start;

	cur_frame = 0;
	while (cur_frame != n && cur_cmd < end)
	{
		cmd = *cur_cmd++;
		switch (cmd)
		{
			case PRECALC_EDGES_CMD:
				cur_cmd += sizeof(int) + 16*sizeof(float);
				break;

			case GRAPHICS_SWAP_LAST_FRAME_CMD:
				break;

			case GRAPHICS_SWAP_NO_TIMEBASE_CMD:
				break;

			case GRAPHICS_SWAP_CMD:
				cur_frame++;
				break;

			case DRAW_GEOM_CMD:
				cur_cmd += sizeof(int) + sizeof(int) + 16*sizeof(float);
				break;

			case CAMERA_MATRIX_CMD:
				cur_cmd += sizeof(float) + 16*sizeof(float);
				break;

			case SET_MATERIAL_CMD:
				cur_cmd += sizeof(int);
				break;

			case EOF_CMD:
				graphics->FadeToBlack();
				return 1;
				break;

			case OMNI_LIGHT_CMD:
				cur_cmd += 3*sizeof(float) + sizeof(float) + 3*sizeof(float);
				break;

			case DIRECT_LIGHT_CMD:
				cur_cmd += 3*sizeof(float) + sizeof(float) + 3*sizeof(float);
				break;

			case Z_WRITE_CMD:
				cur_cmd += 1;
				break;

			case XPARENCY_CMD:
				cur_cmd += sizeof(FxU32);
				break;

			case START_CMD:
				cur_frame = 0;
				break;

			default:
				fprintf( stdout, "Unknown command: 0x%x\n", ( int )cmd );
				return 0;
				break;
		}
	}

	// handle all the commands for this frame
	while (cur_cmd < end)
	{
		cmd = *cur_cmd++;
		if (cmd == GRAPHICS_SWAP_CMD)
		{
			skip_frame = FALSE;
			DoGraphicsSwapCmd();
			return 1;
		}
		else
		{
			if (!HandleCommand(cmd))
			{
				return 1;
			}
		}
	}

	return 1;
}

LARGE_INTEGER timer_freq, start_time, cur_time;

int HandleCommand(unsigned char cmd)
{
	int next_frame;

	switch (cmd)
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

			// Make sure that we don't get behind
			if( next_frame > cur_frame )
				skip_frame = TRUE;
			else
				skip_frame = FALSE;
#else
			skip_frame = FALSE;
#endif // REALTIME
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
			return 0;
			break;

		case OMNI_LIGHT_CMD:
			OmniLightCmd();
			break;

		case DIRECT_LIGHT_CMD:
			DirLightCmd();
			break;

		case Z_WRITE_CMD:
			ZWriteCmd();
			break;

		case XPARENCY_CMD:
			XparencyCmd();
			break;

		case START_CMD:
#ifdef USE_SOUND
			// Play the sound.
#ifdef READ_FROM_FILE
			using_sound = PlaySound( SOUND_FILE, NULL, SND_FILENAME | SND_ASYNC );
#else
			using_sound = PlaySound( (char *)IDR_WAVE1, gModule, SND_RESOURCE | SND_ASYNC );
#endif // READ_FROM_FILE
#endif // USE_SOUND
			QueryPerformanceCounter( &start_time );
			gNormals = 0;
			gWireframe = 0;
			cur_frame = 0;
			has_opacity_map = 0;
			break;

		default:
			fprintf( stdout, "Unknown command: 0x%x\n", ( int )cmd );
			return 0;
			break;
	}

	return 1;
}

int DoCommandLoop( void )
{
	unsigned char cmd, *start, *end;

	start = commands;
	end = commands + commands_size;
	cur_cmd = start;

 	if( !QueryPerformanceFrequency( &timer_freq ) )
	{
		assert( 0 );
	}

#ifdef DO_FRONT_BUFFER
	graphics->RenderBuffer( GLIDE_BUFFER_FRONT );
#endif

	while (cur_cmd < end)
	{
		cmd = *cur_cmd++;
		if (!HandleCommand(cmd))
		{
			break;
		}
	}

	return 1;
}

int LoadSplashData( void )
{
#ifdef READ_FROM_FILE
	gAnimResource = fopen(ANIM_FILE, "rb");
	if (!gAnimResource)
	{
		return 0;
	}

	gPlugResource = fopen(PLUG_FILE, "rb");
	if (!gPlugResource)
	{
		return 0;
	}
#else // READ_FROM_FILE
	HRSRC rsrc;

	rsrc = FindResource(gModule, (char *)IDR_ANIM1, "ANIM");
	if (!rsrc)
	{
		return 0;
	}

	gAnimResource = new ResMem;
	if (!gAnimResource)
	{
		return 0;
	}

	gAnimResource->size = SizeofResource(gModule, rsrc);

	gAnimResource->start = (BYTE *)LoadResource(gModule, rsrc);
	if (!gAnimResource->start)
	{
		return 0;
	}

	gAnimResource->curr = gAnimResource->start;
	gAnimResource->end = gAnimResource->start + gAnimResource->size;

	rsrc = FindResource(gModule, (char *)IDR_PLUG1, "PLUG");
	if (!rsrc)
	{
		return 0;
	}

	gPlugResource = new ResMem;
	if (!gPlugResource)
	{
		return 0;
	}

	gPlugResource->size = SizeofResource(gModule, rsrc);

	gPlugResource->start = (BYTE *)LoadResource(gModule, rsrc);
	if (!gPlugResource->start)
	{
		return 0;
	}

	gPlugResource->curr = gPlugResource->start;
	gPlugResource->end = gPlugResource->start + gPlugResource->size;
#endif // READ_FROM_FILE

	gSplashPlugTexture.data = NULL;
	ClearEdges();
	gCurrMaterialID = -1;
	gCurrGroupID = -1;

	if (!LoadSplashTextureTGA(gPlugResource, &gSplashPlugTexture))
		return 0;

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

#ifdef READ_FROM_FILE
  fclose( gAnimResource );
#else
	delete gAnimResource;
	gAnimResource = NULL;
#endif // READ_FROM_FILE

  return TRUE;
}

void CleanupSplashData()
{
	// Make sure that sound is turned off so we don't screw up the app.
#ifdef USE_SOUND
	if( using_sound )
		PlaySound( NULL, NULL, NULL );
#endif //USE_SOUND

	if (gSplashPlugTexture.data)
	{
		delete [] gSplashPlugTexture.data;
		gSplashPlugTexture.data = NULL;
	}
}

/*
*/

#ifdef SPLASH_DLL

#ifdef __cplusplus
extern "C" {
#endif
FX_ENTRY FxBool FX_CSTYLE fxSplashInit(FxU32 hWnd, FxU32 screenWidth, FxU32 screenHeight, FxU32 numColBuf, FxU32 numAuxBuf, GrColorFormat_t colorFormat);
FX_ENTRY void FX_CSTYLE fxSplashShutdown(void);
FX_ENTRY void FX_CSTYLE fxSplash(float x, float y, float w, float h, FxU32 frameNumber);
FX_ENTRY void* FX_CSTYLE fxSplashPlug(FxU32* w, FxU32* h, FxU32* strideInBytes, GrLfbWriteMode_t* format);
#ifdef __cplusplus
};
#endif

BOOL WINAPI DllMain(HANDLE hModule, DWORD reason_for_call, void *reserved)
{
	gModule = (HMODULE)hModule;

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

FX_EXPORT FxBool FX_CSTYLE fxSplashInit(FxU32 hWnd, FxU32 screenWidth, FxU32 screenHeight, FxU32 numColBuf, FxU32 numAuxBuf, GrColorFormat_t colorFormat)
{
#ifdef PRINT_DEBUG
	fp = fopen("c:\\splash.txt", "w");

#ifdef USE_GLIDE3
	fprintf(fp, "Glide3x\n");
#else
	fprintf(fp, "Glide2x\n");
#endif // USE_GLIDE3

	fprintf(fp, "entered fxSplashInit(%d, %d, %d, %d, %d %d)\n", hWnd, screenWidth, screenHeight, numColBuf, numAuxBuf, colorFormat);
	fflush(fp);
#endif // PRINT_DEBUG

	// must have a depth buffer
  if (numAuxBuf == 0)
	{
    return FXFALSE;
	}

	// must have at least 2 buffers so we can swap
	if (numColBuf < 2)
	{
		return FXFALSE;
	}

	gColorFormat = colorFormat;

	graphics = NULL;
  if (!SetupGraphics((HWND)hWnd, screenWidth, screenHeight))
	{
		return FXFALSE;
	}

	// let the monitor warm up
	Sleep(1000);

  if( !LoadSplashData() )
  {
		CleanupGraphics();
		CleanupSplashData();
		return FXFALSE;
  }

#ifdef PRINT_DEBUG
	fprintf(fp, "fxSplashInit succeeded!!!!\n");
	fflush(fp);
#endif // PRINT_DEBUG

  return FXTRUE;
}

FX_EXPORT void FX_CSTYLE fxSplashShutdown(void)
{
#ifdef PRINT_DEBUG
	fprintf(fp, "entered fxSplashShutdown()\n");
	fflush(fp);
#endif // PRINT_DEBUG

	CleanupGraphics();
	CleanupSplashData();
}

FX_EXPORT void FX_CSTYLE fxSplash(float x, float y, float w, float h, FxU32 frameNumber)
{
#ifdef PRINT_DEBUG
	fprintf(fp, "entered fxSplash(%f, %f, %f, %f, %d)\n", x, y, w, h, frameNumber);
	fflush(fp);
#endif // PRINT_DEBUG

	graphics->StoreGlideState();

	graphics->Viewport((int)x, (int)y, (int)w, (int)h);
	graphics->SetSplashGlideState();

	// frameNumber starts from 1, so I need to subtract 1
	// also, mod it so it wraps in case of a huge number
	if (frameNumber)
	{
		gSingleFrame = FXTRUE;
		frameNumber = (frameNumber-1)%frame_count;
		grColorMask(FXFALSE, FXFALSE);
		grBufferClear(0, 0, gDepthMinMax[1]);
		grColorMask(FXTRUE, FXFALSE);
		DrawNthFrame(frameNumber);
	}
	else
	{
		gSingleFrame = FXFALSE;
		DoCommandLoop();
	}

	graphics->RestoreGlideState();
}

FX_EXPORT void *FX_CSTYLE fxSplashPlug(FxU32 *w, FxU32 *h, FxU32 *strideInBytes, GrLfbWriteMode_t *format)
{
#ifdef PRINT_DEBUG
	fprintf(fp, "entered fxSplashPlug()\n");
	fflush(fp);
#endif // PRINT_DEBUG

	if (gSplashPlugTexture.data)
	{
		*w = gSplashPlugTexture.width;
		*h = gSplashPlugTexture.height;
		*strideInBytes = gSplashPlugTexture.stride;
		*format = GR_LFBWRITEMODE_565;
		return gSplashPlugTexture.data;
	}

	return NULL;
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

	gModule = GetModuleHandle(argv[0]);

	if (!SetupGraphics(GetDesktopWindow(), 640, 480 ))
		return 0;

	Sleep( 1000 );

	if( !LoadSplashData() )
	{
		graphics->Close();
//		fclose( fp );
		return 0;
	}

	graphics->SetSplashGlideState();

	// Do the main event loop to render the animation
	DoCommandLoop();
	/*
	graphics->Viewport(200, 200, 200, 200);
	int frame = 0;
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

				case 'i':
					frame--;
					if (frame < 0) frame = 0;
					break;

				case 'o':
					frame++;
					if (frame > frame_count-1) frame = frame_count-1;
					break;
			}
			printf("frame: %d\n", frame);
			DrawNthFrame(frame);
		}
	}
	Sleep( 1000 );
	*/

	/*
	int mesh_id, id0, id1;
	float mat[4][4];

	mat[0][0] = 1.0f;
	mat[0][1] = 0.0f;
	mat[0][2] = 0.0f;
	mat[0][3] = 0.0f;

	mat[1][0] = 0.0f;
	mat[1][1] = 1.0f;
	mat[1][2] = 0.0f;
	mat[1][3] = 0.0f;

	mat[2][0] = 0.0f;
	mat[2][1] = 0.0f;
	mat[2][2] = 1.0f;
	mat[2][3] = 0.0f;

	mat[3][0] = 0.0f;
	mat[3][1] = 0.0f;
	mat[3][2] = 0.0f;
	mat[3][3] = 1.0f;

	float dx = 0.0f;
	float dy = 0.0f;
	float dz = -200.0f;
	float rot = 0.0f;

	id0 = 0;
	id1 = num_stored_geometry-1;
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
					dx -= 10.0f;
					break;

				case 'd':
					dx += 10.0f;
					break;

				case 'e':
					dy += 10.0f;
					break;

				case 'c':
					dy -= 10.0f;
					break;

				case 'w':
					dz += 10.0f;
					break;

				case 'x':
					dz -= 10.0f;
					break;

				case 'i':
					rot += 2.0f;
					break;

				case 'o':
					rot -= 2.0f;
					break;

				case 'n':
					gNormals ^= 1;
					break;

				case 'm':
					gWireframe ^= 1;
					break;

				case 'h':
					id0--;
					if (id0 < 0) id0 = 0;
					break;

				case 'j':
					id0++;
					if (id0 > num_stored_geometry-1) id0 = num_stored_geometry-1;
					break;

				case 'y':
					id1--;
					if (id1 < 0) id1 = 0;
					break;

				case 'u':
					id1++;
					if (id1 > num_stored_geometry-1) id1 = num_stored_geometry-1;
					break;
			}
		}
		else
		{
			printf("meshes: %d - %d (%d)\n", id0, id1, num_stored_geometry);
			grBufferClear(0, 0, 0xffff);

			grColorCombine(GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE, GR_COMBINE_LOCAL_CONSTANT, GR_COMBINE_OTHER_NONE, FXFALSE);
			grAlphaBlendFunction(GR_BLEND_ONE, GR_BLEND_ZERO, GR_BLEND_ONE, GR_BLEND_ZERO);
			grCullMode(GR_CULL_DISABLE);

			for (mesh_id=0; mesh_id<num_stored_geometry; mesh_id++)
			{
				graphics->PushMatrix();
				graphics->MultMatrix( ( float * )&mat[0][0] );
				graphics->Translate(dx, dy, dz);
				graphics->Rotate(rot, 0, 1, 0);

				grConstantColorValue(0xff7f0000);

				meshes[mesh_id].Draw();
				graphics->PopMatrix();
			}
			for (mesh_id=id0; mesh_id<=id1; mesh_id++)
			{
				graphics->PushMatrix();
				graphics->MultMatrix( ( float * )&mat[0][0] );
				graphics->Translate(dx, dy, dz);
				graphics->Rotate(rot, 0, 1, 0);

				grConstantColorValue(0xff00ff00);

				meshes[mesh_id].Draw();
				graphics->PopMatrix();
			}
			grBufferSwap(1);
		}
	}
	*/

	CleanupGraphics();
	CleanupSplashData();

	return 1;
}
#endif
