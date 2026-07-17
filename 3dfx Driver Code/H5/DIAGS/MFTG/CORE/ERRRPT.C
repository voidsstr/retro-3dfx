/* 
 * errrpt.c
 *
 * Error Reporting 
 *
 *
 * EVENT newEvent(); // give back a unique event number
 * logError(EVENT, DEBUG_LEVEL, message, ...)
 * logMessage(EVENT, DEBUG_LEVEL, message, ...)
 * onFailure(); // tell the code what to do on failure (stop, continue, loop_on_failure)
 
 * A new event "token" will be generated for each failure event which
 * is being described. In this way the diag core can distinguish
 * between many messages for the same error, and many messages for
 * different errors. logError() would be used to report about failing
 * conditions, while logMessage would report about non-failing
 * conditions. The debug levels would be explicitly specified, and
 * would allow the user to control how much information he saw.  
 *
 */

#include <stdio.h>
#include <stdarg.h>

#include "errrpt.h"
#include "time.h"

unsigned long int last_event = 1;

FAILURE_MODE cur_mode = 0; /* we really want to do this _per module_ later */

ERR_EVENT newEvent( void ) {
  return (last_event++);
}

static char error_time[70];
extern FILE *stdout_file;

ERR_EVENT logError( ERR_EVENT evnt, DEBUG_LVL dlvl, char *format, ...) {
  va_list ap;
  time_t now = time(NULL);
  struct tm *localt = localtime(&now);
  
  strftime(error_time,70,"%m%d%y %H:%M",localt);


  if (evnt == 0) {
    evnt = newEvent();
  }


  fprintf(stdout,"E%04d:%02d:%s:",evnt,dlvl,error_time);

  va_start(ap,format);
  vfprintf(stdout,format,ap);
  va_end(ap);
  if (stdout_file) {
    va_start(ap,format);
    vfprintf(stdout_file,format,ap);
    va_end(ap);
  }

  fprintf(stdout,"\n");

  return (evnt);
}

ERR_EVENT logMessage( ERR_EVENT evnt, DEBUG_LVL dlvl, char *format, ...) {
  va_list ap;
  time_t now = time(NULL);
  struct tm *localt = localtime(&now);

  strftime(error_time,70,"%m%m%y %H:%M",localt);
  if (evnt == 0) {
    evnt = newEvent();
  }

  fprintf(stdout,"M%04d:%02d:%s:",evnt,dlvl,error_time);

  va_start(ap,format);
  vfprintf(stdout,format,ap);
  va_end(ap);
  if (stdout_file) {
    va_start(ap,format);
    vfprintf(stdout_file,format,ap);
    va_end(ap);
  }

  fprintf(stdout,"\n");

  return (evnt);
}

FAILURE_MODE onFailure(void) {
  return (cur_mode);
}

/*
 * void signalFailure(void);
 * 
 * this should signal the failure via:
 *        port 0x80
 *        a parallel port pin
 *        a GPIO pin (?)
 */

void signalFailure(void) {

}

#include <lua.h>

lua_Object getCfgObject(char *module, char *name) {
  lua_Object temp = lua_getglobal("cfgvars");
  lua_Object retval = LUA_NOOBJECT;
  
  if (lua_istable(temp)) {
    lua_pushobject(temp);
    lua_pushstring(module);
    temp = lua_gettable();
    if (lua_istable(temp)) {
      lua_pushobject(temp);
      lua_pushstring(name);
      temp = lua_gettable();
      if (lua_isstring(temp)) {
        retval = temp;
      }
    }
  }

  return retval;
}
 
char *getCfgString(char *module, char *name, char *def) {
  char *retval = def;
  lua_Object temp;
  
  if (lua_isstring(temp = getCfgObject(module,name))) {
    retval = lua_getstring(temp);
  }

  return (retval);
}

unsigned long int getCfgNumber(char *module, char *name, unsigned long int  def) {
  unsigned long int retval = def;
  lua_Object temp;

  if (lua_isnumber(temp = getCfgObject(module,name))) {
    retval = lua_getnumber(temp);
  }

  return (retval);
}

double getCfgDouble(char *module, char *name, double def) {
  double retval = def;
  lua_Object temp;

  if (lua_isnumber(temp = getCfgObject(module,name))) {
    retval = lua_getnumber(temp);
  }

  return (retval);
 
}






