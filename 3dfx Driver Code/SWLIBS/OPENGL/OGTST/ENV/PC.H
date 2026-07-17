#pragma warning(disable : 4244)
#pragma warning(disable : 4018)
#pragma warning(disable : 4761)
#pragma warning(disable : 4245)
#pragma warning(disable : 4305)

#include <windows.h>

#include <GL/gl.h>
#include <GL/glu.h>

#define M_PI 3.14156

#define __psint_t int

#define alloca _alloca
#define bcopy(src, dst, n) memcpy((dst), (src), (n))
#define bcmp(b0, b1, n) memcmp((b0), (b1), (n))
#define bzero(b, n) memset((b), 0, (n))
#define floorf(x) (float)floor((double)(x))
#define fabsf(x) (float)fabs((double)(x))
#define ceilf(x) (float)ceil((double)(x))


extern double rint(double);
extern int ffs(unsigned int);
