#if PROFILE
#include <Profiler.h>

#define PROFILE_ENTRY(func) __PROFILE_ENTRY("." func)
#define PROFILE_EXIT() __PROFILE_EXIT()
	
#else

#define PROFILE_ENTRY(func)
#define PROFILE_EXIT()

#endif