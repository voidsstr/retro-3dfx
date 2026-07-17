/* mymalloc.h:  macros to replace malloc/free with my functions */
#include <stdlib.h>
#include <malloc.h>

#define realloc(p, s)	my_realloc ((p), (s), __FILE__, __LINE__)
#define malloc(n)	my_malloc ((n), __FILE__, __LINE__)
#define free(n)		my_free ((n), __FILE__, __LINE__)
