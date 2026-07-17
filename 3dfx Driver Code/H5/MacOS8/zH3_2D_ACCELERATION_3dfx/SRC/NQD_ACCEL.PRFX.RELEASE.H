#ifdef __powerc
	#include	"MacHeadersPPC"
#else
	#include	"MacHeaders68K"
#endif

#define SET_BSWAP 1

#define GDX_SWIZZLE_HACK 1
#define PCI_COPYBACK     1
#define PCI_BUMP_N_GRIND 1

#if DEBUG
#  define DCON 1
#else
#  define DCON 0
#endif