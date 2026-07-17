#include <stdio.h>
#include <windows.h>
#include "g_imfncs.h"
#include "g_disp.h"
#include "db_trace.h"

/* whether tracing is turned on */
GLboolean __gl_debug_trace = GL_FALSE;

GLint __gl_debug_granularity = 20;

/* points at the real dispatch table */
__GLdispatchState *__gl_real_immed;

/* how much to print in the trace log for each function */
GLint __gl_debug_verbosity = 0;

FILE *__gl_debug_log;

typedef struct {
    int count;
    char *funcName;
} tableEntry;

void printHistogram(char *name, int *count, int *cumCount, int frames) {
    int i, total;
    float avg;

    /* if all slots are zero, don't print the histogram */
    total = 0;
    for (i=0; i < __GL_DEBUG_HISTOGRAM_SIZE; i++) {
	total += count[i];
    }
    if (!total) return;
    fprintf(__gl_debug_log, "%s", name);
    for (i=0; i < __GL_DEBUG_HISTOGRAM_SIZE; i++) {
	avg = count[i] / (float)frames;
	fprintf(__gl_debug_log, "%3.0f ", avg);

	if (cumCount) cumCount[i] += count[i];
	count[i] = 0;
    }
    fprintf(__gl_debug_log, "\n");
}

static int frameCount = 1;

static void
printStats(struct __GLdebugStat *table, struct __GLdebugStat *cumTable, int frames) {
    int i, n;
    tableEntry *ent = (tableEntry *) &table->TotalTri;
    tableEntry *cumEnt = (tableEntry *) &cumTable->TotalTri;
    struct __GLdebugStat *next;

    /* skip over the histograms, and count how many entries are left */
    next = table + 1;
    n =  ((char *)next - (char *)ent) / (sizeof(int) + sizeof(char *));

    for (i=0; i < n; i++) {
	if (ent->count) {
	    float avg = (float) ent->count / frames;

	    fprintf(__gl_debug_log, "%10.3f %s\n", avg, ent->funcName);

	    /* add to cumulative table */
	    if (cumTable) cumEnt->count += ent->count;

	    /* clear for the next time */
	    ent->count = 0;
	}
	ent++;
	cumEnt++;
    }
    fprintf(__gl_debug_log, "\n");
}

static DWORD gtime;

void __gldb_CollectFrameStats(void) {
    struct __GLdebugStat *tab = &__gl_debug_table;
    struct __GLdebugStat *cumTab = &__gl_debug_cum_table;
    int frames = __gl_debug_granularity;
    DWORD now = GetTickCount(), elapsed = 0;
    float spf, tps;

    if (frameCount % __gl_debug_granularity) {
	frameCount++;
	return;
    }
    if (gtime) {
      elapsed = now - gtime;
      spf = elapsed / (float)__gl_debug_granularity;
      tps = tab->TotalTri / (float)elapsed;
      fprintf(__gl_debug_log, "frame %4d sec/frame %8.4f tri/sec %8.4f\n", frameCount, 
	      spf, tps);
    } else {
      fprintf(__gl_debug_log, "frame %4d\n", frameCount);
    }
    gtime = now;
    printHistogram("HistTri:       ", tab->HistTri, cumTab->HistTri, frames);
    printHistogram("HistTriFan:    ", tab->HistTriFan, cumTab->HistTriFan, frames);
    printHistogram("HistTriStrip:  ", tab->HistTriStrip, cumTab->HistTriStrip, frames);
    printHistogram("HistQuad:      ", tab->HistQuad, cumTab->HistQuad, frames);
    printHistogram("HistQuadStrip: ", tab->HistQuadStrip, cumTab->HistQuadStrip, frames);

    printStats(tab, cumTab, __gl_debug_granularity);
    frameCount++;
}

void __gldb_CollectOverallStats(void) {
    struct __GLdebugStat *tab = &__gl_debug_cum_table;
    fprintf(__gl_debug_log, "CUMULATIVE STATS %5d frames\n", frameCount);
    printHistogram("HistTri:       ", tab->HistTri, NULL, frameCount);
    printHistogram("HistTriFan:    ", tab->HistTriFan, NULL, frameCount);
    printHistogram("HistTriStrip:  ", tab->HistTriStrip, NULL, frameCount);
    printHistogram("HistQuad:      ", tab->HistQuad, NULL, frameCount);
    printHistogram("HistQuadStrip: ", tab->HistQuadStrip, NULL, frameCount);

    printStats(tab, NULL, frameCount);
}

static GLenum primMode;
static GLint numVert;

void
__gldb_StatBegin(GLenum mode) {
    primMode = mode;
}

void
__gldb_StatEnd(void) {
    struct __GLdebugStat *ent = &__gl_debug_table;
    int numTri;

    switch(primMode) {
    case GL_POINTS:
	break;
    case GL_LINES:
	break;
    case GL_LINE_LOOP:
	break;
    case GL_LINE_STRIP:
	break;
    case GL_TRIANGLES:
	numTri = numVert / 3;
	if (numTri > __GL_DEBUG_HISTOGRAM_SIZE) numTri = __GL_DEBUG_HISTOGRAM_SIZE;
	ent->HistTri[numTri-1]++;
	ent->TriTri += numTri;
	ent->TotalTri += numTri;
	break;
    case GL_TRIANGLE_STRIP:
	numTri = numVert - 2;
	if (numTri > __GL_DEBUG_HISTOGRAM_SIZE) numTri = __GL_DEBUG_HISTOGRAM_SIZE;
	ent->HistTriStrip[numTri-1]++;
	ent->TriStripTri += numTri;
	ent->TotalTri += numTri;
	break;
    case GL_TRIANGLE_FAN:
	numTri = numVert - 2;
	if (numTri > __GL_DEBUG_HISTOGRAM_SIZE) numTri = __GL_DEBUG_HISTOGRAM_SIZE;
	ent->HistTriFan[numTri-1]++;
	ent->TriFanTri += numTri;
	ent->TotalTri += numTri;
	break;
    case GL_QUADS:
	numTri = numVert - 2;
	if (numTri > __GL_DEBUG_HISTOGRAM_SIZE) numTri = __GL_DEBUG_HISTOGRAM_SIZE;
	ent->HistQuad[numTri-1]++;
	ent->QuadTri += numTri;
	ent->TotalTri += numTri;
	break;
    case GL_QUAD_STRIP:
	numTri = numVert / 2;
	if (numTri > __GL_DEBUG_HISTOGRAM_SIZE) numTri = __GL_DEBUG_HISTOGRAM_SIZE;
	ent->HistQuadStrip[numTri-1]++;
	ent->QuadStripTri += numTri;
	ent->TotalTri += numTri;
	break;
    case GL_POLYGON:
	break;
    }
    numVert = 0;
}

void
__gldb_StatVertex(void) {
    numVert++;
}

