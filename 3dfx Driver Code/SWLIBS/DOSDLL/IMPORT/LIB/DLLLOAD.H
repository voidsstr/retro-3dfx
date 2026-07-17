/*
	DLLLOAD.H: Prototypes and definitions for DOS/4GW DLL API.

	Copyright (c) 1996 Dan Teven.
*/

#if !defined(__DLLLOAD_H__)
#define	__DLLLOAD_H__

#include <dos.h>

#define	NullModule	0xFFFF

int __cdecl init_dll_loader (void);
void __cdecl shutdown_dll_loader (void);
void __cdecl fatal_load_error (char *message);

int __cdecl load_module (char *module_name);
int __cdecl free_module (unsigned short handle);
int __cdecl get_module_table (unsigned short **address);
int __cdecl get_module_name (unsigned short handle, char *buffer, unsigned short buf_len);
int __cdecl get_module_handle (char *module_name, unsigned short *handle);
int __cdecl get_exported_address (unsigned short handle, char *name, void __far **address);
int __cdecl relocate_pointer (void __far **address, unsigned short handle);
int __cdecl unrelocate_pointer (void __far **address, unsigned short handle);

#endif
