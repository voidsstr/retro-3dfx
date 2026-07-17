/*________________________________________________________________________________________
** 
** Copyright (c) 1999, 3Dfx Interactive, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of 3Dfx Interactive, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
**
**________________________________________________________________________________________
**
**  Description: Debugging code
**
** 
**
*/

#include "glr.h"
#include "glr_debug.h"
#include <string.h>
#include <DCon.h>

#include <Events.h>

#define __use_glm__ 1

typedef struct TFuncDbg TFuncDbg;
struct TFuncDbg {
	char			name[64];
	const void *	addr;
	long			traceEntry;
	TFuncDbg *		next;
	TFuncDbg *		previous;
};


static int gOutputEnable = 1;

static char gLibName[2048];
static char gLogFileName[2048];
static TFuncDbg * gFirstFunc;


void		glr_debug_print_header( void );
	
void		glr_debug_vprintf(
					const char *			inFormat,
					va_list					inArgs);

void		glr_default_env_setting();

TFuncDbg *	glr_add_func(
					const char *			inName,
					const void *            inAddr,
					long					inTraceEntry);

TFuncDbg *	glr_find_by_name(
					const char *			inName);

TFuncDbg *	glr_find_by_addr(
					const void *			inAddr);


TFuncDbg	kFuncDbg = { "glr_debug", 0, true, 0, 0 };

/*
________________________________________________________________________________________

      glr_debug_init
________________________________________________________________________________________

*/

void
glr_debug_init(
	const unsigned char	*inLibName)
{
	static int			done=0;			/* only execute once */
	long				i;

	if (done) return;

	/* convert the name of the library from a p to c string. */
	{
		char *			theChar = gLibName;
		i = *inLibName++;
		while(i--) *theChar++ = *inLibName++;
		*theChar = 0;
	}
	
	strcpy( gLogFileName, gLibName );
	strcpy( &gLogFileName[ strlen( gLogFileName ) ], ".log" );	
	dopen( gLogFileName );

	done = 1;

	gFirstFunc = 0;	
	glr_default_env_setting();

	glr_debug_print_header();
}


void glr_debug_enable(int enabled)
{
	gOutputEnable = enabled;
}

/*
________________________________________________________________________________________

      glr_debug_print_header
________________________________________________________________________________________

*/

void
glr_debug_print_header()
{
	glr_debug_printf( "________________________________________________________________________________________\n");
	glr_debug_printf( "glr_debug / version 0.1\n\n" );
	
}


/*
________________________________________________________________________________________

      glr_debug_printf
________________________________________________________________________________________

*/

void
glr_debug_printf(
	const char *			inFormat,
							...)
{
    va_list					theArgs;

    va_start( theArgs, inFormat );
    glr_debug_vprintf( inFormat, theArgs );
    va_end( theArgs );
}


/*
________________________________________________________________________________________

      glr_debug_vprintf
________________________________________________________________________________________

*/

void
glr_debug_vprintf(
	const char *			inFormat,
	va_list					inArgs)
{
	if(gOutputEnable)
	    vdprintf( inFormat, inArgs );
}


/*
________________________________________________________________________________________

      glr_debug_entry
________________________________________________________________________________________

*/

void
glr_debug_entry(
	const void *			inAddr,
	const char *			inName)
{
	TFuncDbg *				theFunc;
	
	theFunc = glr_find_by_addr( inAddr );
	
	if ( theFunc == 0 )
	{
	
		theFunc = glr_find_by_name( inName );
		
		if ( theFunc == 0 ) {
			theFunc = glr_add_func( inName, inAddr, true );
		}
		
		if ( theFunc->addr == 0 ) {
		  theFunc->addr = inAddr;
		}
		
	}
	
	if ( theFunc != 0 )
	{
		if ( theFunc->traceEntry ) {
			glr_debug_printf( "%s() : ...\n", theFunc->name );
		}
	}
}


/*
________________________________________________________________________________________

      glr_debug_verbose
________________________________________________________________________________________

*/

void
glr_debug_verbose(
	const void *			inAddr,
	const char *			inFormat,
							...)
{
	TFuncDbg *				theFunc;
    va_list					theArgs;
	
	theFunc = glr_find_by_addr( inAddr );
	
	if ( theFunc != 0 ) {
		if ( theFunc->traceEntry ) {
			glr_debug_printf( "%s() : ", theFunc->name );

			va_start( theArgs, inFormat );
			glr_debug_vprintf( inFormat, theArgs );
			va_end( theArgs );
		}
	} else {
		glr_debug_printf( "ERROR : missing function declaration | %s", inFormat);
	}
}


