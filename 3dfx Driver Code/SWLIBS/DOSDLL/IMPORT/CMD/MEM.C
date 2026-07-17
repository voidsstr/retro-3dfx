#include <stdio.h>
#include <malloc.h>

// If a nonzero argument is passed, this function allocates up to that
// amount of memory and doesn't free it.  If a zero argument is passed,
// this function tests how much memory can be allocated with malloc and
// then returns it to the system.
unsigned long __cdecl mem (unsigned long limit)
	{
	unsigned long total = 0;
	unsigned long try = 0x80000000;
	void *block, *list = NULL;

	do
		{
		try /= 2;
		while (((! limit) || (total + try < limit)) && (block = malloc (try)))
			{
			*((void **) block) = list;
			list = block;
			total += try;
			}
		}
	while (try >= sizeof(void *));
	if (limit == 0)
		{
		while (list)
			{
			block = list;
			list = *((void **) block);
			free (block);
			}
		_heapmin();
		}
	return (total);
	}
