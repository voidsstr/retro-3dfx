/* SAMPLE.DLL

	This DLL exports two functions.  The second one is just an empty
	function so there are two things to list in the import library.
	The first one returns the address of an internal function in
	the DLL; the internal function, if called, returns -123.  (It
	uses atoi, so you can see that "safe" C run-time library functions
	can be used within a DLL.)
*/

#include <stdio.h>
#include <stdlib.h>

#define FULL_RTL

#if defined(FULL_RTL)
	#include <malloc.h>
	#include <errno.h>
	#include <float.h>
	#include <math.h>
	#include <signal.h>
	#include <time.h>
	#include <string.h>
	#include <process.h>
	#include <dos.h>
	#include <conio.h>
#else
	/*	The __acrtused definition allows this DLL to link with the
		C run-time library, without dragging in the startup code and
		requiring a main() definition.  (The presence of a main function
		will confuse the DLL loader.)  If you want to use floating
		point, you should import the run-time library.
	*/
	int __acrtused;
#endif

typedef int (*LPFINT)(void);

void breakpoint (void);
#pragma aux breakpoint = "int 3";

int multiplier = -1;

#include "math.h"

int dllinternal (void)
	{
	return (multiplier * (int) (3.0 * atoi ("41")));
	}

#if defined(FULL_RTL)
int __far critical_error_handler (unsigned deverror, unsigned errcode, unsigned __far *devhdr)
	{
	cprintf ("Critical error!\r\n");
	return (_HARDERR_FAIL);
	}

void signal_handler (int sig_type)
	{
	switch (sig_type)
		{
		case SIGINT:
			cprintf ("Ctrl-C!\r\n");
			break;
		case SIGFPE:
			cprintf ("Floating point exception!\r\n");
			break;
		default:
			cprintf ("Unknown signal!\r\n");
			break;
		}
	signal (sig_type, signal_handler);
	}
#endif


LPFINT __export __cdecl dllentry0 (void)
	{
#if defined(FULL_RTL)
	unsigned long __cdecl mem (unsigned long);
	double a, b, c;
	FILE *fp;

	signal (SIGINT, signal_handler);
	signal (SIGFPE, signal_handler);

	printf ("--------------------\n");
	printf ("Testing environment: COMSPEC=%s\n", getenv("COMSPEC"));
	printf ("Testing spawning (\"VER\"): ");
	fflush (stdout);
	system ("ver");

	printf ("Testing math functions: ");
	fflush (stdout);
	a = sqrt(2.0);								// sqrt 2
	b = tan(atan(0.75));						// 3/4
	c = a / b;									// ((sqrt 2) * 4) / 3
	a *= c;										// 8/3
	b = 3 * a;									// 8
	c = pow(2.0, 3.0);						// 8
	a = 8 * ((1 / a) + 0.625) + 0.0001;	// 8
	b -= 0.0001;
	c += 0.0001;
	if ((floor(a) == 8) && (ceil(b) == 8) && ((int) c == 8))
		printf ("passed.\n");
	else
		printf ("failed.\n");
	printf ("Testing floating-point error handling: ");
	fflush (stdout);
	a = 0;
	floor(b / a);

	printf ("Testing stream I/O, first line of config.sys: \n");
	fp = fopen("c:\\config.sys", "r");
	if (fp)
		{
		puts (fgets (NULL, 128, fp));
		fclose (fp);
		}
	else
		{
		fprintf (stderr, "Failure opening file - ");
		perror("error");
		fprintf (stderr, "\n");
		}

	printf ("Testing DLL memory allocation: %luKB\n", mem(0) / 1024);

	printf ("Testing critical error handling: ");
	fflush (stdout);
	_harderr (critical_error_handler);
	if (fopen ("a:sdfsdkf", "r") != NULL)
		printf ("failed.\n");

	printf ("Testing console input -- press any key ");
	fflush (stdout);
	getch ();
	printf ("\n");

	printf ("\nGrabbing 1MB of memory to simulate a larger DLL.\n");
	mem (1024 * 1024);
	printf ("--------------------\n");

	signal (SIGINT, SIG_DFL);
	signal (SIGFPE, SIG_DFL);
	flushall ();
#endif
	return (dllinternal);
	}

void __export __cdecl dllentry1 (void)
	{
	}

/*	This is the DLL initializer.  It will be called only if the program
	is built with the modified C run-time library.

	The library calls __dll_initialize after it completes its own
	initialization, so it's safe to call library functions here.
	An empty __dll_initialize function is provided by the library itself
	if the DLL code doesn't supply one.

	The DLL initializer runs on a temporary stack.  It appears that
	the stack is 8K but (due to a bug in DOS/4GW) only 3K or so are
	safe to use.  When __dll_initialize is entered, a small amount of
	stack will already have been used, so be careful about further
	stack use!

	In theory, a return value of 1 indicates successful initialization
	and a return value of 0 indicates an error, but the DOS/4G DLL loader
	ignores the return value.

	Note that the DLL loader does not support any sort of notification
	when a DLL is about to be unloaded from memory.
*/
int __dll_initialize (void)
	{
	printf ("DLL initializer called.\n");
	return (1);
	}
