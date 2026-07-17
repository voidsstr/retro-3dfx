#include "util.h"
#include "utilapi.h"

#include <bmmlib.h>
#include "GlideGraphics.h"
#include "texcache.h"
#include <texus.h>
#include "resource.h"

#include <math.h>

#include <assert.h>
#define ASSERT assert

#ifndef M_PI
#define M_PI 3.14159f
#endif

//#define CAMERA_SET_CMD 1
#define GRAPHICS_SWAP_CMD 2
#define DRAW_GEOM_CMD 3
#define CAMERA_MATRIX_CMD 4
#define EOF_CMD 5
#define SET_MATERIAL_CMD 6
#define OMNI_LIGHT_CMD 7
#define Z_WRITE_CMD 8
#define XPARENCY_CMD 9

#define SPLASH_CLASS_ID             0x8fbd04ef

#define MAX_GEOM 1024
#define MAX_LIGHTS 1024
#define MAX_MATERIALS 1024
#define MAX_TEXTURES 1024

GlideGraphics *graphics = NULL;

static TimeValue global_time;

static FILE *debug_fp = NULL;

static FILE *write_fp = NULL;

class SplashMaterial;

static float smult, tmult;

unsigned int constant_color_val;

static int do_write = FALSE;

class SplashLight
{
public:
  SplashLight() { node = NULL; }
  INode *node;
  void Set( INode *node_ ) { node = node_; }
};

static SplashLight stored_lights[MAX_LIGHTS];
static int num_stored_lights;

class SplashGeometry
{
public:
  SplashGeometry() { node = NULL; material = NULL; complex_material = FALSE; material_index = -1; }
  float GetSortZVal( void );
  INode *node;
  SplashMaterial *material;
  void Set( INode *node_ );
  BOOL complex_material;
  int material_index;
};

static SplashGeometry stored_geometry[MAX_GEOM];
static int num_stored_geometry = 0;

static GrTexInfo texinfos[MAX_TEXTURES];
static int num_texinfos = 0;
static int texcache_id_to_texinfo_id[MAX_TEXTURES];

class NullView: public View 
{
public:
  Point2 ViewToScreen(Point3 p) { return Point2(p.x,p.y); }
  NullView() { worldToView.IdentityMatrix(); screenW=640.0f; screenH = 480.0f; }
};

class SplashUtil : public UtilityObj 
{
public:
  IUtil *iu;
  Interface *ip;
  HWND hPanel;
  TimeValue last_geometry_update;
  TimeValue last_material_update;
  TexCache texcache;
        
  SplashUtil();
  void SetupAnimation( INode *node, TimeValue time );
  void SetupAnimationCamera( ViewExp *view, TimeValue time );
  void UpdateLights( TimeValue time );
  void WriteTextures( void );
  void UpdateMaterials( TimeValue time );
  void UpdateMaterialsQuick( TimeValue time );
  void UpdateMaterialsRecursive( Mtl *max_material, TimeValue time, 
                                 SplashMaterial *material, int depth );
  void UpdateMaterial( Mtl *max_material, TimeValue time, 
                       SplashMaterial *material );
  void UpdateMaterialQuick( Mtl *max_material, TimeValue time, 
                       SplashMaterial *material );
        
  BOOL OpenFile();
  void CloseFile();

  void WriteMesh( Mesh *mesh );
  void WriteStoredGeometry( TimeValue time );
  void WriteStoredMaterials( void );
  void SplashTest();
  void SplashWrite();
  void DisplayGeometry( TimeValue time );
  void DisplayGeometryNode( TimeValue time, int geom_id );
  void RenderMesh( Mesh *mesh );

  void BeginEditParams(Interface *ip,IUtil *iu);
  void EndEditParams(Interface *ip,IUtil *iu);
  void DeleteThis() {}
                                
  void Init(HWND hWnd);
  void Destroy(HWND hWnd);
  void CreateNewFloater();
};
static SplashUtil theSplashUtil;

inline void SetLeftHanded( Matrix3 & matrix) 
{
  Matrix3         flip( TRUE );

  flip.SetRow( 2, Point3( 0.0f, 0.0f, -1.0f ) );
  matrix *= flip;
}

static inline void ConvertMatrix3ToGL( float glmat[16], const Matrix3& maxmat )
{
  const MRow *maxdata = maxmat.GetAddr();
  int row, col;

  for( col = 0; col < 3; col++ )
    {
      for( row = 0; row < 4; row++ )
        {
          glmat[row * 4 + col] = maxdata[row][col];
        }
    }
  glmat[3] = 0.0f;
  glmat[7] = 0.0f;
  glmat[11] = 0.0f;
  glmat[15] = 1.0f;
}

class SplashCamera
{
private:
  float gl_camera_matrix[16];
  float hither;
  float yon;
  float fov;
  INode *node;
  TimeValue last_update;
public:
  void Write();
  SplashCamera();
  SplashCamera( INode *node_ );
  SplashCamera( Matrix3 &inverse_transform, float hither, float yon );
  void Set( Matrix3 &inverse_transform, float hither, float yon );
  void Set( INode *node );
  void Update( TimeValue time );
  void MultMatrix( void );
};

SplashCamera::SplashCamera()
{
  node = NULL;
  last_update = TIME_NegInfinity;
}

SplashCamera::SplashCamera( INode *node_ )
{
  node = node_;
  last_update = TIME_NegInfinity;
}

SplashCamera::SplashCamera( Matrix3 &inverse_transform, float hither, float yon )
{
  Set( inverse_transform, hither, yon );
  node = NULL;
  last_update = TIME_NegInfinity;
}

void SplashCamera::Set( Matrix3& inverse_transform, float hither, float yon )
{
  //  Matrix3 camera_transform = Inverse( inverse_transform );
  Matrix3 camera_transform = inverse_transform;
  SetLeftHanded( camera_transform );
  ConvertMatrix3ToGL( gl_camera_matrix, camera_transform );
}

void SplashCamera::Set( INode *node_ )
{
  node = node_;
}

void SplashCamera::Write( void )
{
  unsigned char cmd = CAMERA_MATRIX_CMD;
  fwrite( &cmd, sizeof( cmd ), 1, write_fp );
  fwrite( &fov, sizeof( &fov ), 1, write_fp );
  fwrite( gl_camera_matrix, sizeof( gl_camera_matrix ), 1, write_fp );
}

