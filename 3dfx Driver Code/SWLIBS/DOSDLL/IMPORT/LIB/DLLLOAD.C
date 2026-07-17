/*
	DLLLOAD.C: Initialization and support code for DOS/4GW DLL API.

	Copyright (c) 1996,1997 Dan Teven.
*/

#include <stdlib.h>
#include <string.h>
#include <io.h>
#include "dllload.h"

// Private types
typedef struct
	{
	unsigned short off;
	unsigned short sel;
	} PTR1616;

typedef struct
	{
	PTR1616 apiname;							// expand to ASCIIZ entry point name
	PTR1616 api;								// expand to 16-bit far function taking
	} PackageEntry16;							//   var args and returning ulong

typedef struct
	{
	char *apiname;
	PTR1616 api;
	} PackageEntry;

typedef struct
	{
	PTR1616 next;								// expand to Package __far *
	PTR1616 title;								// expand to ASCIIZ package name
	unsigned short version;
	unsigned short tab1_count;
	PTR1616 table1;							// expand to PackageEntry16 __far *
	unsigned short tab2_count;
	PTR1616 table2;							// expand to PackageEntry16 __far *
	} Package;

// Useful 16-bit functions exported by linear-executable loader
enum
	{
	LoadModule = 0,
	FreeModule = 1,
	GetModuleTable = 2,
	GetModuleName = 3,
	GetModuleHandle = 4,
	GetExportedAddress = 5,
	RelocatePointer = 6,
	UnrelocatePointer = 7,
	NumLoaderExports = 8
	};

PackageEntry __loader_exports[NumLoaderExports] =
	{
		{ "LINEXE_LOADMODULE", },			// load a DLL
		{ "LINEXE_FREEMODULE", },			// unload a DLL
		{ "GETLOADTABLE", },					// get array of loaded module handles
		{ "GETLOADNAME", },					// get module name from handle
		{ "LINEXE_GETMODHANDLE", },		// get module handle from name
		{ "LINEXE_GETPROCADDR", },			// look up an exported name
		{ "REL", },								// relocate a section:offset pointer
		{ "UNREL", }							// unrelocate a selector:offset pointer
	};

unsigned short __data16, __data16_2, __stack16;
static unsigned short loader_ds;
unsigned char __alt_calls;

extern unsigned short _d16infoseg;		// In Watcom RTL
#pragma aux __argv "*";
extern char **__argv;						// "

void breakpoint (void);
#pragma aux breakpoint = "int 3";

unsigned short getcs (void);
#pragma aux getcs = "mov ax, cs";

unsigned short getss (void);
#pragma aux getss = "mov ax, ss";

// Load the limit of a selector into EAX.
unsigned long lsl (unsigned long selector);
#pragma aux lsl parm [eax] modify exact [eax] = "lsl	eax, eax";

// Load the access rights of a selector into the low word of EAX,
// masking out the upper 4 bits of the limit.  The value returned
// in AX is the same format as the value INT 31h/0009h takes in CX.
unsigned long lar (unsigned long selector);
#pragma aux lar parm [eax] modify exact [eax] = \
	"lar	eax, eax" \
	"ror	eax, 8" \
	"and	eax, 0F0FFh";


// Like _fstrcmp, but only the first argument is a far pointer, and the
// sense of the return value is reversed (1 means the strings match).
static int fnstreq (char __far *s1, char *s2)
	{
	while (*s1 == *s2)
		{
		if (*s2 == '\0')
			return (1);
		s1++;
		s2++;
		}
	return (0);
	}

// Locate the entry points to the linear-executable loader, and save
// them in the loader_exports table.  Returns the number of found
// entry points.
static int find_loader_api (void)
	{
	PTR1616 __far *fp;
	Package __far *pkg;
	PackageEntry16 __far *pe;
	char __far *str;
	int i, j;
	int our_package, found = 0;

	if (! _d16infoseg)						// Can't find head of package chain
		return (0);

	fp = MK_FP(_d16infoseg, 0x42);
	fp = MK_FP(fp->sel, fp->off);

	for (pkg = (Package __far *) fp; pkg; pkg = MK_FP(pkg->next.sel, pkg->next.off))
		{
		our_package = 0;
		str = MK_FP(pkg->title.sel, pkg->title.off);
		if (str)
			our_package = fnstreq (str, "LINEXE_LOADER");
		if (our_package)
			{
			loader_ds = pkg->table2.sel;	// save for patching
			pe = MK_FP(pkg->table2.sel, (unsigned long) pkg->table2.off);
			for (i = 0; i < pkg->tab2_count; i++)
				{
				str = MK_FP(pe[i].apiname.sel, pe[i].apiname.off);
				if (! str)
					continue;
//				printf ("Found entry point \"%Fs\"\n", str);	//debug
				for (j = 0; j < NumLoaderExports; j++)
					if (fnstreq (str, __loader_exports[j].apiname))
						{
						__loader_exports[j].api = pe[i].api;
						found++;
						}
				}
			}
		}
	return (found);
	}

