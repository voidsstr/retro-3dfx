/*
 * gplay.h
 */


#ifndef MDC_GPLAY_H
#define MDC_GPLAY_H

#include <glide.h>


int gplayTrace(char *indir, FxU32 framestart, FxU32 frameend,
	void (*cbfn)(int frame));


#endif
