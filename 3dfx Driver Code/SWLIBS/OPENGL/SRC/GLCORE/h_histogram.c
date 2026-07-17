/*
** Copyright 1991-1997, Silicon Graphics, Inc.
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
** $Date: 10/11/00 8:03:32 PM$
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "histogram.h"

/*
** __glHistogramCreate
**
** Creates a histogram after verifying that the performance counter
** in fact exists.
*/
__GLperfHistogram *__glHistogramCreate( struct __GLcontextRec *gc, int max_ticks, unsigned overhead )
{
    __GLperfHistogram *h = 0;

    if ( ( h = ( __GLperfHistogram * ) gc->imports.calloc( gc, 1, sizeof( __GLperfHistogram ) ) ) != 0 )
    {
        memset( h, 0, sizeof( h ) );
        h->overhead  = overhead;
        h->max_ticks = max_ticks;
        if ( ( h->data = ( unsigned long * ) gc->imports.calloc( gc, 1, sizeof( unsigned long ) * max_ticks ) ) != 0 )
        {
            memset( h->data, 0, sizeof( unsigned long ) * max_ticks );
        }
        else
        {
            gc->imports.free( gc, h );
            h = 0;
        }
    }
    return h;
}

/*
** __glHistogramDestroy
*/
void __glHistogramDestroy( __GLcontext *gc, __GLperfHistogram *h )
{
    if ( h )
    {
        if ( h->data )
        {
            gc->imports.free( gc, h->data );
        }
        gc->imports.free( gc, h );
    }
}

/*
** __glHistogramSummarize
*/
void __glHistogramSummarize( __GLperfHistogram *h )
{
    unsigned greatest_so_far = 0, heaviest_bucket = -1;
    float    total = 0.0F;

    if ( h )
    {
        unsigned i;

        if ( h->accepted_samples == 0 )
            return;

        h->lowest = -1;
        for ( i = 0; i < h->max_ticks; i++ )
        {
            if ( h->data[i] )
            {
                if ( h->lowest == -1 )
                    h->lowest = i;
                h->highest = i;
		if ( h->data[i] > greatest_so_far )
		{
		    greatest_so_far = h->data[i];
		    heaviest_bucket = i;
		}
		total += ( float ) ( h->data[i] * i );
            }
        }

        h->mean = total / ( float ) h->accepted_samples;
        h->mode = heaviest_bucket;
    }
}

/*
** __glHistogramStoreTiming
*/
void __glHistogramStoreTiming( __GLperfHistogram *h, unsigned value )
{
    if ( h )
    {
        value -= h->overhead;

        h->total_samples++;

        if ( value < h->max_ticks )
        {
            h->data[value]++;
            h->accepted_samples++;
        }
    }
}

/*
** __glHistogramPrint
*/
void __glHistogramPrint( __GLperfHistogram *h, FILE *fp, float threshold )
{
    if ( h )
    {
        unsigned i;
        float _ts = ( float ) h->total_samples;

        if ( h->total_samples == 0 )
            return;

        fprintf( fp, "latency:          %d\n",   ( unsigned ) h->overhead );
        fprintf( fp, "total samples:    %d\n",   ( unsigned ) h->total_samples );
        fprintf( fp, "accepted samples: %d\n",   ( unsigned ) h->accepted_samples );
        fprintf( fp, "min:              %d\n",   ( unsigned ) h->lowest );
        fprintf( fp, "max:              %d\n",   ( unsigned ) h->highest );
        fprintf( fp, "max allowable:    %d\n",   ( unsigned ) h->max_ticks );
        fprintf( fp, "median:           %d\n",   ( unsigned ) ( h->lowest + ( h->highest - h->lowest ) / 2 ) );
        fprintf( fp, "mode:             %d\n",   ( unsigned ) h->mode );
        fprintf( fp, "mean:             %.2f\n", h->mean );

        fprintf( fp, "\nclocks            %%\n" );

        for ( i = h->lowest; i < (unsigned)h->highest; i++ )
        {
            float p = ( ( float ) h->data[i] ) / _ts;

            if ( p > threshold )
            {
                fprintf( fp, "%-4d              %.2f\n", i, p * 100.0F );
            }
        }
    }
    else
    {
        fprintf( fp, "__glHistogramPrint() -- h == NULL\n" );
    }
}

/*
** __RDTSC
**
** Reads time stamp counter.  Note that we don't need to save
** and restore EDX and EAX since the compiler assumes that these
** registers are going to be trashed anyway.
*/
#pragma warning ( disable : 4035 )
__declspec( naked ) unsigned long __RDTSC( void )
{
    __asm __emit 0fh
    __asm __emit 31h
    __asm ret
}
#pragma warning ( default : 4035 )
