
//Instead of scanning the first two PCI buses, scan the first 16 PCI buses.
//This fixes the problem with flashing on a Camino system  PCI is on bus
//   02h and not 00h.

#define MAX_PCI_DEVICES                480
//#define MAX_PCI_DEVICES                64
#define short int

#define UINT32      unsigned long
#define INT32       long
#define UINT16      unsigned short
#define INT16       short
#define UINT8       unsigned char
#define INT8        char
#undef short

typedef enum 
{
	READ_ONLY,
	WRITE_ONLY,
	READ_WRITE
} PciIOFlag;

typedef struct 
{
	UINT32              regAddress;
	UINT32              sizeInBytes;
	PciIOFlag          rwFlag;
}  PciRegister;
