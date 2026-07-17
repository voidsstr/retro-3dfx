#include "glr_debug_structs.h"

#include <stdio.h>

typedef struct {
	unsigned long value;
	char * name;
} EnumTableEntry;

#define TABLE_ENTRY(v) v, #v,

static EnumTableEntry enum_to_string_table[] = {
	TABLE_ENTRY(GL_ACCUM)
	TABLE_ENTRY(GL_LOAD)
	TABLE_ENTRY(GL_RETURN)
	TABLE_ENTRY(GL_MULT)
	TABLE_ENTRY(GL_ADD)
	TABLE_ENTRY(GL_NEVER)
	TABLE_ENTRY(GL_LESS)
	TABLE_ENTRY(GL_EQUAL)
	TABLE_ENTRY(GL_LEQUAL)
	TABLE_ENTRY(GL_GREATER)
	TABLE_ENTRY(GL_NOTEQUAL)
	TABLE_ENTRY(GL_GEQUAL)
	TABLE_ENTRY(GL_ALWAYS)
	TABLE_ENTRY(GL_POINTS)
	TABLE_ENTRY(GL_LINES)
	TABLE_ENTRY(GL_LINE_LOOP)
	TABLE_ENTRY(GL_LINE_STRIP)
	TABLE_ENTRY(GL_TRIANGLES)
	TABLE_ENTRY(GL_TRIANGLE_STRIP)
	TABLE_ENTRY(GL_TRIANGLE_FAN)
	TABLE_ENTRY(GL_QUADS)
	TABLE_ENTRY(GL_QUAD_STRIP)
	TABLE_ENTRY(GL_POLYGON)
	TABLE_ENTRY(GL_SRC_COLOR)
	TABLE_ENTRY(GL_ONE_MINUS_SRC_COLOR)
	TABLE_ENTRY(GL_SRC_ALPHA)
	TABLE_ENTRY(GL_ONE_MINUS_SRC_ALPHA)
	TABLE_ENTRY(GL_DST_ALPHA)
	TABLE_ENTRY(GL_ONE_MINUS_DST_ALPHA)
	TABLE_ENTRY(GL_DST_COLOR)
	TABLE_ENTRY(GL_ONE_MINUS_DST_COLOR)
	TABLE_ENTRY(GL_SRC_ALPHA_SATURATE)
	TABLE_ENTRY(GL_CLIP_PLANE0)
	TABLE_ENTRY(GL_CLIP_PLANE1)
	TABLE_ENTRY(GL_CLIP_PLANE2)
	TABLE_ENTRY(GL_CLIP_PLANE3)
	TABLE_ENTRY(GL_CLIP_PLANE4)
	TABLE_ENTRY(GL_CLIP_PLANE5)
	TABLE_ENTRY(GL_BYTE)
	TABLE_ENTRY(GL_UNSIGNED_BYTE)
	TABLE_ENTRY(GL_SHORT)
	TABLE_ENTRY(GL_UNSIGNED_SHORT)
	TABLE_ENTRY(GL_INT)
	TABLE_ENTRY(GL_UNSIGNED_INT)
	TABLE_ENTRY(GL_FLOAT)
	TABLE_ENTRY(GL_2_BYTES)
	TABLE_ENTRY(GL_3_BYTES)
	TABLE_ENTRY(GL_4_BYTES)
	TABLE_ENTRY(GL_DOUBLE)
	TABLE_ENTRY(GL_FRONT_LEFT)
	TABLE_ENTRY(GL_FRONT_RIGHT)
	TABLE_ENTRY(GL_BACK_LEFT)
	TABLE_ENTRY(GL_BACK_RIGHT)
	TABLE_ENTRY(GL_FRONT)
	TABLE_ENTRY(GL_BACK)
	TABLE_ENTRY(GL_LEFT)
	TABLE_ENTRY(GL_RIGHT)
	TABLE_ENTRY(GL_FRONT_AND_BACK)
	TABLE_ENTRY(GL_AUX0)
	TABLE_ENTRY(GL_AUX1)
	TABLE_ENTRY(GL_AUX2)
	TABLE_ENTRY(GL_AUX3)
	TABLE_ENTRY(GL_INVALID_ENUM)
	TABLE_ENTRY(GL_INVALID_VALUE)
	TABLE_ENTRY(GL_INVALID_OPERATION)
	TABLE_ENTRY(GL_STACK_OVERFLOW)
	TABLE_ENTRY(GL_STACK_UNDERFLOW)
	TABLE_ENTRY(GL_OUT_OF_MEMORY)
	TABLE_ENTRY(GL_2D)
	TABLE_ENTRY(GL_3D)
	TABLE_ENTRY(GL_3D_COLOR)
	TABLE_ENTRY(GL_3D_COLOR_TEXTURE)
	TABLE_ENTRY(GL_4D_COLOR_TEXTURE)
	TABLE_ENTRY(GL_PASS_THROUGH_TOKEN)
	TABLE_ENTRY(GL_POINT_TOKEN)
	TABLE_ENTRY(GL_LINE_TOKEN)
	TABLE_ENTRY(GL_POLYGON_TOKEN)
	TABLE_ENTRY(GL_BITMAP_TOKEN)
	TABLE_ENTRY(GL_DRAW_PIXEL_TOKEN)
	TABLE_ENTRY(GL_COPY_PIXEL_TOKEN)
	TABLE_ENTRY(GL_LINE_RESET_TOKEN)
	TABLE_ENTRY(GL_EXP)
	TABLE_ENTRY(GL_EXP2)
	TABLE_ENTRY(GL_CW)
	TABLE_ENTRY(GL_CCW)
	TABLE_ENTRY(GL_COEFF)
	TABLE_ENTRY(GL_ORDER)
	TABLE_ENTRY(GL_DOMAIN)
	TABLE_ENTRY(GL_CURRENT_COLOR)
	TABLE_ENTRY(GL_CURRENT_INDEX)
	TABLE_ENTRY(GL_CURRENT_NORMAL)
	TABLE_ENTRY(GL_CURRENT_TEXTURE_COORDS)
	TABLE_ENTRY(GL_CURRENT_RASTER_COLOR)
	TABLE_ENTRY(GL_CURRENT_RASTER_INDEX)
	TABLE_ENTRY(GL_CURRENT_RASTER_TEXTURE_COORDS)
	TABLE_ENTRY(GL_CURRENT_RASTER_POSITION)
	TABLE_ENTRY(GL_CURRENT_RASTER_POSITION_VALID)
	TABLE_ENTRY(GL_CURRENT_RASTER_DISTANCE)
	TABLE_ENTRY(GL_POINT_SMOOTH)
	TABLE_ENTRY(GL_POINT_SIZE)
	TABLE_ENTRY(GL_POINT_SIZE_RANGE)
	TABLE_ENTRY(GL_POINT_SIZE_GRANULARITY)
	TABLE_ENTRY(GL_LINE_SMOOTH)
	TABLE_ENTRY(GL_LINE_WIDTH)
	TABLE_ENTRY(GL_LINE_WIDTH_RANGE)
	TABLE_ENTRY(GL_LINE_WIDTH_GRANULARITY)
	TABLE_ENTRY(GL_LINE_STIPPLE)
	TABLE_ENTRY(GL_LINE_STIPPLE_PATTERN)
	TABLE_ENTRY(GL_LINE_STIPPLE_REPEAT)
	TABLE_ENTRY(GL_LIST_MODE)
	TABLE_ENTRY(GL_MAX_LIST_NESTING)
	TABLE_ENTRY(GL_LIST_BASE)
	TABLE_ENTRY(GL_LIST_INDEX)
	TABLE_ENTRY(GL_POLYGON_MODE)
	TABLE_ENTRY(GL_POLYGON_SMOOTH)
	TABLE_ENTRY(GL_POLYGON_STIPPLE)
	TABLE_ENTRY(GL_EDGE_FLAG)
	TABLE_ENTRY(GL_CULL_FACE)
	TABLE_ENTRY(GL_CULL_FACE_MODE)
	TABLE_ENTRY(GL_FRONT_FACE)
	TABLE_ENTRY(GL_LIGHTING)
	TABLE_ENTRY(GL_LIGHT_MODEL_LOCAL_VIEWER)
	TABLE_ENTRY(GL_LIGHT_MODEL_TWO_SIDE)
	TABLE_ENTRY(GL_LIGHT_MODEL_AMBIENT)
	TABLE_ENTRY(GL_SHADE_MODEL)
	TABLE_ENTRY(GL_COLOR_MATERIAL_FACE)
	TABLE_ENTRY(GL_COLOR_MATERIAL_PARAMETER)
	TABLE_ENTRY(GL_COLOR_MATERIAL)
	TABLE_ENTRY(GL_FOG)
	TABLE_ENTRY(GL_FOG_INDEX)
	TABLE_ENTRY(GL_FOG_DENSITY)
	TABLE_ENTRY(GL_FOG_START)
	TABLE_ENTRY(GL_FOG_END)
	TABLE_ENTRY(GL_FOG_MODE)
	TABLE_ENTRY(GL_FOG_COLOR)
	TABLE_ENTRY(GL_DEPTH_RANGE)
	TABLE_ENTRY(GL_DEPTH_TEST)
	TABLE_ENTRY(GL_DEPTH_WRITEMASK)
	TABLE_ENTRY(GL_DEPTH_CLEAR_VALUE)
	TABLE_ENTRY(GL_DEPTH_FUNC)
	TABLE_ENTRY(GL_ACCUM_CLEAR_VALUE)
	TABLE_ENTRY(GL_STENCIL_TEST)
	TABLE_ENTRY(GL_STENCIL_CLEAR_VALUE)
	TABLE_ENTRY(GL_STENCIL_FUNC)
	TABLE_ENTRY(GL_STENCIL_VALUE_MASK)
	TABLE_ENTRY(GL_STENCIL_FAIL)
	TABLE_ENTRY(GL_STENCIL_PASS_DEPTH_FAIL)
	TABLE_ENTRY(GL_STENCIL_PASS_DEPTH_PASS)
	TABLE_ENTRY(GL_STENCIL_REF)
	TABLE_ENTRY(GL_STENCIL_WRITEMASK)
	TABLE_ENTRY(GL_MATRIX_MODE)
	TABLE_ENTRY(GL_NORMALIZE)
	TABLE_ENTRY(GL_VIEWPORT)
	TABLE_ENTRY(GL_MODELVIEW_STACK_DEPTH)
	TABLE_ENTRY(GL_PROJECTION_STACK_DEPTH)
	TABLE_ENTRY(GL_TEXTURE_STACK_DEPTH)
	TABLE_ENTRY(GL_MODELVIEW_MATRIX)
	TABLE_ENTRY(GL_PROJECTION_MATRIX)
	TABLE_ENTRY(GL_TEXTURE_MATRIX)
	TABLE_ENTRY(GL_ATTRIB_STACK_DEPTH)
	TABLE_ENTRY(GL_CLIENT_ATTRIB_STACK_DEPTH)
	TABLE_ENTRY(GL_ALPHA_TEST)
	TABLE_ENTRY(GL_ALPHA_TEST_FUNC)
	TABLE_ENTRY(GL_ALPHA_TEST_REF)
	TABLE_ENTRY(GL_DITHER)
	TABLE_ENTRY(GL_BLEND_DST)
	TABLE_ENTRY(GL_BLEND_SRC)
	TABLE_ENTRY(GL_BLEND)
	TABLE_ENTRY(GL_LOGIC_OP_MODE)
	TABLE_ENTRY(GL_INDEX_LOGIC_OP)
	TABLE_ENTRY(GL_COLOR_LOGIC_OP)
	TABLE_ENTRY(GL_AUX_BUFFERS)
	TABLE_ENTRY(GL_DRAW_BUFFER)
	TABLE_ENTRY(GL_READ_BUFFER)
	TABLE_ENTRY(GL_SCISSOR_BOX)
	TABLE_ENTRY(GL_SCISSOR_TEST)
	TABLE_ENTRY(GL_INDEX_CLEAR_VALUE)
	TABLE_ENTRY(GL_INDEX_WRITEMASK)
	TABLE_ENTRY(GL_COLOR_CLEAR_VALUE)
	TABLE_ENTRY(GL_COLOR_WRITEMASK)
	TABLE_ENTRY(GL_INDEX_MODE)
	TABLE_ENTRY(GL_RGBA_MODE)
	TABLE_ENTRY(GL_DOUBLEBUFFER)
	TABLE_ENTRY(GL_STEREO)
	TABLE_ENTRY(GL_RENDER_MODE)
	TABLE_ENTRY(GL_PERSPECTIVE_CORRECTION_HINT)
	TABLE_ENTRY(GL_POINT_SMOOTH_HINT)
	TABLE_ENTRY(GL_LINE_SMOOTH_HINT)
	TABLE_ENTRY(GL_POLYGON_SMOOTH_HINT)
	TABLE_ENTRY(GL_FOG_HINT)
	TABLE_ENTRY(GL_TEXTURE_GEN_S)
	TABLE_ENTRY(GL_TEXTURE_GEN_T)
	TABLE_ENTRY(GL_TEXTURE_GEN_R)
	TABLE_ENTRY(GL_TEXTURE_GEN_Q)
	TABLE_ENTRY(GL_PIXEL_MAP_I_TO_I)
	TABLE_ENTRY(GL_PIXEL_MAP_S_TO_S)
	TABLE_ENTRY(GL_PIXEL_MAP_I_TO_R)
	TABLE_ENTRY(GL_PIXEL_MAP_I_TO_G)
	TABLE_ENTRY(GL_PIXEL_MAP_I_TO_B)
	TABLE_ENTRY(GL_PIXEL_MAP_I_TO_A)
	TABLE_ENTRY(GL_PIXEL_MAP_R_TO_R)
	TABLE_ENTRY(GL_PIXEL_MAP_G_TO_G)
	TABLE_ENTRY(GL_PIXEL_MAP_B_TO_B)
	TABLE_ENTRY(GL_PIXEL_MAP_A_TO_A)
	TABLE_ENTRY(GL_PIXEL_MAP_I_TO_I_SIZE)
	TABLE_ENTRY(GL_PIXEL_MAP_S_TO_S_SIZE)
	TABLE_ENTRY(GL_PIXEL_MAP_I_TO_R_SIZE)
	TABLE_ENTRY(GL_PIXEL_MAP_I_TO_G_SIZE)
	TABLE_ENTRY(GL_PIXEL_MAP_I_TO_B_SIZE)
	TABLE_ENTRY(GL_PIXEL_MAP_I_TO_A_SIZE)
	TABLE_ENTRY(GL_PIXEL_MAP_R_TO_R_SIZE)
	TABLE_ENTRY(GL_PIXEL_MAP_G_TO_G_SIZE)
	TABLE_ENTRY(GL_PIXEL_MAP_B_TO_B_SIZE)
	TABLE_ENTRY(GL_PIXEL_MAP_A_TO_A_SIZE)
	TABLE_ENTRY(GL_UNPACK_SWAP_BYTES)
	TABLE_ENTRY(GL_UNPACK_LSB_FIRST)
	TABLE_ENTRY(GL_UNPACK_ROW_LENGTH)
	TABLE_ENTRY(GL_UNPACK_SKIP_ROWS)
	TABLE_ENTRY(GL_UNPACK_SKIP_PIXELS)
	TABLE_ENTRY(GL_UNPACK_ALIGNMENT)
	TABLE_ENTRY(GL_PACK_SWAP_BYTES)
	TABLE_ENTRY(GL_PACK_LSB_FIRST)
	TABLE_ENTRY(GL_PACK_ROW_LENGTH)
	TABLE_ENTRY(GL_PACK_SKIP_ROWS)
	TABLE_ENTRY(GL_PACK_SKIP_PIXELS)
	TABLE_ENTRY(GL_PACK_ALIGNMENT)
	TABLE_ENTRY(GL_MAP_COLOR)
	TABLE_ENTRY(GL_MAP_STENCIL)
	TABLE_ENTRY(GL_INDEX_SHIFT)
	TABLE_ENTRY(GL_INDEX_OFFSET)
	TABLE_ENTRY(GL_RED_SCALE)
	TABLE_ENTRY(GL_RED_BIAS)
	TABLE_ENTRY(GL_ZOOM_X)
	TABLE_ENTRY(GL_ZOOM_Y)
	TABLE_ENTRY(GL_GREEN_SCALE)
	TABLE_ENTRY(GL_GREEN_BIAS)
	TABLE_ENTRY(GL_BLUE_SCALE)
	TABLE_ENTRY(GL_BLUE_BIAS)
	TABLE_ENTRY(GL_ALPHA_SCALE)
	TABLE_ENTRY(GL_ALPHA_BIAS)
	TABLE_ENTRY(GL_DEPTH_SCALE)
	TABLE_ENTRY(GL_DEPTH_BIAS)
	TABLE_ENTRY(GL_MAX_EVAL_ORDER)
	TABLE_ENTRY(GL_MAX_LIGHTS)
	TABLE_ENTRY(GL_MAX_CLIP_PLANES)
	TABLE_ENTRY(GL_MAX_TEXTURE_SIZE)
	TABLE_ENTRY(GL_MAX_PIXEL_MAP_TABLE)
	TABLE_ENTRY(GL_MAX_ATTRIB_STACK_DEPTH)
	TABLE_ENTRY(GL_MAX_MODELVIEW_STACK_DEPTH)
	TABLE_ENTRY(GL_MAX_NAME_STACK_DEPTH)
	TABLE_ENTRY(GL_MAX_PROJECTION_STACK_DEPTH)
	TABLE_ENTRY(GL_MAX_TEXTURE_STACK_DEPTH)
	TABLE_ENTRY(GL_MAX_VIEWPORT_DIMS)
	TABLE_ENTRY(GL_MAX_CLIENT_ATTRIB_STACK_DEPTH)
	TABLE_ENTRY(GL_SUBPIXEL_BITS)
	TABLE_ENTRY(GL_INDEX_BITS)
	TABLE_ENTRY(GL_RED_BITS)
	TABLE_ENTRY(GL_GREEN_BITS)
	TABLE_ENTRY(GL_BLUE_BITS)
	TABLE_ENTRY(GL_ALPHA_BITS)
	TABLE_ENTRY(GL_DEPTH_BITS)
	TABLE_ENTRY(GL_STENCIL_BITS)
	TABLE_ENTRY(GL_ACCUM_RED_BITS)
	TABLE_ENTRY(GL_ACCUM_GREEN_BITS)
	TABLE_ENTRY(GL_ACCUM_BLUE_BITS)
	TABLE_ENTRY(GL_ACCUM_ALPHA_BITS)
	TABLE_ENTRY(GL_NAME_STACK_DEPTH)
	TABLE_ENTRY(GL_AUTO_NORMAL)
	TABLE_ENTRY(GL_MAP1_COLOR_4)
	TABLE_ENTRY(GL_MAP1_INDEX)
	TABLE_ENTRY(GL_MAP1_NORMAL)
	TABLE_ENTRY(GL_MAP1_TEXTURE_COORD_1)
	TABLE_ENTRY(GL_MAP1_TEXTURE_COORD_2)
	TABLE_ENTRY(GL_MAP1_TEXTURE_COORD_3)
	TABLE_ENTRY(GL_MAP1_TEXTURE_COORD_4)
	TABLE_ENTRY(GL_MAP1_VERTEX_3)
	TABLE_ENTRY(GL_MAP1_VERTEX_4)
	TABLE_ENTRY(GL_MAP2_COLOR_4)
	TABLE_ENTRY(GL_MAP2_INDEX)
	TABLE_ENTRY(GL_MAP2_NORMAL)
	TABLE_ENTRY(GL_MAP2_TEXTURE_COORD_1)
	TABLE_ENTRY(GL_MAP2_TEXTURE_COORD_2)
	TABLE_ENTRY(GL_MAP2_TEXTURE_COORD_3)
	TABLE_ENTRY(GL_MAP2_TEXTURE_COORD_4)
	TABLE_ENTRY(GL_MAP2_VERTEX_3)
	TABLE_ENTRY(GL_MAP2_VERTEX_4)
	TABLE_ENTRY(GL_MAP1_GRID_DOMAIN)
	TABLE_ENTRY(GL_MAP1_GRID_SEGMENTS)
	TABLE_ENTRY(GL_MAP2_GRID_DOMAIN)
	TABLE_ENTRY(GL_MAP2_GRID_SEGMENTS)
	TABLE_ENTRY(GL_TEXTURE_1D)
	TABLE_ENTRY(GL_TEXTURE_2D)
	TABLE_ENTRY(GL_FEEDBACK_BUFFER_POINTER)
	TABLE_ENTRY(GL_FEEDBACK_BUFFER_SIZE)
	TABLE_ENTRY(GL_FEEDBACK_BUFFER_TYPE)
	TABLE_ENTRY(GL_SELECTION_BUFFER_POINTER)
	TABLE_ENTRY(GL_SELECTION_BUFFER_SIZE)
	TABLE_ENTRY(GL_TEXTURE_WIDTH)
	TABLE_ENTRY(GL_TEXTURE_HEIGHT)
	TABLE_ENTRY(GL_TEXTURE_INTERNAL_FORMAT)
	TABLE_ENTRY(GL_TEXTURE_BORDER_COLOR)
	TABLE_ENTRY(GL_TEXTURE_BORDER)
	TABLE_ENTRY(GL_DONT_CARE)
	TABLE_ENTRY(GL_FASTEST)
	TABLE_ENTRY(GL_NICEST)
	TABLE_ENTRY(GL_LIGHT0)
	TABLE_ENTRY(GL_LIGHT1)
	TABLE_ENTRY(GL_LIGHT2)
	TABLE_ENTRY(GL_LIGHT3)
	TABLE_ENTRY(GL_LIGHT4)
	TABLE_ENTRY(GL_LIGHT5)
	TABLE_ENTRY(GL_LIGHT6)
	TABLE_ENTRY(GL_LIGHT7)
	TABLE_ENTRY(GL_AMBIENT)
	TABLE_ENTRY(GL_DIFFUSE)
	TABLE_ENTRY(GL_SPECULAR)
	TABLE_ENTRY(GL_POSITION)
	TABLE_ENTRY(GL_SPOT_DIRECTION)
	TABLE_ENTRY(GL_SPOT_EXPONENT)
	TABLE_ENTRY(GL_SPOT_CUTOFF)
	TABLE_ENTRY(GL_CONSTANT_ATTENUATION)
	TABLE_ENTRY(GL_LINEAR_ATTENUATION)
	TABLE_ENTRY(GL_QUADRATIC_ATTENUATION)
	TABLE_ENTRY(GL_COMPILE)
	TABLE_ENTRY(GL_COMPILE_AND_EXECUTE)
	TABLE_ENTRY(GL_CLEAR)
	TABLE_ENTRY(GL_AND)
	TABLE_ENTRY(GL_AND_REVERSE)
	TABLE_ENTRY(GL_COPY)
	TABLE_ENTRY(GL_AND_INVERTED)
	TABLE_ENTRY(GL_NOOP)
	TABLE_ENTRY(GL_XOR)
	TABLE_ENTRY(GL_OR)
	TABLE_ENTRY(GL_NOR)
	TABLE_ENTRY(GL_EQUIV)
	TABLE_ENTRY(GL_INVERT)
	TABLE_ENTRY(GL_OR_REVERSE)
	TABLE_ENTRY(GL_COPY_INVERTED)
	TABLE_ENTRY(GL_OR_INVERTED)
	TABLE_ENTRY(GL_NAND)
	TABLE_ENTRY(GL_SET)
	TABLE_ENTRY(GL_EMISSION)
	TABLE_ENTRY(GL_SHININESS)
	TABLE_ENTRY(GL_AMBIENT_AND_DIFFUSE)
	TABLE_ENTRY(GL_COLOR_INDEXES)
	TABLE_ENTRY(GL_MODELVIEW)
	TABLE_ENTRY(GL_PROJECTION)
	TABLE_ENTRY(GL_TEXTURE)
	TABLE_ENTRY(GL_COLOR)
	TABLE_ENTRY(GL_DEPTH)
	TABLE_ENTRY(GL_STENCIL)
	TABLE_ENTRY(GL_COLOR_INDEX)
	TABLE_ENTRY(GL_STENCIL_INDEX)
	TABLE_ENTRY(GL_DEPTH_COMPONENT)
	TABLE_ENTRY(GL_RED)
	TABLE_ENTRY(GL_GREEN)
	TABLE_ENTRY(GL_BLUE)
	TABLE_ENTRY(GL_ALPHA)
	TABLE_ENTRY(GL_RGB)
	TABLE_ENTRY(GL_RGBA)
	TABLE_ENTRY(GL_LUMINANCE)
	TABLE_ENTRY(GL_LUMINANCE_ALPHA)
	TABLE_ENTRY(GL_BITMAP)
	TABLE_ENTRY(GL_POINT)
	TABLE_ENTRY(GL_LINE)
	TABLE_ENTRY(GL_FILL)
	TABLE_ENTRY(GL_RENDER)
	TABLE_ENTRY(GL_FEEDBACK)
	TABLE_ENTRY(GL_SELECT)
	TABLE_ENTRY(GL_FLAT)
	TABLE_ENTRY(GL_SMOOTH)
	TABLE_ENTRY(GL_KEEP)
	TABLE_ENTRY(GL_REPLACE)
	TABLE_ENTRY(GL_INCR)
	TABLE_ENTRY(GL_DECR)
	TABLE_ENTRY(GL_VENDOR)
	TABLE_ENTRY(GL_RENDERER)
	TABLE_ENTRY(GL_VERSION)
	TABLE_ENTRY(GL_EXTENSIONS)
	TABLE_ENTRY(GL_S)
	TABLE_ENTRY(GL_T)
	TABLE_ENTRY(GL_R)
	TABLE_ENTRY(GL_Q)
	TABLE_ENTRY(GL_MODULATE)
	TABLE_ENTRY(GL_DECAL)
	TABLE_ENTRY(GL_TEXTURE_ENV_MODE)
	TABLE_ENTRY(GL_TEXTURE_ENV_COLOR)
	TABLE_ENTRY(GL_TEXTURE_ENV)
	TABLE_ENTRY(GL_EYE_LINEAR)
	TABLE_ENTRY(GL_OBJECT_LINEAR)
	TABLE_ENTRY(GL_SPHERE_MAP)
	TABLE_ENTRY(GL_TEXTURE_GEN_MODE)
	TABLE_ENTRY(GL_OBJECT_PLANE)
	TABLE_ENTRY(GL_EYE_PLANE)
	TABLE_ENTRY(GL_NEAREST)
	TABLE_ENTRY(GL_LINEAR)
	TABLE_ENTRY(GL_NEAREST_MIPMAP_NEAREST)
	TABLE_ENTRY(GL_LINEAR_MIPMAP_NEAREST)
	TABLE_ENTRY(GL_NEAREST_MIPMAP_LINEAR)
	TABLE_ENTRY(GL_LINEAR_MIPMAP_LINEAR)
	TABLE_ENTRY(GL_TEXTURE_MAG_FILTER)
	TABLE_ENTRY(GL_TEXTURE_MIN_FILTER)
	TABLE_ENTRY(GL_TEXTURE_WRAP_S)
	TABLE_ENTRY(GL_TEXTURE_WRAP_T)
	TABLE_ENTRY(GL_CLAMP)
	TABLE_ENTRY(GL_REPEAT)
	TABLE_ENTRY(GL_POLYGON_OFFSET_FACTOR)
	TABLE_ENTRY(GL_POLYGON_OFFSET_UNITS)
	TABLE_ENTRY(GL_POLYGON_OFFSET_POINT)
	TABLE_ENTRY(GL_POLYGON_OFFSET_LINE)
	TABLE_ENTRY(GL_POLYGON_OFFSET_FILL)
	TABLE_ENTRY(GL_ALPHA4)
	TABLE_ENTRY(GL_ALPHA8)
	TABLE_ENTRY(GL_ALPHA12)
	TABLE_ENTRY(GL_ALPHA16)
	TABLE_ENTRY(GL_LUMINANCE4)
	TABLE_ENTRY(GL_LUMINANCE8)
	TABLE_ENTRY(GL_LUMINANCE12)
	TABLE_ENTRY(GL_LUMINANCE16)
	TABLE_ENTRY(GL_LUMINANCE4_ALPHA4)
	TABLE_ENTRY(GL_LUMINANCE6_ALPHA2)
	TABLE_ENTRY(GL_LUMINANCE8_ALPHA8)
	TABLE_ENTRY(GL_LUMINANCE12_ALPHA4)
	TABLE_ENTRY(GL_LUMINANCE12_ALPHA12)
	TABLE_ENTRY(GL_LUMINANCE16_ALPHA16)
	TABLE_ENTRY(GL_INTENSITY)
	TABLE_ENTRY(GL_INTENSITY4)
	TABLE_ENTRY(GL_INTENSITY8)
	TABLE_ENTRY(GL_INTENSITY12)
	TABLE_ENTRY(GL_INTENSITY16)
	TABLE_ENTRY(GL_R3_G3_B2)
	TABLE_ENTRY(GL_RGB4)
	TABLE_ENTRY(GL_RGB5)
	TABLE_ENTRY(GL_RGB8)
	TABLE_ENTRY(GL_RGB10)
	TABLE_ENTRY(GL_RGB12)
	TABLE_ENTRY(GL_RGB16)
	TABLE_ENTRY(GL_RGBA2)
	TABLE_ENTRY(GL_RGBA4)
	TABLE_ENTRY(GL_RGB5_A1)
	TABLE_ENTRY(GL_RGBA8)
	TABLE_ENTRY(GL_RGB10_A2)
	TABLE_ENTRY(GL_RGBA12)
	TABLE_ENTRY(GL_RGBA16)
	TABLE_ENTRY(GL_TEXTURE_RED_SIZE)
	TABLE_ENTRY(GL_TEXTURE_GREEN_SIZE)
	TABLE_ENTRY(GL_TEXTURE_BLUE_SIZE)
	TABLE_ENTRY(GL_TEXTURE_ALPHA_SIZE)
	TABLE_ENTRY(GL_TEXTURE_LUMINANCE_SIZE)
	TABLE_ENTRY(GL_TEXTURE_INTENSITY_SIZE)
	TABLE_ENTRY(GL_PROXY_TEXTURE_1D)
	TABLE_ENTRY(GL_PROXY_TEXTURE_2D)
	TABLE_ENTRY(GL_TEXTURE_PRIORITY)
	TABLE_ENTRY(GL_TEXTURE_RESIDENT)
	TABLE_ENTRY(GL_TEXTURE_BINDING_1D)
	TABLE_ENTRY(GL_TEXTURE_BINDING_2D)
	TABLE_ENTRY(GL_VERTEX_ARRAY)
	TABLE_ENTRY(GL_NORMAL_ARRAY)
	TABLE_ENTRY(GL_COLOR_ARRAY)
	TABLE_ENTRY(GL_INDEX_ARRAY)
	TABLE_ENTRY(GL_TEXTURE_COORD_ARRAY)
	TABLE_ENTRY(GL_EDGE_FLAG_ARRAY)
	TABLE_ENTRY(GL_VERTEX_ARRAY_SIZE)
	TABLE_ENTRY(GL_VERTEX_ARRAY_TYPE)
	TABLE_ENTRY(GL_VERTEX_ARRAY_STRIDE)
	TABLE_ENTRY(GL_NORMAL_ARRAY_TYPE)
	TABLE_ENTRY(GL_NORMAL_ARRAY_STRIDE)
	TABLE_ENTRY(GL_COLOR_ARRAY_SIZE)
	TABLE_ENTRY(GL_COLOR_ARRAY_TYPE)
	TABLE_ENTRY(GL_COLOR_ARRAY_STRIDE)
	TABLE_ENTRY(GL_INDEX_ARRAY_TYPE)
	TABLE_ENTRY(GL_INDEX_ARRAY_STRIDE)
	TABLE_ENTRY(GL_TEXTURE_COORD_ARRAY_SIZE)
	TABLE_ENTRY(GL_TEXTURE_COORD_ARRAY_TYPE)
	TABLE_ENTRY(GL_TEXTURE_COORD_ARRAY_STRIDE)
	TABLE_ENTRY(GL_EDGE_FLAG_ARRAY_STRIDE)
	TABLE_ENTRY(GL_VERTEX_ARRAY_POINTER)
	TABLE_ENTRY(GL_NORMAL_ARRAY_POINTER)
	TABLE_ENTRY(GL_COLOR_ARRAY_POINTER)
	TABLE_ENTRY(GL_INDEX_ARRAY_POINTER)
	TABLE_ENTRY(GL_TEXTURE_COORD_ARRAY_POINTER)
	TABLE_ENTRY(GL_EDGE_FLAG_ARRAY_POINTER)
	TABLE_ENTRY(GL_V2F)
	TABLE_ENTRY(GL_V3F)
	TABLE_ENTRY(GL_C4UB_V2F)
	TABLE_ENTRY(GL_C4UB_V3F)
	TABLE_ENTRY(GL_C3F_V3F)
	TABLE_ENTRY(GL_N3F_V3F)
	TABLE_ENTRY(GL_C4F_N3F_V3F)
	TABLE_ENTRY(GL_T2F_V3F)
	TABLE_ENTRY(GL_T4F_V4F)
	TABLE_ENTRY(GL_T2F_C4UB_V3F)
	TABLE_ENTRY(GL_T2F_C3F_V3F)
	TABLE_ENTRY(GL_T2F_N3F_V3F)
	TABLE_ENTRY(GL_T2F_C4F_N3F_V3F)
	TABLE_ENTRY(GL_T4F_C4F_N3F_V4F)
	TABLE_ENTRY(GL_ABGR_EXT)
	TABLE_ENTRY(GL_CONSTANT_COLOR_EXT)
	TABLE_ENTRY(GL_ONE_MINUS_CONSTANT_COLOR_EXT)
	TABLE_ENTRY(GL_CONSTANT_ALPHA_EXT)
	TABLE_ENTRY(GL_ONE_MINUS_CONSTANT_ALPHA_EXT)
	TABLE_ENTRY(GL_BLEND_COLOR_EXT)
	TABLE_ENTRY(GL_FUNC_ADD_EXT)
	TABLE_ENTRY(GL_MIN_EXT)
	TABLE_ENTRY(GL_MAX_EXT)
	TABLE_ENTRY(GL_BLEND_EQUATION_EXT)
	TABLE_ENTRY(GL_FUNC_SUBTRACT_EXT)
	TABLE_ENTRY(GL_FUNC_REVERSE_SUBTRACT_EXT)
	TABLE_ENTRY(GL_ARRAY_ELEMENT_LOCK_FIRST_EXT)
	TABLE_ENTRY(GL_ARRAY_ELEMENT_LOCK_COUNT_EXT)
	TABLE_ENTRY(GL_TEXTURE0_ARB)
	TABLE_ENTRY(GL_TEXTURE1_ARB)
	TABLE_ENTRY(GL_TEXTURE2_ARB)
	TABLE_ENTRY(GL_TEXTURE3_ARB)
	TABLE_ENTRY(GL_TEXTURE4_ARB)
	TABLE_ENTRY(GL_TEXTURE5_ARB)
	TABLE_ENTRY(GL_TEXTURE6_ARB)
	TABLE_ENTRY(GL_TEXTURE7_ARB)
	TABLE_ENTRY(GL_TEXTURE8_ARB)
	TABLE_ENTRY(GL_TEXTURE9_ARB)
	TABLE_ENTRY(GL_TEXTURE10_ARB)
	TABLE_ENTRY(GL_TEXTURE11_ARB)
	TABLE_ENTRY(GL_TEXTURE12_ARB)
	TABLE_ENTRY(GL_TEXTURE13_ARB)
	TABLE_ENTRY(GL_TEXTURE14_ARB)
	TABLE_ENTRY(GL_TEXTURE15_ARB)
	TABLE_ENTRY(GL_TEXTURE16_ARB)
	TABLE_ENTRY(GL_TEXTURE17_ARB)
	TABLE_ENTRY(GL_TEXTURE18_ARB)
	TABLE_ENTRY(GL_TEXTURE19_ARB)
	TABLE_ENTRY(GL_TEXTURE20_ARB)
	TABLE_ENTRY(GL_TEXTURE21_ARB)
	TABLE_ENTRY(GL_TEXTURE22_ARB)
	TABLE_ENTRY(GL_TEXTURE23_ARB)
	TABLE_ENTRY(GL_TEXTURE24_ARB)
	TABLE_ENTRY(GL_TEXTURE25_ARB)
	TABLE_ENTRY(GL_TEXTURE26_ARB)
	TABLE_ENTRY(GL_TEXTURE27_ARB)
	TABLE_ENTRY(GL_TEXTURE28_ARB)
	TABLE_ENTRY(GL_TEXTURE29_ARB)
	TABLE_ENTRY(GL_TEXTURE30_ARB)
	TABLE_ENTRY(GL_TEXTURE31_ARB)
	TABLE_ENTRY(GL_ACTIVE_TEXTURE_ARB)
	TABLE_ENTRY(GL_CLIENT_ACTIVE_TEXTURE_ARB)
	TABLE_ENTRY(GL_MAX_TEXTURE_UNITS_ARB)
	TABLE_ENTRY(GL_LIGHT_MODEL_SPECULAR_VECTOR_APPLE)
	TABLE_ENTRY(GL_TRANSFORM_HINT_APPLE)
};