void SplashCamera::MultMatrix()
{
#if 0
  graphics->MatrixMode( REV_PROJECTION );
  // hack
  graphics->Perspective( 90.0f, 1, 1.5, 20000.0f );
  graphics->Viewport( 0, 0, 640.0f, 480.0f );
#endif
  graphics->MatrixMode( GLIDE_MODELVIEW );
  graphics->MultMatrix( gl_camera_matrix );
}

void SplashCamera::Update( TimeValue time )
{
  // The camera is animated only if it has a node.
  if( node )
    {
      Interval camera_valid;
      CameraState camera_state;
      CameraObject *camera_object = ( CameraObject * )node->GetObjectRef();
      camera_object->EvalCameraState( time, camera_valid, &camera_state );

      // need to set last_camera_update
      // If the camera from last time is still valid, then leave it alone.
      if( camera_valid.InInterval( last_update ) )
        return;

      Matrix3 camera_transform = node->GetObjTMAfterWSM( time );
      fov = camera_object->GetFOV( time );
      graphics->SetFovX( fov * 180.0f / M_PI );
      SetLeftHanded( camera_transform );
      Matrix3 blah_transform = Inverse( camera_transform );
      //      ConvertMatrix3ToGL( gl_camera_matrix, camera_transform );
      ConvertMatrix3ToGL( gl_camera_matrix, blah_transform );
#if 0
      if( do_write )
        {
          unsigned char tmp;
          tmp = CAMERA_SET_CMD;
          fwrite( &tmp, sizeof( tmp ), 1, write_fp );
          fwrite( &fov, sizeof( fov ), 1, write_fp );
          fwrite( gl_camera_matrix, sizeof( gl_camera_matrix ), 1, write_fp );
        }
#endif
    }
  last_update = time;
}

class SplashTexHandle : public TexHandle
{
public:
  TexCacheId_t texcache_id;

public:
  SplashTexHandle( TexCache *texcache, GrTexInfo *texinfo );

  virtual DWORD GetHandle() { return ( DWORD )texcache_id; }
  virtual void DeleteThis() {}
};

SplashTexHandle::SplashTexHandle( TexCache *texcache, GrTexInfo *texinfo )
{
  texcache_id = TexCacheInsertTexture( texcache, texinfo );
  texcache_id_to_texinfo_id[texcache_id] = texinfo - texinfos;
}

class SplashTexMaker : public TexHandleMaker 
{
public:
  virtual TexHandle *CreateHandle( Bitmap *bm, int sym_flags = 0 );
  virtual TexHandle *CreateHandle( BITMAPINFO *bmi, int sym_flags = 0 );
  // returned the desired texture size for procedural textures
  virtual int Size() { return 256; }
};

static BOOL has_opacity_map = FALSE;

TexHandle *SplashTexMaker::CreateHandle( Bitmap *bitmap, int sym_flags )
{
  TexHandle *handle;
  BITMAPINFO *bitmap_info;
  
  bitmap_info = bitmap->ToDib( 32 );
  handle = CreateHandle( bitmap_info, sym_flags );
  // hack. . memory leak
  //  delete bitmap_info;
  return handle;
}

static void Convert3dfInfoToTexInfo( Gu3dfInfo *tdfinfo, GrTexInfo *texinfo )
{
  texinfo->smallLod     = tdfinfo->header.small_lod;
  texinfo->largeLod     = tdfinfo->header.large_lod;
  texinfo->aspectRatio  = tdfinfo->header.aspect_ratio;
  texinfo->format       = tdfinfo->header.format;
  texinfo->data         = tdfinfo->data;
}

static BOOL TextureHasAlpha( unsigned int *texture, int width, int height )
{
  unsigned int *end = texture + width * height;
  unsigned int *cur;
  unsigned int last_val = texture[0] & 0xff000000;

  if( ( last_val != 0 ) && ( last_val != 0xff000000 ) )
    return TRUE;
  for( cur = texture; cur < end; cur++ )
    {
      if( last_val != ( *cur & 0xff000000 ) )
        return TRUE;
    }
  return FALSE;
}

static void CreateTexInfoFromDIB( BITMAPINFO *bitmap_info, GrTexInfo *texinfo )
{
  BITMAPINFOHEADER *  bmih; 
  int                 width, height;
  int                 newWidth, newHeight;
  int                 retSize;
  char *              imageData;
  Gu3dfInfo           imgInfo;
  
  bmih = &bitmap_info->bmiHeader;

  imgInfo.data = NULL;
  imgInfo.mem_required = 0;

  width = bmih->biWidth;
  height = (bmih->biHeight > 0) ? bmih->biHeight : -bmih->biHeight;
  imageData = (char *)bitmap_info + sizeof(BITMAPINFOHEADER);
  newWidth = width;
  newHeight = height;

  GrTextureFormat_t target_tex_format;
  // Figure out if the texture has alpha or not
  //  if( TextureHasAlpha( ( unsigned int * )imageData, width, height ) )
  if( has_opacity_map )
    {
      target_tex_format = GR_TEXFMT_ARGB_4444;
    }
  else
    {
      target_tex_format = GR_TEXFMT_RGB_565;
    }

  if( has_opacity_map )
    {
      // fix up the alpha component in the texture
      int i;

      for( i = 0; i < width * height; i++ )
        {
          unsigned int *texel = &( ( unsigned int * )imageData )[i];
          float mag;
          float r, g, b;
          r = ( float )( ( ( *texel ) & 0x00ff0000 ) >> 16 );
          g = ( float )( ( ( *texel ) & 0x0000ff00 ) >> 8 );
          b = ( float )( ( ( *texel ) & 0x000000ff ) );
          mag = r * r + g * g + b * b;
          mag = ( float )sqrt( mag );
          if( mag < 0 )
            mag = 0;
          if( mag > 255.0f )
            mag = 255.0f;
          *texel &= 0x00ffffff;
          *texel |= ( ( ( unsigned int )mag ) << 24 );
          //          *texel = 0xffff0000;
        }
    }

  retSize = txInit3dfInfo( &imgInfo, target_tex_format,
                           &newWidth, &newHeight, -1, TX_AUTORESIZE_GROW );

  imgInfo.data = new unsigned char[retSize];
  ASSERT( imgInfo.data );

  txConvert( &imgInfo, GR_TEXFMT_ARGB_8888,
             width, height,
             imageData,
             TX_DITHER_ERR,
             NULL);

  Convert3dfInfoToTexInfo( &imgInfo, texinfo );
}

TexHandle *SplashTexMaker::CreateHandle( BITMAPINFO *bitmap_info, int sym_flags )
{
  CreateTexInfoFromDIB( bitmap_info, &texinfos[num_texinfos] );
  return new SplashTexHandle( &theSplashUtil.texcache, &texinfos[num_texinfos++] );
}

