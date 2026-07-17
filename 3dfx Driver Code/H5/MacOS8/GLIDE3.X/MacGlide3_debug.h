#ifndef _MAC_GLIDE_H_
#define _MAC_GLIDE_H_

/* Hack */
#define __stdcall

/* Glide Global options */
//#define CVG							1
#define GLIDE_LIB						1
#define GLIDE_PLUG						1
#define GLIDE3							1
#define GLIDE3_ALPHA					1
#define GLIDE3_VERTEX_LAYOUT			1
#define HWC_GDX_INIT					1
#define HWC_GDX_INIT_HRM                1
#define HWC_ACCESS_GDX					1
#define PCI_BUMP_N_GRIND				0
#define FIFO_WRITE_64					0

//#define GLIDE_HW_H3					1
//#define HWC_EXT_INIT					1
#define H3								1
#define H4								1
#define H5                              1
#define FX_GLIDE_NAPALM                 1

/* Glide Platform Options */
#define SET_BSWAP						1
#define SET_SWIZZLEHACK					0
#define HAL_HW							1 
#define INIT_DOS						1
#define HAS_CONSOLE_IO					1
/* Glide Debugging options */
#define DEBUG							1

#if DEBUG
#define GLIDE_DEBUG						1
#define GDBG_INFO_ON					1
#define GLIDE_USE_DEBUG_FIFO			0

//#define FIFO_ASSERT_FULL				0
#define GLIDE_SANITY_SIZE				1	
#define GLIDE_SANITY_ASSERT				1
#ifdef __MWERKS__
#pragma global_optimizer				off
#pragma scheduling						off
#pragma traceback						on
#endif
#else /* !DEBUG */
#ifdef __MWERKS__
#pragma global_optimizer				on
#pragma optimization_level				4
#pragma peephole						on
#pragma scheduling						750
#pragma traceback						off
#pragma side_effects					off
#pragma optimize_for_size				off
#endif
#endif /* !DEBUG */

#define GLIDE_USE_C_TRISETUP			1

#define GLIDE_FP_CLAMP					0
#define GLIDE_FP_CLAMP_TEX				0

/* Glide HW Options */
#define GLIDE_CHIP_BROADCAST			1
#define GLIDE_HW_TRI_SETUP				1
#define GLIDE_PACKET3_TRI_SETUP			1
#define GLIDE_TRI_CULLING				1
#define GLIDE_PACKED_RGB				1
#define GLIDE_DISPATCH_SETUP			0
#define GLIDE_BLIT_CLEAR				1
#define USE_PACKET_FIFO					1
#define GLIDE_INIT_HWC					1

#define GLIDE_DEFAULT_GAMMA				1.3f

#ifndef __VERSION_REZ__
#include <MacHeaders.h>
#endif

#endif /* _MAC_GLIDE_H_ */