char * glr_debug_enum_to_string(GLenum enum_value)
{
	GLint i;
	static char temp_buffer[8][128];
	static GLint buffer_counter = 0;
	
	for(i = 0; i < (sizeof enum_to_string_table / sizeof (EnumTableEntry)); i++){
		if(enum_value == enum_to_string_table[i].value){
			return enum_to_string_table[i].name;
		}
	}

	buffer_counter = (buffer_counter + 1) % 8;
	
	sprintf(temp_buffer[buffer_counter], "unknown enum (0x%08X)", enum_value);
	
	return temp_buffer[buffer_counter];
}

#define ets glr_debug_enum_to_string

void glr_debug_print_GLDState( void * inFunc, GLDState* inState )
{
	DEBUG_VERBOSE( inFunc, "==> state @ 0x%08x :\n", inState );
	DEBUG_VERBOSE( inFunc, "   - major_version = 0x%04x, minor_version = 0x%04x\n", inState->major_version, inState->minor_version );
	DEBUG_VERBOSE( inFunc, "   - aplpha test is %s, ( func = %s, ref = %.2f )\n", ( inState->alpha_test.enable ? "ENABLED" : "disabled" ), ets(inState->alpha_test.func), inState->alpha_test.ref );
	DEBUG_VERBOSE( inFunc, "   - blending operation is %s ( src = %s, dst = %s, equation = %s )\n", ( inState->blend_mode.enable ? "ENABLED" : "disabled" ), ets(inState->blend_mode.src), ets(inState->blend_mode.dst), ets(inState->blend_mode.equation) );
	DEBUG_VERBOSE( inFunc, "   - clear depth ...\n" );
	DEBUG_VERBOSE( inFunc, "   - color buffer settings : read = %s, draw = %s\n", ets(inState->color_buffer.read), ets(inState->color_buffer.draw) );
	DEBUG_VERBOSE( inFunc, "   - depth mode is %s, ( func = %s )\n", ( inState->depth_test.enable ? "ENABLED" : "disabled" ), ets(inState->depth_test.func) );
	DEBUG_VERBOSE( inFunc, "   - dither mode is %s\n", ( inState->dither_mode.enable ? "ENABLED" : "disabled" ) );
	DEBUG_VERBOSE( inFunc, "   - fog mode is %s\n", ( inState->fog_mode.enable ? "ENABLED" : "disabled" ) );
	DEBUG_VERBOSE( inFunc, "   - hint mode : perspective correct. = %s, point = %s, line = %s, poly = %s, fog = %s\n", ets(inState->hint_mode.perspective_correction), ets(inState->hint_mode.point_smooth), ets(inState->hint_mode.line_smooth), ets(inState->hint_mode.polygon_smooth), ets(inState->hint_mode.fog) );
	DEBUG_VERBOSE( inFunc, "   - lines : w = %f, stipple factor = %d, stipple pattern = 0x%04x, stipple = %s, smooth = %s\n", inState->line_mode.width, inState->line_mode.stipple_factor, inState->line_mode.stipple_pattern, ( inState->line_mode.stipple_enable ? "ENABLED" : "disabled" ), ( inState->line_mode.smooth_enable ? "ENABLED" : "disabled" ) );
	DEBUG_VERBOSE( inFunc, "   - logical operation : opcode = %s, index = %s, color = %s\n", 
		ets(inState->logic_op.opcode), 
		( inState->logic_op.index_enable ? "ENABLED" : "disabled" ), 
		( inState->logic_op.color_enable ? "ENABLED" : "disabled" ) );
	DEBUG_VERBOSE( inFunc, "   - buffer masks : index = %d, stencil = %d, r = %d, g = %d, b = %d, a = %d, d = %d\n", inState->mask_mode.index_mask, inState->mask_mode.stencil_mask, inState->mask_mode.color_red_mask, inState->mask_mode.color_green_mask, inState->mask_mode.color_blue_mask, inState->mask_mode.color_alpha_mask, inState->mask_mode.depth_mask );
	DEBUG_VERBOSE( inFunc, "   - pixel mode : zoom x = %f, zoom_y = %f, ...\n", inState->pixel_mode.zoom_x, inState->pixel_mode.zoom_y );
	DEBUG_VERBOSE( inFunc, "   - point mode : zoom x = %f, smooth = %s\n", inState->point_mode.size, ( inState->point_mode.smooth_enable ? "ENABLED" : "disabled" ) );
	DEBUG_VERBOSE( inFunc, "   - polygon mode : ...\n" );
	DEBUG_VERBOSE( inFunc, "   - scissor box (%s): x = %d, y = %d, w = %d, h = %d\n", ( inState->scissor_test.enable ? "ENABLED" : "disabled" ), inState->scissor_test.box.x, inState->scissor_test.box.y, inState->scissor_test.box.w, inState->scissor_test.box.h  );
	DEBUG_VERBOSE( inFunc, "   - shade model : %s\n", ets(inState->shade_model)  );
	DEBUG_VERBOSE( inFunc, "   - stencil model (%s)\n", ( inState->stencil_test.enable ? "ENABLED" : "disabled" ) );
	DEBUG_VERBOSE( inFunc, "   - texture[0] : env. mode = %s, env. color = r%.2f / g%.2f / b%.2f / a%.2f, 1d = %s, 2d = %s\n", ets(inState->texture_mode[0].env_mode), inState->texture_mode[0].env_color.r, inState->texture_mode[0].env_color.g, inState->texture_mode[0].env_color.b, inState->texture_mode[0].env_color.a, ( inState->texture_mode[0].enable_1d ? "ENABLED" : "disabled" ), ( inState->texture_mode[0].enable_2d ? "ENABLED" : "disabled" ) );
	DEBUG_VERBOSE( inFunc, "   - texture[1] : env. mode = %s, env. color = r%.2f / g%.2f / b%.2f / a%.2f, 1d = %s, 2d = %s\n", ets(inState->texture_mode[1].env_mode), inState->texture_mode[1].env_color.r, inState->texture_mode[1].env_color.g, inState->texture_mode[1].env_color.b, inState->texture_mode[1].env_color.a, ( inState->texture_mode[1].enable_1d ? "ENABLED" : "disabled" ), ( inState->texture_mode[1].enable_2d ? "ENABLED" : "disabled" ) );
}