static SplashCamera theSplashCamera;

class SplashUtilClassDesc:public ClassDesc {
public:
  int                     IsPublic() {return 1;}
  void *                  Create(BOOL loading = FALSE) {return &theSplashUtil;}
  const TCHAR *   ClassName() {return GetString(IDS_RB_SPLASHUTIL);}
  SClass_ID               SuperClassID() {return UTILITY_CLASS_ID;}
  Class_ID                ClassID() {return Class_ID(SPLASH_CLASS_ID,0);}
  const TCHAR*    Category() {return _T("");}
};

static SplashUtilClassDesc splashUtilDesc;
ClassDesc* GetSplashUtilDesc() {return &splashUtilDesc;}

class SplashMaterial
{
public:
  Mtl *max_material;
  float wire_color[3];
  Matrix3 *uv_transforms;
  int material_count;
  BOOL has_opacity_map;
  float xparency;

public:
  class SubMaterial
  {
  public:
    SubMaterial() { texcache_id = TEX_CACHE_NULL_ID; }
    float diffuse[3];
    float specular[3];
    TexCacheId_t texcache_id;
  };

  int has_submaterials;
  SubMaterial *submaterials;

  SplashMaterial( Mtl * );
  SplashMaterial( DWORD wireframe_color_ );
  ~SplashMaterial();
};

static int num_stored_materials = 0;
SplashMaterial *stored_materials[MAX_MATERIALS];

static SplashMaterial *FindMaterial( Mtl *max_material, int *material_index )
{
  int i;

  for( i = 0; i < num_stored_materials; i++ )
    {
      if( stored_materials[i]->max_material == max_material )
        {
          *material_index = i;
          return stored_materials[i];
        }
    }
  return NULL;
}

SplashMaterial::SplashMaterial( Mtl *max_material_ )
{
  has_opacity_map = FALSE;
  has_submaterials = 0;
  max_material = max_material_;
  wire_color[0] = wire_color[1] = wire_color[2] = 0;
  uv_transforms = NULL;
  material_count = 0;

  material_count = max_material->NumSubMtls();
  if( material_count == 0 )
    material_count = 1;
  else
    has_submaterials = 1;

  uv_transforms = new Matrix3[material_count];
  submaterials = new SubMaterial[material_count];

  ASSERT( uv_transforms && submaterials );

  SubMaterial *submaterial = submaterials;

  int i;
  for( i = 0; i < material_count; i++ )
    {
      uv_transforms[i].IdentityMatrix();

      Color color;

      color = max_material->GetDiffuse( i );
      submaterial->diffuse[0] = color.r;
      submaterial->diffuse[1] = color.g;
      submaterial->diffuse[2] = color.b;

      color = max_material->GetSpecular( i );
      submaterial->specular[0] = color.r;
      submaterial->specular[1] = color.g;
      submaterial->specular[2] = color.b;

      // Should I do something here to check for two-sided materials?
      
      submaterial++;
    }
}

SplashMaterial::SplashMaterial( DWORD wire_color_ )
{
  has_opacity_map = FALSE;
  has_submaterials = 0;
  max_material = NULL;
  wire_color[0] = ( float )GetRValue( wire_color_ ) / 255.0f;
  wire_color[1] = ( float )GetGValue( wire_color_ ) / 255.0f;
  wire_color[2] = ( float )GetBValue( wire_color_ ) / 255.0f;
  uv_transforms = new Matrix3;
  uv_transforms->IdentityMatrix();
  material_count = 1;

  submaterials = new SubMaterial[1];
  ASSERT( submaterials );
  SubMaterial *submaterial = submaterials;

  submaterial->diffuse[0] = wire_color[0];
  submaterial->diffuse[1] = wire_color[1];
  submaterial->diffuse[2] = wire_color[2];

  submaterial->specular[0] = wire_color[0];
  submaterial->specular[1] = wire_color[1];
  submaterial->specular[2] = wire_color[2];

  // should do something about two-side junk here.
}

void SplashUtil::UpdateLights( TimeValue time )
{
  int i;

  for( i = 0; i < num_stored_lights; i++ )
    {
      SplashLight *light = &stored_lights[i];
      Interval validity;

      LightState light_state;
      ObjectState const &object_state = light->node->EvalWorldState( time, FALSE );
      LightObject *light_object = ( LightObject * )object_state.obj;
      light_object->EvalLightState( time, validity, &light_state );
      Matrix3 object_transform = light->node->GetObjTMAfterWSM( time );
      float glmat[16];
      ConvertMatrix3ToGL( glmat, object_transform );
      if( do_write && ( light_state.type == OMNI_LGT ) )
        {
#if 1
          unsigned char cmd = OMNI_LIGHT_CMD;
          fwrite( &cmd, sizeof( cmd ), 1, write_fp );
          fwrite( &light_state.color.r, sizeof( float ), 1, write_fp );
          fwrite( &light_state.color.g, sizeof( float ), 1, write_fp );
          fwrite( &light_state.color.b, sizeof( float ), 1, write_fp );
          fwrite( &light_state.intens, sizeof( float ), 1, write_fp );
          // write the position of the omni light.
          fwrite( &glmat[12], 3 * sizeof( float ), 1, write_fp );
#endif
        }
    }
}

void SplashUtil::RenderMesh( Mesh *mesh )
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
  
  if( !model_verts )
    return;

  if( tex_verts )
    {
      ASSERT( num_tex_verts >= num_verts );
    }
  
  Face *face;
  RevVert verts[3];
  for( face = faces; face < end_faces; face++ )
    {
      int vertnum;
      for( vertnum = 0; vertnum < 3; vertnum++ )
        {
          Point3 *model_vert = &model_verts[face->getVert( vertnum )];
          Point3 *tex_vert = ( tex_verts ) ? &tex_verts[face->getVert( vertnum )] : NULL;
          Point3& normal = mesh->getNormal( vertnum );
          verts[vertnum].pos.Set( model_vert->x, model_vert->y, model_vert->z );
          verts[vertnum].normal.Set( normal.x, normal.y, normal.z );
#if 0
          if( tex_verts )
            verts[vertnum].texvert.Set( graphics->smult * tex_vert->x,
                                        graphics->tmult * tex_vert->y, 0.0f );
#else
          if( tex_verts )
            verts[vertnum].texvert.Set( tex_vert->x,
                                        tex_vert->y, 0.0f );
#endif
        }

      graphics->DrawPolygon( 3, verts );
    }
}