// Patch DOS/4GW or DOS/4GW Professional to enable the built-in DLL
// loader.
static int patch_loader (void)
	{
	unsigned char __far *fp;
	unsigned char __near *np;
	unsigned i;
	unsigned limit;
	unsigned patch_point_1 = 0;
	unsigned patch_point_3 = 0;
	unsigned patch_point_4 = 0;
	int patched;
	union REGS r;
	static unsigned char sig0[] =
		"DLL modules not supported";
	static unsigned char sig1[] =
		"DOS/4G";
	static unsigned char sig2[] =
		"\xFF\x76\x06\xFF\x76\x04\x6A\x00\x6A\x00\x8D";
	static unsigned char sig3[] =
		"\x16\x50\x6A";
	static unsigned char sig4[] =
		"dll\\msc\0\0\0\0";
	static unsigned char sig5[] =
		".dll\0.exe\0.com";

	if (! loader_ds)
		return (0);
	limit = lsl (loader_ds);
	if ((limit < 0x100) || (limit > 0xFFFF))
		return (0);								// sanity check

	// Find the first patch point, which is right before the error
	// message "DLL modules not supported" in the loader's data segment.
	// The string "DOS/4GW" -- which is used to test whether DLLs 
	// should be supported -- should be patched to "DOS/4G".  We 
	// assume that even though the loader's data segment might get
	// treated differently in future builds of DOS/4GW, the two 
	// strings will remain close together, and that they won't be
	// found in the first or last 32 bytes of the segment.
	//
	// If the string "DLL modules not supported" isn't in this segment,
	// we can assume the DOS extender is DOS/4G; therefore, the DLL
	// loader is already enabled and we can skip this patch.  Likewise,
	// if the name reads "DOS/4G" instead of "DOS/4GW", we can assume
	// the patch has already been done.
	//
	// Since we are scanning the entire segment anyway, watch for the
	// third and fourth patch points and remember them for later.
	patched = 0;
	limit -= 32;
	for (i = 32, fp = MK_FP(loader_ds, i); i < limit; i++, fp++)
		{
		if ((*fp != sig0[0]) && (*fp != sig4[0]) && (*fp != sig5[0]))
			continue;
		if (! _fmemcmp (fp, sig0, sizeof(sig0) - 1))
			{
			patch_point_1 = i;
			if (patch_point_3 && patch_point_4)	// don't stop loop until
				break;									// all other patches found
			}
		if (! _fmemcmp (fp, sig4, sizeof(sig4) - 1))
			patch_point_3 = i;
		if (! _fmemcmp (fp, sig5, sizeof(sig5)))
			patch_point_4 = i;
		}
	if (patch_point_1 != 0)
		{
		fp = MK_FP(loader_ds, patch_point_1 - 32);
		for (i = 0; i < 32; i++, fp++)
			{
			if (! _fmemcmp (fp, sig1, sizeof(sig1) - 1))
				{
				if (fp[6] == 'W')		// succeed if already patched
					fp[6] = '\0';
				patched = 1;
				}
			}
		if (! patched)
			return (0);
		}

	// Find the second patch point, which is in the loader's code
	// segment.  This patch causes the DLL loader to check the LIBPATH
	// environment variable, and standard places such as the main
	// program's directory, when looking for a DLL whose full path
	// name is not specified.  Since load_module now does its own
	// path expansion, the only time this patch should matter is
	// when loading a second DLL through real import records -- so
	// the patch is now optional.
	fp = MK_FP(__loader_exports[LoadModule].api.sel, 0);
	limit = lsl (__loader_exports[LoadModule].api.sel);
	if ((limit >= 0x100) && (limit <= 0xFFFF))
		{
		limit -= 32;
		for (i = 32, fp = MK_FP(__loader_exports[LoadModule].api.sel, i);
			i < limit; i++, fp++)
			{
			if (*fp != sig2[0])
				continue;
			if (! _fmemcmp (fp, sig2, sizeof(sig2) - 1))
				break;
			}
		if (! _fmemcmp (fp + sizeof(sig2) - 1 + 2, sig3, sizeof(sig3) - 1))
			{
			fp += sizeof(sig2) - 1 + 2;
			if (fp[3] == '\x01')				// succeed if already patched
				{
				// Now we want to set *(fp + 3) to zero, but it's a code segment.
				// We have to create a data alias for it first.

				r.x.eax = 0x0006;				// DPMI Get Segment Base
				r.x.ebx = __loader_exports[LoadModule].api.sel;
				int386 (0x31, &r, &r);

				np = (unsigned char __near *)
					((r.x.ecx << 16) | (r.x.edx & 0xFFFF)) + FP_OFF(fp) + 3;
				*np = '\0';
				}
			}
		// In DOS/4GW 2.01a, the try32 parameter has been deleted, and a new
		// try16 parameter (first in the list) has been added.  To patch the
		// value passed in for the try16 parameter we have to look just before
		// the signature.
		else if (*(fp - 2) == '\x6A')		// DOS/4GW 2.01a
			{
			__alt_calls = 1;					// set flag to account for API changes
			if (*(fp - 1) == '\x00')
				{
				// Now we want to set *(fp - 1) to one, but it's a code segment.
				// We have to create a data alias for it first.

				r.x.eax = 0x0006;				// DPMI Get Segment Base
				r.x.ebx = __loader_exports[LoadModule].api.sel;
				int386 (0x31, &r, &r);

				np = (unsigned char __near *)
					((r.x.ecx << 16) | (r.x.edx & 0xFFFF)) + FP_OFF(fp) - 1;
				*np = '\01';
				}
			}
		}

	// Find the third patch point, which is also optional.  The loader
	// has a bug which prevents it from searching the executable file's
	// directory for DLLs that are loaded through real import records.
	// We can fill the path buffer ourselves if we can find it.
	if (patch_point_3 >= 260)
		{
		fp = MK_FP(loader_ds, patch_point_3 - 256);

		// Path buffer is either 256 or 257 bytes before the
		// signature, depending on the DOS/4GW version.
		if ((*fp != '.') && (*(fp - 1) == '.'))
			--fp;
		if (*fp == '.')
			{
			for (i = 1; i < 256; i++)
				{
				if (fp[i] != '\0')
					goto fourth_patch;		// false alarm
				}
			_fstrcpy (fp, __argv[0]);		// copy in full program path
													// strip file name
			for (i = strlen (__argv[0]); i > 0; )
				{
				--i;
				if ((fp[i] == '\\') || (fp[i] == '/'))
					{
					fp[i] = '\0';
					break;
					}
				}
			}
		}

	// Find the fourth patch point, also optional, to force the DLL
	// extension to ".ovl".
fourth_patch:
	if (patch_point_4 != 0)
		{
		fp = MK_FP(loader_ds, patch_point_4);
		_fmemcpy (fp, ".ovl\0.ovl\0.ovl", sizeof(sig5));
		}
	return (1);
	}