void glr_debug_print_GLDConfig( void * inFunc, GLDConfig* inConfig)
{
	DEBUG_VERBOSE( inFunc, "==> config @ 0x%08x :\n", inConfig );
	DEBUG_VERBOSE( inFunc, "   - view_flags = 0x%08x\n", inConfig->view_flags );
	DEBUG_VERBOSE( inFunc, "   - depth scale = %f\n", inConfig->depth_scale );
	DEBUG_VERBOSE( inFunc, "   - subpixel bits = 0x%08x\n", inConfig->subpixel_bits );
	DEBUG_VERBOSE( inFunc, "   - drawable max. : w = %d, h = %d\n", inConfig->drawable_max.w, inConfig->drawable_max.h );
	DEBUG_VERBOSE( inFunc, "   - pixel format : rgba = %s, double buffer = %s, stereo = %s\n", ( inConfig->pixel_format.rgba_mode ? "ENABLED" : "disabled" ), ( inConfig->pixel_format.double_buffer ? "ENABLED" : "disabled" ), ( inConfig->pixel_format.stereo_mode ? "ENABLED" : "disabled" ));
	DEBUG_VERBOSE( inFunc, "   - pixel format : buffer size = %d, red = %d, green = %d, blue = %d, alpha = %d, depth = %d\n", inConfig->pixel_format.buffer_size, inConfig->pixel_format.red_size, inConfig->pixel_format.green_size, inConfig->pixel_format.blue_size, inConfig->pixel_format.alpha_size, inConfig->pixel_format.depth_size);
	DEBUG_VERBOSE( inFunc, "   - pixel format : stencil size = %d, accum red = %d, accum green = %d, accum blue = %d, accum alpha = %d, aux buffers = %d\n", inConfig->pixel_format.stencil_size, inConfig->pixel_format.accum_red_size, inConfig->pixel_format.accum_green_size, inConfig->pixel_format.accum_blue_size, inConfig->pixel_format.accum_alpha_size, inConfig->pixel_format.aux_buffers);
	DEBUG_VERBOSE( inFunc, "   - point size : min = %.2f, max = %.2f, granularity = %.2f,\n", inConfig->point_size.min, inConfig->point_size.max, inConfig->point_size.granularity);
	DEBUG_VERBOSE( inFunc, "   - line width : min = %.2f, max = %.2f, granularity = %.2f,\n", inConfig->line_width.min, inConfig->line_width.max, inConfig->line_width.granularity);
	DEBUG_VERBOSE( inFunc, "   - rendering features : culling = %s\n", ( inConfig->features.does_culling ? "yes" : "no" ));
	DEBUG_VERBOSE( inFunc, "   - vertex type : coord color = %d, color tex = %d, color 2tex = %d,\n", inConfig->vertex_type.coord_color, inConfig->vertex_type.coord_color_tex, inConfig->vertex_type.coord_color_2tex );
	DEBUG_VERBOSE( inFunc, "   - vertex type : coord color fog = %d, color tex fog = %d, color fog 2tex = %d,\n", inConfig->vertex_type.coord_color_fog, inConfig->vertex_type.coord_color_fog_tex, inConfig->vertex_type.coord_color_fog_2tex );
	DEBUG_VERBOSE( inFunc, "   - vertex type : coord_color = %d\n", inConfig->vertex_type.coord_color, inConfig->vertex_type.coord_color_tex, inConfig->vertex_type.coord_color_2tex );
}

