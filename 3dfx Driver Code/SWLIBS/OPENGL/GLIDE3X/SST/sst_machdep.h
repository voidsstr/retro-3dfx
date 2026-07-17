/*
** Machine dependent implementation limits.
*/

#define __GL_SST_MAX_WINDOW_WIDTH	640
#define __GL_SST_MAX_WINDOW_HEIGHT	480

#define __GL_SST_COORD_SUBPIXEL_BITS	  4
#define __GL_SST_MAX_TEXTURE_SIZE	256

#define __GL_SST_POINT_SIZE_MINIMUM	1.0
#define __GL_SST_POINT_SIZE_MAXIMUM	10.0
#define __GL_SST_POINT_SIZE_GRANULARITY	0.0625 /* based on 4 bit subpixel */
#define __GL_SST_LINE_WIDTH_MINIMUM	0.5
#define __GL_SST_LINE_WIDTH_MAXIMUM	10.0
#define __GL_SST_LINE_WIDTH_GRANULARITY	0.0625 /* based on 4 bit subpixel */

#define __GL_SST_MAX_LOD 9
