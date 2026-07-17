#ifdef __powerc
	#include	"MacHeadersPPC"
#else
	#include	"MacHeaders68K"
#endif

#define SET_BSWAP				1

#define PCI_BUMP_N_GRIND		1

#define VOODOO4					1
#define INSTALL_ADVANCED_2D_ACCEL 1
#define INSTALL_QUICKTIME		1




// DEFINES FOR DEBUGGING

// You can check this mode to see if we are in debugging.
// This project that uses this header as its preifx file
// should also have the optimizations, inlining and whatnot
// off.


// USING DCON:  DCON must be defined.  If it isn't the DCON
// header will define it as 1 and turn on DCON logging,
// which is undesirable.  So we set DCON globally to off.
// To turn on and off DCON on a file by file basis,
// set WANT_DCON to true and include DCON_LOADER.
// This should prevent people from accidentally leaving
// DCON on or accidentally have it on by neglect.

#define DCON			 		0		// !!! DON'T EVER CHANGE THIS.

#define DEBUG_MODE				1
#if DEBUG_MODE
	#define ALLOW_DCON			1

	// These are all different logging types for Punt logging.
	#define PUNT_LOGGING			0
		#define DCON_LOGGING		0
		#define APP_LOGGING			0
		#define MB_LOGGING			0
		#define NQD_VARS_DUMP		0
#else
	// These should never be encountered when debugging
	#define ALLOW_DCON			ALLOW_DCON should not be check outside of debugging mode.

	#define PUNT_LOGGING		PUNT LOGGING should not be checked outside debugging mode.
		#define DCON_LOGGING		DCON LOGGING should not be checked outside debugging mode.
		#define APP_LOGGING			APP LOGGING should not be checked outside debugging mode.
		#define MB_LOGGING			MB LOGGING should not be checked outside debugging mode.
		#define NQD_VARS_DUMP		NQD VARS DUMP should not be checked outside debugging mode.
#endif

// NOTES on DCON.  Do not iclude the DCON file youself.
// Always include the "DConLoader.h" if you need DCON.