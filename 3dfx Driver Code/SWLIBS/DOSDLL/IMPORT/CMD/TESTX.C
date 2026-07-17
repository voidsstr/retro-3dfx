/* SAMPLE OF EXPLICIT DLL LOADING */

#include <stdio.h>
#include "dllload.h"

typedef int (__cdecl *LPFINT)(void);
typedef LPFINT (*LPFLPFINT)(void);

void main (void)
	{
	unsigned short hmodule;
	LPFLPFINT dllentry0;
	LPFINT dllfunc;
	unsigned long before, after;
	unsigned long __cdecl mem (unsigned long);

	before = mem(0);

	// When we explicitly load the DLL, the system is initialized.
	// If we can find the DLL, and the address of the function in
	// the DLL, we can call that address through a function pointer.
	// The rest of the sample is essentially the same as TESTI.C.
	dllfunc = (LPFINT) NULL;
	hmodule = load_module ("sample.ovl");
	if (hmodule != NullModule)
		{
		dllentry0 = (LPFLPFINT) NULL;
		get_exported_address (hmodule, "_dllentry0", (void __far **) &dllentry0);

		if (dllentry0 != (LPFLPFINT) NULL)
			dllfunc = dllentry0 ();
		}

	if (dllfunc && (dllfunc() == -123))
		{
		after = mem(0);
		printf ("DLL occupies %luKB of memory.\n", (before - after + 512) / 1024);
		}
	else
		printf ("DLL load failed.\n");
	}
