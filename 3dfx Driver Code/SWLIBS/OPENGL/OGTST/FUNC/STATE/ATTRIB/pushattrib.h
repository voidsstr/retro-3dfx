/*
 *  pushattrib.h - global state for an opengl context 
 *  (borrowed from ucode/ge/opengl_attrib.h)
 *
 *  $Id: pushattrib.h,v 1.1 1997/02/05 08:18:42 pho Exp $
 */

#define _GL_NUM_CLIP_PLANES 6
#define _GL_NUM_LIGHTS 8


/*
 *  opengl internal types
 */
typedef struct {
    GLfloat x, y, z, w;
} _GLcoord;

typedef struct {
    GLfloat r, g, b, a;
} _GLcolor;

typedef struct {
    GLfloat nx, ny, nz;
} _GLnormal;

typedef struct {
    GLfloat s, t, r, q;
} _GLtexCoord;


typedef struct {
    GLfloat rasterZ;

    _GLcoord rasterPos;

    /*
    ** Raster pos valid bit.
    */
    GLboolean validRasterPos;

    /*
    ** Edge tag state.
    */
    GLboolean edgeFlag;

    GLfloat index;
    GLfloat rasterIndex;

    /*
    ** The current operating color.  The r component is the color
    ** index in color index mode.  This is a clamped copy of the
    ** users RGB color when in RGB mode.  This is computed during
    ** the applyColor step of color loading.
    */
    _GLcolor color;
    _GLcolor rasterColor;

    _GLcoord normal;
    _GLtexCoord texCoord;
    _GLtexCoord rasterTexCoord;
} _GLcurrentState;

typedef struct {
    GLfloat smoothSize;
    GLint aliasedSize;
} _GLpointState;

typedef struct {
    GLfloat smoothWidth;
    GLint aliasedWidth;
    GLushort stipple;
    GLint stippleRepeat;
} _GLlineState;

typedef struct {
    GLint listBase;
} _GLlistState;

typedef struct {
    GLenum frontMode;
    GLenum backMode;

    /*
    ** Culling state.  Culling can be enabled/disabled and set to cull
    ** front or back faces.  The FrontFace call determines whether clockwise
    ** or counter-clockwise oriented vertices are front facing.
    */
    GLenum cullMode;
    GLenum frontFaceDirection;
} _GLpolygonState;

typedef struct {
    GLubyte pat[4*32];
} _GLpolygonStippleState;

typedef struct {
    GLboolean mapColor;
    GLboolean mapStencil;
    GLint indexShift;
    GLint indexOffset;
    GLfloat redScale, redBias;
    GLfloat greenScale, greenBias;
    GLfloat blueScale, blueBias;
    GLfloat alphaScale, alphaBias;
    GLfloat depthScale, depthBias;
    GLfloat xZoom, yZoom;
    GLenum readBuffer;
} _GLpixelState;


/*
** Light state.  Contains all the user controllable lighting state.
** Most of the colors kept in user state are scaled to match the
** drawing surfaces color resolution.
*/

typedef struct {
    _GLcolor ambient;
    _GLcolor diffuse;
    _GLcolor specular;
    _GLcolor emission;
    GLfloat shininess;
    GLfloat cmap[3];
} _GLmaterialState;

typedef struct {
    _GLcolor ambient;
    GLboolean localViewer;
    GLboolean twoSide;
} _GLlightModelState;

typedef struct {
    _GLcolor ambient;
    _GLcolor diffuse;
    _GLcolor specular;
    _GLcoord position;
    _GLcoord spotDirection;
    int userpositionflag;   /* did this position come from user? (it is pre-xform?) */
    int userspotdirflag;    /* did this spot dir come from user? (it is pre-xform?) */
    GLfloat spotLightExponent;
    GLfloat spotLightCutOffAngle;
    GLfloat constantAttenuation;
    GLfloat linearAttenuation;
    GLfloat quadraticAttenuation;
} _GLlightSourceState;

typedef struct {
    GLenum colorMaterialFace;
    GLenum colorMaterialMode;
    GLenum shadeModel;
    _GLlightModelState model;
    _GLmaterialState front;
    _GLmaterialState back;
    _GLlightSourceState sources[_GL_NUM_LIGHTS];
} _GLlightState;


typedef struct {
    GLenum mode;
    _GLcolor color;
    GLfloat density, start, end;
    GLfloat index;
} _GLfogState;

