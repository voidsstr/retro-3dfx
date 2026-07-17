#ifndef __DBG_H__
#define __DBG_H__

#include <stdarg.h>


#ifdef __DEBUG_PRINT
extern __declspec(dllimport) void __stdcall OutputDebugStringA(const char *);

static
void ErrorF(char *fmt, ...)

{  char dbuf[256];
   va_list args;

   va_start(args, fmt);
   vsprintf(dbuf, fmt, args);
   va_end(args);

   OutputDebugStringA(dbuf);
}

#else

static
void ErrorF(char *fmt, ...)

{
}

#endif




#endif
