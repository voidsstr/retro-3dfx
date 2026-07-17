/*						      
 * mdc version file
 */


#include "version.h"

#define MDC_VERSION_STR "N_7a"

#ifdef DEBUG
const char *mdc_version_str = "DEBUG BUILD (" MDC_VERSION_STR "_DEBUG)";
#else
const char *mdc_version_str = MDC_VERSION_STR;
#endif

const char *mdc_builddate_str = __DATE__ " " __TIME__;
