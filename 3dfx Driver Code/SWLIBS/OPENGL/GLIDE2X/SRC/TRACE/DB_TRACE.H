#ifndef __db_trace_h_
#define __db_trace_h_

#if !__GL_BUILD_TRACE

/* This is the code that non-trace builds should ever see. */
#define GL_TRACE(name, str)

#else

#define GL_TRACE(name, str) \
    __gl_debug_table.##name##++;

#define __GL_DEBUG_HISTOGRAM_SIZE 10

#include "db_stat.h"
#include <stdio.h>

/* whether tracing is turned on */
extern GLboolean __gl_debug_trace;

/* file where all output goes */
extern FILE *__gl_debug_log;

/* points to the real dispatch table */
extern __GLdispatchState *__gl_real_immed;

/* dispatch table pointing at debugging stubs */
extern __GLdispatchState __glDebugState;

/* how many frames in each interval */
extern GLint __gl_debug_granularity;

/* level of printing in the trace log for each function */
extern GLint __gl_debug_verbosity;

extern void __gldb_CollectFrameStats(void);
extern void __gldb_CollectOverallStats(void);

extern void __gldb_StatBegin(GLenum mode);
extern void __gldb_StatEnd(void);
extern void __gldb_StatVertex(void);

/*********************************/

#endif /* __GL_BUILD_TRACE */
#endif /* __db_trace_h_ */