void SplashUtil::DisplayGeometryNode( TimeValue time, int geom_id )
{
  INode *geom_node;
  GeomObject *geom_object;
  geom_node = stored_geometry[geom_id].node;
  SplashMaterial *material = stored_geometry[geom_id].material;
  ObjectState const &object_state = geom_node->EvalWorldState( time, FALSE );
  geom_object = ( GeomObject * )object_state.obj;
      
  if( !geom_object || geom_node->IsNodeHidden() )
    return;

  if( do_write )
    {
      if( material->has_opacity_map )
        {
          unsigned char cmd = Z_WRITE_CMD;
          fwrite( &cmd, sizeof( cmd ), 1, write_fp );
          unsigned char enable = 0;
          fwrite( &enable, sizeof( enable ), 1, write_fp );
        }
      else
        {
          unsigned char cmd = Z_WRITE_CMD;
          fwrite( &cmd, sizeof( cmd ), 1, write_fp );
          unsigned char enable = 1;
          fwrite( &enable, sizeof( enable ), 1, write_fp );
        }
    }

  Interval object_valid_interval, geom_valid_interval;
  Interval topo_valid_interval, mesh_valid_interval;

  object_valid_interval = geom_object->ObjectValidity( time );
  geom_valid_interval = geom_object->ChannelValidity( time, GEOM_CHANNEL );
  topo_valid_interval = geom_object->ChannelValidity( time, TOPO_CHANNEL );

  mesh_valid_interval = geom_valid_interval & topo_valid_interval;

  NullView null_view;
  BOOL need_delete;
  Mesh *mesh;
  mesh = geom_object->GetRenderMesh( time, geom_node, null_view, need_delete );

  if( mesh )
    {
      float glmat[16];

      Matrix3 object_transform = geom_node->GetObjTMAfterWSM( time );
      SetLeftHanded( object_transform );
      ConvertMatrix3ToGL( glmat, object_transform );
      graphics->PushMatrix();
      graphics->MultMatrix( glmat );
      if( !mesh_valid_interval.InInterval( last_geometry_update ) )
        mesh->buildNormals();

      if( do_write )
        {
          unsigned char cmd;
          cmd = SET_MATERIAL_CMD;
          fwrite( &cmd, sizeof( cmd ), 1, write_fp );
          int material_id = stored_geometry[geom_id].material_index;
          fwrite( &material_id, sizeof( material_id ), 1, write_fp );
        }

      if( do_write )
        {
          unsigned char cmd;
          cmd = XPARENCY_CMD;
          fwrite( &cmd, sizeof( cmd ), 1, write_fp );
          FxU32 xparency;
          xparency = ( unsigned long )( 255 * material->xparency );
          if( xparency < 0 )
            xparency = 0;
          if( xparency > 255 )
            xparency = 255;
          xparency <<= 24;
          fwrite( &xparency, sizeof( xparency ), 1, write_fp );
        }

      if( do_write )
        {
          unsigned char cmd;
          cmd = DRAW_GEOM_CMD;
          fwrite( &cmd, sizeof( cmd ), 1, write_fp );
          fwrite( &geom_id, sizeof( geom_id ), 1, write_fp );
          fwrite( glmat, sizeof( glmat ), 1, write_fp );
        }

      constant_color_val = 
        0xff000000 | 
        ( unsigned int )( material->submaterials[0].diffuse[0] * 255.0f ) << 16 |
        ( unsigned int )( material->submaterials[0].diffuse[1] * 255.0f ) << 8 |
        ( unsigned int )( material->submaterials[0].diffuse[2] * 255.0f );
          
      // Setup the material here.
      if( material->submaterials[0].texcache_id != TEX_CACHE_NULL_ID )
        {
          TexCacheSetCurrent( &texcache, 
                              material->submaterials[0].texcache_id, 
                              FXFALSE );
          guColorCombineFunction( GR_COLORCOMBINE_DECAL_TEXTURE );
          grTexCombineFunction( GR_TMU0, GR_TEXTURECOMBINE_DECAL );
          RenderMesh( mesh );
        }
      else
        {
          guColorCombineFunction( GR_COLORCOMBINE_CCRGB );
          grConstantColorValue( constant_color_val );
          RenderMesh( mesh );
        }

      graphics->PopMatrix();
    }
}

float SplashGeometry::GetSortZVal( void )
{
  INode *geom_node;
  GeomObject *geom_object;
  geom_node = node;
  ObjectState const &object_state = geom_node->EvalWorldState( global_time, FALSE );
  geom_object = ( GeomObject * )object_state.obj;
      
  if( !geom_object || geom_node->IsHidden( 0 ) )
    return 0.0f;

  Interval object_valid_interval, geom_valid_interval;
  Interval topo_valid_interval, mesh_valid_interval;

  object_valid_interval = geom_object->ObjectValidity( global_time );
  geom_valid_interval = geom_object->ChannelValidity( global_time, GEOM_CHANNEL );
  topo_valid_interval = geom_object->ChannelValidity( global_time, TOPO_CHANNEL );
  mesh_valid_interval = geom_valid_interval & topo_valid_interval;

  NullView null_view;
  BOOL need_delete;
  Mesh *mesh;
  mesh = geom_object->GetRenderMesh( global_time, geom_node, null_view, need_delete );

  if( !mesh )
    return 0;

  float glmat[16];

  Matrix3 object_transform = geom_node->GetObjTMAfterWSM( global_time );
  SetLeftHanded( object_transform );
  ConvertMatrix3ToGL( glmat, object_transform );
  graphics->PushMatrix();
  graphics->MultMatrix( glmat );
  Point3 *model_verts = mesh->getVertPtr( 0 );
  if( !model_verts )
    {
      graphics->PopMatrix();
      return 0;
    }
  RevPoint point, vpoint;
  point.Set( model_verts->x, model_verts->y, model_verts->z );
  graphics->ObjectToView( point, vpoint );
  graphics->PopMatrix();
  return vpoint.GetZ();
}

int __cdecl CompareFunc( const void *a_, const void *b_ )
{
  SplashGeometry *a = &stored_geometry[*( int * )a_];
  SplashGeometry *b = &stored_geometry[*( int * )b_];

  float az, bz;
  az = a->GetSortZVal();
  bz = b->GetSortZVal();
  if( az < bz )
    return 1;
  else if( az > bz )
    return -1;
  else
    return 0;
}

