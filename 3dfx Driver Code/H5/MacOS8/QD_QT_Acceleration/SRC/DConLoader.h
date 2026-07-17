// This file is used to set DCON access and load the DCON header.
// To load DCon.h into a file 

#ifndef DCON
	Danger!  DCON should always be define because will automatically be set to 1 by DCON.h
#endif

#ifdef DEBUG_MODE
	#ifndef ALLOW_DCON
		#define ALLOW_DCON 0
	#endif

	#ifndef WANT_DCON
		#define WANT_DCON 0
	#endif
#endif

#if DEBUG_MODE
	#if ALLOW_DCON
		#if WANT_DCON
			#undef	DCON
			#define DCON 	1
		#else
			#undef	DCON
			#define DCON 	0
		#endif
	#else
		#undef	DCON
		#define DCON 	0
	#endif
#else
	#undef	DCON
	#define DCON 	0
#endif

#include <DCON.h>
