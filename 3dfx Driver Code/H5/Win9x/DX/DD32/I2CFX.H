#ifndef _I2CFX_H_
#define _I2CFX_H_

//typedef unsigned char   BYTE;
//typedef unsigned int    WORD;
//typedef unsigned long   DWORD;
typedef char * 			PSTR;

#define SCL                     	0x00
#define SDA                     	0x01   
#define LOCLK                   	0x00
#define HICLK                   	0x01

//I2C bitmasks
#define I2C_WRITE					0xfe		//LSBit 0
#define I2C_READ					0x01		//LSBit 1


#define I2C_REGSPERDEV          	120			//Number of registers

/*I2C Devices*/      
#define MAX_I2C_DEVICES				2			//We support 1 device only for now
#define DEV_SAA7110                	0x9e		//Slave address
#define DEV_BT829               	0x88		//Slave address


typedef struct 
{
    BYTE        Index;                  // Register index
    BYTE        Data;                   // Register data
} REGENTRY;

typedef struct 
{
    WORD        wRegCnt;                // Number of registers in device
    WORD        wDevAddr;               // Address of device
    REGENTRY    Regs[I2C_REGSPERDEV];   // One entry per register
} I2C_DEV;
				    
#endif