void SplashUtil::DisplayGeometry( TimeValue time )
{
  int geom_id;
  static int sorted_geom[MAX_GEOM];

  global_time = time;

  //  ip->SetRendHidden( FALSE );

  //  graphics->RenderBuffer( REV_BUFFER_FRONT );
  graphics->Clear( GLIDE_GRAPHICS_CLEAR_COLOR | GLIDE_GRAPHICS_CLEAR_DEPTH, 0.0f, 0.0f, 1.0f );

  // Need to sort the geometry so that the alpha junk can be rendered from 
  // back to front
  for( geom_id = 0; geom_id < num_stored_geometry; geom_id++ )
    {
      sorted_geom[geom_id] = geom_id;
    }

  qsort( sorted_geom, num_stored_geometry, sizeof( int ), 
         CompareFunc );

  for( geom_id = 0; geom_id < num_stored_geometry; geom_id++ )
    {
      SplashMaterial *material = stored_geometry[sorted_geom[geom_id]].material;
      if( material->has_opacity_map )
        continue;
      DisplayGeometryNode( time, sorted_geom[geom_id] );
    }
  for( geom_id = 0; geom_id < num_stored_geometry; geom_id++ )
    {
      SplashMaterial *material = stored_geometry[sorted_geom[geom_id]].material;
      if( !material->has_opacity_map )
        continue;
      DisplayGeometryNode( time, sorted_geom[geom_id] );
    }
  last_geometry_update = time;
}

void SplashUtil::SplashTest( void )
{
  //  MessageBox(NULL, "SplashTest", "blah", MB_OK);
  Interval anim_range = ip->GetAnimRange();
  ViewExp *view = ip->GetActiveViewport();
  TimeValue start_time, end_time;
  int ticks_per_frame;
  int total_frames;
  float oo_total_frames;
  int tick_skip;
  int frame_rate;
  //  int skip_count;

  //  skip_count = frameIncrement;
  start_time = anim_range.Start();
  end_time = anim_range.End();
  ticks_per_frame = GetTicksPerFrame();
  total_frames = anim_range.Duration() / GetTicksPerFrame();
  oo_total_frames = 1.0f / ( float )total_frames;
  //  tick_skip = ticks_per_frame * skip_count;
  tick_skip = ticks_per_frame;
  frame_rate = GetFrameRate();

  SetupAnimation( ip->GetRootNode(), start_time );
  UpdateMaterials( start_time );
  //  UpdateLights( start_time );
  SetupAnimationCamera( view, start_time );
  BOOL done = FALSE;
  int tick;
  while( !done )
    {
      for( tick = start_time; tick < end_time; tick += tick_skip )
        {
          MSG msg;

          // Update the camera for this frame.
          theSplashCamera.Update( tick );
          graphics->PushMatrix();
          graphics->LoadIdentity();
          theSplashCamera.MultMatrix();

          UpdateLights( tick );

          // Bring the materials up to date in case there are animated materials.
          // Hack: Don't do any stinking animated materials.
          //          UpdateMaterials( tick );
          UpdateMaterialsQuick( tick );

          // Show all of the geometry.
          DisplayGeometry( tick );
          graphics->Swap();

          graphics->PopMatrix();

          if( PeekMessage( &msg, NULL, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE ) )
            {
              if( msg.message == WM_KEYDOWN )
                {
                  switch( msg.wParam & 0xff )
                    {
                    case VK_ESCAPE:
                      done = TRUE;
                      break;
                    default:
                      break;
                    }
                }
            }
          if( done )
            goto end_of_animation_loop;
        }
    }
end_of_animation_loop:
  num_stored_geometry = 0;
  num_stored_materials = 0;
  num_stored_lights = 0;
}

static BOOL OpenFileDialog( HWND parent, char *filter, 
                            DWORD flags, char *startDir, 
                            char *file, FxU32 fileLen ) {
  OPENFILENAME ofn;

  ofn.lStructSize       = sizeof( ofn );
  ofn.hwndOwner         = parent;
  ofn.hInstance         = NULL;
  ofn.lpstrFilter       = filter;
  ofn.lpstrCustomFilter = 0;
  ofn.nMaxCustFilter    = 0;
  ofn.nFilterIndex      = 0;
  file[0]               = 0;
  ofn.lpstrFile         = file;
  ofn.nMaxFile          = fileLen;
  ofn.lpstrFileTitle    = 0;
  ofn.nMaxFileTitle     = 0;
  ofn.lpstrInitialDir   = startDir;
  ofn.lpstrTitle        = "Open File";
  ofn.Flags             = flags;
  ofn.lpstrDefExt       = 0;

  return GetOpenFileName( &ofn );
}

BOOL SplashUtil::OpenFile( void )
{
  char filename[512];
  char    filter[] = 
    "3Dfx Splash Animation File (*.splash)\0*.splash\0\0";
  OpenFileDialog( 0,
                  filter, 
                  //                  OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST, 
                  OFN_PATHMUSTEXIST, 
                  0, 
                  filename,
                  512 );
  if( !( write_fp = fopen( filename, "wb" ) ) )
    return FALSE;

  fprintf( write_fp, "3Dfx Splash Version 1.1\n" );
  return TRUE;
}

void SplashUtil::CloseFile( void )
{
  fclose( write_fp );
}

void SplashUtil::WriteTextures( void )
{
  int i;

  // Write the total number of textures.
  fwrite( &num_texinfos, sizeof( num_texinfos ), 1, write_fp );
  for( i = 0; i < num_texinfos; i++ )
    {
      FxU32 size;
      GrTexInfo *texinfo = &texinfos[i];

      // Write the texinfo structure for each texture
      fwrite( texinfo, sizeof( GrTexInfo ), 1, write_fp );
      size = grTexCalcMemRequired( texinfo->smallLod, texinfo->largeLod,
                                   texinfo->aspectRatio, texinfo->format );
      fwrite( &size, sizeof( size ), 1, write_fp );
      fwrite( texinfo->data, size, 1, write_fp );
    }
}

void SplashUtil::WriteStoredMaterials( void )
{
  WriteTextures();
  
  fwrite( &num_stored_materials, sizeof( num_stored_materials ), 1, write_fp );

  int i, j;

  SplashMaterial *material;

  for( i = 0; i < num_stored_materials; i++ )
    {
      material = stored_materials[i];
      fwrite( &material->material_count, sizeof( material->material_count),
              1, write_fp );
      for( j = 0; j < material->material_count; j++ )
        {
          SplashMaterial::SubMaterial *submaterial = &material->submaterials[j];

          fwrite( submaterial->diffuse, sizeof( submaterial->diffuse ), 1, write_fp );
          fwrite( submaterial->specular, sizeof( submaterial->specular ), 1, write_fp );
          if( submaterial->texcache_id == TEX_CACHE_NULL_ID )
            {
              int tmp = -1;
              fwrite( &tmp, sizeof( tmp ), 1, write_fp );
            }
          else
            {
              fwrite( &texcache_id_to_texinfo_id[submaterial->texcache_id],
                      sizeof( int ), 1, write_fp );
            }
        }
    }
}