void glr_debug_print_pixel_format( void * inFunc, GLIPixelFormat* inPixelFormat )
{
	while ( inPixelFormat )
	{
		DEBUG_VERBOSE( gldChoosePixelFormat, "==> Pixel Format @ 0x%08x :\n", inPixelFormat );
		DEBUG_VERBOSE( gldChoosePixelFormat, "    Renderer ID = 0x%08x\n", inPixelFormat->renderer_id );
		DEBUG_VERBOSE( gldChoosePixelFormat, "    os support = 0x%08x\n", inPixelFormat->os_support );
			DEBUG_PRINT_OS_SUPPORT( gldChoosePixelFormat, inPixelFormat->os_support );
		DEBUG_VERBOSE( gldChoosePixelFormat, "    buffer mode = 0x%08x\n", inPixelFormat->buffer_mode );
			DEBUG_PRINT_BUFFER_MODE( gldChoosePixelFormat, inPixelFormat->buffer_mode );
		DEBUG_VERBOSE( gldChoosePixelFormat, "    color mode = 0x%08x\n", inPixelFormat->buffer_mode );
			DEBUG_PRINT_COLOR_MODE( gldChoosePixelFormat, inPixelFormat->color_mode );
		DEBUG_VERBOSE( gldChoosePixelFormat, "    accumulation buffer size = 0x%08x\n", inPixelFormat->accum_mode );
			DEBUG_PRINT_BUFFER_DEPTH( gldChoosePixelFormat, inPixelFormat->accum_mode );
		DEBUG_VERBOSE( gldChoosePixelFormat, "    depth buffer size = 0x%08x\n", inPixelFormat->depth_mode );
			DEBUG_PRINT_BUFFER_DEPTH( gldChoosePixelFormat, inPixelFormat->depth_mode );
		DEBUG_VERBOSE( gldChoosePixelFormat, "    stencil buffer size = 0x%08x\n", inPixelFormat->stencil_mode );
			DEBUG_PRINT_BUFFER_DEPTH( gldChoosePixelFormat, inPixelFormat->stencil_mode );
		DEBUG_VERBOSE( gldChoosePixelFormat, "    buffer level = 0x%08x\n", inPixelFormat->buffer_level );
		DEBUG_VERBOSE( gldChoosePixelFormat, "    aux buffers = %d\n", inPixelFormat->aux_buffers );
		DEBUG_VERBOSE( gldChoosePixelFormat, "    number of devices = %d\n", inPixelFormat->num_devices );
		{
			short i = 0;
			while ( i < inPixelFormat->num_devices )
			{
				DEBUG_VERBOSE( gldChoosePixelFormat, "    device[ %d ] = 0x%08x\n", i+1, inPixelFormat->devices[i] );
				i++;
			}
		}
		
		inPixelFormat = inPixelFormat->next_pixel_format;
	}
}

