#include <stdio.h>
#include <i86.h>
#include "3dfx.h"
#include "fxpci.h"
#include "h3regs.h"
#include "ediag.h"
#include "misc.h"

void ReadSCL(unsigned char*);
void ReadSDA(unsigned char*);
void SetSDALine();
void ResetSDALine();
void SetSCLLine();
void ResetSCLLine();
unsigned char I2CReadByte(int);
int I2CSendByte(unsigned char);
void I2CStart();
void I2CStop();
int I2cAck(int);

#define H3_VMI_ENABLE_MASK              0x00800000
#define H3_VMI_CLOCK_MASK               0x01000000
#define H3_VMI_DATA_MASK                0x02000000
#define H3_VMI_CLOCK_STATE_MASK         0x04000000
#define H3_VMI_DATA_STATE_MASK          0x08000000
#define ACK		 	      	 0	   	// I2C Acknowledge
#define NACK	      			 1	   	// I2C No Acknowledge
#define MASTER_WRITE (0x00)
#define MASTER_READ (0x01)
#define SCL_BIT_ON (0x01)
#define SCL_BIT_OFF (0x00)

// Bit definition for vidSerialParallelPort 78h
// 18 DDC Port Enable
// 19 DDC SCK
// 20 DDC SDA
// 21 DDC SCK Read
// 22 DDC SDA Read
// 23 I2C Port Enable
// 24 I2C SCK
// 25 I2C SDA
// 26 I2C SCK Read
// 27 I2C SDA Read
static volatile FxU32 *VidSerialParallel;



/* Routine to print all devices found on the PCI bus */
void printPciInfo(void) {
    FxU32  deviceNumber;
    FxBool firstDeviceDetected = FXFALSE;
    FxU32  deviceID = 0;
    FxU32  vendorID = 0;
    FxU32  baseAddress0 = 0;
    FxU32  baseAddress1 = 0;
    FxU32  command = 0;
    FxU32  classCode = 0;
    FxU32  revID = 0;
    FxU32  subvendorID = 0;
    FxU32  subsystemID = 0;

    for ( deviceNumber = 0; deviceNumber < MAX_PCI_DEVICES; deviceNumber++ ) {
        if ( pciDeviceExists( deviceNumber ) ) {

            pciGetConfigData( PCI_DEVICE_ID, deviceNumber, &deviceID );
            pciGetConfigData( PCI_VENDOR_ID, deviceNumber, &vendorID );
            pciGetConfigData( PCI_BASE_ADDRESS_0, deviceNumber, &baseAddress0 );
            pciGetConfigData( PCI_BASE_ADDRESS_1, deviceNumber, &baseAddress1 );
            pciGetConfigData( PCI_COMMAND, deviceNumber, &command );
            pciGetConfigData( PCI_CLASS_CODE, deviceNumber, &classCode );
	     pciGetConfigData( PCI_REVISION_ID, deviceNumber, &revID );
            pciGetConfigData( PCI_SUBVENDOR_ID, deviceNumber, &subvendorID);
            pciGetConfigData( PCI_SUBSYSTEM_ID, deviceNumber, &subsystemID);


       /* new output style */

            if (!firstDeviceDetected) {
                firstDeviceDetected = FXTRUE;
                printf("--------+--------------------------------------------\n");
            }

            printf("Bus Slot| %s (%s)\n", 
                   pciGetVendorName((FxU16)vendorID),
                   pciGetClassName(classCode,deviceID));
            printf(" %.02d  %.02d | VendorID  = 0x%.04lx       DeviceID = 0x%.04lx\n",
                   deviceNumber>>5, deviceNumber & 0x1f, 
                   vendorID, deviceID);
            printf("        | Subvendor = 0x%.08lx   Subsystem = 0x%.04lx\n",
                           subvendorID, subsystemID);
            printf("        | baseAddr0 = 0x%.08lx   revID     = 0x%.04lx\n",
                           baseAddress0, revID);
            printf("        | baseAddr1 = 0x%.08lx   cmd       = 0x%.04lx\n",
                           baseAddress1, command);
            printf("--------+--------------------------------------------\n");

        }
    }

    if ( !firstDeviceDetected ) puts( "No PCI devices detected." );
}