static void cancel_alias16_descriptor (unsigned short desc)
	{
	union REGS r;

	if (desc)
		{
		r.x.eax = 0x0001;						// DPMI Free Descriptor
		r.x.ebx = desc;
		int386 (0x31, &r, &r);
		}
	}

// Allocate scratch descriptors for code or data.  The descriptors
// will be 16-bit and have 64K limits, but their base addresses
// will be left set to zero.
static unsigned short make_alias16_descriptor (int make_code)
	{
	unsigned long desc = 0;
	union REGS r;

	r.x.eax = 0x0000;							// DPMI Allocate Descriptor
	r.x.ecx = 1;
	int386 (0x31, &r, &r);
	if (r.x.cflag)
		return (0);
	desc = r.x.eax;

	r.x.eax = 0x0009;							// DPMI Set Access Rights
	r.x.ebx = desc;
	r.x.ecx = lar(desc) & ~0x4000;		// mask off big bit
	if (make_code)
		r.x.ecx |= 0x0009;					// or in code access bits
	int386 (0x31, &r, &r);
	if (r.x.cflag)
		{
		cancel_alias16_descriptor (desc);
		return (0);
		}
	r.x.eax = 0x0008;							// DPMI Set Segment Limit
	r.x.ebx = desc;
	r.x.ecx = 0x0000;							// limit in CX:DX == 64K
	r.x.edx = 0xFFFF;
	int386 (0x31, &r, &r);
	if (r.x.cflag)
		{
		cancel_alias16_descriptor (desc);
		return (0);
		}
	return ((unsigned short) desc);
	}