void glr_debug_print_renderer_info( void * inFunc, GLIRendererInfo* inInfo )
{
	while ( inInfo )
	{
		DEBUG_VERBOSE( gldGetRendererInfo, "  Renderer Info :\n" );
		DEBUG_VERBOSE( gldGetRendererInfo, "      Renderer ID = 0x%08x\n", inInfo->renderer_id );
		DEBUG_VERBOSE( gldGetRendererInfo, "      os support = 0x%08x\n", inInfo->os_support );
			DEBUG_PRINT_OS_SUPPORT( gldGetRendererInfo, inInfo->os_support );
		DEBUG_VERBOSE( gldGetRendererInfo, "      buffer mode = 0x%08x\n", inInfo->buffer_modes );
			DEBUG_PRINT_BUFFER_MODE( gldGetRendererInfo, inInfo->buffer_modes );
		DEBUG_VERBOSE( gldGetRendererInfo, "      color mode = 0x%08x\n", inInfo->buffer_modes );
			DEBUG_PRINT_COLOR_MODE( gldGetRendererInfo, inInfo->color_modes );
		DEBUG_VERBOSE( gldGetRendererInfo, "      accumulation buffer size = 0x%08x\n", inInfo->accum_modes );
			DEBUG_PRINT_BUFFER_DEPTH( gldGetRendererInfo, inInfo->accum_modes );
		DEBUG_VERBOSE( gldGetRendererInfo, "      depth buffer size = 0x%08x\n", inInfo->depth_modes );
			DEBUG_PRINT_BUFFER_DEPTH( gldGetRendererInfo, inInfo->depth_modes );
		DEBUG_VERBOSE( gldGetRendererInfo, "      stencil buffer size = 0x%08x\n", inInfo->stencil_modes );
			DEBUG_PRINT_BUFFER_DEPTH( gldGetRendererInfo, inInfo->stencil_modes );
		DEBUG_VERBOSE( gldGetRendererInfo, "      buffer levels: min = 0x%08x, max = 0x%08x\n", inInfo->min_buffer_level , inInfo->max_buffer_level );
		DEBUG_VERBOSE( gldGetRendererInfo, "      max aux buffers = %d\n", inInfo->max_aux_buffers );
		DEBUG_VERBOSE( gldGetRendererInfo, "      video memory = %d\n", inInfo->video_memory );
		DEBUG_VERBOSE( gldGetRendererInfo, "      texture memory = %d\n", inInfo->texture_memory );
		DEBUG_VERBOSE( gldGetRendererInfo, "      next renderer info = 0x%08x\n", inInfo->next_renderer_info );
		
		inInfo = inInfo->next_renderer_info;
	}
}

