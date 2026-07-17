#ifndef __glr_Debug_Struct_H__
#define __glr_Debug_Struct_H__

#include "glr.h"
#include "glr_debug.h"


#if GLR_DEBUG
#  define DEBUG_PRINT_RENDERER_INFO     glr_debug_print_renderer_info
#  define DEBUG_PRINT_GLDSTATE          glr_debug_print_GLDState
#  define DEBUG_PRINT_GLDCONFIG         glr_debug_print_GLDConfig
#  define DEBUG_PRINT_PIXEL_FORMAT      glr_debug_print_pixel_format
#  define DEBUG_PRINT_COLOR_MODE        glr_debug_print_color_mode
#  define DEBUG_PRINT_OS_SUPPORT        glr_debug_print_os_support
#  define DEBUG_PRINT_BUFFER_MODE       glr_debug_print_buffer_mode
#  define DEBUG_PRINT_BUFFER_DEPTH      glr_debug_print_buffer_depth
#else
#  define DEBUG_PRINT_RENDERER_INFO     0 && (unsigned long)
#  define DEBUG_PRINT_GLDSTATE          0 && (unsigned long)
#  define DEBUG_PRINT_GLDCONFIG         0 && (unsigned long)
#  define DEBUG_PRINT_PIXEL_FORMAT      0 && (unsigned long)
#  define DEBUG_PRINT_COLOR_MODE        0 && (unsigned long)
#  define DEBUG_PRINT_OS_SUPPORT        0 && (unsigned long)
#  define DEBUG_PRINT_BUFFER_MODE       0 && (unsigned long)
#  define DEBUG_PRINT_BUFFER_DEPTH      0 && (unsigned long)
#endif


#define DEBUG_DUMP_GLDVERTEX(d) \
	DEBUG_PRINTF( "vertex(%s) win.x = %.1f %.1f, r = %.1f, g = %.1f, b = %.1f, a = %.1f\n", #d, (d).window.x,  (d).window.y,  (d).color.r,  (d).color.g,  (d).color.b,  (d).color.a)



#define DEBUG_DUMP_VERTEX(v)\
{\
	glr_debug_printf( " vertex (" #v ") : x = %.1f, y = %.1f, ooz = %.2f, a = %.0f, r = %.0f, g = %.0f, b = %.0f\n", \
	(v).x, (v).y, (v).ooz, \
	(FxFloat)(((v).pargb >> 24) & 0xff), \
	(FxFloat)(((v).pargb >> 16) & 0xff), \
	(FxFloat)(((v).pargb >> 8) & 0xff), \
	(FxFloat)(((v).pargb >> 0) & 0xff)); \
}

char * glr_debug_enum_to_string(GLenum enum_value);

void glr_debug_print_GLDState( void * inFunc, GLDState* inState );
void glr_debug_print_GLDConfig( void * inFunc, GLDConfig* inConfig);
void glr_debug_print_renderer_info( void * inFunc, GLIRendererInfo* inInfo );
void glr_debug_print_pixel_format( void * inFunc, GLIPixelFormat* inPixelFormat );
void glr_debug_print_os_support( void * inFunc, GLbitfield inValue );
void glr_debug_print_color_mode( void * inFunc, GLbitfield inValue );
void glr_debug_print_buffer_depth( void * inFunc, GLbitfield inValue );
void glr_debug_print_buffer_mode( void * inFunc, GLbitfield inValue );

#endif /* __glr_Debug_Struct_H__ */