typedef struct {
    /*
    ** Depth buffer test function.  The z value is compared using zFunction
    ** against the current value in the zbuffer.  If the comparison
    ** succeeds the new z value is written into the z buffer masked
    ** by the z write mask.
    */
    GLenum depthFunc;

    /*
    ** Writemask enable.  When GL_TRUE writing to the depth buffer is
    ** allowed.
    */
    GLboolean writeEnable;

    /*
    ** Value used to clear the z buffer when glClear is called.
    */
    GLdouble zclear;
} _GLdepthState;

typedef struct {
    _GLcolor aclear;
} _GLaccumState;

typedef struct {
    /*
    ** Stencil test function.  When the stencil is enabled this
    ** function is applied to the reference value and the stored stencil
    ** value as follows:
    **		result = ref comparision (mask & stencilBuffer[x][y])
    ** If the test fails then the fail op is applied and rendering of
    ** the pixel stops.
    */
    GLenum testFunc;

    /*
    ** Stencil clear value.  Used by glClear.
    */
    GLint sclear;

    /*
    ** Reference stencil value.
    */
    GLint funcRef;

    /*
    ** Stencil mask.  This is anded against the contents of the stencil
    ** buffer during comparisons.
    */
    GLuint funcMask;

    /*
    ** Stencil write mask
    */
    GLuint stencilMask;

    /*
    ** When the stencil comparison fails this operation is applied to
    ** the stencil buffer.
    */
    GLenum fail;

    /*
    ** When the stencil comparison passes and the depth test
    ** fails this operation is applied to the stencil buffer.
    */
    GLenum pass;

    /*
    ** When both the stencil comparison passes and the depth test
    ** passes this operation is applied to the stencil buffer.
    */
    GLenum depthPass;
} _GLstencilState;

typedef struct {
    /*
    ** Viewport parameters from user.
    */
    GLint x, y, width, height;

    /*
    ** Depthrange parameters from user.
    */
    GLdouble zNear, zFar;
} _GLviewportState;

typedef struct {
    /*
    ** Current mode of the matrix stack.  This determines what effect
    ** the various matrix operations (load, mult, scale) apply to.
    */
    GLenum matrixMode;

    /*
    ** User clipping planes in eye space.  These are the user clip planes
    ** projected into eye space.  
    */
    GLdouble clipPlanes[_GL_NUM_CLIP_PLANES][4];
} _GLtransformState;

typedef struct {
    GLboolean color4;
    GLboolean index;
    GLboolean normal;
    GLboolean textureCoord1;
    GLboolean textureCoord2;
    GLboolean textureCoord3;
    GLboolean textureCoord4;
    GLboolean vertex3;
    GLboolean vertex4;
} _GLevalEnable;

typedef struct {
    GLboolean alpha_test;
    GLboolean auto_normal;
    GLboolean blend;
    GLboolean color_material;
    GLboolean cull_face;
    GLboolean depth_test;
    GLboolean dither;
    GLboolean fog;
    GLboolean lighting;
    GLboolean line_smooth;
    GLboolean line_stipple;
    GLboolean logic_op;
    GLboolean normalize;
    GLboolean point_smooth;
    GLboolean polygon_smooth;
    GLboolean polygon_stipple;
    GLboolean scissor_test;
    GLboolean stencil_test;
    GLboolean texture1d;
    GLboolean texture2d;
    GLboolean texture3d;
    GLboolean tex_gen_s;
    GLboolean tex_gen_t;
    GLboolean tex_gen_r;
    GLboolean tex_gen_q;
    GLboolean lights[_GL_NUM_LIGHTS];
    GLboolean clipPlanes[_GL_NUM_CLIP_PLANES];

    /* enable extensions */
    GLboolean multisampling;
    GLboolean MSsampleMask;      /* The ms mask is being used */
    GLboolean MSalphaToOne;      /* The ms alpha set to one before sending
                                     to the ms buffer */
    GLboolean MSalphaToMask;     /* The alpha value is used to generate
                                     a to mask */
    GLboolean convolution1d;
    GLboolean convolution2d;
    GLboolean histogram;
    GLboolean minmax;
    GLboolean polygon_offset;
    GLboolean separable2d;

    GLboolean referencePlane;

    _GLevalEnable eval1d;
    _GLevalEnable eval2d;
    GLboolean auto_normalize;
} _GLenableState;