void glr_debug_print_os_support( void * inFunc, GLbitfield inValue )
{
	if ( inValue & GLI_WINDOW_BIT )
	{
	DEBUG_VERBOSE( inFunc, "          - windowed drawable\n" );
	}
	if ( inValue & GLI_FULLSCREEN_BIT )
	{
	DEBUG_VERBOSE( inFunc, "          - fullscreen drawable\n" );
	}
	if ( inValue & GLI_OFFSCREEN_BIT )
	{
	DEBUG_VERBOSE( inFunc, "          - fullscreen drawable\n" );
	}
	if ( inValue & GLI_BACKING_STORE_BIT )
	{
	DEBUG_VERBOSE( inFunc, "          - backing store\n" );
	}
	if ( inValue & GLI_MP_SAFE_BIT )
	{
	DEBUG_VERBOSE( inFunc, "          - MP safe\n" );
	}
	if ( inValue & GLI_AUTO_UPDATE_BIT )
	{
	DEBUG_VERBOSE( inFunc, "          - auto update\n" );
	}
	if ( inValue & GLI_ROBUST_BIT )
	{
	DEBUG_VERBOSE( inFunc, "          - robust\n" );
	}
	if ( inValue & GLI_RECOVERABLE_BIT )
	{
	DEBUG_VERBOSE( inFunc, "          - recoverable\n" );
	}
	if ( inValue & GLI_ACCELERATED_BIT )
	{
	DEBUG_VERBOSE( inFunc, "          - accelerated\n" );
	}
	if ( inValue & GLI_MULTISCREEN_BIT )
	{
	DEBUG_VERBOSE( inFunc, "          - multiscreen\n" );
	}
	if ( inValue & GLI_COMPLIANT_BIT )
	{
	DEBUG_VERBOSE( inFunc, "          - compliant\n" );
	}
}