void SplashUtil::WriteMesh( Mesh *mesh )
{
  int num_faces = mesh->getNumFaces();
  int num_verts = mesh->getNumVerts();
  int num_tex_verts = mesh->getNumTVerts();
  Point3 *model_verts = mesh->getVertPtr( 0 );
  Point3 *tex_verts = mesh->getTVertPtr( 0 );
  Face *faces = mesh->faces;
  Face *end_faces = mesh->faces + num_faces;
  
  ASSERT( model_verts );

  if( tex_verts )
    {
      ASSERT( num_tex_verts >= num_verts );
    }

  Face *face;
  int i;
  fwrite( &num_faces, sizeof( num_faces ), 1, write_fp );
  for( i = 0; i < num_faces; i++ )
    {
      face = &faces[i];
      int vertnum;

      for( vertnum = 0; vertnum < 3; vertnum++ )
        {
          int vert_id;
          vert_id = face->getVert( vertnum );
          fwrite( &vert_id, sizeof( vert_id ), 1, write_fp );
        }
    }

  fwrite( &num_verts, sizeof( num_verts ), 1, write_fp );
  mesh->buildNormals();
  for( i = 0; i < num_verts; i++ )
    {
      // do the objects space vert
      Point3 *model_vert = &model_verts[i];
      fwrite( &model_vert->x, sizeof( model_vert->x ), 1, write_fp );
      fwrite( &model_vert->y, sizeof( model_vert->y ), 1, write_fp );
      fwrite( &model_vert->z, sizeof( model_vert->z ), 1, write_fp );

      // do the normal
      Point3& normal = mesh->getNormal( i );
      fwrite( &normal.x, sizeof( normal.x ), 1, write_fp );
      fwrite( &normal.y, sizeof( normal.y ), 1, write_fp );
      fwrite( &normal.z, sizeof( normal.z ), 1, write_fp );
    }

  fwrite( &num_tex_verts, sizeof( num_tex_verts ), 1, write_fp );
  for( i = 0; i < num_tex_verts; i++ )
    {
      Point3 *tex_vert = &tex_verts[i];
      fwrite( &tex_vert->x, sizeof( tex_verts->x ), 1, write_fp );
      fwrite( &tex_vert->y, sizeof( tex_verts->x ), 1, write_fp );
    }
}

void SplashUtil::WriteStoredGeometry( TimeValue time )
{
  fwrite( &num_stored_geometry, sizeof( num_stored_geometry ), 1, write_fp );

  int i;

  for( i = 0; i < num_stored_geometry; i++ )
    {
      INode *geom_node;
      geom_node = stored_geometry[i].node;
      ObjectState const &object_state = geom_node->EvalWorldState( time, FALSE );
      GeomObject *geom_object;
      geom_object = ( GeomObject * )object_state.obj;
      if( !geom_object )
        continue;
      NullView null_view;
      BOOL need_delete;
      Mesh *mesh;
      mesh = geom_object->GetRenderMesh( time, geom_node, null_view, need_delete );
      if( mesh )
        {
          fwrite( &i, sizeof( i ), 1, write_fp );
          WriteMesh( mesh );
        }
    }
  i = -1;
  fwrite( &i, sizeof( i ), 1, write_fp );
}

void SplashUtil::SplashWrite( void )
{
  do_write = TRUE;

  if( !OpenFile() )
    return;

  Interval anim_range = ip->GetAnimRange();
  ViewExp *view = ip->GetActiveViewport();
  TimeValue start_time, end_time;
  int ticks_per_frame;
  int total_frames;
  float oo_total_frames;
  int tick_skip;
  int frame_rate;
  //  int skip_count;

  //  skip_count = frameIncrement;
  start_time = anim_range.Start();
  end_time = anim_range.End();
  ticks_per_frame = GetTicksPerFrame();
  total_frames = anim_range.Duration() / GetTicksPerFrame();
  fwrite( &total_frames, sizeof( total_frames ), 1, write_fp );
  oo_total_frames = 1.0f / ( float )total_frames;
  //  tick_skip = ticks_per_frame * skip_count;
  tick_skip = ticks_per_frame;
  frame_rate = GetFrameRate();
  fwrite( &frame_rate, sizeof( frame_rate ), 1, write_fp );

  SetupAnimation( ip->GetRootNode(), start_time );
  UpdateMaterials( start_time );
  //  UpdateLights( start_time );
  WriteStoredMaterials();
  WriteStoredGeometry( start_time );
  SetupAnimationCamera( view, start_time );
  BOOL done = FALSE;
  int tick;
  for( tick = start_time; tick < end_time; tick += tick_skip )
    {
      MSG msg;

      // Update the camera for this frame.
      theSplashCamera.Update( tick );
      graphics->PushMatrix();
      graphics->LoadIdentity();
      theSplashCamera.MultMatrix();
      theSplashCamera.Write();

      UpdateLights( tick );

      // Bring the materials up to date in case there are animated materials.
      // Hack: Don't do any stinking animated materials.
      //          UpdateMaterials( tick );
      UpdateMaterialsQuick( tick );

      // Show all of the geometry.
      DisplayGeometry( tick );

      graphics->Swap();
      {
        char tmp;
        tmp = GRAPHICS_SWAP_CMD;
        fwrite( &tmp, sizeof( tmp ), 1, write_fp );
      }

      graphics->PopMatrix();

      if( PeekMessage( &msg, NULL, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE ) )
        {
          if( msg.message == WM_KEYDOWN )
            {
              switch( msg.wParam & 0xff )
                {
                case VK_ESCAPE:
                  done = TRUE;
                  break;
                default:
                  break;
                }
            }
        }
      if( done )
        goto end_of_animation_loop;
    }
end_of_animation_loop:
  {
    char tmp;
    tmp = EOF_CMD;
    fwrite( &tmp, sizeof( tmp ), 1, write_fp );
  }
  num_stored_geometry = 0;
  num_stored_materials = 0;
  num_stored_lights = 0;
  do_write = FALSE;
  CloseFile();
}

