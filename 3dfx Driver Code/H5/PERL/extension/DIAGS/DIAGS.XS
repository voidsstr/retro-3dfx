#ifdef __cplusplus
extern "C" {
#endif
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
#ifdef __cplusplus
}
#endif

static int
not_here(s)
char *s;
{
    croak("%s not implemented on this architecture", s);
    return -1;
}

static double
constant(name, arg)
char *name;
int arg;
{
    errno = 0;
    switch (*name) {
    }
    errno = EINVAL;
    return 0;

not_there:
    errno = ENOENT;
    return 0;
}


MODULE = diags          PACKAGE = diags         


double
constant(name,arg)
	char *          name
	int             arg

int
solid(base, value, size)
	unsigned long base
	unsigned long value
	unsigned long size

	PROTOTYPE: $$$
	CODE:
	int     i, ret;
	unsigned long *addr;

        for (i = 0; i < size; i+=4) {
		addr = (unsigned long *)(base + i);
		*addr = value;
	}

	ret = 1;
        for (i = 0; i < size; i+=4) {
		addr = (unsigned long *)(base + i);
		if (*addr != value) {
			ret = 0;
			printf ("Address %x  == %x, should be %x\n",
				i, *addr, value);
		}
	}
	RETVAL = ret;
	OUTPUT:
	RETVAL

int
address(base, size)
	unsigned long base
	unsigned long size

	PROTOTYPE: $$
	CODE:
	int     i, ret;
	unsigned long *addr;

        for (i = 0; i < size; i+=4) {
		addr = (unsigned long *)(base + i);
		*addr = i;
	}

	ret = 1;
        for (i = 0; i < size; i+=4) {
		addr = (unsigned long *)(base + i);
		if (*addr != i) {
			ret = 0;
			printf ("Address %x == %x, should be %x\n",
				i, *addr, i);
		}
	}
	RETVAL = ret;
	OUTPUT:
	RETVAL

int
random(base, size, count)
	unsigned long base
	unsigned long size
	unsigned long count

	PROTOTYPE: $$$
	CODE:
	int     i, ret, r, d;
	unsigned long *addr;

	ret = 1;
	for (i = 0; i < count; i++) {
                r = ((rand() << 16) | rand()) & 0xFFFFFFFC;
		d = (rand() << 16) | rand();
		addr = (unsigned long *)(base + (r % size));
		*addr = d;
		if (*addr != d) {
			ret = 0;
			printf ("Address %x == %x, should be %x\n",
				r % size, *addr, d);
		}
	}
	RETVAL = ret;
	OUTPUT:
	RETVAL

