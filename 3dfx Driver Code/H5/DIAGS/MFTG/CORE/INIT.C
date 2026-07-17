/*
 * init.c
 *
 * This code just handles calling everyones init functions.
 */

#include <stdio.h>
#include <stdlib.h>
// #include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>

#include "3dfx.h"
#include "fxpci.h"
#include "vgamisc.h"

#include "init.h"

/* internal prototypes */

int mdc_init_core(void);


/* 
 * mdc_init
 *
 * This is the top level init 
 */

int mdc_original_video_mode = 0;
int mdc_text_output_enable = 1;

int mdc_set_output_enable(int state) {
  int retval = mdc_text_output_enable;

  mdc_text_output_enable = state;

  return (retval);
}

int mdc_init(void) {
  int retval = FXTRUE;

   mdc_original_video_mode = GetMode();

  retval |= mdc_init_core();

  return retval;
}


/***********************************
 *  MDC CORE INIT 
 **********************************/

#include "lua.h"
#include "luadebug.h"
#include "lualib.h"
#include "mdclua.h"
#include "builtin.h"


/* we're going to do the nasty here and directly declare some extern references */

BOOL OpenMonoDevice(void);
BOOL CloseMonoDevice(void);
BOOL WriteStringMonoDevice(void);
BOOL MonoFullScreenScroll(void); // scroll the screen up one row
BOOL MonoSetCursorPosition(int nCol, int nRow);

int mono_open = 0;


extern int lua_debug;  /* debug mode on or off! */


/* declare prototypes! */

#define MOD_START(fn) void fn(void);
#include "mod.def"
#undef MOD_START

#define MOD_START(fn) fn();
void mdc_startlua() {

  /* startup lua */
  lua_open();
  lua_debug = 1;
  lua_iolibopen();
  lua_strlibopen();
  lua_mathlibopen();
  
  /* register functions */
  mdc_register_luafns();   /* call main module init */

#include "mod.def"

  lua_dostring(bin_data);
  /* lua_dofile("builtin.lua"); */ /* load builtins, these should be brought into the EXE */
  
}

#undef MOD_START

/*
 * push command line args into lua "_args" global 
 */

void mdc_args_to_lua(int argc, char **argv) {
  int x=0;
  lua_Object args_tbl;
  int args_ref;

  args_tbl = lua_createtable();
  lua_pushobject(args_tbl);
  args_ref = lua_ref(1); /* locked! */

  lua_pushobject(args_tbl);
  lua_setglobal("_args");

  for(x=0;x<argc;x++) {
    lua_pushobject(lua_getref(args_ref));
    lua_pushnumber(x);
    lua_pushstring(argv[x]);
    lua_settable();
  }

  lua_unref(args_ref); /* unlock! */
}

void mdc_run_luascript(void) {
  int retval;
  lua_beginblock();

  retval = lua_dofile("lua\\init.lua");
  lua_endblock();

  /* run lua init file */
  switch (retval) {
  case 0: /* no errors */
    break;
  case 2: /* file could not be opened */
    printf("No .\\lua\\init.lua could be opened\n");
    break;
  default: /* some other error */
    printf("Lua error\n");
    mdc_exit();
  }
}

#include <stdarg.h>

FILE *stdout_file = NULL;
FILE *stderr_file = NULL;

int fputs(const char *string, FILE *fp) {
  int retval;
  retval = fprintf(fp,"%s",string);
  return (retval);
}

int puts(const char *string) {
  int retval;
  retval = fputs(string,stdout);

  return (retval);
}

int printf(const char *format, ...) {
  va_list ap;
  int retval;

  if (mdc_text_output_enable) {
    va_start(ap,format);
    retval = vfprintf(stdout,format,ap);
    va_end(ap);
  }

  if (stdout_file) {
    va_start(ap,format);
    vfprintf(stdout_file,format,ap);
    va_end(ap);
    fflush(stdout_file);
  }
  return (retval);
}

int fprintf(FILE *fp,const char *format,...) {
  va_list ap;
  int retval;

  if (mdc_text_output_enable || ((fp != stdout) && (fp != stderr))) {
    va_start(ap,format);
    retval = vfprintf(fp,format,ap);
    va_end(ap);
  }

  va_start(ap,format);
  if (fp == stdout) {
    vfprintf(stdout_file,format,ap);
    fflush(stdout_file);
  } else if (fp == stderr) {
    vfprintf(stderr_file,format,ap);
    fflush(stderr_file);
  }
  va_end(ap);

  return (retval);
}


int break_requested = 0;

void null_signal(int sig) {
 printf("signal %d\n",sig);
 break_requested = 1;
 mdc_exit(); // restore text and stuff!
 printf("*** MDC Exit -- interrupted by user **\n");
 exit(1);// exit from user termination
// signal(sig,null_signal);
}


int mdc_init_core(void) {

   signal(SIGINT,null_signal);
   signal(SIGBREAK, null_signal);

   
  /* open FXPCI */

  if ( !pciOpen() ) {
    puts( pciGetErrorString() );
    return FXFALSE;
  }

  /*
   * open mono device 
   * this should get cleaned up eventually...
   */

//   if (mono_open = OpenMonoDevice()) {
//    WriteStringMonoDevice("test");
//  }

  mdc_startlua(); 

  return FXTRUE;
}


void mdc_restore_text(void) {
  if (mdc_original_video_mode != GetMode()) { 
    SetMode(mdc_original_video_mode);
  }
}

/**************************
 * MDC EXIT 
 *************************/

void mdc_exit(void) {
  if ( !pciClose() ) {
    puts( pciGetErrorString() );
    /* return; */
  }


  mdc_restore_text();
  /* need to clear the screen or do something intelligent */

  /* lua_close(); */
  
}


#ifdef JESKE_FIGURED_OUT_EXIT

__exit() {
	/* close lua */
	printf("Lua closed\n");
	lua_close();
}
#endif