#if 0
void SplashUtil::UpdateMaterialsRecursive( Mtl *max_material, TimeValue time,
                                           SplashMaterial *material, int depth )
{
  char *material_name = NULL;

  material_name = max_material->GetName();
  int i;
  int j;

  for( j = 0; j < depth; j++ )
    fprintf( debug_fp, "  " );
  fprintf( debug_fp, "material: \"%s\" class_id: 0x%x\n", material_name, max_material->ClassID() );
  fflush( debug_fp );

  Texmap *texmap;
  //  texmap = max_material->GetActiveTexmap();

  for( i = 0; i < max_material->NumSubTexmaps(); i++ )
    {
      texmap = max_material->GetSubTexmap( i );
      if( !texmap )
        continue;

      //      texmap->GetUVTransform( uv_transforms[j] );
      if( texmap->SupportTexDisplay() )
        {
          SplashTexMaker texmaker;
          int tiling;
          TexCacheId_t texcache_id;
          
          char *tex_name = texmap->GetName();
          for( j = 0; j < depth; j++ )
            fprintf( debug_fp, "  " );
          fprintf( debug_fp, " texture: \"%s\"\n", tex_name );
          fflush( debug_fp );
          tiling = texmap->GetTextureTiling();
          texmap->ActivateTexDisplay( FALSE );
          texcache_id = ( TexCacheId_t )texmap->GetActiveTexHandle( time, texmaker );
          //          TexCacheSetCurrent( &texcache, texcache_id, FXFALSE );
          material->submaterials[0].texcache_id = texcache_id;
          texmap->ActivateTexDisplay( FALSE );
        }
    }

  for( i = 0; i < max_material->NumSubMtls(); i++ )
    {
      Mtl *submaterial = max_material->GetSubMtl( i );
      UpdateMaterialsRecursive( submaterial, time, material, depth + 1 );
    }
}
#endif

void SplashUtil::UpdateMaterial( Mtl *max_material, TimeValue time,
                                 SplashMaterial *material )
{
#if 1
  material->xparency = max_material->GetXParency();
#endif

  if( max_material->ClassID() == Class_ID( MIXMAT_CLASS_ID, 0 ) )
    {
      //      fprintf( debug_fp, "blend material: \"%s\"\n", max_material->GetName() );

      ASSERT( max_material->NumSubMtls() == 2 );
      //      UpdateMaterial( max_material->GetSubMtl( 0 ), time, material );
      UpdateMaterial( max_material->GetSubMtl( 1 ), time, material );
    }
  else
    {
#if 0
      Texmap *texmap;
      //      texmap = max_material->GetActiveTexmap();

      int i;
      int depth = 0;
      fprintf( debug_fp, "\nmaterial: \"%s\"\n", max_material->GetName() );
      fflush( debug_fp );
      for( i = 0; i < max_material->NumSubTexmaps(); i++ )
        {
          texmap = max_material->GetSubTexmap( i );
          TSTR string;
          string = max_material->GetSubTexmapSlotName( i );
          fprintf( debug_fp, "textureslot: \"%s\" %s\n", ( const char * )string,
                   texmap ? ( const char * )texmap->GetName() : "empty" );
          fflush( debug_fp );
          if( strcmp( ( const char * )string, "Diffuse" ) != 0 )
            continue;

          Texmap *opacity_map = max_material->GetSubTexmap( 6 );
          has_opacity_map = opacity_map ? 1 : 0;

          material->has_opacity_map = has_opacity_map;

          if( !texmap )
            continue;

          //      texmap->GetUVTransform( uv_transforms[j] );
          if( texmap->SupportTexDisplay() )
            {
              SplashTexMaker texmaker;
              int tiling;
              TexCacheId_t texcache_id;
          
              char *tex_name = texmap->GetName();
#if 0
              int j;
              for( j = 0; j < depth; j++ )
                fprintf( debug_fp, "  " );
              fprintf( debug_fp, " texture: \"%s\"\n", tex_name );
              fflush( debug_fp );
#endif
              tiling = texmap->GetTextureTiling();
              texmap->ActivateTexDisplay( FALSE );
              texcache_id = ( TexCacheId_t )texmap->GetActiveTexHandle( time, texmaker );
              //              unsigned int *raw_image = ( unsigned int * )texmap->GetActiveTexHandle( time, texmaker );
              material->submaterials[0].texcache_id = texcache_id;
              texmap->ActivateTexDisplay( FALSE );
            }
        }
      for( i = 0; i < max_material->NumSubMtls(); i++ )
        {
          Mtl *submaterial = max_material->GetSubMtl( i );
          UpdateMaterial( submaterial, time, material );
        }
#endif

#if 1
      Texmap *diffuse_texmap;
      Texmap *opacity_map;
      SplashTexMaker texmaker;

      diffuse_texmap = max_material->GetSubTexmap( 1 );
      opacity_map = max_material->GetSubTexmap( 6 );

      if( diffuse_texmap && !diffuse_texmap->SupportTexDisplay() )
        diffuse_texmap = NULL;
      if( opacity_map && !opacity_map->SupportTexDisplay() )
        opacity_map = NULL;
      
      TexCacheId_t texcache_id = TEX_CACHE_NULL_ID;
      if( diffuse_texmap && !opacity_map )
        {
          material->has_opacity_map = has_opacity_map = FALSE;
          texcache_id = ( TexCacheId_t )diffuse_texmap->GetActiveTexHandle( time, texmaker );
        }
      else if( diffuse_texmap && opacity_map )
        {
          material->has_opacity_map = has_opacity_map = TRUE;
          texcache_id = ( TexCacheId_t )diffuse_texmap->GetActiveTexHandle( time, texmaker );
        }
      else if( !diffuse_texmap && opacity_map )
        {
          material->has_opacity_map = has_opacity_map = TRUE;
          texcache_id = ( TexCacheId_t )opacity_map->GetActiveTexHandle( time, texmaker );
        }

      material->submaterials[0].texcache_id = texcache_id;

      int i;
      for( i = 0; i < max_material->NumSubMtls(); i++ )
        {
          Mtl *submaterial = max_material->GetSubMtl( i );
          UpdateMaterial( submaterial, time, material );
        }
#endif
    }
}

void SplashUtil::UpdateMaterialQuick( Mtl *max_material, TimeValue time,
                                      SplashMaterial *material )
{
  material->xparency = max_material->GetXParency();
}

void SplashUtil::UpdateMaterialsQuick( TimeValue time )
{
  int i;

  SplashMaterial *material;
  Interval material_valid_interval;
  Mtl *max_material;
  Matrix3 *uv_transforms;

  for( i = 0; i < num_stored_materials; i++ )
    {

      material = stored_materials[i];
      max_material = material->max_material;
      material_valid_interval = max_material->Validity( time );
      uv_transforms = material->uv_transforms;
      
      if( material_valid_interval.InInterval( last_material_update ) )
        continue;

      max_material->Update( time, material_valid_interval );
      //      UpdateMaterialsRecursive( max_material, time, material, 0 );
      UpdateMaterialQuick( max_material, time, material );
    }
  last_material_update = time;
}