void glr_debug_print_buffer_mode( void * inFunc, GLbitfield inValue )
{
	if ( inValue & GLI_MONOSCOPIC_BIT )
	{
	DEBUG_VERBOSE( inFunc, "          - monoscopic\n" );
	}
	if ( inValue & GLI_STEREOSCOPIC_BIT )
	{
	DEBUG_VERBOSE( inFunc, "          - stereoscopic\n" );
	}
	if ( inValue & GLI_SINGLEBUFFER_BIT )
	{
	DEBUG_VERBOSE( inFunc, "          - single buffer\n" );
	}
	if ( inValue & GLI_DOUBLEBUFFER_BIT )
	{
	DEBUG_VERBOSE( inFunc, "          - double buffer\n" );
	}
}

void glr_debug_print_buffer_depth( void * inFunc, GLbitfield inValue )
{
	if ( inValue == 0 )
	{
	DEBUG_VERBOSE( inFunc, "          - none\n" );
	}
	if ( inValue & GLI_0_BIT )
	{
	DEBUG_VERBOSE( inFunc, "          - 0 bit\n" );
	}
	if ( inValue & GLI_1_BIT )
	{
	DEBUG_VERBOSE( inFunc, "          - 1 bit\n" );
	}
	if ( inValue & GLI_2_BIT )
	{
	DEBUG_VERBOSE( inFunc, "          - 2 bit\n" );
	}
	if ( inValue & GLI_3_BIT )
	{
	DEBUG_VERBOSE( inFunc, "          - 3 bit\n" );
	}
	if ( inValue & GLI_4_BIT )
	{
	DEBUG_VERBOSE( inFunc, "          - 4 bit\n" );
	}
	if ( inValue & GLI_5_BIT )
	{
	DEBUG_VERBOSE( inFunc, "          - 5 bit\n" );
	}
	if ( inValue & GLI_6_BIT )
	{
	DEBUG_VERBOSE( inFunc, "          - 6 bit\n" );
	}
	if ( inValue & GLI_8_BIT )
	{
	DEBUG_VERBOSE( inFunc, "          - 8 bit\n" );
	}
	if ( inValue & GLI_10_BIT )
	{
	DEBUG_VERBOSE( inFunc, "          - 10 bit\n" );
	}
	if ( inValue & GLI_12_BIT )
	{
	DEBUG_VERBOSE( inFunc, "          - 12 bit\n" );
	}
	if ( inValue & GLI_16_BIT )
	{
	DEBUG_VERBOSE( inFunc, "          - 16 bit\n" );
	}
	if ( inValue & GLI_24_BIT )
	{
	DEBUG_VERBOSE( inFunc, "          - 24 bit\n" );
	}
	if ( inValue & GLI_32_BIT )
	{
	DEBUG_VERBOSE( inFunc, "          - 32 bit\n" );
	}
	if ( inValue & GLI_48_BIT )
	{
	DEBUG_VERBOSE( inFunc, "          - 48 bit\n" );
	}
	if ( inValue & GLI_64_BIT )
	{
	DEBUG_VERBOSE( inFunc, "          - 64 bit\n" );
	}
	if ( inValue & GLI_96_BIT )
	{
	DEBUG_VERBOSE( inFunc, "          - 96 bit\n" );
	}
	if ( inValue & GLI_128_BIT )
	{
	DEBUG_VERBOSE( inFunc, "          - 128 bit\n" );
	}
}