/****************************************************************************/
/*                                                                          */
/*	                   Voodoo3 I2C Support 	                     	     */
/*                                                                          */
/****************************************************************************/

int VD3I2cWrite(unsigned char ChipAdr, FxU32 AdrLen, unsigned char* Adr, FxU32 DataLen, unsigned char* Data)
{
    I2CStart();
    if (!I2CSendByte((unsigned char)((ChipAdr) | MASTER_WRITE))) {
        for ( ; AdrLen; AdrLen--) 
        {
            if (I2CSendByte(*Adr++) )        // send sub-register byte(s)
            {
//		printf("I2cWrite:  Acknowledge failed in response to 0x00 write\n");
               I2CStop();                    // acknowledge failed --> generate stop condition
               return -1; 
            }
        }

        for ( ; DataLen; DataLen--)            // send data byte(s)
        {
           if ( I2CSendByte(*Data++ ))
           {
//		printf("I2cWrite:  Acknowledge failed in response to data write\n");
               I2CStop();                     // acknowledge failed --> generate stop condition
               return -1; 
           }
        }
    } else {
//	printf("I2cWrite:  Acknowledge failed in response to 0xA0 write\n");
       I2CStop();
       return -1;
    }
    I2CStop();

    return 0;
}




int VD3I2cRead(unsigned char ChipAdr, FxU32 AdrLen, unsigned char* Adr, FxU32 DataLen, unsigned char* Data)
{
    unsigned char dataread;

    if( AdrLen > 0 )
    {
        I2CStart();
        if(I2CSendByte((unsigned char)((ChipAdr)|MASTER_WRITE)) )        // send chip adr. with write bit
 	{
//		printf("I2cRead:  Acknowledge failed in response to 0xA0 write\n");
               I2CStop();                         // acknowledge failed --> generate stop condition
		return -1; 
        }
    	for ( ; AdrLen; AdrLen--)               // send sub-register address byte(s)
	{
                if (I2CSendByte(*Adr++ ) )
		{
//			printf("I2cRead:  Acknowledge failed in response to 0x00 write\n");
                       I2CStop();                      // acknowledge failed --> generate stop condition
			return -1;
		}
        }

    }
    I2CStart();                             // send again chip address for switching to read mode
    if ( I2CSendByte((unsigned char)((ChipAdr)|MASTER_READ)) )  // send chip adr. with read bit
    {
//	 printf("I2cRead:  Acknowledge failed in response to 0xA1 read\n");
        I2CStop();                         // acknowledge failed --> generate stop condition
        return -1; 
    }
    
    for ( ; DataLen ; DataLen--) 
    {
        dataread = I2CReadByte((DataLen == 1) ? NACK : ACK);         // receive byte(s)
	 *Data++ = dataread;
    }
    I2CStop();
    
    return 0;
}

void VD3I2CInit(FxU32 regBase)
{
    VidSerialParallel = (void *)(regBase + 0x78);
//  *VidSerialParallel |= (H3_VMI_ENABLE_MASK |  H3_VMI_CLOCK_MASK | H3_VMI_DATA_MASK);

    *VidSerialParallel &= 0x0D07FFFFF;  	   // Clear GPIO1 for BT and others
    *VidSerialParallel |= 0x083800000;  	   // Enable I2C interface

    I2cDelay15us (3);
    SetSCLLine();
    WaitHighSCLLine();
    SetSDALine();
}


int I2CSendByte(unsigned char byte)
{
  int  i;

  for (i=0; i<8 ; i++)
  {
    ResetSCLLine();
    if (byte & 0x80)
        SetSDALine();
    else
        ResetSDALine();
    SetSCLLine(); WaitHighSCLLine();
    byte <<= 1;
  }
  return (I2cAck());
}

