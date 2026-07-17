/***************************************************************************\
* Module Name: debug.h
*
* Commonly used debugging macros.
*
* Copyright (c) 1992-1996 Microsoft Corporation
\***************************************************************************/

extern
LONG  UseCSIM;

extern
VOID
DebugPrint(
    LONG DebugPrintLevel,
    PCHAR DebugMessage,
    ...
    );

/* ------------------------------------------------------------------ *
 * V5DLog - UNCONDITIONAL diagnostic logger for the Voodoo5/XP         *
 * "stuck 640x480 / garbled desktop after Quake III exits" teardown    *
 * investigation.  Unlike DISPDBG/DebugPrint (compiled out in the free *
 * build) this is always compiled in and always emits, so the mode-    *
 * transition trace is visible on a retail 3dfxv5d.dll via a kernel    *
 * debugger / DebugView capture.  Every line is prefixed "3DFXV5D: ".  *
 * Keep it LOW VOLUME - mode transitions/escapes only, never per-blt.  *
 * ------------------------------------------------------------------ */
extern
VOID
V5DLog(
    PCHAR DebugMessage,
    ...
    );

#if DBG

VOID DebugLog(LONG, CHAR*, ...);

#define DISPDBG(arg) DebugPrint arg
#define STATEDBG(level) DebugState(level)
#define RIP(x) { DebugPrint(0, x); EngDebugBreak();}
#define ASSERTDD(x, y) if (!(x)) RIP (y)

extern void DumpH3Regs(PDEV *, ULONG);

#define DUMP_H3_REGS(ppdev, debuglevel)   DumpH3Regs((ppdev),(debuglevel))

// If we are not in a debug environment, we want all of the debug
// information to be stripped out.

#else

#define DISPDBG(arg)
#define STATEDBG(level)
#define LOGDBG(arg)
#define RIP(x)
#define ASSERTDD(x, y)

#define DUMP_H3_REGS(ppdev, debuglevel)

#endif