void glr_debug_print_color_mode( void * inFunc, GLbitfield inValue )
{
	if ( inValue & GLI_RGB8_BIT )
	{
	DEBUG_VERBOSE( inFunc, "          - 8 rgb bit/pixel,     RGB=7:0, inverse colormap\n" );
	}
	if ( inValue & GLI_RGB8_A8_BIT )
	{
	DEBUG_VERBOSE( inFunc, "          - 8-8 argb bit/pixel,  A=7:0, RGB=7:0, inverse colormap\n" );
	}
	if ( inValue & GLI_BGR233_BIT )
	{
	DEBUG_VERBOSE( inFunc, "          - 8 rgb bit/pixel,     B=7:6, G=5:3, R=2:0 \n" );
	}
	if ( inValue & GLI_BGR233_A8_BIT )
	{
	DEBUG_VERBOSE( inFunc, "          - 8-8 argb bit/pixel,  A=7:0, B=7:6, G=5:3, R=2:0\n" );
	}
	if ( inValue & GLI_RGB332_BIT )
	{
	DEBUG_VERBOSE( inFunc, "          - 8 rgb bit/pixel,     R=7:5, G=4:2, B=1:0\n" );
	}
	if ( inValue & GLI_RGB444_BIT )
	{
	DEBUG_VERBOSE( inFunc, "          - 16 rgb bit/pixel,    R=11:8, G=7:4, B=3:0\n" );
	}
	if ( inValue & GLI_ARGB4444_BIT )
	{
	DEBUG_VERBOSE( inFunc, "          - 16 argb bit/pixel,   A=15:12, R=11:8, G=7:4, B=3:0\n" );
	}
	if ( inValue & GLI_RGB444_A8_BIT )
	{
	DEBUG_VERBOSE( inFunc, "          - 8-16 argb bit/pixel, A=7:0, R=11:8, G=7:4, B=3:0\n" );
	}
	if ( inValue & GLI_RGB555_BIT )
	{
	DEBUG_VERBOSE( inFunc, "          - 16 rgb bit/pixel,    R=14:10, G=9:5, B=4:0\n" );
	}
	if ( inValue & GLI_ARGB1555_BIT )
	{
	DEBUG_VERBOSE( inFunc, "          - 16 argb bit/pixel,   A=15, R=14:10, G=9:5, B=4:0 \n" );
	}
	if ( inValue & GLI_RGB555_A8_BIT )
	{
	DEBUG_VERBOSE( inFunc, "          - 8-16 argb bit/pixel, A=7:0, R=14:10, G=9:5, B=4:0 \n" );
	}
	if ( inValue & GLI_RGB565_BIT )
	{
	DEBUG_VERBOSE( inFunc, "          - 16 rgb bit/pixel,    R=15:11, G=10:5, B=4:0\n" );
	}
	if ( inValue & GLI_RGB565_A8_BIT )
	{
	DEBUG_VERBOSE( inFunc, "          - 8-16 argb bit/pixel, A=7:0, R=15:11, G=10:5, B=4:0\n" );
	}
	if ( inValue & GLI_RGB888_BIT )
	{
	DEBUG_VERBOSE( inFunc, "          - 32 rgb bit/pixel,    R=23:16, G=15:8, B=7:0\n" );
	}
	if ( inValue & GLI_ARGB8888_BIT )
	{
	DEBUG_VERBOSE( inFunc, "          - 32 argb bit/pixel,   A=31:24, R=23:16, G=15:8, B=7:0\n" );
	}
	if ( inValue & GLI_RGB888_A8_BIT )
	{
	DEBUG_VERBOSE( inFunc, "          - 8-32 argb bit/pixel, A=7:0, R=23:16, G=15:8, B=7:0\n" );
	}
	if ( inValue & GLI_RGB101010_BIT )
	{
	DEBUG_VERBOSE( inFunc, "          - 32 rgb bit/pixel,    R=29:20, G=19:10, B=9:0\n" );
	}
	if ( inValue & GLI_ARGB2101010_BIT )
	{
	DEBUG_VERBOSE( inFunc, "          - 32 argb bit/pixel,   A=31:30  R=29:20, G=19:10, B=9:0\n" );
	}
	if ( inValue & GLI_RGB101010_A8_BIT )
	{
	DEBUG_VERBOSE( inFunc, "          - 8-32 argb bit/pixel, A=7:0  R=29:20, G=19:10, B=9:0\n" );
	}
	if ( inValue & GLI_RGB121212_BIT )
	{
	DEBUG_VERBOSE( inFunc, "          - 64 rgb bit/pixel,    R=35:24, G=23:12, B=11:0\n" );
	}
	if ( inValue & GLI_ARGB12121212_BIT )
	{
	DEBUG_VERBOSE( inFunc, "          - 64 argb bit/pixel,   A=47:36, R=35:24, G=23:12, B=11:0\n" );
	}
	if ( inValue & GLI_RGB161616_BIT )
	{
	DEBUG_VERBOSE( inFunc, "          - 64 rgb bit/pixel,    R=47:32, G=31:16, B=15:0\n" );
	}
	if ( inValue & GLI_ARGB16161616_BIT )
	{
	DEBUG_VERBOSE( inFunc, "          - 64 argb bit/pixel,   A=63:48, R=47:32, G=31:16, B=15:0\n" );
	}
	if ( inValue & GLI_INDEX8_BIT )
	{
	DEBUG_VERBOSE( inFunc, "          - 8 bit color look up table\n" );
	}
	if ( inValue & GLI_INDEX16_BIT )
	{
	DEBUG_VERBOSE( inFunc, "          - 16 bit color look up table\n" );
	}
}