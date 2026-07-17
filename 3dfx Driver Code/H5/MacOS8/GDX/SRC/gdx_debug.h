/*
** Copyright (c) 1996-1999, 3Dfx Interactive, Inc.
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
** File name:   gdx_debug.h
**
** Description: HAL layer for MacOS Display Driver.
**
**
*/


#ifndef __GDX_DEBUG_H__
#define __GDX_DEBUG_H__

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <MacTypes.h>


#if DEBUG
  #if DEBUG_USE_PRINTF

    #define LOG_ENTRY(_level) do { \
      if(VERBOSE_DEBUG >= _level) { \
        gdx_debug_printf("%s: Entry...\n", FN_NAME); \
      } \
    } while(0)

    #define LOG_EXIT(_level,_err) do { \
      if(VERBOSE_DEBUG >= _level) { \
        if ( _err ) \
          gdx_debug_printf( "%s: ##### exit with ERROR: %d\n",FN_NAME,_err); \
        else \
          gdx_debug_printf( "%s: exit with no error.\n",FN_NAME); \
      } \
    } while(0)

    #define LOG_PRINTF(_level,_fmt) do {\
      if(VERBOSE_DEBUG >= _level) { \
        gdx_debug_printf( "%s: %s",FN_NAME,_fmt); \
      } \
    } while(0)

    #define LOG_PRINTF1(_level,_fmt,_val) do {\
      if(VERBOSE_DEBUG >= _level) { \
        gdx_debug_printf( "%s: ",FN_NAME); \
        gdx_debug_printf(_fmt,_val); \
      } \
    } while(0)

    #define LOG_PRINTF2(_level,_fmt,_val1,_val2) do {\
      if(VERBOSE_DEBUG >= _level) { \
        gdx_debug_printf( "%s: ",FN_NAME); \
        gdx_debug_printf(_fmt,_val1,_val2); \
      } \
    } while(0)

    #define LOG_PRINTF3(_level,_fmt,_val1,_val2,_val3) do {\
      if(VERBOSE_DEBUG >= _level) { \
        gdx_debug_printf( "%s: ",FN_NAME); \
        gdx_debug_printf(_fmt,_val1,_val2,_val3); \
      } \
    } while(0)

    #define LOG_REFRESH gdx_debug_refresh

  #else

    #define LOG_ENTRY(_level) do {\
    if(VERBOSE_DEBUG >= _level) SysDebugStr("\p" FN_NAME ": Entry\r"); \
    } while(0)

    #define LOG_EXIT(_level,_err) do {\
    unsigned char _errBuff[256]; \
    if(VERBOSE_DEBUG >= _level) { \
    sprintf((char *)&_errBuff[1],(const char *)"%s exit with error: %d",FN_NAME,_err); \
    _errBuff[0] = (unsigned char)strlen((const char *)&_errBuff[1]); \
    SysDebugStr((const unsigned char *)_errBuff); \
    } \
    } while(0)

    #define LOG_PRINTF(_level,_fmt) do {\
    unsigned char _errBuff[256]; \
    if(VERBOSE_DEBUG >= _level) { \
    sprintf((char *)&_errBuff[1],(const char *)_fmt); \
    _errBuff[0] = (unsigned char)strlen((const char *)&_errBuff[1]); \
    SysDebugStr((const unsigned char *)_errBuff); \
    } \
    } while(0)

    #define LOG_PRINTF1(_level,_fmt,_val) do {\
    unsigned char _errBuff[256]; \
    if(VERBOSE_DEBUG >= _level) { \
    sprintf((char *)&_errBuff[1],(const char *)_fmt,_val); \
    _errBuff[0] = (unsigned char)strlen((const char *)&_errBuff[1]); \
    SysDebugStr((const unsigned char *)_errBuff); \
    } \
    } while(0)

    #define LOG_PRINTF2(_level,_fmt,_val1,_val2) do {\
    unsigned char _errBuff[256]; \
    if(VERBOSE_DEBUG >= _level) { \
    sprintf((char *)&_errBuff[1],(const char *)_fmt,_val1,_val2); \
    _errBuff[0] = (unsigned char)strlen((const char *)&_errBuff[1]); \
    SysDebugStr((const unsigned char *)_errBuff); \
    } \
    } while(0)

    #define LOG_PRINTF3(_level,_fmt,_val1,_val2,_val3) do {\
    unsigned char _errBuff[256]; \
    if(VERBOSE_DEBUG >= _level) { \
    sprintf((char *)&_errBuff[1],(const char *)_fmt,_val1,_val2,_val3); \
    _errBuff[0] = (unsigned char)strlen((const char *)&_errBuff[1]); \
    SysDebugStr((const unsigned char *)_errBuff); \
    } \
    } while(0)

    #define LOG_REFRESH() 

  #endif

#else

  #define LOG_ENTRY(_level)
  #define LOG_EXIT(_level,_err)
  #define LOG_PRINTF(_level,_fmt)
  #define LOG_PRINTF1(_level,_fmt,_val1)
  #define LOG_PRINTF2(_level,_fmt,_val1,_val2)
  #define LOG_PRINTF3(_level,_fmt,_val1,_val2,_val3)
  #define LOG_REFRESH() 

#endif

#define k3DfxDebugRequest        1234


void gdx_debug_init( void );
void gdx_debug_refresh( void );
void gdx_debug_printf( const char * inFormat, ...);
void gdx_debug_vprintf( const char * inFormat, va_list inArgs);

void gdx_ddc_printf( const char * inFormat, ...);
void gdx_ddc_vprintf( const char * inFormat, va_list inArgs);

void gdx_scc_printf( const char * inFormat, ...);
void gdx_scc_vprintf( const char * inFormat, va_list inArgs);
void gdx_scc_shutdown(void);


#endif /* __GDX_DEBUG_H__ */ 