/*
________________________________________________________________________________________

      glr_debug_error
________________________________________________________________________________________

*/

void
glr_debug_error(
	const void *			inAddr,
	const char *			inFormat,
							...)
{
	TFuncDbg *				theFunc;
    va_list					theArgs;
	
	va_start( theArgs, inFormat );

	if ( inAddr != 0 ) {

		theFunc = glr_find_by_addr( inAddr );
		
		if ( theFunc != 0 ) {
			glr_debug_printf( "%s() : ###ERROR### ", theFunc->name );
			glr_debug_vprintf( inFormat, theArgs );
		} else {
			glr_debug_printf( "ERROR : missing function declaration | %s", inFormat);
		}
	} else {
		glr_debug_printf( "???? : ###ERROR### " );
		glr_debug_vprintf( inFormat, theArgs );
	}

	va_end( theArgs );
}


/*
________________________________________________________________________________________

      glr_debug_not_yet_implemented
________________________________________________________________________________________

*/

void
glr_debug_not_yet_implemented(
	const char *			inName,
	long					inLine,
	char *					inFile)
{
	glr_debug_printf( "### NOT YET IMPLEMENTED ### : %s @ line %d, file = %s\n", inName, inLine, inFile );
}


/*
________________________________________________________________________________________

      glr_debug_terminate
________________________________________________________________________________________

*/

void
glr_debug_terminate()
{
	TFuncDbg *				theFunc;
	TFuncDbg *				theNextFunc;
	
	theFunc = gFirstFunc;
	
	while ( theFunc ) {
		theNextFunc = theFunc->next;
#if __use_glm__
		glmFree( theFunc );
#else
		free( theFunc );
#endif
		theFunc = theNextFunc;
	}

}


/*
________________________________________________________________________________________

      glr_debug_default_env_setting
________________________________________________________________________________________

*/

void
glr_default_env_setting()
{

#if DEBUG_DONT_TRACE_THESE

	glr_add_func( "gldCreateTexture", 0, false );
	glr_add_func( "gldBindTexture", 0, false );
	glr_add_func( "gldSwapBuffers", 0, false );
	glr_add_func( "gldClear", 0, false );

	glr_add_func( "gldGetRenderDispatch", 0, false );

#endif
	
	glr_add_func( "gldClear", 0, false );
	
}


/*
________________________________________________________________________________________

      glr_add_func
________________________________________________________________________________________

*/

TFuncDbg *
glr_add_func(
	const char *			inName,
	const void *			inAddr,
	long					inTraceEntry)
{
	TFuncDbg *				theFunc = 0;

#if __use_glm__
	theFunc = glmMalloc( sizeof( TFuncDbg ) );
#else
	theFunc = malloc( sizeof( TFuncDbg ) );
#endif
	if ( theFunc != 0 ) {

		strcpy( theFunc->name, inName );
		theFunc->addr = inAddr;
		theFunc->traceEntry = inTraceEntry;
		
		theFunc->previous = 0;
		theFunc->next = gFirstFunc;
		gFirstFunc = theFunc;
	}
	
	return theFunc;
}



/*
________________________________________________________________________________________

      glr_find_by_name
________________________________________________________________________________________

*/

TFuncDbg *
glr_find_by_name(
	const char *			inName)
{
	TFuncDbg *				theFunc;

	theFunc = gFirstFunc;

	while( theFunc && strcmp( theFunc->name, inName ) ) {
		theFunc = theFunc->next;
	}
	
	return theFunc;
}



/*
________________________________________________________________________________________

      glr_find_by_addr
________________________________________________________________________________________

*/

TFuncDbg *
glr_find_by_addr(
	const void *			inAddr)
{
	TFuncDbg *				theFunc;

	theFunc = gFirstFunc;

	while( theFunc && ( theFunc->addr != inAddr) ) {
		theFunc = theFunc->next;
	}
	
	return theFunc;
}


/*
________________________________________________________________________________________

      glr_check_for_key
________________________________________________________________________________________

*/

static KeyMap key_map[2];
static int current_key_map = 0;

void glr_get_keys(void)
{
	current_key_map = (current_key_map + 1) & 1;
	GetKeys(key_map[current_key_map]);
}


GLint glr_check_for_key(GLint key)
{
	return key_map[current_key_map][key >> 5] & (1L << (key & 0x1f));
}


GLint glr_check_for_key_press(GLint key)
{
	if(!(key_map[current_key_map][key >> 5] & (1L << (key & 0x1f)))){
		return 0;
	}
	
	if(key_map[current_key_map ^ 1][key >> 5] & (1L << (key & 0x1f))){
		return 0;
	}
	
	return 1;
}