typedef struct {
    /*
    ** Alpha function.  The alpha function is applied to the alpha color
    ** value and the reference value.  If it fails then the pixel is
    ** not rendered.
    */
    GLenum alphaFunc;
    GLfloat alphaRef;

    /*
    ** Alpha blending source and destination factors.
    */
    GLenum blendSrc;
    GLenum blendDst;

    /*
    ** Constant blend color values
    */
    _GLcolor blendColor;

    /*
    ** Logic op.
    */
    GLenum logicOp;

    /*
    ** BlendEquation.  Relevant only in RGB mode
    */
    GLenum blendEquation;

    /*
    ** Color to fill the color portion of the framebuffer when clear
    ** is called.
    */
    _GLcolor cclear;
    GLfloat clearIndex;

    /*
    ** Color index write mask.  The color values are masked with this
    ** value when writing to the frame buffer so that only the bits set
    ** in the mask are changed in the frame buffer.
    */
    GLuint writeMask;

    /*
    ** RGB write masks.  These booleans enable or disable writing of
    ** the r, g, b, and a components.
    */
    GLboolean rgbaMask[4];

    /*
    ** This state variable tracks which buffer(s) is being drawn into.
    */
    GLenum drawBuffer;
} _GLrasterState;

/*
** Hint state.  Contains all the user controllable hint state.
*/
typedef struct {
    GLenum perspectiveCorrection;
    GLenum pointSmooth;
    GLenum lineSmooth;
    GLenum polygonSmooth;
    GLenum fog;
} _GLhintState;

typedef struct {
    GLfloat domain1d[2];
    GLint segments1d;
    GLfloat domain2d[2][2];
    GLint segments2d[2];
} _GLevalState;

/*
** Client state set with glTexGen
*/
typedef struct {
    /* How coordinates are being generated */
    GLenum mode;

    /* eye plane equation (used iff mode == GL_EYE_LINEAR) */
    _GLcoord eyePlaneEquation;

    /* object plane equation (used iff mode == GL_OBJECT_LINEAR) */
    _GLcoord objectPlaneEquation;
} _GLtextureCoordState;

typedef struct {
    GLenum mode;
    _GLcolor color;
    _GLcolor bias;
} _GLtexEnv;

/*
** Stackable client texture state. This does not include
** the mipmaps, or level dependent state.  Only state which is
** stackable via glPushAttrib/glPopAttrib is here.  The rest of the
** state is in the machine structure below.
*/
typedef struct {
    /* Per coordinate texture state (set with glTexGen) */
    _GLtextureCoordState s;
    _GLtextureCoordState t;
    _GLtextureCoordState r;
    _GLtextureCoordState q;

    /* XXX so much texture state is still missing! */
    GLenum minf2d;
    
    /* SGIS_texture_select */
    GLint dual_texsel;
    GLint quad_texsel;
    
    /* SGIX_texture_scale_bias */
    GLfloat tsb_scales[4];
    GLfloat tsb_biases[4];

    _GLcolor border;
    _GLtexEnv env;
} _GLtextureState;

/*
** Multi sampling information
*/
typedef struct {
    GLfloat mask;              /* Multisample mask.  Negative==invert */
    GLfloat activeMask;        /* The mask in use = 1 when SAMPLE_MASK is
                                   off, mask (from above) otherwise. */
    long alphaMode;             /* Effective alpha mode - IrisGL msalpha modes +
                                 * OPENGL_MSA_ONE == alpha to one, no alpha to
                                 * mask.
                                 */
    long samplePattern[4];      /* The weights used in the sampling pattern,
                                 * saved away when multisampling is off. 
                                 */
} _GLmsState;
    


/*
** Scissor state from user.
*/
typedef struct {
    GLint x, y, width, height;
} _GLscissorState;

typedef struct {
    GLfloat zmin, zmax;
    GLboolean hit;
} _GLselect;

typedef struct {
    GLint index_bits;
    GLint stereo;
    GLint double_buffer;
    GLint depth_bits;
    GLint accum_bits;
    GLint rgba_bits;
    GLint aux_bufs;
    GLint display_buffer;
    GLint readbank;
    GLint hires;
    GLint lf, lb, rf, rb;
    GLint drawmode;
} _GLconfig;


typedef struct {
    _GLaccumState accum;
    _GLrasterState raster;
    _GLcurrentState current;
    _GLdepthState depth;
    _GLenableState enables;
    _GLevalState evals;
    _GLfogState fog;
    _GLhintState hints;
    _GLlightState light;
    _GLlineState line;
    _GLlistState list;
    _GLpixelState pixel;
    _GLpointState point;
    _GLpolygonState polygon;
    GLubyte polystipple[4*32];
    _GLscissorState scissor;
    _GLstencilState stencil;
    _GLtextureState texture;
    _GLtransformState transform;
    _GLviewportState viewport;
    
    _GLmsState multiSampling;

    GLfloat referencePlaneEquation[4];

    GLbitfield ffdMask;
    
    /*_GLselect select;*/
    /*_GLconfig config;*/
} OpenGLAttrib;
