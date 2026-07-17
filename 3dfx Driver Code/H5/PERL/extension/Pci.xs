#ifdef __cplusplus
extern "C" {
#endif
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

static unsigned long devnum;
static unsigned long board;
static unsigned long sizeofcard;
static unsigned long configmech = 0;

unsigned long * __stdcall pciMapCardMulti (unsigned long, unsigned long, unsigned long, unsigned long *, unsigned long, unsigned long);
int __stdcall pciClose(void);
void __stdcall pciUnmapPhysical(unsigned long, unsigned long);
unsigned long __stdcall _pciFetchRegister(unsigned long, unsigned long, unsigned long, unsigned long);
void __stdcall _pciUpdateRegister(unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
int __stdcall pciFindCardMulti(unsigned long, unsigned long, unsigned long *, unsigned long);

#ifdef __cplusplus
}
#endif


MODULE = Pci                PACKAGE = Pci               


unsigned long
mapCard(vendor, devid, size, base, card)
	unsigned long vendor
	unsigned long devid
	unsigned long size
	unsigned long base
	unsigned long card

	PROTOTYPE: $$$$$
	CODE:
	board = (unsigned long)pciMapCardMulti(vendor, devid, size, &devnum, base, card); 
	RETVAL = board;
	sizeofcard = size;
	configmech = 1;
	OUTPUT:
	RETVAL

int
find(vendor, devid, card)
	unsigned long vendor
	unsigned long devid
	unsigned long card

	PROTOTYPE: $$$
	CODE:
	int flag;
	flag = pciFindCardMulti(vendor, devid, &devnum, card);
	if (flag) {
		configmech = 1;
	}
	RETVAL = flag;
	OUTPUT:
	RETVAL

unsigned char
read(address)
	unsigned long address

	PROTOTYPE:  $
	CODE:
	unsigned char *ptr;
	ptr = (unsigned char *)address;
	RETVAL = *ptr;
	OUTPUT:
	RETVAL

unsigned short
readw(address)
	unsigned long address

	PROTOTYPE:  $
	CODE:
	unsigned short *ptr;
	ptr = (unsigned short *)address;
	RETVAL = *ptr;
	OUTPUT:
	RETVAL

unsigned long
readd(address)
	unsigned long address

	PROTOTYPE:  $
	CODE:
	unsigned long *ptr;
	ptr = (unsigned long *)address;
	RETVAL = *ptr;
	OUTPUT:
	RETVAL

void
write(address, data)
	unsigned long address
	unsigned char data

	PROTOTYPE: $$
	CODE:
	unsigned char *ptr;
	ptr = (unsigned char *)address;
	*ptr = data;

void
writew(address, data)
	unsigned long address
	unsigned short data

	PROTOTYPE: $$
	CODE:
	unsigned short *ptr;
	ptr = (unsigned short *)address;
	*ptr = data;

void
writed(address, data)
	unsigned long address
	unsigned long data

	PROTOTYPE: $$
	CODE:
	unsigned long *ptr;
	ptr = (unsigned long *)address;
	*ptr = data;

unsigned long
inp(port)
	unsigned long port

	PROTOTYPE: $
	CODE:
	RETVAL = _inp(port);
	OUTPUT:
	RETVAL

unsigned long
inpw(port)
	unsigned long port

	PROTOTYPE: $
	CODE:
	RETVAL = _inpw(port);
	OUTPUT:
	RETVAL

unsigned long
inpd(port)
	unsigned long port

	PROTOTYPE: $
	CODE:
	RETVAL = _inpd(port);
	OUTPUT:
	RETVAL

void
outp(port, data)
	unsigned long port
	unsigned long data

	PROTOTYPE: $$
	CODE:
	_outp(port, data);

void
outpw(port, data)
	unsigned long port
	unsigned long data

	PROTOTYPE: $$
	CODE:
	_outpw(port, data);

void
outpd(port, data)
	unsigned long port
	unsigned long data

	PROTOTYPE: $$
	CODE:
	_outpd(port, data);

unsigned long
readcfg(address)
	unsigned long address

	PROTOTYPE: $
	CODE:
	RETVAL =_pciFetchRegister(address, 4, devnum, configmech);
	OUTPUT:
	RETVAL

void
writecfg(address, data)
	unsigned long address
	unsigned long data

	PROTOTYPE: $$
	CODE:
	_pciUpdateRegister(address, data, 4, devnum, configmech);

void
close()

	CODE:
	pciUnmapPhysical (board, sizeofcard);
	pciClose();
	configmech = 0;
