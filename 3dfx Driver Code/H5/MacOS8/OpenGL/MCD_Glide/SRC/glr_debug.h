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
**
** 
**
*/


#ifndef __glr_Debug_H__
#define __glr_Debug_H__



#if GLR_DEBUG
#  define DEBUG_INIT                    glr_debug_init
#  define DEBUG_ENABLE                  glr_debug_enable
#  define DEBUG_PRINTF                  glr_debug_printf
#  define DEBUG_ENTRY(x)                glr_debug_entry( (void*) x, #x)
#  define DEBUG_ENTRY_INPUT             glr_debug_verbose
#  define DEBUG_ERROR                   glr_debug_error
#  define DEBUG_VERBOSE                 glr_debug_verbose
#  define DEBUG_NOT_YET_IMPLEMENTED(x)  glr_debug_not_yet_implemented( #x, __LINE__, __FILE__ )
#  define DEBUG_TERMINATE()				glr_debug_terminate()
#  define DEBUG_ASSERT(x)				if(!(x)) { DEBUG_PRINTF("Assertion failed. (%s) File %s, Line %d\n", #x, __FILE__, __LINE__);}
#else
#  define DEBUG_INIT                    0 && (unsigned long)
#  define DEBUG_ENABLE                  0 && (unsigned long)
#  define DEBUG_PRINTF                  0 && (unsigned long)
#  define DEBUG_ENTRY(x)                0 && (unsigned long) x
#  define DEBUG_ENTRY_INPUT             0 && (unsigned long)
#  define DEBUG_ERROR                   0 && (unsigned long)
#  define DEBUG_VERBOSE                 0 && (unsigned long)
#  define DEBUG_NOT_YET_IMPLEMENTED(x)  0 && (unsigned long) x
#  define DEBUG_TERMINATE()
#  define DEBUG_ASSERT(x)
#endif

#if DEBUG_FASTPATH && GLR_DEBUG
#define DEBUG_FASTPATH_ENTRY(x)			glr_debug_entry( (void *)x, #x)
#else
#define DEBUG_FASTPATH_ENTRY(x)			0 && (unsigned long) #x
#endif

#if DEBUG_SLOWPATH && GLR_DEBUG
#define DEBUG_SLOWPATH_ENTRY(x)			glr_debug_entry( (void *)x, #x)
#else
#define DEBUG_SLOWPATH_ENTRY(x)			0 && (unsigned long) #x
#endif

void		glr_debug_init(
				const unsigned char	*	inLibName);
				
void		glr_debug_enable(
				int enabled);
				
void		glr_debug_printf(
				const char *			inFormat,
										...);
										
void		glr_debug_entry(
				const void *			inAddr,
				const char *			inName);

void		glr_debug_error(
				const void *			inAddr,
				const char *			inFormat,
										...);

void		glr_debug_verbose(
				const void *			inAddr,
				const char *			inFormat,
										...);

void		glr_debug_not_yet_implemented(
				const char *			inName,
				long					inLine,
				char *					inFile);

void		glr_debug_terminate( void );

void 		glr_get_keys(void);
GLint		glr_check_for_key(GLint key);
GLint		glr_check_for_key_press(GLint key);

#endif // __glr_Debug_H__
