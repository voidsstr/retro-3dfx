//=======================================================================================
// DYNAMIC MODE TABLE
//=======================================================================================

#include <stdlib.h>   

#define CRTC_TABLE_SIZE				24

//
// Parameter Structure for CRTC
//
typedef struct _TIMING_PARAMS
{
    FxU32 width;                // width in pixels
    FxU32 height;               // height in pixels
    FxU32 refresh;              // refresh rate in Hz
    FxU32 HTotal;    			// Make sure to use only 32 bit variables in this or any
    FxU32 HSyncStart;    		// structure else the Win9x Mini VDD may read them incorrectly 
    FxU32 HSyncEnd;    
    FxU32 VTotal;    
    FxU32 VSyncStart;    
    FxU32 VSyncEnd;    
    FxU32 CRTCflags;    
    FxU32 PixelClock;    
    FxU32 CharWidth;    
    FxU32 UseGTF;    
    FxU32 UseAltTiming;    
    FxU32 Checksum; 
    FxU8  AltTiming[CRTC_TABLE_SIZE];   			

} TIMING_PARAMS;

#define HTOTAL_FIELD				1
#define HSYNCSTART_FIELD   			2
#define HSYNCEND_FIELD   			3
#define VTOTAL_FIELD				4
#define VSYNCSTART_FIELD   			5
#define VSYNCEND_FIELD   			6
#define CRTCFLAG_FIELD   			7
#define PIXCLOCK_FIELD   			8
#define REFRESH_FIELD   			9
#define CHARWIDTH_FIELD   			10
#define CHECKSUM_FIELD   			11

#define ALT_CHECKSUM_FIELD   		CRTC_TABLE_SIZE - 2

#define ALT_HTOTAL					0
#define ALT_MISC_OUTPUT				16
#define ALT_SR1						17
#define ALT_PLLCTRL0_LOW			18
#define ALT_PLLCTRL0_HIGH			19
#define ALT_DACMODE					20

#define BPP_8_CAP_FLAG				0x00000002
#define BPP_16_CAP_FLAG				0x00000004
#define BPP_24_CAP_FLAG				0x00000008
#define BPP_32_CAP_FLAG				0x00000010

#define DDRAW_CAPABLE 				0x00000100
#define TVOUT_DESKTOP_CAPABLE 		0x00000200
#define TVOUT_DDRAW_CAPABLE 		0x00000400
#define DFP_DESKTOP_CAPABLE 		0x00000800
#define DFP_DDRAW_CAPABLE 			0x00001000
#define NTSC_CAPABLE 				0x00002000
#define PAL_CAPABLE 				0x00004000

#define DESKTOP_CAPABLE 			0x40000000
#define ALL_MODES_CAPABLE 			0x80000000

#define DESKTOP_CAPABLE_STR 		"DESKTOP"
#define DDRAW_CAPABLE_STR 			"DDRAW"
#define TVOUT_DESKTOP_CAPABLE_STR 	"TVO_DESK"
#define TVOUT_DDRAW_CAPABLE_STR 	"TVO_DD"
#define DFP_DESKTOP_CAPABLE_STR 	"DFP_DESK"
#define DFP_DDRAW_CAPABLE_STR 		"DFP_DD"
#define NTSC_CAPABLE_STR 			"NTSC"
#define PAL_CAPABLE_STR 			"PAL"
#define BPP_CAP_STR 				"BPP"

#define TIMINGS_KEY_STR 			"\\TIMINGS"
#define TIMINGS_VALUE_STR 			""
#define DFP_TIMINGS_VALUE_STR 	   	"AltDFP"
#define SUPPORT_VALUE_STR 			"Supported"
#define HERTZ_KEY_STR				"Hz"
#define USE_GTF_VALUE_STR 			"UseGTF"
#define OPT_REFRESH_LIMIT_STR 		"OptimalRefreshLimit"
#define OPT_NONEDID_LIMIT_STR 		"OptimalNonEDIDLimit"

#if (_WIN32_WINNT >= 0x0500)
#define NT_DEVNODE_PATH_STR			"SYSTEM\\CurrentControlSet\\Services\\3dfxvs\\Device0"
#else
#define NT_DEVNODE_PATH_STR			"SYSTEM\\CurrentControlSet\\Services\\3dfxvsn\\Device0"
#endif // (_WIN32_WINNT >= 0x0500)

#define NT_DEVNODE_SEARCH_STR		"DevNode_Found"
#define NT_DEVNODE_RESET_STR		"Inactive"
#define NT_DEVNODE_SEARCH_KEY_STR	"DevNodeSearch"


#define MAX_EDID_LIMIT				-2
#define MAX_REFRESH_RATE			255
#define MAX_NONEDID_RATE			85
#define MIN_DESKTOP_WIDTH			640
#define MIN_DESKTOP_HEIGHT			480

#define DEFAULT_CHAR_WIDTH			8
// Default CRTC Flags = NON-DOUBLE SCANNED, NON-INTERLACED, H SYNC POLARITY NEGATIVE, V SYNC POSITIVE. 
#define DEFAULT_CRTC_FLAGS			4
		
#define DECIMAL 					10
#define HEX 						16

#define MAX_NUMERIC_STRING_SIZE 	36
#define MAX_TIMING_STRING_SIZE 		132
#define MAX_CAP_STRING_SIZE 		132
#define	MAX_TIMINGS_REG_KEY_LEN 	132 + sizeof(TIMINGS_KEY_STR)

#define CR_SUCCESS					0
#define CR_FAILURE					1

//=======================================================================================
// Taken from modelist.h
//=======================================================================================

#define IS_VALID_MODE    (0x80000000L)        // Mode + Monitor is Valid
#define SCANLINE_DBL     (0x40000000L)        // Modes uses Scan Line Double
#define IS_DDRAW_MODE  	 (0x20000000L)        // Mode is OK for DDRAW.   // DYNAMIC MODE TABLE 
#define TVOUT_DESKTOP  	 (0x10000000L)        // Mode is OK for TV, for DESKTOP 
#define TVOUT_DDRAW    	 (0x08000000L)        // Mode is OK for TV, but for DDRAW only.
#define TVOUT_NTSC     	 (0x04000000L)        // Mode is OK for NTSC 	// DYNAMIC MODE TABLE 
#define TVOUT_PAL      	 (0x02000000L)        // Mode is OK for PAL,	// DYNAMIC MODE TABLE 
#define LCD_DESKTOP    	 (0x01000000L)        // Mode is OK for LCD, for DESKTOP    // DYNAMIC MODE TABLE 
#define LCD_DDRAW      	 (0x00800000L)        // Mode is OK for LCD, for DDRAW only // DYNAMIC MODE TABLE 
#define INTERLACED     	 (0x00400000L)        // Mode is Interlaced,    // DYNAMIC MODE TABLE 
#define IS_LCD_MODE      (0x00200000L)        // Obsolete, kept in to prevent compile error // DYNAMIC MODE TABLE 
#define FAILS_ON_VOODOO3 (0x00100000L)        // Mode does not work on a Voodoo3
#define REND2PIX_PER_CLK (0x00080000L)        // Mode should be used with 2 pixel-per-clock rendering

//=======================================================================================
// END DYNAMIC MODE TABLE
//=======================================================================================

