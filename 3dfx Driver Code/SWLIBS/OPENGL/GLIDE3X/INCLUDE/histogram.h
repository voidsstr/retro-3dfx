/*
** Copyright 1996,1997 Silicon Graphics, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of Silicon Graphics, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of Silicon Graphics, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
**
** $Revision: 2$
** $Date: 10/11/00 7:57:22 PM$
*/

/*
** HISTOGRAM.H
**
** Cycle count histogram management routines.  These routines were written so
** that an application can count cycles and manage a histogram of these cycle
** counts.  To use this an application should first create a histogram.  The
** max_ticks parameter informs the histogram how many counter buckets need to
** be coputed.  The overhead parameter tells the histogram routines how much
** overhead is introduced when doing cycle counts so that this can be unbiased
** from timings stored in the histogram later.
**
** When an application has a histogram allocated it can then proceed to store
** timings using __glHistogramStoreTiming.  The value parameter tells the
** histogram routine which bucket to increment.
**
** When an application is done collecting data it calls __glHistogramSummarize,
** which will compute the various statistics.  After this an application can
** retrieve the data directly from the __glHistogram structure or dump it to
** a FILE by calling __glHistogramPrint.
**
** An application should always call __glHistogramDestroy when done using a
** histogram to avoid memory leaks.
**
** On the PC I would recommend that you use the macro RDTSC to read the Pentium
** timestamp counter.  This has more consistent latency than the Win32 API
** function QueryPerformanceCounter.
**
*/
#ifndef __HISTOGRAM_H__
#define __HISTOGRAM_H__

#ifdef WIN32
/*
** included for FILE
*/
#include <stdio.h>

/*
** RDTSC macro
**
** We zero out EDX and EAX first because this is the only way we can tell the
** compiler that those registers are going to be trashed.  The RDTSC opcode
** is not supported by MSVC's inline assembler, so we have to emit the opcode
** directly.  RDTSC returns the value of the timestamp counter as a long in 
** EDX:EAX.  We only care about the EAX portion because the odds of a roll over
** are VERY unlikely, and even if it does it'll wrap to a big number that will
** be rejected by the histogram routines.
*/
#include "context.h"

#define RDTSC( a ) \
    __asm xor edx, edx \
    __asm xor eax, eax \
    __asm __emit 0fh   \
    __asm __emit 31h   \
    __asm mov a, eax

typedef struct
{
    unsigned long  *data;
    unsigned        max_ticks, overhead;

    float           mean;

    unsigned int    mode;
    unsigned        total_samples, accepted_samples;

    int             lowest, highest;
} __GLperfHistogram;

__GLperfHistogram *__glHistogramCreate( __GLcontext *gc, int max_ticks, unsigned overhead );
void               __glHistogramDestroy( __GLcontext *gc, __GLperfHistogram *h );
void               __glHistogramSummarize( __GLperfHistogram *h );
void               __glHistogramStoreTiming( __GLperfHistogram *h, unsigned value );
void               __glHistogramPrint( __GLperfHistogram *h, FILE *fp, float threshold );

#endif /* WIN32 */
#endif /* __HISOGRAM_H__ */
