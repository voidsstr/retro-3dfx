/* SAMPLE OF IMPLICIT DLL LOADING */

#include <stdio.h>

typedef int (*LPFINT)(void);

extern LPFINT __cdecl dllentry0 (void);	// in import library for SAMPLE.DLL

void main (void)
	{
	LPFINT dllfunc;
	unsigned long before, after;
	unsigned long __cdecl mem (unsigned long);

	before = mem(0);

	// The first call to a function in the DLL will cause the DLL to
	// be loaded and its initializer (if any) to run, before the
	// function actually runs.
	dllfunc = dllentry0 ();

	if (dllfunc && (dllfunc() == -123))
		{
		after = mem(0);
		printf ("DLL occupies %luKB of memory.\n", (before - after + 512) / 1024);
		}
	else
		printf ("DLL load failed.\n");
	}
