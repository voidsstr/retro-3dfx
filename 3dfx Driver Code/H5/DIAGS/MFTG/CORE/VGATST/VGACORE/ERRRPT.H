/*
 * errrpt.h
 *
 */

typedef enum {
  cont = 0,    /* continue on failure */
  stop,            /* stop on failure */
  loop,            /* loop on failure */
} FAILURE_MODE;


typedef unsigned long int ERR_EVENT;

typedef enum {
  fatal             = 0x0000,        /* required messages */
  fatal_ex          = 0x1000,        /* explanation of a fatal message */

  
  info              = 0x0004,        /* information (i.e. memory size, number of tmus, etc) */
  info_ex           = 0x1004,        /* information explanation */
  code_debugging    = 0x0006,
} DEBUG_LVL;

ERR_EVENT newEvent( void );
ERR_EVENT logError( ERR_EVENT evnt, DEBUG_LVL dlvl, char *format, ...);
ERR_EVENT logMessage( ERR_EVENT evnt, DEBUG_LVL, char *format, ...);
FAILURE_MODE onFailure(void);
void signalFailure(void);

char *getCfgString(char *module, char *name, char *def);
unsigned long int  getCfgNumber(char *module, char *name, unsigned long int  def);
double getCfgDouble(char *module, char *name, double def);
