#define JAPANESE_APP


	// Option for using PowerPlant namespace
#define PP_Uses_PowerPlant_Namespace		0	// OFF, don't use PowerPlant namespace

	// Standard Dialogs
#define PP_StdDialogs_Option				PP_StdDialogs_Conditional	// use conditional standard dialog
																		// implementation

	// Force the use of new types
#define PP_Uses_Old_Integer_Types			0	// OFF, use new types

	// Don't allow use of the obsolete AllowTargetSwitch
#define PP_Obsolete_AllowTargetSwitch		0

	// Don't show release notes warnings for old projects
#define PP_Suppress_Notes_20				1


// this is to fix a problem with power.h from Universal Headers 3.3.2
#define __MACERRORS__

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

/*#if DEBUG
#define GLIDE_DEBUG					0
#define GDBG_INFO_ON				0
#define GLIDE_USE_DEBUG_FIFO		0

#define FIFO_ASSERT_FULL			0
#define GLIDE_SANITY_SIZE			0	
#define GLIDE_SANITY_ASSERT			0

#endif *//* !DEBUG */


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


#define ROM_FLASHING_ENABLE_NAPALM	0


/*	International Language		*/
#define	MT_US_VERSION				0
#define MT_US_DEBUG_VERSION			0
#define MT_JAP_VERSION				0
#define MT_FR_VERSION				1