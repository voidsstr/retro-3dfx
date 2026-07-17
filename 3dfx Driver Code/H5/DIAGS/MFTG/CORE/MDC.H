
#include <stdio.h>

#define ASSERT(a,b) if (!(a)) { printf("ASSERT Failure in %s line %d: %s\n", __FILE__, __LINE__ , b); exit(1);}
				    