static void pass_selectors (void)
	{
	union REGS r;
	struct SREGS sr;

	r.h.ah = 0x30;								// DOS Get Version: force _psp update
	int386 (0x21, &r, &r);					// (if dllloads.obj is linked in)

	r.x.eax = 0x25E9;							// set protected mode interrupt vector
	sr.ds = _psp;								// use an arbitrary unused p.m. vector
	sr.es = 0;									// (the run-time library must cooperate)
	r.x.edx = _d16infoseg;
	int386x (0x21, &r, &r, &sr);
	}

int __cdecl init_dll_loader (void)
	{
	extern void __far __cdecl __flat_cs();
	extern void __far __cdecl __flat_ss();
	unsigned short __far *fp;
	unsigned short __near *np;

	if (__data16 && __data16_2 && __stack16)
		return (1);								// already inited?

	if (! find_loader_api ())
		return (0);

	// If we can't patch the loader, this function should fail.
	if (! patch_loader ())
		return (0);

	// The DLL loader doesn't set up two segment registers that the
	// Watcom run-time library needs, the PSP selector and the
	// kernel data (D16INFO) selector.  Since the DLL will run in
	// the same address space as this program, the easiest way
	// to pass the selector values to the run-time startup code
	// is by grabbing an unused protected-mode interrupt vector.
	// (It's no more a hack than the rest of this mechanism.)
	pass_selectors ();

	// Save the flat-model SS and CS values in the 16-bit code segment.
	fp = (unsigned short __far *) __flat_ss;
	np = (unsigned short __near *) fp;
	*np = getss();
	fp = (unsigned short __far *) __flat_cs;
	np = (unsigned short __near *) fp;
	*np = getcs();

	// Allocate scratch descriptors for code and data.  The descriptors
	// will be 16-bit and have 64K limits, but their base addresses
	// will not left set to zero.
	__data16 = make_alias16_descriptor (0);
	__data16_2 = make_alias16_descriptor (0);
	__stack16 = make_alias16_descriptor (0);
	if (__data16 && __data16_2 && __stack16)
		return (1);

	shutdown_dll_loader ();
	return (0);
	}

void __cdecl shutdown_dll_loader (void)
	{
	cancel_alias16_descriptor (__stack16);
	cancel_alias16_descriptor (__data16_2);
	cancel_alias16_descriptor (__data16);
	}

// Locate the DLL explicitly so that we don't have to depend on the
// DOS/4G loader to do it the way we want to.  The search order here is:
// 1. current directory
// 2. directory containing this client program
// 3. directories on the PATH
// 4. C:\WINDOWS\SYSTEM
// 5. the COMSPEC directory
//
// Note that the DLL name passed in is not expected to contain a path 
// specifier or an extension.  An .OVL extension is added.
char * __cdecl locate_dll (char *module_name)
	{
	char *p;
	char module_with_ext[8+1+3+1];
	static char dll_path[_MAX_PATH];

	// Prevent clobbering return address if bad input
	if (strlen (module_name) > 8)
		return (NULL);

	// Append an .OVL extension to the module name.
	strcpy (module_with_ext, module_name);
	strcat (module_with_ext, ".ovl");

	// Check for the file in the current directory.
	strcpy (dll_path, module_with_ext);
	if (_access (dll_path, 0) == 0)
		return (dll_path);

	// Search the directory containing the client program (this program).
	strcpy (dll_path, __argv[0]);
	p = strrchr (dll_path, '\\');
	if (! p)
		p = strrchr (dll_path, '/');
	if (p)
		{
		strcpy (p + 1, module_with_ext);
		if (_access (dll_path, 0) == 0)
			return (dll_path);
		}

	// Search the PATH directories.
	_searchenv (module_with_ext, "PATH", dll_path);
	if (dll_path[0])
		return (dll_path);

	// Search C:\WINDOWS\SYSTEM.
	strcpy (dll_path, "C:\\WINDOWS\\SYSTEM\\");
	strcat (dll_path, module_with_ext);
	if (_access (dll_path, 0) == 0)
		return (dll_path);

	// Search the COMSPEC directory.
	strcpy (dll_path, getenv ("COMSPEC"));
	p = strrchr (dll_path, '\\');
	if (! p)
		p = strrchr (dll_path, '/');
	if (p)
		{
		strcpy (p + 1, module_with_ext);
		if (_access (dll_path, 0) == 0)
			return (dll_path);
		}
	return (NULL);
	}

void __cdecl fatal_load_error (char *message)
	{
	union REGS r;
	struct SREGS sr;

	r.h.ah = 0x09;								// DOS Display String
	r.x.edx = (unsigned long) message;
	sr.ds = getss();
	sr.es = 0;
	int386x (0x21, &r, &r, &sr);
	exit (1);
	}