unsigned char I2CReadByte(int ack)  {
    unsigned char data=0;
    unsigned char byte = 0;
    int i;
    
    ResetSCLLine();
    SetSDALine();   
    
    for (i=0; i<8 ; i++)
    {
        ResetSCLLine();
        ResetSCLLine();
        SetSCLLine();  WaitHighSCLLine();
        ReadSDA(&data);
        byte <<= 1;
        byte  |= (data == 1);
    }

    ResetSCLLine();
    if (ack)
    {
        SetSDALine();         // send Nack
    }
    else  {
        ResetSDALine();       // send Ack
    }
            
    SetSCLLine(); WaitHighSCLLine();
    ResetSCLLine();
    return byte;
}


void I2CStart()
{
    SetSDALine();
    SetSCLLine();
    WaitHighSCLLine();
    ResetSDALine();
    ResetSCLLine();
}

void I2CStop()
{
    ResetSCLLine();
    ResetSDALine();
    SetSCLLine();
    WaitHighSCLLine();
    SetSDALine();
}


void ReadSCL(unsigned char* data)
{
	_disable();
 	*data = (*VidSerialParallel & H3_VMI_CLOCK_STATE_MASK) ? 1:0;
	_enable();
}


void ReadSDA(unsigned char* data)
{                       
	_disable();
	*data = (*VidSerialParallel & H3_VMI_DATA_STATE_MASK) ? 1:0;
	_enable();
}

void SetSDALine()
{                                                          
	_disable();
	*VidSerialParallel = *VidSerialParallel | H3_VMI_DATA_MASK;
	_enable();
  	I2cDelay15us (3);
}


void ResetSDALine()
{
	_disable();
	*VidSerialParallel = *VidSerialParallel & ~H3_VMI_DATA_MASK;
	_enable();
  	I2cDelay15us (3);
}


void SetSCLLine()
{
	_disable();
	*VidSerialParallel = *VidSerialParallel | H3_VMI_CLOCK_MASK;
	_enable();
	I2cDelay15us (3);
}


void ResetSCLLine()
{
	_disable();
	*VidSerialParallel = *VidSerialParallel & ~H3_VMI_CLOCK_MASK;
 	_enable();
  	I2cDelay15us (3);
}
                                                 
                                                 
/*
 * waits for a specified line til it goes high
 * giving up after MAX_WAIT_STATES attempts
 * return:  0 OK
 *         -1 fail (time out)
 */

void WaitHighSCLLine()
{
    unsigned char data_in;
    FxU32  retries = 10;
    
    do
    {
        ReadSCL(&data_in);      // wait for the line going high
          if (data_in)
            break;
    } while (retries--);              // count down is running
}

/*
 * I2cAck() returns 1: fail
 *                  0: acknolege
 */

int I2cAck()
{
    unsigned char ack;
    ResetSCLLine();
    SetSDALine();
    SetSCLLine();
    ReadSDA(&ack);
    ResetSCLLine();
    return ((int)ack);
}


//
//		I2cDelay15us - Delay for a number of 15 microsecond intervals
//
//		Entry:	cLoop		Number of 15 microsecond intervals to delay
//		Exit:		None
//

// Useful I/O values
#define	IODELAYPORT		0xED		// Unused I/O port
#define	PORTB	   		0x61
#define	FLAG_REFRESHSTATUS	0x10		// Port B refresh signal status
BYTE _inp (WORD);

void I2cDelay15us (int cLoop)
{
  unsigned char byPBStat, byMaskState;

  if (cLoop <= 0) cLoop = 1;
  byPBStat = byMaskState = 0;
  while (cLoop--)
  {
  	while (byPBStat == byMaskState)
  	{
  	  _inp (IODELAYPORT);			// Read from an unused I/O port for delay
  	  byPBStat = _inp (PORTB);		// Get Port B status
  	  byPBStat = (unsigned char) (byPBStat & FLAG_REFRESHSTATUS);
  	}
  	byMaskState = byPBStat;
	}
}

