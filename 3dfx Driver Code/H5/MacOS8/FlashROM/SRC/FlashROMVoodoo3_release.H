#ifndef _MAC_GLIDE_H_
#define _MAC_GLIDE_H_

/* Hack */
#define __stdcall
#define FX_CSTYLE

/* Glide Global options */
//#define CVG						1
#define MACOS_GDX					1
#define HARDWARE_CURSOR				1
#define GDX_SWIZZLE_HACK			1
#define RESOLUTION_HACK				0
#define GLIDE_LIB					0
#define GLIDE_PLUG					0
#define PCI_BUMP_N_GRIND			0
#define PCI_COPYBACK				0

//#define HWC_EXT_INIT				1//
//#define H3							1
#define H4							1

#define COMPACT_MODE_TABLE 			0

/* Glide Platform Options */
#define SET_BSWAP					1
#define HAL_HW						1 
#define INIT_DOS					1
#define HAS_CONSOLE_IO				1
/* Glide Debugging options */
//#define DEBUG						0	// don't set it here! set it in gdx_prefix_debug.h instead

#if DEBUG
#define GLIDE_DEBUG					0
#define GDBG_INFO_ON				0
#define GLIDE_USE_DEBUG_FIFO		0

#define FIFO_ASSERT_FULL			0
#define GLIDE_SANITY_SIZE			0	
#define GLIDE_SANITY_ASSERT			0
#ifdef __MWERKS__
#pragma global_optimizer		    off
#pragma scheduling					off
#pragma traceback					on
#endif
#else /* !DEBUG */
#ifdef __MWERKS__
#if 1
#pragma global_optimizer		    on
#pragma optimization_level			4
#pragma peephole					on
#pragma scheduling					750
#pragma traceback					off
#pragma side_effects				off
#pragma optimize_for_size			on
#pragma auto_inline					off
#else
#pragma global_optimizer		    off
#pragma peephole					off
#pragma traceback					off
#pragma side_effects				off
#pragma optimize_for_size			off
#pragma auto_inline					off
#endif
#endif
#endif /* !DEBUG */


#define GLIDE_USE_C_TRISETUP		1

#define GLIDE_FP_CLAMP			    0
#define GLIDE_FP_CLAMP_TEX			0

/* Glide HW Options */
#define GLIDE_CHIP_BROADCAST		1
#define GLIDE_HW_TRI_SETUP			1
#define GLIDE_PACKET3_TRI_SETUP		1
#define GLIDE_TRI_CULLING			1
#define GLIDE_PACKED_RGB			1
#define GLIDE_DISPATCH_SETUP		0
#define GLIDE_BLIT_CLEAR			1
#define USE_PACKET_FIFO				1
#define GLIDE_INIT_HWC				1

#define GLIDE_DEFAULT_GAMMA			1.3f

#ifndef __VERSION_REZ__
#include <MacHeaders.h>
#endif

#endif /* _MAC_GLIDE_H_ */
