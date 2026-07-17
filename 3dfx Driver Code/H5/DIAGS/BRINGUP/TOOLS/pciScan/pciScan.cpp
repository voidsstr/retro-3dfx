#include <conio.h>
#include <stdio.h>
#include <windows.h>

#define TDFX_VENDOR_ID 0x121a

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

#define CONFIG_SPACE_VENDOR_ID        0x0
#define CONFIG_SPACE_DEVICE_ID        0x2
#define CONFIG_SPACE_BASE_ADDRESS0    0x10
#define CONFIG_SPACE_BASE_ADDRESS1    0x14
#define CONFIG_SPACE_IO_BASE          0x18
#define CONFIG_SPACE_ROM_BASE         0x30


unsigned int generateConfigAddress(unsigned int busNumber,
				   unsigned int deviceNumber,
				   unsigned int functionNumber,
				   unsigned int registerNumber);

unsigned int pciConfigRead(unsigned int busNumber,
			   unsigned int deviceNumber,
			   unsigned int functionNumber,
			   unsigned int registerNumber);
bool probeDeviceInfo(unsigned int busNumber,
		     unsigned int deviceNumber,
		     unsigned int functionNumber);

int main(void)
{
  unsigned int busNumber, deviceNumber, functionNumber;
  int n3dfxDevices=0;

  for(busNumber=0; busNumber<256; busNumber++)
    for(deviceNumber=0; deviceNumber<32; deviceNumber++)
      for(functionNumber=0; functionNumber<8; functionNumber++)
	{
	  if(probeDeviceInfo(busNumber, deviceNumber, functionNumber))
	    n3dfxDevices++;
	}

  if(n3dfxDevices == 0)
    {
      printf("SHIT! There aren't any f'n 3dfx devices on the bus.\n");
      printf("Press a key to continue\n");
      getch();
    }

  return(0);
}

unsigned int generateConfigAddress(unsigned int busNumber,
				   unsigned int deviceNumber,
				   unsigned int functionNumber,
				   unsigned int registerNumber)
{
  unsigned int address;

  address =  ((busNumber & 0xFF) << 16);
  address |= ((deviceNumber & 0x1F) << 11);
  address |= ((functionNumber & 0x7) << 8);
  address |= ((registerNumber & 0x3F) << 2);
  address |= 1 | (1<<31);

  return(address);
}

unsigned int pciConfigRead(unsigned int busNumber,
			   unsigned int deviceNumber,
			   unsigned int functionNumber,
			   unsigned int registerNumber)
{
  unsigned int address, data;

  address = generateConfigAddress(busNumber,
				  deviceNumber,
				  functionNumber,
				  registerNumber);
  
  _outpd(PCI_CONFIG_ADDRESS, address);
  data = _inpd(PCI_CONFIG_DATA);
  
  return(data);
}


bool probeDeviceInfo(unsigned int busNumber,
		     unsigned int deviceNumber,
		     unsigned int functionNumber)
{
  unsigned int vendorID, deviceID;
  unsigned int baseAddress0, baseAddress1;
  unsigned int ioBase, romBase;
  unsigned int address, data;

  bool is3dfxDevice=false;

  data = pciConfigRead(busNumber, deviceNumber, functionNumber,
		       CONFIG_SPACE_VENDOR_ID >> 2);
  
  vendorID = data & 0xFFFF;
  deviceID = data >> 16;
  
  if(vendorID != 0xFFFF)
    {
      baseAddress0 = pciConfigRead(busNumber, deviceNumber, functionNumber,
				   CONFIG_SPACE_BASE_ADDRESS0 >> 2);
      baseAddress1 = pciConfigRead(busNumber, deviceNumber, functionNumber,
				   CONFIG_SPACE_BASE_ADDRESS1 >> 2);

      ioBase = pciConfigRead(busNumber, deviceNumber, functionNumber,
			     CONFIG_SPACE_IO_BASE >> 2);
      romBase = pciConfigRead(busNumber, deviceNumber, functionNumber,
			      CONFIG_SPACE_ROM_BASE >> 2);

      if(vendorID == TDFX_VENDOR_ID)
	{
	  is3dfxDevice = true;
	  
	  printf("\n");
	  printf("busNumber: 0x%02x deviceNumber: 0x%02x  functionNumber: 0x%02x\n",
		 busNumber, deviceNumber, functionNumber);
	  printf("   vendorID: 0x%04x  deviceID: 0x%04x\n", vendorID, deviceID);
	  printf("   baseAddress0: 0x%08x  baseAddress1: 0x%08x\n",
		 baseAddress0, baseAddress1);
	  printf("   ioBase: 0x%08x  romBase: 0x%08x\n", ioBase, romBase);		 
	}
    }
  
  return(is3dfxDevice);
}