void SplashUtil::UpdateMaterials( TimeValue time )
{
  int i;

  SplashMaterial *material;
  Interval material_valid_interval;
  Mtl *max_material;
  Matrix3 *uv_transforms;

  for( i = 0; i < num_stored_materials; i++ )
    {

      material = stored_materials[i];
      max_material = material->max_material;
      material_valid_interval = max_material->Validity( time );
      uv_transforms = material->uv_transforms;
      
      if( material_valid_interval.InInterval( last_material_update ) )
        continue;

      max_material->Update( time, material_valid_interval );
      //      UpdateMaterialsRecursive( max_material, time, material, 0 );
      UpdateMaterial( max_material, time, material );
    }
  last_material_update = time;
}

void SplashUtil::SetupAnimationCamera( ViewExp *view, TimeValue time )
{
  GraphicsWindow *graphics_window = view->getGW();
  INode *camera_node = view->GetViewCamera();

  if( camera_node )
    {
      theSplashCamera.Set( camera_node );
    }
  else
    {
      float           camera_matrix[4][4];
      float           hither, yon;
      BOOL            persp;
      Matrix3         inverse_transform;
      graphics_window->getCameraMatrix( camera_matrix, &inverse_transform, 
                                        &persp, &hither, &yon );
      theSplashCamera.Set( inverse_transform, hither, yon );
    }
}

void SplashUtil::SetupAnimation( INode *node, TimeValue time )
{
  int num_children = node->NumberOfChildren();
  TCHAR *node_name;
  char buf[256];

  node_name = node->GetName();

  sprintf( buf, "Preparing node %s", node_name );
  theSplashUtil.ip->DisplayTempPrompt( buf );

  ObjectState object_state = node->EvalWorldState( time, FALSE );
  if( object_state.obj && ( object_state.obj->SuperClassID() == LIGHT_CLASS_ID ) )
    {
      // Do something with the light.
      stored_lights[num_stored_lights++].Set( node );
      ASSERT( num_stored_lights < MAX_LIGHTS - 1 );
    }
  else if( object_state.obj && ( object_state.obj->SuperClassID() == GEOMOBJECT_CLASS_ID ) )
    {
      // Do something with the geometry.
      stored_geometry[num_stored_geometry++].Set( node );
      ASSERT( num_stored_geometry < MAX_GEOM - 1 );
    }

  int child_id;
  for( child_id = 0; child_id < num_children; child_id++ )
    {
      SetupAnimation( node->GetChildNode( child_id ), time );
    }
}

static BOOL CALLBACK SplashUtilDlgProc(
                                       HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg) {
  case WM_INITDIALOG:
    theSplashUtil.Init(hWnd);
    break;
                
  case WM_DESTROY:
    theSplashUtil.Destroy(hWnd);
    break;
                
  case WM_COMMAND:
    switch (LOWORD(wParam)) 
      {
      case IDOK:
        theSplashUtil.iu->CloseUtility();
        break;
      case IDSPLASHTEST:
        theSplashUtil.SplashTest();
        break;
      case IDC_SPLASHWRITE:
        theSplashUtil.SplashWrite();
        break;
      }
    break;
  case WM_LBUTTONDOWN:
  case WM_LBUTTONUP:
  case WM_MOUSEMOVE:
    //                        theColorClip.ip->RollupMouseMessage(hWnd,msg,wParam,lParam); 
    break;
                                
  default:
    return FALSE;
  }
  return TRUE;
}       

SplashUtil::SplashUtil()
{
  iu = NULL;
  ip = NULL;      
  hPanel = NULL;
  debug_fp = fopen( "bug.out", "w" );
  ASSERT( debug_fp );
}

void SplashUtil::BeginEditParams(Interface *ip,IUtil *iu) 
{
  this->iu = iu;
  this->ip = ip;
  hPanel = ip->AddRollupPage(
                             hInstance,
                             MAKEINTRESOURCE(IDD_SPLASHUTIL),
                             SplashUtilDlgProc,
                             GetString(IDS_RB_SPLASHUTIL),
                             0);
}

void SplashUtil::EndEditParams(Interface *ip,IUtil *iu) 
{
  this->iu = NULL;
  this->ip = NULL;
  ip->DeleteRollupPage(hPanel);
  hPanel = NULL;
}

void SplashUtil::Init(HWND hWnd)
{
  // BUG: Memory leak and stuff
  num_stored_geometry = 0;
  num_stored_lights = 0;
  last_geometry_update = TIME_NegInfinity;
  last_material_update = TIME_NegInfinity;

  if( !graphics )
    {
      graphics = new GlideGraphics;
      graphics->Open( NULL );
      // hack
      // need to fix this to make it work on Voodoo Rush.
      graphics->Resize();
      graphics->Clear( GLIDE_GRAPHICS_CLEAR_COLOR | GLIDE_GRAPHICS_CLEAR_DEPTH, 0.0f, 0.0f, 1.0f );
      graphics->MatrixMode( GLIDE_PROJECTION );
      graphics->LoadIdentity();
      // hack
      graphics->Perspective( 90.0f, 1, 1.5, 20000.0f );
      graphics->Viewport( 0, 0, 640, 480 );
      graphics->SetFovX( 90.0f );
      graphics->MatrixMode( GLIDE_MODELVIEW );
      graphics->Swap();
      TexCacheInit( &texcache, grTexMinAddress( GR_TMU0 ), grTexMaxAddress( GR_TMU0 ),
                    1024, GR_TMU0, "the cache" );
    }
}

void SplashUtil::Destroy(HWND hWnd)
{
  //    graphics->Close();
  grBufferClear( 0xffff0000, 0, 0 );
  grBufferSwap( 1 );
}

void SplashGeometry::Set( INode *node_ )
{
  node = node_;

  Mtl *material_node = node->GetMtl();

  Material *blah = node->Mtls();
  int num_mtls = node->NumMtls();

  if( material_node )
    {
      material = FindMaterial( material_node, &material_index );
      if( !material )
        {
          material = new SplashMaterial( material_node );
          material_index = num_stored_materials;
          stored_materials[num_stored_materials++] = material;
        }
      complex_material = TRUE;
    }
  else
    {
      DWORD wire_color = node->GetWireColor();
      material = new SplashMaterial( wire_color );
      complex_material = FALSE;
    }
}
