
// We use the timer() function from fxos.h
#include <time.h>

#define timer() ( clock()/(float)CLOCKS_PER_SEC )

#define MAX_BYTES 200000
#define MAX_BUFFER 1000
#define MAX_TIME 20000

#define MESSAGE_STRING "This is a message99.This is a message99.This is a message99." 
